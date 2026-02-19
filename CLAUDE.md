# CLAUDE.md

This file provides guidance to Claude Code (claude.ai/code) when working with code in this repository.

## Project Overview

Katalog is a Qt6/KDE desktop application for managing file catalogs across storage devices. Users can create catalogs from folders/drives, search files when devices are disconnected, explore catalog contents offline, and find duplicates/differences between files.

**Tech Stack:** C++17, Qt6 (Widgets, Sql, Charts), KDE Frameworks 6 (KF6)
**Platforms:** Linux (KDE Plasma), Windows

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

### Two-Layer Design

1. **Core Library (`src/core/`)** - UI-agnostic business logic compiled as `katalog-core` static library
2. **UI Layer (`src/`)** - Qt/KDE UI components, linked against `katalog-core`

### Core Module Organization (`src/core/`)

| Component | Files | Purpose |
|-----------|-------|---------|
| Data Models | `collection`, `device`, `catalog`, `storage`, `database` | Domain objects and SQLite persistence |
| Search | `search`, `searchjob`, `searchjobstoppable`, `searchmanager`, `searchprogressmanager`, `searchresultsthrottler` | Search criteria, execution, cancellation, progress |
| Catalog Ops | `catalogjob`, `catalogjobstoppable`, `catalogmanager`, `catalogprogressmanager` | Catalog creation and updates |
| Device Ops | `devicejobstoppable`, `devicemanager`, `deviceupdatemanager` | Device management and updates |
| File Processing | `filemetadata`, `filetypemapping`, `parallelmetadataextractor`, `filechecksum` | Metadata extraction and checksums |

### UI Layer Organization (`src/`)

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

## Dependencies

**Qt6 Components:**
- Core, Widgets, Gui, Sql (SQLite), Charts, Network, LinguistTools

**KDE Frameworks 6:**
- CoreAddons, XmlGui, Config, FileMetaData, Completion

## Database

SQLite with multiple connection modes (file, hosted, memory). Schema versioning handles migrations. Separate files for: devices, catalogs, storage, statistics, search history.

### Memory Mode Caveat

**IMPORTANT:** In Memory database mode, catalog data (files, folders) is NOT pre-loaded in the in-memory database. It must be explicitly loaded from CSV/idx files before querying. When writing code that queries the `file`, `filetemp`, or `folder` tables, always check `collection->databaseMode == "Memory"` and call the appropriate load method first:
- `catalog->loadFoldersToTable()` — loads folder data from `.folders.idx` into the `folder` table
- `catalog->loadCatalogFileListToTable(mutex, stopRequested)` — loads file data from `.idx` into the `file` table

In File/Hosted mode, the data is already in the SQLite database and no pre-loading is needed. Forgetting this check is a recurring source of bugs — features work in File mode but return empty results in Memory mode.

## Translation

24+ languages supported. Translation files in `src/translations/`. Uses Qt Linguist (lupdate/lrelease). Files compiled to `.qm` format and bundled via `translations.qrc`.
