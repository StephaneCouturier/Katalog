cmake_minimum_required(VERSION 3.16)
project(Katalog LANGUAGES CXX)

# Set C++ standard
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

set(CMAKE_CXX_FLAGS_DEBUG "$ENV{CXXFLAGS} -O0 -Wall -g2")
set(CMAKE_CXX_FLAGS_RELEASE "$ENV{CXXFLAGS} -O3 -Wall")

# Windows-specific: Better dependency tracking
if(WIN32)
    set(CMAKE_DEPENDS_USE_COMPILER FALSE)
    set(CMAKE_DEPENDS_CHECK_CXX 1)

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
    LinguistTools
)

qt_standard_project_setup()

# Enable Qt's automatic processing
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTORCC ON)

# Find KF6 components
find_package(KF6CoreAddons REQUIRED)
#find_package(KF6I18n REQUIRED)
find_package(KF6XmlGui QUIET)
find_package(KF6Config REQUIRED)
#find_package(KF6IconThemes REQUIRED)
find_package(KF6FileMetaData REQUIRED)
find_package(KF6Completion QUIET)

add_definitions(-DQT_DEPRECATED_WARNINGS)

# Core library (always built — UI-agnostic)
add_subdirectory(core)

# Shared translation sources
add_subdirectory(translations)

# Variants — build one at a time
option(BUILD_QT_WIDGETS "Build Katalog2 - QtWidgets/KF6 desktop" ON)
option(BUILD_QT_QUICK   "Build Katalog3 - QtQuick/KF6 desktop"   OFF)
option(BUILD_WEB_SERVER  "Build KatalogWeb - REST API server"     OFF)

if(BUILD_QT_WIDGETS)
    add_subdirectory(qt_widgets)
endif()
if(BUILD_QT_QUICK)
    add_subdirectory(qt_quick)
endif()
if(BUILD_WEB_SERVER)
    add_subdirectory(web/server)
endif()

# Optional: unit tests
option(BUILD_TESTS "Build unit tests" OFF)
if(BUILD_TESTS)
    enable_testing()
    # add_subdirectory(tests)
endif()
