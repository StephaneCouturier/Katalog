# BACKUP

![Status](https://img.shields.io/badge/Status-Draft-orange) ![Implementation](https://img.shields.io/badge/Implementation-complete-brightgreen)

## Context

Backup copies the files of a **source** Catalog device onto a **target** Catalog
device, so the files survive loss of the source. It is driven by a reusable
**mapping** (source → target) that the user runs repeatedly. Each run is
**incremental**: only what is new or changed since the previous run is
transferred. An **Archive** variant *moves* files (copy, then remove from the
source) to free space on the source.

This spec is the single source of truth for what Backup does and — as important —
what it must **not** do. It applies to both K2 (`qt_widgets`) and K3
(`qt_quick`), which share the same `core/` logic.

> **Reading the requirement IDs.** Each requirement has a permanent ID.
> IDs are never renumbered or reused; a retired requirement is marked
> `[Removed]`, not deleted. Status is one of
> `[Implemented] / [Planned] / [Backlog] / [Removed]`.

---

## Scope at a glance

**In scope:** defining a mapping, previewing a run, replicating the source
directory tree (empty directories included or excluded per mapping option),
incrementally copying new/changed files, handling name-and-size duplicates,
handling conflicts, archiving (move), reporting results, recording last-run
metadata.

**Out of scope (non-goals):** anything that removes or alters existing target
content. Backup only ever *adds* to the target. See `BKP-C1` / `BKP-C2` —
these are the boundaries, and any change that touches them requires explicit
approval and a spec update first.

---

## Operational requirements — *why / for whom*

Goals in real use, independent of how they are built.

| ID | Requirement | Status |
|----|-------------|--------|
| BKP-O1 | A user replicates a catalog's files onto another device so the files survive loss of the source. | [Implemented] |
| BKP-O2 | A user defines a reusable source→target mapping and runs it repeatedly; each run transfers only what is new or changed since the last run. | [Implemented] |
| BKP-O3 | A user archives a catalog — moving files off the source onto the target to free space on the source. | [Implemented] |
| BKP-O4 | A user previews exactly what a run will transfer before committing to it. | [Implemented] |

## Functional requirements — *what the system does*

Observable behaviour that can be triggered and watched.

| ID | Requirement | Status |
|----|-------------|--------|
| BKP-F1 | Source and target of a mapping are both Catalog-type devices; a run refuses to start otherwise. | [Implemented] |
| BKP-F2 | Before any file is copied, the source directory tree is replicated on the target. Catalog mode reads folders from the catalog index; Drive mode walks the connected source filesystem. Whether directories that are empty are replicated is governed by `BKP-F17`. | [Implemented] |
| BKP-F3 | Files present in the source but absent from the target are copied to the target, preserving their relative folder structure. | [Implemented] |
| BKP-F4 | With `strictCopy` on, the target folder structure mirrors the source exactly. With `strictCopy` off, a source file is skipped when a file of the same name **and** size already exists anywhere in the target (deduplication). | [Implemented] |
| BKP-F5 | For a file that exists in the target but differs: **Skip** leaves the target untouched; **RenameOldest** renames the older target file with a datetime stamp (`stem_YYYYMMDD-HHmmss.ext`) and copies the source **only** when the source is newer, otherwise skips to protect the newer target. | [Implemented] |
| BKP-F6 | An **Archive** mapping moves files (copy to target, then remove from the **source**) instead of copying. | [Implemented] |
| BKP-F7 | The run can optionally update both source and target catalogs before comparing. | [Implemented] |
| BKP-F8 | Before transfer, target free space is checked: the run is blocked when space is insufficient and warns when space is low. | [Implemented] |
| BKP-F9 | Progress and outcome (created / copied / skipped / errors) are reported via the standard status-bar message builder. | [Implemented] |
| BKP-F10 | On completion the mapping records the last-run date and the number of bytes transferred. | [Implemented] |
| BKP-F11 | A standalone "Replicate directories" action replicates the source directory tree onto the target without copying any file. | [Implemented] |
| BKP-F12 | A running backup can be paused and stopped. | [Implemented] |
| BKP-F13 | Additional conflict handling where the source always wins (overwrite) or the target is always renamed. | [Backlog] |
| BKP-F14 | When the run updates catalogs before comparing (BKP-F7), that catalog update reports progress through the standard status-bar message builder (UPDATE format) shown on the Backup page's status area. The Backup Preview page opens only once the preview report is ready; it is never shown while the update or preview is still computing. | [Implemented] |
| BKP-F15 | In the backup preview, the file-list table (Status, File Name, Path, Size) presents a resizable header: the user can adjust each column's width by dragging its header divider, consistent with the K3 search-results and explore tables. | [Implemented] |
| BKP-F16 | The backup preview computation (loading catalog file lists and comparing source vs target) runs in the background so the UI stays responsive, and reports progress through the standard status-bar message builder, consistent with catalog updates. The user can cancel a running preview. | [Implemented] |
| BKP-F17 | A mapping carries an **Include empty** option under a **Directories** field. A directory is **empty** when it contains no entry at all — no file **and** no subdirectory — hidden entries counted as entries. With the option **on**, empty directories are created on the target; with it **off**, they are not, while every non-empty directory still is (a directory whose only content is a subdirectory, or only a hidden file, is not empty and is created). The option governs both the pre-copy replication (`BKP-F2`) and the standalone Replicate directories action (`BKP-F11`). Default is **on**, and every existing mapping keeps **on**. Turning it off never removes an empty directory already present on the target (`BKP-C1`). | [Implemented] |

> **Note on emptiness in Catalog mode (`BKP-F17`).** Catalog mode evaluates
> emptiness against the **catalog index**, which is authoritative for what the
> catalog knows. A folder whose only content is a hidden file therefore counts as
> empty when the source catalog was scanned with hidden entries excluded, whereas
> Drive mode walks the real filesystem and sees it as non-empty. This divergence
> is accepted and documented; it is not to be worked around in the replicator.

## Constructional requirements — *how it is built / limits / MUST-NOTs*

Boundaries and implementation constraints, not user-visible behaviour.

| ID | Requirement | Status |
|----|-------------|--------|
| BKP-C1 | Backup is **add-only / incremental**. It MUST NOT delete or modify any existing file or folder on the **target**. | [Implemented] |
| BKP-C2 | It MUST NOT mirror source deletions onto the target — a folder or file removed from the source is left in place on the target. | [Implemented] |
| BKP-C3 | Archive removes files from the **source** only (its purpose); it MUST NOT remove anything from the target. `BKP-C1` still holds for the target. | [Implemented] |
| BKP-C4 | Filesystem and database logic (directory replication, file copy/move, comparison) live in `core/` (`DirectoryReplicator`, `BackupJobStoppable`). The UI layer only orchestrates and reports. | [Implemented] |
| BKP-C5 | Works in Memory and File/Hosted database modes. In Memory mode, file lists and folder lists are loaded into their tables before comparison and replication. | [Implemented] |
| BKP-C6 | K2 and K3 have identical backup behaviour; both call the same `core/` code. User-visible strings are byte-identical in both: the `BKP-F17` control uses the field label `Directories` (an existing K2 string, reused — no new translation slot) and the checkbox text `Include empty` (one new string). | [Implemented] |
| BKP-C7 | The UI layer contains no raw SQL for backup. | [Implemented] |
| BKP-C8 | The preview/compare computation MUST keep the UI responsive and cancellable while it runs — yielding cooperatively on the main thread (periodic processEvents + a stop flag), consistent with how catalog update and search run — and MUST NOT block the UI thread with a single synchronous call. The compare/orchestration logic lives in `core/` (per BKP-C4); the UI layer only triggers it and displays progress and result. | [Implemented] |
| BKP-C9 | The `BKP-F17` flag is persisted per mapping in `device_mapping` and passed from `BackupMappingManager` to `DirectoryReplicator`; the UI only sets and reads it, and the emptiness test lives in `core/` (per `BKP-C4`). The column default is "on", so existing rows keep the previous behaviour without data migration. Being a 2.13-cycle column that has never shipped, it is added through the existing unconditional column guard rather than a new migration step. | [Implemented] |

---

## Manual test charter

For each row: set up the stated condition, run the operation, confirm the result.

- **BKP-F2 / BKP-F17 (on)** — Give the source an empty folder, with **Include empty** on. Run the backup. The empty folder exists on the target.
- **BKP-F17 (off)** — Same source, **Include empty** off. Run. The empty folder is not created; folders holding copied files still are. A folder whose only content is a subdirectory, and a folder whose only content is a hidden file, are both created.
- **BKP-F17 / BKP-C1** — Run once with **Include empty** on, then re-run with it off. The empty folder created by the first run is still present on the target.
- **BKP-F11 / BKP-F17** — Run the standalone Replicate directories action with the option off, then on. It honours the mapping's setting in the same way as a full run.
- **BKP-C1 / BKP-C2** — Put a file and a folder on the target that are **not** in the source. Run the backup. Both are still present on the target afterwards, unchanged.
- **BKP-F3** — Add a new file to the source. Run. Only that file is copied; unchanged files are not re-copied.
- **BKP-F4 (dedup)** — With `strictCopy` off, place a same-name-and-size copy of a source file in a different target folder. Run. The source file is reported skipped.
- **BKP-F5** — Modify a target file so it differs, with the source newer. In RenameOldest, the old target file is renamed with a datetime stamp and the source is copied; in Skip, the target is untouched.
- **BKP-F6** — Run an Archive mapping. Files appear on the target and are removed from the source; the target keeps everything it already had.
- **BKP-F8** — Point at a target with insufficient free space. The run is blocked with a message.
- **BKP-F14** — Enable "Update catalogs" and request a Preview. The pre-preview catalog update shows a standard `UPDATE | In Progress | ...` builder message on the Backup page; the Backup Preview page appears only when the report is ready, never as an empty page with a progress indicator.
- **BKP-F15** — Open a backup preview with files listed. Drag a column-header divider; that column resizes to the dragged width.
- **BKP-F16** — With a large source and target catalog, trigger a Preview. The UI stays responsive while the compare runs and a `... | In Progress | ...` builder message is shown; the preview can be cancelled; the Preview page opens when the compare completes.
- **BKP-C8** — Trigger a preview on a large catalog; the UI stays responsive (repaints, Stop works) during the compare and does not freeze.
