---
id: SpecCollectionExport
title: Collection Export — Convert a Collection to Another Mode
description: K3 export of the open collection to a SQLite file or to a Memory-mode CSV folder
---

# COLLECTION EXPORT — CONVERT A COLLECTION TO ANOTHER MODE

![Status](https://img.shields.io/badge/Status-Approved-brightgreen) ![Implementation](https://img.shields.io/badge/Implementation-planned-lightgrey) ![K2](https://img.shields.io/badge/K2-no%20change-lightgrey)

## Context

K2 can export the open collection to another storage mode from its Settings tab:
to a **SQLite file** (File mode) or to a **folder of CSV and `.idx` files**
(Memory mode). K3 has no export. This page specifies the K3 port, listed in
`SpecK3ParityGaps.md` ("Settings — Export a collection to another mode").

The K2 behaviour is the reference for the port. It is not a requirement in
itself. This page adopts it with these deliberate differences:

- K3 also exports folder data from a Memory collection (K2 defect, CEX-F5).
- The CSV export also writes the catalog filters. This is a fix in `core/`,
  so K2 gets it too (CEX-C1).
- The CSV export requires an empty target folder (CEX-F13).
- K3 has no tooltip on the export buttons (CEX-F2).
- K3 reports progress and results through the status bar and a passive
  notification instead of modal dialogs (CEX-F9 to CEX-F11).

Ratified by the maintainer on 2026-10-10 (relayed by the main agent).

| Current mode | Export to SQLite file | Export to Memory mode (csv) |
|---|:---:|:---:|
| Memory | yes | — |
| File | — | yes |
| Hosted | yes | yes |

## Scope at a glance

| In scope | Not in scope |
|----------|--------------|
| K3: the export actions on the Settings › Collection tab, the target picker, the export itself, progress, cancel, the result and the busy-state rule. | K2 (unchanged). Opening the exported collection. Exporting from Memory to Memory or from File to File. Any `core/` change other than the one in CEX-C1. Reporting a failure reason for the CSV export (`SpecBacklogNotes.md`). Import / Update across collections (`SpecCollection.md`). |

---

## Operational requirements — *why / for whom*

| ID | Requirement | Status |
|----|-------------|--------|
| CEX-O1 | A user can convert the open collection to another storage mode (SQLite file, or Memory-mode CSV folder) and keep all its content: devices, catalogs, files, folders, tags, device mappings, catalog filters, search history and statistics. | [Planned] |

## Functional requirements — *what the system does*

| ID | Requirement | Status |
|----|-------------|--------|
| CEX-F1 | The Settings › Collection tab, in the "Collection & Database" section, shows an "Export to SQLite file" button when the open collection is in Memory or Hosted mode, and an "Export to Memory mode (csv)" button when it is in File or Hosted mode. Both buttons use the icon `document-export`. | [Planned] |
| CEX-F2 | The labels are the K2 strings, copied byte for byte: "Export to SQLite file" and "Export to Memory mode (csv)". The buttons have no tooltip. *(K2's Memory-mode tooltip `Export to convert and open the collection in "File" mode.` is not ported; it says the collection is opened, which neither K2 nor K3 does. K2 is unchanged.)* | [Planned] |
| CEX-F3 | "Export to SQLite file" opens a save-file dialog titled "Select", proposing `<collection folder>/export.db`. Cancelling the dialog does nothing. | [Planned] |
| CEX-F4 | "Export to Memory mode (csv)" opens a folder dialog titled "Select a folder", starting at the collection folder. Cancelling the dialog does nothing. | [Planned] |
| CEX-F5 | Before a Memory → SQLite export, every catalog's files and folders are loaded into the in-memory database, so the SQLite file holds the `file` and `folder` data of every catalog. *(Deliberate fix: K2 does not load folders and writes an empty `folder` table. K2 is not changed.)* | [Planned] |
| CEX-F6 | The SQLite export writes a new SQLite file holding every table of the collection, built with the current schema. | [Planned] |
| CEX-F7 | The Memory-mode export writes the collection CSV table files, plus one `<device name>.idx` and one `<device name>.folders.idx` per catalog, into the chosen folder. | [Planned] |
| CEX-F8 | If the chosen SQLite target file already exists, it is moved to the trash before anything is written, with no confirmation beyond the save dialog's own overwrite prompt. If the move to the trash fails, the export stops with the Error result (CEX-F11) and the existing file is left as it was. It is never overwritten or deleted. | [Planned] |
| CEX-F9 | While the export runs, the status bar shows its progress in the format of `SpecProgressReport.md`, with operation "Export" and status "In Progress". During the Memory-mode loading phase it shows the process "Loaded" with its count n of total. During the copy phase it shows no process title and gives the name of the table or catalog being written as the current item. Because the builder shows a count only with a process title, the copy phase shows no count. | [Planned] |
| CEX-F10 | The export can be cancelled. On cancel, the partial output is removed (within the limit of CEX-C5) and the status bar reports status "Cancelled". | [Planned] |
| CEX-F11 | On success, a passive notification reports the export ("Export") and the target path, and the status bar reports status "Completed". On failure, the status bar reports operation "Export" and status "Error", using the same pattern as K3 Collection Import. A reason is shown only where this page approves a reason string (CEX-F13); otherwise only "Error" is shown. No modal dialog is shown for the result. | [Planned] |
| CEX-F12 | While a catalog create, a device update or a search is running, the export buttons are disabled and an export request is refused. | [Planned] |
| CEX-F13 | The Memory-mode export requires the chosen folder to be empty. If the folder holds anything (files or subfolders, from a collection or not), the export is refused before anything is written: the status bar shows operation "Export", status "Error" and the reason "The export folder must be empty.". No overwrite confirmation is offered. | [Planned] |

## Constructional requirements — *how it is built / limits / MUST-NOTs*

| ID | Requirement | Status |
|----|-------------|--------|
| CEX-C1 | The only `core/` change allowed is one call to `saveCatalogFilterTableToFile()` added to `Collection::exportAllToMemoryMode`, so the CSV export also writes `catalog_filter.csv`. *(Approved by the maintainer on 2026-10-10. It fixes K2 and K3.)* No other new or changed `core/` method. The export uses only `Collection::loadAllCatalogFiles`, `Catalog::loadFoldersToTable`, `Collection::exportToSQLiteFile`, `Collection::exportAllToMemoryMode` and `Collection::exportAllCatalogFiles`. No `QSqlQuery` in `qt_quick/`. | [Planned] |
| CEX-C2 | Memory → SQLite uses `Collection::exportToSQLiteFile` on the in-memory connection, after CEX-F5. K2's `MainWindow::backupMemoryDatabaseToFile` (K2 UI layer) is not ported. | [Planned] |
| CEX-C3 | The user-visible strings are exactly these, approved byte for byte on 2026-10-10: "Export to SQLite file", "Export to Memory mode (csv)", "Select", "Select a folder", "Export", "Error", "Loading", "Cancel" (reused from K2), and "The export folder must be empty." (new). Status values come from `SpecProgressReport.md`. K2 messages that are not wrapped in `tr()` (for example "Dumping Tables...", "Database Error") are not ported, and neither is the K2 tooltip. | [Planned] |
| CEX-C4 | Progress, cancel and error messages are built with `StatusBarMessageBuilder`. No raw status string. | [Planned] |
| CEX-C5 | The cancel cleanup for the Memory-mode export removes **only** files that this export run created. It MUST NOT remove a file that was in the chosen folder before the run. Because of CEX-F13, the folder is empty when the run starts. | [Planned] |
| CEX-C6 | The export does not change the open collection: same mode, same connection, same collection folder, and no change to the collection or application `.ini`. Beta1/beta2 `katalog3_prerelease_settings.ini` files are unaffected. | [Planned] |
| CEX-C7 | K2's Settings tab and its export code are not changed. K2 gains only the `catalog_filter.csv` fix through the shared core (CEX-C1). | [Planned] |

---

## Known limitation

**CEX-F5 — folders without files can be missing after an earlier load (accepted by the maintainer on 2026-10-10).**
In Memory mode, the K3 export loads each catalog's folders with `Catalog::loadFoldersToTable()`, then the files
with `Collection::loadAllCatalogFiles()`. Suppose a catalog's file list was already loaded earlier in the
session, for example by a search. That load recreated only the folders that hold files, and
`loadFoldersToTable()` then skips the catalog because its folder rows are present and their load date is current.
The folders of that catalog that hold no files are then missing from the exported SQLite file. An export run in
a fresh session is complete. This is the same mechanism as `loadCatalogForRootReplace` (`core/device.cpp`, near
line 1250). The fix needs a `core/` method that clears and reloads a catalog's folders, which CEX-C1 does not
allow. It is backlogged in `SpecBacklogNotes.md`, "Collection export — folders without files missing after an
earlier load".

---

## Observed behaviour of the existing core functions (for reference, not requirements)

Read on 2026-10-10. These notes describe the code before the port.

- **Same-named files are overwritten without warning.** `Collection::exportAllToMemoryMode` points the
  collection paths at the chosen folder, then calls the `save*TableToFile` functions. Each opens its file with
  `WriteOnly` (some also `Truncate`): `parameters.csv`, `device.csv`, `storage.csv`, `tags.csv`,
  `device_mapping.csv`, `search_history.csv`, `statistics.csv` are truncated and rewritten.
  `generateCollectionFiles` only creates the files that are missing. `exportAllCatalogFiles` opens
  `<device name>.idx` and `<device name>.folders.idx` with `WriteOnly`, so these are overwritten as well.
  Unrelated files in the folder are left alone. K2 shows no warning. K3 avoids this by requiring an empty folder (CEX-F13).
- **`catalog_filter.csv` is not written.** `exportAllToMemoryMode` does not call `saveCatalogFilterTableToFile`,
  so the catalog include/exclude filters are missing from the CSV export. Fixed by the core change approved in CEX-C1.
- **Errors are not reported.** `exportAllToMemoryMode` always returns `true`. `exportAllCatalogFiles` skips a
  catalog silently (`continue`) when its `.idx` file cannot be opened or its query fails. Reporting a reason is
  backlogged (`SpecBacklogNotes.md`, "Collection export — no failure reason for the CSV export").
- **SQLite cancel already cleans up.** `exportToSQLiteFile` deletes its own partial target file when the
  callback cancels, and returns `false`.
- **K2 reports success after a cancel** in the CSV export: it ignores the return value of `exportAllCatalogFiles`.

## Notes

- **Translation context.** `tr("Export")`, `tr("Select")`, `tr("Select a folder")`, `tr("Loading")` and
  `tr("Cancel")` exist in K2's `MainWindow` context. The translations agent decides at implementation which
  K3 context resolves them. The new string "The export folder must be empty." needs `lupdate` and translation.

---

## Manual test charter

Run every line in **Memory**, **File** and **Hosted** mode where the mode applies.

- **CEX-F1 / CEX-F2** — Settings › Collection: Memory shows only "Export to SQLite file"; File shows only "Export to Memory mode (csv)"; Hosted shows both. No button has a tooltip. Switch language: labels are translated.
- **CEX-F3 / CEX-F4** — Each button opens its dialog with the K2 title and default location; cancelling the dialog changes nothing.
- **CEX-F5 / CEX-F6 / CEX-O1** — In a fresh session, Memory → SQLite, then open the file in K3 File mode: all devices, catalogs, tags, mappings, filters, search history and statistics are present; Explore shows each catalog's folder tree; Search finds files.
- **CEX-F6** — Hosted → SQLite: same checks as above.
- **CEX-F7 / CEX-O1 / CEX-C1** — File → Memory and Hosted → Memory into an empty folder, then open the folder in K3 Memory mode: same checks; every catalog has its `.idx` and `.folders.idx`; `catalog_filter.csv` is present and the catalog filters survive. Repeat once in K2: `catalog_filter.csv` is now written there too.
- **CEX-F13** — Choose a folder holding one unrelated file, then a folder holding only an empty subfolder: each time the status bar shows EXPORT, Error, "The export folder must be empty.", and nothing is written.
- **CEX-F8** — Export to an existing `export.db`: the old file is in the trash. Make the trash move fail (e.g. a read-only volume): status bar Error, old file intact.
- **CEX-F9** — Memory → SQLite: the status bar shows EXPORT, In Progress, "Loaded" n of total, then the name of each table as it is copied. CSV export: the name of each catalog as it is written.
- **CEX-F10 / CEX-C5** — Cancel a SQLite export: no target file remains, status Cancelled. Cancel a CSV export: the folder is empty again, and nothing outside it was touched.
- **CEX-F11** — Success: passive notification with path, status Completed, no modal dialog. Export to an unwritable location: status bar EXPORT, Error, no modal dialog.
- **CEX-F12** — Start a device update or a search: export buttons are disabled until it ends.
- **CEX-C6** — After any export: same mode and collection open; settings files unchanged.
- **CEX-C7** — K2 Settings export behaves as before.
- **CEX-F5 known limitation** — Memory mode: run a search over a catalog that has empty folders, then export to SQLite. The empty folders of that catalog are missing (accepted, see Known limitation).
