---
id: SpecExplore
title: Explore — Directory Tree
description: Requirements for the visibility of the Explore directory tree and for the size figures of the Explore file list
---

# EXPLORE — DIRECTORY TREE

![Status](https://img.shields.io/badge/Status-Approved-brightgreen) ![Implementation](https://img.shields.io/badge/Implementation-complete-brightgreen)

## Context

The Explore screen shows a catalog's directory hierarchy in its left panel and
the files of the selected directory on the right. K2 renders that hierarchy in a
tree widget, which brings expansion and collapsing for free and opens the tree to
its first rank on load. K3 renders the same hierarchy as a flat, depth-ordered
list with indentation; before this spec nothing could be folded away and the
whole hierarchy was shown at once whatever its size.

This spec defines how much of the hierarchy is visible at any moment, and how the
user changes it. It is the only source of requirements for that behaviour.

It also defines the size figures shown in the Explore file list: the recursive
total carried by each listed folder row, the direct total of the folder currently
open, and the catalog's own total. Those were added on 2026-08-29, so that a user
browsing a catalog while the device is disconnected can tell which folders hold
the space. Before them a folder row's size cell was blank and only the open
folder's direct total was shown.

**Decision of 2026-10-11 (narrow window).** The maintainer, checking every page
against `SpecPageLayout.md` PGL-O3: "only Explore has some issues left: the top
row gets clustered or items are hidden." The three display options of the
file-list header row move into a Kirigami toolbar that folds what does not fit
into its overflow menu, as on the Devices page (`EXP-F24`, `EXP-C20`). Unlike
`DVP-F28`, the two tick boxes stay tick boxes. `EXP-F13` (catalog total size on
this same row, Planned) must fold or elide when it is built, so that it does not
bring the clipping back. The count row below it wraps its two groups onto a
second line rather than clipping (`EXP-F26`, "yes, make the second row wrap").

**Decision of 2026-10-13 (beta feedback #8).** The maintainer approved a K3-only
behaviour that goes beyond K2: while Explore is open, selecting a Catalog card on
the Selection page opens that catalog in Explore at once (`EXP-O7`, `EXP-F27`,
`EXP-C21`). K2 opens Explore only through its explicit Explore action and its
startup restore. K2's Memory-mode warning for catalogs over 200,000 files is not
ported; whether to port it is an open point (*Open, not authorised*).

**Decision of 2026-10-13 (Memory-mode catalog load).** Follow-up to `EXP-F27`.
In Memory mode, opening a catalog in Explore first loads its files from its
`.idx` file into memory; until now that ran with no feedback and could not be
stopped. The maintainer approved reporting that load in the activity panel with a
Stop control, letting a later open replace a running load, and excluding it
against catalog creation, device update and search (`EXP-O8`, `EXP-F28`–`EXP-F32`,
`EXP-C22`–`EXP-C25`; `SpecOperationQueue.md` `OPQ-F19`, `OPQ-F20`, `OPQ-C18`).
Loading the data into the Explore model and screen, in every mode, is a separate
item, listed under *Open, not authorised*.

> **Reading the requirement IDs.** Each requirement has a permanent ID.
> IDs are never renumbered or reused; a retired requirement is marked
> `[Removed]`, not deleted. Status is one of
> `[Implemented] / [Planned] / [Backlog] / [Removed]`.

---

## Scope at a glance

**In scope:** which directory rows of the Explore tree are visible; the per-row
disclosure control; the initial expansion depth; the four header controls
(collapse one level, expand one level, collapse all, expand all) and when they
are available; revealing a row that external navigation has selected; and the
aggregate size figures presented in the Explore file list and its header; and
the column set of the Explore file list and the persistence of its sort
(`EXP-F16`–`EXP-F18`, added 2026-10-02); and the directory tree's context menu,
with its single "Tag this folder" entry and the hand-off to the Tags page
(`EXP-F21`–`EXP-F23`, added 2026-10-10); and opening a catalog in an open
Explore page when a Catalog card is selected on the Selection page (`EXP-F27`,
`EXP-C21`, added 2026-10-13); and, in Memory mode, the load of a catalog's files into memory when Explore opens it — its report in the activity panel, its Stop, its replacement by a later open, and its exclusion with other operations (`EXP-F28`–`EXP-F32`, added 2026-10-13).

**Out of scope (non-goals):** everything else on the Explore screen — loading the
catalog (other than the selection trigger of `EXP-F27` and the Memory-mode load of `EXP-F28`–`EXP-F32`), loading the catalog's data into the Explore model and screen, the file list beyond the size figures and column set named above, the
display options, the context menus other than their copy entries (`EXP-F19`, `EXP-F20`, added 2026-10-03) and the directory tree's "Tag this folder" entry (`EXP-F21`), the Tags page itself beyond being opened pre-filled (`EXP-F22`), sorting other than its persistence, and the Selection page's device tree. None of those
are governed by this spec; this spec does not authorise changing any of them.
Persisting expansion state is explicitly excluded — see `EXP-C2`. The directory
tree rows carry no aggregate figures at all: the always-zero item count that the
tree computes today is a known defect, deliberately deferred and recorded in
`SpecBacklogNotes.md`, not a requirement of this spec.

**Applies to:** K3 (`qt_quick`) only. K2's Explore tree already provides
`EXP-F1` through its tree widget, and an initial expansion of its own that
`EXP-F2` deliberately no longer matches; the header controls of
`EXP-F3`–`EXP-F7` are not required in K2, which is in maintenance mode. The one
K2 change this spec authorises is the bug fix `EXP-C19` (2026-10-10).
`EXP-F27` is K3-only and goes beyond K2 (`EXP-C21`).

---

## Operational requirements — *why / for whom*

Goals in real use, independent of how they are built.

| ID | Requirement | Status |
|----|-------------|--------|
| EXP-O1 | A user browsing a catalog opens only the branches of interest and keeps the rest of the hierarchy folded away, so a deep or wide directory tree stays readable. | [Implemented] |
| EXP-O2 | A user changes how much of the hierarchy is shown in one action — one rank at a time, or the whole tree at once — instead of clicking through every branch. | [Implemented] |
| EXP-O3 | A user browsing a catalog while the device is disconnected sees how much space each sub-folder holds, and how large the catalog is in total, so the largest space consumers can be identified offline. | [Planned] |
| EXP-O5 | A user copying a file's or folder's name or path from Explore finds the same entries, with the same wording, as in K2 and in Search results, listed from the shortest copied text to the longest. Approved by the user on 2026-10-03. | [Planned] |
| EXP-O6 | A user browsing a catalog in Explore tags a folder of the directory tree without retyping or browsing for its path, and comes back to the same place in Explore afterwards, as K2 allows from its Explore tree. Approved 2026-10-10. | [Implemented] |
| EXP-O7 | A user with Explore open moves from one catalog to another by selecting it on the Selection page, without going through the card's "Explore" action each time. Goes beyond K2. Approved 2026-10-13 (beta feedback #8). | [Implemented] |
| EXP-O8 | A user opening a large catalog in Explore in Memory mode sees that it is loading and how far it has got, and can stop it or move to another catalog instead of waiting for a load no longer wanted. Approved 2026-10-13. | [Implemented] |
| EXP-O4 | A user exploring a catalog sees the same file attributes as in K2's Explore file list — type, MIME type, checksum and, for catalogs that include metadata, media dimensions, duration and audio tags — so moving from K2 to K3 loses no information. Approved by the user on 2026-10-02. | [Planned] |

## Functional requirements — *what the system does*

Observable behaviour that can be triggered and watched.

| ID | Requirement | Status |
|----|-------------|--------|
| EXP-F1 | A directory row that has sub-directories carries a disclosure control; activating it hides its descendants, activating it again shows them. A row without sub-directories has no control but reserves the same space, so row geometry and indentation never shift between rows. | [Implemented] |
| EXP-F2 | When the Explore directory tree loads a catalog, the root row, its children and its grandchildren are shown; every rank below the grandchildren starts collapsed. **Deliberate divergence from K2**, approved 2026-08-27: K2 opens the first rank only, K3 opens two. Rationale: a catalog's meaningful content usually starts a folder or two below the root, so opening a single rank made "expand" the first action on every catalog. This is no longer a parity requirement and MUST NOT be "corrected" back to K2's single-rank depth. | [Implemented] |
| EXP-F3 | **Collapse one level** collapses the deepest rank that is currently visible and open. | [Implemented] |
| EXP-F4 | **Expand one level** opens the shallowest rank that is currently collapsed. | [Implemented] |
| EXP-F5 | **Collapse all** collapses every row that has sub-directories, including the root row. | [Implemented] |
| EXP-F6 | **Expand all** clears all collapse state, so every directory of the catalog is visible. | [Implemented] |
| EXP-F7 | Each of the four controls is disabled whenever activating it would change nothing: the collapse controls when nothing further can be collapsed, the expand controls when nothing further can be expanded. | [Implemented] |
| EXP-F8 | When navigation originating outside the directory tree selects a directory whose row is hidden under a collapsed ancestor, the ancestors of that row are expanded so the selected row is visible. A selected row is never left hidden. | [Implemented] |
| EXP-F9 | The four header controls are icon-only and identified by tooltips. Their exact texts are `Collapse one level`, `Expand one level`, `Collapse all`, `Expand all`. The first two are reused verbatim from the Selection page's pair; the last two are new strings, approved on 2026-08-27. | [Implemented] |
| EXP-F10 | A folder row in the Explore file list shows, in the Size column, the **recursive** total size of the files it holds: those directly in it plus those in every folder beneath it, at any depth. A folder containing no files at any depth shows a zero size, not a blank cell. | [Planned] |
| EXP-F11 | The file-list header keeps showing, for the folder currently open, the **direct** count and total size of the files directly in it, excluding its sub-folders. The header figure and the row figures are deliberately computed differently: the header describes the folder being viewed, each row describes a different folder listed inside it, so the same folder never carries two different numbers on one screen. They MUST NOT be unified. *(Ratifies behaviour that already existed but had never been authorised.)* | [Planned] |
| EXP-F12 | When sub-folders are listed recursively ("and all sub-folders"), every listed folder row carries its own recursive total. Rows therefore overlap by design: a listed folder's size is counted again in each of its listed ancestors. The Size column MUST NOT be treated as a partition, and no row total is derived from it. | [Planned] |
| EXP-F13 | The header row that carries the current folder path also shows the total size of the whole catalog, placed before the display options. | [Planned] |
| EXP-F14 | Every figure of `EXP-F10`, `EXP-F11` and `EXP-F13` is computed from the catalog's file records at the moment the listing is produced. None is read from a stored or cached total, so none can disagree with the file records or with each other, and none can be stale. | [Planned] |
| EXP-F15 | The Explore tree's per-row disclosure control follows the shared rule `CDT-F3` (`SpecCardsAndTables.md`). Concretely it adopts the **symbolic** chevrons in place of the filled navigation arrows it used, and keeps everything else it already did: the control kept in place on childless rows so names stay aligned, the shared indent unit, and no tooltip. The four header controls of `EXP-F9` are **unaffected** and stay Explore's alone (`CDT-C5`). Approved by the user on 2026-09-13. | [Planned] |
| EXP-F16 | The K3 Explore file list shows the columns Name, Size, Date, File Type, MIME Type, Checksum (SHA256) and Checksum Date, as K2's Explore file list does. Directory, Catalog Name, Catalog ID, the folders-first key and Path are not shown. Approved 2026-10-02. | [Planned] |
| EXP-F17 | When the open catalog's metadata setting is anything other than none, the list also shows Width (image width, else video width), Height (image height, else video height), Duration (video duration, else audio duration), Artist, Album and Title. When it is none, these columns are not shown. The separate Video Width, Video Height and Audio Duration columns are never shown. Mirrors K2. Approved 2026-10-02. | [Planned] |
| EXP-F19 | The K3 Explore file-row context menu offers the copy entries in this order: "Copy file name without extension", "Copy file name with extension", "Copy folder path", "Copy file absolute path". The wording is K2's. **The order deliberately differs from K2's** (shortest copied text first). Approved 2026-10-03. | [Planned] |
| EXP-F20 | The K3 Explore folder-row context menu offers "Copy folder name" and then "Copy folder path". Approved 2026-10-03. | [Planned] |
| EXP-F21 | Right-click, or press-and-hold, on a folder row of the K3 Explore directory tree opens a context menu with exactly one entry, "Tag this folder", icon `tag` (K2's string and icon). The menu applies to the right-clicked (or held) folder row. Opening the menu or choosing its entry does not change the selected folder of the tree and does not reload the file list. Approved 2026-10-10. | [Implemented] |
| EXP-F22 | Triggering "Tag this folder" opens the Tags page with its folder path field pre-filled with that folder's absolute path. Choosing the tag and confirming happen on the Tags page as usual; nothing is tagged until the user confirms there. Mirrors K2 (fills the Tags folder path, then shows Tags). Approved 2026-10-10. | [Implemented] |
| EXP-F23 | The Tags page opened by `EXP-F22` is shown as an overlay layer covering the Explore page entirely, like Settings, not as a column beside it. If Tags was already open in the page stack, it is removed from there first. Tags' Close, and therefore `Esc` (`KBS-F1` step 1), closes only that layer and returns to the same Explore page, on the same catalog and folder, with the tree and file list unchanged. Tags opened any other way keeps its current behaviour (a page in the stack whose Close returns to Selection). Approved 2026-10-10; *amended 2026-10-10* after the maintainer's test (Tags shown partly over Explore was inconsistent with the rest of the UX). | [Implemented] |
| EXP-F18 | The sort column and order of the Explore file list are restored when the Explore page is reopened, from the same collection-settings keys K2 uses (`Explore/lastExploreSortSection`, `Explore/lastExploreSortOrder`). A given index means the same column in K2 and K3, so a sort chosen in one version is restored as the same sort in the other. "Order folders first" is index 6. Approved 2026-10-02 (resolves the earlier drift of K3 persisting index 4). | [Planned] |
| EXP-F24 | **No control of the file-list header row is ever lost when the window narrows.** "Display folders", "and all sub-folders" and "Order folders first" sit in a toolbar at the right end of the row that shows the current folder path. What does not fit moves into the toolbar's overflow menu with its full text and the same effect; the two tick boxes stay tick boxes while in the row and appear in the overflow menu as checkable entries. The order of the three controls is unchanged. They fold in this order: "Order folders first" first, then "and all sub-folders"; "Display folders" is the last to go. Behaviour is unchanged: "and all sub-folders" is enabled only while "Display folders" is on, turning "Display folders" off also turns "and all sub-folders" off, each change reloads the list, and "Order folders first" keeps its icon, its enable rule and the folders-first sort of `EXP-F18`. The row keeps its height, matching the directory-tree header and the Selection page. The folder path keeps its left elision and takes the width the toolbar leaves. Mirrors `DVP-F27` (`SpecDevicesPage.md`), under `SpecPageLayout.md` PGL-O3. Approved 2026-10-11. | [Planned] |
| EXP-F26 | **The file-list count row wraps rather than clipping.** The row below the folder path shows two groups, "Number of Files" with its count and "Total Size" with its size; each label stays with its value. When both groups fit on one line, the row keeps its height, matching the directory-tree header. When they do not fit, "Total Size" and its size move to a second line and the row grows taller only then. Nothing is elided or hidden. Wrapping follows the available width, with no threshold of our own (`SpecPageLayout.md` PGL-C11); the labels are reused byte for byte, with no string added, changed or removed. Under PGL-O3. *(EXP-F25 is not used: that number was a dropped draft.)* Approved 2026-10-11. | [Planned] |
| EXP-F27 | **Explore follows the selected catalog.** While the Explore page is open in the K3 page stack, selecting a **Catalog** device by activating its card on the Selection page opens that catalog in Explore at once, exactly as its "Explore" action would: same open path, directory tree reset to the catalog root at the `EXP-F2` initial depth, file list of the root. (a) Selecting a **Storage** or **Virtual** device leaves Explore showing what it already shows. (b) Selecting the catalog Explore already shows does nothing. (c) The card's "Explore" context action (`SEL-F3`), which selects and then opens, loads the catalog **once**, not twice. (d) When Explore is not open, selecting a card changes nothing beyond the selection. Applies in File, Hosted and Memory modes; in Memory mode the catalog's files are loaded from its `.idx` file as on any Explore open (`EXP-C9`). Approved 2026-10-13 (beta feedback #8). | [Implemented] |
| EXP-F28 | **The Memory-mode load is reported.** In Memory mode, when opening a catalog in Explore needs its files loaded from its `.idx` file, the activity panel is shown before the load starts (`OPQ-F11`), with its busy indicator, its Stop control and the message `EXPLORE \| In Progress \| Catalog 1 of 1 \| <catalog name> \| Loaded: n of N (p%)`, refreshed as the load advances. When the load completes, the panel shows `EXPLORE \| Completed \| Catalog 1 of 1 \| <catalog name> \| Loaded: N of N (100%)` for the linger of `OPQ-F12`, then hides if nothing else runs. Same shape and strings as K2's Explore load and as Search's loading line. Pause is not offered. Approved 2026-10-13. | [Implemented] |
| EXP-F29 | **No load, no panel.** In Memory mode, when the catalog's files are already in memory and up to date, the open loads nothing and the activity panel neither appears nor flashes. In File and Hosted modes an Explore open loads no file and the panel never appears for it. The busy cursor shown while a catalog opens (as in K2) applies in every mode. Approved 2026-10-13. | [Implemented] |
| EXP-F30 | **Stop.** Stop in the activity panel ends the load. The panel shows `EXPLORE \| Cancelled \| Catalog 1 of 1 \| <catalog name> \| Loaded: n of N (p%)` for the linger of `OPQ-F12`. Explore keeps showing the catalog, folder and file list it showed before the open was requested, or stays empty if it showed none. The stopped catalog is not marked as loaded, so its next open loads it again from the start. Approved 2026-10-13. | [Implemented] |
| EXP-F31 | **The last open wins.** When another catalog is opened in Explore while a load runs — by selecting its card (`EXP-F27`) or by any Explore action — the running load is stopped and the most recently requested catalog is opened once it has stopped. Only the latest request is kept; requests in between are dropped, not run in turn. Requesting the catalog being loaded does nothing. The interrupted load shows no final message of its own; the panel moves straight to the new load. Approved 2026-10-13. | [Implemented] |
| EXP-F32 | **One operation at a time.** While a catalog creation, a device update or a search runs, opening a catalog in Explore is refused — including the selection trigger of `EXP-F27` — and Explore keeps showing what it showed. While an Explore load runs, a creation, an update or a search is refused (`OPQ-F20`). No refusal is silent (`OPQ-O2`): a refused Explore open shows the passive notification `A device operation is already running.` (existing string). Approved 2026-10-13. | [Implemented] |

> **Verification note (2026-08-27).** `EXP-F2` is marked `[Implemented]` on the
> built code: the two-rank initial depth is in place and builds clean. The user
> confirmed the Explore tree working in the running application *before* asking
> for this depth change, so the two-rank state itself has not yet been seen in
> use. Re-run the `EXP-F2` charter step at the next opportunity. Every other row
> was confirmed working in the application.

## Constructional requirements — *how it is built / limits / MUST-NOTs*

Boundaries and implementation constraints, not user-visible behaviour.

| ID | Requirement | Status |
|----|-------------|--------|
| EXP-C1 | Expansion state is held in the QML layer of the Explore folders view (`qt_quick/PageExploreFolders.qml`) as a set of collapsed folder paths, applied over the flat, depth-ordered folder list already provided by `AppManager::getExploreFolders()`. Whether a row has sub-directories is inferred from the depth of the following row. The `EXP-F2` initial depth is a single named constant of that view, so it can be re-tuned in one place; the root row is rank 0 and directories start at rank 1. | [Implemented] |
| EXP-C2 | Expansion state is transient UI state. It MUST NOT be persisted — not to the collection settings file, not to the application settings, not to the database. It is rebuilt from `EXP-F2` each time the Explore view loads a catalog. | [Implemented] |
| EXP-C3 | The expansion and collapse behaviour of `EXP-F1`–`EXP-F9` MUST NOT require a change to `core/`, to the QML adapters, to the database schema, or to the folder data returned to QML. It MUST NOT introduce a new source file. | [Implemented] |
| EXP-C4 | **Accepted divergence.** The Selection page's device tree keeps its collapse state in C++ (`DeviceListModel`), while the Explore directory tree keeps its own in QML. This is a deliberate choice: Explore's expansion is purely visual and carries no domain meaning, so it does not justify a core or adapter change. The two trees behaving differently in code MUST NOT be reported as drift. | [Implemented] |
| EXP-C5 | The header controls MUST reuse the existing translated strings `Collapse one level` and `Expand one level` byte-for-byte rather than creating variants of them. | [Implemented] |
| EXP-C6 | The recursive sizes of one listing come from **one grouped aggregate** over the file records of the open folder's subtree, grouping by folder path and summing file size. Each returned group's total is then distributed in C++, in a single pass over the grouped rows, onto every listed folder that is an ancestor-or-self of that group's path — which is what produces the deliberate overlap of `EXP-F12`. The database MUST NOT be asked for a per-folder total: neither a query per listed folder, nor a correlated scalar subquery, which is one statement but is still evaluated once per row. "A single statement" is not the requirement; **a single aggregate pass** is. | [Planned] |
| EXP-C7 | The subtree restriction MUST be expressed as a range comparison on the folder path — lower bound inclusive, upper bound exclusive — and MUST NOT be expressed with `LIKE`. Only the range form is guaranteed to use `idx_file_catalog_folder`; a prefix `LIKE` depends on collation and on `case_sensitive_like` in SQLite, and behaves differently again on MariaDB. | [Planned] |
| EXP-C8 | These figures MUST NOT add or alter a database column, change the schema, change the `.folders.idx` format, or persist any computed total anywhere. | [Planned] |
| EXP-C9 | In Memory database mode these aggregates depend on the catalog's file records having been loaded into the `file` table by the catalog open that precedes any listing. They MUST NOT be issued before that load, and MUST NOT trigger a load of their own. The failure mode if this is broken is a zero size on every folder, which is indistinguishable from a genuinely empty catalog — a silent wrong answer rather than an error. | [Planned] |
| EXP-C10 | **Accepted divergence.** `EXP-F10`–`EXP-F14` apply to K3 only. They MUST NOT modify `FolderTreeLoader`, the Explore directory tree, or K2's Explore SQL and UI — K2 builds its own file list from separate statements and does not share the K3 query. K2 continuing to show blank folder sizes and a zero tree item count MUST NOT be reported as drift against these rows. | [Planned] |
| EXP-C11 | The Size cell of the file list MUST render the value for folder rows as well as for file rows. The view MUST NOT re-suppress it by entry type; suppressing it there would leave `EXP-F10` unobservable even once the value is supplied. | [Planned] |
| EXP-C12 | An **empty folder path** passed to the core folder-statistics call means "the whole catalog", mirroring the conditional K2 already uses for the same display. `EXP-F13` obtains the catalog total that way. This dual meaning of one argument is a requirement recorded here so that it is documented rather than folklore; the `core/` signature change it implies was approved on 2026-08-29. | [Planned] |
| EXP-C13 | The K3 Explore file model uses the 21-column index layout of the core Search model (`core/search.cpp` headerData, indices 0–20), with header texts reused byte for byte from those `MainWindow`-context strings. No new header string is created without per-string approval. Approved 2026-10-02. | [Planned] |
| EXP-C14 | The extra values come from a widened `Catalog::ExploreFileEntry` / `getExploreEntries` that also selects `image_width`, `image_height`, `video_width`, `video_height`, `audio_duration_seconds` and `checksum_extraction_date`. The widening MUST NOT alter the `EXP-C6`/`EXP-C7` aggregate, the schema (`EXP-C8`) or the Memory-mode ordering (`EXP-C9`). The `core/` change was approved by the user on 2026-10-02. | [Planned] |
| EXP-C15 | A stored sort index that names a column not shown in K3 Explore falls back to the default sort, with no error. This covers a beta1/beta2 prerelease settings file holding the old K3 value 4. Nothing else in existing settings files is affected. Approved 2026-10-02. | [Planned] |
| EXP-C17 | `EXP-F19`/`EXP-F20` reuse K2's existing strings byte for byte. They remove the K3-only "Copy absolute path" and "Copy file name", and add no string. Each copy action copies exactly what it did before. No other menu entry, its visibility or its order changes. Nothing changes in `core/` or K2, and no file is created. Approved 2026-10-03. | [Planned] |
| EXP-C16 | K2 MUST NOT be modified by `EXP-F16`–`EXP-F18`. Approved 2026-10-02. | [Planned] |
| EXP-C18 | `EXP-F21`–`EXP-F23` reuse the existing K2 string "Tag this folder" byte for byte and add no other user-visible string. They pre-fill the existing `folderPathField` of `qt_quick/PageTagsForm.qml`; they change nothing in `core/`, add no settings key and create no new source file. The Close of the Tags layer MUST reuse the existing layer Close handling (`KBS-C1`), not a duplicate of it. Approved 2026-10-10 *(amended 2026-10-10: layer, not stack page)*. | [Implemented] |
| EXP-C19 | **K2 bug fix (maintenance mode).** K2's "Tag this folder" MUST take the folder of the right-clicked tree row, resolved with its parent (the row at the click position, `indexAt(pos)`, with `index.parent()`), as the directory click handler already does. Today `exploreContextTagFolder()` reads `currentIndex()` under the root parent, so a sub-folder fills the Tags folder path with the wrong path. No string, menu or other K2 behaviour changes. Approved 2026-10-10. | [Implemented] |
| EXP-C21 | `EXP-F27` reuses the existing Explore open path (`AppManager::exploreOpenCatalog`, via the Explore page's open-by-device function); it MUST NOT add a second way of opening a catalog. It adds **no** user-visible string, changes nothing in `core/`, persists nothing (no settings key, no database write beyond what the selection itself already persists, `Selection/SelectedDeviceID`), creates no new source file and leaves K2 unchanged. **Accepted divergence:** K2 opens Explore only through its explicit Explore action and its startup restore; K2 not following the selection MUST NOT be reported as drift. K3 has **no** equivalent of K2's Memory-mode confirmation for catalogs over 200,000 files (`qt_widgets/mainwindow_tab_explore.cpp:500`); `EXP-F27` MUST NOT add one — see *Open, not authorised*. Approved 2026-10-13. | [Implemented] |
| EXP-C20 | The folding of `EXP-F24` is decided by Kirigami.ActionToolBar, with no pixel threshold or width breakpoint of our own (`SpecPageLayout.md` PGL-C11). The three labels are reused byte for byte; no user-visible string is added, changed or removed. No change to `core/`, to K2, to any persisted setting or to existing collections. The files table itself and the fixed side-by-side tree / file panels are out of scope (tables are the stated limit of PGL-O3). Approved 2026-10-11. | [Planned] |
| EXP-C22 | The load of `EXP-F28` runs on the GUI thread, pumping events from the existing `Catalog::loadProgress` signal, as Search's Memory-mode loading does (`SearchJobStoppable::searchFilesInCatalog`, progress code -4). It MUST NOT use a worker thread and MUST NOT change `core/`: Stop uses the existing stop flag of `Catalog::loadCatalogFileListToTable`. `Catalog::loadFoldersToTable` is part of the reported load but carries no count and cannot be stopped; that is accepted. Whether a load is needed (`EXP-F29`) is decided before the panel is shown, with core's own test (`dateLoaded < dateUpdated`). Approved 2026-10-13. | [Implemented] |
| EXP-C23 | Every Explore open goes through `AppManager::exploreOpenCatalog`. Its running flag is set synchronously when the open is requested (`OPQ-C13`). An open requested while a load runs — delivered by the pumped event loop — MUST NOT start a second load inside the first one's call stack: it raises the stop flag and records the requested catalog in a single pending slot (a later request overwrites it), which is opened after the running load has returned. Approved 2026-10-13. | [Implemented] |
| EXP-C24 | The catalog Explore reads from (the one used by its folder, entry and size queries) MUST switch to the new catalog only after that catalog's load has completed. A stopped, interrupted or refused open MUST NOT leave Explore reading the partly loaded catalog and MUST NOT empty the page; the page's failure path is for a failed open only. A stopped load leaves the catalog's load date unstamped (existing core behaviour, relied on by `EXP-F30`). Approved 2026-10-13. | [Implemented] |
| EXP-C25 | `EXP-F28`–`EXP-F32` reuse `In Progress`, `Loaded`, `Completed`, `Cancelled`, `Stop` and `A device operation is already running.` byte for byte. The only string added is the operation name `Explore` in the AppManager context, identical to K2's `tr("Explore")` (`qt_widgets/mainwindow_tab_explore.cpp:531`). No settings key, no schema change, no `core/` change, no new source file; K2 unchanged. Nothing changes for existing collections or for beta settings files. An Explore load is not an operation-queue entry (`OPQ-C18`). Approved 2026-10-13. | [Implemented] |

> **Cost note (2026-08-29) — not a requirement.** The per-listing aggregate of
> `EXP-C6` is bounded by the open folder's subtree, but that bound is **not** a
> saving in the case that matters: at the catalog root with "and all sub-folders"
> enabled, the subtree *is* the whole catalog, and that is the first screen a user
> sees after opening one. The cost there equals a full-catalog aggregate — measured
> at 194 ms cold and 27 ms warm over 120k files, using `idx_file_catalog_folder`
> with no temporary B-tree — and it is paid on every navigation rather than once.
> The mechanism was chosen because it holds no cache and therefore has no
> invalidation to get wrong, **not** because it is cheaper than computing the whole
> catalog once. No future reader should assume a bound exists that does not.

---

## Open, not authorised

| Item | Detail |
|------|--------|
| Large-catalog warning in Memory mode (`EXP-F27`) | K2 asks for confirmation before reloading a catalog of more than 200,000 files in Memory mode (`qt_widgets/mainwindow_tab_explore.cpp:500`). K3 has no such warning, and `EXP-F27` makes reloads easier to trigger by a simple selection. The maintainer will test Memory-mode performance himself and decide later whether to port the warning. Not authorised until then. Since 2026-10-13 the Memory-mode load is reported and can be stopped (`EXP-F28`, `EXP-F30`); that may weigh in this decision, but does not authorise the warning. |
| "Follow the selection" on other pages | The same behaviour could apply to other pages, such as Device edit, which would need a rule for unsaved changes. Recorded 2026-10-13 as a possible later extension; not authorised, not to be built now. |
| Loading data into the Explore model and screen (all modes) | Once the catalog's records are in the database, in every mode, Explore builds its directory tree, file list and size figures with no feedback. The maintainer measured 10–20 s on a 1.5M-file catalog in Hosted mode. The cause is not measured (queries, tree building, QML model population, or the `EXP-C6` aggregate) and must be measured before any design. Recorded 2026-10-13; not authorised, not to be built now. |

## Manual test charter

For each row: set up the stated condition, run the operation, confirm the result.

- **EXP-F1** — Open a catalog with a mixed hierarchy. A directory that contains sub-directories shows a disclosure control; a leaf directory shows none. Compare a leaf row and a parent row at the same depth: their height, indentation and the horizontal position of the directory name are identical. Toggle a parent: its descendants disappear, then reappear unchanged.
- **EXP-F2** — Open a catalog at least four ranks deep. On load, the root, its children and its grandchildren are listed; no fourth-rank directory is visible. Confirm this is the intended divergence from K2, whose tree opens one rank less.
- **EXP-F3** — From the initial state, expand two further ranks, then press collapse one level repeatedly. Each press removes exactly the deepest visible rank; collapsing does not stop at the `EXP-F2` initial depth.
- **EXP-F4** — From the initial state, press expand one level. Exactly one further rank appears — the fourth — and branches already open stay open.
- **EXP-F5** — Press collapse all. Only the root row remains, and it is collapsed.
- **EXP-F6** — After collapse all, press expand all. Every directory of the catalog is listed.
- **EXP-F7** — After collapse all, the two collapse controls are disabled and the two expand controls are enabled. After expand all, the reverse. In the initial state of a catalog deeper than the `EXP-F2` initial depth (four ranks or more), all four are enabled; in the initial state of a catalog exactly three ranks deep, nothing is left to expand, so both expand controls are disabled.
- **EXP-F8** — Collapse a branch, then navigate into one of its hidden sub-directories from the file list on the right. The tree opens that branch and the selected row is visible and highlighted; it is never highlighted while off-screen or hidden.
- **EXP-F9 / EXP-C5** — Hover each of the four controls: the tooltips read exactly `Collapse one level`, `Expand one level`, `Collapse all`, `Expand all`. Switch the interface to French: the first two are translated (they already exist), confirming the strings were reused and not duplicated.
- **EXP-C2** — Collapse several branches, leave the Explore screen and come back, or reload the catalog. The tree is back to the `EXP-F2` initial state (root, children and grandchildren open); no collapse state survived.
- **EXP-F10** — Open a folder whose sub-folders hold known content. Each listed folder row shows a size, and that size equals everything beneath that folder, not only the files sitting directly in it. A sub-folder holding no files at any depth shows a zero size, not an empty cell.
- **EXP-F11** — Note the header size of a folder, go up one level and read that same folder's row. The two figures differ whenever the folder has populated sub-folders, and that difference is correct: the header is the direct total, the row is the recursive one. They coincide only when the folder has no populated sub-folders.
- **EXP-F12** — Tick "Display folders" and "and all sub-folders". A nested folder's size appears again inside each of its listed ancestors; the Size column deliberately does not add up to the catalog total.
- **EXP-F13** — The catalog total appears in the path header, before the display options, and matches the recursive figure of the catalog root. It does not change as the user navigates between folders.
- **EXP-F14** — Update the catalog so its content changes, then reopen Explore. Every figure follows the new content; none shows a pre-update value.
- **EXP-F15** — In the Explore tree, confirm the chevrons are the thin disclosure variants, not the filled navigation arrows, in both the collapsed and expanded states, and that they are legible in a dark colour scheme as well as a light one. Confirm the four header controls are untouched.
- **EXP-C9** — Run the whole `EXP-F10`–`EXP-F14` charter again with the collection in Memory database mode. The figures are identical to File mode; no folder shows zero where File mode showed a value.
- **EXP-C10** — Open the same catalog in K2. Folder rows in its file list still show a blank size and its directory tree still shows a zero item count. Both are expected and are not defects of these rows.
- **EXP-C11** — Confirm the size is visible on folder rows, not only on file rows, in the same column and right-aligned like the file sizes.
- **EXP-F16 / EXP-F17** — Open one catalog that includes metadata and one that does not. Compare the visible columns with K2's Explore on the same catalogs: same set, Directory absent, metadata columns only for the first.
- **EXP-F18** — In a release build sharing one settings file, sort by Date descending in K2, then open K3: Explore opens sorted by Date descending. Press "Order folders first" in K3, then open K2: folders come first.
- **EXP-F24 / EXP-C20** — In French and in English, on Breeze and on Fusion, narrow the window step by step. At every width, all three controls are reachable, in the row or in the overflow menu, with their full text; "Order folders first" folds first, then "and all sub-folders", "Display folders" last. In the row the two options are tick boxes; in the menu they are checkable entries. From either place: "and all sub-folders" is disabled while "Display folders" is off, turning "Display folders" off clears it, each change reloads the list, and "Order folders first" puts folders first. The row height still matches the tree header. `git diff` shows no qsTr change and nothing under core/ or qt_widgets/.
- **EXP-F26** — In French and in English, narrow the window to phone width with a folder open. "Total Size" and its size move to a second line under "Number of Files" and its count, each label still beside its value, nothing cut short. Widen the window: both groups return to one line and the row lines up with the tree header again.
- **EXP-C15** — Start K3 with a beta settings file holding `Explore/lastExploreSortSection=4`. Explore opens on the default sort, with no error.
- **EXP-F19 / EXP-C17** — In K3 Explore, right-click a file. The four copy entries appear in the order name without extension, name with extension, folder path, file absolute path, with K2's wording. Paste each to check the copied text. The other entries are unchanged.
- **EXP-F20** — Right-click a folder row. "Copy folder name" comes before "Copy folder path", and each copies the right text.
- **EXP-F21** — In K3 Explore, select folder A in the tree, then right-click a different folder B several levels deep (and, on touch, press-and-hold B). A menu shows only "Tag this folder" with the tag icon. Folder A stays selected and its file list is not reloaded.
- **EXP-F22 / EXP-C18** — Choose "Tag this folder" on B. The Tags page opens with its folder path field holding B's absolute path. Nothing is tagged until a tag is chosen and confirmed. Switch the interface to French: the menu entry is translated (string reused).
- **EXP-F23** — At the default window size, Tags opened by "Tag this folder" covers Explore entirely: no part of the directory tree or file list shows beside it. Press Close: you are back on the same Explore page, same catalog, same selected folder and file list. Repeat with `Esc`. Repeat from an Explore opened by "Explore folder" on Results: Close on Tags returns to Explore, and Close on Explore then returns to Results. Open Tags from the drawer first, then go to Explore and use "Tag this folder": only one Tags is shown, as the layer. Open Tags from the drawer alone: it is a page in the stack and its Close returns to Selection.
- **EXP-C19** — In K2 Explore, right-click a sub-folder at depth two or more (without left-clicking it first) and choose "Tag this folder". The Tags tab shows that sub-folder's full path, not the path of another row.
- **EXP-F27** — In each of File, Hosted and Memory mode: open catalog A in Explore, keep Explore open, then tap catalog B's card on the Selection page. Explore shows B, its tree at the root with the initial two ranks open and the root's file list. Tap a Storage card, then a Virtual card: Explore still shows B. Tap B again: nothing reloads. In Memory mode, B's files are listed (loaded from its `.idx`), not an empty list.
- **EXP-F27 (c)** — With Explore open on A, right-click catalog B's card and choose "Explore": B opens once (one load, no second progress or flicker).
- **EXP-F27 (d) / EXP-C21** — Close Explore, then tap a Catalog card: only the selection changes, Explore is not opened. In K2, selecting a catalog in the Selection panel does not open it in Explore; that is expected. `git diff` shows no qsTr change, nothing under core/ or qt_widgets/, and no new settings key.
- **EXP-F28 / EXP-C22** — In Memory mode, open a catalog of several hundred thousand files in Explore. The panel appears before the load with a turning indicator, Stop, and `EXPLORE | In Progress | Catalog 1 of 1 | <name> | Loaded: n of N (p%)` advancing. On completion the `Completed … (100%)` line stays for the linger, then the panel hides. `git diff` shows nothing under core/.
- **EXP-F29** — Reopen the same catalog unchanged: no panel appears, not even briefly. In File and in Hosted mode, open any catalog: no panel; the busy cursor shows while it opens.
- **EXP-F30 / EXP-C24** — With catalog A shown, open B and press Stop mid-load. The final line reads `Cancelled` and lingers. Explore still shows A, same folder and file list; navigate A's folders, which show A's content. Reopen B: the load starts again from 0. Repeat with Explore empty beforehand: it stays empty.
- **EXP-F31 / EXP-C23** — During B's load, tap C's card, then D's card. B stops, C is never loaded, D loads and is shown. During D's load, tap D again: nothing changes.
- **EXP-F32** — During a search, a device update, then a catalog creation, try to open a catalog in Explore, and separately tap a Catalog card with Explore open. Each time the notification appears and Explore is unchanged. During an Explore load, try Search, an update and a creation: none starts.
- **EXP-C25** — The translation source gains only `Explore` in the AppManager context; no settings key; K2 unchanged.
- **Sorting (supporting check, no requirement)** — Sort the list by Size with folders displayed. Folder rows sort by their value among the files instead of collapsing at one end of the list.
