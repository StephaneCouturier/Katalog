---
id: SpecBacklogNotes
title: Backlog Notes
description: Collected known issues and edge cases to address in future development
---

# Backlog Notes

![Development](https://img.shields.io/badge/Status-Development-blue)

This page collects known issues and edge cases that were identified during development or testing but deferred for later resolution.

---

## Tags — Symlink path mismatch

**Identified:** 2026-03-15
**Affects:** K2 and K3 (core: `searchjobstoppable.cpp`)

If a tag is created by selecting a folder **via a symlink**, the path stored in the `tag` table is the symlink path (e.g. `/home/user/link-to-music`). However, catalog files are indexed using the **real/resolved path** (e.g. `/media/disk/Music`). The tag path comparison in `searchFilesInCatalog` and `searchFilesInDirectory` uses `QString::contains()` on the literal string, so symlinked tag paths never match real catalog paths.

**Current comparison logic (`searchFilesInCatalog` line ~674):**
```cpp
(filePath + "/").contains(queryTag.value(0).toString() + "/")
```

**Suggested fix:** Resolve the tag path and/or the file path with `QFileInfo::canonicalFilePath()` before comparing. Also consider resolving at tag creation time (store the canonical path in the DB) so existing records remain valid after the symlink is removed.

---

## Source path — no retro-normalization of already-stored paths

**Identified:** 2026-08-12
**Affects:** K2 and K3 (core: `devicejobstoppable.cpp`)
**Related requirements:** `SpecValidationRules.md` N1–N6

This is a **note, not a requirement.** Nothing below is authorised work.

N1 normalizes a path only at the moment it is picked or typed. A device whose
path was stored **with** a trailing separator before N1 existed keeps that stored
value until the user re-picks or re-edits the path; nothing sweeps the existing
`device.device_path` / `catalog.catalog_source_path` rows.

Two propagation points bypass the single core rule:

- `core/devicejobstoppable.cpp:351` — `device->catalog->sourcePath = device->path;`
  assigns directly instead of going through `Catalog::setSourcePath()`, so a
  stale stored path is copied verbatim onto the catalog at update time.
- `qt_widgets/` — K2 never normalizes at all (documented as accepted drift by N7),
  so a path entered in K2 can re-introduce a trailing separator that K3 then
  inherits.

**Options if this is ever taken up:** (a) route the assignment through
`Catalog::setSourcePath()`, (b) add a one-shot normalization pass over existing
device/catalog rows, or (c) accept the residual and close it when N7 is closed.
A requirement would have to be approved and written into
`SpecValidationRules.md` first.

---
