---
id: SpecTypography
title: Typography — Text Size Derived from the System Font
description: Requirements for K3 text sizes following the operating system font setting through a single base size, and for the user-facing text-size setting that multiplies it.
version: "2.13"
---

# TYPOGRAPHY — TEXT SIZE DERIVED FROM THE SYSTEM FONT

![Status](https://img.shields.io/badge/Status-Approved-brightgreen) ![Version](https://img.shields.io/badge/Version-2.13-blue) ![Implementation](https://img.shields.io/badge/Implementation-planned-lightgrey)

## Context

**Decision.** The maintainer decided on 2026-09-25 that K3 uses a **single base
font size, aligned by default with the system**: the operating system's default
font. Every text size in K3 is derived from it, so a user who changes the system
font size sees K3 follow.

**Decision of 2026-09-26 (open questions 1 and 3).** The maintainer decided that
all K3 text is at the default size, except the sizes Kirigami components apply by
themselves. Section headings that K3 places itself go back to the base size;
hierarchy comes from bold only, as in K2. Kirigami built-in sizing that K3 does
not set stays as Kirigami draws it: the page title in the header bar, the
empty-state (`PlaceholderMessage`) title and `FormLayout` section titles. See
TYP-F3, TYP-F4, TYP-F5 and TYP-C3.

**Audit of 2026-09-25.**

- The progress line in `qt_quick/PageBackupForm.qml` used a `font.pixelSize`
  computed from `Kirigami.Units.gridUnit`. This was fixed.
- The type badge at `qt_quick/PageBackupForm.qml:460` still derives its size
  from `gridUnit`. This is a **known non-compliance** with TYP-C2.

**K2 practice (reference, not a requirement).** `qt_widgets/mainwindow.ui` sets
no font size: K2 uses the system font everywhere and marks emphasis with bold or
italic only. The exceptions use fixed sizes:

- a Windows-only stylesheet with Calibri 16px (`qt_widgets/mainwindow.cpp:233-234`);
- statistics chart titles at 14, 16 and 18px, and the chart legend at 8pt;
- the metadata HTML table at 13px.

> **Reading the requirement IDs.** Each requirement has a permanent ID.
> IDs are never renumbered or reused; a retired requirement is marked
> `[Removed]`, not deleted. Status is one of
> `[Implemented] / [Planned] / [Backlog] / [Removed]`.

---

## Scope at a glance

**In scope:** the size of every text in K3 (`qt_quick/`), and how a user-facing
text-size setting acts on it.

**Out of scope:** see [Out of scope](#out-of-scope).

---

## Operational requirements — *why / for whom*

| ID | Requirement | Status |
|----|-------------|--------|
| TYP-O1 | K3 text follows the user's OS font setting on Linux, Windows and macOS. A user who enlarges or shrinks the system font sees K3 text change to match. | [Planned] |

## Functional requirements — *what the system does*

| ID | Requirement | Status |
|----|-------------|--------|
| TYP-F1 | Every text size in K3 is a ratio of one base size, the system default font (`Kirigami.Theme.defaultFont`). | [Planned] |
| TYP-F2 | A user-facing text-size setting multiplies the base size of TYP-F1. It does not replace it. | [Planned] |
| TYP-F3 | Apart from the Kirigami built-ins of TYP-F5 and the one K3-set exception of TYP-F6, every K3 text is displayed at the base size of TYP-F1, multiplied only by the text-size setting of TYP-F2. K3 uses no smaller or larger text roles: no ratios, and not `Kirigami.Theme.smallFont`. | [Planned] |
| TYP-F4 | K3 expresses hierarchy and emphasis by weight (bold), and by its existing italic, opacity or colour, never by size. A section heading that K3 places itself is displayed at the base size, in bold. | [Planned] |
| TYP-F5 | The only Kirigami built-in exceptions to TYP-F3 (see TYP-F6 for the one K3-set exception) are the sizes a Kirigami component applies by itself where K3 sets no size: the page title in the header bar, the empty-state (`PlaceholderMessage`) title, and `FormLayout` section titles (`FormData.isSection`). | [Planned] |
| TYP-F6 | On the Selection page only, the device card's second line (the device description: files, size, used space) is displayed at 0.8 × the base size of TYP-F1 × the card text-size setting, so that a Storage device's line fits on one line at the default card size with one slider step to spare. The Devices page cards stay at the base size. | [Implemented] |

## Constructional requirements — *how it is built / limits / MUST-NOTs*

| ID | Requirement | Status |
|----|-------------|--------|
| TYP-C1 | No K3 text size MUST be an absolute value (fixed points or pixels). | [Planned] |
| TYP-C2 | No K3 text size MUST be derived from a layout unit (`Kirigami.Units.gridUnit`, spacing tokens). Layout measures may scale with the text-size setting; font sizes must not come from layout units. | [Planned] |
| TYP-C3 | K3 code MUST NOT set a font size other than the base of TYP-F1 multiplied by the text-size setting of TYP-F2, except the one exception of TYP-F6. A heading placed by K3 MUST NOT rely on a `Kirigami.Heading` level for its size. | [Planned] |

---

## Open questions

These are not requirements. Each open item needs a maintainer decision before
it can become a row above.

1. **Text roles and ratios — resolved on 2026-09-26.** K3 mixed ratios of 0.7,
   0.8, 0.85 and 0.9. The maintainer chose a single size with hierarchy
   expressed by weight, as K2 does: see TYP-F3, TYP-F4, TYP-F5 and TYP-C3. The
   inventory below (2026-09-25) lists every K3 element whose text is **not** at
   the system default size, with its target size.
2. **Scope of the text-size setting.** The maintainer wants to evaluate an
   app-wide setting (a user feedback request). Still open: whether it also
   scales icons and spacing. An app-wide setting would also have to cover
   headings explicitly: `Kirigami.Heading` does not follow a user text-size
   multiplier unless a page overrides its `font.pointSize`, as
   `qt_quick/DeviceIdentity.qml` does with its `fontScale` (see item 5). Also
   open: the Kirigami built-in sizes kept by TYP-F5 (page title, empty-state
   title, `FormLayout` section titles) are derived from `defaultFont` and so
   would not follow a K3 text-size multiplier.
3. **`Kirigami.Theme.smallFont` — resolved on 2026-09-26.** Used at
   `qt_quick/PageSearchResultsForm.qml:167`. It is not an accepted text role:
   see TYP-F3.
4. **Existing "Card text size" slider — drift.** The drawer slider in
   `qt_quick/Main.qml` (around lines 697-736, range 0.7 to 1.3, step 0.1) has no
   requirement row. Open: its label, range, placement and persistence. It is
   currently persisted by a QML `Settings` block (`savedCardScale`) in the
   platform-native store, not in `katalog3_prerelease_settings.ini`. Beta
   testers already have a value on disk. The maintainer leans towards keeping
   the existing value and extending it app-wide.
5. **`Kirigami.Heading` and TYP-F1 — resolved.** Verified on 2026-09-25 in the
   installed Kirigami (`controls/Heading.qml`): Heading sets `font.pointSize`
   to `Theme.defaultFont.pointSize` times a level factor (level 1: 1.35,
   2: 1.20, 3: 1.15, 4: 1.10, other levels: 1.0). It uses weight DemiBold for
   the Primary type and opacity 0.75 for the Secondary type. Headings therefore
   comply with TYP-F1. The remaining point, their behaviour under TYP-F2, is
   part of item 2.

### Inventory for open question 1 — text not at the system default size

Snapshot of 2026-09-25. "Base" is `Kirigami.Theme.defaultFont.pointSize`. The
target column records the maintainer's decision of 2026-09-26 (TYP-F3, TYP-F4,
TYP-F5).

**Smaller than base**

| Page / view | Item | Current size | Source | Target |
|---|---|---|---|---|
| Explore — file table | Sort mark ▲▼ in the column header | base × 0.7 | `PageExploreFiles.qml:205` | base |
| Search results — table | Sort mark ▲▼ in the column header | base × 0.7 | `PageSearchResultsForm.qml:323` | base |
| Devices — table | Sort mark ▲▼ in the column header | base × 0.7 | `PageDevicesView.qml:744` | base |
| Selection — device card | Description line under the name | base × card slider × 0.8 | `PageSelectionDelegate.qml:171` | base × card slider × 0.8 (TYP-F6) |
| Devices — device card | Comment beside the name | base × card slider × 0.8 | `PageDevicesViewDelegate.qml:257` | base |
| Devices — device card | Detail line (wraps, ends with the date) | base × card slider × 0.8 | `PageDevicesViewDelegate.qml:279` | base |
| Backup — link card | "Backup" / "Archive" type badge | `gridUnit` × 0.75 × card slider, in pixels (breaks TYP-C1/C2) | `PageBackupForm.qml:460` | base |
| Main — Backup prep footer | Preparation status text | base × 0.85 | `Main.qml:2142` | base |
| Operation queue | Status text | base × 0.85 | `OperationQueueView.qml:91` | base |
| Operation queue | "Queue" label | base × 0.85 | `OperationQueueView.qml:123` | base |
| Operation queue | "%1 waiting" count | base × 0.85 | `OperationQueueView.qml:128` | base |
| Operation queue | Queue entry ("Create" / "Update" …) | base × 0.85 | `OperationQueueView.qml:166` | base |
| Settings | Import status text | base × 0.85 | `PageSettings.qml:261` | base |
| Explore — checksum mismatch dialog | Expected / actual checksum values (monospace) | base × 0.85 | `PageExploreFiles.qml:691`, `:699` | base |
| Search results — checksum mismatch dialog | Expected / actual checksum values (monospace) | base × 0.85 | `PageSearchResultsForm.qml:830`, `:838` | base |
| Main — global drawer | "Drawer pinned" / "Drawer floating" | base × 0.9 | `Main.qml:667` | base |
| Main — global drawer | "Selection shown" / "Selection hidden" | base × 0.9 | `Main.qml:692` | base |
| Explore — file table | Column header text | base × 0.9 | `PageExploreFiles.qml:196` | base |
| Explore — file table | Cell text | base × 0.9 | `PageExploreFiles.qml:345` | base |
| Search results — table | Column header text | base × 0.9 | `PageSearchResultsForm.qml:313` | base |
| Search results — table | Cell text | base × 0.9 | `PageSearchResultsForm.qml:496` | base |
| Search results — header | Device path | base × 0.9 | `PageSearchResultsForm.qml:100` | base |
| Devices — table | Column header text | base × 0.9 | `PageDevicesView.qml:736` | base |
| Devices — table | Cell text | base × 0.9 | `PageDevicesView.qml:921` | base |
| Metadata dialog | "Field" / "Value" column headers | base × 0.9 | `MetadataDialog.qml:85`, `:99` | base |
| Metadata dialog | Field labels and values | base × 0.9 | `MetadataDialog.qml:138`, `:160` | base |
| Search results — header | Date range text | `Kirigami.Theme.smallFont` (OS-defined, see question 3) | `PageSearchResultsForm.qml:167` | base |

**Larger than base (Kirigami headings, factors verified in item 5)**

| Page / view | Item | Current size | Source | Target |
|---|---|---|---|---|
| Every page | Page title in the header bar | Heading, default level (× 1.35) | Kirigami page title | Kirigami built-in (TYP-F5) |
| Backup — link list, Backup preview, Devices | Empty-state messages (`PlaceholderMessage` title) | Heading, default level (× 1.35) | `PageBackupForm.qml:361`, `PageBackupPreviewForm.qml:62`, `:259`, `PageDevicesView.qml:629`, `:775` | Kirigami built-in (TYP-F5) |
| Create | "Catalog definition", "Content options", "Global Parameters" | Heading level 3 (× 1.15) | `PageCreateForm.qml:130`, `:190`, `:354` | base, bold |
| Device edit | "Device", "Location", "Content options", "Storage details" | Heading level 3 (× 1.15) | `PageDeviceEditForm.qml:246`, `:295`, `:329`, `:505` | base, bold |
| Tags | "Add a tag", "Current folders and tags" | Heading level 3 (× 1.15) | `PageTagsForm.qml:51`, `:127` | base, bold |
| Settings | "Collection & Database" | Heading level 3 (× 1.15) | `PageSettings.qml:276` | base, bold |
| Settings | "Collection Import & Synchronization", "Application" (link colour, bold) | Heading level 3 (× 1.15) | `PageSettings.qml:469`, `:585` | base, bold |
| Settings | "Search" (link colour, not bold) | Heading level 3 (× 1.15) | `PageSettings.qml:788` | base, bold |
| Backup mapping form | "Source", "Target", "Options" form section titles (`FormData.isSection`) | Heading level 3 (× 1.15) | `PageBackupMappingForm.qml:108`, `:121`, `:134` | Kirigami built-in (TYP-F5) |
| Settings — quality check dialog | Section titles | Heading level 4 (× 1.10) | `PageSettings.qml:170` | base, bold |

**Follows the card slider only (base × slider — equal to base at the default 1.0)**

| Page / view | Item | Source |
|---|---|---|
| Every page with a device header | Selected device name (a level-2 Heading whose size is overridden, so it is **not** × 1.20) | `DeviceIdentity.qml:77` |
| Backup — link card | Link name, Source / Target / Diff labels and values, last-run info, progress line | `PageBackupForm.qml:451`–`:666` |

---

## Manual test charter

- **TYP-O1 / F1**: change the OS default font size, restart K3, and confirm that the text on every page follows.
- **TYP-F2**: move the text-size setting and confirm that the affected text scales from the new OS base.
- **TYP-C1 / C2**: search `qt_quick/*.qml` for `font.pixelSize`, for a numeric `font.pointSize`, and for `gridUnit` or `Units.*Spacing` inside a font expression. There must be no hits.
- **TYP-F3 / C3**: search `qt_quick/*.qml` for `defaultFont.pointSize *` followed by a ratio (such as `0.9`) and for `smallFont`. The only allowed multiplier is the text-size setting; there must be no other hits, except the × 0.8 of TYP-F6 in `PageSelectionDelegate.qml`.
- **TYP-F4 / C3**: search `qt_quick/*.qml` for `Kirigami.Heading`. A K3-placed section heading must not take its size from a Heading level; open Create, Device edit, Tags, Settings and the quality-check dialog and confirm their section headings are at the size of the surrounding text, in bold.
- **TYP-F5**: confirm that the page title in the header bar, the empty-state title (for example an empty Backup link list) and the Backup mapping form section titles ("Source", "Target", "Options") keep the size Kirigami gives them, with no size set by K3.
- **TYP-F6**: at the default card size, confirm that a Storage device's second line on the Selection page fits on one line, and still does after one slider step up (+0.1).

---

## Out of scope

- K2 (`qt_widgets/`): it is in maintenance mode, and its fixed-size exceptions
  listed in Context are recorded for reference only.
- Font family, weight and colour choices.
- Icon and spacing sizes, except as raised in open question 2.
