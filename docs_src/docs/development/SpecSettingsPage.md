---
id: SpecSettingsPage
title: Settings Page — Application and Collection Tabs
description: Layout of the K3 Settings page in two tabs, Application and Collection
---

# SETTINGS PAGE — APPLICATION AND COLLECTION TABS

![Status](https://img.shields.io/badge/Status-Approved-brightgreen) ![Implementation](https://img.shields.io/badge/Implementation-planned-lightgrey) ![Verification](https://img.shields.io/badge/Retest-pending-orange)

## Context

The K3 Settings page holds two parts of similar size, one after the other: the
settings of the **open collection** (data mode, collection, database version
and Quality check, hosted connection, import) and the **application**
preferences (behavior, theme, language, settings file). A user changing a
preference had to scroll past the whole collection part, and the reverse.

The maintainer chose on 2026-10-10 to split the page into two tabs, Application
first, rather than moving the collection part to a separate drawer page. A
separate page was set aside: its drawer label would need a new string, and it
would add a page, a drawer entry and a further split from K2. It may be
reconsidered if the collection part grows.

The page also contains a "Search" section (Layout, "Keep Selection visible with
Search and Results") that is **commented out** in the source and never shown. It
is not part of this layout; restoring it is a separate decision.

## Scope at a glance

| In scope | Not in scope |
|----------|--------------|
| The layout of the K3 Settings page (`PageSettings.qml`): the tab bar, which section goes in which tab, and which tab shows when the page opens. | The content of the sections, owned by their own specs (`SpecWindowGeometry` WIN-F4, `SpecTypography` TYP-F9, `SpecTheme`, `SpecLanguages` LNG-F5, `SpecQualityCheck` QCK-F13, `SpecCollectionOpen`, `SpecCollectionExport` CEX-F1). K2's Settings tab. `core/`. The commented-out Search section. |

---

## Operational requirements — *why / for whom*

| ID | Requirement | Status |
|----|-------------|--------|
| SET-O1 | The user finds the application preferences and the collection/database settings in two separate places on the Settings page, so changing a preference never requires going past the collection connection form, and the reverse. | [Planned] |

## Functional requirements — *what the system does*

| ID | Requirement | Status |
|----|-------------|--------|
| SET-F1 | The K3 Settings page shows a tab bar at the top with two tabs, in this order: "Application", then "Collection". The titles reuse the existing PageSettings strings `qsTr("Application")` and `qsTr("Collection")` byte-for-byte. | [Planned] |
| SET-F2 | The Application tab holds the rows of the former Application section in this order: Version, Language, Theme (including the text size and *Reset window size*), Behavior, settings file. *(Order set by the maintainer on 2026-10-10: Language before Theme, Behavior after Theme.)* The section heading "Application" is **not** shown inside the tab: the tab title replaces it. The rows are unchanged. | [Planned] |
| SET-F3 | The Collection tab holds the "Collection & Database" section (Database Mode, Collection, Database Version with the Quality check button, Database Name / hosted form) and then the "Collection Import & Synchronization" section, each with its heading. The rows are unchanged, except for the export buttons added to "Collection & Database", which are owned by `SpecCollectionExport.md` (CEX-F1). *(Amendment approved by the maintainer on 2026-10-10.)* | [Planned] |
| SET-F4 | Opening Settings from the drawer *Settings* entry, or by restoring the last page at startup, always shows the Application tab. | [Planned] |
| SET-F5 | Opening Settings from the drawer *Open... > Hosted Database...* shows the Collection tab, with the hosted form visible. | [Planned] |

## Constructional requirements — *how it is built / limits / MUST-NOTs*

| ID | Requirement | Status |
|----|-------------|--------|
| SET-C1 | K3 only. No change to `core/` or to K2's Settings tab. | [Planned] |
| SET-C2 | The tab split adds no user-visible string. Every row keeps its existing `qsTr` text. Strings added by a section's own spec (for example the export buttons, `SpecCollectionExport.md` CEX-C3) are governed by that spec, not by this constraint. *(Amendment approved by the maintainer on 2026-10-10.)* | [Planned] |
| SET-C3 | The tab choice is not persisted: no new key in `katalog3_prerelease_settings.ini`. Beta1/beta2 settings files are unaffected. | [Planned] |

---

## Manual test charter

- **SET-F1 / SET-F2 / SET-F3** — Open Settings: the tabs are *Application* then *Collection*. The Application tab shows Version, Language, Theme, Behavior and the settings file, in that order, with no "Application" heading. The Collection tab shows "Collection & Database" then "Collection Import & Synchronization". Switch the language: both tab titles are translated.
- **SET-F4** — Open Settings from the drawer, switch to Collection, close, reopen: the Application tab shows. Quit with Settings as the last page and restart: the Application tab shows.
- **SET-F5** — Drawer *Open... > Hosted Database...*: the Collection tab shows with the hosted form.
- **SET-C3** — After the steps above, the settings file has no new key.
- Re-run the charter lines of CEX-F1 / CEX-F2: the export buttons are in "Collection & Database" on the Collection tab.
- Re-run the charter lines of WIN-F4, TYP-F9 and QCK-F13: their rows are still reachable in their tab.
