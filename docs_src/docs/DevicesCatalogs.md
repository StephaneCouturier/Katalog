---
version: "2.10"
---
# Devices: Catalogs
![2.10](https://img.shields.io/badge/Version-2.10-blue)

## Summary
This page describes all the features of the **Catalogs list** view of the [Devices](Devices) screen and how to use them.

A *Catalog* is an index of files from a given directory referred to as the catalog **path**.<br/>
The **Catalogs list** displays all catalogs of the [Collection](Settings#collection), filtered according to the [Selection](Selection).

![Catalog list showing catalog names, paths, file counts and storage devices](/img/devices_catalogs_01.png)

## List and selection
The list of catalogs can be narrowed down using the **[Selection](Selection)** left panel.

When the source path of a catalog points to a connected and mounted location, the catalog icon is shown in color (blue), indicating the catalog is **active**.

## Action buttons

| Button | Enabled when | Description |
|--------|-------------|-------------|
| *Update* | A catalog is selected | Re-scans the selected catalog from its source path, according to its criteria (file type, hidden files, etc.) |
| *All active* | Always (Catalog list view only) | Updates all displayed catalogs whose source path is reachable |
| *Stop* | An update is running | Cancels the running update operation |
| *Verify MIME Types* | An active catalog is selected | Re-checks the file types of all files in the catalog using the system MIME database |
| *Import* | Always | Imports catalogs from a VVV export file — see [Import](#import) below |

:::note
The *All active* button is only available in the **Catalog list** view. It is disabled when the Device Tree or Storage list view is selected.
:::

## Context menu {#catalog-context-menu}

Right-clicking on a catalog in the list opens a context menu:

![Catalog context menu showing available actions](/img/devices_catalogs_02_context2.png)

| Action | Condition | Description |
|--------|-----------|-------------|
| *Update* | Active catalog only | Re-scans the catalog from its source path |
| *Explore* | Always | Opens the catalog in the [Explore](Explore) screen to browse its folders and files |
| *Edit* | Always | Opens the [Edit panel](#edit) to modify the catalog's settings |
| *Open folder* | Path is set and not an export | Opens the catalog's source folder in the file manager |
| *Verify Checksums* | Always | Re-calculates and compares checksums for all files in the catalog |
| *Filelight* | Active catalog only | Opens [Filelight](https://apps.kde.org/filelight/) in the catalog's source path |
| *Unassign this catalog* | Catalog is assigned to a virtual group | Removes the catalog from its virtual group (the catalog itself is not deleted) |
| *Delete this catalog* | Physical group catalogs and exports | Permanently removes the catalog from the collection |

## Edit {#edit}

The edit panel gives access to modify the following fields:

![Edit panel for a catalog showing all configurable fields](/img/devices_catalogs_03_edit.png)

| Field | Description |
|-------|-------------|
| *Device Name* | The display name of the catalog |
| *Parent device* | The storage device this catalog belongs to |
| *Source Path* | The folder path from which the catalog is built |
| *File Type* | Restricts the catalog to a specific file type (All, Audio, Image, Text, Video) |
| *Include Hidden Files* | Whether hidden files and folders are included when scanning |
| *Metadata* | Level of metadata indexing: *None*, *Standard*, or *Extended* |
| *Checksum* | Whether file checksums are calculated: *None* or *SHA-256* |
| *Exclude Folders* | List of sub-folders to exclude from the catalog scan |

It is generally recommended to set the correct options when **creating** a catalog rather than editing them later.

## Import {#import}

Catalogs can be imported from a **VVV** (Virtual Volumes View) export file using TAB as the separator.

Each VVV physical volume becomes one separate Katalog catalog.

Steps:
1. In VVV, choose *File / Export…* and select TAB as the separator.
2. In Katalog, go to the Catalogs list and click *Import*, then select the exported file.

:::note
The source path and other details about VVV volumes are not available in the export. Use the *Edit* panel afterward to add the source path if you want to enable future updates.
:::

## Development
Some ideas of developments for this screen:
* See the backlog of [Devices development](https://github.com/users/StephaneCouturier/projects/7/views/1?filterQuery=devices).
