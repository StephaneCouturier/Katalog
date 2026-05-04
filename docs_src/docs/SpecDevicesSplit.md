# DEVICES: Split a Catalog

![Draft](https://img.shields.io/badge/Status-Draft-orange) ![Implemented](https://img.shields.io/badge/Implementation-complete-brightgreen)

## Context

A catalog is a flat snapshot of an entire directory tree. As collections grow, a single large catalog may mix unrelated content (photos, videos, documents, code) or span a very deep directory hierarchy. Users sometimes want finer-grained catalogs — one per content type or one per top-level subdirectory — to improve browsing, filtering, and virtual-device organisation.

This spec defines two split operations accessible from the Devices context menu on any Catalog-type device.

---

## Current State

| Aspect | Current behaviour |
|--------|-------------------|
| Catalog granularity | One catalog = one full directory tree snapshot |
| File type filter | Each catalog already has a `fileType` field (Audio / Image / Video / Text / Other / All) that restricts what is indexed at creation time |
| Sub-directory filter | Available in Explore tab only; no sub-tree catalog |
| Context menu | No split action |

---

## Feature: Two Split Modes

Both modes share the same entry point and overall flow. They differ only in the grouping criterion.

### Trigger

Right-click a **Catalog-type device** in the Devices list → context menu:

- **Split Catalog per sub directory**
- **Split Catalog per file type**

Both entries are only shown when the selected device is of type `Catalog`; they are hidden for all other device types.

---

## Mode A — Split per Sub Directory ![Implemented](https://img.shields.io/badge/status-implemented-brightgreen)

### Definition

Each **immediate child directory** of the catalog's root becomes a separate catalog containing all files and folders under it (the full sub-tree, not just one level deep).

Files located **directly at the catalog root** (not inside any subdirectory) are collected into a dedicated catalog.

### Catalog Naming

| Group | New catalog name |
|-------|-----------------|
| Files at root | `[CatalogName]_(root)` |
| Sub-directory `Photos` | `[CatalogName]_Photos` |
| Sub-directory `Videos` | `[CatalogName]_Videos` |

If two immediate subdirectories share the same name (impossible on a real filesystem but possible if the catalog was built from merged sources), a numeric suffix is appended: `[CatalogName]_Photos_2`.

### Empty Groups

A catalog is created for every immediate subdirectory, even if it contains no files (empty folder tree).

If the source catalog has no subdirectories at all, the operation does nothing — no new catalogs are created and the original is left unchanged.

### Depth

Only the first level of subdirectories is used as the split criterion. All descendants of a first-level directory go into that directory's catalog.

### Root files

Files located directly at the catalog root (not inside any subdirectory) are collected into a dedicated catalog named `[CatalogName]_(root)`. The opening parenthesis sorts before any letter or digit, so this catalog always appears first in alphabetical order.

This catalog is created with `includeSubDir = false` so that a subsequent update only re-scans the root level and does not pick up files from subdirectories (which belong to the other split catalogs).

---

## Mode B — Split per File Type ![Implemented](https://img.shields.io/badge/status-implemented-brightgreen)

### Definition

Each split catalog is created from the same `sourcePath` as the original, with its `fileType` field set to the corresponding type. This reuses the existing catalog `fileType` mechanism — the split is essentially automating the creation of N catalogs that a user could otherwise create manually by selecting a file type at catalog-creation time.

Each file is assigned to exactly one catalog based on its MIME-type category, using the same classification as `FileTypeMapping::UserFileType`:

| Category | Catalog suffix | Coverage |
|----------|---------------|----------|
| Audio | `_(Audio)` | MIME type `audio/*` |
| Image | `_(Images)` | MIME type `image/*` |
| Video | `_(Videos)` | MIME type `video/*` |
| Text | `_(Text)` | Documents, code, scripts, data files, web content, ebooks |
| Other | `_(Other)` | Files with extensions not matched above, plus extensionless files (`None`) |

The `None` type (extensionless files) is merged into **Other** rather than getting its own catalog — extensionless files are rare and `Other` is already the "everything else" bucket. A future option could split them out if needed.

The **Other** category therefore acts as the exhaustive catch-all, so every file is guaranteed to fall into exactly one catalog. No file is lost or duplicated.

### Pre-split: Verify MIME Types

File types stored in the catalog are initially derived from file extensions, which can be inaccurate (e.g. a `.jpg` that is actually a video, or an extensionless binary). The existing **Verify MIME Types** operation reads the actual file content from disk and updates the `file_type` field in the database to the correct value.

Before executing the split, the UI must offer the user the choice to run this verification first:

> **Verify file types before splitting?**
> File types in the catalog may be based on extensions only. Running a verification reads each file from disk and ensures the split uses accurate types. The device must be connected.
>
> **[Verify then Split]** · **[Split without verifying]** · **[Cancel]**

| Choice | Behaviour |
|--------|-----------|
| Verify then Split | Run `VerifyMimeTypes` on the catalog; on completion, proceed with the split |
| Split without verifying | Proceed immediately using the types already stored in the catalog |
| Cancel | Abort — no changes made |

If the device is not connected (source path not accessible), **Verify then Split** is disabled and a note explains why.

### Catalog Naming

`[CatalogName]_(Audio)`, `[CatalogName]_(Images)`, etc. (suffixes as in the table above). Parentheses make it unambiguous that the suffix denotes a split criterion, not an actual subdirectory name.

### Empty Groups

A catalog is created for every type, even if it contains no files. This is consistent with Mode A.

### Text sub-categories

All six `TEXT` sub-categories (Documents, Code, Scripts, Data, Web, Ebooks) are grouped together into a single `[CatalogName]_(Text)` catalog. No further split by text sub-category.

---

## Common Behaviour (Both Modes)

### Confirmation

Before executing, a confirmation dialog is shown:

> **Split catalog "[CatalogName]"?**
> This will create N new catalogs and remove the original.
> This operation cannot be undone.

The user must confirm before the split proceeds.

### Original Catalog

After a successful split, **the original catalog is deleted**. The split catalogs replace it. The user can verify the result before triggering any further action; if they want to keep the original, they should duplicate it first.

### Source Path

| Mode | `sourcePath` of each split catalog |
|------|-------------------------------------|
| Mode A — per sub directory | The path of the corresponding subdirectory (e.g. `/mnt/drive/MyFiles/Photos`). The `(root)` catalog keeps the original `sourcePath`. |
| Mode B — per file type | The original `sourcePath` unchanged; the `fileType` field restricts what is indexed on the next update. |

### Virtual Device Assignment

If the original catalog was assigned to a **virtual device**:

| Situation | Behaviour |
|-----------|-----------|
| Original catalog was the only catalog on the virtual device | All split catalogs are assigned to the same virtual device |
| Original catalog was one of several on the virtual device | All split catalogs are added to the virtual device; the original entry is removed |

This preserves the virtual device's logical grouping.

### Physical Device Assignment

If the original catalog was attached directly to a physical (Storage or Drive) device, the split catalogs are attached to the same physical device.

### Progress

For large catalogs the split may take a few seconds. A progress indicator should be shown in the status bar.

---

## Impact Summary

| Component | Change | Status |
|-----------|--------|--------|
| UI — Devices context menu | Two new entries, enabled only for Catalog-type devices | done |
| Core — `Catalog::executeSplitBySubDirectory()` | Returns `QList<Catalog*>` of new catalogs | done |
| Core — `Catalog::executeSplitByFileType()` | Returns `QList<Catalog*>` of new catalogs | done |
| Core — `Collection::executeSplitBySubDirectory(Device*)` | Full operation: preload + split + device tree update | done |
| Core — `Collection::executeSplitByFileType(Device*)` | Full operation: preload + split + device tree update | done |
| Core — virtual device reassignment | `Collection::applySplitResult` re-assigns splits to all virtual devices the original was on; `Device::deleteDevice` cleans up orphan assignment rows | done |
| Core — virtual assignment redirect | `Collection::resolvePhysicalDevice` redirects split to primary row when triggered from a virtual assignment | done |
| `Catalog` field | New `includeSubDir` bool (default `true`); set to `false` for the `(root)` catalog in Mode A | done |
| `CatalogJobStoppable` | Respects `includeSubDir` when setting `QDirIterator` flags | done |
| Database | New column `catalog_include_sub_dir INTEGER DEFAULT 1` — added to migration 2.12 and CREATE TABLE | done |

---

## Suggested Implementation Order

| Step | Description | Status |
|------|-------------|--------|
| 1 | Add context menu entries | done |
| 2 | Implement `Catalog::executeSplitBySubDirectory` in core | done |
| 3 | Wire up sub-directory split end-to-end | done |
| 4 | Implement `Catalog::executeSplitByFileType` in core | done |
| 5 | Wire up file-type split end-to-end with verify dialog | done |
| 6 | Handle virtual device reassignment | done |
| 7 | Redirect split to physical device when triggered from virtual assignment | done |
