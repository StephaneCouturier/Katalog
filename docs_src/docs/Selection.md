---
version: "3.0"
---
# Selection
![3.0](https://img.shields.io/badge/Version-3.0-blue)

## Summary
This page describes the **Selection** page: the list of all devices of the collection, shown as a tree of cards. It is the first page of the window, and the other screens open next to it (see [Interface](Interface#pages)).

Selecting a device sets the **scope** of the other screens:
* [Search](Search): the search runs in the files of the selected device,
* [Devices](Devices): with *Filter from Selection*, only the selected device and its sub-devices are listed,
* [Create](Create): the selected storage is proposed for the new catalog,
* [Backup](BackUp): the backup links are filtered on the selected device,
* [Statistics](Statistics): the charts show the selected device.

![Selection page: selected device at the top, filter field, card tree with Photo Drive selected and device info shown](/img/selection_01_overview.png)

## Selected device {#selected-device}
The top of the page always shows the **selected device**, with its icon and name. When no device is selected, it shows *All*: the whole collection.

Clicking it scrolls the list to the selected card, which is useful in a long list. It does not change the selection.

## Device cards {#device-cards}
Each device of the collection is shown as a card, indented under its parent device.
* **Click** a card to select the device. The selected card is highlighted.
* **Right-click** a card (or press and hold on a touch screen) to open its [context menu](#context-menu).
* The arrow on the right of a card hides or shows its sub-devices.

The type of device is shown by its icon and its name style:

| Device | Icon | Name |
|--------|------|------|
| **Virtual** device or group | several drives | bold italic, dimmed |
| **Storage** device | hard drive | bold, slightly dimmed |
| **Catalog** | disc (one disc icon when the catalog is available, another when it is not) | normal |

See [Devices](Devices) for what Virtual, Storage and Catalog devices are.

When the device info is shown, a second line under the name gives:
* for a **Catalog**: its number of files and their total size,
* for a **Storage** or **Virtual** device: the number of files and total size of its catalogs, then the used space.

## Filter and tree buttons {#filter}

![Device filter with "photo" typed: only the matching devices are listed](/img/selection_02_filter.png)

Above the cards:

| Control | Use |
|---------|-----|
| *Name...* field | Filters the cards **by device name**: only the devices whose name contains the typed text are listed, whatever the case. It filters devices, not files. The cross button clears it. `Ctrl+F` puts the cursor in this field. |
| *Show device info* | Shows or hides the second line of the cards. |
| *Collapse one level* | Hides the deepest level of devices currently shown. |
| *Expand one level* | Shows one more level of devices. |

## Context menu {#context-menu}

![Context menu of a Catalog card](/img/selection_03_context_menu.png)

The context menu starts with the name of the device, followed by:

| Item | Shown for | Action |
|------|-----------|--------|
| *Search* | all devices | Selects the device and opens the [Search](Search) page. |
| *Update* | Virtual and Storage devices, and available Catalogs | Updates the device: scans the files of its catalogs again. Requested while another operation is running, it is added to the [queue](Interface#queue). |
| *Explore* | Catalogs | Selects the catalog and opens it in the [Explore](Explore) page. |
| *Open folder* | all devices | Opens the folder of the device in the file manager. |
| *Edit* | all devices | Opens the device edit page (see [Devices](Devices)). |

*Search* and *Explore* select the device first, so the highlighted card always matches what is searched or explored. *Update*, *Open folder* and *Edit* act on the right-clicked device **without changing the selection**.

## Search in a connected drive
Searching directly in a folder of a connected drive, without a catalog, is chosen on the [Search](Search) page itself (*Connected drive*), not in the Selection.

## Development
* See the backlog of [Selection development](https://github.com/users/StephaneCouturier/projects/7/views/1?filterQuery=selection).
