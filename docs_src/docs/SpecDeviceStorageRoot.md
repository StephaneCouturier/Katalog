# Device/Storage/Root

## Context

When a Storage device's mount point or drive letter changes (e.g. `/mnt/drive1` → `/mnt/drive01`, or `F:\` → `G:\`), all child Catalog `sourcePath` values and every indexed file/folder path become stale. The current fallback is a full re-scan via `updateDeviceHierarchy()`, which reads every file from disk and fails when the device is offline.

**Goal:** a fast, disk-free alternative that performs a prefix-replace pass on the database (and `.idx` files in Memory mode), without requiring the device to be mounted.

---

## Trigger

In `saveDeviceForm()` (~line 617 of `mainwindow_tab_device_pr.cpp`), inside the existing block:

```cpp
if (activeDevice->type != "Catalog" && activeDevice->path != previousPath)
```

When `activeDevice->type == "Storage"`, instead of immediately calling `updateDeviceHierarchy()`, branch to the new flow described below.

---

## UX — integration into the existing progress panel

No separate dialog. The existing Devices progress panel (`setCatalogUpdateUIState`) is reused. The choice is presented as a `QMessageBox::question` before the operation starts, then the chosen operation runs inside the same update-state frame (buttons disabled, stop enabled, wait cursor).

```
Storage path changed
Old path: /mnt/drive1
New path: /mnt/drive01

How should the catalog indexes be updated?

[ Update path prefix ]   [ Full re-scan ]   [ Skip ]
```

| Choice | Behaviour |
|--------|-----------|
| **Update path prefix** | New `"replaceRoot"` operation (see below) |
| **Full re-scan** | Existing `updateDeviceHierarchy(..., "update")` unchanged |
| **Skip** | `saveDeviceForm()` continues; indexes remain stale |

---

## Scope: what changes

Only direct Catalog-type children of the changed Storage device. (Storage cannot contain Storage — no recursion needed.)

| Table | Column(s) | Change |
|-------|-----------|--------|
| `catalog` | `catalog_source_path` | Prefix replace: `oldRoot` → `newRoot` |
| `file` | `file_full_path`, `file_folder_path` | Prefix replace for all rows in affected catalogs |
| `folder` | `folder_path` | Prefix replace for all rows in affected catalogs |
| `storage` | `storage_path` | Set to `newRoot` (currently skipped by `saveDeviceForm`) |

`catalog_file_path` is **not** touched — the collection location is independent of the device mount point.

---

## Memory mode handling

`saveCatalogToFile()` already serialises the in-memory SQLite tables back to `.idx` / `.folders.idx`. The sequence for each affected catalog in Memory mode:

1. If not yet loaded (`dateLoaded < dateUpdated`): call `catalog->loadCatalogFileListToTable()` and `catalog->loadFoldersToTable()`
2. Run the SQL prefix-replace UPDATEs on the in-memory database
3. Update `catalog->sourcePath` on the C++ object
4. Call `catalog->saveCatalogToFile("Memory", collectionFolder)` — rewrites both `.idx` (with the updated `<catalogSourcePath>` header and corrected `file_full_path` rows) and `.folders.idx`

In File/Hosted mode, steps 1 and 4 are skipped — the SQL UPDATE is sufficient.

---

## New core method

**Location:** `core/device.h` / `core/device.cpp`

```cpp
struct StorageRootReplaceResult {
    int catalogsUpdated;
    int filesUpdated;
    int foldersUpdated;
};

StorageRootReplaceResult Device::replaceStorageRootInIndexes(
    const QString& oldRoot,
    const QString& newRoot,
    const QString& connectionName,
    const QString& databaseMode,
    const QString& collectionFolder);
```

Implementation steps:
1. Walk `subDevices` to collect all Catalog-type children
2. For each catalog: load if needed (Memory mode), run SQL UPDATEs, save back to file (Memory mode)
3. Update `storage.storage_path`
4. Return counts

SQL pattern (parameterized, applied per catalog):

```sql
UPDATE file
SET file_full_path    = :newRoot || SUBSTR(file_full_path,    LENGTH(:oldRoot)+1),
    file_folder_path  = :newRoot || SUBSTR(file_folder_path,  LENGTH(:oldRoot)+1)
WHERE file_catalog_id = :catalogId
  AND file_full_path LIKE :oldRootPattern;

UPDATE folder
SET folder_path       = :newRoot || SUBSTR(folder_path, LENGTH(:oldRoot)+1)
WHERE folder_catalog_id = :catalogId
  AND folder_path LIKE :oldRootPattern;

UPDATE catalog
SET catalog_source_path = :newRoot || SUBSTR(catalog_source_path, LENGTH(:oldRoot)+1)
WHERE catalog_id = :catalogId
  AND catalog_source_path LIKE :oldRootPattern;
```

Where `:oldRootPattern` = `oldRoot + "%"`.

---

## New operation type in `DeviceUpdateManager`

Add `"replaceRoot"` as a valid `m_updateType`. The manager:
- Calls `rootDevice->replaceStorageRootInIndexes(...)` (synchronous — fast, no disk reads except Memory mode `.idx` writes)
- Emits `operationStarted` / `operationFinished` / `operationError` as usual
- Reports the result via `statusBarLabel`: *"N catalogs updated — X files, Y folders"*

---

## Edge cases

| Case | Handling |
|------|---------|
| `oldRoot` not a prefix of a catalog's `catalog_source_path` | Skip that catalog silently |
| Memory mode `.idx` file not found on disk | Log warning, skip catalog, count as failed |
| `oldRoot == newRoot` | Blocked upstream — `saveDeviceForm` already checks `path != previousPath` |
| Windows path separators | Normalize both paths with `QDir::fromNativeSeparators()` before the replace; re-apply `QDir::toNativeSeparators()` when writing back |

---

## Out of scope / deferred

- **Undo** — user can re-save the device with the old root and run the prefix replace again
- **Partial path rename** (only a subfolder renamed, not the root) — full root replacement only
- **`.idx` file patching for hosted mode** — hosted mode uses direct SQLite; no `.idx` files
