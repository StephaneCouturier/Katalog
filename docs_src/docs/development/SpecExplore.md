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
(`EXP-F16`–`EXP-F18`, added 2026-10-02).

**Out of scope (non-goals):** everything else on the Explore screen — loading the
catalog, the file list beyond the size figures and column set named above, the
display options, the context menus, sorting other than its persistence, and the Selection page's device tree. None of those
are governed by this spec; this spec does not authorise changing any of them.
Persisting expansion state is explicitly excluded — see `EXP-C2`. The directory
tree rows carry no aggregate figures at all: the always-zero item count that the
tree computes today is a known defect, deliberately deferred and recorded in
`SpecBacklogNotes.md`, not a requirement of this spec.

**Applies to:** K3 (`qt_quick`) only. K2's Explore tree already provides
`EXP-F1` through its tree widget, and an initial expansion of its own that
`EXP-F2` deliberately no longer matches; the header controls of
`EXP-F3`–`EXP-F7` are not required in K2, which is in maintenance mode.

---

## Operational requirements — *why / for whom*

Goals in real use, independent of how they are built.

| ID | Requirement | Status |
|----|-------------|--------|
| EXP-O1 | A user browsing a catalog opens only the branches of interest and keeps the rest of the hierarchy folded away, so a deep or wide directory tree stays readable. | [Implemented] |
| EXP-O2 | A user changes how much of the hierarchy is shown in one action — one rank at a time, or the whole tree at once — instead of clicking through every branch. | [Implemented] |
| EXP-O3 | A user browsing a catalog while the device is disconnected sees how much space each sub-folder holds, and how large the catalog is in total, so the largest space consumers can be identified offline. | [Planned] |
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
| EXP-F18 | The sort column and order of the Explore file list are restored when the Explore page is reopened, from the same collection-settings keys K2 uses (`Explore/lastExploreSortSection`, `Explore/lastExploreSortOrder`). A given index means the same column in K2 and K3, so a sort chosen in one version is restored as the same sort in the other. "Order folders first" is index 6. Approved 2026-10-02 (resolves the earlier drift of K3 persisting index 4). | [Planned] |

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
| EXP-C16 | K2 MUST NOT be modified by `EXP-F16`–`EXP-F18`. Approved 2026-10-02. | [Planned] |

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
- **EXP-C15** — Start K3 with a beta settings file holding `Explore/lastExploreSortSection=4`. Explore opens on the default sort, with no error.
- **Sorting (supporting check, no requirement)** — Sort the list by Size with folders displayed. Folder rows sort by their value among the files instead of collapsing at one end of the list.
