---
id: SpecApplicationIcon
title: Application Icon and Desktop Integration
description: Requirements for the window icon, the installed desktop entry, and the installed hicolor icon set, for both the Qt Widgets and Qt Quick user interfaces
---

# APPLICATION ICON AND DESKTOP INTEGRATION

![Status](https://img.shields.io/badge/Status-Approved-brightgreen) ![Part 1](https://img.shields.io/badge/Part%201%20defect%20fix-implemented-brightgreen) ![Part 2](https://img.shields.io/badge/Part%202%20packaging-planned-blue) ![K2](https://img.shields.io/badge/K2-2.13-blue) ![K3](https://img.shields.io/badge/K3-3.0-blue)

## Context

On Plasma Wayland the Katalog titlebar icon renders with a black rim and stray
impossible colours, in both user interfaces. The cause is not the artwork. Both
UIs set the window icon from a multi-entry `.ico` whose smallest entry is 16x16;
Qt 6.9 and later hand every entry of that icon to the compositor through
`xdg_toplevel_icon_v1`, and the compositor picks the entry nearest its advertised
icon size. KWin advertises 96; the nearest available entries are small, and a
small buffer *upscaled* into the titlebar slot magnifies a per-pixel colour
error instead of averaging it away.

The per-pixel error itself is an upstream Qt defect: qtwayland allocates those
icon buffers as `QImage::Format_ARGB32` (straight alpha) while declaring
`WL_SHM_FORMAT_ARGB8888`, which Wayland defines as premultiplied. Katalog cannot
fix that. What Katalog controls is which buffer it offers. Offering a single
large buffer makes the compositor downscale rather than upscale, and the error
becomes invisible. That is the mitigation this spec requires, and it is recorded
as a mitigation rather than a preference so it is not "tidied away" later.

Measurements, the Wayland protocol trace and the A/B probe results are in
`SpecApplicationIconBacklogNotes.md`. They are evidence, not requirements, and
are deliberately kept out of the tables below.

This spec covers two separable pieces of work:

- **The defect fix** (`ICO-F1`, `ICO-F2`) — which asset each platform sets the
  window icon from. This alone resolves the reported corruption.
- **Desktop integration** (`ICO-F3`, `ICO-F4`) — installing a desktop entry and
  a full hicolor icon set. This is a separate improvement, not part of the
  defect, and neither depends on the other.

> **Reading the requirement IDs.** Each requirement has a permanent ID.
> IDs are never renumbered or reused; a retired requirement is marked
> `[Removed]`, not deleted. Status is one of
> `[Implemented] / [Planned] / [Backlog] / [Removed]`.

---

## Scope at a glance

**In scope:** the icon each platform sets as the window icon in K2 and K3; the
desktop entry installed on Linux; the hicolor icon sizes installed on Linux; the
constraints that keep all of the above from regressing.

**Out of scope (non-goals):** the artwork itself — no icon is redrawn or
restyled by this spec. Fixing the upstream qtwayland premultiplied-alpha defect;
Katalog mitigates, it does not patch Qt. In-application icons (toolbar, action
and device icons) — those come from the icon theme and are untouched. The
Windows installer and the macOS bundle layout beyond the icon resource. Any
change to the desktop entry's translated strings. Flatpak or other downstream
packaging manifests.

---

## Operational requirements — *why / for whom*

Goals in real use, independent of how they are built.

| ID | Requirement | Status |
|----|-------------|--------|
| ICO-O1 | A user sees the Katalog icon rendered cleanly in the window titlebar, the task manager and the window switcher, on every supported platform and in both user interfaces. | [Planned] |
| ICO-O2 | A user who installs Katalog finds it in the application launcher under its own name and icon, and can start it from there, whichever user interface is installed. | [Planned] |

## Functional requirements — *what the system does*

Observable behaviour that can be triggered and watched.

| ID | Requirement | Status |
|----|-------------|--------|
| ICO-F1 | On Linux, both K2 and K3 set the window icon from the 256-pixel PNG master, not from the `.ico`. | [Implemented] |
| ICO-F2 | On Windows and macOS, both K2 and K3 continue to set the window icon from the `.ico`. | [Implemented] |
| ICO-F3 | Installing either user interface on Linux installs exactly one desktop entry, named `io.github.stephanecouturier.Katalog.desktop`, into `share/applications`. | [Planned] |
| ICO-F4 | Installing either user interface on Linux installs the application icon at each of the eight sizes listed in *Installed icon set* below, into `share/icons/hicolor/<size>x<size>/apps`. | [Planned] |

## Constructional requirements — *how it is built / limits / MUST-NOTs*

Boundaries and implementation constraints, not user-visible behaviour.

| ID | Requirement | Status |
|----|-------------|--------|
| ICO-C1 | The image handed to the compositor as the Linux window icon MUST be a single large raster. It MUST NOT be a multi-entry `.ico`, and MUST NOT be any image smaller than 256 pixels. This is the root-cause guard: a smaller buffer is upscaled into the titlebar slot and exposes the upstream alpha defect. | [Implemented] |
| ICO-C2 | The `.ico` MUST be retained. It is the Windows `.rc` resource and the macOS bundle icon, and it stays listed in both `images.qrc` files. It MUST NOT be deleted as "unused" on the strength of the Linux change. | [Implemented] |
| ICO-C3 | K2 sets its Linux window icon in C++, immediately after `setupUi`. The `windowIcon` property in `mainwindow.ui` MUST NOT be used as the platform-conditional mechanism, and MUST NOT be removed: it remains the Windows and macOS value and the designer-visible icon. The C++ call overrides it on Linux only. | [Implemented] |
| ICO-C4 | The K2 change is limited to the icon source. No other K2 UI change is authorised by this spec — K2 is in maintenance mode. | [Implemented] |
| ICO-C5 | The installed icons are pre-generated PNG files committed under `assets/`. They MUST NOT be produced by a build-time image conversion step: no ImageMagick or equivalent may be added as a build dependency, because the Windows build must remain viable. | [Planned] |
| ICO-C6 | K2 and K3 both install as `bin/Katalog` and deliberately share one desktop entry and one application id. They are mutually exclusive installs and MUST NOT be installed into the same prefix at the same time. A second desktop file, a second application id, or a distinct `Exec`/`StartupWMClass` for K3 MUST NOT be created. | [Planned] |
| ICO-C7 | The desktop entry is reused as-is. Its `Name`, `GenericName`, `Comment` and `Keywords` values, and all of their localised variants, MUST NOT be edited, reordered or re-generated by this work. | [Planned] |
| ICO-C8 | No user-visible application string is added, changed or removed by this spec. | [Implemented] |
| ICO-C9 | No `core/` change. This spec touches only UI entry points and build files. | [Implemented] |

---

## Installed icon set

The complete and exclusive size list authorised by `ICO-F4`. Every file is a
pre-generated PNG committed under `assets/icons/`, derived from the master
`assets/Katalog_logo_256.png`, and installed with `RENAME` to
`io.github.stephanecouturier.Katalog.png` in its size directory.

| Size | Source asset | Installed to |
|------|--------------|--------------|
| 16 | `assets/icons/katalog_16.png` | `share/icons/hicolor/16x16/apps` |
| 22 | `assets/icons/katalog_22.png` | `share/icons/hicolor/22x22/apps` |
| 24 | `assets/icons/katalog_24.png` | `share/icons/hicolor/24x24/apps` |
| 32 | `assets/icons/katalog_32.png` | `share/icons/hicolor/32x32/apps` |
| 48 | `assets/icons/katalog_48.png` | `share/icons/hicolor/48x48/apps` |
| 64 | `assets/icons/katalog_64.png` | `share/icons/hicolor/64x64/apps` |
| 128 | `assets/icons/katalog_128.png` | `share/icons/hicolor/128x128/apps` |
| 256 | `assets/Katalog_logo_256.png` | `share/icons/hicolor/256x256/apps` |

Seven new shipped assets are required, at 16 through 128. The 256 row
deliberately reuses the existing `assets/Katalog_logo_256.png` rather than adding
a copy under `assets/icons/`: that file is simultaneously the generation master
and the Linux window icon of `ICO-F1`, and a duplicate could drift from it.

Installing a size set rather than a single 256 master lets the launcher and the
task manager pick an exact match instead of rescaling. This is separate from
`ICO-C1`: the window icon handed to the compositor is still the single 256
raster, precisely because the compositor rescales it itself.

## Window icon sources

The complete mapping authorised by `ICO-F1` and `ICO-F2`.

| Platform | Asset | K2 set at | K3 set at |
|----------|-------|-----------|-----------|
| Linux | `:/images/Katalog_logo_256.png` | `qt_widgets/mainwindow.cpp:115-124`, under `Q_OS_LINUX`, immediately after `ui->setupUi(this)` | `qt_quick/main.cpp:36-47`, under `Q_OS_LINUX`, before the QML engine loads |
| Windows | `:/images/Katalog_logo_64.ico` | `qt_widgets/mainwindow.ui:31-35` property | `qt_quick/main.cpp:36-47`, `#else` branch |
| macOS | `:/images/Katalog_logo_64.ico` | `qt_widgets/mainwindow.ui:31-35` property | `qt_quick/main.cpp:36-47`, `#else` branch |

`Katalog_logo_256.png` is registered at `qt_widgets/images.qrc:4` and
`qt_quick/images.qrc:47`. The `.ico` entries alongside them are retained per
`ICO-C2`.

K2 takes no `#else` branch: on Windows and macOS the `.ui` property is simply
left in force, which is what `ICO-C3` requires and why that property must not be
removed as dead.

The delivered change is 23 insertions and 0 deletions across those four files.
Nothing was removed, which is the direct evidence for `ICO-C2` and `ICO-C3`.

---

## Manual test charter

For each row: set up the stated condition, run the operation, confirm the result.

- **ICO-O1 / ICO-F1** — On Plasma Wayland, start K2 and then K3. In each, look at the titlebar icon, the task manager entry and the Alt-Tab switcher. The icon is clean: no black rim, no stray colours at any edge. Compare against a screenshot taken before the change to confirm the rim is gone.
- **ICO-F2** — Build and run on Windows, and on macOS. The window icon, the taskbar or Dock icon, and the executable's own icon are unchanged from the previous release.
- **ICO-C1** — Run either UI under `WAYLAND_DEBUG=1` and capture the `xdg_toplevel_icon_v1` traffic. Exactly one `add_buffer` call is made, and it carries the 256 raster. Four buffers, or any buffer smaller than 256, is a failure. *Verified after implementation:* `QIcon(":/images/Katalog_logo_256.png")` reports `availableSizes count=1` at 256x256, so Qt offers a single buffer where the `.ico` previously offered four (16/32/48/256). That is the configuration measured clean as probe B2.
- **ICO-C2** — Confirm the `.ico` is still present in `assets/`, still listed in both `.qrc` files, and still referenced by the Windows `.rc` and the macOS bundle configuration.
- **ICO-C3** — Open `mainwindow.ui` in Designer: the `windowIcon` property is still set to the `.ico`. Then run K2 on Linux and confirm the effective icon is the PNG, proving the C++ call overrides it. Then build for Windows and confirm the effective icon is the `.ico`.
- **ICO-C4** — Review the K2 diff: it touches the icon source and nothing else. No layout, no string, no behaviour change.
- **ICO-C5** — Configure a clean build tree with no ImageMagick installed. Both UIs configure and build successfully. `grep` the CMake files for any image conversion command: none is present.
- **ICO-C6** — Install K2 into a clean prefix: `bin/Katalog`, one desktop entry, eight icons. Remove it, install K3 into the same clean prefix: the same `bin/Katalog`, the same single desktop entry, the same eight icons. Confirm only one desktop entry exists in `share/applications` in each case.
- **ICO-F3 / ICO-O2** — After installing either UI, open the application launcher and search for Katalog. One entry appears, with the correct name, description and icon. Launching it starts the installed UI.
- **ICO-F4** — List `share/icons/hicolor/*/apps/` after install. All eight sizes are present, each named `io.github.stephanecouturier.Katalog.png`, and each opens as a valid PNG at its nominal size.
- **ICO-C7** — Diff the installed desktop entry against `packaging/qt_widgets/Katalog.desktop`: byte-identical apart from the filename. Confirm every localised `Name`, `GenericName`, `Comment` and `Keywords` line is unchanged.
- **ICO-C8** — Review the full diff for `tr(` and `qsTr(`: no addition, no change, no removal.
- **ICO-C9** — Confirm no file under `core/` is modified.
