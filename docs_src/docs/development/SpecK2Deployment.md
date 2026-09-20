---
id: SpecK2Deployment
title: K2 Deployment — Startup of the Linux AppImage on Any Desktop
description: Requirements for the platform theme the packaged Katalog 2 Linux AppImage selects, so that it starts and reaches its main window on desktops other than Plasma
---

# K2 DEPLOYMENT — STARTUP OF THE LINUX APPIMAGE ON ANY DESKTOP

![Status](https://img.shields.io/badge/Status-Approved-brightgreen) ![Implementation](https://img.shields.io/badge/Implementation-planned-yellow) ![K2](https://img.shields.io/badge/K2-2.13-blue)

## Context

The published K2 Qt6 Linux AppImage
(`Katalog-linux_qt6_v2.13.beta1-x86_64.AppImage`) **crashes at startup on a
desktop that is not a KDE session**. The maintainer reproduced it on
Linux Mint / Cinnamon in a VirtualBox virtual machine:

- A plain run prints only the two
  `kf.i18n: KLocalizedString: Domain is not set...` lines and dies. It never
  reaches the application's own first output
  (`WARNING: Unknown SearchIn value, using default: ""` and the three lines
  that follow it).
- The same AppImage run as
  `QT_QPA_PLATFORMTHEME=generic ./Katalog-...AppImage` starts fully, prints
  those four application lines, and displays the K2 user interface.
- A `QT_DEBUG_PLUGINS=1` run dies at the same point, after Qt has scanned the
  `styles` and `accessible` plugin directories and loaded nothing from
  `styles`.

So the failure is in platform-theme and style initialisation, **before** the
application's own settings are loaded — no K2 code has run yet — and naming a
platform theme that is present in the bundle avoids it.

A mitigation for this exact failure was already attempted. Both K2 Qt6 AppImage
workflows carry a byte-identical AppRun that only *unsets* the variable on a
non-KDE session
(`.github/workflows/Katalog_2_Build_linux_qt6_buildKF6_appimage.yml:404-425`
and `.github/workflows/Katalog_2_Build_linux_qt6_kdeneon_appimage.yml:296-315`),
under a comment claiming it fixes "SIGSEGV on non-KDE desktops (Cinnamon,
GNOME, etc.)". Unsetting the variable does not stop Qt from auto-selecting a
platform theme from the session: on a GTK desktop such as Cinnamon or XFCE Qt
selects `gtk3` on its own and loads the bundled plugin against the host's GTK
stack. Neither workflow removes that plugin from the bundle. The mitigation is
therefore **incomplete, not absent** — which is why the defect reads as new
while the code looks as though it were already handled.

Two things this page does *not* claim, because they are not established:

- **Which bundled plugin fails.** The evidence above locates the failure in
  theme and style initialisation and proves that forcing `generic` avoids it. It
  does not name the plugin. The authorised fix does not depend on that answer
  (see `K2D-C5`).
- **Which builds are affected.** The defect is confirmed on the 2.13.beta1
  AppImage. Whether the **released 2.12** AppImage fails the same way on the
  same desktops has not been tested. If it does, this reaches users of a
  released version and not only beta testers, so nothing here is written as a
  beta-only concern.

This page covers **whether the packaged K2 build starts**. It does not cover
what K2 does once it is running, and it authorises no change to the K2 user
interface: K2 is in maintenance mode, and the fix is in the launcher only.

> **Reading the requirement IDs.** Each requirement has a permanent ID.
> IDs are never renumbered or reused; a retired requirement is marked
> `[Removed]`, not deleted. Status is one of
> `[Implemented] / [Planned] / [Backlog] / [Removed]`.

---

## Scope at a glance

**In scope:** the platform theme the K2 Qt6 Linux AppImage selects at launch;
that it selects one which is present in the bundle; that a KDE session is left
as it is today; and the rule that keeps the two K2 Qt6 workflows' launchers
identical.

**Out of scope (non-goals):** the appearance of the packaged K2 build beyond
starting — the Controls style, the icon theme, and host-native file dialogs
through the XDG desktop portal, which K3 adopted (`DPL-F3` in
[SpecK3Deployment](SpecK3Deployment.md)) and K2 deliberately does not in this
round (`K2D-C4`). Removing any plugin from the bundle (`K2D-C5`). The Windows
and macOS portable bundles, and any downstream manifest. K3 packaging, which is
[SpecK3Deployment](SpecK3Deployment.md). No change to any file under
`qt_widgets/` or `core/` is authorised by this page (`K2D-C1`).

---

## Operational requirements — *why / for whom*

Goals in real use, independent of how they are built.

| ID | Requirement | Status |
|----|-------------|--------|
| K2D-O1 | A user can start the published K2 Linux AppImage and reach the main window on any Linux desktop, not only on a KDE/Plasma session, without setting any environment variable first. | [Planned] |

## Functional requirements — *what the system does*

Observable behaviour that can be triggered and watched.

| ID | Requirement | Status |
|----|-------------|--------|
| K2D-F1 | On a session that is **not** a KDE session, the K2 Qt6 AppImage launcher sets a platform theme that is present in the bundle (`generic`), rather than leaving Qt to auto-select one from the session. The mitigation is measured: on Linux Mint / Cinnamon the published 2.13.beta1 AppImage dies before its own startup output, and the same AppImage run with `QT_QPA_PLATFORMTHEME=generic` reaches that output and shows the user interface. That evidence covers the *mitigation*, not the shipped launcher, which is why this row is `[Planned]`. | [Planned] |
| K2D-F2 | On a KDE session the launcher leaves the host's platform theme in effect, so K2 keeps the Plasma appearance it has today. `K2D-F1` MUST NOT be applied unconditionally. | [Planned] |
| K2D-F3 | A published AppImage that dies before the application's own startup output on any tested desktop is a **release blocker**, not a per-desktop caveat noted in the release text. It means the application cannot be started at all by everyone on that desktop. | [Planned] |

## Constructional requirements — *how it is built / limits / MUST-NOTs*

Boundaries and implementation constraints, not user-visible behaviour.

| ID | Requirement | Status |
|----|-------------|--------|
| K2D-C1 | This is **packaging only**: the AppRun scripts of the two K2 Qt6 AppImage workflows. It MUST NOT change any file under `qt_widgets/` or `core/`, and MUST NOT add, change or remove any user-visible string. | [Planned] |
| K2D-C2 | The two K2 Qt6 AppImage workflows (`buildKF6` and `kdeneon`) carry **byte-identical** AppRun scripts. Any change to one MUST be applied to the other in the same change. They are identical today; fixing only one would leave one published AppImage still crashing. | [Planned] |
| K2D-C3 | `unset QT_QPA_PLATFORMTHEME` MUST NOT be relied on as the mitigation. Qt still auto-selects a theme plugin from the session, so on a GTK desktop it loads a bundled plugin resolved against the host's GTK stack. The workflow comments that assert the unset works, and give a wrong reason for it (`Katalog_2_Build_linux_qt6_buildKF6_appimage.yml:404-411` and `Katalog_2_Build_linux_qt6_kdeneon_appimage.yml:296-303`), MUST be corrected in the same change — otherwise the next reader re-derives the same incomplete mitigation from a comment that claims it is already solved. | [Planned] |
| K2D-C4 | K2 does **not** adopt the K3 portal approach (`DPL-F3`) in this round. K2 is in maintenance mode, and the authorised fix is the launcher variable only. Host-native dialogs, a bundled Controls style and a bundled icon theme for the K2 AppImage are each a **separate decision**, requiring the maintainer's approval and an amendment to this page first. They are not ruled out; they are simply not sanctioned here. | [Planned] |
| K2D-C5 | Removing a platform-theme plugin from the K2 AppDir — for example `AppDir/usr/plugins/platformthemes/libqgtk3.so`, so that no other route can load it — is **not authorised** by this page. It is recorded as a **candidate**, pending evidence that names the plugin which fails: the measurements behind `K2D-F1` locate the failure in theme and style initialisation but do not identify the plugin, and `K2D-F1` does not depend on the answer. Taking it on is a separate decision on its own merits. | [Planned] |

---

## Known limitations of the packaged K2 build

Recorded so a future reader does not mistake them for defects introduced here,
or for work that was agreed.

| Item | Detail |
|------|--------|
| `generic` platform theme, not the desktop's | On a non-KDE session the packaged K2 build uses Qt's `generic` platform theme by `K2D-F1`. It therefore does not pick up that desktop's colours, icon theme or native file dialog. This is the price of starting at all, and it is the state a user already gets today when the launcher merely unsets the variable and Qt finds nothing usable. Improving it is `K2D-C4`. |
| Failing plugin not named | The bundle keeps every platform-theme plugin it ships today (`K2D-C5`). The fix prevents Qt from selecting the failing one on a non-KDE session; it does not make the bundle incapable of loading it. |
| Affected released versions unknown | Confirmed on 2.13.beta1. Whether the released 2.12 AppImage fails identically on the same desktops has not been tested, so the size of the affected user base is not yet known. |

---

## Manual test charter

Each line must be run against the **AppImage**, not against a build from
source — the whole subject of this page is the difference between the two.

- **K2D-O1 / K2D-F1** — On Linux Mint / Cinnamon, run the AppImage with no
  environment overrides at all. The main window appears, and the application's
  own startup lines (`WARNING: Unknown SearchIn value, using default: ""` and
  the three that follow) are printed. *This is the case that fails today: the
  published 2.13.beta1 AppImage stops after the two
  `kf.i18n: KLocalizedString` lines.*
- **K2D-F1 (second desktop)** — Repeat on one more non-KDE, non-GTK-native
  desktop if one is available (XFCE, GNOME). The window appears there too. One
  passing desktop shows the mitigation works; it does not show the condition in
  the launcher is right for every session.
- **K2D-F2** — On Plasma, run the same AppImage. It starts, and it looks as it
  does today: same colours, same icons, same dialogs. Compare against the
  previous build on the same machine — a regression here is as serious as the
  crash being fixed.
- **K2D-F3** — Capture the full stderr of both runs above. Neither stops at the
  `kf.i18n: KLocalizedString` lines. A run that does is a release blocker.
- **K2D-C1** — Review the diff: no file under `qt_widgets/` or `core/` is
  touched, and no `tr(` is added, changed or removed.
- **K2D-C2** — Diff the AppRun block of
  `Katalog_2_Build_linux_qt6_buildKF6_appimage.yml` against the one in
  `Katalog_2_Build_linux_qt6_kdeneon_appimage.yml`: byte-identical. Then confirm
  the change touched both files.
- **K2D-C3** — Read the comment above each AppRun heredoc. It no longer claims
  that unsetting the variable is sufficient, and no longer states that
  auto-detection makes the unset safe on a non-KDE desktop.
- **K2D-C5** — List `AppDir/usr/plugins/platformthemes/` in the built AppDir:
  unchanged from before this round, with no plugin removed.
- **Released version exposure** — Run the released **2.12** AppImage on the same
  Linux Mint / Cinnamon virtual machine. Record whether it fails the same way.
  This does not gate the fix; it establishes whether the defect reaches users of
  a released version.
