---
version: "2.10"
---
# Settings
![2.10](https://img.shields.io/badge/Version-2.10-blue)

## Summary
This page describes all the features of the **Settings** screen and how to use them.
* Data management
* Language & Theme
* About

![Settings screen overview](/img/screen_settings_01.png)

## Data management

### Collection

A Collection is a single group of devices and all related information such as statistics.<br/>
The *Collection folder* is the folder on your computer where all data of a collection are stored.<br/>
It is possible to have several Collections.

### Data modes
Katalog provides 3 "*Data modes*" or different ways of storing and handling data.

Note 1: there is no feature yet to convert a collection in one mode into another mode.

Note 2: changing the mode requires to click *Apply and restart*

| Mode   | Database type      | Data storage |Files | Search Speed | Cataloging speed |
| -------| -------------------|---|---|---|---|
| **Memory** (default)| computer memory |  in .csv tab separated files (for devices, statistics, etc.) and in .idx files (for catalogs file lists)|Better for regular synch of the files to a cloud|Fastest search speed once catalogs are in memory (longer time for the first time a catalog is used) | Slightly Faster|
| **File**   | SQLite file, low memory use | all in the SQLite file|all data clubbed in 1 file that can grow to several hundred Mb in size  |Faster for 1st search, slower for repetitive search in a big collection|Slightly slower|
| **Hosted** | MySQL/MariaDB server | all data stored in the hosted database server|Data centralized on a server, accessible from multiple machines on the network|Server-side query performance, suitable for large collections|Slightly slower (network overhead)|

![Data modes diagram](/img/settings_database-model.png)

### Database Memory mode {#database-memory-mode}
![Memory mode settings showing the collection folder path and related options](/img/screen_settings_02_memory.png)

Collection folder actions:
* Type the path of the Collection folder and press Enter to load the collection
* Select the path of the Collection folder and load the collection
* Open the collection folder in the system's default file manager
* Export the collection to an SQLite database file, to switch to using the collection in *File* mode

Settings for Memory mode:
* *Back up*: Enable or disable (default) keeping a copy of a catalog before updating it (the copy will have a `.bak` extension)
* *Start up*: Enable or disable (default) preloading the last used catalogs (last selection) to get faster search after
* *Start up*: Enable or disable (default) loading the last opened Catalog in the [Explore](Explore) screen

### Database File mode
![File mode settings showing the database file path and related options](/img/screen_settings_03_file.png)

Collection file actions:
* Type the path of the file and press Enter to load the collection
* *Select and open database file*: select and load an existing collection file
* *Edit*: Open the SQLite database in a database editor (e.g. [SQLite Browser](http://sqlitebrowser.org))
* *New*: Create a new collection file and load it
* *Export to Memory mode*: Export the collection to CSV and index files, to switch to using the collection in *Memory* mode

### Database Hosted mode
![Hosted mode settings showing the server connection fields](/img/screen_settings_05_hosted.png)

The collection data is saved to a database hosted on a local or network server (MySQL/MariaDB).

Connection settings:
* **Host Name** — The hostname or IP address of the database server (default: `localhost`)
* **Database Name** — The name of the database on the server
* **Port** — The port number (default: `3306` for MySQL/MariaDB)
* **User Name** — The database user name
* **Password** — The database password

Fill in all fields and click *Apply and restart* to connect.

#### Security
* Only **local** (localhost, 127.x.x.x) and **private network** (192.168.x.x, 10.x.x.x, 172.16-31.x.x) hostnames are accepted. Public IPs and domain names are rejected.
* When connecting to a private network address, a confirmation dialog is displayed before proceeding.

#### Prerequisites
* A MySQL/MariaDB server must be running and accessible.
* The database must already exist on the server (Katalog will create the required tables automatically).
* The corresponding Qt SQL driver plugin must be installed (`QMYSQL` for MySQL/MariaDB).

## Language & Theme
* Choose the Language for the application.
* Choose the theme for the application and restart to apply it.
* Option to use bigger icon size.
* Option to enable case-sensitive file sorting.
* Button to *Open Settings file* (local file where Katalog options and last selections are stored).

### Themes
| Katalog theme | Context for use |
|---|---|
| Default Theme | auto adapts to any OS and light/dark themes |
| Katalog Color (light) | only for Light desktop themes (not suitable for dark desktop themes) |
| Katalog Color (dark) | only for Dark desktop themes (not suitable for light desktop themes) |

Examples:

- Light desktop / Katalog Colors (light)
![Light desktop with Katalog Colors light theme](/img/settings-themes-light-desktop-katalog-colors-light.png)

- Dark desktop / Katalog Colors (dark)
![Dark desktop with Katalog Colors dark theme](/img/settings-themes-dark-desktop-katalog-colors-dark.png)

- Dark desktop / Default Theme
![Dark desktop with default theme](/img/settings-themes-dark-desktop-default-theme.png)

## About
* Version and Date of the application.
* Option to check for a new version at start up.
* Button to open this Documentation site.
* Button to open the Release Notes.
