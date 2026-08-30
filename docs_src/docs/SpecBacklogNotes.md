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
`device.device_path` / `catalog.catalog_source_path` rows.

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
