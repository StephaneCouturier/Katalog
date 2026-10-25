---
id: SpecCatalogChecksum
title: "Catalog: File Checksum"
description: "Specification of file checksum features in catalogs"
---

# Catalog: File Checksum

## **Introduction**
This document specifies the implementation of features using **File checksums**.

### Benefits
- Data Integrity Verification: Checksums can be used to verify the integrity of files, ensuring that files have not been corrupted or altered.
- Duplicate/Differences File Detection: By comparing checksums, users can identify duplicate files or differences, independantly of file names, size, dates.
- Error Detection: Checksums can help detect errors that may occur during file transfer or storage.
- Security: Checksums can provide a basic level of security by detecting unauthorized changes to files.


### Available Algorithms & features

![Available](https://img.shields.io/badge/Status-Available-green) ![2.9](https://img.shields.io/badge/Version-2.9-green) ![664](https://img.shields.io/badge/github-664-blue?logo=github)

Katalog has currently 1 single algorithms **SHA-256** selected for balance (secure, fast, widely supported, 256-bit/32 bytes).


### Roadmap

![Planned](https://img.shields.io/badge/Status-Planned-orange) ![3.1](https://img.shields.io/badge/Version-3.1-blue) ![824](https://img.shields.io/badge/github-824-blue?logo=github)
**#824 (minor) SEARCH/EXPLORE: individual MD5 calculation** — K3 only. Calculate and show the MD5 of a single file from the file context menu; not stored. Specified below in [#824 - Individual MD5 calculation](#824---individual-md5-calculation).

![Backlog](https://img.shields.io/badge/Status-Backlog-lightgrey) ![825](https://img.shields.io/badge/github-825-blue?logo=github)
**#825 (major) Collection MD5 calculation** — calculate and store MD5 for catalog files, like SHA-256 today. Storage: Strategy A (dedicated columns), see [Increment 2 - Decision](#decision-strategy-a-retained-2026-10-25).

![Backlog](https://img.shields.io/badge/Status-Backlog-lightgrey) ![826](https://img.shields.io/badge/github-826-blue?logo=github)
**#826 (major) Compare checksums to an external source** — compare stored checksums to an external MD5 or SHA-256 source. Relies on the Strategy A column names to know the algorithm.


## **Architecture & Feature steps**

**Increment 1: add catalog option, single algo, option in Search Duplicates/Differences**

* 1 algorithm set for the app (not changeable)
* 1 algo to start with, Qt supported, to be selected among: Retained **SHA-256**
* 1 new "file" column: checksum_sha256
* Like for Metadata, creating or searching with this option shall remain stoppable
* Like for Metadata, the process shall be able to "resume/continue" so only addressing file that do not have a checksum calculated yet.
* Feature: SEARCH: find duplicates  based on Checksum  (combine with other options: name, size, date )
* Feature: SEARCH: find differences based on Checksum  (combine with other options: name, size, date )
* Feature: DEVICES/Catalogs: change detection (verify checksum of catalog files) (option on context menu, for catalogs only and if checksum is not "None")
* Feature: SEARCH: False duplicates: combinaison of name, size, date that are the same, but different checksum

**Increment 2: more Qt algos, per collection, at least 2 checksums per file (to support algo transition)**
* 1 algorithm can be set per Collection. Choice saved using paramter table:  parameter_name = "checksum_algorithm", parameter_type="collection", parameter_value1="MD5"
(not for the full app: different collection may have different purpose and size)
(not per catalog: it would make the search features useless if different algo across catalogs)
* 1 new column: file_checksum_2  (for second checksum)
if changing policy, keeping the old checksum allows to check the old file for integrity before switching to new format
* any avalaible algo from Qt can be used
* features: management of algo transitions (ex: warnings if use changes the collection_file_checksum_algorithm, transition coverage report, etc.)

**Increment 3: further librairies, more checksums per file**

To consider Indexing and Checksum work, within incremental update design<br/>
* Like for Metadata, it is an option
* If catalog can offer several checksum engines, the user shall be able to select the engine
* several engines for the same catalog, so storing several checksums
* Like for Metadata, when editing this option, the user will be asked to trigger the update or not

    
---
## **#824 - Individual MD5 calculation**

![Planned](https://img.shields.io/badge/Status-Planned-orange) ![3.1](https://img.shields.io/badge/Version-3.1-blue) ![824](https://img.shields.io/badge/github-824-blue?logo=github)

Approved 2026-10-25. K3 only (no K2 change).

> **Reading the requirement IDs.** Each requirement has a permanent ID.
> IDs are never renumbered or reused; a retired requirement is marked
> `[Removed]`, not deleted. Status is one of
> `[Implemented] / [Planned] / [Backlog] / [Removed]`.

**In scope:** calculating and showing the MD5 of one file, on demand, from the K3 file context menu (Search Results and Explore), with a copy action.<br/>
**Out of scope:** storing the MD5 anywhere; MD5 for a whole catalog or collection (#825); comparing against an external source (#826); any K2 change.

#### Operational requirements — *why / for whom*

| ID | Requirement | Status |
|----|-------------|--------|
| CKS-O1 | A user obtains the MD5 of a single cataloged file on demand (for example to compare it with an MD5 published elsewhere), without changing anything the collection stores. | [Planned] |

#### Functional requirements — *what the system does*

| ID | Requirement | Status |
|----|-------------|--------|
| CKS-F1 | In K3, the file context menu of the **Search Results** page and of the **Explore** page carries an item that calculates the MD5 of that single file. The item is offered for every file row, whether or not the file already has a SHA-256 checksum. | [Planned] |
| CKS-F2 | The calculated MD5 is shown to the user in a result dialog with a way to copy it to the clipboard, following the same dialog pattern as the existing single-file SHA-256 calculation. | [Planned] |

#### Constructional requirements — *how it is built / limits / MUST-NOTs*

| ID | Requirement | Status |
|----|-------------|--------|
| CKS-C1 | The MD5 calculated by `CKS-F1` MUST NOT be stored: no database write, no `.idx` write, no settings change. No schema change; zero impact on existing collections. | [Planned] |
| CKS-C2 | The calculation uses the existing core `FileChecksum::calculateChecksum(path, QCryptographicHash::Md5)`. No `core/` method is added or modified for #824. | [Planned] |
| CKS-C3 | #824 is K3 only: no change to K2 (`qt_widgets/`). | [Planned] |
| CKS-C4 | Stored checksums follow **Strategy A** (one dedicated column per algorithm, see [Decision](#decision-strategy-a-retained-2026-10-25)). `checksum_sha256` MUST NOT be renamed or replaced by generic slots. Any new checksum column is added by a new versioned migration, never in `runMigration_3_0` (schema 3.0 is frozen). | [Planned] |

#### Known limitation (backlog, not part of #824)

Like the existing single-file SHA-256 calculation in K3, the MD5 calculation runs synchronously on the GUI thread, without progress or cancel. Large files freeze the window until done.

#### Manual test charter

- **CKS-F1** — On the Search Results page and on the Explore page, open the context menu of a file **with** a SHA-256 checksum and of a file **without** one. The MD5 item is present in all four cases.
- **CKS-F2** — Run the MD5 item on a file. The dialog shows the MD5; the copy action puts it in the clipboard. Compare with `md5sum <file>`: identical.
- **CKS-C1** — Note the collection files (database / `.idx`) and the settings file modification dates, run the MD5 item, close Katalog. Nothing changed; the file's stored SHA-256 (if any) is unchanged.

---
## **Increment 1 - Implemented Features**

### FileChecksum Class (`src/core/filechecksum.h/.cpp`)

Core class managing checksum calculation, storage, verification, and retrieval.

#### Calculation & Storage Methods

| Method | Purpose |
|--------|---------|
| `calculateAndStore()` | Main entry point during catalog creation/updates. Checks if enabled, verifies file exists, calculates SHA-256, stores in database. |
| `calculateChecksum()` | Core SHA-256 calculation using Qt's `QCryptographicHash`. Uses 8 MB buffer, reports progress every 25 MB, supports cancellation. |
| `updateFileChecksum()` | Updates single file record with checksum value and extraction timestamp. |
| `batchUpdateFileChecksum()` | Batch updates multiple files in single transaction for performance. Uses database transaction with rollback on failure. |

#### Verification Methods

| Method | Purpose |
|--------|---------|
| `verifyChecksum()` | Verifies single file. Returns `VerificationResult` struct with success, match, expected/actual checksums, error message. |
| `verifyCatalogChecksums()` | Verifies all checksums in a catalog. Returns `CatalogVerificationResult` with counts (totalFiles, verified, mismatches, missing) and file lists. Supports cancellation and progress reporting. |

#### Utility Methods

| Method | Purpose |
|--------|---------|
| `getFileChecksum()` | Retrieves stored checksum from database for a file. |
| `countFilesWithChecksum()` | Returns count of files with non-null, non-empty checksums. |
| `getAlgorithmFromString()` | Converts algorithm name string to `QCryptographicHash::Algorithm` enum. |

---

### Catalog Operations Integration

#### During Catalog Creation/Update (`src/core/catalogjobstoppable.cpp`)

| Function | Purpose |
|----------|---------|
| `findFilesWithoutChecksum()` | Queries files missing checksums (WHERE `checksum_extraction_date IS NULL`). |
| `extractChecksumsForFiles()` | Processes batches of files, calculates checksums, updates database. Progress: `__CHECKSUM_CALCULATION__` markers. |

**Flow:**
- Step 9b: After metadata extraction, if `catalog->includeChecksum != "None"`:
  1. Call `findFilesWithoutChecksum()` to get files needing checksums
  2. Call `extractChecksumsForFiles()` for batch processing
  3. Stoppable and resumable (only processes files with NULL extraction date)

---

### Search Features

#### Duplicate Search (`src/core/searchjobstoppable.cpp`)

| Option | SQL Logic |
|--------|-----------|
| Checksum Equal (`=`) | Groups files by checksum, finds identical content |
| Checksum Not Equal (`≠`) | Finds files matching on name/size/date but with different checksums (modified copies) |

#### Differences Search

| Option | SQL Logic |
|--------|-----------|
| Checksum Equal (`=`) | Cross-device comparison for identical files |
| Checksum Not Equal (`≠`) | Files matching on other criteria but different checksums between devices |

**UI Controls** (Search tab):
- `Search_checkBox_DuplicatesChecksum` / `Search_comboBox_DuplicateChecksumSign`
- `Search_checkBox_DifferencesChecksum` / `Search_comboBox_DifferenceChecksumSign`

---

### Context Menu Actions

#### File-Level Actions (Explore & Search tabs)

| Action | When Shown | Handler |
|--------|------------|---------|
| **Copy Checksum** | File has checksum | `exploreContextCopyFileChecksum()` / `searchContextCopyFileChecksum()` |
| **Calculate Checksum (SHA-256)** | File has no checksum | `calculateAndSaveChecksum()` |
| **Verify Checksum (SHA-256)** | File has checksum | `verifyFileChecksum()` |

#### Catalog-Level Actions (Devices tab)

| Action | When Shown | Handler |
|--------|------------|---------|
| **Verify Checksums** | Catalog selected, has checksums | `verifyCatalogChecksums()` |

---

### UI Verification Functions (`src/mainwindow_tab_search_ui.cpp`)

| Function | Purpose |
|----------|---------|
| `verifyFileChecksum()` | Shows progress dialog, calculates checksum, compares with stored value, displays result or mismatch warning. |
| `showChecksumResult()` | Displays success message box with checksum and "Copy to Clipboard" button. |
| `showChecksumMismatch()` | Warning dialog showing expected vs actual, offers "Update Database with New Checksum" option. |
| `calculateAndSaveChecksum()` | On-demand calculation for files without checksum. Shows progress, saves to database, displays result. |

---

### Catalog Edit Behavior (`src/mainwindow_tab_device_pr.cpp`)

When editing a catalog's checksum option in `saveCatalogChanges()`:

| Transition | First Dialog | Second Dialog | Rescan |
|------------|--------------|---------------|--------|
| None → SHA256 | ✅ Shows changes | ✅ "Update catalog content?" | ✅ Yes (to compute checksums) |
| SHA256 → None | ✅ Shows changes | ❌ Not shown | ❌ No (keeps existing checksums) |
| SHA256 → SHA256 | ❌ No change | ❌ Not shown | ❌ No |

---

### Database Schema (Implemented)

**File/Filetemp tables:**
```sql
checksum_sha256             TEXT,     -- 64 hex chars (lowercase)
checksum_extraction_date    TEXT,     -- yyyy/MM/dd hh:mm:ss
```

**Catalog table:**
```sql
catalog_include_checksum    TEXT,     -- "None" or "SHA256"
```

---

### Constants (`src/core/catalog.cpp`)

```cpp
const QString Catalog::CHECKSUM_NONE = "None";
const QString Catalog::CHECKSUM_SHA256 = "SHA256";
```

---

### Original Implementation Strategy (for reference)

Phase 1: Infrastructure
1. Database migration adding checksum columns
2. Catalog settings UI for checksum options
3. `FileChecksum` class (similar to `FileMetadata`)
4. Integration into update process (Step 9b)
5. Progress reporting

Phase 2: Search Features
6. Duplicate detection by checksum
7. Corruption detection

Implementation Notes:
1. **Performance**: 
   - Use `QFile::open()` + `QCryptographicHash::addData(QIODevice*)` for large files
   - Read in chunks (64KB-1MB) to avoid memory issues
   - SHA-256 ~200-400 MB/s on modern CPUs
   - Process during idle time, stoppable

2. **Storage**:
   - Hex string storage (human-readable, debuggable)
   - ~32-64 bytes per file per algorithm
   - 1 million files × 64 bytes = 64 MB (negligible)


### Database Schema Updates

Add to `file` and `filetemp` tables:

```sql
-- Checksum fields (following your metadata pattern)
checksum_sha256             TEXT,     -- 64 hex chars
checksum_extraction_date    TEXT,     -- ISO timestamp (like metadata_extraction_date)
```

Add to `catalog` table:
```sql
catalog_include_checksum    TEXT,     -- "None", "MD5", "SHA1", "SHA256", "BLAKE2b-256", "Multiple"
```

### UI
UI Naming Convention:
Based on existing code pattern (Create_checkBox_IncludeMetadata, Create_comboBox_FileType):

```
UI Elements (like metadata):
└─ Create_comboBox_IncludeChecksum      (dropdown: None, SHA256)
```

* Use single dropdown with "None" as default option (1 widget, like metadata)

Existing metadata uses: `catalog_include_metadata` with values "None", "Media_Basic", etc.
So checksum should follow: `catalog_include_checksum` with values "None", "MD5", "SHA256", etc.


### Feature Design (Following Metadata Pattern)

**1. Catalog Settings**

```
catalog_include_checksum Options:
- None           : No checksums calculated
- SHA256         : SHA-256 only (secure, recommended default)
```

**2. Update Process Integration**

**Step 9b: Checksum Extraction (after metadata extraction)**
```
Location: In updateCatalogIncremental(), after Step 9 (metadata extraction)
Query: WHERE checksum_extraction_date IS NULL 
       AND (catalog_include_checksum != 'None')
Processing: Batches of 100 files
Stoppable: Yes (same pattern as metadata)
Resumable: Yes (queries for NULL checksum_extraction_date)
Progress: "Calculating checksums X/Y files..." consistently with other progress reporting
```

**3. Incremental Philosophy**
Same as for Metadata:
- **includeChecksum = "None"**: Skip checksum calculation entirely
- **includeChecksum != "None"**: Calculate checksums for files WHERE `checksum_extraction_date IS NULL`
- **New files**: Automatically get checksums during update (if enabled)



---

### Search Features to Implement

#### Phase 1: Find Duplicates
```
Search Options:
☐ Find duplicates by checksum
   Algorithm: [SHA256 ▼] (dropdown: MD5, SHA1, SHA256, BLAKE2b-256)
   
Combined with existing:
☐ Same name
☐ Same size  
☐ Same date

Results: Show groups of files with identical checksums
```

#### Phase 2: Find Differences
```
Search Options:
☐ Find files with same name/size/date but different checksum
   
Use case: Detect file corruption or silent modifications
Results: Flag potential data integrity issues
```

#### Phase 3: Verification
```
"Verify Catalog Integrity" feature:
- Re-calculate checksums for files that still exist
- Compare with stored checksums
- Report:
  ✓ Verified: X files
  ⚠ Modified: Y files (checksum changed)
  ✗ Missing: Z files (file no longer exists)
```





## **Increment 2 - Algorithm Choice Strategies**


### Other Algorithm options

The Qt6 librairy **QCryptographicHash** provides: MD4, MD5, SHA-1, SHA-224, SHA-256, SHA-384, SHA-512, SHA3 variants, Keccak variants, and BLAKE2b/BLAKE2s algorithms.<br/>
1. **MD5** - Legacy compatibility (fast but cryptographically broken, 128-bit/16 bytes)
2. **SHA-1** - Middle ground (faster than SHA-256, more secure than MD5, 160-bit/20 bytes)
3. **SHA-256** - Best balance (secure, fast, widely supported, 256-bit/32 bytes)

**Out of Qt:** (external library)
1. **CRC32** - A cyclic redundancy check that is faster but less secure than cryptographic hash functions like SHA-256.
2. **xxHash** - Extremely fast Hash algorithm, processing at RAM speed limits. <br/>Code is highly portable, and produces hashes identical across all platforms (little / big endian). |  https://xxhash.com/ <br/> https://github.com/Cyan4973/xxHash



### Current Implementation (Increment 1)
- **DB columns**: `checksum_sha256`, `checksum_extraction_date` (single algorithm)
- **Catalog setting**: `catalog_include_checksum` = "None" | "SHA256"

### Goal
Allow users to choose checksum algorithm(s) per catalog. Support use cases:
- Speed-focused (xxHash for large video files)
- Security-focused (SHA256 for archives)
- Migration (keep old algorithm, add new one)
- Dual checksums (fast comparison + secure verification)

---

### Strategy A: Multiple Dedicated Columns

One column per supported algorithm.

```sql
-- File table:
checksum_md5              TEXT,     -- 32 hex chars
checksum_sha1             TEXT,     -- 40 hex chars
checksum_sha256           TEXT,     -- 64 hex chars (already exists)
checksum_xxhash           TEXT,     -- 16 hex chars
checksum_extraction_date  TEXT

-- Catalog table:
catalog_include_checksum  TEXT,     -- "None", "MD5", "SHA1", "SHA256", "xxHash"
```

| Pros | Cons |
|------|------|
| Simple queries: `WHERE checksum_sha256 IS NOT NULL` | Wasted space for unused columns |
| Can keep multiple checksums per file (migration) | Schema change for each new algorithm |
| Cross-catalog comparison when same algo | More columns to manage |
| Already partially implemented (sha256) | |

**Complexity**: Low - just add columns, update FileChecksum class

---

### Strategy B: Single Column + Algorithm Field

One generic column, algorithm stored separately.

```sql
-- File table:
file_checksum             TEXT,     -- Variable length hash
file_checksum_algorithm   TEXT,     -- "MD5", "SHA256", etc.
checksum_extraction_date  TEXT
```

| Pros | Cons |
|------|------|
| No wasted space | Complex queries: `WHERE algorithm='SHA256' AND checksum=...` |
| Easy to add new algorithms | Can only store ONE checksum per file |
| Simpler schema | No migration path (lose old checksum when changing) |
| | Breaking change from current implementation |

**Complexity**: High - requires migration, all queries need update

---

### Strategy C: Hybrid (Dedicated + Generic)

Keep SHA256 column, add one generic slot.

```sql
-- File table:
checksum_sha256           TEXT,     -- Current, keep for performance
checksum_secondary        TEXT,     -- For migration/other algos
checksum_secondary_algo   TEXT,     -- "MD5", "SHA1", "xxHash", etc.
checksum_extraction_date  TEXT
```

| Pros | Cons |
|------|------|
| Keeps current implementation | Inconsistent design |
| Allows one additional algorithm | Limited flexibility |
| Minimal migration | Confusing model |

**Complexity**: Medium

---

### Strategy D: JSON/Blob Storage

Store all checksums in a single JSON field.

```sql
-- File table:
checksums                 TEXT,     -- JSON: {"sha256":"...", "md5":"...", "xxhash":"..."}
checksum_extraction_date  TEXT
```

| Pros | Cons |
|------|------|
| Unlimited algorithms | Cannot index/query efficiently |
| No schema changes ever | Parsing overhead |
| Maximum flexibility | SQLite JSON support varies |
| | Breaking change |

**Complexity**: High

---

### Strategy E: 2 Generic Slots + Per-Catalog Algorithm

> **Not retained** (2026-10-25): see [Decision: Strategy A retained](#decision-strategy-a-retained-2026-10-25).

Two generic checksum columns, each catalog chooses which algorithm(s) to use.

```sql
-- Catalog table:
catalog_checksum1_algorithm  TEXT,  -- "None", "MD5", "SHA1", "SHA256", "xxHash"
catalog_checksum2_algorithm  TEXT,  -- "None", "MD5", "SHA1", "SHA256", "xxHash"

-- File table:
checksum1                    TEXT,  -- Value for catalog's algo1
checksum2                    TEXT,  -- Value for catalog's algo2
checksum1_extraction_date    TEXT,  -- When checksum1 was calculated
checksum2_extraction_date    TEXT,  -- When checksum2 was calculated (allows separate timing)
```

#### Example Configurations

| Catalog | checksum1_algo | checksum2_algo | Use Case |
|---------|----------------|----------------|----------|
| Photos  | SHA256         | None           | Security-focused |
| Videos  | xxHash         | None           | Speed-focused (large files) |
| Archive | SHA256         | MD5            | Migration: keeping old MD5, adding SHA256 |
| Backup  | SHA256         | xxHash         | Both security + fast comparison |

#### File Data Example

```
Catalog "Archive" (algo1=SHA256, algo2=MD5):
file.txt: checksum1="a1b2c3..." (SHA256), checksum2="x9y8z7..." (MD5)

Catalog "Videos" (algo1=xxHash, algo2=None):
video.mp4: checksum1="f5e4d3..." (xxHash), checksum2=NULL
```

| Pros | Cons |
|------|------|
| Only 2 columns regardless of algorithm count | Cross-catalog search needs algo matching logic |
| Flexible: any 2 algorithms per catalog | Must track which algo is in which slot |
| Supports migration (old + new algo simultaneously) | Queries need: `JOIN ON catalog to check algo` |
| Supports speed+security combo | Max 2 algorithms per catalog |
| Minimal schema (2 columns forever) | |
| Easy to add new algorithms (no schema change) | |
| Separate extraction dates per slot | |

#### Cross-Catalog Search Logic

When searching duplicates across catalogs with different algorithm configurations:

```sql
-- Find matching checksums across catalogs A and B
WHERE (catA.checksum1_algo = catB.checksum1_algo AND fileA.checksum1 = fileB.checksum1)
   OR (catA.checksum1_algo = catB.checksum2_algo AND fileA.checksum1 = fileB.checksum2)
   OR (catA.checksum2_algo = catB.checksum1_algo AND fileA.checksum2 = fileB.checksum1)
   OR (catA.checksum2_algo = catB.checksum2_algo AND fileA.checksum2 = fileB.checksum2)
```

#### Migration from Current Implementation

```sql
-- Step 1: Rename existing column
ALTER TABLE file RENAME COLUMN checksum_sha256 TO checksum1;
ALTER TABLE file RENAME COLUMN checksum_extraction_date TO checksum1_extraction_date;

-- Step 2: Add new columns
ALTER TABLE file ADD COLUMN checksum2 TEXT;
ALTER TABLE file ADD COLUMN checksum2_extraction_date TEXT;

-- Step 3: Add catalog algorithm fields
ALTER TABLE catalog ADD COLUMN catalog_checksum1_algorithm TEXT DEFAULT 'None';
ALTER TABLE catalog ADD COLUMN catalog_checksum2_algorithm TEXT DEFAULT 'None';

-- Step 4: Migrate existing catalogs (those with checksums become SHA256)
UPDATE catalog
SET catalog_checksum1_algorithm = 'SHA256'
WHERE catalog_include_checksum = 'SHA256';

-- Step 5: Drop old column (optional, after verification)
-- ALTER TABLE catalog DROP COLUMN catalog_include_checksum;
```

**Complexity**: Medium - migration needed, but clean design going forward

---

### Strategy Comparison Summary

| Strategy | Columns | Algos/File | Schema Changes | Query Complexity | Migration |
|----------|---------|------------|----------------|------------------|-----------|
| **A: Multiple Dedicated (retained)** | **4-5** | **Unlimited** | **Per new algo** | **Low** | **Easy** |
| B: Single + Algo Field | 2 | 1 | None | Medium | Hard |
| C: Hybrid | 3 | 2 | Minimal | Medium | Easy |
| D: JSON | 1 | Unlimited | None | High | Hard |
| E: 2 Generic Slots (not retained) | 4 | 2 | None | Medium | Medium |

### Decision: Strategy A retained (2026-10-25)

**Strategy A (multiple dedicated columns) is retained** for #825 (collection MD5 calculation) and #826 (compare checksums to an external MD5 or SHA-256 source). It replaces the former recommendation of Strategy E (kept below for history, **not retained**). Bound by `CKS-C4`.

**Future columns** (`file` and `filetemp`), added only when #825 is implemented:
```sql
checksum_md5                     TEXT,     -- 32 hex chars
checksum_md5_extraction_date     TEXT,     -- yyyy/MM/dd hh:mm:ss
```
* Added by a **new versioned migration**. Schema 3.0 is frozen: no change in `runMigration_3_0`.
* `checksum_sha256` (and `checksum_extraction_date`) stay **unchanged**: no rename to generic slots.

**Rationale:**
1. No rename of `checksum_sha256`: avoids touching ~40 query sites, the `.idx` format and the importer.
2. Cross-catalog duplicates/differences stay a single-column equality (no algorithm-matching logic).
3. #826 knows the algorithm from the column name.
4. Independent resume per algorithm, through a separate extraction date per algorithm.

**Open points for #825** (noted, not decided):
* `catalog_include_checksum` is single-valued (`"None"` / `"SHA256"`); it must be able to express SHA-256 and MD5 together.
* Memory mode: the `.idx` would gain fields 30/31. Forward-compatible for readers (`fieldListCount` guards), but a 3.0 install rewriting the `.idx` would drop them — release-note item.

---

### Former recommendation: Strategy E (not retained)

> Historical / alternative only. Superseded on 2026-10-25 by the [Strategy A decision](#decision-strategy-a-retained-2026-10-25).

**Reasons:**
1. **Bounded complexity**: Max 2 checksums per file covers all realistic use cases
2. **Future-proof**: Add xxHash, BLAKE3, SHA3 without schema changes
3. **Supports real workflows**: Migration, speed+security dual checksums
4. **Clean design**: Algorithm choice is per-catalog, not hardcoded in schema
5. **Separate timing**: Can add second checksum later without recalculating first

---

## **Increment 3 (future work)**

### Advanced Features
- Cross-catalog duplicate detection with algorithm matching
- Checksum export/import for external verification
- Scheduled/background checksum verification
- Checksum comparison reports

---

### "Verify Catalog" Feature - Full Specification

**Purpose**: Verify that files on disk still match their stored checksums (detect corruption/modification).

**UI Location**: 
- Button in Devices/Catalogs list view: "Verify Catalog"
- Or right-click menu on catalog: "Verify Integrity..."

**Feature Behavior**:

**Phase 1: Pre-check**
```
1. Check if catalog has checksums:
   - Query: SELECT COUNT(*) FROM file 
            WHERE file_catalog_id = X 
            AND checksum_extraction_date IS NOT NULL
   
   - If count = 0: Show error "Catalog has no checksums. Enable checksums and update catalog first."
   - If count > 0: Proceed to Phase 2
```

**Phase 2: Verification Process**
```
2. For each file with a checksum:
   a. Check if file still exists on disk (using file_full_path)
   b. If exists: Re-calculate checksum using same algorithm
   c. If not exists: Mark as "Missing"
   d. Compare:
      - Match: File verified ✓
      - Mismatch: File modified/corrupted ⚠
      - Missing: File deleted ✗

3. Progress reporting:
   - "Verifying X / Y files..."
   - Stoppable (like catalog update)
   - Process in batches of 100 files
```

**Phase 3: Results Dialog**
```
╔══════════════════════════════════════════════╗
║  Catalog Verification Results                ║
╠══════════════════════════════════════════════╣
║  Catalog: "My Photos 2024"                   ║
║  Algorithm: SHA-256                          ║
║  Verified: 2024-01-15 14:30:22              ║
║                                              ║
║  ✓ Verified:    48,532 files (97.1%)        ║
║  ⚠ Modified:       856 files (1.7%)         ║
║  ✗ Missing:        612 files (1.2%)         ║
║  ━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━  ║
║  Total:        50,000 files                  ║
║                                              ║
║  [ View Modified Files ]  [ Export Report ] ║
║  [ View Missing Files  ]       [ Close ]    ║
╚══════════════════════════════════════════════╝
```

**Phase 4: Detailed Views**

**"View Modified Files" opens new window/tab:**
```
File Name       | Path                      | Catalog Checksum | Current Checksum | Date Modified
─────────────────────────────────────────────────────────────────────────────────────────────────
photo_001.jpg   | /Photos/2024/Jan         | a1b2c3d4...      | x9y8z7w6...     | 2024-06-15
document.pdf    | /Documents/Work          | e5f6g7h8...      | i1j2k3l4...     | 2024-08-22
```

**"View Missing Files" opens new window/tab:**
```
File Name       | Path                      | Catalog Checksum | Last Known Date
─────────────────────────────────────────────────────────────────────────────────────────────
backup.zip      | /Backups/2023            | m5n6o7p8...      | 2023-12-01
old_project.tar | /Archive/Projects        | q1r2s3t4...      | 2022-09-15
```

**Phase 5: Actions**

User can:
1. **Export Report**: Save verification results to CSV/TXT file
2. **Update Catalog**: Option to update checksums for modified files
3. **Remove Missing**: Remove missing files from catalog (optional)

**Database Update** (optional):
```
Option: "Update checksums for modified files"
Action: For files with mismatched checksums:
        - Update checksum_* column with new calculated value
        - Update checksum_extraction_date to current timestamp
        - Log change: old checksum → new checksum
```


**Additional Features to Consider**:

1. **Verify on Catalog Load** (Settings option):
   - Automatically verify integrity when loading catalog
   - Show warning if mismatches detected

2. **Scheduled Verification**:
   - Background task to verify catalogs periodically
   - Notify user if issues found

3. **Verify Specific Files**:
   - Right-click file in search results → "Verify Integrity"
   - Quick check for single file



**Technical Implementation Notes**:

- **Stoppable**: User can cancel verification mid-process
- **Resumable**: NO (verification is one-time check, not stored in DB)
- **Performance**: Same as checksum calculation (~200-400 MB/s for SHA-256)
- **Memory**: Process in batches, don't load all results at once
- **Thread safety**: Use CatalogJobStoppable pattern (same as update)



To be considered:
A. Should "Verify Catalog" update checksums for modified files automatically, or just report?
B. Should verification results be saved to database, or just temporary report?
C. Should there be a "partial verification" option (verify only X random files, faster)?
D. Back to architecture: Do you want **multiple columns** (checksum_md5, checksum_sha256) or **single column** (file_checksum + file_checksum_algorithm)? — *Answered 2026-10-25: multiple columns ([Strategy A](#decision-strategy-a-retained-2026-10-25)).*










