project(Katalog3 VERSION 0.1 LANGUAGES CXX)

# Qt Quick-specific dependencies not already found by root CMakeLists.txt
find_package(Qt6 6.5 REQUIRED COMPONENTS Quick Sql)
find_package(KF6I18n REQUIRED)
find_package(KF6Archive)

# Required for qt_add_qml_module (Qt 6.5+ QML module support)
qt_standard_project_setup(REQUIRES 6.5)

qt_add_executable(appKatalog
    main.cpp
)

target_compile_definitions(appKatalog PRIVATE
    QT_OPENGL_ES_2_ANGLE_STATIC
)

# Define core sources
set(CORE_SOURCES
    core/collection.h   core/collection.cpp
    core/device.h       core/device.cpp
    core/storage.h      core/storage.cpp
    core/catalog.h      core/catalog.cpp
    core/search.h            core/search.cpp
    core/devicelistmodel.h   core/devicelistmodel.cpp
    core/tag.h               core/tag.cpp
    #core/commandline.h core/commandline.cpp
    #core/database.cpp core/database.h
    #core/searchjob.cpp core/searchjob.h
    #core/searchjobstoppable.cpp core/searchjobstoppable.h
    #core/searchmanager.cpp core/searchmanager.h
)

# Define UI sources
set(UI_SOURCES
    PageSearch.h PageSearch.cpp
    appmanager.h appmanager.cpp
    filesview.h filesview.cpp
    database.h
)

qt_add_qml_module(appKatalog
    URI Katalog3
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
        PageSelectionView.qml
    SOURCES
        ${CORE_SOURCES}
        ${UI_SOURCES}
    RESOURCES
        images/logo-64.png
        images/Katalog_logo_64.ico
        images/logo-64.ico
)

# Qt for iOS sets MACOSX_BUNDLE_GUI_IDENTIFIER automatically since Qt 6.1.
# If you are developing for iOS or macOS you should consider setting an
# explicit, fixed bundle identifier manually though.
set_target_properties(appKatalog PROPERTIES
#    MACOSX_BUNDLE_GUI_IDENTIFIER com.example.appKatalog
    MACOSX_BUNDLE_BUNDLE_VERSION ${PROJECT_VERSION}
    MACOSX_BUNDLE_SHORT_VERSION_STRING ${PROJECT_VERSION_MAJOR}.${PROJECT_VERSION_MINOR}
    MACOSX_BUNDLE TRUE
    WIN32_EXECUTABLE TRUE
)

target_link_libraries(appKatalog
    PRIVATE
    Qt6::Quick
    Qt6::Qml
    Qt6::Gui
    Qt6::Sql
    #Qt6::QuickControls2
    #Qt6::Widgets
    KF6::I18n
    KF6::CoreAddons
    KF6::Archive
    #KF6::IconThemes
)

include(GNUInstallDirs)
install(TARGETS appKatalog
    BUNDLE DESTINATION .
    LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
    RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
)
