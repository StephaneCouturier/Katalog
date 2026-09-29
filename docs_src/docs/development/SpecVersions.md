# Version Numbers

## **Summary**

Katalog uses THREE different version numbers for different purposes:

| Version | Scope | Purpose | Updated When |
|---------|-------|---------|--------------|
| **Application Version** | Global | Running app version | Never (read-only) |
| **DB Schema Version** | Per database | Database structure | When schema changes |
| **Catalog Data Version** | Per catalog | Catalog data format | When catalog data migrated |

**Key Rule:** `catalog_app_version` should reflect the DATA FORMAT VERSION, not just which app created it.

A catalog is "v2.8" when its data includes all v2.8 fields, regardless of when it was created.

---

### 1. Application Version

**What it is:** The version of the Katalog application currently running

**Where it's stored:** 
- CMake: `KATALOG_VERSION_STRING` from `src/version.h.in`
- Runtime: `collection->appVersion` loaded at startup

**Value:** `"2.8"`, `"2.9"`, etc.

**Purpose:** 
- Displayed in "About" dialog
- Used to check for updates
- Reference for feature compatibility

**Updated:** Never (it's the current running version)

---

### 2. Database Schema Version

**What it is:** The version of the database structure (tables, columns, indexes)

**Where it's stored:** 
- Database: `parameter` table, `parameter_name = 'version'`
- Runtime: `collection->dbSchemaVersion`

**Value:** `"2.6"`, `"2.8"`, etc.

**Purpose:**
- **Triggers database migrations** in `runDatabaseMigrations()`
- Ensures database structure matches application requirements
- ONE-TIME migration per database

**Updated:** When database structure changes requiring migration

**Examples:**
- v2.6 → v2.8: Add columns to `file` table (`file_extension`, `file_type`, `mime_type`, etc.)
- Future: Add new tables, indexes, or columns

**Migration Trigger:**
```cpp
if (currentSchemaVersion < "2.8") {
    qDebug() << "Running database migration to 2.8...";
    Database::runMigration_2_8(m_connectionName);
    collection->dbSchemaVersion = "2.8";
    collection->setDatabaseSchemaVersion();
}
```

---

### 3. Catalog Data Format Version
(`catalog->appVersion` / `catalog_app_version`)

**What it is:** The version of the data format WITHIN each catalog

**Where it's stored:**
- Database: `catalog` table, `catalog_app_version` column (one per catalog)
- Memory: `.idx` file header
- Runtime: `catalog->appVersion`

**Value:** `"2.6"`, `"2.8"`, etc.

**Purpose:**
- **Definition:** Indicates to which database schema version the catalog is compliant and mandatory fields are populated in the catalog's files
- **Triggers catalog-specific migrations** when loading old `.idx` files
- **Shows catalog compatibility** in Devices/Catalog list (column 30, when "Display Full Table" is checked)

**Updated:** 
1. When a new Catalog is created (set to current app version)
2. When old Catalog is loaded and migrated

**Examples:**
- Catalog created in v2.6: `catalog_app_version = "2.6"`
  - Files have: `file_name`, `file_size`, etc. (6 columns)
  - Files DON'T have: `file_extension`, `file_type`, `mime_type`, metadata fields

- Catalog updated to v2.8: `catalog_app_version = "2.8"`
  - Database hase: All v2.6 fields PLUS new fields (28 columns)
  - Files have: `file_extension`, `file_type`, `mime_type` populated
  - Files MAY have: metadata fields (if `includeMetadata != "None"`)

**Migration Trigger:**
```cpp
// When loading .idx file
bool needsMigration = (appVersion < "2.8");
if (needsMigration) {
    // Convert 6-column format to 28-column format
    // Populate file_extension, file_type, mime_type from file names
    appVersion = "2.8";
    saveCatalogToFile();  // Save in new format
}
```

---

## **Relationship Between Versions**

```
Application v2.8
├─ Database Schema v2.8     (ONE per database)
│  └─ Ensures: file table has 28 columns
│
└─ Catalog A: v2.6          (ONE per catalog)
│  └─ Files: 6 columns, needs migration
│
└─ Catalog B: v2.8          (ONE per catalog)
   └─ Files: 28 columns, fully migrated
```

**Key Insight:** A database can have schema v2.8 but contain both:
- Old catalogs (v2.6 format) - need per-catalog migration
- New catalogs (v2.8 format) - already migrated

---

## **Use Cases**

### Use Case 1: User Upgrades from v2.6 to v2.8

**Initial State:**
- Application: v2.6
- Database Schema: v2.6
- Catalog A: v2.6 (in-memory `.idx` file, 6 columns)
- Catalog B: v2.6 (in-memory `.idx` file, 6 columns)

**User Actions:**
1. Install Katalog v2.8
2. Open collection

**What Happens:**
```
Step 1: Database Migration (ONE TIME)
├─ Detect: dbSchemaVersion (2.6) < appVersion (2.8)
├─ Run: Database::runMigration_2_8()
│  └─ Add columns to file/filetemp tables
├─ Update: collection->dbSchemaVersion = "2.8"
└─ Save to parameter table

Step 2: First Catalog Load (PER CATALOG)
├─ User searches or opens Catalog A
├─ Detect: catalog->appVersion (2.6) < current (2.8)
├─ Load .idx file (6 columns)
├─ Convert on-the-fly to 28 columns
│  └─ file_extension, file_type, mime_type = derived from filename
│  └─ All metadata fields = NULL
├─ Update: catalog->appVersion = "2.8"
└─ Save .idx file in new format

Step 3: Catalog Update (PER CATALOG)
├─ User updates Catalog A
├─ Step 8a: migrateMimeTypesForExistingFiles()
│  └─ Populate file_extension, file_type, mime_type for ALL files
├─ Step 9: Extract metadata (if enabled)
└─ [MISSING] Update: catalog->appVersion = "2.8"
```

**Final State:**
- Application: v2.8
- Database Schema: v2.8 ✓
- Catalog A: v2.8 ✓ (fully migrated)
- Catalog B: v2.6 (not yet loaded/used)

---

### Use Case 2: User Creates New Catalog in v2.8

**Actions:**
1. Click "Create New Catalog"
2. Select source path
3. Start creation

**What Happens:**
```
Step 1: Catalog Creation
├─ Scan filesystem
├─ Insert files with ALL fields populated:
│  ├─ file_extension, file_type, mime_type ✓
│  └─ metadata fields (if includeMetadata enabled) ✓
├─ Set: catalog->appVersion = collection->appVersion  // "2.8"
└─ Save catalog
```

**Result:**
- Catalog created with `catalog_app_version = "2.8"`
- All files have complete v2.8 format
- No migration needed

---

### Use Case 3: Display Catalog Version in UI

**Location:** Devices/Catalog list, column 30 (shown when "Display Full Table" is checked)

**Purpose:** User can see which catalogs are fully migrated

**Display:**
```
Name          | Type    | Files  | Size    | Updated    | App Version
--------------|---------|--------|---------|------------|-------------
Music Library | Catalog | 43,000 | 250 GB  | 2025-01-15 | 2.8         ✓
Photo Archive | Catalog | 12,000 | 180 GB  | 2024-11-20 | 2.6         ⚠️
```

**Interpretation:**
- **v2.8:** Catalog fully migrated, all features available
- **v2.6:** Catalog needs update to use new features (search by file type, metadata)

---

## **Current Implementation Status**


Helper Method in Catalog
```cpp
// In catalog.h
public:
    bool hasFilesNeedingMigration() const;
```

 Update Version After Operations Complete
```cpp
// In catalog.cpp - loadCatalogFileListToTable()
if (catalogWasMigrated) {
    qDebug() << "Catalog migrated, saving to v2.8 format...";
    appVersion = "2.8";  // Already doing this
    
    if (saveCatalogToFile("Memory", collectionFolder)) {
        qDebug() << "Catalog successfully saved in v2.8 format";
    }
    
    // NEW: After saving, check if migration is 100% complete
    if (!hasFilesNeedingMigration() && appVersion < "2.8") {
        appVersion = "2.8";
        saveCatalog();  // Update database
        qDebug() << "✓ Catalog fully migrated to v2.8";
    }
}
```

 Update After Search-Triggered Migration
```cpp
// In catalog.cpp - migrateCatalogFieldsForSearch()
// ... existing migration code ...

db.commit();
qDebug() << "Migration completed:" << processed << "/" << filesToMigrate;

// NEW: Check if this completed the migration
if (!hasFilesNeedingMigration() && appVersion < "2.8") {
    appVersion = "2.8";
    saveCatalog();
    qDebug() << "✓ Catalog fully migrated to v2.8 via search";
}
```

 Update After Update-Triggered Migration
 
 After Update-Triggered Migration
 
 SearchJobStoppable::searchFilesInCatalog
 ```cpp
                // Emit finished signal
                emit searchProgress(-3);

                qDebug() << "Migration completed";

                if (!device->catalog->hasFilesNeedingMigration() && device->catalog->appVersion < "2.8") {
                    device->catalog->appVersion = "2.8";
                    device->catalog->saveCatalog();
                    qDebug() << "✓ Catalog fully migrated to v2.8 via search";
                }
```

---

## **Compatibility with older versions**

What happens when a collection that has been opened by Katalog 3.0 is opened
again with an older application: Katalog 2.12 or earlier, or the K2 2.13 binary
published alongside the 3.0 betas. This section
states facts and risks; it does not add a requirement. The release notes link to
the user-facing list below.

### Schema number at release: 3.0

Decided by the user before this section was written, recorded here on
2026-09-27:

- **2.13 is only the beta working number** of the schema. At release, the target
  schema number is **3.0**.
- **Everyone runs the full 2.12 → 3.0 migration**, including beta testers whose
  databases are already stamped 2.13.
- **Consequence for development:** every migration step written during the
  2.13 cycle MUST be idempotent — it must run safely on a database already
  stamped 2.13, where some or all of its changes are already present (for
  example `SpecStorageIdentity.md` `STI-C16`, `STI-C17`, `STI-C19` and `STI-C20`, removing columns
  that a beta database may or may not still have). The migration is
  `Database::runMigration_3_0`; until release it runs on every open.

### Facts (verified by code reading, 2026-09-27)

- **2.12 has no newer-schema check.** The released 2.12 (tag `v2.12.1`,
  `core/databasemanager.cpp` `runMigrations`) only migrates schemas *older*
  than itself. It opens a 2.13 collection silently, and writes back only the
  columns it knows.
- **One core library, one migration.** The K2 built from the current tree
  (released together with 3.0) and K3 3.0 share the same core library and the
  same migration dispatcher. Both run the same unconditional column guards on
  open: `ensureDeviceCommentColumn`, `ensureStorageUserIdColumn`,
  `ensureMappingIncludeEmptyDirsColumn`, `ensureMappingSourceCollectionColumn`,
  and both run without `storage.storage_name` and `catalog.catalog_storage`,
  which the 2.12 → 3.0 migration drops (`SpecStorageIdentity.md` `STI-C13`,
  `STI-C16`). K2's only raw UI writes on these tables (storage update in
  `qt_widgets/mainwindow_tab_device_pr.cpp`, backup mapping insert in
  `qt_widgets/mainwindow_tab_backup.cpp`) include the 2.13 columns. So that K2
  and 3.0 can be used alternately on the same collection.
- **The K2 2.13 binary published with 3.0 beta2 is an older application.** It
  predates the column removal and names `storage_name` / `catalog_storage` in
  its SQL, exactly like 2.12 — see risk 5.
- **2.12 cannot be patched.** Anything that protects a collection from an older
  application must already be in that older application.

**Open option, not decided:** a newer-schema guard (refuse, or open read-only,
a collection whose schema is newer than the application) would protect
collections from *future* older versions only — for example a 3.x collection
opened by 3.0. It cannot protect against 2.12.

### Technical risks — a 3.0 collection used with K2 2.12 or older, or with the K2 2.13 beta2 binary

| # | What is lost or wrong | Modes | Triggered when | Severity |
|---|---|---|---|---|
| 1 | `device_order`: 2.12 inserts devices with an uninitialised `device_order`, so the device tree can show them in the wrong order | All | 2.12 creates a device | Low — known; guidance is acceptable |
| 2 | `storage_user_id`: 2.12's `storage.csv` writer has no UserID column (`v2.12.1` `core/collection.cpp` `saveStorageTableToFile`). On reopen in 2.13/3.0 the loader falls back to `storage_id` (`core/collection.cpp` ~745), so user-edited Storage IDs are lost | Memory | Any 2.12 save of the storage table (create, rename or update a storage) | High — the Storage ID is the number written on the physical disk |
| 2b | Storages created by 2.12 get `storage_user_id` = 0 (column default) | File / Hosted | 2.12 creates a storage | Medium |
| 3 | `device_comment`: 2.12's `device.csv` writer has 14 columns and no comment, so all device comments are lost | Memory | Any 2.12 save of the device table (nearly every device action) | High |
| 4 | `mapping_include_empty_dirs`: 2.12's backup mapping writer lacks it, so the setting reverts to its default 1 (replicate empty folders) | Memory | 2.12 saves backup mappings | Medium |
| 5 | `storage_name` / `catalog_storage` (`STI-C16`), `catalog_name` (`STI-C17`), `storage_path` / `catalog_source_path` / `catalog_source_path_is_active` (`STI-C19`), and `catalog_file_count` / `catalog_total_file_size` / `storage_total_space` / `storage_free_space` / `storage_location` (`STI-C20`), and `search.search_location` (never given a value; user decision 2026-09-29) are removed by the 2.12 → 3.0 migration. K2 ≤ 2.12 and the K2 2.13 binary published with beta2 name these columns in their SQL, so storage details fail to load, catalogs fail to load, and creating a storage, creating a catalog or saving a catalog fails | File / Hosted | Opening the collection after the 3.0 migration | High — accepted by the user: no return from K3 3.0 to older K2, expected for a major version |
| 5b | Memory-mode file formats are unchanged, so these applications still open the collection; but the storage "Name" column of `storage.csv` and the `<catalogStorage>` line of each `.idx` are written empty by 3.0. 2.12's Collection Import resolves a catalog's storage by `catalog_storage` → `storage_name` (`v2.12.1` `core/collectionimporter.cpp` ~1260-1290), so importing from such a collection finds no storage row | Memory | An older application imports from a collection saved by 3.0 | Low-medium |

### What this means for users

This is the list the release notes link to. Plain language, one entry per risk
above.

- If you use Katalog 2.12 or older and you add a device, then the new device can
  appear in the wrong place in the device list. *(risk 1)*
- If you use Katalog 2.12 or older with a collection saved in memory mode and you
  create, rename or edit a storage device, then the Storage ID numbers you
  entered are replaced by internal numbers. *(risk 2)*
- If you use Katalog 2.12 or older with a collection saved in a database file or
  on a database server and you create a storage device, then that storage device
  has no Storage ID. *(risk 2b)*
- If you use Katalog 2.12 or older with a collection saved in memory mode and you
  change almost anything about your devices, then all device comments are
  lost. *(risk 3)*
- If you use Katalog 2.12 or older with a collection saved in memory mode and you
  save your backup settings, then every backup goes back to copying empty
  folders. *(risk 4)*
- If you use Katalog 2.12 or older, or the Katalog 2.13 published with the 3.0
  beta, with a collection saved in a database file or on a database server
  after Katalog 3.0 opened it, then storage details and catalogs do not load,
  and creating a storage device or a catalog, or saving a catalog, fails. *(risk 5)*
- If you use Katalog 2.12 or older, or the Katalog 2.13 published with the 3.0
  beta, and you import from a collection saved in memory mode by Katalog 3.0,
  then the storage device details of the imported catalogs can be
  missing. *(risk 5b)*

**Recommendation:** a collection opened with Katalog 3.0 is best used only with
Katalog 3.0, or the Katalog 2 released together with it, from then on, and a
backup copy of it is needed before it is opened with an older version.

---
