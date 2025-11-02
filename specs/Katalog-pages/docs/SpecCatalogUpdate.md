# Updates with metadata

## **Introduction**
This document specifices the use and Implementation of catalog and related files updates in the context of the use of File Metadata.

### Key features & technical changes in v2.8

#### 1. File Type fields
- Feature: enhanced file type management using KFileMetadata extension list.
- `file_type` has a specific definition in Katalog: see [Create#enhanced-file-type-filtering](Create#enhanced-file-type-filtering)
- New database columns: `file_extension`, `file_type`, `mime_type`, `mime_verified`, `type_mismatch`
- `file_extension` and `file_type` are required for Search functionality to work


#### 2. Metadata Fields
- To be populated based on `includeMetadata` field of the table `catalog`.
- see settings: **None, Media_Basic, Media_Extended, Full_Extended** [Create#metadata-extraction](Create#metadata-extraction)
- New database columns in `file` and `filetemp` tables for Media_Basic: 
<br/>- Image: `image_width`, `image_height`, `image_orientation`
<br/>- Video: `video_duration_seconds`, `video_width`, `video_height`, `video_codec`, `video_framerate`, `video_bitrate`
<br/>- Audio: `audio_duration_seconds`, `audio_bitrate`, `audio_sample_rate`, `audio_artist`, `audio_album`,`audio_title`, `audio_genre`, `audio_year`, `audio_track_number`
- New database columns in `file` and `filetemp` tables for Media_Extended and Full_Extended: 
<br/>- `metadata_extended` TEXT (JSON)
- New database columns in `file` and `filetemp` tables for overall metadata management: 
- `metadata_extraction_date` TEXT (ISO format timestamp)


### 3. Main user needs
- The User shall be able to transition from Katalog v2.7 (or earlier) to v2.8 without being forced to run long batch process immediately
- The User shall be able to see when Katalog is running specific conversion processes and their progress
- The User shall be able to change a catalog includeMetadata option at any time and to decide when to run the updates










---





#### 2. Incremental Indexing
- New mechanism limiting metadata extraction processing:
- **includeMetadata = "None"**: Only updates  exact file changes (unchanged/deleted/added/modified)
- **includeMetadata != "None"**: also extracts metadata for changed files or files missing `metadata_extraction_date`




## **Database Schema: Metadata Fields**





#### 2. Incremental Indexing
- New mechanism limiting metadata extraction processing:
- **includeMetadata = "None"**: Only updates  exact file changes (unchanged/deleted/added/modified)
- **includeMetadata != "None"**: also extracts metadata for changed files or files missing `metadata_extraction_date`



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
  - Remaining files (ex 101-43000) still have NULL metadata_extraction_date
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

#### **2.1: "None" → "any other includeMetadata value", no file changes**
- **Expected**: Extract metadata for all existing media files
- **Process**:
  1. Catalog saved with includeMetadata = "Media_Basic" or "Media_Extended" or "Full_Extended"
  2. Scan filesystem → 0 changes
  3. Specific for "Media_Basic" or "Media_Extended", step 9 query `WHERE metadata_extraction_date IS NULL AND file_type IN ('image','audio','video')`
  4. Extract metadata in batches of 100
  5. All files now have metadata_extraction_date populated

#### **2.2: "Media_Basic" → "Media_Extended"**, no file changes**
- **Expected**: Re-extract all media files with extended metadata
- **Process**:
  1. Save catalog with includeMetadata = "Media_Extended" and clear `metadata_extraction_date` for all the catalog's files
  2. Next update: same specific for "Media_Basic" or "Media_Extended",  step 9 query `WHERE metadata_extraction_date IS NULL AND file_type IN ('image','audio','video')`
  3. Re-extract with extended level

#### **2.3: "Media_Extended" → "Media_Basic"**, no file changes**
- **Expected**: Clear extended metadata, keep basic
- **Process**:
  1. Save catalog  with includeMetadata = "Media_Basic" and clear `metadata_extended` column only
  2. Next update runs normally (metadata_extraction_date still populated, no re-extraction)

#### **2.4: "Media_Extended" → "Full_Extended"**, no file changes**
- **Expected**: Extract metadata only for non-media files (media already have extended)
- **Process**:
  1. Save catalog  with includeMetadata = "Full_Extended". DO NOT clear metadata_extraction_date
  2. Next update: Query finds files WHERE `metadata_extraction_date IS NULL` (only non-media files)
  3. Extract for those files only

#### **2.5: "Media_Basic" → "Full_Extended"**, no file changes**
- **Expected**: Extract metadata for non-media files (Media files already have metadata)
- **Process**:
  1. Save catalog with includeMetadata = "Full_Extended", and clear `metadata_extraction_date` for all files (so as to trigger extended metadata extraction for existing files with Media_Basic)
  2. Next update: Extract for ALL supported file types (not just media)

  Note: we could imagine only extracting extended metadata for existing files, but that would complexify the process for low benefits (anyhow those files are to be processed and extracting the basic info again should not be that much longer as already dealing with the extended part).
  
  
  
  
#### **2.6: "Full_Extended" → "Media_Basic"**, no file changes**
- **Expected**: Extract metadata only for non-media files (media already have extended)
- **Process**:
  1. Save catalog  with includeMetadata = "Full_Extended". Clear metadata_extended, clear metadata_extraction_date for non Media files.
  2. Next update: Query finds files WHERE `metadata_extraction_date IS NULL` (only for Media files)
  3. Extract for those new files only
  
#### **2.7: "Full_Extended" → "Media_Extended"**, no file changes**
- **Expected**: Extract metadata only for non-media files (media already have extended)
- **Process**:
  1. Save catalog  with includeMetadata = "Full_Extended". Clear metadata_extraction_date for non Media files.
  2. Next update: Query finds files WHERE `metadata_extraction_date IS NULL` (only for Media files)
  3. Extract for those new files only
  
  
  
  
  
#### **2.8: Any level → "None"**
- **Expected**: Clear all metadata fields to reduce DB size
- **Process**:
  1. Save catalog with includeMetadata = "None", and clear all metadata fields:
     - `metadata_extraction_date = NULL`
     - All image_*, video_*, audio_* fields = NULL
     - `metadata_extended = NULL`
  2. **Keep**: `file_extension`, `file_type`, `mime_type` (never cleared)
  3. Next update: includeMetadata = "None" → skip extraction entirely

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
  
## Sequencing
  
  Robustness & Incremental Philosophy
Option B aligns perfectly with your incremental design:

File changes are incremental (only new/modified)
Metadata extraction is incremental (only files with NULL extraction date)
Each phase can be stopped and resumed independently
Database state is always consistent (all files indexed, metadata is additive)

My Strong Recommendation
Option B (Sequential) for these reasons:

✅ Already your design - just needs progress reporting
✅ Search availability - usable after Phase 1
✅ Simple stop/resume - clean phase boundaries
✅ User flexibility - can postpone metadata
✅ Clear progress - two distinct phases
✅ Database consistency - no mixed states
✅ Matches incremental philosophy

The only thing needed is: Add progress reporting to Phase 2 (metadata extraction)
Should we proceed with Option B and discuss the progress reporting implementation?



    const int BATCH_SIZE = 100;  // Process 100 files per batch as per spec
  
