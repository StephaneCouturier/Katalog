---
id: SpecMimeVerification
title: MIME Verification — Verify MIME Types on a Catalog
description: Requirements for the K3 standalone "Verify MIME types" action on a catalog, its Memory-mode support, and its reuse by "Verify then Split"
---

# MIME VERIFICATION — VERIFY MIME TYPES ON A CATALOG

![Status](https://img.shields.io/badge/Status-Approved-brightgreen) ![Implementation](https://img.shields.io/badge/Implementation-planned-lightgrey)

## Context

At indexing, a file's type is guessed from its extension. K2 offers *Verify MIME types*
on an active catalog (`qt_widgets/mainwindow_tab_device_ui.cpp:751-822`): it reads each
not-yet-verified file's content with the system MIME database
(`CatalogJobStoppable::VerifyMimeTypes`), records the result, and writes a mismatch report
in the collection folder. K3 had no standalone action (gap listed in
`SpecK3ParityGaps.md`), and its *Verify then Split* (`SpecDevicesSplit.md`, "Pre-split:
Verify MIME Types") ran a normal device update instead of the verification. In K2, the job
neither loads the file list before nor saves the `.idx` after, so it does not work in
Memory mode.

Bootstrapped and approved by the user on 2026-10-10.

> **Reading the requirement IDs.** IDs are permanent, never renumbered or reused; a
> retired requirement is marked `[Removed]`. Status is one of `[Implemented]`,
> `[Planned]`, `[Backlog]`, `[Removed]`.

## Scope at a glance

**In scope:** the K3 "Verify MIME types" entry on a Catalog device; its enabling; running
the verification; its result messages; Memory mode; the verification step of *Verify then
Split*.

**Out of scope:** the split itself (`SpecDevicesSplit.md`); the verification algorithm and
report format in `core/`; queueing (`SpecOperationQueue.md` — this action is refused when
busy, not queued); K2.

**Applies to:** K3 (`qt_quick`) only.

---

## Operational requirements — *why / for whom*

| ID | Requirement | Status |
|----|-------------|--------|
| MIV-O1 | A user can correct the file types that were guessed from extensions, on a catalog that is already indexed, and learn how many files had a type that did not match their content. Approved 2026-10-10. | [Implemented] |

## Functional requirements — *what the system does*

| ID | Requirement | Status |
|----|-------------|--------|
| MIV-F1 | The K3 device card menu shows the entry "Verify MIME types" for a Catalog device, in the Catalog section next to "Verify Checksums". It is not shown for other device types. Approved 2026-10-10. | [Implemented] |
| MIV-F2 | The entry is enabled only when the catalog is active and neither a search nor another device operation is running. While busy it is shown disabled; the request is refused, never queued. Approved 2026-10-10. | [Implemented] |
| MIV-F3 | Choosing the entry runs the MIME verification on that catalog: files not yet verified are checked against their content, marked verified, and mismatches are written to a report in the browse folder (Memory: collection folder; File: the `.db` file's folder; Hosted: the user's home folder). Progress is shown on the existing device status line. Approved 2026-10-10; report location amended 2026-10-10. | [Implemented] |
| MIV-F4 | On completion: with one or more mismatches, a modal dialog titled "MIME Verification Complete" shows "MIME verification completed.<br/>%1 mismatch(es) found between file extensions and actual content.<br/><br/>Report saved to:<br/>%2" with an "Open Report" button, shown only when a report path came back (amended 2026-10-10); with no mismatch, a passive notification shows "MIME verification completed successfully.<br/>No mismatches found between file extensions and actual content." without "Open Report"; on failure, "MIME verification failed:<br/>%1" is shown per the message presentation rules. Approved 2026-10-10. | [Implemented] |
| MIV-F5 | The verification works in Memory mode: the catalog's file list is loaded before the job and the catalog's `.idx` is saved after it, so verified types persist. Approved 2026-10-10. | [Implemented] |
| MIV-F6 | *Verify then Split* (`SpecDevicesSplit.md`) runs this same MIME verification on the catalog, then the split. It no longer runs a device update. Approved 2026-10-10. | [Implemented] |

## Constructional requirements — *how it is built / limits / MUST-NOTs*

| ID | Requirement | Status |
|----|-------------|--------|
| MIV-C1 | No `core/` change: the action uses `CatalogJobStoppable::VerifyMimeTypes`, `Catalog::loadCatalogFileListToTable` and `Catalog::saveCatalogToFile` as they exist. Progress messages use `StatusBarMessageBuilder` (`SpecProgressReport.md`). Approved 2026-10-10. | [Implemented] |
| MIV-C2 | K2 MUST NOT change, including its Memory-mode behaviour. Approved 2026-10-10. | [Implemented] |
| MIV-C3 | User-visible strings are K2's, byte-for-byte, each approved on 2026-10-10: "Verify MIME types", "MIME Verification Complete", "MIME verification completed successfully.<br/>No mismatches found between file extensions and actual content.", "MIME verification completed.<br/>%1 mismatch(es) found between file extensions and actual content.<br/><br/>Report saved to:<br/>%2", "Open Report", "MIME verification failed:<br/>%1", "A device operation is already running.". K2's tooltip is not ported (K3 menu entries have none). No other new string without approval. | [Implemented] |

---

## Manual test charter

- **MIV-F1** — Open the card menu of a Catalog: "Verify MIME types" is next to "Verify Checksums". Open it on a Storage or Virtual device: the entry is absent.
- **MIV-F2** — Unmount the catalog's drive: the entry is disabled. Start a device update, then a search: the entry is disabled during each, and nothing is queued.
- **MIV-F3** — Run it on an active catalog: progress shows on the device status line; a report file appears in the browse folder when mismatches exist (Memory: collection folder; File: `.db` folder; Hosted: home folder) — check all three modes.
- **MIV-F4** — With a renamed file (e.g. a `.txt` that is a PNG): modal dialog with the count, report path and "Open Report", which opens the report. If no report could be saved, the dialog has no "Open Report". On a clean catalog: passive notification only. Make the job fail (e.g. remove read access): the failure message is shown.
- **MIV-F5** — In a Memory-mode collection, run it, close and reopen the collection: the corrected types remain; running it again finds nothing left to verify.
- **MIV-F6** — Split by file type, choose *Verify then Split*: the verification runs (not an update), then the split uses the verified types.
- **MIV-C2** — K2: the Verify MIME types button behaves as before.
- **MIV-C3** — Compare each string with K2's `tr()` source byte-for-byte; no tooltip on the K3 entry.
