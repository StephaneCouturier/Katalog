---
version: "2.13"
---

# Specifications

![Status](https://img.shields.io/badge/Status-Index-lightgrey) ![Pages](https://img.shields.io/badge/Pages-27-blue)

Every specification page lives in `docs_src/docs/development/`. This page is the
index: **new specifications are added here, not to the sidebar.**

A specification defines what Katalog is required to do. The code is never the
source of requirements — it may contain unauthorised or buggy behaviour. Where a
specification and the code disagree, the specification is what was agreed.

| Page | Subject | Status | Implementation | Test plan |
|------|---------|--------|----------------|-----------|
| [SpecAbout](SpecAbout.md) | ABOUT — VERSION AND SYSTEM INFORMATION | Approved | — | — |
| [SpecApplicationIcon](SpecApplicationIcon.md) | APPLICATION ICON AND DESKTOP INTEGRATION | Approved | — | — |
| [SpecApplicationIconBacklogNotes](SpecApplicationIconBacklogNotes.md) | APPLICATION ICON — RESEARCH NOTES | Reference | — | — |
| [SpecBacklogNotes](SpecBacklogNotes.md) | Backlog Notes | — | — | — |
| [SpecBackup](SpecBackup.md) | BACKUP | Draft | complete | — |
| [SpecCardsAndTables](SpecCardsAndTables.md) | CARDS, TABLES AND TREES — SHARED DISPLAY CONVENTIONS | Approved | planned | — |
| [SpecCatalogIncludeExclude](SpecCatalogIncludeExclude.md) | Spec: Catalog Include / Exclude Rules | — | — | — |
| [SpecCollection](SpecCollection.md) | SpecCollection — Import / Update across Collections (#750) | — | — | — |
| [SpecCollectionIdentity](SpecCollectionIdentity.md) | COLLECTION IDENTITY — NAMING COLLECTIONS IN THE K3 DRAWER | Approved | planned | — |
| [SpecCollectionOpen](SpecCollectionOpen.md) | COLLECTION OPEN — FIRST RUN AND NEW DATABASE | Approved | complete | — |
| [SpecDeviceActiveStatus](SpecDeviceActiveStatus.md) | DEVICE Active Status | Approved | — | — |
| [SpecDeviceComment](SpecDeviceComment.md) | DEVICE Comment | Approved | complete | — |
| [SpecDeviceStorageRoot](SpecDeviceStorageRoot.md) | DEVICE Storage Root | Approved | partial | — |
| [SpecDevicesPage](SpecDevicesPage.md) | DEVICES PAGE — CARDS AND TABLE DISPLAY | Approved | planned | — |
| [SpecDevicesSplit](SpecDevicesSplit.md) | DEVICES: Split a Catalog | — | — | — |
| [SpecExplore](SpecExplore.md) | EXPLORE — DIRECTORY TREE | Approved | complete | — |
| [SpecImportVVV2K](SpecImportVVV2K.md) | Feature Specification: VVV2K — Full Import of a VVV Database into Katalog | — | — | — |
| [SpecOperationQueue](SpecOperationQueue.md) | OPERATION Queue | Specified | — | — |
| [SpecProgressReport](SpecProgressReport.md) | Progress Reporting | — | — | — |
| [SpecQualityCheck](SpecQualityCheck.md) | QUALITY Check | Draft | backlog | — |
| [SpecSearchList](SpecSearchList.md) | SEARCH List as Input | Approved | — | — |
| [SpecSearchResultsFilters](SpecSearchResultsFilters.md) | SEARCH RESULTS — QUICK FILTER AND CATALOGS FILTER | Approved | planned | — |
| [SpecSelection](SpecSelection.md) | SELECTION — SELECTED DEVICE | Approved | planned | — |
| [SpecStorageIdentity](SpecStorageIdentity.md) | STORAGE Identity | Approved | planned | [TestStorageIdentity](TestStorageIdentity.md) |
| [SpecTheme](SpecTheme.md) | Theme, colour derivation and icon size | Draft | partial | — |
| [SpecValidationRules](SpecValidationRules.md) | Form Validation Rules | Approved | partial | — |
| [SpecVersions](SpecVersions.md) | Version Numbers | — | — | — |

---

## Conventions

- Requirement IDs use a per-page prefix (`STI-`, `DSR-`, `OPQ-`…) and are
  **permanent**: never renumbered, never reused, retired as `[Removed]` rather
  than deleted.
- Requirements are grouped as **Operational** (`-O`, why and for whom),
  **Functional** (`-F`, what the system does) and **Constructional** (`-C`,
  how it is built, and the MUST-NOTs).
- Each row carries a status: `[Implemented]` / `[Planned]` / `[Backlog]` /
  `[Removed]`.
- Specification pages are **English only**.

Page design rules are in [Dev-Documentation](Dev-Documentation.md).
