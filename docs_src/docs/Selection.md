---
version: "2.11"
---
# Selection
![2.11](https://img.shields.io/badge/Version-2.11-blue)

## Summary
This page describes all the features of the **Selection** panel, the left part of the user interface.
* This panel is used to refine a selection for the various screens and features.
* With the mode *Search in file catalogs*, the selection will filter information for the [Search](Search), [Devices](Devices), [Create](Create), and [Statistics](Statistics) screens.
* With the mode *Search in connected drives*, a directory directly from connected devices can be selected. This is only used for the [Search](Search) screen.

![Selection panel showing device tree and selection controls](/img/selection_01.png)

## Interface
Top buttons:
* *Show/Hide*: The top left button can be used to Hide and Show again the panel.
* *Reset*: this button with the broom icon resets the current selection, so that all catalogs/data are selected.
* *Reload*: this reloads the entire collection data, helping for instance to update the application for changes on data done outside of it.

## Search in Catalogs
Selection information:
This section shows the current selection of Virtual, Storage, or Catalog device.

The two buttons next to the device tree label collapse or expand the tree by one level.

### Context menu (right-click)

The context menu varies depending on the type of device selected.

**For Storage and Virtual devices:**
* *Search*: selects the item and goes to the Search screen (shown only when not already on the Search screen)
* *Update*: triggers the update (scan of files) of the selected device and all its catalogs

**For Catalog devices:**
* *Search*: selects the item and goes to the Search screen (shown only when not already on the Search screen)
* *Update*: triggers the update (scan of files) of the selected catalog (available only when the catalog is active)
* *Explore*: opens the selected catalog in the [Explore](Explore) screen
* *Open folder*: opens the catalog's source folder in the system's file manager (shown only when a source path is defined)

## Search in Connected drives
With this option, a search can be done directly in any directory of a connected/mounted drive, without the need of a Catalog.

Use the *Pick path* button or the directory tree to select the folder to search in.

![Selection panel in connected drives mode showing the directory tree and path selection](/img/selection_02.png)
