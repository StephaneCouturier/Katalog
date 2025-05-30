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
// File Name:   search_stoppable.cpp
// Purpose:     methods for the search class that can be stopped
// Description: https://stephanecouturier.github.io/Katalog/docs/Features/Search
// Author:      Stephane Couturier
/////////////////////////////////////////////////////////////////////////////
*/

#include "../mainwindow.h"
#include "search_stoppable.h"
#include "qsqlerror.h"
#include <QElapsedTimer>

SearchStoppable::SearchStoppable(QObject *parent) : Search(parent), stopRequested(false)
{
    // Constructor implementation
    connectionName = "dbSearchConnection";

    // Initialize catalog tracking
    currentCatalogIndex = 0;
    totalCatalogs = 0;
    currentCatalogName = "";
}

SearchStoppable::~SearchStoppable()
{
    // Destructor implementation
}

bool SearchStoppable::initializeDatabase()
{
    // Close and remove any existing connection with this name
    if (QSqlDatabase::contains(connectionName)) {
        {
            QSqlDatabase db = QSqlDatabase::database(connectionName);
            db.close();
        }
        QSqlDatabase::removeDatabase(connectionName);
    }

    // Get a pointer to the MainWindow instance
    MainWindow* mainWindow = qobject_cast<MainWindow*>(parent());
    if (!mainWindow) {
        qWarning() << "SearchStoppable::initializeDatabase - Failed to get MainWindow pointer";
        return false;
    }

    // Use the MainWindow's initializeDatabase method
    QSqlError err = mainWindow->initializeDatabase(connectionName);
    if (err.type() != QSqlError::NoError) {
        qWarning() << "SearchStoppable::initializeDatabase - Failed to initialize database:" << err.text();
        return false;
    }

    // Clear any cached results to ensure we're using the new database
    clearResults();

    return true;
}

void SearchStoppable::searchFiles(Device *selectedDevice)
{
    QElapsedTimer totalTimer;
    totalTimer.start();
    QElapsedTimer stepTimer;
    stepTimer.start();

    // Clear previous results
    clearResults();

    // Reset catalog tracking
    currentCatalogIndex = 0;
    totalCatalogs = 0;
    currentCatalogName = "";

    // Initialize progress tracking
    initializeProgressTracking(selectedDevice);

    // Ensure we're using the current database connection
    if (!initializeDatabase()) {
        qWarning() << "SearchStoppable: Failed to initialize database connection";
        return;
    }

    // Setup common search patterns
    prepareSearchPatterns();
    qDebug() << "TIMER: Prepare took:" << stepTimer.elapsed() << "ms";
    stepTimer.restart();

    // Reset stop flag
    stopRequested = false;

    // Process the SEARCH in CATALOGS or DIRECTORY
    if (searchInCatalogsChecked) {
        stepTimer.restart();
        // Count total catalogs for progress tracking
        if (searchOnDifferences) {
            // For differences, we need to count catalogs in both devices
            totalCatalogs = 0;

            // Count catalogs in diffDevice1
            diffDevice1->loadDevice(connectionName);
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
            diffDevice2->loadDevice(connectionName);
            if (diffDevice2->type == "Catalog") {
                totalCatalogs++;
            } else {
                for (const auto& row : diffDevice2->deviceListTable) {
                    if (row.type == "Catalog") {
                        totalCatalogs++;
                    }
                }
            }
        }
        else {
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
        qDebug() << "TIMER: Count total catalogs took:" << stepTimer.elapsed() << "ms";
        stepTimer.restart();

        // Reset current catalog index
        currentCatalogIndex = 0;

        // Emit progress start
        emit searchProgress(0);

        // For differences, only process the 2 selected catalogs
        if (searchOnDifferences) {
            // Load diffDevice1 files
            diffDevice1->loadDevice(connectionName);

            if (diffDevice1->type == "Catalog") {
                currentCatalogIndex++;
                currentCatalogName = diffDevice1->name;
                searchFilesInCatalog(diffDevice1, mutex, stopRequested);
            } else {
                foreach(const Device::deviceListRow & row, diffDevice1->deviceListTable) {
                    if (row.type == "Catalog") {
                        currentCatalogIndex++;
                        Device *device = new Device;
                        device->ID = row.ID;
                        device->loadDevice(connectionName);
                        currentCatalogName = device->name;
                        searchFilesInCatalog(device, mutex, stopRequested);
                        delete device;
                    }
                    if (stopRequested) break;
                }
            }

            if (!stopRequested) {
                // Load diffDevice2 files
                diffDevice2->loadDevice(connectionName);

                if (diffDevice2->type == "Catalog") {
                    currentCatalogIndex++;
                    currentCatalogName = diffDevice2->name;
                    searchFilesInCatalog(diffDevice2, mutex, stopRequested);
                } else {
                    foreach(const Device::deviceListRow & row, diffDevice2->deviceListTable) {
                        if (row.type == "Catalog") {
                            currentCatalogIndex++;
                            Device *device = new Device;
                            device->ID = row.ID;
                            device->loadDevice(connectionName);
                            currentCatalogName = device->name;
                            searchFilesInCatalog(device, mutex, stopRequested);
                            delete device;
                        }
                        if (stopRequested) break;
                    }
                }
            }
        }
        // Otherwise (not a "difference" search), search in the list of catalogs in the selectedDevice
        else {
            if (selectedDevice->type == "Catalog") {
                currentCatalogIndex++;
                currentCatalogName = selectedDevice->name;
                qDebug() << "TIMER: reaching searchFilesInCatalog took:" << stepTimer.elapsed() << "ms";
                stepTimer.restart();
                searchFilesInCatalog(selectedDevice, mutex, stopRequested);
                qDebug() << "TIMER: searchFilesInCatalog took:" << stepTimer.elapsed() << "ms";
                stepTimer.restart();
            }
            else {
                foreach(const Device::deviceListRow & row, selectedDevice->deviceListTable) {
                    if (row.type == "Catalog") {
                        currentCatalogIndex++;
                        qDebug() << "TIMER: entering searchFilesInCatalog:" << stepTimer.elapsed() << "ms";
                        stepTimer.restart();
                        Device *device = new Device;
                        device->ID = row.ID;
                        device->loadDevice(connectionName);
                        currentCatalogName = device->name;
                        qDebug() << "TIMER: reaching searchFilesInCatalog took:" << stepTimer.elapsed() << "ms";
                        stepTimer.restart();
                        searchFilesInCatalog(device, mutex, stopRequested);
                        qDebug() << "TIMER: searchFilesInCatalog took:" << stepTimer.elapsed() << "ms";
                        stepTimer.restart();
                        delete device;
                    }
                    if (stopRequested) break;
                }
            }
        }
    }
    // Process the SEARCH in SELECTED DIRECTORY
    else if (searchInConnectedChecked) {
        // For directories, we count it as a single catalog
        totalCatalogs = 1;
        currentCatalogIndex = 1;
        currentCatalogName = connectedDirectory;
        searchFilesInDirectory(connectedDirectory, mutex, stopRequested);
    }

    // If search was not stopped, process results
    if (!stopRequested) {
        // Process search results
        qDebug() << "TIMER: reaching processResults took:" << stepTimer.elapsed() << "ms";
        stepTimer.restart();
        processResults();
        qDebug() << "TIMER: processResults took:" << stepTimer.elapsed() << "ms";
        stepTimer.restart();

        // Process DUPLICATES
        if (searchOnFileCriteria && searchOnDuplicates &&
            (searchDuplicatesOnName || searchDuplicatesOnSize || searchDuplicatesOnDate)) {
            processDuplicates(connectionName);
        }

        // Process DIFFERENCES
        if (searchOnFileCriteria && searchOnDifferences &&
            (differencesOnName || differencesOnSize || differencesOnDate)) {
            processDifferences(connectionName);
        }

        // Calculate statistics
        calculateStatistics();
        qDebug() << "TIMER: calculateStatistics took:" << stepTimer.elapsed() << "ms";
        stepTimer.restart();
    }
    // Final progress report - confirm 100% completion
    emit searchProgress(totalFilesProcessed);
    qDebug() << "TIMER: Total search time:" << totalTimer.elapsed() << "ms \n";
}

void SearchStoppable::stopSearch()
{
    QMutexLocker locker(&mutex);
    stopRequested = true;

    // Calculate statistics for partial results
    calculateStatistics();

    // Emit a final progress signal to update the status bar
    emit searchProgress(-1); // -1 indicates the search was interrupted
}

void SearchStoppable::searchFilesInCatalog(Device *device, QMutex &mutex, bool &stopRequested)
{
    QElapsedTimer totalTimer;
    totalTimer.start();

    QElapsedTimer stepTimer;
    stepTimer.start();

    // Emit signal to indicate catalog loading started
    emit searchProgress(-2);

    // Initialize Regular Expression
    QRegularExpression regex(regexPattern);
    if (caseSensitive != true) {
        regex.setPatternOptions(QRegularExpression::CaseInsensitiveOption);
    }

    // Get a local reference to the stopRequested flag
    bool &localStopRequested = stopRequested;

    // Search loop for all lines in the catalog file
    // Load the files of the Catalog
    QSqlQuery getFilesQuery(QSqlDatabase::database(connectionName));
    QString getFilesQuerySQL = QLatin1String(R"(
                                            SELECT  file_name,
                                                    file_folder_path,
                                                    file_size,
                                                    file_date_updated
                                            FROM  file
                                            WHERE file_catalog_id = :file_catalog_id
                                        )");

    // Add matching size range
    if (searchOnFileCriteria == true && searchOnSize == true) {
        getFilesQuerySQL = getFilesQuerySQL + " AND file_size >= :file_size_min ";
        getFilesQuerySQL = getFilesQuerySQL + " AND file_size <= :file_size_max ";
    }
    // Add matching date range
    if (searchOnFileCriteria == true && searchOnDate == true) {
        getFilesQuerySQL = getFilesQuerySQL + " AND file_date_updated >= :file_date_updated_min ";
        getFilesQuerySQL = getFilesQuerySQL + " AND file_date_updated <= :file_date_updated_max ";
    }
    getFilesQuery.prepare(getFilesQuerySQL);
    getFilesQuery.bindValue(":file_catalog_id", device->externalID);
    getFilesQuery.bindValue(":file_size_min", selectedMinimumSize * sizeMultiplierMin);
    getFilesQuery.bindValue(":file_size_max", selectedMaximumSize * sizeMultiplierMax);
    getFilesQuery.bindValue(":file_date_updated_min", selectedDateMin.toString("yyyy/MM/dd hh:mm:ss"));
    getFilesQuery.bindValue(":file_date_updated_max", selectedDateMax.toString("yyyy/MM/dd hh:mm:ss"));
    getFilesQuery.exec();

    qDebug() << "   TIMER2: getFilesQuery took:" << stepTimer.elapsed() << "ms";
    stepTimer.restart();

    // File by file, test if the file is matching all search criteria
    int filesProcessed = 0;
    int totalFiles = 0;
    int batchCount = 0;
    emit searchProgress(-3);

    // Get total count for progress reporting
    QSqlQuery countQuery(QSqlDatabase::database(connectionName));
    countQuery.prepare("SELECT COUNT(*) FROM file WHERE file_catalog_id = :file_catalog_id");
    countQuery.bindValue(":file_catalog_id", device->externalID);
    countQuery.exec();
    if (countQuery.next()) {
        totalFiles = countQuery.value(0).toInt();
    }
    qDebug() << "   TIMER2: countQuery took:" << stepTimer.elapsed() << "ms";
    stepTimer.restart();

    while (getFilesQuery.next() && !localStopRequested) {
        QString lineFileName = getFilesQuery.value(0).toString();
        QString lineFileFolderPath = getFilesQuery.value(1).toString();
        QString lineFileFullPath = lineFileFolderPath + "/" + lineFileName;
        bool fileIsMatchingTag;

        // Update progress every 100 files
        batchCount++;
        filesProcessed++;
        // Emit searchProgress using configurable refresh rate
        if (filesProcessed % progressRefreshRate == 0 && totalFiles > 0) {
            emit searchProgress(filesProcessed);
        }

        // Continue to the next file if the current file is not matching the tags
        if (searchOnFolderCriteria == true && searchOnTags == true && selectedTagName != "") {
            fileIsMatchingTag = false;

            // Set query to get a list of folder paths matching the selected tag
            QSqlQuery queryTag(QSqlDatabase::database(connectionName));
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
                QMutexLocker locker(&mutex); // Lock when modifying shared data
                filesFoundList << lineFileFolderPath;
                deviceFoundIDList.insert(0, QString::number(device->ID));

                // Populate result lists
                fileNames.append(lineFileName);
                filePaths.append(lineFileFolderPath);
                fileSizes.append(getFilesQuery.value(2).toLongLong());
                fileDateTimes.append(getFilesQuery.value(3).toString());
                fileCatalogs.append(device->name);
                fileCatalogIDs.append(device->externalID);
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
                QMutexLocker locker(&mutex); // Lock when modifying shared data
                filesFoundList << lineFileFolderPath;
                deviceFoundIDList.insert(0, QString::number(device->ID));

                // Populate result lists
                fileNames.append(lineFileName);
                filePaths.append(lineFileFolderPath);
                fileSizes.append(getFilesQuery.value(2).toLongLong());
                fileDateTimes.append(getFilesQuery.value(3).toString());
                fileCatalogs.append(device->name);
                fileCatalogIDs.append(device->externalID);
            }
        }
        // Report progress using configurable refresh rate
        if (batchCount >= progressRefreshRate) {
            updateProgress(batchCount);
            batchCount = 0;
        }
    }

    qDebug() << "   TIMER2: getFilesQuery parsing took:" << stepTimer.elapsed() << "ms";
    stepTimer.restart();

    // Report any remaining files in the last batch
    if (batchCount > 0) {
        updateProgress(batchCount);
    }
     qDebug() << "   TIMER2: Total catalog search time:" << totalTimer.elapsed() << "ms";
}

void SearchStoppable::searchFilesInDirectory(const QString &sourceDirectory, QMutex &mutex, bool &stopRequested)
{
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

    // Filetypes
    // Get the file type for the catalog
    QStringList fileTypes;

    // Scan directory and create a list of files
    QDirIterator iterator(sourceDirectory, fileTypes, QDir::Files | QDir::Hidden, QDirIterator::Subdirectories);

    // Count files for progress reporting
    int totalFiles = 0;
    int filesProcessed = 0;

    // Count files first (optional - can be expensive for large directories)
    if (!stopRequested) {
        QDirIterator countIterator(sourceDirectory, fileTypes, QDir::Files | QDir::Hidden, QDirIterator::Subdirectories);
        while (countIterator.hasNext() && !stopRequested) {
            countIterator.next();
            totalFiles++;

            // If counting takes too long, stop at 1000 files
            if (totalFiles >= 1000) {
                break;
            }
        }
    }

    while (iterator.hasNext() && !stopRequested) {
        // Get file information (absolute path, size, datetime)
        QString filePath = iterator.next();
        QFileInfo fileInfo(filePath);
        QDateTime fileDate = fileInfo.lastModified();

        // Update progress periodically
        filesProcessed++;
        totalFilesProcessed++;

        if (filesProcessed % progressRefreshRate == 0) {
            emit searchProgress(totalFilesProcessed);
        }

        // Line with file info
        QString line = fileInfo.absoluteFilePath() + "\t" + QString::number(fileInfo.size()) + "\t" + fileDate.toString("yyyy/MM/dd hh:mm:ss");

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

        // Exclude catalog metadata lines which are starting with the character
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
            QSqlQuery queryTag(QSqlDatabase::database(connectionName));
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
                QString reducedLine = file.fileName();

                match = regex.match(reducedLine);
            }
            else if (selectedSearchIn == QCoreApplication::translate("MainWindow", "Folder path only")) {
                // Keep only the folder name, so all characters left of the last occurrence of / in the path.
                QString reducedLine = lineFilePath.left(lineFilePath.lastIndexOf("/"));

                // Check the folder name matches the search text
                regex.setPattern(regexSearchtext);
                foldermatch = regex.match(reducedLine);

                // If it does, then check that the file matches the selected file type
                if (foldermatch.hasMatch() && searchOnType == true) {
                    regex.setPattern(regexFileType);
                    match = regex.match(lineFilePath);
                } else {
                    match = foldermatch;
                }
            }
            else {
                match = regex.match(lineFilePath);
            }

            // If the file is matching the criteria, add it and its catalog to the search results
            if (match.hasMatch()) {
                QMutexLocker locker(&mutex); // Lock when modifying shared data
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
                QMutexLocker locker(&mutex); // Lock when modifying shared data
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
    // Emit signal for progress completion
    emit searchProgress(filesProcessed);
}

void SearchStoppable::processDuplicates(const QString &connectionName)
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
    for (int i = 0; i < rows && !stopRequested; i++) {
        // Emit progress periodically using configurable refresh rate
        if (i % progressRefreshRate == 0) {
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
    if (stopRequested) {
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
    QMutexLocker locker(&mutex); // Lock when modifying shared data
    fileNames.clear();
    fileSizes.clear();
    filePaths.clear();
    fileDateTimes.clear();
    fileCatalogs.clear();
    fileCatalogIDs.clear();

    while (duplicatesQuery.next() && !stopRequested) {
        fileNames.append(duplicatesQuery.value(0).toString());
        fileSizes.append(duplicatesQuery.value(1).toLongLong());
        fileDateTimes.append(duplicatesQuery.value(2).toString());
        filePaths.append(duplicatesQuery.value(3).toString());
        fileCatalogs.append(duplicatesQuery.value(4).toString());
        fileCatalogIDs.append(duplicatesQuery.value(5).toInt());
    }
}

void SearchStoppable::processDifferences(const QString &connectionName)
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
    for (int i = 0; i < rows && !stopRequested; i++) {
        // Emit progress periodically using configurable refresh rate
        if (i % progressRefreshRate == 0) {
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
    if (stopRequested) {
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

    // Run Query
    QSqlQuery differencesQuery(QSqlDatabase::database(connectionName));
    differencesQuery.prepare(selectSQL);

    if (!differencesQuery.exec()) {
        qWarning() << "SearchStoppable::processDifferences - Query error:" << differencesQuery.lastError().text();
    }

    // Commit transaction
    transactionQuery.exec("COMMIT");

    // Recapture file results for stats
    QMutexLocker locker(&mutex); // Lock when modifying shared data
    fileNames.clear();
    fileSizes.clear();
    filePaths.clear();
    fileDateTimes.clear();
    fileCatalogs.clear();
    fileCatalogIDs.clear();

    while (differencesQuery.next() && !stopRequested) {
        fileNames.append(differencesQuery.value(0).toString());
        fileSizes.append(differencesQuery.value(1).toLongLong());
        fileDateTimes.append(differencesQuery.value(2).toString());
        filePaths.append(differencesQuery.value(3).toString());
        fileCatalogs.append(differencesQuery.value(4).toString());
        fileCatalogIDs.append(differencesQuery.value(5).toInt());
    }
}
