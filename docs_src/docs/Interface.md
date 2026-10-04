---
version: "2.13"
---

# Interface
![2.13](https://img.shields.io/badge/Version-2.13-blue)

## Summary
This page describes the parts of the Katalog window that are shared by all screens: the drawer, the way pages open side by side, the keyboard shortcuts, the activity panel, the appearance options and the *About* page.

<!-- screenshot: k3/interface_01_overview.png — main window: drawer pinned on the left, Selection page, Search page and Results page side by side -->

The window has three areas:
* the **drawer** on the left, to open a collection and go to any screen,
* the **pages**, shown side by side, starting with the [Selection](Selection),
* the **activity panel** at the bottom, while a catalog creation, a device update or a search is running.

## Pages {#pages}
The [Selection](Selection) page is the starting point: it lists the devices of the collection and is always the first column.

Opening a screen from the drawer (for example *Search*, *Devices* or *Explore*) shows its page next to the Selection. Only one screen is open at a time: opening another one replaces it.<br/>
Running a search opens the *Results* page to the right of the *Search* page. From a result, *Explore folder* opens the [Explore](Explore) page on that folder.

Each page has a *Close* button in its toolbar, which goes back one step. *Settings* and *About* open over the other pages, and also close with their *Close* button.

### Show or hide the Selection {#show-hide-selection}
When a screen is open next to the Selection, the Selection can be hidden to give more room to that screen.<br/>
Use the button at the bottom of the drawer: it reads *Selection shown* or *Selection hidden*. It is available only when a screen is open next to the Selection. The choice is kept for the next start.

### Window size and position
The size and position of the window, and whether it is maximized, are restored at the next start.

## Drawer {#drawer}

<!-- screenshot: k3/interface_02_drawer.png — drawer pinned, with the Open... menu unfolded showing the recent collections -->

The top of the drawer shows the **open collection**: an icon for its mode (Memory, File or Hosted) and its name:
* in *Memory* mode, the name of the collection folder,
* in *File* mode, the name of the database file,
* in *Hosted* mode, the server name, a `/`, then the database name.

Below it:

| Item | Use |
|------|-----|
| *Open...* | Opens a collection: *Collection Folder...* (Memory mode), *SQLite Database...* (File mode) or *Hosted Database...* (Hosted mode). The menu then lists the **10 most recently opened collections**, most recent first: click one to reopen it directly. |
| *New...* | Creates a new collection: *SQLite Database...* creates a new database file and opens it. |
| *Selection* … *Tags* | Open the screen: [Selection](Selection), [Search](Search), [Devices](Devices), [Explore](Explore), [Create](Create), [Backup](BackUp), [Statistics](Statistics), [Tags](Tags). |
| *Settings* | Opens the [Settings](Settings). |
| *Documentation* | Opens this documentation in the web browser. |
| *About* | Opens the [About](#about) page. |
| *Quit* | Closes Katalog. |

Switching to another collection does not restart Katalog: the collection is opened directly and all screens are refreshed.

### Pinned or floating drawer
The first button at the bottom of the drawer reads *Drawer pinned* or *Drawer floating*:
* **pinned**: the drawer stays open on the left of the pages,
* **floating**: the drawer opens over the pages and closes after a choice.

The choice is kept for the next start.

## Keyboard shortcuts {#keyboard-shortcuts}

| Key | Action |
|-----|--------|
| `Esc` | Goes back one step: same as the *Close* button of the last open page (or *Cancel* on the device edit page). |
| `Ctrl+F` | Puts the cursor in the device filter of the [Selection](Selection) page. |
| `Ctrl+Q` | Quits Katalog on Linux (`Cmd+Q` on macOS). |

How `Esc` works:
* An open dialog, menu or drop-down list closes first; the page under it stays open.
* `Esc` closes the page even when the cursor is in a text field.
* On the device edit page and on the [Create](Create) page, `Esc` discards what was typed, like *Cancel* and *Close*. There is no confirmation.

Keyboard navigation in the search results is described on the [Search](Search) page.

## Activity panel {#activity-panel}

<!-- screenshot: k3/interface_03_activity_panel.png — activity panel at the bottom: a device update running, with the queue showing one waiting update -->

The activity panel appears at the bottom of the pages while a **catalog creation**, a **device update** or a **search** is running. It is the one place to follow these operations, whichever page is open:
* the status message shows what is being processed and how far it has got,
* *Stop* stops the running operation.

When an operation ends (completed, stopped or failed), the panel stays visible a few seconds with its final status, then hides.

### Queue {#queue}
Katalog runs one catalog creation or device update at a time. A creation or update requested while another one is running is **added to the queue** instead of being refused:
* the panel shows *Queue* and the number of operations waiting,
* each waiting operation is listed (*Create* or *Update*, with the device name) and can be taken out with *Remove*,
* *Clear* empties the queue,
* the queued operations run one after another, in the order they were requested.

*Stop* ends only the running operation: the next one in the queue then starts. A device already running or waiting is not added a second time.

A search cannot start while a creation or update is running (the *Search* button is disabled), and a creation or update cannot start while a search is running.

## Appearance {#appearance}
The look of Katalog can be adjusted in the *Application* section of the [Settings](Settings#application):
* **Theme**: desktop colours or Katalog's own colours, always following the light or dark mode of the system.
* **Icon and text size**: bigger icons, and a smaller or larger text.
* **Language**: the language of the interface.

Buttons that show only an icon display their name when the mouse pointer stays over them.

## About {#about}

<!-- screenshot: k3/interface_04_about.png — About page with the "Copy version and system information" action -->

The *About* page shows the version of Katalog, its licence and its author.

*Copy version and system information* copies a short text to the clipboard, ready to be pasted in a bug report. It contains:
* the Katalog version and release date,
* the versions of Qt and KDE Frameworks,
* the operating system, kernel and processor architecture,
* the language of the interface,
* the database mode and database version,
* the display server and desktop environment.

It contains **no personal information**: no user or computer name, no path to files or collections, no collection name, and no server address or password.

## Development
* See the backlog of [Interface development](https://github.com/users/StephaneCouturier/projects/7/views/1?filterQuery=Interface).
