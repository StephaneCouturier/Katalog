set(CORE_HEADERS
    collection.h
    commandline.h
    catalog.h
    database.h
    device.h
    search.h
    searchjob.h
    searchjobstoppable.h
    searchmanager.h
    searchprogressmanager.h
    searchresultsthrottler.h
    storage.h
    tag.h
    language.h
    catalogjob.h
    catalogjobstoppable.h
    catalogmanager.h
    catalogprogressmanager.h
    devicejobstoppable.h
    devicemanager.h
    deviceupdatemanager.h
    filemetadata.h
    filetypemapping.h
    parallelmetadataextractor.h
    statusbarmessagebuilder.h
    backupprofilegenerator.h
    backupmappingmanager.h
    filechecksum.h
    foldertreeloader.h
    catalogdifferenceengine.h
    directoryreplicator.h
    backupjob.h
    backupjobstoppable.h
)

set(CORE_SOURCES
    collection.cpp
    commandline.cpp
    catalog.cpp
    database.cpp
    device.cpp
    search.cpp
    searchjob.cpp
    searchjobstoppable.cpp
    searchmanager.cpp
    searchprogressmanager.cpp
    searchresultsthrottler.cpp
    storage.cpp
    tag.cpp
    language.cpp
    catalogjob.cpp
    catalogjobstoppable.cpp
    catalogmanager.cpp
    catalogprogressmanager.cpp
    devicejobstoppable.cpp
    devicemanager.cpp
    deviceupdatemanager.cpp
    filemetadata.cpp
    filetypemapping.cpp
    parallelmetadataextractor.cpp
    statusbarmessagebuilder.cpp
    backupprofilegenerator.cpp
    backupmappingmanager.cpp
    filechecksum.cpp
    foldertreeloader.cpp
    catalogdifferenceengine.cpp
    directoryreplicator.cpp
    backupjob.cpp
    backupjobstoppable.cpp
)

add_library(katalog-core STATIC ${CORE_SOURCES} ${CORE_HEADERS})

target_link_libraries(katalog-core PUBLIC
    Qt6::Core
    Qt6::Gui
    Qt6::Sql
    Qt6::Network
    KF6::CoreAddons
    KF6::FileMetaData
)

target_include_directories(katalog-core
    PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}
    PUBLIC  ${CMAKE_SOURCE_DIR}
    PUBLIC  ${KF6CoreAddons_INCLUDE_DIRS}
    PUBLIC  ${KF6FileMetaData_INCLUDE_DIRS}
)

set_target_properties(katalog-core PROPERTIES
    AUTOMOC_MOC_OPTIONS "-I${CMAKE_SOURCE_DIR}"
)
