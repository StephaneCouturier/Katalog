# Feature Specification: VVV2K — Full Import of a VVV Database into Katalog

**Version:** 2.0 draft
**Date:** 2026-03-16
**Status:** Draft — awaiting mapping decisions before implementation
**Delivery:** Standalone Python tool (`vvv2katalog.py`), no Katalog C++ changes

---

## 1. Context & Motivation

[VVV (Virtual Volumes View)](https://www.fsoft.it/VVV/index.html) is a mature, cross-platform (Windows/Linux/macOS) open-source disk cataloging application (GPL-2.0). It catalogs removable volumes (CDs, DVDs, USB drives) for offline searching and allows organizing files into a virtual folder hierarchy. VVV stores its data in a **Firebird** relational database (`.vvv` or `.fdb` files), using an embedded Firebird 2.0.x/2.1.x engine.

Katalog already has a basic `importFromVVV()` method that reads a VVV CSV export (tab-separated). However, this requires a manual export from VVV first, only captures physical files (no virtual folders, no audio metadata), and loses data fidelity.

This feature provides a **standalone converter tool (VVV2K)** that reads a VVV Firebird database and produces a complete, ready-to-use Katalog SQLite database file. The user then opens this `.db` file directly in Katalog — no import step needed.

### 1.1 Existing VVV CSV Import (preserve as-is)

The current `importFromVVV()` in `mainwindow_tab_device_pr.cpp` remains unchanged. It imports from a VVV CSV export file and is already working. VVV2K is an independent, more complete alternative path.

---

## 2. Architecture

### 2.1 Overview

```
   ┌──────────────┐         ┌─────────────────────┐         ┌────────────────┐
   │  VVV .vvv    │         │  VVV2K               │         │  Katalog       │
   │  (Firebird)  │────────▶│  vvv2katalog.py      │────────▶│  "Open DB"     │
   │              │         │  outputs Katalog .db  │         │  (Settings)    │
   └──────────────┘         └─────────────────────┘         └────────────────┘
                             Interactive CLI:
                             may ask user decisions
```

**VVV2K (`vvv2katalog.py`):**
- Reads the `.vvv` Firebird database using the Python `fdb` library
- May prompt the user for mapping decisions (e.g., device types, grouping)
- Outputs a **complete, valid Katalog SQLite `.db` file** with full schema, correct schema version, proper IDs, and computed statistics
- The output file is directly openable in Katalog via Settings > "Select database file"

**No Katalog C++ changes.** The only future Katalog-side work would be a separate, generic "import/merge another Katalog database" feature — that is a different specification.

### 2.2 Rationale

| Concern | Decision |
|---------|----------|
| Firebird dependency | **Isolated in Python.** Firebird client libs are ~30 MB, use old ODS formats (11.0), and complicate cross-platform C++ builds. Python `fdb` handles this cleanly. |
| Output format | **Native Katalog .db.** No intermediate format, no import step. The user gets a file they can open directly. |
| User decisions | **Interactive CLI prompts.** Some mapping decisions may require human judgment (e.g., how to group volumes). The CLI asks during conversion. |
| License | VVV is GPL-2.0. VVV2K reads the database file; no VVV code is linked or included. No license concern. |

---

## 3. VVV Data Model (Firebird)

### 3.1 Source Information

Reconstructed from:
- VVV 1.5 source code (GPL-2.0) at [github.com/pabloveliz/vvv](https://github.com/pabloveliz/vvv)
- SQL traces in VVV SourceForge forum posts (INSERT statements, ALTER TABLE statements, error messages)
- VVV forum discussions about database access (table SERVICE, FK_FILES_PATHS constraint)

**IMPORTANT: The exact schema must be confirmed from a real VVV database before implementation (see Phase 0 in section 8).** The tables and columns below are best estimates. VVV2K includes a runtime schema-discovery step to handle version differences.

### 3.2 Concepts

VVV organizes data around these concepts:

- **Catalog file** (`.vvv`): A single Firebird database file containing all data for one catalog. A user may have multiple catalog files.
- **Volume**: A physical disk/drive that has been cataloged (CD, DVD, USB, hard drive). Each volume has a name, the path where it was scanned, and catalog/update dates.
- **Path**: A directory within a volume. Paths form a **tree** (self-referencing parent-child via `PARENT_PATH_ID`). Each path stores only its own directory name component, not the full path.
- **File**: An individual file within a path. Stores name (without extension), extension separately, size, and modification timestamp.
- **Virtual folder**: A user-created logical grouping. Virtual folders form their own tree hierarchy (separate from paths). Physical volumes, paths, or individual files can be **assigned** to virtual folders. A single item can appear in multiple virtual folders.
- **Audio metadata**: Optional per-file metadata for audio files (artist, title, album, year, genre, track, comment). VVV uses TagLib to extract these at catalog time.
- **Service**: A housekeeping table storing the database schema version.

### 3.3 Core Tables

**SERVICE** — Database version tracking
```sql
SERVICE (
    SERVICE_ID          INTEGER PRIMARY KEY,
    DB_VERSION          INTEGER              -- schema version number
)
```

**VOLUMES** — Physical volumes (disks, drives)
```sql
VOLUMES (
    VOLUME_ID           INTEGER PRIMARY KEY,  -- generator-based sequence
    VOLUME_NAME         VARCHAR,              -- user-visible name (e.g. "Backup_2024")
    VOLUME_DESCRIPTION  BLOB SUB_TYPE 2,      -- optional text description (added in VVV 1.3)
    VOLUME_CATALOG_DATE TIMESTAMP,            -- when first cataloged
    VOLUME_UPDATE_DATE  TIMESTAMP,            -- when last updated/rescanned
    VOLUME_PHYSICAL_PATH VARCHAR              -- path when cataloged (e.g. "/media/cdrom", "E:\")
)
```

**PATHS** — Directory tree within volumes
```sql
PATHS (
    PATH_ID             INTEGER PRIMARY KEY,  -- generator-based sequence
    VOLUME_ID           INTEGER,              -- FK → VOLUMES
    PARENT_PATH_ID      INTEGER,              -- FK → PATHS (self-referencing; NULL for volume root)
    PATH_NAME           VARCHAR,              -- single directory name component (NOT full path)
    PATH_DESCRIPTION    BLOB SUB_TYPE 2       -- optional text description
)
-- Tree example:
--   PATH_ID=1, VOLUME_ID=1, PARENT=NULL, NAME="photos"       → /photos
--   PATH_ID=2, VOLUME_ID=1, PARENT=1,    NAME="2024"         → /photos/2024
--   PATH_ID=3, VOLUME_ID=1, PARENT=2,    NAME="january"      → /photos/2024/january
```

**FILES** — Individual files
```sql
FILES (
    FILE_ID             INTEGER PRIMARY KEY,  -- generator-based sequence
    FILE_NAME           VARCHAR,              -- filename WITHOUT extension (e.g. "sunset")
    FILE_EXT            VARCHAR,              -- extension only (e.g. "jpg"), may be NULL
    FILE_SIZE           BIGINT,               -- file size in bytes
    FILE_DATETIME       TIMESTAMP,            -- file modification date/time
    PATH_FILE_ID        INTEGER,              -- (nullable, possibly legacy/unused)
    PATH_ID             INTEGER,              -- FK → PATHS (constraint FK_FILES_PATHS)
    FILE_DESCRIPTION    BLOB SUB_TYPE 2       -- optional text description
)
-- Full filename = FILE_NAME + "." + FILE_EXT (when FILE_EXT is not NULL)
-- Full path = reconstructed by walking PATHS tree from PATH_ID to root
```

### 3.4 Virtual Folder Tables

**VIRTUAL_FOLDERS** — User-created virtual folder hierarchy
```sql
VIRTUAL_FOLDERS (
    VFOLDER_ID          INTEGER PRIMARY KEY,
    PARENT_VFOLDER_ID   INTEGER,              -- FK → VIRTUAL_FOLDERS (NULL = root)
    VFOLDER_NAME        VARCHAR               -- display name
)
```

**VIRTUAL_FILES** — Assignments of physical items to virtual folders
```sql
VIRTUAL_FILES (
    VFOLDER_ID          INTEGER,              -- FK → VIRTUAL_FOLDERS
    FILE_ID             INTEGER,              -- FK → FILES (nullable)
    PATH_ID             INTEGER,              -- FK → PATHS (nullable, for folder assignments)
    VOLUME_ID           INTEGER               -- FK → VOLUMES (nullable, for volume assignments)
)
-- Each row assigns ONE item (file, path, or volume) to a virtual folder.
-- A single item can appear in multiple virtual folders (multiple rows).
```

### 3.5 Audio Metadata Table

```sql
-- Exact table name TBD: possibly AUDIO_METADATA or FILES_AUDIO
AUDIO_METADATA (
    FILE_ID             INTEGER,              -- FK → FILES
    TAG_TITLE           VARCHAR,
    TAG_ARTIST          VARCHAR,
    TAG_ALBUM           VARCHAR,
    TAG_YEAR            VARCHAR,
    TAG_COMMENT         VARCHAR,
    TAG_TRACK           INTEGER,
    TAG_GENRE           VARCHAR
)
```

### 3.6 Key Characteristics and Gotchas

- **Path reconstruction required:** VVV stores paths as a parent-child tree. To get a full path string (e.g., `/photos/2024/january`), you must walk from a PATH_ID up through PARENT_PATH_ID links to the root. This is the most complex part of the conversion.
- **Filename split:** `FILE_NAME` and `FILE_EXT` are stored separately. Some files may have NULL extension.
- **Firebird encoding:** VVV databases typically use `ISO8859_1` (Western European) character set. Must convert to UTF-8 for SQLite.
- **Timestamp bugs:** VVV has known bugs producing invalid dates (e.g., `292278994-08-16 17:47:04`). The converter must sanitize these.
- **Firebird ODS versions:** VVV ships with embedded Firebird 2.0.x (ODS 11.0). Users may have databases from FB 2.1.x (ODS 11.1), FB 2.5.x (ODS 11.2), or rarely FB 3.x (ODS 12.0). The Python `fdb` library with a FB 2.5.x client can read ODS 11.0–11.2.

---

## 4. Katalog Data Model (target)

The complete Katalog SQLite schema as defined in `core/database.h` and `core/database.cpp`. 
VVV2K must create **all** of these tables in the output `.db` file, even if some are left empty. 
Latest released schema version is **2.10**; current dev version is **2.11**.

### 4.1 Tables Created by VVV2K

| Table | Populated from VVV? | Notes |
|-------|---------------------|-------|
| `device` | ✅ Yes | VVV volumes → Katalog devices |
| `catalog` | ✅ Yes | One catalog per VVV volume |
| `file` | ✅ Yes | VVV files with reconstructed paths |
| `folder` | ✅ Yes | Derived from path reconstruction |
| `filetemp` | ❌ Empty | Temporary table, not needed |
| `storage` | ⚠️ TBD | Depends on mapping decisions |
| `metadata` | ✅ Yes | VVV audio metadata → key-value pairs |
| `device_mapping` | ❌ Empty | Backup mappings, not from VVV |
| `search` | ❌ Empty | Search history, not from VVV |
| `tag` | ❌ Empty | Tags, not from VVV |
| `parameter` | ✅ Yes | Schema version + basic parameters |
| `statistics_device` | ❌ Empty | Snapshots, not from VVV |

### 4.2 Full Table Schemas

see Katalog repo.

### 4.3 Schema Version & Parameter Table

VVV2K must write the `parameter` table with at least the schema version entry so that Katalog recognizes the database:

```sql
INSERT INTO parameter (parameter_name, parameter_type, parameter_value1, parameter_value2)
VALUES ('SchemaVersion', 'schema', '2.11', NULL);
```

*(Schema version should match the current development version so migrations don't run.)*

---

## 5. Data Mapping: VVV → Katalog

### 5.1 Status

**⚠️ TO BE DEFINED** — The mapping decisions below require human judgment. The sections marked "TBD" will be filled in by the Katalog developer before implementation begins.

### 5.2 Physical Data (clear mapping)

These mappings are straightforward and can be implemented directly:

**Path tree → flat path strings:**
VVV2K must walk the VVV `PATHS` parent-child tree and reconstruct full path strings for each PATH_ID. This is a prerequisite for all file and folder data.

**File name reconstruction:**
`FILE_NAME` + `"."` + `FILE_EXT` (handle NULL extension → just `FILE_NAME`).

**Timestamp conversion:**
Firebird TIMESTAMP → ISO 8601 text string. Invalid dates (e.g., `292278994-08-16`) → NULL or a sentinel value. (date of import?)

**Encoding conversion:**
Firebird `ISO8859_1` → UTF-8 for all text fields.

### 5.3 Mapping Decisions Needed

The following require explicit decisions before implementation:

**5.3.1 VVV Volume → Katalog device + storage + catalog**

| Question | Options | Decision |
|----------|---------|----------|
| What `device_type` for imported volumes? | "Catalog" / "Storage" / ask user per volume | _TBD_ |
| Device tree structure? | All under one Virtual parent? Group by original VVV catalog file? Flat? | _TBD_ |
| `device_group_id`? | 0 (Physical) or 1 (Virtual) or ask? | _TBD_ |
| `catalog_source_path`? | Use `VOLUME_PHYSICAL_PATH` from VVV? | _TBD_ |
| `catalog_storage`? | Create a storage entry per volume? Leave empty? | _TBD_ |
| Volume descriptions? | Map to which field? (No direct equivalent in Katalog device/catalog) | _TBD_ |

**5.3.2 VVV Virtual Folders → Katalog virtual_storage**

| Question | Options | Decision |
|----------|---------|----------|
| Import virtual folders at all? | Yes / No / Optional flag | _TBD_ |
| Hierarchy mapping? | Direct tree → virtual_storage parent-child? | _TBD_ |
| Single-file assignments? | VVV allows assigning individual files to virtual folders. Katalog virtual_storage_catalog uses catalog_name + directory_path. How to handle? | _TBD_ |
| Volume-level assignments? | VVV allows assigning entire volumes. Map how? | _TBD_ |

**5.3.3 VVV Audio Metadata → Katalog metadata**

| Question | Options | Decision |
|----------|---------|----------|
| Key naming convention? | `"audio:title"`, `"audio:artist"`, etc.? Or match KF6 FileMetaData property names? | _TBD_ |
| Import audio metadata at all? | Yes / No / Optional flag | _TBD_ |

**5.3.4 User Prompts During Conversion**

| Question | Options | Decision |
|----------|---------|----------|
| Which decisions need user prompts vs. sensible defaults? | _(list TBD)_ | _TBD_ |
| CLI interactive mode vs. config file vs. command-line flags? | _(TBD)_ | _TBD_ |

---

## 6. VVV2K Tool Specification

### 6.1 Dependencies

```
pip install fdb          # Firebird driver (supports FB 2.x embedded/server)
```

The script needs the Firebird client library (`libfbclient.so` / `fbclient.dll`). Options:
- Install Firebird 2.5.x client (last version supporting ODS 11.x)
- Point to VVV's own embedded Firebird libraries (shipped with VVV)

### 6.2 Usage

```bash
python3 vvv2katalog.py <input.vvv> <output.db> [options]

Options:
  --fb-client-lib PATH    Path to Firebird client library
  --fb-user USER          Firebird user (default: SYSDBA)
  --fb-password PASS      Firebird password (default: masterkey)
  --skip-virtual          Skip virtual folder import
  --skip-audio            Skip audio metadata import
  --verbose               Print progress to stdout
  --dry-run               Analyze VVV database without producing output
  --non-interactive       Use defaults for all decisions (no prompts)
```

### 6.3 Processing Steps

```
1. Connect to Firebird database (handle ODS version mismatch)
2. Discover actual schema (query Firebird system tables)
3. Read SERVICE table → get VVV DB_VERSION
4. Create output SQLite file with full Katalog schema (all tables, latest version)
5. Write parameter table with schema version
6. Resolve PATHS tree → build PATH_ID → full_path_string map
7. Ask user decisions (or use defaults in --non-interactive mode)
8. Convert VOLUMES → device + catalog entries (with proper IDs, statistics)
9. Convert FILES → file entries (with reconstructed paths, combined name.ext)
10. Derive FOLDERS from file paths → folder entries
11. Convert VIRTUAL_FOLDERS → virtual_storage entries (if not --skip-virtual)
12. Convert VIRTUAL_FILES → virtual_storage_catalog / assignments (if not --skip-virtual)
13. Convert AUDIO_METADATA → metadata entries (if not --skip-audio)
14. Compute and fill device statistics (total_file_size, total_file_count)
15. Verify output integrity (row counts, referential consistency)
16. Report summary
```

### 6.4 ID Generation

VVV2K generates Katalog-style IDs (large integers, typically timestamp-based). It must ensure:
- All `device_id` values are unique
- All `catalog_id` values are unique
- `device_external_id` links correctly to `catalog_id` or `storage_id`
- No collisions with IDs that might exist in other Katalog databases (use current-timestamp-based generation, same as Katalog's `generateDeviceID()`)

### 6.5 Error Handling

| Error | Behavior |
|-------|----------|
| Firebird version mismatch (ODS not supported) | Clear error message explaining which FB client version is needed |
| Corrupted timestamps (e.g., `292278994-08-16`) | Sanitize to NULL, log warning, continue |
| Missing tables (older VVV version) | Skip gracefully, warn which data was not imported |
| Character encoding issues | Convert ISO8859_1 → UTF-8, replace unconvertible bytes with `?` |
| Duplicate volume names | Append suffix (e.g., `_2`) to ensure unique catalog names |
| Empty database | Produce valid empty Katalog database, warn |

---

## 7. Firebird Version Compatibility

| VVV Version | Firebird Engine | ODS | Python `fdb` with FB 2.5 client | Notes |
|-------------|----------------|-----|------|-------|
| 0.9–1.2 | FB 2.0.x embedded | 11.0 | ✅ | Oldest versions |
| 1.3–1.5 | FB 2.0.x/2.1.x embedded | 11.0–11.1 | ✅ | Current release (most common) |
| User-upgraded | FB 2.5.x server | 11.2 | ✅ | Some users |
| User-upgraded | FB 3.x server | 12.0 | ❌ (needs `firebird-driver`) | Rare; via backup/restore |

**Recommendation for users:** Install Firebird 2.5.x client library (last version supporting ODS 11.x). On Linux, this may require downloading from [firebirdsql.org](https://www.firebirdsql.org/en/firebird-2-5/) as distributions typically only package FB 3.x+.

---

## 8. Implementation Plan — Claude Code Guidance

### Phase 0: Schema Discovery (Day 1)

**Goal:** Confirm the actual VVV schema from a real database.

1. Create a test VVV database with VVV (portable version or Linux):
   - 2–3 volumes with nested folders and files
   - Virtual folder assignments
   - A few audio files with metadata
2. Write a small Python script to dump the real Firebird schema:
```python
# Query: SELECT RDB$RELATION_NAME FROM RDB$RELATIONS WHERE RDB$SYSTEM_FLAG = 0
# Query: SELECT RDB$FIELD_NAME FROM RDB$RELATION_FIELDS WHERE RDB$RELATION_NAME = ?
```
3. Update section 3 of this spec with confirmed table/column names
4. Identify any tables or columns not accounted for

**Claude Code prompt:**
> Write a Python script `tools/vvv_schema_dump.py` that connects to a VVV Firebird database (.vvv file) using the `fdb` library and dumps the complete schema: table names, column names, types, constraints, and row counts. Accept --fb-client-lib, --fb-user, --fb-password arguments.

### Phase 1: Core Converter — Volumes + Files (Days 2–4)

**Claude Code prompts (sequential):**

1. > Create `tools/vvv2katalog.py`. Step 1: schema discovery function + Firebird connection with proper error handling for ODS mismatches. Step 2: create output SQLite with the complete Katalog schema (all tables from database.h, including migration-added columns). Write the parameter table with schema version "2.11".

2. > Add to `vvv2katalog.py`: path tree resolution. Read the VVV PATHS table, build a PATH_ID → full_path_string dictionary by walking parent links. Handle the root path case. Include tests with synthetic data.

3. > Add to `vvv2katalog.py`: volume and file conversion. Read VOLUMES and FILES, create device + catalog entries in the output Katalog DB. Use timestamp-based ID generation. Combine FILE_NAME + FILE_EXT. Insert files with reconstructed folder paths. Derive and insert folder entries. Compute per-device statistics.

### Phase 2: Virtual Folders + Metadata (Days 5–6)

*(Blocked on mapping decisions in section 5.3. Start once those are filled in.)*

1. > Add to `vvv2katalog.py`: virtual folder conversion per the mapping decisions in the spec section 5.3.2.

2. > Add to `vvv2katalog.py`: audio metadata conversion per the mapping decisions in the spec section 5.3.3.

### Phase 3: CLI Polish + Testing (Days 7–8)

1. > Add to `vvv2katalog.py`: CLI argument parsing (argparse), interactive user prompts for mapping decisions, --non-interactive mode with defaults, progress reporting, --dry-run mode, and a summary report at the end.

2. > Add a self-test mode to `vvv2katalog.py` that creates a mock Firebird-like SQLite database (simulating VVV's schema), converts it, then verifies the output Katalog database is valid: all tables exist, schema version is correct, file counts match, folder paths are properly reconstructed.

---

## 9. Testing Strategy

### 9.1 Python Converter Tests

- **Path tree resolution:** Synthetic PATHS tree → verify full path strings
- **File name combining:** `("sunset", "jpg")` → `"sunset.jpg"`, `("README", NULL)` → `"README"`
- **Timestamp sanitization:** Valid dates pass through, `292278994-08-16` becomes NULL
- **Encoding:** ISO8859_1 special characters (é, ñ, ü) convert correctly to UTF-8
- **Empty volumes:** Volume with no files produces valid catalog with 0 counts
- **Deeply nested paths:** 50+ levels of nesting resolve correctly

### 9.2 Output Database Validation

- Open the output `.db` in Katalog (File mode) — does it load without errors?
- Are all devices visible in the device tree?
- Can files be searched?
- Can catalog contents be explored?
- Are statistics correct (file counts, total sizes)?
- Does Katalog's version check accept the schema version?

### 9.3 Test Data

Create a reference VVV database containing:
- 3 volumes: one small (10 files), one medium (1,000 files), one with deep nesting
- Files with special characters in names (Unicode, spaces, dots)
- Audio files with metadata
- Virtual folders with file, path, and volume assignments
- At least one file with a corrupted timestamp

---

## 10. Risks & Mitigations

| Risk | Likelihood | Impact | Mitigation |
|------|-----------|--------|------------|
| VVV schema differs from documented | Medium | High | Phase 0 confirms schema before coding |
| Firebird 2.5 client hard to install on modern Linux | Medium | Medium | Document steps; allow pointing to VVV's own libs |
| Large VVV databases (1M+ files) slow | Low | Medium | Batch processing, progress reporting |
| Virtual folder mapping doesn't fit cleanly | Medium | Low | Separate phase, flagged as optional |
| Python not available on user's system | Low | Low | Document requirement; could package with PyInstaller later |

---

## 11. Future Considerations (out of scope)

- **Generic "merge Katalog databases"** feature in Katalog itself — separate specification
- **Direct .vvv file handling in Katalog UI** via QProcess calling VVV2K
- **Incremental re-import** — detect changes in a VVV database that was previously converted
- **VVV `.fbk` backup file support** — would require Firebird `gbak` tool
- **PyInstaller packaging** — bundle VVV2K as a standalone executable (no Python needed)

---

## 12. Glossary

| Term | Definition |
|------|-----------|
| VVV | Virtual Volumes View — the source application |
| VVV2K | The converter tool specified here (`vvv2katalog.py`) |
| Firebird | Open-source relational database engine used by VVV |
| ODS | On-Disk Structure — Firebird database file format version |
| `.vvv` / `.fdb` | VVV catalog file (a Firebird database file) |
| Volume (VVV) | A cataloged disk/drive in VVV |
| Virtual Folder (VVV) | User-created logical grouping in VVV |
| `fdb` | Python library for connecting to Firebird databases |
