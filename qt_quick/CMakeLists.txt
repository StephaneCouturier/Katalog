project(Katalog3 VERSION 3.0 LANGUAGES CXX)

set(KATALOG_VERSION_STRING "3.0.beta1")
set(KATALOG_RELEASE_DATE  "2026-08-28")

configure_file(
    "${CMAKE_CURRENT_SOURCE_DIR}/version.h.in"
    "${CMAKE_CURRENT_BINARY_DIR}/version.h"
)

# Qt Quick-specific dependencies not already found by root CMakeLists.txt
find_package(Qt6 6.5 REQUIRED COMPONENTS Quick QuickControls2 Sql Charts)
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
    adapters/explorefilesmodel.h adapters/explorefilesmodel.cpp
    adapters/devicetablemodel.h  adapters/devicetablemodel.cpp
    adapters/backuppreviewmodel.h adapters/backuppreviewmodel.cpp
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
        DeviceIdentity.qml
        IconButton.qml
        PageSelectionDelegate.qml
        PageSearchResultsForm.qml
        PageSettings.qml
        PageDevicesView.qml
        PageDevicesViewDelegate.qml
        PageCreateForm.qml
        PageDeviceEditForm.qml
        PageTagsForm.qml
        PageStatisticsForm.qml
        PageBackupForm.qml
        PageBackupMappingForm.qml
        PageBackupPreviewForm.qml
        PageSelectionView.qml
        DeviceTreeComboBox.qml
        SearchTermList.qml
        MetadataDialog.qml
        OperationQueueView.qml
        PageExploreFolders.qml
        PageExploreFiles.qml
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
    # The shipped program is called Katalog: Katalog 3 IS Katalog, and the "3" is a
    # transition marker, the same way the pre-release settings file carries one and drops
    # it at release (AppManager). Only the cmake target keeps the suffix, because
    # qt_widgets already declares a target named Katalog and two targets cannot share a
    # name if both variants are ever enabled in one tree.
    OUTPUT_NAME "Katalog"
#    MACOSX_BUNDLE_GUI_IDENTIFIER com.example.Katalog3
    MACOSX_BUNDLE_BUNDLE_VERSION ${PROJECT_VERSION}
    MACOSX_BUNDLE_SHORT_VERSION_STRING ${PROJECT_VERSION_MAJOR}.${PROJECT_VERSION_MINOR}
    MACOSX_BUNDLE TRUE
    WIN32_EXECUTABLE TRUE
)

if(APPLE)
    # The bundle icon must be an .icns and must also be copied into
    # Contents/Resources: naming it alone leaves Finder with no icon at all.
    # It is deliberately NOT a qrc resource — it is a bundle file, and listing
    # it in images.qrc would embed a second copy in every platform's binary.
    # See SpecApplicationIcon.md, ICO-F5, ICO-C10 and ICO-C12.
    if(EXISTS "${CMAKE_SOURCE_DIR}/assets/Katalog.icns")
        target_sources(Katalog3 PRIVATE "${CMAKE_SOURCE_DIR}/assets/Katalog.icns")
        set_source_files_properties("${CMAKE_SOURCE_DIR}/assets/Katalog.icns"
            PROPERTIES MACOSX_PACKAGE_LOCATION "Resources"
        )
        set_target_properties(Katalog3 PROPERTIES
            MACOSX_BUNDLE_ICON_FILE "Katalog.icns"
        )
    endif()
endif()

target_link_libraries(Katalog3
    PRIVATE
    katalog-core
    Qt6::Quick
    Qt6::Qml
    Qt6::Gui
    Qt6::Sql
    Qt6::QuickControls2
    Qt6::Charts
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

# Desktop entry. K2 and K3 both install as bin/Katalog and deliberately share
# one desktop entry and one application id — they are mutually exclusive
# installs, so the K2 file is reused as-is rather than duplicated. Its
# translated Name/GenericName/Comment/Keywords values must not be edited.
# See SpecApplicationIcon.md (ICO-F3, ICO-C6, ICO-C7).
install(FILES ${CMAKE_SOURCE_DIR}/packaging/qt_widgets/Katalog.desktop
    DESTINATION share/applications
    RENAME io.github.stephanecouturier.Katalog.desktop)

# Application icon, installed at every size a desktop asks for so launchers and
# the window manager find an exact match instead of rescaling the 256 master.
# The size list is fixed by SpecApplicationIcon.md (ICO-F4); the files are
# pre-generated PNGs, never a build-time conversion (ICO-C5).
foreach(katalog_icon_size 16 22 24 32 48 64 128)
    install(FILES ${CMAKE_SOURCE_DIR}/assets/icons/katalog_${katalog_icon_size}.png
        DESTINATION share/icons/hicolor/${katalog_icon_size}x${katalog_icon_size}/apps
        RENAME io.github.stephanecouturier.Katalog.png)
endforeach()

# The 256 entry is the master itself, which is also the Linux window icon
# (ICO-F1) — kept as one file so a copy cannot drift from it.
install(FILES ${CMAKE_SOURCE_DIR}/assets/Katalog_logo_256.png
    DESTINATION share/icons/hicolor/256x256/apps
    RENAME io.github.stephanecouturier.Katalog.png)
