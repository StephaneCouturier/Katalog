---
id: SpecSearchResultsFilters
title: Search Results — Quick Filter and Catalogs Filter
description: Requirements for the K3 Search results footer bar — a file-name quick filter and a catalogs-with-results filter that narrow what is displayed without re-running the search
version: "2.13"
---

# SEARCH RESULTS — QUICK FILTER AND CATALOGS FILTER

![Status](https://img.shields.io/badge/Status-Approved-brightgreen) ![Version](https://img.shields.io/badge/Version-2.13-blue) ![Implementation](https://img.shields.io/badge/Implementation-planned-lightgrey)

## Context

A search can return more rows than a user can read. K2 offers one way to cut
them down: a *catalogs with results* tree beside the results
(`Search_treeView_CatalogsFound`), built by `Search::deviceFoundModel`
(`core/search.cpp:475-508`) with one row per distinct catalog in the results.
Clicking a row there does a great deal — it sets the application-wide selected
device to that catalog, loads it, refreshes the selected-device name and
**re-runs the search** scoped to it
(`qt_widgets/mainwindow_tab_search_ui.cpp:95-107`).

K3 has no equivalent. This page adds two controls, both in the results page's
**footer bar** — the position a file manager puts its filter bar in — and both
purely *display* filters: they change which of the rows already found are shown,
and nothing else.

The interesting decision is what a catalog click should do. K2's re-search and
the filter proposed here **show the same rows**, because the catalog list only
ever contains catalogs already present in the results. K2 pays a full search and
a change of application-wide state to arrive at a set it already had. What K2
gains for that price is persistence: its narrowing survives into subsequent
searches, because it changed the Selection. The user was shown that trade-off and
chose the filter on 2026-09-12; `SRF-C1` records it.

> **Reading the requirement IDs.** Each requirement has a permanent ID.
> IDs are never renumbered or reused; a retired requirement is marked
> `[Removed]`, not deleted. Status is one of
> `[Implemented] / [Planned] / [Backlog] / [Removed]`.

---

## Scope at a glance

**In scope:** the two footer controls of the K3 Search results page — a
file-name quick filter and a catalogs filter — what they filter, how they
combine, when they reset, and their divergence from K2.

**Out of scope (non-goals):** the search itself and every search criterion, which
belong to the search form and to `SpecSearchList.md`; what the results list
displays per row and its columns; the row context menu; K2's catalogs tree, which
is unchanged (`SRF-C5`); and filtering on any field other than the file name
(`SRF-C4`).

**Applies to:** K3 (`qt_quick`) only.

---

## Operational requirements — *why / for whom*

| ID | Requirement | Status |
|----|-------------|--------|
| SRF-O1 | A user faced with more results than they can read narrows what is shown immediately, without waiting for another search. | [Planned] |
| SRF-O2 | A user sees which catalogs the current results come from, and can look at one catalog's results at a time. | [Planned] |

## Functional requirements — *what the system does*

| ID | Requirement | Status |
|----|-------------|--------|
| SRF-F1 | The results page has a **footer bar** carrying both filters. | [Planned] |
| SRF-F2 | A **quick filter** text field narrows the displayed rows by **file name**, as the user types, with no search re-run. Emptying the field restores every row. Matching is case-insensitive. | [Planned] |
| SRF-F3 | A **catalogs** control lists the catalogs present in the current results — one entry per distinct catalog, as K2's tree does. | [Planned] |
| SRF-F4 | Choosing a catalog **hides the result rows belonging to other catalogs**. It MUST NOT re-run the search and MUST NOT change the application-wide device selection. | [Planned] |
| SRF-F5 | The catalogs filter holds a **set of selected catalogs**. An **empty set is the unfiltered state**: every catalog's rows are shown. Toggling a catalog adds it to the set or removes it, and removing the last one returns to the unfiltered state, so clearing the filter never *requires* a dedicated entry. *(The original clause went further and said none is needed; `SRF-F11` supersedes it — a `None` entry is now required as a convenience, not because the set semantics need one.)* *(Amended 2026-09-13 at the user's request. This row originally specified **single-select**, one catalog at a time, matching K2. It was built with checkable list items, and a checkbox is an additive affordance: the control promised "add this one" and then replaced the previous choice. Single-select as built was self-contradictory, and the user chose to change the behaviour rather than the affordance.)* | [Planned] |
| SRF-F6 | The two filters **stack**: a row is shown only if its file name matches the quick filter **and** its catalog is in the set of `SRF-F5` — or that set is empty. | [Planned] |
| SRF-F7 | **Both filters reset when new results arrive.** A filter left over from a previous search would silently hide rows of the new one, which reads as results missing rather than as a filter being active. The user decided this on 2026-09-13. | [Planned] |
| SRF-F8 | The footer bar is **always visible** on the results page, with no show / hide control and nothing persisted for it. This is a deliberate simplification against K2, which persists its catalogs *panel*'s shown / hidden state in the collection `.ini` under `Settings/ShowHideCatalogResults`: that panel occupies a side of the window, a one-row footer does not, and hiding the bar would also hide the fact that a filter is narrowing the list. **K3 has no such control, so that key is unused by K3 and MUST NOT be written by it**, nor may K3 add a key of its own. The user decided this on 2026-09-13 and scoped it explicitly **"for now"**: a hide or persist option is a **separate request** if it is ever wanted, and MUST NOT be built pre-emptively against the possibility. | [Planned] |
| SRF-F9 | **Several catalogs can be selected in one visit to the list.** The list stays open while catalogs are toggled; selecting three MUST NOT cost three trips to reopen it. This is a requirement about behaviour, not about a widget: a plain menu that closes on every activation does not satisfy it, and which control replaces it is the implementer's choice. | [Planned] |
| SRF-F10 | The catalogs control's label reports the state of the set: **none** selected shows the existing `Catalog with results`; **exactly one** shows that catalog's name; **more than one** shows the existing string followed by the count in parentheses, for example `Catalog with results (3)`. The number is generated, not translatable text, so this adds **no** translation slot (`SRF-C3`). | [Planned] |
| SRF-F11 | The catalogs list offers two helper entries, **`All`** and **`None`**, placed above the catalogs and visually separated from them because they are actions rather than selectable catalogs. `All` selects every catalog currently listed; `None` empties the set. Both leave the list open (`SRF-F9`), neither runs a search and neither touches the application-wide selection (`SRF-C1`). The user asked for them on 2026-09-13. **The two show the same rows, and that is correct, not a defect:** an empty set is the unfiltered state, so `None` displays every row, and selecting every catalog displays every row too. They differ as *starting points* — `All` then unticking two is how a user asks for "everything except these two", which is impractical by hand when a search spans a dozen catalogs — and in the label of `SRF-F10`, which reads `Catalog with results (12)` after `All` and the plain string after `None`. `All` earns its place as the entry point to an **exclusion** workflow, not as a way to see more rows. | [Planned] |

## Constructional requirements — *how it is built / limits / MUST-NOTs*

| ID | Requirement | Status |
|----|-------------|--------|
| SRF-C1 | `SRF-F4` is a **deliberate divergence from K2**, chosen by the user on 2026-09-12 with the trade-off stated: both approaches display the same rows, since the catalog list contains only catalogs already in the results, so K2's re-search spends a full search and a change of global state to reach a set it already holds; what K2 alone gives is that its narrowing **persists into later searches**, because it narrowed the Selection. K3 accepts losing that. This MUST NOT be reported as an incomplete port, and re-introducing the re-search — or any write to the application-wide selection from this control — requires a new requirement. | [Planned] |
| SRF-C2 | Built in `qt_quick/` only: the results already pass through K3's own proxy (`qt_quick/filesview.h`, created at `qt_quick/appmanager.cpp:137`), and the roles both filters need — file name, catalog name and catalog id — already exist (`core/search.h:61-67`). **No `core/` change and no SQL in `qt_quick/`** is authorised; needing either is a stop-and-ask. The live text filter follows the idiom the Selection page already uses for a single-role case-insensitive proxy filter (`qt_quick/appmanager.cpp:378-410`). | [Planned] |
| SRF-C3 | **Zero new user-visible strings.** `SRF-F11`'s `All` and `None` are existing translated strings reused verbatim — they already appear in other K3 contexts, so making them available here is a sync bridge and costs no translation work. The catalogs control reuses K2's existing `Catalog with results` verbatim — singular, as K2 has it (`qt_widgets/mainwindow_tab_search_pr.cpp:791`) — and the quick filter carries the search field's own placeholder, which the toolkit translates. If the design turns out to need a label with no K2 equivalent, work **stops** and the string goes to the user for per-string approval; it MUST NOT be invented. | [Planned] |
| SRF-C4 | `SRF-F2` filters on the **file name only**. Path, size, date, type and every other column are out, by the user's explicit "name only to start with". Widening it is a separate request. | [Planned] |
| SRF-C5 | K2 MUST NOT be changed: it keeps its catalogs tree, its re-search behaviour, its collapsible panel and its persisted `Settings/ShowHideCatalogResults`. | [Planned] |

---

## Open, not authorised

| Item | Detail |
|------|--------|
| ~~Multi-select catalogs~~ | **Closed 2026-09-13** — requested by the user and now required by `SRF-F5`. Kept here as the record that it was an open item first, not a feature that arrived unasked. |
| Filtering on other fields | Path, size, date and type — see `SRF-C4`. |
| A persisted show / hide for the footer | `SRF-F8` keeps it always visible, explicitly *for now*. If the bar later proves intrusive, a toggle — and any key to persist it — is a new requirement, not an implementation detail, and not something to anticipate in the first build. |
| Carrying a filter across searches | The persistence K2 gets from re-searching (`SRF-C1`). If it is missed in use, it needs its own row — not a quiet return to changing the Selection. |

---

## Manual test charter

- **SRF-F2** — Run a search returning many rows. Type a fragment of a file name
  in the quick filter: the list narrows as you type, with no progress report and
  no new search. Clear the field: every row returns. Type the fragment in a
  different case: the same rows match.
- **SRF-F3 / SRF-F4** — With results spanning several catalogs, choose one in the
  catalogs control: only that catalog's rows remain. Confirm **no search runs** —
  the status bar reports nothing and the result count in the header is the count
  of displayed rows, not of a new search — and that the device selected on the
  Selection page is **unchanged**.
- **SRF-F5 / SRF-F9 (one visit)** — With results spanning three or more
  catalogs, open the catalogs list **once** and select two catalogs without the
  list closing between them. Both are shown as selected, and the number of rows
  displayed equals the sum of the two catalogs' rows — not one catalog's, which
  is what a replacing filter would give.
- **SRF-F5 (empty set)** — Deselect the selected catalogs one by one. After the
  last is removed every catalog's rows are back, and **no search runs** to bring
  them back: the status bar reports nothing and the operation is instant.
- **SRF-F10** — With none selected the control reads `Catalog with results`;
- **SRF-F11 (exclusion workflow)** — With results spanning several catalogs, open the list and choose `All`: every catalog is ticked and the label shows the count. Without closing the list, untick two catalogs. The rows displayed are exactly those of the remaining catalogs, and **no search runs** — nothing is reported in the status bar and the change is instant.
- **SRF-F11 (All and None)** — Choose `None`: the set empties and every row is shown, with the label back to the plain string. Choose `All`: every row is shown again, with the count in the label. Confirm the two row sets are identical — that is the documented behaviour, not a defect — and that the difference is the label and what unticking next does.
- **SRF-F11 (placement)** — Confirm `All` and `None` sit above the catalogs and are visually separated from them, and that neither can be mistaken for a catalog in the list.
- **SRF-F11 / SRF-C3** — Run `ninja translations_lupdate`: no new untranslated string appears. With the interface in French, both helper entries are translated.
  with one, that catalog's name; with three, `Catalog with results (3)`. Switch
  the interface to French and confirm the text part is translated and the count
  still appears.
- **SRF-F6** — Set a file-name filter and select two catalogs: only rows whose
  name matches **and** whose catalog is one of the two are shown. Clear the
  name filter: both catalogs' full row sets return. Empty the catalog set: the
  name filter still applies across every catalog.
- **SRF-F7** — With a name filter set and two catalogs selected, run a new
  search. Both filters are cleared — the catalog set is **emptied**, not carried
  over — and the full new result set is shown.
- **SRF-F8** — Confirm the footer bar is present whenever results are shown, that
  no show / hide control exists for it, and that nothing is written to the
  collection `.ini` for it — in particular that `Settings/ShowHideCatalogResults`
  is left exactly as K2 wrote it, untouched by K3.
- **SRF-C1** — Note the rows shown after choosing a catalog in K3. Do the
  equivalent in K2 on the same collection: the same rows, but K2 has re-run the
  search and the Selection now names that catalog. Both are correct.
- **SRF-C3** — Run `ninja translations_lupdate`: **no** new untranslated string
  appears for the results page. With the interface in French, the catalogs
  control's heading and the quick filter's placeholder are both translated.
