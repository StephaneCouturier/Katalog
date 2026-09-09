---
id: SpecApplicationIconBacklogNotes
title: Application Icon — Research Notes
description: Evidence, measurements and upstream defect analysis behind SpecApplicationIcon
---

# APPLICATION ICON — RESEARCH NOTES

![Status](https://img.shields.io/badge/Status-Reference-lightgrey)

Reference material for `SpecApplicationIcon.md`. **Nothing here is a
requirement.** These are the measurements that justified `ICO-C1`, kept out of
the requirement tables so the tables stay testable statements. If any of this
becomes a rule, it must be promoted into a numbered row in the spec first.

## Environment observed

Plasma Wayland, Qt 6.11.2, KWin 6.7.4. The corruption reproduces in both K2 and
K3, which is what first ruled out the UI layer as the cause.

## Root cause — upstream qtwayland defect

Qt 6.9 and later implement `xdg_toplevel_icon_v1`; KWin 6.7 honours it.
`QWaylandXdgToplevelIcon` allocates its icon buffers as
`QImage::Format_ARGB32` — straight alpha — while declaring
`WL_SHM_FORMAT_ARGB8888`, which the Wayland specification defines as
**premultiplied**. Every pixel whose colour channels exceed its alpha channel is
therefore invalid as premultiplied data, and the compositor renders it as an
out-of-range colour.

This is a Qt defect. Katalog cannot fix it and does not attempt to. Katalog
controls only *which* buffer it offers, which is what `ICO-C1` constrains.

Invalid-as-premultiplied pixels measured per buffer, from the `.ico`:

| Buffer | Bad pixels |
|--------|-----------|
| 16 | 53 |
| 32 | 144 |
| 48 | 239 |
| 256 | 364 |

## Why the size matters more than the count

The 256 buffer has the *most* bad pixels in absolute terms, yet looks clean. The
titlebar slot is roughly 19 pixels. A 256 buffer is downscaled into it by a
factor of about 13, so each bad pixel is averaged against many good neighbours
and disappears. The `.ico`'s 16x16 entry is *upscaled* to 19 pixels, so every bad
pixel is reproduced at or above full strength.

KWin advertises `icon_size(96)`. Qt offers no 96 buffer, so the compositor
selects from what it is given. With the `.ico` set, Qt sends four `add_buffer`
calls — 16, 32, 48 and 256, the `.ico`'s entries. With no icon set, Qt sends
`set_icon(toplevel, nil)` and the compositor resolves the themed icon instead.

Captured with `WAYLAND_DEBUG=1`.

## A/B probe results

Six probe windows, one variable each. Confirmed by the user and measured from
their screenshot.

| Probe | Window icon | Result |
|-------|-------------|--------|
| A | none; Katalog app id; compositor resolves themed icon | clean |
| B1 | `Katalog_logo_64.ico` | black rim, 19 near-black pixels |
| B2 | `Katalog_logo_256.png`, same artwork | clean |
| C | `firefox.png`, 64 px, foreign artwork, same code path | corrupted, 32 impossible colours |
| D | flat 64 px image with alpha | clean |
| E | flat 64 px image without alpha | clean |

Conclusions drawn:

- The artwork files are innocent. B2 uses the same artwork as B1 and is clean;
  C uses foreign artwork through the same path and is corrupted.
- The trigger is handing the compositor small icon buffers, not any property of
  the Katalog logo.
- B2 is the direct evidence for `ICO-F1`; A is the direct evidence that a
  correctly installed themed icon is also a valid resolution, which is what
  `ICO-F3` and `ICO-F4` deliver for launchers.
- D and E show that flat images survive at 64 px, which is why the constraint in
  `ICO-C1` is expressed as a minimum size rather than a ban on small icons in
  general: the defect only becomes visible when upscaling meets gradient edges.

## Upstream report

A bug report against qtwayland has been drafted covering the
`Format_ARGB32` / `WL_SHM_FORMAT_ARGB8888` mismatch described above. It is held
outside version control and is not part of this repository; ask the maintainer
for it rather than looking for a tracked path.

Katalog's mitigation stands regardless of whether that report is accepted:
`ICO-C1` constrains only which buffer Katalog offers, and remains correct even
after the upstream defect is fixed, because a 256 raster is the right thing to
hand a compositor in either case.

## Standing note — the `.ico` filename is misleading

`assets/Katalog_logo_64.ico` contains entries at 16, 32, 48 and 256. It contains
**no 64-pixel entry at all**; the `64` in its name describes nothing in the file.
This is worth knowing because the name invites the assumption that the file is a
single 64 px icon, which is precisely the assumption that made this bug hard to
see: the four buffers Qt sent to the compositor came from a file whose name
suggests one.

A rename was considered and rejected: the file is referenced from the Windows
`.rc`, the macOS bundle configuration and both `.qrc` files, and the blast radius
is larger than this bug warrants. The name stays. This note exists so the next
reader is not misled by it.

## Rejected alternatives

- **Set no window icon at all and rely on the themed icon** (probe A). Clean on
  Linux, but leaves Windows and macOS with no icon, and makes the titlebar
  depend on the desktop-file install having succeeded. Rejected as fragile.
- **Ship a 96 px buffer to match KWin's advertised size.** Fixes KWin
  specifically and nothing else; another compositor advertising a different size
  reopens the bug. Rejected as non-portable.
- **Resize icons at build time from the 256 master.** Would avoid committing
  seven PNGs, but adds an ImageMagick-class build dependency and puts the
  Windows build at risk. Rejected; see `ICO-C5`.
