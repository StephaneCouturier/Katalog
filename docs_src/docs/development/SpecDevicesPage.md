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

**In scope:** how the K3 Devices page renders the **Storage list**, the
**Catalogs list** and the **Device tree**; the Cards / Table display choice; the
Table mode column sets,
their sorting, and the `Full Table` option; the persistence of both choices;
the persistence and restoration of the three-view choice (Device tree / Storage
list / Catalogs list).

**Also in scope:** the **default value** of the `Filter from Selection`
checkbox when the collection has stored none, and where that value is stored
(`DVP-F9`, `DVP-C8`); and whether the card's file count and total size are shown
when they are zero (`DVP-F10`, `DVP-C9`); the card's **space figures** and the
font and colour of the **device name on the card** (`DVP-F16`, `DVP-F17`,
`DVP-C13`, `DVP-C14`); the **wrapping** of the Devices cards' text, applying the
app-wide rule of `SpecCardsAndTables.md` to this page (`DVP-F18`, `DVP-C16`);
the **space-saving card revision** of 2026-09-12 — card spacing, icon size,
indentation, separator and expand / collapse (`DVP-F19` to `DVP-F25`, `DVP-C18`
to `DVP-C21`); the **filter bar's behaviour at a narrow window** (`DVP-F27`,
`DVP-F28`, `DVP-C23` to `DVP-C25`); the **labels of the three view buttons**
(`DVP-F30`, `DVP-C27`); and the **Device tree in Table mode**
— its column set, its expand and collapse, its sorting and its display key
(`DVP-F11` to `DVP-F15`, `DVP-C10` to `DVP-C12`).

**Out of scope (non-goals):** the Device tree's **card** rendering, which is
unchanged — `DVP-F11` adds a Table rendering beside it, it does not replace it;
persisting which rows are expanded; the device editor; the device
context menu's contents; which devices the list contains once the checkbox is
set — the filtering behaviour itself, and the rest of the filter bar, stay
outside these rows even though the checkbox's default and persistence are now
inside them; when the active-status cache is probed, which
belongs to `SpecDeviceActiveStatus.md`; the device comment, which belongs to
`SpecDeviceComment.md`. None of them is governed by this spec and this spec does
not authorise changing any of them.

**Applies to:** K3 (`qt_quick`) only. K2 is in maintenance mode and is not
touched; see `DVP-C3`.

> **Naming note.** Throughout this page the three views are called the **Device
> tree**, the **Storage list** and the **Catalogs list**. Since `DVP-F30` those
> are descriptive names for the views, **not** the text on their buttons — the
> labels are given by `DVP-F30`, and the stored values of `DVP-F7` are different
> again. Three vocabularies, deliberately kept apart: the view, its label, its
> stored value.

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
| DVP-F1 | The Devices page offers a **Cards / Table** display choice, placed in the filter bar at the right of the view buttons, as the display-mode cluster *Cards, Table, `Full Table`*, with `Filter from Selection` following that whole cluster — **not** preceding it. *(Amended 2026-09-12 at the user's request: the original wording placed the Cards / Table control immediately after `Filter from Selection`. The order is now the reverse. `Filter from Selection` sits after `Full Table`, which is itself a table option, so the display-mode cluster is not split. A separator on each side of the cluster, both shown on the same condition as the cluster, keeps the Device tree view's bar reading as it does today.)* This row fixes the **order** of the controls, not the widget the bar is built from: `DVP-F27` replaces that widget and the order is unchanged by it. The buttons' **labels** are given by `DVP-F30`; the names used here are the views', not the buttons'. It applies to **all three views** — the Storage list, the Catalogs list and the Device tree. *(Amended 2026-09-12 at the user's request: this row originally applied the choice to the Storage and Catalogs lists only and stated that the Device tree "keeps its current card rendering — the tree is explicitly deferred to later work and MUST NOT be converted by this work". That deferral is now lifted and the tree's Table rendering is authorised by `DVP-F11` to `DVP-F15`. The tree's **card** rendering is not removed: Table is a second rendering of the same view, chosen by the same control.)* | [Planned] |
| DVP-F2 | In Table mode, clicking a column header sorts the rows by that column; clicking the same header again reverses the order. Numeric columns — file counts, file sizes, used / free / total space, and IDs — sort **numerically**, not as text, so `10` follows `9` and a formatted size sorts by its underlying byte value. | [Planned] |
| DVP-F3 | In Table mode a **Full Table** option reveals extra columns, matching K2's `Devices_checkBox_DisplayFullTable` column set (`qt_widgets/mainwindow_tab_device_pr.cpp:1627,1901`). It reuses K2's existing `Full Table` string verbatim. Column sets are as listed in [Table mode columns](#table-mode-columns) below. | [Planned] |
| DVP-F4 | The display mode and the Full Table state persist **per collection**, in the collection's `.ini` (`collection->settingsFilePath`), and are restored when the collection is opened. Keys: `Devices/DisplayAsTable` — new, no K2 precedent — and `Devices/DisplayFullDeviceTable`, which is the key K2 already uses, so both versions share one stored value. | [Planned] |
| DVP-F5 | Table mode replicates K2's `DeviceTreeView` proxy rendering (`qt_widgets/devicetreeview.cpp`) in full: the device icon in the Name cell (`drive-harddisk` for Storage, `media-optical-blu-ray` for an active Catalog and `media-optical` otherwise, `drive-multidisk` for Virtual); a tick icon (`dialog-ok-apply`) for the boolean columns Active and Hidden; the **Parent storage** column shows the storage **name as text** — a deliberate divergence from K2, approved by the user on 2026-09-11, because K2 lists that column in its boolean set (`qt_widgets/devicetreeview.cpp:66`), which blanks the storage name the column actually holds and leaves it permanently empty; K2 is not modified, per `DVP-C3`, and neither version's behaviour is drift against the other; locale-formatted and **right-aligned** file sizes and counts; **bold** for Storage rows and **bold italic** for Virtual rows; and the theme's grey foreground for **Virtual rows only** — **Storage rows take the normal text colour**. *(Amended 2026-09-12: this row originally required the grey for Virtual **and** Storage. The user reported that Storage devices looked lighter on the Devices page than on the Selection page, where every device including Storage is drawn in the normal text colour. K2 does not support the old wording either: it uses two distinct tones, `#999`/`#666` for Virtual and `#CCC`/`#444` for Storage (dark/light theme, `qt_widgets/devicetreeview.cpp:199-218`), so Storage is markedly **darker** than Virtual and, at `#444`, close to normal text — never a disabled colour. K2's literal hex values are deliberately **not** copied: they are fixed colours that ignore the active theme, whereas K3 renders from the theme's palette, and the theme's normal text colour is the nearest faithful equivalent of `#444`. Note also that K2 applies these colours only when a Katalog theme is set; with the plain desktop theme K2 leaves both types at the default text colour, which is a further reason not to treat its hex values as the requirement. The Virtual grey, and the bold / bold-italic weighting, are unchanged.)* | [Planned] |
| DVP-F6 | Row actions in Table mode offer the **same context menu** the card delegate already offers (`qt_quick/PageDevicesViewDelegate.qml`), unchanged — same entries, same conditions, same confirmations. Table mode adds no action and removes none. | [Planned] |
| DVP-F7 | The Devices page's **three-view choice** — the Device tree, the Storage list and the Catalogs list, labelled per `DVP-F30` — persists **per collection** in the collection's `.ini` (`collection->settingsFilePath`) under `Devices/DisplayContents`, the key K2 already uses (`qt_widgets/mainwindow_tab_device_ui.cpp:45,55,65`), so both versions share one stored value. K3's `viewFilter` values map to it as `"All"` ↔ `"Tree"`, `"Storage"` ↔ `"Storage"`, `"Catalogs"` ↔ `"Catalogs"`. The stored view is applied when the Devices page first opens and again when another collection is opened. Restoring `"Tree"` restores *which view is shown* only; the tree is then rendered in whichever display mode `DVP-F15`'s shared key holds. | [Planned] |
| DVP-F8 | **When `Devices/DisplayContents` is absent, K3 opens on the Device tree (`"All"`)** — a deliberate divergence from K2, which effectively falls back to the Catalogs list. K2's fallback is not a stated requirement but an artefact: `qt_widgets/mainwindow.cpp:344` reads the key with **no default**, so none of its three branches matches and the `.ui` default stands — `Devices_radioButton_CatalogList` is pre-checked (`qt_widgets/mainwindow.ui:5502-5509`) — while `qt_widgets/mainwindow_tab_device_pr.cpp:2104` reads the *same* key defaulting to `"Tree"`, so K2 is internally inconsistent about it. K3 therefore keeps its existing opening view, so collections that never stored a choice see no change; the user chose this over matching K2 on 2026-09-12. This divergence is intended and MUST NOT be reported as drift against `DVP-F7` or `DVP-C3`. | [Planned] |
| DVP-F9 | **When `Devices/FilterFromSelection` is absent, `Filter from Selection` is ON.** A new user therefore sees the Devices page already scoped to the device chosen on the Selection page, which is the reading most users expect; showing every device of the collection regardless of the selection was an artefact of the hard `false` fallback, never a stated requirement. The choice persists **per collection** in the collection's `.ini` (`collection->settingsFilePath`) under `Devices/FilterFromSelection`. A collection that already stored a value keeps it unchanged — including a stored `false`; only the absent-key case changes. There is **no K2 equivalent**: K2's Devices tab has no such checkbox, so this is K3-only and MUST NOT be reported as divergence from K2. The user approved the default on 2026-09-12. | [Planned] |
| DVP-F10 | **A Catalog card shows its file count and total size even when both are zero.** An empty catalog reads `0` files and a zero size, instead of the count and size disappearing from the card's detail line as they do today. Hiding them was never a stated requirement: K3's own Table view already prints `0` and the zero size unconditionally (`qt_quick/adapters/devicetablemodel.cpp:161-164`), and so does K2's device tree (`qt_widgets/devicetreeview.cpp:101-107`), so the card was the only place in either version where an empty catalog looked like a catalog with unknown contents. This row closes that inconsistency; the user requested it on 2026-09-12. It applies to **Catalog** devices only — Storage and Virtual cards keep today's behaviour, which is deliberately left unchanged and MUST NOT be widened without a further row. | [Planned] |
| DVP-F11 | The **Device tree** — the view labelled `All devices` since `DVP-F30` — has a Table rendering, reached by the same Cards / Table control as the other two views. In Table mode it shows K2's device-tree columns, in K2's order: **Name, Device Type, Active, ID, Parent ID, External ID, Number of files, Total Size, Used space, Free space, Total space, Date updated, Path, Group ID** (`qt_widgets/mainwindow_tab_device_pr.cpp:1227-1353`, `qt_widgets/devicetreeview.cpp`). Cell formatting, row colouring and alignment follow `DVP-F5`, which already matches K2. The hierarchy remains visible: each row is indented by its depth, and a row that has children carries an expand / collapse control. That control follows the shared rule `CDT-F3` (`SpecCardsAndTables.md`): *(amended 2026-09-13)* it becomes a control with hover and press feedback and a full-size click target, in place of the bare icon with a small hit area it was first built with — the aesthetic preference for a light-looking cell was traded against the affordance, and the affordance wins. Its symbolic chevrons, its reserved space on childless rows and its indent unit are unchanged and are now stated once, in `CDT-F3`. | [Planned] |
| DVP-F12 | In the Device tree, `Full Table` toggles **column visibility only and never the row set**, as in K2. Unticked it hides **Device Type, Active, ID, Parent ID, External ID and Group ID**; **Used space, Free space and Total space are always shown**, ticked or not. No device disappears from the tree when `Full Table` is unticked. | [Planned] |
| DVP-F13 | The Device tree in Table mode opens **fully expanded**, as K2 does (`expandAll()` on every load, `qt_widgets/mainwindow_tab_device_pr.cpp:1353`). Collapsing a row hides its descendants. The expanded / collapsed state is **transient**: it is not written to the collection `.ini` nor to the application settings, and every fresh load returns to fully expanded. K2 persists no collapse state either, so this matches it. | [Planned] |
| DVP-F14 | Sorting the Device tree sorts **children within their own parent** and leaves the hierarchy intact — the same result K2 gets from sorting through a proxy over a tree model. Sorting MUST NOT flatten the tree into one ordered list of devices, nor move a device out from under its parent. The initial sort is the **Name column, ascending**, as K2 sets it (`:1311`). | [Planned] |
| DVP-F15 | The Cards / Table choice is **one preference shared by all three views**, stored in the single existing key `Devices/DisplayAsTable` of `DVP-F4`. Switching between the Device tree, the Storage list and the Catalogs list therefore never changes the display mode: a user in Table mode stays in Table mode across all three. The user chose the shared key over a per-view key on 2026-09-12. | [Planned] |
| DVP-F16 | A **Storage** or **Virtual** card shows all three space figures — **used, free, total, in that order** — where it shows only free space today. A **Catalog** card shows none of them: a catalog has no space of its own. The figures are already carried by the existing device list, used space included, derived as total minus free exactly as K2 derives it (`qt_quick/appmanager.cpp:2541-2557`); **no new data and no `core/` change** are authorised, per `DVP-C4`. The user requested this on 2026-09-12. | [Planned] |
| DVP-F17 | The **device name on a Devices card** uses the same font weight, italic and per-type opacity as the device name on the **Selection** page — Virtual bold italic at 0.60, Storage bold at 0.78, Catalog plain at 1.0, in the theme's text colour. The card previously drew the name as a plain heading with none of these rules, which is the difference the user reported on 2026-09-12. | [Planned] |
| DVP-F18 | **Every label on a Devices card wraps rather than being cut short.** This applies to the three that truncate today: the **device name**, the **comment** beside it, and the **detail line** carrying the type, the file figures, the `DVP-F16` space group and the date. A value too long for the card continues on the next line and the card grows taller; none is ended with an ellipsis or clipped to a single line. This applies the app-wide rule `CDT-F1` (`SpecCardsAndTables.md`) to this page, at the user's request on 2026-09-12: tables are the traditional desktop rendering, where eliding is correct, and cards are the phone and tablet rendering, where hidden text is simply unreachable. The **table** cells of this page are unaffected and keep their eliding, per `CDT-F2`. | [Planned] |
| DVP-F19 | The vertical spacing between Devices cards is reduced, so more devices fit on screen, and matches the **Selection** card's spacing. The Selection card already tightened its own top and bottom padding for this reason while leaving left and right alone (`qt_quick/PageSelectionDelegate.qml:14-21`); the Devices card follows it rather than inventing a third value. Requested by the user on 2026-09-12. | [Planned] |
| DVP-F20 | The icon on a Devices card is drawn at the size chosen by the **bigger icon size** setting (`THM-F7`, `SpecTheme.md`): the medium token when it is set, the small-medium token when it is not. | [Planned] |
| DVP-F21 | Cards are indented by their depth **only in the Device tree view**. In the **Storage list** and the **Catalogs list** the cards start flush, because those views do not show the parents the indentation refers to — an indent that points at rows the user cannot see states a relationship the view does not present. | [Planned] |
| DVP-F22 | In the Device tree, **one level of indentation equals the current icon size** of `DVP-F20`, so the indent grows and shrinks with the small / big choice instead of staying at a fixed width. | [Planned] |
| DVP-F23 | A Devices card draws a **separator line between the name row and the detail line below it**, as the Selection card does (`qt_quick/PageSelectionDelegate.qml:144-147`), and shows it only when there is something below the name to separate. | [Planned] |
| DVP-F24 | Each Devices card in the **Device tree view** carries an **expand / collapse button**, matching the Selection card's in every respect: the same `go-down` / `go-up` icons at the small token, the same reused tooltips `Expand` and `Collapse`, and the same always-in-layout placement — a card whose device has no children keeps the button's space, invisible, so every card is the same height. Collapsing a card hides **all** of its descendants, not only its direct children. | [Planned] |
| DVP-F25 | The Devices cards open **fully expanded**, and the expanded / collapsed state is **transient**: never written to the collection `.ini` or the application settings, and reset to fully expanded on every fresh load. This is the same rule the Device tree's table already follows (`DVP-F13`) and the same as K2's `expandAll()`. | [Planned] |
| DVP-F26 | The Devices card's **second line is indented to line up with the device name**, as the Selection card's is (`SEL-C8`): its left margin is the current icon size plus the small spacing, so name and second line align on both pages and at both icon sizes. Today it has no left margin and starts under the icon. Requested by the user on 2026-09-12. | [Planned] |
| DVP-F27 | **No filter-bar control is ever lost when the window narrows.** Today the bar simply clips and controls disappear with no way to reach them. Instead, what does not fit moves into the bar's overflow menu, where it keeps its full text and stays operable, and the bar behaves like the standard page toolbar the rest of the application uses. The order of `DVP-F1` is preserved. Controls collapse in a defined order: the **three view buttons are the last to go**, because the view choice changes *what* the user is looking at, while the display options — Cards, Table, `Full Table` — only change *how* it is drawn and collapse first. The user requested this on 2026-09-12. | [Planned] |
| DVP-F28 | `Full Table` and `Filter from Selection` are drawn as **toggle buttons** — pressed in when on — rather than as tick boxes, which is how the toolbar of `DVP-F27` renders a checkable control. Their labels, their meaning, their stored keys and their defaults are unchanged (`DVP-F4`, `DVP-F9`, `DVP-F12`). The user was shown this appearance change before approving it on 2026-09-12. | [Planned] |
| DVP-F29 | The three view buttons — Device tree, Storage list, Catalogs list — carry an icon: the **Virtual** device icon for the Device tree, the **Storage** icon for the Storage list, and the **inactive Catalog** icon for the Catalogs list. With an icon they can shrink to **icon-only** before folding into the overflow menu of `DVP-F27`, which is the reason the user asked for them on 2026-09-12 — the buttons stay usable at widths where the labels no longer fit. Their collapse ranking is unchanged: they are still the last to leave the bar. | [Planned] |
| DVP-F30 | The three view buttons are labelled **`All devices`**, **`Storage`** and **`Catalogs`** — the Device tree, the Storage list and the Catalogs list respectively. The shorter wording narrows the buttons, which is why the user asked for it on 2026-09-12, supplying the wording and approving it per string against the quoted before / after. The previous labels were `Device tree`, `Storage list` and `Catalogs list`. Only the text changes: the icons of `DVP-F29`, the collapse ranking of `DVP-F27`, the internal view values and the `Devices/DisplayContents` key of `DVP-F7` are all untouched. | [Planned] |

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

**Device tree** (`DVP-F11`, `DVP-F12`) — `Full Table` here hides columns rather
than adding them, matching K2: every column below exists in both states, six of
them only when `Full Table` is ticked.

| | Columns |
|---|---|
| Always shown | Name, Number of files, Total Size, Used space, Free space, Total space, Date updated, Path |
| `Full Table` adds | Device Type, Active, ID, Parent ID, External ID, Group ID |

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
| DVP-C10 | The tree's Table mode is built as **flat rows in the existing table**, with a third column set in the existing table adapter — **not** as a second model adapter or a separate tree-model implementation. The existing adapter's column kinds, widths, `Full Table` handling, cell formatting and row colours are already matched to K2, so the tree reuses them and cannot drift from the other two views; a second adapter is exactly the drift risk this row forbids. The hierarchy is available without new data: the existing device-tree load already returns the rows flattened depth-first with a level (`qt_quick/appmanager.cpp:2504,2549`). Consequently **no `core/` change and no new SQL** are authorised, per `DVP-C4`; needing either is a stop-and-ask. | [Planned] |
| DVP-C11 | Neither switching display mode nor expanding or collapsing a row may trigger a device-list reload or an active-status probe. This extends `DVP-C2` to the tree and keeps `DAS-O4` intact: both operations act on rows already loaded — expand and collapse change only which loaded rows are visible. The restore path of `DVP-C6` is unchanged, and the shared key of `DVP-F15` MUST NOT introduce a reload when the user switches view, beyond the single existing probe that switching view already performs per `DAS-F3`. | [Planned] |
| DVP-C12 | The tree's column headers reuse existing translated wording **byte-for-byte**, per `DVP-C5` — K2's 14 header strings, or their existing K3 equivalents in the table adapter. **No new user-visible string is authorised by these rows**, including for the expand / collapse control, which is icon-only. A K2 header with no existing K3 counterpart is a stop-and-ask for per-string approval, never an invented label. | [Planned] |
| DVP-C13 | The `DVP-F16` space group appears **only when the device has space information at all** — a device reporting no total space shows no space group, not three zeros. This is deliberately **not** the `DVP-F10` rule: an empty catalog is a meaningful fact about a catalog, whereas a Virtual group that aggregates no storage has nothing to report, and three zeros would assert a measurement that was never taken. Within the group, each value is rendered with the **zero-rendering formatter** of `DVP-C9`, so a legitimately zero figure — the free space of a full disk — reads as a zero size instead of leaving its label followed by nothing. **Two** new user-visible strings are authorised for this row and no more: `used` and `total`, lowercase, matching the existing lowercase `free`, which is unchanged and reused. The user was offered the already-translated `Used space` / `Free space` / `Total space` wording at zero translation cost and chose this compact pair at a cost of two slots in 30 languages, per string, on 2026-09-12. | [Planned] |
| DVP-C14 | `DVP-F17` MUST be satisfied by **using the shared device-identity component**, not by copying its per-type font and opacity rules into the card. `SEL-C1` (`SpecSelection.md`) already forbids a second copy of those rules, and a Devices card disagreeing with the Selection card for the same device is the drift this prevents. The component may gain a property to suppress its own icon, defaulting to the present behaviour so its existing callers are untouched. *(The allowance for the card to draw its own larger icon instead is **superseded by `DVP-C20`**, which requires icon and name to come from the shared component now that the two cards must match; the icon-suppressing property survives only for callers that genuinely draw no icon.)* **No new user-visible string** is involved in this row. | [Planned] |
| DVP-C15 | The card's per-type opacity (`DVP-F17`) and the table's row colouring (`DVP-F5`) are **deliberately different renderings and MUST NOT be unified**. Cards mirror the Selection page, where Storage is dimmed to 0.78; the table restores Storage to the normal text colour because it was being drawn in the much lighter *disabled* colour. Both follow K2, which also dims Storage less than Virtual (`#444` against `#666` in a light theme) rather than treating them alike. Reporting either as drift against the other is a misreading of these two rows. | [Planned] |
| DVP-C16 | The device name is drawn by the **shared** device-identity component, which `SEL-C1` and `DVP-C14` require the Selection page, the `SEL-F5` reminder and the Devices card to share. `DVP-F18` MUST therefore be applied through a **property on that component, defaulting to the present non-wrapping behaviour**, exactly as the icon-suppressing property of `DVP-C14` does — only the Devices card sets it. The Selection cards and the reminder MUST NOT start wrapping as a side effect: the user scoped this correction to the Devices page, and the reminder is a single line above a list where wrapping would push the list down as the name grows (`CDT-C4`). Extending it to the Selection page is a separate request, recorded as open in `SpecCardsAndTables.md`. | [Planned] |
| DVP-C17 | The comment's width cap of half the card is **kept**: it stops a long comment from crowding out the device name it annotates (`DCM-C7`, `SpecDeviceComment.md`). `DVP-F18` changes what happens **inside** that cap — the comment wraps within its half of the row instead of being cut short — so the cap and the wrapping are complementary, not alternatives. Removing the cap is not authorised here. **No new user-visible string** is involved in `DVP-F18`, `DVP-C16` or this row. | [Planned] |
| DVP-C18 | The Devices cards' collapse state is **independent** of the Selection page's. Collapsing a branch on one page MUST NOT collapse it on the other. The two show different row sets — the Devices page also filters by Storage / Catalogs and by `Filter from Selection` (`DVP-F9`) — so a shared set would collapse rows the other page never listed, and the Device tree's **table** already keeps its own state (`DVP-F13`). *`DVP-F24` asks the two cards to look alike, not to share state.* If the user later wants one shared collapsed set, that is a **new row**, not an implementation detail to be changed quietly. | [Planned] |
| DVP-C19 | The cards' collapse state is held in the **page**, and the visible cards are derived from the flat device list already loaded: a row "has children" when the next row is one level deeper, and a row is hidden when an ancestor is collapsed — the same one-pass technique as `qt_quick/PageExploreFolders.qml:97-140` and the tree table. **No `core/` change, no new SQL, and no change to the device list adapters** is authorised, per `DVP-C4` and `DVP-C10`. | [Planned] |
| DVP-C20 | With `DVP-F19` to `DVP-F25` the Devices card and the Selection card converge deliberately: the **only** permitted differences are the Devices card's **second-line information** and its **hamburger menu button**. It follows that the Devices card MUST render its **icon and name through the shared device-identity component**, not through a second copy of the icon rule — which amends `DVP-C14`'s allowance for the card to draw its own larger icon, an allowance made before the two cards were required to match. `SEL-C1` (`SpecSelection.md`) forbids a second copy of these rules and is satisfied by this, not strained by it. | [Planned] |
| DVP-C21 | These rows add **no new user-visible string**. `Expand` and `Collapse` already exist in K3 and are reused verbatim; the only string in this batch is the Settings label of `THM-C9`, which is K2's existing one. `DVP-F19` to `DVP-F26` MUST NOT change the Selection page: it is the reference the Devices card is being aligned **to**, and any change to it is a separate request under `SpecSelection.md`. *(Amended 2026-09-12: the user has since made that separate request, and `SEL-F7` and `SEL-C8` now authorise exactly two Selection-page edits — the icon size and the second line's margin. This row continues to forbid every other change to that page, and those two edits are authorised by `SpecSelection.md`, not by these rows.)* | [Planned] |
| DVP-C22 | The separator of `DVP-F23` spans the **full card width**, starting flush with the icon and not at the second line's indent. The Selection card's separator is full width, and `DVP-C20` requires the two cards to match; an indented separator would also read as a divider belonging to the text rather than to the card. | [Planned] |
| DVP-C23 | **No icon is invented.** Icons are taken from the set the application already uses, never designed for this bar. *(Amended 2026-09-12: this row originally left all five of the non-Cards/Table controls icon-less and reserved giving them an icon as a separate request. The user then made that request for the three view buttons, which `DVP-F29` now covers. The mechanism worked as intended.)* `Full Table` and `Filter from Selection` **remain icon-less** and therefore skip the icon-only step and go straight to the overflow menu, where their text is intact and nothing is lost; giving either of them an icon is still a separate request. These rows add, change and delete **no user-visible string**. | [Planned] |
| DVP-C24 | This is the codebase's **first** overflow-capable toolbar. It therefore sets a pattern other K3 pages may follow — but it authorises **only** the Devices filter bar. Converting any other page's bar is a separate request, judged on its own. No new Qt or KDE component is added: the toolkit already in use provides it, per the dependency rule in `CLAUDE.md`. | [Planned] |
| DVP-C25 | `DVP-F27` changes the **bar only**. Every existing visibility rule is preserved exactly — Cards and Table shown when a table view is available, `Full Table` only in Table mode, `Filter from Selection` always — and the band behind the bar is kept, so the page is unchanged at full width. The list and the table are untouched: `DVP-F12`'s *column visibility only, never the row set*, the expand and collapse of `DVP-F13` and `DVP-F24`, and the card rules of `DVP-F19` to `DVP-F26` all continue to hold unmodified. `CDT-F2` is not engaged: it governs eliding inside **table cells**, not toolbar controls. | [Planned] |
| DVP-C26 | `DVP-F29`'s three names are **category icons for a view chooser, not a rendering of any device**, and are written where the toolbar is built. The Catalogs button shows the **inactive** Catalog icon whatever any catalog's state is, which is precisely what a per-device rule must not do — so this is deliberately *not* a third user of the per-type device-icon mapping, even though it names the same three icons. **The duplication is recorded, not hidden:** that mapping already exists twice, in `qt_quick/DeviceIdentity.qml` for the cards and in `qt_quick/adapters/devicetablemodel.cpp` for the table rows, and these three constants sit beside them. They cannot drift silently — nothing recomputes them, and a wrong icon is visible the moment the bar is drawn — whereas the two device mappings can. Unifying all of them is **not** authorised here: it means moving the mapping into C++ and exposing it to QML, a refactor of three files across two languages, and it must be requested in its own right rather than carried in behind an icon change. | [Planned] |
| DVP-C27 | `DVP-F30` is a **deliberate K3-only divergence**. K2 carries the same three labels on its Devices tab (`qt_widgets/mainwindow.ui:5478,5504,5514`) and **keeps them**: the user was offered renaming in both versions or in K3 alone and chose **K3 alone** on 2026-09-12. K2 MUST NOT be edited, per `DVP-C3`, and the difference MUST NOT be reported as drift, nor as a breach of the K2 / K3 label-sync rule, which the user has exercised explicitly here. The separate K2 Selection-panel label naming a device tree (`qt_widgets/mainwindow.ui:253`) is **not** part of this rename — it names a tree, not a view chooser — and keeps its wording in both versions. The three old strings stay in the translation files because K2 still uses them, so none is orphaned. | [Planned] |

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


---

## Open, not authorised

| Item | Detail |
|------|--------|
| One source for the per-type device icon | The mapping of device type to icon exists in `qt_quick/DeviceIdentity.qml` (cards) and in `qt_quick/adapters/devicetablemodel.cpp` (table rows); `DVP-F29` adds three like-named constants for the view chooser, which `DVP-C26` explains are a different thing. Unification cannot be done with a QML singleton, because the C++ model cannot read QML: it means putting the mapping in C++, exposing it to QML, and changing three files. Worth doing, **not** authorised, and not to be attempted as a side effect of any card or toolbar work. The user decides whether to request it. |

## Manual test charter

For each row: set up the stated condition, run the operation, confirm the result.

- **DVP-F1** — On the Storage list, switch to Table: the devices render as an aligned table. Switch to the Catalogs list: still Table. Switch to the Device tree: still Table, rendered per `DVP-F11`. Return to the Storage list: still Table.
- **DVP-F1 (placement)** — Confirm the filter bar reads, left to right: the three view buttons, then the display-mode cluster `Cards`, `Table`, `Full Table`, then `Filter from Selection` last. Confirm `Filter from Selection` still filters the list in both modes, and that its checked state is unchanged by the move.
- **DVP-F1 (placement, Device tree)** — Switch to the Device tree view: the display-mode cluster and both separators are hidden, and `Filter from Selection` remains the only control at the right of the bar.
- **DVP-F2** — In Table mode click the *Number of files* header: rows sort ascending by count; click again: descending. Confirm a device with 1000 files sorts above one with 999 — not below it as a text sort would give. Repeat on *Total Size* with sizes spanning KB / MB / GB, and on *Storage ID*.
- **DVP-F3 (Storage)** — On the Storage list in Table mode with `Full Table` off, confirm exactly the default column set. Tick `Full Table`: Active, Type, Brand, Model, Serial Number, Build Date and Comment 1/2/3 appear. Open the same collection in K2 and confirm the same columns appear there.
- **DVP-F3 (Catalogs)** — On the Catalogs list in File mode with `Full Table` on, confirm Active, Catalog ID and App Version appear and that **Date Loaded and File Path do not**. Open the same collection in Memory mode: both now appear.
- **DVP-F3 (string)** — Confirm the option reads exactly `Full Table` and, with the interface in French, that it is translated — proving the existing K2 string was reused and no new slot was spent.
- **DVP-F4** — Set Table + `Full Table` on, close the application, reopen: both are restored. Open a second collection: its own stored values apply, not the first collection's. Confirm `Devices/DisplayAsTable` and `Devices/DisplayFullDeviceTable` are written to the collection `.ini`, not to the default application settings.
- **DVP-F4 (shared key)** — Tick `Full Table` in K3, then open the same collection in K2: K2's `Full Table` checkbox is ticked. Untick it in K2 and reopen in K3: it is unticked.
- **DVP-F5** — In Table mode confirm, against the same collection open in K2: Storage rows bold, Virtual rows bold italic; **Virtual rows greyed and Storage rows in the normal text colour**; the correct icon per device type, and `media-optical-blu-ray` only while the Catalog is active; a tick in Active and Hidden rather than a word; sizes and counts locale-formatted and right-aligned.
- **DVP-F5 (Storage colour)** — Put the Devices page in Table mode beside the Selection page and compare the same Storage device on both: the two are the same colour, neither lighter than the other. Confirm a Virtual row is still visibly lighter than a Storage row, so the two types remain distinguishable. Repeat in a dark colour scheme.
- **DVP-F5 (Parent storage)** — On the Catalogs list in Table mode, confirm the **Parent storage** column shows the parent storage's **name**. Open the same collection in K2 and confirm that column is empty there. Both are correct; the difference is the approved divergence, not a defect.
- **DVP-F6** — Right-click a row in Table mode and compare the menu entry by entry with the same device's card menu in Cards mode: identical entries, identical enabling conditions. Trigger a destructive entry from the table and confirm the same confirmation dialog appears.
- **DVP-C1** — Confirm Cards is what a collection with no stored setting opens in, and that a device comment still shows beside the name on its card. Confirm no table column shows the comment.
- **DVP-C2** — Unmount a device from a terminal without leaving the Devices page, then toggle Cards/Table and toggle `Full Table`. The active status does not change, proving no probe ran. Now switch the type filter: the status updates, per `DAS-F3`.
- **DVP-C2 (network mount)** — With an unreachable network mount in the collection, toggle the display mode repeatedly. The interface does not freeze.
- **DVP-C4** — Search `qt_quick/` for `QSqlQuery`: no new occurrence. Confirm by inspection that the extra columns come from `Storage::loadStorage()` and `Catalog::loadCatalog()` and that `core/` is unchanged.
- **DVP-C5** — Run `ninja translations_lupdate`. Exactly two new untranslated strings appear for the Devices page, `Cards` and `Table`; every column header resolves to an existing K2 translation.
- **DVP-F7** — Select the Storage list, close the application, reopen: it opens on the Storage list. Repeat for the Catalogs list and for the Device tree. Open a second collection: its own stored view applies, not the first collection's. Confirm `Devices/DisplayContents` is written to the collection `.ini`, not to the default application settings.
- **DVP-F7 (shared key)** — Select the Catalogs list in K3, then open the same collection in K2: `Devices_radioButton_CatalogList` is checked. Select the Storage list in K2 and reopen in K3: the Storage list is shown.
- **DVP-F7 (tree rendering)** — With `"Tree"` stored, reopen: the Device tree is shown, rendered in the display mode stored by `DVP-F15`'s shared key — Cards if Cards was last chosen, Table if Table was.
- **DVP-F8** — Delete the `Devices/DisplayContents` key from a collection `.ini` and open that collection in K3: the Device tree is shown. Open the same collection in K2: K2 shows the Catalogs list. Both are correct, not a defect.
- **DVP-F9** — Open a collection whose `.ini` has no `Devices/FilterFromSelection` key (a new collection, or delete the key from an existing one). On the Devices page `Filter from Selection` is **checked**, and the list is scoped to the device selected on the Selection page rather than showing the whole collection.
- **DVP-F9 (stored value wins)** — Set `Devices/FilterFromSelection` to `false` in a collection `.ini` and open it: the checkbox is **unchecked** and the list shows all devices. Set it to `true` and reopen: checked. A collection that had already stored a choice is unaffected by the new default.
- **DVP-F9 / DVP-C8 (no seeding)** — Open a collection with the key absent, visit the Devices page, and close the application **without touching the checkbox**. The key is still absent from the collection `.ini` — the default was applied on read, not written. Untick the checkbox and close: now the key is present and reads `false`.
- **DVP-F10** — Create or open a catalog containing no files and look at its card in Cards mode: the detail line shows `0 files` and a zero size, not a line with the figures missing. Confirm the same catalog reads the same zero figures in Table mode and, opened in K2, in the device tree.
- **DVP-F10 (non-empty unchanged)** — A catalog with files shows exactly the count and size it shows today, with no change of wording, separator or order.
- **DVP-F10 (other types unchanged)** — A Storage card and a Virtual card with a zero file count look exactly as they do today; the zero figures are not added to them.
- **DVP-C9** — With the interface in French, confirm the empty catalog's size renders as the localised zero rather than blank. Run `ninja translations_lupdate`: no new untranslated string appears for the Devices page.
- **DVP-F11** — Switch to the Device tree and to Table mode: the devices render as an aligned table with the 14 K2 columns in K2's order. Open the same collection in K2 and compare column by column. Confirm each row is indented by its depth and that rows with children carry an expand / collapse control.
- **DVP-F11 (cards kept)** — Switch back to Cards on the Device tree: the previous card rendering is intact, indentation included.
- **DVP-F11 (disclosure control)** — In the Devices tree table, hover a chevron: it gives hover feedback and the click target covers the whole control, not a few pixels. Click a row with no children's position: nothing happens and the neighbouring names remain aligned. Compare the same control with the Explore tree's, per `CDT-F3`: they match.
- **DVP-F12** — In the Device tree in Table mode, untick `Full Table`: Device Type, Active, ID, Parent ID, External ID and Group ID disappear, while Used, Free and Total space remain. **Count the rows before and after: the number is identical** — no device is hidden. Tick it again and compare the visible columns with K2's tree with `Full Table` ticked.
- **DVP-F13** — Open the Devices page on the Device tree in Table mode: every branch is expanded. Collapse a branch, switch to the Catalogs list and back: expanded again. Close and reopen the application: expanded again, and no expand / collapse key appears in the collection `.ini`.
- **DVP-F13 (collapse)** — Collapse a branch with several levels beneath it: all of its descendants disappear, not just its direct children. Expand it: they return in their original order.
- **DVP-F14** — Sort the Device tree by *Number of files* descending: within each parent the children are reordered, every device still sits under its own parent, and no device has moved to the top level. Compare with the same sort in K2's tree. Then reopen the page: the sort is Name ascending.
- **DVP-F15** — In Table mode on the Storage list, switch to the Device tree: still Table. Switch to the Catalogs list and back: still Table. Set Cards on the tree, switch to the Storage list: Cards. Confirm one `Devices/DisplayAsTable` value in the collection `.ini`, not one per view.
- **DVP-C10** — Search `qt_quick/` for `QSqlQuery`: no new occurrence, and `core/` is unchanged. Confirm by inspection that the tree uses the existing table adapter with a third column set, not a second adapter.
- **DVP-C11** — Instrument the active-state probe. With an unreachable network mount in the collection, toggle Cards / Table on the tree and expand and collapse several branches: no probe runs and the interface does not freeze. Switch view: exactly one probe, as `DAS-F3` allows.
- **DVP-C12** — Run `ninja translations_lupdate`: no new untranslated string appears for the Devices page. With the interface in French, confirm all 14 tree headers are translated.
- **DVP-F16** — Look at a Storage card: used, free and total appear, in that order, after the existing parts of the detail line. Compare each figure with the same device's Used / Free / Total space columns in Table mode: they match. Repeat on a Virtual card that aggregates storage.
- **DVP-F16 (catalog)** — A Catalog card shows no space figures at all, whatever its parent storage reports.
- **DVP-C13 (no space info)** — A Virtual device that aggregates no storage, or a Storage device reporting no total space, shows **no** space group — not `0 bytes` three times.
- **DVP-C13 (zero free space)** — On a device whose free space is genuinely zero, the free figure reads as a zero size; its label is not left followed by a blank.
- **DVP-C13 (strings)** — Run `ninja translations_lupdate`: exactly two new untranslated strings appear for the Devices page, `used` and `total`. Confirm `free` is unchanged and still resolves to its existing translation with the interface in French.
- **DVP-F17** — Put the Devices page in Cards mode beside the Selection page and compare the same device on both: the name is the same weight, the same italic and the same shade. Repeat for a Virtual, a Storage and a Catalog device.
- **DVP-C14** — Confirm by inspection that the card renders the name through the shared component and that the per-type weight, italic and opacity rules appear in that one file only. Confirm the Selection page and the selected-device reminder are unchanged, including their icons.
- **DVP-C15** — Compare a Storage device's card with its row in Table mode: the card name is slightly dimmed, the table row is not. Both are correct.
- **DVP-F18 (detail line)** — Narrow the window until a Storage card's detail line no longer fits on one line: it wraps onto a second line and the card grows taller. No ellipsis appears, and the date at its end is still readable.
- **DVP-F18 (name)** — Select a device with a very long name: the name wraps over as many lines as it needs and is readable in full. Confirm the same card in Table mode still elides that name in the Name column — both are correct.
- **DVP-F18 (comment) / DVP-C17** — Give a device a long comment. On its card the comment wraps inside its half of the row; it is not cut short, and it does not push the device name out or overrun it.
- **DVP-C16** — After the change, open the Selection page: its cards and the selected-device reminder above the list are **unchanged**, still one line, and the list below the reminder does not move when a long-named device is selected.
- **DVP-F19** — Put the Devices page in Cards mode beside the Selection page and compare the gap between two cards: they match. Count how many devices fit on one screen before and after the change: more fit after.
- **DVP-F20 / THM-F7** — Tick *Use bigger icon size* in Settings: the Devices card icons grow. Untick it: they shrink back. Confirm the same setting moves K2's tree icons, proving the shared key.
- **DVP-F21** — In the Storage list and in the Catalogs list every card starts at the same left edge, with no indentation. Switch to the Device tree: the indentation returns and reflects each device's depth.
- **DVP-F22** — In the Device tree, note the indent of a second-level device, then toggle the icon size: the indent changes with it, staying one icon wide per level.
- **DVP-F23** — A Devices card shows a separator line between the name row and the detail line, matching the Selection card. On a card with nothing below the name, no stray line is drawn.
- **DVP-F24** — In the Device tree, collapse a branch with several levels beneath it: every descendant disappears, not just the direct children. Expand it: they return in order. Confirm the button matches the Selection card's icons and tooltips, and that a device with no children still occupies the same card height as one with children.
- **DVP-F25** — Collapse a branch, switch to the Catalogs list and back: expanded again. Restart the application: expanded again, and no expand / collapse key appears in the collection `.ini`.
- **DVP-C18** — Collapse a branch on the Devices page, then open the Selection page: the same branch is still expanded there. Collapse it on the Selection page and return: the Devices page keeps its own state.
- **DVP-C20** — Put the two cards side by side for the same device: icon, name, weight, italic, opacity, spacing and separator all match; the only differences are the Devices card's second line and its hamburger button. Confirm by inspection that the icon rule exists in the shared component only.
- **DVP-C21** — Run `ninja translations_lupdate`: the only new string from this batch is the Settings label, and it resolves to an existing K2 translation. Confirm the Selection page is unchanged in every other respect.
- **DVP-F26** — On a Devices card, confirm the second line starts exactly under the first character of the device name. Toggle the icon size: the alignment holds at both sizes. Put the card beside a Selection card: the two second lines start at the same offset.
- **DVP-C22** — Confirm the separator runs the full width of the Devices card, matching the Selection card's, and does not start at the second line's indent.
- **DVP-F27 (narrow window)** — Narrow the window in steps until the bar can no longer hold everything. At every width, **every** control is still reachable: what left the bar is in the overflow menu, with its full text, and operating it there has the same effect as before. Nothing is clipped or silently gone.
- **DVP-F27 (collapse order)** — Keep narrowing: the display options go first, and the three view buttons are the last to leave the bar. At the narrowest width the view choice is still usable, from the bar or from the menu.
- **DVP-F27 (order)** — At full width the bar reads, left to right: the three view buttons, then Cards, Table, `Full Table`, then `Filter from Selection` — the order of `DVP-F1`, unchanged by the new widget.
- **DVP-F28** — Confirm `Full Table` and `Filter from Selection` appear pressed in when on and unpressed when off, that their labels are unchanged, and that toggling them still writes `Devices/DisplayFullDeviceTable` and `Devices/FilterFromSelection` and still behaves per `DVP-F12` and `DVP-F9`.
- **DVP-C23** — Run `ninja translations_lupdate`: no new untranslated string appears. Confirm no icon has been added to the five icon-less controls, and that each of them is legible by text in the overflow menu.
- **DVP-F29 (icon-only)** — Narrow the window until the three view buttons lose their labels: each shows its icon alone, and the three are **distinguishable from one another** at that size — the Device tree, the Storage list and the Catalogs list do not read as the same button. Confirm the checked one is still visibly the checked one, and that clicking each still switches view.
- **DVP-F29 (icons)** — At full width, confirm the Device tree carries the Virtual device icon, the Storage list the Storage icon, and the Catalogs list the **inactive** Catalog icon — the latter unchanged whether or not any catalog in the collection is active.
- **DVP-F29 / DVP-F27** — Keep narrowing past the icon-only step: the three view buttons are still the last controls to fold into the overflow menu.
- **DVP-F30** — At full width the three buttons read `All devices`, `Storage` and `Catalogs`. Each still switches to the view it names, and the stored `Devices/DisplayContents` value is unchanged by the rename — a collection storing the Storage list still reopens on it.
- **DVP-F30 (width)** — Compare the bar's width before and after: the three buttons are narrower, and more of the bar survives before anything folds into the overflow menu.
- **DVP-F30 (translation)** — Run `ninja translations_lupdate`: exactly **one** new untranslated string appears, `All devices`. With the interface in French, `Storage` and `Catalogs` are already translated, proving both were bridged from existing entries rather than newly created.
- **DVP-C27** — Open the same collection in K2: its Devices tab still reads `Device tree`, `Storage list` and `Catalogs list`, and the Selection panel's own device-tree label is unchanged in both versions. This difference is intended.
- **DVP-C25** — At full width, compare the page with its previous appearance: the band is there and the layout reads the same. Switch to the Device tree and confirm the display options and their separators still hide as before; switch to a table view and confirm `Full Table` still appears only in Table mode.
- **DVP-C6 (start)** — Instrument `Device::updateActiveState()`. With a stored view of `"Storage"`, start the application and open the Devices page: the probes amount to one pass over the devices, not two.
- **DVP-C6 (collection open)** — With the Devices page open and a stored view differing from the one on screen, open another collection: exactly one probe pass runs, and the page ends on the new collection's stored view.
- **DVP-C6 (network mount)** — With an unreachable network mount configured, open a collection whose stored view differs from the current one. The interface does not freeze twice over.
