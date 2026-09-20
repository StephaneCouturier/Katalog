# DEVICE Active Status

![Status](https://img.shields.io/badge/Status-Approved-brightgreen) ![Implementation](https://img.shields.io/badge/K3-3.0-blue) ![K2](https://img.shields.io/badge/K2-core%20fix%20only-lightgrey)

## Context

A device is **active** when the drive or folder it points at is currently
reachable. The Devices and Selection screens show this so the user knows which
catalogs can be searched on the connected drive, updated, or used as a backup
source or target.

Active is **not** stored knowledge about the device — it is a *probe of the
filesystem, cached in the database*:

```
Device::updateActiveState()   →   QDir(path).exists()
                              →   UPDATE device SET device_active = …
```

The path tested is `device.device_path`, and this is the same test for both
device types. For a **Storage** device the path is the drive or folder the device
represents. For a **Catalog** device the path is the **catalogued source folder** —
the folder that was scanned to build the catalog. A catalog is therefore active
when that source folder currently exists, which is true when its drive is mounted
*and* the folder itself has not been renamed, moved or deleted. Both cases run the
same `QDir::exists()`.

Everything that displays active status reads the cached `device.device_active`
column, never the filesystem. The whole question is therefore **when the cache is
recomputed**, which is what this spec fixes.

---

## Starting point (state before this spec)

This section records the baseline this spec was written against. It is history, not
current behaviour — the requirement tables below are what holds today.

Two ways the cache was refreshed:

| Mechanism | Where |
|-----------|-------|
| Bulk refresh of all Storage + Catalog devices | `Collection::updateAllDeviceActive()` |
| Implicit, for one device | every `Device::loadDevice()` |

Trigger points, K2 versus K3:

| Trigger | K2 | K3 |
|---------|----|----|
| Collection opened / settings applied | Yes | Yes |
| After a device operation or update | Yes | Yes — `AppManager::refreshDeviceList()` |
| Device list rebuilt for display | **Yes** — inside all three model loaders | **No** |
| Switching the All / Storage / Catalogs filter | **Yes** | **No** |
| Opening the Devices page | n/a — K2 tab switch does not refresh either | **No** |
| Automatically, without user action | No | No |

**The divergence.** K2 calls `updateAllDeviceActive()` *inside* the functions that
populate the device tree, so any repopulation re-probes the filesystem. K3's
Devices page reads `AppManager::getDeviceList()` → `Device::loadDeviceTree()`,
which selects `device_active` straight from the database with no probe. The K3
Selection page is unaffected: it binds to the device list model, which only
reloads through `refreshDeviceList()`, and that does refresh.

**A defect in the shared path.** `Collection::updateAllDeviceActive()` calls
`loadDevice()` and then `updateActiveState()` on each device, but `loadDevice()`
*already* calls `updateActiveState()` internally. Every device therefore pays two
`QDir::exists()` calls and two `UPDATE` statements per pass — twice the necessary
cost of the operation this spec makes more frequent.

---

## Cost and risk

| Factor | Detail |
|--------|--------|
| Blocking probe | `QDir::exists()` on an unreachable network mount (NFS, SMB, sshfs) blocks for the OS timeout — seconds to tens of seconds |
| Synchronous on the UI thread | The probe runs on the UI thread by DAS-C7; moving it to a worker is out of scope. This is the whole reason network mounts are excluded from the routine check — there is no thread to absorb a blocking call |
| Database writes | One `UPDATE` per device per pass; in Hosted mode that is one round trip per device. DAS-C11 reduces this to writes for devices whose value actually changed |
| Worst moment | An unguarded refresh on window activation freezes the app exactly as the user returns to it, which is more disruptive than the same delay during an explicit action |

**Cost of one activation, as designed:** one `QDir::exists()` per device on a
local mount; nothing at all for devices on network mounts unless the mount table
changed; a database write only for a device whose value changed; and a reload of
the device list only when at least one value changed. A local existence check on
a mounted filesystem is negligible, which is why local devices need no throttle.

The blocking case is the only expensive one, and it is what the mount-signature
gate exists to avoid.

---

## The mount-change gate

Before paying for N probes, compare the current mount table against the previous
one. On Linux `QStorageInfo::mountedVolumes()` reads the mount table and is cheap
when only the root paths are used; no per-volume space query is performed, so
dead network mounts are not touched.

```
signature = hash of the sorted rootPath() list
if (signature == lastSignature)  → nothing mounted or unmounted,
                                   skip the devices on network mounts
else                             → run the full probe
                                 → store the new signature
```

The signature is stored **only when a probe actually runs**. Storing it earlier
would mark a change as seen and then drop it, leaving the status stale until
something else happened to reload the list.

**The gate is not applied to local mounts.** A signature of mount root paths
cannot see a change *inside* a mounted filesystem: renaming, moving or deleting a
catalogued source folder on an internal disk leaves the mount table byte-identical.
Local paths are therefore probed on every activation, with no delay and no
throttle. What the gate protects against is not probe cost in general — a
`QDir::exists()` on a mounted local disk is negligible — but the one case that
hurts: an unreachable network mount, where `QDir::exists()` blocks for the OS
timeout on the UI thread. Those devices stay gated, and the classification itself
never touches them.

---

## Scope at a glance

| | |
|---|---|
| **In scope** | K3 refresh on device-list rebuild (K2 alignment); K3 refresh on window activation — unconditional for local mounts, mount-signature-gated for network mounts; removal of the duplicate probe in core; **the open-action guard** — refusing to open a file or folder from the K3 Search results list or the K3 Explore file list when the owning device is not active |
| **Out of scope** | Timer-based polling; mount/unmount event subscription (Solid); moving the probe off the UI thread; any K2 UI change — **including K2's six open handlers, which keep opening unconditionally**; refreshing active status at search launch (considered and rejected, see the note below the table) |
| **Applies to** | K3 3.0; the core fix benefits K2 as well |

**Refresh at search launch — considered and rejected.** Adding a trigger point that
re-probed device status when a search is launched was weighed as part of `DAS-O7`
and turned down. It does not address the case: the drive can be disconnected
after the search completes and before the row is clicked, so a status taken at
launch is already capable of being wrong by the time it would be used. And a
search is an explicit user action, which under `DAS-F7` refreshes unconditionally
— including devices on network mounts — so it would place exactly the blocking
probe that `DAS-F5` and `DAS-C10` exist to avoid on the UI thread (`DAS-C7`), at
the start of every search. The click-time probe of `DAS-F12` answers the same
question with one existence test and no staleness window. No requirement ID was
allocated; this is a rejected option, not a retired requirement.

## Operational requirements — *why / for whom*

| ID | Requirement | Status |
|----|-------------|--------|
| DAS-O1 | A user sees which devices are currently reachable, so they know what can be searched, updated, or used for backup. | [Implemented] |
| DAS-O2 | A user who opens the Devices page, or switches its All / Storage / Catalogs filter, sees status that reflects reality at that moment — as K2 already does. | [Implemented] |
| DAS-O3 | A user who connects or disconnects a drive while working in another application can have the correct status shown on returning to Katalog, without triggering a refresh by hand. | [Planned] |
| DAS-O5 | A user who does not want the application touching devices on its own keeps that behaviour off — it is opt-in, so nobody acquires a background filesystem probe they did not ask for. | [Implemented] |
| DAS-O4 | A user is never made to wait for a status refresh that had nothing to detect. | [Planned] |
| DAS-O6 | A user who renames, moves, removes or restores a folder on a local disk while working in another application sees the corrected status on returning to Katalog, without a drive having to be mounted or unmounted. | [Planned] |
| DAS-O7 | A user clicking a result whose device is disconnected is told by Katalog that the device is not active, instead of receiving the desktop environment's generic file-not-found message, which does not explain the cause. | [Implemented] |

## Functional requirements — *what the system does*

| ID | Requirement | Status |
|----|-------------|--------|
| DAS-F1 | Active status is the result of testing whether the device path exists, cached in `device.device_active`. Displays read the cache. | [Implemented] |
| DAS-F2 | The cache is refreshed when a collection is opened and after any device operation that changes the device list. | [Implemented] |
| DAS-F3 | The cache is refreshed whenever the K3 device list is rebuilt for display — opening the Devices page and switching its type filter — matching K2. | [Implemented] |
| DAS-F4 | The cache is refreshed when the K3 application window becomes active, subject to DAS-F5. | [Implemented] |
| DAS-F8 | DAS-F4 is **off by default** and enabled by a Settings option. When off, no activation handler work is performed at all — not even the mount-table read of DAS-F5. | [Implemented] |
| DAS-F9 | The setting persists per collection in the settings `.ini` under `Settings/RefreshDeviceStatusOnActivation`, defaulting to `false`. | [Implemented] |
| DAS-F5 | On activation the mount table is compared with the previously recorded signature. When unchanged, no probe and no database write occur **for devices whose path is on a network mount**, which is what keeps an unreachable share from blocking the UI thread. Devices on local mounts are probed per DAS-F10. The signature is recorded only when a probe actually runs. | [Planned] |
| DAS-F6 | ~~On activation a refresh is skipped when one was performed less than **30 seconds** earlier, which also absorbs the activation that follows application startup.~~ Withdrawn: the 30-second window was sized when every check probed every device including network shares. With network mounts excluded by DAS-F5, a check costs one existence test per local device and needs no throttle, while the window made the feature appear broken — a folder renamed and checked within 30 seconds was never picked up. ID retired, not reused. | [Removed] |
| DAS-F7 | An explicit user action (DAS-F2, DAS-F3) refreshes unconditionally, including the devices on network mounts; the gate of DAS-F5 applies only to the automatic path of DAS-F4. | [Implemented] |
| DAS-F10 | On activation, devices whose path is on a local mount are probed whether or not the mount table changed — the mount signature cannot see a folder renamed inside a mounted filesystem. The check runs on every activation with no delay and no throttle; its cost is one existence check per local device. | [Planned] |
| DAS-F11 | Opening a file or folder from the K3 Search results list or the K3 Explore file list first determines the active status of the device that owns the row. When it is not active, the open is **not attempted** — no `openUrl` call is made — and Katalog reports the reason itself. The report is refuse-and-inform, never an offer to try anyway: there is no "open anyway" choice, so no decision is put to the user. Presentation is a `Kirigami.InlineMessage`, `type: MessageType.Warning`, `showCloseButton: true`, anchored at the top of the page the row was clicked on — the Search results page and the Explore file list page each carry their own. The channel follows the *Message presentation* table of `SpecValidationRules.md`; that spec is cited, not extended, since an open action is not form validation. A passive notification and a modal dialog are both excluded. The message text is exactly, and only, `The device is not active. It may be disconnected, or its path may have changed.` — no device name, no path, no placeholder, no parenthetical, no second sentence. | [Implemented] |
| DAS-F12 | The `DAS-F11` check probes that **one** device's path at the moment of the click rather than reading the cached `device_active` column, so the answer cannot be stale — a drive disconnected after the results were produced is detected. It costs one existence test on one path, and it updates the cache subject to `DAS-C11` (a write only when the value changed). It MUST NOT probe any other device, and MUST NOT trigger a full `Collection::updateAllDeviceActive()` pass. | [Implemented] |

## Constructional requirements — *how it is built / limits / MUST-NOTs*

| ID | Requirement | Status |
|----|-------------|--------|
| DAS-C1 | **Approved core change.** `Collection::updateAllDeviceActive()` MUST NOT probe and write twice per device. `Device::loadDevice()` already calls `updateActiveState()`, so the redundant second call is removed. | [Implemented] |
| DAS-C2 | **New core method, needs approval.** `Collection::mountSignature()` returns a stable signature of the currently mounted volumes' root paths. Reading the mount table is a system query and belongs in `core/`, not in `AppManager`. | [Proposed] |
| DAS-C3 | The refresh *policy* — when to check, the retained signature — lives in the K3 UI layer (`AppManager`). `core/` exposes the probe and the signature; it does not decide when they run. | [Implemented] |
| DAS-C4 | `mountSignature()` MUST NOT query per-volume space or availability, only root paths, so an unreachable network mount is never touched by the gate itself. | [Implemented] |
| DAS-C5 | K2 MUST NOT change other than inheriting DAS-C1 and DAS-C11. K2 is in maintenance mode and keeps its existing trigger points. | [Implemented] |
| DAS-C6 | The refresh itself is silent — no status-bar message, no notification, no progress. Exactly **one** new translatable string is added, the Settings checkbox label `Refresh device status when returning to the application`, approved for this purpose. Its row label cell is left empty rather than introducing a second string. | [Implemented] |
| DAS-C7 | The probe remains synchronous on the UI thread. Moving it to a worker is out of scope; DAS-F5 is what keeps the common case free. Revisit if users report freezes with network devices. | [Implemented] |
| DAS-C8 | K3 uses the application-level activation signal (`Qt.application.state`), not a per-window one, so behaviour is unaffected by any future secondary window. | [Implemented] |
| DAS-C9 | **Approved core change.** Deciding whether a device path sits on a local or a network mount is a system query and belongs in `core/`. Preferred shape: a parameter on the existing probe, `Collection::updateAllDeviceActive(bool skipNetworkPaths = false)`, so the mount table is read once per pass rather than once per device. `AppManager` decides *whether* to skip, per DAS-C3; `core/` performs the classification. | [Planned] |
| DAS-C10 | Local/network classification MUST be derived from the mount-table listing already used by DAS-C2 — root path plus filesystem type — matched to the device path by longest root-path prefix. It MUST NOT stat, statfs or otherwise touch the device path itself, and MUST NOT query per-volume space or readiness, so an unreachable share is never contacted by the classification. A path matching no mount root is treated as **network**, so the default is the non-blocking one. | [Planned] |
| DAS-C11 | **Approved core change.** `Device::updateActiveState()` MUST issue the `UPDATE` only when the probed value differs from the value loaded for that device; without this, DAS-F10 costs one write per device per focus change, which in Hosted mode is one network round trip each, on the UI thread. The stored value MUST come from the row `Device::loadDevice()` already reads — adding `device_active` to that existing SELECT — so no extra query is introduced. Where no loaded value is available (e.g. `Device::updateStorageOnly()` during early initialisation), the write occurs unconditionally. | [Planned] |
| DAS-C13 | `DAS-F11` and `DAS-F12` are **K3 only**. This is a named exception to `DAS-C5`, which is **not** amended: K2 stays in maintenance mode and its six open handlers MUST NOT change — `on_Search_treeView_FilesFound_clicked`, `searchContextOpenFile`, `searchContextOpenFolder` in `qt_widgets/mainwindow_tab_search_ui.cpp`, and `on_Explore_treeView_FileList_clicked`, `exploreContextOpenFile`, `exploreContextOpenFolder` in `qt_widgets/mainwindow_tab_explore.cpp`. K2 continuing to open unconditionally, and to let the desktop environment report the failure, MUST NOT be reported as drift against `DAS-O7`, `DAS-F11` or `DAS-F12`. | [Implemented] |
| DAS-C14 | `DAS-F11` spends exactly **one** new source text, the text quoted verbatim in that row, approved for this purpose. No existing string covers this case. Under K3's per-QML-file `qsTr()` contexts that one source text is carried as **two `.ts` entries** — one in the `PageSearchResultsForm` context and one in `PageExploreFiles`, since each page holds its own banner. Two entries for this one text is **authorised and is not a defect**; it MUST NOT be reported as drift against the one-source-text budget. The two entries MUST stay byte-identical to each other and to the approved text, and any future reword MUST update both in the same change. The reason it is two: K3 scopes `qsTr()` by QML file name, and the K2→K3 sync script harvests only from non-K3 contexts, so there is no K3→K3 propagation and the two entries are translated and maintained independently. That drift risk was weighed against hoisting the banner into a shared component and **accepted by the user** on 2026-09-09; the refactor was declined. `confirmCannotUpdateCatalog` (`qt_widgets/mainwindow_ui_wrapper_catalog.cpp`) MUST NOT be edited to be reused: it is one composite sentence bound to catalog update with its own `%1`/`%2`, and changing it would drop it to English in all 30 languages. Any **second source text** for this feature requires its own per-string approval before it is written. | [Implemented] |

---

## Known limitation

The automatic refresh is **catch-up-on-return**, not monitoring. A drive
connected or removed — or a local source folder renamed — while Katalog stays in
the foreground is not detected until the window is next activated or the list is
rebuilt.

**Devices on network mounts are not covered by the routine check.** A folder
renamed, moved or removed on a network share is not detected on activation; only
mounting or unmounting the share itself is. This is an acknowledged gap, not a
partial feature: the probe is synchronous on the UI thread (DAS-C7), and testing a
path on an unreachable share blocks for the OS timeout, so those devices are
deliberately left to the mount-signature gate. Network use is to be addressed as a
separate use case — whether Katalog can work usefully in that context at all is an
open question — and is out of scope here. An explicit refresh (DAS-F7) still
checks them, so a user who needs the status now has a way to get it. Closing that gap needs
either timer polling or mount/unmount event subscription; both were considered
and deferred. Because the gate of DAS-F5 makes a check nearly free, adding a
low-frequency timer later is a small change, and is the recommended next step if
the limitation proves annoying in use.

---

## Manual test charter

For each row: set up the stated condition, perform the action, confirm the result.

- **DAS-F3 (page open)** — With Katalog open, unmount a device from a terminal. Navigate to the Devices page. The device shows as inactive.
- **DAS-F3 (filter switch)** — Unmount a device, then switch the device view between All, Storage and Catalogs. The status is correct after the switch.
- **DAS-F8 (default off)** — On a fresh collection, confirm the Settings checkbox is unchecked. Switch away, unmount a device, switch back. The status does **not** change until the page is rebuilt.
- **DAS-F4** — Enable the setting. Switch to another application, connect a drive holding a catalogued device, switch back to Katalog. The device shows as active without any further action.
- **DAS-F4 (reverse)** — With the setting on, switch away, unmount the drive, switch back. The device shows as inactive.
- **DAS-F9** — Toggle the setting, close and reopen the application. The choice is retained. Confirm `Settings/RefreshDeviceStatusOnActivation` in the collection's `.ini`.
- **DAS-F5** — With no network device configured and nothing mounted or unmounted, alt-tab away and back several times. No database write occurs and there is no perceptible delay. Confirm with query logging or by watching the collection file's modification time.
- **DAS-F7** — With a network device configured, switch the device filter. The network device is probed too, unlike on the automatic path.
- **DAS-F10 / DAS-O6** — With the setting on, switch to another application, rename a catalogued source folder on an internal disk, switch back. The device shows as inactive with no mount change having occurred. Rename it back, switch away and back: it shows as active again.
- **DAS-O4** — With the setting on, open the Devices page (an explicit refresh), then alt-tab away and back with no mount change. No second probe runs.
- **DAS-C1** — Instrument or log `updateActiveState()`. Opening a collection with N devices produces N probes, not 2 × N.
- **DAS-C4** — Configure a device on an unreachable network path. Alt-tab away and back with no mount change. The window becomes responsive immediately; the dead path is not probed.
- **DAS-C10** — With the same unreachable network device configured, switch away and back with no mount change. The window is responsive immediately and the dead path is not probed, while local devices are still refreshed in the same pass.
- **DAS-C11** — Instrument `UPDATE device SET device_active`. Switch away and back repeatedly with nothing changed on disk. Probes run for local devices; no UPDATE statements are issued.
- **DAS-C5** — K2 behaviour is unchanged: status refreshes on device-tree rebuild as before, and there is no activation refresh.
- **DAS-F11 / DAS-O7 (Search results)** — Run a search that returns files from a device on a removable drive. Unmount the drive from a terminal, leaving the results on screen. Left-click a result row, then repeat with *Open file* and *Open folder* from the row's context menu. Each time, no external application is launched and a warning banner appears at the top of the results page reading exactly `The device is not active. It may be disconnected, or its path may have changed.` The banner has a close button, and closing it dismisses it.
- **DAS-F11 (Explore file list)** — Open a catalog in Explore, unmount its drive, then click a file row and use *Open file* and *Open folder* from the context menu. The same banner appears at the top of the Explore file list page and nothing is opened.
- **DAS-F11 (channel)** — Confirm the report is an inline banner on the page: no modal dialog blocks the list, no passive-notification toast appears, and no "open anyway" button is offered.
- **DAS-F11 (no false positive)** — With the drive mounted, click a result row and an Explore file row. The file opens in the default application, no banner appears, and any banner left over from an earlier attempt does not block the open.
- **DAS-F12 (freshness)** — Run a search while the drive is mounted, so the results are produced with the device active. Unmount the drive without touching the Devices page, the device filter, or the application window focus, then click a result. The banner appears — the answer came from a probe at click time, not from the cached column.
- **DAS-F12 (single probe)** — Instrument `updateActiveState()`. Configure several devices, including one on an unreachable network path. Click a result row belonging to a local device. Exactly one probe runs, on that row's device only; the unreachable path is not touched and the click does not block.
- **DAS-C13** — Repeat the two `DAS-F11` steps in K2. K2 attempts the open as before and the desktop environment reports the failure; no Katalog banner appears. This is correct, not a defect.
- **DAS-C14** — Search the K3 sources for the message text. It appears exactly **twice**, once per page — `PageSearchResultsForm.qml` and `PageExploreFiles.qml` — as a `qsTr()` string with no placeholder, and the two occurrences are byte-identical to each other and to the approved text. Search the `.ts` files: each carries exactly two entries for it, one under the `PageSearchResultsForm` context and one under `PageExploreFiles`, with identical `<source>`. A third occurrence, or the two differing by so much as one character, is a defect. `confirmCannotUpdateCatalog` is byte-identical to its previous text.
