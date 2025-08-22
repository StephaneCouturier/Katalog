# Devices: Virtual device tree

## Summary
This page describes all the features of the **Device Tree** view of the [Devices](Devices) screen and how to use them.
* A **Virtual** storage device is any non-physical drive or group used to link catalogs from different physical devices.
* Using them can help  to perform [Search](Search) or get [Statistics](Statistics) independently of the physical devices.
![](/img/devices_tree_01.png)
## Top bar options

### Display options
* bouton ![](/img/device_tree_button_apply.png): Apply options to the Selection device tree. This enable to create a simpler and limited device tree in the [Selection](Selection) panel.
* **Physical Group**: Display the *Physical Group* and its related Devices.
* **Virtual Groups**: Display the *Virtual Groups* and their related Devices.
* **Storage**: Display the *Storage* devices (if unticked, the Catalogs would be hidden too).
* **Catalogs**: Display the *Catalog* devices.

### Actions buttons
* **Insert Virtual Group**: Create and insert a new Virtual Group device at the top of the hierarchy, and open the Edit panel.
* **Add Virtual**: Create and insert a new Virtual device under the selected device in the hierarchy, and open the Edit panel.

## Context menu (right click)
### Virtual devices creation
| Case  | Menu entry                                   |Result                                    |
| ------------| --------------------------------------------------|--------------------------------------------------|
| Physical group / Virtual device  | Add Virtual device |A Virtual device is created under the selected device|
| Physical group / Virtual device  | Add Storage device |A Storage device is created under the selected device |
### Catalog assignment in Virtual groups
| Case  | Menu entry                                   |Result                                    |
| ------------| --------------------------------------------------|--------------------------------------------------|
| Virtual group / Virtual device   | Assign selected Catalog | If a catalog is selected in the [Selection](Selection) panel, it is assigned to the selected device |
| Virtual group / Catalog device   | Unassign this catalog | The selected catalog is removed from the selected virtual device (the catalog itself is not deleted) |
### Virtual storage tree and items creation
* Each item can be renamed using the button *Edit*
* Each item can be deleted, given that it does not have sub-items nor cataloged assigned.
