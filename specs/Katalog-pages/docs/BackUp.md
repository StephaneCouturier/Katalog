# BackUp
## Summary
This page describes all the features of the **BackUp** screen and how to use them.<br/>
From this screen, the user can **manage catalog backups**.<br/>

## Main concepts & current features

**BackUp Links**

The screen helps to <b>associate a Catalog</b>, considered as the <b><i>source</i></b>, to and another Catalog, considered as the backup <b><i>target</i></b>.<br/>This is the core enabler of all features of this screen.

**Catalogs' Comparison**

The BackUp Links section provides the list of links and <b>coverage of catalog backups</b> for the entire selection and per link, comparing source and backup target size, number of files and date updated.

**Replicate directory**

Necessary during the backup process, it is possible to trigger the copy of the folder hierarchy separately, without the files.

**Incremental backup**

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






**LuckyBackup profile**

It is possbile to export the BackUp Links into a [LuckyBackup](https://luckybackup.sourceforge.net) profile.
See dedicate page: [LuckyBackup profile](BackUp_luckybackup_profile) 


## BackUp Links management

To help listing and comparing source directories and their backup, Katalog can help mapping catalogs.

This assume that the user creates manually 

### Create a BackUp Link

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

## Development
Some ideas of developments for this screen:
* Catalog operations from this screen (update, search differences, etc.)
* Basic file copying
* For more, see the backlog of [BackUp development](https://github.com/users/StephaneCouturier/projects/7/views/1?filterQuery=BackUp).
