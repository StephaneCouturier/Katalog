---
version: "2.13"
---

# QUALITY Check

![Status](https://img.shields.io/badge/Status-Draft-orange) ![Implementation](https://img.shields.io/badge/Implementation-planned-orange) ![Issue](https://img.shields.io/badge/Issue-%23809-blue) ![K2](https://img.shields.io/badge/K2-no%20change-lightgrey) ![K3](https://img.shields.io/badge/K3-3.0-blue)

## Context

A collection can end up internally inconsistent without the user doing anything
wrong. The first case found — 2026-09-19, while investigating a real collection
where *"some storage are not matching their device anymore"* — is Collection
Import leaving a Storage device pointing at a storage row that no longer exists,
or at the wrong one.

The response is **not** a silent automatic repair on open. A collection is the
user's data; a program that quietly rewrites links is exactly what created the
problem. Instead: a **Quality Check** the user runs, which reports what it
found, and repairs only what the user tells it to.

This page records what can break and how to detect it — see issue #809.

**Phases.** The feature is delivered in two phases:

- **Phase 1 — diagnosis only.** The Quality Check runs the checks and reports
  what it found. It repairs nothing and writes nothing (`QCK-C7`). Its rows are
  `[Planned]`.
- **Phase 2 — repair.** The *Repair* section below, `QCK-F5`, `QCK-F6` and the
  Memory-mode write-back of `QCK-C5`. Not in the first cut; those rows stay
  `[Backlog]`.

> **Reading the requirement IDs.** Each requirement has a permanent ID.
> IDs are never renumbered or reused; a retired requirement is marked
> `[Removed]`, not deleted. Status is one of
> `[Implemented] / [Planned] / [Backlog] / [Removed]`.

---

## Scope at a glance

| | |
|---|---|
| **In scope** | Phase 1: a user-triggered, read-only check over the open collection, started from the K3 Settings page; a report of what it found, which can be copied as plain text; the checks listed below. Phase 2 (later): repairs the user chooses to apply |
| **Out of scope** | Automatic repair on open; repair without a report; any write at all in phase 1; checking file contents on disk; the `file`, `filetemp` and `folder` tables; anything that writes outside the collection |
| **Applies to** | K3 3.0. K2 is in maintenance mode and gets no new UI for this |
| **Depends on** | `SpecStorageIdentity.md` for what a correct storage link looks like, and for the rule that a duplicate written storage number is allowed (`STI-O3`, `STI-F6`); `SpecValidationRules.md` for the report window's buttons and the Copy confirmation channel |

---

## What is known to break

### 1. Storage device pointing at nothing

A Storage device's `device.device_external_id` names a `storage.storage_id` that
does not exist. The device shows blank storage details — no brand, no model, no
serial.

**Cause:** Collection Import renumbers the storage row (`storage_id` becomes
`source id + MAX(storage_id) + 1` in the target) but copies
`device_external_id` across unchanged. The two no longer agree.

### 2. Storage device pointing at the wrong disk

Same cause, worse outcome. The stale `device_external_id` happens to match a
**different** storage row that already existed in the target. The device then
silently shows another drive's brand, model, serial and label — and the number
written on its case.

This is the dangerous one, because nothing looks broken.

### 3. Storage device with no storage row at all

`importStorageForCatalog()` runs only for Catalog devices. A Storage device with
no catalogs beneath it — or whose catalogs name a storage already present in the
target — never gets a storage row created for it at all.

### The delete hazard

`Device::deleteDevice()` resolves the storage row the same way the display does:

```
storage->ID = externalID;
storage->deleteStorage();
```

So deleting a Storage device affected by case 2 **deletes a different disk's
storage row**. Until the Quality Check exists, a collection suspected of this
should have its storage links checked before any Storage device is deleted.

---

## Detection

These are the checks the feature performs. They are written here as SQL so they
can be run by hand on a File-mode collection in the meantime.

**Check 1 — devices pointing at a missing storage row**

```sql
SELECT d.device_id, d.device_name, d.device_external_id
FROM   device d
LEFT JOIN storage s ON s.storage_id = d.device_external_id
WHERE  d.device_type = 'Storage' AND s.storage_id IS NULL;
```

**Check 2 — devices pointing at a storage row with a different name**

```sql
SELECT d.device_id, d.device_name, s.storage_id, s.storage_name
FROM   device d
JOIN   storage s ON s.storage_id = d.device_external_id
WHERE  d.device_type = 'Storage' AND d.device_name <> s.storage_name;
```

**Check 3 — storage rows nothing points at**

```sql
SELECT s.storage_id, s.storage_name
FROM   storage s
LEFT JOIN device d ON d.device_external_id = s.storage_id
                  AND d.device_type = 'Storage'
WHERE  d.device_id IS NULL;
```

**Check 4 — storage rows sharing the same written number (informational)**

```sql
SELECT s.storage_user_id, s.storage_id, s.storage_name
FROM   storage s
WHERE  s.storage_user_id IS NOT NULL AND s.storage_user_id <> 0
  AND  s.storage_user_id IN (SELECT storage_user_id FROM storage
                             GROUP BY storage_user_id HAVING COUNT(*) > 1)
ORDER BY s.storage_user_id, s.storage_id;
```

A duplicate written number is **allowed**: `SpecStorageIdentity.md` `STI-O3`
and `STI-F6` let two disks carry the same number and only warn on save; zero
means "no number" and is exempt. This check therefore reports duplicates for
the user's information. It is not a defect, and phase 2 has no repair for it.

**Check 5 — storage rows sharing the same name**

```sql
SELECT s.storage_name, s.storage_id
FROM   storage s
WHERE  s.storage_name IN (SELECT storage_name FROM storage
                          GROUP BY storage_name HAVING COUNT(*) > 1)
ORDER BY s.storage_name, s.storage_id;
```

These rows cannot be told apart by name, so the name-based repair (Repair §4)
can never resolve a device pointing at one of them.

**Check 6 — Catalog devices pointing at a missing catalog row**

```sql
SELECT d.device_id, d.device_name, d.device_external_id
FROM   device d
LEFT JOIN catalog c ON c.catalog_id = d.device_external_id
WHERE  d.device_type = 'Catalog' AND c.catalog_id IS NULL;
```

**Check 7 — catalog rows no Catalog device points at**

```sql
SELECT c.catalog_id, c.catalog_name
FROM   catalog c
LEFT JOIN device d ON d.device_external_id = c.catalog_id
                  AND d.device_type = 'Catalog'
WHERE  d.device_id IS NULL;
```

**Check 8 — devices whose parent does not exist**

```sql
SELECT d.device_id, d.device_name, d.device_parent_id
FROM   device d
LEFT JOIN device p ON p.device_id = d.device_parent_id
WHERE  d.device_parent_id IS NOT NULL AND d.device_parent_id <> 0
  AND  p.device_id IS NULL;
```

A `device_parent_id` of 0 or NULL marks a root device and is not reported.

Check 2 is the one that finds silent damage. Checks 1 and 3 are usually two
views of the same break: a device orphaned by the renumbering, and the row it
should have been pointing at. Checks 6 and 7 are the same pair for catalogs.

**Tables each check reads.** Checks 1, 2 and 3: `device`, `storage`. Checks 4
and 5: `storage`. Checks 6 and 7: `device`, `catalog`. Check 8: `device`. No
check reads `file`, `filetemp` or `folder`.

**Memory mode.** All three tables are loaded when the collection is opened
(`Collection::load()`): `device` from `device.csv`, `storage` from
`storage.csv`, and `catalog` from the header lines of every `.idx` file in the
collection folder. So every check applies as soon as the collection is open, and
none needs the per-catalog file/folder pre-load. One caution for checks 6 and 7:
in Memory mode `catalog_id` comes from the `.idx` header, and a catalog written
before 2.8 carries no id there and loads as 0 — such a catalog will show up in
check 7, and a Catalog device pointing at it in check 6. That is a true
finding about the loaded data, not a false positive of the check.

---

## Repair

> **Phase 2 — not in the first cut.** Phase 1 reports only; nothing in this
> section is built until phase 2.

The name is what survives the renumbering, so it is what the repair matches on.

1. For each device found by check 1, look for a storage row whose
   `storage_name` equals the device's `device_name`.
2. Exactly one match → offer to set `device_external_id` to that row's
   `storage_id`.
3. No match → report it. The storage row was never imported (case 3); the user
   decides whether to recreate it or delete the device.
4. More than one match → report it and change nothing. Two storage rows sharing
   a name cannot be told apart by name.

Nothing is written until the user accepts. A repair that cannot be made
unambiguously is reported, never guessed.

---

## Operational requirements — *why / for whom*

| ID | Requirement | Status |
|----|-------------|--------|
| QCK-O1 | A user who suspects something is wrong with a collection can ask Katalog to check it, and gets a plain list of what it found. | [Planned] |
| QCK-O2 | A user decides which problems get fixed. Katalog does not repair a collection on its own initiative. | [Planned] |
| QCK-O3 | A user is warned about damage that is invisible in normal use — above all a storage device showing another disk's details. | [Planned] |

## Functional requirements — *what the system does*

| ID | Requirement | Status |
|----|-------------|--------|
| QCK-F1 | The Quality Check is started by the user and runs over the collection that is open. | [Planned] |
| QCK-F2 | It reports every Storage device whose `device_external_id` matches no `storage.storage_id` (check 1). | [Planned] |
| QCK-F3 | It reports every Storage device whose `device_external_id` matches a storage row whose `storage_name` differs from the device's `device_name` (check 2). | [Planned] |
| QCK-F4 | It reports every storage row that no Storage device points at (check 3). | [Planned] |
| QCK-F5 | For each problem it can repair unambiguously, it offers the repair and applies it only when the user accepts. *Phase 2 — not in the first cut.* | [Backlog] |
| QCK-F6 | A problem with no unambiguous repair is reported with what was found, and nothing is written. *Phase 2 — not in the first cut.* | [Backlog] |
| QCK-F7 | The report says plainly when nothing was found, so a clean collection gives a clear answer rather than an empty screen. | [Planned] |
| QCK-F8 | It reports every group of two or more storage rows sharing the same non-zero `storage_user_id` (check 4). This is **informational**: a duplicate written number is allowed by `STI-O3` / `STI-F6`, so the report presents it as something to know, not as damage, and zero is never reported. | [Planned] |
| QCK-F9 | It reports every group of two or more storage rows sharing the same `storage_name` (check 5). | [Planned] |
| QCK-F10 | It reports every Catalog device (`device_type = 'Catalog'`) whose `device_external_id` matches no `catalog.catalog_id` (check 6). | [Planned] |
| QCK-F11 | It reports every catalog row that no Catalog device points at (check 7). | [Planned] |
| QCK-F12 | It reports every device whose `device_parent_id` is neither 0/NULL (root) nor an existing `device.device_id` (check 8). | [Planned] |
| QCK-F13 | In K3, the Quality Check is started by a button on the Settings page, in the "Collection & Database" section, on the "Database Version" row, placed next to the version value — the same value-plus-button pattern as the Collection row's Edit button. K2 gets no entry point. | [Planned] |
| QCK-F14 | The report is shown in a window with one section per check. Each section shows the number of rows found and lists them by their ids and names. | [Planned] |
| QCK-F15 | Each check that finds nothing says so explicitly in its own section, in addition to the overall statement of `QCK-F7`. | [Planned] |
| QCK-F16 | The report window has a Copy action that puts the whole report on the clipboard as plain text, and confirms it with a transient passive notification. | [Planned] |

## Constructional requirements — *how it is built / limits / MUST-NOTs*

| ID | Requirement | Status |
|----|-------------|--------|
| QCK-C1 | The checks and the repairs live in `core/`. The UI starts the check, shows the report and passes back what the user accepted. | [Planned] |
| QCK-C2 | The Quality Check MUST NOT run automatically — not on open, not on import, not on a timer. It runs when the user asks for it. | [Planned] |
| QCK-C3 | It MUST NOT write anything the user has not accepted, and MUST NOT delete a device or a storage row as part of a repair. | [Planned] |
| QCK-C4 | It reads and writes the open collection only. It MUST NOT touch files outside the collection folder. | [Planned] |
| QCK-C5 | It works in Memory, File and Hosted modes. In Memory mode the repaired tables are written back to their `.csv` files, or the repair is lost on close. *Phase 1 builds the first sentence; the Memory-mode write-back is phase 2.* | [Planned] |
| QCK-C6 | New checks are added as new `QCK-F` rows. The feature is not a general "fix the database" command and MUST NOT grow into one without its own requirements. | [Planned] |
| QCK-C7 | Phase 1 is strictly read-only. It MUST NOT write to the database or to any collection file — no `INSERT`, `UPDATE`, `DELETE` or schema change, and no `.csv`, `.idx` or `.ini` write. | [Planned] |

---

## User-visible text

None approved yet. Every string this feature needs — the report headings, the
repair offer, the "nothing found" message — requires its own approval before it
is written. No wording is proposed here.

---

## Manual test charter

- **QCK-F2 / QCK-F5** — Import a collection into a non-empty one so a Storage device is orphaned. Run the check: the device is listed, the repair is offered, and accepting it restores the storage details.
- **QCK-F3** — Arrange a collection where a device's `device_external_id` matches a differently-named storage row. Run the check: it is reported as a wrong match, not as a missing one.
- **QCK-F4** — Leave a storage row nothing points at. Run the check: it is listed.
- **QCK-F6** — Give two storage rows the same name and orphan a device of that name. Run the check: the ambiguity is reported and nothing is written.
- **QCK-F7** — Run the check on a healthy collection: it says so.
- **QCK-C2** — Open a collection, import into it, wait: the check never runs by itself.
- **QCK-C5** — Repeat the QCK-F5 repair in Memory mode. Close and reopen the collection: the repair is still there. *(Phase 2.)* Phase 1: run the check on the same collection in Memory, File and Hosted mode: it completes and reports in each.
- **QCK-F8** — Give two storages the same non-zero Storage ID and two others Storage ID 0. Run the check: the pair is listed as informational, the zeros are not listed.
- **QCK-F9** — Give two storage rows the same name. Run the check: both rows are listed with their ids.
- **QCK-F10** — Set a Catalog device's `device_external_id` to an id no catalog has. Run the check: the device is listed.
- **QCK-F11** — Leave a catalog row no Catalog device points at. Run the check: the catalog is listed.
- **QCK-F12** — Set a device's `device_parent_id` to an id no device has. Run the check: the device is listed; root devices (parent 0) are not.
- **QCK-F13** — In K3, open Settings: the button sits on the "Database Version" row next to the version value, laid out like the Collection row's Edit button, and starts the check. In K2 nothing has changed.
- **QCK-F14** — Run the check on a damaged collection: the window shows one section per check, each with its count and the ids and names of the rows found.
- **QCK-F15** — Run the check on a collection with only one kind of damage: every other section says it found nothing.
- **QCK-F16** — Use Copy, paste into a text editor: the whole report arrives as plain text, and a passive notification confirmed the copy.
- **QCK-C7** — On a File-mode collection, note the database file's modification time and checksum; on a Memory-mode collection, those of every `.csv`, `.idx` and `.ini` in the collection folder. Run the check. Nothing has changed.

---

## Related

- `SpecStorageIdentity.md` — what a correct storage link is, and the import fix that stops new damage
- Issue #809
