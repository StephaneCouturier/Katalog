# CLAUDE.md

This file provides guidance to Claude Code (claude.ai/code) when working with code in this repository.

> **PROMPT SHORTHANDS:**
> - **K2** at the start of a prompt → Katalog 2, the Qt Widgets version (`qt_widgets/`)
> - **K3** at the start of a prompt → Katalog 3, the Qt Quick / QML / Kirigami version (`qt_quick/`)

> **CRITICAL — File safety:**
> - **NEVER delete any file** without the user explicitly and unambiguously saying to delete it.

> **CRITICAL — UI structure:**
> - **NEVER create a new source file** (`.cpp`, `.h`, `.qml`) for a new tab (Katalog2), section, or Page (Katalog3) without the user explicitly requesting it.
> - If unsure where new code belongs, **ask before creating new files**.
> - **K2 UI is in maintenance mode:** keep K2 changes minimal (bug fixes, small tweaks). New major UI features are designed and built in K3 only, and only after the full K2 feature set has been migrated to K3.

> **CRITICAL — User-visible text:**
> - **NEVER alter existing `tr()` strings or any user-visible label text** — not for brevity, not for layout reasons, not for any reason. Any change breaks all 30 translations and diverges K3 from K2.
> - If a layout is too wide, solve it with layout changes only. Never shorten label text as a workaround.
> - K3 labels must stay in sync with K2 unless the user explicitly requests a change in both.

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

**Enforcement rules:**
- `core/` files must **never** include Qt Widgets headers (`QWidget`, `QDialog`, `QMessageBox`, etc.)
- `qt_widgets/mainwindow_tab_*.cpp` files must **not** contain raw `QSqlQuery` — delegate to a core method instead
- `qt_quick/` files must **not** contain raw `QSqlQuery` — delegate to a core method instead
- Core methods return plain Qt value types (`QString`, `QList`, `QStringList`, `QPair`, etc.), never widget types

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

SQLite with multiple connection modes (file, hosted, memory). Schema versioning handles migrations. Separate files for: devices, catalogs, storage, statistics, search history.

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
