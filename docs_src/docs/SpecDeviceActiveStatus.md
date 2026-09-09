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

## Current State

Two ways the cache is refreshed today:

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
| Blocking probe | `QDir::exists()` on an unreachable network mount (NFS, SMB, sshfs) blocks for the OS timeout — seconds to tens of seconds — and runs on the UI thread |
| Database writes | One `UPDATE` per device per pass; in Hosted mode that is one round trip per device |
| Worst moment | An unguarded refresh on window activation freezes the app exactly as the user returns to it, which is more disruptive than the same delay during an explicit action |

These are why the automatic refresh is **gated** rather than unconditional.

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
Local paths are therefore probed on every activation. What the gate protects
against is not probe cost in general — a `QDir::exists()` on a mounted local disk
is negligible — but the one case that hurts: an unreachable network mount, where
`QDir::exists()` blocks for the OS timeout on the UI thread. Those devices stay
gated, and the classification itself never touches them.

The debounce is the second half of the trade-off. It protects against a user who
alt-tabs repeatedly, or an activation that arrives immediately after startup where
the collection was just opened and already probed — without it, each of those
would pay a fresh pass over every local device.

---

## Scope at a glance

| | |
|---|---|
| **In scope** | K3 refresh on device-list rebuild (K2 alignment); K3 refresh on window activation — unconditional for local mounts, mount-signature-gated for network mounts; removal of the duplicate probe in core |
| **Out of scope** | Timer-based polling; mount/unmount event subscription (Solid); moving the probe off the UI thread; any K2 UI change |
| **Applies to** | K3 3.0; the core fix benefits K2 as well |

## Operational requirements — *why / for whom*

| ID | Requirement | Status |
|----|-------------|--------|
| DAS-O1 | A user sees which devices are currently reachable, so they know what can be searched, updated, or used for backup. | [Implemented] |
| DAS-O2 | A user who opens the Devices page, or switches its All / Storage / Catalogs filter, sees status that reflects reality at that moment — as K2 already does. | [Implemented] |
| DAS-O3 | A user who connects or disconnects a drive while working in another application can have the correct status shown on returning to Katalog, without triggering a refresh by hand. | [Planned] |
| DAS-O5 | A user who does not want the application touching devices on its own keeps that behaviour off — it is opt-in, so nobody acquires a background filesystem probe they did not ask for. | [Implemented] |
| DAS-O4 | A user is never made to wait for a status refresh that had nothing to detect. | [Planned] |
| DAS-O6 | A user who renames, moves, removes or restores a folder on a local disk while working in another application sees the corrected status on returning to Katalog, without a drive having to be mounted or unmounted. | [Planned] |

## Functional requirements — *what the system does*

| ID | Requirement | Status |
|----|-------------|--------|
| DAS-F1 | Active status is the result of testing whether the device path exists, cached in `device.device_active`. Displays read the cache. | [Implemented] |
| DAS-F2 | The cache is refreshed when a collection is opened and after any device operation that changes the device list. | [Implemented] |
| DAS-F3 | The cache is refreshed whenever the K3 device list is rebuilt for display — opening the Devices page and switching its type filter — matching K2. | [Implemented] |
| DAS-F4 | The cache is refreshed when the K3 application window becomes active, subject to DAS-F5 and DAS-F6. | [Implemented] |
| DAS-F8 | DAS-F4 is **off by default** and enabled by a Settings option. When off, no activation handler work is performed at all — not even the mount-table read of DAS-F5. | [Implemented] |
| DAS-F9 | The setting persists per collection in the settings `.ini` under `Settings/RefreshDeviceStatusOnActivation`, defaulting to `false`. | [Implemented] |
| DAS-F5 | On activation the mount table is compared with the previously recorded signature. When unchanged, no probe and no database write occur **for devices whose path is on a network mount**, which is what keeps an unreachable share from blocking the UI thread. Devices on local mounts are probed per DAS-F10. The signature is recorded only when a probe actually runs, so a change arriving inside the DAS-F6 window stays pending rather than being consumed and lost. | [Planned] |
| DAS-F6 | On activation a refresh is skipped when one was performed less than **30 seconds** earlier, which also absorbs the activation that follows application startup. | [Implemented] |
| DAS-F7 | An explicit user action (DAS-F2, DAS-F3) refreshes unconditionally; the gate of DAS-F5 and DAS-F6 applies only to the automatic path of DAS-F4. | [Implemented] |
| DAS-F10 | On activation, devices whose path is on a local mount are probed whether or not the mount table changed — the mount signature cannot see a folder renamed inside a mounted filesystem. The DAS-F6 debounce still applies, so at most one such pass runs per 30 seconds, which is what stops repeated alt-tabbing from re-probing every local device. | [Planned] |

## Constructional requirements — *how it is built / limits / MUST-NOTs*

| ID | Requirement | Status |
|----|-------------|--------|
| DAS-C1 | **Approved core change.** `Collection::updateAllDeviceActive()` MUST NOT probe and write twice per device. `Device::loadDevice()` already calls `updateActiveState()`, so the redundant second call is removed. | [Implemented] |
| DAS-C2 | **New core method, needs approval.** `Collection::mountSignature()` returns a stable signature of the currently mounted volumes' root paths. Reading the mount table is a system query and belongs in `core/`, not in `AppManager`. | [Proposed] |
| DAS-C3 | The refresh *policy* — when to check, the debounce, the retained signature — lives in the K3 UI layer (`AppManager`). `core/` exposes the probe and the signature; it does not decide when they run. | [Implemented] |
| DAS-C4 | `mountSignature()` MUST NOT query per-volume space or availability, only root paths, so an unreachable network mount is never touched by the gate itself. | [Implemented] |
| DAS-C5 | K2 MUST NOT change other than inheriting DAS-C1 and DAS-C11. K2 is in maintenance mode and keeps its existing trigger points. | [Implemented] |
| DAS-C6 | The refresh itself is silent — no status-bar message, no notification, no progress. Exactly **one** new translatable string is added, the Settings checkbox label `Refresh device status when returning to the application`, approved for this purpose. Its row label cell is left empty rather than introducing a second string. | [Implemented] |
| DAS-C7 | The probe remains synchronous on the UI thread. Moving it to a worker is out of scope; DAS-F5 is what keeps the common case free. Revisit if users report freezes with network devices. | [Implemented] |
| DAS-C8 | K3 uses the application-level activation signal (`Qt.application.state`), not a per-window one, so behaviour is unaffected by any future secondary window. | [Implemented] |
| DAS-C9 | **Approved core change.** Deciding whether a device path sits on a local or a network mount is a system query and belongs in `core/`. Preferred shape: a parameter on the existing probe, `Collection::updateAllDeviceActive(bool skipNetworkPaths = false)`, so the mount table is read once per pass rather than once per device. `AppManager` decides *whether* to skip, per DAS-C3; `core/` performs the classification. | [Planned] |
| DAS-C10 | Local/network classification MUST be derived from the mount-table listing already used by DAS-C2 — root path plus filesystem type — matched to the device path by longest root-path prefix. It MUST NOT stat, statfs or otherwise touch the device path itself, and MUST NOT query per-volume space or readiness, so an unreachable share is never contacted by the classification. A path matching no mount root is treated as **network**, so the default is the non-blocking one. | [Planned] |
| DAS-C11 | **Approved core change.** `Device::updateActiveState()` MUST issue the `UPDATE` only when the probed value differs from the value loaded for that device; without this, DAS-F10 costs one write per device per focus change, which in Hosted mode is one network round trip each, on the UI thread. The stored value MUST come from the row `Device::loadDevice()` already reads — adding `device_active` to that existing SELECT — so no extra query is introduced. Where no loaded value is available (e.g. `Device::updateStorageOnly()` during early initialisation), the write occurs unconditionally. | [Planned] |

---

## Known limitation

The automatic refresh is **catch-up-on-return**, not monitoring. A drive
connected or removed — or a local source folder renamed — while Katalog stays in
the foreground is not detected until the window is next activated or the list is
rebuilt.

A change detected inside the 30-second debounce window of DAS-F6 is not lost, but
is applied on the *next* activation rather than the current one. A user who
returns once and stays therefore still sees the previous status until they switch
away and back, or until an explicit refresh of DAS-F7 occurs. Closing that gap needs
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
- **DAS-F5 (change inside the debounce)** — With the setting on, trigger a refresh, then within 30 seconds switch away, connect a drive, and switch back. Switch away and back once more. The device shows as active; the mount change is not lost.
- **DAS-F6** — Start the application and immediately alt-tab away and back. Only one refresh runs, not two.
- **DAS-F7** — Immediately after an automatic refresh, switch the device filter. The refresh happens despite being inside the 30-second window.
- **DAS-F10 / DAS-O6** — With the setting on, switch to another application, rename a catalogued source folder on an internal disk, switch back. The device shows as inactive with no mount change having occurred. Rename it back, switch away and back: it shows as active again.
- **DAS-O4** — With the setting on, open the Devices page (an explicit refresh), then alt-tab away and back with no mount change. No second probe runs.
- **DAS-C1** — Instrument or log `updateActiveState()`. Opening a collection with N devices produces N probes, not 2 × N.
- **DAS-C4** — Configure a device on an unreachable network path. Alt-tab away and back with no mount change. The window becomes responsive immediately; the dead path is not probed.
- **DAS-C10** — With the same unreachable network device configured, switch away and back with no mount change. The window is responsive immediately and the dead path is not probed, while local devices are still refreshed in the same pass.
- **DAS-C11** — Instrument `UPDATE device SET device_active`. Switch away and back repeatedly with nothing changed on disk. Probes run for local devices; no UPDATE statements are issued.
- **DAS-C5** — K2 behaviour is unchanged: status refreshes on device-tree rebuild as before, and there is no activation refresh.
