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
if (signature == lastSignature)  → nothing mounted or unmounted, stop
else                             → run the full probe, store the new signature
```

In the common case — the user alt-tabbed and nothing changed — the entire
automatic refresh costs one mount-table read: no `QDir::exists()` calls, no
database writes.

---

## Scope at a glance

| | |
|---|---|
| **In scope** | K3 refresh on device-list rebuild (K2 alignment); K3 refresh on window activation, gated; removal of the duplicate probe in core |
| **Out of scope** | Timer-based polling; mount/unmount event subscription (Solid); moving the probe off the UI thread; any K2 UI change |
| **Applies to** | K3 3.0; the core fix benefits K2 as well |

## Operational requirements — *why / for whom*

| ID | Requirement | Status |
|----|-------------|--------|
| DAS-O1 | A user sees which devices are currently reachable, so they know what can be searched, updated, or used for backup. | [Implemented] |
| DAS-O2 | A user who opens the Devices page, or switches its All / Storage / Catalogs filter, sees status that reflects reality at that moment — as K2 already does. | [Planned] |
| DAS-O3 | A user who connects or disconnects a drive while working in another application can have the correct status shown on returning to Katalog, without triggering a refresh by hand. | [Planned] |
| DAS-O5 | A user who does not want the application touching devices on its own keeps that behaviour off — it is opt-in, so nobody acquires a background filesystem probe they did not ask for. | [Planned] |
| DAS-O4 | A user is never made to wait for a status refresh that had nothing to detect. | [Planned] |

## Functional requirements — *what the system does*

| ID | Requirement | Status |
|----|-------------|--------|
| DAS-F1 | Active status is the result of testing whether the device path exists, cached in `device.device_active`. Displays read the cache. | [Implemented] |
| DAS-F2 | The cache is refreshed when a collection is opened and after any device operation that changes the device list. | [Implemented] |
| DAS-F3 | The cache is refreshed whenever the K3 device list is rebuilt for display — opening the Devices page and switching its type filter — matching K2. | [Planned] |
| DAS-F4 | The cache is refreshed when the K3 application window becomes active, subject to DAS-F5 and DAS-F6. | [Planned] |
| DAS-F8 | DAS-F4 is **off by default** and enabled by a Settings option. When off, no activation handler work is performed at all — not even the mount-table read of DAS-F5. | [Planned] |
| DAS-F9 | The setting persists per collection in the settings `.ini` under `Settings/RefreshDeviceStatusOnActivation`, defaulting to `false`. | [Planned] |
| DAS-F5 | On activation the mount table is compared with the previously recorded signature. When unchanged, no probe and no database write occur. | [Planned] |
| DAS-F6 | On activation a refresh is skipped when one was performed less than **30 seconds** earlier, which also absorbs the activation that follows application startup. | [Planned] |
| DAS-F7 | An explicit user action (DAS-F2, DAS-F3) refreshes unconditionally; the gate of DAS-F5 and DAS-F6 applies only to the automatic path of DAS-F4. | [Planned] |

## Constructional requirements — *how it is built / limits / MUST-NOTs*

| ID | Requirement | Status |
|----|-------------|--------|
| DAS-C1 | **Approved core change.** `Collection::updateAllDeviceActive()` MUST NOT probe and write twice per device. `Device::loadDevice()` already calls `updateActiveState()`, so the redundant second call is removed. | [Planned] |
| DAS-C2 | **New core method, needs approval.** `Collection::mountSignature()` returns a stable signature of the currently mounted volumes' root paths. Reading the mount table is a system query and belongs in `core/`, not in `AppManager`. | [Proposed] |
| DAS-C3 | The refresh *policy* — when to check, the debounce, the retained signature — lives in the K3 UI layer (`AppManager`). `core/` exposes the probe and the signature; it does not decide when they run. | [Planned] |
| DAS-C4 | `mountSignature()` MUST NOT query per-volume space or availability, only root paths, so an unreachable network mount is never touched by the gate itself. | [Planned] |
| DAS-C5 | K2 MUST NOT change other than inheriting DAS-C1. K2 is in maintenance mode and keeps its existing trigger points. | [Planned] |
| DAS-C6 | The refresh itself is silent — no status-bar message, no notification, no progress. Exactly **one** new translatable string is added, the Settings checkbox label `Refresh device status when returning to the application`, approved for this purpose. Its row label cell is left empty rather than introducing a second string. | [Planned] |
| DAS-C7 | The probe remains synchronous on the UI thread. Moving it to a worker is out of scope; DAS-F5 is what keeps the common case free. Revisit if users report freezes with network devices. | [Planned] |
| DAS-C8 | K3 uses the application-level activation signal (`Qt.application.state`), not a per-window one, so behaviour is unaffected by any future secondary window. | [Planned] |

---

## Known limitation

The automatic refresh is **catch-up-on-return**, not monitoring. A drive
connected or removed while Katalog stays in the foreground is not detected until
the window is next activated or the list is rebuilt. Closing that gap needs
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
- **DAS-F5** — With nothing mounted or unmounted, alt-tab away and back several times. No database write occurs and there is no perceptible delay. Confirm with query logging or by watching the collection file's modification time.
- **DAS-F6** — Start the application and immediately alt-tab away and back. Only one refresh runs, not two.
- **DAS-F7** — Immediately after an automatic refresh, switch the device filter. The refresh happens despite being inside the 30-second window.
- **DAS-C1** — Instrument or log `updateActiveState()`. Opening a collection with N devices produces N probes, not 2 × N.
- **DAS-C4** — Configure a device on an unreachable network path. Alt-tab away and back with no mount change. The window becomes responsive immediately; the dead path is not probed.
- **DAS-C5** — K2 behaviour is unchanged: status refreshes on device-tree rebuild as before, and there is no activation refresh.
