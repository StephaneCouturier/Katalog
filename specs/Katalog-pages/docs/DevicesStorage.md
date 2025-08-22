# Devices: Storage

## Summary
This page describes all the features of the **Storage list** view of the [Devices](Devices) screen and how to use them.
* A **Storage** device is an actual physical drive, disk or other type of memory storing data as files, and from which you can create one of more catalogs.
* The data associated with this type of device are a combination of 3 types of information:
    * *physical storage* : **free space**, **use space**, **total space**. **label**, **file system**.
    * *calculated*: the **total number of files** and **total file size** will be automatically populated if the storage has catalogs.
    * *user custom*: **path**, **type**, **brand**, **model**, **serial number**, **build date**, **comment 1**, **comment 2**, **comment 3**.
* Using them can help you search on multiple catalogs associated with this particular device.
* They will also help you see more [statistics](Statistics) about all your devices and what they store.
* Storage can only be part of the *Physical Group*, and can be put under any *virtual device* in that group, which can be useful for Search or Statistics features.
![](/img/devices_storage_01.png)
## List and selection
The list of catalogs can be limited by using the [Selection](Selection) left panel.

## Actions buttons
* **Update**: (enabled when a catalog is selected) Update the selected catalog by listing all files again from its source path, according to its criteria.
* **All active**:   Update all the displayed catalogs that are active (the path is reachable).

## Context menu (right click)
A right click on any of the catalogs listed opens a context menu for action on this active catalog.
![](/img/devices_storage_02_context.png)
        | Menu entry  | Related Action                                    |
        | ------------| --------------------------------------------------|
        | **Update**  | Update the selected storage: this will update the storage itself and the update of all catalogs below. |
        | **[Edit](#edition)**  | Open a panel to change name, path, etc. |
        | **Filelight** | Open [Filelight](https://apps.kde.org/filelight/) in the path of the storage device. |
        | **Delete** | Delete the storage device. It is only possible if no catalog is associated with it. This does not delete the related values in the statistics. |

## Edition
The panel gives access to modify all storage device fields, except the device ID itself.
![](/img/devices_storage_03_edit.png)

## Device picture
This feature is currently only available for the [Memory date mode](Settings#database-memory-mode) as pictures are stored with the collection folder.

It is possible to associate a picture to a Storage device.<br/>
A folder *images* must be created in the collection folder, and the image named with the storage ID (not the device ID, but the storage ID).<br/>
Example: /home/user/Documents/KatalogCollectionFolder/images/3.jpg


![](/img/devices_storage_04_picture.png)
