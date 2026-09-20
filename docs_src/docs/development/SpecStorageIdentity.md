---
version: "2.13"
---

# STORAGE Identity

![Status](https://img.shields.io/badge/Status-Approved-brightgreen) ![Implementation](https://img.shields.io/badge/Implementation-planned-orange) ![Issue](https://img.shields.io/badge/Issue-%23809-blue) ![K2](https://img.shields.io/badge/K2-2.13-blue) ![K3](https://img.shields.io/badge/K3-3.0-blue)

## Impact on existing collections

Read this first.

- **Schema.** One new nullable column, `storage.storage_user_id` (NUMERIC). No
  table is re-keyed, no primary key changes, no row is rewritten beyond the
  back-fill. Added by an unconditional column guard, not by a versioned
  migration — see `STI-C4`.
- **On upgrade.** Every existing storage row receives `storage_user_id` = its
  current `storage_id`, so **every device shows the same number after the
  upgrade as before it**. Nothing the user sees changes on the day of the
  upgrade.
- **Memory mode, one-way loss.** The field is appended as the 18th column of
  `storage.csv`. A released **2.12** binary opening such a collection reads
  fields 0-16 and ignores the extra one, so it opens correctly — but **the first
  time 2.12 saves the storage table it rewrites the file with 17 columns and the
  user numbers are gone**, silently and with no way back. Beta testers who move
  a Memory-mode collection between 2.12 and 2.13/3.0 must be told. File and
  Hosted collections are unaffected in both directions, because every `SELECT`
  names its columns explicitly and an unknown extra column is simply left alone.
- **Collections already damaged by the import defect are not repaired by this
  page.** This page stops new damage only. Repair of existing damage is issue
  #809, specified in `SpecQualityCheck.md` — see `STI-C10`.

---

## Context

A Storage device's "ID" is **a number the user physically writes on the hard
disk** so they can recognise it on a shelf. It is user data.

It is also, today, the primary key of the `storage` table, the value
`device.device_external_id` joins on, and a number Collection Import
renumbers at will. One field is doing two incompatible jobs, and the collision
between them has already destroyed data in a real collection.

This page does two things, in this order:

1. **Part A — stop the import defect.** Collection Import renumbers a storage
   row without repointing the device that describes it, and in some cases never
   imports the storage row at all. The result is a Storage device that shows
   another disk's brand, model and serial number, or none at all.
2. **Part B — split the two concepts.** `storage.storage_id` stays as the
   internal key. A new `storage.storage_user_id` holds the number written on the
   disk. The edit form and the device tables show the user number; nothing joins
   on it.

> **Reading the requirement IDs.** Each requirement has a permanent ID.
> IDs are never renumbered or reused; a retired requirement is marked
> `[Removed]`, not deleted. Status is one of
> `[Implemented] / [Planned] / [Backlog] / [Removed]`.

---

## Scope at a glance

| | |
|---|---|
| **In scope** | The storage import path in `CollectionImporter`; `storage_user_id NUMERIC` and its column guard; the "Storage ID" field in the K2 and K3 device edit forms; the "Storage ID" column in the K2 storage tree and the K3 Devices storage table; duplicate-number handling; `Storage::generateID()`; the Memory-mode `storage.csv` round trip; the storage-picture filename rule |
| **Out of scope** | Repairing collections already damaged (that is `SpecQualityCheck.md`, issue #809); re-keying `storage`; changing `catalog.catalog_storage` from a name link to an id link; the dormant `statistics_storage` table; any new Storage field beyond the user number; `qt_quick/database.h`, a stale unused copy of the schema that MUST NOT be edited by this work |
| **Applies to** | K2 2.13 and K3 3.0 — both UIs edit this field, so unlike most 2.13 work this is **not** K3-only |

---

## Part A — what the import defect actually is

Three distinct broken outcomes, from one root cause.

`CollectionImporter::buildIdOffsets()` computes `m_storageIdOffset` as
`MAX(storage_id) + 1` in the target, and `importStorageForCatalog()` inserts the
storage row at `srcStorageId + m_storageIdOffset` — the row is **renumbered**.
But `remapAndInsertDevice()` applies an offset to `device_external_id` **only**
when the device type is `Catalog`. For a Storage device the source value is
carried over untouched.

`importStorageForCatalog()` is called from exactly one place: inside the
`Catalog` branch of `importSubTree()`. It resolves the storage by the catalog's
`catalog_storage` **name**, and returns early if a storage of that name already
exists in the target. There is **no storage-import path for a Storage-type
device**.

So, after importing into a non-empty collection:

1. **Storage device pointing at the wrong disk.** Its catalogs pulled the
   storage row in, the row landed at a new id, the device still points at the
   source id — which in the target belongs to a different disk. The device
   silently displays that disk's brand, model, serial number and label.
2. **Storage device pointing at nothing.** The device has no catalogs beneath
   it, so no storage row was ever created for it.
3. **Storage device with a row that exists but is unreachable.** The target
   already had a storage of that name, so the import returned early — and
   nothing ever repointed the imported device at the existing row either.

**The delete hazard.** `Device::deleteDevice()` repeats the same
`storage->ID = externalID; storage->deleteStorage();` pattern that
`Device::loadDevice()` uses. Deleting a mispointed imported Storage device
therefore **deletes a different disk's storage row**. This is what makes the
defect data-destroying rather than cosmetic, and it is why `STI-C2` is stated as
an absolute.

**The name-collision trap that comes with the fix.** `catalog.catalog_storage`
links a catalog to a storage **by name**, and `remapAndInsertCatalog()` copies
that name verbatim. If the fix disambiguates a colliding storage name — the
decision recorded in `STI-F3` — then every imported catalog still carries the
*old* name and silently attaches itself to the target's pre-existing disk. That
would trade one mismatch for another, which is why `STI-F4` exists and is not
optional.

---

## Part B — what the split changes, and what it deliberately does not

`storage.storage_id` is untouched: still NUMERIC, still the primary key, still
the target of `device.device_external_id`. It is not migrated, not re-keyed and
never shown to the user again.

`storage.storage_user_id` is new and holds the number written on the disk. It is
what the edit form's "Storage ID" field reads and writes, and what the
"Storage ID" column displays, in both UIs.

**The user number stays NUMERIC.** An earlier draft proposed TEXT, so that a
disk labelled `A12` could be recorded. That was reversed once it was established
that the "Storage ID" column is a real, numerically sorted column in both UIs —
`qt_quick/adapters/devicetablemodel.cpp` declares it `Kind::Number`, which sorts
on `toLongLong()` and right-aligns, and the K2 storage tree reads it with
`toInt()`. Keeping it numeric preserves that sorting and alignment unchanged.
**Accepted trade-off, stated by the user: a disk labelled `A12` still cannot be
recorded.** See `STI-C9`.

### The reference footprint of `storage.storage_id`

Established by reading the code, and recorded here because the split depends on
it being complete:

- **`device.device_external_id` — the only live reference.** Consumed by
  `Device::loadDevice()`, `Device::deleteDevice()`,
  `Device::assignStorageToDevice()`, `Storage::updateStorageInfo()`, and the K2
  storage tree's `JOIN storage s ON d.device_external_id = s.storage_id`.
- **`catalog.catalog_storage` joins by name, not by id.** It is TEXT. This is
  why `STI-F4` is needed.
- **`statistics_storage.storage_id` is dormant.** The table exists in the schema
  but is **never populated**: the only statements anywhere are two
  `DELETE FROM statistics_storage` and one `UPDATE … SET storage_name`. There is
  no `INSERT`, and `statisticsStorageFilePath` is assigned and never read.
  Storage history actually lives in `statistics_device`, keyed by `device_id`.
  **Statistics history is therefore unaffected by this page.**
- **`virtual_storage.virtual_storage_id`** is an unrelated legacy table.
- **The storage picture filename** is built from the storage id
  (`imageFolderPath + "/" + <id> + ".jpg"`). It keeps following the internal key
  — see `STI-C8`.

### The shadow columns on `storage`

Discovered while specifying `STI-C6`, and recorded because it changes how urgent
`STI-F11` is:

- **`storage_location` is dead.** `Storage` has no `location` member and
  `Storage::loadStorage()` does not select it. It is written by the Memory-mode
  CSV loader and by Collection Import, and read by nothing. Blanking it loses
  nothing a user can see today.
- **`storage_total_space` and `storage_free_space` are shadow copies.**
  `Storage::loadStorage()` does read them, but `Device::loadDevice()`
  immediately overwrites them from the **device** row — exactly as it does for
  `storage_path`. The authoritative runtime values are `device_total_space` and
  `device_free_space`. The stale storage-row copies still matter in three
  places: they are written to `storage.csv` in Memory mode, they are carried
  into other collections by Collection Import, and
  `Device::assignStorageToDevice()` seeds a new device row from them.

So the K2 blanking defect is **low severity, not zero**: it corrupts a row that
is copied across collections and persisted to disk, but nothing a user reads in
normal use comes from it. It is fixed here only because `STI-C6` rewrites that
exact statement anyway.

---

## Operational requirements — *why / for whom*

Goals in real use, independent of how they are built.

| ID | Requirement | Status |
|----|-------------|--------|
| STI-O1 | The number a user writes on a physical disk is their own data. Katalog displays it back unchanged and never alters it on its own initiative. | [Planned] |
| STI-O2 | After importing one collection into another, every storage device still describes the same physical disk — brand, model, serial number, label, and the number written on it. | [Planned] |
| STI-O3 | A user may give two disks the same written number without Katalog refusing to save. Katalog points out the duplicate and lets the user decide what to do about it. | [Planned] |

## Functional requirements — *what the system does*

Observable behaviour that can be triggered and watched.

### Part A — the import fix

| ID | Requirement | Status |
|----|-------------|--------|
| STI-F1 | Importing a device leaves every imported Storage device pointing at the storage row that describes it: when the import creates a storage row at a new internal id, the imported device's `device_external_id` is set to that new id **in the same operation**. | [Planned] |
| STI-F2 | Importing a Storage device imports the storage row that describes it, whether or not any catalog exists beneath it and whether or not any catalog references that storage by name. | [Planned] |
| STI-F3 | When the target already holds a storage row with the same `storage_name`, the import creates a **second** storage row with a disambiguated name, using the existing `CollectionImporter::resolveNameConflict()` (which yields `mystorage (2)`), and points the imported device at that new row. It MUST NOT point the imported device at the target's pre-existing row. | [Planned] |
| STI-F4 | When `STI-F3` renames an imported storage, every catalog imported in the same operation that referenced the old name has its `catalog.catalog_storage` updated to the new name, in the same operation. Without this the imported catalogs silently attach to the target's pre-existing disk. | [Planned] |

### Part B — the identity split

| ID | Requirement | Status |
|----|-------------|--------|
| STI-F5 | A storage carries a user-facing number stored in `storage.storage_user_id`, NUMERIC. The "Storage ID" field of the device edit form reads and writes **this field only**, in both K2 and K3. Zero means "no number written on this disk" and is a normal, valid state; it is the value a non-numeric entry falls back to. | [Planned] |
| STI-F6 | Saving a storage whose user number is already used by another storage in the same collection shows a **warning** and **completes the save**. A duplicate user number is never a reason to refuse a save. Zero is exempt: several unnumbered disks do not warn. | [Planned] |
| STI-F7 | A newly created storage has `storage_user_id` stamped from the internal `storage_id` assigned to it at creation, and `Storage::generateID()` appends **that user number** to the storage name. The user may then edit the number freely. | [Planned] |
| STI-F8 | Collection Import copies `storage_user_id` verbatim. Update from an external collection does **not** overwrite the target's `storage_user_id`. | [Planned] |
| STI-F9 | On upgrade, every existing storage row receives `storage_user_id` = its current `storage_id`, so every device shows the same number after the upgrade as before it. | [Planned] |
| STI-F10 | The "Storage ID" column in the K2 storage tree and in the K3 Devices storage table displays `storage_user_id`, and keeps the numeric sorting and right alignment it has today. | [Planned] |
| STI-F11 | Saving a storage from the device edit form preserves `storage_location`, `storage_total_space` and `storage_free_space`. Today the K2 statement names all three as bind placeholders and binds none of them, so every K2 storage save writes them NULL. | [Planned] |

## Constructional requirements — *how it is built / limits / MUST-NOTs*

Boundaries and implementation constraints, not user-visible behaviour.

| ID | Requirement | Status |
|----|-------------|--------|
| STI-C1 | `storage.storage_id` remains the internal primary key. It MUST NOT be presented to the user as an editable value, and no UI may write it. | [Planned] |
| STI-C2 | For a Storage device, `device.device_external_id` MUST always equal an existing `storage.storage_id` in the same collection. No operation may leave it dangling. This is stated as an absolute because `Device::deleteDevice()` deletes the storage row that `device_external_id` names: a dangling value is not a display defect, it is a deletion pointed at the wrong row. | [Planned] |
| STI-C3 | Collection Import MUST NOT alter `storage_user_id` in any way — no offset, no suffix, no auto-disambiguation, no renumbering. Silently editing the number written on a physical disk is the failure this whole page exists to prevent. Collisions are surfaced by `STI-F6` and never resolved silently. `STI-F3` disambiguates the storage **name**, never the number. | [Planned] |
| STI-C4 | Schema: `storage_user_id` is added to the `storage` CREATE TABLE in `Database` and, for existing databases, by an **unconditional column guard** rather than by a step inside `runMigration_2_13` — the field arrives after databases were already stamped 2.13, so the versioned migration no longer runs for them and the column would never appear. Same shape and same reasoning as `DCM-C4`; the general rule is recorded in `SpecDeviceComment.md`. | [Planned] |
| STI-C5 | In Memory mode the field is appended as the **last** column of `storage.csv` (the 18th), and the hand-written header line gains its label in the same change — the writer iterates the query record generically, but the header does not. The reader MUST bounds-check, because files written before this change have 17 fields and the reader indexes positions 0-16 today with no check at all. | [Planned] |
| STI-C6 | The device edit form MUST NOT write `storage.storage_id` or `device.device_external_id`. The existing ID-change branches in `qt_widgets/mainwindow_tab_device_pr.cpp` and in `AppManager::saveStorageDetails()` are **removed, not repointed**: leaving them standing while the field is redirected would make them write `storage_id = 0` on every save, which is strictly worse than today. | [Planned] |
| STI-C7 | `Device::verifyStorageExternalIDExists()` is retired for this purpose. The duplicate test of `STI-F6` reads `storage.storage_user_id`. Uniqueness of the internal key is enforced by the primary key alone and needs no application check. | [Planned] |
| STI-C8 | The storage picture filename stays keyed on the internal `storage_id`, never on `storage_user_id`. A user number is editable, and keying an image file on an editable value orphans the image the first time it changes. | [Planned] |
| STI-C9 | `storage_user_id` is NUMERIC. A non-numeric marking such as `A12` **cannot** be recorded; it falls back to zero. This is an accepted trade-off, taken deliberately to keep the numeric sorting and right alignment of the existing "Storage ID" columns in both UIs. Changing it to TEXT requires a new requirement **and** a decision about how those columns then sort. | [Planned] |
| STI-C10 | This page's scope is **stopping new damage**. It MUST NOT add an automatic repair pass over existing collections, on open, on import or otherwise. Detecting and repairing collections already damaged is `SpecQualityCheck.md` (issue #809), which is user-triggered by `QCK-C2`. | [Planned] |
| STI-C11 | `storage_path`, `storage_total_space` and `storage_free_space` are shadow copies of the authoritative `device_path`, `device_total_space` and `device_free_space`. They MUST NOT be made authoritative, and no new read path may prefer them. `STI-F11` keeps them correct because they are persisted and copied across collections, not because anything displays them. | [Planned] |
| STI-C12 | `qt_quick/database.h` is a stale, unused copy of the schema — it is listed in `qt_quick/CMakeLists.txt` but never included, since `appmanager.cpp` includes `core/database.h`. It MUST NOT be edited by this work. Removing it is a separate decision. | [Planned] |

---

## User-visible text

**No wording is approved and none is proposed here.** Every string below needs
its own per-string approval before it is written.

1. **Duplicate user-number warning** — one string, shared by K2 and K3. It
   replaces three existing blocking strings, which are removed with the blocking
   behaviour they belong to:
   - K2: `There is already a Storage with this ID.<b>` +
     `Choose a different ID and try again.`
   - K3: `There is already a Storage with this ID. Choose a different ID.`

   The replacement must read as a warning about a save that **has happened**,
   not as a refusal. No "Please", no imperative, sentence case. Per the project's
   message-presentation rule, K3 renders it as a `Kirigami.InlineMessage` with
   `MessageType.Warning` at the top of the form — not a dialog, not a passive
   notification.

2. **Import collision notice** — `STI-F6` is also how an import surfaces a
   duplicate number. If it can reuse string 1 unchanged, no second translation
   slot is spent; that is the preferred outcome. A separate string is needed
   only if the notice must name the colliding storage or collection.

3. **The `Storage ID` label itself — no change, deliberately.** It is already
   translated across 30 languages in three places (the K2 form, the K3 form and
   the K3 table header). From the user's point of view its meaning has not
   changed: it is still the number on their disk. Only the column behind it has.

---

## Manual test charter

For each row: set up the stated condition, perform the action, confirm the result.

- **STI-F1** — Import a collection containing a Storage device with catalogs into a **non-empty** target. Open the imported Storage device: its brand, model, serial number and label are its own, not another disk's.
- **STI-F2** — Import a Storage device that has **no catalogs beneath it** into a non-empty target. It arrives with a storage row of its own and its details are intact.
- **STI-F3** — Import a Storage device whose name already exists in the target. A second storage row is created, named `<name> (2)`, and the imported device points at it. The target's original storage row is unchanged.
- **STI-F4** — Repeat the `STI-F3` case with catalogs beneath the imported storage. Every imported catalog reports the **new** storage name, not the target's pre-existing one.
- **STI-F1 / delete hazard** — After the `STI-F1` import, delete the imported Storage device. No other storage device in the collection loses its details.
- **STI-F5** — Edit a Storage device, change the Storage ID, save, reopen the form: the new number is shown. Confirm no other device's details changed.
- **STI-F5 (zero)** — Set the Storage ID to 0 and save. It saves without complaint and the field reads 0 on reopen.
- **STI-F6** — Give two storage devices the same number. A warning appears and **both saves succeed**. Set a third to 0 while another is already 0: no warning.
- **STI-F7** — Create a new Storage device. Its Storage ID field is pre-filled and its name carries the same number as a suffix.
- **STI-F8 (import)** — Import a collection whose storages are numbered 3, 7 and 12 into a target already holding storages 1 and 2. The imported devices still read 3, 7 and 12 — never 4, 8, 13, and never `3 (2)`.
- **STI-F8 (update)** — Change a storage's number in the target, then run Update from the source collection. The target's number is **not** overwritten.
- **STI-F9** — Open a collection created before this change. Every storage device shows exactly the number it showed before the upgrade.
- **STI-F10** — Sort the Devices storage table by Storage ID. It sorts numerically (10 after 9, not before it) and the column is right-aligned.
- **STI-F11** — In Memory mode, note a storage's recorded total and free space, save the device from the edit form without changing them, then reopen the collection. The values are still there and `storage.csv` does not contain empty fields in their place.
- **STI-C4 (already stamped)** — Take a File-mode collection **already stamped schema 2.13** from earlier in the cycle, without the new column. Open it. The guard adds `storage_user_id`, the back-fill runs, and every storage device lists as before.
- **STI-C4 (from 2.12)** — Open a 2.12 collection in File mode. The column is added, the numbers are preserved, and the schema version reads 2.13.
- **STI-C5 (old file)** — Open a Memory-mode collection whose `storage.csv` has 17 columns. It loads without error and the numbers come from the back-fill.
- **STI-C5 (round trip)** — Save that collection, confirm `storage.csv` now has 18 columns and a matching header, then reopen it. The numbers survive.
- **STI-C5 (2.12 regression)** — Open the 18-column collection with a released **2.12** binary. It opens correctly. Save it there, reopen in 2.13: the user numbers are gone and the back-fill has restored them from the internal ids. This is the documented one-way loss, not a bug to fix.
- **STI-C6** — Save a Storage device from the edit form in K2 and in K3 without changing anything. `storage_id` and `device_external_id` are unchanged in both cases, and the device still shows its own details.
- **STI-C8** — Change a storage's user number on a device that has a picture. The picture is still displayed afterwards.

---

## Related

- `SpecQualityCheck.md` — detecting and repairing collections **already** damaged by the import defect (issue #809). This page stops new damage; that one cleans up the old.
- `SpecCollection.md` — the Import / Update design note. Its "ID remapping" section documents `device_id_offset` and `catalog_id_offset` only; `storage_id_offset` appears in no document, which is how the defect stayed invisible.
- `SpecDeviceComment.md` — `DCM-C4`, the column-guard rule this page reuses.
- `SpecDeviceStorageRoot.md` — `DSR-C8`, the neighbouring rule about `storage_path` following the save.
