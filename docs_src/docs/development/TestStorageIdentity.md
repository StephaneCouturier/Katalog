---
version: "2.13"
---

# TEST Storage Identity

![Status](https://img.shields.io/badge/Status-Ready%20to%20run-blue) ![Spec](https://img.shields.io/badge/Spec-SpecStorageIdentity-lightgrey) ![K2](https://img.shields.io/badge/K2-2.13-blue) ![K3](https://img.shields.io/badge/K3-3.0-blue)

Test plan for [SpecStorageIdentity](SpecStorageIdentity.md).
Conventions common to all test plans are in [Tests](Tests.md).

---

## Before you start

**Two collections are needed**, and the point of most of these tests is that the
target is **not empty** — an import into an empty collection cannot show the
defect, because the ID offsets are then 1 and nothing collides.

| | |
|---|---|
| **Source** | A collection with at least: one Storage device with two catalogs under it, and one Storage device with **no** catalogs. Give the storages recognisable brands/serials so a wrong match is obvious. |
| **Target** | A different, **non-empty** collection — at least one Storage device with a catalog, so `MAX(storage_id)` is greater than 0. |
| **Back it up** | These tests import into the target and change it. Work on copies. |

**The check query.** Several cases end with "the check query returns no rows".
It finds every Storage device pointing at no storage row, by id only:

```sql
SELECT d.device_id, d.device_name, d.device_external_id
FROM   device d
LEFT JOIN storage s ON s.storage_id = d.device_external_id
WHERE  d.device_type = 'Storage' AND s.storage_id IS NULL;
```

It does **not** compare names: the storage row has no name any more
(`STI-C13`), and a name could not tell a correct link from a wrong one anyway. A device
pointing at the **wrong** disk is caught by reading the device's own details —
brand, model and serial — which is why the source storages must carry
recognisable values.

---

## Part A — Collection Import

| ID | Verifies | Test |
|----|----------|------|
| STI-T1 | STI-F1, STI-C2 | Import the source into the **non-empty** target. For every imported Storage device, open it and read its brand, model and serial: each shows **its own** values, not blank and not another disk's. The check query returns no rows. *(Before the fix this is where storages came out blank or wearing another disk's identity.)* |
| STI-T2 | STI-F2 | In the same import, find the Storage device that had **no catalogs** under it. It exists in the target **and has a storage row**: its brand/model/serial are present, not blank. |
| STI-T3 | STI-F3, STI-C14 | Rename a Storage device in the source so its name matches a storage already in the target. Import. The target now holds **both** storage rows, with **different internal ids**, and both devices keep their shared name — no `(2)` suffix. The imported device points at the new row and shows **its own** brand/model/serial; the pre-existing device still shows the original's. The check query returns no rows. |
| STI-T4 | STI-F3, STI-C14 | Continuing STI-T3: the catalogs that came in with the imported Storage device sit **under that device** in the tree, not under the target's same-name storage. A catalog that was already in the target under the original storage still sits under it, unchanged. |
| STI-T5 | STI-F8, STI-C3 | Note the Storage ID shown for a storage in the source. Import it. The imported storage shows **the same number**, unchanged — no offset, no `(2)` appended. |
| STI-T6 | STI-F8 | Use *update from an external collection* on a storage whose user number differs between source and target. Afterwards the target keeps **its own** number, while the disk's details (brand, model, free space) are refreshed from the source. |
| STI-T7 | STI-F1 | Import twice into the same target **in one session**, without restarting. The second import's links are as correct as the first's; the check query returns no rows. *(Guards the per-run id maps: if they are not reset, the second import reuses ids from the first.)* |
| STI-T8 | STI-F2, STI-C14 | Import a source where **five catalogs sit under one Storage device**. The target gains **one** storage row for it — the one imported with the Storage device — not five, and all five catalogs sit under that device. |
| STI-T23 | STI-F13 | Import **only a Catalog device**, without its parent Storage device, into a non-empty target. Count the target's storage rows before and after: **no** storage row is added. |
| STI-T24 | STI-C14 | Give two source devices the same names as devices already in the target (one Storage, one Catalog beneath it). Import. Every imported device sits under **its own imported parent**, and each points at its own imported storage or catalog row — none attaches to a same-name target device. |

## Part A2 — name copies and removed columns

| ID | Verifies | Test |
|----|----------|------|
| STI-T25 | STI-F12, STI-C15 | *Retired: `storage_name` and `catalog_storage` are removed (`STI-C13`, `STI-C16`); `STI-F12` is `[Removed]`.* |
| STI-T26 | STI-F12, STI-C15 | *Retired: `storage_name` and `catalog_storage` are removed (`STI-C13`, `STI-C16`); `STI-F12` is `[Removed]`.* |
| STI-T27 | STI-C13 | *Retired: `storage_name` and `catalog_storage` are removed (`STI-C13`, `STI-C16`); `STI-F12` is `[Removed]`.* |
| STI-T28 | STI-F14 | In **File mode**, rename a catalog **in K3**. Every `file` row of that catalog has the new name in `file.file_catalog`, and a duplicate search lists its files under the new name, never the old one. Repeat in K2, and in Hosted mode: same result. |
| STI-T29 | STI-C14 | Import from a **Memory-mode collection written before 2.8** (catalog ids load as 0). Its catalogs and their files arrive in the target (legacy name fallback). Then import from a 2.13 collection in which one catalog row's `catalog_name` was changed directly to a new, unique value that no longer matches its device's name: that catalog device still brings **its own** catalog and files, found by id. |
| STI-T30 | STI-C13, STI-C16 | Open a **2.12 collection in File mode** with 3.0: the `storage` and `catalog` tables have **no** `storage_name` / `catalog_storage` column, and every device, storage detail and catalog shows as before. Open a **File-mode database already stamped 2.13** by a beta: the migration completes without error and the columns are gone. Open a **2.12 collection in Memory mode** with 3.0, change a storage and a catalog, save, close: `storage.csv` still has its "Name" column, empty, and each `.idx` still has an empty `<catalogStorage>` header line. Reopen with 3.0: the collection opens and shows everything as before. |

## Part B — the identity split

| ID | Verifies | Test |
|----|----------|------|
| STI-T9 | STI-F9 | Take a collection created **before** this change (schema already stamped 2.13, no `storage_user_id`). Open it. Every storage shows the **same Storage ID it showed before** the upgrade, and no storage details are lost. |
| STI-T10 | STI-F5, STI-C1, STI-C6 | Edit a storage, change its Storage ID, save. Reopen it: the new number is shown. Then check that the **device is still linked** — its brand/model/serial are still its own, and the check query returns no rows. *(Before this change, changing the ID in K3 silently discarded every storage field and orphaned the device; in K2 it lost the name and path.)* |
| STI-T11 | STI-F6 | Give storage B the number already carried by storage A. Save. **The save completes** — reopen B and the number is there — and a warning appears saying another storage already uses this ID. In K3 the warning is inline in the form, not a dialog, and disappears when the ID field is edited. |
| STI-T12 | STI-F6 | Leave two storages with ID `0` (no number written on them). Save each. **No warning** — zero is exempt. |
| STI-T13 | STI-F7 | Create a new storage. It receives a Storage ID automatically, and that number appears in its name suffix. Edit the number to something else and save: the number changes, and the device stays linked (check query returns no rows). |
| STI-T14 | STI-F10 | Open the Devices storage list in **K2 and in K3**. The Storage ID column shows each storage's user number — the same value its edit form shows. Sort by that column: it sorts **numerically** (2 before 10, not 10 before 2) and stays right-aligned. |
| STI-T15 | STI-C9 | In the Storage ID field, type `A12` and save. It is not recorded — the field falls back to `0`. *(Documented limitation, not a defect: the field is numeric by decision.)* |
| STI-T16 | STI-F11 | Note a storage's total and free space. Open its edit form, change something unrelated (a comment), save. Reopen: **total and free space are still there**, not blank. *(Before this change every K2 storage save wrote them NULL.)* |
| STI-T17 | STI-C8 | Give a storage a picture. Change its Storage ID and save. The picture is **still shown** — it is keyed on the internal id, so the user's number can change without orphaning the image. |

## Memory mode

| ID | Verifies | Test |
|----|----------|------|
| STI-T18 | STI-C5 | Repeat STI-T10 on a **Memory-mode** collection. Close and reopen it: the number is still there. *(It has to survive the `storage.csv` round trip, not just the in-memory table.)* |
| STI-T19 | STI-C5 | Open a Memory-mode collection written **before** this change — its `storage.csv` has 17 columns, not 18. It opens without error and every storage shows its number, back-filled from the internal id. |
| STI-T20 | STI-C5, STI-F9 | Open a Memory collection saved by the **new** version with a released **2.12** binary. It opens and works. Then save in 2.12 and reopen in 2.13: **the user numbers are gone**, back to the internal ids. This is the known one-way loss stated in the spec — the test confirms it is *that*, and not a crash or a broken collection. |

## Regression — what must not have broken

| ID | Verifies | Test |
|----|----------|------|
| STI-T21 | STI-C2 | After any of the import tests, delete an imported Storage device. **Only that device's storage row disappears**; every other storage keeps its brand/model/serial. *(`deleteDevice()` resolves the row through `device_external_id`, so a mispointed device would take another disk's row with it — this is the data-destroying case the import fix closes.)* |
| STI-T22 | STI-C10 | Confirm nothing repairs itself: open a collection known to have broken links from an **older** import. It is **not** silently fixed — repairing existing damage is `SpecQualityCheck.md`, deliberately out of scope here. |

---

## Coverage

Every requirement in `SpecStorageIdentity.md` is cited above except the
following, which are not runnable tests:

| Requirement | Why no test case |
|-------------|------------------|
| STI-O1, STI-O2, STI-O3 | Operational intent. They are covered in substance by STI-T5, STI-T1 and STI-T11 respectively. |
| STI-C4 | Schema mechanics, verified by STI-T9 (the upgrade works) rather than by inspecting the guard. |
| STI-C7 | Internal refactor with no user-visible behaviour of its own; its effect is STI-T11. |
| STI-C11 | Records why three columns are shadow copies. STI-T16 covers the behaviour that depends on it. |
| STI-C12 | A "do not edit this file" rule for developers, not behaviour. |

---

## Related

- [SpecStorageIdentity](SpecStorageIdentity.md) — the requirements these cases verify
- [SpecVersions](SpecVersions.md) — "Compatibility with older versions": schema 3.0 at release and what older applications do without the removed columns
- [SpecQualityCheck](SpecQualityCheck.md) — detecting and repairing collections already damaged
- [Test plan index](Tests.md)
