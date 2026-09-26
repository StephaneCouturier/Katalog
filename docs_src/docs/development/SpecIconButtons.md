---
id: SpecIconButtons
title: Icon Buttons — Tooltips on Icon-Only Buttons
description: Requirements for K3 icon-only buttons to name their action in a tooltip, reusing existing K3 or K2 wording, with the tree expand / collapse chevrons as the one exception.
version: "2.13"
---

# ICON BUTTONS — TOOLTIPS ON ICON-ONLY BUTTONS

![Status](https://img.shields.io/badge/Status-Approved-brightgreen) ![Version](https://img.shields.io/badge/Version-2.13-blue) ![Implementation](https://img.shields.io/badge/Implementation-complete-brightgreen)

## Context

**Decision of 2026-09-26.** An icon-only button in K3 names its action in a
tooltip. An icon alone does not tell every user what the button does; the
tooltip does, without spending layout width on a text label.

**Exception.** The per-item expand / collapse chevrons of the trees carry no
tooltip (ICB-C3). Their meaning is conveyed by the chevron itself and by the
tree around it, and a tooltip on every row would be noise. This keeps the
"no tooltip" clause of `CDT-F3` (`SpecCardsAndTables.md`) valid, and keeps the
Selection card collapse button without tooltip, as it has been since its
tooltip was removed (#759).

**Earlier specs superseded on this point.** Several specs recorded icon-only
buttons as deliberately tooltip-free, to add no translatable string:
`SRL-C6` and the `SearchTermList` design note (`SpecSearchList.md`), the queue
remove control (`SpecOperationQueue.md`), and `DVP-F24` (`SpecDevicesPage.md`).
They now point here.

> **Reading the requirement IDs.** Each requirement has a permanent ID.
> IDs are never renumbered or reused; a retired requirement is marked
> `[Removed]`, not deleted. Status is one of
> `[Implemented] / [Planned] / [Backlog] / [Removed]`.

---

## Scope at a glance

**In scope:** the tooltip of every icon-only button in K3 (`qt_quick/`): a
button that shows an icon and no text label.

**Out of scope:** see [Out of scope](#out-of-scope).

---

## Operational requirements — *why / for whom*

| ID | Requirement | Status |
|----|-------------|--------|
| ICB-O1 | A user who does not recognise an icon can find out what a button does without clicking it. | [Implemented] |

## Functional requirements — *what the system does*

| ID | Requirement | Status |
|----|-------------|--------|
| ICB-F1 | Every K3 button that shows an icon and no visible text shows a tooltip on hover naming its action. | [Implemented] |
| ICB-F2 | A button that is present in the layout but not shown (kept only to reserve space) shows no tooltip. | [Implemented] |

## Constructional requirements — *how it is built / limits / MUST-NOTs*

| ID | Requirement | Status |
|----|-------------|--------|
| ICB-C1 | The tooltip text is chosen in this order: (1) an existing K3 string that fits the action is reused first; (2) otherwise K2's existing tooltip string for the same action is reused byte-for-byte; (3) otherwise a new string is written, which needs per-string user approval before it is written. *(Amended 2026-09-26: an existing K3 string now takes precedence over the K2 string.)* | [Implemented] |
| ICB-C2 | This rule overrides the earlier "no tooltip" rows: CDT-F3's disclosure clause, SRL-C6 for tooltip strings, and the SearchTermList and Operation Queue design notes. Each of those rows is amended to point to ICB-F1 (or marked as an exception) in the same edit. | [Implemented] |
| ICB-C3 | **Exception to ICB-F1.** The per-item expand / collapse chevrons of the trees carry **no tooltip**: on the Selection page device cards, on the Devices page (cards and tree table), and in the Explore folder tree. This keeps `CDT-F3`'s "no tooltip" clause valid. It covers the per-item control only: the Explore tree's four header controls (`EXP-F9`) keep their tooltips. | [Implemented] |

---

## Approved tooltip strings

Reference list of the strings approved under ICB-C1. It is a record, not a
requirement row.

| Button | String | Source |
|--------|--------|--------|
| Search text list, exclude list and metadata field: Paste | `Paste the text from the clipboard` | K2, verbatim |
| Search text list and exclude list: Clean | `Clean the search Text from characters such as _ - . ,` | K2, verbatim |
| Folder pickers (6): Create path, Create per-catalog exclude, Create global exclude, Device edit path, Device edit exclude, Search connected drive | `Select the path` | K2, verbatim |
| Tags folder picker | `Select a folder` | K2, verbatim |
| Search results: run button | `Run process on all results` | K2, verbatim |
| Create exclusion lists (2), Device edit exclusion list, Operation Queue entry remove | `Remove` | K2, verbatim |
| Tags: delete | `Remove this tag` | K2, verbatim |
| Create: Global Parameters section toggle (a section toggle, not a per-item tree chevron, so not covered by ICB-C3) | `Expand` / `Collapse` | K3, reused |
| Search min / max date and Statistics start date: calendar buttons | `Select a date` | K3, reused |
| Inline clear icon of the search-term rows, metadata text field, Selection device filter field; Statistics clear start date | `Clear` | K3, reused |
| SearchTermList: `+` | `Add a term` | New, approved |
| SearchTermList: `−` | `Remove this term` | New, approved |
| Settings: text size buttons | `Decrease text size` / `Increase text size` | New, approved |

**Buttons without a tooltip, by design:**

- Drawer pin and Selection-column pin buttons (main window): each has an
  adjacent label naming it, treated as its visible text, so they are not
  icon-only under ICB-F1.
- Hidden toolbar-height probe button (main window): present only to reserve /
  measure space (ICB-F2).
- Devices card expand / collapse chevron: tooltip removed per ICB-C3; `Expand` /
  `Collapse` kept as accessible text.

No string is pending approval.

---

## Manual test charter

- **ICB-O1 / F1**: on every K3 page, hover each button that shows an icon and no visible text and confirm a tooltip names its action; the only buttons without one are those of ICB-C3 and those listed under "Buttons without a tooltip, by design" above.
- **ICB-F2**: find a button kept in the layout only to reserve space (invisible), and hover its position: no tooltip appears.
- **ICB-C1**: switch the interface to French and hover a K3-reused tooltip (e.g. an inline `Clear` icon or a `Select a date` calendar button) and a K2-reused one (e.g. search list Paste and Clean): all are translated, confirming existing strings were reused and not duplicated. Run `ninja translations_lupdate`: the only new untranslated tooltip strings are the four marked "New, approved" above.
- **ICB-C2**: confirm CDT-F3 (`SpecCardsAndTables.md`) cites the exception ICB-C3, and that SRL-C6 and the SearchTermList note (`SpecSearchList.md`) and the queue remove-control note (`SpecOperationQueue.md`) cite ICB-F1.
- **ICB-C3**: hover the expand / collapse chevron of a Selection card, a Devices card, a Devices tree table row and an Explore folder row: no tooltip appears. Hover the Explore tree's four header controls: their tooltips still appear.

---

## Out of scope

- K2 (`qt_widgets/`): it is in maintenance mode; its tooltips are the source of
  reused strings (ICB-C1), not changed by this spec.
- Buttons that carry a visible text label.
- The wording of any specific tooltip beyond the approved list: each one needs
  its own approval (ICB-C1).
