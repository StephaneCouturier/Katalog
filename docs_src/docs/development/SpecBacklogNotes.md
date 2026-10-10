---
id: SpecBacklogNotes
title: Backlog Notes
description: Collected known issues and edge cases to address in future development
---

# Backlog Notes

![Development](https://img.shields.io/badge/Status-Development-blue)

This page collects known issues and edge cases that were identified during development or testing but deferred for later resolution.

---

## Tags — Symlink path mismatch

**Identified:** 2026-03-15
**Affects:** K2 and K3 (core: `searchjobstoppable.cpp`)

If a tag is created by selecting a folder **via a symlink**, the path stored in the `tag` table is the symlink path (e.g. `/home/user/link-to-music`). However, catalog files are indexed using the **real/resolved path** (e.g. `/media/disk/Music`). The tag path comparison in `searchFilesInCatalog` and `searchFilesInDirectory` uses `QString::contains()` on the literal string, so symlinked tag paths never match real catalog paths.

**Current comparison logic (`searchFilesInCatalog` line ~674):**
```cpp
(filePath + "/").contains(queryTag.value(0).toString() + "/")
```

**Suggested fix:** Resolve the tag path and/or the file path with `QFileInfo::canonicalFilePath()` before comparing. Also consider resolving at tag creation time (store the canonical path in the DB) so existing records remain valid after the symlink is removed.

---

## Source path — no retro-normalization of already-stored paths

**Identified:** 2026-08-12
**Affects:** K2 and K3 (core: `devicejobstoppable.cpp`)
**Related requirements:** `SpecValidationRules.md` N1–N6

This is a **note, not a requirement.** Nothing below is authorised work.

N1 normalizes a path only at the moment it is picked or typed. A device whose
path was stored **with** a trailing separator before N1 existed keeps that stored
value until the user re-picks or re-edits the path; nothing sweeps the existing
`device.device_path` rows (`catalog.catalog_source_path` is removed, `STI-C19`).

Two propagation points bypass the single core rule:

- `core/devicejobstoppable.cpp:351` — `device->catalog->sourcePath = device->path;`
  assigns directly instead of going through `Catalog::setSourcePath()`, so a
  stale stored path is copied verbatim onto the catalog at update time.
- `qt_widgets/` — K2 never normalizes at all (documented as accepted drift by N7),
  so a path entered in K2 can re-introduce a trailing separator that K3 then
  inherits.

**Options if this is ever taken up:** (a) route the assignment through
`Catalog::setSourcePath()`, (b) add a one-shot normalization pass over existing
device/catalog rows, or (c) accept the residual and close it when N7 is closed.
A requirement would have to be approved and written into
`SpecValidationRules.md` first.

---

## Explore — directory tree item count is always zero

**Identified:** 2026-08-29
**Affects:** K2 (visible), K3 (value received and ignored) — core: `foldertreeloader.cpp`
**Related requirements:** `SpecExplore.md` — see `EXP-C10` and the Scope section

This is a **note, not a requirement.** Nothing below is authorised work.

`FolderTreeLoader` queries a table or view named `filesall` at
`core/foldertreeloader.cpp:54`, `:62` and `:136`, and nothing in the repository
ever creates it — there is no `CREATE TABLE` or `CREATE VIEW` for that name
anywhere. Both queries therefore fail silently and `FolderNode::fileCount` stays
at its initialised 0. As a result K2's *No of items* column
(`qt_widgets/exploretreemodel.cpp:39` and `:175`) shows only zeros, and K3
receives the value and ignores it. The count at `:136` is also issued once per
folder node rather than once per tree.

**Deferred deliberately.** K2 is in maintenance mode and no UI investment is
wanted there. Because K2 and K3 share `FolderTreeLoader`, making `fileCount`
correct would change K2's visible column from zeros to real numbers — that is a
K2 UI change, not a K3 feature, and it is out of scope of the Explore size work
approved on 2026-08-29, which deliberately leaves the directory tree untouched.

**Separate finding:** `FolderTreeLoader::loadDirectoryTree`
(`core/foldertreeloader.cpp:54`, `:62`) is the other reader of `filesall`. It
serves `DirectoryTreeModel`, which is compiled into K2
(`qt_widgets/CMakeLists.txt`) but instantiated nowhere. That path is dead code
and reaches no user; it is equally broken and equally deferred.

**Options if this is ever taken up:** (a) replace both `filesall` reads with one
grouped aggregate over `file` and roll the totals up the tree, the shape already
required by `SpecExplore.md` `EXP-C6`, (b) suppress the count column in K2 and
drop `FolderNode::fileCount`, or (c) create the missing `filesall` view. Any of
them needs an approved requirement first, and (a) and (c) both change what K2
displays.

---

## Search history — raw SQL in the K3 UI layer

**Identified:** 2026-08-30
**Affects:** K3 (`qt_quick/appmanager.cpp`)
**Related requirements:** `SpecSelection.md` — see `SEL-C5`

This is a **note, not a requirement.** Nothing below is authorised work, and the
maintainer has not ruled on it.

`AppManager::getSearchHistory()` (`qt_quick/appmanager.cpp:1357`) builds and
executes a `QSqlQuery` directly against the `search` table and formats the
result into the history summary. The UI/core boundary rule is that `qt_quick/`
files must not contain raw `QSqlQuery` and must delegate to a core method; this
one does not. It predates the Selection work and was found while judging it.

`SpecSelection.md` `SEL-C5` forbids **deepening** the departure — the device
name required by `SEL-F2` must come from an existing device accessor, not from a
new `device` join added to this query — but `SEL-C5` does not authorise moving
the existing query, and this note does not either.

**Options if this is ever taken up:** (a) move the query and the summary's data
retrieval behind a core method on `Search` or `Collection`, leaving only the
`tr()` fragments and the joining in `AppManager`, or (b) accept it as recorded
drift. Either way a requirement has to be approved and written first, and (a)
touches `core/`, which needs its own prior approval.

---

## Selection — *Open folder* context item is always visible in K3

**Identified:** 2026-08-30
**Affects:** K3 (`qt_quick/PageSelectionDelegate.qml`)
**Related requirements:** `SpecSelection.md` — Scope section (the context menu's
item set is not governed by that spec)

This is a **note, not a requirement.** Nothing below is authorised work, and the
maintainer has not ruled on it.

The *Open folder* item of the Selection card context menu carries no `visible:`
guard, so it is offered for every device of every type and whether or not a
source path exists. The user page `Selection.md` documents it under **Catalog
devices** only, and as "shown only when a source path is defined". K2 matches the
documentation: `qt_widgets/mainwindow_tab_filters.cpp:317-323` builds the action
inside the Catalog branch only, and only when `selectedDevice->path` is not
empty. K2 also uses the icon `document-open-folder` where K3 uses
`document-open`.

The code is therefore doing something no requirement authorises and the user
documentation contradicts. It was found while judging the Selection work of
2026-08-30 and is outside that work's scope.

**Options if this is ever taken up:** (a) add the two guards so K3 matches
`Selection.md` and K2, (b) approve a requirement that K3 deliberately offers the
item for every device, and correct `Selection.md` and its translations to match,
or (c) leave it recorded. The behaviour must not be enshrined as a requirement
merely because it exists.

---

## Backup — K3 run progress does not use the standard message builder

**Identified:** 2026-08-31
**Affects:** K3 (`qt_quick/PageBackupForm.qml`)
**Related requirements:** `SpecBackup.md` — see `BKP-F9`

This is a **note, not a requirement.** Nothing below is authorised work, and the
maintainer has not ruled on it.

`BKP-F9` is marked `[Implemented]` and requires that backup progress and outcome
be "reported via the standard status-bar message builder". `SpecBackup.md`
applies to K2 and K3 alike. On the K3 Backup page the running mapping's progress
is a `ProgressBar` plus a label assembled in QML from raw numbers
(`PageBackupForm.qml:445-473`: percent, files done over total, speed, ETA,
current file), not a `StatusBarMessageBuilder` message. `BKP-F9` may well be
satisfied on the K2 side; only the K3 surface was examined.

Found while judging the activity-panel work of 2026-08-31, and outside that
work's scope. Note that `SpecOperationQueue.md` `OPQ-C16` is deliberately scoped
to creation, update and search, so it does not reach this screen either.

**Options if this is ever taken up:** (a) build the K3 backup progress text with
the standard builder, keeping the progress bar as an addition rather than a
replacement, (b) qualify `BKP-F9` per version if the builder is genuinely not the
right vehicle for a byte-and-ETA progress bar, or (c) leave it recorded.

---

## Activity panel — duplicated five-second delays

**Identified:** 2026-08-31 (revised the same day, after the search timer was removed)
**Affects:** K3 (`qt_quick/OperationQueueView.qml`, `qt_quick/appmanager.cpp`)
**Related requirements:** `SpecOperationQueue.md` — see `OPQ-C14`, `OPQ-F15`

This is a **note, not a requirement.** Nothing below is authorised work, and the
maintainer has not ruled on it.

`OPQ-C14` requires the panel's linger to be a single named constant defined in one
place. It is `[Planned]` and is **not** satisfied today. Two hard-coded `5000`
values remain:

- `OperationQueueView.qml` — the panel's own `completionLinger`;
- `AppManager::startNextDeviceUpdate()` — a `QTimer::singleShot(5000, ...)` that
  clears `m_deviceUpdateStatusText` when the queue empties.

The two race on the same message for the device-update path, which is the exact
situation `OPQ-C14` exists to prevent. The equivalent timer on the search path was
removed when `OPQ-F15` was implemented, so search is now clean; the update path is
not. Satisfying `OPQ-C14` means the update timer goes the same way and the constant
gets a single definition.

---

## Keyboard shortcuts — Ctrl+F always targets the Selection filter

**Related requirements:** `SpecKeyboardShortcuts.md` (out of scope there)

This is a **note, not a requirement.** Nothing below is authorised work, and the
maintainer has not ruled on it.

`Ctrl+F` focuses the Selection page's device filter on every page. The filter is a
Kirigami `ActionTextField` with `focusSequence: StandardKey.Find` (`qt_quick/Main.qml`,
the `deviceSearchField`), so the binding is window-wide. The open question is whether
`Ctrl+F` should focus the Search page's own field while Search is open. To be reviewed
later.

---

## Collection import — no report after *Import* (K3)

**Identified:** 2026-10-04 (K3 Settings user page)
**Affects:** K3 (`qt_quick/PageSettings.qml`, `AppManager::importDevice`, `AppManager::updateAllImportsFromSource`)
**Related requirements:** `SpecCollection.md` (#780 update report)

This is a **note, not a requirement.** Nothing below is authorised work. The maintainer
asked on 2026-10-04 to log it as a gap to close.

In K3, *Collection Import & Synchronization* shows a summary message (catalogs updated,
files and size before/after) only after **Update** (`importReportText` is set in
`updateAllImportsFromSource` only). After **Import**, the user gets the progress-panel
status but no summary of what was added.

This is not a K2 parity gap: K2 shows neither summary, only a status-bar message for
both actions. The K3 update report is itself a K3 addition (#780).

**To close it:** show, after an Import, a summary of what was added (for example
devices, catalogs, files and total size imported), in the same message area as the
update report. Needs an approved requirement and its new strings.

---

## Device tree — self-parented default storage

**Identified:** 2026-10-04
**Affects:** K2 and K3 (core: `Collection::insertPhysicalStorageGroup`, `Device::loadSubDeviceList`)
**Related requirements:** `SpecCollectionOpen.md` OPN-O4, OPN-F8, OPN-C7

This is a **note, not a requirement.** Nothing below is authorised work.

Before the OPN-F8 fix, opening a collection with device 1 present, device 2 absent
and no Storage device created a default "Local disk" that was its own parent
(device 2, parent 2). The recursive query in `Device::loadSubDeviceList` (`UNION ALL`
over `device_parent_id`) never ends on that cycle, so every open of such a collection
hangs at start-up. The fix prevents new cases; a collection already damaged keeps the
row and still hangs. The Quality check cannot help: the collection hangs before it can
run, and check 8 (missing parent) does not see a parent that exists.

**Options, each needing its own approved requirement:**
- A Quality check for self-parented or cyclic `device_parent_id`.
- Make `loadSubDeviceList` robust to cycles (`UNION` instead of `UNION ALL`, or a depth guard), so a damaged collection at least opens.
- A manual repair procedure (set the row's `device_parent_id` to 1).

---

## Duplicated data — further candidates, not decided

**Status:** candidates only, recorded 2026-09-28 from the redundancy diagnosis
that led to `STI-C16`, `STI-C17`, `STI-C19` and `STI-C20` (`SpecStorageIdentity.md`).
Nothing below is authorised work; each needs its own user decision.

- **`search.search_location`** — *Done (user decision 2026-09-29):* removed by
  `Database::runMigration_3_0`. It was never given a value; reads keep its index
  as `'' AS search_location`, and Memory-mode `search_history.csv` keeps slot 21,
  written empty.
- **`search.search_storage` / `search.search_catalog`** — *Done (user decision
  2026-09-29):* removed by `Database::runMigration_3_0`; the scope is
  `selected_device_ID_list`. Reads keep their indexes as `''` placeholders, and
  Memory-mode `search_history.csv` keeps slots 22–23, written empty. K2's history
  restore derives its Storage/Catalog labels from the first id in
  `selected_device_ID_list`, by the same rule as `displaySelectedDeviceName()`.
  K2's `migrateExistingSearchDeviceData_2_6` is removed: a collection older than
  2.6 must be opened with 2.12 first (`SpecVersions.md`).
- **`file.file_full_path`** — equals `file_folder_path + "/" + file_name`.
- **`catalog.catalog_file_path`** — derivable in Memory mode.

---

## K3 user-doc pass (3.0) — K3 features not yet in the user documentation

**Related requirements:** see each entry

The English user pages in `docs_src/docs/` describe K3 and are kept accurate with every
user-facing K3 change, in the same piece of work (maintainer rule, 2026-10-10). No K3
feature waits here for its English documentation. The three entries listed earlier (Esc
goes back one step, Selection device filter, Quality check) are documented in
`Interface.md`, `Selection.md` and `Settings.md`.

The French and Czech pages (`i18n/fr`, `i18n/cs`) are translated later, in the #803
translation phase; until then some of them still hold the K2 text.

---

## Progress report: catalog name in theme colour (K3)

**Identified:** 2026-09-30
**Affects:** K3 (progress/status messages built by `StatusBarMessageBuilder`)
**Related requirements:** `SpecProgressReport.md` — Component Breakdown, Part 3 (Device Context)

This is a **note, not a requirement.** Nothing below is authorised work.

The catalog name in the Device Context part ("Catalog X of Y | CatalogName") is
specified as **bold, no colour** in K2 and K3. It was previously #39b2e5 blue,
but K3 renders these messages with Qt Quick `Text.StyledText`, which ignores
`<span style='color:…'>`, so the blue never showed in K3. The preferred colour
is the theme's `Kirigami.Theme.linkColor` (same colour as the K3 Settings section
titles), but `core/` is UI-agnostic and K2 has no Kirigami, so a theme colour
would need a core-level colour-injection mechanism, which the user declined.

**When to take it up:** once K2 is discontinued and K3 can implement it without
the K2/K3-agnostic core constraint, display the catalog name in
`Kirigami.Theme.linkColor`.

**Technical note:** `Text.StyledText` needs `<font color="…">` rather than
`<span style="color:…">` (span colour is ignored). Switching to `Text.RichText`
would render the span but loses eliding.

---

## Collection export — no failure reason for the CSV export

**Identified:** 2026-10-10 (`SpecCollectionExport.md`)
**Affects:** K3 (and K2) — core: `Collection::exportAllToMemoryMode`, `Collection::exportAllCatalogFiles`

When a Memory-mode (CSV) export fails, K3 reports only status "Error" (CEX-F11), with no reason.
`exportAllToMemoryMode` always returns `true`, and `exportAllCatalogFiles` silently skips (`continue`) a catalog
whose `.idx` file cannot be opened or whose query fails. A user can end up with an incomplete export and no message.

**When to take it up:** when a reason is wanted. It needs a `core/` change (approval) to return or expose the
failure, plus new user-visible strings (per-string approval).

---

## Collection export — folders without files missing after an earlier load

**Identified:** 2026-10-10 (`SpecCollectionExport.md`, CEX-F5 known limitation)
**Affects:** K3, Memory mode, Memory → SQLite export. Core: `Catalog::loadFoldersToTable()`, `Collection::loadAllCatalogFiles()`

The K3 export calls `loadFoldersToTable()` for every catalog, then `loadAllCatalogFiles()`. If a catalog's file
list was loaded earlier in the session (for example by a search), that load created only folders that hold
files. `loadFoldersToTable()` then skips the catalog (folder rows present, load date current), so its folders
without files are missing from the exported SQLite file. An export in a fresh session is complete.

**Same mechanism:** `loadCatalogForRootReplace` in `core/device.cpp` (near line 1250), which clears the
catalog's folder rows before calling `loadFoldersToTable()`.

**Fix:** a `core/` method that clears and reloads a catalog's folders, used by the export (and possibly by
`loadCatalogForRootReplace`). It needs core approval, which was not given on 2026-10-10.

---
