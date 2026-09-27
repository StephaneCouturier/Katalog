---
id: SpecComboBoxes
title: Combo Boxes — Width and Drop-Down List
description: Requirements for K3 combo boxes to take the width of their widest entry without pushing the page past its edge, and for their open lists to show every entry in full, inside the application window.
version: "2.13"
---

# COMBO BOXES — WIDTH AND DROP-DOWN LIST

![Status](https://img.shields.io/badge/Status-Approved-brightgreen) ![Version](https://img.shields.io/badge/Version-2.13-blue) ![Implementation](https://img.shields.io/badge/Implementation-planned-orange)

## Context

**Decision of 2026-09-27.** A K3 combo box is as wide as its widest entry, and
its open list shows every entry in full, in any interface language. A combo
box that is too narrow cuts its entries; one that pushes the page content past
its edge breaks the layout; a list cut off at the window edge hides entries.

**Forms that align their fields.** Where a form lines its fields up, a combo
box keeps the width its layout gives it; the edge and list rules (CBX-F2 to
CBX-F5) still apply to it (CBX-C7).

**No separate window for the list.** The list stays inside the application
window and scrolls when its entries do not fit (CBX-F5, CBX-F6). Katalog
supports Qt 6.5 as a minimum (CBX-C6).

> **Reading the requirement IDs.** Each requirement has a permanent ID.
> IDs are never renumbered or reused; a retired requirement is marked
> `[Removed]`, not deleted. Status is one of
> `[Implemented] / [Planned] / [Backlog] / [Removed]`.

---

## Scope at a glance

**In scope:** every combo box in K3 (`qt_quick/`), including the device tree
combo box, the small unit combo boxes (size unit, checksum sign) and the
editable Tags combo box.

**Out of scope:** see [Out of scope](#out-of-scope).

---

## Operational requirements — *why / for whom*

| ID | Requirement | Status |
|----|-------------|--------|
| CBX-O1 | In any interface language, the user can read every entry of a combo box, closed or open, and no combo box pushes the page's content past its edge. | [Planned] |

## Functional requirements — *what the system does*

| ID | Requirement | Status |
|----|-------------|--------|
| CBX-F1 | A combo box's natural width is its widest entry (plus its indicator), the same on Breeze (Linux) and Fusion (Windows/macOS). Combo boxes covered by CBX-C7 keep their layout width instead. | [Planned] |
| CBX-F2 | A combo box may be narrower than its natural width only to fit the width available on its page — or its dialog, for a combo box inside a dialog — for example the Search form column while the Results are shown. It never extends past that edge; the displayed text then elides. | [Planned] |
| CBX-F3 | The open list is at least as wide as its combo box. | [Planned] |
| CBX-F4 | The open list is wide enough to show its widest entry in full, capped to the window width; only then are its entries elided. | [Planned] |
| CBX-F5 | The open list opens below the combo box, or above it when there is more room above. When it cannot show every entry, it scrolls. It never shows an entry cut off at the window edge without a way to scroll to it. | [Planned] |
| CBX-F6 | The open list stays inside the application window; it is never a separate native popup window. | [Planned] |
| CBX-F7 | An editable combo box (the Tags page tag field) follows F1–F5; a long typed or selected value elides and never widens the page. | [Planned] |

## Constructional requirements — *how it is built / limits / MUST-NOTs*

| ID | Requirement | Status |
|----|-------------|--------|
| CBX-C1 | Fitting is done by layout — sizing and eliding. No entry string is shortened, reworded or replaced to fit a width (CLAUDE.md user-visible text rule). | [Planned] |
| CBX-C2 | Widths come from the measured text of the entries, which follows the text-size setting (TYP-F8), and from the available width — never from fixed pixel widths. The size-unit and checksum-sign combo boxes lose their 75 / 60 px widths and take their natural width. | [Planned] |
| CBX-C3 | Width and placement changes to the combo boxes with a custom list (FileType ×3, batch action, language, DeviceTree) MUST NOT change their row painting — that stays the separate SpecTheme sweep. | [Planned] |
| CBX-C4 | No combo box inside a card delegate may elide (CDT-C1). | [Planned] |
| CBX-C5 | No exceptions to F1–F5. DeviceTreeComboBox (a button with its own popup) follows F1/F2 for its button — natural width from its widest device name, icon and arrow, shrinking to fit, no fixed pixel width — and applies F3–F5 in its popup. *(Amended 2026-09-27: the button now follows F1/F2 instead of a fixed 200 px width.)* | [Planned] |
| CBX-C6 | Works on the project minimum Qt 6.5: no API newer than 6.5 (in particular not popupType / Popup.Window). | [Planned] |
| CBX-C7 | A combo box that stretches (Layout.fillWidth) or has a set layout width (gridUnit*12 / *24) in a form that aligns its fields keeps that layout width; F2–F5 still apply to it. | [Planned] |
| CBX-C8 | All K3 combo boxes use one shared component, qt_quick/ComboBoxFitted.qml, which carries F1–F5; no call site carries its own fix. | [Planned] |

---

## Manual test charter

Run with the interface in French, on Linux and on Windows.

- **CBX-O1 / F2**: on Search with the Results visible (narrow column), no combo box passes the column edge. Closing the Results restores their natural widths (**CBX-F1**).
- **CBX-F3 / F4**: every open list is at least as wide as its combo box and shows its longest entry in full, up to the window width.
- **CBX-F5 / F6**: a combo box near the bottom of the window opens its list upward or scrolls; no entry is cut off, and the list never leaves the window.
- **CBX-F7**: on the Tags page, enter or select a long tag: it elides and the page does not widen.
- **All rows**: repeat on Settings, Create, Device Edit, Backup mapping, Statistics and Tags, at full and at minimum window width.
- **CBX-C1**: `git diff` shows no `qsTr` / `tr` string change.
- **CBX-C2**: set the text size to 0.8, then 1.2: the combo boxes and their lists resize.

---

## Out of scope

- K2 (`qt_widgets/`) combo boxes: K2 is in maintenance mode.
- The painting of list rows (surface colour, platform style) of the custom-list
  combo boxes: that is the separate sweep noted under "Other lists that inherit
  the platform style" in `SpecTheme.md` (CBX-C3).
- The entries a combo box offers, their order and their wording (CBX-C1).
