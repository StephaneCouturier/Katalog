# CLAUDE.md

This file provides guidance to Claude Code (claude.ai/code) when working with code in this repository.

> **PROMPT SHORTHANDS:**
> - **K2** at the start of a prompt → Katalog 2, the Qt Widgets version (`qt_widgets/`)
> - **K3** at the start of a prompt → Katalog 3, the Qt Quick / QML / Kirigami version (`qt_quick/`)

> **CRITICAL — File access boundary:**
> - **Full access:** `/home/stephane/Documents/Informatique/Katalog/_Source_Katalog/` — always allowed, no restrictions.
> - **Forbidden:** any other path under `/home/` — never access, read, search, or browse.
> - **System files** (e.g. `/usr/include/KF6/`, `/usr/lib/`, etc.) — request explicit user authorization before accessing, for debugging purposes only.

> **CRITICAL — File safety:**
> - **NEVER delete any file** without the user explicitly and unambiguously saying to delete it.

> **CRITICAL — Core class changes:**
> - **NEVER add or modify methods in `core/` classes** without explicit user approval first.
> - Before proposing a new core method, always check: does the equivalent SQL/logic already exist in K2's UI layer? If so, state where it is and ask for approval to move it to core.
> - The core library was deliberately cleaned up (all SQL moved from K2 UI to core) before K3 was started. Any further change to `core/` is a deliberate architectural decision and must be approved in advance.

> **CRITICAL — UI structure:**
> - **NEVER create a new source file** (`.cpp`, `.h`, `.qml`) for a new tab (Katalog2), section, or Page (Katalog3) without the user explicitly requesting it.
> - If unsure where new code belongs, **ask before creating new files**.
> - **K2 UI is in maintenance mode:** keep K2 changes minimal (bug fixes, small tweaks). New major UI features are designed and built in K3 only, and only after the full K2 feature set has been migrated to K3.

> **CRITICAL — K3 migration process:**
> - **K3 is a portage of K2, not a new design.** Almost every K3 page/feature already exists in K2. Before proposing or asking anything about a K3 feature, **assume K2 already implements it and go find it.** Investigate the K2 *why / what / how* first, then propose the best portage. **Do NOT ask the user open design questions (e.g. multiple-choice on behavior, persistence, or triggers) for behavior that K2 already defines** — read K2, state what it does, and propose the matching K3 portage. Only ask the user when K2 and K3 architectures genuinely diverge (e.g. K2 side-panel vs K3 page-stack) and a real decision remains.
> - **K2 ↔ K3 naming can diverge — search by concept, not just by label.** The biggest trap: the **K3 "Selection" page is K2's "Filters" panel.** All K2 objects are named `Filters_*` / `splitter_widget_Filters` and the code lives in `qt_widgets/mainwindow_tab_filters.cpp` (header: *"methods for the SELECTION panel"*, doc `docs/Features/Selection`). Searching K2 for "Selection" will NOT find it. When you can't locate a K2 equivalent by name, search by the file's documented purpose and the `docs/Features/<Page>` mapping before concluding it's new.
>   - **Selection panel show/hide (reference example):** K2 has a collapsible Selection panel. `mainwindow_tab_filters.cpp:38-54` — Hide hides `splitter_widget_Filters` + shows `main_widget_ShowFilters` and writes `Settings/ShowHideFilters = "go-next"`; Show reverses it with `"go-previous"`. Restored on startup at `mainwindow_setup.cpp:293-301`. Persistence is in **`collection->settingsFilePath`** via `QSettings(collection->settingsFilePath, QSettings::IniFormat)` — the collection's `.ini`, not the default app QSettings.
> - Before implementing any K3 feature that mirrors a K2 feature, **read ALL of the K2 source code** for that feature: the loading method, the display/model population, every user interaction handler (including destructive actions and their confirmation dialogs), side-effects triggered on search/save/complete events, and the restoration/apply logic. Never implement a partial migration.
> - K3 must achieve **full feature parity** with K2 for each migrated feature. Checklist for every migrated feature:
>   1. **Display** — same data shown to the user (all fields, not just a subset)
>   2. **Destructive actions** — every Clear/Delete/Keep button must have a confirmation dialog matching K2
>   3. **Lifecycle hooks** — any side-effect K2 triggers on search start, search complete, or save (e.g. saving to history table, persisting to CSV) must be replicated in K3
>   4. **Restore/apply** — clicking a history/saved entry must restore all criteria, not just the visible ones

> **CRITICAL — User-visible text:**
> - **Never add, change, or delete any user-visible string** (`tr`/`qsTr`/label/message) without quoting the exact before/after and getting explicit per-string approval — even if trivial or a side effect. Matching is byte-for-byte: one character drops the string to English across all 30 languages. Porting K2→K3: copy the K2 source verbatim. Full architecture & rationale: `docs_src/docs/SpecLanguages.md`.
> - If a layout is too wide, solve it with layout changes only. Never shorten label text as a workaround.
> - K3 labels must stay in sync with K2 unless the user explicitly requests a change in both.
> - **Before writing any K3 user-visible text** (labels, status messages, progress reports, dialog text, tooltips, notifications) **always read the K2 equivalent first** and reuse its exact `tr()` string. Creating a new string when K2 already has one wastes a translation slot across 30 languages.
> - **Progress/status messages in K3 MUST follow `SpecProgressReport.md`** (`docs_src/docs/SpecProgressReport.md`). Every status bar message must use `StatusBarMessageBuilder`. NEVER write a raw string where the builder is the specification. Read the spec before implementing any progress or status message.

> **CRITICAL — Version context:**
> - Last **released** version: **2.11**
> - Current **development** version: **2.12** (branch `katalog_development`)
> - Database migrations 2.12 were introduced **during** the 2.12 development cycle and have **never been shipped**. Any field added by those migrations can be changed in-place (schema + migration ALTER TABLE) — no additional migration step is needed.
> - **Rule:** When a new DB field is introduced in the current development version, note it here so future work knows it has not been released yet and can be edited directly rather than adding a new migration.
>
> **New fields added in 2.12 (unreleased — edit in place, no extra migration needed):**
> - `catalog.catalog_include_sub_dir` INTEGER DEFAULT 1 — whether the catalog scanner recurses into subdirectories; `false` for the `(root)` split catalog in the Devices Split feature

## Project Overview

Catalog your devices to search, analyze, and backup your files.

**Tech Stack:** C++17, Qt6 (Widgets, QtQuick, Sql, Charts), KDE Frameworks 6 (KF6)
**Platforms:** Linux (KDE Plasma), Windows, macOS

## Build Commands

```bash
# Configure and build (from project root)
mkdir build && cd build
cmake .. -GNinja -DCMAKE_BUILD_TYPE=Release
ninja

# Debug build
cmake .. -GNinja -DCMAKE_BUILD_TYPE=Debug
ninja

# Update translation source files (.ts)
ninja translations_lupdate

# Compile translations (.qm)
ninja translations_lrelease
```

## Architecture

### Three-Part Structure

| Directory | Role |
|-----------|------|
| `core/` | UI-agnostic business logic, compiled as `katalog-core` static library |
| `qt_widgets/` | K2 — Qt Widgets / KXmlGui UI, linked against `katalog-core` |
| `qt_quick/` | K3 — Qt Quick / QML / Kirigami UI, linked against `katalog-core` |

### Core Module Organization (`core/`)

| Component | Files | Purpose |
|-----------|-------|---------|
| Data Models | `collection`, `device`, `catalog`, `storage`, `database` | Domain objects and SQLite persistence |
| Search | `search`, `searchjob`, `searchjobstoppable`, `searchmanager`, `searchprogressmanager`, `searchresultsthrottler` | Search criteria, execution, cancellation, progress |
| Catalog Ops | `catalogjob`, `catalogjobstoppable`, `catalogmanager`, `catalogprogressmanager` | Catalog creation and updates |
| Device Ops | `devicejobstoppable`, `devicemanager`, `deviceupdatemanager` | Device management and updates |
| File Processing | `filemetadata`, `filetypemapping`, `parallelmetadataextractor`, `filechecksum` | Metadata extraction and checksums |

### K2 UI Layer Organization (`qt_widgets/`)

The main window (`mainwindow.h/cpp`) is split across multiple implementation files by tab:

- `mainwindow_tab_create.cpp` - Catalog creation
- `mainwindow_tab_search_ui.cpp` / `_pr.cpp` - Search interface and processing
- `mainwindow_tab_device_ui.cpp` / `_pr.cpp` - Device management
- `mainwindow_tab_explore.cpp` - Catalog exploration
- `mainwindow_tab_statistics.cpp` - Statistics display
- `mainwindow_tab_settings.cpp` / `_exp.cpp` - Settings and export
- `mainwindow_tab_backup.cpp` - Backup functionality
- `mainwindow_tab_filters.cpp` - Filter configuration
- `mainwindow_tab_tags.cpp` - Tag management

### Key Design Patterns

- **Stoppable Jobs**: Long-running operations (`*Stoppable` classes) support cancellation
- **Progress Managers**: Dedicated `*ProgressManager` classes track operation progress
- **Qt Model-View**: Standard item models with proxy models for filtering/sorting
- **UI Wrappers**: `mainwindow_ui_wrapper_*.cpp` files abstract UI component access

### UI / Core Boundary — STRICT RULE

The two-layer separation is enforced for future **QtQuick compatibility**. The goal is that core can be reused without modification if the UI is ever ported from Qt Widgets to QtQuick/QML.

**What belongs in `src/core/`:**
- All SQL queries (`QSqlQuery`) — wrap in methods on the relevant domain class (`Catalog`, `Device`, `Collection`, `Tag`, `FileMetadata`, etc.)
- All domain object operations: creating, inserting, updating, deleting `Device`, `Catalog`, `Storage`, `Tag`, etc. — even if they don't contain raw SQL
- File I/O for data files
- Business logic, data transformations, algorithms
- Domain-specific sorting/formatting logic (e.g. metadata field priority order)
- Progress callbacks via `std::function<bool(int, int, QString)>`

**What belongs in `src/` (UI layer only):**
- Widget construction, layout, signal/slot wiring
- `tr()` translations and all user-visible strings
- HTML/CSS generation for display
- Qt Charts series building
- Error dialogs and user confirmations
- Calling core methods and refreshing UI from their results

**Enforcement rules:**
- `core/` files must **never** include Qt Widgets headers (`QWidget`, `QDialog`, `QMessageBox`, etc.)
- `qt_widgets/mainwindow_tab_*.cpp` files must **not** contain raw `QSqlQuery` — delegate to a core method instead
- `qt_widgets/mainwindow_tab_*.cpp` files must **not** call domain mutating methods directly (`insertDevice()`, `deleteDevice()`, `generateNextDeviceID()`, `saveDevice()`, `insertPhysicalStorageGroup()`, etc.) — wrap multi-step domain operations in a single core method
- `qt_quick/` files must **not** contain raw `QSqlQuery` — delegate to a core method instead
- Core methods return plain Qt value types (`QString`, `QList`, `QStringList`, `QPair`, etc.), never widget types
- **When reviewing any `mainwindow_tab_*.cpp` method:** ask "does this touch domain objects or the database?" — if yes, it belongs in core regardless of whether it uses raw SQL

**Reference implementations (established patterns):**
- Single-row DB lookup: `Catalog::getFileChecksum(fileName, folderPath)` → `QString`
- Aggregate query: `Device::getMaxHierarchyDepth(connectionName)` → `int`
- Model self-loading: `Tag::loadFromDatabase(connectionName, filterName)`
- Structured data for display: `FileMetadata::parseExtendedMetadataFields(jsonObj)` → `QList<QPair<QString,QString>>`
- Collection-level query: `Collection::getExcludeDirectories()` → `QStringList`

### K3 UI Layer Organization (`qt_quick/`)

| File / Directory | Role |
|-----------------|------|
| `main.cpp` | App entry point — creates AppManager, registers QML types, sets context properties |
| `appmanager.h/cpp` | Central QML context object: exposes core to QML via Q_PROPERTY / Q_INVOKABLE / signals |
| `adapters/search.h/cpp` | QML-visible Search adapter inheriting `SearchJobStoppable` |
| `adapters/devicelistmodel.h/cpp` | `QAbstractListModel` exposing the device list to QML |
| `Main.qml` | Application window, GlobalDrawer, page stack, dialogs |
| `PageSelection*.qml` | Device selection page and card delegate |
| `PageSearch*.qml` | Search form and results pages |
| `PageDevices*.qml` | Devices page (placeholder) |
| `PageSettings.qml` | Settings page: current connection status + hosted DB config |
| `version.h.in` | CMake-generated version header |

**K3 build commands:**
```bash
cd qt_quick
mkdir -p build/Debug && cd build/Debug
cmake ../.. -GNinja -DCMAKE_BUILD_TYPE=Debug -DBUILD_QT_QUICK=ON
ninja
```

**K3 current state (alpha 1):**
- Working: Open Collection (Memory/File/SQLite/Hosted), device selection, search, about, alpha warning
- Placeholder pages: Devices, Explore, Create, Statistics, Tags
- `AppManager` carries too much orchestration logic (reconnect, settings management) that should eventually move to a core `DatabaseManager` — acceptable for now

**K3 / K2 key differences:**
- K3 uses live `reconnectToDatabase()` — no app restart needed when switching collection
- K2 saves settings + restarts; K3 saves settings + reconnects in-place
- Settings `.ini` keys are intentionally aligned so both versions share the same settings file
- K3 `qt_quick/adapters/` contains QML-specific adapter classes; `core/` is shared unchanged
- Password is stored plain-text in `.ini` (same as K2) and must be loaded back to pre-fill the Settings form

## Dependencies

**Qt6 Components:**
- Core, Widgets, Gui, Sql (SQLite), Charts, Network, LinguistTools

**KDE Frameworks 6:**
- CoreAddons, XmlGui, Config, FileMetaData, Completion

### KDE / KF6 Usage Rules

- **Maximize use of existing KF6 libraries** — especially those already integrated, to avoid cross-platform build risk
- **Approval required before adding any new library** — adding a new Qt6 module or KF6 component to `CMakeLists.txt` requires explicit user approval before writing code
- **Cross-platform simplicity** — prefer Qt6 built-ins over platform-specific APIs; keep CMake configuration minimal; the Windows build must remain viable

## Database

Multiple connection modes: file (SQLite file), memory (SQLite), hosted (Mariadb/MySQL). Schema versioning handles migrations. Memeory mode: Separate files for: devices, catalogs, storage, statistics, search history.

### Memory Mode Caveat

**IMPORTANT:** In Memory database mode, catalog data (files, folders) is NOT pre-loaded in the in-memory database. It must be explicitly loaded from CSV/idx files before querying. When writing code that queries the `file`, `filetemp`, or `folder` tables, always check `collection->databaseMode == "Memory"` and call the appropriate load method first:
- `catalog->loadFoldersToTable()` — loads folder data from `.folders.idx` into the `folder` table
- `catalog->loadCatalogFileListToTable(mutex, stopRequested)` — loads file data from `.idx` into the `file` table

In File/Hosted mode, the data is already in the SQLite database and no pre-loading is needed. Forgetting this check is a recurring source of bugs — features work in File mode but return empty results in Memory mode.

## Translation

**30 languages** supported. Translation files in `src/translations/`. Uses Qt Linguist (lupdate/lrelease). Files compiled to `.qm` format and bundled via `translations.qrc`.

**Rules:**
- **Do not change existing `tr()` strings** — any change breaks all 30 translations at once
- **Reuse existing messages** wherever semantically appropriate before creating new ones
- All new user-visible strings must be wrapped in `tr()`; run `ninja translations_lupdate` after adding them

## Documentation Requirements

Any change that affects users or future developers must be documented:

- **User-facing feature or change** → document in `docs_src/docs/` (the feature page for the relevant screen)
- **Technical practice, architecture decision, limitation, or risk** → document in `docs_src/docs/` as a Markdown file with the `Spec` prefix (e.g. `SpecBackupStrategy.md`)

### Documentation page rules

All documentation pages must follow the design guidelines in `docs_src/docs/Development-Documentation.md`. Key rules:

- Every page starts with a YAML frontmatter block (`id`, `title`, `description`), then `# Title`, then Status + Version shields (shields.io)
- **Feature pages are for end-users**: no code, no method names, no variable names, no source filenames
- **When updating or creating a feature page, provide translations for all languages: French (`i18n/fr/`) and Czech (`i18n/cs/`)**
- Update the `Version` badge to the current release version whenever a translated page is updated
- Development and Specification pages are **English only** — do not create FR/CS translations for them
- Published site: https://stephanecouturier.github.io/Katalog/
