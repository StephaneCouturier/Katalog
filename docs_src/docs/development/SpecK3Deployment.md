---
id: SpecK3Deployment
title: K3 Deployment — QML Modules and Platform Integration in Packaged Builds
description: Requirements for what a packaged Katalog 3 Linux build must contain so that every feature that works in a development build also works for a tester
---

# K3 DEPLOYMENT — QML MODULES AND PLATFORM INTEGRATION IN PACKAGED BUILDS

![Status](https://img.shields.io/badge/Status-Approved-brightgreen) ![Implementation](https://img.shields.io/badge/Implementation-partial-yellow) ![K3](https://img.shields.io/badge/K3-3.0-blue)

## Context

K3 3.0 beta1 is distributed as a portable Linux AppImage. Five testers
(openSUSE Leap/Plasma, Manjaro/XFCE, Linux Mint/Cinnamon, CachyOS/Plasma,
openSUSE MicroOS/Kalpa) and one Tumbleweed box all reported the same defect:
**no file or folder dialog appeared** when opening or creating a File-mode
(SQLite) collection. The same build from source on the development machine
worked. Nothing was wrong with the application code.

The AppImage bundles a **hand-written list** of QML modules. It ships no native
file-dialog helper, so `QtQuick.Dialogs` falls back to its non-native QML
implementation — and that implementation imports `Qt.labs.folderlistmodel`,
which the list did not include. The dialog therefore never became visible and
the only trace was a log line:

```
QML FileDialog: Failed to load non-native FileDialog implementation:
module "Qt.labs.folderlistmodel" is not installed
```

A silent feature loss, visible only to whoever reads the packaged build's
stderr. That is the failure mode this page exists to prevent, not just the one
missing module.

Adding the module was verified on the openSUSE Leap Plasma virtual machine: the
dialog now appears (`DPL-F1`). With it appearing, a second, smaller report
followed — the dialog that appears is Qt's own Fusion-styled Qt Quick dialog
with a foreign icon set, not the Plasma dialog the locally built K3 shows. The
cause is the same gap: the bundle carries no KDE desktop integration. The
approved response is to use the host's **own** dialog through the XDG desktop
portal (`DPL-F3`), not to bundle a KDE style and icon theme — see *Known
limitations*.

This page covers what a packaged K3 build must **contain**. It does not cover
what any dialog or page does once it works: the collection dialogs themselves
are `SpecCollectionOpen.md`.

> **Reading the requirement IDs.** Each requirement has a permanent ID.
> IDs are never renumbered or reused; a retired requirement is marked
> `[Removed]`, not deleted. Status is one of
> `[Implemented] / [Planned] / [Backlog] / [Removed]`.

---

## Scope at a glance

**In scope:** the QML modules a packaged K3 Linux build must contain, including
the modules that bundled modules load at runtime; the platform theme that
decides whether file and folder selection uses the host's dialog; the fallback
when the host offers none; and the rule that keeps the hand-written module list
in step with the sources.

**Out of scope (non-goals):** the Controls **style** and the **icon theme** of
packaged builds — recorded below as a known limitation, and deliberately *not*
planned work (`DPL-C4`). Bundling Breeze icon files, building
`qqc2-desktop-style` and building `frameworkintegration` were each considered
and **not** authorised. The Windows and macOS portable bundles, Flatpak and any
other downstream manifest. K2 packaging. The behaviour of any dialog or page
once it loads. No `qt_quick` source change is authorised by this page.

---

## Operational requirements — *why / for whom*

Goals in real use, independent of how they are built.

| ID | Requirement | Status |
|----|-------------|--------|
| DPL-O1 | A tester running a released package gets the same **working** features as someone running a build from source: every dialog and page that works in a development build also works in the package. A QML module absent from the package MUST NOT silently disable a feature. | [Planned] |
| DPL-O2 | A user selecting a file or folder in a packaged build gets the file dialog of the desktop they are actually running, and can still select a file or folder on a desktop that provides none. | [Planned] |

`DPL-O1` is marked `[Planned]`, not `[Implemented]`: its first instance — the
file and folder dialogs — is closed and verified by `DPL-F1`, but the goal is
only as strong as its verification, and no other page has been walked through
in a packaged build yet.

## Functional requirements — *what the system does*

Observable behaviour that can be triggered and watched.

| ID | Requirement | Status |
|----|-------------|--------|
| DPL-F1 | The Linux AppImage contains every QML module the `qt_quick` sources import **and** every module those modules load at runtime. This explicitly includes `Qt.labs.folderlistmodel`, which no Katalog source imports: the non-native `QtQuick.Dialogs` implementation requires it, and that implementation is what runs whenever the host offers no native dialog helper. Verified on openSUSE Leap / Plasma: the File-mode dialog appears. | [Implemented] |
| DPL-F2 | A packaged build that logs `module "X" is not installed`, or `Failed to load non-native ... implementation`, is a **release blocker**. Such a line means a feature is silently unavailable to every user of that package. | [Planned] |
| DPL-F3 | In a packaged Linux build, file and folder selection uses the **host desktop's own** dialog — Plasma's dialog on Plasma, the host's dialog elsewhere — obtained through Qt's `xdgdesktopportal` platform theme. | [Planned] |
| DPL-F4 | When the host provides no portal backend, file and folder selection degrades to Qt's non-native `QtQuick.Dialogs` implementation and still works. Degradation is a supported path, not an error state. | [Planned] |

## Constructional requirements — *how it is built / limits / MUST-NOTs*

Boundaries and implementation constraints, not user-visible behaviour.

| ID | Requirement | Status |
|----|-------------|--------|
| DPL-C1 | The AppImage QML module list is a **hand-maintained allow-list**, not a scan of the sources. Adding, changing or removing a QML `import` in any `qt_quick` source therefore MUST update that list in the **same change**. A source import that the list does not cover is the defect recorded in *Context*, and it fails silently. | [Implemented] |
| DPL-C2 | The `xdgdesktopportal` platform-theme plugin MUST be present in the bundle, otherwise `DPL-F3` cannot take effect however the theme is selected. | [Planned] |
| DPL-C3 | `DPL-F1` remains mandatory **after** `DPL-F3` is in place: it is a hard dependency of `DPL-F3`, not an alternative to it, because `DPL-F4` is the path taken on every host without a portal backend. The bundled `Qt.labs.folderlistmodel` MUST NOT be dropped as "no longer needed now that dialogs are native". | [Planned] |
| DPL-C4 | The Controls style and icon theme of packaged builds are **out of scope and not planned work**. `DPL-F3` addresses the reported appearance by using the host's own dialog. Bundling Breeze icon files, building `qqc2-desktop-style` and building `frameworkintegration` were each put to the maintainer and **not** authorised; none of them may be introduced on the strength of this page. See *Known limitations*. | [Planned] |
| DPL-C5 | This work adds, changes and removes **no** user-visible string, and changes **no** file under `core/`. It is deployment configuration only. | [Implemented] |

---

## Known limitations of packaged Linux builds

Recorded so that a future reader does not mistake them for defects introduced
here, or for work that was agreed.

| Item | Detail |
|------|--------|
| Fusion style, not the desktop style | The bundle contains no `qqc2-desktop-style`, so Qt Quick Controls resolve to Fusion. This is visible in a tester's log as `QtQuick/Controls/Fusion/Dialog.qml`. Building that framework into the bundle was **not** authorised. |
| Generic icon theme | `breeze-icons` is built when the bundle's KF6 stack is built, but only its shared libraries are copied — the icon **files** under `share/icons/breeze` never reach the AppDir, so icons fall back to whatever the host offers. Copying them was **not** authorised. |
| No KDE platform theme | `frameworkintegration`, which provides the `kde` platform theme plugin, is not built for the bundle. Building it was **not** authorised. `DPL-F3` reaches the host's dialog through the portal instead. |

These three are the reason a packaged build looks unlike a local build. They are
listed as **limitations**, not as requirements and not as a backlog: nothing on
this list is authorised work.

---

## Manual test charter

Each line is a case that must hold for a packaged build. Run them against the
**AppImage**, not against a build from source — the whole subject of this page
is the difference between the two.

- **DPL-F1 / DPL-O1** — Launch the AppImage with `QT_FORCE_STDERR_LOGGING=1`,
  with no settings file present. At first run the File-mode file dialog appears
  and a database can be created; the Memory-mode folder dialog appears too.
  *Verified on openSUSE Leap / Plasma after the module was added.*
- **DPL-F2** — Capture the full stderr of that same run, and of a session that
  visits every page. No `module "..." is not installed` line and no
  `Failed to load non-native ... implementation` line appears. Either one is a
  release blocker, not a cosmetic warning.
- **DPL-F3** — On a Plasma host with a portal backend running, open the
  collection file dialog from the AppImage. The dialog is the host's own file
  dialog, not Qt's Fusion-styled Qt Quick dialog. Repeat on a non-Plasma host
  (XFCE or Cinnamon): the dialog is that desktop's own.
- **DPL-F4** — On a host with no portal backend available, open the same dialog.
  It still appears, as Qt's non-native implementation, and a file can be
  selected. This is the case that `DPL-F1` keeps alive.
- **DPL-C1** — Diff the QML `import` lines of every `qt_quick` source against
  the module list in the AppImage workflow. Every imported module is covered.
  Then confirm the last change that touched a QML import also touched the list.
- **DPL-C2** — List the platform-theme plugins inside the built AppDir: the
  `xdgdesktopportal` plugin is present.
- **DPL-C3** — Confirm `Qt.labs.folderlistmodel` is still in the bundled module
  list after the portal work. Its removal is a failure even if every test host
  happens to have a portal backend.
- **DPL-C4** — Review the diff: no Breeze icon files are copied into the AppDir,
  and neither `qqc2-desktop-style` nor `frameworkintegration` has been added to
  the bundle's framework build list.
- **DPL-C5** — Review the diff for `tr(` and `qsTr(`: no addition, no change, no
  removal. Confirm no file under `core/` is modified.
