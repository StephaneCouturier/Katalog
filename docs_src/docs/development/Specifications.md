---
version: "2.13"
---

# Specifications

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
| [SpecBackupLuckyBackupProfile](SpecBackupLuckyBackupProfile.md) | BACKUP — GENERATE LUCKYBACKUP PROFILE | Available | complete | — |
| [SpecCardsAndTables](SpecCardsAndTables.md) | CARDS, TABLES AND TREES — SHARED DISPLAY CONVENTIONS | Approved | planned | — |
| [SpecCatalogChecksum](SpecCatalogChecksum.md) | CATALOG — FILE CHECKSUM | Available | — | — |
| [SpecCatalogIncludeExclude](SpecCatalogIncludeExclude.md) | Spec: Catalog Include / Exclude Rules | — | — | — |
| [SpecCatalogMetadata](SpecCatalogMetadata.md) | CATALOG — FILE METADATA | — | — | — |
| [SpecCollection](SpecCollection.md) | SpecCollection — Import / Update across Collections (#750) | — | — | — |
| [SpecCollectionExport](SpecCollectionExport.md) | COLLECTION EXPORT — CONVERT A COLLECTION TO ANOTHER MODE | Approved | planned | — |
| [SpecCollectionIdentity](SpecCollectionIdentity.md) | COLLECTION IDENTITY — NAMING COLLECTIONS IN THE K3 DRAWER | Approved | planned | — |
| [SpecCollectionOpen](SpecCollectionOpen.md) | COLLECTION OPEN — FIRST RUN AND NEW DATABASE | Approved | complete | — |
| [SpecComboBoxes](SpecComboBoxes.md) | COMBO BOXES — WIDTH AND DROP-DOWN LIST | Approved | planned | — |
| [SpecCommandLine](SpecCommandLine.md) | COMMAND LINE — RUNNING KATALOG ACTIONS FROM A TERMINAL | Draft | K2 partial, K3 planned | — |
| [SpecDeviceActiveStatus](SpecDeviceActiveStatus.md) | DEVICE Active Status | Approved | — | — |
| [SpecDeviceComment](SpecDeviceComment.md) | DEVICE Comment | Approved | complete | — |
| [SpecDeviceStorageRoot](SpecDeviceStorageRoot.md) | DEVICE Storage Root | Approved | partial | — |
| [SpecDevicesPage](SpecDevicesPage.md) | DEVICES PAGE — CARDS AND TABLE DISPLAY | Approved | planned | — |
| [SpecDevicesSplit](SpecDevicesSplit.md) | DEVICES: Split a Catalog | — | — | — |
| [SpecExplore](SpecExplore.md) | EXPLORE — DIRECTORY TREE | Approved | complete | — |
| [SpecHostedSecurity](SpecHostedSecurity.md) | HOSTED SECURITY — ONLY LOCAL OR PRIVATE-NETWORK SERVERS | Approved | K2 complete, K3 planned | — |
| [SpecIconButtons](SpecIconButtons.md) | ICON BUTTONS — TOOLTIPS ON ICON-ONLY BUTTONS | Approved | planned | — |
| [SpecImportVVV2K](SpecImportVVV2K.md) | Feature Specification: VVV2K — Full Import of a VVV Database into Katalog | — | — | — |
| [SpecKeyboardShortcuts](SpecKeyboardShortcuts.md) | KEYBOARD SHORTCUTS | Approved | complete | — |
| [SpecK2Deployment](SpecK2Deployment.md) | K2 DEPLOYMENT — STARTUP OF THE LINUX APPIMAGE ON ANY DESKTOP | Approved | mostly complete | — |
| [SpecK3Deployment](SpecK3Deployment.md) | K3 DEPLOYMENT — QML MODULES AND PLATFORM INTEGRATION IN PACKAGED BUILDS | Approved | mostly complete | — |
| [SpecK3ParityGaps](SpecK3ParityGaps.md) | K3 PARITY GAPS — K2 FEATURES NOT YET IN K3 | Reference | — | — |
| [SpecLanguages](SpecLanguages.md) | LANGUAGES — INTERFACE LANGUAGE SELECTION | Approved | complete | — |
| [SpecPageLayout](SpecPageLayout.md) | PAGE LAYOUT — SCROLLBAR MARGIN AND SECTION TITLES | Approved | planned | — |
| [SpecMimeVerification](SpecMimeVerification.md) | MIME VERIFICATION — VERIFY MIME TYPES ON A CATALOG | Approved | planned | — |
| [SpecOperationQueue](SpecOperationQueue.md) | OPERATION Queue | Specified | — | — |
| [SpecProgressReport](SpecProgressReport.md) | Progress Reporting | — | — | — |
| [SpecQualityCheck](SpecQualityCheck.md) | QUALITY Check | Draft | planned (phase 1) | — |
| [SpecSearchList](SpecSearchList.md) | SEARCH List as Input | Approved | — | — |
| [SpecSearchResults](SpecSearchResults.md) | SEARCH RESULTS | Approved | planned | — |
| [SpecSearchResultsFilters](SpecSearchResultsFilters.md) | SEARCH RESULTS — QUICK FILTER AND CATALOGS FILTER | Approved | planned | — |
| [SpecSelection](SpecSelection.md) | SELECTION — SELECTED DEVICE | Approved | planned | — |
| [SpecSettingsPage](SpecSettingsPage.md) | SETTINGS PAGE — APPLICATION AND COLLECTION TABS | Approved | planned | — |
| [SpecStorageIdentity](SpecStorageIdentity.md) | STORAGE Identity | Approved | planned | [TestStorageIdentity](TestStorageIdentity.md) |
| [SpecTheme](SpecTheme.md) | Theme, colour derivation and icon size | Draft | partial | — |
| [SpecTypography](SpecTypography.md) | TYPOGRAPHY — TEXT SIZE DERIVED FROM THE SYSTEM FONT | Approved | partial | — |
| [SpecValidationRules](SpecValidationRules.md) | Form Validation Rules | Approved | partial | — |
| [SpecVersionCheck](SpecVersionCheck.md) | VERSION CHECK — NEW VERSION AVAILABLE | Approved | complete | — |
| [SpecVersions](SpecVersions.md) | Version Numbers | — | — | — |
| [SpecWindowGeometry](SpecWindowGeometry.md) | WINDOW GEOMETRY — SIZE AND POSITION OF THE K3 WINDOW | Approved | partial | — |

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
