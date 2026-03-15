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
