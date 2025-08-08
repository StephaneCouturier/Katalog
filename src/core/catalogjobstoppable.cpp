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
    estimatedTotalFiles = 0;
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
            qDebug() << "=== Catalog operation completed successfully ===";
            emit catalogOperationFinished();
        }

    } catch (const std::exception &e) {
        qDebug() << "=== EXCEPTION in processCatalog():" << e.what() << "===";
        emit catalogOperationError(QString("Catalog operation failed: %1").arg(e.what()));
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

    // Estimate total files for progress calculation
    qDebug() << "Estimating total files...";
    emitProgressUpdate(0, 0, "Estimating total files...");
    estimatedTotalFiles = countTotalFiles(catalog->sourcePath, catalog);
    qDebug() << "Estimated total files:" << estimatedTotalFiles;

    qDebug() << "Step 3: Estimating total files...";
    emitProgressUpdate(0, 0, "Starting file count estimation...");

    auto startTime = QDateTime::currentDateTime();
    estimatedTotalFiles = countTotalFiles(catalog->sourcePath, catalog);
    auto endTime = QDateTime::currentDateTime();

    qDebug() << "Estimation completed in" << startTime.msecsTo(endTime) << "ms";
    qDebug() << "Estimated total files:" << estimatedTotalFiles;

    if (!shouldContinue()) {
        qDebug() << "Stop requested during estimation";
        return;
    }

    // Initialize database transaction for efficiency
    QSqlDatabase db = QSqlDatabase::database(m_connectionName);
    if (!db.exec("BEGIN TRANSACTION").isActive()) {
        qDebug() << "Warning: Could not start transaction:" << db.lastError().text();
    }

    qDebug() << "Step 4: Starting database transaction";
    // Save catalog to database first
    catalog->insertCatalog();

    // Process files with progress
    qint64 processedCount = 0;
    processDirectoryWithProgress(catalog->sourcePath, catalog, processedCount);

    if (!shouldContinue()) {
        db.exec("ROLLBACK");
        qDebug() << "Catalog creation cancelled, transaction rolled back";
        return;
    }

    // Commit transaction
    if (!db.exec("COMMIT").isActive()) {
        qDebug() << "Warning: Could not commit transaction:" << db.lastError().text();
    }

    // Update catalog metadata
    catalog->updateFileCount();
    catalog->updateTotalFileSize();
    catalog->saveCatalog();

    // Final progress update
    qDebug() << "About to emit final progress update";
    emitProgressUpdate(processedCount, estimatedTotalFiles, "Catalog creation completed");
    qDebug() << "Final progress update emitted";

    qDebug() << "=== CatalogJobStoppable::createCatalogWithProgress() completed successfully ===";
}

void CatalogJobStoppable::updateCatalogWithProgress()
{
    qDebug() << "=== Updating catalog with progress ===";

    if (!m_device || !m_device->catalog) {
        throw std::runtime_error("Invalid device or catalog");
    }

    emitProgressUpdate(0, 0, "Starting catalog update...");

    // Use the existing updateCatalogFiles method but with progress monitoring
    QList<qint64> updateResults = m_device->catalog->updateCatalogFiles(m_databaseMode, m_collectionFolder, false);

    if (!shouldContinue()) return;

    // Process update results
    if (updateResults.isEmpty()) {
        throw std::runtime_error("Update failed: No results returned");
    }

    qint64 resultCode = updateResults[0];
    if (resultCode != 1) {
        QString errorMsg;
        switch (resultCode) {
        case -1: errorMsg = "Catalog path not accessible"; break;
        case -2: errorMsg = "Catalog path is empty"; break;
        case -3: errorMsg = "Old catalog format - needs migration"; break;
        case -4: errorMsg = "Missing catalog information"; break;
        default: errorMsg = "Unknown update error"; break;
        }
        throw std::runtime_error(errorMsg.toStdString());
    }

    // Report successful update
    if (updateResults.size() >= 5) {
        qint64 newFileCount = updateResults[1];
        qint64 newTotalSize = updateResults[3];

        // Emit final progress
        emitProgressUpdate(newFileCount, newFileCount,
                           QString("Update completed. Files: %1, Size: %2 bytes")
                               .arg(newFileCount).arg(newTotalSize));
    }

    qDebug() << "Catalog update completed successfully";
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

    // Database batching arrays
    QStringList fileNames, filePaths, fileDateTimes, fileCatalogs;
    QList<qint64> fileSizes;

    int batchSize = 1000; // Database batch size
    int batchCount = 0;   // Database batch counter

    // Progress tracking (separate from database batching)
    qint64 filesProcessedInThisCall = 0;

    // Emit the transition message once
    emitProgressUpdate(0, estimatedTotalFiles, "Processing files...");

    while (it.hasNext() && shouldContinue()) {
        waitIfPaused();

        QString filePath = it.next();
        QFileInfo fileInfo(filePath);

        // Skip excluded folders
        bool isExcluded = false;
        for (const QString &excludedFolder : catalog->excludedFolders) {
            if (filePath.contains(excludedFolder)) {
                isExcluded = true;
                break;
            }
        }
        if (isExcluded) continue;

        // Process file - add to database batch
        fileNames << fileInfo.fileName();
        filePaths << filePath;
        fileDateTimes << fileInfo.lastModified().toString("yyyy-MM-dd hh:mm:ss");
        fileCatalogs << QString::number(catalog->ID);
        fileSizes << fileInfo.size();

        // Update counters
        processedCount++;           // Total files processed (passed by reference)
        filesProcessedInThisCall++; // Files processed in this method call
        batchCount++;              // Database batch counter

        // *** KEY FIX: Simple progress updates every 250 files (like working version) ***
        if (filesProcessedInThisCall % 250 == 0) {
            emitProgressUpdate(processedCount, estimatedTotalFiles, fileInfo.absoluteFilePath());
            QCoreApplication::processEvents();
        }

        // Database batching (separate from progress reporting)
        if (batchCount >= batchSize) {
            // Insert batch to database
            if (!fileNames.isEmpty()) {
                QSqlQuery query(QSqlDatabase::database(m_connectionName));
                query.prepare(R"(
                    INSERT INTO file (file_catalog_id, file_name, file_folder_path, file_size, file_date_updated)
                    VALUES (:catalog_id, :name, :path, :size, :date)
                )");

                for (int i = 0; i < fileNames.size(); ++i) {
                    if (!shouldContinue()) break;

                    query.bindValue(":catalog_id", catalog->ID);
                    query.bindValue(":name", fileNames[i]);
                    query.bindValue(":path", filePaths[i]);
                    query.bindValue(":size", fileSizes[i]);
                    query.bindValue(":date", fileDateTimes[i]);

                    if (!query.exec()) {
                        qDebug() << "Database insert error:" << query.lastError().text();
                    }
                }

                // Insert folders for this batch
                QStringList uniqueFolders = filePaths;
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
            filePaths.clear();
            fileDateTimes.clear();
            fileCatalogs.clear();
            fileSizes.clear();
            batchCount = 0;
        }
    }

    // Process remaining files in final batch
    if (!fileNames.isEmpty() && shouldContinue()) {
        QSqlQuery query(QSqlDatabase::database(m_connectionName));
        query.prepare(R"(
            INSERT INTO file (file_catalog_id, file_name, file_folder_path, file_size, file_date_updated)
            VALUES (:catalog_id, :name, :path, :size, :date)
        )");

        for (int i = 0; i < fileNames.size(); ++i) {
            if (!shouldContinue()) break;

            query.bindValue(":catalog_id", catalog->ID);
            query.bindValue(":name", fileNames[i]);
            query.bindValue(":path", filePaths[i]);
            query.bindValue(":size", fileSizes[i]);
            query.bindValue(":date", fileDateTimes[i]);

            if (!query.exec()) {
                qDebug() << "Database insert error:" << query.lastError().text();
            }
        }

        // Insert remaining folders
        QStringList uniqueFolders = filePaths;
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
        emitProgressUpdate(processedCount, estimatedTotalFiles, "Processing completed");
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
            emitProgressUpdate(0, 0, QString("Estimating... %1 files found").arg(totalFiles));
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
    return m_objectValid.loadAcquire() && !m_stopRequested.loadAcquire();
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
    estimatedTotalFiles = total;

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
