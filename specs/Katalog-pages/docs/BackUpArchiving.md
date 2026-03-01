# Archiving

## 1. Core Concepts & Definitions

| operation | file process  |  Purpose                                                         |
|-----------|---------------|------------------------------------------------------------------|
| **Backup**    | A **copy** operation from a source catalog to a target catalog. | The source is never modified. The goal is redundancy and recovery. <br/> Multiple strategies will exist over time (full, incremental, sync). |
| **Archive**   | A **move** operation from a source catalog to a target catalog. | Files are transferred and then removed from the source once the copy is verified. The goal is long-term offload and storage organisation. Empty directories left behind in the source are optionally cleaned up. |

---

## 2. Shared Infrastructure

Both features share the same foundational layer (mappings).

### 2.1 Catalog Mapping (`device_mapping` table)

The existing table already supports both feature types via the `mapping_type` field, which currently stores `"Backup"`. Archive mappings will store `"Archive"`. No schema changes are required for the type distinction itself.

Two new optional columns are added to support Archive-specific and future Backup options:

| Column                | Type    | Default | Purpose                                                         |
|-----------------------|---------|---------|-----------------------------------------------------------------|
| `mapping_archive_mode`  | TEXT    | NULL    | Reserved for future use (currently always `"Move"` for Archive)   |
| `mapping_reindex_after` | INTEGER | 1       | Whether to re-index both catalogs after operation (0=no, 1=yes) |

Backup mappings leave these NULL or use default values. This keeps the schema forward-compatible without breaking existing data.

### 2.2 `DirectoryReplicator` — Shared Core Class

A new backend-only class, usable by Backup, Archive, and the future Classify feature.

**Responsibilities:**

- Walk the source catalog's directory tree, using data already indexed in the database (no live filesystem scan needed for structure)
- Create all missing directories on the target in a single batched pass before any file operation begins — replacing the current per-file `mkpath` pattern
- Optionally prune directories in the target that have no matching source directory (orphan removal)
- Support a dry-run mode for preview
- Return a structured result (directories created, skipped, removed, errors)

**Depth limiting** is an option on this class but is scoped to the future **folder management** feature, not to Backup or Archive in their initial releases. It is included in the class design for completeness but not exposed in the Backup or Archive UI at this stage.

### 2.3 Folder Selection Model (shared, phased)

A common mechanism to define a sub-set of folders within a catalog mapping, usable by both Backup and Archive. This is a two-phase addition:

**Phase 1 (first release):** No selection — the entire catalog path is used as source, the entire mapped target path as destination. Full directory structure is replicated.

**Phase 2 (subsequent release):** A folder selection tree, stored per mapping, allows the user to check/uncheck individual folders before running the operation. This selection is persisted in the database linked to the mapping and is shared between Backup and Archive (same storage mechanism, same UI component).

For Phase 2, a new table `mapping_folder_selection` is anticipated:

| Column                | Type       | Purpose                         |
|-----------------------|------------|---------------------------------|
| `selection_id`          | INTEGER PK | Unique ID                       |
| `selection_mapping_id`  | INTEGER FK | Links to `device_mapping`         |
| `selection_folder_path` | TEXT       | Relative path from catalog root |
| `selection_included`    | INTEGER    | 1=included, 0=excluded          |

---

## 3. File Operation Engine

Shared by both features, parameterised by the mapping type.

| Behaviour                                | Backup                  | Archive                             |
|------------------------------------------|-------------------------|-------------------------------------|
| Copy files                               | Yes                     | Yes (as first step)                 |
| Verify copy (checksum)                   | Optional                | Mandatory before delete             |
| Delete source files after copy           | No                      | Yes                                 |
| Remove empty source dirs after move      | No                      | Optional (user setting per mapping) |
| Skip files already present and identical | Optional                | Optional                            |
| Re-index source and target catalogs      | Optional (default: yes) | Optional (default: yes)             |

**Checksum strategy:** MD5 or xxHash comparison on the copied file vs. original before deletion in Archive mode. If checksums do not match, the source file is kept and an error is logged.

**Conflict handling:** If a file already exists at the target, the user's per-mapping setting applies:

- Skip (keep existing target file)
- Overwrite unconditionally
- Overwrite if source is newer (date comparison)

---

## 4. Operation Result & Logging

After every Backup or Archive run, a structured result is produced and stored, linked to the mapping:

- Files copied / moved / skipped / failed
- Directories created / already existing / removed (orphan pruning)
- Total size transferred
- Duration
- Checksum errors (Archive only)
- Date and time of the operation (updates `mapping_backup_last_date` and `mapping_backup_last_size` in the existing table)

---

## 5. UI Architecture

### 5.1 Single Tab with Type Filter

All mapping management and operations remain in the existing **BackUp** tab (which may be renamed **BackUp / Archive** in the tab label).

At the top of the tab, a type selector filters what is displayed:

```
[ All ]   [ BackUp ]   [ Archive ]
```

The mapping table shows all mappings or only those of the selected type. A **Type** column displays the mapping type, colour-coded:

- **Blue** for Backup
- **Orange** for Archive

### 5.2 Mapping Panel (shared, unchanged structure)

The existing panels for source device list, target device list, mapping name input, and the comparison statistics area are reused without structural change. When creating a new mapping, the user selects the type (Backup or Archive) before saving — a simple radio button or dropdown added to the existing mapping creation form.

### 5.3 Operation Panel (type-specific, shown below the mapping table)

When a mapping is selected in the table, the operation panel below adapts to the mapping type:

**For a Backup mapping:**

- Summary of last run (date, size, file count)
- Options: skip identical files, re-index after (checkbox, default checked)
- Action: **Run Backup**

**For an Archive mapping:**

- Summary of last run
- Options: remove empty source dirs after move (checkbox), skip identical files, re-index after (checkbox, default checked)
- Action: **Preview** (dry run, shows what would happen) then **Run Archive**

The Preview step is specifically valuable for Archive because files will be deleted from source — the user should see what will move before committing.

### 5.4 Future: Folder Selection UI (Phase 2)

A folder tree panel, positioned between the mapping table and the operation panel, appears when a mapping is selected and allows checking/unchecking of folders. This panel is shared between Backup and Archive — same component, same persistence mechanism.

---

## 6. Phased Delivery Summary

| Phase   | Scope                                                                                                                                                                                                                                                       |
|---------|-------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------|
| **Phase 1** | `DirectoryReplicator` class (used by Backup to replace per-file `mkpath`); Archive mapping type in `device_mapping`; Archive full-catalog move operation with checksum verify; orphan directory pruning option; UI type filter and colour coding; re-index option |
| **Phase 2** | Folder selection model and UI (shared between Backup and Archive); depth limiting exposed in folder management feature                                                                                                                                      |
| **Phase 3** | Backup incremental / sync strategies; scheduling / auto-trigger on device connect; retention policies for Archive                                                                                                                                           |

---

## 7. Files Affected (Phase 1)

| File                                 | Change                                                                                 |
|--------------------------------------|----------------------------------------------------------------------------------------|
| `device_mapping` schema                | Add 2 optional columns                                                                 |
| `collection.cpp`                       | Schema migration, save/load mapping with new columns                                   |
| New file: `directoryreplicator.h/.cpp` | New shared core class                                                                  |
| `mainwindow_tab_backup.cpp`            | Type filter UI, colour coding, Archive operation panel, mapping creation type selector |
| `mainwindow.h`                         | New method declarations                                                                |
| UI form                              | Type selector radio buttons, Archive operation panel widgets                           |
| Documentation (`BackUp.md`)            | Update to reflect Archive addition                                                     |
