/*LICENCE
    This file is part of Katalog

    Copyright (C) 2021, the Katalog Development team

    Author: Stephane Couturier (Symbioxy)

    Katalog is free software; you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation; either version 2 of the License, or
    (at your option) any later version.

    Katalog is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with Katalog; if not, write to the Free Software
    Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA
*/
/*FILE DESCRIPTION
/////////////////////////////////////////////////////////////////////////////
// Application: Katalog
// File Name:   searchjobstoppable.cpp
// Purpose:     implementation for the search class that can be stopped (based on KJob)
// Description: https://stephanecouturier.github.io/Katalog/docs/Features/Search
// Author:      Stephane Couturier
/////////////////////////////////////////////////////////////////////////////
*///

#include "core/searchjobstoppable.h"
#include "core/catalogmanager.h"
#include "core/filemetadata.h"
#include "core/filetypemapping.h"
#include <QSqlQuery>
#include <QSqlError>
#include <QElapsedTimer>
#include <QCoreApplication>
#include <QDebug>

SearchJobStoppable::SearchJobStoppable(QObject *parent)
    : Search(parent)
{
    // Initialize catalog tracking
    currentCatalogIndex = 0;
    totalCatalogs = 0;
    currentCatalogName = "";

    // Set a reasonable default refresh rate
    progressRefreshRate = 1000;
}
//----------------------------------------------------------------------
SearchJobStoppable::~SearchJobStoppable()
{
    //qDebug() << "SearchJobStoppable destructor called";

    m_objectValid.storeRelease(0);
    stopSearch();

    //qDebug() << "SearchJobStoppable destructor complete";
}
//----------------------------------------------------------------------
QString getEffectiveCatalogIDForSearch(int catalogID, CatalogManager* catalogManager)
{
    if (catalogManager && catalogManager->isCatalogBeingUpdated(catalogID)) {
        return catalogManager->getEffectiveCatalogID(catalogID);
    }
    return QString::number(catalogID);
}
//----------------------------------------------------------------------
void SearchJobStoppable::setDatabaseConnection(const QString &connectionName)
{
    m_connectionName = connectionName;
}
//----------------------------------------------------------------------
void SearchJobStoppable::searchFiles(Device *selectedDevice)
{
    // qDebug() << "SearchJobStoppable::searchFiles() starting";
    // qDebug() << "  - Selected device type:" << selectedDevice->type;
    // qDebug() << "  - Selected device name:" << selectedDevice->name;
    // qDebug() << "  - Database connection:" << m_connectionName;
    // qDebug() << "  - Search in catalogs checked:" << searchInCatalogsChecked;
    // qDebug() << "  - Search text:" << searchText;

    // Add this debug check to see if there's leftover data
    QSqlQuery checkQuery(QSqlDatabase::database(m_connectionName));
    checkQuery.exec("SELECT COUNT(*) FROM filetemp");
    if (checkQuery.next()) {
        int tempCount = checkQuery.value(0).toInt();
        qDebug() << "  - filetemp table has" << tempCount << "records at start";
        if (tempCount > 0) {
            qDebug() << "  - WARNING: filetemp table not empty! Clearing it...";
            QSqlQuery clearQuery(QSqlDatabase::database(m_connectionName));
            clearQuery.exec("DELETE FROM filetemp");
        }
    }

    // Clear previous results
    clearResults();

    // Add this debug check to see if there's leftover data
    QSqlQuery checkQuery2(QSqlDatabase::database(m_connectionName));
    checkQuery2.exec("SELECT COUNT(*) FROM filetemp");
    if (checkQuery2.next()) {
        int tempCount = checkQuery2.value(0).toInt();
        //qDebug() << "  - filetemp table has" << tempCount << "records at start";
        if (tempCount > 0) {
            qDebug() << "  - WARNING: filetemp table not empty! Clearing it...";
            QSqlQuery clearQuery(QSqlDatabase::database(m_connectionName));
            clearQuery.exec("DELETE FROM filetemp");
        }
    }

    // Reset tracking
    currentCatalogIndex = 0;
    totalCatalogs = 0;
    currentCatalogName = "";
    m_stopRequested.storeRelease(0);
    m_paused.storeRelease(0);

    // Initialize progress tracking
    initializeProgressTracking(selectedDevice);

    // Setup common search patterns
    prepareSearchPatterns();

    if (!shouldContinue()) return;

    // Process the SEARCH in CATALOGS or DIRECTORY
    if (searchInCatalogsChecked) {
        // Count total catalogs for progress tracking
        if (searchOnDifferences) {
            // For differences, count catalogs in both devices
            totalCatalogs = 0;

            diffDevice1->loadDevice(m_connectionName);
            if (diffDevice1->type == "Catalog") {
                totalCatalogs++;
            } else {
                for (const auto& row : diffDevice1->deviceListTable) {
                    if (row.type == "Catalog") {
                        totalCatalogs++;
                    }
                }
            }

            diffDevice2->loadDevice(m_connectionName);
            if (diffDevice2->type == "Catalog") {
                totalCatalogs++;
            } else {
                for (const auto& row : diffDevice2->deviceListTable) {
                    if (row.type == "Catalog") {
                        totalCatalogs++;
                    }
                }
            }
        } else {
            // Count catalogs in the selected device
            totalCatalogs = 0;
            if (selectedDevice->type == "Catalog") {
                totalCatalogs = 1;
            } else {
                for (const auto& row : selectedDevice->deviceListTable) {
                    if (row.type == "Catalog") {
                        totalCatalogs++;
                    }
                }
            }
        }

        currentCatalogIndex = 0;
        emit searchProgress(0);

        // Execute search based on type
        if (searchOnDifferences) {
            // Search differences between two devices
            QMutex dummyMutex;
            bool dummyStop = false;

            // Process diffDevice1
            diffDevice1->loadDevice(m_connectionName);
            if (diffDevice1->type == "Catalog" && shouldContinue()) {
                currentCatalogIndex++;
                currentCatalogName = diffDevice1->name;
                searchFilesInCatalog(diffDevice1, dummyMutex, dummyStop);
            } else {
                for (const auto& row : diffDevice1->deviceListTable) {
                    if (!shouldContinue()) break;
                    if (row.type == "Catalog") {
                        currentCatalogIndex++;
                        Device *device = new Device;
                        device->ID = row.ID;
                        device->loadDevice(m_connectionName);
                        currentCatalogName = device->name;
                        searchFilesInCatalog(device, dummyMutex, dummyStop);
                        delete device;
                    }
                }
            }

            // Process diffDevice2
            if (shouldContinue()) {
                diffDevice2->loadDevice(m_connectionName);
                if (diffDevice2->type == "Catalog") {
                    currentCatalogIndex++;
                    currentCatalogName = diffDevice2->name;
                    searchFilesInCatalog(diffDevice2, dummyMutex, dummyStop);
                } else {
                    for (const auto& row : diffDevice2->deviceListTable) {
                        if (!shouldContinue()) break;
                        if (row.type == "Catalog") {
                            currentCatalogIndex++;
                            Device *device = new Device;
                            device->ID = row.ID;
                            device->loadDevice(m_connectionName);
                            currentCatalogName = device->name;
                            searchFilesInCatalog(device, dummyMutex, dummyStop);
                            delete device;
                        }
                    }
                }
            }
        } else {
            // Regular search in selected device
            QMutex dummyMutex;
            bool dummyStop = false;

            if (selectedDevice->type == "Catalog" && shouldContinue()) {
                currentCatalogIndex++;
                currentCatalogName = selectedDevice->name;
                searchFilesInCatalog(selectedDevice, dummyMutex, dummyStop);
            } else {
                for (const auto& row : selectedDevice->deviceListTable) {
                    if (!shouldContinue()) break;
                    if (row.type == "Catalog") {
                        currentCatalogIndex++;
                        Device *device = new Device;
                        device->ID = row.ID;
                        device->loadDevice(m_connectionName);
                        currentCatalogName = device->name;
                        searchFilesInCatalog(device, dummyMutex, dummyStop);
                        delete device;
                    }
                }
            }
        }
    } else if (searchInConnectedChecked && shouldContinue()) {
        qDebug() << "SearchJobStoppable::searchFiles() searching in connected directory:" << connectedDirectory;
        // Search in directory
        totalCatalogs = 1;
        currentCatalogIndex = 1;
        currentCatalogName = connectedDirectory;
        QMutex dummyMutex;
        bool dummyStop = false;
        qDebug() << "  - Searching in directory:" << connectedDirectory;
        searchFilesInDirectory(connectedDirectory, dummyMutex, dummyStop);
    }

    if (!shouldContinue()) {
        emit searchProgress(-1); // Indicate interruption
        return;
    }

    // Process results
    processResults();

    // Process DUPLICATES
    if (shouldContinue() && searchOnFileCriteria && searchOnDuplicates &&
        (searchDuplicatesOnName || searchDuplicatesOnSize || searchDuplicatesOnDate)) {
        processDuplicates(m_connectionName);
    }

    // Process DIFFERENCES
    if (shouldContinue() && searchOnFileCriteria && searchOnDifferences &&
        (differencesOnName || differencesOnSize || differencesOnDate)) {
        processDifferences(m_connectionName);
    }

    // Calculate statistics
    calculateStatistics();

    // Final progress report
    if (shouldContinue()) {
        emit searchProgress(totalFilesProcessed);
    }
}
//----------------------------------------------------------------------
void SearchJobStoppable::stopSearch()
{
    //qDebug() << "=== SearchJobStoppable::stopSearch() called ===";
    //qDebug() << "Setting stop flag to 1";

    m_stopRequested.storeRelease(1);

    //Also stop CSV loading if in progress
    m_csvLoadingStopFlag = true;

    // Wake up any paused operations
    if (m_paused.loadAcquire()) {
        qDebug() << "Waking up paused operation";
        m_paused.storeRelease(0);
    }

    //qDebug() << "Stop flag set - shouldContinue() now returns:" << shouldContinue();
    //qDebug() << "=== SearchJobStoppable::stopSearch() complete ===";
}
//----------------------------------------------------------------------
void SearchJobStoppable::pauseSearch()
{
    m_paused.storeRelease(1);
}
//----------------------------------------------------------------------
void SearchJobStoppable::resumeSearch()
{
    m_paused.storeRelease(0);
}
//----------------------------------------------------------------------
void SearchJobStoppable::searchFilesInCatalog(Device *device, QMutex &mutex, bool &stopRequested)
{
    qDebug() << "SearchJobStoppable::searchFilesInCatalog() starting for device:" << device->name;
    qDebug() << "  - Device external ID:" << device->externalID;
    qDebug() << "  - Database connection:" << m_connectionName;

    // Add memory mode CSV loading with proper progress reporting with stop/pause control
    if (memoryModeEnabled) {
        qDebug() << "Memory mode: Loading catalog CSV file for" << device->name;

        // Reset catalog file loading counters (same as SearchMemory)
        currentCatalogFilesLoaded = 0;
        currentCatalogTotalFiles = device->totalFileCount;

        // Track loading progress (same as SearchMemory)
        emit searchProgress(-2); // Special signal to indicate catalog loading started

        // Use a simple approach with immediate disconnection on stop
        bool localStopRequested = false;
        QMutex csvMutex;
        QMetaObject::Connection progressConnection;

        // Connect with immediate access to connection handle for safe disconnection
        progressConnection = connect(device->catalog, &Catalog::loadProgress, this,
                                     [this, &localStopRequested, &progressConnection](int filesLoaded, int totalFiles) {
                                         // STOP CHECK: If stop requested, disconnect immediately and exit
                                         if (!shouldContinue()) {
                                             qDebug() << "Stop detected in CSV loading callback - disconnecting";
                                             QObject::disconnect(progressConnection);
                                             localStopRequested = true;
                                             return;
                                         }

                                         currentCatalogFilesLoaded = filesLoaded;
                                         currentCatalogTotalFiles = totalFiles;

                                         // Handle pause requests
                                         waitIfPaused();

                                         // Check again after pause
                                         if (!shouldContinue()) {
                                             qDebug() << "Stop detected after pause - disconnecting";
                                             QObject::disconnect(progressConnection);
                                             localStopRequested = true;
                                             return;
                                         }

                                         // Send progress signal
                                         emit searchProgress(-4); // -4 is catalog loading progress update
                                     }, Qt::DirectConnection); // Use DirectConnection for immediate callback

        // Load CSV files into database
        device->catalog->loadCatalogFileListToTable(csvMutex, localStopRequested);

        // Ensure disconnection (in case not already disconnected by callback)
        if (progressConnection) {
            disconnect(progressConnection);
        }

        // Check if stopped during loading
        if (!shouldContinue() || localStopRequested) {
            qDebug() << "Stop requested during CSV loading";
            return;
        }

        // Signal that catalog loading is complete (same as SearchMemory)
        emit searchProgress(-3); // Special signal to indicate catalog loading finished

        qDebug() << "Memory mode: CSV loading complete for" << device->name;
    }

    //Rest of method is common for all types of database modes
    Q_UNUSED(mutex);
    Q_UNUSED(stopRequested);

    if (!shouldContinue()) {
        qDebug() << "SearchJobStoppable::searchFilesInCatalog - stop requested at start";
        return;
    }

    // Emit signal to indicate catalog processing started
    emit searchProgress(-2);

    // Initialize Regular Expression
    QRegularExpression regex(regexPattern);
    if (!caseSensitive) {
        regex.setPatternOptions(QRegularExpression::CaseInsensitiveOption);
    }

    // Build SQL query
    QSqlQuery getFilesQuery(QSqlDatabase::database(m_connectionName));
    QString getFilesQuerySQL = QLatin1String(R"(
    SELECT  file_name,
            file_folder_path,
            file_size,
            file_date_updated,
            file_type,
            mime_type,
            image_width,
            image_height,
            video_duration_seconds,
            video_width,
            video_height,
            audio_duration_seconds,
            audio_artist,
            audio_album,
            audio_title,
            metadata_extended
    FROM  file
    WHERE file_catalog_id = :file_catalog_id
)");

    // Add size filter
    if (searchOnFileCriteria && searchOnSize) {
        getFilesQuerySQL += " AND file_size >= :file_size_min ";
        getFilesQuerySQL += " AND file_size <= :file_size_max ";
    }

    // Add date filter
    if (searchOnFileCriteria && searchOnDate) {
        getFilesQuerySQL += " AND file_date_updated >= :file_date_updated_min ";
        getFilesQuerySQL += " AND file_date_updated <= :file_date_updated_max ";
    }

    // Add Metadata filter
    if (searchOnFileMetadata && searchOnMetadataText && !metadataTextSearch.isEmpty()) {
        QString metadataCondition = buildMetadataSearchConditions();
        if (!metadataCondition.isEmpty()) {
            getFilesQuerySQL += " AND " + metadataCondition + " ";
        }
    }
    // Add Metadata size filter
    if (searchOnFileMetadata && searchOnMetadataSize) {
        QStringList sizeConditions;

        // Height condition - check both image and video height
        QString heightCondition = QString("((image_height >= %1 AND image_height <= %2) OR (video_height >= %1 AND video_height <= %2))")
                                      .arg(metadataMinimumHeight)
                                      .arg(metadataMaximumHeight);
        sizeConditions.append(heightCondition);

        // Width condition - check both image and video width
        QString widthCondition = QString("((image_width >= %1 AND image_width <= %2) OR (video_width >= %1 AND video_width <= %2))")
                                     .arg(metadataMinimumWidth)
                                     .arg(metadataMaximumWidth);
        sizeConditions.append(widthCondition);

        // Combine height and width with AND
        getFilesQuerySQL += " AND (" + sizeConditions.join(" AND ") + ") ";
    }

    // Add Metadata duration filter
    if (searchOnFileMetadata && searchOnMetadataDuration) {
        // Convert QDateTime to seconds for comparison
        int minSeconds = metadataDurationMin.time().hour() * 3600 +
                         metadataDurationMin.time().minute() * 60 +
                         metadataDurationMin.time().second();
        int maxSeconds = metadataDurationMax.time().hour() * 3600 +
                         metadataDurationMax.time().minute() * 60 +
                         metadataDurationMax.time().second();

        // Duration condition - check both video and audio duration
        QString durationCondition = QString("((video_duration_seconds >= %1 AND video_duration_seconds <= %2) OR (audio_duration_seconds >= %1 AND audio_duration_seconds <= %2))")
                                        .arg(minSeconds)
                                        .arg(maxSeconds);

        getFilesQuerySQL += " AND (" + durationCondition + ") ";
    }

    // Add FileType filter using FileTypeMapping
    if (searchOnType && selectedUserFileType != FileTypeMapping::ALL) {
        QString fileTypeFilter = FileTypeMapping::getSqlFilter(selectedUserFileType);
        if (!fileTypeFilter.isEmpty()) {
            getFilesQuerySQL += " AND (" + fileTypeFilter + ") ";
        }
    }

    getFilesQuery.prepare(getFilesQuerySQL);
    getFilesQuery.bindValue(":file_catalog_id", device->externalID);

    if (searchOnFileCriteria && searchOnSize) {
        getFilesQuery.bindValue(":file_size_min", selectedMinimumSize * sizeMultiplierMin);
        getFilesQuery.bindValue(":file_size_max", selectedMaximumSize * sizeMultiplierMax);
    }

    if (searchOnFileCriteria && searchOnDate) {
        getFilesQuery.bindValue(":file_date_updated_min", selectedDateMin.toString("yyyy/MM/dd hh:mm:ss"));
        getFilesQuery.bindValue(":file_date_updated_max", selectedDateMax.toString("yyyy/MM/dd hh:mm:ss"));
    }

    if (!getFilesQuery.exec()) {
        qDebug() << "Query error:" << getFilesQuery.lastError().text();
        return;
    }
    qDebug() << "  - SQL query executed successfully" << getFilesQuerySQL;

    emit searchProgress(-3); // Processing files

    int filesProcessed = 0;
    int batchCount = 0;

    while (getFilesQuery.next() && shouldContinue()) {
        // STOP check (existing)
        if (stopRequested || !shouldContinue()) {
            qDebug() << "SearchJobStoppable::searchFilesInCatalog - stop requested in main loop at file" << filesProcessed;
            break;
        }

        // Add pause check every N files (simple approach)
        if (filesProcessed % 100 == 0) {  // Check every 100 files
            waitIfPaused();  // This will pause if needed

            // After resuming, check if stop was requested while paused
            if (!shouldContinue()) {
                qDebug() << "SearchJobStoppable::searchFilesInCatalog - stop requested after pause";
                break;
            }
        }

        QString fileName = getFilesQuery.value(0).toString();
        QString filePath = getFilesQuery.value(1).toString();
        QString fileFullPath = filePath + "/" + fileName;
        qint64 fileSize = getFilesQuery.value(2).toLongLong();
        QString fileDateTime = getFilesQuery.value(3).toString();

        qDebug() << "Processing file:" << fileFullPath;

        // NEW METADATA FIELDS:
        QString fileType = getFilesQuery.value(4).toString();
        QString mimeType = getFilesQuery.value(5).toString();
        int imageWidth = getFilesQuery.value(6).toInt();
        int imageHeight = getFilesQuery.value(7).toInt();
        int videoDuration = getFilesQuery.value(8).toInt();
        int videoWidth = getFilesQuery.value(9).toInt();
        int videoHeight = getFilesQuery.value(10).toInt();
        int audioDuration = getFilesQuery.value(11).toInt();
        QString audioArtist = getFilesQuery.value(12).toString();
        QString audioAlbum = getFilesQuery.value(13).toString();
        QString audioTitle = getFilesQuery.value(14).toString();

        filesProcessed++;
        batchCount++;
        if (filesProcessed % 100 == 0) {
            qDebug() << "  - Processed" << filesProcessed << "files so far";
        }
        // Tag filtering
        if (searchOnFolderCriteria && searchOnTags && !selectedTagName.isEmpty()) {
            bool fileIsMatchingTag = false;

            QSqlQuery queryTag(QSqlDatabase::database(m_connectionName));
            QString queryTagSQL = QLatin1String(R"(
                SELECT path FROM tag WHERE name=:name
            )");
            queryTag.prepare(queryTagSQL);
            queryTag.bindValue(":name", selectedTagName);
            queryTag.exec();

            while (queryTag.next()) {
                if ((filePath + "/").contains(queryTag.value(0).toString() + "/")) {
                    fileIsMatchingTag = true;
                    break;
                }
            }
            if (!fileIsMatchingTag) {
                continue;
            }
        }

        // Text search criteria
        QRegularExpressionMatch match;

        if (searchOnFileName) {
            if (selectedSearchIn == SEARCH_IN_FILE_NAMES) {

                match = regex.match(fileName);
                qDebug() << "selectedSearchIn == SEARCH_IN_FILE_NAMES: match" << match;
            } else if (selectedSearchIn == SEARCH_IN_FOLDER_PATH) {
                regex.setPattern(regexSearchtext);
                auto foldermatch = regex.match(filePath);
                if (foldermatch.hasMatch() && searchOnType) {
                    regex.setPattern(regexFileType);
                    match = regex.match(fileName);
                } else {
                    match = foldermatch;
                }
            } else { // SEARCH_IN_FILES_AND_FOLDERS
                match = regex.match(fileFullPath);
            }

            if (match.hasMatch()) {

                qDebug() << "Processing file:" << fileFullPath << "match.hasMatch()"<< match.hasMatch();
                filesFoundList << filePath;
                deviceFoundIDList.insert(0, QString::number(device->ID));

                fileNames.append(fileName);
                filePaths.append(filePath);
                fileSizes.append(fileSize);
                fileDateTimes.append(fileDateTime);
                fileCatalogs.append(device->name);
                fileCatalogIDs.append(device->externalID);

                fileTypes.append(fileType);
                mimeTypes.append(mimeType);
                imageWidths.append(imageWidth);
                imageHeights.append(imageHeight);
                videoDurations.append(videoDuration);
                videoWidths.append(videoWidth);
                videoHeights.append(videoHeight);
                audioDurations.append(audioDuration);
                audioArtists.append(audioArtist);
                audioAlbums.append(audioAlbum);
                audioTitles.append(audioTitle);
            }
        }

        // Progress reporting
        if (batchCount >= progressRefreshRate) {
            totalFilesProcessed += batchCount;
            emit searchProgress(totalFilesProcessed);
            batchCount = 0;

            // Process events to keep UI responsive
            QCoreApplication::processEvents();
        }
    }

    // Report remaining files
    if (batchCount > 0) {
        totalFilesProcessed += batchCount;
        emit searchProgress(totalFilesProcessed);
    }
}
//----------------------------------------------------------------------
void SearchJobStoppable::searchFilesInDirectory(const QString &sourceDirectory, QMutex &mutex, bool &stopRequested)
{
    qDebug() << "SearchJobStoppable::searchFilesInDirectory starting for:" << sourceDirectory;

    // Use the same pattern as searchFilesInCatalog - get local reference to stopRequested
    bool &localStopRequested = stopRequested;

    // But ALSO check shouldContinue() since we use atomic variables
    if (!shouldContinue()) {
        qDebug() << "SearchJobStoppable::searchFilesInDirectory - stop requested at start";
        return;
    }

    // Check if directory exists
    QDir dir(sourceDirectory);
    if (!dir.exists()) {
        qWarning() << "Warning: Directory does not exist:" << sourceDirectory;
        return;
    }

    // Initialize Regular Expression
    QRegularExpression regex(regexPattern, QRegularExpression::CaseInsensitiveOption);
    if (caseSensitive == true) {
        regex.setPatternOptions(QRegularExpression::NoPatternOption);
    }

    // Scan directory and create a list of files
    QDirIterator iterator(sourceDirectory, fileTypes, QDir::Files | QDir::Hidden, QDirIterator::Subdirectories);

    // Track both processed files and found files separately
    int filesProcessedCount = 0;  // All files examined
    int filesFoundCount = 0;      // Files that match criteria
    int batchProcessedCount = 0;  // Batch counter for progress reporting

    qDebug() << "SearchJobStoppable::searchFilesInDirectory - starting main loop";

    // Main loop - similar pattern to searchFilesInCatalog but track processing vs finding separately
    while (iterator.hasNext()) {
        // STOP check (existing)
        if (localStopRequested || !shouldContinue()) {
            qDebug() << "SearchJobStoppable::searchFilesInDirectory - stop requested in main loop at file" << filesProcessedCount;
            break;
        }

        // Add pause check every N files (simple approach)
        if (filesProcessedCount % 100 == 0) {  // Check every 100 files
            waitIfPaused();  // This will pause if needed

            // After resuming, check if stop was requested while paused
            if (!shouldContinue()) {
                qDebug() << "SearchJobStoppable::searchFilesInDirectory - stop requested after pause";
                break;
            }
        }

        // Get file information
        QString filePath = iterator.next();
        QFileInfo fileInfo(filePath);
        QDateTime fileDate = fileInfo.lastModified();

        filesProcessedCount++;      // Count ALL files processed
        batchProcessedCount++;      // Count for batch progress reporting

        // Create line with file info (compatible with catalog format)
        QString line = fileInfo.absoluteFilePath() + "\t" + QString::number(fileInfo.size()) + "\t" + fileDate.toString("yyyy/MM/dd hh:mm:ss");

        // Split the line text with tabulations into a list
        QRegularExpression lineSplitExp("\t");
        QStringList lineFieldList = line.split(lineSplitExp);
        int fieldListCount = lineFieldList.count();

        // Get the file absolute path from this list
        QString lineFilePath = lineFieldList[0];

        // Get the FileSize from the list if available
        qint64 lineFileSize;
        if (fieldListCount >= 2) {
            lineFileSize = lineFieldList[1].toLongLong();
        } else {
            lineFileSize = fileInfo.size();
        }

        // Get the File DateTime from the list if available
        QDateTime lineFileDateTime;
        if (fieldListCount >= 3) {
            lineFileDateTime = QDateTime::fromString(lineFieldList[2], "yyyy/MM/dd hh:mm:ss");
        } else {
            lineFileDateTime = fileDate;
        }

        // Exclude catalog metadata lines which start with "<"
        if (lineFilePath.left(1) == "<") {
            continue;
        }

        // Apply size filter
        if (searchOnSize == true) {
            if (!(lineFileSize >= selectedMinimumSize * sizeMultiplierMin
                  && lineFileSize <= selectedMaximumSize * sizeMultiplierMax)) {
                continue;
            }
        }

        // Apply date filter
        if (searchOnDate == true) {
            if (!(lineFileDateTime >= selectedDateMin
                  && lineFileDateTime <= selectedDateMax)) {
                continue;
            }
        }

        // Apply tags filter
        if (searchOnTags == true) {
            bool fileIsMatchingTag = false;

            QSqlQuery queryTag(QSqlDatabase::database(m_connectionName));
            QString queryTagSQL = QLatin1String(R"(
                SELECT path
                FROM tag
                WHERE name=:name
            )");
            queryTag.prepare(queryTagSQL);
            queryTag.bindValue(":name", selectedTagName);

            if (!queryTag.exec()) {
                qWarning() << "SearchJobStoppable::searchFilesInDirectory - Tag query failed:" << queryTag.lastError().text();
                continue;
            }

            while (queryTag.next()) {
                if (lineFilePath.contains(queryTag.value(0).toString()) == true) {
                    fileIsMatchingTag = true;
                    break;
                }
            }

            if (!fileIsMatchingTag) {
                continue;
            }
        }

        // Apply filename/text search criteria
        QRegularExpressionMatch match;
        QRegularExpressionMatch foldermatch;

        if (searchOnFileName == true) {
            if (selectedSearchIn == Search::SEARCH_IN_FILE_NAMES) {
                QFileInfo file(lineFilePath);
                QString reducedLine = file.fileName();
                match = regex.match(reducedLine);
            }
            else if (selectedSearchIn == Search::SEARCH_IN_FOLDER_PATH) {
                QString reducedLine = lineFilePath.left(lineFilePath.lastIndexOf("/"));
                regex.setPattern(regexSearchtext);
                foldermatch = regex.match(reducedLine);

                if (foldermatch.hasMatch() && searchOnType == true) {
                    regex.setPattern(regexFileType);
                    match = regex.match(lineFilePath);
                } else {
                    match = foldermatch;
                }
            }
            else { // Search::SEARCH_IN_FILES_AND_FOLDERS
                match = regex.match(lineFilePath);
            }

            // If the file is matching the criteria, add it to the search results
            if (match.hasMatch()) {
                QMutexLocker locker(&mutex);

                filesFoundList << lineFilePath;
                QFileInfo file(lineFilePath);

                QString lineFileDatetime;
                if (fieldListCount >= 3) {
                    lineFileDatetime = lineFieldList[2];
                } else {
                    lineFileDatetime = fileDate.toString("yyyy/MM/dd hh:mm:ss");
                }

                fileNames.append(file.fileName());
                fileSizes.append(lineFileSize);
                fileDateTimes.append(lineFileDatetime);
                filePaths.append(file.path());
                fileCatalogs.append(sourceDirectory);
                fileCatalogIDs.append(0);
                // metadata arrays
                QString extension = file.suffix().toLower();
                QString fileType = FileMetadata::getFileTypeFromExtension(extension);
                fileTypes.append(fileType);
                mimeTypes.append(""); // Empty for connected drive search
                imageWidths.append(0);
                imageHeights.append(0);
                videoDurations.append(0);
                videoWidths.append(0);
                videoHeights.append(0);
                audioDurations.append(0);
                audioArtists.append("");
                audioAlbums.append("");
                audioTitles.append("");
                if (!deviceFoundIDList.contains(sourceDirectory)) {
                    deviceFoundIDList.append(sourceDirectory);
                }

                // Count found files separately
                filesFoundCount++;
            }
        }
        else {
            // Add all files when not searching on filename
            if (!lineFilePath.isEmpty()) {
                QMutexLocker locker(&mutex);

                filesFoundList << lineFilePath;
                QFileInfo file(lineFilePath);

                QString lineFileDatetime;
                if (fieldListCount >= 3) {
                    lineFileDatetime = lineFieldList[2];
                } else {
                    lineFileDatetime = fileDate.toString("yyyy/MM/dd hh:mm:ss");
                }

                fileNames.append(file.fileName());
                fileSizes.append(lineFileSize);
                fileDateTimes.append(lineFileDatetime);
                filePaths.append(file.path());
                fileCatalogs.append(sourceDirectory);
                fileCatalogIDs.append(0);

                // ADD metadata arrays to keep them in sync with connected drive search
                QString extension = file.suffix().toLower();
                QString fileType = FileMetadata::getFileTypeFromExtension(extension);

                fileTypes.append(fileType);
                mimeTypes.append(""); // Empty for connected drive search
                imageWidths.append(0);
                imageHeights.append(0);
                videoDurations.append(0);
                videoWidths.append(0);
                videoHeights.append(0);
                audioDurations.append(0);
                audioArtists.append("");
                audioAlbums.append("");
                audioTitles.append("");


                if (!deviceFoundIDList.contains(sourceDirectory)) {
                    deviceFoundIDList.append(sourceDirectory);
                }

                // Count found files separately
                filesFoundCount++;
            }
        }

        // Progress reporting based on FILES PROCESSED (not found)
        if (batchProcessedCount >= progressRefreshRate) {
            totalFilesProcessed += batchProcessedCount;  // This now represents files actually processed
            emit searchProgress(totalFilesProcessed);
            batchProcessedCount = 0;

            // CRITICAL: Process events to keep UI responsive (same as searchFilesInCatalog)
            QCoreApplication::processEvents();

            qDebug() << "SearchJobStoppable::searchFilesInDirectory - processed" << totalFilesProcessed << "files, found" << filesFoundCount << "matches";
        }
    }

    // Report remaining processed files
    if (batchProcessedCount > 0) {
        totalFilesProcessed += batchProcessedCount;
        emit searchProgress(totalFilesProcessed);
    }

    qDebug() << "SearchJobStoppable::searchFilesInDirectory completed for:" << sourceDirectory
             << "- Total files processed:" << totalFilesProcessed
             << "- Files found:" << filesFoundCount
             << "- Files in results:" << fileNames.size();
}
//----------------------------------------------------------------------
void SearchJobStoppable::processDuplicates(const QString &connectionName)
{
    qDebug() << "SearchJobStoppable::processDuplicates() starting";

    if (!shouldContinue()) return;

    // Clear database
    QSqlQuery deleteQuery(QSqlDatabase::database(connectionName));
    deleteQuery.exec("DELETE FROM filetemp");

    if (!shouldContinue()) return;

    // Prepare query to load file info
    QSqlQuery insertQuery(QSqlDatabase::database(connectionName));
    QString insertSQL = QLatin1String(R"(
        INSERT INTO filetemp (
            file_name,
            file_folder_path,
            file_size,
            file_date_updated,
            file_catalog,
            file_catalog_id
        ) VALUES(
            :file_name,
            :file_folder_path,
            :file_size,
            :file_date_updated,
            :file_catalog,
            :file_catalog_id
        )
    )");
    insertQuery.prepare(insertSQL);

    // Loop through the result list and populate database
    int rows = rowCount();
    for (int i = 0; i < rows && shouldContinue(); i++) {
        // Check for pause/stop periodically and allow UI updates
        if (i % progressRefreshRate == 0) {
            waitIfPaused();
            if (!shouldContinue()) break;

            int progress = (i * 100) / rows;
            emit searchProgress(progress);
        }

        // Append data to the database
        insertQuery.bindValue(":file_name", index(i, 0).data().toString());
        insertQuery.bindValue(":file_size", index(i, 1).data().toString());
        insertQuery.bindValue(":file_folder_path", index(i, 3).data().toString());
        insertQuery.bindValue(":file_date_updated", index(i, 2).data().toString());
        insertQuery.bindValue(":file_catalog", index(i, 4).data().toString());
        insertQuery.bindValue(":file_catalog_id", index(i, 5).data().toString());
        insertQuery.exec();
    }

    // If search was stopped, return early
    if (!shouldContinue()) {
        qDebug() << "SearchJobStoppable::processDuplicates() stopped during data loading";
        return;
    }

    // Prepare duplicate SQL query
    QString selectSQL;

    // Generate grouping of fields based on user selection, determining what are duplicates
    QString groupingFields; // this value should be a concatenation of fields, like "fileName||fileSize"

    // Same name
    if (searchDuplicatesOnName == true) {
        groupingFields = groupingFields + "file_name";
    }
    // Same size
    if (searchDuplicatesOnSize == true) {
        groupingFields = groupingFields + "||file_size";
    }
    // Same date modified
    if (searchDuplicatesOnDate == true) {
        groupingFields = groupingFields + "||file_date_updated";
    }

    // Remove starting || if any
    if (groupingFields.startsWith("||"))
        groupingFields.remove(0, 2);

    if (!shouldContinue()) return;

    // Generate SQL based on grouping of fields
    selectSQL = QLatin1String(R"(
        SELECT  file_name,
                file_size,
                file_date_updated,
                file_folder_path,
                file_catalog,
                file_catalog_id,
                file_type,
                image_width,
                image_height,
                video_duration_seconds,
                video_width,
                video_height,
                audio_duration_seconds,
                audio_artist,
                audio_album,
                audio_title
        FROM filetemp
        WHERE %1 IN
            (SELECT %1
            FROM filetemp
            GROUP BY %1
            HAVING count(%1)>1)
        ORDER BY %1
    )").arg(groupingFields);

    // Run Query and load to model
    QSqlQuery duplicatesQuery(QSqlDatabase::database(connectionName));
    duplicatesQuery.prepare(selectSQL);
    duplicatesQuery.exec();

    if (!shouldContinue()) return;

    // Recapture file results for stats
    fileNames.clear();
    fileSizes.clear();
    filePaths.clear();
    fileDateTimes.clear();
    fileCatalogs.clear();
    fileCatalogIDs.clear();

    int duplicateCount = 0;
    while (duplicatesQuery.next() && shouldContinue()) {
        // Check for pause/stop periodically
        if (duplicateCount % progressRefreshRate == 0) {
            waitIfPaused();
            if (!shouldContinue()) break;
        }

        fileNames.append(duplicatesQuery.value(0).toString());
        fileSizes.append(duplicatesQuery.value(1).toLongLong());
        fileDateTimes.append(duplicatesQuery.value(2).toString());
        filePaths.append(duplicatesQuery.value(3).toString());
        fileCatalogs.append(duplicatesQuery.value(4).toString());
        fileCatalogIDs.append(duplicatesQuery.value(5).toInt());

        // ADD metadata arrays to keep them in sync
        QString fileName = duplicatesQuery.value(0).toString();
        QFileInfo fileInfo(fileName);
        QString extension = fileInfo.suffix().toLower();
        QString fileType = FileMetadata::getFileTypeFromExtension(extension);

        fileTypes.append(fileType);
        mimeTypes.append(""); // Empty for duplicates search
        imageWidths.append(0);
        imageHeights.append(0);
        videoDurations.append(0);
        videoWidths.append(0);
        videoHeights.append(0);
        audioDurations.append(0);
        audioArtists.append("");
        audioAlbums.append("");
        audioTitles.append("");

        duplicateCount++;
    }

    // Final progress report if completed successfully
    if (shouldContinue()) {
        emit searchProgress(100);
        qDebug() << "SearchJobStoppable::processDuplicates() completed - Found" << duplicateCount << "duplicate entries";
    } else {
        qDebug() << "SearchJobStoppable::processDuplicates() stopped during result processing";
    }
}
//----------------------------------------------------------------------
void SearchJobStoppable::processDifferences(const QString &connectionName)
{
    qDebug() << "SearchJobStoppable::processDifferences() starting";

    if (!shouldContinue()) return;

    // Clear database
    QSqlQuery deleteQuery(QSqlDatabase::database(connectionName));
    deleteQuery.exec("DELETE FROM filetemp");

    if (!shouldContinue()) return;

    // Prepare query to load file info
    QSqlQuery insertQuery(QSqlDatabase::database(connectionName));
    QString insertSQL = QLatin1String(R"(
        INSERT INTO filetemp (
            file_name,
            file_folder_path,
            file_size,
            file_date_updated,
            file_catalog,
            file_catalog_id
        ) VALUES(
            :file_name,
            :file_folder_path,
            :file_size,
            :file_date_updated,
            :file_catalog,
            :file_catalog_id
        )
    )");
    insertQuery.prepare(insertSQL);

    // Loop through the result list and populate database
    int rows = rowCount();
    for (int i = 0; i < rows && shouldContinue(); i++) {
        // Check for pause/stop periodically and allow UI updates
        if (i % progressRefreshRate == 0) {
            waitIfPaused();
            if (!shouldContinue()) break;

            int progress = (i * 100) / rows;
            emit searchProgress(progress);
        }

        // Append data to the database
        insertQuery.bindValue(":file_name", index(i, 0).data().toString());
        insertQuery.bindValue(":file_size", index(i, 1).data().toString());
        insertQuery.bindValue(":file_folder_path", index(i, 3).data().toString());
        insertQuery.bindValue(":file_date_updated", index(i, 2).data().toString());
        insertQuery.bindValue(":file_catalog", index(i, 4).data().toString());
        insertQuery.bindValue(":file_catalog_id", index(i, 5).data().toString());
        insertQuery.exec();
    }

    // If search was stopped, return early
    if (!shouldContinue()) {
        qDebug() << "SearchJobStoppable::processDifferences() stopped during data loading";
        return;
    }

    // Prepare difference SQL
    QString selectSQL;

    // Generate grouping of fields based on user selection, determining what are differences
    QString groupingFieldsDifferences; // this value should be a concatenation of fields, like "fileName||fileSize"

    // Same name
    if (differencesOnName == true) {
        groupingFieldsDifferences += "||file_name";
    }
    // Same size
    if (differencesOnSize == true) {
        groupingFieldsDifferences += "||file_size";
    }
    // Same date modified
    if (differencesOnDate == true) {
        groupingFieldsDifferences += "||file_date_updated";
    }

    // Remove the || at the start
    if (groupingFieldsDifferences.startsWith("||"))
        groupingFieldsDifferences.remove(0, 2);

    if (!shouldContinue()) return;

    // Populate listOfCatalogDeviceIDs1
    QString listOfCatalogDeviceIDs1;
    diffDevice1->loadDevice(connectionName);
    if (diffDevice1->type == "Catalog") {
        listOfCatalogDeviceIDs1 = listOfCatalogDeviceIDs1 + QString::number(diffDevice1->ID) + ",";
    }
    else {
        for (const auto& row : diffDevice1->deviceListTable) {
            if (!shouldContinue()) break;
            if (row.type == "Catalog") {
                listOfCatalogDeviceIDs1 = listOfCatalogDeviceIDs1 + QString::number(row.ID) + ",";
            }
        }
    }
    if (listOfCatalogDeviceIDs1.endsWith(","))
        listOfCatalogDeviceIDs1.remove(listOfCatalogDeviceIDs1.length() - 1, 1);

    if (!shouldContinue()) return;

    // Populate listOfCatalogDeviceIDs2
    QString listOfCatalogDeviceIDs2;
    diffDevice2->loadDevice(connectionName);
    if (diffDevice2->type == "Catalog") {
        listOfCatalogDeviceIDs2 = listOfCatalogDeviceIDs2 + QString::number(diffDevice2->ID) + ",";
    }
    else {
        for (const auto& row : diffDevice2->deviceListTable) {
            if (!shouldContinue()) break;
            if (row.type == "Catalog") {
                listOfCatalogDeviceIDs2 = listOfCatalogDeviceIDs2 + QString::number(row.ID) + ",";
            }
        }
    }
    if (listOfCatalogDeviceIDs2.endsWith(","))
        listOfCatalogDeviceIDs2.remove(listOfCatalogDeviceIDs2.length() - 1, 1);

    if (!shouldContinue()) return;

    // Begin transaction for better performance
    QSqlQuery transactionQuery(QSqlDatabase::database(connectionName));
    transactionQuery.exec("BEGIN TRANSACTION");

    // Generate SQL based on grouping of fields
    selectSQL = QString(R"(
        SELECT  file_name,
                file_size,
                file_date_updated,
                file_folder_path,
                file_catalog,
                file_catalog_id
        FROM filetemp
        WHERE file_catalog_id IN(
            SELECT device_external_id
            FROM device
            WHERE device_id IN(%2)
            AND device_type ='Catalog'
        )
        AND %1 NOT IN(
            SELECT %1
            FROM filetemp
            WHERE file_catalog_id IN(
                SELECT device_external_id
                FROM device
                WHERE device_id IN(%3)
                AND device_type ='Catalog'
            )
        )
        UNION
        SELECT  file_name,
                file_size,
                file_date_updated,
                file_folder_path,
                file_catalog,
                file_catalog_id
        FROM filetemp
        WHERE file_catalog_id IN(
            SELECT device_external_id
            FROM device
            WHERE device_id IN(%3)
            AND device_type ='Catalog'
        )
        AND %1 NOT IN(
            SELECT %1
            FROM filetemp
            WHERE file_catalog_id IN(
                SELECT device_external_id
                FROM device
                WHERE device_id IN(%2)
                AND device_type ='Catalog'
            )
        )
    )").arg(groupingFieldsDifferences, listOfCatalogDeviceIDs1, listOfCatalogDeviceIDs2);

    if (!shouldContinue()) {
        transactionQuery.exec("ROLLBACK");
        return;
    }

    // Run Query
    QSqlQuery differencesQuery(QSqlDatabase::database(connectionName));
    differencesQuery.prepare(selectSQL);

    if (!differencesQuery.exec()) {
        qWarning() << "SearchJobStoppable::processDifferences - Query error:" << differencesQuery.lastError().text();
        transactionQuery.exec("ROLLBACK");
        return;
    }

    // Commit transaction
    transactionQuery.exec("COMMIT");

    if (!shouldContinue()) return;

    // Recapture file results for stats
    fileNames.clear();
    fileSizes.clear();
    filePaths.clear();
    fileDateTimes.clear();
    fileCatalogs.clear();
    fileCatalogIDs.clear();

    // ADD metadata arrays to keep them in sync
    QString fileName = differencesQuery.value(0).toString();
    QFileInfo fileInfo(fileName);
    QString extension = fileInfo.suffix().toLower();
    QString fileType = FileMetadata::getFileTypeFromExtension(extension);

    fileTypes.append(fileType);
    mimeTypes.append(""); // Empty for differences search
    imageWidths.append(0);
    imageHeights.append(0);
    videoDurations.append(0);
    videoWidths.append(0);
    videoHeights.append(0);
    audioDurations.append(0);
    audioArtists.append("");
    audioAlbums.append("");
    audioTitles.append("");

    int differenceCount = 0;
    while (differencesQuery.next() && shouldContinue()) {
        // Check for pause/stop periodically
        if (differenceCount % progressRefreshRate == 0) {
            waitIfPaused();
            if (!shouldContinue()) break;
        }

        fileNames.append(differencesQuery.value(0).toString());
        fileSizes.append(differencesQuery.value(1).toLongLong());
        fileDateTimes.append(differencesQuery.value(2).toString());
        filePaths.append(differencesQuery.value(3).toString());
        fileCatalogs.append(differencesQuery.value(4).toString());
        fileCatalogIDs.append(differencesQuery.value(5).toInt());

        differenceCount++;
    }

    // Final progress report if completed successfully
    if (shouldContinue()) {
        emit searchProgress(100);
        qDebug() << "SearchJobStoppable::processDifferences() completed - Found" << differenceCount << "difference entries";
        qDebug() << "  - Device 1 catalogs:" << listOfCatalogDeviceIDs1;
        qDebug() << "  - Device 2 catalogs:" << listOfCatalogDeviceIDs2;
        qDebug() << "  - Grouping fields:" << groupingFieldsDifferences;
    } else {
        qDebug() << "SearchJobStoppable::processDifferences() stopped during result processing";
    }
}
//----------------------------------------------------------------------
bool SearchJobStoppable::shouldContinue() const
{
    return !m_stopRequested.loadAcquire();
}
//----------------------------------------------------------------------
void SearchJobStoppable::waitIfPaused()
{
    while (m_paused.loadAcquire() && shouldContinue()) {
        QCoreApplication::processEvents();
        QThread::msleep(100);
    }
}
//----------------------------------------------------------------------
