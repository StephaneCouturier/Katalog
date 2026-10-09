---
sidebar_position: 1
version: "3.0"
---

# Tutorial
![3.0](https://img.shields.io/badge/Version-3.0-blue)

Let's discover **Katalog in less than 5 minutes**: create a collection, catalog a folder, and search in it.

## Download and run
Download Katalog for Linux, Windows or macOS: see [Multi-Platform Support](Overview#multi-platform-support) for the available packages and the system requirements.

## First start {#first-start}
On the first start, a welcome message gives the language and theme that were applied (they can be changed later in the [Settings](Settings#application)), and offers:
* *Create new...*: creates a new **collection**, the place where Katalog keeps all its data. A dialog proposes where to save it, as a database file (`.db`). A folder in your user directory is a good place.
* *Open existing...*: opens a collection database (`.db`) created before, for example with Katalog 2.

Closing the dialogs without choosing is possible: Katalog starts without a collection, and one is opened or created later from the drawer (see [Interface](Interface)).

![First start: welcome message](/img/tutorial_01_welcome.png)

After *Create new...*, a message lists the three steps to create a first catalog. *OK* opens the [Create](Create) page.

## Create a catalog {#create}
A **catalog** is the index of the files of one folder.
1. *Source path*: type the folder to catalog, or choose it with *Select the path*. For example, your *Documents* folder.
1. *Storage*: keep *Local disk*, the storage device that Katalog created for your computer.
1. *Catalog name*: type a name, or use *Generate name from path*.
1. Click *Create* at the top of the page.

![Create page with a folder selected](/img/tutorial_02_create.png)

The progress is shown at the bottom of the window. When it ends, a message confirms that the catalog was created, and it appears in the device tree of the [Selection](Selection) page, under *Local disk*.

## Search files {#search}
1. On the [Selection](Selection) page, right-click the new catalog and choose *Search*: the catalog is selected and the [Search](Search) page opens next to it.
1. Type a text in the file name field, for example `report`.
1. Click *Search* (or press `Enter`).

The [Results](SearchResults) page opens to the right, with the files found. Click a file to open it, or right-click it for more actions.

![Search and results](/img/interface_01_overview.png)

The catalog stays searchable when the folder or the drive is not connected: this is what Katalog is for. Catalog your external drives the same way, each under its own storage device (see [Devices](Devices)), and search all of them at once by selecting the *Physical Group* on the Selection page.

## Next steps
* Use more [search criteria](Search): size, dates, file types, duplicates, differences.
* Browse a catalog in [Explore](Explore).
* Organize your drives in [Devices](Devices).
* Back up a folder with [Backup](BackUp).
* All the parts of the window are described in [Interface](Interface).
