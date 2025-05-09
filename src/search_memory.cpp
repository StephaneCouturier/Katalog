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
#include <QCoreApplication>

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
        // Every 100 files
        if (filesProcessed % 100 == 0 && totalFiles > 0) {
            emit searchProgress(filesProcessed);
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
            if (selectedSearchIn == QCoreApplication::translate("MainWindow", "File names only")) {
                match = regex.match(lineFileName);
            }
            else if (selectedSearchIn == QCoreApplication::translate("MainWindow", "Folder path only")) {
                // Check that the folder name matches the search text
                regex.setPattern(regexSearchtext);
                foldermatch = regex.match(lineFileFolderPath);
                // If it does, then check that the file matches the selected file type
                if (foldermatch.hasMatch() && searchOnType == true) {
                    regex.setPattern(regexFileType);
                    match = regex.match(lineFileName);
                }
                else
                    match = foldermatch; // selectedSearchIn == QCoreApplication::translate("MainWindow", "Files and Folder paths")
            }
            else {
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
        // Report progress periodically
        if (batchCount >= 100) {
            totalFilesProcessed += batchCount;
            emit searchProgress(totalFilesProcessed);
            // Process events to update UI
            QApplication::processEvents();
            batchCount = 0;
        }
    }
    //Report any remaining files in the last batch
    if (batchCount > 0) {
        totalFilesProcessed += batchCount;
        emit searchProgress(totalFilesProcessed);
        QApplication::processEvents();
    }
}

void SearchMemory::searchFilesInDirectory(const QString &sourceDirectory, QMutex &mutex, bool &stopRequested)
{
/*    // Initialize Regular Expression
    QRegularExpression regex(regexPattern, QRegularExpression::CaseInsensitiveOption);

    // Filetypes
    // Get the file type for the catalog
    QStringList fileTypes;

    // Progress tracking variables
    int filesProcessed = 0;

    // Scan directory and create a list of files
    QString line;
    QString reducedLine;

    QDirIterator iterator(sourceDirectory, fileTypes, QDir::Files | QDir::Hidden, QDirIterator::Subdirectories);

    while (iterator.hasNext() && !stopRequested) {
        // Get file information (absolute path, size, datetime)
        QString filePath = iterator.next();

        // Report progress based on files processed
        filesProcessed++;
        if (filesProcessed % 100 == 0) {
            // For directory searches, we don't know the total count,
            // so just report the number of files processed
            emit searchProgress(filesProcessed);
        }

        QFileInfo fileInfo(filePath);
        QDateTime fileDate = fileInfo.lastModified();
        line = fileInfo.absoluteFilePath() + "\t" + QString::number(fileInfo.size()) + "\t" + fileDate.toString("yyyy/MM/dd hh:mm:ss");

        // Split the line text with tabulations into a list
        QRegularExpression lineSplitExp("\t");
        QStringList lineFieldList = line.split(lineSplitExp);
        int fieldListCount = lineFieldList.count();

        // Get the file absolute path from this list
        QString lineFilePath = lineFieldList[0];

        // Get the FileSize from the list if available
        qint64 lineFileSize;
        if (fieldListCount == 3) { lineFileSize = lineFieldList[1].toLongLong(); }
        else lineFileSize = 0;

        // Get the File DateTime from the list if available
        QDateTime lineFileDateTime;
        if (fieldListCount == 3) { lineFileDateTime = QDateTime::fromString(lineFieldList[2], "yyyy/MM/dd hh:mm:ss"); }
        else lineFileDateTime = QDateTime::fromString("0001/01/01 00:00:00", "yyyy/MM/dd hh:mm:ss");

        // Exclude catalog metadata lines which are starting with the character <
        if (lineFilePath.left(1) == "<") { continue; }

        // Continue if the file is matching the size range
        if (searchOnSize == true) {
            if (!(lineFileSize >= selectedMinimumSize * sizeMultiplierMin
                  && lineFileSize <= selectedMaximumSize * sizeMultiplierMax)) {
                continue;
            }
        }

        // Continue if the file is matching the date range
        if (searchOnDate == true) {
            if (!(lineFileDateTime >= selectedDateMin
                  && lineFileDateTime <= selectedDateMax)) {
                continue;
            }
        }

        // Continue if the file is matching the tags
        if (searchOnTags == true) {
            bool fileIsMatchingTag = false;

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
                if (lineFilePath.contains(queryTag.value(0).toString()) == true) {
                    fileIsMatchingTag = true;
                    break;
                }
                // else tagIsMatching==false
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
            // Depending on the "Search in" criteria,
            // reduce the absolute path to the required text string and match the search text
            if (selectedSearchIn == QCoreApplication::translate("MainWindow", "File names only")) {
                // Extract the file name from the lineFilePath
                QFileInfo file(lineFilePath);
                reducedLine = file.fileName();

                match = regex.match(reducedLine);
            }
            else if (selectedSearchIn == QCoreApplication::translate("MainWindow", "Folder path only")) {
                // Keep only the folder name, so all characters left of the last occurrence of / in the path.
                reducedLine = lineFilePath.left(lineFilePath.lastIndexOf("/"));

                // Check the folder name matches the search text
                regex.setPattern(regexSearchtext);
                foldermatch = regex.match(reducedLine);

                // If it does, then check that the file matches the selected file type
                if (foldermatch.hasMatch()) {
                    regex.setPattern(regexFileType);
                    match = regex.match(lineFilePath);
                }
            }
            else {
                match = regex.match(lineFilePath);
            }

            // If the file is matching the criteria, add it and its catalog to the search results
            if (match.hasMatch()) {
                filesFoundList << lineFilePath;

                // Retrieve other file info
                QFileInfo file(lineFilePath);

                // Get the fileDateTime from the list if available
                QString lineFileDatetime;
                if (fieldListCount == 3) {
                    lineFileDatetime = lineFieldList[2];
                }
                else lineFileDatetime = "";

                // Populate result lists
                fileNames.append(file.fileName());
                fileSizes.append(lineFileSize);
                fileDateTimes.append(lineFileDatetime);
                filePaths.append(file.path());
                fileCatalogs.append(sourceDirectory);
                fileCatalogIDs.append(0);
            }
        }
        else {
            // Add the file and its catalog to the results, excluding blank lines
            if (lineFilePath != "") {
                filesFoundList << lineFilePath;
                deviceFoundIDList.insert(0, sourceDirectory);

                // Retrieve other file info
                QFileInfo file(lineFilePath);

                // Get the fileDateTime from the list if available
                QString lineFileDatetime;
                if (fieldListCount == 3) {
                    lineFileDatetime = lineFieldList[2];
                }
                else lineFileDatetime = "";

                // Populate result lists
                fileNames.append(file.fileName());
                fileSizes.append(lineFileSize);
                fileDateTimes.append(lineFileDatetime);
                filePaths.append(file.path());
                fileCatalogs.append(sourceDirectory);
                fileCatalogIDs.append(0);
            }
        }
    }

    // Final progress report when finished
    emit searchProgress(filesProcessed);
*/}

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

void SearchMemory::saveSearchHistoryToTable(const QString &connectionName)
{
    // Save Search to db
    QDateTime nowDateTime = QDateTime::currentDateTime();
    searchDateTime = nowDateTime.toString("yyyy-MM-dd hh:mm:ss");

    QSqlQuery query(QSqlDatabase::database(connectionName));
    QString querySQL = QLatin1String(R"(
        INSERT INTO search(
            date_time,
            text_checked,
            text_phrase,
            text_criteria,
            text_search_in,
            file_criteria_checked,
            file_type_checked,
            file_type,
            file_size_checked,
            file_size_min,
            file_size_min_unit,
            file_size_max,
            file_size_max_unit,
            date_modified_checked,
            date_modified_min,
            date_modified_max,
            duplicates_checked,
            duplicates_name,
            duplicates_size,
            duplicates_date_modified,
            differences_checked,
            differences_name,
            differences_size,
            differences_date_modified,
            differences_catalogs,
            folder_criteria_checked,
            show_folders,
            tag_checked,
            tag,
            search_location,
            search_storage,
            search_catalog,
            search_catalog_checked,
            search_directory_checked,
            selected_directory,
            text_exclude,
            case_sensitive
        ) VALUES(
            :date_time,
            :text_checked,
            :text_phrase,
            :text_criteria,
            :text_search_in,
            :file_criteria_checked,
            :file_type_checked,
            :file_type,
            :file_size_checked,
            :file_size_min,
            :file_size_min_unit,
            :file_size_max,
            :file_size_max_unit,
            :date_modified_checked,
            :date_modified_min,
            :date_modified_max,
            :duplicates_checked,
            :duplicates_name,
            :duplicates_size,
            :duplicates_date_modified,
            :differences_checked,
            :differences_name,
            :differences_size,
            :differences_date_modified,
            :differences_catalogs,
            :folder_criteria_checked,
            :show_folders,
            :tag_checked,
            :tag,
            :search_location,
            :search_storage,
            :search_catalog,
            :search_catalog_checked,
            :search_directory_checked,
            :selected_directory,
            :text_exclude,
            :case_sensitive
        )
    )");

    query.prepare(querySQL);
    query.bindValue(":date_time", searchDateTime);
    query.bindValue(":text_checked", searchOnFileName);
    query.bindValue(":text_phrase", searchText);
    query.bindValue(":text_criteria", selectedTextCriteria);
    query.bindValue(":text_search_in", selectedSearchIn);
    query.bindValue(":file_criteria_checked", searchOnFileCriteria);
    query.bindValue(":file_type_checked", searchOnType);
    query.bindValue(":file_type", selectedFileType);
    query.bindValue(":file_size_checked", searchOnSize);
    query.bindValue(":file_size_min", selectedMinimumSize);
    query.bindValue(":file_size_min_unit", selectedMinSizeUnit);
    query.bindValue(":file_size_max", selectedMaximumSize);
    query.bindValue(":file_size_max_unit", selectedMaxSizeUnit);
    query.bindValue(":date_modified_checked", searchOnDate);
    query.bindValue(":date_modified_min", selectedDateMin);
    query.bindValue(":date_modified_max", selectedDateMax);
    query.bindValue(":duplicates_checked", searchOnDuplicates);
    query.bindValue(":duplicates_name", searchDuplicatesOnName);
    query.bindValue(":duplicates_size", searchDuplicatesOnSize);
    query.bindValue(":duplicates_date_modified", searchDuplicatesOnDate);
    query.bindValue(":differences_checked", searchOnDifferences);
    query.bindValue(":differences_name", differencesOnName);
    query.bindValue(":differences_size", differencesOnSize);
    query.bindValue(":differences_date_modified", differencesOnDate);
    query.bindValue(":differences_catalogs", QString::number(differencesDeviceID1) + "||" + QString::number(differencesDeviceID2));
    query.bindValue(":folder_criteria_checked", searchOnFolderCriteria);
    query.bindValue(":show_folders", showFoldersOnly);
    query.bindValue(":tag_checked", searchOnTags);
    query.bindValue(":tag", selectedTagName);
    query.bindValue(":search_storage", selectedStorage);
    query.bindValue(":search_catalog", selectedCatalog);
    query.bindValue(":search_catalog_checked", searchInCatalogsChecked);
    query.bindValue(":search_directory_checked", searchInConnectedChecked);
    query.bindValue(":selected_directory", connectedDirectory);
    query.bindValue(":text_exclude", selectedSearchExclude);
    query.bindValue(":case_sensitive", caseSensitive);
    query.exec();
}

void SearchMemory::loadSearchHistoryCriteria(const QString &connectionName)
{
    // Query
    QSqlQuery query(QSqlDatabase::database(connectionName));
    QString querySQL = QLatin1String(R"(
        SELECT
            date_time,
            text_checked,
            text_phrase,
            text_criteria,
            text_search_in,
            case_sensitive,
            text_exclude,
            file_criteria_checked,
            file_size_checked,
            file_size_min,
            file_size_min_unit,
            file_size_max,
            file_size_max_unit,
            file_type_checked,
            file_type,
            date_modified_checked,
            date_modified_min,
            date_modified_max,
            duplicates_checked,
            duplicates_name,
            duplicates_size,
            duplicates_date_modified,
            differences_checked,
            differences_name,
            differences_size,
            differences_date_modified,
            differences_catalogs,
            folder_criteria_checked,
            show_folders,
            tag_checked,
            tag,
            search_location,
            search_storage,
            search_catalog,
            search_catalog_checked,
            search_directory_checked,
            selected_directory
        FROM search
        WHERE date_time =:date_time
    )");
    query.prepare(querySQL);
    query.bindValue(":date_time", searchDateTime);
    query.exec();

    if (query.next()) {
        searchOnFileName = query.value(1).toBool();
        searchText = query.value(2).toString();
        selectedTextCriteria = query.value(3).toString();
        selectedSearchIn = query.value(4).toString();
        caseSensitive = query.value(5).toBool();
        selectedSearchExclude = query.value(6).toString();
        searchOnFileCriteria = query.value(7).toBool();
        searchOnSize = query.value(8).toBool();
        selectedMinimumSize = query.value(9).toLongLong();
        selectedMinSizeUnit = query.value(10).toString();
        selectedMaximumSize = query.value(11).toLongLong();
        selectedMaxSizeUnit = query.value(12).toString();
        searchOnType = query.value(13).toBool();
        selectedFileType = query.value(14).toString();
        searchOnDate = query.value(15).toBool();
        selectedDateMin = query.value(16).toDateTime();
        selectedDateMax = query.value(17).toDateTime();
        searchOnDuplicates = query.value(18).toBool();
        searchDuplicatesOnName = query.value(19).toBool();
        searchDuplicatesOnSize = query.value(20).toBool();
        searchDuplicatesOnDate = query.value(21).toBool();
        searchOnDifferences = query.value(22).toBool();
        differencesOnName = query.value(23).toBool();
        differencesOnSize = query.value(24).toBool();
        differencesOnDate = query.value(25).toBool();
        differencesDevices = query.value(26).toString().split("||");
        if (differencesDevices.length() > 1) {
            differencesDeviceID1 = differencesDevices[0].toInt();
            differencesDeviceID2 = differencesDevices[1].toInt();
        }
        searchOnFolderCriteria = query.value(27).toBool();
        showFoldersOnly = query.value(28).toBool();
        searchOnTags = query.value(29).toBool();
        selectedTagName = query.value(30).toString();
        selectedStorage = query.value(32).toString();
        selectedCatalog = query.value(33).toString();
        searchInCatalogsChecked = query.value(34).toBool();
        searchInConnectedChecked = query.value(35).toBool();
        connectedDirectory = query.value(36).toString();

        // Calculate multipliers based on loaded units
        setMultipliers();
    }
}
