project(Katalog VERSION 2.13 LANGUAGES CXX)

set(KATALOG_VERSION_MAJOR ${PROJECT_VERSION_MAJOR})
set(KATALOG_VERSION_MINOR ${PROJECT_VERSION_MINOR})
set(KATALOG_VERSION_STRING "${PROJECT_VERSION_MAJOR}.${PROJECT_VERSION_MINOR}")

# Generate build timestamp
string(TIMESTAMP KATALOG_BUILD_DATE "%Y-%m-%d %H:%M:%S" UTC)
string(TIMESTAMP KATALOG_BUILD_YEAR "%Y" UTC)

# Configure version header
configure_file(
    "${CMAKE_CURRENT_SOURCE_DIR}/version.h.in"
    "${CMAKE_CURRENT_BINARY_DIR}/version.h"
    @ONLY
)

# ALL mainwindow implementation files
set(MAINWINDOW_IMPL_SOURCES
    mainwindow_setup.cpp
    mainwindow_tab_backup.cpp
    mainwindow_tab_create.cpp
    mainwindow_tab_device_pr.cpp
    mainwindow_tab_device_ui.cpp
    mainwindow_tab_explore.cpp
    mainwindow_tab_filters.cpp
    mainwindow_tab_search_pr.cpp
    mainwindow_tab_search_ui.cpp
    mainwindow_tab_settings.cpp
    mainwindow_tab_settings_exp.cpp
    mainwindow_tab_statistics.cpp
    mainwindow_tab_tags.cpp
)

set(MAIN_SOURCES
    devicemappingview.cpp
    devicetreeview.cpp
    directorytreeitem.cpp
    directorytreemodel.cpp
    exploretreeitem.cpp
    exploretreemodel.cpp
    exploretreeview.cpp
    filesview.cpp
    main.cpp
    mainwindow.cpp
    mainwindow_ui_wrapper_device.cpp
    mainwindow_ui_wrapper_storage.cpp
    mainwindow_ui_wrapper_catalog.cpp
    widgets/treecombobox.cpp
    ${MAINWINDOW_IMPL_SOURCES}
)

set(MAIN_HEADERS
    devicemappingview.h
    devicetreeview.h
    directorytreeitem.h
    directorytreemodel.h
    exploretreeitem.h
    exploretreemodel.h
    exploretreeview.h
    filesview.h
    mainwindow.h
    mainwindow_ui_wrapper_device.h
    mainwindow_ui_wrapper_storage.h
    mainwindow_ui_wrapper_catalog.h
    widgets/treecombobox.h
    version.h.in
)

set(UI_FILES
    mainwindow.ui
)

# K3 QML files — included in lupdate scan so qsTr() strings land in the shared .ts files.
# qt_quick/main.cpp is also scanned: it builds KAboutData with translatable strings
# via QCoreApplication::translate("Main", …) that lupdate must capture into the Main context.
set(K3_QML_FILES
    ${CMAKE_SOURCE_DIR}/qt_quick/main.cpp
    # K3 C++ sources that build user-visible strings with tr() (status/progress
    # messages, error dialogs). Scanned so those strings land in the shared .ts
    # files under their class context (AppManager, DeviceListModel).
    ${CMAKE_SOURCE_DIR}/qt_quick/appmanager.cpp
    ${CMAKE_SOURCE_DIR}/qt_quick/adapters/devicelistmodel.cpp
    ${CMAKE_SOURCE_DIR}/qt_quick/Main.qml
    ${CMAKE_SOURCE_DIR}/qt_quick/PageSearchForm.qml
    ${CMAKE_SOURCE_DIR}/qt_quick/PageSearchResultsForm.qml
    ${CMAKE_SOURCE_DIR}/qt_quick/PageSelectionDelegate.qml
    ${CMAKE_SOURCE_DIR}/qt_quick/PageSelectionView.qml
    ${CMAKE_SOURCE_DIR}/qt_quick/PageSettings.qml
    ${CMAKE_SOURCE_DIR}/qt_quick/PageDevicesView.qml
    ${CMAKE_SOURCE_DIR}/qt_quick/PageDevicesViewDelegate.qml
    ${CMAKE_SOURCE_DIR}/qt_quick/PageCreateForm.qml
    ${CMAKE_SOURCE_DIR}/qt_quick/DeviceTreeComboBox.qml
    ${CMAKE_SOURCE_DIR}/qt_quick/SearchTermList.qml
    ${CMAKE_SOURCE_DIR}/qt_quick/PageDeviceEditForm.qml
    ${CMAKE_SOURCE_DIR}/qt_quick/PageBackupForm.qml
    ${CMAKE_SOURCE_DIR}/qt_quick/PageBackupMappingForm.qml
    ${CMAKE_SOURCE_DIR}/qt_quick/PageBackupPreviewForm.qml
    ${CMAKE_SOURCE_DIR}/qt_quick/PageExploreFiles.qml
    ${CMAKE_SOURCE_DIR}/qt_quick/PageExploreFolders.qml
    ${CMAKE_SOURCE_DIR}/qt_quick/PageStatisticsForm.qml
    ${CMAKE_SOURCE_DIR}/qt_quick/PageTagsForm.qml
)

# Core library C++ sources — scanned so user-visible strings built in core
# (progress/status messages via QCoreApplication::translate("MainWindow", …),
# e.g. StatusBarMessageBuilder / *ProgressManager) land in the shared .ts files.
# Without this, those strings are marked vanished by lupdate and show in English.
file(GLOB CORE_TR_SOURCES
    ${CMAKE_SOURCE_DIR}/core/*.cpp
    ${CMAKE_SOURCE_DIR}/core/*.h
)

set(TS_FILES
    ${CMAKE_SOURCE_DIR}/translations/Katalog_bg_BG.ts
    ${CMAKE_SOURCE_DIR}/translations/Katalog_cs_CZ.ts
    ${CMAKE_SOURCE_DIR}/translations/Katalog_da_DK.ts
    ${CMAKE_SOURCE_DIR}/translations/Katalog_de_DE.ts
    ${CMAKE_SOURCE_DIR}/translations/Katalog_en_US.ts
    ${CMAKE_SOURCE_DIR}/translations/Katalog_el_GR.ts
    ${CMAKE_SOURCE_DIR}/translations/Katalog_es_ES.ts
    ${CMAKE_SOURCE_DIR}/translations/Katalog_et_EE.ts
    ${CMAKE_SOURCE_DIR}/translations/Katalog_fr_FR.ts
    ${CMAKE_SOURCE_DIR}/translations/Katalog_fi_FI.ts
    ${CMAKE_SOURCE_DIR}/translations/Katalog_hi_IN.ts
    ${CMAKE_SOURCE_DIR}/translations/Katalog_hu_HU.ts
    ${CMAKE_SOURCE_DIR}/translations/Katalog_it_IT.ts
    ${CMAKE_SOURCE_DIR}/translations/Katalog_ja_JP.ts
    ${CMAKE_SOURCE_DIR}/translations/Katalog_lt_LT.ts
    ${CMAKE_SOURCE_DIR}/translations/Katalog_lv_LV.ts
    ${CMAKE_SOURCE_DIR}/translations/Katalog_nl_NL.ts
    ${CMAKE_SOURCE_DIR}/translations/Katalog_nb_NO.ts
    ${CMAKE_SOURCE_DIR}/translations/Katalog_pl_PL.ts
    ${CMAKE_SOURCE_DIR}/translations/Katalog_pt_PT.ts
    ${CMAKE_SOURCE_DIR}/translations/Katalog_ro_RO.ts
    ${CMAKE_SOURCE_DIR}/translations/Katalog_sv_SE.ts
    ${CMAKE_SOURCE_DIR}/translations/Katalog_zh_CN.ts
    ${CMAKE_SOURCE_DIR}/translations/Katalog_sk_SK.ts
    ${CMAKE_SOURCE_DIR}/translations/Katalog_si_SI.ts
    ${CMAKE_SOURCE_DIR}/translations/Katalog_sr_RS.ts
    ${CMAKE_SOURCE_DIR}/translations/Katalog_uk_UA.ts
    ${CMAKE_SOURCE_DIR}/translations/Katalog_hr_HR.ts
    ${CMAKE_SOURCE_DIR}/translations/Katalog_id_ID.ts
)

set(QRC_FILES
    icons.qrc
    images.qrc
    styles.qrc
    translations.qrc
)

# Set up translations
if(${Qt6_VERSION} VERSION_GREATER_EQUAL "6.2")
    qt6_add_translations(Katalog
        TS_FILES ${TS_FILES}
        SOURCES ${MAIN_SOURCES} ${MAIN_HEADERS} ${UI_FILES} ${K3_QML_FILES} ${CORE_TR_SOURCES}
        LUPDATE_OPTIONS "-extensions" "cpp,h,ui,qml" "-locations" "relative"
        LRELEASE_OPTIONS "-compress" "-nounfinished"
    )

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
    COMMAND Qt6::lupdate ${MAIN_SOURCES} ${MAIN_HEADERS} ${UI_FILES} ${K3_QML_FILES} ${CORE_TR_SOURCES} -ts ${TS_FILES}
    WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
    COMMENT "Manually updating translation source files"
)

# Copy .qm files from build back to source directory
add_custom_target(translations_copy
    COMMENT "Copy .qm files from build to source directory"
)

foreach(ts_file ${TS_FILES})
    get_filename_component(qm_filename ${ts_file} NAME_WE)
    set(qm_file_build "${CMAKE_CURRENT_BINARY_DIR}/${qm_filename}.qm")
    set(qm_file_source "${CMAKE_SOURCE_DIR}/translations/${qm_filename}.qm")

    add_custom_command(TARGET translations_copy POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy_if_different
                ${qm_file_build}
                ${qm_file_source}
        COMMENT "Copying ${qm_filename}.qm to source directory"
    )
endforeach()

# Add custom target for refreshing timestamps
add_custom_target(refresh_timestamps
    COMMAND ${CMAKE_COMMAND} -E remove -f CMakeCache.txt
    COMMAND ${CMAKE_COMMAND} -E remove -f qt_widgets/version.h
    COMMENT "Clearing cache to refresh build timestamps"
    WORKING_DIRECTORY ${CMAKE_BINARY_DIR}
)

# Create the main executable
qt6_add_executable(Katalog
    ${MAIN_SOURCES}
    ${MAIN_HEADERS}
    ${UI_FILES}
    ${QRC_FILES}
)

add_dependencies(Katalog Katalog_lrelease)

target_link_libraries(Katalog PRIVATE
    katalog-core
    Qt6::Core
    Qt6::Widgets
    Qt6::Gui
    Qt6::Sql
    Qt6::Charts
    Qt6::Network
    KF6::CoreAddons
    #KF6::I18n
    KF6::XmlGui
    KF6::ConfigCore
    #KF6::IconThemes
    KF6::FileMetaData
)

target_include_directories(Katalog PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}
    ${CMAKE_CURRENT_BINARY_DIR}
    ${KF6XmlGui_INCLUDE_DIRS}
    ${KF6Config_INCLUDE_DIRS}
)

# To avoid threat detection of a separate freetype dll on Windows
if(WIN32)
    target_compile_definitions(Katalog PRIVATE
        QT_NO_FREETYPE
        -DNOMINMAX
        -DWIN32_LEAN_AND_MEAN
    )
endif()

# Platform-specific settings
if(WIN32)
    enable_language(RC)

    set_target_properties(Katalog PROPERTIES
        WIN32_EXECUTABLE TRUE
    )

    set_target_properties(Katalog PROPERTIES
        ADDITIONAL_CLEAN_FILES "${CMAKE_CURRENT_BINARY_DIR}/CMakeFiles/Katalog.dir"
    )

    set(CMAKE_DEPENDS_CHECK_CXX 1)

    target_compile_definitions(Katalog PRIVATE
        -DNOMINMAX
        -DWIN32_LEAN_AND_MEAN
    )

    if(EXISTS "${CMAKE_SOURCE_DIR}/assets/Katalog_logo_64.ico")
        set(RC_CONTENT "IDI_ICON1 ICON \"${CMAKE_SOURCE_DIR}/assets/Katalog_logo_64.ico\"")
        file(WRITE "${CMAKE_CURRENT_BINARY_DIR}/katalog.rc" ${RC_CONTENT})
        target_sources(Katalog PRIVATE "${CMAKE_CURRENT_BINARY_DIR}/katalog.rc")
    endif()
endif()

if(APPLE)
    if(EXISTS "${CMAKE_SOURCE_DIR}/assets/Katalog_logo_64.ico")
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

install(FILES ${CMAKE_SOURCE_DIR}/LICENSE.md
    DESTINATION share/licenses/io.github.stephanecouturier.Katalog
    RENAME LICENSE)

install(FILES ${CMAKE_SOURCE_DIR}/packaging/qt_widgets/Katalog.desktop
    DESTINATION share/applications
    RENAME io.github.stephanecouturier.Katalog.desktop)

install(FILES ${CMAKE_SOURCE_DIR}/packaging/qt_widgets/io.github.stephanecouturier.katalog.appdata.xml
    DESTINATION share/metainfo
    RENAME io.github.stephanecouturier.Katalog.metainfo.xml)

install(FILES ${CMAKE_SOURCE_DIR}/assets/Katalog_logo_256.png
    DESTINATION share/icons/hicolor/256x256/apps
    RENAME io.github.stephanecouturier.Katalog.png)

# Debug information
if(CMAKE_BUILD_TYPE STREQUAL "Debug")
    message(STATUS "Building Katalog in Debug mode")
    target_compile_definitions(Katalog PRIVATE DEBUG_BUILD)

    if(WIN32)
        add_custom_command(TARGET Katalog PRE_BUILD
            COMMAND ${CMAKE_COMMAND} -E echo "Checking dependencies..."
            COMMENT "Force dependency check on Windows"
        )
    endif()
endif()

message(STATUS "Katalog (qt_widgets) Configuration Summary:")
message(STATUS "  Version: ${PROJECT_VERSION}")
message(STATUS "  Qt6 Version: ${Qt6_VERSION}")
message(STATUS "  Build Type: ${CMAKE_BUILD_TYPE}")
message(STATUS "  C++ Standard: ${CMAKE_CXX_STANDARD}")
