---
version: "3.0"
---
# Settings
![3.0](https://img.shields.io/badge/Version-3.0-blue)

## Summary
This page describes all the features of the **Settings** screen and how to use them. The screen has two tabs:
* *Application*:
  * [Interface](#interface): language, theme, text size, icon size, window size and behavior options,
  * [App information](#app-information): version and settings file,
* *Collection*:
  * [Collection & Database](#collection-database): the open collection, its data mode, the [export to another data mode](#export), the database version and quality check, and the images folder,
  * [Collection Import & Synchronization](#import-update): import devices from another collection, or update previously imported ones.

*Settings* opens from the drawer, over the other pages, on the *Application* tab. *Open... > Hosted Database...* in the drawer opens it on the *Collection* tab, with the hosted connection form. *Close* (or `Esc`) goes back to the other pages (see [Interface](Interface)).

## Application {#application}

![Application tab: version, language, theme, text size, Reset window size and behavior options](/img/settings_05_application.png)

### Interface {#interface}

#### Language {#language}
The language of the interface. On the first start, Katalog uses the language of the system when it is available, and English (US) otherwise.

#### Theme {#theme}
| Theme | Description |
|-------|-------------|
| *Desktop Theme* | Follows the colours of the desktop. |
| *Desktop Theme (gray)* | Follows the colours of the desktop, with grey surfaces and highlights. |
| *Katalog Colors* | Katalog's own blue colours. |

With every theme, Katalog follows the **light or dark mode** of the system, on Linux (KDE Plasma and other desktops), Windows and macOS, and changes with it without a restart. Outside KDE Plasma, Katalog uses the Breeze light and dark colours rather than the desktop's own colour scheme.

:::note
On some Linux desktops (for example Linux Mint with Cinnamon), the dark mode is not detected when Katalog starts: the window opens light, then follows the next change of light or dark mode.
:::

#### Text size {#text-size}
Makes all the text of Katalog smaller or larger, from 80% to 120%, with the slider or its two buttons. The size starts from the font size set in the system, so it also follows a change of the system font. The collection name at the top of the drawer and the page titles keep the system size.

#### Icon size {#icon-size}
*Use bigger icon size*: shows larger icons in the lists.

#### Window size {#window-size}
*Reset window size* puts the window back to its default size, **1280×800** (or the size of the screen, if smaller). A maximized window is restored first. The position of the window is not changed. This is also the size of the window the first time Katalog starts.

#### Behavior {#behavior}
| Option | Description |
|--------|-------------|
| *File sorting is Case Sensitive* | When enabled, sorting respects letter case, giving this order: AA, AB, AC, Aa, Ab, Ac. |
| *Refresh device status when returning to the application* | When enabled, the status of devices (available or not) is checked again each time the Katalog window becomes active, so a drive connected or removed meanwhile is shown correctly. Disabled by default. |
| *Authorize actions that delete files (no recovery)* | When enabled, files can be **permanently deleted** from the [Search](Search) results and the [Explore](Explore) screen. Disabled by default: these actions are then greyed out. *Move to Trash*, which can be undone, is always available. |

### App information {#app-information}

#### Version
* The version and release date of Katalog, and *Release Notes* to read what changed.
* *Check for a new version on startup*: when enabled, Katalog checks at start if a newer version is available.

#### Settings file
The path of the file where Katalog keeps its settings and last choices. *Open* opens it in the associated application.

## Collection & Database {#collection-database}

![Settings page, Collection tab: Collection & Database section, demo collection in File mode](/img/settings_01_overview.png)

### Collection {#collection}
A **collection** is a single group of devices and all related information: catalogs, statistics, tags, search history and backup links.<br/>
It is possible to have several collections and to switch between them.

Collections are opened and created from the drawer, with *Open...* and *New...* (see [Interface](Interface#drawer)). Switching to another collection does not restart Katalog.

The section shows:
* *Database Mode*: the data mode of the open collection (see below),
* *Collection*: where the collection is stored (folder, database file, or server and database name), with an *Edit* button:
  * in *File* mode, it opens the database file in the application associated with it (for example [DB Browser for SQLite](https://sqlitebrowser.org)),
  * in *Memory* mode, it opens the collection folder in the file manager,
  * in *Hosted* mode, it opens the database in phpMyAdmin, at the *phpMyAdmin URL* set for the connection.

### Data modes {#database-memory-mode}
Katalog provides 3 *data modes*, or ways of storing and handling the data of a collection.

| Mode | Database type | Data storage | Files | Search speed | Cataloging speed |
|------|---------------|--------------|-------|--------------|------------------|
| **Memory** | computer memory | in `.csv` tab-separated files (for devices, statistics, etc.) and in `.idx` files (for the file lists of catalogs) | better for regular synchronization of the files to a cloud | fastest search once catalogs are in memory (longer the first time a catalog is used) | slightly faster |
| **File** (default) | SQLite file, low memory use | all in the SQLite file | all data in 1 file, which can grow to several hundred MB | faster for the first search, slower for repeated searches in a big collection | slightly slower |
| **Hosted** | MySQL/MariaDB server | all in the database server | data centralized on a server, accessible from several computers on the network | server-side query performance, suitable for large collections | slightly slower (network) |

### Hosted mode {#hosted-mode}

![Settings page with the Hosted connection form](/img/settings_02_hosted.png)

In *Hosted* mode, the collection is stored in a MySQL/MariaDB database on a local or network server.<br/>
The connection form is shown when the open collection is in *Hosted* mode, or when choosing *Open...* > *Hosted Database...* in the drawer.

| Field | Description |
|-------|-------------|
| *Host Name* | `localhost` or the IP address of the database server, on this computer or the local network (see [Security](#hosted-security)). Left empty, it becomes `localhost`. |
| *Database Name* | Name of the database on the server. |
| *Port* | Port of the server (`3306` for MySQL/MariaDB). |
| *User Name* | Database user name. |
| *Password* | Database password. |
| *phpMyAdmin URL* | Address of phpMyAdmin, used by the *Edit* button. |

*Connect* opens the collection on the server.

Prerequisites:
* A MySQL/MariaDB server must be running and reachable.
* The database must already exist on the server; Katalog creates the tables it needs.

#### Security {#hosted-security}
To keep the data of a collection on computers the user controls, Katalog only connects to a database server on this computer or on the local network. The host is checked on *Connect*, and when a *Hosted* collection is opened from *Recent collections* in the drawer:

| Host | What happens |
|------|--------------|
| This computer: `localhost`, `127.x.x.x` | Katalog connects. |
| Local network: `192.168.x.x`, `10.x.x.x`, `172.16.x.x` to `172.31.x.x` | Katalog asks for a confirmation first (*Private network connection*, *Your data will be sent to this network database.*). *No* is the default; nothing is changed if it is chosen. |
| Anything else: a public address, or a server name (for example `mydb.example.com` or `nas`) | Refused: *Invalid hostname.* is shown at the top of the form with the accepted addresses. Nothing is connected or saved. A refused collection opened from *Recent collections* shows the form, filled in, with the same message. |

A server on the local network is given by its IP address, not by its name.

### Export to another data mode {#export}
The open collection can be copied to another data mode, for example to move a collection from a server to a single file. The open collection is not changed and stays open.

| Data mode of the open collection | *Export to SQLite file* | *Export to Memory mode (csv)* |
|---|:---:|:---:|
| *Memory* | yes | — |
| *File* | — | yes |
| *Hosted* | yes | yes |

* *Export to SQLite file* asks for the file to create, `export.db` in the collection folder by default (in the home folder in *Hosted* mode). If the file already exists, it is moved to the trash first. In *Memory* mode, the files and folders of all catalogs are loaded first, which can take a while for a big collection.
* *Export to Memory mode (csv)* asks for a folder, which **must be empty**. Katalog writes there the `.csv` files of the collection and the `.idx` files of each catalog.

The progress is shown at the bottom of the page, with *Cancel*. Cancelling removes what the export had written. A message shows the path of the export when it is done. The export buttons are not available while a catalog is created or updated, or while a search runs.

The exported collection can then be opened from the drawer with *Open...*.

:::note
In *Memory* mode, if a search was run before the export, folders that contain no files may be missing from the SQLite file for the catalogs that were searched. To get them all, export just after opening the collection.
:::

### Database version and quality check {#quality-check}

![Quality check report of the demo collection](/img/settings_03_quality_check.png)

*Database Version* shows the version of the database structure of the open collection.

*Quality check* runs a series of read-only checks on the open collection and shows a report. It looks for inconsistencies between devices, storage and catalogs:
* storage devices linked to a missing storage,
* storage not linked to any device,
* Storage IDs used by more than one storage,
* catalog devices linked to a missing catalog,
* catalogs not linked to any device,
* devices with a missing parent device.

For each check, the report lists what was found, or *Nothing found*. *Copy to Clipboard* copies the report, for example to attach it to a bug report.<br/>
The quality check **repairs nothing**: it only reports.

### Images folder {#images-folder}
The *Images folder* is the folder where the pictures of storage devices are kept. It is a per-collection setting.
* Type a path in the field, or click *Select* to choose a folder.
* When it is not set, the folder `images` inside the collection folder is used.

Pictures placed in this folder can then be chosen for a storage device when editing it in the [Devices](Devices) screen.

## Collection Import & Synchronization {#import-update}

![Import section with a source collection selected, a device chosen in the source tree, and the import report](/img/settings_04_import.png)

Katalog can import devices from another collection into the open one, and later update them when the source changes.
* The **source** is the other collection. It is only read, never modified.
* The **target** is the open collection, where the devices and catalogs are added.

| Field | Use |
|-------|-----|
| *Data mode* | Type of the source: *Katalog / File* (a `.db` file), *Katalog / Memory* (a collection folder), or *VVV / Tab Separated Values* (an export file from VVV). |
| *Source* | The source collection or file, chosen with *Select*. |
| *Device* | The device to import, chosen in the device tree of the source. *Import* imports it with all its contents. |
| *Update* | The source collections that devices were imported from. *Update* refreshes, from the chosen source, the catalogs of all devices imported from it. |

**What is imported**<br/>
Devices, their catalogs and file lists, statistics, backup links and excluded folders. The source's hierarchy is kept in the target by adding the necessary parent devices. When a device or catalog name already exists in the target, the imported one is renamed (for example `My Drive (2)`).

**Report**<br/>
When an update ends, a message summarizes what changed: the number of catalogs updated, and the number and total size of files before and after.

:::note
The source collection must have the same database version as the open one. If it was made with an older version of Katalog, open it once with this version first: its database is then updated.
:::

**Import from VVV**<br/>
[VVV (Virtual Volumes View)](https://www.fsoft.it/VVV/) is another cataloging application. With *VVV / Tab Separated Values*, select a file exported from VVV, then click *Import*: its catalogs are added under a new virtual group.

## Development
* See the backlog of [Settings development](https://github.com/users/StephaneCouturier/projects/7/views/1?filterQuery=settings).
