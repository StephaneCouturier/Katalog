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
<br/>There is no external tool dependency.

- Compares source and target catalogs to find files **missing from the target**.
- Copies missing files to the target, recreating folder structure.
- Does **not** overwrite existing files in the target (even if different).
- Does **not** delete files from the target that are absent from the source.

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
Select the entire line by clicking somewhere the row, and click the button "Delete".

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
