/*LICENCE
    This file is part of Katalog

    Copyright (C) 2020, the Katalog Development team

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
// File Name:   catalogjobstoppable.cpp
// Purpose:     Implementation of stoppable catalog creation/update operations
// Description: Handles actual catalog work with progress reporting and stop/pause capabilities
// Author:      Stephane Couturier
/////////////////////////////////////////////////////////////////////////////
*/

#include "catalogjobstoppable.h"
#include <QDebug>
#include <QDir>
#include <QDirIterator>
#include <QFileInfo>
#include <QCoreApplication>
#include <QSqlQuery>
#include <QSqlError>
#include <QSqlDatabase>
#include <QDateTime>
#include <QThread>

CatalogJobStoppable::CatalogJobStoppable(QObject *parent)
    : QObject(parent)
    , m_storageWasUpdated(false)
{
    qDebug() << "CatalogJobStoppable created";
}

CatalogJobStoppable::~CatalogJobStoppable()
{
    // Mark object as invalid to prevent use after destruction
    m_objectValid.storeRelease(0);
    qDebug() << "CatalogJobStoppable destroyed";
}

void CatalogJobStoppable::configureOperation(Device *device,
                                             OperationType operationType,
                                             const QString &databaseMode,
                                             const QString &collectionFolder,
                                             const QString &connectionName)
{
    m_device = device;
    m_operationType = operationType;
    m_databaseMode = databaseMode;
    m_collectionFolder = collectionFolder;
    m_connectionName = connectionName;

    // Reset state
    m_stopRequested.storeRelease(0);
    m_paused.storeRelease(0);
    countedTotalFiles = 0;
    filesProcessed = 0;
    currentCatalogName = device ? device->catalog->name : QString();

    qDebug() << "CatalogJobStoppable configured:"
             << "Operation:" << (operationType == CreateCatalog ? "Create" : "Update")
             << "Catalog:" << currentCatalogName
             << "DatabaseMode:" << databaseMode;
}

void CatalogJobStoppable::processCatalog()
{
    qDebug() << "=== CatalogJobStoppable::processCatalog() START ===";

    if (!m_objectValid.loadAcquire()) {
        qDebug() << "Object is invalid, aborting";
        return;
    }

    if (!m_device || !m_device->catalog) {
        qDebug() << "No device or catalog configured";
        emit catalogOperationError("No device or catalog configured");
        return;
    }

    if (!shouldContinue()) {
        qDebug() << "Stop was requested before processing";
        return;
    }

    try {
        // Reset counters
        filesProcessed = 0;
        currentCatalogName = m_device->catalog->name;

        qDebug() << "Starting" << (m_operationType == CreateCatalog ? "creation" : "update")
                 << "of catalog:" << currentCatalogName;

        if (m_operationType == CreateCatalog) {
            createCatalogWithProgress();

            // NEW: Complete the catalog creation with all backend tasks
            if (shouldContinue()) {
                qDebug() << "Scanning completed, starting post-processing...";
                completeCatalogCreation();
                qDebug() << "Post-processing completed successfully";
            }
        } else {
            updateCatalogWithProgress();
        }

        if (shouldContinue()) {
            qDebug() << "=== DIAGNOSTIC: Catalog indexing completed, about to emit catalogOperationFinished ===";
            emit catalogOperationFinished();
            qDebug() << "=== DIAGNOSTIC: catalogOperationFinished signal emitted! ===";
        }

    } catch (const std::exception &e) {
        qDebug() << "=== EXCEPTION in processCatalog():" << e.what() << "===";
        emit catalogOperationError(QString("Catalog operation failed: %1").arg(e.what()));
    }

    if (shouldContinue()) {
        qDebug() << "=== DIAGNOSTIC: Catalog indexing completed, about to emit catalogOperationFinished ===";
        emit catalogOperationFinished();
        qDebug() << "=== DIAGNOSTIC: catalogOperationFinished signal emitted! ===";
    }

    qDebug() << "=== CatalogJobStoppable::processCatalog() END ===";
}

void CatalogJobStoppable::createCatalogWithProgress()
{
    qDebug() << "=== CATALOG CREATION STARTED ===";
    qDebug() << "Device ID:" << m_device->ID;
    qDebug() << "Catalog Name:" << m_device->catalog->name;
    qDebug() << "Source Path:" << m_device->catalog->sourcePath;
    qDebug() << "Database Mode:" << m_databaseMode;
    qDebug() << "Collection Folder:" << m_collectionFolder;

        if (!m_device || !m_device->catalog) {
        qDebug() << "ERROR: Invalid device or catalog";
        throw std::runtime_error("Invalid device or catalog");
    }

    Catalog* catalog = m_device->catalog;

    // Validate source path
    qDebug() << "Step 1: Validating source directory";
    QDir sourceDir(catalog->sourcePath);
    if (!sourceDir.exists()) {
        qDebug() << "ERROR: Source directory does not exist:" << catalog->sourcePath;
        throw std::runtime_error("Source directory does not exist: " + catalog->sourcePath.toStdString());
    }

    // Check if directory is empty
    int entryCount = sourceDir.entryInfoList(QDir::NoDotAndDotDot | QDir::AllEntries).count();
    qDebug() << "Source directory entry count:" << entryCount;
    if (entryCount == 0) {
        qDebug() << "WARNING: Source directory is empty:" << catalog->sourcePath;
        throw std::runtime_error("Source directory is empty: " + catalog->sourcePath.toStdString());
    }

    // Get file extensions to scan for and load excluded folders
    qDebug() << "Step 2: Loading file extensions and excluded folders";
    catalog->getFileExtensions();
    catalog->loadExcludedFolders();
    qDebug() << "File extensions loaded, excluded folders loaded";

    // Count total files for progress calculation
    qDebug() << "Step 3: Counting total files...";
    //emitProgressUpdate(0, 0, "Starting file counting...");

    auto startTime = QDateTime::currentDateTime();
    countedTotalFiles = countTotalFiles(catalog->sourcePath, catalog);
    auto endTime = QDateTime::currentDateTime();

    qDebug() << "Counting completed in" << startTime.msecsTo(endTime) << "ms";
    qDebug() << "Counting total files:" << countedTotalFiles;

    if (!shouldContinue()) {
        qDebug() << "Stop requested during estimation";
        return;
    }

    // Initialize database transaction for efficiency
    QSqlQuery transactionQuery(QSqlDatabase::database("defaultConnection"));
    if (!transactionQuery.exec("BEGIN TRANSACTION")) {
        qDebug() << "Warning: Could not BEGIN transaction:" << transactionQuery.lastError().text();
    }

    qDebug() << "Step 4: Starting database transaction";
    // Save catalog to database first
    catalog->insertCatalog();

    // Process files with progress
    qint64 processedCount = 0;
    processDirectoryWithProgress(catalog->sourcePath, catalog, processedCount);

    if (!shouldContinue()) {
        QSqlQuery rollbackQuery(QSqlDatabase::database("defaultConnection"));
        if (!rollbackQuery.exec("ROLLBACK")) {
            qDebug() << "Warning: Could not ROLLBACK transaction:" << rollbackQuery.lastError().text();
        }
    }

    // Commit transaction
    QSqlQuery commitQuery(QSqlDatabase::database("defaultConnection"));
    if (!commitQuery.exec("COMMIT")) {
        qDebug() << "Warning: Could not COMMIT transaction:" << commitQuery.lastError().text();
    }

    // Update catalog metadata
    catalog->updateFileCount();
    catalog->updateTotalFileSize();
    catalog->saveCatalog();

    // Final progress update
    qDebug() << "About to emit final progress update";
    emitProgressUpdate(processedCount, countedTotalFiles, "Catalog creation completed");
    qDebug() << "Final progress update emitted";

    qDebug() << "=== CatalogJobStoppable::createCatalogWithProgress() completed successfully ===";
}

void CatalogJobStoppable::updateCatalogWithProgress()
{
    qDebug() << "=== CATALOG UPDATE STARTED ===";
    qDebug() << "Device ID:" << m_device->ID;
    qDebug() << "Catalog Name:" << m_device->catalog->name;
    qDebug() << "Source Path:" << m_device->catalog->sourcePath;
    qDebug() << "Database Mode:" << m_databaseMode;
    qDebug() << "Collection Folder:" << m_collectionFolder;

    if (!m_device || !m_device->catalog) {
        qDebug() << "ERROR: Invalid device or catalog";
        throw std::runtime_error("Invalid device or catalog");
    }

    Catalog* catalog = m_device->catalog;

    // Capture original values before clearing for delta calculation
    m_originalFileCount = catalog->fileCount;
    m_originalTotalFileSize = catalog->totalFileSize;
    qDebug() << "Captured original values - Files:" << m_originalFileCount << "Size:" << m_originalTotalFileSize;

    // Validate source path (same as creation)
    qDebug() << "Step 1: Validating source directory";
    QDir sourceDir(catalog->sourcePath);
    if (!sourceDir.exists()) {
        qDebug() << "ERROR: Source directory does not exist:" << catalog->sourcePath;
        throw std::runtime_error("Source directory does not exist: " + catalog->sourcePath.toStdString());
    }

    // Check if directory is empty (same as creation)
    int entryCount = sourceDir.entryInfoList(QDir::NoDotAndDotDot | QDir::AllEntries).count();
    qDebug() << "Source directory entry count:" << entryCount;
    if (entryCount == 0) {
        qDebug() << "WARNING: Source directory is empty:" << catalog->sourcePath;
        // For updates, empty directory might be OK, just warn
        qDebug() << "Empty directory detected during update - will clear catalog";
    }

    // Get file extensions to scan for and load excluded folders (same as creation)
    qDebug() << "Step 2: Loading file extensions and excluded folders";
    catalog->getFileExtensions();
    catalog->loadExcludedFolders();
    qDebug() << "File extensions loaded, excluded folders loaded";

    // Count total files for progress calculation (SAME AS CREATION)
    qDebug() << "Step 3: Counting total files...";
    emitProgressUpdate(0, 0, "Starting file counting...");

    auto startTime = QDateTime::currentDateTime();
    countedTotalFiles = countTotalFiles(catalog->sourcePath, catalog);
    auto endTime = QDateTime::currentDateTime();

    qDebug() << "Counting completed in" << startTime.msecsTo(endTime) << "ms";
    qDebug() << "Counting total files:" << countedTotalFiles;

    if (!shouldContinue()) {
        qDebug() << "Stop requested during estimation";
        return;
    }

    // Initialize database transaction for efficiency (same as creation)
    QSqlQuery transactionQuery(QSqlDatabase::database(m_connectionName));
    if (!transactionQuery.exec("BEGIN TRANSACTION")) {
        qDebug() << "Warning: Could not start transaction:" << transactionQuery.lastError().text();
    }

    qDebug() << "Step 4: Starting database transaction";

    // FOR UPDATE: Clear existing catalog data first
    qDebug() << "Step 5: Clearing existing catalog data for update";
    catalog->clearCatalogData();  // This method should exist to clear old files

    // Process files with progress (SAME AS CREATION)
    qint64 processedCount = 0;
    processDirectoryWithProgress(catalog->sourcePath, catalog, processedCount);

    if (!shouldContinue()) {
        transactionQuery.exec("ROLLBACK");
        qDebug() << "Catalog creation cancelled, transaction rolled back";
        return;
    }

    // Commit transaction (same as creation)
    QSqlQuery commitQuery(QSqlDatabase::database(m_connectionName));
    if (!commitQuery.exec("COMMIT")) {
        qDebug() << "Warning: Could not commit transaction:" << commitQuery.lastError().text();
    }

    // Update catalog metadata (same as creation)
    catalog->updateFileCount();
    catalog->updateTotalFileSize();
    catalog->saveCatalog();

    // Clean up temp files on success
    qDebug() << "Update successful - cleaning up temp ID";
    catalog->cleanupTempID();

    // Final progress update (ONLY DIFFERENCE: "update" instead of "creation")
    qDebug() << "About to emit final progress update";
    emitProgressUpdate(processedCount, countedTotalFiles, "Catalog update completed");
    qDebug() << "Final progress update emitted";

    // Update the Device object
        // 1. Set update date
        m_device->dateTimeUpdated = QDateTime::currentDateTime();

        // 2. Update device file counts from catalog results
        m_device->totalFileCount = catalog->fileCount;
        m_device->totalFileSize  = catalog->totalFileSize;

        // 3. Save statistics
        m_device->saveStatistics(m_device->dateTimeUpdated, "update");

        // 4. Save device to update the date in database
        m_device->saveDevice();

        // 5. Update parent storage space info
        qDebug() << "CRITICAL FIX: Updating parent storage space after catalog update";
        //updateParentStorageAfterCatalogUpdate();

        // 6. Update aggregated file counts up the hierarchy
        qDebug() << "Updating parent device hierarchy numbers";
        try {
            m_device->updateParentsNumbers();
            qDebug() << "Parent numbers updated successfully";
        } catch (const std::exception& e) {
            qDebug() << "Error updating parent numbers:" << e.what();
        }

        // 7. Update related catalog devices
        qDebug() << "Updating related catalog devices";
        updateRelatedCatalogDevices();

        // 8. Save catalog files to disk (Memory mode) - same as creation
        if (!catalog->saveCatalogToFile(m_databaseMode, m_collectionFolder)) {
            qDebug() << "Warning: Failed to save updated catalog to file";
        }
        if (!catalog->saveFoldersToFile(m_databaseMode, m_collectionFolder)) {
            qDebug() << "Warning: Failed to save updated folders to file";
        }

    // Emit final progress
    emitProgressUpdate(m_device->totalFileCount, m_device->totalFileSize,
                       QString("Update completed. %1 files, %2 bytes")
                           .arg(m_device->totalFileCount).arg(m_device->totalFileSize));

    qDebug() << "=== CatalogJobStoppable::updateCatalogWithProgress() completed successfully ===";
}

void CatalogJobStoppable::processDirectoryWithProgress(const QString &directory,
                                                       Catalog *catalog,
                                                       qint64 &processedCount)
{
    if (!shouldContinue()) return;

    qDebug() << "Processing directory:" << directory;

    // Get file extensions for filtering
    QStringList extensions;
    if (catalog->fileType == "Image") {
        extensions << "*.png" << "*.jpg" << "*.jpeg" << "*.gif" << "*.xcf" << "*.tif" << "*.tiff" << "*.bmp";
    } else if (catalog->fileType == "Audio") {
        extensions << "*.mp3" << "*.wav" << "*.ogg" << "*.aif" << "*.aiff" << "*.flac";
    } else if (catalog->fileType == "Video") {
        extensions << "*.wmv" << "*.avi" << "*.mp4" << "*.mkv" << "*.flv" << "*.webm" << "*.m4v" << "*.vob" << "*.ogv" << "*.mov";
    } else if (catalog->fileType == "Text") {
        extensions << "*.txt" << "*.pdf" << "*.odt" << "*.idx" << "*.html" << "*.rtf" << "*.doc" << "*.docx" << "*.epub";
    } else {
        // Default: include all files
        extensions << "*";
    }

    // Use QDirIterator for efficient directory traversal
    QDir::Filters filters = QDir::Files | QDir::Readable;
    if (catalog->includeHidden) {
        filters |= QDir::Hidden;
    }

    QDirIterator::IteratorFlags iteratorFlags = QDirIterator::Subdirectories;
    if (catalog->includeSymblinks) {
        iteratorFlags |= QDirIterator::FollowSymlinks;
    }

    QDirIterator it(directory, extensions, filters, iteratorFlags);

    // Database batching arrays - separate arrays for folder paths and full paths
    QStringList fileNames, fileFolderPaths, fileFullPaths, fileDateTimes, fileCatalogs;
    QList<qint64> fileSizes;

    int batchSize = 1000; // Database batch size
    int batchCount = 0;   // Database batch counter

    // Progress tracking (separate from database batching)
    qint64 filesProcessedInThisCall = 0;

    // Emit the transition message once
    emitProgressUpdate(0, countedTotalFiles, "Processing files...");

    while (it.hasNext() && shouldContinue()) {
        waitIfPaused();

        QString fileFullPath = it.next();
        QFileInfo fileInfo(fileFullPath);

        // Skip excluded folders
        bool isExcluded = false;
        for (const QString &excludedFolder : catalog->excludedFolders) {
            if (fileFullPath.contains(excludedFolder)) {
                isExcluded = true;
                break;
            }
        }
        if (isExcluded) continue;

        // Process file - correctly separate folder path from full path
        fileNames << fileInfo.fileName();                                    // Just the filename
        fileFolderPaths << fileInfo.path();                                  // Just the directory path
        fileFullPaths << fileFullPath;                                       // Complete file path
        fileDateTimes << fileInfo.lastModified().toString("yyyy/MM/dd hh:mm:ss");
        fileCatalogs << catalog->name;                                       // Use catalog NAME, not ID
        fileSizes << fileInfo.size();

        // Update counters
        processedCount++;           // Total files processed (passed by reference)
        filesProcessedInThisCall++; // Files processed in this method call
        batchCount++;              // Database batch counter

        // Simple progress updates every 250 files (like working version) ***
        if (filesProcessedInThisCall % progressRefreshRate == 0) {
            emitProgressUpdate(processedCount, countedTotalFiles, fileInfo.absoluteFilePath());
            QCoreApplication::processEvents();
        }

        // Database batching (separate from progress reporting)
        if (batchCount >= batchSize) {
            // Insert batch to database
            if (!fileNames.isEmpty()) {
                QSqlQuery query(QSqlDatabase::database(m_connectionName));
                // Updated SQL to include file_catalog field
                query.prepare(R"(
                    INSERT INTO file (file_catalog_id, file_name, file_folder_path, file_full_path, file_size, file_date_updated, file_catalog)
                    VALUES (:catalog_id, :name, :folder_path, :full_path, :size, :date, :catalog_name)
                )");

                for (int i = 0; i < fileNames.size(); ++i) {
                    if (!shouldContinue()) break;

                    query.bindValue(":catalog_id", catalog->ID);
                    query.bindValue(":name", fileNames[i]);                     // Just filename
                    query.bindValue(":folder_path", fileFolderPaths[i]);        // Just directory path
                    query.bindValue(":full_path", fileFullPaths[i]);            // Complete file path
                    query.bindValue(":size", fileSizes[i]);
                    query.bindValue(":date", fileDateTimes[i]);
                    query.bindValue(":catalog_name", fileCatalogs[i]);          // Add catalog name

                    if (!query.exec()) {
                        qDebug() << "Database insert error:" << query.lastError().text();
                    }
                }

                // Insert folders for this batch - Use folder paths, not full file paths
                QStringList uniqueFolders = fileFolderPaths;
                uniqueFolders.removeDuplicates();

                QSqlQuery folderQuery(QSqlDatabase::database(m_connectionName));
                folderQuery.prepare(R"(
                    INSERT OR IGNORE INTO folder (folder_catalog_id, folder_path)
                    VALUES (:catalog_id, :path)
                )");

                for (const QString &folderPath : uniqueFolders) {
                    if (!shouldContinue()) break;

                    folderQuery.bindValue(":catalog_id", catalog->ID);
                    folderQuery.bindValue(":path", folderPath);

                    if (!folderQuery.exec()) {
                        qDebug() << "Folder insert error:" << folderQuery.lastError().text();
                    }
                }
            }

            // Clear batch arrays
            fileNames.clear();
            fileFolderPaths.clear();        // Clear new array
            fileFullPaths.clear();          // Clear new array
            fileDateTimes.clear();
            fileCatalogs.clear();           // Contains catalog names
            fileSizes.clear();
            batchCount = 0;
        }
    }

    // Process remaining files in final batch
    if (!fileNames.isEmpty() && shouldContinue()) {
        QSqlQuery query(QSqlDatabase::database(m_connectionName));
        query.prepare(R"(
            INSERT INTO file (file_catalog_id, file_name, file_folder_path, file_full_path, file_size, file_date_updated, file_catalog)
            VALUES (:catalog_id, :name, :folder_path, :full_path, :size, :date, :catalog_name)
        )");

        for (int i = 0; i < fileNames.size(); ++i) {
            if (!shouldContinue()) break;

            query.bindValue(":catalog_id", catalog->ID);
            query.bindValue(":name", fileNames[i]);                     // Just filename
            query.bindValue(":folder_path", fileFolderPaths[i]);        // Just directory path
            query.bindValue(":full_path", fileFullPaths[i]);            // Complete file path
            query.bindValue(":size", fileSizes[i]);
            query.bindValue(":date", fileDateTimes[i]);
            query.bindValue(":catalog_name", fileCatalogs[i]);          // Add catalog name

            if (!query.exec()) {
                qDebug() << "Database insert error:" << query.lastError().text();
            }
        }

        // Insert remaining folders
        QStringList uniqueFolders = fileFolderPaths;
        uniqueFolders.removeDuplicates();

        QSqlQuery folderQuery(QSqlDatabase::database(m_connectionName));
        folderQuery.prepare(R"(
            INSERT OR IGNORE INTO folder (folder_catalog_id, folder_path)
            VALUES (:catalog_id, :path)
        )");

        for (const QString &folderPath : uniqueFolders) {
            if (!shouldContinue()) break;

            folderQuery.bindValue(":catalog_id", catalog->ID);
            folderQuery.bindValue(":path", folderPath);

            if (!folderQuery.exec()) {
                qDebug() << "Folder insert error:" << folderQuery.lastError().text();
            }
        }
    }

    // Final progress update for this directory
    if (shouldContinue()) {
        emitProgressUpdate(processedCount, countedTotalFiles, "Processing completed");
        QCoreApplication::processEvents(); // Final UI update
    }

    qDebug() << "Processed" << filesProcessedInThisCall << "files in directory:" << directory;
}

qint64 CatalogJobStoppable::countTotalFiles(const QString &directory, Catalog *catalog)
{
    qint64 totalFiles = 0;

    if (!shouldContinue()) return 0;

    // Get file extensions for filtering (same logic as in processDirectoryWithProgress)
    QStringList extensions;
    if (catalog->fileType == "Image") {
        extensions << "*.png" << "*.jpg" << "*.jpeg" << "*.gif" << "*.xcf" << "*.tif" << "*.tiff" << "*.bmp";
    } else if (catalog->fileType == "Audio") {
        extensions << "*.mp3" << "*.wav" << "*.ogg" << "*.aif" << "*.aiff" << "*.flac";
    } else if (catalog->fileType == "Video") {
        extensions << "*.wmv" << "*.avi" << "*.mp4" << "*.mkv" << "*.flv" << "*.webm" << "*.m4v" << "*.vob" << "*.ogv" << "*.mov";
    } else if (catalog->fileType == "Text") {
        extensions << "*.txt" << "*.pdf" << "*.odt" << "*.idx" << "*.html" << "*.rtf" << "*.doc" << "*.docx" << "*.epub";
    } else {
        // Default: include all files
        extensions << "*";
    }

    QDir::Filters filters = QDir::Files | QDir::Readable;
    if (catalog->includeHidden) {
        filters |= QDir::Hidden;
    }

    QDirIterator::IteratorFlags iteratorFlags = QDirIterator::Subdirectories;
    if (catalog->includeSymblinks) {
        iteratorFlags |= QDirIterator::FollowSymlinks;
    }

    QDirIterator it(directory, extensions, filters, iteratorFlags);

    while (it.hasNext() && shouldContinue()) {
        QString filePath = it.next();

        // Skip excluded folders
        bool isExcluded = false;
        for (const QString &excludedFolder : catalog->excludedFolders) {
            if (filePath.contains(excludedFolder)) {
                isExcluded = true;
                break;
            }
        }
        if (isExcluded) continue;

        totalFiles++;

        // Emit estimation progress periodically (every 1000 files)
        if (totalFiles % 1000 == 0) {
            emitProgressUpdate(0, 0, QString("__COUNTING_STATE__|%1").arg(totalFiles));
            QCoreApplication::processEvents(); // Allow UI updates during estimation
        }
    }

    emitProgressUpdate(totalFiles, totalFiles, QString("Found %1 files. Starting processing...").arg(totalFiles));

    return totalFiles;
}

void CatalogJobStoppable::stopCatalogOperation()
{
    qDebug() << "CatalogJobStoppable::stopCatalogOperation() - Stop requested";
    m_stopRequested.storeRelease(1);
}

void CatalogJobStoppable::pauseCatalogOperation()
{
    qDebug() << "CatalogJobStoppable::pauseCatalogOperation() - Pause requested";
    m_paused.storeRelease(1);
}

void CatalogJobStoppable::resumeCatalogOperation()
{
    qDebug() << "CatalogJobStoppable::resumeCatalogOperation() - Resume requested";
    m_paused.storeRelease(0);
}

bool CatalogJobStoppable::shouldContinue() const
{
    return !m_stopRequested.loadAcquire() && !m_hardStopRequested.loadAcquire();
}

void CatalogJobStoppable::waitIfPaused()
{
    while (m_paused.loadAcquire() && shouldContinue()) {
        QMutexLocker locker(&m_pauseMutex);
        QThread::msleep(100); // Sleep briefly while paused
    }
}

void CatalogJobStoppable::emitProgressUpdate(qint64 processed, qint64 total, const QString &currentPath)
{
    filesProcessed = processed;
    countedTotalFiles = total;

    // Throttle progress updates to avoid overwhelming the UI
    QDateTime now = QDateTime::currentDateTime();
    if (m_lastProgressEmit.isValid() &&
        m_lastProgressEmit.msecsTo(now) < PROGRESS_UPDATE_INTERVAL_MS &&
        processed > 0 && processed < total) {
        // Skip this update to avoid flooding
        return;
    }
    m_lastProgressEmit = now;

    qDebug() << "Progress:" << processed << "/" << total << "("
             << (total > 0 ? (processed * 100 / total) : 0) << "%) -" << currentPath;

    emit catalogProgress(processed, total, currentPath);
}

void CatalogJobStoppable::updateRelatedCatalogDevices()
{
    if (!m_device) return;

    qDebug() << "Updating related catalog devices...";

    try {
        // Update related devices (other catalog devices using the same catalog ID)
        QSqlQuery queryRelatedDevice(QSqlDatabase::database("defaultConnection"));
        QString queryRelatedDeviceSQL = QLatin1String(R"(
                            SELECT device_id
                            FROM device
                            WHERE device_external_id = :device_external_id
                            AND device_type = 'Catalog'
                            AND device_id != :device_id
                        )");

        queryRelatedDevice.prepare(queryRelatedDeviceSQL);
        queryRelatedDevice.bindValue(":device_external_id", m_device->externalID);
        queryRelatedDevice.bindValue(":device_id", m_device->ID);
        queryRelatedDevice.exec();

        while (queryRelatedDevice.next()) {
            Device relatedDevice;
            relatedDevice.ID = queryRelatedDevice.value(0).toInt();
            relatedDevice.loadDevice("defaultConnection");

            // Update related device with same file counts
            relatedDevice.totalFileCount = m_device->totalFileCount;
            relatedDevice.totalFileSize = m_device->totalFileSize;
            relatedDevice.dateTimeUpdated = m_device->dateTimeUpdated;
            relatedDevice.saveDevice();

            qDebug() << "Updated related catalog device:" << relatedDevice.name;
        }

    } catch (const std::exception& e) {
        qDebug() << "Error updating related catalog devices:" << e.what();
    }
}

void CatalogJobStoppable::completeCatalogCreation()
{
    qDebug() << "=== CatalogJobStoppable::completeCatalogCreation() START ===";

    if (!m_device || !m_device->catalog) {
        qDebug() << "ERROR: Invalid device or catalog in completion";
        return;
    }

    try {
        // Step 1: Update device statistics from catalog
        qDebug() << "Step 1: Updating device statistics";
        m_device->totalFileCount = m_device->catalog->fileCount;
        m_device->totalFileSize = m_device->catalog->totalFileSize;
        m_device->dateTimeUpdated = QDateTime::currentDateTime();
        qDebug() << "Device stats - Files:" << m_device->totalFileCount << "Size:" << m_device->totalFileSize;

        // Step 2: Save device to database
        qDebug() << "Step 2: Saving device to database";
        m_device->saveDevice();

        // Step 3: Save catalog files (Memory mode)
        if (m_databaseMode == "Memory") {
            qDebug() << "Step 3: Saving catalog files for Memory mode";

            qDebug() << "Step 3a: About to call saveCatalogToFile()";
            if (!m_device->catalog->saveCatalogToFile(m_databaseMode, m_collectionFolder)) {
                qDebug() << "Warning: Failed to save catalog to file";
            } else {
                qDebug() << "Catalog saved to file successfully";
            }

            qDebug() << "Step 3b: About to call saveFoldersToFile()";
            if (!m_device->catalog->saveFoldersToFile(m_databaseMode, m_collectionFolder)) {
                qDebug() << "Warning: Failed to save folders to file";
            } else {
                qDebug() << "Folders saved to file successfully";
            }
        }

        // Step 4: Update catalog loaded version
        qDebug() << "Step 4: Setting catalog loaded date";
        QDateTime currentDateTime = QDateTime::currentDateTime();
        m_device->catalog->setDateLoaded(currentDateTime, m_connectionName);

        qDebug() << "=== Backend post-processing completed successfully ===";

    } catch (const std::exception& e) {
        qDebug() << "EXCEPTION in completeCatalogCreation():" << e.what();
        throw; // Re-throw to let caller handle
    } catch (...) {
        qDebug() << "UNKNOWN EXCEPTION in completeCatalogCreation()";
        throw;
    }

    qDebug() << "=== CatalogJobStoppable::completeCatalogCreation() END ===";
}

QList<qint64> CatalogJobStoppable::getResults() const
{
    QList<qint64> results;

    if (!m_device || !m_device->catalog) {
        results << -1; // Error code
        for (int i = 1; i < 14; ++i) results << 0;
        return results;
    }

    // Calculate proper deltas using original values
    qint64 newFileCount = m_device->totalFileCount;
    qint64 deltaFileCount = newFileCount - m_originalFileCount;
    qint64 newTotalFileSize = m_device->totalFileSize;
    qint64 deltaTotalFileSize = newTotalFileSize - m_originalTotalFileSize;

    // Catalog update results (indices 0-4)
    results << 1; // Success code
    results << newFileCount; // Total files after update
    results << deltaFileCount; // Actual delta files
    results << newTotalFileSize; // Total size after update
    results << deltaTotalFileSize; // Actual delta size

    // Catalog counters (indices 5-6) - not used for individual reports
    results << 0; // Updated catalogs count (not relevant for single catalog)
    results << 0; // Skipped catalogs count (not relevant for single catalog)

    // Storage update results (indices 7-13)
    if (m_storageWasUpdated && m_storageUpdateResult.wasUpdated) {
        results << 1; // Storage updated flag
        results << m_storageUpdateResult.newUsedSpace;     // Used space
        results << m_storageUpdateResult.deltaUsedSpace;   // Delta used space
        results << m_storageUpdateResult.newFreeSpace;     // Free space
        results << m_storageUpdateResult.deltaFreeSpace;   // Delta free space
        results << m_storageUpdateResult.newTotalSpace;    // Total space
        results << m_storageUpdateResult.deltaTotalSpace;  // Delta total space
    } else {
        // No storage update
        results << 0; // Storage updated flag = false
        for (int i = 8; i < 14; ++i) results << 0; // No storage values
    }

    return results;
}

void CatalogJobStoppable::requestHardStop()
{
    qDebug() << "CatalogJobStoppable::requestHardStop() - Hard stop requested";
    m_hardStopRequested.storeRelease(1);

    // Also set the regular stop flag to exit processing loops
    m_stopRequested.storeRelease(1);
}
