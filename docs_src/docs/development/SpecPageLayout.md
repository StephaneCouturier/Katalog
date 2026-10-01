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

**Decision of 2026-10-01 (section body margin).** Requested by the maintainer:
on the Search form, use one common left margin for the body of every section,
so that the body starts aligned with the section title; a checkbox that opens a
body row is left-aligned with the section title checkbox above it (PGL-F10,
PGL-C7).

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
"Add Link" / "Edit Link" form (PGL-F5 to F9); the left margin of section
bodies on the K3 Search form, relative to their section title (PGL-F10,
PGL-C7).

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
| PGL-F10 | On the Search form, the body of every section listed for Search in PGL-F8 (File name; File attributes; File metadata; Folder criteria; Duplicates; Differences) starts at the same left margin as its section title row, and that margin is the same for all six sections. When the first item of a body row is a checkbox (e.g. File attributes "Type", Folder criteria "Tag", the File metadata checkboxes), that checkbox is left-aligned with the section title checkbox above it. Layout nested inside a body (e.g. the device lists of Duplicates and Differences, offset by the label column) keeps its offset relative to the body. The text of a plain label in the left column of a section body ("text", "with", "in", "exclude"; Duplicates "On", "Scope"; Differences "On") is left-aligned with the text of the section title checkbox, i.e. it starts one checkbox width in. *(Amended 2026-10-02.)* | [Planned] |
| PGL-F11 | On the Search form, the left label column is wide enough for the French Folder criteria "Etiquette" checkbox to fit on one line. A longer translation (e.g. German, Latvian) may wrap onto a second line but is never elided (PGL-C2). The column is not widened further. *(User's choice, 2026-10-02.)* | [Planned] |
| PGL-F12 | On the Search form, every left-column label or checkbox is vertically centred on the first line of the fields in its row, as File name "text" already is. This includes File attributes "Size", "Date"; File metadata "Size", "Duration"; Duplicates "On", "Scope"; Differences "On". | [Planned] |
| PGL-F13 | On the Search form, the vertical gap between rows of a section body and between wrapped lines inside a row is the same: Kirigami.Units.smallSpacing, as on the other K3 forms. Consequently the horizontal gap between groups on a wrapped line (e.g. "> 0 Bytes" and "< 1000 GiB") becomes smallSpacing too. | [Planned] |

## Constructional requirements — *how it is built / limits / MUST-NOTs*

| ID | Requirement | Status |
|----|-------------|--------|
| PGL-C1 | The scrollbar's width and position are read from the running style at run time (Breeze on Linux, Fusion on Windows and macOS), never a hardcoded pixel value. | [Planned] |
| PGL-C2 | Fitting is done by layout only (margins, available width). No item is hidden and no text is shortened to make room. | [Planned] |
| PGL-C3 | Adds no user-visible string; no qsTr / tr change. | [Planned] |
| PGL-C4 | K2 (qt_widgets/) is not changed. | [Planned] |
| PGL-C5 | Every K3 scrollable page uses one shared page component, qt_quick/ScrollablePageFitted.qml, carrying PGL-F1/F2; no page carries its own copy. | [Planned] |
| PGL-C6 | A section title keeps its text (same qsTr string) and its colour: the application title colour of THM-F17 (`SpecTheme.md`), which is Kirigami.Theme.linkColor under theme id 0 and Kirigami.Theme.textColor under theme id 2; its weight follows TYP-F4. *(Amended 2026-09-30, twice: was Kirigami.Theme.linkColor under every theme.)* Only the separator changes. No core/ change, no persisted setting, no effect on existing collections or settings files, including beta testers'. | [Planned] |
| PGL-C7 | The left margin of PGL-F10 is defined once on the Search form page and used by all six section title rows and all six section bodies; no section carries its own copy of the value. Its value is the title rows' existing margin (Kirigami.Units.smallSpacing), so the title rows do not move. | [Planned] |
| PGL-C8 | The left padding that PGL-F10 adds to plain labels is the x position, within the section title checkbox control, of the first visible pixel column of the box the running style draws for it. It is measured at run time from an image of the title checkbox's indicator (its control padding and indicator position plus the transparent inset inside the indicator), because no Qt Quick Controls property exposes that inset. It is never a hardcoded pixel value and has no per-style branch. The measurement is taken once, when the title checkbox is first shown. If it cannot be taken, the padding falls back to the indicator's x position within the control. It is defined once on the Search form page, beside the margin of PGL-C7, and sits inside the label, so the label column width does not change. *(Amended 2026-10-02: was the checkbox's own left padding, which is 0 on Breeze while the visible box starts a few pixels in, and still left labels misaligned on Windows.)* *(Removed 2026-10-02 at the user's request, never implemented: plain-label alignment with the checkbox box abandoned; PGL-F10 keeps checkbox alignment only.)* | [Removed] |
| PGL-C9 | The offset of PGL-F10's plain labels is where the section title checkbox's text starts (its content item's x plus that item's left padding), read at run time. It has no hardcoded value and no style branch, is defined once on the Search form page beside the margin of PGL-C7, and is applied as padding inside the label, so the label column, the fields and the Duplicates/Differences device lists do not move. | [Planned] |
| PGL-C10 | The Search form's label column width (Kirigami.Units.gridUnit * 5) and its row gap (PGL-F13) are each defined once on the page, beside the margin of PGL-C7. The centring of PGL-F12 is computed at run time from the actual height of the row's first line of fields, with no hardcoded pixel value. | [Planned] |

---

## Manual test charter

Run with the interface in French, on Linux (Breeze) and on Windows (Fusion).

- **PGL-O1 / F1 / F4**: open Search with the Results beside it, and make the window small enough that Search scrolls. No combo box, checkbox label or separator reaches or passes under the scrollbar, including while hovering the scrollbar; a gap stays visible between content and scrollbar.
- **PGL-F2**: make the window tall enough that the scrollbar disappears: the right margin returns to the normal page padding.
- **PGL-F3**: repeat both checks on Selection, Create, Device Edit, Tags, Backup, Backup mapping and Settings.
- **PGL-C1**: a code search finds no literal scrollbar width.
- **PGL-F8 / F9 (Backup)**: open Backup, then "Add Link" and "Edit Link" on an existing link. "Source", "Target" and "Options" are in the title colour of THM-F17 (link colour under theme 0, text colour under theme 2), regular weight, each followed by a line on the same row. Field labels sit left-aligned in the left column, dimmed like the Create form, fields in the right column, in the same order as before; every field still works (catalog pickers, name, type, directories, strict copy, on conflict, source mode) and saving a link behaves as before.
- **PGL-C3**: `git diff` shows no `qsTr` / `tr` string change.
- **PGL-O2 / F5 to F8 / C6**: open Settings, Search, Create, Tags and Device Edit (for a Catalog, a Storage and a Virtual device), in French and English. Each title listed in PGL-F8, including the first one on the page, shows its text followed by a line on the same row, vertically centred, filling up to the page margin (PGL-F1). There is no full-width separator on its own row above any of these titles. On Create "Global Parameters" and Tags "Current folders and tags", the line stops before the button or combo. On Search the line follows each section checkbox. On Tags, "Current folders and tags" and its line span the full width, with the tag filter combo on the row below, above the "Tag" column header. `git diff` shows no qsTr change and nothing under core/ or qt_widgets/.
- **PGL-F10 / C7**: open Search with every section switched on, in French and English. In each of the six sections, the left edge of the body content lines up with the section title checkbox; the "Type" (File attributes), "Tag" (Folder criteria) and File metadata checkboxes sit exactly under their section title checkbox; all six sections share the same body margin. The device lists in Duplicates and Differences keep their position relative to the label column. A code search shows the margin defined once on the page. `git diff` shows no qsTr change and nothing under core/ or qt_widgets/. The text of the plain labels ("text", "with", "in", "exclude", "On", "Scope") starts under the text of the checkboxes above, on Breeze and on Windows; no label is elided (PGL-C2).
- **PGL-F11 to F13 / C10**: in French, on a normal and then a narrowed Search page: "Etiquette" fits on one line; every left-column label and checkbox is centred on the first line of its fields ("Taille" level with "Largeur"); every gap between rows and between wrapped lines is the same. In German and Latvian, long labels wrap but are never elided. A code search shows the label width and the row gap each defined once on the page.

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
- The width of the Search form label column (a separate request); the left
  margins of forms other than Search.
