# Catalog Update System - Complete Specification v2.8

## **Target Users**
- Users transitioning from Katalog v2.7 (or earlier) to v2.8
- Users changing catalog includeMetadata options at any time

## **Key Changes in v2.8**

### **1. File Type Conversion**
New database columns: `file_extension`, `file_type`, `mime_type` (plus `mime_verified`, `type_mismatch`)
- Required for Search functionality to work
- Automatically applied to all files (old and new)

### **2. Incremental Indexing**
New mechanism limiting metadata extraction processing:
- **includeMetadata = "None"**: Only updates  exact file changes (unchanged/deleted/added/modified)
- **includeMetadata != "None"**: also extracts metadata for changed files or files missing `metadata_extraction_date`

---

## **Database Schema: Metadata Fields**

### **File Type Fields** (always populated, never cleared):
- `file_extension` TEXT
- `file_type` TEXT
- `mime_type` TEXT
- `mime_verified` NUMERIC
- `type_mismatch` NUMERIC

### **Metadata Fields** (populated based on includeMetadata setting):
- `image_width`, `image_height`, `image_orientation` NUMERIC
- `video_duration_seconds`, `video_width`, `video_height` NUMERIC
- `video_codec` TEXT, `video_framerate`, `video_bitrate` NUMERIC
- `audio_duration_seconds`, `audio_bitrate`, `audio_sample_rate` NUMERIC
- `audio_artist`, `audio_album`, `audio_title`, `audio_genre` TEXT
- `audio_year`, `audio_track_number` NUMERIC
- `metadata_extended` TEXT (JSON)
- **`metadata_extraction_date` TEXT** (ISO format timestamp)

---

## **Catalog Parameters Affecting File Selection**

Changes to these 4 fields require catalog update:
1. `fileType` (All/Image/Audio/Video/Text/Other/None)
2. `includeHidden` (boolean)
3. `includeMetadata` (None/Media_Basic/Media_Extended/Full_Extended)
4. `isFullDevice` (boolean) - **Currently not used**

### **Update Trigger Behavior**
- UI prompts user to update immediately after editing these fields
- User can accept (immediate) or decline (postpone)
- **The update system is independent**: It determines required processing based on current catalog definition and file state

### **No Force Full Rescan in Regular Updates**
- **Full rescan is never part of regular update process**
- Changes to `fileType`/`includeHidden` are handled incrementally (delete old files, add new ones matching new criteria)
- Full rescan only offered as separate testing/troubleshooting option in Devices/Catalog list view

---

## **File Type Conversion Process**

### **When It Runs**
Automatically during:
1. **Search operation** (triggers catalog load)
   - Calls `catalog->migrateCatalogFieldsForSearch()` 
   - Detects files with NULL/empty `file_extension`, `file_type`, or `mime_type`
   - Processes them before search continues

2. **Catalog Update operation**
   - Step 8a in `updateCatalogIncremental()`: calls `migrateMimeTypesForExistingFiles()`
   - Finds files missing mime_type fields
   - Populates from extension

3. **Catalog Load from .idx file** (Memory mode)
   - During `loadCatalogFileToTable()`, detects v2.6 format (6 columns vs 28)
   - Converts on-the-fly and saves catalog in v2.8 format

### **How It Works**
- Queries: `WHERE (file_extension IS NULL OR file_extension = '' OR file_type IS NULL OR file_type = '' OR mime_type IS NULL OR mime_type = '')`
- Extracts extension from `file_name` or `file_full_path`
- Calls `FileMetadata::getFileTypeFromExtension()` and `getMimeTypeFromExtension()`
- Updates fields in batches (1000 files per batch for efficiency)
- **Stoppable**: Checks `stopRequested` flag during processing
- **Resumable**: On next operation, queries for remaining NULL fields and continues

### **Progress Reporting**
- Emits `loadProgress(filesProcessed, totalFiles)` 
- Shows: "Converting X/Y files..."
- Updates every 100 files or 1% of total

---

## **Incremental Update Process - Order of Operations**

### **Standard Update Flow** (`updateCatalogIncremental()`):

1. **Clear temporary table**: `DELETE FROM filetemp`

2. **Count total files**: Walk filesystem to estimate progress denominator

3. **Scan filesystem → filetemp**: 
   - Use `QDirIterator` with configured filters
   - Insert into `filetemp` table with file_extension, file_type, mime_type already populated
   - Batch size: 1000 files (no metadata) or 100 files (with metadata)
   - Check `shouldContinue()` every file

4. **Analyze differences (SQL-based)**:
   - New files: `SELECT * FROM filetemp WHERE NOT EXISTS IN file`
   - Modified files: `SELECT * FROM filetemp JOIN file WHERE file_date_updated differs`
   - Deleted files: `SELECT * FROM file WHERE NOT EXISTS IN filetemp`
   - Unchanged files: `COUNT(*) WHERE file matches filetemp`

5. **Database updates (in transaction)**:
   - Insert new files: `INSERT INTO file`
   - Update modified files: `UPDATE file SET ...`
   - Delete removed files: `DELETE FROM file WHERE ...`
   - `COMMIT` transaction

6. **File type conversion** (Step 8a):
   - Call `migrateMimeTypesForExistingFiles()`
   - Find files with NULL mime_type/file_type
   - Populate from extension
   - Batch update (1000 files at a time)

7. **Metadata extraction** (Step 9):
   - **IF** `includeMetadata != "None"`:
     - Query files WHERE `metadata_extraction_date IS NULL`
     - Filter by file_type based on metadata level:
       - Media_Basic/Extended: `file_type IN ('Image', 'Audio', 'Video')`
       - Full_Extended: all supported types
     - Extract in parallel batches using `ParallelMetadataExtractor`
     - Call `FileMetadata::batchUpdateFileMetadata()`
   - **ELSE**: Skip entirely

8. **Finalize**:
   - Update catalog stats (`fileCount`, `totalFileSize`)
   - Save catalog to DB
   - Save device stats
   - Update parent hierarchy

---

## **Metadata Extraction - Batching & Stopping**

### **Batch Processing**
- Uses `ParallelMetadataExtractor` with 4-8 threads (depending on CPU cores)
- Batch size: 100 files per batch
- Each batch:
  1. Extract metadata in parallel threads
  2. Collect results
  3. Call `batchUpdateFileMetadata()` to UPDATE all fields in single transaction
  4. Transaction commits → **metadata_extraction_date written to DB**

### **Stopping Behavior**
- Checks `shouldContinue()` between batches
- If stopped:
  - Current batch transaction commits (files 0-100 have metadata_extraction_date set)
  - Remaining files (101-43000) still have NULL metadata_extraction_date
- **Resumable**: Next update/search automatically finds files with NULL metadata_extraction_date and processes them

### **What Gets Written Per Batch**
All fields including:
- `metadata_extraction_date` (critical for resume detection)
- `file_type`, `mime_type` (even though inserted earlier, batch update includes them)
- All extracted metadata fields (image_width, audio_artist, etc.)
- `metadata_extended` JSON

---

## **SCENARIOS**

### **Scenario 1: Post-Migration to v2.8 (Search Triggers)**

#### **1.1: includeMetadata = "None", no file changes**
- **Expected**: Fast incremental update, seconds
- **Process**:
  1. Scan filesystem → 0 new, 0 modified, 0 deleted
  2. includeMetadata = "None" → skip metadata extraction (Step 9)
  3. Complete quickly

#### **1.2: includeMetadata = "None", file changes (2 new, 3 modified, 10 deleted)**
- **Expected**: Fast incremental update, seconds
- **Process**:
  1. Scan filesystem → find changes
  2. Insert/update/delete file records
  3. includeMetadata = "None" → skip metadata extraction
  4. Complete quickly

---

### **Scenario 2: Changing includeMetadata Option**

#### **2.1: "None" → "Media_Basic", no file changes**
- **Expected**: Extract metadata for all existing media files (~43k files)
- **Process**:
  1. Catalog saved with includeMetadata = "Media_Basic"
  2. Scan filesystem → 0 changes
  3. Step 9: Query `WHERE metadata_extraction_date IS NULL AND file_type IN ('Image','Audio','Video')`
  4. Extract metadata for ~43k files in batches of 100
  5. All files now have metadata_extraction_date populated

#### **2.2: "Media_Basic" → "Media_Extended"**, no file changes**
- **Expected**: Re-extract all media files with extended metadata
- **Process**:
  1. **In UI (`saveCatalogChanges`)**: Clear `metadata_extraction_date` for all files
  2. Save catalog with includeMetadata = "Media_Extended"
  3. Next update: Query finds all files with NULL metadata_extraction_date
  4. Re-extract with extended level

#### **2.3: "Media_Extended" → "Media_Basic"**, no file changes**
- **Expected**: Clear extended metadata, keep basic
- **Process**:
  1. **In UI**: Clear `metadata_extended` column only
  2. Save catalog
  3. Next update runs normally (metadata_extraction_date still populated, no re-extraction)

#### **2.4: "Media_Basic" → "Full_Extended"**, no file changes**
- **Expected**: Extract metadata for non-media files (media files already have metadata)
- **Process**:
  1. **In UI**: Clear `metadata_extraction_date` for all files
  2. Save catalog
  3. Next update: Extract for ALL supported file types (not just media)

#### **2.5: "Media_Extended" → "Full_Extended"**, no file changes**
- **Expected**: Extract metadata only for non-media files (media already have extended)
- **Process**:
  1. **In UI**: Do NOT clear metadata_extraction_date
  2. Save catalog
  3. Next update: Query finds files WHERE `metadata_extraction_date IS NULL` (only non-media files)
  4. Extract for those files only

#### **2.6: Any level → "None"**
- **Expected**: Clear all metadata fields to reduce DB size
- **Process**:
  1. **In UI**: Clear all metadata fields:
     - `metadata_extraction_date = NULL`
     - All image_*, video_*, audio_* fields = NULL
     - `metadata_extended = NULL`
  2. **Keep**: `file_extension`, `file_type`, `mime_type` (never cleared)
  3. Save catalog
  4. Next update: includeMetadata = "None" → skip extraction entirely

---

### **Scenario 3: Updates Without includeMetadata Changes**

#### **3.1: includeMetadata = "Media_Basic", all metadata populated, no file changes**
- **Expected**: Fast completion, seconds
- **Process**:
  1. Scan → 0 changes
  2. Step 9: Query `WHERE metadata_extraction_date IS NULL AND file_type IN ('Image','Audio','Video')` → 0 results
  3. Skip extraction, complete immediately

#### **3.2: includeMetadata = "Media_Basic", new files added**
- **Expected**: Extract metadata only for new files
- **Process**:
  1. Scan → find new files
  2. Insert new file records (no metadata_extraction_date)
  3. Step 9: Query finds only new files
  4. Extract metadata for new files only

---

## **QUESTIONS FOR YOU**

1. **Scenario 2 metadata clearing**: WHERE exactly should the clearing happen?
   - In `saveCatalogChanges()` immediately after saving to DB?
   - Or add methods like `catalog->clearMetadataFields()` and call before `saveCatalog()`?

2. **Scenario 2.2, 2.4**: Clearing metadata_extraction_date for ALL files - should this happen in UI or be detected automatically during update?

3. **Search-triggered conversion**: Currently shows "Converting" progress - is this acceptable UX or should it be silent?

4. **Order of operations** (section above) - is this accurate?

Is this specification now complete and clear for implementation?
