# BackUp
## Summary
This page describes all the features of the **BackUp** screen and how to use them.<br/>
From this screen, the user can **manage catalog backups**.<br/>

<b>Mapping feature</b>: for now, the screen helps to associate a Catalog, considered as the source, to and another Catalog, considered the backup target.
This help checking the coverage of backup for devices, and compare source and backup target size, number of files and date updated.

<b>Katalog does not have functions to automatically copy files yet.</b><br/>
Many simple to advanced applications are available, such as [KBackUp](https://apps.kde.org/fr/kbackup) or [LuckyBackup](https://luckybackup.sourceforge.net)

## Catalogs mapping and comparison

To help listing and comparing source directories and their backup, Katalog can help mapping catalogs.

This assume that the user creates manually 

### Create a Mapping

#### Example & Catalogs
![](/img/screen_backup_1_devices.png)

#### Select source
![](/img/screen_backup_2_select_source.png)

#### Select target
![](/img/screen_backup_3_select_target.png)

#### Name and create
![](/img/screen_backup_4_create_mapping.png)

#### Compare source and backup target
![](/img/screen_backup_5_comparison.png)

### Delete a Mapping
Select the entire line by clicking on the row number (left of the table),<br/>
And click the button "Delete selected".

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
