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
- **Two columns removed (`STI-C13`, `STI-C16`).** `storage.storage_name` and
  `catalog.catalog_storage` are dropped from the schema by the 2.12 → 3.0
  migration, on every File and Hosted collection, including beta collections
  already stamped 2.13 (see `SpecVersions.md`, schema 3.0 at release). No data a
  user sees is lost: the device name is the only name, and nothing in 3.0 read
  these columns. Memory-mode **file formats are unchanged**: the "Name" column
  of `storage.csv` and the `<catalogStorage>` line of each `.idx` header stay,
  written empty and never read.
- **`catalog.catalog_name` removed too (`STI-C17`, `STI-C18`).** Removed by the
  same migration (a `catalog` table rebuild on SQLite, because of its UNIQUE
  constraint); every catalog row and id is kept. The catalog's name is its
  device's name, and uniqueness among Catalog devices is enforced by the
  application instead.
- **Duplicated paths removed too (`STI-C19`).** `storage.storage_path`,
  `catalog.catalog_source_path` and the dead `catalog.catalog_source_path_is_active`
  are removed by the same migration. `device.device_path` is the only path of a
  Storage or Catalog device; nothing a user sees changes. Memory mode keeps its
  file slots: the "Path" column of `storage.csv` is written empty, and the
  `<catalogSourcePath>` line of each `.idx` header keeps holding the device path
  for readability — neither is read back.
- **No return to older K2.** After the migration, K2 2.12 and older, and the K2
  2.13 binary published with 3.0 beta2, name these columns in their SQL and fail
  on File and Hosted collections. Accepted by the user for a major version; see
  `SpecVersions.md`, "Compatibility with older versions", risk 5.
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
| **In scope** | The rule that the device name is the only name read (`STI-C13`) and the removal of `storage.storage_name`, `catalog.catalog_storage` (`STI-C16`) and `catalog.catalog_name` (`STI-C17`); Catalog device name uniqueness (`STI-C18`, `STI-F15`); the upkeep of the catalog's synced name copies on rename (`STI-F14`, `STI-C15`); the storage import path in `CollectionImporter`; `storage_user_id NUMERIC` and its column guard; the "Storage ID" field in the K2 and K3 device edit forms; the "Storage ID" column in the K2 storage tree and the K3 Devices storage table; duplicate-number handling; `Storage::generateID()`; the Memory-mode `storage.csv` round trip; the storage-picture filename rule |
| **Out of scope** | Repairing collections already damaged (that is `SpecQualityCheck.md`, issue #809); re-keying `storage`; removing `file.file_catalog` (a later topic, per the user); changing the Memory-mode file formats (`storage.csv`, `.idx` header); the dormant `statistics_storage` table; any new Storage field beyond the user number |
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

**The name-collision trap, and why the fix no longer goes through names.**
`catalog.catalog_storage` linked a catalog to a storage **by name**, and the
import used to resolve storage rows by that name. An earlier version of this
page answered a name clash by disambiguating the storage name and remapping
`catalog_storage` to match. Both name columns are now removed (`STI-C13`): the
import resolves a Storage device's storage row **by id**
(`device_external_id` → `storage_id`) and always creates a new row for it
(`STI-F3`), so a clash between two device names can no longer attach anything
to the wrong disk.

**Principle — only ids manage the device hierarchy.** In the user's words:
*"only IDs should be the reliable way to manage a device hierarchy."* Collection
Import resolves the device hierarchy (`device_parent_id`) and the storage and
catalog links (`device_external_id`) by internal ids only, never by names —
`STI-C14`.

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
- **`catalog.catalog_storage` was a name, not an id.** It is removed
  (`STI-C13`); a catalog's place in the hierarchy is found through the device
  tree, by id.
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
  immediately overwrites them from the **device** row — exactly as it did for
  the former `storage_path` (removed, `STI-C19`). The authoritative runtime values are `device_total_space` and
  `device_free_space`. The stale storage-row copies still matter in three
  places: they are written to `storage.csv` in Memory mode, they are carried
  into other collections by Collection Import, and
  `Device::assignStorageToDevice()` seeds a new device row from them.

So the K2 blanking defect is **low severity, not zero**: it corrupts a row that
is copied across collections and persisted to disk, but nothing a user reads in
normal use comes from it. It is fixed here only because `STI-C6` rewrites that
exact statement anyway.

---

## Device name — the only name that is read

A device's name lives in `device.device_name`. It is the **only** name. The
two old storage-name columns are removed; the catalog's remaining synced copies
are kept equal to it on rename — stated once, as `STI-C13`; other specs
reference it.

| Copy | Kind | Rule |
|---|---|---|
| `storage.storage_name` | **Removed** | Dropped by the 2.12 → 3.0 migration (`STI-C16`). `Storage::name` survives as an in-memory field only, filled from `device_name` by `Device::loadDevice()` and used only in messages. In Memory mode the "Name" column of `storage.csv` stays, written empty and never read. |
| `catalog.catalog_storage` | **Removed** | Dropped by the 2.12 → 3.0 migration (`STI-C16`). A catalog's place in the hierarchy is its device's parent id. In Memory mode the `<catalogStorage>` line of the `.idx` header stays, written empty and never read. |
| `catalog.catalog_name` | **Removed** | Removed by the 2.12 → 3.0 migration (`STI-C17`). The catalog's name is its device's `device_name`. Catalog-name uniqueness is kept, but as a rule on devices: no two Catalog devices share a name (`STI-C18`), because Memory mode stores each catalog as `<name>.idx`. |
| `file.file_catalog` | Synced copy, still in use | Kept equal to the Catalog device's name on rename (`STI-F14`). Its own removal is a later topic, out of scope here. |
| Memory-mode `.idx` file name and `catalog_file_path` | Synced copy, still in use | Renamed with the Catalog device (`STI-F14`). |

What older applications do on a collection without these columns is risk 5 in
`SpecVersions.md`, "Compatibility with older versions".

**Consequence for Collection Import (confirmed by the user).** Because
`catalog_storage` is not used (and is now removed), a storage row is imported only together with
the Storage device that describes it (`STI-F2`, `STI-F3`), never looked up from
a catalog's `catalog_storage`. Importing a catalog **without** its parent
Storage device therefore imports **no** storage row (`STI-F13`).

**Consequence for the Quality Check.** The checks that compared or grouped
storage names (former checks 2 and 5, `QCK-F3`, `QCK-F9`) are retired —
see `SpecQualityCheck.md`.

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
| STI-F3 | Collection Import resolves an imported Storage device's source storage row **by id** (the source device's `device_external_id` → source `storage_id`), never by `storage_name`, and always inserts it into the target as a **new** storage row keyed by a new internal id, even when the target already holds a storage of the same name. It MUST NOT point the imported device at a pre-existing target row. No name is written to, compared on or disambiguated in the storage row (`STI-C13`). | [Planned] |
| STI-F4 | Collection Import writes each imported catalog's `catalog.catalog_storage` from the name of the catalog's **direct parent device** in the target after import, whatever that device's type, for older applications (`STI-C13`). It does not read the source `catalog_storage`, and does not remap it by name. *Retired: the column is removed (`STI-C13`, `STI-C16`).* | [Removed] |

### Part B — the identity split

| ID | Requirement | Status |
|----|-------------|--------|
| STI-F5 | A storage carries a user-facing number stored in `storage.storage_user_id`, NUMERIC. The "Storage ID" field of the device edit form reads and writes **this field only**, in both K2 and K3. Zero means "no number written on this disk" and is a normal, valid state; it is the value a non-numeric entry falls back to. | [Planned] |
| STI-F6 | Saving a storage whose user number is already used by another storage in the same collection shows a **warning** and **completes the save**. A duplicate user number is never a reason to refuse a save. Zero is exempt: several unnumbered disks do not warn. | [Planned] |
| STI-F7 | A newly created storage gets `storage_user_id` = the highest `storage_user_id` in the collection + 1 (1 in a collection without any), independently of its internal `storage_id` (still the highest `storage_id` + 1). `Storage::generateID()` appends nothing to any name. In K2 and K3 the new Storage device is named `Storage_<that user number>`, built from the existing translated "Storage" string, never from the internal `device_id`. The user may then edit the number and the name freely. | [Planned] |
| STI-F8 | Collection Import copies `storage_user_id` verbatim. Update from an external collection does **not** overwrite the target's `storage_user_id`. | [Planned] |
| STI-F9 | On upgrade, every existing storage row receives `storage_user_id` = its current `storage_id`, so every device shows the same number after the upgrade as before it. | [Planned] |
| STI-F10 | The "Storage ID" column in the K2 storage tree and in the K3 Devices storage table displays `storage_user_id`, and keeps the numeric sorting and right alignment it has today. | [Planned] |
| STI-F11 | Saving a storage from the device edit form preserves `storage_location`, `storage_total_space` and `storage_free_space`. Today the K2 statement names all three as bind placeholders and binds none of them, so every K2 storage save writes them NULL. | [Planned] |
| STI-F12 | The legacy name copies of `STI-C13` are kept current, in K2 and K3: `storage.storage_name` is written when the storage is created and updated when the Storage device is renamed. `catalog.catalog_storage` holds the name of the catalog's direct parent device, whatever its type: it is written when the catalog is created; set to the new direct parent's name when the catalog is moved (empty when moved to the root); and updated for every Catalog device directly beneath a device of **any** type (Storage, Virtual, …) when that device is renamed. The children are found through the device tree, never by matching the old name in `catalog_storage`. K3 today writes it at creation only (`qt_quick/appmanager.cpp` ~2490); K2 updates it by name (`qt_widgets/mainwindow_tab_device_pr.cpp` ~796-880). *Retired: both columns are removed (`STI-C13`, `STI-C16`); nothing is kept current.* | [Removed] |
| STI-F13 | Importing a Catalog device without its parent Storage device imports **no** storage row: no storage row is looked up from the catalog's `catalog_storage` or created for it. | [Planned] |
| STI-F14 | Renaming a Catalog device updates `file.file_catalog` and, in Memory mode, the `.idx` file name (and `catalog_file_path`) in the same operation, in K2 and K3, in every database mode, so no file row or file keeps the old catalog name. | [Planned] |
| STI-F15 | When an imported Catalog device's name is already used by a Catalog device in the target, Collection Import renames the imported **device** (`resolveNameConflict()` → `<name> (2)`, `(3)`, …), so the device, its `.idx` file and its `file.file_catalog` values all carry that same new name. | [Planned] |

## Constructional requirements — *how it is built / limits / MUST-NOTs*

Boundaries and implementation constraints, not user-visible behaviour.

| ID | Requirement | Status |
|----|-------------|--------|
| STI-C1 | `storage.storage_id` remains the internal primary key. It MUST NOT be presented to the user as an editable value, and no UI may write it. | [Planned] |
| STI-C2 | For a Storage device, `device.device_external_id` MUST always equal an existing `storage.storage_id` in the same collection. No operation may leave it dangling. This is stated as an absolute because `Device::deleteDevice()` deletes the storage row that `device_external_id` names: a dangling value is not a display defect, it is a deletion pointed at the wrong row. | [Planned] |
| STI-C3 | Collection Import MUST NOT alter `storage_user_id` in any way — no offset, no suffix, no auto-disambiguation, no renumbering. Silently editing the number written on a physical disk is the failure this whole page exists to prevent. Collisions are surfaced by `STI-F6` and never resolved silently. | [Planned] |
| STI-C4 | Schema: `storage_user_id` is added to the `storage` CREATE TABLE in `Database` and, for existing databases, by an **unconditional column guard** rather than by a step inside `runMigration_2_13` — the field arrives after databases were already stamped 2.13, so the versioned migration no longer runs for them and the column would never appear. Same shape and same reasoning as `DCM-C4`; the general rule is recorded in `SpecDeviceComment.md`. | [Planned] |
| STI-C5 | In Memory mode the field is appended as the **last** column of `storage.csv` (the 18th), and the hand-written header line gains its label in the same change — the writer iterates the query record generically, but the header does not. The reader MUST bounds-check, because files written before this change have 17 fields and the reader indexes positions 0-16 today with no check at all. | [Planned] |
| STI-C6 | The device edit form MUST NOT write `storage.storage_id` or `device.device_external_id`. The existing ID-change branches in `qt_widgets/mainwindow_tab_device_pr.cpp` and in `AppManager::saveStorageDetails()` are **removed, not repointed**: leaving them standing while the field is redirected would make them write `storage_id = 0` on every save, which is strictly worse than today. | [Planned] |
| STI-C7 | `Device::verifyStorageExternalIDExists()` is retired for this purpose. The duplicate test of `STI-F6` reads `storage.storage_user_id`. Uniqueness of the internal key is enforced by the primary key alone and needs no application check. | [Planned] |
| STI-C8 | The storage picture filename stays keyed on the internal `storage_id`, never on `storage_user_id`. A user number is editable, and keying an image file on an editable value orphans the image the first time it changes. | [Planned] |
| STI-C9 | `storage_user_id` is NUMERIC. A non-numeric marking such as `A12` **cannot** be recorded; it falls back to zero. This is an accepted trade-off, taken deliberately to keep the numeric sorting and right alignment of the existing "Storage ID" columns in both UIs. Changing it to TEXT requires a new requirement **and** a decision about how those columns then sort. | [Planned] |
| STI-C10 | This page's scope is **stopping new damage**. It MUST NOT add an automatic repair pass over existing collections, on open, on import or otherwise. Detecting and repairing collections already damaged is `SpecQualityCheck.md` (issue #809), which is user-triggered by `QCK-C2`. | [Planned] |
| STI-C11 | `storage_total_space` and `storage_free_space` are shadow copies of the authoritative `device_total_space` and `device_free_space`. They MUST NOT be made authoritative, and no new read path may prefer them. `STI-F11` keeps them correct because they are persisted and copied across collections, not because anything displays them. The former `storage_path` copy is removed (`STI-C19`); removing the space copies is a later topic. | [Planned] |
| STI-C12 | `qt_quick/database.h` was a stale, unused copy of the schema — listed in `qt_quick/CMakeLists.txt` but never included, since `appmanager.cpp` includes `core/database.h`. It MUST NOT be edited by this work. *Retired: the file was deleted and removed from `qt_quick/CMakeLists.txt` with the user's approval (2026-09-28); the only schema is `core/database.h` / `core/database.cpp`.* | [Removed] |
| STI-C13 | A device's name lives in `device.device_name`, which is the **only** name. `storage.storage_name`, `catalog.catalog_storage` (`STI-C16`) and `catalog.catalog_name` (`STI-C17`) are removed: no code in K2, K3 or `core/` (including Collection Import and the Quality Check) reads or writes them. In Memory mode the slots of the first two — the "Name" column of `storage.csv` and the `<catalogStorage>` line of the `.idx` header — stay for format stability, written empty and never read. `Storage::name` is an in-memory field filled from `device_name`. `file.file_catalog` and the Memory-mode `.idx` file name are synced copies that remain in use and are kept equal to the device name on rename (`STI-F14`). | [Planned] |
| STI-C14 | Collection Import resolves the device hierarchy (`device_parent_id`) and the storage and catalog links (`device_external_id`) by internal ids only, never by names. Names are written from the imported data; they are never used to find, match or de-duplicate a row. There is **no exception**: the former pre-2.8 fallback by `catalog_name` is removed. Import requires identical schema stamps in source and target, in K2 and K3 (`SpecCollection.md`, strict abort), so an older source must first be opened — and migrated — by 3.0. | [Planned] |
| STI-C15 | The upkeep of the catalog's synced name copies (`STI-F14`: `file_catalog`, Memory-mode `.idx` file name and `catalog_file_path`) lives in **one** `core/` method, called by both K2 and K3 after a device save. | [Planned] |
| STI-C16 | The `storage` and `catalog` CREATE TABLE statements no longer declare `storage_name` and `catalog_storage`, and the 2.12 → 3.0 migration drops both columns from existing File and Hosted databases. The drop is **idempotent**: it runs safely on a database already stamped 2.13 by a beta, and on one where the columns are already gone. Memory-mode file formats are not changed. | [Planned] |
| STI-C17 | `catalog.catalog_name` is removed: no code reads or writes it, it leaves the `catalog` CREATE TABLE, and the 2.12 → 3.0 migration (`Database::runMigration_3_0`, which runs on every open until release) removes it from existing File and Hosted databases. The step is **idempotent**. On SQLite it rebuilds the `catalog` table, because the column carries a UNIQUE constraint and cannot simply be dropped; the rebuild keeps every catalog row, its `catalog_id` and all other columns. | [Planned] |
| STI-C18 | No two Catalog devices in a collection may share a `device_name`, because Memory mode stores each catalog as `<name>.idx`. The rule is enforced by the application, no longer by a database UNIQUE constraint: **create and rename** refuse a name already in use (existing K2/K3 dialogs; the check, `Device::verifyDeviceNameExists`, covers devices of every type, not only Catalog devices — stricter than needed, accepted); **split** gives each new catalog a unique name by appending `_2`, `_3`, … (checked against Catalog device names and against names already chosen in the same split); **import** appends ` (2)`, ` (3)`, … (`STI-F15`). | [Planned] |
| STI-C19 | `device.device_path` is the **only** path of a Storage or Catalog device. `storage.storage_path`, `catalog.catalog_source_path` and `catalog.catalog_source_path_is_active` (never read; activity is `device_active`) are removed: no code reads or writes them, they leave the CREATE TABLE statements, and `Database::runMigration_3_0` (every open until release) drops them from existing File and Hosted databases, **idempotently**. In memory, `Catalog::sourcePath` and `Storage::path` are the owning device's path: `Device::loadDevice()` sets them, and a standalone `Catalog::loadCatalog()` / `Storage::loadStorage()` reads `device_path` of the owning device (for a catalog, the one with the lowest `device_group_id`, i.e. the Physical one first). Memory-mode file slots are kept: the "Path" column of `storage.csv` is written empty and never read; the `<catalogSourcePath>` line of the `.idx` header keeps holding the device path, for readability, and is never read back as a source. | [Planned] |

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
- **STI-F3** — Import a Storage device whose name already exists in the target. A second storage row is created with a new internal id and the same name, and the imported device points at it and shows its own brand, model and serial. The target's original storage row and device are unchanged.
- **STI-F4** — *(Removed.)*
- **STI-F1 / delete hazard** — After the `STI-F1` import, delete the imported Storage device. No other storage device in the collection loses its details.
- **STI-F5** — Edit a Storage device, change the Storage ID, save, reopen the form: the new number is shown. Confirm no other device's details changed.
- **STI-F5 (zero)** — Set the Storage ID to 0 and save. It saves without complaint and the field reads 0 on reopen.
- **STI-F6** — Give two storage devices the same number. A warning appears and **both saves succeed**. Set a third to 0 while another is already 0: no warning.
- **STI-F7** — In a collection whose highest Storage ID is 12 and whose highest internal storage id is different, create a new Storage device in K2 and in K3. Its Storage ID field shows 13, and it is named `Storage_13` (with "Storage" in the interface language).
- **STI-F8 (import)** — Import a collection whose storages are numbered 3, 7 and 12 into a target already holding storages 1 and 2. The imported devices still read 3, 7 and 12 — never 4, 8, 13, and never `3 (2)`.
- **STI-F8 (update)** — Change a storage's number in the target, then run Update from the source collection. The target's number is **not** overwritten.
- **STI-F9** — Open a collection created before this change. Every storage device shows exactly the number it showed before the upgrade.
- **STI-F10** — Sort the Devices storage table by Storage ID. It sorts numerically (10 after 9, not before it) and the column is right-aligned.
- **STI-F11** — In Memory mode, note a storage's recorded total and free space, save the device from the edit form without changing them, then reopen the collection. The values are still there and `storage.csv` does not contain empty fields in their place.
- **STI-F12** — *(Removed.)*
- **STI-F14** — Rename a catalog in K2 and in K3. File mode: every `file` row of that catalog carries the new name in `file_catalog`, and duplicate search shows the new name. Memory mode: the `.idx` file carries the new name and the catalog reopens.
- **STI-F15** — Import a catalog whose name is already used by a Catalog device in the target. The imported device, its `.idx` file and its `file_catalog` values are all `<name> (2)`.
- **STI-C17 / STI-C18** — Open a 2.12 and a beta-2.13 collection with 3.0: `catalog_name` is gone, catalog count, ids and files are intact; reopen three times without error. Creating or renaming a catalog to a name another device already has is refused; splitting a catalog whose new name is taken gives it `_2`, `_3`, ….
- **STI-C19** — Open a 2.12 and a beta-2.13 collection with 3.0: `storage_path`, `catalog_source_path` and `catalog_source_path_is_active` are gone, and every catalog still updates (scans) from its device path. In Memory mode, after a save, `storage.csv` has an empty "Path" column and each `.idx` has `<catalogSourcePath>` holding the device path.
- **STI-F13** — Import only a Catalog device, without its parent Storage device, into a non-empty target. No storage row is added to the target.
- **STI-C14** — Import a source in which two devices share a name with devices already in the target. Every imported device sits under its own imported parent and points at its own imported storage or catalog row.
- **STI-C14 (no name fallback)** — Import from a collection whose schema stamp differs from the target's: the import aborts. Open that source with 3.0 first, then import: it succeeds, and each catalog is found by id.
- **STI-C13 / STI-C16** — Open a 2.12 collection in File mode and in Memory mode with 3.0. File mode: the `storage` and `catalog` tables have no `storage_name` / `catalog_storage` column. Memory mode: after a save, `storage.csv` still has its "Name" column, empty, and each `.idx` still has an empty `<catalogStorage>` line; the collection reopens correctly. Open again a File-mode database already stamped 2.13: the migration completes without error.
- **STI-C4 (already stamped)** — Take a File-mode collection **already stamped schema 2.13** from earlier in the cycle, without the new column. Open it. The guard adds `storage_user_id`, the back-fill runs, and every storage device lists as before.
- **STI-C4 (from 2.12)** — Open a 2.12 collection in File mode. The column is added, the numbers are preserved, and the schema version reads 2.13.
- **STI-C5 (old file)** — Open a Memory-mode collection whose `storage.csv` has 17 columns. It loads without error and the numbers come from the back-fill.
- **STI-C5 (round trip)** — Save that collection, confirm `storage.csv` now has 18 columns and a matching header, then reopen it. The numbers survive.
- **STI-C5 (2.12 regression)** — Open the 18-column collection with a released **2.12** binary. It opens correctly. Save it there, reopen in 2.13: the user numbers are gone and the back-fill has restored them from the internal ids. This is the documented one-way loss, not a bug to fix.
- **STI-C6** — Save a Storage device from the edit form in K2 and in K3 without changing anything. `storage_id` and `device_external_id` are unchanged in both cases, and the device still shows its own details.
- **STI-C8** — Change a storage's user number on a device that has a picture. The picture is still displayed afterwards.

---

## Related

- `SpecVersions.md` — "Compatibility with older versions": schema 3.0 at release, and what older applications do without the removed columns (risk 5) and with `storage_user_id` (risks 2, 2b).
- `SpecQualityCheck.md` — detecting and repairing collections **already** damaged by the import defect (issue #809). This page stops new damage; that one cleans up the old.
- `SpecCollection.md` — the Import / Update design note. Its "ID remapping" section documents `device_id_offset` and `catalog_id_offset` only; `storage_id_offset` appears in no document, which is how the defect stayed invisible.
- `SpecDeviceComment.md` — `DCM-C4`, the column-guard rule this page reuses.
- `SpecDeviceStorageRoot.md` — `DSR-C8` (now `[Removed]`), the former rule keeping `storage_path` in step with the save; superseded by `STI-C19`.
