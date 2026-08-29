# DEVICE Comment

![Status](https://img.shields.io/badge/Status-Approved-brightgreen) ![Implementation](https://img.shields.io/badge/Implementation-complete-brightgreen) ![K3](https://img.shields.io/badge/K3-3.0-blue) ![K2](https://img.shields.io/badge/K2-core%20only-lightgrey)

## Context

A device carried no free-text note. A user who keeps several similar
devices — two external drives of the same brand, several catalogs of the same
folder tree taken at different dates — has only the device name to tell them
apart, and the name is also what every list, tree and report displays. Anything
longer than a name has nowhere to go.

This spec adds **one optional free-text comment per device**, stored in the
`device` table, editable in the K3 device edit form and shown beside the device
name on the K3 Devices page card.

Two existing things it must not be confused with:

- **Storage comments.** The `storage` table already has `storage_comment1/2/3`,
  which describe a *physical storage*. The new comment is a *device-level* note
  that exists for every device type. Both are kept — see `DCM-F5`.
- **Search.** The comment is an annotation for the user's eyes. It is not a
  search field and no logic reads it — see `DCM-C7`.

> **Reading the requirement IDs.** Each requirement has a permanent ID.
> IDs are never renumbered or reused; a retired requirement is marked
> `[Removed]`, not deleted. Status is one of
> `[Implemented] / [Planned] / [Backlog] / [Removed]`.

---

## Scope at a glance

| | |
|---|---|
| **In scope** | `device_comment TEXT` column; K3 edit-form field; K3 Devices page card display; Memory-mode device-file round trip; escaping of free-text device columns in the Memory-mode device file |
| **Out of scope** | Any K2 UI for the field; searching, filtering or sorting on the comment; migrating or merging `storage_comment1/2/3`; multi-line rich text; comments on anything other than a device |
| **Applies to** | K3 3.0; `core/` shared, so K2 inherits the column and the file-format change without a UI |

---

## Operational requirements — *why / for whom*

Goals in real use, independent of how they are built.

| ID | Requirement | Status |
|----|-------------|--------|
| DCM-O1 | A user records a free-text note about a device — what is on it, why it is kept, where it physically lives — without having to encode it into the device name. | [Implemented] |
| DCM-O2 | A user can do this for **any** device, whatever its type, so the note is not a privilege of one kind of device. | [Implemented] |
| DCM-O3 | A user tells similar devices apart at a glance in the device list, without opening each one's edit form. | [Implemented] |

## Functional requirements — *what the system does*

Observable behaviour that can be triggered and watched.

| ID | Requirement | Status |
|----|-------------|--------|
| DCM-F1 | Every device has one optional free-text comment, persisted in `device.device_comment`. It is empty by default and an empty comment is a normal, valid state. | [Implemented] |
| DCM-F2 | The comment is editable on the K3 device edit form, in a field placed **immediately after `Parent device`**. Saving the form stores it; reopening the form shows the stored value. | [Implemented] |
| DCM-F3 | On the K3 **Devices page** card the comment is displayed **beside the device name** when it is non-empty, at the **same font size as the detail line below the name** — not at heading size. When the comment is empty nothing is displayed: no label, no placeholder, no reserved space. | [Implemented] |
| DCM-F4 | Exactly one new user-visible string is introduced: `Comment`, used as the edit-form field label. The Devices page card shows the comment **value only**, with no accompanying label. | [Implemented] |
| DCM-F5 | The device comment and the Storage comments are **independent and both retained**. `Comment 1`, `Comment 2`, `Comment 3` remain Storage-specific fields on the `storage` table; the new comment is the device-level note and is available on Storage devices too. A Storage edit form therefore presents **four** comment fields. Existing `storage_comment1/2/3` values are **not** migrated, copied or merged into `device_comment`. | [Implemented] |
| DCM-F6 | In Memory database mode the comment survives a save/load round trip through the device file, like every other device column. | [Implemented] |

## Constructional requirements — *how it is built / limits / MUST-NOTs*

Boundaries and implementation constraints, not user-visible behaviour.

| ID | Requirement | Status |
|----|-------------|--------|
| DCM-C1 | The Memory-mode device file is tab-separated and newline-terminated with no quoting, so **all free-text device columns MUST be escaped on write and unescaped on read**: `device_name`, `device_path` and `device_comment`. The existing `escapeHistoryField` / `unescapeHistoryField` helpers are reused; no new helper is introduced. This also closes existing drift — name and path are written raw today, so a tab or newline in either already corrupts the file and every record after it. | [Implemented] |
| DCM-C2 | `Device::saveDevice()` is a **full-row UPDATE from the in-memory object**: a code path that does not intend to edit the comment MUST NOT clear it. Every caller that saves a `Device` must have loaded the comment into the object first, and `device_comment` must be part of every load path that feeds a save. | [Implemented] |
| DCM-C3 | **The device file's header line is the format marker.** A header **without** the comment column identifies a legacy file, which MUST be read **raw, with no unescaping**; a header **with** the comment column identifies a new-format file, which MUST be read unescaped. Writing always produces the new format. This is what makes `DCM-C1` safe to deploy: legacy files were written raw, so a device literally named `Backup\test` contains the two characters `\t`, and unescaping it would turn them into a tab. (`unescapeHistoryField` passes unknown escapes through unchanged, so only literal `\\`, `\t`, `\n` and `\r` are at risk; paths are normalised to forward slashes via `QDir::fromNativeSeparators` and `toNativeSeparators` is never used, so device **names** are the real hazard, not paths.) | [Implemented] |
| DCM-C4 | Schema: `device_comment TEXT` is added to the `device` CREATE TABLE in `Database` and, for existing databases, by the unconditional column guard `Database::ensureDeviceCommentColumn()` rather than by a step inside `runMigration_2_13` — the field arrived after databases were already stamped 2.13, so the versioned migration no longer runs for them and the column would never appear. No new versioned migration step is added, because schema 2.13 has never been released; the column can still change in place until 2.13 ships. This is the **first structural schema change** of the 2.13/3.0 cycle — earlier 2.13 migration work only normalised data. | [Implemented] |
| DCM-C5 | K2 gets **no new UI** for this field. K2 is in maintenance mode; it inherits the column, the migration and the device-file format change through `core/` only. | [Implemented] |
| DCM-C6 | The comment is read and written through `core/` (`Device`, `Collection`, `Database`). Neither `qt_quick/` nor `qt_widgets/` contains SQL for it. | [Implemented] |
| DCM-C7 | The comment is annotation only. It MUST NOT be used as a search field, a filter, a sort key, or an input to any device, catalog or backup logic. Widening its use requires a new requirement. | [Implemented] |
| DCM-C8 | The comment is plain single-line free text. No length limit, no format validation, no markup rendering. | [Implemented] |
| DCM-C9 | `Collection::saveDeviceTableToFile()` MUST verify the device `SELECT` succeeded **before** opening the device file, because opening it `WriteOnly` truncates it. On a failed query it returns early and leaves the previous file untouched. Without this, a query naming a column the database does not have replaces every device with a header-only file — silent, total loss of the Memory-mode device list. | [Implemented] |

---

## Data path, as built

The Devices page reads `AppManager::getDeviceList()`, which returns a
`QVariantList` built from `Device::DeviceTreeNode`; the comment travels on that
node and is exposed as the `comment` key. This is **not** the Selection page's
path — Selection binds to `DeviceListModel`, which has no comment role, since
nothing displays the comment there. The two paths are independent, so a future
change to one does not carry the comment to the other by itself.

The edit form saves through `AppManager::saveDeviceBasicFields()`, which calls
`Device::loadDevice()` before `Device::saveDevice()` — this is what satisfies
`DCM-C2`.

---

## Why the column guard, not the versioned migration

This is worth reading before adding any further field to an unreleased schema
version.

The field was first added as a step inside `runMigration_2_13`, following the
"2.13 is unreleased, edit the migration in place" guidance. **It failed in
testing.** Collections had already been stamped schema 2.13 by earlier work in
the cycle, so the versioned migration never ran again for them, `device_comment`
never appeared, and every query naming the column failed — **devices vanished
from both the Devices and the Selection pages** in File mode.

The remedy is an unconditional column guard, `ensureDeviceCommentColumn()`,
which checks for the column on every open regardless of the recorded schema
version. This is exactly the trap `ensureMappingSourceCollectionColumn()` was
created for in the 2.11 cycle; recognising the pattern is the point of recording
it here.

The Memory-mode half of the same failure was the more dangerous one, and is now
guarded separately by `DCM-C9`: the device file is opened `WriteOnly`, which
truncates it, so a failing `SELECT` would have produced a header-only file and
destroyed the device list rather than merely hiding it.

**General rule, stated here as the local instance:** a field added to an
unreleased schema version **after any database has been stamped with that
version** must be added by an unconditional column guard, not by editing the
versioned migration in place. The "edit in place while the version is
unreleased" guidance is only safe *before* anything has been stamped. This is a
project-wide rule rather than a device-comment rule, and belongs in
`SpecVersions.md`; it is recorded here only because this feature is where it was
learned.

---

## Open question — cross-version collection compatibility

**Unresolved, deliberately not decided here.** It is not established whether the
project supports opening a 2.13 / 3.0 collection with a released **2.12** binary
at all; `SpecVersions.md` is silent on backward compatibility of a collection
with an older application.

The known consequence, recorded without ruling on it: in Memory mode, a device
saved by a 2.12 binary **drops its comment**, because 2.12's
`saveDeviceTableToFile` writes only the 14 columns it knows about. `DCM-C3`
protects the *reading* side against corruption but cannot preserve a value that
an older writer never wrote.

Deciding this belongs in `SpecVersions.md`, as a general policy for all schema
additions, not as a device-comment special case.

---

## Manual test charter

For each row: set up the stated condition, perform the action, confirm the result.

- **DCM-F1** — Create a device and save it without touching the comment. It saves normally and the comment is empty.
- **DCM-F2** — Open a device's edit form. The `Comment` field sits immediately after `Parent device`. Enter text, save, reopen: the text is there.
- **DCM-F3 (shown)** — Give a device a comment. On the Devices page the comment appears beside the device name, at the same size as the detail line below the name, not as a heading.
- **DCM-F3 (empty)** — Clear a device's comment. The Devices page card shows nothing in its place and the card layout does not change height.
- **DCM-F4** — Confirm the edit-form label reads exactly `Comment` and that the Devices page card shows no label, only the value.
- **DCM-F5** — On a Storage device, confirm four comment fields are present: `Comment 1`, `Comment 2`, `Comment 3` and the new `Comment`. Set the device comment; the three Storage comments are unchanged. Open a collection that already had Storage comments; they are untouched and the device comment is empty.
- **DCM-F6** — In Memory mode, set a comment, close and reopen the collection. The comment is still there.
- **DCM-C1** — In Memory mode, set a device **name** containing a tab and a comment containing a newline. Save, reopen. Both survive intact and every device after them in the file still loads.
- **DCM-C2** — Set a comment on a device, then run an operation that saves the device without editing the comment (update the device, change its parent, run a catalog update). The comment is still present afterwards.
- **DCM-C3 (legacy)** — Take a Memory-mode collection written by 2.12 that contains a device named with a literal backslash-t, e.g. `Backup\test`. Open it in 2.13. The name is unchanged — not turned into a tab. Save; the file is rewritten in the new format with a comment column.
- **DCM-C3 (new)** — Reopen the file saved by the previous step. Escaped values are unescaped correctly and nothing is double-escaped after a second save.
- **DCM-C4 (already stamped)** — Take a File-mode collection **already stamped schema 2.13** from earlier in the cycle, without the comment column. Open it. The column guard adds `device_comment`, and the Devices and Selection pages list every device as before.
- **DCM-C4 (from 2.12)** — Open a 2.12 collection in File mode. The column is added, no data is lost, and the schema version reads 2.13.
- **DCM-C5** — Confirm no comment field appears anywhere in the K2 UI.
- **DCM-C7** — Search for a word that exists only in a device comment. It is not matched; the comment plays no part in search, filter or sort.
- **DCM-C9** — In Memory mode, make the device `SELECT` fail (for instance by opening a collection whose device table lacks the column before the guard runs). The existing device file is left intact — not replaced by a header-only file — and the devices are still there after restarting.
