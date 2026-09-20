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
disclosure control of any tree** K3 draws (`CDT-F3`), added 2026-09-13.

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

## Functional requirements — *what the system does*

Observable behaviour that can be triggered and watched.

| ID | Requirement | Status |
|----|-------------|--------|
| CDT-F1 | **Text on a card wraps; it is never hidden.** No label on a card truncates its content with an ellipsis or clips it at a fixed line count. A value too long for the card's width continues on the next line and the card grows taller. | [Planned] |
| CDT-F2 | **Table cells may elide.** A value too long for its column is cut short, because the column has a fixed width the user can adjust and the alignment of the columns is what a table is for. This is the deliberate counterpart of `CDT-F1`, not an oversight, and MUST NOT be "corrected" to match it. | [Planned] |
| CDT-F3 | **A tree's per-row disclosure control is the same wherever a tree appears in K3.** It uses the *symbolic* chevrons — the variants the icon theme ships as disclosure indicators — not the filled navigation arrows; it is a control with **hover and press feedback and a full-size click target**, not a bare icon with a small hit area, because that feedback is what tells the user the chevron is clickable; a row with **no children keeps the control's space**, so names stay aligned down the column; the indent per level is the same unit everywhere; and it carries **no tooltip**. The two trees that exist today — the Explore directory tree and the Devices tree table — each adopt the better half of what they had: Explore takes the symbolic chevrons, the Devices table takes the proper control. The user approved this two-way alignment on 2026-09-13; the divergence was historical, not a decision. | [Planned] |

## Constructional requirements — *how it is built / limits / MUST-NOTs*

Boundaries and implementation constraints, not user-visible behaviour.

| ID | Requirement | Status |
|----|-------------|--------|
| CDT-C1 | `CDT-F1` binds **every** K3 card, including cards added later. A new card delegate MUST NOT introduce eliding or a fixed line cap; a width cap that protects the layout is allowed only if the capped text wraps **inside** that cap. | [Planned] |
| CDT-C2 | These rows add **no** user-visible string. Wrapping changes the shape of existing text, never its wording. | [Planned] |
| CDT-C3 | Applying `CDT-F1` to a card that already exists is a change to that page and MUST be authorised by that page's own spec before it is made. `CDT-F1` states the rule; it does not by itself authorise editing any particular file. The Devices page correction is authorised by `DVP-F18` and `DVP-C16` (`SpecDevicesPage.md`). | [Planned] |
| CDT-C4 | A component **shared** between a card and a non-card context MUST NOT be made to wrap unconditionally. The selected-device reminder of `SEL-F5` is a single line above a list, where wrapping would push the list down as the name grows; such a caller keeps its present behaviour until the user asks otherwise. The wrapping is therefore a property of the caller, defaulting to the existing behaviour. | [Planned] |
| CDT-C5 | `CDT-F3` aligns the **per-row control only**. It MUST NOT be "completed" by giving every tree the same **bulk** controls: the Explore tree's four header controls (`EXP-F9`, `SpecExplore.md`) exist because a folder tree is arbitrarily deep, while the device tree is at most three levels, so a page having them and another not is a difference in the **data**, not an inconsistency to be ironed out. Adding them anywhere else is a separate request. | [Planned] |

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
- **CDT-F3 (side by side)** — Open the Explore directory tree and the Devices tree table and compare a row that has children: the same chevron shape in the same two states, the same indent per level, and the same hover and press feedback in both. Neither shows a tooltip.
- **CDT-F3 (childless rows)** — On **both** pages, find a row with no children: its name starts at the same left edge as the name of a sibling that does have children. The column of names is straight; no row is shifted left by a missing control.
- **CDT-F3 (click target)** — On both pages, click just inside the edge of the chevron rather than its centre: the row expands. The pointer feedback appears before the click, on hover.
- **CDT-C5** — Confirm the Explore tree still has its four header controls and that the Devices page has **not** acquired them.
  still occupies one line, and that the list below it does not move when a
  device with a very long name is selected.
