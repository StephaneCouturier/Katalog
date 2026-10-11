---
id: SpecWindowGeometry
title: Window Geometry — Size and Position of the K3 Window
description: Requirements for restoring the K3 window size and position at start, and for resetting the window to its default size from Settings.
version: "2.13"
---

# WINDOW GEOMETRY — SIZE AND POSITION OF THE K3 WINDOW

![Status](https://img.shields.io/badge/Status-Approved-brightgreen) ![Version](https://img.shields.io/badge/Version-2.13-blue) ![Implementation](https://img.shields.io/badge/Implementation-partial-yellow) ![Issue](https://img.shields.io/badge/Issue-%23805-blue) ![K2](https://img.shields.io/badge/K2-no%20change-lightgrey) ![K3](https://img.shields.io/badge/K3-3.0-blue)

## Context

K3 restores its window size and position at start (issue #805, beta1 report "position
& size should be restored"). That behaviour was built and shipped without a
specification; the maintainer ratified it as it works on 2026-10-04 (`WIN-F1` to
`WIN-F3`).

The same day the maintainer asked for a way to put the window back to a default
size, for users and for the documentation screenshots, which need a reproducible
window size (`WIN-O2`). The reset is a K3-only addition with no K2 equivalent; it is
one button driving the existing mechanism, not a major UI feature, so it does not
wait for K2 parity. K2 keeps its own window handling (KConfig, default 1080×720).

---

## Scope at a glance

**In scope:** storing and restoring the K3 main window's size, maximized state and
position; the default size; the *Reset window size* button on the Settings page.

**Out of scope (non-goals):** K2's window handling; the widths of the pages inside the
window; dialog sizes; the drawer's pinned state.

---

## Operational requirements — *why / for whom*

| ID | Requirement | Status |
|----|-------------|--------|
| WIN-O1 | A user who reopens K3 finds the window at the size (and, where the platform allows, the position) they left it, without it ever opening off-screen or larger than the screen. | [Implemented] |
| WIN-O2 | A user, or the maintainer taking documentation screenshots, can return the window to a known, reproducible default size in one action, without editing settings files. | [Planned] |

## Functional requirements — *what the system does*

| ID | Requirement | Status |
|----|-------------|--------|
| WIN-F1 | When the window closes, and when the application quits without closing the window first (Cmd+Q on macOS), K3 stores the window's windowed width and height, its maximized state and its position in its settings file. The stored size and position are the **windowed** ones, so a maximized window comes back maximized and un-maximizes to the size it had before. | [Implemented] |
| WIN-F2 | On start, K3 restores the stored size, clamped to the available area of the screen it reopens on (not always the primary screen, so a window kept on a wide secondary display keeps its width). With no stored size it uses the default size of `WIN-C1`. A stored maximized state reopens maximized. | [Implemented] |
| WIN-F3 | The stored position is reused only if the centre of the title bar falls on a screen that exists now; otherwise the platform places the window. On a platform that does not let an application place its window (Wayland), the request is ignored and the platform places it. | [Implemented] |
| WIN-F4 | The Settings page, Application tab (`SpecSettingsPage.md` SET-F2), holds a text button *Reset window size* in the "Window size" row of the Interface section (`SpecSettingsPage.md` SET-F2). *(Amended 2026-10-11: was in the "Theme" row.)* Clicking it un-maximizes the window if it is maximized and resizes it to the default size of `WIN-C1`, clamped to the available area of the screen the window is on, as in `WIN-F2`. K3 does not change the position. | [Planned] |
| WIN-F5 | After `WIN-F4`, the new size is stored by the normal mechanism of `WIN-F1`, so the next start reopens at the default size. | [Planned] |

## Constructional requirements — *how it is built / limits / MUST-NOTs*

| ID | Requirement | Status |
|----|-------------|--------|
| WIN-C1 | The default window size is **1280×800** (logical pixels). It is defined once in K3 and used both at first start (`WIN-F2`) and by the reset (`WIN-F4`); there is no second copy of the value. *(Changed 2026-10-04 by the maintainer: was 900×600.)* | [Planned] |
| WIN-C2 | Geometry uses only the existing keys `Settings/WindowWidth`, `Settings/WindowHeight`, `Settings/WindowMaximized`, `Settings/WindowX`, `Settings/WindowY` of the K3 settings file. The reset MUST NOT add or delete keys, so existing beta1/beta2 settings files stay readable; a tester with a stored size keeps it until pressing the button. | [Planned] |
| WIN-C3 | The reset MUST NOT ask for confirmation, MUST NOT show a notification (the result is visible), and MUST NOT change any other setting (theme, icon size, text size, page widths). | [Planned] |
| WIN-C4 | K3 only. K2's window handling (KConfig, default 1080×720) is unchanged. | [Planned] |
| WIN-C5 | The button's only new translated string is `Reset window size` (approved by the maintainer 2026-10-04). Its icon is `edit-clear-history`, the icon of the Search *Reset* action, which exists in K3's fallback icon set (so it shows in the Windows and macOS packages). Its tooltip is the default size, `1280×800`, **not translated** (maintainer's choice, 2026-10-04) and built from the `WIN-C1` value, so the size stays defined once. | [Planned] |

---

## Manual test charter

- **WIN-F1 / WIN-F2** — Resize, close and reopen: same size. Maximize, close and
  reopen: maximized; un-maximize: the previous windowed size. With a settings file
  holding no window keys, the window opens at 1280×800 (or the screen's available
  area if smaller).
- **WIN-F3** — Move the window to a second screen, close, unplug that screen and
  reopen: the window is on a visible screen. On Wayland, the position is not
  restored and that is expected.
- **WIN-F4 / WIN-F5** — Maximize, then press *Reset window size*: the window is
  windowed at 1280×800. Resize by hand, press again: back to 1280×800. Close and
  reopen: 1280×800 is kept. Check on Linux (X11 and Wayland), Windows and macOS.
- **WIN-C1** — On a screen smaller than 1280×800, the reset and the first start both
  give a window that fits the screen's available area.
- **WIN-C5** — The button shows the reset icon; hovering it shows `1280×800` in
  every interface language.
- **WIN-C3** — After a reset, theme, icon size and text size are unchanged, and no
  dialog or notification appears.
