---
id: SpecCollectionOpen
title: Collection Open — First Run and New Database
description: Requirements for choosing, creating and opening a collection database on first run and from Settings
---

# COLLECTION OPEN — FIRST RUN AND NEW DATABASE

![Status](https://img.shields.io/badge/Status-Approved-brightgreen) ![Implementation](https://img.shields.io/badge/Implementation-complete-brightgreen) ![Verification](https://img.shields.io/badge/Retest-pending-orange)

## Context

Before Katalog can do anything it needs a collection: a database file in **File**
mode, or a folder of CSV and index files in **Memory** mode. The user chooses one
on first run, and can create another later from Settings. Three separate pieces
of code create a collection database — the K2 first-run flow, the K2 Settings
*New* action, and the K3 create-new-collection action — and they had drifted
apart from each other. That drift is what this spec exists to close.

The defect that prompted it was reported by an end user on their very first
launch: they asked to create a new database, picked a folder, gave a file name,
and Katalog answered that the file did not exist and asked again — with no way
out of the dialog. They escaped only by selecting an unrelated database that
happened to be on their disk, then went to Settings and created a collection
there, which worked.

> **The root cause was the database *mode*, not the missing folder.**
> The first-run flow wrote the collection *path* but never wrote the database
> *mode*. `Settings/databaseMode` was written in exactly one place — the Settings
> tab (`qt_widgets/mainwindow_tab_settings.cpp`) — so on a fresh install the key
> stayed unset. `Database::initialize` reloads `databaseMode` from the settings
> file (`core/database.cpp`), found it empty, matched none of `Memory` / `File` /
> `Hosted`, left the connection object default-constructed, and failed to open it
> however valid the chosen path was. That is also why visiting Settings appeared
> to cure it: Settings was the only code that set the mode.
>
> A future reader must not credit the wrong fix. Creating the parent directory
> (`OPN-F1`) and reporting write failures (`OPN-F2`) are real defects and are
> fixed here, but on their own they would **not** have resolved the report.
> `OPN-F5` is the one that did.

This spec covers choosing, creating and opening a collection. It does **not**
cover importing or updating across collections — that is `SpecCollection.md`.

> **Reading the requirement IDs.** Each requirement has a permanent ID.
> IDs are never renumbered or reused; a retired requirement is marked
> `[Removed]`, not deleted. Status is one of
> `[Implemented] / [Planned] / [Backlog] / [Removed]`.

> **On the status of the rows below.** Every row is marked `[Implemented]`: the
> code is written and builds. This project's status vocabulary has no separate
> *verified* value, so `[Implemented]` means built, not retested. The maintainer
> had not yet re-run the manual test charter when this page was written — the
> charter at the foot of this page is the instrument for that, and the *Retest*
> shield above stays orange until it has been walked through.

---

## Scope at a glance

**In scope:** the first-run collection dialog in File and Memory mode, cancelling
it, creating a new database file from Settings, creating a new SQLite collection
in K3, creating the containing folder, the `.db` extension, reporting creation
failures, and recording the database mode alongside the path.

**Out of scope (non-goals):** any redesign of the first-run sequence — K2 is in
maintenance mode and this is repair of the existing flow only (`OPN-C4`).
Hosted-mode configuration, collection import/update (`SpecCollection.md`), and
the contents of a collection once opened.

---

## Operational requirements — *why / for whom*

Goals in real use, independent of how they are built.

| ID | Requirement | Status |
|----|-------------|--------|
| OPN-O1 | On first run a user either reaches a working collection or leaves the dialog without being trapped, in every database mode. | [Implemented] |
| OPN-O2 | Creating a new collection database from Settings gives the same result as creating one on first run. | [Implemented] |

## Functional requirements — *what the system does*

Observable behaviour that can be triggered and watched.

| ID | Requirement | Status |
|----|-------------|--------|
| OPN-F1 | When a new collection database is created, its containing folder is created if absent. A folder typed into the dialog rather than picked from it is not an error: the save dialog returns a path without creating anything, so the folder may legitimately not exist yet. | [Implemented] |
| OPN-F2 | When the database file cannot be created, the reason is reported to the user and the flow stops. The application MUST NOT go on to open a file it did not create — doing so produced the misleading *"Database file not found"* the reporter saw for a file Katalog itself had failed to write. | [Implemented] |
| OPN-F3 | Cancelling the first-run collection dialog **ends** it. The application starts with no collection configured, and the collection is then chosen from Settings. It MUST NOT re-prompt. This applies to the File-mode file dialog **and** the Memory-mode folder dialog. On cancel the corresponding path is cleared, so the "not configured" state is unambiguous and the next launch offers first run again. | [Implemented] |
| OPN-F4 | The *"Ready to create a file catalog"* guidance appears only when a collection was actually created — never after a cancel, in either mode. | [Implemented] |
| OPN-F5 | The first-run flow records the chosen database **mode** together with the chosen path, and flushes both to the settings file before reconnecting, so the connection attempted immediately afterwards and the one made on the next launch both use that mode. `"File"` in the file branch, `"Memory"` in the folder branch. **This is the fix for the reported defect** — see Context. | [Implemented] |
| OPN-F6 | A database file created from Settings receives the same `.db` extension handling as the first-run path: the extension is appended when the user did not type it. | [Implemented] |

## Constructional requirements — *how it is built / limits / MUST-NOTs*

Boundaries and implementation constraints, not user-visible behaviour.

| ID | Requirement | Status |
|----|-------------|--------|
| OPN-C1 | The three creation paths — K2 first run (`qt_widgets/mainwindow_setup.cpp`), K2 Settings *New* (`qt_widgets/mainwindow_tab_settings.cpp`), and K3 create-new-collection (`qt_quick/appmanager.cpp`) — MUST agree on folder creation, extension handling and failure reporting. A change to one is a change to all three. Divergence between them is the defect this spec records, not an acceptable variation. | [Implemented] |
| OPN-C2 | These fixes introduce **no** new user-visible string. K2 reuses the existing `MainWindow` string `Failed to open the database file: %1` in both its creation paths; K3 keeps its existing `Could not create file: %1`. | [Implemented] |
| OPN-C3 | An empty `databaseFilePath` MUST remain a legal *"not configured yet"* state that `Database::initialize` returns cleanly from, with no error. `OPN-F3` depends on it: cancelling leaves the path empty and the application must still start. It MUST NOT be turned into an error condition. | [Implemented] |
| OPN-C4 | K2 is in maintenance mode. This is repair of the existing flow only: no redesign of the first-run sequence, no new dialog, no new source file. | [Implemented] |
| OPN-C5 | The `.db` extension of `OPN-F6` is **required for correctness, not cosmetic**. A File-mode collection is located by scanning a folder for `*.db`: `Collection::validateCollectionFolder` (`core/collection.cpp`) and the command-line collection resolver (`core/commandline.cpp`) both do this. An extensionless database file is therefore classified as user data rather than a collection, and `--collection <dir>` reports that no `.db` file was found. The extension MUST NOT be treated as a naming preference that a creation path may skip. | [Implemented] |

---

## Known defects not closed here

| Item | Detail |
|------|--------|
| Dead `ACTION_CANCEL` case | In the Memory-mode invalid-folder dialog, the `ACTION_CANCEL` branch breaks out of its `switch` and falls back into the surrounding loop, so it does not cancel anything. The first-run trap itself is gone — the outer cancel of `OPN-F3` now ends the dialog — so this case is misleading rather than harmful. It is **not** fixed and must not be recorded as such. Closing it means routing that case to the same exit as `OPN-F3`. |

---

## Manual test charter

Each line below is a case that must hold after any change to the first-run flow
or to either *New database* action. Cases marked (K2) apply to the Qt Widgets
version, (K3) to the Qt Quick version.

- **OPN-F1** (K2, K3) — In the create dialog, type a path whose parent folder
  does not exist. The database is created and opens; no *"file not found"*.
- **OPN-F2** (K2, K3) — Point the dialog at a read-only folder. The failure is
  reported with its reason, and no connection is attempted against the file that
  was never written.
- **OPN-F3** (K2) — Delete the settings file. Start, and at the File-mode file
  dialog press Cancel. The dialog closes once and does not reappear. The
  application starts, and a collection can then be created from Settings. Repeat
  the same in Memory mode at the folder dialog.
- **OPN-F3** (K2) — After that cancel, restart. First run is offered again
  rather than the application starting against an unconfigured collection.
- **OPN-F4** (K2) — After the `OPN-F3` cancel, the *"Ready to create a file
  catalog"* box does not appear. After a successful creation, it does.
- **OPN-F5** (K2) — Delete the settings file, start, and create a database. It
  opens immediately, with no error. Inspect the settings file: `databaseMode` is
  set. Relaunch: the same collection reopens without first run being offered.
  This is the regression test for the reported defect; run it in Memory mode too,
  where `databaseMode` must be `Memory`.
- **OPN-F6 / OPN-C5** (K2) — Settings → *New*, and enter a name with no
  extension. The file is created as `<name>.db`. The folder then validates as a
  File-mode collection, and launching with `--collection <that folder>` finds it.
- **OPN-C1** (K2, K3) — Walk `OPN-F1` and `OPN-F2` through all three creation
  paths, including K3's create-new-collection. The behaviour is identical.
- **OPN-C2** (K2, K3) — No new entry appears in the translation files for these
  paths; the failure message is the existing one.
- **Dead `ACTION_CANCEL`** (K2) — Known open item, expected to fail: in Memory
  mode select a folder that fails validation, then press Cancel in the
  invalid-folder dialog. It returns to the folder dialog instead of ending. That
  is the documented state, not a regression.
