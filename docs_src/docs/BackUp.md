---
version: "2.12"
---
# BackUp
![2.12](https://img.shields.io/badge/Version-2.12-blue)

## Summary
This page describes all the features of the **BackUp** screen and how to use them.<br/>
From this screen, the user can **manage catalog BackUp (copy) or Archive (move)**.<br/>

![](/img/screen_backup_01.png)

## Main concepts & current features

### Catalog Links

The screen helps to <b>associate a Catalog</b>, considered as the <b><i>source</i></b>, to another Catalog, considered as the backup <b><i>target</i></b>.<br/>This is the core enabler of all features of this screen.

### Catalogs' Comparison

The screen provides the list of links and <b>coverage of catalog backups</b> for the entire selection and per link, comparing source and backup target size, number of files and date updated.

### Replicate directory

Necessary during the backup process, it is possible to trigger the copy of the folder hierarchy separately, without the files.

## BackUp or Archive

| operation | file process  |  Purpose                                                         |
|-----------|---------------|------------------------------------------------------------------|
| **Backup**    | A **copy** operation from a source catalog to a target catalog. | The source is never modified. The goal is redundancy and recovery. <br/> Multiple strategies will exist over time (full, incremental, sync). |
| **Archive**   | A **move** operation from a source catalog to a target catalog. | Files are transferred and then removed from the source once the copy is verified. The goal is long-term offload and storage organisation. Empty directories left behind in the source are **not** deleted. |



### Incremental backup (file copy)

Katalog's backup will copy files from a **source catalog** to a **target catalog** using Katalog's own indexed data.
<br/>For that to happen fully, updates of the Catalogs will be run before the backup and after (optional but highly recommanded).
<br/>There is no external tool dependency.
<br/><br/>Sequence of actions:
- Compare source and target catalogs to find **files missing from the target**.
- Copies missing files to the target, recreating folder structure.
- Does **not** overwrite existing files in the target (even if different).
- Does **not** delete files from the target that are absent from the source.

Options "Strict copy"
- <i>Strict copy</i> (default): Katalog will copy files even if they are present once already in the target. If unticked, there will likely be no duplicates on Name, Size, Date in the target.

Options of Conflict Resolution Modes
- <i>On Conflict</i> (default is <i>Rename oldest</i>)

Katalog can address different conflicts in different manners, when the file exists on the target but the date, size, and checksum are different.


A **conflict** occurs when a file exists at both the source and the target path, but the date, size, or checksum differ. The mode controls what the executor does in that case.

#### Available modes {#available-modes}

| Mode | Behaviour |
|------|-----------|
| **Skip** (default) | No file operation — source is not copied, target is not modified. Conflict is reported for user review. |
| **Rename oldest** | If the source is newer: rename the older target file (adding a timestamp suffix), then copy the source. If the target is newer or same date: skip (protect the newer target). |

#### Full scenario space

| # | Situation | Skip | Rename oldest |
|---|-----------|------|---------------|
| A | Source newer than target | conflict reported | rename target → copy source ✓ |
| B | Target newer than source | conflict reported | skip (protect newer target) |
| C | Same date, different size | conflict reported | skip (no clear winner) |
| D | Source file missing on disk | error | error |

> **Rename oldest — archived filename format**: the old target file is renamed to `originalname_YYYYMMDD-HHmmss.ext` (e.g. `report_20260225-102559.docx`). The timestamp is inserted before the extension so the file remains openable. These files accumulate on the target and must be cleaned up manually to reclaim space.

> **Rename oldest — safety guarantee**: if the copy of the source file fails after the target has already been renamed, the renamed file is automatically restored to its original name. No data is lost.


### Archive (file move)

The Archive operation **moves** files from source to target instead of copying them. Source files are deleted after a confirmed successful transfer — if the transfer fails, the source file is left untouched.

- On the **same filesystem**: the move is instant — no data is physically copied; only the file location changes.
- **Across filesystems**: the file is first copied to the target, then deleted from the source once the copy is confirmed complete.

### Source Mode {#source-mode}

Each backup link has a **Source Mode** that controls what is used as the source during comparison and file copy.

| Mode | DB value | Description |
|------|----------|-------------|
| **Catalog** (default) | `'Catalog'` | Uses the catalog index (`.idx` file). Works offline — the source device does not need to be connected. Catalog exclude-folder rules are applied: excluded folders are not backed up. |
| **Drive** | `'Drive'` | Walks the source filesystem directly. The source device **must be connected and mounted**. All files under the source path are included — catalog index and exclude-folder rules are entirely bypassed. |

**Architecture decision (Option B):** Drive mode replaces the catalog as the file enumeration source entirely — it does not "supplement" the catalog with excluded folders. The same filesystem-walk logic used by `CatalogJobStoppable` is reused. Catalog mode remains the default because it is offline-capable; Drive mode is for users who want a guaranteed full backup regardless of catalog state.

**UI:** the "Scan source drive directly" checkbox on the Create Link panel controls this field. Unchecked = Catalog (default), checked = Drive.

> **Implementation status:** Drive mode is implemented for the Strict Copy path (`compareStrictFromDrive()` in `CatalogDifferenceEngine`). The Dedup path (`strictCopy=false`) currently falls back to the catalog index when Drive mode is selected — full Drive+Dedup support is a future backlog item.

### LuckyBackup profile

It is possbile to export the BackUp Links into a [LuckyBackup](https://luckybackup.sourceforge.net) profile.
See dedicate page: [LuckyBackup profile](BackUp_luckybackup_profile) 


## BackUp Links management

To help listing and comparing source directories and their backup, Katalog can help linking catalogs.

This assume that the user creates manually 

### Create a BackUp Link

The *Create Link* panel can be collapsed or expanded using the toggle button at the top of the panel.

#### Link fields

| Field | Description |
|-------|-------------|
| Name |  Label for the link. Can be auto-generated as `"<source name> -> <target name>"`. |
| Type |  `BackUp` (copy) or `Archive` (move). Controls whether source files are deleted after transfer. |
| Source device  | The source catalog device. Files are read from its indexed path. |
| Target device  | The backup destination catalog device. Files are written to its path. |
| Last backup date  | Date of the last completed backup run. Updated automatically. |
| Last backup size  | Total bytes transferred in the last backup run. Updated automatically. |
| Strict copy | If enabled (default), copies files by path — even if the file already exists elsewhere on the target. If disabled, skips files already present anywhere on the target (de-duplication mode). Not applicable for *Archive* links (automatically disabled). |
| On conflict | What to do when a file exists at the same path on both source and target but differs. Default: `RenameOldest`. See [Conflict Resolution Modes](#available-modes). |
| Source mode | `Catalog` (default) or `Drive`. Controls whether the source is read from the catalog index or by walking the filesystem directly. See [Source Mode](#source-mode). |

#### Example & Catalogs
Goal: create a link between the source on local disk and the target on external drive.
![](/img/screen_backup_1_devices.png)

#### Select source & target
- With the Selection panel and the 2 "Load Catalogs" buttons for the source and target, get the list of catalogs for selection.
- The button "without links" can help limit the nmber of catalogs in the list, by displaying only catalogs that do not have a Link as source (or as target) already.
- Select a source and a target

![](/img/screen_backup_2_select_source_target.png)


#### Set the Name, options, and create
- Generate a name: source catalog name + " -> " + target katalog name
- Define the "Strict copy" option
- Define behavior on Conflict detection
- Create link

![](/img/screen_backup_4_create_mapping.png)

#### Compare source and backup target
- The Link appears in the list and coverage is calculated
![](/img/screen_backup_5_comparison.png)

### Links list filters

The links list can be filtered to show only relevant links:
- **Source / Target** radio buttons — show only links where the currently selected device acts as the source or as the target.
- **Type** dropdown — filter by *BackUp* or *Archive*.
- **Display full table** checkbox — toggles additional detail columns in the list.

### Context menu on a link

Right-clicking a link in the list opens a context menu with the following actions:

| Action | Description |
|--------|-------------|
| *Run Backup* / *Run Archive* | Start the backup or archive operation for this link. |
| *Preview Backup* / *Preview Archive* | Run a preview (simulation) without copying any files. |
| *Replicate directories* | Copy the folder structure only, without the files. |
| *Invert (swap source and target)* | Swap the source and target of the link in one click — useful to reverse a backup direction. |
| *Delete* | Remove the link (does not affect the files on disk). |

**LuckyBackUp profile creation**
Katalog can generate a ready-to-use LuckyBackUp profile based on the BackUp links:
- Saves to `~/.luckyBackup/profiles/` directory
- Each backup link becomes one task in the profile
- By default, **all** links are included
- Check *Only selected links* to include only the links currently visible in the filtered list (filtered by Source/Target device and/or Type)

## BackUp or Archive execution

### Prerequisites
- A BackUp/Archive Link is selected
- Both catalogs must belong to devices with valid, accessible paths.
- There is enough space to copy/move the files to the target
- while optional, it is recommanded to keep "Update catalogs" selected for update prior and after the process completion.

### Preview
- A Preview (simulation) can be run to test the effect of the BackUp or Archive process and generate a report.
- The preview result can be **exported** using the *Export* button, saving the list of planned operations to a file for review.

### Pause, Resume, and Cancel

During execution the **Run Backup** button changes label and function:
- While **running** → click to **Pause** (suspends after the current file finishes)
- While **paused** → click to **Resume**

A **Cancel** button is always available while the backup is running or paused. Cancelling stops the operation cleanly — any file currently being copied is removed from the target (no partial files left).

### Catalog update after execution

After a successful backup, the target catalog will be updated automatically to reflect the newly copied files, without requiring a full re-index. *(Planned — not yet available.)*

## Core Behavior Incremental Copy or Archive

Comparison criteria
- Match by **file name + relative folder path** (same file in same relative location).
- A file is "missing" if no match exists in the target catalog.

**On conflict management**
| Decision | Choice | Rationale |
|----------|--------|-----------|
| Delete files in target? | No (v1). Future backlog item. | Start safe — incremental only. |
| Overwrite conflicts? | No (v1). Report them. | Avoid data loss. Future: user choice per-file. |
| Create missing directories? | Yes, always. | Required for any file copy to work. |

### Disk Space Management

Disk space is a critical constraint for both Backup and Archive operations. Running out of space
mid-operation leaves the target in a partial state and produces a series of I/O errors. Space
must be estimated and checked before execution begins.

Space Requirements by Operation

| Operation | What consumes target space | Net effect on source |
|-----------|---------------------------|----------------------|
| **Backup** | All files-to-copy | None |
| **Archive (same FS)** | Zero — `rename()` moves metadata only | Space freed on source |
| **Archive (cross-FS)** | Files copied before source deletion | Space freed on source after delete |
| **RenameOldest conflict** | Old target file renamed and retained (+1 copy) | None until user manually purges archived files |


### Space Check Logic (implemented)

Computed before execution and shown in Preview using `QStorageInfo(targetDevice.path).bytesAvailable()`.

**Required bytes** = sum of all `filesToCopy` sizes + (if RenameOldest) sum of `fileConflicts` sizes.

| Condition | Threshold | Action |
|-----------|-----------|--------|
| **Insufficient** | `available < required` | Block: `QMessageBox::warning`, operation not started |
| **Low** | `available − required < 512 MB` | Ask: `QMessageBox::question` (Yes/No to proceed) |
| **OK** | `available − required ≥ 512 MB` | Proceed silently |

In Preview, the space status is appended to the summary label:
- **Insufficient** → red warning: `⚠ Target space: X available, Y needed`
- **Low** → orange warning: `⚠ Low target space: Z remaining after operation`
- **OK** → no annotation (clean display)

When `QStorageInfo` cannot determine available space (remote mount, virtual FS), the check
is silently skipped.



> **Note**: The RenameOldest mode permanently adds renamed copies to the target. Users can
> manually clean archived files (`stem_YYYYMMDD-HHmmss.ext`) to reclaim space if required.

## Report
After execution, the preview table is replaced by a **backup report** — a table with four columns: **Status**, **File name**, **Path**, and **Size**.

Each row corresponds to one file, with the following status values:

| Status | Meaning |
|--------|---------|
| *Copied* | File was successfully copied to the target. |
| *Moved* | File was successfully moved to the target (Archive operation). |
| *Archived & Copied* | A conflicting target file was renamed (RenameOldest mode), then the source was copied. |
| *Conflict* | File exists at the same path on both source and target but differs — not overwritten, listed for review. |
| *Error* | File could not be copied or moved (permission denied, disk full, etc.). |

A summary line above the table shows totals: files copied, archived & copied, conflicts, and errors.

---

## Development
Some ideas of developments for this screen:

### Future Features
- [ ] Snapshot management
- [ ] Exclude/include patterns
- [ ] Scheduling (cron/systemd/Task Scheduler)
- [ ] Restore functionality
- [ ] Compression options
- [ ] Remote backups (ssh)

### Future conflict modes

| Mode | Behaviour |
|------|-----------|
| **Overwrite** | Source always wins — overwrites the target silently. For users who want the source to be authoritative regardless of date. |
| **Rename always** | Always renames the target and copies the source, even when the target is newer — aggressive, explicit archiving. |

### Future Options

- **Archive source cleanup**: opt-in option to delete empty directories left behind in the source after an Archive (move) operation.
- **Delete mode**: opt-in option to remove target files absent from source.
- **Checksum comparison**: detect content changes even when name/size/date match.
- **Scheduled/automated backup**: run on timer or on catalog update.
- **Backup history**: log of past backup runs with dates and statistics.

### Future Features for Disk Space
- **Per-link minimum free space setting**: user-configurable floor (e.g., always keep 5 GB free).
- **In-flight space exhaustion early abort**: detect ENOSPC errors during copy and stop immediately rather than continuing to fail file after file.
- **Post-archive source verification**: confirm actual source space was freed after Archive completes.
- **Space trend display**: show target space over time in the Statistics tab.
- **Archived file cleanup tool**: list and bulk-delete `stem_YYYYMMDD-HHmmss.ext` files produced by RenameOldest mode.

* For more, see the backlog of [BackUp development](https://github.com/users/StephaneCouturier/projects/7/views/1?filterQuery=BackUp).
