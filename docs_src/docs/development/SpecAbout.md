---
id: SpecAbout
title: About — Version and System Information
description: Requirements for the About page action that copies version and environment information to the clipboard
---

# ABOUT — VERSION AND SYSTEM INFORMATION

![Status](https://img.shields.io/badge/Status-Approved-brightgreen) ![Implementation](https://img.shields.io/badge/K3-planned-blue) ![K2](https://img.shields.io/badge/K2-no%20change-lightgrey)

## Context

A user filing a bug report needs the version and environment facts a maintainer
requires — Katalog version, Qt and KDE Frameworks versions, operating system,
language, database mode. Before this feature those facts were scattered across
the UI or not shown at all, so reports arrived incomplete and the maintainer had
to ask for them.

This spec covers a single action on the K3 About page that assembles those facts
into a plain-text block and copies it to the clipboard, ready to paste into a bug
report.

The block is **designed to be pasted into a public bug tracker**. That is what
makes its content a boundary and not a detail: the constructional requirements
below define exactly what may leave the application, and nothing else may be
added to the block without changing this spec first.

> **Reading the requirement IDs.** Each requirement has a permanent ID.
> IDs are never renumbered or reused; a retired requirement is marked
> `[Removed]`, not deleted. Status is one of
> `[Implemented] / [Planned] / [Backlog] / [Removed]`.

---

## Scope at a glance

**In scope:** one action on the K3 About page; the fixed field list of the
copied block; the confirmation notification; the privacy boundary on what the
block may contain.

**Out of scope (non-goals):** K2 — the K2 About section is unchanged. Any
identifying or credential data in the block (see `ABT-C1`). Sending the block
anywhere automatically: the user pastes it, the application never transmits it.
Log collection, crash dumps, or any diagnostics beyond the field list in
`ABT-F2`.

---

## Operational requirements — *why / for whom*

Goals in real use, independent of how they are built.

| ID | Requirement | Status |
|----|-------------|--------|
| ABT-O1 | A user reporting a bug obtains, in one action, the version and environment facts a maintainer needs, without hunting through the UI for them. | [Planned] |
| ABT-O2 | A user can paste that information into a public bug tracker without disclosing who they are, what their machine is called, where their data lives, or any database credential. | [Planned] |

## Functional requirements — *what the system does*

Observable behaviour that can be triggered and watched.

| ID | Requirement | Status |
|----|-------------|--------|
| ABT-F1 | The About page offers an action, alongside the existing Close action, that copies a plain-text system-information block to the clipboard and confirms with a transient notification. | [Planned] |
| ABT-F2 | The copied block contains exactly the fields listed in *Block content* below — no more, no fewer. | [Planned] |
| ABT-F3 | The block is plain text, readable as-is when pasted into a bug tracker without further formatting. | [Planned] |

## Constructional requirements — *how it is built / limits / MUST-NOTs*

Boundaries and implementation constraints, not user-visible behaviour.

| ID | Requirement | Status |
|----|-------------|--------|
| ABT-C1 | The block MUST NOT contain: machine hostname, user name, any filesystem path (collection, catalog, database file, image folder), any hosted-database host, port, database name, user or password, or the collection name. | [Planned] |
| ABT-C2 | The block is assembled from a fixed allowlist of named fields. It MUST NOT be produced by iterating settings or `.ini` keys, nor by dumping a configuration object, a settings map, or any environment survey. | [Planned] |
| ABT-C3 | K3 only. No `core/` change: `AppManager` assembles the block and exposes it to QML. The K2 UI is not modified. | [Planned] |
| ABT-C4 | Adding a field to the block requires updating *Block content* in this spec first; the field list in the code and the field list here must match. | [Planned] |

---

## Block content

The complete and exclusive field list authorised by `ABT-F2`.

| # | Field | Source |
|---|-------|--------|
| 1 | Katalog version | `KATALOG_VERSION_STRING` |
| 2 | Katalog release date | `KATALOG_RELEASE_DATE` |
| 3 | Qt version — runtime | Qt runtime version |
| 4 | Qt version — compile-time | Qt compile-time version |
| 5 | KDE Frameworks version | KF6 version |
| 6 | Operating system product name | `QSysInfo::prettyProductName()` |
| 7 | Kernel type | `QSysInfo::kernelType()` |
| 8 | Kernel version | `QSysInfo::kernelVersion()` |
| 9 | CPU architecture | `QSysInfo::currentCpuArchitecture()` |
| 10 | UI language / locale identifier | locale identifier only |
| 11 | Database mode | `Memory` / `File` / `Hosted` |
| 12 | Database schema version | `collection->dbSchemaVersion` |

### Why the database mode and schema version are included

Both were reviewed against `ABT-C1` and accepted. Neither adds any disclosure
beyond what the user already sees on screen: both are displayed on the Settings
page (`qt_quick/PageSettings.qml:115,142-154,161`). The mode is a three-value
enum carrying no identifying content, and Memory-mode defects are otherwise
unreportable — a whole class of bugs behaves differently in Memory mode, so a
report that omits the mode is usually unusable. Only the mode and the schema
version are included; the database **path**, **host**, **port**, **name**,
**user** and **password** are excluded by `ABT-C1`.

`ABT-C1` blocks the leaks known today. `ABT-C2` blocks the ones that would
otherwise be introduced later by a convenience change, such as serialising the
settings object wholesale.

---

## User-visible text

Approved byte-for-byte. Both strings are new: no K2 equivalent exists (K2 has no
such action) and no existing K3 string fits. The explicit label was chosen over a
shorter "Copy to clipboard", and the matching longer notification over the
shorter `"<X> copied to clipboard"` house form — both are deliberate.

| Element | String |
|---------|--------|
| Action label | `Copy version and system information` |
| Confirmation notification | `Version and system information copied to clipboard` |

The notification is a transient success message and uses
`showPassiveNotification(...)`, per `SpecValidationRules.md` and the Kirigami
HIG.

---

## Manual test charter

For each row: set up the stated condition, run the operation, confirm the result.

- **ABT-F1** — Open the About page. The action is present next to Close. Trigger it: the notification `Version and system information copied to clipboard` appears and the clipboard is not empty.
- **ABT-F2** — Paste the block into a text editor. Exactly the twelve fields of *Block content* are present; count them. No thirteenth field.
- **ABT-F3** — Paste the block into a plain-text field. It is readable as-is, with no markup artefacts and no truncated lines.
- **ABT-C1** — Search the pasted block for the machine hostname, the user name, and any `/` or `\` path fragment: none is present. Then open a **Hosted** collection with host, port, database name, user and password filled in, copy again, and search the block for each of those five values and for the collection name: none is present.
- **ABT-C1 / ABT-F2 (mode and schema)** — Copy the block in Memory mode, then in File mode, then in Hosted mode. Each block states the correct mode and the schema version, and nothing more about the database.
- **ABT-C2** — Add an unrelated key to the collection `.ini` by hand, then copy the block. The new key does not appear, confirming the block is an allowlist and not a settings dump.
- **ABT-C3** — Confirm the K2 About section is unchanged and that the feature required no `core/` change.
