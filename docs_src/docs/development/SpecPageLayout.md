---
id: SpecPageLayout
title: Page Layout — Scrollbar Margin and Section Titles
description: Requirements for K3 scrollable pages to keep their content clear of the vertical scrollbar, with a margin read from the running style, through one shared page component, and how section titles are drawn on K3 form pages.
version: "2.13"
---

# PAGE LAYOUT — SCROLLBAR MARGIN AND SECTION TITLES

![Status](https://img.shields.io/badge/Status-Approved-brightgreen) ![Version](https://img.shields.io/badge/Version-2.13-blue) ![Implementation](https://img.shields.io/badge/Implementation-planned-orange)

## Context

**Decision of 2026-09-27.** On a scrollable K3 page, the vertical scrollbar
never covers or touches the page content. Some styles draw the scrollbar over
the content (overlay) and widen it on hover; others reserve room for it beside
the content. The page's right margin is derived from the scrollbar as the
running style actually draws it, so the content always ends a small gap before
it.

**Reflow accepted.** When the scrollbar appears or disappears, the content may
move by a few pixels (PGL-F2). This was accepted as the price of a normal
margin on pages that do not scroll.

**Link with combo boxes.** The width left by this rule is the page width that
`CBX-F2` (`SpecComboBoxes.md`) refers to (PGL-F4).

**Decision of 2026-09-30 (section titles).** Requested by the maintainer: "for
the section titles, to have a line after the text instead of below. (I saw some
Plasma/kirigami app doing that)". Approved after the Settings trial: "I like it,
let's implement on pages with sections: Search, Create, Tags"; the same day
Device Edit, Backup and the (currently disabled) Settings "Search" section were
added. A section title is now one row: the text, then a line filling the rest of
the width. This replaces the full-width separator that was drawn on its own row
above the title (PGL-F5 to F8).

**Decision of 2026-09-30 (after review of the implementation).** The maintainer:
"Tags: the combobox should be on the line below, not same line as title & line."
and "Backup: same design principles to be applied to the form to add a link
(including colors, no bold & line)". The Tags tag filter combo leaves the title
row (PGL-F7, amended). The Backup "Add Link" / "Edit Link" form comes into scope:
it moves from Kirigami.FormLayout to the two-column grid of Create and Settings,
so its section titles follow PGL-F5/F6 (PGL-F8, PGL-F9).

> **Reading the requirement IDs.** Each requirement has a permanent ID.
> IDs are never renumbered or reused; a retired requirement is marked
> `[Removed]`, not deleted. Status is one of
> `[Implemented] / [Planned] / [Backlog] / [Removed]`.

---

## Scope at a glance

**In scope:** the right margin of every scrollable K3 (`qt_quick/`) page with
respect to its vertical scrollbar: Selection, Search, Create, Device Edit,
Tags, Backup, Backup mapping and Settings; the presentation of section titles
on the K3 form pages Settings, Search, Create, Tags, Device Edit and the Backup
"Add Link" / "Edit Link" form (PGL-F5 to F9).

**Out of scope:** see [Out of scope](#out-of-scope).

---

## Operational requirements — *why / for whom*

| ID | Requirement | Status |
|----|-------------|--------|
| PGL-O1 | On every scrollable K3 page, in any window size and interface language, the page's vertical scrollbar never covers or touches the page content; content and scrollbar stay visibly separate. | [Planned] |
| PGL-O2 | On K3 form pages, every section title is set apart from the content above it in the same way, as in other Plasma/Kirigami applications, so all section titles of a page look alike. | [Planned] |

## Functional requirements — *what the system does*

| ID | Requirement | Status |
|----|-------------|--------|
| PGL-F1 | While a page's vertical scrollbar is shown over the page content, the content ends a small gap (Kirigami.Units.smallSpacing) to the left of the scrollbar, taking the scrollbar at its widest (as drawn when hovered). The page's right margin is then the larger of the normal page padding and that width plus the gap. Where the style already places the scrollbar beside the content, the normal page padding is kept. | [Planned] |
| PGL-F2 | When the page's vertical scrollbar is not shown, the page's right margin is the normal page padding, unchanged. The content may reflow by a few pixels as the scrollbar appears or disappears. | [Planned] |
| PGL-F3 | PGL-F1/F2 apply to every K3 scrollable page: Selection, Search, Create, Device Edit, Tags, Backup, Backup mapping and Settings. | [Planned] |
| PGL-F4 | The width left by PGL-F1 is the page width CBX-F2 refers to: no page item, combo box or otherwise, extends into the scrollbar or its gap. | [Planned] |
| PGL-F5 | A section title is drawn as one row: the title text, then on the same line a horizontal separator (Kirigami.Separator) that fills the remaining row width, vertically centred on the text. No separator is drawn on a row of its own above the title. | [Planned] |
| PGL-F6 | The first section title of a page carries the line of PGL-F5 too. | [Planned] |
| PGL-F7 | When a title row carries a trailing control, the line sits between the title and that control: Create "Global Parameters" (Expand/Collapse button). On Search, where the title is a checkbox that switches the section on, the line follows the checkbox. Tags "Current folders and tags" carries no trailing control: its title and line span the full width, and the tag filter combo sits on the row below, in the right-hand column it occupied before (above the "Tag" column header). *(Amended 2026-09-30: the Tags combo was on the title row.)* | [Planned] |
| PGL-F8 | PGL-F5 to F7 apply to these titles: Settings (Collection & Database; Collection Import & Synchronization; Application; Search, when that section is enabled), Search (File name; File attributes; File metadata; Folder criteria; Duplicates; Differences), Create (Catalog definition; Content options; Global Parameters), Tags (Add a tag; Current folders and tags), Device Edit (Device; Location; Content options; Storage details — each shown only for the device types that show it today), Backup "Add Link" / "Edit Link" form (Source; Target; Options). The Backup page itself has no section title. *(Amended 2026-09-30: Backup link form added.)* | [Planned] |
| PGL-F9 | The Backup "Add Link" / "Edit Link" form uses the two-column grid of Create and Settings instead of Kirigami.FormLayout: field labels in the left column, left-aligned, at the dimmed opacity (0.7) of the other forms; fields in the right column. Its fields, their order and their behaviour are unchanged; each label keeps its former text verbatim (no colon added). | [Planned] |

## Constructional requirements — *how it is built / limits / MUST-NOTs*

| ID | Requirement | Status |
|----|-------------|--------|
| PGL-C1 | The scrollbar's width and position are read from the running style at run time (Breeze on Linux, Fusion on Windows and macOS), never a hardcoded pixel value. | [Planned] |
| PGL-C2 | Fitting is done by layout only (margins, available width). No item is hidden and no text is shortened to make room. | [Planned] |
| PGL-C3 | Adds no user-visible string; no qsTr / tr change. | [Planned] |
| PGL-C4 | K2 (qt_widgets/) is not changed. | [Planned] |
| PGL-C5 | Every K3 scrollable page uses one shared page component, qt_quick/ScrollablePageFitted.qml, carrying PGL-F1/F2; no page carries its own copy. | [Planned] |
| PGL-C6 | A section title keeps its text (same qsTr string) and colour (Kirigami.Theme.linkColor); its weight follows TYP-F4. Only the separator changes. No core/ change, no persisted setting, no effect on existing collections or settings files, including beta testers'. | [Planned] |

---

## Manual test charter

Run with the interface in French, on Linux (Breeze) and on Windows (Fusion).

- **PGL-O1 / F1 / F4**: open Search with the Results beside it, and make the window small enough that Search scrolls. No combo box, checkbox label or separator reaches or passes under the scrollbar, including while hovering the scrollbar; a gap stays visible between content and scrollbar.
- **PGL-F2**: make the window tall enough that the scrollbar disappears: the right margin returns to the normal page padding.
- **PGL-F3**: repeat both checks on Selection, Create, Device Edit, Tags, Backup, Backup mapping and Settings.
- **PGL-C1**: a code search finds no literal scrollbar width.
- **PGL-F8 / F9 (Backup)**: open Backup, then "Add Link" and "Edit Link" on an existing link. "Source", "Target" and "Options" are in link colour, regular weight, each followed by a line on the same row. Field labels sit left-aligned in the left column, dimmed like the Create form, fields in the right column, in the same order as before; every field still works (catalog pickers, name, type, directories, strict copy, on conflict, source mode) and saving a link behaves as before.
- **PGL-C3**: `git diff` shows no `qsTr` / `tr` string change.
- **PGL-O2 / F5 to F8 / C6**: open Settings, Search, Create, Tags and Device Edit (for a Catalog, a Storage and a Virtual device), in French and English. Each title listed in PGL-F8, including the first one on the page, shows its text followed by a line on the same row, vertically centred, filling up to the page margin (PGL-F1). There is no full-width separator on its own row above any of these titles. On Create "Global Parameters" and Tags "Current folders and tags", the line stops before the button or combo. On Search the line follows each section checkbox. On Tags, "Current folders and tags" and its line span the full width, with the tag filter combo on the row below, above the "Tag" column header. `git diff` shows no qsTr change and nothing under core/ or qt_widgets/.

---

## Out of scope

- K2 (`qt_widgets/`): it is in maintenance mode (PGL-C4).
- Horizontal scrollbars.
- Scroll views inside a page (lists, tables, trees, text areas) other than the
  page's own vertical scrollbar.
- The width rules of combo boxes themselves: see `SpecComboBoxes.md`; this spec
  only defines the page width they fit into (PGL-F4).
- Column-header underline separators inside lists and tables (e.g. the Tags
  list header, the Backup preview header).
- Separators that do not sit above a section title listed in PGL-F8 (e.g. the
  Device Edit separator above the Storage picture row, the separator inside a
  Backup card).
- Sub-titles that are not section titles (e.g. Device Edit "Exclude folders or
  files", inside Content options).
- The quality-check dialog.
