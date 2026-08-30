---
id: SpecSelection
title: Selection — Selected Device
description: Requirements for which device is selected on the Selection page, how the selection changes, and how it is shown
---

# SELECTION — SELECTED DEVICE

![Status](https://img.shields.io/badge/Status-Approved-brightgreen) ![Implementation](https://img.shields.io/badge/Implementation-planned-lightgrey)

## Context

The Selection page carries the collection's device tree and holds the one device
that the rest of the application acts on — the scope of a search, of the Devices
page, of Create and of Statistics. In K2 the same panel is named *Filters*: its
objects are `Filters_*`, its code is `qt_widgets/mainwindow_tab_filters.cpp`
(header: *"methods for the SELECTION panel"*) and its user page is
`Selection.md`. Searching K2 for "Selection" does not find it.

Nothing had ever written down which device is selected, how the selection
changes, or how it is shown. Three consequences were found on 2026-08-30, all of
them silent:

- Restoring a search from history restored every form criterion **except** the
  device the search had been run against, so the restored search ran against
  whatever happened to be selected and produced different results with no error
  and no visible sign.
- A card's context menu acted on the card that was right-clicked while the
  selection — and therefore the highlight — stayed on a different card.
- With a long device list scrolled past the highlighted card, nothing on screen
  said which device was selected.

This page is the only source of requirements for those behaviours.

> **Reading the requirement IDs.** Each requirement has a permanent ID.
> IDs are never renumbered or reused; a retired requirement is marked
> `[Removed]`, not deleted. Status is one of
> `[Implemented] / [Planned] / [Backlog] / [Removed]`.

---

## Scope at a glance

**In scope:** which device is selected; the ways the selection changes — card
activation, the card context menu, and restoring a search from history; the
display of the current selection at the top of the Selection page; and the
display of each history entry's device scope.

**Out of scope (non-goals):** the device tree's collapse and expand state and its
header controls; the ordering and filtering of the device list; the Devices page
and the device editor; connected-drives mode and its directory picker; every
search criterion other than the device scope — those belong to the search form
and, for the text and exclude term lists, to `SpecSearchList.md`. None of them is
governed by this spec and this spec does not authorise changing any of them.

**Applies to:** K3 (`qt_quick`) only. K2 is in maintenance mode and already
satisfies `SEL-F1` and a labelled variant of `SEL-F5`; see `SEL-C6`.

---

## Operational requirements — *why / for whom*

Goals in real use, independent of how they are built.

| ID | Requirement | Status |
|----|-------------|--------|
| SEL-O1 | A user working in a collection with many devices can see which device the application is acting on without scrolling the device list to find the highlighted card. | [Planned] |
| SEL-O2 | A user who re-runs a search from history gets the results the original run produced, because the device it was run against comes back with it. | [Planned] |
| SEL-O3 | A user reading the search history can tell which device each past search covered before choosing one. | [Planned] |
| SEL-O4 | A user acting on a device through its context menu has the action applied to the device pointed at, not to a different one. | [Planned] |

## Functional requirements — *what the system does*

Observable behaviour that can be triggered and watched.

| ID | Requirement | Status |
|----|-------------|--------|
| SEL-F1 | Restoring a search from history restores the device selection the search was run against, in addition to every form criterion. If no device was stored, or the stored device no longer exists in the collection, the selection becomes **All**. | [Planned] |
| SEL-F2 | Each search history entry displays the device its search was run against, so the scope is visible before the entry is chosen. A search run against all devices displays `All`. | [Planned] |
| SEL-F3 | Activating **Search** or **Explore** from a Selection card's context menu selects that card's device before performing the action, so the highlighted card and the action always agree. | [Planned] |
| SEL-F4 | **Edit**, **Update** and **Open folder** MUST NOT change the selection. They act on the right-clicked device while the highlight stays where it was. This is a **deliberate, informed decision** taken on 2026-08-30: the resulting mismatch — the same one `SEL-F3` exists to remove for Search and Explore — was put to the maintainer explicitly for these three items and accepted. It is also a **divergence from K2**, which selects the device when the context menu is *opened* (`qt_widgets/mainwindow_tab_filters.cpp:176-183` calls `on_Filters_treeView_Devices_clicked()` before building the menu), so in K2 every item selects and merely dismissing the menu selects too. K3 selects per item. This MUST NOT be "corrected" to match `SEL-F3`, and MUST NOT be "corrected" back to K2's select-on-menu-open. | [Planned] |
| SEL-F5 | The Selection page shows the currently selected device at the very top, immediately **before** the filter and expand row, rendered as a device card: the type icon and the device name with the per-type font rules. No label, heading or title precedes it. When no device is selected the row shows `All` with the `folder` icon — the same icon the *All* entry already uses elsewhere in K3. The row is **always present**, so the layout never shifts as the selection changes. Deliberate divergence from K2, which labels three separate rows Virtual / Storage / Catalog and fills the two that do not apply with `All`. | [Planned] |
| SEL-F6 | Clicking the `SEL-F5` reminder scrolls the device list so the selected card is visible. It is a navigation affordance only — see `SEL-C7`. | [Planned] |

## Constructional requirements — *how it is built / limits / MUST-NOTs*

Boundaries and implementation constraints, not user-visible behaviour.

| ID | Requirement | Status |
|----|-------------|--------|
| SEL-C1 | The `SEL-F5` reminder and the device cards render their icon and name from **one shared QML component**. The icon rule (`drive-multidisk` for Virtual, `drive-harddisk` for Storage, `media-optical-blu-ray` for an active Catalog and `media-optical` otherwise), the per-type `font.weight` and `font.italic`, and the per-type opacity (0.60 Virtual / 0.78 Storage / 1.0 Catalog) MUST NOT be duplicated in a second file. Two copies would drift apart silently, and the reminder disagreeing with the card it mirrors is the exact failure this row prevents. | [Planned] |
| SEL-C2 | `SEL-F1` MUST NOT change `core/`. **`core/` is already complete and is not at fault.** `Search::loadSearchHistoryCriteria` reads `selected_device_ID_list` at result column index **44** and assigns it to `selectedDeviceIDList` at `core/search.cpp:903-911`; `AppManager::selectDeviceById()` already applies a selection and persists `Selection/SelectedDeviceID`. The defect is downstream: the K3 adapter's `properties()` map (`qt_quick/adapters/search.h:54`) **omits** `selectedDeviceIDList`, so `AppManager::restoreSearchHistory()` returns a map without it and `PageSearchForm.applyHistoryCriteria()` has nothing to apply. The change is confined to those three `qt_quick/` points. *(Recorded because the fault was twice attributed to `core/` before being traced; a future reader should not repeat that.)* | [Planned] |
| SEL-C3 | Only the **first** ID of the stored list is applied, matching K2 (`qt_widgets/mainwindow_tab_search_ui.cpp:674`), because the Selection page holds a single device. The stored comma-separated format, the `search.selected_device_ID_list` column and the database schema MUST NOT change. | [Planned] |
| SEL-C4 | These rows MUST add **zero** new translatable strings. `SEL-F5` carries no label by definition and reuses the existing `All`; `SEL-F2` renders the device name itself, or the existing `All`, as one more bare fragment of the existing history summary line. The K2 column header `Selected Device ID List` labels a raw-ID debug column and MUST NOT be reused. | [Planned] |
| SEL-C5 | The device name of `SEL-F2` MUST be obtained through an existing device accessor. It MUST NOT be obtained by adding a `device` join, or any further raw SQL, to `AppManager::getSearchHistory()` (`qt_quick/appmanager.cpp:1357`), whose existing raw `QSqlQuery` is already a departure from the UI/core boundary. That departure is recorded in `SpecBacklogNotes.md` as reported and unruled; it is neither authorised nor to be deepened here. | [Planned] |
| SEL-C6 | K2 MUST NOT change. K2 already satisfies `SEL-F1` (`qt_widgets/mainwindow_tab_search_ui.cpp:651-693`, including its "device still exists, else All" guard) and shows a labelled variant of `SEL-F5` (`displaySelectedDeviceName()`, `qt_widgets/mainwindow_tab_filters.cpp:449-470`). K2's labelled three-row display, its select-on-menu-open behaviour and its raw-ID history column MUST NOT be reported as drift against these rows. | [Planned] |
| SEL-C7 | The `SEL-F6` click MUST NOT change or clear the selection. Clearing to *All* was considered and **rejected** on 2026-08-30: the reminder sits directly above the filter field, so a misclick would silently widen the scope of the next search with no error and no visible cause — the same class of silent wrong answer that `SEL-F1` exists to remove. The reminder scrolls the list and does nothing else. | [Planned] |

---

## Manual test charter

For each row: set up the stated condition, run the operation, confirm the result.

- **SEL-F1** — Select a Catalog, run a search, then select a different device. Reopen the history and click that entry: the first Catalog is selected again and re-running the search gives the original results. Repeat after deleting the stored device from the collection: the selection falls back to All, the form still restores, and nothing errors.
- **SEL-F1 / SEL-C3** — Restore a history entry saved with no device scope. The selection becomes All rather than staying on the previous device.
- **SEL-F2** — Open the history list. Every entry names the device its search covered; an all-devices search reads `All`. Switch the interface to French: the device name stays as stored and `All` is translated, confirming no new string was created.
- **SEL-F3** — Right-click a card other than the selected one and choose Search. The highlight moves to that card before the Search page opens, and running the search covers that device. Repeat with Explore on a Catalog.
- **SEL-F4** — Right-click a card other than the selected one and choose Edit: the editor opens on that device and the highlight **stays** on the previously selected card. Repeat for Update and for Open folder: each acts on the right-clicked device while the highlight does not move. All three are intended and MUST NOT be reported as defects.
- **SEL-F4 (K2 divergence)** — In K2, right-click a device in the Selection panel and press Escape without choosing anything: the selection has moved to that device. In K3, do the same: the selection has not moved. Both are correct for their version.
- **SEL-F5** — With a long device list scrolled well past the highlighted card, the top of the Selection page shows the selected device as a card, with no label or heading above it, and it sits before the filter and expand row. Select a Virtual, then a Storage, then an active Catalog, then an inactive one: the reminder's icon, boldness, italics and dimming match that device's own card exactly in each case.
- **SEL-F5 (All)** — Reset the selection so no device is selected. The reminder shows `All` with the `folder` icon, and the row keeps the same height, so nothing below it moves.
- **SEL-C1** — Change one styling value in the shared component and confirm the reminder and the cards both follow it. Confirm by inspection that the icon rule, the font rules and the opacity values exist in exactly one file.
- **SEL-F6 / SEL-C7** — Scroll the device list until the selected card is off screen, then click the reminder: the list scrolls to the selected card. Confirm the selection is unchanged — same device highlighted, same device name in the reminder — and that no search scope has widened. Click the reminder repeatedly: nothing changes but the scroll position.
- **SEL-C4** — Run `ninja translations_lupdate`. No new untranslated string appears for the Selection page or the search history list.
- **SEL-C6** — Open the same collection in K2. Its Selection panel still shows the three labelled Virtual / Storage / Catalog rows and its history table still shows a raw ID column. Both are expected and are not defects of these rows.
