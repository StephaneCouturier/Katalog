---
version: "2.11"
---
# Devices
![2.11](https://img.shields.io/badge/Version-2.11-blue)

## Summary
This page describes the concept of **Device** in Katalog and the top part of the **Devices** screen.

![Example showing catalogs organized under physical drives and a virtual Photos group](/img/devices_example1_cut.png)

In this example, several catalogs have been created from 2 physical drives.<br/>
The catalogs with photos/images/pictures have been assigned to a virtual *Photos* device.<br/>
This gives a way to search in these only, and provides totals of number and size of photo files.

## Model

### Definitions

* A **[Catalog](DevicesCatalogs)** device is an index of files from a particular directory.

* A **[Storage](DevicesStorage)** device is a physical drive on which files are stored. Typically it is mounted or connected to the computer, and it has physical storage space.

* A **Virtual** device is any non-physical item used to group other devices together. It has no properties on its own and can aggregate numbers from the related sub-devices.

* A **Group** is a virtual device at the top of the hierarchy.

    * The **Physical group** is a unique and reserved group for the hierarchy of physical devices (computer, phone, disk, etc.).

    * Any other is a **Virtual group** to which existing catalogs can be assigned to facilitate searching and statistics.

### Hierarchy

![Diagram showing the device hierarchy with groups, storage devices, and catalogs](/img/devices_model.png)

## Features

Always available at the top of the screen:

### Choosing between 3 views

The devices can be listed and managed in 3 ways:

**[Device Tree](DevicesTree)**: Shows the full and unfiltered list of all devices in a hierarchy / tree structure.

**[Storage list](DevicesStorage)**: Shows only Storage devices, filtered by the [Selection](Selection) panel.

**[Catalog list](DevicesCatalogs)**: Shows only Catalog devices, filtered by the [Selection](Selection) panel.

### Display full table
When enabled, all available columns are shown in the current view.

When unticked, columns that are not needed on a day-to-day basis (such as internal IDs) are hidden, keeping the view simpler and more readable.

### Update active device
Updates the currently selected device — re-scans its files from the source path.

This button is enabled when a device with a reachable source path (shown with a colored icon) is selected in any of the three views.

### Record a snapshot
Records the current values of all devices (file count, file size, free space, total space) independently of the current selection or filters.

After recording, a summary is shown with the new totals and the change since the previous snapshot (delta).

These records support [Statistics](Statistics) and allow tracking the collection globally over time.
