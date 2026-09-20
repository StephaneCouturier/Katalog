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

**The check query.** Several cases end with "the links are correct". That means
this returns no rows:

```sql
SELECT d.device_id, d.device_name, d.device_external_id, s.storage_id, s.storage_name
FROM   device d
LEFT JOIN storage s ON s.storage_id = d.device_external_id
WHERE  d.device_type = 'Storage'
  AND (s.storage_id IS NULL OR d.device_name <> s.storage_name);
```

It catches both failure modes at once: a device pointing at nothing, and a
device pointing at the wrong disk. The three separate queries are in
`SpecQualityCheck.md` if you want to tell them apart.

---

## Part A — Collection Import

| ID | Verifies | Test |
|----|----------|------|
| STI-T1 | STI-F1, STI-C2 | Import the source into the **non-empty** target. For every imported Storage device, open it and read its brand, model and serial: each shows **its own** values, not blank and not another disk's. The check query returns no rows. *(Before the fix this is where storages came out blank or wearing another disk's identity.)* |
| STI-T2 | STI-F2 | In the same import, find the Storage device that had **no catalogs** under it. It exists in the target **and has a storage row**: its brand/model/serial are present, not blank. |
| STI-T3 | STI-F3 | Rename a storage in the source so its name matches one already in the target. Import. The target now holds **both**: the original, untouched, and a second row named `<name> (2)`. The imported device points at `(2)`, the pre-existing device still points at the original. |
| STI-T4 | STI-F4 | Continuing STI-T3: open the catalogs that came in with the renamed storage. Each is attached to `<name> (2)`, **not** to the target's original. Then open a catalog that was already in the target under the original name — it is still attached to the original. *(Getting STI-F3 without STI-F4 moves the mismatch instead of fixing it, and this case is the only one that catches that.)* |
| STI-T5 | STI-F8, STI-C3 | Note the Storage ID shown for a storage in the source. Import it. The imported storage shows **the same number**, unchanged — no offset, no `(2)` appended to the number. The name may have been disambiguated; the number never is. |
| STI-T6 | STI-F8 | Use *update from an external collection* on a storage whose user number differs between source and target. Afterwards the target keeps **its own** number, while the disk's details (brand, model, free space) are refreshed from the source. |
| STI-T7 | STI-F1 | Import twice into the same target **in one session**, without restarting. The second import's links are as correct as the first's; the check query returns no rows. *(Guards the per-run id maps: if they are not reset, the second import reuses ids from the first.)* |
| STI-T8 | STI-F2, STI-F3 | Import a source where **five catalogs sit on one storage**. The target gains **one** storage row for it, not five. |

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
- [SpecQualityCheck](SpecQualityCheck.md) — detecting and repairing collections already damaged
- [Test plan index](Tests.md)
