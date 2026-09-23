---
version: "2.13"
---

# DEVICE Storage Root

![Status](https://img.shields.io/badge/Status-Approved-brightgreen) ![Implementation](https://img.shields.io/badge/Implementation-partial-orange) ![K3](https://img.shields.io/badge/K3-3.0-blue) ![K2](https://img.shields.io/badge/K2-2.13-blue)

## Context

When a Storage device's mount point or drive letter changes — `/mnt/drive1` →
`/mnt/drive01`, or `F:\` → `G:\` — every path recorded for that device goes
stale at once: the source path of each catalog below it, and every file and
folder path those catalogs indexed. The same happens when a catalog's own source
folder is moved or renamed.

The only repair available before this feature was a **full re-scan**, which reads
every file from disk and cannot run at all when the device is not connected — the
very situation a drive-letter change usually accompanies.

This spec adds a second, **disk-free** repair: a prefix replacement performed
entirely on the stored index (and on the `.idx` files in Memory mode). It is
offered as a choice, never imposed: the re-scan remains available, and so does
doing nothing.

> **Reading the requirement IDs.** Each requirement has a permanent ID.
> IDs are never renumbered or reused; a retired requirement is marked
> `[Removed]`, not deleted. Status is one of
> `[Implemented] / [Planned] / [Backlog] / [Removed]`.

---

## Scope at a glance

| | |
|---|---|
| **In scope** | The three-way choice offered when a Storage device's path or a Catalog device's source path changes; the prefix-replacement operation over catalog source paths, file paths and folder paths; the Memory-mode `.idx` round trip; the completion report in both UIs; the K3 operation-queue registration of the replacement; ownership of the `storage.storage_path` column |
| **Out of scope** | Undo; partial path renames below the root; any change to what a full re-scan does; moving files on disk; repairing a path that was never set |
| **Applies to** | K2 2.13 and K3 3.0; the operation itself lives in `core/` and is shared |
| **Depends on** | `DeviceUpdateManager` and its progress reporting; in K3, the operation queue of `SpecOperationQueue.md` |

---

## The choice presented to the user

No separate progress dialog: the choice is a question asked **before** the work
starts, and whichever operation the user picks then runs inside the existing
device-update frame (buttons disabled, Stop available, busy indication).

```
The storage path changed.

Old path:  /mnt/drive1
New path:  /mnt/drive01

How should the catalog indexes be updated?

[ Replace path root ]   [ Full re-scan ]   [ Skip ]
```

| Choice | Behaviour |
|--------|-----------|
| **Replace path root** | The prefix replacement specified below |
| **Full re-scan** | The existing device update, unchanged |
| **Skip** | The save completes; the indexes are knowingly left stale |

---

## Operational requirements — *why / for whom*

| ID | Requirement | Status |
|----|-------------|--------|
| DSR-O1 | A user whose drive is now mounted somewhere else — `/mnt/drive1` becomes `/mnt/drive01`, `F:\` becomes `G:\` — fixes every catalog under it in seconds, with the drive unplugged and without re-reading one file from disk. | [Implemented] |
| DSR-O2 | A user who changes a path picks what happens to the catalogs already indexed: fix the stored paths, rebuild them from disk, or leave them as they are. Katalog does not pick for them. | [Implemented] |
| DSR-O3 | A user who picks *Replace path root* is told it ran, and how many catalogs, files and folders it changed. It takes a fraction of a second, so with no message on screen there is nothing to tell it apart from a button that did nothing. | [Planned] |

## Functional requirements — *what the system does*

| ID | Requirement | Status |
|----|-------------|--------|
| DSR-F1 | Saving a **Storage** device whose path changed, when the previous path was not empty, asks how the catalog indexes should be updated and offers exactly three buttons: *Replace path root*, *Full re-scan*, *Skip*. | [Implemented] |
| DSR-F2 | *Replace path root* on a Storage device replaces the old root with the new one at the start of `catalog.catalog_source_path` for every Catalog device below it, and of `file.file_full_path`, `file.file_folder_path` and `folder.folder_path` in those catalogs. `device.device_path` of each catalog is written with the same new value, so it keeps matching `catalog_source_path`. | [Implemented] |
| DSR-F3 | Saving a **Catalog** device whose source path changed offers the same three buttons, and *Replace path root* rewrites that one catalog's own `file` and `folder` rows. (The Catalog dialog labels the middle button *Full re-index*, not *Full re-scan* — see User-visible text.) | [Implemented] |
| DSR-F4 | A catalog whose `catalog_source_path` does not start with the old root is skipped: none of its rows are touched and it is not included in the catalog count reported by DSR-F7. | [Implemented] |
| DSR-F5 | In Memory mode the `file` and `folder` tables are empty until loaded, so the replacement calls `loadCatalogFileListToTable()` and `loadFoldersToTable()` first, and `saveCatalogToFile()` afterwards to write the corrected rows back to `.idx` and `.folders.idx`. Without the write-back the repair is lost on restart. | [Implemented] |
| DSR-F6 | The replacement runs with the drive unplugged. It reads and writes the collection only — it never opens the device's own files. | [Implemented] |
| DSR-F7 | When a replacement completes, its result is reported as a single status message built with `StatusBarMessageBuilder`: operation `Update`, status `Completed`, device context `Catalog N of N \| <device name>` where N is the number of catalogs updated, process `Paths Updated: F of F (100%)` where F is the number of file rows rewritten, and result `Folders found: D` where D is the number of folder rows rewritten. When no catalog matched, `StatusBarMessageBuilder` omits the device-context field (it drops it whenever the total is 0), so the message reads `UPDATE | Completed | Paths Updated: 0 | Folders found: 0`. That is accepted rather than worked around: the message still appears, and the builder is shared by every status message in the application. | [Planned] |
| DSR-F8 | The DSR-F7 message goes to the K2 status bar (cleared after the usual 5 seconds) and to the K3 activity panel (lingering under `OPQ-F12` / `OPQ-C14`). Neither opens a dialog for it — there is nothing for the user to answer. In K2 `reportAllUpdates()` therefore shows no `QMessageBox` for this operation type. | [Planned] |

## Constructional requirements — *how it is built / limits / MUST-NOTs*

| ID | Requirement | Status |
|----|-------------|--------|
| DSR-C1 | The replacement is its own `DeviceUpdateManager` operation type, emitting the same `operationStarted` / `operationCompleted` / `operationError` signals as `updateDeviceHierarchy()`. It works on the collection only and MUST NOT call into the file system or list the device's contents. | [Implemented] |
| DSR-C2 | The replacement MUST NOT write `catalog.catalog_file_path`. That is where the collection keeps its own `.idx` files, which has nothing to do with where the drive is mounted. | [Implemented] |
| DSR-C3 | Both roots go through `QDir::fromNativeSeparators()` and lose any trailing `/` before they are compared or written, so a path typed with backslashes still matches rows stored with forward slashes. Paths are stored in that form — `toNativeSeparators()` is **not** applied on write-back, matching how Katalog stores paths everywhere else. | [Implemented] |
| DSR-C4 | The DSR-F1 / DSR-F3 question is asked before any work starts; whichever button is pressed then runs inside the existing device-update frame — `setCatalogUpdateUIState()` in K2, the activity panel in K3. No extra window and no second progress bar is added for it. | [Implemented] |
| DSR-C5 | The message of DSR-F7 is built once, in `core/`, and translated in the `MainWindow` context, so K2 and K3 render identical text from one set of translations. A UI layer MUST NOT compose its own variant, and the message MUST NOT be assembled as a raw concatenated string — `StatusBarMessageBuilder` is the only permitted producer. | [Planned] |
| DSR-C6 | A path-root replacement is a device operation: in K3 it MUST be requested through the same single entry point as a device update, and queued as an *update* entry, so that the one-operation-at-a-time guard (`OPQ-C4`, `OPQ-C15`) and the synchronous running flag (`OPQ-C13`) apply to it unchanged. It MUST NOT drive `DeviceUpdateManager` directly. | [Planned] |
| DSR-C7 | A replacement MUST NOT call `finishRunningOperation()` or `scheduleNextOperation()` unless it is itself the entry that is running — otherwise it ends somebody else's operation and lets the next one start on top of it. It MUST NOT emit `catalogCreationCompleted`, which today makes a replacement announce *"Catalog created successfully."* whenever the Create page is open. | [Planned] |
| DSR-C8 | The path of a Storage device is stored twice: `device.device_path` and `storage.storage_path`. Saving the device MUST write **both**, from the same value — K2 `saveDeviceForm()` and K3 `AppManager::saveStorageDetails()`, neither of which writes `storage_path` today. `Device::replaceStorageRootInIndexes()` MUST stop writing it: it is currently the only code that does, so pressing *Full re-scan* or *Skip* leaves `storage_path` at the old path while `device_path` holds the new one. | [Planned] |

---

## Memory mode

`saveCatalogToFile()` already serialises the in-memory tables back to `.idx` and
`.folders.idx`. For each affected catalog in Memory mode the sequence is: load
the file list and the folder list into the in-memory tables, apply the
replacement, then write both files back — the `.idx` header carries the catalog
source path, so it is corrected by the same write. In File and Hosted mode the
stored update is sufficient and both extra steps are skipped (DSR-F5).

## Edge cases

| Case | Handling |
|------|----------|
| The old root is not a prefix of a catalog's source path | That catalog is skipped and not counted (DSR-F4) |
| A Memory-mode index file is missing | The failure is logged and the catalog is skipped; the operation continues |
| Old and new roots are equal | Cannot occur: the choice is only offered when the path actually changed |
| The path is being set for the first time (previous path empty) | No choice is offered — there are no stale indexes to repair (DSR-F1) |
| Mixed path separators between the stored index and the new path | Both sides are normalised before comparison (DSR-C3) |

---

## Note on `storage.storage_path`

The path of a Storage device is stored in two columns, `device.device_path` and
`storage.storage_path`. **Only the first one is ever read.**

`Storage::loadStorage()` does read `storage_path` into `Storage::path`, but
`Device::loadDevice()` overwrites it from `device_path` on the very next line,
and every other assignment to `Storage::path` in the codebase copies
`device->path` into it the same way. The one place a `Storage` is loaded on its
own — the K3 devices page detail — does not read `.path` at all. What remains
writes the column, saves it to `storage.csv`, reads it back and copies it when a
collection is imported; nothing acts on the value.

It is legacy, from before `device` became the object that owns the hierarchy.
The column is **kept, not dropped**: `storage.csv` is positional, so removing a
field shifts every one after it and a released 2.12 binary would misread
collections written by a newer version. DSR-C8 keeps it truthful instead, which
costs a few lines and removes the question permanently.

**If you are reading this because you found `storage_path` and wondered whether
you need to maintain it: you do not need to do anything beyond DSR-C8.**

---

## User-visible text

New string — this one and no other:

| String | Use |
|--------|-----|
| `Paths Updated` | the process title of the completion message (DSR-F7) |

Reused verbatim from existing K2 strings — no new translation slots:

| String | Use |
|--------|-----|
| `The storage path changed.` | opening line of the Storage choice dialog |
| `The catalog source path changed.` | opening line of the Catalog choice dialog |
| `Old path:` / `New path:` | the two path lines of the dialog |
| `How should the catalog indexes be updated?` | closing question of the dialog |
| `Replace path root` / `Skip` | two of the three choices |
| `Full re-scan` / `Full re-index` | the third choice. **The two dialogs do not agree**: the Storage dialog says *Full re-scan* and the Catalog dialog says *Full re-index*. Both are existing strings; unifying them would retire one across every language and needs its own decision. Recorded here rather than silently corrected |
| `Update` / `Completed` | operation and status fields of the completion message |
| `Folders found` | result field of the completion message |

In K3 these are reached through the K2→K3 context bridge described in
`.claude/agents/translations.md` — interface language *selection* is
[SpecLanguages](SpecLanguages.md), which scopes the bridge out — and the bridge
matches on the **byte-exact** source text. `Old path:`
and `New path:` carry their colon inside the string and no trailing space; the
separating space belongs to the surrounding layout.

Any further string requires its own approval before it is written.

---

## Manual test charter

For each row: set up the stated condition, perform the action, confirm the result.

- **DSR-O1** — Disconnect a storage device, change its path, choose *Replace path root*. The repair completes without the device being present and without a visible scan.
- **DSR-O2** — Change a path three times, taking a different one of the three choices each time. Each choice does what it says; none is applied without being chosen.
- **DSR-O3** — After a replacement, confirm the outcome is stated somewhere the user can read it, not merely in the log.
- **DSR-F1** — Save a Storage device with a changed path. The dialog appears with exactly three choices. Choose *Skip*: the save completes and the indexes still hold the old paths.
- **DSR-F2** — Under a Storage device with two catalogs, change the storage path and choose *Replace path root*. Both catalogs' source paths, and their file and folder paths, now start with the new root. The device path of each catalog matches its source path.
- **DSR-F3** — Change a Catalog device's own source path and choose *Replace path root*. Its file and folder paths are rewritten; other catalogs are untouched.
- **DSR-F4** — Put one catalog under the storage whose source path lies outside the old root. Run the replacement: that catalog is unchanged and the reported catalog count excludes it.
- **DSR-F5** — Repeat DSR-F2 in Memory mode. Close and reopen the collection: the repaired paths are still there, and a search finds files at the new root.
- **DSR-F6** — Run the whole of DSR-F2 with the drive physically unplugged.
- **DSR-F7** — Run a replacement over catalogs holding a known number of files and folders. The message reads `UPDATE | Completed | Catalog N of N | <device> | Paths Updated: F of F (100%) | Folders found: D`, with N, F and D matching what was changed. Run one that matches no catalog: the message still appears, without the `Catalog N of N` field — `UPDATE | Completed | Paths Updated: 0 | Folders found: 0`.
- **DSR-F8** — Confirm the message appears in the K2 status bar and clears on its own, and in the K3 activity panel where it lingers like any other completed operation. Confirm no dialog opens in either.
- **DSR-C1** — Watch disk activity during a replacement on a large catalog: there is none beyond the collection's own storage (and the `.idx` write in Memory mode).
- **DSR-C2** — Note a catalog's index file location before a replacement and after: unchanged.
- **DSR-C3** — On Windows, change a drive letter and run the replacement; then search. Paths resolve. Repeat with a path typed using the opposite separator.
- **DSR-C4** — During a replacement, confirm the usual device-update frame is what is shown: no extra window, no second progress bar.
- **DSR-C5** — Run the same replacement in K2 and in K3 with the language set to French: the two messages are identical, word for word.
- **DSR-C6** — In K3, start a long device update, then change a storage path and choose *Replace path root*. It is accepted as a queued entry, appears in the queue, and runs only after the update finishes. Repeat while a search runs: it is refused, as any update would be.
- **DSR-C7** — In K3, queue two device updates, then trigger a replacement. The running update keeps its place in the panel and the queue does not jump ahead. With the Create page open, run a replacement: no *"Catalog created successfully."* notice appears.
- **DSR-C8** — Change a Storage device's path and choose *Skip*. The storage record's path matches the new device path. Repeat choosing *Full re-scan*, and again choosing *Replace path root*: all three agree.

---

## Deferred

- **Undo** — a replacement can be reversed by setting the old root back and running it again; a dedicated undo is not planned.
- **Partial renames** — only the root is replaceable. A folder renamed deeper in the tree still needs a re-scan.
- **Repairing a device whose path was never set** — nothing to repair; the update fills in the device's own figures instead.
