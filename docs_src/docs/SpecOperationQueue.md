# OPERATION Queue

![Status](https://img.shields.io/badge/Status-Specified-blue) ![Implementation](https://img.shields.io/badge/K3-not%20started-lightgrey) ![K2](https://img.shields.io/badge/K2-no%20change-lightgrey)

## Context

Catalog creation and device updates are **long-running and strictly sequential** —
only one may run at a time. Today that constraint is enforced by *refusing* new
requests, and the refusal is close to invisible:

| Trigger while an operation runs | Current behaviour |
|---------------------------------|-------------------|
| Update a device (`AppManager::updateDevice`) | `if (m_deviceUpdateIsRunning) return;` — **silently dropped**. No message, no queue entry, nothing happens |
| Create a catalog (`AppManager::createCatalog`) | Refused with *"A device operation is already running."* |
| Any new request | The pending list is `clear()`ed and rebuilt, **discarding** anything already waiting |

A user who queues work by clicking is therefore either ignored or rebuffed, and
has to remember to come back and retry. Because the serialisation is never shown,
the application appears unresponsive or broken rather than busy.

The sequential machinery already exists: `m_pendingDeviceUpdates` with
`AppManager::startNextDeviceUpdate()` drains a list one device at a time — it is
how *Update all active* works. What is missing is the ability to **add** to that
list while it is draining, and any way to see it.

This spec covers making the queue additive and visible. It does **not** make
operations run in the background: they continue on the main thread, so the UI
remains sluggish while one runs. That is a separate piece of work.

---

## Scope at a glance

| | |
|---|---|
| **In scope** | Accepting new create/update requests while one runs; a visible queue; removing entries; K3, plus the cancellation *timing* in `core/DeviceUpdateManager` where it makes OPQ-C4 unattainable |
| **Out of scope** | Running operations off the main thread; parallel execution; scheduling; persisting the queue across restarts; K2 |
| **Applies to** | K3 3.0 |
| **Depends on** | The existing `m_pendingDeviceUpdates` / `startNextDeviceUpdate()` runner |

## Why not parallel

Every catalog writes to the same collection database, and **SQLite permits a
single writer**. Two operations on different drives would parallelise scanning
and metadata extraction, then serialise again at every write. Running jobs on
separate physical drives concurrently would also cause seek contention when they
share a disk. Serial execution is therefore the correct model for this phase, and
parallelism is deferred until measurement shows scanning dominates.

---

## Operational requirements — *why / for whom*

| ID | Requirement | Status |
|----|-------------|--------|
| OPQ-O1 | A user requests several catalog operations in a row and walks away; they run one after another without further attention. | [Planned] |
| OPQ-O2 | A user is never silently ignored — every accepted request is visibly recorded, and anything refused says why. | [Planned] |
| OPQ-O3 | A user can see what is running and what is waiting, so a busy application is recognisable as busy rather than broken. | [Planned] |
| OPQ-O4 | A user can change their mind: waiting work can be removed without disturbing what is already running. | [Planned] |

## Functional requirements — *what the system does*

| ID | Requirement | Status |
|----|-------------|--------|
| OPQ-F1 | Requesting a catalog creation or device update while an operation is running **appends** it to the queue instead of dropping or refusing it. | [Planned] |
| OPQ-F2 | Queued entries run **in the order they were added** (FIFO), one at a time, continuing automatically until the queue is empty. | [Planned] |
| OPQ-F3 | Enqueuing a device that is already queued or already running is **ignored**, and the queue is left unchanged. A device cannot appear twice. | [Planned] |
| OPQ-F4 | A new request never discards existing queued entries — the queue is only added to, drained, or explicitly edited by the user. | [Planned] |
| OPQ-F5 | The queue is visible: the entry currently running is distinguished from those waiting, and the number waiting is shown. | [Planned] |
| OPQ-F6 | A waiting entry can be removed individually, and the whole queue can be cleared, without affecting the running operation. | [Planned] |
| OPQ-F7 | Stopping the running operation ends **only that operation**; the queue is left intact and the next entry starts immediately. Emptying the queue is the separate Clear action (OPQ-F6). This deliberately changes the pre-queue behaviour of `stopDeviceUpdate()`, which cleared everything: once the queue is visible, "Stop" reads as "stop this job", and a queue that could never resume after a stop would strand its entries. | [Planned] |
| OPQ-F8 | Queued creations and queued updates share **one** queue, since they contend for the same single-operation-at-a-time constraint. | [Planned] |
| OPQ-F9 | When an operation ends — completed, stopped, or failed — the next entry starts automatically; a failure does not abandon the rest of the queue. | [Planned] |
| OPQ-F10 | Per-operation report acknowledgement continues to work: when a report awaits acknowledgement, the next entry starts only once it is acknowledged. | [Planned] |

## Constructional requirements — *how it is built / limits / MUST-NOTs*

| ID | Requirement | Status |
|----|-------------|--------|
| OPQ-C1 | The queue extends the existing `m_pendingDeviceUpdates` / `startNextDeviceUpdate()` runner. A second, parallel mechanism MUST NOT be introduced. | [Planned] |
| OPQ-C2 | Entries are typed (create or update) so one queue can hold both, per OPQ-F8. | [Planned] |
| OPQ-C3 | Operations continue to run on the main thread. This phase MUST NOT change the execution model — UI responsiveness during a run is explicitly out of scope. | [Planned] |
| OPQ-C4 | Exactly one operation runs at a time. The queue MUST NOT start an entry while another is running. | [Planned] |
| OPQ-C5 | The queue lives in memory only. It is **not** persisted; closing the application discards anything waiting. | [Planned] |
| OPQ-C6 | The queue is held in `AppManager` for this phase, consistent with where the runner already lives. It is orchestration and belongs in `core/` eventually — see the note below — but MUST NOT be moved as part of this phase. | [Planned] |
| OPQ-C7 | K2 MUST NOT change; it keeps its current refuse-when-busy behaviour. | [Planned] |
| OPQ-C8 | The queue MUST behave identically in Memory, File and Hosted database modes — it schedules operations and touches no database itself. | [Planned] |
| OPQ-C11 | Where core's stop handling makes OPQ-C4 unattainable, this phase may change **only when** `DeviceUpdateManager` declares an operation cancelled — deferring it to the catalog job's own cancellation signal instead of a fixed timer that fires inside the running job. No core method may be added, removed or renamed, and the execution model MUST NOT change (OPQ-C3). | [Planned] |

### Note on placement (OPQ-C6)

`CLAUDE.md` records that `AppManager` already carries more orchestration than it
should, and that this belongs in a future core manager. Extending the queue there
adds to that debt knowingly: the alternative — moving the runner into `core/`
first — is a larger change that would block a small, high-value improvement.
When the orchestration move happens, the queue moves with it.

---

## User-visible text

Reused from existing K3 QML — no new translation slots:

| String | Use |
|--------|-----|
| `Create` / `Update` | the operation type shown on each entry |
| `Clear` | empties the queue; the running entry is unaffected (OPQ-F6) |
| `Stop` | stops the running operation and the queue (OPQ-F7) |
| `In Progress` | the state of the entry currently executing — the same string every other Katalog operation uses through `StatusBarMessageBuilder` |

Approved new strings — these two and no others:

| String | Use |
|--------|-----|
| `Queue` | section title |
| `%1 waiting` | count of pending entries. Takes no plural form: "1 waiting" and "3 waiting" both read correctly |

`Running` was approved earlier as the marker for the entry currently executing and
has been **withdrawn**. It translates poorly: in several languages the obvious
translation carries the everyday "moving on foot" sense rather than the computing
one. The running entry is described by the activity panel's status row, which
already carries `In Progress`. Should a per-entry marker ever be added, it reuses
`In Progress` — no new string.

The per-entry remove control is **icon-only** (`list-remove`), with no label and
no tooltip, consistent with the icon buttons in `SearchTermList` and the search
toolbar. Device names are data, not translatable strings.

Any further string requires its own approval before it is written.

---

## Manual test charter

For each row: set up the stated condition, perform the action, confirm the result.

- **OPQ-F1 (update)** — Start a long update. While it runs, request an update on a second device. It is accepted and appears in the queue; nothing is silently dropped.
- **OPQ-F1 (create)** — While an update runs, create a catalog. It is accepted and queued rather than refused with *"A device operation is already running."*
- **OPQ-F2** — Queue three devices while one runs. They start in the order added, one at a time.
- **OPQ-F3** — Request an update for a device that is already waiting in the queue. The queue is unchanged and contains it once. Repeat for the device currently running.
- **OPQ-F4** — With two entries waiting, add a third. The first two are still present.
- **OPQ-F5** — With work queued, confirm the running entry is distinguishable from the waiting ones and the waiting count is correct.
- **OPQ-F6** — Remove the second of three waiting entries. The running operation is unaffected; the remaining two run in order. Clear the whole queue: the running operation still finishes.
- **OPQ-F7** — With entries waiting, stop the running operation. It stops, the queue is unchanged, and the next entry starts on its own. Repeat for a running *creation* with entries waiting behind it: the same applies.
- **OPQ-F7 (abandon everything)** — Clear the queue, then stop the running operation. Nothing remains and nothing restarts.
- **OPQ-F9** — Queue two devices and make the first fail (for example point it at a disconnected drive). The failure is reported and the second still runs.
- **OPQ-F10** — With per-operation reports enabled, queue two updates. The second starts only after the first report is acknowledged.
- **OPQ-C4** — Throughout all of the above, confirm two operations never run at once.
- **OPQ-C5** — Queue several entries and close the application. On restart the queue is empty and nothing resumes.
- **OPQ-C8** — Repeat OPQ-F2 in Memory mode and in File mode; queue behaviour is identical.
