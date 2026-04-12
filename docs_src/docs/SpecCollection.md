# SpecCollection — Import / Update across Collections (#750)

---

## Operations

| Operation | Scope | Notes |
|-----------|-------|-------|
| **Import** | Selected device(s) from source → target | Source is read-only. Selecting the collection root imports all devices and additionally includes tags. |
| **Update** | Re-sync previously imported device(s) from source | Uses stored cross-collection link; source reopens automatically. |

---

## Data coverage

| Table / data | Import | Update | Notes |
|---|:---:|:---:|---|
| `device` | ✅ | ✅ | All types (Catalog, Virtual, Storage). Update: source authoritative for all metadata. |
| `catalog` | ✅ | ✅ | Metadata + file indexes. |
| `file` | ✅ | ✅ | Full file list per catalog. |
| `folder` | ✅ | ✅ | Folder structure per catalog. |
| `catalog_filter` | ✅ | ✅ | Per-catalog exclude folders. Update: replaced from source. |
| `storage` (table row) | ✅ | ✅ | Imported via Catalog's associated storage. Update: full row replaced via `device_external_id → storage_id`. |
| `device_mapping` (CollectionImport) | ✅ | — | Created for every imported device (all types, except Physical root id=1); used by Update to locate source. |
| `device_mapping` (BackUp) | ✅ | ✅ | Existing BackUp mappings carried over. |
| `statistics_device` | ✅ | ✅ | Per-device statistics. |
| `tag` | ✅ (collection only) | ❌ | Tags imported only when the collection root is selected. Not updated. |
| `storage` image files | ❌ | ❌ | Not yet — storage picture files not transferred. |
| `parameter` (exclude dirs) | ✅ | ❌ | Import: `exclude_directory` rows copied, skipping duplicates by path. Update: not covered. |
| `parameter` (other: imageFolderPath, …) | ❌ | ❌ | Not imported — collection-level settings stay in the target collection. |

Source modes: **File** (`.db`) and **Memory** (CSV folder). Hosted source: use export first (see below).

---

## Source connection

Opened as a separate read-only connection named `"importSourceConnection"` alongside the active target.

| Source mode | Opening strategy |
|-------------|-----------------|
| File | `QSqlDatabase::addDatabase("QSQLITE", ...)` |
| Memory | In-memory SQLite, schema created via `Database::createAllTables()`, then CSV + `.idx` files loaded |
| Hosted | Not direct — export to File or Memory first, then import from the exported collection |

Schema version guard: abort if source and target `dbSchemaVersion` differ.

---

## ID remapping (Import only)

```
device_id_offset  = MAX(device_id)  in target + 1
catalog_id_offset = MAX(catalog_id) in target + 1
```

Cascading updates: `device.device_parent_id`, `device.device_external_id`, `catalog.catalog_id`, `file.file_catalog_id`, `folder.folder_catalog_id`, `catalog_filter.filter_catalog_id`.

Not needed for Update — device and catalog IDs already exist in the target.

---

## Cross-collection links (`device_mapping`)

One `mapping_type = 'CollectionImport'` row is created per Catalog-type device on Import:

| Column | Value |
|--------|-------|
| `mapping_device_source_id` | original `device_id` in source |
| `mapping_device_target_id` | new `device_id` in target |
| `mapping_source_collection` | path to source collection (File path or Memory folder) |

`mapping_source_collection` is NULL for BackUp-type rows. Persisted in `device.csv` for Memory-mode targets (included in `saveMappingTableToFile()` / `loadMappingFileToTable()`).

---

## Update algorithm (`updateDeviceFromExternalCollection`)

1. Recursive CTE on target `device_mapping` to collect all `CollectionImport` rows for the selected device and its descendants.
2. Auto-open source from stored `mapping_source_collection` — no user browse needed.
3. For each mapped device, update the device row from source (source authoritative for all metadata), then branch on type:
   - **Catalog**: delete file/folder/catalog_filter, re-insert from source, update catalog metadata.
   - **Storage**: UPDATE storage table row via `device_external_id → storage_id` (both source and target).
   - **Virtual** and other containers: device row update only.
4. Missing source device: skip with warning, continue remaining.
5. Single transaction; Memory-mode target: call `persistImportToFiles()` after commit.

Button enabled when selected target device **or any descendant** has a CollectionImport mapping.

---

## Name conflicts (Import)

| Scope | Default policy |
|-------|---------------|
| `catalog_name` (UNIQUE) | Rename: append ` (2)`, ` (3)`, … |
| `device_name` within parent | Rename |
| Virtual ancestor | Reuse existing if name matches |

---

## Ancestor chain

Importing a sub-device inserts its full ancestor chain as Virtual-type parents. Source `device_id = 1` (Physical group) always maps directly to target `id = 1` — never re-inserted.

---

## Memory mode specifics

**Source Memory**: load CSV + `.idx` files into `"importSourceConnection"` via `Collection::loadCatalogFilterFileToTable()` etc. — including `loadParameterFileToTable()` first (required for schema version check).

**Target Memory**: after any write operation, call `persistImportToFiles(collection)` before `loadCollection()` to persist device/mapping/filter CSVs. `.idx` files are written inside `insertFileData()`.

---

## Export modes (for Hosted source workaround)

| Source mode | → SQLite file | → Memory mode |
|-------------|:---:|:---:|
| Memory | ✅ `backupMemoryDatabaseToFile()` | — |
| File | — | ✅ `exportAllToMemoryMode()` + `exportAllCatalogFiles()` |
| Hosted | ✅ `exportToSQLiteFile()` (table copy) | ✅ same as File (temporarily fakes Memory mode) |

---

## Status reporting

`StatusBarMessageBuilder` in K2 UI. Operation names: `tr("COLLECTION IMPORT")` / `tr("COLLECTION UPDATE")`.
Progress driven by `importProgress(int current, int total, QString itemName)` signal.
Errors: `.setOperation(...).setStatus(tr("Error")).setCurrentItem(message)`.

---

## Key decisions

| # | Decision |
|---|----------|
| 1 | Partial import: full ancestor chain as Virtual parents; reuse by name match |
| 2 | Hosted source: not direct — export first |
| 3 | Schema mismatch: strict abort |
| 4 | Update scope: full sub-tree under selected device; respects device type (Catalog, Storage, Virtual) |
| 5 | Update ID remapping: none needed (IDs already in target) |
| 6 | Update — device row: source authoritative for all metadata |
| 7 | catalog_filter on Update: Replace from source |
| 8 | Update source: auto-opens from stored path; no manual browse |
| 9 | Manual mapping of existing devices (no prior Import): backlog |
| 10 | CollectionImport mappings saved in mapping CSV (not excluded) for Memory-mode persistence |
| 11 | Tags: imported only at collection-root level; not updated |
| 12 | parameter exclude dirs: imported (INSERT with duplicate check by path); other parameter rows (imageFolderPath, …) not imported — they are collection-level settings that belong to the target |

---

## Known bugs fixed

| Bug | Root cause | Fix |
|-----|-----------|-----|
| catalog_filter not imported | `insertCatalogFilter()` used `filter_path` instead of `filter_value` | Column name corrected; `INSERT OR IGNORE` added |
| CollectionImport mappings lost on reload (Memory target) | `saveMappingTableToFile()` excluded `CollectionImport` rows; `source_collection` column missing from CSV | Filter removed; column added to save/load cycle |
| Schema version check failed (Memory source) | `loadParameterFileToTable()` not called during `openSource()` | Added as first load step |
| Import completed but device lost (Memory target) | `loadCollection()` wiped in-memory tables before CSV was saved | `persistImportToFiles()` called before every `loadCollection()` |
| Physical group duplicated on import | `ensureAncestors()` tried to re-insert source `device_id = 1` | Early-exit: map source id=1 → target id=1 directly |
| Catalog path warning / wrong file deleted after import | `loadCatalogFilesToTable()` inserted into wrong connection | `setConnectionName()` called before insert; `insertFileData()` does full SELECT before `saveCatalogToFile()` |
