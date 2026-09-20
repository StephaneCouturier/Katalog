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

