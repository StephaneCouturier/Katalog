---
id: SpecCardsAndTables
title: Cards, Tables and Trees — Shared Display Conventions
description: The app-wide K3 rules that tables are the desktop rendering and cards the phone and tablet rendering, that a card wraps its text rather than hiding it, and that a tree's disclosure control is the same wherever a tree appears
version: "2.13"
---

# CARDS, TABLES AND TREES — SHARED DISPLAY CONVENTIONS

![Status](https://img.shields.io/badge/Status-Approved-brightgreen) ![Version](https://img.shields.io/badge/Version-2.13-blue) ![Implementation](https://img.shields.io/badge/Implementation-planned-lightgrey)

## Context

K3 pages increasingly offer the same data in two renderings. The Devices page
already does (`SpecDevicesPage.md`), and later pages are expected to follow. Both
renderings were being built without a written answer to the question that decides
every detail of them: *who is each one for?*

The user stated it on 2026-09-12: **tables are for traditional desktop use, cards
are the phone- and tablet-friendly display.** That single sentence settles a
recurring design question. A table has fixed column widths and a horizontal
scroll; cutting a value short with an ellipsis is a reasonable price for keeping
columns aligned, and the user can widen the column. A card has none of that. It
is read on a narrow screen, it has no columns to align to, and there is nothing
to widen — so text cut short on a card is simply text the user cannot reach.

Hence the rule this page exists to record: **a card wraps, a table may elide.**
It is deliberately written here rather than inside one page's spec, because it
governs every card K3 draws, including cards not yet built.

> **Reading the requirement IDs.** Each requirement has a permanent ID.
> IDs are never renumbered or reused; a retired requirement is marked
> `[Removed]`, not deleted. Status is one of
> `[Implemented] / [Planned] / [Backlog] / [Removed]`.

---

## Scope at a glance

**In scope:** what each display mode is for, and the consequence for text that
does not fit — app-wide, for every K3 card and every K3 table; and the **per-row
disclosure control of any tree** K3 draws (`CDT-F3`), added 2026-09-13; and the
**double-click fit of a table column** (`CDT-F4`), added 2026-10-03; and the
**keyboard navigation of a table** (`CDT-F5`–`CDT-F11`), added 2026-10-03; and the
**persisted width of selected table columns** (`CDT-F12`) and the **persisted sort of
the Search results table** (`CDT-F13`), added 2026-10-03.

**Out of scope (non-goals):** which pages offer a display choice at all, and how
that choice is stored — each page's own spec decides that (`DVP-F1`, `DVP-F15`
for the Devices page); the column sets of any table; K2, which has no cards and
is in maintenance mode; and the visual styling of cards — spacing, elevation and
colour, which belong to `SpecTheme.md` and to the Kirigami defaults.

**Applies to:** K3 (`qt_quick`) only.

---

## Operational requirements — *why / for whom*

Goals in real use, independent of how they are built.

| ID | Requirement | Status |
|----|-------------|--------|
| CDT-O1 | A desktop user gets a dense, column-aligned table; a phone or tablet user gets a card list that suits a narrow touch screen. Each mode is designed for its own audience rather than being a variation on the other. | [Planned] |
| CDT-O2 | A user reading a card can read **all** of what it says. No value on a card is unreachable because it did not fit. | [Planned] |
| CDT-O4 | A desktop user who widens or narrows the key columns of a file table to suit their file names and folder depth finds them at that width the next time, instead of resizing them on every visit. Approved by the user on 2026-10-03. | [Planned] |
| CDT-O3 | A desktop user can move through a table's rows, open a row and reach its context menu from the keyboard, without the mouse. Approved by the user on 2026-10-03. | [Planned] |

## Functional requirements — *what the system does*

Observable behaviour that can be triggered and watched.

| ID | Requirement | Status |
|----|-------------|--------|
| CDT-F1 | **Text on a card wraps; it is never hidden.** No label on a card truncates its content with an ellipsis or clips it at a fixed line count. A value too long for the card's width continues on the next line and the card grows taller. | [Planned] |
| CDT-F2 | **Table cells may elide.** A value too long for its column is cut short, because the column has a fixed width the user can adjust and the alignment of the columns is what a table is for. This is the deliberate counterpart of `CDT-F1`, not an oversight, and MUST NOT be "corrected" to match it. | [Planned] |
| CDT-F3 | **A tree's per-row disclosure control is the same wherever a tree appears in K3.** It uses the *symbolic* chevrons — the variants the icon theme ships as disclosure indicators — not the filled navigation arrows; it is a control with **hover and press feedback and a full-size click target**, not a bare icon with a small hit area, because that feedback is what tells the user the chevron is clickable; a row with **no children keeps the control's space**, so names stay aligned down the column; the indent per level is the same unit everywhere; and it carries **no tooltip** (the exception `ICB-C3` to the icon-button tooltip rule, `SpecIconButtons.md`). The two trees that exist today — the Explore directory tree and the Devices tree table — each adopt the better half of what they had: Explore takes the symbolic chevrons, the Devices table takes the proper control. The user approved this two-way alignment on 2026-09-13; the divergence was historical, not a decision. | [Planned] |
| CDT-F4 | **Double-clicking a column divider fits the column.** In any K3 table with a column header, double-clicking the divider between two header cells resizes the column to the **left** of that divider to the width of its widest content: its header text or the widest value among the rows currently loaded. This is K2's default `QHeaderView` behaviour. The fitted width is not persisted, except for the columns named in `CDT-F12`; for all other columns it lasts until the user resizes the column again or the view is rebuilt. Cells that still do not fit keep eliding (`CDT-F2`). Approved by the user on 2026-10-03. | [Planned] |
| CDT-F5 | **A table takes keyboard focus** when one of its rows is clicked or when the user tabs into it. The selected row is the keyboard's current row. If no row is selected, the first navigation key selects the first row. | [Planned] |
| CDT-F6 | **Arrow, page and end keys move the selection.** Up/Down move it by one row, PageUp/PageDown by one visible page, Home/End to the first/last row. The view scrolls so the selected row is always visible. This is K2's default `QTreeView` behaviour. | [Planned] |
| CDT-F7 | **Enter/Return performs the selected row's existing left-click action**: on Explore and Search results it opens the file or navigates into the folder. On the Devices page and the Backup preview it does nothing, because they have no row-open action. **Deliberate divergence from K2**, where Enter does nothing (`activated()` is not connected). Approved by the user on 2026-10-03. | [Planned] |
| CDT-F8 | **The Menu key and Shift+F10 open the selected row's existing context menu** on Explore, Search results and the Devices page (Table mode), as K2's custom context menu does. The menu opens at the selected row. The Backup preview has no row menu and is unaffected. | [Planned] |
| CDT-F9 | **Type-ahead:** typing printable characters in quick succession selects the next row whose Name starts with the typed text, ignoring case, and scrolls it into view. Name is the Name column on Explore, Search results and Devices, and File Name on the Backup preview (a small divergence from K2, which matches column 0 — Status there; approved by the user on 2026-10-03). Otherwise the same behaviour as K2's `keyboardSearch`. | [Planned] |
| CDT-F10 | **In the Devices tree (Table mode, `DVP-F11`), Left and Right behave as in K2's tree view.** Left collapses an expanded row; on a collapsed row or a leaf it moves the selection to the parent row. Right expands a collapsed row; on an expanded row it moves the selection to its first child. Expanding and collapsing have the same effect as the disclosure control (`CDT-F3`, `DVP-F13`). Approved by the user on 2026-10-03. | [Planned] |
| CDT-F12 | **Selected column widths persist per collection.** On Search results, the widths of Name, Directory and Catalog Name are saved whenever the user changes them, by dragging a divider or by the `CDT-F4` double-click fit. On the Explore file list, the width of Name is saved the same way. A saved width is restored as that column's default width the next time the table is shown, including after the `CDT-F4` reset to default. All other columns keep their built-in defaults and are not persisted. **Beyond K2**, which persists no column width; approved by the user on 2026-10-03. | [Planned] |
| CDT-F13 | **The Search results sort persists per collection.** The sort column and order of the Search results table are restored the next time the table is shown, from the same collection-settings keys K2 uses (`Search/lastSearchSortSection`, `Search/lastSearchSortOrder`), with the same column indices as K2, so a sort chosen in one version is restored as the same sort in the other. Mirrors `EXP-F18` for Explore. *(Ratifies behaviour that already existed but had never been authorised; approved by the user on 2026-10-03.)* | [Planned] |
| CDT-F11 | **In a flat table** (every table except the Devices tree), Left and Right scroll the table horizontally, as in K2. Approved by the user on 2026-10-03. | [Planned] |

## Constructional requirements — *how it is built / limits / MUST-NOTs*

Boundaries and implementation constraints, not user-visible behaviour.

| ID | Requirement | Status |
|----|-------------|--------|
| CDT-C1 | `CDT-F1` binds **every** K3 card, including cards added later. A new card delegate MUST NOT introduce eliding or a fixed line cap; a width cap that protects the layout is allowed only if the capped text wraps **inside** that cap. | [Planned] |
| CDT-C2 | These rows add **no** user-visible string. Wrapping changes the shape of existing text, never its wording. | [Planned] |
| CDT-C3 | Applying `CDT-F1` to a card that already exists is a change to that page and MUST be authorised by that page's own spec before it is made. `CDT-F1` states the rule; it does not by itself authorise editing any particular file. The Devices page correction is authorised by `DVP-F18` and `DVP-C16` (`SpecDevicesPage.md`). | [Planned] |
| CDT-C4 | A component **shared** between a card and a non-card context MUST NOT be made to wrap unconditionally. The selected-device reminder of `SEL-F5` is a single line above a list, where wrapping would push the list down as the name grows; such a caller keeps its present behaviour until the user asks otherwise. The wrapping is therefore a property of the caller, defaulting to the existing behaviour. | [Planned] |
| CDT-C5 | `CDT-F3` aligns the **per-row control only**. It MUST NOT be "completed" by giving every tree the same **bulk** controls: the Explore tree's four header controls (`EXP-F9`, `SpecExplore.md`) exist because a folder tree is arbitrarily deep, while the device tree is at most three levels, so a page having them and another not is a difference in the **data**, not an inconsistency to be ironed out. Adding them anywhere else is a separate request. | [Planned] |
| CDT-C6 | `CDT-F4` MUST NOT trigger the header's single-click action: a click or double-click on a divider never sorts, and never changes or stores the sort order (`DVP-F2`, `DVP-F14`, `EXP-F18`). Divider dragging (`BKP-F15`) is unchanged. In a tree table the fitted width of the name column includes the row's indent, disclosure control and icon (`CDT-F3`, `DVP-F11`). The change adds no user-visible string, changes nothing under `core/`, persists nothing (column widths: see `CDT-F12`) and creates no new file. It applies to the existing tables of Explore, Search results, the Devices page (Table mode, all three views) and the Backup preview. Approved by the user on 2026-10-03. | [Planned] |
| CDT-C9 | `CDT-F12` storage: the collection settings file (`collection->settingsFilePath`), keys `Search/ColumnWidthName`, `Search/ColumnWidthDirectory`, `Search/ColumnWidthCatalog`, `Explore/ColumnWidthName`. A missing, non-numeric or non-positive value falls back to today's default. A value of 0 MUST NOT be stored or restored, because width 0 is how K3 hides a column. K2 MUST NOT read or write these keys and is unchanged. No user-visible string, no `core/` change, no new file. Sort, selection and expansion persistence are unaffected (`CDT-C6`, `CDT-C8`, `EXP-C2`, `EXP-F18`, `CDT-F13`). Approved by the user on 2026-10-03. | [Planned] |
| CDT-C7 | Table keyboard handling MUST NOT accept `Esc`. `Esc` keeps its `KBS-F1` behaviour while a table has focus, and an open context menu keeps its own `Esc` (`KBS-F4`). Keys never change the sort order (`CDT-C6`). Key-driven expand/collapse obeys `DVP-C11`: no reload, no active-status probe. | [Planned] |
| CDT-C8 | `CDT-F7` and `CDT-F8` MUST call the existing row click and context-menu handlers. They MUST NOT duplicate or redefine them, and they add no menu entry. The change adds no user-visible string, changes nothing under `core/`, persists nothing (neither the selected row nor any expansion), creates no new file and leaves K2 unchanged (`DVP-C3`, `KBS-C5`). It applies to the tables of Explore (file list only), Search results, the Devices page (Table mode, all three views) and the Backup preview. The Explore directory tree and the Selection device list are not covered. | [Planned] |

---

## Open, not authorised

| Item | Detail |
|------|--------|
| Selection page cards | The Selection page is a card list and so falls under `CDT-F1`, but the user scoped the 2026-09-12 correction to the Devices page. Applying it to the Selection cards, and deciding what the `SEL-F5` reminder should do, is a **separate request** the user has not made. It MUST NOT be done as a side effect of the Devices work. |

---

## Manual test charter

- **CDT-F1** — On a card list, narrow the window until a long value no longer
  fits. The value wraps onto a second line and the card grows; no ellipsis
  appears and no text is lost. Read the card on a phone-sized window and confirm
  every value is legible in full.
- **CDT-F2** — In the same data in Table mode, narrow a column until a value no
  longer fits: it is cut short with an ellipsis, and widening the column reveals
  it again. This is correct, not a defect.
- **CDT-C1** — Inspect any newly added card delegate: no `elide`, no fixed
  `maximumLineCount`. Where a width cap exists, confirm the capped text wraps
  within it.
- **CDT-C4** — Confirm the selected-device reminder above the Selection list
  still occupies one line, and that the list below it does not move when a
  device with a very long name is selected.
- **CDT-F3 (side by side)** — Open the Explore directory tree and the Devices tree table and compare a row that has children: the same chevron shape in the same two states, the same indent per level, and the same hover and press feedback in both. Neither shows a tooltip.
- **CDT-F3 (childless rows)** — On **both** pages, find a row with no children: its name starts at the same left edge as the name of a sibling that does have children. The column of names is straight; no row is shifted left by a missing control.
- **CDT-F3 (click target)** — On both pages, click just inside the edge of the chevron rather than its centre: the row expands. The pointer feedback appears before the click, on hover.
- **CDT-C5** — Confirm the Explore tree still has its four header controls and that the Devices page has **not** acquired them.
- **CDT-F4** — In each of Explore, Search results, Devices (Table mode: Storage, Catalogs, All devices) and the Backup preview, narrow a column until a value is cut short, then double-click the divider to its right. The column widens to show its widest loaded value or its header, whichever is wider, and no ellipsis remains in it. Double-click a divider of a column wider than its content: the column shrinks to fit. Reopen the page: columns other than those of `CDT-F12` are back to their defaults.
- **CDT-F5** — On each of the four tables, click a row, then press Down: the next row is selected. Tab into a table with no selection, then press Down: the first row is selected.
- **CDT-F6** — On a list longer than the window, use Down/Up, PageDown/PageUp, End and Home. The selection moves by one row, one page, to the last row and to the first row, and the selected row always stays on screen.
- **CDT-F7** — On Search results and Explore, select a folder row and press Enter: it navigates as a left click does. Select a file row and press Enter: it opens. On Devices and the Backup preview, Enter does nothing.
- **CDT-F8** — On Search results, Explore and Devices, select a row and press the Menu key, then Shift+F10. The row's context menu opens at that row. On the Backup preview, nothing opens.
- **CDT-F9** — Type the first letters of a name further down: that row is selected and scrolled into view. On the Backup preview, matching uses File Name.
- **CDT-F10** — In the Devices tree in Table mode, select an expanded parent and press Left: its descendants hide; press Left again: the selection moves to its parent. Select a collapsed parent and press Right: its children appear; press Right again: the selection moves to its first child. The list does not reload.
- **CDT-F11** — In a wide flat table, press Right and Left: the table scrolls horizontally; the selected row does not change.
- **CDT-F12** — On Search results, resize Name, Directory and Catalog Name (one by drag, one by double-click fit), close and reopen Results, then re-run a search: the three widths are kept. Resize Size: on reopen it is back to its default. On Explore, resize Name and reopen Explore: the width is kept. Open another collection: its own widths (or the defaults) apply.
- **CDT-F13** — Sort Search results by Date descending, close and reopen Results: still sorted by Date descending. In a release build sharing one settings file, open K2's Search: the same sort.
- **CDT-C9** — Start K3 with a beta settings file lacking the keys: default widths, no error. Set `Search/ColumnWidthName=0` or `abc`: the default width is used and the column stays visible. Open the same collection in K2: no change in behaviour. `ninja translations_lupdate` shows no new string; the `core/` and `qt_widgets/` diffs are empty.
- **CDT-C7** — With a table focused, press Esc: the page closes (`KBS-F1`). With a row context menu open, Esc closes only the menu. No key changes the sort indicator.
- **CDT-C8** — `ninja translations_lupdate` shows no new source text. `core/` and `qt_widgets/` diffs are empty. Reopen the page: no row is preselected from a previous session.
- **CDT-C6** — Double-click a divider: the sort indicator and row order do not change. Single-click the middle of a header: it still sorts. In the Devices tree, fit the Name column: the deepest row's name is not cut short. Run `ninja translations_lupdate`: no new string appears. `core/` is unchanged.
