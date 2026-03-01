# Devices

## Summary
This page describes the concept of **Device** in Katalog and the top part of the **Devices** screen.

![](/img/devices_example1_cut.png)

In this example, several catalogs have been created from 2 physical drives.<br/>
The catalogs with photos/images/pictures have been assigned to a virtual *Photos* device.<br/>
This gives a way to search in these only, and it provides totals of number and size of photos files.<br/>

## Model

### Definitions

* A **[Catalog](DevicesCatalogs)** device is a list of files in a particular directory.

* A **[Storage](DevicesStorage)** device is a physical drive on which files are stored. Typically it is "mounted" or "connected" to the computer, and it has physical storage space.

* A **Virtual** device is any non-physical item used to group other devices together. It has no properties on its own and can aggregate numbers from the related sub-devices.

* A **Group** is a virtual device at the top of the hierarchy.

    * The **Physical group** is a unique and reserved group for the hierarchy of physical devices (computer, phone, disk, etc.).

    * Any other is a **Virutal group** to which existing catalogs can be "assigned" to faciliate searching and statistics.

### Hierarchy

![](/img/devices_model.png)


## Features

Always available at the top of the screen:

### Chosing between 3 views

The devices can be listed and managed in 3 ways:

**[Device Tree](DevicesTree)**: This view is showing the full and unfiltered list of devices in a hierarchy / tree structure.

**[Storage list](DevicesStorage)**: This view is showing only Storage devices and is filtered based on the [Selection](Selection) panel.

**[Catalog list](DevicesCatalogs)**: This view is showing only Catalog devices and is filtered based on the [Selection](Selection) panel.

### Display full table
Click this option to show all the data available in the view.

If unticked, this is typically hiding data that may not be needed on a day to day basis (ex: internal ID).

This may help keeping a simpler and more readable view.

### Record a snapshot of the data
This button trigger a record all devices values (size, files, space, etc.) independently of the current selection.

These records support the creation of [Statistics](Statistics), and particularly to keep track of the collection globally, and independently of the individual device updates.
