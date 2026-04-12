# Source Repository

## Summary

This page documents the structure of the source repository of Katalog.

All Katalog variants are consolidated into a single repository, avoiding one repo per "major version":

- **Katalog 2** — QtWidgets + KF6 (current, production)
- **Katalog 3** — QtQuick + KF6 (early development, alpha) see [Development Roadmap / Katalog 3](Development-Roadmap#katalog-3)
- **KatalogWeb** — REST API + web frontend (future, not started)

## Directory Structure

```
katalog/
├── CMakeLists.txt                 ← top-level orchestrator (thin)
├── README.md
├── LICENSE.md
├── CLAUDE.md
│
├── assets/                        ← logos, banners, icons, screenshots (project-level)
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
├── docs/                          ← GitHub Pages static site (must stay at root)
│
├── docs_src/                      ← sources of Pages (using Docusaurus)
│
├── qt_widgets/                    ← Katalog2 (QtWidgets + KF6)
│   ├── CMakeLists.txt
│   ├── CMakeLists_qt5.txt         ← Qt5 legacy build (kept while Qt5 release is maintained)
│   ├── main.cpp
│   ├── mainwindow.*
│   ├── mainwindow_tab_*
│   ├── mainwindow_ui_wrapper_*
│   ├── widgets/                   ← treecombobox, etc.
│   ├── katalog.rc                 ← Windows resource file
│   ├── icons.qrc / images.qrc     ← resource files (icons, images)
│   └── ...
│
├── qt_quick/                      ← Katalog3 (QtQuick + KF6)
│   ├── CMakeLists.txt
│   ├── main.cpp
│   ├── appmanager.cpp/h           ← central QML context object
│   ├── adapters/                  ← QML-specific adapter classes
│   ├── Main.qml
│   ├── Page*.qml
│   └── ...
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
│   └── qt_widgets/                ← desktop files, appdata, wix, etc.
│
├── scripts/                       ← build/deploy scripts
│   ├── linux/
│   └── win64/
│
├── external/                      ← third-party projects for integration study only
│   │                                 (not Katalog source; never built as part of Katalog)
│   ├── luckybackup/
│   └── VVV-1.5-src/
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

Thin orchestrator — build one variant at a time, not all. `core` and `translations` are always built; variants are opt-in via CMake options (`BUILD_QT_WIDGETS` defaults to `ON`):

```cmake
cmake_minimum_required(VERSION 3.16)
project(Katalog LANGUAGES CXX)

# Core is always built
add_subdirectory(core)
add_subdirectory(translations)

# Variants — build one at a time
option(BUILD_QT_WIDGETS "Build Katalog2 - QtWidgets/KF6 desktop" ON)
option(BUILD_QT_QUICK   "Build Katalog3 - QtQuick/KF6 desktop"   OFF)
option(BUILD_WEB_SERVER  "Build KatalogWeb - REST API server"    OFF)

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
cmake .. -DBUILD_QT_WIDGETS=ON    # Katalog2 (default)
cmake .. -DBUILD_QT_QUICK=ON      # Katalog3
cmake .. -DBUILD_WEB_SERVER=ON    # REST API (future)
```

## Key Design Decisions

1. **Translations are shared** — one pool of .ts files used by all variants. Translators see strings from all variants, but maintenance is simpler.

2. **Build one variant at a time** — not all variants every time. Each variant has its own CMakeLists.txt and links against `katalog-core`.

3. **Web approach undecided** — two options were considered:
   - **Option A**: C++ REST server wrapping `katalog-core` (e.g., QHttpServer). Single source of truth, no logic drift, but heavier deployment.
   - **Option B**: Separate backend (Python/Node/Go). Web-native tooling, but two implementations to maintain.
   - Decision deferred. The repo structure supports either approach.

4. **`core/` is fully UI-independent** — no Qt Widgets, Charts, or other UI headers. Stale `Qt6::Widgets`, `Qt6::Charts`, `KF6::XmlGui`, `KF6::Completion` links were removed from `katalog-core` during the migration.

5. **`external/` stays at root, never built** — contains third-party projects (`luckybackup`, `VVV-1.5-src`) used only for integration study. It is never part of the Katalog build and must not be moved into any variant directory.

6. **`docs/` and `specs/` stay at root** — `docs/` is required at the repository root for GitHub Pages; `specs/` contains design documentation sources.

7. **`CMakeLists_qt5.txt` lives in `qt_widgets/`** — kept until Qt5 packaging (e.g. Flathub fallback) is retired. Sits alongside the Qt6 UI CMakeLists.

8. **`katalog.rc` lives in `qt_widgets/`** — Windows resource file; belongs with the UI variant that uses it.

9. **File moves used `git mv`** — preserves per-file history. `git log --follow <file>` traces history across renames. GitHub's default log view does not follow renames automatically, but `git log --follow` and most IDE integrations do.

## Notes

- The `core/` library enforces strict UI independence: no QtWidgets headers, no QDialog, no QMessageBox, etc. Progress callbacks use `std::function<bool(int, int, QString)>`.
- `version.h.in` lives in `core/` as a shared template (same macro structure for all variants), but **each variant owns its version**. The root `CMakeLists.txt` has no `VERSION`. Each variant's `CMakeLists.txt` declares `project(... VERSION x.y)` and runs `configure_file()` on the shared template, writing `version.h` into its own build output directory. `core/CMakeLists.txt` has no version number — it is an internal static library with no published API version.
- The 30 supported languages and existing translation messages should not be disrupted during restructuring.
