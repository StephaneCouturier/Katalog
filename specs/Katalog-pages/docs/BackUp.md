# BackUp
## Summary
This page describes all the features of the **BackUp** screen and how to use them.<br/>
From this screen, the user can **manage catalog backups**.<br/>

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

#### Available modes

| Mode | DB value | Behaviour |
|------|----------|-----------|
| **Skip** (default) | `"Skip"` | No file operation — source is not copied, target is not modified. Conflict is reported for user review. |
| **Rename oldest** | `"RenameOldest"` | If the source is newer: rename the older target file (adding a timestamp suffix), then copy the source. If the target is newer or same date: skip (protect the newer target). |
| **Overwrite** *(backlog)* | `"Overwrite"` | Source always wins — overwrite the target silently, no rename backup. For users who want the source to be authoritative regardless of date. |
| **Rename always** *(backlog)* | `"RenameAlways"` | Always rename the target and copy the source, even when the target is newer — aggressive, explicit archiving. |

#### Full scenario space

| # | Situation | Skip | Rename oldest | Overwrite *(backlog)* | Rename always *(backlog)* |
|---|-----------|------|---------------|-----------------------|---------------------------|
| A | Source newer than target | conflict reported | rename target → copy source ✓ | overwrite target | rename target → copy source |
| B | Target newer than source | conflict reported | skip (protect newer target) | overwrite target | rename target → copy source |
| C | Same date, different size | conflict reported | skip (no clear winner) | overwrite target | rename target → copy source |
| D | Source file missing on disk | error | error | error | error |

Cases B and C are currently left as reported conflicts. `Overwrite` and `RenameAlways` are the natural future modes to cover them when the user wants the source to be unconditionally authoritative.

> **Rename oldest — archived filename format**: the old target file is renamed to `originalname_YYYYMMDD-HHmmss.ext` (e.g. `report_20260225-102559.docx`). The timestamp is inserted before the extension so the file remains openable. These files accumulate on the target and must be cleaned up manually to reclaim space.

> **Rename oldest — safety guarantee**: if the copy of the source file fails after the target has already been renamed, the renamed file is automatically restored to its original name. No data is lost.


### Archive (file move)

The Archive operation **moves** files from source to target instead of copying them. Source files are deleted after a confirmed successful transfer — if the transfer fails, the source file is left untouched.

- On the **same filesystem**: the move is instant — no data is physically copied; only the file location changes.
- **Across filesystems**: the file is first copied to the target, then deleted from the source once the copy is confirmed complete.

### Source Mode

Each backup link has a **Source Mode** that controls what is used as the source during comparison and file copy.

| Mode | DB value | Description |
|------|----------|-------------|
| **Catalog** (default) | `'Catalog'` | Uses the catalog index (`.idx` file). Works offline — the source device does not need to be connected. Catalog exclude-folder rules are applied: excluded folders are not backed up. |
| **Drive** | `'Drive'` | Walks the source filesystem directly. The source device **must be connected and mounted**. All files under the source path are included — catalog index and exclude-folder rules are entirely bypassed. |

The mode is stored in `device_mapping.mapping_source_mode` (TEXT, default `'Catalog'`).

**Architecture decision (Option B):** Drive mode replaces the catalog as the file enumeration source entirely — it does not "supplement" the catalog with excluded folders. The same filesystem-walk logic used by `CatalogJobStoppable` is reused. Catalog mode remains the default because it is offline-capable; Drive mode is for users who want a guaranteed full backup regardless of catalog state.

**UI:** the "Scan source drive directly" checkbox on the Create Link panel controls this field. Unchecked = Catalog (default), checked = Drive.

> **Implementation status:** Drive mode is implemented for the Strict Copy path (`compareStrictFromDrive()` in `CatalogDifferenceEngine`). The Dedup path (`strictCopy=false`) currently falls back to the catalog index when Drive mode is selected — full Drive+Dedup support is a future backlog item.

### LuckyBackup profile

It is possbile to export the BackUp Links into a [LuckyBackup](https://luckybackup.sourceforge.net) profile.
See dedicate page: [LuckyBackup profile](BackUp_luckybackup_profile) 


## BackUp Links management

To help listing and comparing source directories and their backup, Katalog can help mapping catalogs.

This assume that the user creates manually 

### Create a BackUp Link

#### Link fields

| Field | Description | 
|-------|-------------|
| name | Name of the link, can be generated from the 2 catalog names |
| type | `BackUp` or `Archive` |
| device_source | `"RenameAlways"` |
| device_target_id | mapping_device_source_id |
| backup_last_date | mapping_device_source_id |
| backup_last_size | mapping_device_source_id |
| strict_copy | mapping_device_source_id |
| conflict_mode | RenameOldest |

#### Example & Catalogs
Goal: create a mapping between the source on local disk and the target on external drive.
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

### Delete a Mapping
Right click on the link line to display the context menu, and select "Delete".

![](/img/screen_backup_6_delete.png)

**LuckyBackUp profile creation**
Katalog can generate LuckyBackUp a ready-to-use profile based on the BackUp links
- creates `.profile` files from backup links
- Saves to `~/.luckyBackup/profiles/` directory
- Option to generate from ALL backup links or only filtered ones (by source/target device)
- Profile naming: `Katalog_<timestamp>.profile`
- Each Katalog backup link becomes one task in the LuckyBackUp profile

## BackUp or Archive execution

### Prerequisites
- A BackUp/Archive Link is selected
- Both catalogs must belong to devices with valid, accessible paths.
- There is enough space to copy/move the files to the target
- while optional, it is recommanded to keep "Update catalogs" selected for update prior and after the process completion.

### Preview
- A Preview (simulation) can be run to test the effect of the BackUp or Archive process and generate a report.

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
After execution, a **backup report** lists:
- Files copied (count, total size)
- Files skipped — already exist in target (count)
- Files with conflicts — exist in target but differ (newer date, different size, different checksum). Listed for user review, **not overwritten**.
- Errors — files that failed to copy (permission denied, disk full, etc.)



## Development


### Future Features
- [ ] Snapshot management
- [ ] Exclude/include patterns
- [ ] Scheduling (cron/systemd/Task Scheduler)
- [ ] Restore functionality
- [ ] Compression options
- [ ] Remote backups (ssh)

### Future Backlog — Various
Some ideas of developments for this screen:
- **Archive source cleanup**: opt-in option to delete empty directories left behind in the source after an Archive (move) operation.
- **Delete mode**: opt-in option to remove target files absent from source.
- **Overwrite mode**: options per conflict (skip, overwrite, keep both, ask).
- **Checksum comparison**: detect content changes even when name/size/date match.
- **Scheduled/automated backup**: run on timer or on catalog update.
- **Backup history**: log of past backup runs with dates and statistics.

### Future Backlog — Disk Space

- **Per-mapping minimum free space setting**: user-configurable floor (e.g., always keep 5 GB free).
- **Source space check for Archive**: warn when source free space after archive will be very low.
- **In-flight space exhaustion early abort**: detect ENOSPC errors during copy and stop immediately rather than continuing to fail file after file.
- **Post-archive source verification**: confirm actual source space was freed after Archive completes.
- **Space trend display**: show target space over time in the Statistics tab.
- **Archived file cleanup tool**: list and bulk-delete `stem_YYYYMMDD-HHmmss.ext` files produced by RenameOldest mode.

* For more, see the backlog of [BackUp development](https://github.com/users/StephaneCouturier/projects/7/views/1?filterQuery=BackUp).
