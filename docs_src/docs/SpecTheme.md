---
id: SpecTheme
title: Theme and colour derivation
description: How K3 derives its surface, row and highlight colours from the desktop colour scheme, and what the Theme setting is allowed to change.
version: "2.13"
---

# Theme and colour derivation

![Status](https://img.shields.io/badge/Status-Draft-orange) ![Version](https://img.shields.io/badge/Version-2.13-blue) ![Implementation](https://img.shields.io/badge/Implementation-partial-yellow)

## Context

K3 (`qt_quick`) paints several surfaces that are not plain page background: the
alternating rows of a file list, the selected row, the logo band and the
Selection page surface. This spec defines **where those colours come from** and
**what the Theme setting is allowed to change**.

The rule behind it: a colour is either taken from the desktop colour scheme
through `Kirigami.Theme`, or it does not exist. Literal colour constants
(`"white"`, `"black"`, hex brand blues) do not follow the user's scheme and are
what this spec removes.

> **Reading the requirement IDs.** Each requirement has a permanent ID.
> IDs are never renumbered or reused; a retired requirement is marked
> `[Removed]`, not deleted. Status is one of
> `[Implemented] / [Planned] / [Backlog] / [Removed]`.

---

## Scope at a glance

**In scope:** K3 colour derivation only — the file-list row pair, and the reach
of the three `Settings/Theme` values on K3's derived colours.

**Out of scope (non-goals):**

- **K2 (`qt_widgets`)** — in maintenance mode. It shares the same
  `Settings/Theme` key and keeps its own Katalog Colors rendering, untouched
  (`THM-C5`).
- **Icon theme** — icon set selection is not governed here.
- **Removing the "Katalog Colors" entry from the Settings combo** — explicitly
  future work, not authorised by this spec (`THM-C3`).
- Typography, spacing, and any other non-colour theming.

> **Note on the stored default.** `Settings/Theme` defaults to `1` for a new K3
> profile. Under `THM-C2` that default now renders exactly as theme `0`. This is
> recorded, not changed: altering the default value is not authorised here.

---

## Operational requirements — *why / for whom*

Goals in real use, independent of how they are built.

| ID | Requirement | Status |
|----|-------------|--------|
| THM-O1 | A user scanning a long file list can tell one row from the next at a glance, and does not lose the line they are reading — under their own desktop colour scheme, light or dark, and whichever Theme value is stored. | [Planned] |

## Functional requirements — *what the system does*

Observable behaviour that can be triggered and watched.

| ID | Requirement | Status |
|----|-------------|--------|
| THM-F1 | File-list rows alternate between exactly **two** background colours, both taken from the desktop colour scheme: (a) the **View** background — `Kirigami.Theme.backgroundColor` read under `Kirigami.Theme.colorSet: Kirigami.Theme.View` with `inherit: false`; on Breeze this is `#fcfcfc` light / `#1b1e20` dark, so it reads as the near-white / near-black extreme, while still following any other scheme — and (b) the **Window** background, `Kirigami.Theme.backgroundColor` as inherited by the page today (`#eff0f1` / `#2a2e32` on Breeze). Neither colour is a literal white, black or hex constant. | [Planned] |
| THM-F2 | Parity: **even** rows — row index 0, the first row — take the **View** colour of `THM-F1`; **odd** rows take the **Window** background. This is the ordinary list-view convention. | [Planned] |
| THM-F3 | The `THM-F1` / `THM-F2` row pair applies to the **Search results** file list and the **Explore** file list, identically in both. | [Planned] |
| THM-F4 | The row pair is the same under **every** `Settings/Theme` value (0, 1 and 2). The Theme setting does not change file-list row colours. | [Planned] |
| THM-F5 | A **selected** row is painted with the selection highlight instead of its parity colour; the row pair applies only to unselected rows. Existing behaviour, unchanged by this spec. | [Implemented] |
| THM-F6 | Derived colours are bindings on `Kirigami.Theme`, so changing the desktop colour scheme while Katalog is running repaints the lists to the new scheme without restarting the application. | [Planned] |

## Constructional requirements — *how it is built / limits / MUST-NOTs*

Boundaries and implementation constraints, not user-visible behaviour.

| ID | Requirement | Status |
|----|-------------|--------|
| THM-C1 | **Both** colours of the `THM-F1` pair are defined **once**, in `Main.qml` at `applicationWindow()` scope, and consumed from there by both file-list delegates. A page or delegate MUST NOT hardcode, re-derive or locally redefine either colour — including the Window colour, which delegates currently read inline. One definition, two consumers. | [Planned] |
| THM-C2 | **Katalog Colors (theme id 1) is retired in K3.** It MUST NOT apply any brand override: not to the row colours, not to `selectionHighlightColor`, not to `logoBandColor`, and not to `selectionPageColor`. Theme id 1 renders **exactly** as theme id 0 — the accent-derived variant — and **not** as theme id 2. | [Planned] |
| THM-C3 | The stored value `Settings/Theme = 1` and the `Katalog Colors` entry in the Settings combo are **preserved**. No user-visible string is added, changed or removed by this spec, and no translation slot is spent. The entry simply has no visual effect in K3. Removing it is out of scope (see Scope). | [Planned] |
| THM-C4 | Theme id 2, `Desktop Theme (gray)`, keeps its existing distinct derivation from the desktop **background** (no hue). Unchanged by this spec. | [Implemented] |
| THM-C5 | K2 (`qt_widgets`) MUST NOT be modified by this work. It shares the `Settings/Theme` key and keeps its own Katalog Colors rendering; the divergence between K2 and K3 on theme id 1 is accepted and deliberate. | [Planned] |
| THM-C6 | The View colour set MUST be entered in a scope narrow enough that it does not leak onto surrounding items — `inherit: false` on the item that reads it, so sibling and parent surfaces keep the Window colour set. | [Planned] |
| THM-C7 | The item that reads the View colour set MUST NOT be hidden (`visible: false`). A hidden item is given no resolved palette by the KDE platform theme and its background colour reads back as opaque black, which paints the first row of every file list black on a light desktop. This failure does not reproduce under `QT_QPA_PLATFORM=offscreen`, where a hidden item still returns the correct colour, so it cannot be caught by an offscreen run — it must be checked against the real desktop. | [Planned] |

---

## Manual test charter

For each row: set up the stated condition, look at the result.

- **THM-F1 / THM-F2 (light desktop)** — On a light Breeze scheme, open the Search results list. Row 0 is the near-white View colour; row 1 is the slightly darker Window colour; the alternation continues. Neither is pure `#ffffff`.
- **THM-F1 / THM-F2 (dark desktop)** — Switch to Breeze Dark. Row 0 is the near-black View colour; row 1 is the lighter Window colour. Neither is pure `#000000`.
- **THM-F1 (non-Breeze scheme)** — Apply a Plasma colour scheme whose View colour is not near-white (a tinted or sepia scheme). The rows take that scheme's View and Window colours, not white/black.
- **THM-F3** — Repeat the light and dark checks on the **Explore** file list. The two lists look identical row for row.
- **THM-F4 (theme id 0)** — Settings → Theme → *Desktop Theme*. Note the row colours.
- **THM-F4 (theme id 1)** — Switch to *Katalog Colors*. The file-list rows are unchanged, **and** the logo band, Selection page surface and selection highlight are identical to theme id 0 — no brand blue anywhere (`THM-C2`).
- **THM-F4 (theme id 2)** — Switch to *Desktop Theme (gray)*. File-list rows are still the same pair; the logo band and Selection page surface show the grey derivation, still distinct from theme 0 (`THM-C4`).
- **THM-C3** — After the above, reopen Settings: the *Katalog Colors* entry is still listed and still selectable, and the stored `Settings/Theme` value survives a restart.
- **THM-F6 (live scheme switch)** — With Katalog open on the Search results list, change the Plasma colour scheme from light to dark in System Settings. The list repaints to the new scheme without restarting Katalog; both row colours follow, and the selected row's highlight follows too.
- **THM-C1** — Compare a Search results row and an Explore row of the same parity under the same scheme: they are the same colour. Grep confirms neither page defines a row colour of its own.
- **THM-C5** — Open K2 and select *Katalog Colors*. K2 still renders its own brand palette; the two applications differ here by design.
