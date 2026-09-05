# OPERATION Queue

![Status](https://img.shields.io/badge/Status-Specified-blue) ![Implementation](https://img.shields.io/badge/K3-in%20progress-orange) ![K2](https://img.shields.io/badge/K2-no%20change-lightgrey)

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
| **In scope** | Accepting new create/update requests while one runs; a visible queue; removing entries; the activity panel's visibility timing (when it appears, how long it stays); refusing a search while a create or update runs and refusing a create or update while a search runs; reporting search progress in the activity panel; K3, plus the cancellation *timing* in `core/DeviceUpdateManager` where it makes OPQ-C4 unattainable |
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
| OPQ-O5 | A user who runs a very short operation still sees that it ran and how it ended: the activity panel is never a flash too brief to be noticed, and the outcome stays readable before it disappears. | [Planned] |
| OPQ-O6 | A user looks in **one** place to see whether a catalog creation, a device update or a search is running and how far it has got. The progress of these three does not move around with whichever page happens to be open, and leaving a page never hides work that is still running. | [Planned] |

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
| OPQ-F11 | The activity panel, including its running indicator, is displayed and painted **before** the requested operation begins its work, so that an operation lasting a fraction of a second is still visibly acknowledged. | [Planned] |
| OPQ-F12 | When an operation ends — completed, stopped, cancelled or failed — the activity panel stays visible for at least the linger duration (OPQ-C14) showing that operation's final status message, and only then hides if nothing is running and nothing is queued. The message is the one the progress manager already builds for that outcome; the linger is what makes it readable. Applies to every operation the panel reports. | [Planned] |
| OPQ-F13 | A search cannot start while a catalog creation or a device update is running. Every route into a search — the Search action and the Enter key in the search field — refuses, so a search never begins nested inside a running operation's call stack. The refusal is visible: the Search action is shown disabled rather than accepting a dead click. | [Implemented] |
| OPQ-F14 | A catalog creation or a device update cannot start while a search is running, for the same reason and by the same means. A device update **refuses** here rather than queuing, unlike OPQ-F1 — see the note below; that difference is deliberate and MUST NOT be "corrected" back to queuing. | [Implemented] |
| OPQ-F15 | The activity panel reports search progress: it is shown while a search runs, it displays the search's standard status message, and its Stop control stops the running search. The Search page and the Search Results page carry no progress row of their own — the two page footers that carry one today are removed. The search's own five-second message-clearing timer is removed with them: the panel's linger (OPQ-F12) under its single constant (OPQ-C14) is the only one, and two independent timers MUST NOT race on the same message. | [Implemented] |

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
| OPQ-C12 | The busy indicator turns only while work is actually running. During the linger of OPQ-F12 it MUST NOT turn: a turning indicator never sits beside a *Completed* line. It MUST NOT turn while an operation is paused either — a paused search is not work in progress — even though the operation's running flag is still set. | [Implemented] |
| OPQ-C13 | The flag that marks an operation as running MUST be set synchronously at the moment the operation is *requested*, even when the start of the work is deferred to a later event-loop turn to let the panel paint (OPQ-F11). Deferring the start MUST NOT open a window in which a second operation can be admitted (OPQ-C4), and MUST NOT move work off the main thread (OPQ-C3). | [Planned] |
| OPQ-C14 | The linger duration of OPQ-F12 is a **single named constant**, defined in one place and applied to every operation type. It MUST NOT be duplicated at each call site or varied per operation. | [Planned] |
| OPQ-C15 | The one-operation-at-a-time rule of OPQ-C4 covers searches as well as creations and updates. The guard MUST be evaluated at the moment an operation is *requested*, at the single entry point for each operation type, and MUST NOT rely on the UI being unresponsive: operations pump the event loop on the main thread (OPQ-C3), so every control stays clickable while one runs. A guard in the UI layer alone is not sufficient — it MUST also exist where the work would be done. | [Implemented] |
| OPQ-C16 | Catalog creation, device update and search MUST report progress through the activity panel and nowhere else: none of these three may own a progress indicator or a status row on a page. Checkable as: outside `OperationQueueView.qml`, nothing is bound to the running flag or the status text of a creation, an update or a search. This rule reaches those three operations only — see the note below. Transient outcome notices and blocking validation messages are not progress and are unaffected. | [Implemented] |

### Note on placement (OPQ-C6)

`CLAUDE.md` records that `AppManager` already carries more orchestration than it
should, and that this belongs in a future core manager. Extending the queue there
adds to that debt knowingly: the alternative — moving the runner into `core/`
first — is a larger change that would block a small, high-value improvement.
When the orchestration move happens, the queue moves with it.

### Note on the linger duration (OPQ-C14)

The chosen value is **5000 ms**. It was picked to match the delay K2 uses when it
clears a completion message from the status bar — but that K2 delay is *implemented
behaviour recorded in `SpecProgressReport.md`'s message-sources inventory, not a
ratified requirement*. `SpecProgressReport.md` holds no requirement table and no
requirement IDs, so 5000 ms has no spec ancestry: it is a free design choice, made
here for familiarity, and it can be changed to any other value without contradicting
anything. That is precisely why OPQ-C14 requires it to live in one named constant —
changing the choice must stay a one-line edit.

### Note on the reach of OPQ-C16

OPQ-C16 covers catalog creation, device updates and search, and nothing else. Two
other K3 screens report progress outside the activity panel and sit deliberately
outside this rule:

- **Backup preparation** — the pre-preview catalog update and the preview compare
  report on the Backup page's own status area. That is what `SpecBackup.md`
  BKP-F14 and BKP-F16 require.
- **The VVV import** on the Settings page, which has its own progress row.

Neither is a violation of OPQ-C16. Whether the panel should eventually be the
single place for those as well is an open question that nothing here settles.

### Note on the backup sequencer (OPQ-C1)

`SpecBackup.md` `BKP-F18`–`BKP-F21` add a K3 control that runs the currently
listed backup/archive links one after another. That is a **second, separate
sequential runner**, and it is not a violation of OPQ-C1: OPQ-C1 forbids a
parallel mechanism *for the create/update queue*, and a backup run is neither a
creation nor an update. It also runs on its own `QThread`, so OPQ-C3
(main-thread execution) and OPQ-C4 (one operation at a time) do not reach it
either — those govern creations, updates and, via OPQ-C15, searches.

Two contact points remain and are stated in `BKP-C11`: the per-link catalog
update that a backup may perform first **is** a device update and stays under
OPQ-C4; and backup progress stays on the Backup page, already carved out by the
note on the reach of OPQ-C16 above.

`BKP-F20` deliberately departs from OPQ-F7: stopping a multi-link backup run
abandons the remaining links, where stopping a queued create/update leaves its
queue intact. The difference is the visibility of the queue — the operation
queue is shown (OPQ-F5), the backup pending list is not.

### Note on refusing rather than queuing (OPQ-F13, OPQ-F14)

Both guards *refuse*. Queuing the refused work instead — so that it runs once the
operation ahead of it finishes — is a separate decision that remains open, and the
refusal is not a considered rejection of queuing.

OPQ-F14 makes a device update refuse while a search runs, even though OPQ-F1 has
that same request *queue* while another create or update runs. The difference is
not an inconsistency to tidy away: **nothing drains the queue when a search ends**,
so an entry queued behind a search would sit there until the user happened to
start another catalog operation. Refusing is visible; a stranded entry is not.
Restoring the queuing behaviour without first giving the search's completion a
path into `startNextDeviceUpdate()` would reintroduce exactly the silent-drop
problem this spec was written to remove (OPQ-O2).

### Note on the string used by OPQ-F14

The refusal from catalog creation reuses the existing string
`A device operation is already running.` rather than adding a new one. A search is
not literally a device operation, so the message is inaccurate; it was reused to
avoid spending a translation slot across all languages on a path that may yet be
replaced by queuing. The maintainer is aware and may substitute an accurate new
string later — that substitution needs its own per-string approval.

---

## User-visible text

Reused from existing K3 QML — no new translation slots:

| String | Use |
|--------|-----|
| `Create` / `Update` | the operation type shown on each entry |
| `Clear` | empties the queue; the running entry is unaffected (OPQ-F6) |
| `Stop` | stops the running operation; the queue is left intact (OPQ-F7). Stops the running search when a search is what the panel reports (OPQ-F15) |
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

**Verification status (2026-08-31):** the rows marked `[Implemented]` for the
guards and for search-in-the-panel — OPQ-F13, F14, F15, C12, C15, C16 — were
verified by reading the source, not by running the charter below. None of their
charter lines has been executed yet.

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
- **OPQ-F11** — Create a catalog on a device holding only a handful of files. The activity panel and its turning indicator appear and are actually seen before the scan runs; the operation is never a panel that flashes without painting. Repeat with an update on the same device.
- **OPQ-F12 (completed)** — Repeat the very short creation. When it finishes, the panel stays on screen long enough to read `CREATE | Completed | Catalog 1 of 1 | <name> | Indexed: N of N (100%)`, then hides.
- **OPQ-F12 (stopped / failed)** — Stop a running operation, and separately let one fail (point a device at a disconnected drive). In each case the panel lingers with that operation's final message before hiding.
- **OPQ-F12 (update path)** — Repeat the completed and stopped cases on a device update, not a creation. The panel behaves identically.
- **OPQ-F13** — Start a long device update. While it runs, open Search, press the Search action, then press Enter in the search field. Neither starts a search and the Search action is visibly disabled. Repeat during a catalog creation.
- **OPQ-F14** — Start a long search. While it runs, request a device update and, separately, a catalog creation. Neither starts.
- **OPQ-F15** — Run a search. Progress and the completion line appear in the activity panel, once. No progress row appears on the Search page or on the Search Results page. Stop from the panel stops the search.
- **OPQ-F12 (search path)** — Let a search complete, then run one and stop it. In each case the panel lingers with that search's final `SEARCH | ...` message before hiding, exactly as for a creation.
- **OPQ-C12 (paused)** — Pause a running search. The indicator stops turning and the message stays readable. Resume: it turns again.
- **OPQ-C16** — Search the K3 QML for progress indicators and status rows bound to a creation, an update or a search: they exist only in `OperationQueueView.qml`.
- **OPQ-C12** — During the linger, watch the indicator: it is stationary. It turns only while the operation is running.
- **OPQ-C13** — Request a creation and immediately request a second operation before anything appears to have started. Only one is admitted; the other is queued or refused, and the two never run at once.
- **OPQ-C14** — Time the linger after a creation and after a device update: they are the same length.
- **OPQ-C4** — Throughout all of the above, confirm two operations never run at once.
- **OPQ-C5** — Queue several entries and close the application. On restart the queue is empty and nothing resumes.
- **OPQ-C8** — Repeat OPQ-F2 in Memory mode and in File mode; queue behaviour is identical.
