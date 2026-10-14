---
id: SpecVersionCheck
title: "Version Check — New Version Available"
description: "Requirements for telling the user at startup that a newer Katalog release exists"
---

# VERSION CHECK — NEW VERSION AVAILABLE

![Status](https://img.shields.io/badge/Status-Approved-brightgreen) ![Implementation](https://img.shields.io/badge/Implementation-complete-brightgreen) ![K3](https://img.shields.io/badge/K3-3.0-blue) ![K2](https://img.shields.io/badge/K2-existing-lightgrey)

## Context

K2 checks at startup whether a newer release is published on GitHub and, if so,
offers to download it. K3 carried the *Check for a new version on startup*
setting (`SpecSettingsPage.md` `SET-F2`) but no check behind it, so a K3 user
would never learn that a newer version exists. This spec ports the K2 check to
K3, with the differences K3's distribution channels and pre-releases require.

Decided by the user on 2026-10-15.

> **Reading the requirement IDs.** Each requirement has a permanent ID.
> IDs are never renumbered or reused; a retired requirement is marked
> `[Removed]`, not deleted. Status is one of
> `[Implemented] / [Planned] / [Backlog] / [Removed]`.

---

## Scope at a glance

| | |
|---|---|
| **In scope** | The K3 startup check, its dialog, when it is skipped |
| **Out of scope** | Any change to K2's check; automatic download or installation; update checks for packages managed by a store |
| **Applies to** | K3 3.0 |

---

## Operational requirements — *why / for whom*

| ID | Requirement | Status |
|----|-------------|--------|
| VCK-O1 | A K3 user learns at startup that a newer release exists, unless they turned the check off. | [Implemented] |

## Functional requirements — *what the system does*

| ID | Requirement | Status |
|----|-------------|--------|
| VCK-F1 | When `Settings/CheckVersion` is true (default **true**, as in K2), K3 asks GitHub for the latest release (`https://github.com/StephaneCouturier/Katalog/releases/latest`, which redirects to `…/releases/tag/v<version>`), reads the version from it, and compares it with the running version. | [Implemented] |
| VCK-F2 | Only when the published version is newer, K3 shows a modal dialog with K2's message, verbatim: *This is version: v%1 … A new version is available: v%2 … Release Notes … Do you want to download it?*, with **Yes** and **Cancel**. The Release Notes link opens the release page. **Yes** opens `https://sourceforge.net/projects/katalogg/files/latest/download`. | [Implemented] |
| VCK-F3 | A **pre-release** (a version containing `alpha` or `beta`) is older than the final release with the same number: a tester on 3.0.beta4 is told when 3.0 is published. | [Implemented] |
| VCK-F4 | Nothing is shown when the network request fails, the version cannot be read, or the published version is not newer. | [Implemented] |
| VCK-F5 | The check is skipped when K3 runs inside **Flatpak**: Flathub and the software centre announce its updates, and a SourceForge download is the wrong channel for it. | [Implemented] |
| VCK-F6 | The check is skipped on **first run**, so its dialog never appears over the first-run dialog. | [Implemented] |

## Constructional requirements — *how it is built / limits / MUST-NOTs*

| ID | Requirement | Status |
|----|-------------|--------|
| VCK-C1 | The request is asynchronous: it never blocks or delays startup or the first paint (unlike K2, which waits for the reply). | [Implemented] |
| VCK-C2 | No new user-visible string: the message is K2's `tr()` source text, byte for byte. No `core/` change, no schema change, no new setting. | [Implemented] |

---

## Manual test charter

- **VCK-F1 / F2** — Build K3 with a version lower than the latest GitHub release: the dialog appears shortly after startup; the Release Notes link opens the release page; *Yes* opens the SourceForge download.
- **VCK-F3** — Build with `3.0.beta4` while `v3.0` is the latest release: the dialog appears. With `3.0`: no dialog.
- **VCK-F4** — Start offline: no dialog, no message, startup normal.
- **VCK-F5** — Run the Flatpak build: no request is made, no dialog.
- **VCK-F6** — Start with no settings file: only the first-run dialog appears.
- **VCK-C1** — With a slow or unreachable network, the window paints and is usable at once.
- **Setting off** — Untick *Check for a new version on startup*, restart: no request, no dialog.
