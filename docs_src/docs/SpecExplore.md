---
id: SpecExplore
title: Explore — Directory Tree
description: Requirements for expanding and collapsing the directory tree of the Explore screen
---

# EXPLORE — DIRECTORY TREE

![Status](https://img.shields.io/badge/Status-Draft-orange) ![Implementation](https://img.shields.io/badge/Implementation-planned-yellow)

## Context

The Explore screen shows a catalog's directory hierarchy in its left panel and
the files of the selected directory on the right. K2 renders that hierarchy in a
tree widget, which brings expansion and collapsing for free and opens the tree to
its first rank on load. K3 renders the same hierarchy as a flat, depth-ordered
list with indentation only: nothing can be folded away, and the whole hierarchy
is shown at once whatever its size.

This spec defines how much of the hierarchy is visible at any moment, and how the
user changes it. It is the only source of requirements for that behaviour.

> **Reading the requirement IDs.** Each requirement has a permanent ID.
> IDs are never renumbered or reused; a retired requirement is marked
> `[Removed]`, not deleted. Status is one of
> `[Implemented] / [Planned] / [Backlog] / [Removed]`.

---

## Scope at a glance

**In scope:** which directory rows of the Explore tree are visible; the per-row
disclosure control; the initial expansion depth; the four header controls
(collapse one level, expand one level, collapse all, expand all) and when they
are available; revealing a row that external navigation has selected.

**Out of scope (non-goals):** everything else on the Explore screen — loading the
catalog, the file list, the display options, the context menus, sorting, and the
Selection page's device tree. None of those are governed by this spec; this spec
does not authorise changing any of them. Persisting expansion state is
explicitly excluded — see `EXP-C2`.

**Applies to:** K3 (`qt_quick`) only. K2's Explore tree already provides
`EXP-F1` through its tree widget, and an initial expansion of its own that
`EXP-F2` deliberately no longer matches; the header controls of
`EXP-F3`–`EXP-F7` are not required in K2, which is in maintenance mode.

---

## Operational requirements — *why / for whom*

Goals in real use, independent of how they are built.

| ID | Requirement | Status |
|----|-------------|--------|
| EXP-O1 | A user browsing a catalog opens only the branches of interest and keeps the rest of the hierarchy folded away, so a deep or wide directory tree stays readable. | [Planned] |
| EXP-O2 | A user changes how much of the hierarchy is shown in one action — one rank at a time, or the whole tree at once — instead of clicking through every branch. | [Planned] |

## Functional requirements — *what the system does*

Observable behaviour that can be triggered and watched.

| ID | Requirement | Status |
|----|-------------|--------|
| EXP-F1 | A directory row that has sub-directories carries a disclosure control; activating it hides its descendants, activating it again shows them. A row without sub-directories has no control but reserves the same space, so row geometry and indentation never shift between rows. | [Planned] |
| EXP-F2 | When the Explore directory tree loads a catalog, the root row, its children and its grandchildren are shown; every rank below the grandchildren starts collapsed. **Deliberate divergence from K2**, approved 2026-08-27: K2 opens the first rank only, K3 opens two. Rationale: a catalog's meaningful content usually starts a folder or two below the root, so opening a single rank made "expand" the first action on every catalog. This is no longer a parity requirement and MUST NOT be "corrected" back to K2's single-rank depth. | [Planned] |
| EXP-F3 | **Collapse one level** collapses the deepest rank that is currently visible and open. | [Planned] |
| EXP-F4 | **Expand one level** opens the shallowest rank that is currently collapsed. | [Planned] |
| EXP-F5 | **Collapse all** collapses every row that has sub-directories, including the root row. | [Planned] |
| EXP-F6 | **Expand all** clears all collapse state, so every directory of the catalog is visible. | [Planned] |
| EXP-F7 | Each of the four controls is disabled whenever activating it would change nothing: the collapse controls when nothing further can be collapsed, the expand controls when nothing further can be expanded. | [Planned] |
| EXP-F8 | When navigation originating outside the directory tree selects a directory whose row is hidden under a collapsed ancestor, the ancestors of that row are expanded so the selected row is visible. A selected row is never left hidden. | [Planned] |
| EXP-F9 | The four header controls are icon-only and identified by tooltips. Their exact texts are `Collapse one level`, `Expand one level`, `Collapse all`, `Expand all`. The first two are reused verbatim from the Selection page's pair; the last two are new strings, approved on 2026-08-27. | [Planned] |

## Constructional requirements — *how it is built / limits / MUST-NOTs*

Boundaries and implementation constraints, not user-visible behaviour.

| ID | Requirement | Status |
|----|-------------|--------|
| EXP-C1 | Expansion state is held in the QML layer of the Explore folders view (`qt_quick/PageExploreFolders.qml`) as a set of collapsed folder paths, applied over the flat, depth-ordered folder list already provided by `AppManager::getExploreFolders()`. Whether a row has sub-directories is inferred from the depth of the following row. The `EXP-F2` initial depth is a single named constant of that view, so it can be re-tuned in one place; the root row is rank 0 and directories start at rank 1. | [Planned] |
| EXP-C2 | Expansion state is transient UI state. It MUST NOT be persisted — not to the collection settings file, not to the application settings, not to the database. It is rebuilt from `EXP-F2` each time the Explore view loads a catalog. | [Planned] |
| EXP-C3 | This feature MUST NOT require a change to `core/`, to the QML adapters, to the database schema, or to the folder data returned to QML. It MUST NOT introduce a new source file. | [Planned] |
| EXP-C4 | **Accepted divergence.** The Selection page's device tree keeps its collapse state in C++ (`DeviceListModel`), while the Explore directory tree keeps its own in QML. This is a deliberate choice: Explore's expansion is purely visual and carries no domain meaning, so it does not justify a core or adapter change. The two trees behaving differently in code MUST NOT be reported as drift. | [Planned] |
| EXP-C5 | The header controls MUST reuse the existing translated strings `Collapse one level` and `Expand one level` byte-for-byte rather than creating variants of them. | [Planned] |

---

## Manual test charter

For each row: set up the stated condition, run the operation, confirm the result.

- **EXP-F1** — Open a catalog with a mixed hierarchy. A directory that contains sub-directories shows a disclosure control; a leaf directory shows none. Compare a leaf row and a parent row at the same depth: their height, indentation and the horizontal position of the directory name are identical. Toggle a parent: its descendants disappear, then reappear unchanged.
- **EXP-F2** — Open a catalog at least four ranks deep. On load, the root, its children and its grandchildren are listed; no fourth-rank directory is visible. Confirm this is the intended divergence from K2, whose tree opens one rank less.
- **EXP-F3** — From the initial state, expand two further ranks, then press collapse one level repeatedly. Each press removes exactly the deepest visible rank; collapsing does not stop at the `EXP-F2` initial depth.
- **EXP-F4** — From the initial state, press expand one level. Exactly one further rank appears — the fourth — and branches already open stay open.
- **EXP-F5** — Press collapse all. Only the root row remains, and it is collapsed.
- **EXP-F6** — After collapse all, press expand all. Every directory of the catalog is listed.
- **EXP-F7** — After collapse all, the two collapse controls are disabled and the two expand controls are enabled. After expand all, the reverse. In the initial state of a catalog deeper than the `EXP-F2` initial depth (four ranks or more), all four are enabled; in the initial state of a catalog exactly three ranks deep, nothing is left to expand, so both expand controls are disabled.
- **EXP-F8** — Collapse a branch, then navigate into one of its hidden sub-directories from the file list on the right. The tree opens that branch and the selected row is visible and highlighted; it is never highlighted while off-screen or hidden.
- **EXP-F9 / EXP-C5** — Hover each of the four controls: the tooltips read exactly `Collapse one level`, `Expand one level`, `Collapse all`, `Expand all`. Switch the interface to French: the first two are translated (they already exist), confirming the strings were reused and not duplicated.
- **EXP-C2** — Collapse several branches, leave the Explore screen and come back, or reload the catalog. The tree is back to the `EXP-F2` initial state (root, children and grandchildren open); no collapse state survived.
