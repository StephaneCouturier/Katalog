# File checksums
![Draft](https://img.shields.io/badge/Status-Draft-orange) ![2.9](https://img.shields.io/badge/Version-2.9-orange) ![664](https://img.shields.io/badge/github-664-blue?logo=github)
## **Introduction**
This document specifies the implementation of features using **File checksums**.

### Benefits
- Data Integrity Verification: Checksums can be used to verify the integrity of files, ensuring that files have not been corrupted or altered.
- Duplicate/Differences File Detection: By comparing checksums, users can identify duplicate files or differences, independantly of file names, size, dates.
- Error Detection: Checksums can help detect errors that may occur during file transfer or storage.
- Security: Checksums can provide a basic level of security by detecting unauthorized changes to files.


### Available Algorithms

The Qt6 librairy **QCryptographicHash** provides: MD4, MD5, SHA-1, SHA-224, SHA-256, SHA-384, SHA-512, SHA3 variants, Keccak variants, and BLAKE2b/BLAKE2s algorithms.<br/>
1. **MD5** - Legacy compatibility (fast but cryptographically broken, 128-bit/16 bytes)
2. **SHA-1** - Middle ground (faster than SHA-256, more secure than MD5, 160-bit/20 bytes)
3. **SHA-256** - Best balance (secure, fast, widely supported, 256-bit/32 bytes)

**Out of Qt:** (external library)
1. **CRC32** - A cyclic redundancy check that is faster but less secure than cryptographic hash functions like SHA-256.
2. **xxHash** - Extremely fast Hash algorithm, processing at RAM speed limits. <br/>Code is highly portable, and produces hashes identical across all platforms (little / big endian). |  https://xxhash.com/ <br/> https://github.com/Cyan4973/xxHash


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
## **Increment 1 (current work)**

### Implementation Strategy

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





## **Increment 2 (future work)**

### Database Schema Updates

Add to `file` and `filetemp` tables:

```sql
-- Checksum fields (depending on new algo retained)
checksum_md5                TEXT,     -- 32 hex chars
checksum_sha1               TEXT,     -- 40 hex chars  
```

**Why multiple checksum columns?**
* Like metadata levels, users may want to upgrade/downgrade
* Storage is cheap (64 bytes max per algorithm)
* Allows future migration between algorithms

---
## **Increment 3 (future work)**

#### Phase 3: Advanced Features
8. Catalog verification (re-check existing files)
9. Cross-catalog duplicate detection
10. Checksum export/import for verification


### 2. "Future migration between algorithms" - Example

**Scenario**: User has 50,000 files cataloged with MD5 in 2025.

In 2028, they decide to switch to SHA-256 (new company policy, or MD5 deprecated).

**Without multiple columns** (single `file_checksum` TEXT):
```
Before: file_checksum = "a1b2c3..." (MD5)
After:  file_checksum = "x9y8z7..." (SHA-256)
Result: Lost all MD5 checksums forever
```

**With multiple columns**:
```
Before: checksum_md5 = "a1b2c3...", checksum_sha256 = NULL
After:  checksum_md5 = "a1b2c3...", checksum_sha256 = "x9y8z7..."
Result: Kept MD5 history, added SHA-256
```

**Benefits**:
- Can verify old backups still match MD5
- Can compare files across catalogs (one using MD5, one using SHA-256)
- Gradual migration without losing history

3. "Duplicate detection with different algorithms"

**CANNOT compare checksums from different algorithms**

**Scenario**: You have 2 catalogs:
- Catalog A: uses MD5
- Catalog B: uses SHA-256

**Search for duplicates:**
```
Option 1 (current): Search within Catalog A only (MD5 vs MD5) ✓
Option 2 (current): Search within Catalog B only (SHA256 vs SHA256) ✓
Option 3 (impossible): Compare Catalog A vs Catalog B (MD5 vs SHA256) ✗
```

**With multiple columns**, if you later add SHA-256 to Catalog A:
```
Now possible: Compare Catalog A vs Catalog B (both using SHA256) ✓
```

So the benefit is: **Standardize catalogs to same algorithm** for cross-catalog comparison.

Not "hash conversion" (impossible), but "recalculate with new algorithm."

---

### 4. Catalog Model - One algorithm per catalog, multiple columns in DB

**Architecture**:
```
Database (file table):
├─ checksum_md5         TEXT  (32 hex chars)
├─ checksum_sha1        TEXT  (40 hex chars)
├─ checksum_sha256      TEXT  (64 hex chars)
└─ checksum_extraction_date TEXT

Catalog settings:
└─ catalog_include_checksum = "SHA256"  (only ONE algorithm active)
```

**Example Catalog A** (using SHA-256):
```
File 1: checksum_md5=NULL, checksum_sha256="x9y8z7...", checksum_extraction_date="2025-01-01"
File 2: checksum_md5=NULL, checksum_sha256="a1b2c3...", checksum_extraction_date="2025-01-01"
```

**Example Catalog B** (using MD5):
```
File 1: checksum_md5="f5e4d3...", checksum_sha256=NULL, checksum_extraction_date="2025-01-02"
File 2: checksum_md5="c2b1a0...", checksum_sha256=NULL, checksum_extraction_date="2025-01-02"
```

**Is this the right model?**

**Pros**:
- Simple: one algorithm active per catalog
- Flexible: can change algorithm without losing old data
- Query-friendly: `WHERE checksum_sha256 IS NOT NULL`

**Cons**:
- Wasted space: unused columns remain NULL
- More database columns (but only ~5 algorithms total)

**Alternative Model** (single column):
```
Database:
├─ file_checksum              TEXT (variable length)
├─ file_checksum_algorithm    TEXT ("MD5", "SHA256", etc.)
└─ checksum_extraction_date   TEXT
```

**Pros**: Less wasted space
**Cons**: 
- Must always check algorithm column in queries
- Harder to compare: `WHERE file_checksum_algorithm='SHA256' AND file_checksum='x9y8...'`
- Can't easily keep multiple checksums per file

**My recommendation**: Multiple columns (your original understanding is correct).

---

### 5. "Multiple" option - Calculate MD5 + SHA256 simultaneously

**Use Case**: Transition period when changing algorithms.

**Scenario**:
1. You have 50,000 files with MD5 checksums (from 2020-2024)
2. You decide to switch to SHA-256 (company policy, Jan 2025)
3. You want to **keep MD5 for old files** but **add SHA-256 for all files**

**Without "Multiple" option**:
```
Step 1: Set catalog_include_checksum = "SHA256"
Step 2: Update catalog
Result: Only NEW files get SHA-256
        Old files keep MD5 only
        
To get SHA-256 for old files:
Step 3: Manually trigger re-calculation (expensive, 50,000 files)
```

**With "Multiple" option**:
```
Step 1: Set catalog_include_checksum = "Multiple_MD5_SHA256"
Step 2: Update catalog
Result: NEW files get BOTH MD5 + SHA-256
        OLD files get SHA-256 added (MD5 preserved)
        
After transition (6 months later):
Step 3: Set catalog_include_checksum = "SHA256"
Step 4: Clear MD5 column if desired (optional)
```

Simpler approach:
```
User wants to change MD5 → SHA-256:
1. Set catalog_include_checksum = "SHA256"
2. Clear checksum_extraction_date for all files
3. Next update calculates SHA-256 for all files
4. MD5 column remains (preserved history)
```

### 5. "Verify Catalog" Feature - Full Specification

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
D. Back to architecture: Do you want **multiple columns** (checksum_md5, checksum_sha256) or **single column** (file_checksum + file_checksum_algorithm)?










