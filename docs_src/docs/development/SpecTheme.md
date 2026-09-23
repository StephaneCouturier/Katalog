---
id: SpecTheme
title: Theme, colour derivation and icon size
description: How K3 derives its surface, row and highlight colours from the desktop colour scheme, what the Theme setting is allowed to change, and the shared bigger-icon-size preference.
version: "2.13"
---

# Theme, colour derivation and icon size

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

**In scope:** K3 colour derivation — the file-list row pair, and the reach of the
three `Settings/Theme` values on K3's derived colours — and the **icon size
preference** of `THM-F7`, which belongs here because it is stored in the same
`Settings/Theme*` key family and shared with K2 — and the **Explore folder list's** row surfaces
(`THM-F8`, `THM-F9`, `THM-C11`) — and the surface of the **Selection page's
`SEL-F5` reminder row** (`THM-F10`, `THM-C13`, `THM-C14`). *(Scope widened
2026-09-12, first from colour derivation alone, then to the folder list; widened
again 2026-09-21 to the reminder row.)* — and the operational goal `THM-O2`,
whose delivery is owned by `SpecK3Deployment.md` (`DPL-F5`). *(Widened
2026-09-23.)*

**Out of scope (non-goals):**

- **K2 (`qt_widgets`)** — in maintenance mode. It shares the same
  `Settings/Theme` key and keeps its own Katalog Colors rendering, untouched
  (`THM-C5`).
- **Icon theme** — icon set selection is not governed here; `THM-F7` sets a
  size, never which icon set is used.
- **Which K3 views honour `THM-F7`** beyond the Devices cards — see `THM-C10`.
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
| THM-O2 | A user who has set their system to dark (or light) sees K3 in that mode as a **whole window**, not a mix of dark surfaces and light controls — in packaged builds on non-Plasma Linux desktops, on Windows and on macOS, as well as on Plasma. Delivered by packaging, per `DPL-F5` / `DPL-C6` (`SpecK3Deployment.md`). | [Planned] |

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
| THM-F7 | The Settings page offers a **bigger icon size** choice, as K2 does. Unset, icons are drawn at the smaller size; set, at the larger one. K2's pixel values map onto the Kirigami size tokens: checked → the *medium* token (32), unchecked → the *small-medium* token (22). The user asked for this port on 2026-09-12 and chose the Settings page, as in K2, over a per-page control. | [Planned] |
| THM-F8 | The `THM-F1` / `THM-F2` row pair **also applies to the Explore folder list**, identically to the file list beside it — the folder rows stripe. *(Added 2026-09-12. `THM-F3` named only the Search results and Explore **file** lists, so the folder list was never covered by any row; it painted no surface of its own at all.)* The list is a tree and its rows are indented, which was the argument for leaving it unstriped; it is overruled because K2 stripes its own Explore directory tree (`alternatingRowColors` on `Explore_treeview_Directories`, `qt_widgets/mainwindow.ui`) and because two lists side by side on one page with different row surfaces is the more visible inconsistency. | [Planned] |
| THM-F9 | A row surface in these lists is **painted by the application, not left to the active Qt Quick Controls style**. The Explore folder list drew no background of its own, so its rows took the style's — Breeze under Plasma, Fusion on every other platform (`qt_quick/main.cpp:32-34`) — while their text colour came from Kirigami either way, which is how a Kirigami text colour ended up on a Fusion background. The user reported the resulting difference between Linux and Windows on 2026-09-12. The lists governed by `THM-F3` and `THM-F8` MUST look the same under either style. | [Planned] |
| THM-F10 | The `SEL-F5` selection reminder at the top of the Selection page (`SpecSelection.md`) paints **its own** surface instead of taking the active Qt Quick Controls style's: **transparent when idle**, so the tinted Selection page surface shows through, and a **hover tint from `Kirigami.Theme`** — `hoverColor`, see `THM-C13` — while the pointer is over it, because the row is clickable (`SEL-F6`). Reported 2026-09-21 against the 3.0.beta2 AppImage on a non-Plasma desktop, and measured against the real styles: under **Fusion** — the style every packaged build resolves to, see `SpecK3Deployment.md` *Known limitations* — `Controls.ItemDelegate`'s background is an opaque `#ffffff` rectangle with `visible: true` that paints unconditionally, so the row showed as a white band; under `org.kde.desktop` the same delegate is Kirigami's `DefaultListItemBackground`, transparent until hovered or selected, which is why a local development build never showed it. This is `THM-F9` applied to **one further element**, not a sweep. | [Planned] |

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
| THM-C8 | `THM-F7` is stored in the **existing** key `Settings/ThemeBiggerIconSize`, the one K2 already writes (`qt_widgets/mainwindow_tab_settings.cpp:624-645`, restored at `qt_widgets/mainwindow_setup.cpp:384`), so both versions share one preference. K2 stores the Qt check state (`2` / `0`) and reads it with `toBool()`, so a K3 boolean round-trips through it; K3 MUST NOT introduce a second key, nor change the value encoding. | [Planned] |
| THM-C9 | `THM-F7` reuses K2's existing label **`Use bigger icon size`** byte-for-byte — approved per string by the user on 2026-09-12 — so the 30 existing translations carry it and no slot is spent. **No other new string** is authorised by these rows. K2 MUST NOT be modified, per `THM-C5`. | [Planned] |
| THM-C10 | `THM-F7` is consumed by the **Devices page cards** (`DVP-F20`, `SpecDevicesPage.md`) and the **Selection page cards** (`SEL-F7`, `SpecSelection.md`) — and by nothing else. *(Amended 2026-09-12: the Selection page was added at the user's explicit request, which is the "separate request" this row originally reserved. The mechanism worked as intended: the widening was asked for, not assumed.)* K2 applies its equivalent to all eight of its tree views; K3 MUST NOT be widened to every icon in the application as a side effect. Each further view honouring the setting remains a separate request. | [Planned] |
| THM-C11 | The Explore folder delegate consumes the colours defined once in `Main.qml` — the two row colours and the selection highlight — and MUST NOT derive, hardcode or locally redefine any of them, exactly as `THM-C1` requires of the file-list delegates. `THM-C1`'s wording *"one definition, two consumers"* becomes **three consumers**; the definition stays single. The selected row keeps the highlight of `THM-F5`, and the highlighted text colour it already sets is unchanged — only the surface beneath it is now painted. | [Planned] |
| THM-C12 | A **code comment is not a requirement.** `Main.qml` already claimed these colours were consumed by *"explore files and folders"* while the folder delegate consumed neither. The comment was ahead of both the code and the spec; it described an intention, and the requirement is `THM-F8`, not the comment. Correcting the comment is part of this work; it MUST NOT be cited as authorisation for anything. | [Planned] |
| THM-C13 | `THM-F10` is **one** QML element: the `background` of the `Controls.ItemDelegate` in the `pageSelection` header (`qt_quick/Main.qml`). It MUST NOT change the reminder's content, its `SEL-F6` scroll behaviour or `SEL-C7`'s prohibition on changing or clearing the selection; MUST NOT touch the shared `DeviceIdentity` component (`SEL-C1`); adds, changes and removes **no** user-visible string; and changes nothing under `core/`. `background: null` was considered and **rejected** — approved per option by the maintainer on 2026-09-21 in favour of the hover tint: `null` removes hover feedback from a row that is clickable, and `CDT-F3` (`SpecCardsAndTables.md`) already decided that such feedback is what tells the user a target is clickable. The idle state MUST be fully transparent and the hover tint MUST come from `Kirigami.Theme` — no literal colour constant, per this page's *Context*. As built: the delegate (`id: selectionReminder`, added so the binding can reference it) reads **`Kirigami.Theme.hoverColor` inline**, with `"transparent"` as the idle value. The tint is deliberately **not** lifted to an `applicationWindow()`-level property: it has exactly one consumer, and `THM-C1`'s single-definition rule covers the two row colours and the highlight, not this. Whoever takes on the parked sweep of the other seven delegates may lift it then; nothing here promises it. `"transparent"` is the absence of a surface, not a colour constant, and MUST NOT be reported as a breach of the no-literals rule. | [Implemented] |
| THM-C14 | Painting K3's own surfaces so they look the same under any Controls style is **not** the work `DPL-C4` withholds. `DPL-C4` (`SpecK3Deployment.md`) withholds **bundling** `qqc2-desktop-style`, `frameworkintegration` or Breeze icon files into a package; it does not reserve K3's own QML. `THM-F9` and `THM-F10` take the opposite approach — make the application independent of the style it is given — so they need no amendment to `SpecK3Deployment.md`, and packaged builds continue to resolve to Fusion. That page's *Known limitations* are the **cause** of the `THM-F10` report and MUST NOT be cited as its authorisation: `SpecK3Deployment.md` authorises no `qt_quick` source change, and the authority for `THM-F10` is this page. | [Implemented] |

---


---

## Open, not authorised

| Item | Detail |
|------|--------|
| Other lists that inherit the platform style | The same pattern — a bare `Controls.ItemDelegate` painting no surface of its own — exists in **seven** other places: the device tree combo box's popup rows (`DeviceTreeComboBox.qml`); the search history list rows and the file-type combo popup rows (`PageSearchForm.qml`, two places); the file-type combo popup rows of the Create form (`PageCreateForm.qml`) and of the device editor (`PageDeviceEditForm.qml`); the batch-action combo popup rows (`PageSearchResultsForm.qml`); and the language combo popup rows (`PageSettings.qml`). Each will show the same difference between Linux and Windows, and between a local and a packaged build. Only what the user reports is ever authorised: so far the **Explore folder list** (`THM-F8`, `THM-F9`, reported 2026-09-12) and the **Selection reminder row** (`THM-F10`, reported 2026-09-21 — it has left this list). A sweep across the seven that remain is a separate request and MUST NOT be carried in behind either fix. *(Line numbers were dropped on 2026-09-21: every number first recorded here had drifted — `Main.qml:1304`, the reminder row, had become `Main.qml:1399` — and a stale line number is worse than none, so the elements are identified by file and role instead.)* |

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
- **THM-F7 / THM-C8 (shared key)** — Tick *Use bigger icon size* in K3, then open K2: its own checkbox is ticked and its tree icons are the larger size. Untick it in K2 and reopen K3: K3 draws the smaller icons. Confirm only `Settings/ThemeBiggerIconSize` is written, with no second key.
- **THM-F7 (sizes)** — With the option off, a Devices card icon and a Selection card icon both match the small-medium token; with it on, both match the medium token. The change takes effect without restarting.
- **THM-C9** — Run `ninja translations_lupdate`: **no** new untranslated string appears. With the interface in French, the Settings label is translated, proving the K2 string was reused verbatim.
- **THM-C10** — With the option on, confirm the Devices cards and the Selection cards both follow it, and that icons elsewhere in the application — the drawer, the toolbars, the other pages — are unchanged.
- **THM-F8** — On the Explore page, compare the folder list with the file list beside it: the same two row colours in the same parity, the first row of each taking the View colour. Confirm a selected folder row takes the selection highlight and that its text stays legible on it.
- **THM-F9 (Fusion)** — Run the application with the Fusion style forced (`QT_QUICK_CONTROLS_STYLE=Fusion`) and compare the Explore page with the same page under `org.kde.desktop`: the folder rows and the file rows are the same colours in both, because the application paints them. This is the Linux stand-in for the Windows report and MUST be run before the fix is called done.
- **THM-F9 / THM-F6** — With the folder list open, change the desktop colour scheme from light to dark: the folder rows follow the new scheme without a restart, as the file rows do.
- **THM-C11** — Grep the folder delegate: it references the application-level colour properties and defines no colour of its own.
- **THM-F10 (Fusion)** — Run the application with the Fusion style forced (`QT_QUICK_CONTROLS_STYLE=Fusion`) and compare the Selection page with the same page under `org.kde.desktop`: in both, the reminder row at the top shows the tinted Selection page surface, with **no white band** and no surface of its own while the pointer is elsewhere. *Open: not yet run. The QML was applied on 2026-09-21 without launching the application.*
- **THM-F10 (hover)** — Move the pointer over the reminder row under each style: the same light tint appears in both, and it disappears when the pointer leaves. Compare it with a Selection card, which keeps its own card surface either way. *Open: not yet run.*
- **THM-F10 (packaged build)** — Repeat the first two checks against the **AppImage** on a non-Plasma desktop, the case that was reported. This is the check that closes the row; until it is confirmed there, `THM-F10` stays `[Planned]`. *Open: the QML is in as of 2026-09-21 and builds, but no visual check has been made on any style — this is the only thing between `THM-F10` and `[Implemented]`.*
- **THM-C13** — Click the reminder row: the list still scrolls to the selected card and the selection is unchanged (`SEL-F6`, `SEL-C7`). Run `ninja translations_lupdate`: no new untranslated string appears. Review the diff: one `background` in `qt_quick/Main.qml`, nothing under `core/`, and no change to `DeviceIdentity.qml`. *Verified 2026-09-21 by diff review: one file changed, no `core/` file, no `DeviceIdentity.qml`, no `tr(` or `qsTr(` added, changed or removed; `Katalog3_qmllint` reports nothing on the inserted lines and the build links. The click behaviour was not re-run — the change adds a `background` and an `id` and touches no handler.*
- **THM-C14** — Review the same diff against `SpecK3Deployment.md`: no Breeze icon file is copied into the AppDir and neither `qqc2-desktop-style` nor `frameworkintegration` has been added to the bundle, so `DPL-C4` still holds. The packaged build still resolves to Fusion — and now looks the same as the local build on this row anyway. *Verified 2026-09-21: the diff is one `qt_quick` file and touches no part of the bundle.*
- **THM-O2** — Run the three `DPL-F5` checks in `SpecK3Deployment.md`. On each OS, set the system to dark: the Search results and Explore lists show the Breeze Dark View/Window row pair (`THM-F1`), and the surrounding controls are dark too. Nothing in the window stays light.
