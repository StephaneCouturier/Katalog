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
// File Name:   search_memory.cpp
// Purpose:     methods for the search class that can not be stopped
// Description: https://stephanecouturier.github.io/Katalog/docs/Features/Search
// Author:      Stephane Couturier
/////////////////////////////////////////////////////////////////////////////
*///

#include "search_memory.h"
#include "qapplication.h"
#include <QSqlQuery>
#include <QSqlError>
#include <QFileInfo>
#include <QDateTime>
#include <QDirIterator>

SearchMemory::SearchMemory(QObject *parent) : Search(parent)
{
    // Initialize progress tracking
    currentCatalogIndex = 0;
    totalCatalogs = 0;
    currentCatalogFilesLoaded = 0;
    currentCatalogTotalFiles = 0;
    currentCatalogName = "";
}

void SearchMemory::searchFiles(Device *selectedDevice)
{
    // Clear previous results
    clearResults();

    // Reset catalog tracking
    currentCatalogIndex = 0;
    totalCatalogs = 0;
    currentCatalogFilesLoaded = 0;
    currentCatalogTotalFiles = 0;
    currentCatalogName = "";

    // Initialize progress tracking
    initializeProgressTracking(selectedDevice);

    // Setup common search patterns
    prepareSearchPatterns();

    // Create temporary mutex and stop flag for memory mode (not really used for thread control)
    QMutex tempMutex;
    bool tempStopRequested = false;

    // Process the SEARCH in CATALOGS or DIRECTORY
    if (searchInCatalogsChecked == true) {
        // Count total catalogs for progress tracking
        if (searchOnDifferences == true) {
            // For differences, we need to count catalogs in both devices
            totalCatalogs = 0;

            // Count catalogs in diffDevice1
            diffDevice1->loadDevice("defaultConnection");
            if (diffDevice1->type == "Catalog") {
                totalCatalogs++;
            } else {
                for (const auto& row : diffDevice1->deviceListTable) {
                    if (row.type == "Catalog") {
                        totalCatalogs++;
                    }
                }
            }

            // Count catalogs in diffDevice2
            diffDevice2->loadDevice("defaultConnection");
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

        // Reset current catalog index
        currentCatalogIndex = 0;

        // Emit progress start
        emit searchProgress(0);

        // For differences, only process the 2 selected catalogs
        if (searchOnDifferences == true) {
            // Load diffDevice1 files
            diffDevice1->loadDevice("defaultConnection");

            if (diffDevice1->type == "Catalog") {
                currentCatalogIndex++;
                currentCatalogName = diffDevice1->name;
                searchFilesInCatalog(diffDevice1, tempMutex, tempStopRequested);
            }
            else {
                foreach(const Device::deviceListRow & row, diffDevice1->deviceListTable) {
                    if (row.type == "Catalog") {
                        currentCatalogIndex++;
                        Device *device = new Device;
                        device->ID = row.ID;
                        device->loadDevice("defaultConnection");
                        currentCatalogName = device->name;
                        searchFilesInCatalog(device, tempMutex, tempStopRequested);
                        delete device;
                    }
                }
            }

            // Load diffDevice2 files
            diffDevice2->loadDevice("defaultConnection");

            if (diffDevice2->type == "Catalog") {
                currentCatalogIndex++;
                currentCatalogName = diffDevice2->name;
                searchFilesInCatalog(diffDevice2, tempMutex, tempStopRequested);
            }
            else {
                foreach(const Device::deviceListRow & row, diffDevice2->deviceListTable) {
                    if (row.type == "Catalog") {
                        currentCatalogIndex++;
                        Device *device = new Device;
                        device->ID = row.ID;
                        device->loadDevice("defaultConnection");
                        currentCatalogName = device->name;
                        searchFilesInCatalog(device, tempMutex, tempStopRequested);
                        delete device;
                    }
                }
            }
        }
        // Otherwise (not a "difference" search), search in the list of catalogs in the selectedDevice
        else {
            if (selectedDevice->type == "Catalog") {
                currentCatalogIndex++;
                currentCatalogName = selectedDevice->name;
                searchFilesInCatalog(selectedDevice, tempMutex, tempStopRequested);
            }
            else {
                foreach(const Device::deviceListRow & row, selectedDevice->deviceListTable) {
                    if (row.type == "Catalog") {
                        currentCatalogIndex++;
                        Device *device = new Device;
                        device->ID = row.ID;
                        device->loadDevice("defaultConnection");
                        currentCatalogName = device->name;
                        searchFilesInCatalog(device, tempMutex, tempStopRequested);
                        delete device;
                    }
                }
            }
        }
    }
    // Process the SEARCH in SELECTED DIRECTORY
    else if (searchInConnectedChecked == true) {
        // For directories, we count it as a single catalog
        totalCatalogs = 1;
        currentCatalogIndex = 1;
        currentCatalogName = connectedDirectory;
        searchFilesInDirectory(connectedDirectory, tempMutex, tempStopRequested);
    }

    // Process search results
    processResults();

    // Process DUPLICATES
    if (searchOnFileCriteria == true && searchOnDuplicates == true
        && (searchDuplicatesOnName == true
            || searchDuplicatesOnSize == true
            || searchDuplicatesOnDate == true)) {
        // Emit progress update
        emit searchProgress(50);

        processDuplicates("defaultConnection");
    }

    // Process DIFFERENCES
    if (searchOnFileCriteria == true && searchOnDifferences == true
        && (differencesOnName == true
            || differencesOnSize == true
            || differencesOnDate == true)) {
        // Emit progress update
        emit searchProgress(75);

        processDifferences("defaultConnection");
    }

    // Calculate statistics
    calculateStatistics();

    // Save the search criteria to the search history
    saveSearchHistoryToTable("defaultConnection");

    emit searchProgress(totalFilesProcessed);
}

void SearchMemory::searchFilesInCatalog(Device *device, QMutex &mutex, bool &stopRequested)
{
    // Reset catalog file loading counters
    currentCatalogFilesLoaded = 0;
    currentCatalogTotalFiles = device->totalFileCount;

    // Track loading progress
    emit searchProgress(-2); // Special signal to indicate catalog loading started

    // Connect to the loadProgress signal of the catalog to track loading progress
    connect(device->catalog, &Catalog::loadProgress, this,
            [this](int filesLoaded, int totalFiles) {
                currentCatalogFilesLoaded = filesLoaded;
                currentCatalogTotalFiles = totalFiles;

                // Send a special progress signal with loading info
                emit searchProgress(-4); // -4 is catalog loading progress update
            });

    // Load the catalog file contents if not already loaded in memory
    device->catalog->loadCatalogFileListToTable("defaultConnection", mutex, stopRequested);

    // Disconnect to prevent memory leaks
    disconnect(device->catalog, &Catalog::loadProgress, this, nullptr);

    // Signal that catalog loading is complete
    emit searchProgress(-3); // Special signal to indicate catalog loading finished

    // Initialize Regular Expression
    QRegularExpression regex(regexPattern);
    if (caseSensitive != true) {
        regex.setPatternOptions(QRegularExpression::CaseInsensitiveOption);
    }

    // Search loop for all lines in the catalog file
    // Load the files of the Catalog
    QSqlQuery getFilesQuery(QSqlDatabase::database("defaultConnection"));
    QString getFilesQuerySQL = QLatin1String(R"(
                                            SELECT  file_name,
                                                    file_folder_path,
                                                    file_size,
                                                    file_date_updated
                                            FROM  file
                                            WHERE file_catalog_id =:file_catalog_id
                                        )");

    // Add matching size range
    if (searchOnFileCriteria == true && searchOnSize == true) {
        getFilesQuerySQL = getFilesQuerySQL + " AND file_size>=:file_size_min ";
        getFilesQuerySQL = getFilesQuerySQL + " AND file_size<=:file_size_max ";
    }
    // Add matching date range
    if (searchOnFileCriteria == true && searchOnDate == true) {
        getFilesQuerySQL = getFilesQuerySQL + " AND file_date_updated>=:file_date_updated_min ";
        getFilesQuerySQL = getFilesQuerySQL + " AND file_date_updated<=:file_date_updated_max ";
    }
    getFilesQuery.prepare(getFilesQuerySQL);
    getFilesQuery.bindValue(":file_catalog_id", device->catalog->ID);
    getFilesQuery.bindValue(":file_size_min", selectedMinimumSize * sizeMultiplierMin);
    getFilesQuery.bindValue(":file_size_max", selectedMaximumSize * sizeMultiplierMax);
    getFilesQuery.bindValue(":file_date_updated_min", selectedDateMin.toString("yyyy/MM/dd hh:mm:ss"));
    getFilesQuery.bindValue(":file_date_updated_max", selectedDateMax.toString("yyyy/MM/dd hh:mm:ss"));
    getFilesQuery.exec();

    // Get total file count for progress reporting
    int totalFiles = 0;
    QSqlQuery countQuery(QSqlDatabase::database("defaultConnection"));
    countQuery.prepare("SELECT COUNT(*) FROM file WHERE file_catalog_id = :file_catalog_id");
    countQuery.bindValue(":file_catalog_id", device->catalog->ID);
    if (countQuery.exec() && countQuery.next()) {
        totalFiles = countQuery.value(0).toInt();
    }

    // Local counter for batch processing
    int batchCount = 0;
    int filesProcessed = 0;

    // File by file, test if the file is matching all search criteria
    while (getFilesQuery.next() && !stopRequested) {
        // Progress reporting
        batchCount++;
        filesProcessed++;

        // Emit searchProgress using configurable refresh rate
        if (batchCount >= progressRefreshRate) {
            totalFilesProcessed += batchCount;
            emit searchProgress(totalFilesProcessed);
            batchCount = 0;
        }

        QString lineFileName = getFilesQuery.value(0).toString();
        QString lineFileFolderPath = getFilesQuery.value(1).toString();
        QString lineFileFullPath = lineFileFolderPath + "/" + lineFileName;
        bool fileIsMatchingTag;

        // Continue to the next file if the current file is not matching the tags
        if (searchOnFolderCriteria == true && searchOnTags == true && selectedTagName != "") {
            fileIsMatchingTag = false;

            // Set query to get a list of folder paths matching the selected tag
            QSqlQuery queryTag(QSqlDatabase::database("defaultConnection"));
            QString queryTagSQL = QLatin1String(R"(
                                                SELECT path
                                                FROM tag
                                                WHERE name=:name
                            )");
            queryTag.prepare(queryTagSQL);
            queryTag.bindValue(":name", selectedTagName);
            queryTag.exec();

            // Test if the FilePath contains a path from the list of folders matching the selected tag name
            while (queryTag.next()) {
                if ((lineFileFolderPath + "/").contains(queryTag.value(0).toString() + "/") == true) {
                    fileIsMatchingTag = true;
                    break;
                }
            }

            // If the file is not matching any of the paths, process the next file
            if (!fileIsMatchingTag == true) {
                continue;
            }
        }

        // Finally, verify the text search criteria
        QRegularExpressionMatch match;
        QRegularExpressionMatch foldermatch;

        if (searchOnFileName == true) {
            // Depends on the "Search in" criteria,
            // Reduces the absolute path to the required text string and matches the search text
            if (selectedSearchIn == Search::SEARCH_IN_FILE_NAMES) {
                match = regex.match(lineFileName);
            }
            else if (selectedSearchIn == Search::SEARCH_IN_FOLDER_PATH) {
                // Check that the folder name matches the search text
                regex.setPattern(regexSearchtext);
                foldermatch = regex.match(lineFileFolderPath);
                // If it does, then check that the file matches the selected file type
                if (foldermatch.hasMatch() && searchOnType == true) {
                    regex.setPattern(regexFileType);
                    match = regex.match(lineFileName);
                }
                else
                    match = foldermatch;
            }
            else { // Search::SEARCH_IN_FILES_AND_FOLDERS
                match = regex.match(lineFileFullPath);
            }

            // If the file is matching the criteria, add it and its catalog to the search results
            if (match.hasMatch()) {
                filesFoundList << lineFileFolderPath;
                deviceFoundIDList.insert(0, QString::number(device->ID));

                // Populate result lists
                fileNames.append(lineFileName);
                filePaths.append(lineFileFolderPath);
                fileSizes.append(getFilesQuery.value(2).toLongLong());
                fileDateTimes.append(getFilesQuery.value(3).toString());
                fileCatalogs.append(device->name);
                fileCatalogIDs.append(device->catalog->ID);
            }
        }
        else {
            // Verify file matches the selected file type
            if (searchOnType == true) {
                regex.setPattern(regexFileType);
            }
            match = regex.match(lineFileFolderPath);
            if (!match.hasMatch()) {
                continue;
            }

            // Add the file and its catalog to the results, excluding blank lines
            if (lineFileFolderPath != "") {
                filesFoundList << lineFileFolderPath;
                deviceFoundIDList.insert(0, QString::number(device->ID));

                // Populate result lists
                fileNames.append(lineFileName);
                filePaths.append(lineFileFolderPath);
                fileSizes.append(getFilesQuery.value(2).toLongLong());
                fileDateTimes.append(getFilesQuery.value(3).toString());
                fileCatalogs.append(device->name);
                fileCatalogIDs.append(device->catalog->ID);
            }
        }
        // Report progress periodically using configurable rate
        if (batchCount >= progressRefreshRate) {
            totalFilesProcessed += batchCount;
            emit searchProgress(totalFilesProcessed);
            // Process events to update UI
            QApplication::processEvents();
            batchCount = 0;
        }
    }
    //Report any remaining files in the last batch
    // Report any remaining files in the last batch
    if (batchCount > 0) {
        totalFilesProcessed += batchCount;
        emit searchProgress(totalFilesProcessed);
    }
}

void SearchMemory::searchFilesInDirectory(const QString &sourceDirectory, QMutex &mutex, bool &stopRequested)
{
    // Not a case, searchFilesInDirectory is addressed as a stoppable search.
}

void SearchMemory::processDuplicates(const QString &connectionName)
{
    // Clear database
    QSqlQuery deleteQuery(QSqlDatabase::database(connectionName));
    deleteQuery.exec("DELETE FROM filetemp");

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
    for (int i = 0; i < rows; i++) {
        // Report progress using configurable refresh rate
        if (i % progressRefreshRate == 0) {
            int filesProcessed = (i * 100) / rows;
            emit searchProgress(filesProcessed);
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

    // Generate SQL based on grouping of fields
    selectSQL = QLatin1String(R"(
        SELECT  file_name,
                file_size,
                file_date_updated,
                file_folder_path,
                file_catalog,
                file_catalog_id
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

    // Recapture file results for stats
    fileNames.clear();
    fileSizes.clear();
    filePaths.clear();
    fileDateTimes.clear();
    fileCatalogs.clear();
    fileCatalogIDs.clear();

    while (duplicatesQuery.next()) {
        fileNames.append(duplicatesQuery.value(0).toString());
        fileSizes.append(duplicatesQuery.value(1).toLongLong());
        fileDateTimes.append(duplicatesQuery.value(2).toString());
        filePaths.append(duplicatesQuery.value(3).toString());
        fileCatalogs.append(duplicatesQuery.value(4).toString());
        fileCatalogIDs.append(duplicatesQuery.value(5).toInt());
    }

    // Final progress report
    emit searchProgress(100);
}

void SearchMemory::processDifferences(const QString &connectionName)
{
    // Clear database
    QSqlQuery deleteQuery(QSqlDatabase::database(connectionName));
    deleteQuery.exec("DELETE FROM filetemp");

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
    for (int i = 0; i < rows; i++) {
        // Report progress
        if (i % 100 == 0) {
            int filesProcessed = (i * 100) / rows;
            emit searchProgress(filesProcessed);
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

    // Populate listOfCatalogDeviceIDs1
    QString listOfCatalogDeviceIDs1;
    diffDevice1->loadDevice(connectionName);
    if (diffDevice1->type == "Catalog") {
        listOfCatalogDeviceIDs1 = listOfCatalogDeviceIDs1 + QString::number(diffDevice1->ID) + ",";
    }
    else {
        for (const auto& row : diffDevice1->deviceListTable) {
            if (row.type == "Catalog") {
                listOfCatalogDeviceIDs1 = listOfCatalogDeviceIDs1 + QString::number(row.ID) + ",";
            }
        }
    }
    if (listOfCatalogDeviceIDs1.endsWith(","))
        listOfCatalogDeviceIDs1.remove(listOfCatalogDeviceIDs1.length() - 1, 1);

    // Populate listOfCatalogDeviceIDs2
    QString listOfCatalogDeviceIDs2;
    diffDevice2->loadDevice(connectionName);
    if (diffDevice2->type == "Catalog") {
        listOfCatalogDeviceIDs2 = listOfCatalogDeviceIDs2 + QString::number(diffDevice2->ID) + ",";
    }
    else {
        for (const auto& row : diffDevice2->deviceListTable) {
            if (row.type == "Catalog") {
                listOfCatalogDeviceIDs2 = listOfCatalogDeviceIDs2 + QString::number(row.ID) + ",";
            }
        }
    }
    if (listOfCatalogDeviceIDs2.endsWith(","))
        listOfCatalogDeviceIDs2.remove(listOfCatalogDeviceIDs2.length() - 1, 1);

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

    // Run Query
    QSqlQuery differencesQuery(QSqlDatabase::database(connectionName));
    differencesQuery.prepare(selectSQL);
    differencesQuery.exec();

    // Recapture file results for stats
    fileNames.clear();
    fileSizes.clear();
    filePaths.clear();
    fileDateTimes.clear();
    fileCatalogs.clear();
    fileCatalogIDs.clear();

    while (differencesQuery.next()) {
        fileNames.append(differencesQuery.value(0).toString());
        fileSizes.append(differencesQuery.value(1).toLongLong());
        fileDateTimes.append(differencesQuery.value(2).toString());
        filePaths.append(differencesQuery.value(3).toString());
        fileCatalogs.append(differencesQuery.value(4).toString());
        fileCatalogIDs.append(differencesQuery.value(5).toInt());
    }

    // Final progress report
    emit searchProgress(100);
}
