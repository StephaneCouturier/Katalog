# BACKUP

![Status](https://img.shields.io/badge/Status-Draft-orange) ![Implementation](https://img.shields.io/badge/Implementation-complete-green)

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
metadata, and (K3 only) running the currently listed links one after another
from a single control.

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
| BKP-O5 | A user who maintains several links runs them in one gesture and walks away: the links they can currently see are carried out one after another, without returning to start each one. | [Implemented] |

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
| BKP-F18 | The K3 Backup page carries a **Run listed links** action that runs several links one after another without the user starting each one. Its scope is exactly what is on screen: the links currently listed under the page's active device filter and Backup/Archive type filter. Filtering **is** the selection mechanism — there is no separate selection UI, no checkboxes, no "select all". Of the listed links, only the **runnable** ones run: runnable means the link's source device and target device are both active, the same condition that enables the per-card Run button. Listed links that are not runnable are skipped, never started, and their number is reported to the user before the run begins (`BKP-F19`). Archive links are **not** excluded — they run alongside Backup links, which is why `BKP-F19` requires a confirmation. | [Implemented] |
| BKP-F19 | **Run listed links** never starts without a confirmation dialog stating what is about to happen. The dialog reports, each line shown only when its count is non-zero: the number of links that will run; the number of **archive** links with the total size that may move out of the source; the number of **backup** links with an approximate size to copy; and the number of listed links that will be skipped because a source or target is unavailable. The two size figures have **different quality and MUST be worded to say so**: the archive figure is the source size, a sound **upper bound** on what moves off the source, and it is the destructive one; the backup figure is source size minus target size, a **weak estimate** that MUST be presented as approximate, because the target only ever grows (`BKP-C1`/`BKP-C2`) and so may hold files the source never had — the difference can therefore be zero or negative while copying is still needed. The dialog MUST NOT present either figure as an exact volume: an exact volume is knowable only by running the per-link compare, which is Preview (`BKP-F16`). Confirming starts the run; cancelling leaves nothing started. | [Implemented] |
| BKP-F20 | Stopping during a **Run listed links** run cancels the **whole** run: the link currently transferring stops, and every link still waiting is abandoned rather than started. This is a deliberate divergence from `OPQ-F7` (which stops one job and keeps its queue): the pending links here are not shown anywhere, so continuing after a Stop would surprise the user. Pause and Resume (`BKP-F12`) act on the link currently running only and do not abandon the rest. | [Implemented] |
| BKP-F21 | Each link in a **Run listed links** run behaves exactly as if the user had pressed that link's own Run: `BKP-F7` (update catalogs before comparing, when the option is on) applies per link, `BKP-F10` records that link's last-run date and bytes, and progress is reported on that link's card as it is today. Running links in sequence introduces no behaviour that a single run does not already have, and changes nothing about how a single run reports. | [Implemented] |
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
| BKP-C6 | K2 and K3 have identical backup behaviour; both call the same `core/` code. User-visible strings are byte-identical in both: the `BKP-F17` control uses the field label `Directories` (an existing K2 string, reused — no new translation slot) and the checkbox text `Include empty` (one new string). **Exception, and only where a requirement names it explicitly:** `BKP-F18`–`BKP-F21` are K3-only and have no K2 counterpart. What a single backup *run* does stays identical between the two — the divergence is confined to K3 offering to start several runs in sequence. No other divergence is authorised by this exception. | [Implemented] |
| BKP-C7 | The UI layer contains no raw SQL for backup. | [Implemented] |
| BKP-C8 | The preview/compare computation MUST keep the UI responsive and cancellable while it runs — yielding cooperatively on the main thread (periodic processEvents + a stop flag), consistent with how catalog update and search run — and MUST NOT block the UI thread with a single synchronous call. The compare/orchestration logic lives in `core/` (per BKP-C4); the UI layer only triggers it and displays progress and result. | [Implemented] |
| BKP-C10 | `BKP-F18`–`BKP-F21` are built as the **minimum** change and add no core code and no new queue infrastructure. `AppManager` holds a pending list of mapping ids (`m_pendingBackupMappings`) filled by a `Q_INVOKABLE` that receives the already-eligible ids from QML; `onBackupFinishedInternal` pops the next id and starts it through `QTimer::singleShot(0, ...)` so the finished thread's `deleteLater` settles first — the same deferred-restart pattern the post-backup catalog re-scan already uses. The next link MUST be started via `runBackup()`, **not** `executeBackupJob()`, so that each link still gets its catalogs updated when `BKP-F7` is on. `m_runningBackupMappingId` is exposed as a notifying `Q_PROPERTY` and the Backup page binds its local running-id to it instead of assigning it in the card's click handler. `stopBackup()` clears the pending list (`BKP-F20`). The card's own progress reporting MUST NOT otherwise change. K2 is not touched (`BKP-C6`). | [Implemented] |
| BKP-C11 | This sequencer is **not** the operation queue of `SpecOperationQueue.md` and MUST NOT be merged into it. `OPQ-C1` forbids a second, parallel mechanism *for the create/update queue*; that prohibition does not reach here, and `OPQ-C3`/`OPQ-C4` (main-thread execution, one operation at a time) do not either, because a backup run executes on its own `QThread` and is not one of the three operations those rules govern. Two boundaries still hold: (a) the per-link catalog update of `BKP-F7` **is** a device update and MUST continue to go through the existing guarded path, so it obeys `OPQ-C4` and never starts a second device operation concurrently; (b) backup progress stays on the Backup page and MUST NOT be moved into the activity panel — `SpecOperationQueue.md`'s note on the reach of `OPQ-C16` already places backup outside that rule. | [Implemented] |
| BKP-C12 | `BKP-F18`/`BKP-F19` spend **six** new translation slots and no more — the six strings listed under *User-visible text* below. Every other word in the feature comes from strings that already exist. Any seventh string requires its own per-string approval before it is written. | [Implemented] |
| BKP-C9 | The `BKP-F17` flag is persisted per mapping in `device_mapping` and passed from `BackupMappingManager` to `DirectoryReplicator`; the UI only sets and reads it, and the emptiness test lives in `core/` (per `BKP-C4`). The column default is "on", so existing rows keep the previous behaviour without data migration. Being a 2.13-cycle column that has never shipped, it is added through the existing unconditional column guard rather than a new migration step. | [Implemented] |

---

## User-visible text

Applies to `BKP-F18` / `BKP-F19` (K3 only), recorded under `BKP-C12`.

Approved new strings — these six and no others:

| String | Use |
|--------|-----|
| `Run listed links` | the page action, and the title of its confirmation dialog |
| `Continue` | the confirming button of the dialog; starts the run |
| `%1 link(s) will run, one after another.` | how many listed links are runnable |
| `%1 archive link(s) - up to %2 moved out of the source.` | archive volume — an upper bound, and the destructive figure |
| `%1 backup link(s) - roughly %2 to copy.` | backup volume — an estimate, worded as approximate per `BKP-F19` |
| `%1 listed link(s) will be skipped: source or target not available.` | listed links that are not runnable |

Reused, already present in the catalogues — no new slot:

| String | Where it already exists |
|--------|-------------------------|
| `link(s)` | the Backup page totals row |
| `source` | the Backup page totals row |
| `files` | the Backup page card, and the Devices page |
| `Cancel` | the running-backup control, and every K3 dialog |

> **Note on the reuse claim.** The four sentence strings above are self-contained;
> they *contain* the words "link(s)" and "source" rather than composing them from
> the existing entries. The reuse is of vocabulary — keeping the wording familiar —
> not of translation slots. The slot count that matters is the six new strings.

> **Note on wording.** No line gives the user an order and none uses "Please",
> per the project's copy rule: each line states what will happen, not what the
> user must do.

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
- **BKP-F18 (scope = what is on screen)** — Set the device filter and the Backup/Archive type filter so that only some links are listed. Press **Run listed links**. Exactly the listed links are considered; a link scrolled out of view but still matching the filters is included, and a link excluded by the filters is not touched. Change the type filter to Backup only and repeat: no archive link runs.
- **BKP-F18 (runnable only)** — Among the listed links, make one link's source or target device inactive. Press **Run listed links**. That link is never started, and the dialog reported it as skipped. Confirm the count matches the number of listed links whose own Run button is disabled for that reason.
- **BKP-F19 (dialog content)** — With a mix of backup links, archive links and one unavailable link listed, press **Run listed links**. All four lines appear with correct counts. Filter to backup links only and repeat: the archive line and the skipped line are absent, not shown as zero.
- **BKP-F19 (honest figures)** — Read the archive line and the backup line. The archive line reads as an upper bound; the backup line reads as approximate. Then arrange a backup link whose target is larger than its source (add a file to the target that the source never had) and re-open the dialog: the wording still makes sense and the run still copies what is missing.
- **BKP-F19 (cancel)** — Open the dialog and press `Cancel`. No link starts, no card shows progress, and no mapping records a new last-run date.
- **BKP-F20** — Start a run over three links. During the second link, press the run control's Stop. The second link stops and the third **never starts**. Confirm the page returns to idle and the Run button is enabled again.
- **BKP-F20 (pause)** — Start a run over three links, pause during the first, then resume. Only the first link is paused; the run continues to the remaining links afterwards.
- **BKP-F21 (parity with a single run)** — With "Update catalogs" on, run three links from the button. Each link updates its catalogs before comparing, reports on its own card exactly as a single run does, and records its own last-run date and bytes afterwards.
- **BKP-C10** — During a multi-link run, confirm the highlighted running card follows the link actually running, and that no card is left marked as running after the run ends or is stopped.
- **BKP-C11** — Start a multi-link run with "Update catalogs" on, and while it is going try to start a device update or a catalog creation. The existing one-operation-at-a-time refusal still applies; two device operations never run at once. Confirm backup progress appears on the Backup page card, not in the activity panel.
