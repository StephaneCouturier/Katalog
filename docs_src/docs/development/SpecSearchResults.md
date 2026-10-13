---
id: SpecSearchResults
title: Search Results
description: Requirements for the K3 Search results page — its columns in file mode and in folders-only mode, its context menu, and its CSV export
version: "2.13"
---

# SEARCH RESULTS

![Status](https://img.shields.io/badge/Status-Approved-brightgreen) ![Version](https://img.shields.io/badge/Version-2.13-blue) ![Implementation](https://img.shields.io/badge/Implementation-planned-lightgrey)

## Context

A search can return files or, with *Folder criteria* and *only list folders in
results* both ticked, one row per distinct folder. In that folders-only mode the
core search model fills each row with an empty file name, a zero size and an
empty date (`core/search.cpp:514-570`). K2 therefore
hides the Name, Size and Date columns for that mode, sizes Directory to its
content and draws the folder icon in Directory
(`qt_widgets/mainwindow_tab_search_pr.cpp:807-813`, `qt_widgets/filesview.cpp`).

Before this spec, K3 showed the same columns in both modes, so a folders-only
search produced three blank columns, and it decided the mode from *only list
folders in results* alone — a box that stays ticked when *Folder criteria* is
unticked, in which case the search returns files but the results were titled as
folders.

This page was created on 2026-10-03 at the user's request ("align the search
results when folder only option ... with K2"). It was renamed from "Search
Results — Column Set" to "Search Results" on 2026-10-13, at the user's request,
since it now also covers the context menu and CSV export.

> **Reading the requirement IDs.** Each requirement has a permanent ID.
> IDs are never renumbered or reused; a retired requirement is marked
> `[Removed]`, not deleted. Status is one of
> `[Implemented] / [Planned] / [Backlog] / [Removed]`.

---

## Scope at a glance

**In scope:** which columns the K3 Search results table shows in file mode and in
folders-only mode, where the folder icon is drawn in folders-only mode, the
Directory width in that mode, the condition that defines folders-only mode; and
the order of the copy entries in the file-row context menu (`SRS-F5`, added
2026-10-03); and the "Explore folder" entry of the row context menu (`SRS-F7`,
`SRS-F8`, added 2026-10-03); the *Export to CSV* action on all results: where
the file is written, how success is reported, and the CSV content (`SRS-O4`,
`SRS-F9`, `SRS-F10`, `SRS-C4`, added 2026-10-13).

**Out of scope (non-goals):** the footer filters (`SpecSearchResultsFilters.md`);
*Export to Catalog*, which keeps its passive
notification `Results exported to catalog: %1` (`SRS-C4`); column widths in general, their persistence, the double-click fit and keyboard
navigation (`SpecCardsAndTables.md`); row colours and icon size
(`SpecTheme.md`); K2, which is in maintenance mode.

**Applies to:** K3 (`qt_quick`) only.

---

## Operational requirements — *why / for whom*

| ID | Requirement | Status |
|----|-------------|--------|
| SRS-O2 | A user copying a result's name or path finds the copy entries listed from the shortest copied text to the longest, in the same order as in Explore. Approved by the user on 2026-10-03. | [Planned] |
| SRS-O3 | A user who finds a file or folder in Search results can go straight to that folder in Explore, browse the catalog around it, and come back to the same results, as in K2. Approved by the user on 2026-10-03. | [Planned] |
| SRS-O4 | A user who exports search results to CSV finds the file in the same place as K2 and the K3 Backup exports put it, and can read its path and open it from the confirmation. Approved by the user on 2026-10-13, after a beta tester reported the confirmation as unreadable. | [Implemented] |
| SRS-O1 | A user who searches for folders only sees a list of folders, not a table of empty file columns. Approved by the user on 2026-10-03. | [Planned] |

## Functional requirements — *what the system does*

| ID | Requirement | Status |
|----|-------------|--------|
| SRS-F1 | When a search returns folders only, the Search results table does not show the Name, Size and Date columns. Every other column is shown exactly as in file mode. As K2 (`mainwindow_tab_search_pr.cpp:807-813`). Approved 2026-10-03. | [Planned] |
| SRS-F2 | In folders-only results, each folder row shows the folder icon in the Directory column and no icon elsewhere, as K2's `FilesView` does. The icon size follows `THM-F7` (`THM-F15`). Approved 2026-10-03. | [Planned] |
| SRS-F3 | In folders-only results the Directory column has **its own saved width**, separate from the file-mode Directory width (`CDT-F12`, key in `CDT-C9`). While none is saved yet, Directory is fitted to its content when the search completes, like K2's auto-sizing, and that fitted width becomes the saved one. The column stays resizable by drag or by the `CDT-F4` double-click fit, and each change is saved. **Deliberate divergence from K2**, where it is auto-sized and cannot be resized. *(Amended 2026-10-03 at the user's request: was "neither saved nor restored".)* | [Planned] |
| SRS-F4 | Folders-only mode is defined as K2 defines it: *Folder criteria* **and** *only list folders in results* are both ticked for the search (`searchOnFolderCriteria && showFoldersOnly`). The same condition drives `SRS-F1`–`SRS-F3`, the results title and the status-bar result wording; with only the second box ticked the search returns files and is presented as files. The texts themselves are unchanged. Approved 2026-10-03 (corrects a K3 divergence that tested the second box alone). | [Planned] |
| SRS-F5 | The K3 Search results file-row context menu offers the copy entries in the `EXP-F19` order: name without extension, name with extension, folder path, file absolute path. The labels stay unchanged. **The order deliberately differs from K2's.** Approved 2026-10-03. | [Planned] |
| SRS-F6 | In folders-only results (`SRS-F4`), the file-row context menu keeps "Copy folder path" active and shows "Copy file name without extension", "Copy file name with extension" and "Copy file absolute path" disabled, since a folder row has no file. They stay visible, in the `SRS-F5` order. Requested by the user on 2026-10-03. | [Planned] |
| SRS-F7 | The row context menu entry "Explore folder" is enabled when the row belongs to a catalog device (row device ID > 0), and disabled otherwise — in practice for results of a search run directly on a disk folder (no catalog), or whose catalog device no longer exists. This applies to file rows and to folders-only rows (`SRS-F4`). As K2 `searchContextOpenExplore` (`qt_widgets/mainwindow_tab_search_ui.cpp:895-925`). Approved 2026-10-03. | [Planned] |
| SRS-F8 | Triggering "Explore folder" opens the Explore page on the row's catalog, after Results in the page stack (Search, Results, Explore), selects the row's Directory in the folder tree (revealed per `EXP-F8`) and lists that folder's files. It does not check the device's active status (`DAS-F11`/`F12` cover opening only), as K2. Explore's Close, and therefore `Esc` (`KBS-F1`), then removes only Explore and returns to the same Results page, unchanged. Approved 2026-10-03 (option b: Results kept, as K2 keeps its Search tab). | [Planned] |
| SRS-F9 | *Export to CSV* on all results writes the CSV file to the per-mode folder used by K2 and by K3 Backup (`BKP-F26`, `AppManager::getCollectionBrowsePath`): the folder of the `.db` in File mode, the collection folder in Memory mode, the user's home folder in Hosted mode. It no longer writes to the Downloads folder. On success, the page shows a `Kirigami.InlineMessage`, `type: Positive`, `showCloseButton: true`, at the top of the page. Its text is `Exported to: %1` in every mode, where %1 is the written file's path, rendered as a clickable link that opens the file. The clickable link is parity with K2's export dialog. Approved by the user on 2026-10-13. | [Implemented] |
| SRS-F10 | The CSV file written by `SRS-F9` keeps K3's current content: tab-separated, a header row `Name\tSize\tDate\tFolder\tCatalog`, then one row per result with those five values. **Accepted divergence from K2**, which writes full path, size and date. Approved by the user on 2026-10-13. | [Implemented] |

## Constructional requirements — *how it is built / limits / MUST-NOTs*

| ID | Requirement | Status |
|----|-------------|--------|
| SRS-C1 | Folders-only mode does not show the internal columns K2 may also show in that mode (Catalog ID, order value, Path, the raw video/audio columns); they are not ported. The change adds no user-visible string, changes nothing under `core/`, creates no new source file, leaves K2 unchanged and adds no settings key other than `Search/ColumnWidthFolderDirectory` (`SRS-F3`, `CDT-C9`; amended 2026-10-03). A hidden column never stores a width of 0 (`CDT-C9`). Approved 2026-10-03. | [Planned] |
| SRS-C2 | `SRS-F5` only reorders entries. It adds, renames and removes no string, changes no action, leaves every other entry where it is, and leaves K2 unchanged. Approved 2026-10-03. | [Planned] |
| SRS-C3 | `SRS-F8` MUST open the catalog through the same path as the Devices page Explore action (`exploreFolders.openByDeviceId`, last page "Explore") and MUST NOT duplicate catalog loading, so `EXP-C9` holds. Running a new search while Explore sits after Results MUST remove only the pages after Results and reuse the Results page in place — never remove and re-push the static Results page in one pass. It reuses the existing "Explore folder" string and adds no new user-visible string. It changes nothing in `core/` or `qt_widgets/`, creates no new file and adds no settings key. Getting the row's device ID is a `qt_quick` (AppManager) helper shared with the existing row active-status lookup. Approved 2026-10-03. | [Planned] |
| SRS-C4 | The `SRS-F9` message is a **deliberate exception** to the rule "transient success → `showPassiveNotification`" (CLAUDE.md, `SpecValidationRules.md`): it carries an action (the link) and its path must stay readable, and a beta tester found the passive notification unreadable. It reuses the existing string `Exported to: %1` verbatim and adds no user-visible string. K2's `Results exported to the collection folder:` is not used, because it is wrong in Hosted mode (the file goes to the home folder). The folder comes from the existing `AppManager::getCollectionBrowsePath`, shared with Backup. *Export to Catalog* keeps its passive notification `Results exported to catalog: %1`, unchanged. K2 is unchanged. Approved by the user on 2026-10-13. | [Implemented] |

---

## Manual test charter

- **SRS-F1** — Run a search with *Folder criteria* and *only list folders in results* ticked: no Name, Size or Date column; Directory and Catalog Name are shown as in file mode. Run a file search: Name, Size and Date are back.
- **SRS-F2** — In folders-only results each row shows the folder icon in Directory and none in other columns. Toggle *Use bigger icon size*: the icon follows.
- **SRS-F3** — With no saved folders-only width, run a folders-only search: Directory fits its longest loaded path. Drag it to another width and run the folders-only search again: that width is restored, not refitted. Run a file search: Directory has its own file-mode width, unchanged by the folders-only session.
- **SRS-F4** — Tick *only list folders in results*, then untick *Folder criteria* and search: the rows are files, all file columns are shown, and the title and status bar speak of files, not folders.
- **SRS-C1** — Set Name to 300 in file mode, run a folders-only search, then a file search: Name is 300. The settings file never holds `Search/ColumnWidthName=0`. `ninja translations_lupdate` shows no new string; the `core/` and `qt_widgets/` diffs are empty.
- **SRS-F5 / SRS-C2** — In K3 Search results, right-click a file. The copy entries appear in the same order as in Explore, with unchanged labels and copied text, and the other entries are unchanged.
- **SRS-F6** — In folders-only results, right-click a row: "Copy folder path" is active and copies the folder; the three file copy entries are greyed out. In file results all four are active.
- **SRS-F7** — Right-click a result from a catalog: "Explore folder" is enabled. Right-click a result of a search run directly on a disk folder: it is greyed out. In folders-only results it is enabled on catalog rows.
- **SRS-F8** — Pick "Explore folder" on a result whose folder is several levels deep. Explore opens after Results on that catalog, with the folder selected and visible (its ancestors expanded) and its files listed. Press `Esc`, or Explore's Close: you are back on the same Results, same scroll and selection. Repeat in folders-only results with a folder row. Unmount the drive and repeat: Explore still opens.
- **SRS-F9** — In File mode, run a search and pick *Export to CSV*. The file is written next to the `.db`, not in Downloads. A positive inline message with a close button appears at the top of Results, reading `Exported to: ` followed by the path. Click the path: the file opens. Close the message: it disappears. Repeat in Memory mode (file in the collection folder) and Hosted mode (file in the home folder): same message text.
- **SRS-F10** — Open the exported file: tab-separated, header `Name\tSize\tDate\tFolder\tCatalog` (tabs), one row per result.
- **SRS-C4** — `ninja translations_lupdate` shows no new source text. Pick *Export to Catalog*: the passive notification `Results exported to catalog: ...` still appears. The `qt_widgets/` diff is empty.
- **SRS-C3** — With Explore open after Results, go to Search and run a new search: Results shows the new results, Explore is gone, nothing crashes. In Memory mode, use "Explore folder": files and folder sizes are listed, not zeros. `ninja translations_lupdate` shows no new source text; the `core/` and `qt_widgets/` diffs are empty.
