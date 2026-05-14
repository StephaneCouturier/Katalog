project(Katalog3 VERSION 3.0 LANGUAGES CXX)

set(KATALOG_VERSION_STRING "3.alpha2")
set(KATALOG_RELEASE_DATE  "2026-03-31")

configure_file(
    "${CMAKE_CURRENT_SOURCE_DIR}/version.h.in"
    "${CMAKE_CURRENT_BINARY_DIR}/version.h"
)

# Qt Quick-specific dependencies not already found by root CMakeLists.txt
find_package(Qt6 6.5 REQUIRED COMPONENTS Quick Sql)
find_package(KF6I18n REQUIRED)
find_package(KF6Archive)

# Required for qt_add_qml_module (Qt 6.5+ QML module support)
qt_standard_project_setup(REQUIRES 6.5)

qt_add_executable(Katalog3
    main.cpp
    images.qrc
    icons.qrc
    translations.qrc
)

target_compile_definitions(Katalog3 PRIVATE
    QT_OPENGL_ES_2_ANGLE_STATIC
)

# Define core sources
set(CORE_SOURCES
    adapters/search.h            adapters/search.cpp
    adapters/devicelistmodel.h   adapters/devicelistmodel.cpp
    #adapters/commandline.h adapters/commandline.cpp
    #adapters/database.cpp adapters/database.h
    #adapters/searchjob.cpp adapters/searchjob.h
    #adapters/searchjobstoppable.cpp adapters/searchjobstoppable.h
    #adapters/searchmanager.cpp adapters/searchmanager.h
)

# Define UI sources
set(UI_SOURCES
    PageSearch.h PageSearch.cpp
    appmanager.h appmanager.cpp
    filesview.h filesview.cpp
    database.h
)

# URI uses reverse-domain path (io/github/stephanecouturier/Katalog/) — no conflict with Katalog3 binary
qt_add_qml_module(Katalog3
    URI io.github.stephanecouturier.Katalog
    VERSION 1.0
    QML_FILES
        Main.qml
        PageSearchForm.qml
        PageSelectionDelegate.qml
        PageSearchResultsForm.qml
        PageSettings.qml
        PageDevicesView.qml
        PageDevicesViewDelegate.qml
        PageCreateForm.qml
        PageDeviceEditForm.qml
        PageTagsForm.qml
        PageSelectionView.qml
        DeviceTreeComboBox.qml
    SOURCES
        ${CORE_SOURCES}
        ${UI_SOURCES}
    RESOURCES
        ../assets/Katalog_logo_64.ico
)

# Qt for iOS sets MACOSX_BUNDLE_GUI_IDENTIFIER automatically since Qt 6.1.
# If you are developing for iOS or macOS you should consider setting an
# explicit, fixed bundle identifier manually though.
set_target_properties(Katalog3 PROPERTIES
#    MACOSX_BUNDLE_GUI_IDENTIFIER com.example.Katalog3
    MACOSX_BUNDLE_BUNDLE_VERSION ${PROJECT_VERSION}
    MACOSX_BUNDLE_SHORT_VERSION_STRING ${PROJECT_VERSION_MAJOR}.${PROJECT_VERSION_MINOR}
    MACOSX_BUNDLE TRUE
    WIN32_EXECUTABLE TRUE
)

target_link_libraries(Katalog3
    PRIVATE
    katalog-core
    Qt6::Quick
    Qt6::Qml
    Qt6::Gui
    Qt6::Sql
    #Qt6::QuickControls2
    Qt6::Widgets
    KF6::I18n
    KF6::CoreAddons
    KF6::Archive
    #KF6::IconThemes
)

target_include_directories(Katalog3 PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}
    ${CMAKE_CURRENT_BINARY_DIR}
    ${CMAKE_SOURCE_DIR}
)

include(GNUInstallDirs)
install(TARGETS Katalog3
    BUNDLE DESTINATION .
    LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
    RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
)
