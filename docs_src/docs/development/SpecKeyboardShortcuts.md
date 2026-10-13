---
id: SpecKeyboardShortcuts
title: Keyboard Shortcuts
description: Requirements for K3 keyboard shortcuts: Esc goes back one step; Ctrl+E explores the selected search result's folder
---

# KEYBOARD SHORTCUTS

![Status](https://img.shields.io/badge/Status-Approved-brightgreen) ![Implementation](https://img.shields.io/badge/K3-implemented-brightgreen) ![K2](https://img.shields.io/badge/K2-no%20change-lightgrey)

## Context

Beta-tester feedback on K3 3.0 beta: there is no quick way to go back one step.
To leave a page the user must reach a far-away Close button with the mouse.

This spec binds `Esc` to the same path as that Close button, without interfering
with `Esc` in dialogs, menus, popups or input-method composition. It adds no new
behaviour: `Esc` is only a second trigger for an action that already exists.
Approved by the maintainer on 2026-09-24.

K2 is out of scope: it is built on tabs and has no notion of going back.

> **Reading the requirement IDs.** Each requirement has a permanent ID.
> IDs are never renumbered or reused; a retired requirement is marked
> `[Removed]`, not deleted. Status is one of
> `[Implemented] / [Planned] / [Backlog] / [Removed]`.

---

## Scope at a glance

**In scope:** the `Esc` key in K3 (`qt_quick/`) as a trigger for the existing
Close action of overlay layers and pages, or the existing Cancel action on Device
Edit; the guards that leave `Esc` to open popups and to input-method composition; `Ctrl+E` (`Cmd+E` on macOS) on the Search Results page as a second trigger of the existing "Explore folder" row action (`SRS-F7`/`SRS-F8`, `SpecSearchResults.md`), added 2026-10-13.

**Out of scope:** K2 (`qt_widgets/`). Any other shortcut. The existing Quit and Find (`Ctrl+F`) bindings are not specified here. `Ctrl+F` is an open
question, recorded in `SpecBacklogNotes.md` and not a requirement.

---

## Operational requirements — *why / for whom*

| ID | Requirement | Status |
|----|-------------|--------|
| KBS-O1 | A user can go back one step from the page they are on with the keyboard, without moving the mouse to a far-away Close button. | [Implemented] |
| KBS-O2 | A user browsing Search results with the keyboard can jump to the selected result's folder in Explore without opening the context menu. Requested in beta feedback #3; approved by the user on 2026-10-13. | [Implemented] |

## Functional requirements — *what the system does*

| ID | Requirement | Status |
|----|-------------|--------|
| KBS-F1 | `Esc` triggers exactly the action of the visible Close button of the last open page, or of Cancel on Device Edit. The target is chosen in this order: (1) if an overlay layer is open (Settings, About, Backup mapping form, Backup preview form, or Tags opened from Explore by "Tag this folder" — `EXP-F23`, `SpecExplore.md`), that layer's Close *(Tags layer added 2026-10-10)*; (2) otherwise the rightmost page in the page stack: Results goes back to Search; Search to Selection; Devices, Explore, Create, Statistics, Tags and Backup to Selection, except Explore opened from Results by "Explore folder" (`SRS-F8`, `SpecSearchResults.md`), whose Close goes back to Results *(amended 2026-10-03)*; Device Edit runs its Cancel; (3) with Selection as the only page, `Esc` does nothing. | [Implemented] |
| KBS-F2 | On Device Edit and Create, `Esc` discards unsaved input exactly as their Cancel and Close buttons do today. No confirmation is added. This is a deliberate maintainer decision. | [Implemented] |
| KBS-F3 | `Esc` pressed while a text field has focus still closes the page, per KBS-F1. The exception is input-method composition: while text is being composed, the input method keeps `Esc` and the page stays open. *(The input-method composition exception is not yet verified.)* | [Implemented] |
| KBS-F4 | Any open popup keeps its own `Esc`: a dialog (modal or not), a menu, a combo dropdown, or another popup such as the Search Results catalogs filter. That `Esc` closes only the popup; the page underneath stays open. | [Implemented] |
| KBS-F5 | On the Search Results page, `Ctrl+E` (`Cmd+E` on macOS) triggers exactly the "Explore folder" action (`SRS-F8`) for the table's selected row. It acts only under the `SRS-F7` condition: a row is selected and it belongs to a catalog device. Otherwise it does nothing. It works whether or not the table has focus, and only while Results is the current page. It does not fire while a popup, menu or dialog is open (as `KBS-F4`). | [Implemented] |
| KBS-F6 | The "Explore folder" entry of the Results row context menu shows the `Ctrl+E` shortcut hint, in the platform's native shortcut text, where the Qt Quick Controls style displays shortcuts. | [Implemented] |

## Constructional requirements — *how it is built / limits / MUST-NOTs*

| ID | Requirement | Status |
|----|-------------|--------|
| KBS-C1 | `Esc` MUST call the existing Close or Cancel handler of its target, for example `closeFeaturePage()` or the Search and Results Close actions in `qt_quick/Main.qml`. The close logic MUST NOT be duplicated or reimplemented. | [Implemented] |
| KBS-C2 | No user-visible string is added, changed or removed. | [Implemented] |
| KBS-C3 | One press of `Esc` closes at most one layer or page. Kirigami's own handling MUST NOT also close something on the same press. *(Measured on 2026-09-24 with an offscreen QtTest probe on Qt 6.11.2 and the installed Kirigami: Kirigami does not pop a layer on `Esc` by itself.)* | [Implemented] |
| KBS-C4 | `Esc` MUST NOT stop or cancel a running operation. It never triggers the Create page's Stop, the Backup page footer's Cancel for a running preview, or the activity panel's Stop. Closing a page leaves the work running, as `OPQ-O6` requires. | [Implemented] |
| KBS-C5 | K2 (`qt_widgets/`) MUST NOT change. Its only shortcut remains Quit (`qt_widgets/mainwindow.cpp:127`). | [Implemented] |
| KBS-C6 | `Ctrl+E` MUST call the same handler and use the same enable condition as the "Explore folder" menu entry (`pageSearchResults_column.exploreFolderRequested`, `appManager1.searchRowDeviceId(row) > 0`). It MUST NOT duplicate or reimplement the Explore opening (`SRS-C3`). It adds no user-visible string (the hint is Qt's native key text). It changes nothing in `core/` or `qt_widgets/`, creates no new file and adds no settings key. | [Implemented] |

---

## Evidence

An offscreen QtTest probe on Qt 6.11.2 with the installed Kirigami (2026-09-24)
showed the following:

- A window-level `Shortcut` for `Esc` does **not** fire while a Menu, a modal or
  non-modal Dialog, or a ComboBox popup is open. The popup closes instead, which
  matches KBS-F4.
- The same shortcut **does** fire when a TextField has focus, which matches KBS-F3.
- Kirigami does not pop a layer on `Esc` by itself, which matches KBS-C3.
- Input-method composition was **not** tested. The KBS-F3 exception is still
  unverified.

---

## Manual test charter

- **KBS-F1 (layers)**: open Settings, then press `Esc`: Settings closes. Repeat for About, the Backup mapping form and the Backup preview form. From Explore, use "Tag this folder" on a directory tree row, then press `Esc`: the Tags layer closes and you are back on Explore, same folder (`EXP-F23`).
- **KBS-F1 (pages)**: open Search, run a search to open Results, then press `Esc` three times. You land on Search, then Selection, then nothing more happens. Open each of Devices, Explore, Create, Statistics, Tags and Backup: `Esc` returns to Selection. From Results, use "Explore folder", then press `Esc`: you are back on Results, unchanged. With Selection alone, `Esc` does nothing.
- **KBS-F2**: type into Device Edit, then press `Esc`: the edit is discarded as with Cancel. Repeat on Create.
- **KBS-F3**: focus a text field on Search, then press `Esc`: the page closes. With an input method (for example a CJK IME), start composing and press `Esc`: the composition is cancelled and the page stays open.
- **KBS-F4**: with a dialog, a context menu, a combo dropdown and the Search Results catalogs filter each open in turn, press `Esc`. Only the popup closes.
- **KBS-C1 / C2**: grep `qt_quick/` for the `Esc` binding. It calls the existing handlers and nothing else. `ninja translations_lupdate` shows no new source text.
- **KBS-C3**: on every layer and page, one `Esc` goes back exactly one step.
- **KBS-C4**: start a catalog creation and press `Esc` on Create. The page closes and the creation carries on in the activity panel. Repeat on Backup while a preview runs.
- **KBS-C5**: K2's diff is empty.
- **KBS-F5**: on Results, select (by mouse or arrow keys) a catalog result and press `Ctrl+E` (`Cmd+E` on macOS). Explore opens on its folder, as with the menu entry. Press `Esc`: you are back on Results. Select a result of a search run directly on a disk folder and press `Ctrl+E`: nothing happens. Click outside the table so it loses focus, then press `Ctrl+E`: it still works. With the context menu or the catalogs filter open, `Ctrl+E` does nothing. On any page other than Results, `Ctrl+E` does nothing.
- **KBS-F6**: right-click a result. "Explore folder" shows `Ctrl+E` (`⌘E` on macOS) where the style supports it.
- **KBS-C6**: `ninja translations_lupdate` shows no new source text. The `core/` and `qt_widgets/` diffs are empty. The shortcut's handler is the menu entry's handler.
