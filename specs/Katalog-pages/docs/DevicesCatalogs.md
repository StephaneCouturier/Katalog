# Devices: Catalogs

## Summary
This page describes all the features of the **Catalogs list** view of the [Devices](Devices) screen and how to use them.
* A *Catalog* is an index of files from a given directory referred as catalog **path**.
* The **Catalogs list** displays all catalogs of the [Collection](Settings#collection), filtered according to the [Selection](Selection).

![](/img/devices_catalogs_01.png)

## List and selection
The list of catalogs can be limited by using the [Selection](Selection) left panel.
If the source path of a catalog points to a path that is active/connected/mounted, the catalog icon is colored in blue.

## Actions buttons
* **Update**: (enabled when a catalog is selected) Update the selected catalog by listing all files again from its source path, according to its criteria.
* **All active**:   Update all the displayed catalogs that are active (the path is reachable).
* **[Import](#import)**:   Imports a catalog from another tool. Currently, the import from a VVV export (csv, tab separated) is supported.

## Context menu (right click)

A right click on any of the catalogs listed opens a context menu for action on this active catalog.

![](/img/devices_catalogs_02_context2.png)
        | Menu entry  | Related Action                                    |
        | ------------| --------------------------------------------------|
        | **Update**  | Update the selected catalog by listing all files again from its source path, according to its criteria (type of file, hidden files, etc.) |
        | **Explore** | Open the selected catalog file in the [Explore](Explore) screen to view the catalog's folders and files. |
        | **[Edit](#edition)**  | Open a panel to change name, path, storage device, etc. See details below before modifying any field to understand the consequences. |
        | **Filelight** | Open [Filelight](https://apps.kde.org/filelight/) in the path of the catalog. |
        | **Delete** | Delete the catalog. (This does not delete the backup file, nor the related values in the statistics) |

## Edition
It is generally recommended to create a new catalog with the right initial settings:<br/>
Otherwise, the panel gives access to modify the following fields:
* **Device Name**
* **Parent name (ID)** (different Storage device).
* **Source Path**
* **File Type**
* **Include Hidden Files**
![](/img/devices_catalogs_03_edit.png)
## Import
It is now possible to import all VVV physical volumes from one export file made using tabulation as separator.<br/>
Each VVV physical volume will become 1 separate catalog. <br/>
Limit: the source path and other info about the volume are not available in the exportfrom VVV.
- In VVV, choose File / Export...,  and select TAB as Separator
- From Katalog, screen Catalogs: click the Import button and select the file created previously.
- Using the Edit button, it is possible to add the catalog source path to enable updates later.
