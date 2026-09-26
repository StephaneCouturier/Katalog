---
id: SpecTypography
title: Typography — Text Size Derived from the System Font
description: Requirements for K3 text sizes following the operating system font setting through a single base size, and for the user-facing text-size setting that multiplies it.
version: "2.13"
---

# TYPOGRAPHY — TEXT SIZE DERIVED FROM THE SYSTEM FONT

![Status](https://img.shields.io/badge/Status-Approved-brightgreen) ![Version](https://img.shields.io/badge/Version-2.13-blue) ![Implementation](https://img.shields.io/badge/Implementation-partial-yellow)

## Context

**Decision.** The maintainer decided on 2026-09-25 that K3 uses a **single base
font size, aligned by default with the system**: the operating system's default
font. Every text size in K3 is derived from it, so a user who changes the system
font size sees K3 follow.

**Decision of 2026-09-26.** All K3 text is at the base size. Section headings
that K3 places itself are at the base size, in bold: hierarchy comes from weight,
as in K2. The only sizes K3 does not set are those Kirigami components apply by
themselves (page title, empty-state title, `FormLayout` section titles); Kirigami
derives them from the same base (`Heading` level factors: 1: 1.35, 2: 1.20,
3: 1.15, 4: 1.10). The one exception K3 sets itself is the Selection card second
line (TYP-F6), kept on one line at the default card size.

**Decision of 2026-09-26 (text-size setting).** As a first step, the text-size
setting scales **text only**, app-wide; it does not scale icons or spacing. The
existing drawer "Card text size" slider becomes that setting: its value now
applies to all K3 text, not only to cards. Its label, range, placement and
persistence are kept unchanged for now.

**Decision of 2026-09-26 (setting moved to Settings).** The text-size setting
leaves the global drawer and moves to the Settings page, Application section, in
a "Theme" row together with the theme selector and "Use bigger icon size". Its
label becomes "Text size" (the drawer label "Card text size" is removed), and its
range narrows to 0.8 to 1.2 (TYP-F9). A saved value outside the new range is
clamped (TYP-F10). The top line of the window keeps the system size (TYP-F11).
Persistence is unchanged for now (open question 2).

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
| TYP-F1 | Every text size in K3 is a ratio of one base size, the system default font (`Kirigami.Theme.defaultFont`). | [Implemented] |
| TYP-F2 | A user-facing text-size setting multiplies the base size of TYP-F1. It does not replace it. | [Planned] |
| TYP-F3 | Apart from the Kirigami built-ins of TYP-F5 and the one K3-set exception of TYP-F6, every K3 text is displayed at the base size of TYP-F1, multiplied only by the text-size setting of TYP-F2. K3 uses no smaller or larger text roles: no ratios, and not `Kirigami.Theme.smallFont`. | [Implemented] |
| TYP-F4 | K3 expresses hierarchy and emphasis by weight (bold), and by its existing italic, opacity or colour, never by size. A section heading that K3 places itself is displayed at the base size, in bold. | [Implemented] |
| TYP-F5 | The only Kirigami built-in exceptions to TYP-F3 (see TYP-F6 for the one K3-set exception) are the sizes a Kirigami component applies by itself where K3 sets no size: the page title in the header bar, the empty-state (`PlaceholderMessage`) title, and `FormLayout` section titles (`FormData.isSection`). | [Implemented] |
| TYP-F6 | On the Selection page only, the device card's second line (the device description: files, size, used space) is displayed at 0.8 × the base size of TYP-F1 × the text-size setting of TYP-F2, so that a Storage device's line fits on one line at the default card size with one slider step to spare. The Devices page cards stay at the base size. | [Implemented] |
| TYP-F7 | The text-size setting of TYP-F2 applies to all K3 text (every page, table, form, dialog, the drawer and device cards), not only to device cards. | [Planned] |
| TYP-F8 | The text-size setting of TYP-F2 scales text only: it does not change icon sizes or spacing. A layout measure that depends on text size (such as the Backup card narrow-layout width) may follow it, as TYP-C2 allows. | [Planned] |
| TYP-F9 | The text-size setting of TYP-F2 is a slider on the Settings page, Application section, in the "Theme" row together with the theme selector and "Use bigger icon size". It is labelled "Text size", with range 0.8 to 1.2, step 0.1, default 1.0, and zoom-out / zoom-in buttons that move it by one step. It is no longer in the global drawer, and the drawer label "Card text size" is removed. *(Amended 2026-09-26: was the drawer slider "Card text size", range 0.7 to 1.3.)* | [Planned] |
| TYP-F10 | A value already saved for the text-size setting (including by beta testers) is reused at startup; it is not reset. A saved value outside the range of TYP-F9 (0.7 or 1.3, possible from an earlier build) is clamped to the nearest limit (0.8 or 1.2); a value within the range is reused unchanged. *(Amended 2026-09-26: clamping added with the narrower range.)* | [Planned] |
| TYP-F11 | The top line of the window keeps the system size and does not follow the text-size setting of TYP-F2: the collection name at the top of the drawer and the page titles, including those of the Settings and About layers. This is an exception to TYP-F7. | [Planned] |

**Known limitation (first step).** The Kirigami built-in sizes of TYP-F5 (page
title, empty-state title, `FormLayout` section titles) are derived by Kirigami
from `defaultFont`. They follow the OS font (TYP-O1) but are **not** required to
follow the text-size setting of TYP-F2 in this first step.

**Known limitation (page toolbar buttons, Linux).** Under the KDE style on Linux,
the text of the page toolbar buttons is drawn with the application font, so those
buttons currently follow the text-size setting even though they sit on the top
line of TYP-F11. This is not a requirement; a decision on it is pending with the
maintainer.

## Constructional requirements — *how it is built / limits / MUST-NOTs*

| ID | Requirement | Status |
|----|-------------|--------|
| TYP-C1 | No K3 text size MUST be an absolute value (fixed points or pixels). | [Implemented] |
| TYP-C2 | No K3 text size MUST be derived from a layout unit (`Kirigami.Units.gridUnit`, spacing tokens). Layout measures may scale with the text-size setting; font sizes must not come from layout units. | [Implemented] |
| TYP-C3 | K3 code MUST NOT set a font size other than the base of TYP-F1 multiplied by the text-size setting of TYP-F2, except the one exception of TYP-F6. A heading placed by K3 MUST NOT rely on a `Kirigami.Heading` level for its size. | [Implemented] |

---

## Open questions

These are not requirements. Each open item needs a maintainer decision before
it can become a row above.

1. ~~**Final label of the text-size slider.**~~ Resolved 2026-09-26: the label
   is "Text size" (TYP-F9).
2. **Persistence location.** The slider value is currently persisted by a QML
   `Settings` block (`savedCardScale`) in the platform-native store, not in
   `katalog3_prerelease_settings.ini`. Kept as-is for now; whether to move it is
   not decided. Any move must still honour TYP-F10 (no reset of the saved value).

---

## Manual test charter

- **TYP-O1 / F1**: change the OS default font size, restart K3, and confirm that the text on every page follows.
- **TYP-F2**: move the text-size setting and confirm that the affected text scales from the new OS base.
- **TYP-F7**: move the text-size setting and confirm that text scales in a results table, a form (for example Create), a dialog, the drawer itself and the device cards.
- **TYP-F8**: move the text-size setting from 0.8 to 1.2 and confirm that icons and spacing keep their size; only text (and text-dependent layout measures such as the Backup card narrow-layout width) changes.
- **TYP-F9**: open Settings, Application section, and confirm the "Theme" row holds the theme selector, "Use bigger icon size" and the "Text size" slider; confirm the slider runs from 0.8 to 1.2 in steps of 0.1, starts at 1.0 on a fresh settings store, and that each zoom-out / zoom-in button moves it by one step, stopping at the limits. Confirm the global drawer no longer shows a "Card text size" slider.
- **TYP-F10**: with a value already saved by an earlier build, start the new build: a saved 1.1 starts at 1.1; a saved 1.3 starts at 1.2; a saved 0.7 starts at 0.8. In each case the slider and all K3 text start at that value.
- **TYP-F11**: set the text size to 0.8 then 1.2 and confirm the collection name at the top of the drawer and the page titles (including the Settings and About layers) keep the same size, while the rest of the text changes.
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
- Icon and spacing sizes: the text-size setting does not scale them (TYP-F8).
- Making the Kirigami built-in sizes of TYP-F5 follow the text-size setting
  (known limitation of the first step).
