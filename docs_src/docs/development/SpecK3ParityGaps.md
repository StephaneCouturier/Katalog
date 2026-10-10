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

## Settings — Hosted mode: only local or private-network servers (security)

**Found:** 2026-10-04, while writing the K3 Settings user page.
**K2 user doc:** `Settings.md`, section *Hosted mode › Security*.

K2 restricts which database server a Hosted collection may use. Before saving the
connection it validates the host name with `Database::validateHostname()` (core):

- **Localhost** (`localhost`, `127.x.x.x`): accepted without a message.
- **Private network** (`192.168.x.x`, `10.x.x.x`, `172.16-31.x.x`): a confirmation is
  asked — *"Private network connection: &lt;host&gt;"*, *"Your data will be sent to this
  network database."* / *"Continue?"*, default *No*.
- **Anything else** (public IP, domain name): refused — *"Invalid hostname."*, *"Only
  local or private network allowed."*, *"Valid:"* followed by the accepted ranges.
- An empty host name becomes `localhost`.

**K2 code:** `qt_widgets/mainwindow_tab_settings.cpp:107-145` (Hosted branch of the
apply-and-restart handler). The strings above are existing K2 `tr()` strings, to be
reused verbatim.

**Core already available:** `Database::validateHostname()` (`core/database.cpp:448`) —
no core change needed.

**K3:** `Database::validateHostname()` is never called. `AppManager::openCollectionHosted()`
(`qt_quick/appmanager.cpp:2138`) connects to any host, from the Settings *Connect*
button, from the drawer's recent-collections list (`:2251`) and at start-up with
*Connect automatically on startup*. A K3 user can therefore send collection data to a
public server, which K2 deliberately prevents.

**Porting note:** follow the K3 message rules (CLAUDE.md, `SpecValidationRules.md`): the
refused host is a blocking form-validation error → `Kirigami.InlineMessage` on the
Settings form; the private-network question is a decision → modal dialog with *No* as
default. Decide whether a recent entry or the auto-connect path pointing to a refused
host is blocked the same way (K2 only validates on save).

**K3 user doc:** `Settings.md` (K3) does not claim the restriction until it is ported.

---

## Explore — "Tag this folder" on the directory tree

**Found:** 2026-10-07, while writing the K3 Explore user page.
**K2 user doc:** `Explore.md`, section *Directory context menu (right-click)*.

In K2, right-clicking a directory in the Explore directory tree opens a menu with one
entry, *Tag this folder* (icon `tag`). It fills the folder path of the Tags tab with
that directory and switches to the Tags tab, where the user picks the tag and confirms.

**K2 code:** `qt_widgets/mainwindow_tab_explore.cpp` —
`on_Explore_treeview_Directories_customContextMenuRequested()` (menu) and
`exploreContextTagFolder()` (fills `Tags_lineEdit_FolderPath`, switches tab). The string
*Tag this folder* is an existing K2 `tr()` string, to be reused verbatim.

**Core already available:** nothing needed — the action only pre-fills a UI field.

**K3:** the Explore directory tree (`qt_quick/PageExploreFolders.qml`) has no context
menu. The Tags page has a folder path field (`folderPathField` in
`qt_quick/PageTagsForm.qml`) that the portage would pre-fill before showing the page.

**Porting note:** K2 switches tab; K3 opens the Tags page in the page stack. Decide
whether `Esc` / *Close* on Tags then returns to Explore, as Explore returns to Search
results after *Explore folder*.

**K3 user doc:** `Explore.md` (K3) does not describe a directory context menu until it is
ported.

---

## Devices — "Verify MIME Types" on a catalog

**Found:** 2026-10-07, while writing the K3 Create user page.
**K2 user doc:** `DevicesCatalogs.md`, *Action buttons* (*Verify MIME Types*), and `Create.md`
*Accuracy note* (types are guessed from the extension at indexing; MIME verification on a
completed catalog corrects them).

K2 offers *Verify MIME Types* for an available catalog: it reads each file's content with
the system MIME database, reports the number of mismatches with the extension, and saves a
report file.

**K2 code:** `qt_widgets/mainwindow_tab_device_ui.cpp:751`
(`on_Catalogs_pushButton_VerifyMIMETypes_clicked`), `CatalogJobStoppable::VerifyMimeTypes`;
strings *MIME Verification Complete* and the two result messages are existing K2 `tr()`
strings.

**Core already available:** `CatalogJobStoppable::VerifyMimeTypes` (already used by K3's
*Verify then Split*, `qt_quick/appmanager.cpp:3476`).

**K3:** MIME verification only runs inside *Split catalog by file type* → *Verify then
Split*. No standalone action.

**K3 user doc:** `Create.md` (K3) does not mention a standalone verification until it is ported.

---

## Create — end of creation: report, selection of the new catalog, Devices shown

**Found:** 2026-10-07, while writing the K3 Create user page.
**K2 user doc:** `Create.md`, *Create the catalog*.

When a creation completes, K2:
- shows a report dialog: *Catalog created:* name, *Path:*, *Number of files:*, *Total file
  size:*, and when the parent storage was updated, *Storage updated:* name, path, *Used Space:*
  / *Free Space:* with *(added:* deltas;
- selects the new catalog (`selectedDevice->ID = reportDevice->ID`, Selection label set);
- switches to the Devices tab.

**K2 code:** `qt_widgets/mainwindow_tab_device_pr.cpp:1863-1905` (selection and tab) and
`:3034-3060` (report). All strings are existing K2 `tr()` strings.

**K3:** a passive notification *Catalog created successfully.* and a refresh of the device
lists (`qt_quick/Main.qml`, `pageCreate` `onCatalogCreationCompleted`). No report, no
selection change, the page stays on Create.

**Porting note:** the report content may belong in the activity panel (SpecProgressReport /
OPQ) rather than a dialog; decide with the specifications agent.

---

## Create and Tags — file system tree to pick a folder

**Found:** 2026-10-07 (Create page).
K2's Create tab has a file-system tree (`Create_treeView_Explorer`, `QFileSystemModel`,
`qt_widgets/mainwindow_tab_create.cpp:45`, `:273`): clicking a folder fills the source path.
K2's Tags tab has the same tree (`Tags_treeview_Explorer`, `qt_widgets/mainwindow_tab_tags.cpp:90`)
to fill the folder to tag.
K3 offers typing and a folder dialog only (*Select the path* on Create, *Select a folder* on Tags).
**To decide:** port, or accept the folder dialog as the K3 equivalent.

---

## Create — "Add Storage" button

**Found:** 2026-10-07 (Create page).
K2's Create tab has *Add Storage* (`on_Create_pushButton_AddStorage_clicked`,
`qt_widgets/mainwindow_tab_create.cpp:117`): it switches to the Devices tab in tree view so
a storage can be added. K3's Create page has no such button; *Add Storage* exists on the
Devices page toolbar.
**K3 user doc:** `Create.md` points to *Add Storage* on the Devices page.
**To decide:** port (open Devices, or create the storage and open Edit device), or accept.

---

## Backup — per-file report after a run

**Found:** 2026-10-07, while writing the K3 Backup user page.
**K2 user doc:** `BackUp.md`, *Report*.
After a run, K2 replaces the preview table with a backup report: one row per file with
*Status* (*Copied*, *Moved*, *Archived & Copied*, *Conflict*, *Error*), *File name*, *Path*,
*Size*, and a totals line.
**K2 code:** `qt_widgets/mainwindow_tab_backup.cpp` (report after execution).
**K3:** only a one-line summary at the top of the Backup page (`qt_quick/PageBackupForm.qml`,
`onBackupFinished`): copied/moved, archived & copied, conflicts, errors. The files in conflict
or in error cannot be listed.
**K3 user doc:** `BackUp.md` (K3) describes the summary line only.

---

# To review and fix before the 3.0 release

Defects and drift found while writing the K3 user documentation (#803). Not parity
gaps: K3 has the feature, but something in it differs from the spec, from K2 or from
the K3 message rules. Each one goes through the specifications gate before it is fixed.
When one is fixed, remove its entry here.

## Devices — storage path-change dialog: "Full re-index" instead of "Full re-scan"

**Found:** 2026-10-07 (Edit device page).
`SpecDeviceStorageRoot.md` `DSR-F1` requires the **Storage** dialog to offer *Replace path
root*, *Full re-scan*, *Skip*; `DSR-F3` gives *Full re-index* to the **Catalog** dialog
only. K3 uses one dialog for both (`qt_quick/Main.qml`, `editStoragePathDialog`, button
`editPathRescanBtn`) and always shows *Full re-index*.
**User doc:** `DevicesEdit.md` documents *Full re-index* (what K3 shows today); update it if
the label changes.

## Devices — storage picture fallback keyed on the device ID

**Found:** 2026-10-07 (Edit device page).
When no picture is chosen, K3 shows `<images folder>/<deviceId>.jpg`
(`qt_quick/PageDeviceEditForm.qml`, `Image.source`). K2 uses the storage's internal ID
(`qt_widgets/mainwindow_tab_device_pr.cpp:2202`, `activeDevice->storage->ID`), and
`SpecStorageIdentity.md` `STI-C8` keys the filename on the internal `storage_id`. The device
ID and the storage ID are different numbers, so a picture named for K2 is not found by K3.
**User doc:** `DevicesEdit.md` does not describe the fallback until this is settled.

## Devices — Edit Device: name validation shown in a modal dialog

**Found:** 2026-10-07 (Edit device page).
Saving with an empty name shows *"Provide a name for this device."* through
`editValidationDialog`, a modal dialog (`qt_quick/Main.qml`, `pageDeviceEdit`
`onSaveError`). CLAUDE.md and `SpecValidationRules.md` require a blocking form-validation
error to be a `Kirigami.InlineMessage` (Warning) at the top of the form, hidden again when
the field is edited. Check the other `saveError` messages of the form at the same time.

## Devices — typo in the "All active" question

**Found:** 2026-10-07 (Devices page).
The *All active* dialog (`qt_quick/Main.qml`, `devUpdateAllDialog`) reads
*"Do you want a the summary of updates for each catalog?"* — "a the". Fixing it changes a
translated string (all 30 languages; check whether K2 uses the same source text) and needs
per-string approval.
**User doc:** `Devices.md` describes the question without quoting it.

## Backup — no target space check when a link is run

**Found:** 2026-10-07 (Backup page).
`SpecBackup.md` `BKP-F8` ([Implemented]): before transfer, the run is **blocked** when target
space is insufficient and **warns** when it is low. K2 does it at run time
(`qt_widgets/mainwindow_tab_backup.cpp:999-1010`). In K3 the space status is computed only for
the preview (`AppManager::startBackupPreview` → `spaceStatus`); `AppManager::runBackup` and
*Run listed links* start without any space check.
**User doc:** `BackUp.md` (K3) says the preview checks the space; it does not claim a check at run.

## Backup — "Export to CSV" in a link's menu exports the last preview, of any link

**Found:** 2026-10-07 (Backup page).
*More actions* → *Export to CSV* on a card calls `appManager1.exportLastBackupPreviewToCsv()`
(`qt_quick/PageBackupForm.qml`), which writes `m_lastPreviewRows`: the last preview computed,
whichever link it was for, or nothing if no preview ran in this session. The entry sits on a
specific link, so a user expects that link's data. The Preview page has its own *Export to CSV*,
which is unambiguous.
**To decide:** remove the menu entry, disable it unless the last preview is this link's, or
compute this link's preview on demand.
**User doc:** `BackUp.md` (K3) states that it exports the last preview.

## First run — welcome message says "tab"

**Found:** 2026-10-07 (tutorial page).
The K3 welcome dialog (`qt_quick/Main.qml`, `firstRunWelcomeDialog`) ends with *"You can change
these in the tab %3."*, where %3 is *Settings*. K3 has pages, not tabs. The sentence is the K2
`tr()` string reused verbatim; changing it costs a translation slot and needs per-string approval.
It also names the theme *System*, which is not one of the K3 theme names (*Desktop Theme*,
*Desktop Theme (gray)*, *Katalog Colors*).
**User doc:** `tutorial.md` describes the message without quoting it.

## Settings — Hosted: devices missing in Selection after *Connect*

**Found:** 2026-10-10 (Settings page, maintainer test).
After connecting to a hosted database with *Connect* on the Settings page, the Selection page
showed no devices. After restarting K3 on the same hosted collection, the devices were listed.
Not reproduced or analysed yet. *Connect* calls `AppManager::openCollectionHosted()`, which goes
through the same `reconnectToDatabase()` → `refreshAllUI()` → `refreshDeviceList()` path as
opening a File or Memory collection, so the difference is not visible in the code alone.
**To collect:** the mode of the collection open before *Connect*, whether the hosted database was
new or empty, and the console output after *Connect*. A private MariaDB instance in the
scratchpad is an option to reproduce it without the maintainer's server.
**User doc:** none; `Settings.md` (K3) says the collection opens directly.
