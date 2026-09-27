---
id: SpecPageLayout
title: Page Layout — Content and the Vertical Scrollbar
description: Requirements for K3 scrollable pages to keep their content clear of the vertical scrollbar, with a margin read from the running style, through one shared page component.
version: "2.13"
---

# PAGE LAYOUT — CONTENT AND THE VERTICAL SCROLLBAR

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

> **Reading the requirement IDs.** Each requirement has a permanent ID.
> IDs are never renumbered or reused; a retired requirement is marked
> `[Removed]`, not deleted. Status is one of
> `[Implemented] / [Planned] / [Backlog] / [Removed]`.

---

## Scope at a glance

**In scope:** the right margin of every scrollable K3 (`qt_quick/`) page with
respect to its vertical scrollbar: Selection, Search, Create, Device Edit,
Tags, Backup, Backup mapping and Settings.

**Out of scope:** see [Out of scope](#out-of-scope).

---

## Operational requirements — *why / for whom*

| ID | Requirement | Status |
|----|-------------|--------|
| PGL-O1 | On every scrollable K3 page, in any window size and interface language, the page's vertical scrollbar never covers or touches the page content; content and scrollbar stay visibly separate. | [Planned] |

## Functional requirements — *what the system does*

| ID | Requirement | Status |
|----|-------------|--------|
| PGL-F1 | While a page's vertical scrollbar is shown over the page content, the content ends a small gap (Kirigami.Units.smallSpacing) to the left of the scrollbar, taking the scrollbar at its widest (as drawn when hovered). The page's right margin is then the larger of the normal page padding and that width plus the gap. Where the style already places the scrollbar beside the content, the normal page padding is kept. | [Planned] |
| PGL-F2 | When the page's vertical scrollbar is not shown, the page's right margin is the normal page padding, unchanged. The content may reflow by a few pixels as the scrollbar appears or disappears. | [Planned] |
| PGL-F3 | PGL-F1/F2 apply to every K3 scrollable page: Selection, Search, Create, Device Edit, Tags, Backup, Backup mapping and Settings. | [Planned] |
| PGL-F4 | The width left by PGL-F1 is the page width CBX-F2 refers to: no page item, combo box or otherwise, extends into the scrollbar or its gap. | [Planned] |

## Constructional requirements — *how it is built / limits / MUST-NOTs*

| ID | Requirement | Status |
|----|-------------|--------|
| PGL-C1 | The scrollbar's width and position are read from the running style at run time (Breeze on Linux, Fusion on Windows and macOS), never a hardcoded pixel value. | [Planned] |
| PGL-C2 | Fitting is done by layout only (margins, available width). No item is hidden and no text is shortened to make room. | [Planned] |
| PGL-C3 | Adds no user-visible string; no qsTr / tr change. | [Planned] |
| PGL-C4 | K2 (qt_widgets/) is not changed. | [Planned] |
| PGL-C5 | Every K3 scrollable page uses one shared page component, qt_quick/ScrollablePageFitted.qml, carrying PGL-F1/F2; no page carries its own copy. | [Planned] |

---

## Manual test charter

Run with the interface in French, on Linux (Breeze) and on Windows (Fusion).

- **PGL-O1 / F1 / F4**: open Search with the Results beside it, and make the window small enough that Search scrolls. No combo box, checkbox label or separator reaches or passes under the scrollbar, including while hovering the scrollbar; a gap stays visible between content and scrollbar.
- **PGL-F2**: make the window tall enough that the scrollbar disappears: the right margin returns to the normal page padding.
- **PGL-F3**: repeat both checks on Selection, Create, Device Edit, Tags, Backup, Backup mapping and Settings.
- **PGL-C1**: a code search finds no literal scrollbar width.
- **PGL-C3**: `git diff` shows no `qsTr` / `tr` string change.

---

## Out of scope

- K2 (`qt_widgets/`): it is in maintenance mode (PGL-C4).
- Horizontal scrollbars.
- Scroll views inside a page (lists, tables, trees, text areas) other than the
  page's own vertical scrollbar.
- The width rules of combo boxes themselves: see `SpecComboBoxes.md`; this spec
  only defines the page width they fit into (PGL-F4).
