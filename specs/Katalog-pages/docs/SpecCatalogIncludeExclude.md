# Spec: Catalog Include / Exclude Rules

## Context

Katalog currently supports **global directory exclusions** only — a single list that applies to every catalog during creation and update. This spec defines the evolution toward **per-catalog include/exclude rules**, drawing on LuckyBackup's mature filter system as a reference.

---

## Current State

| Aspect | Current behaviour |
|--------|-------------------|
| Scope | Global — applies to all catalogs |
| Storage | `parameter` table, `parameter_type = 'exclude_directory'`, `parameter_value1 = 'All catalogs'` |
| Where managed | Create tab (add/delete), Settings tab (view) |
| What is supported | Directory paths only — no wildcards, no file patterns |
| Applied in | `CatalogJobStoppable` during catalog build |

---

## LuckyBackup Reference — Feature Map

LuckyBackup is an rsync frontend; its filter system is the rsync include/exclude engine. The table below maps each feature to Katalog's context.

### Exclude

| Feature | LuckyBackup | Katalog relevance |
|---------|-------------|-------------------|
| Exclude specific directory | ✅ user-defined | Already exists (global) → move to per-catalog |
| Exclude by filename pattern (`*.tmp`, `*.bak`) | ✅ rsync glob | New — high value |
| Template: temp folders (`**/*tmp*/`) | ✅ checkbox | New — quick-select |
| Template: cache folders (`**/*cache*/`) | ✅ checkbox | New — quick-select |
| Template: `lost+found` | ✅ checkbox | Useful for full-device catalogs |
| Template: system folders (`/var /proc /dev /sys`) | ✅ checkbox | Linux-specific, optional |
| Template: hidden virtual dirs (`.gvfs`) | ✅ checkbox | Partially covered by Include Hidden option |
| Read exclusions from a file | ✅ | Backlog |

### Include

| Feature | LuckyBackup | Katalog relevance |
|---------|-------------|-------------------|
| "Only include" mode — whitelist specific paths/patterns | ✅ | New — useful for partial catalogs |
| "Normal include" — override specific excludes | ✅ | Complex; backlog |

### Patterns

LuckyBackup uses rsync glob syntax. Katalog will use a simplified subset (Qt `QDir::match()` style):

| Pattern | Meaning | Katalog support |
|---------|---------|-----------------|
| `*.ext` | Any file with that extension | ✅ Phase 1 |
| `name` | Any file/folder named exactly `name` anywhere in tree | ✅ Phase 1 |
| `/name` | Anchored to catalog root | Phase 2 |
| `name/` | Directories only | Phase 2 |
| `**/name` | Anywhere in tree (recursive) | Phase 2 |
| `?` | Single character (not slash) | Phase 2 |
| `[a-z]` | Character class | Backlog |

---

## Proposed Scope — Phase 1

### 1. Per-catalog exclude folders
Move the current global folder exclusions to be **per-catalog**. A catalog's own exclusion list overrides (or extends) the global list during its build/update.

**Storage**: extend `parameter` table with a catalog reference, or add a dedicated `catalog_filter` table:

| Column | Type | Description |
|--------|------|-------------|
| `filter_id` | INTEGER PK | |
| `filter_catalog_id` | INTEGER FK → `catalog` | NULL = global |
| `filter_type` | TEXT | `'exclude_folder'`, `'exclude_pattern'`, `'include_only'` |
| `filter_value` | TEXT | Path or glob pattern |

### 2. Per-catalog exclude file patterns
Pattern matching by filename or extension: `*.tmp`, `*.log`, `Thumbs.db`.
Implemented in `CatalogJobStoppable` using `QDir::match()`.

### 3. Quick-select templates
Checkboxes for common categories that generate standard patterns automatically:
- **Temp folders** → adds pattern `*tmp*` (folder)
- **Cache folders** → adds pattern `*cache*` (folder)
- **Backup files** → adds pattern `*.bak`, `*~`

---

## Future / Backlog

- **"Only include" mode** — whitelist: catalog only indexes files/folders matching the list
- **Normal include** — override specific excludes (rsync-style ordered rules)
- **Read patterns from file** — point to a `.gitignore`-style file
- **Pattern editor GUI** — visual builder for non-trivial patterns
- **Size filter** — skip files above/below a size threshold (e.g. skip files > 4 GB)
- **Date filter** — only index files newer than a given date
- **Global template defaults** — admin-level defaults applied when creating a new catalog

---

## UI Placement

### Selected approach: Devices tab → Catalog edit form — left/right split

The `Devices_widget_EditCatalogFields` widget (File Type, Metadata, Checksum, Hidden Files) is split into two horizontal panels within the existing edit area:

**Left panel** — existing catalog settings (unchanged):
- File Type, Include Metadata, Include Checksum, Include Hidden Files, Is Full Device

**Right panel** — new Include / Exclude section (per-catalog):
- Excluded folders list
- Add / remove entry (path or glob pattern)
- *[Phase 2]* Quick-select template checkboxes
- *[Phase 2]* "Only include" mode toggle + include list

### Alternatives considered

| Option | Assessment |
|--------|-----------|
| Separate sub-tab inside Devices tab | Requires TabWidget, adds navigation depth — rejected |
| Settings tab (global only) | Already exists for global list — keep as global view |
| Modal dialog via button on edit form | Simpler but hides the feature — rejected for Phase 1 |

---

## Migration Notes

- New `catalog_filter` table introduced in the current dev version (2.10, unreleased) — can be created directly in the 2.10 migration, no additional migration step needed.
- Existing global exclusions in `parameter` remain as-is; per-catalog rules are additive.
- `CatalogJobStoppable` needs to load both global and per-catalog rules before traversal, applying exclusions in order: catalog-level first, then global.
