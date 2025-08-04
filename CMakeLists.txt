cmake_minimum_required(VERSION 3.16)
project(Katalog VERSION 2.7 LANGUAGES CXX)

# Set C++ standard
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

set(CMAKE_CXX_FLAGS_DEBUG "$ENV{CXXFLAGS} -O0 -Wall -g2")
set(CMAKE_CXX_FLAGS_RELEASE "$ENV{CXXFLAGS} -O3 -Wall")

# Windows-specific: Better dependency tracking
if(WIN32)
    # Force CMake to use file timestamps for dependency checking
    set(CMAKE_DEPENDS_USE_COMPILER FALSE)
    set(CMAKE_DEPENDS_CHECK_CXX 1)

    # Use Ninja generator for better incremental builds on Windows
    if(CMAKE_GENERATOR STREQUAL "Ninja")
        set(CMAKE_EXPORT_COMPILE_COMMANDS ON)
    endif()
endif()

# Find required Qt6 packages
find_package(Qt6 REQUIRED COMPONENTS
    Core
    Widgets
    Gui
    Sql
    Charts
    Network
    LinguistTools      # Required for translations
)

qt_standard_project_setup()

# Find KF6 components
find_package(KF6CoreAddons REQUIRED)
find_package(KF6I18n REQUIRED)
find_package(KF6XmlGui REQUIRED)
find_package(KF6Config REQUIRED)
find_package(KF6IconThemes REQUIRED)

# Enable Qt's automatic processing
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTORCC ON)

# Add Qt6 features check (equivalent to requires() in .pro)
if(NOT TARGET Qt6::Widgets)
    message(FATAL_ERROR "Qt6 Widgets not found - tableview, treeview, listview required")
endif()

# Add compile definitions
add_definitions(-DQT_DEPRECATED_WARNINGS)
# Uncomment if you want to disable deprecated APIs
# add_definitions(-DQT_DISABLE_DEPRECATED_BEFORE=0x060000)

# Core library sources

set(CORE_HEADERS
    src/core/collection.h
    src/core/commandline.h
    src/core/catalog.h
    src/core/database.h
    src/core/device.h
    src/core/search.h
    src/core/searchjob.h
    src/core/searchjobstoppable.h
    src/core/searchmanager.h
    src/core/searchprogressmanager.h
    src/core/searchresultsthrottler.h
    src/core/storage.h
    src/core/tag.h
    src/core/language.h
)

set(CORE_SOURCES
    src/core/collection.cpp
    src/core/commandline.cpp
    src/core/catalog.cpp
    src/core/database.cpp
    src/core/device.cpp
    src/core/search.cpp
    src/core/searchjob.cpp
    src/core/searchjobstoppable.cpp
    src/core/searchmanager.cpp
    src/core/searchprogressmanager.cpp
    src/core/searchresultsthrottler.cpp
    src/core/storage.cpp
    src/core/tag.cpp
    src/core/language.cpp
)

# ALL mainwindow implementation files - compile them all separately
# This ensures they're treated as actual compilation units, not just included files
set(MAINWINDOW_IMPL_SOURCES
    src/mainwindow_setup.cpp
    src/mainwindow_tab_backup.cpp
    src/mainwindow_tab_create.cpp
    src/mainwindow_tab_device_pr.cpp
    src/mainwindow_tab_device_ui.cpp
    src/mainwindow_tab_explore.cpp
    src/mainwindow_tab_filters.cpp
    src/mainwindow_tab_search_pr.cpp
    src/mainwindow_tab_search_ui.cpp
    src/mainwindow_tab_settings.cpp
    src/mainwindow_tab_settings_exp.cpp
    src/mainwindow_tab_statistics.cpp
    src/mainwindow_tab_tags.cpp
)

# Main application sources (excluding the included ones)
set(MAIN_SOURCES
    src/devicemappingview.cpp
    src/devicetreeview.cpp
    src/directorytreeitem.cpp
    src/directorytreemodel.cpp
    src/exploretreeitem.cpp
    src/exploretreemodel.cpp
    src/exploretreeview.cpp
    src/filesview.cpp
    src/main.cpp
    src/mainwindow.cpp
    src/mainwindow_ui_wrapper_device.cpp
    src/mainwindow_ui_wrapper_storage.cpp
    src/mainwindow_ui_wrapper_catalog.cpp
    ${MAINWINDOW_IMPL_SOURCES}  # Always include these as separate compilation units
)

set(MAIN_HEADERS
    src/devicemappingview.h
    src/devicetreeview.h
    src/directorytreeitem.h
    src/directorytreemodel.h
    src/exploretreeitem.h
    src/exploretreemodel.h
    src/exploretreeview.h
    src/filesview.h
    src/mainwindow.h
    src/mainwindow_ui_wrapper_device.h
    src/mainwindow_ui_wrapper_storage.h
    src/mainwindow_ui_wrapper_catalog.h
)

# UI files
set(UI_FILES
    src/mainwindow.ui
)

# Translation files
set(TS_FILES
    src/translations/Katalog_bg_BG.ts
    src/translations/Katalog_cz_CZ.ts
    src/translations/Katalog_da_DK.ts
    src/translations/Katalog_de_DE.ts
    src/translations/Katalog_en_US.ts
    src/translations/Katalog_el_GR.ts
    src/translations/Katalog_es_ES.ts
    src/translations/Katalog_et_EE.ts
    src/translations/Katalog_fr_FR.ts
    src/translations/Katalog_fi_FI.ts
    src/translations/Katalog_hi_IN.ts
    src/translations/Katalog_hu_HU.ts
    src/translations/Katalog_it_IT.ts
    src/translations/Katalog_ja_JP.ts
    src/translations/Katalog_lt_LT.ts
    src/translations/Katalog_lv_LV.ts
    src/translations/Katalog_nl_NL.ts
    src/translations/Katalog_nb_NO.ts
    src/translations/Katalog_pl_PL.ts
    src/translations/Katalog_pt_PT.ts
    src/translations/Katalog_ro_RO.ts
    src/translations/Katalog_sv_SE.ts
    src/translations/Katalog_zh_CN.ts
    src/translations/Katalog_sk_SK.ts
    src/translations/Katalog_si_SI.ts
    src/translations/Katalog_sr_RS.ts
    src/translations/Katalog_uk_UA.ts
    src/translations/Katalog_hr_HR.ts
)

# Make sure LinguistTools is found (should already be in your find_package)
find_package(Qt6 REQUIRED COMPONENTS LinguistTools)

# Set up translations with both lupdate and lrelease support
if(${Qt6_VERSION} VERSION_GREATER_EQUAL "6.2")
    # Modern Qt6 approach with lupdate support
    qt6_add_translations(Katalog
        TS_FILES ${TS_FILES}
        SOURCES ${MAIN_SOURCES} ${CORE_SOURCES} ${MAIN_HEADERS} ${CORE_HEADERS} ${UI_FILES}
        LUPDATE_OPTIONS "-extensions" "cpp,h,ui" "-locations" "relative"
        LRELEASE_OPTIONS "-compress" "-nounfinished"
    )

    # Create aliases for the auto-generated targets for easier access
    add_custom_target(translations_lupdate
        DEPENDS Katalog_lupdate
        COMMENT "Alias for updating translation files"
    )

    add_custom_target(translations_lrelease
        DEPENDS Katalog_lrelease
        COMMENT "Alias for compiling translation files"
    )
endif()

# Custom lupdate target for manual control
add_custom_target(lupdate_manual
    COMMAND Qt6::lupdate ${MAIN_SOURCES} ${CORE_SOURCES} ${MAIN_HEADERS} ${CORE_HEADERS} ${UI_FILES} -ts ${TS_FILES}
    WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
    COMMENT "Manually updating translation source files"
)

# Optional: Create a target to copy translations manually
add_custom_target(translations_copy
    COMMENT "Copy .qm files from build to source directory"
)

foreach(ts_file ${TS_FILES})
    get_filename_component(qm_filename ${ts_file} NAME_WE)
    set(qm_file_build "${CMAKE_CURRENT_BINARY_DIR}/${qm_filename}.qm")
    set(qm_file_source "${CMAKE_CURRENT_SOURCE_DIR}/src/translations/${qm_filename}.qm")

    add_custom_command(TARGET translations_copy POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy_if_different
                ${qm_file_build}
                ${qm_file_source}
        COMMENT "Copying ${qm_filename}.qm to source directory"
    )
endforeach()

# Resource files
set(QRC_FILES
    src/icons.qrc
    src/images.qrc
    src/Resources.qrc
    src/styles.qrc
    src/translations.qrc
)

# Create the core library (reusable, UI-agnostic)
add_library(katalog-core STATIC ${CORE_SOURCES} ${CORE_HEADERS})

target_link_libraries(katalog-core PUBLIC
    Qt6::Core
    Qt6::Gui     # Required for QStandardItemModel and other model classes
    Qt6::Sql
    Qt6::Network
    Qt6::Widgets # Temporarily needed for QApplication in catalog.cpp
    Qt6::Charts  # Required for QDateTimeAxis used in mainwindow.h
    KF6::CoreAddons
    KF6::I18n
    KF6::XmlGui
)

target_include_directories(katalog-core PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}/src
    ${CMAKE_CURRENT_SOURCE_DIR}
    ${KF6CoreAddons_INCLUDE_DIRS}
    ${KF6I18n_INCLUDE_DIRS}
    ${KF6XmlGui_INCLUDE_DIRS}
)

# Create the main executable with all sources at once
qt6_add_executable(Katalog
    ${MAIN_SOURCES}
    ${MAIN_HEADERS}
    ${UI_FILES}
    ${QRC_FILES}
)

# Link libraries to the main executable (using keyword signature to match Qt6)
target_link_libraries(Katalog PRIVATE
    katalog-core
    Qt6::Core
    Qt6::Widgets
    Qt6::Gui
    Qt6::Sql
    Qt6::Charts
    Qt6::Network
    KF6::CoreAddons
    KF6::I18n
    KF6::XmlGui
    KF6::ConfigCore
    KF6::IconThemes
)

# To avoid threat detection of a separate freetype dll.
if(WIN32)
    target_compile_definitions(Katalog PRIVATE
        QT_NO_FREETYPE
        -DNOMINMAX
        -DWIN32_LEAN_AND_MEAN
    )
endif()

set_target_properties(katalog-core PROPERTIES
    AUTOMOC_MOC_OPTIONS "-I${CMAKE_CURRENT_SOURCE_DIR}/src"
)

# Add include directories to help with relative paths
target_include_directories(Katalog PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/src
    ${CMAKE_CURRENT_SOURCE_DIR}
    ${KF6CoreAddons_INCLUDE_DIRS}
    ${KF6I18n_INCLUDE_DIRS}
    ${KF6XmlGui_INCLUDE_DIRS}
    ${KF6Config_INCLUDE_DIRS}
)

# Platform-specific settings
if(WIN32)
    # Enable RC compiler for Windows resources
    enable_language(RC)

    # Set application icon for Windows
    set_target_properties(Katalog PROPERTIES
        WIN32_EXECUTABLE TRUE
    )

    # Windows-specific: Force dependency checking
    set_target_properties(Katalog PROPERTIES
        ADDITIONAL_CLEAN_FILES "${CMAKE_CURRENT_BINARY_DIR}/CMakeFiles/Katalog.dir"
    )

    # Force CMake to always check file timestamps on Windows
    set(CMAKE_DEPENDS_CHECK_CXX 1)

    # Windows-specific compiler flags to help with included files
    target_compile_definitions(Katalog PRIVATE
        -DNOMINMAX  # Prevent min/max macro conflicts
        -DWIN32_LEAN_AND_MEAN  # Reduce Windows header bloat
    )

    # Add Windows resource file for icon and version info
    if(EXISTS "${CMAKE_CURRENT_SOURCE_DIR}/src/images/Katalog_logo_64.ico")
        # Create a basic .rc file content
        set(RC_CONTENT "IDI_ICON1 ICON \"${CMAKE_CURRENT_SOURCE_DIR}/src/images/Katalog_logo_64.ico\"")
        file(WRITE "${CMAKE_CURRENT_BINARY_DIR}/katalog.rc" ${RC_CONTENT})
        target_sources(Katalog PRIVATE "${CMAKE_CURRENT_BINARY_DIR}/katalog.rc")
    endif()
endif()

if(APPLE)
    # Set application icon for macOS
    if(EXISTS "${CMAKE_CURRENT_SOURCE_DIR}/src/images/Katalog_logo_64.ico")
        set_target_properties(Katalog PROPERTIES
            MACOSX_BUNDLE TRUE
            MACOSX_BUNDLE_ICON_FILE "Katalog_logo_64.ico"
        )
    endif()
endif()

# Installation
install(TARGETS Katalog
    BUNDLE DESTINATION .
    RUNTIME DESTINATION bin
)

# Debug information
if(CMAKE_BUILD_TYPE STREQUAL "Debug")
    message(STATUS "Building Katalog in Debug mode")
    target_compile_definitions(Katalog PRIVATE DEBUG_BUILD)

    # Windows: Add custom command to force dependency checks
    if(WIN32)
        add_custom_command(TARGET Katalog PRE_BUILD
            COMMAND ${CMAKE_COMMAND} -E echo "Checking dependencies..."
            COMMENT "Force dependency check on Windows"
        )
    endif()
endif()

# Print configuration summary
message(STATUS "Katalog Configuration Summary:")
message(STATUS "  Version: ${PROJECT_VERSION}")
message(STATUS "  Qt6 Version: ${Qt6_VERSION}")
message(STATUS "  Build Type: ${CMAKE_BUILD_TYPE}")
message(STATUS "  C++ Standard: ${CMAKE_CXX_STANDARD}")

# Optional: Enable testing
option(BUILD_TESTS "Build unit tests" OFF)
if(BUILD_TESTS)
    enable_testing()
    # Add test subdirectory when you create tests
    # add_subdirectory(tests)
endif()

# Optional: Future QtQuick support
option(BUILD_QUICK_UI "Build QtQuick UI" OFF)
if(BUILD_QUICK_UI)
    find_package(Qt6 REQUIRED COMPONENTS Quick)

    # This would be for a future QtQuick application
    # add_subdirectory(src/quick)

    message(STATUS "QtQuick UI support enabled (future)")
endif()
