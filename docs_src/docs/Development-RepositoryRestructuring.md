# Katalog Mono-Repo Restructuring Plan

## Goal

Consolidate all Katalog variants into a single repository, avoiding one repo per "version":

- **Katalog2** — QtWidgets + KF6 (current, production)
- **Katalog3** — QtQuick + KF6 (separate repo, to be merged later)
- **KatalogWeb** — REST API + web frontend (future, not started)

## Target Directory Structure

```
katalog/
├── CMakeLists.txt                 ← top-level orchestrator (thin)
├── CMakeLists_qt5.txt             ← Qt5 legacy build (kept while Qt5 release is maintained)
├── README.md
├── LICENSE.md
├── CLAUDE.md
│
├── core/                          ← katalog-core (C++ static lib, UI-agnostic)
│   ├── CMakeLists.txt
│   ├── collection.cpp/h
│   ├── catalog.cpp/h
│   ├── database.cpp/h
│   ├── device.cpp/h
│   ├── search*.cpp/h
│   ├── ...
│   └── version.h.in
│
├── qt_widgets/                    ← Katalog2 (QtWidgets + KF6)
│   ├── CMakeLists.txt
│   ├── CMakeLists_qt5.txt         ← Qt5 variant of the UI build
│   ├── main.cpp
│   ├── mainwindow.*
│   ├── mainwindow_tab_*
│   ├── mainwindow_ui_wrapper_*
│   ├── ui/                        ← .ui files
│   ├── widgets/                   ← treecombobox, etc.
│   ├── katalog.rc                 ← Windows resource file
│   └── resources/                 ← .qrc files (icons, images, styles)
│
├── qt_quick/                      ← Katalog3 (QtQuick + KF6, merged later)
│   ├── CMakeLists.txt
│   ├── main.cpp
│   └── qml/
│
├── web/                           ← KatalogWeb (future, undecided tech)
│   ├── server/                    ← REST API (C++ wrapping core, or separate tech)
│   │   ├── CMakeLists.txt         ← (if C++ approach)
│   │   └── main.cpp
│   └── frontend/                  ← Web UI
│       ├── package.json
│       └── src/
│
├── translations/                  ← shared .ts/.qm files (all variants)
│   ├── CMakeLists.txt
│   └── Katalog_*.ts
│
├── packaging/                     ← per-variant packaging
│   ├── qt_widgets/                ← desktop files, appdata, wix, etc.
│   ├── qt_quick/
│   └── web/                       ← Dockerfile, etc.
│
├── scripts/                       ← build/deploy scripts
│   ├── linux/
│   ├── win64/
│   └── macos/
│
├── external/                      ← third-party projects for integration study only
│   │                                 (not Katalog source; never built as part of Katalog)
│   ├── luckybackup/
│   └── VVV-1.5-src/
│
├── docs/                          ← GitHub Pages static site (must stay at root)
│
├── specs/                         ← design specs, documentation sources (stays at root)
│
└── .github/workflows/             ← per-variant CI
    ├── build_qt_widgets_linux.yml
    ├── build_qt_widgets_macos.yml
    ├── build_qt_widgets_windows.yml
    ├── build_qt_quick_linux.yml
    └── build_web.yml
```

## Root CMakeLists.txt Design

Thin orchestrator — build one variant at a time, not all:

```cmake
cmake_minimum_required(VERSION 3.16)
project(Katalog VERSION 2.11 LANGUAGES CXX)

# Shared settings
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Choose what to build (only one at a time typically)
option(BUILD_QT_WIDGETS "Build Katalog2 - QtWidgets/KF6 desktop" OFF)
option(BUILD_QT_QUICK   "Build Katalog3 - QtQuick/KF6 desktop"   OFF)
option(BUILD_WEB_SERVER  "Build KatalogWeb - REST API server"     OFF)

# Core is always built
add_subdirectory(core)
add_subdirectory(translations)

if(BUILD_QT_WIDGETS)
    add_subdirectory(qt_widgets)
endif()
if(BUILD_QT_QUICK)
    add_subdirectory(qt_quick)
endif()
if(BUILD_WEB_SERVER)
    add_subdirectory(web/server)
endif()
```

Build commands:
```bash
cmake .. -DBUILD_QT_WIDGETS=ON    # just Katalog2
cmake .. -DBUILD_QT_QUICK=ON      # just Katalog3
cmake .. -DBUILD_WEB_SERVER=ON    # just the REST API
```

## Key Design Decisions

1. **Translations are shared** — one pool of .ts files used by all variants. Translators see strings from all variants, but maintenance is simpler.

2. **Build one variant at a time** — not all variants every time. Each variant has its own CMakeLists.txt and links against `katalog-core`.

3. **Web approach undecided** — two options were considered:
   - **Option A**: C++ REST server wrapping `katalog-core` (e.g., QHttpServer). Single source of truth, no logic drift, but heavier deployment.
   - **Option B**: Separate backend (Python/Node/Go). Web-native tooling, but two implementations to maintain.
   - Decision deferred. The repo structure supports either approach.

4. **`core/` must be fully UI-independent** — no Qt Widgets, Charts, or other UI headers. The source code separation is already complete; only CMakeLists.txt cleanup was needed (stale `Qt6::Widgets`, `Qt6::Charts`, `KF6::XmlGui`, `KF6::Completion` links removed from `katalog-core`).

5. **`external/` stays at root, never built** — contains third-party projects (`luckybackup`, `VVV-1.5-src`) used only for integration study. It is never part of the Katalog build and must not be moved into any variant directory.

6. **`docs/` and `specs/` stay at root** — `docs/` is required at the repository root for GitHub Pages; `specs/` contains design documentation sources.

7. **`CMakeLists_qt5.txt` is kept** until Qt5 packaging (e.g. Flathub fallback) is retired. It moves to `qt_widgets/` alongside the Qt6 UI CMakeLists.

8. **`katalog.rc` moves to `qt_widgets/`** — Windows resource file; belongs with the UI variant that uses it.

9. **File moves use `git mv`** — preserves per-file history. `git log --follow <file>` is needed to trace history across renames. No significant drawback; GitHub's default log view won't follow renames automatically but `git log --follow` and most IDE integrations do.

## Incremental Migration Steps

Each step is one commit (or small set of commits). Build must stay green at every step.

### Step 1 — Extract `core/`
- `git mv src/core/ core/` (preserves file history)
- Create `core/CMakeLists.txt` for the `katalog-core` static library (no `VERSION`, no `configure_file`)
- Root CMakeLists.txt gains `add_subdirectory(core)` and adjusts include paths
- `src/` still contains all UI code and still builds as before
- Note: source-level UI/core separation was already done; only CMakeLists cleanup was needed (already completed prior to migration)

### Step 2 — Extract `qt_widgets/`
- `git mv` remaining `src/` contents → `qt_widgets/`:
  - `main.cpp`, `mainwindow.*`, `mainwindow_tab_*`, `mainwindow_ui_wrapper_*`
  - `widgets/` (treecombobox etc.)
  - `.ui` files, `.qrc` files
- `git mv katalog.rc qt_widgets/` (Windows resource file)
- Create `qt_widgets/CMakeLists.txt` with `project(Katalog VERSION 2.x)` and the `configure_file()` call for `version.h` (moved from root)
- Root CMakeLists.txt becomes the thin orchestrator with `BUILD_QT_WIDGETS` option and no `VERSION`

### Step 3 — Extract shared `translations/`
- `git mv src/translations/ translations/`
- Create `translations/CMakeLists.txt`
- Both `qt_widgets/` and (future) `qt_quick/` reference the shared translations

### Step 4 — Reorganize packaging, scripts, CI
- `git mv packaging/ packaging/qt_widgets/` (current files are all Qt Widgets variant)
- Move `CMakeLists_qt5.txt` to `qt_widgets/CMakeLists_qt5.txt`
- Update scripts paths
- Update `.github/workflows/` to reflect new directory layout

### Step 5 — Merge Katalog3 (later)
- Import Katalog3 code into `qt_quick/`
- Create `qt_quick/CMakeLists.txt` linking against `katalog-core`
- Adapt Katalog3 to use the shared core (some work needed for QtQuick compatibility)

### Step 6 — Add web variant (much later)
- Create `web/server/` and/or `web/frontend/`
- Choose Option A or B based on experience at that point

## Notes

- The `core/` library enforces strict UI independence: no QtWidgets headers, no QDialog, no QMessageBox, etc. Progress callbacks use `std::function<bool(int, int, QString)>`.
- `version.h.in` lives in `core/` as a shared template (same macro structure for all variants), but **each variant owns its version**. The root `CMakeLists.txt` has no `VERSION`. Each variant's `CMakeLists.txt` declares `project(... VERSION x.y)` and runs `configure_file()` on the shared template, writing `version.h` into its own build output directory. `core/CMakeLists.txt` has no version number — it is an internal static library with no published API version.
- The 30 supported languages and existing translation messages should not be disrupted during restructuring.
