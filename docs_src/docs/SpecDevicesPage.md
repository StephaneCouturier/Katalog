---
id: SpecDevicesPage
title: Devices Page — Cards and Table Display
description: Requirements for how the K3 Devices page renders the Storage list and Catalogs list, including the Cards / Table display choice and the Full Table columns
---

# DEVICES PAGE — CARDS AND TABLE DISPLAY

![Status](https://img.shields.io/badge/Status-Approved-brightgreen) ![Implementation](https://img.shields.io/badge/Implementation-planned-lightgrey) ![2.13](https://img.shields.io/badge/Version-2.13-blue)

## Context

K2 has always displayed devices as a table. `Devices_treeView_DeviceList` is fed by
a `QStandardItemModel` through the `DeviceTreeView` proxy
(`qt_widgets/devicetreeview.cpp`), which formats sizes and counts, right-aligns
numbers, draws a tick for boolean columns and weights the font by device type. The
same widget serves all three views — Device tree, Storage list, Catalogs list —
and a `Full Table` checkbox reveals the columns that are not needed day to day.

K3 rebuilt the Devices page around `Kirigami.CardsListView` and a card delegate.
Cards read well for a handful of devices and badly for a collection of a hundred:
values are not column-aligned, so nothing can be compared down a column and
nothing can be sorted.

Nothing had ever written down how the Devices page renders its lists. On
2026-09-11 a request to add a table display found **no spec for the page at all** —
a SPEC GAP. `SpecSelection.md` puts the Devices page explicitly out of its own
scope, and the Devices specs that exist (`SpecDevicesSplit.md`,
`SpecDeviceActiveStatus.md`, `SpecDeviceComment.md`, `SpecDeviceStorageRoot.md`)
each cover one narrow topic and say nothing about rendering. This page is that
missing source of requirements, bootstrapped and ratified by the user on
2026-09-11.

> **Reading the requirement IDs.** Each requirement has a permanent ID.
> IDs are never renumbered or reused; a retired requirement is marked
> `[Removed]`, not deleted. Status is one of
> `[Implemented] / [Planned] / [Backlog] / [Removed]`.

---

## Scope at a glance

**In scope:** how the K3 Devices page renders the **Storage list** and the
**Catalogs list**; the Cards / Table display choice; the Table mode column sets,
their sorting, and the `Full Table` option; the persistence of both choices;
the persistence and restoration of the three-view choice (Device tree / Storage
list / Catalogs list).

**Also in scope:** the **default value** of the `Filter from Selection`
checkbox when the collection has stored none, and where that value is stored
(`DVP-F9`, `DVP-C8`); and whether the card's file count and total size are shown
when they are zero (`DVP-F10`, `DVP-C9`).

**Out of scope (non-goals):** the **Device tree** view, which keeps its current
card rendering and is deferred to later work; the device editor; the device
context menu's contents; which devices the list contains once the checkbox is
set — the filtering behaviour itself, and the rest of the filter bar, stay
outside these rows even though the checkbox's default and persistence are now
inside them; when the active-status cache is probed, which
belongs to `SpecDeviceActiveStatus.md`; the device comment, which belongs to
`SpecDeviceComment.md`. None of them is governed by this spec and this spec does
not authorise changing any of them.

**Applies to:** K3 (`qt_quick`) only. K2 is in maintenance mode and is not
touched; see `DVP-C3`.

---

## A correction worth recording

The K2 setting `Devices/DisplayContents` is **not** a display-mode setting. It
stores *which of the three views* is shown — `"Tree"`, `"Storage"` or
`"Catalogs"` (`qt_widgets/mainwindow_tab_device_ui.cpp:45,55,65`, restored at
`qt_widgets/mainwindow.cpp:345`). The `Full Table` checkbox has its own key,
`Devices/DisplayFullDeviceTable` (`qt_widgets/mainwindow_tab_device_ui.cpp:259`,
read at `qt_widgets/mainwindow_setup.cpp:434`).

K2 therefore has **no** cards-versus-table notion at all — it is always a table.
`DVP-F3` is a genuine port of a K2 feature; `DVP-F1` and its `Devices/DisplayAsTable`
key are new to K3 with no K2 precedent. K3's own handling of
`Devices/DisplayContents` — which it shares with K2 — is `DVP-F7`, added on
2026-09-12.

---

## Operational requirements — *why / for whom*

Goals in real use, independent of how they are built.

| ID | Requirement | Status |
|----|-------------|--------|
| DVP-O1 | A user comparing many devices can see them in a dense, column-aligned table, as K2 does, without losing the card view. | [Planned] |

## Functional requirements — *what the system does*

Observable behaviour that can be triggered and watched.

| ID | Requirement | Status |
|----|-------------|--------|
| DVP-F1 | The Devices page offers a **Cards / Table** display choice, placed in the filter bar at the right of the view buttons, as the display-mode cluster *Cards, Table, `Full Table`*, with `Filter from Selection` following that whole cluster — **not** preceding it. *(Amended 2026-09-12 at the user's request: the original wording placed the Cards / Table control immediately after `Filter from Selection`. The order is now the reverse. `Filter from Selection` sits after `Full Table`, which is itself a table option, so the display-mode cluster is not split. A `ToolSeparator` on each side of the cluster, both shown on the same condition as the cluster, keeps the Device tree view's bar reading as it does today.)* It applies to the **Storage list** and the **Catalogs list** views only. The **Device tree** view is unaffected and keeps its current card rendering — the tree is explicitly deferred to later work and MUST NOT be converted by this work. | [Planned] |
| DVP-F2 | In Table mode, clicking a column header sorts the rows by that column; clicking the same header again reverses the order. Numeric columns — file counts, file sizes, used / free / total space, and IDs — sort **numerically**, not as text, so `10` follows `9` and a formatted size sorts by its underlying byte value. | [Planned] |
| DVP-F3 | In Table mode a **Full Table** option reveals extra columns, matching K2's `Devices_checkBox_DisplayFullTable` column set (`qt_widgets/mainwindow_tab_device_pr.cpp:1627,1901`). It reuses K2's existing `Full Table` string verbatim. Column sets are as listed in [Table mode columns](#table-mode-columns) below. | [Planned] |
| DVP-F4 | The display mode and the Full Table state persist **per collection**, in the collection's `.ini` (`collection->settingsFilePath`), and are restored when the collection is opened. Keys: `Devices/DisplayAsTable` — new, no K2 precedent — and `Devices/DisplayFullDeviceTable`, which is the key K2 already uses, so both versions share one stored value. | [Planned] |
| DVP-F5 | Table mode replicates K2's `DeviceTreeView` proxy rendering (`qt_widgets/devicetreeview.cpp`) in full: the device icon in the Name cell (`drive-harddisk` for Storage, `media-optical-blu-ray` for an active Catalog and `media-optical` otherwise, `drive-multidisk` for Virtual); a tick icon (`dialog-ok-apply`) for the boolean columns Active and Hidden; the **Parent storage** column shows the storage **name as text** — a deliberate divergence from K2, approved by the user on 2026-09-11, because K2 lists that column in its boolean set (`qt_widgets/devicetreeview.cpp:66`), which blanks the storage name the column actually holds and leaves it permanently empty; K2 is not modified, per `DVP-C3`, and neither version's behaviour is drift against the other; locale-formatted and **right-aligned** file sizes and counts; **bold** for Storage rows and **bold italic** for Virtual rows; and the theme's grey foreground for Virtual and Storage rows. | [Planned] |
| DVP-F6 | Row actions in Table mode offer the **same context menu** the card delegate already offers (`qt_quick/PageDevicesViewDelegate.qml`), unchanged — same entries, same conditions, same confirmations. Table mode adds no action and removes none. | [Planned] |
| DVP-F7 | The Devices page's **three-view choice** — Device tree, Storage list, Catalogs list — persists **per collection** in the collection's `.ini` (`collection->settingsFilePath`) under `Devices/DisplayContents`, the key K2 already uses (`qt_widgets/mainwindow_tab_device_ui.cpp:45,55,65`), so both versions share one stored value. K3's `viewFilter` values map to it as `"All"` ↔ `"Tree"`, `"Storage"` ↔ `"Storage"`, `"Catalogs"` ↔ `"Catalogs"`. The stored view is applied when the Devices page first opens and again when another collection is opened. Restoring `"Tree"` restores *which view is shown* only; the Device tree keeps its card rendering per `DVP-F1`. | [Planned] |
| DVP-F8 | **When `Devices/DisplayContents` is absent, K3 opens on the Device tree (`"All"`)** — a deliberate divergence from K2, which effectively falls back to the Catalogs list. K2's fallback is not a stated requirement but an artefact: `qt_widgets/mainwindow.cpp:344` reads the key with **no default**, so none of its three branches matches and the `.ui` default stands — `Devices_radioButton_CatalogList` is pre-checked (`qt_widgets/mainwindow.ui:5502-5509`) — while `qt_widgets/mainwindow_tab_device_pr.cpp:2104` reads the *same* key defaulting to `"Tree"`, so K2 is internally inconsistent about it. K3 therefore keeps its existing opening view, so collections that never stored a choice see no change; the user chose this over matching K2 on 2026-09-12. This divergence is intended and MUST NOT be reported as drift against `DVP-F7` or `DVP-C3`. | [Planned] |
| DVP-F9 | **When `Devices/FilterFromSelection` is absent, `Filter from Selection` is ON.** A new user therefore sees the Devices page already scoped to the device chosen on the Selection page, which is the reading most users expect; showing every device of the collection regardless of the selection was an artefact of the hard `false` fallback, never a stated requirement. The choice persists **per collection** in the collection's `.ini` (`collection->settingsFilePath`) under `Devices/FilterFromSelection`. A collection that already stored a value keeps it unchanged — including a stored `false`; only the absent-key case changes. There is **no K2 equivalent**: K2's Devices tab has no such checkbox, so this is K3-only and MUST NOT be reported as divergence from K2. The user approved the default on 2026-09-12. | [Planned] |
| DVP-F10 | **A Catalog card shows its file count and total size even when both are zero.** An empty catalog reads `0` files and a zero size, instead of the count and size disappearing from the card's detail line as they do today. Hiding them was never a stated requirement: K3's own Table view already prints `0` and the zero size unconditionally (`qt_quick/adapters/devicetablemodel.cpp:161-164`), and so does K2's device tree (`qt_widgets/devicetreeview.cpp:101-107`), so the card was the only place in either version where an empty catalog looked like a catalog with unknown contents. This row closes that inconsistency; the user requested it on 2026-09-12. It applies to **Catalog** devices only — Storage and Virtual cards keep today's behaviour, which is deliberately left unchanged and MUST NOT be widened without a further row. | [Planned] |

### Table mode columns {#table-mode-columns}

**Storage list**

| | Columns |
|---|---|
| Default | Name, Storage ID, Number of files, Total Size, Used space, Free space, Total space, Date updated, Path, Label, FileSystem |
| `Full Table` adds | Active, Type, Brand, Model, Serial Number, Build Date, Comment 1, Comment 2, Comment 3 |

**Catalogs list**

| | Columns |
|---|---|
| Default | Name, Number of files, Total Size, Date updated, Path, File Type, Hidden, Metadata, Checksum, Parent storage |
| `Full Table` adds | Active, Catalog ID, Date Loaded, App Version, File Path |

**Date Loaded** and **File Path** appear **only in Memory database mode**, as K2
does (`qt_widgets/mainwindow_tab_device_pr.cpp:1917-1920`). In File and Hosted
mode they are absent even with `Full Table` on.

**App Version** in the Catalogs table is the catalog-compatibility column
described in [`SpecVersions.md`](SpecVersions) — see *Use Case 3: Display Catalog
Version in UI*, which documents it as K2's column 30, shown when
`Display Full Table` is checked.

## Constructional requirements — *how it is built / limits / MUST-NOTs*

Boundaries and implementation constraints, not user-visible behaviour.

| ID | Requirement | Status |
|----|-------------|--------|
| DVP-C1 | **Cards remains the default rendering.** Card rendering is unchanged by the Cards / Table work of `DVP-F1` to `DVP-F8` — `DVP-F10` is a separate, later change to the card and is not governed by this sentence — including `DCM-F3` (`SpecDeviceComment.md`) — the device comment beside the device name. The comment is **not** a table column: K2's table has none, and `DCM-C7` limits the comment to annotation. A future table column for it needs a new requirement. | [Planned] |
| DVP-C2 | Toggling the display mode, or toggling `Full Table`, MUST NOT trigger an active-status probe. Only opening the page and switching the type filter probe (`DAS-F3`); adding a third trigger would break `DAS-O4`, which exists to keep a blocking `QDir::exists()` off the UI thread at moments the user did not ask for. Both toggles re-render rows already loaded. | [Planned] |
| DVP-C3 | K2 (`qt_widgets`) MUST NOT be modified. It is in maintenance mode and already satisfies `DVP-O1` in its own way; its always-table Devices view, its three-view radio buttons and its `Devices/DisplayContents` key MUST NOT be reported as drift against these rows. | [Planned] |
| DVP-C4 | **No SQL in `qt_quick/`.** The extra Storage and Catalog columns are sourced by calling the existing core methods `Storage::loadStorage()` and `Catalog::loadCatalog()`. **No new or changed `core/` method** is authorised by this spec; needing one is a stop-and-ask, per the core-class rule in `CLAUDE.md`. | [Planned] |
| DVP-C5 | All column header strings reuse K2's existing `tr()` wording **byte-for-byte** — copied from the K2 model headers, not retyped. Exactly **two** new user-visible strings are authorised: `Cards` and `Table`, the toggle labels, approved per string by the user on 2026-09-11. Any further new string needs its own approval. | [Planned] |
| DVP-C6 | Restoring the view per `DVP-F7` MUST NOT cause a second device-list load, and therefore no second active-status probe, on application start or on collection open: `AppManager::getDeviceList()` probes on **every** call (`collection->updateAllDeviceActive()`, `qt_quick/appmanager.cpp:2422`), and a second pass would breach `DVP-C2` and `DAS-O4`. The restored value MUST be in place **before** the list is built. Note that on collection open `refreshAllUI()` runs **before** `emit databaseModeChanged()` (`qt_quick/appmanager.cpp:977-978`), so the existing `onDatabaseModeChanged` handler in `qt_quick/PageDevicesView.qml` is the **wrong place** for this restore even though `DVP-F4`'s display-mode restore sits there safely — that one only re-renders rows already loaded, whereas a view change arriving after the list was rebuilt with the previous view would force a reload. Switching the view *by user action* keeps its existing single probe, per `DAS-F3`. | [Planned] |
| DVP-C7 | `Devices/DisplayContents` is read and written through `AppManager`, alongside the `DVP-F4` keys; **no `core/` change** is authorised, per `DVP-C4`. **No new user-visible string:** the three view labels already exist. | [Planned] |
| DVP-C8 | `DVP-F9`'s default is applied **where the value is read**, as the fallback of the settings read — it MUST NOT be seeded by writing the key at first run or on collection open. Writing it would make an untouched collection indistinguishable from one the user deliberately set, and would defeat the guarantee that only the absent-key case changes. `Filter from Selection` carries **no new user-visible string**: the existing label is unchanged. | [Planned] |
| DVP-C9 | `DVP-F10` needs a size formatter that renders zero. The formatter the card uses today returns an **empty string** for a value of zero or less, which would blank the size even once the card's own condition is lifted — so the zero-rendering formatter must be used for this figure. Both are already exposed to QML and produce identical output for every value above zero, so this is a substitution, not a new capability, and **no `core/` change** is authorised. **No new user-visible string:** the existing plural `files` is reused, so an empty catalog reads `0 files`. A singular form for the zero or one case would cost a new translation slot in 30 languages and is **not** authorised here — it needs its own per-string approval. | [Planned] |

---

## Implementation notes

Not requirements — the shape agreed at approval time, recorded so the work does
not have to rediscover it.

- The table is built on a new `qt_quick/adapters/devicetablemodel.{h,cpp}`
  `QAbstractTableModel`, following the existing `explorefilesmodel` pattern.
- It is driven by `TableView` + `Controls.HorizontalHeaderView`, as
  `qt_quick/PageExploreFiles.qml` already does.
- Creating those two files was explicitly approved by the user on 2026-09-11,
  under the "never create a new source file without the user asking" rule.
- K2 reference points: `loadDevicesStorageToModel()`
  (`qt_widgets/mainwindow_tab_device_pr.cpp:1400`), `loadDevicesCatalogToModel()`
  (`:1652`), and the rendering proxy `qt_widgets/devicetreeview.cpp`.

---

## Manual test charter

For each row: set up the stated condition, run the operation, confirm the result.

- **DVP-F1** — On the Storage list, switch to Table: the devices render as an aligned table. Switch to the Catalogs list: still Table. Switch to the Device tree: it still renders as cards and shows no Cards / Table control effect. Return to the Storage list: still Table.
- **DVP-F1 (placement)** — Confirm the filter bar reads, left to right: the three view buttons, then the display-mode cluster `Cards`, `Table`, `Full Table`, then `Filter from Selection` last. Confirm `Filter from Selection` still filters the list in both modes, and that its checked state is unchanged by the move.
- **DVP-F1 (placement, Device tree)** — Switch to the Device tree view: the display-mode cluster and both separators are hidden, and `Filter from Selection` remains the only control at the right of the bar.
- **DVP-F2** — In Table mode click the *Number of files* header: rows sort ascending by count; click again: descending. Confirm a device with 1000 files sorts above one with 999 — not below it as a text sort would give. Repeat on *Total Size* with sizes spanning KB / MB / GB, and on *Storage ID*.
- **DVP-F3 (Storage)** — On the Storage list in Table mode with `Full Table` off, confirm exactly the default column set. Tick `Full Table`: Active, Type, Brand, Model, Serial Number, Build Date and Comment 1/2/3 appear. Open the same collection in K2 and confirm the same columns appear there.
- **DVP-F3 (Catalogs)** — On the Catalogs list in File mode with `Full Table` on, confirm Active, Catalog ID and App Version appear and that **Date Loaded and File Path do not**. Open the same collection in Memory mode: both now appear.
- **DVP-F3 (string)** — Confirm the option reads exactly `Full Table` and, with the interface in French, that it is translated — proving the existing K2 string was reused and no new slot was spent.
- **DVP-F4** — Set Table + `Full Table` on, close the application, reopen: both are restored. Open a second collection: its own stored values apply, not the first collection's. Confirm `Devices/DisplayAsTable` and `Devices/DisplayFullDeviceTable` are written to the collection `.ini`, not to the default application settings.
- **DVP-F4 (shared key)** — Tick `Full Table` in K3, then open the same collection in K2: K2's `Full Table` checkbox is ticked. Untick it in K2 and reopen in K3: it is unticked.
- **DVP-F5** — In Table mode confirm, against the same collection open in K2: Storage rows bold, Virtual rows bold italic, both greyed; the correct icon per device type, and `media-optical-blu-ray` only while the Catalog is active; a tick in Active and Hidden rather than a word; sizes and counts locale-formatted and right-aligned.
- **DVP-F5 (Parent storage)** — On the Catalogs list in Table mode, confirm the **Parent storage** column shows the parent storage's **name**. Open the same collection in K2 and confirm that column is empty there. Both are correct; the difference is the approved divergence, not a defect.
- **DVP-F6** — Right-click a row in Table mode and compare the menu entry by entry with the same device's card menu in Cards mode: identical entries, identical enabling conditions. Trigger a destructive entry from the table and confirm the same confirmation dialog appears.
- **DVP-C1** — Confirm Cards is what a collection with no stored setting opens in, and that a device comment still shows beside the name on its card. Confirm no table column shows the comment.
- **DVP-C2** — Unmount a device from a terminal without leaving the Devices page, then toggle Cards/Table and toggle `Full Table`. The active status does not change, proving no probe ran. Now switch the type filter: the status updates, per `DAS-F3`.
- **DVP-C2 (network mount)** — With an unreachable network mount in the collection, toggle the display mode repeatedly. The interface does not freeze.
- **DVP-C4** — Search `qt_quick/` for `QSqlQuery`: no new occurrence. Confirm by inspection that the extra columns come from `Storage::loadStorage()` and `Catalog::loadCatalog()` and that `core/` is unchanged.
- **DVP-C5** — Run `ninja translations_lupdate`. Exactly two new untranslated strings appear for the Devices page, `Cards` and `Table`; every column header resolves to an existing K2 translation.
- **DVP-F7** — Select the Storage list, close the application, reopen: it opens on the Storage list. Repeat for the Catalogs list and for the Device tree. Open a second collection: its own stored view applies, not the first collection's. Confirm `Devices/DisplayContents` is written to the collection `.ini`, not to the default application settings.
- **DVP-F7 (shared key)** — Select the Catalogs list in K3, then open the same collection in K2: `Devices_radioButton_CatalogList` is checked. Select the Storage list in K2 and reopen in K3: the Storage list is shown.
- **DVP-F7 (tree rendering)** — With `"Tree"` stored, reopen: the Device tree is shown **as cards**, with no Cards / Table control effect.
- **DVP-F8** — Delete the `Devices/DisplayContents` key from a collection `.ini` and open that collection in K3: the Device tree is shown. Open the same collection in K2: K2 shows the Catalogs list. Both are correct, not a defect.
- **DVP-F9** — Open a collection whose `.ini` has no `Devices/FilterFromSelection` key (a new collection, or delete the key from an existing one). On the Devices page `Filter from Selection` is **checked**, and the list is scoped to the device selected on the Selection page rather than showing the whole collection.
- **DVP-F9 (stored value wins)** — Set `Devices/FilterFromSelection` to `false` in a collection `.ini` and open it: the checkbox is **unchecked** and the list shows all devices. Set it to `true` and reopen: checked. A collection that had already stored a choice is unaffected by the new default.
- **DVP-F9 / DVP-C8 (no seeding)** — Open a collection with the key absent, visit the Devices page, and close the application **without touching the checkbox**. The key is still absent from the collection `.ini` — the default was applied on read, not written. Untick the checkbox and close: now the key is present and reads `false`.
- **DVP-F10** — Create or open a catalog containing no files and look at its card in Cards mode: the detail line shows `0 files` and a zero size, not a line with the figures missing. Confirm the same catalog reads the same zero figures in Table mode and, opened in K2, in the device tree.
- **DVP-F10 (non-empty unchanged)** — A catalog with files shows exactly the count and size it shows today, with no change of wording, separator or order.
- **DVP-F10 (other types unchanged)** — A Storage card and a Virtual card with a zero file count look exactly as they do today; the zero figures are not added to them.
- **DVP-C9** — With the interface in French, confirm the empty catalog's size renders as the localised zero rather than blank. Run `ninja translations_lupdate`: no new untranslated string appears for the Devices page.
- **DVP-C6 (start)** — Instrument `Device::updateActiveState()`. With a stored view of `"Storage"`, start the application and open the Devices page: the probes amount to one pass over the devices, not two.
- **DVP-C6 (collection open)** — With the Devices page open and a stored view differing from the one on screen, open another collection: exactly one probe pass runs, and the page ends on the new collection's stored view.
- **DVP-C6 (network mount)** — With an unreachable network mount configured, open a collection whose stored view differs from the current one. The interface does not freeze twice over.
