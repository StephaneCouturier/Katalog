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
#include "core/catalogdifferenceengine.h"
#include "core/filemetadata.h"
#include "core/filetypemapping.h"
#include "core/database.h"

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
        if (tempCount > 0) {
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
        // Search in directory
        totalCatalogs = 1;
        currentCatalogIndex = 1;
        currentCatalogName = connectedDirectory;
        QMutex dummyMutex;
        bool dummyStop = false;
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
        (searchDuplicatesOnName || searchDuplicatesOnSize || searchDuplicatesOnDate || searchDuplicatesOnChecksum)) {
        if (duplicatesCompareDevices) {
            // New mode: Compare two devices (find files in common)
            processDuplicatesCompareDevices(m_connectionName);
        } else {
            // Existing mode: Within selected device
            processDuplicates(m_connectionName);
        }
    }

    // Process DIFFERENCES
    if (shouldContinue() && searchOnFileCriteria && searchOnDifferences &&
        (differencesOnName || differencesOnSize || differencesOnDate || differencesOnChecksum)) {
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

    // Add memory mode CSV loading with proper progress reporting with stop/pause control
    if (memoryModeEnabled) {

        // Set opeartion context for csv loading
        currentOperationVerb = "Loaded";
        //currentOperationUnit = "files loaded";
        showSearchStatistics = true;

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
            return;
        }

        // Signal that catalog loading is complete (same as SearchMemory)
        emit searchProgress(-3); // Special signal to indicate catalog loading finished

    }

    //Rest of method is common for all types of database modes
    Q_UNUSED(mutex);
    Q_UNUSED(stopRequested);

    if (!shouldContinue()) {
        return;
    }

    // Emit signal to indicate catalog processing started
        emit searchProgress(-2);

    // Initialize Regular Expression
    QRegularExpression regex(regexPattern);
    if (!caseSensitive) {
        regex.setPatternOptions(QRegularExpression::CaseInsensitiveOption);
    }
    if (!regex.isValid()) {
        emit searchProgress(-1);
        return;
    }

    // Ensure file types are populated for File mode catalogs
    if (!memoryModeEnabled && device->catalog) {

        // Check if migration is actually needed
        QSqlQuery checkQuery(QSqlDatabase::database(m_connectionName));
        checkQuery.prepare("SELECT COUNT(*) FROM file WHERE file_catalog_id = ? "
                           "AND (file_type IS NULL OR file_type = '' "
                           "OR file_extension IS NULL OR file_extension = '' "
                           "OR mime_type IS NULL OR mime_type = '')");
        checkQuery.bindValue(0, device->catalog->ID);

        bool needsMigration = false;
        if (checkQuery.exec() && checkQuery.next()) {
            int filesToMigrate = checkQuery.value(0).toInt();
            needsMigration = (filesToMigrate > 0);

            if (needsMigration) {

                // Set operation context for file type update
                currentOperationVerb = QCoreApplication::translate("MainWindow","File Types Updated");
                //currentOperationUnit = QCoreApplication::translate("MainWindow","updated");
                showSearchStatistics = false;

                // Reset counters for file type update progress
                currentCatalogFilesLoaded = 0;
                currentCatalogTotalFiles = filesToMigrate;

                // Emit start signal
                emit searchProgress(-2);

                // Create Stop flag
                bool localStopRequested = false;
                QMutex dummyMutex;

                // Connect to migration progress
                QMetaObject::Connection progressConnection = connect(
                    device->catalog, &Catalog::loadProgress,
                    this, [this, &localStopRequested, &progressConnection](int filesLoaded, int totalFiles) {
                        if (!shouldContinue()) {
                            QObject::disconnect(progressConnection);
                            localStopRequested = true;
                            return;
                        }

                        currentCatalogFilesLoaded = filesLoaded;
                        currentCatalogTotalFiles = totalFiles;
                        waitIfPaused();
                        emit searchProgress(-4);
                    },
                    Qt::DirectConnection);

                // Perform file type update
                device->catalog->populateFileTypes(dummyMutex, localStopRequested);

                disconnect(progressConnection);

                // Restore normal operation context
                currentOperationVerb = QCoreApplication::translate("MainWindow","Loaded");
                //currentOperationUnit = QCoreApplication::translate("MainWindow","loaded");
                showSearchStatistics = true;

                // Check if stopped
                if (!shouldContinue() || localStopRequested) {
                    return;
                }

                emit searchProgress(-3);
            } else {
            }
        }
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
            file_extension,
            mime_verified,
            type_mismatch,
            image_orientation,
            video_codec,
            video_framerate,
            video_bitrate,
            audio_genre,
            audio_year,
            audio_track_number,
            audio_bitrate,
            audio_sample_rate,
            metadata_extended,
            metadata_extraction_date,
            checksum_sha256,
            checksum_extraction_date
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
        qWarning() << "WARNING: Query error:" << getFilesQuery.lastError().text();
        return;
    }

    emit searchProgress(-3); // Processing files

    int filesProcessed = 0;
    int batchCount = 0;

    while (getFilesQuery.next() && shouldContinue()) {
        // STOP check (existing)
        if (stopRequested || !shouldContinue()) {
            break;
        }

        // Add pause check every N files (simple approach)
        if (filesProcessed % 100 == 0) {  // Check every 100 files
            waitIfPaused();  // This will pause if needed

            // After resuming, check if stop was requested while paused
            if (!shouldContinue()) {
                break;
            }
        }

        QString fileName = getFilesQuery.value(0).toString();
        QString filePath = getFilesQuery.value(1).toString();
        QString fileFullPath = filePath + "/" + fileName;
        qint64 fileSize = getFilesQuery.value(2).toLongLong();
        QString fileDateTime = getFilesQuery.value(3).toString();

        // Metadata fields
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
        QString fileExtension = getFilesQuery.value(15).toString();
        bool mimeVerifiedValue = getFilesQuery.value(16).toBool();
        bool typeMismatchValue = getFilesQuery.value(17).toBool();
        int imageOrientation = getFilesQuery.value(18).toInt();
        QString videoCodec = getFilesQuery.value(19).toString();
        double videoFramerate = getFilesQuery.value(20).toDouble();
        int videoBitrate = getFilesQuery.value(21).toInt();
        QString audioGenre = getFilesQuery.value(22).toString();
        int audioYear = getFilesQuery.value(23).toInt();
        int audioTrackNumber = getFilesQuery.value(24).toInt();
        int audioBitrate = getFilesQuery.value(25).toInt();
        int audioSampleRate = getFilesQuery.value(26).toInt();
        QString metadataExtended = getFilesQuery.value(27).toString();
        QString metadataExtractionDate = getFilesQuery.value(28).toString();

        QString checksumSha256 = getFilesQuery.value(29).toString();
        QString checksumExtractionDate = getFilesQuery.value(30).toString();

        filesProcessed++;
        batchCount++;
        if (filesProcessed % 100 == 0) {
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

            if (!queryTag.exec()) {
                qWarning() << "SearchJobStoppable::searchFilesInCatalog - Tag query failed:" << queryTag.lastError().text();
                continue;
            }

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
                //qDebug() << "Processing file:" << fileFullPath << "match.hasMatch()"<< match.hasMatch();
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
                fileExtensions.append(fileExtension);
                mimeVerified.append(mimeVerifiedValue);
                typeMismatch.append(typeMismatchValue);
                imageOrientations.append(imageOrientation);
                videoCodecs.append(videoCodec);
                videoFramerates.append(videoFramerate);
                videoBitrates.append(videoBitrate);
                audioGenres.append(audioGenre);
                audioYears.append(audioYear);
                audioTrackNumbers.append(audioTrackNumber);
                audioBitrates.append(audioBitrate);
                audioSampleRates.append(audioSampleRate);
                metadataExtendeds.append(metadataExtended);
                metadataExtractionDates.append(metadataExtractionDate);
                checksumSha256s.append(checksumSha256);
                checksumExtractionDates.append(checksumExtractionDate);
            }
        }
        else {
            // Add all files when not searching on filename
            // Files still need to pass other criteria (size, date, type, tags, metadata)
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
            fileExtensions.append(fileExtension);
            mimeVerified.append(mimeVerifiedValue);
            typeMismatch.append(typeMismatchValue);
            imageOrientations.append(imageOrientation);
            videoCodecs.append(videoCodec);
            videoFramerates.append(videoFramerate);
            videoBitrates.append(videoBitrate);
            audioGenres.append(audioGenre);
            audioYears.append(audioYear);
            audioTrackNumbers.append(audioTrackNumber);
            audioBitrates.append(audioBitrate);
            audioSampleRates.append(audioSampleRate);
            metadataExtendeds.append(metadataExtended);
            metadataExtractionDates.append(metadataExtractionDate);
            checksumSha256s.append(checksumSha256);
            checksumExtractionDates.append(checksumExtractionDate);
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

    // Use the same pattern as searchFilesInCatalog - get local reference to stopRequested
    bool &localStopRequested = stopRequested;

    // But ALSO check shouldContinue() since we use atomic variables
    if (!shouldContinue()) {
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


    // Main loop - similar pattern to searchFilesInCatalog but track processing vs finding separately
    while (iterator.hasNext()) {
        // STOP check (existing)
        if (localStopRequested || !shouldContinue()) {
            break;
        }

        // Add pause check every N files (simple approach)
        if (filesProcessedCount % 100 == 0) {  // Check every 100 files
            waitIfPaused();  // This will pause if needed

            // After resuming, check if stop was requested while paused
            if (!shouldContinue()) {
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

        // Apply file type filter
        if (searchOnFileCriteria == true && searchOnType == true && selectedUserFileType != FileTypeMapping::ALL) {
            // Get file extension and determine file type
            QFileInfo fileInfo(lineFilePath);
            QString extension = fileInfo.suffix().toLower();
            QString fileType = FileMetadata::getFileTypeFromExtension(extension);

            // Check if file type matches the selected filter
            bool typeMatches = false;
            switch (selectedUserFileType) {
            case FileTypeMapping::AUDIO:
                typeMatches = (fileType == "audio");
                break;
            case FileTypeMapping::IMAGE:
                typeMatches = (fileType == "image");
                break;
            case FileTypeMapping::VIDEO:
                typeMatches = (fileType == "video");
                break;
            case FileTypeMapping::TEXT:
                typeMatches = (fileType == "text");
                break;
            case FileTypeMapping::OTHER:
                typeMatches = (fileType == "other");
                break;
            case FileTypeMapping::NONE:
                typeMatches = (fileType == "none" || fileType.isEmpty());
                break;
            default:
                typeMatches = true; // ALL - no filtering
                break;
            }

            if (!typeMatches) {
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
                    QFileInfo fileInfo(lineFilePath);
                    match = regex.match(fileInfo.fileName());
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
                checksumSha256s.append("");
                checksumExtractionDates.append(""); // No checksum for connected drive search


                if (!deviceFoundIDList.contains(sourceDirectory)) {
                    deviceFoundIDList.append(sourceDirectory);
                }

                // Count found files separately
                filesFoundCount++;
            }
        }

        // Progress reporting based on FILES PROCESSED (not files found)
        if (batchProcessedCount >= progressRefreshRate) {
            totalFilesProcessed += batchProcessedCount;
            currentFilePath = iterator.filePath();  // Store current file path
            emit searchProgress(totalFilesProcessed);
            batchProcessedCount = 0;

            // CRITICAL: Process events to keep UI responsive (same as searchFilesInCatalog)
            QCoreApplication::processEvents();

        }
    }

    // Report remaining processed files
    if (batchProcessedCount > 0) {
        totalFilesProcessed += batchProcessedCount;
        emit searchProgress(totalFilesProcessed);
    }

}
//----------------------------------------------------------------------
void SearchJobStoppable::processDuplicates(const QString &connectionName)
{

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
            file_catalog_id,
            checksum_sha256
        ) VALUES(
            :file_name,
            :file_folder_path,
            :file_size,
            :file_date_updated,
            :file_catalog,
            :file_catalog_id,
            :checksum_sha256
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
        insertQuery.bindValue(":checksum_sha256", index(i, 17).data().toString());
        insertQuery.exec();
    }

    // If search was stopped, return early
    if (!shouldContinue()) {
        return;
    }

    // Generate grouping of fields based on user selection, determining what are duplicates
    QString groupingFields;
    QString checksumGroupingFields;  // Fields WITHOUT checksum for ≠ case

    // Same name
    if (searchDuplicatesOnName) {
        groupingFields += "file_name";
        checksumGroupingFields += "file_name";
    }
    // Same size
    if (searchDuplicatesOnSize) {
        groupingFields += "||file_size";
        checksumGroupingFields += "||file_size";
    }
    // Same date modified
    if (searchDuplicatesOnDate) {
        groupingFields += "||file_date_updated";
        checksumGroupingFields += "||file_date_updated";
    }
    // Checksum (only for = case, added to main grouping)
    if (searchDuplicatesOnChecksum && searchDuplicatesChecksumEqual) {
        groupingFields += "||checksum_sha256";
    }

    // Remove starting || if any
    if (groupingFields.startsWith("||"))
        groupingFields.remove(0, 2);
    if (checksumGroupingFields.startsWith("||"))
        checksumGroupingFields.remove(0, 2);

    if (!shouldContinue()) return;

    // Prepare duplicate SQL query
    QString selectSQL;

    if (searchDuplicatesOnChecksum && !searchDuplicatesChecksumEqual) {
        // Checksum ≠ case: find groups (by other fields) with multiple DISTINCT checksums
        // Exclude files without checksum (NULL or empty)
        selectSQL = QString(R"(
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
                    audio_title,
                    checksum_sha256
            FROM filetemp
            WHERE checksum_sha256 IS NOT NULL
              AND checksum_sha256 != ''
              AND %1 IN (
                SELECT %1
                FROM filetemp
                WHERE checksum_sha256 IS NOT NULL
                  AND checksum_sha256 != ''
                GROUP BY %1
                HAVING COUNT(DISTINCT checksum_sha256) > 1
              )
            ORDER BY %1, checksum_sha256
        )").arg(checksumGroupingFields);
    } else {
        // Standard case (including Checksum = case)
        selectSQL = QString(R"(
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
                    audio_title,
                    checksum_sha256
            FROM filetemp
            WHERE %1 IN (
                SELECT %1
                FROM filetemp
                GROUP BY %1
                HAVING count(%1) > 1
            )
            ORDER BY %1
        )").arg(groupingFields);
    }

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
        checksumSha256s.append("");
        checksumExtractionDates.append("");

        duplicateCount++;
    }

    // Final progress report if completed successfully
    if (shouldContinue()) {
        emit searchProgress(100);
    } else {
    }
}

//----------------------------------------------------------------------
void SearchJobStoppable::processDuplicatesCompareDevices(const QString &connectionName)
{

    if (!shouldContinue()) return;

    // Clear filetemp
    QSqlQuery deleteQuery(QSqlDatabase::database(connectionName));
    deleteQuery.exec("DELETE FROM filetemp");

    if (!shouldContinue()) return;

    // Generate grouping fields based on user selection
    QString groupingFields;
    QString checksumGroupingFields;  // Fields WITHOUT checksum for ≠ case

    if (searchDuplicatesOnName) {
        groupingFields += "||file_name";
        checksumGroupingFields += "||file_name";
    }
    if (searchDuplicatesOnSize) {
        groupingFields += "||file_size";
        checksumGroupingFields += "||file_size";
    }
    if (searchDuplicatesOnDate) {
        groupingFields += "||file_date_updated";
        checksumGroupingFields += "||file_date_updated";
    }
    if (searchDuplicatesOnChecksum && searchDuplicatesChecksumEqual) {
        groupingFields += "||checksum_sha256";
    }

    // Remove leading ||
    if (groupingFields.startsWith("||"))
        groupingFields.remove(0, 2);
    if (checksumGroupingFields.startsWith("||"))
        checksumGroupingFields.remove(0, 2);

    if (!shouldContinue()) return;

    // Build list of catalog IDs for Device1
    QString listOfCatalogDeviceIDs1;
    duplicatesDevice1->loadDevice(connectionName);
    if (duplicatesDevice1->type == "Catalog") {
        listOfCatalogDeviceIDs1 = QString::number(duplicatesDevice1->ID);
    } else {
        QStringList ids;
        for (const auto& row : duplicatesDevice1->deviceListTable) {
            if (!shouldContinue()) break;
            if (row.type == "Catalog") {
                ids << QString::number(row.ID);
            }
        }
        listOfCatalogDeviceIDs1 = ids.join(",");
    }

    if (!shouldContinue()) return;

    // Build list of catalog IDs for Device2
    QString listOfCatalogDeviceIDs2;
    duplicatesDevice2->loadDevice(connectionName);
    if (duplicatesDevice2->type == "Catalog") {
        listOfCatalogDeviceIDs2 = QString::number(duplicatesDevice2->ID);
    } else {
        QStringList ids;
        for (const auto& row : duplicatesDevice2->deviceListTable) {
            if (!shouldContinue()) break;
            if (row.type == "Catalog") {
                ids << QString::number(row.ID);
            }
        }
        listOfCatalogDeviceIDs2 = ids.join(",");
    }


    if (!shouldContinue()) return;

    // Begin transaction
    Database::beginTransaction(connectionName);

    QString selectSQL;

    if (searchDuplicatesOnChecksum && !searchDuplicatesChecksumEqual) {
        // Checksum ≠ case: files matching on other fields but with different checksums
        selectSQL = QString(R"(
            SELECT  f1.file_name,
                    f1.file_size,
                    f1.file_date_updated,
                    f1.file_folder_path,
                    f1.file_catalog,
                    f1.file_catalog_id,
                    f1.file_type,
                    f1.image_width,
                    f1.image_height,
                    f1.video_duration_seconds,
                    f1.video_width,
                    f1.video_height,
                    f1.audio_duration_seconds,
                    f1.audio_artist,
                    f1.audio_album,
                    f1.audio_title,
                    f1.checksum_sha256
            FROM file f1
            INNER JOIN file f2 ON %1
            WHERE f1.checksum_sha256 IS NOT NULL
              AND f1.checksum_sha256 != ''
              AND f2.checksum_sha256 IS NOT NULL
              AND f2.checksum_sha256 != ''
              AND f1.checksum_sha256 != f2.checksum_sha256
              AND f1.file_catalog_id IN (
                  SELECT device_external_id FROM device
                  WHERE device_id IN (%2) AND device_type = 'Catalog'
              )
              AND f2.file_catalog_id IN (
                  SELECT device_external_id FROM device
                  WHERE device_id IN (%3) AND device_type = 'Catalog'
              )
            ORDER BY %4
        )").arg(
                            buildJoinCondition("f1", "f2", checksumGroupingFields),
                            listOfCatalogDeviceIDs1,
                            listOfCatalogDeviceIDs2,
                            checksumGroupingFields.replace("||", ", ")
                            );
    } else {
        // Standard case: find files that exist in BOTH devices (intersection)
        // Files from Device1 that have a match in Device2
        selectSQL = QString(R"(
            SELECT  f1.file_name,
                    f1.file_size,
                    f1.file_date_updated,
                    f1.file_folder_path,
                    f1.file_catalog,
                    f1.file_catalog_id,
                    f1.file_type,
                    f1.image_width,
                    f1.image_height,
                    f1.video_duration_seconds,
                    f1.video_width,
                    f1.video_height,
                    f1.audio_duration_seconds,
                    f1.audio_artist,
                    f1.audio_album,
                    f1.audio_title,
                    f1.checksum_sha256
            FROM file f1
            WHERE f1.file_catalog_id IN (
                SELECT device_external_id FROM device
                WHERE device_id IN (%1) AND device_type = 'Catalog'
            )
            AND EXISTS (
                SELECT 1 FROM file f2
                WHERE f2.file_catalog_id IN (
                    SELECT device_external_id FROM device
                    WHERE device_id IN (%2) AND device_type = 'Catalog'
                )
                AND %3
            )
            UNION
            SELECT  f2.file_name,
                    f2.file_size,
                    f2.file_date_updated,
                    f2.file_folder_path,
                    f2.file_catalog,
                    f2.file_catalog_id,
                    f2.file_type,
                    f2.image_width,
                    f2.image_height,
                    f2.video_duration_seconds,
                    f2.video_width,
                    f2.video_height,
                    f2.audio_duration_seconds,
                    f2.audio_artist,
                    f2.audio_album,
                    f2.audio_title,
                    f2.checksum_sha256
            FROM file f2
            WHERE f2.file_catalog_id IN (
                SELECT device_external_id FROM device
                WHERE device_id IN (%2) AND device_type = 'Catalog'
            )
            AND EXISTS (
                SELECT 1 FROM file f1
                WHERE f1.file_catalog_id IN (
                    SELECT device_external_id FROM device
                    WHERE device_id IN (%1) AND device_type = 'Catalog'
                )
                AND %3
            )
            ORDER BY %4
        )").arg(
                            listOfCatalogDeviceIDs1,
                            listOfCatalogDeviceIDs2,
                            buildExistsCondition("f1", "f2", groupingFields),
                            groupingFields.replace("||", ", ")
                            );
    }


    if (!shouldContinue()) {
        Database::commitTransaction(connectionName);
        return;
    }

    // Execute query
    QSqlQuery duplicatesQuery(QSqlDatabase::database(connectionName));
    duplicatesQuery.prepare(selectSQL);

    if (!duplicatesQuery.exec()) {
        qWarning() << "processDuplicatesCompareDevices - Query error:" << duplicatesQuery.lastError().text();
        Database::rollbackTransaction(connectionName);
        return;
    }

    Database::commitTransaction(connectionName);

    if (!shouldContinue()) return;

    // Clear and populate results
    fileNames.clear();
    fileSizes.clear();
    filePaths.clear();
    fileDateTimes.clear();
    fileCatalogs.clear();
    fileCatalogIDs.clear();
    fileTypes.clear();
    mimeTypes.clear();
    imageWidths.clear();
    imageHeights.clear();
    videoDurations.clear();
    videoWidths.clear();
    videoHeights.clear();
    audioDurations.clear();
    audioArtists.clear();
    audioAlbums.clear();
    audioTitles.clear();
    checksumSha256s.clear();
    checksumExtractionDates.clear();

    int resultCount = 0;
    while (duplicatesQuery.next() && shouldContinue()) {
        if (resultCount % progressRefreshRate == 0) {
            waitIfPaused();
            if (!shouldContinue()) break;
        }

        fileNames.append(duplicatesQuery.value(0).toString());
        fileSizes.append(duplicatesQuery.value(1).toLongLong());
        fileDateTimes.append(duplicatesQuery.value(2).toString());
        filePaths.append(duplicatesQuery.value(3).toString());
        fileCatalogs.append(duplicatesQuery.value(4).toString());
        fileCatalogIDs.append(duplicatesQuery.value(5).toInt());
        fileTypes.append(duplicatesQuery.value(6).toString());
        imageWidths.append(duplicatesQuery.value(7).toInt());
        imageHeights.append(duplicatesQuery.value(8).toInt());
        videoDurations.append(duplicatesQuery.value(9).toInt());
        videoWidths.append(duplicatesQuery.value(10).toInt());
        videoHeights.append(duplicatesQuery.value(11).toInt());
        audioDurations.append(duplicatesQuery.value(12).toInt());
        audioArtists.append(duplicatesQuery.value(13).toString());
        audioAlbums.append(duplicatesQuery.value(14).toString());
        audioTitles.append(duplicatesQuery.value(15).toString());
        checksumSha256s.append(duplicatesQuery.value(16).toString());

        mimeTypes.append("");
        checksumExtractionDates.append("");

        resultCount++;
    }

    if (shouldContinue()) {
        emit searchProgress(100);
    }
}
//----------------------------------------------------------------------
void SearchJobStoppable::processDifferences(const QString &connectionName)
{

    if (!shouldContinue()) return;

    // Step 1: Populate filetemp with current search results
    QSqlQuery deleteQuery(QSqlDatabase::database(connectionName));
    deleteQuery.exec("DELETE FROM filetemp");

    if (!shouldContinue()) return;

    QSqlQuery insertQuery(QSqlDatabase::database(connectionName));
    QString insertSQL = QLatin1String(R"(
        INSERT INTO filetemp (
            file_name,
            file_folder_path,
            file_size,
            file_date_updated,
            file_catalog,
            file_catalog_id,
            checksum_sha256
        ) VALUES(
            :file_name,
            :file_folder_path,
            :file_size,
            :file_date_updated,
            :file_catalog,
            :file_catalog_id,
            :checksum_sha256
        )
    )");
    insertQuery.prepare(insertSQL);

    int rows = rowCount();
    for (int i = 0; i < rows && shouldContinue(); i++) {
        if (i % progressRefreshRate == 0) {
            waitIfPaused();
            if (!shouldContinue()) break;

            int progress = (i * 100) / rows;
            emit searchProgress(progress);
        }

        insertQuery.bindValue(":file_name", index(i, 0).data().toString());
        insertQuery.bindValue(":file_size", index(i, 1).data().toString());
        insertQuery.bindValue(":file_folder_path", index(i, 3).data().toString());
        insertQuery.bindValue(":file_date_updated", index(i, 2).data().toString());
        insertQuery.bindValue(":file_catalog", index(i, 4).data().toString());
        insertQuery.bindValue(":file_catalog_id", index(i, 5).data().toString());
        insertQuery.bindValue(":checksum_sha256", index(i, 19).data().toString());
        insertQuery.exec();
    }

    if (!shouldContinue()) {
        return;
    }

    // Step 2: Build comparison fields from user selection
    CatalogDifferenceEngine::CompareFields matchFields;
    if (differencesOnName)
        matchFields |= CatalogDifferenceEngine::Name;
    if (differencesOnSize)
        matchFields |= CatalogDifferenceEngine::Size;
    if (differencesOnDate)
        matchFields |= CatalogDifferenceEngine::Date;
    if (differencesOnChecksum && differencesChecksumEqual)
        matchFields |= CatalogDifferenceEngine::Checksum;

    bool checksumNotEqual = differencesOnChecksum && !differencesChecksumEqual;

    if (!shouldContinue()) return;

    // Step 3: Resolve device IDs to catalog device IDs
    QList<int> sourceIds = CatalogDifferenceEngine::resolveCatalogDeviceIds(diffDevice1, connectionName);
    if (!shouldContinue()) return;
    QList<int> targetIds = CatalogDifferenceEngine::resolveCatalogDeviceIds(diffDevice2, connectionName);
    if (!shouldContinue()) return;

    // Step 4: Run comparison via engine
    CatalogDifferenceEngine engine(connectionName);
    DifferenceResult diffResult = engine.compare(sourceIds, targetIds, matchFields, checksumNotEqual);

    if (!shouldContinue()) return;

    // Step 5: Convert engine results to search result arrays
    fileNames.clear();
    fileSizes.clear();
    filePaths.clear();
    fileDateTimes.clear();
    fileCatalogs.clear();
    fileCatalogIDs.clear();
    fileTypes.clear();
    mimeTypes.clear();
    imageWidths.clear();
    imageHeights.clear();
    videoDurations.clear();
    videoWidths.clear();
    videoHeights.clear();
    audioDurations.clear();
    audioArtists.clear();
    audioAlbums.clear();
    audioTitles.clear();
    checksumSha256s.clear();
    checksumExtractionDates.clear();

    // Combine results: for standard mode both sides, for checksum≠ mode the conflicts
    QList<DifferenceFileEntry> allEntries;
    if (checksumNotEqual) {
        allEntries = diffResult.differentContent;
    } else {
        allEntries = diffResult.onlyInSource + diffResult.onlyInTarget;
    }

    int differenceCount = 0;
    for (const auto &entry : allEntries) {
        if (differenceCount % progressRefreshRate == 0) {
            waitIfPaused();
            if (!shouldContinue()) break;
        }

        fileNames.append(entry.fileName);
        fileSizes.append(entry.fileSize);
        fileDateTimes.append(entry.dateUpdated);
        filePaths.append(entry.folderPath);
        fileCatalogs.append(entry.catalog);
        fileCatalogIDs.append(entry.catalogId);

        // Metadata placeholders (not available from filetemp comparison)
        QFileInfo fileInfo(entry.fileName);
        QString extension = fileInfo.suffix().toLower();
        fileTypes.append(FileMetadata::getFileTypeFromExtension(extension));
        mimeTypes.append("");
        imageWidths.append(0);
        imageHeights.append(0);
        videoDurations.append(0);
        videoWidths.append(0);
        videoHeights.append(0);
        audioDurations.append(0);
        audioArtists.append("");
        audioAlbums.append("");
        audioTitles.append("");
        checksumSha256s.append(entry.checksum);
        checksumExtractionDates.append("");

        differenceCount++;
    }

    if (shouldContinue()) {
        emit searchProgress(100);
    } else {
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
QString SearchJobStoppable::buildJoinCondition(const QString &alias1, const QString &alias2, const QString &fields)
{
    // Convert "file_name||file_size" to "f1.file_name = f2.file_name AND f1.file_size = f2.file_size"
    QStringList fieldList = fields.split("||");
    QStringList conditions;
    for (const QString &field : fieldList) {
        conditions << QString("%1.%2 = %3.%2").arg(alias1, field.trimmed(), alias2);
    }
    return conditions.join(" AND ");
}

//----------------------------------------------------------------------
QString SearchJobStoppable::buildExistsCondition(const QString &alias1, const QString &alias2, const QString &fields)
{
    // Same as buildJoinCondition
    return buildJoinCondition(alias1, alias2, fields);
}
//----------------------------------------------------------------------
