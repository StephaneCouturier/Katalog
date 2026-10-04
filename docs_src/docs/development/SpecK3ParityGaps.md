---
version: "2.13"
---

# K3 PARITY GAPS — K2 FEATURES NOT YET IN K3

![Status](https://img.shields.io/badge/Status-Reference-lightgrey) ![Issue](https://img.shields.io/badge/Issue-%23803-blue) ![K2](https://img.shields.io/badge/K2-no%20change-lightgrey) ![K3](https://img.shields.io/badge/K3-gap-orange)

## Context

K3 is a port of K2 and must reach full feature parity with it. While preparing the
K3 user documentation (#803, 2026-10-03), the K2 user pages were compared with the
K3 pages. The features below are documented for K2 and implemented in K2, but have
no equivalent in K3.

This page is a **list of known gaps, not a requirement.** Each gap is ported later,
once the documentation work is well advanced; porting one follows the K3 migration
process (read all the K2 code for the feature, get a specifications verdict, port it
in full). Until a gap is ported, the K3 user documentation does not describe it; the
K2 description stays available in the Katalog 2 documentation copy.

When a gap is ported, remove its entry here and add it to the K3 user page.

---

## Settings — Export a collection to another mode

**K2 user doc:** `Settings.md`, sections *Database File mode*, *Hosted mode › Export*,
*Collection export summary*.

K2 can export the open collection to another storage mode:

| Current mode | Export to SQLite file | Export to Memory mode |
|---|---|---|
| Memory | yes | — |
| File | — | yes |
| Hosted | yes | yes |

- **Export to SQLite file** (Memory, Hosted): the user picks the target file
  (default `<collection folder>/export.db`); an existing file at that path is moved
  to the trash first. A progress dialog with Cancel is shown; a message reports the
  result and the target path.
- **Export to Memory mode (csv)** (File, Hosted): the user picks a target folder; all
  CSV table files and the per-catalog `.idx` files are written there, with a
  progress dialog and Cancel, and a result message.

**K2 code:** `qt_widgets/mainwindow_tab_settings_exp.cpp` — `exportToSQLiteFile()`,
`exportToMemoryMode()`; buttons in `mainwindow_tab_settings.cpp`
(`Settings_pushButton_ExportToSQLitFile`, `_ExportToMemoryMode`,
`_Hosted_ExportToSQLiteFile`, `_Hosted_ExportToMemoryMode`).

**Core already available:** `Collection::exportToSQLiteFile()` (Hosted → SQLite),
`Collection::loadAllCatalogFiles()`, `Collection::exportAllToMemoryMode()`,
`Collection::exportAllCatalogFiles()`.

**Porting note:** the Memory → SQLite path calls `MainWindow::backupMemoryDatabaseToFile()`,
which lives in the K2 UI layer. Porting it to K3 needs that logic in `core/` first —
a core change that requires approval.

**K3:** no export action on the Settings page.

---
