---
version: "2.13"
---

# K3 PARITY GAPS — K2 FEATURES NOT YET IN K3

![Status](https://img.shields.io/badge/Status-Reference-lightgrey) ![Issue](https://img.shields.io/badge/Issue-%23803-blue) ![K2](https://img.shields.io/badge/K2-no%20change-lightgrey) ![K3](https://img.shields.io/badge/K3-gap-orange)

## Context

K3 is a port of K2 and must reach full feature parity with it. While preparing the
K3 user documentation (#803, 2026-10-03), the K2 user pages were compared with the
K3 pages. The features below are documented for K2 and implemented in K2, but have
no equivalent in K3.

This page is a **list of known gaps, not a requirement.** Each gap is ported later,
once the documentation work is well advanced; porting one follows the K3 migration
process (read all the K2 code for the feature, get a specifications verdict, port it
in full). Until a gap is ported, the K3 user documentation does not describe it; the
K2 description stays available in the Katalog 2 documentation copy.

When a gap is ported, remove its entry here and add it to the K3 user page.

---

## Create and Tags — file system tree to pick a folder

**Found:** 2026-10-07 (Create page).
K2's Create tab has a file-system tree (`Create_treeView_Explorer`, `QFileSystemModel`,
`qt_widgets/mainwindow_tab_create.cpp:45`, `:273`): clicking a folder fills the source path.
K2's Tags tab has the same tree (`Tags_treeview_Explorer`, `qt_widgets/mainwindow_tab_tags.cpp:90`)
to fill the folder to tag.
K3 offers typing and a folder dialog only (*Select the path* on Create, *Select a folder* on Tags).
**Decided:** accepted the folder dialog as the K3 equivalent.

---

## Create — "Add Storage" button

**Found:** 2026-10-07 (Create page).
K2's Create tab has *Add Storage* (`on_Create_pushButton_AddStorage_clicked`,
`qt_widgets/mainwindow_tab_create.cpp:117`): it switches to the Devices tab in tree view so
a storage can be added. K3's Create page has no such button; *Add Storage* exists on the
Devices page toolbar.
**K3 user doc:** `Create.md` points to *Add Storage* on the Devices page.
**Decided:** accepted as is.

---

---

# To review and fix before the 3.0 release

Defects and drift found while writing the K3 user documentation (#803). Not parity
gaps: K3 has the feature, but something in it differs from the spec, from K2 or from
the K3 message rules. Each one goes through the specifications gate before it is fixed.
When one is fixed, remove its entry here.

*All review items were fixed on 2026-10-10 (see `SpecDeviceStorageRoot` DSR-F1, `SpecStorageIdentity` STI-C8, `SpecValidationRules` Edit Device, `SpecBackup` BKP-F8/F26). The Hosted "devices missing after Connect" report is logged in the GitHub backlog.*
