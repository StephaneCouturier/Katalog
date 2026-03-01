# BackUp management

## Introduction
This document specifices the implementation of next features for the **[BackUp](BackUp) capability**.

## Scope
Katalog's backup feature syncs files from a **source catalog** to a **target catalog** using Katalog's own indexed data. No external tool dependency (rsync optional export only).

## Prerequisites
- Both catalogs must belong to devices with valid, accessible paths.

## **Existing Features & Architecture**

Katalog enables the definition of links between Catalogs, defining one as Source and the other as Target.<br/>
This help checking the coverage of backup for devices, and compare source and backup target size, number of files and date updated.<br/>
see: [BackUp](BackUp)

Table _device_mapping_
```
device_mapping
      mapping_id                  %1,
      mapping_name                TEXT,
      mapping_type                TEXT,
      mapping_device_source_id    %2,
      mapping_device_target_id    %2,
      mapping_backup_last_date    TEXT,
      mapping_backup_last_size    %2,
      mapping_strict_copy         INTEGER DEFAULT 1,
      mapping_conflict_mode       INTEGER DEFAULT 0)

```

**Device Properties (from code):**
- `device_id`, `device_name`, `device_path`, `device_type`
- `device_active` (bool - indicates if path is reachable)
- `device_total_file_size`, `device_total_file_count`
- `device_date_updated`
- Device with `type='Catalog'` represents indexed directories

**UI Components:**
- `mainwindow_tab_backup.cpp` - BackUp screen implementation
- TreeViews for Source/Target selection
- TableView for mapping display with comparison metrics
- Buttons: SaveMapping, DeleteMapping, Reload lists

---


## Core Behavior (v1 — Incremental Copy)

### What it does
- Compares source and target catalogs to find files **missing from the target**.
- Copies missing files to the target, recreating folder structure via `QDir::mkpath()`.
- Does **not** overwrite existing files in the target (even if different).
- Does **not** delete files from the target that are absent from the source.

### Comparison criteria (v1)
- Match by **file name + relative folder path** (same file in same relative location).
- A file is "missing" if no match exists in the target catalog.

### What it reports
After execution, a **backup report** lists:
- Files copied (count, total size)
- Files skipped — already exist in target (count)
- Files with conflicts — exist in target but differ (newer date, different size, different checksum). Listed for user review, **not overwritten**.
- Errors — files that failed to copy (permission denied, disk full, etc.)

## Decisions Log

| Decision | Choice | Rationale |
|----------|--------|-----------|
| Delete files in target? | No (v1). Future backlog item. | Start safe — incremental only. |
| Overwrite conflicts? | No (v1). Report them. | Avoid data loss. Future: user choice per-file. |
| Create missing directories? | Yes, always. | Required for any file copy to work. |
| Engine dependency? | Qt `QFile::copy()` only. | Cross-platform, no rsync needed. |
| Catalog requirement? | Both must be cataloged. | Engine compares catalog data, not filesystem. |
| rsync export? | Keep as separate advanced option. | For Linux power users. |

## Future Backlog (not v1)

- **Delete mode**: opt-in option to remove target files absent from source.
- **Overwrite mode**: options per conflict (skip, overwrite, keep both, ask).
- **Checksum comparison**: detect content changes even when name/size/date match.
- **Date-based comparison**: copy only files newer in source than in target.
- **Scheduled/automated backup**: run on timer or on catalog update.
- **Backup history**: log of past backup runs with dates and statistics.



## **#000 Native BackUp execution**

### Goals
1. Full backup within Katalog (rsync-based is 1 possbility, to be investigated and confirmed)
2. No external dependencies (except rsync if retained)
3. Cross-platform support
4. KDE integration (KJob, notifications) (consider KBackUp)

### Architecture Vision


**Features Roadmap:**
- [ ] Basic file copy (rsync)
- [ ] Incremental backups
- [ ] Snapshot management
- [ ] Exclude/include patterns
- [ ] Progress tracking
- [ ] Scheduling (cron/systemd/Task Scheduler)
- [ ] Restore functionality
- [ ] Compression options
- [ ] Remote backups (ssh)
- [ ] Verification

---


# Implementation Phases

## Phase 1: CatalogDifferenceEngine (core)

**Goal**: Reusable comparison engine, no UI changes yet.

**New files**:
- `src/core/catalogdifferenceengine.h`
- `src/core/catalogdifferenceengine.cpp`

**Deliverable**: Given two sets of device IDs and comparison fields, returns a `DifferenceResult` with files only-in-source, only-in-target, and in-both-but-different.

**Validation**: Can be called from a test or debug button. SQL queries extracted from `searchjobstoppable.cpp::processDifferences()`.

## Phase 2: Refactor Search Differences

**Goal**: Search > Differences uses `CatalogDifferenceEngine` instead of inline SQL.

**Modified files**:
- `src/core/searchjobstoppable.cpp` — `processDifferences()` delegates to engine
- `CMakeLists.txt` — add new source files

**Validation**: Search differences behaves identically to before.

## Phase 3: Backup Preview

**Goal**: User can see what a backup would do before executing.

**UI changes**:
- New button: "Preview Backup"
- Summary label: `Preview [mode] — To copy: N file(s) (X GB) | Conflicts: M | Already in target: K`
- Table: Status ("to copy" / "conflict"), File Name, Path, Size
- Mode shown in summary: "strict copy" or "dedup"

**Preview categories** (consistent with executor report):
- **to copy**: file needs to be copied to target
- **conflict**: file exists at target path (or anywhere for dedup) but has different size — will be skipped
- **already in target**: count only (files that match and need no action)

**Comparison**: `MainWindow::compareForBackup()` delegates to strict or dedup mode.

## Phase 4: Backup Executor

**Goal**: Actually copy files.

**New files**:
- `src/core/backupjob.h` — `BackupReport` struct (copied, conflicts, errors, totalBytesCopied, wasCancelled)
- `src/core/backupjobstoppable.h/.cpp` — QObject worker that runs in a QThread

**Behavior**:
- Takes `filesToCopy` list (from `compareForBackup()`) as input
- For each file: `QDir::mkpath()` + `QFile::copy()`
- File-system conflict (target file already exists despite catalog check) → added to `report.conflicts`, not overwritten
- Emits progress signals (files done, bytes copied, current file)
- Supports cancellation via `QAtomicInt m_stopRequested`
- Produces a `BackupReport` (copied, conflicts, errors)

**Progress reporting**:
- Signal: `backupProgress(int filesDone, int totalFiles, qint64 bytesCopied, qint64 totalBytes, QString currentFile)`
- Granularity: emitted before each file starts
- Progress bar: byte-based (0–1000 scale) — more accurate than file count for mixed file sizes
- Status label during run: `Copying X/N: filename  —  1.2 GB / 4.5 GB  |  45 MB/s  |  ETA: 1m 23s`
  - Speed and ETA shown only after ≥ 500 ms elapsed (avoids wild numbers at start)
  - Speed = total bytes copied / total elapsed seconds (cumulative average, stable)
  - ETA = (totalBytes − bytesCopied) / speed
- Status label on finish: `Backup complete — X GB copied in Ym Zs` (or cancelled variant)
- Note: per-file chunk progress is a future enhancement (requires replacing `QFile::copy()`)

**UI changes**:
- Execution panel (hidden until backup starts): status label, progress bar, cancel button
- Report display after completion (reuses preview table)

### Strict Copy vs Dedup — `mapping_strict_copy` option

Corner case: source has FolderA and FolderB with identical contents; target already has FolderA.

| Mode | Behaviour | When to use |
|------|-----------|-------------|
| **Strict copy** (default, `mapping_strict_copy = 1`) | Each source folder is mirrored to its corresponding target folder. FolderB gets backed up even if its files are identical to FolderA already in target. | User expects target to mirror source structure exactly. |
| **Dedup** (`mapping_strict_copy = 0`) | A file is skipped if a file with the same name+size exists **anywhere** in the target catalog, regardless of path. FolderB is silently skipped. | User wants space-efficient backup; duplicates are intentionally omitted. |

**Implementation** (`MainWindow::compareForBackup()`):
- **Strict copy**: direct SQL with path mapping — `targetFolder = targetRoot + SUBSTR(sourceFolderPath, sourceRootLen+1)`. Two queries: one for files-to-copy (no match at path), one for conflicts (match at path but different size).
- **Dedup**: `CatalogDifferenceEngine::compare()` by Name+Size across all target files, then classify by whether the name exists in target.

**Schema change**: `device_mapping.mapping_strict_copy INTEGER DEFAULT 1` (migration 2.10).

**UI**: Checkbox `BackUp_checkBox_StrictCopy` ("Strict copy", checked by default) in the mapping creation row.

### Conflict Resolution Modes — `mapping_conflict_mode` option

When a conflict is detected (file exists at the target path but with a different size), the executor can handle it in two ways:

| Mode | Value | Behaviour | Typical use case |
|------|-------|-----------|-----------------|
| **Skip** (default, `mapping_conflict_mode = 0`) | `ConflictMode::Skip` | Report the conflict, do nothing. Target file is preserved. | Safe default; user reviews conflicts manually. |
| **Keep Both** (`mapping_conflict_mode = 1`) | `ConflictMode::KeepBoth` | If source is newer than target (by filesystem date): rename the target file inserting a datetime stamp before its extension, then copy the source file to the original target path. If target is newer or same date: fall back to Skip. | Working documents that evolved on the source — old version is archived, new version is synced. |

**Archived filename format**: `stem_YYYYMMDD-HHmmss.ext`
- Example: `report.docx` → `report_20260225-102559.docx`
- Timestamp inserted before the extension so the file remains openable.
- Uses `QDateTime::currentDateTime()` at execution time (not catalog date).

**Direction check** (executor, at runtime):
- Compares `QFileInfo(sourceFile).lastModified()` vs `QFileInfo(targetFile).lastModified()`.
- Filesystem dates are used (more reliable than catalog dates for this check).
- Catalog data carries `DifferenceFileEntry::targetDateUpdated` for preview display.

**Failure recovery**: if the rename succeeds but the subsequent copy fails, the archived file is renamed back to the original path to avoid data loss.

**`DifferenceFileEntry` extension**: `targetDateUpdated` field added (populated by `compareStrict()` conflicts query, empty for filesToCopy). Allows preview to show direction before execution.

**`compareStrict()` conflicts query**: switched from `EXISTS` subquery to `JOIN` so the target file's `file_date_updated` can be selected alongside the source fields.

**`BackupReport` extension**: `renamed` list added (`QList<DifferenceFileEntry>`) for files that were archived+replaced. `renamedCount()` helper and `totalBytesCopied` includes bytes from renamed+replaced files.

**Schema change**: `device_mapping.mapping_conflict_mode INTEGER DEFAULT 0` (migration 2.11).

**UI**: Combobox `BackUp_comboBox_ConflictMode` ("On conflict: Skip / Archive & Replace") in the mapping creation row. Index maps directly to the `ConflictMode` enum (0 = Skip, 1 = KeepBoth). Placed between `BackUp_checkBox_StrictCopy` and `BackUp_pushButton_SaveMapping` in `BackUp_horizontalLayout_Save`.

## Phase 5: Post-Backup Target Catalog Update

**Goal**: After backup, update the target catalog to reflect newly copied files without full re-index.

**Approach**: Insert the copied files' metadata into the target catalog's `file` and `folder` tables, sourced from the source catalog data (already known).

## Phase 6 (optional): CatalogDuplicateEngine

**Goal**: Extract duplicate logic like Phase 1-2 did for differences.

**New files**:
- `src/core/catalogduplicateengine.h`
- `src/core/catalogduplicateengine.cpp`

**Then**: Refactor `searchjobstoppable.cpp::processDuplicates()` to use it.

## Phase 7 (optional): Keep rsync Export

**Goal**: Retain "Generate LuckyBackup Profile" as an advanced export option.

No code changes needed — already works. Just relabel in UI as "Export to rsync/LuckyBackup" to clarify it's an export, not the primary backup method.


