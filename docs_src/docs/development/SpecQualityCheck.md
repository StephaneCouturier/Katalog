---
version: "2.13"
---

# QUALITY Check

![Status](https://img.shields.io/badge/Status-Draft-orange) ![Implementation](https://img.shields.io/badge/Implementation-backlog-lightgrey) ![Issue](https://img.shields.io/badge/Issue-%23809-blue) ![K2](https://img.shields.io/badge/K2-no%20change-lightgrey) ![K3](https://img.shields.io/badge/K3-3.0-blue)

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

This page records what can break and how to detect it. The feature itself is
backlog — see issue #809.

> **Reading the requirement IDs.** Each requirement has a permanent ID.
> IDs are never renumbered or reused; a retired requirement is marked
> `[Removed]`, not deleted. Status is one of
> `[Implemented] / [Planned] / [Backlog] / [Removed]`.

---

## Scope at a glance

| | |
|---|---|
| **In scope** | A user-triggered check over the open collection; a report of what it found; repairs the user chooses to apply; the storage-identity checks listed below |
| **Out of scope** | Automatic repair on open; repair without a report; checking file contents on disk; anything that writes outside the collection |
| **Applies to** | K3 3.0. K2 is in maintenance mode and gets no new UI for this |
| **Depends on** | `SpecStorageIdentity.md` for what a correct storage link looks like |

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

Check 2 is the one that finds silent damage. Checks 1 and 3 are usually two
views of the same break: a device orphaned by the renumbering, and the row it
should have been pointing at.

In Memory mode the `device` and `storage` tables are loaded from
`device.csv` and `storage.csv` at open, so the same checks apply once the
collection is open.

---

## Repair

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
| QCK-O1 | A user who suspects something is wrong with a collection can ask Katalog to check it, and gets a plain list of what it found. | [Backlog] |
| QCK-O2 | A user decides which problems get fixed. Katalog does not repair a collection on its own initiative. | [Backlog] |
| QCK-O3 | A user is warned about damage that is invisible in normal use — above all a storage device showing another disk's details. | [Backlog] |

## Functional requirements — *what the system does*

| ID | Requirement | Status |
|----|-------------|--------|
| QCK-F1 | The Quality Check is started by the user and runs over the collection that is open. | [Backlog] |
| QCK-F2 | It reports every Storage device whose `device_external_id` matches no `storage.storage_id` (check 1). | [Backlog] |
| QCK-F3 | It reports every Storage device whose `device_external_id` matches a storage row whose `storage_name` differs from the device's `device_name` (check 2). | [Backlog] |
| QCK-F4 | It reports every storage row that no Storage device points at (check 3). | [Backlog] |
| QCK-F5 | For each problem it can repair unambiguously, it offers the repair and applies it only when the user accepts. | [Backlog] |
| QCK-F6 | A problem with no unambiguous repair is reported with what was found, and nothing is written. | [Backlog] |
| QCK-F7 | The report says plainly when nothing was found, so a clean collection gives a clear answer rather than an empty screen. | [Backlog] |

## Constructional requirements — *how it is built / limits / MUST-NOTs*

| ID | Requirement | Status |
|----|-------------|--------|
| QCK-C1 | The checks and the repairs live in `core/`. The UI starts the check, shows the report and passes back what the user accepted. | [Backlog] |
| QCK-C2 | The Quality Check MUST NOT run automatically — not on open, not on import, not on a timer. It runs when the user asks for it. | [Backlog] |
| QCK-C3 | It MUST NOT write anything the user has not accepted, and MUST NOT delete a device or a storage row as part of a repair. | [Backlog] |
| QCK-C4 | It reads and writes the open collection only. It MUST NOT touch files outside the collection folder. | [Backlog] |
| QCK-C5 | It works in Memory, File and Hosted modes. In Memory mode the repaired tables are written back to their `.csv` files, or the repair is lost on close. | [Backlog] |
| QCK-C6 | New checks are added as new `QCK-F` rows. The feature is not a general "fix the database" command and MUST NOT grow into one without its own requirements. | [Backlog] |

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
- **QCK-C5** — Repeat the QCK-F5 repair in Memory mode. Close and reopen the collection: the repair is still there.

---

## Related

- `SpecStorageIdentity.md` — what a correct storage link is, and the import fix that stops new damage
- Issue #809
