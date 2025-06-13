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

#include "searchjobstoppable.h"
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

SearchJobStoppable::~SearchJobStoppable()
{
    // Ensure search is stopped
    stopSearch();
}

void SearchJobStoppable::setDatabaseConnection(const QString &connectionName)
{
    m_connectionName = connectionName;
}

void SearchJobStoppable::searchFiles(Device *selectedDevice)
{
    qDebug() << "SearchJobStoppable::searchFiles() starting";
    qDebug() << "  - Selected device type:" << selectedDevice->type;
    qDebug() << "  - Selected device name:" << selectedDevice->name;
    qDebug() << "  - Database connection:" << m_connectionName;
    qDebug() << "  - Search in catalogs checked:" << searchInCatalogsChecked;
    qDebug() << "  - Search text:" << searchText;

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
        qDebug() << "  - filetemp table has" << tempCount << "records at start";
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

void SearchJobStoppable::stopSearch()
{
    m_stopRequested.storeRelease(1);

    // Wake up any paused operations
    if (m_paused.loadAcquire()) {
        m_paused.storeRelease(0);
    }
}

void SearchJobStoppable::pauseSearch()
{
    m_paused.storeRelease(1);
}

void SearchJobStoppable::resumeSearch()
{
    m_paused.storeRelease(0);
}

void SearchJobStoppable::searchFilesInCatalog(Device *device, QMutex &mutex, bool &stopRequested)
{
    qDebug() << "SearchJobStoppable::searchFilesInCatalog() starting for device:" << device->name;
    qDebug() << "  - Device external ID:" << device->externalID;
    qDebug() << "  - Database connection:" << m_connectionName;

    Q_UNUSED(mutex);
    Q_UNUSED(stopRequested);

    if (!shouldContinue()) return;

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
                file_date_updated
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
    qDebug() << "  - SQL query executed successfully";

    emit searchProgress(-3); // Processing files

    int filesProcessed = 0;
    int batchCount = 0;

    while (getFilesQuery.next() && shouldContinue()) {
        waitIfPaused();
        if (filesProcessed == 0) {
            qDebug() << "  - Starting to process files from database";
        }
        QString lineFileName = getFilesQuery.value(0).toString();
        QString lineFileFolderPath = getFilesQuery.value(1).toString();
        QString lineFileFullPath = lineFileFolderPath + "/" + lineFileName;

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
                if ((lineFileFolderPath + "/").contains(queryTag.value(0).toString() + "/")) {
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
                match = regex.match(lineFileName);
            } else if (selectedSearchIn == SEARCH_IN_FOLDER_PATH) {
                regex.setPattern(regexSearchtext);
                auto foldermatch = regex.match(lineFileFolderPath);
                if (foldermatch.hasMatch() && searchOnType) {
                    regex.setPattern(regexFileType);
                    match = regex.match(lineFileName);
                } else {
                    match = foldermatch;
                }
            } else { // SEARCH_IN_FILES_AND_FOLDERS
                match = regex.match(lineFileFullPath);
            }

            if (match.hasMatch()) {
                filesFoundList << lineFileFolderPath;
                deviceFoundIDList.insert(0, QString::number(device->ID));

                fileNames.append(lineFileName);
                filePaths.append(lineFileFolderPath);
                fileSizes.append(getFilesQuery.value(2).toLongLong());
                fileDateTimes.append(getFilesQuery.value(3).toString());
                fileCatalogs.append(device->name);
                fileCatalogIDs.append(device->externalID);
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

void SearchJobStoppable::searchFilesInDirectory(const QString &sourceDirectory, QMutex &mutex, bool &stopRequested)
{
    Q_UNUSED(mutex);
    Q_UNUSED(stopRequested);

    // This implementation is similar to SearchStoppable but cleaned up
    // For now, we'll keep it simple and just indicate it's not implemented
    qDebug() << "SearchJobStoppable::searchFilesInDirectory not yet implemented for:" << sourceDirectory;
}

void SearchJobStoppable::processDuplicates(const QString &connectionName)
{
    if (!shouldContinue()) return;

    // Implementation similar to SearchStoppable but with stop/pause checks
    // For brevity, keeping the core logic but adding shouldContinue() checks
    Search::processDuplicates(connectionName);
}

void SearchJobStoppable::processDifferences(const QString &connectionName)
{
    if (!shouldContinue()) return;

    // Implementation similar to SearchStoppable but with stop/pause checks
    Search::processDifferences(connectionName);
}

void SearchJobStoppable::checkStopAndPause()
{
    waitIfPaused();
}

bool SearchJobStoppable::shouldContinue() const
{
    return !m_stopRequested.loadAcquire();
}

void SearchJobStoppable::waitIfPaused()
{
    while (m_paused.loadAcquire() && shouldContinue()) {
        QCoreApplication::processEvents();
        QThread::msleep(100);
    }
}
