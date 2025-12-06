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
#include "filemetadata.h"
#include "filetypemapping.h"
#include "parallelmetadataextractor.h"
#include "database.h"
#include "filechecksum.h"

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
#include <QSettings>
#include <QElapsedTimer>

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

    // Adapt refresh rate: lower refresh rate for metadata extraction
    if (device && device->catalog && device->catalog->includeMetadata != Catalog::METADATA_NONE) {
        setProgressRefreshRate(100);  // More frequent updates for slower metadata operations
    } else {
        setProgressRefreshRate(100); // Default rate for faster operations without metadata
    }

    qDebug() << "CatalogJobStoppable configured:"
             << "Operation:" << (operationType == CreateCatalog ? "Create" : "Update")
             << "Catalog:" << currentCatalogName
             << "DatabaseMode:" << databaseMode
             << "IncludeMetadata:" << (device && device->catalog ? device->catalog->includeMetadata : "N/A")
             << "ProgressRefreshRate:" << progressRefreshRate;
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

        qDebug() << "Starting operation type:" << m_operationType
                 << "for catalog:" << currentCatalogName;

        // Route to correct operation
        if (m_operationType == CreateCatalog) {
            createCatalogWithProgress();
            if (shouldContinue()) {
                completeCatalogCreation();
            }
        } else if (m_operationType == VerifyMimeTypes) {
            verifyMimeTypes();
        } else if (m_operationType == ExtractMissingMetadata) {
            extractMissingMetadata();
        } else {
            // Treat all other operations as standard update
            updateCatalogWithProgress();
        }

        if (shouldContinue()) {
            emit catalogOperationFinished();
        }

    } catch (const std::exception &e) {
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

    // Count total files for progress calculation
    qDebug() << "Step 3: Counting total files...";
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
    Database::beginTransaction(m_connectionName);

    qDebug() << "Step 4: Starting database transaction";
    // Save catalog to database first
    catalog->insertCatalog();

    // Process files with progress
    qint64 processedCount = 0;
    processDirectoryWithProgress(catalog->sourcePath, catalog, processedCount);

    if (!shouldContinue()) {
        qDebug() << "Catalog creation STOPPED - cleaning up...";

        // Rollback file insertions
        Database::rollbackTransaction(m_connectionName);

        // ALSO delete any committed files for this catalog
        QSqlQuery cleanupQuery(QSqlDatabase::database(m_connectionName));
        cleanupQuery.prepare("DELETE FROM file WHERE file_catalog_id = :catalog_id");
        cleanupQuery.bindValue(":catalog_id", catalog->ID);
        if (cleanupQuery.exec()) {
            qDebug() << "Deleted" << cleanupQuery.numRowsAffected() << "files for stopped catalog";
        } else {
            qDebug() << "Failed to cleanup files:" << cleanupQuery.lastError().text();
        }

        // Delete folders too
        QSqlQuery cleanupFoldersQuery(QSqlDatabase::database(m_connectionName));
        cleanupFoldersQuery.prepare("DELETE FROM folder WHERE folder_catalog_id = :catalog_id");
        cleanupFoldersQuery.bindValue(":catalog_id", catalog->ID);
        cleanupFoldersQuery.exec();

        qDebug() << "Catalog creation cleanup complete";
        return; // Exit without completing
    }

    // Commit transaction
    QSqlQuery commitQuery(QSqlDatabase::database(m_connectionName));
    Database::commitTransaction(m_connectionName);

    // Step: Calculate checksums for all files (if enabled)
    if (catalog->includeChecksum != Catalog::CHECKSUM_NONE) {
        qDebug() << "=== Starting checksum calculation for new catalog ===";
        qDebug() << "Checksum setting:" << catalog->includeChecksum;

        QList<QVariantList> filesToChecksum = findFilesWithoutChecksum();

        if (!filesToChecksum.isEmpty()) {
            qDebug() << "Found" << filesToChecksum.size() << "files needing checksum calculation";
            extractChecksumsForFiles(filesToChecksum);
        } else {
            qDebug() << "No files need checksum calculation";
        }
    } else {
        qDebug() << "Checksum calculation disabled for this catalog (setting: None)";
    }

    // Update catalog metadata
    catalog->updateFileCount();
    catalog->updateTotalFileSize();
    catalog->saveCatalog();

    // Final progress update
    qDebug() << "About to emit final progress update";
    //emitProgressUpdate(processedCount, countedTotalFiles, "temp_text_for_test2");
    qDebug() << "Final progress update emitted";

    qDebug() << "=== CatalogJobStoppable::createCatalogWithProgress() completed successfully ===";
}

void CatalogJobStoppable::updateCatalogWithProgress()
{
    qDebug() << "=== CATALOG UPDATE STARTED ===";
    qDebug() << "Device ID:" << m_device->ID;
    qDebug() << "Catalog Name:" << m_device->catalog->name;
    bool shouldUseFullRescan = false;  //Full Rescan is not used, keeping for later user/test option
    // Decide between full rescan and incremental update
    if (shouldUseFullRescan) {
        // Clear existing files since we're doing a full rescan
        QSqlQuery clearQuery(QSqlDatabase::database(m_connectionName));
        if (!clearQuery.exec(QString("DELETE FROM file WHERE file_catalog_id = %1").arg(m_device->catalog->ID))) {
            qDebug() << "Warning: Could not clear existing files:" << clearQuery.lastError().text();
        }

        // Do full rescan (same as creation)
        createCatalogWithProgress();

        if (shouldContinue()) {
            completeCatalogCreation();
        }
    } else {
        qDebug() << "Using INCREMENTAL UPDATE";
        updateCatalogIncremental();  // Fast incremental update
    }

    qDebug() << "=== CATALOG UPDATE COMPLETED ===";
}

void CatalogJobStoppable::processDirectoryWithProgress(const QString &directory,
                                                       Catalog *catalog,
                                                       qint64 &processedCount)
{
    if (!shouldContinue()) return;

    qDebug() << "Processing directory:" << directory;

    // Get file extensions for filtering (for files only)
    QStringList extensions;
    if (catalog->fileType == "Image") {
        extensions = FileTypeMapping::getExtensionsForCataloging("image");
    } else if (catalog->fileType == "Audio") {
        extensions = FileTypeMapping::getExtensionsForCataloging("audio");
    } else if (catalog->fileType == "Video") {
        extensions = FileTypeMapping::getExtensionsForCataloging("video");
    } else if (catalog->fileType == "Text") {
        extensions = FileTypeMapping::getExtensionsForCataloging("text");
    } else if (catalog->fileType == "Other") {
        extensions = FileTypeMapping::getExtensionsForCataloging("other");
        if (extensions.isEmpty()) {
            qDebug() << "WARNING: Other type returned empty extensions list!";
        }
    } else if (catalog->fileType == "None") {
        extensions << "*";
    } else {
        extensions << "*";
    }

    // Use QDirIterator for efficient directory traversal - RESTORE ORIGINAL APPROACH
    // Changed from QDir::Files to QDir::AllEntries to include directories (like v2.6)
    QDir::Filters filters = QDir::AllEntries | QDir::NoDotAndDotDot | QDir::Readable;
    if (catalog->includeHidden) {
        filters |= QDir::Hidden;
    }

    QDirIterator::IteratorFlags iteratorFlags = QDirIterator::Subdirectories;
    if (catalog->includeSymblinks) {
        iteratorFlags |= QDirIterator::FollowSymlinks;
    }

    QDirIterator it(directory, extensions, filters, iteratorFlags);

    // Single set of arrays for batching
    QStringList fileNames, fileFolderPaths, fileFullPaths, fileDateTimes, fileCatalogs;
    QStringList fileExtensions, fileTypes, mimeTypes;
    QList<qint64> fileSizes;
    QStringList directoryPaths;

    //int batchSize = (catalog->includeMetadata != Catalog::METADATA_NONE) ? 10 : 1000;
    // Batch size depends on database mode and whether metadata extraction is enabled
    int batchSize;
    if (catalog->includeMetadata == Catalog::METADATA_NONE) {
        batchSize = 1000;  // No metadata extraction - can batch more
    } else {
        // With metadata extraction, batch size depends on database mode
        if (m_databaseMode == "Memory") {
            batchSize = 100;   // Memory mode can handle larger batches
        } else {
            batchSize = 100;   // File/SQLite mode - smaller batches
        }
    }

    while (it.hasNext() && shouldContinue()) {
        waitIfPaused();

        QString entryPath = it.next();
        QFileInfo entry(entryPath);

        // Skip excluded folders
        bool isExcluded = false;
        for (const QString &excludedFolder : catalog->excludedFolders) {
            if (entryPath.contains(excludedFolder)) {
                isExcluded = true;
                break;
            }
        }
        if (isExcluded) continue;

        // Handle directories and files
        if (entry.isDir()) {
            directoryPaths << entryPath;
        } else if (entry.isFile()) {
            // Add file to batch arrays
            QString extension = entry.suffix().toLower();
            QString quickFileType = FileMetadata::getFileTypeFromExtension(extension);
            QString quickMimeType = FileMetadata::getMimeTypeFromExtension(extension);

            fileNames << entry.fileName();
            fileFolderPaths << entry.path();
            fileFullPaths << entryPath;
            fileDateTimes << entry.lastModified().toString("yyyy/MM/dd hh:mm:ss");
            fileCatalogs << catalog->name;
            fileSizes << entry.size();
            fileExtensions << extension;
            fileTypes << quickFileType;
            mimeTypes << quickMimeType;

            processedCount++;

            // Progress update
            if (processedCount % progressRefreshRate == 0) {
                emitProgressUpdate(processedCount, countedTotalFiles, entryPath);
                QCoreApplication::processEvents();
            }
        }

        // Process batch when full (SINGLE CONDITION)
        if (fileNames.size() >= batchSize) {
            processBatch(fileNames, fileFolderPaths, fileFullPaths, fileDateTimes,
                         fileCatalogs, fileSizes, fileExtensions, fileTypes, mimeTypes,
                         directoryPaths, catalog);
        }
    }

    // Process any remaining items (SINGLE FINAL CALL)
    if (!fileNames.isEmpty() || !directoryPaths.isEmpty()) {
        processBatch(fileNames, fileFolderPaths, fileFullPaths, fileDateTimes,
                     fileCatalogs, fileSizes, fileExtensions, fileTypes, mimeTypes,
                     directoryPaths, catalog);
    }

    // Final progress update for this directory
    if (shouldContinue()) {
        emitProgressUpdate(processedCount, countedTotalFiles, "");
        QCoreApplication::processEvents(); // Final UI update
    }

    //qDebug() << "Processed" << filesProcessedInThisCall << "files in directory:" << directory;
}

qint64 CatalogJobStoppable::countTotalFiles(const QString &directory, Catalog *catalog)
{
    qint64 totalFiles = 0;

    if (!shouldContinue()) return 0;

    // Get file extensions for filtering (same logic as in processDirectoryWithProgress)
    QStringList extensions;
    if (catalog->fileType == "Image") {
        extensions = FileTypeMapping::getExtensionsForCataloging("image");
    } else if (catalog->fileType == "Audio") {
        extensions = FileTypeMapping::getExtensionsForCataloging("audio");
    } else if (catalog->fileType == "Video") {
        extensions = FileTypeMapping::getExtensionsForCataloging("video");
    } else if (catalog->fileType == "Text") {
        extensions = FileTypeMapping::getExtensionsForCataloging("text");
    } else if (catalog->fileType == "Other") {
        extensions = FileTypeMapping::getExtensionsForCataloging("other");
    } else if (catalog->fileType == "None") {
        extensions << "*";
    } else {
        // Default/All: include all files
        extensions << "*";
    }

    // FIXED: Use same approach as processDirectoryWithProgress - count FILES only, not directories
    // We only count files for progress tracking, but directories will still be processed
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

        if (catalog->fileType == "None") {
            QFileInfo fileInfo(filePath);
            QString extension = fileInfo.suffix();
            if (!extension.isEmpty()) {
                continue; // Skip files that have extensions
            }
        }

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

    emitProgressUpdate(0, totalFiles, QString("__COUNTING_STATE__|%1").arg(totalFiles));
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

    // qDebug() << "Progress:" << processed << "/" << total << "("
    //          << (total > 0 ? (processed * 100 / total) : 0) << "%) -" << currentPath;

    emit catalogProgress(processed, total, currentPath);
}

void CatalogJobStoppable::updateRelatedCatalogDevices()
{
    if (!m_device) return;

    qDebug() << "Updating related catalog devices...";

    try {
        // Update related devices (other catalog devices using the same catalog ID)
        QSqlQuery queryRelatedDevice(QSqlDatabase::database(m_connectionName));
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
            relatedDevice.loadDevice(m_connectionName);

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
        m_device->catalog->setDateLoaded(QDateTime());

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

void CatalogJobStoppable::verifyMimeTypes()
{
    qDebug() << "=== Starting MIME type verification for catalog:" << m_device->catalog->name;

    if (!m_device || !m_device->catalog) {
        qDebug() << "No device or catalog configured";
        emit catalogOperationError("No device or catalog configured");
        return;
    }

    // Reset counters
    m_mismatchCount = 0;
    m_reportFilePath.clear();

    // Query for files without MIME verification
    QSqlQuery query(QSqlDatabase::database(m_connectionName));
    query.prepare(R"(
        SELECT file_full_path, file_extension, file_type
        FROM file
        WHERE file_catalog_id = :catalog_id
        AND (mime_verified = 0 OR mime_verified IS NULL)
        ORDER BY file_size ASC
    )");
    query.bindValue(":catalog_id", m_device->catalog->ID);

    if (!query.exec()) {
        qDebug() << "Failed to query files for MIME verification:" << query.lastError().text();
        emit catalogOperationError("Failed to query files for MIME verification");
        return;
    }

    // Count total files to process
    int totalFiles = 0;
    QList<QVariantList> filesToProcess;
    while (query.next()) {
        QVariantList fileData;
        fileData << query.value(0)  // file_full_path
                 << query.value(1)  // file_extension
                 << query.value(2);  // file_type
        filesToProcess.append(fileData);
        totalFiles++;
    }

    qDebug() << "Found" << totalFiles << "files to verify MIME types";

    if (totalFiles == 0) {
        emitProgressUpdate(0, 0, "No files need MIME verification");
        // Emit completion with 0 mismatches
        emit mimeVerificationCompleted(0, QString());
        return;
    }

    // Process files
    int processedFiles = 0;
    QStringList mismatches;

    // Begin transaction for better performance
    QSqlDatabase db = QSqlDatabase::database(m_connectionName);
    db.transaction();

    emitProgressUpdate(0, totalFiles, "Starting MIME verification...");

    for (const QVariantList &fileData : filesToProcess) {
        if (!shouldContinue()) {
            qDebug() << "MIME verification stopped by user";
            db.rollback();
            return;
        }

        // Check for pause
        waitIfPaused();

        QString filePath = fileData[0].toString();
        QString extension = fileData[1].toString();
        QString extensionType = fileData[2].toString();

        // Check if file still exists
        QFileInfo fileInfo(filePath);
        if (!fileInfo.exists()) {
            qDebug() << "File no longer exists:" << filePath;

            // Mark as verified even if file doesn't exist
            QSqlQuery updateQuery(QSqlDatabase::database(m_connectionName));
            updateQuery.prepare(R"(
                UPDATE file
                SET mime_verified = 1,
                    type_mismatch = 0
                WHERE file_full_path = :file_full_path
            )");
            updateQuery.bindValue(":file_full_path", filePath);
            updateQuery.exec();

            processedFiles++;
            continue;
        }

        // Verify MIME type
        QVariantMap verifyResult = FileMetadata::verifyMimeType(filePath, extensionType);

        if (verifyResult.contains("error")) {
            qDebug() << "Error verifying" << filePath << ":" << verifyResult["error"].toString();

            // Mark as verified with no mismatch on error
            QSqlQuery updateQuery(QSqlDatabase::database(m_connectionName));
            updateQuery.prepare(R"(
                UPDATE file
                SET mime_verified = 1,
                    type_mismatch = 0
                WHERE file_full_path = :file_full_path
            )");
            updateQuery.bindValue(":file_full_path", filePath);
            updateQuery.exec();

            processedFiles++;
            continue;
        }

        QString mimeType = verifyResult["mime_type"].toString();
        QString mimeBasedType = verifyResult["file_type"].toString();
        bool hasMismatch = verifyResult["has_mismatch"].toBool();

        // Track mismatches
        if (hasMismatch) {
            m_mismatchCount++;
            QString mismatchInfo = QString("%1 [%2]: Extension suggests '%3', MIME detected '%4' (%5)")
                                       .arg(fileInfo.fileName())
                                       .arg(extension.isEmpty() ? "no ext" : extension)
                                       .arg(extensionType)
                                       .arg(mimeBasedType)
                                       .arg(mimeType);
            mismatches << mismatchInfo;

            // Log first few mismatches
            if (m_mismatchCount <= 10) {
                qDebug() << "Mismatch found:" << mismatchInfo;
            }
        }

        // Update database - ALWAYS mark as verified
        QSqlQuery updateQuery(QSqlDatabase::database(m_connectionName));
        updateQuery.prepare(R"(
            UPDATE file
            SET mime_type = :mime_type,
                file_type = :file_type,
                mime_verified = 1,
                type_mismatch = :mismatch
            WHERE file_full_path = :file_full_path
        )");
        updateQuery.bindValue(":mime_type", mimeType);
        updateQuery.bindValue(":file_type", mimeBasedType);  // Update to accurate type
        updateQuery.bindValue(":mismatch", hasMismatch ? 1 : 0);
        updateQuery.bindValue(":file_full_path", filePath);

        if (!updateQuery.exec()) {
            qDebug() << "Failed to update file" << filePath << ":" << updateQuery.lastError().text();
        }

        processedFiles++;

        // Progress update every 10 files
        if (processedFiles % 10 == 0 || processedFiles == totalFiles) {
            int percentComplete = (processedFiles * 100) / totalFiles;
            QString progressMsg = QString("Verifying MIME types: %1/%2 (%3%) - %4 mismatches found")
                                      .arg(processedFiles)
                                      .arg(totalFiles)
                                      .arg(percentComplete)
                                      .arg(m_mismatchCount);

            emitProgressUpdate(processedFiles, totalFiles, progressMsg);
            QCoreApplication::processEvents();

            // Commit every 100 files
            if (processedFiles % 100 == 0 && processedFiles < totalFiles) {
                db.commit();
                db.transaction();
            }
        }
    }

    // Final commit
    db.commit();

    // Save mismatch report ONLY if there are actual mismatches
    if (m_mismatchCount > 0 && !mismatches.isEmpty()) {
        qDebug() << "Total mismatches found:" << m_mismatchCount;
        m_reportFilePath = saveMismatchReportAndReturnPath(mismatches);
    } else {
        m_reportFilePath.clear(); // Ensure empty path when no mismatches
    }

    // Final progress update
    QString finalMsg = QString("MIME verification completed: %1 files processed, %2 mismatches found")
                           .arg(processedFiles)
                           .arg(m_mismatchCount);
    emitProgressUpdate(processedFiles, totalFiles, finalMsg);

    // Emit completion with results
    emit mimeVerificationCompleted(m_mismatchCount, m_reportFilePath);
}

void CatalogJobStoppable::saveMismatchReport(const QStringList &mismatches)
{
    // Save to a report file in the collection folder
    QString reportPath = m_collectionFolder + "/mime_mismatches_" +
                         m_device->catalog->name + "_" +
                         QDateTime::currentDateTime().toString("yyyyMMdd_hhmmss") + ".txt";

    QFile reportFile(reportPath);
    if (reportFile.open(QIODevice::WriteOnly | QIODevice::Text)) {
        QTextStream stream(&reportFile);
        stream << "MIME Type Verification Report\n";
        stream << "Catalog: " << m_device->catalog->name << "\n";
        stream << "Date: " << QDateTime::currentDateTime().toString() << "\n";
        stream << "Total mismatches: " << mismatches.size() << "\n";
        stream << "=====================================\n\n";

        for (const QString &mismatch : mismatches) {
            stream << mismatch << "\n";
        }

        reportFile.close();
        qDebug() << "Mismatch report saved to:" << reportPath;
    } else {
        qDebug() << "Failed to save mismatch report to:" << reportPath;
    }
}

bool CatalogJobStoppable::shouldExtractMetadata(const QString &filePath, Catalog *catalog) const
{
    // No metadata extraction if disabled
    if (catalog->includeMetadata == Catalog::METADATA_NONE) {
        return false;
    }

    // Get the file info
    QFileInfo fileInfo(filePath);
    QString extension = fileInfo.suffix().toLower();

    // Get the quick file type (from extension cache)
    QString fileType = FileMetadata::getFileTypeFromExtension(extension);

    // Check metadata level restrictions
    if (catalog->includeMetadata == Catalog::METADATA_MEDIA_BASIC ||
        catalog->includeMetadata == Catalog::METADATA_MEDIA_EXTENDED) {
        // Only extract for media files
        if (fileType != "Image" && fileType != "Audio" && fileType != "Video") {
            return false;  // Skip non-media files
        }
    }
    // For METADATA_FULL, extract for all supported files

    // Now check extraction strategy (only for files that passed the above checks)
    QSettings settings(m_collectionFolder + "/settings.ini", QSettings::IniFormat);
    QString strategy = settings.value("Settings/MetadataExtractionStrategy", "Effective").toString();

    if (strategy == "Thorough") {
        // Always attempt extraction for files that pass the metadata level check
        return true;
    }
    else if (strategy == "Fastest") {
        // Use cached extension support check
        return FileMetadata::isExtensionSupported(extension);
    }
    else { // "Effective" (default)
        // Use MIME-based check (but only if MIME has been verified)
        // First check if we have verified MIME for this file
        QSqlQuery query(QSqlDatabase::database(m_connectionName));
        query.prepare(R"(
            SELECT mime_verified, mime_type
            FROM file
            WHERE file_catalog_id = :catalog_id
            AND file_full_path = :path
            LIMIT 1
        )");
        query.bindValue(":catalog_id", catalog->ID);
        query.bindValue(":path", filePath);

        if (query.exec() && query.next()) {
            bool mimeVerified = query.value(0).toBool();
            if (mimeVerified) {
                // Use the verified MIME type
                QString mimeType = query.value(1).toString();
                KFileMetaData::ExtractorCollection extractors;
                auto extractorsList = extractors.fetchExtractors(mimeType);
                return !extractorsList.isEmpty();
            }
        }

        // Fallback to file-based MIME detection if not verified
        return FileMetadata::isMetadataSupported(filePath);
    }
}

void CatalogJobStoppable::extractMissingMetadata()
{
    qDebug() << "=== Starting metadata extraction for files missing metadata ===";

    if (!m_device || !m_device->catalog) {
        qDebug() << "No device or catalog configured";
        emit catalogOperationError("No device or catalog configured");
        return;
    }

    // Query for files without metadata
    QSqlQuery query(QSqlDatabase::database(m_connectionName));
    query.prepare(R"(
        SELECT file_id, file_full_path, file_name, file_folder_path, file_type
        FROM file
        WHERE file_catalog_id = :catalog_id
        AND (metadata_extraction_date IS NULL OR metadata_extraction_date = '')
        AND file_type IN ('Image', 'Audio', 'Video')
        ORDER BY file_size ASC
    )");
    query.bindValue(":catalog_id", m_device->catalog->ID);

    if (!query.exec()) {
        qDebug() << "Failed to query files for metadata extraction:" << query.lastError().text();
        emit catalogOperationError("Failed to query files for metadata extraction");
        return;
    }

    // Count total files to process
    int totalFiles = 0;
    QList<QVariantList> filesToProcess;
    while (query.next()) {
        QVariantList fileData;
        fileData << query.value(0)  // file_id
                 << query.value(1)  // file_full_path
                 << query.value(2)  // file_name
                 << query.value(3)  // file_folder_path
                 << query.value(4); // file_type
        filesToProcess.append(fileData);
        totalFiles++;
    }

    qDebug() << "Found" << totalFiles << "files missing metadata";

    if (totalFiles == 0) {
        emitProgressUpdate(0, 0, "No files need metadata extraction");
        return;
    }

    // Process files
    int processedFiles = 0;

    for (const QVariantList &fileData : filesToProcess) {
        if (!shouldContinue()) {
            qDebug() << "Metadata extraction stopped by user";
            return;
        }

        waitIfPaused();

        QString filePath = fileData[1].toString();
        QString fileName = fileData[2].toString();
        QString folderPath = fileData[3].toString();
        QString fileType = fileData[4].toString();

        // Check if file still exists
        QFileInfo fileInfo(filePath);
        if (!fileInfo.exists()) {
            qDebug() << "File no longer exists:" << filePath;
            processedFiles++;
            continue;
        }

        // Extract metadata based on catalog settings
        if (FileMetadata::isMetadataSupported(filePath)) {
            FileMetadata::extractAndStore(filePath, m_connectionName,
                                          m_device->catalog->ID,
                                          m_device->catalog->includeMetadata);
        }

        processedFiles++;

        // Progress update
        if (processedFiles % 10 == 0 || processedFiles == totalFiles) {
            QString marker = QString("__METADATA_EXTRACTION__|%1|%2").arg(processedFiles).arg(totalFiles);
            emitProgressUpdate(processedFiles, totalFiles, marker);
            QCoreApplication::processEvents();
        }
    }

    // Final progress update
    QString marker = QString("__METADATA_EXTRACTION__|%1|%2").arg(processedFiles).arg(totalFiles);
    emitProgressUpdate(processedFiles, totalFiles, marker);

    qDebug() << "=== Metadata extraction completed: %1 files processed ===" << processedFiles;

    //qDebug() << "=== Metadata extraction completed ===" << finalMsg;
}

QString CatalogJobStoppable::saveMismatchReportAndReturnPath(const QStringList &mismatches)
{
    // Save to a report file in the collection folder
    QString reportPath = m_collectionFolder + "/mime_mismatches_" +
                         m_device->catalog->name + "_" +
                         QDateTime::currentDateTime().toString("yyyyMMdd_hhmmss") + ".txt";

    QFile reportFile(reportPath);
    if (reportFile.open(QIODevice::WriteOnly | QIODevice::Text)) {
        QTextStream stream(&reportFile);
        stream << "MIME Type Verification Report\n";
        stream << "Catalog: " << m_device->catalog->name << "\n";
        stream << "Date: " << QDateTime::currentDateTime().toString() << "\n";
        stream << "Total mismatches: " << mismatches.size() << "\n";
        stream << "=====================================\n\n";

        for (const QString &mismatch : mismatches) {
            stream << mismatch << "\n";
        }

        reportFile.close();
        qDebug() << "Mismatch report saved to:" << reportPath;
        return reportPath;
    } else {
        qDebug() << "Failed to save mismatch report to:" << reportPath;
        return QString();
    }
}

void CatalogJobStoppable::processBatch(QStringList& fileNames, QStringList& fileFolderPaths,
                                       QStringList& fileFullPaths, QStringList& fileDateTimes,
                                       QStringList& fileCatalogs, QList<qint64>& fileSizes,
                                       QStringList& fileExtensions, QStringList& fileTypes,
                                       QStringList& mimeTypes,
                                       QStringList& directoryPaths, Catalog* catalog)
{
    // Insert files if any
    if (!fileNames.isEmpty()) {
        QSqlQuery query(QSqlDatabase::database(m_connectionName));
        query.prepare(R"(
            INSERT INTO file (file_catalog_id, file_name, file_folder_path, file_full_path,
                            file_size, file_date_updated, file_catalog, file_extension, file_type, mime_type)
            VALUES (:catalog_id, :name, :folder_path, :full_path,
                    :size, :date, :catalog_name, :extension, :file_type, :mime_type)
        )");

        for (int i = 0; i < fileNames.size(); ++i) {
            if (!shouldContinue()) break;

            query.bindValue(":catalog_id", catalog->ID);
            query.bindValue(":name", fileNames[i]);
            query.bindValue(":folder_path", fileFolderPaths[i]);
            query.bindValue(":full_path", fileFullPaths[i]);
            query.bindValue(":size", fileSizes[i]);
            query.bindValue(":date", fileDateTimes[i]);
            query.bindValue(":catalog_name", fileCatalogs[i]);
            query.bindValue(":extension", fileExtensions[i]);
            query.bindValue(":file_type", fileTypes[i]);
            query.bindValue(":mime_type", mimeTypes[i]);

            if (!query.exec()) {
                qDebug() << "Database insert error:" << query.lastError().text();
            }
        }

/*        // Metadata extraction - BATCHED
        if (catalog->includeMetadata != Catalog::METADATA_NONE) {
            QElapsedTimer batchTimer;
            batchTimer.start();

            QStringList batchFileNames;
            QStringList batchFolderPaths;
            QList<QVariantMap> batchMetadata;

            QElapsedTimer extractTimer;
            extractTimer.start();

            for (int i = 0; i < fileFullPaths.size(); ++i) {
                if (!shouldContinue()) break;

                QString filePath = fileFullPaths[i];
                if (!FileMetadata::isMetadataSupported(filePath)) {
                    continue;
                }

                // Extract metadata (without storing)
                QVariantMap metadata = FileMetadata::extractMetadata(filePath, catalog->includeMetadata);

                if (!metadata.isEmpty()) {
                    batchFileNames << fileNames[i];
                    batchFolderPaths << fileFolderPaths[i];
                    batchMetadata << metadata;
                }
            }

            int extractDurationMs = extractTimer.elapsed();

            // Batch update all metadata at once (much faster than individual updates)
            QElapsedTimer updateTimer;
            updateTimer.start();

            if (!batchFileNames.isEmpty()) {
                FileMetadata::batchUpdateFileMetadata(m_connectionName, catalog->ID,
                                                      batchFileNames, batchFolderPaths,
                                                      batchMetadata);
            }

            int updateDurationMs = updateTimer.elapsed();
            int totalBatchMs = batchTimer.elapsed();

            // DETAILED TIMING OUTPUT
            qDebug() << "=== BATCH TIMING ==="
                     << "Files:" << batchFileNames.size()
                     << "| Extract:" << extractDurationMs << "ms"
                     << "(" << (extractDurationMs / qMax(1, (int)batchFileNames.size())) << "ms/file)"
                     << "| Update:" << updateDurationMs << "ms"
                     << "| Total batch:" << totalBatchMs << "ms";
        }
*/

        // Metadata extraction - PARALLEL for performance
        if (catalog->includeMetadata != Catalog::METADATA_NONE) {
            QElapsedTimer batchTimer;
            batchTimer.start();

            QStringList extractFilePaths;
            QStringList extractFileNames;
            QStringList extractFolderPaths;

            for (int i = 0; i < fileFullPaths.size(); ++i) {
                QString filePath = fileFullPaths[i];
                if (!FileMetadata::isMetadataSupported(filePath)) {
                    continue;
                }

                extractFilePaths << filePath;
                extractFileNames << fileNames[i];
                extractFolderPaths << fileFolderPaths[i];
            }

            if (!extractFilePaths.isEmpty()) {
                QElapsedTimer extractTimer;
                extractTimer.start();

                // Determine thread count based on system and database mode
                int optimalThreads = 4;  // Safe default

                int availableCores = QThread::idealThreadCount();
                if (availableCores > 0) {
                    // Use formula: min(available_cores - 1, 8) for safety
                    // Leave 1 core for UI thread and system
                    optimalThreads = qMin(availableCores - 1, 8);

                    // For file-based DBs, be more conservative (I/O bound)
                    if (m_databaseMode != "Memory") {
                        optimalThreads = qMin(optimalThreads, 4);
                    }
                }

                qDebug() << "Parallel extraction:" << optimalThreads << "threads"
                         << "(" << availableCores << "cores available)"
                         << "Database mode:" << m_databaseMode;

                ParallelMetadataExtractor extractor;
                QList<MetadataExtractionResult> results = extractor.extractBatch(
                    extractFilePaths,
                    extractFileNames,
                    extractFolderPaths,
                    catalog->includeMetadata,
                    optimalThreads  // Use calculated nb of threads
                    );

                int extractDurationMs = extractTimer.elapsed();

                QStringList batchFileNames;
                QStringList batchFolderPaths;
                QList<QVariantMap> batchMetadata;

                for (const auto& result : results) {
                    if (!result.metadata.isEmpty()) {
                        batchFileNames << result.fileName;
                        batchFolderPaths << result.folderPath;
                        batchMetadata << result.metadata;
                    }
                }

                QElapsedTimer updateTimer;
                updateTimer.start();

                if (!batchFileNames.isEmpty()) {
                    FileMetadata::batchUpdateFileMetadata(m_connectionName, catalog->ID,
                                                          batchFileNames, batchFolderPaths,
                                                          batchMetadata);
                }

                int updateDurationMs = updateTimer.elapsed();
                int totalBatchMs = batchTimer.elapsed();

                qDebug() << "=== PARALLEL BATCH ==="
                         << "Files:" << extractFileNames.size()
                         << "| Extract:" << extractDurationMs << "ms"
                         << "(" << (extractDurationMs / qMax(1, (int)extractFileNames.size())) << "ms/file)"
                         << "| Update:" << updateDurationMs << "ms"
                         << "| Total batch:" << totalBatchMs << "ms";
            }
        }

        // Insert folders from files
        QStringList uniqueFolders = fileFolderPaths;
        uniqueFolders.removeDuplicates();
        insertFolders(uniqueFolders, catalog);
    }

    // Insert directories
    if (!directoryPaths.isEmpty()) {
        insertFolders(directoryPaths, catalog);
    }

    // Clear all arrays
    fileNames.clear();
    fileFolderPaths.clear();
    fileFullPaths.clear();
    fileDateTimes.clear();
    fileCatalogs.clear();
    fileSizes.clear();
    fileExtensions.clear();
    fileTypes.clear();
    mimeTypes.clear();
    directoryPaths.clear();
}

void CatalogJobStoppable::insertFolders(const QStringList& folderPaths, Catalog* catalog)
{
    if (folderPaths.isEmpty()) return;

    Database::DatabaseType databaseType = Database::getDatabaseType(m_connectionName);
    QString insertPrefix = Database::getInsertOrIgnorePrefix(databaseType);
    QString insertSQL;

    if (databaseType == Database::DatabaseType::PostgreSQL) {
        // PostgreSQL requires ON CONFLICT clause
        insertSQL = QString(R"(
        %1 INTO folder (folder_catalog_id, folder_path)
        VALUES (:catalog_id, :path)
        ON CONFLICT (folder_catalog_id, folder_path) DO NOTHING
    )").arg(insertPrefix);
    } else {
        insertSQL = QString(R"(
        %1 INTO folder (folder_catalog_id, folder_path)
        VALUES (:catalog_id, :path)
    )").arg(insertPrefix);
    }

    QSqlQuery folderQuery(QSqlDatabase::database(m_connectionName));
    folderQuery.prepare(insertSQL);

    for (const QString& folderPath : folderPaths) {
        if (!shouldContinue()) break;
        folderQuery.bindValue(":catalog_id", catalog->ID);
        folderQuery.bindValue(":path", folderPath);
        if (!folderQuery.exec()) {
            qDebug() << "Folder insert error:" << folderQuery.lastError().text();
        }
    }
}

void CatalogJobStoppable::updateCatalogIncremental()
{
    qDebug() << "=== INCREMENTAL CATALOG UPDATE STARTED ===";
    qDebug() << "Device ID:" << m_device->ID;
    qDebug() << "Catalog Name:" << m_device->catalog->name;
    qDebug() << "Source Path:" << m_device->catalog->sourcePath;
    qDebug() << "Database Mode:" << m_databaseMode;

    if (!m_device || !m_device->catalog) {
        qDebug() << "ERROR: No device or catalog configured";
        emit catalogOperationError("No device or catalog configured");
        return;
    }

    Catalog *catalog = m_device->catalog;
    m_updateStats.clear();

    m_originalFileCount = catalog->fileCount;
    m_originalTotalFileSize = catalog->totalFileSize;
    qDebug() << "Captured original values - Files:" << m_originalFileCount << "Size:" << m_originalTotalFileSize;

    // Step 1a: Clear filetemp table
    qDebug() << "Step 1: Clearing filetemp table";
    QSqlQuery clearQuery(QSqlDatabase::database(m_connectionName));
    if (!clearQuery.exec("DELETE FROM filetemp")) {
        qDebug() << "ERROR: Could not clear filetemp:" << clearQuery.lastError().text();
        emit catalogOperationError("Failed to prepare temporary table");
        return;
    }

    // Step 1b: Load existing catalog data for incremental comparison (conditional)
    bool shouldLoadCatalog = false;

    if (m_databaseMode == "Memory") {
        // Determine if we need to load the catalog
        bool metadataEnabled = (catalog->includeMetadata != Catalog::METADATA_NONE);
        bool userForcedLoad = false;

        // Check user setting for forced loading (for metadata = None case)
        QSqlQuery settingQuery(QSqlDatabase::database(m_connectionName));
        if (settingQuery.exec("SELECT setting_value FROM parameter WHERE setting_name = 'LoadCatalogBeforeUpdate'")) {
            if (settingQuery.next()) {
                userForcedLoad = (settingQuery.value(0).toString() == "true");
            }
        }

        // Load catalog if: (1) metadata extraction enabled, OR (2) user enabled the setting
        shouldLoadCatalog = metadataEnabled || userForcedLoad;

        qDebug() << "Step 1a: Checking if catalog loading needed";
        qDebug() << "  Metadata enabled:" << metadataEnabled;
        qDebug() << "  User forced load:" << userForcedLoad;
        qDebug() << "  Will load catalog:" << shouldLoadCatalog;

        if (shouldLoadCatalog) {
            qDebug() << "Loading existing catalog data for incremental comparison";
            qDebug() << "Catalog dateLoaded:" << catalog->dateLoaded.toString("yyyy-MM-dd hh:mm:ss");
            qDebug() << "Catalog dateUpdated:" << catalog->dateUpdated.toString("yyyy-MM-dd hh:mm:ss");

            // Check if catalog needs loading
            if (catalog->dateLoaded < catalog->dateUpdated || catalog->dateLoaded.isNull()) {
                qDebug() << "Catalog data needs loading from .idx file";

                // Count existing files before loading
                QSqlQuery countBeforeQuery(QSqlDatabase::database(m_connectionName));
                countBeforeQuery.prepare("SELECT COUNT(*) FROM file WHERE file_catalog_id = :catalog_id");
                countBeforeQuery.bindValue(":catalog_id", catalog->ID);
                if (countBeforeQuery.exec() && countBeforeQuery.next()) {
                    qDebug() << "Files in file table before loading:" << countBeforeQuery.value(0).toInt();
                }

                // Emit loading start progress
                emitProgressUpdate(0, catalog->fileCount, QString("__LOADING_CATALOG__|0|%1").arg(catalog->fileCount));
                QCoreApplication::processEvents();

                bool localStopRequested = false;
                QMutex catalogMutex;
                QMetaObject::Connection progressConnection;

                // Emit initial loading marker BEFORE loading starts
                QString initialMarker = QString("__CATALOG_LOADING__|0|%1").arg(catalog->fileCount);
                emitProgressUpdate(0, catalog->fileCount, initialMarker);
                QCoreApplication::processEvents();

                // Connect to catalog loading progress (same pattern as search)
                progressConnection = connect(catalog, &Catalog::loadProgress, this,
                                             [this, &localStopRequested, &progressConnection](int filesLoaded, int totalFiles) {
                                                 if (!shouldContinue()) {
                                                     qDebug() << "Stop detected during catalog loading - disconnecting";
                                                     QObject::disconnect(progressConnection);
                                                     localStopRequested = true;
                                                     return;
                                                 }

                                                 // Emit loading progress using marker pattern
                                                 QString marker = QString("__CATALOG_LOADING__|%1|%2").arg(filesLoaded).arg(totalFiles);
                                                 emitProgressUpdate(filesLoaded, totalFiles, marker);

                                                 QCoreApplication::processEvents();
                                             }, Qt::DirectConnection);

                catalog->loadCatalogFileListToTable(catalogMutex, localStopRequested);

                // Disconnect after loading completes
                if (progressConnection) {
                    disconnect(progressConnection);
                }

                if (localStopRequested || !shouldContinue()) {
                    qDebug() << "Stop requested during catalog loading";
                    return;
                }

                // Count after loading
                QSqlQuery countAfterQuery(QSqlDatabase::database(m_connectionName));
                countAfterQuery.prepare("SELECT COUNT(*) FROM file WHERE file_catalog_id = :catalog_id");
                countAfterQuery.bindValue(":catalog_id", catalog->ID);
                if (countAfterQuery.exec() && countAfterQuery.next()) {
                    qDebug() << "Files in file table after loading:" << countAfterQuery.value(0).toInt();
                }

                qDebug() << "Existing catalog data loaded successfully";
            } else {
                qDebug() << "Catalog data already loaded (dateLoaded >= dateUpdated)";

                // Still verify we have data
                QSqlQuery verifyQuery(QSqlDatabase::database(m_connectionName));
                verifyQuery.prepare("SELECT COUNT(*) FROM file WHERE file_catalog_id = :catalog_id");
                verifyQuery.bindValue(":catalog_id", catalog->ID);
                if (verifyQuery.exec() && verifyQuery.next()) {
                    int fileCount = verifyQuery.value(0).toInt();
                    qDebug() << "Files already in memory:" << fileCount;

                    if (fileCount == 0 && catalog->fileCount > 0) {
                        qDebug() << "WARNING: Expected" << catalog->fileCount << "files but found 0 - forcing load";
                        QMutex tempMutex;
                        bool tempStopRequested = false;
                        catalog->loadCatalogFileListToTable(tempMutex, tempStopRequested);
                    }
                }
            }
        } else {
            qDebug() << "Step 1a: Skipped catalog loading (metadata=None and user setting disabled)";
            qDebug() << "  Note: If all files show as 'new', enable 'Load Catalog Before Update' in Settings";
        }
    } else {
        qDebug() << "Step 1a: Skipped (File database mode - data already in database)";
    }

    // Step 2: Estimate total files for progress
    qDebug() << "Step 2: Estimating file count";
    QDateTime startTime = QDateTime::currentDateTime();
    qint64 countedTotalFiles = countTotalFiles(catalog->sourcePath, catalog);
    QDateTime endTime = QDateTime::currentDateTime();
    qDebug() << "Counting completed in" << startTime.msecsTo(endTime) << "ms";
    qDebug() << "Estimated total files:" << countedTotalFiles;

    if (!shouldContinue()) {
        qDebug() << "Stop requested during estimation";
        return;
    }

    // Step 3: Scan filesystem and populate filetemp
    qDebug() << "Step 3: Scanning filesystem into temporary table";
    Database::beginTransaction(m_connectionName);

    qint64 indexedCount = 0;
    scanDirectoryIntoFiletemp(catalog->sourcePath, catalog, indexedCount);

    if (!shouldContinue()) {
        Database::rollbackTransaction(m_connectionName);
        return;
    }

    qDebug() << "Step 3b: Scan completed - showing Indexed 100% | Saving";
    emitProgressUpdate(countedTotalFiles, countedTotalFiles,
                       QCoreApplication::translate("MainWindow", "Saving"));
    QCoreApplication::processEvents();

    Database::commitTransaction(m_connectionName);

    qDebug() << "Indexed" << indexedCount << "files into filetemp";

    // Step 4: Analyze differences using SQL
    //qDebug() << "Step 4: Analyzing differences with SQL";
    //emitProgressUpdate(indexedCount, countedTotalFiles, "Analyzing file changes...");

    QList<QVariantList> newFiles = findNewFiles();
    QList<QVariantList> modifiedFiles = findModifiedFiles();
    QStringList deletedFiles = findDeletedFiles();
    int unchangedCount = countUnchangedFiles();

    m_updateStats.newFiles = newFiles.size();
    m_updateStats.modifiedFiles = modifiedFiles.size();
    m_updateStats.deletedFiles = deletedFiles.size();
    m_updateStats.unchangedFiles = unchangedCount;

    qDebug() << "=== UPDATE ANALYSIS RESULTS ===";
    qDebug() << "  New files:       " << m_updateStats.newFiles;
    qDebug() << "  Modified files:  " << m_updateStats.modifiedFiles;
    qDebug() << "  Deleted files:   " << m_updateStats.deletedFiles;
    qDebug() << "  Unchanged files: " << m_updateStats.unchangedFiles;
    qDebug() << "  Total changes:   " << m_updateStats.totalChanges();

    if (!shouldContinue()) {
        qDebug() << "Stop requested during analysis";
        return;
    }

    // Step 5: Begin transaction for database updates
    Database::beginTransaction(m_connectionName);

    // Step 6: Process NEW files
    if (!newFiles.isEmpty()) {
        qDebug() << "Step 6a: Inserting" << newFiles.size() << "new files";
        emitProgressUpdate(0, m_updateStats.totalChanges(),
                           QString("Inserting %1 new files...").arg(newFiles.size()));
        insertNewFilesFromFiletemp(newFiles);
    }

    // Step 7: Process MODIFIED files
    if (!modifiedFiles.isEmpty()) {
        qDebug() << "Step 7: Updating" << modifiedFiles.size() << "modified files";
        emitProgressUpdate(newFiles.size(), m_updateStats.totalChanges(),
                           QString("Updating %1 modified files...").arg(modifiedFiles.size()));
        updateModifiedFilesFromFiletemp(modifiedFiles);
    }

    // Step 8: Process DELETED files
    if (!deletedFiles.isEmpty()) {
        qDebug() << "Step 8: Deleting" << deletedFiles.size() << "removed files";
        emitProgressUpdate(newFiles.size() + modifiedFiles.size(), m_updateStats.totalChanges(),
                           QString("Deleting %1 removed files...").arg(deletedFiles.size()));
        deleteRemovedFiles(deletedFiles);
    }

    if (!shouldContinue()) {
        Database::rollbackTransaction(m_connectionName);
        return;
    }

    // Commit database changes
    Database::commitTransaction(m_connectionName);

    // One-time migration for existing files without mime_type
    qDebug() << "Step 8a: Checking for mime_type migration";
    migrateMimeTypesForExistingFiles();

    // After migration completes, update catalog to current version
    if (!catalog->hasFilesNeedingMigration() && catalog->appVersion < "2.8") {
        qDebug() << "✓ All files migrated - updating catalog version to 2.8";
        catalog->appVersion = "2.8";
    }

    // Step 9: Extract metadata for new/modified/missing files (if enabled)
    if (catalog->includeMetadata != Catalog::METADATA_NONE) {
        QList<QVariantList> filesToExtract;

        // Add modified files (need re-extraction because content changed)
        filesToExtract.append(modifiedFiles);

        // Find existing files without metadata (excludes files just inserted in Step 6)
        // This handles transition case: None → Media_Basic
        QList<QVariantList> filesNeedingMetadata = findFilesWithoutMetadata();
        if (!filesNeedingMetadata.isEmpty()) {
            qDebug() << "Step 9a: Found" << filesNeedingMetadata.size() << "existing files without metadata";
            filesToExtract.append(filesNeedingMetadata);
        }

        if (!filesToExtract.isEmpty()) {
            qDebug() << "Step 9: Extracting metadata for" << filesToExtract.size() << "files";
            m_updateStats.metadataExtracted = filesToExtract.size();
            extractMetadataForChangedFiles(filesToExtract);
        }
    }

    // Step 9b: Calculate checksums for files (if enabled)
    if (catalog->includeChecksum != Catalog::CHECKSUM_NONE) {
        qDebug() << "Step 9b: Calculating checksums (setting:" << catalog->includeChecksum << ")";

        QList<QVariantList> filesToChecksum = findFilesWithoutChecksum();

        if (!filesToChecksum.isEmpty()) {
            qDebug() << "Step 9b: Found" << filesToChecksum.size() << "files needing checksum";
            extractChecksumsForFiles(filesToChecksum);
        } else {
            qDebug() << "Step 9b: All files have checksums, skipping";
        }
    } else {
        qDebug() << "Step 9b: Checksum calculation disabled (setting: None)";
    }

    // Step 10: Update catalog's file statistics
    qDebug() << "Step 10: Updating catalog's file statistics";
    catalog->updateFileCount();
    catalog->updateTotalFileSize();
    catalog->saveCatalog();

    // Step 11: Update Device object
    m_device->dateTimeUpdated = QDateTime::currentDateTime();
    m_device->totalFileCount = catalog->fileCount;
    m_device->totalFileSize = catalog->totalFileSize;
    m_device->saveStatistics(m_device->dateTimeUpdated, "update");
    m_device->saveDevice();

    // Step 12: Update parent hierarchy
    qDebug() << "Step 12: Updating parent device hierarchy";
    try {
        m_device->updateParentsNumbers();
    } catch (const std::exception& e) {
        qDebug() << "Error updating parent numbers:" << e.what();
    }

    // Step 13: Update related catalog devices
    updateRelatedCatalogDevices();

    // Step 14: Save catalog files to disk (Memory mode)
    if (!catalog->saveCatalogToFile(m_databaseMode, m_collectionFolder)) {
        qDebug() << "Warning: Failed to save updated catalog to file";
    }

    // Step 15: Complete catalog update (same as creation)
    qDebug() << "Step 15: Completing catalog update";
    // Save catalog files to disk (Memory mode)
    if (m_databaseMode == "Memory") {
        if (!catalog->saveCatalogToFile(m_databaseMode, m_collectionFolder)) {
            qDebug() << "Warning: Failed to save updated catalog to file";
        }
        if (!catalog->saveFoldersToFile(m_databaseMode, m_collectionFolder)) {
            qDebug() << "Warning: Failed to save updated folders to file";
        }
    }

    // Set catalog loaded date
    catalog->setDateLoaded(QDateTime());

    qDebug() << "=== INCREMENTAL CATALOG UPDATE COMPLETED SUCCESSFULLY ===";
    qDebug() << "Summary:" << m_updateStats.newFiles << "new,"
             << m_updateStats.modifiedFiles << "modified,"
             << m_updateStats.deletedFiles << "deleted,"
             << m_updateStats.unchangedFiles << "unchanged";
}

QList<QVariantList> CatalogJobStoppable::findFilesWithoutMetadata()
{
    QList<QVariantList> results;

    if (!m_device || !m_device->catalog) {
        return results;
    }

    Catalog* catalog = m_device->catalog;

    // Guard: If metadata disabled, return empty immediately
    if (catalog->includeMetadata == Catalog::METADATA_NONE) {
        qDebug() << "findFilesWithoutMetadata: metadata disabled, returning empty";
        return results;
    }

    // Build file_type filter based on metadata level
    QString fileTypeFilter;
    if (catalog->includeMetadata == Catalog::METADATA_MEDIA_BASIC ||
        catalog->includeMetadata == Catalog::METADATA_MEDIA_EXTENDED) {
        // Only media files
        fileTypeFilter = "AND file_type IN ('image', 'audio', 'video')";
    } else if (catalog->includeMetadata == Catalog::METADATA_FULL) {
        // All supported types - no additional filter needed
        // (files with unsupported types will be skipped by FileMetadata::isMetadataSupported)
        fileTypeFilter = "";
    }

    // Query for files without metadata_extraction_date
    QSqlQuery query(QSqlDatabase::database(m_connectionName));
    QString querySQL = QString(R"(
        SELECT file_full_path, file_size
        FROM file
        WHERE file_catalog_id = :catalog_id
        AND (metadata_extraction_date IS NULL OR metadata_extraction_date = '')
        %1
        ORDER BY file_size ASC
    )").arg(fileTypeFilter);

    query.prepare(querySQL);
    query.bindValue(":catalog_id", catalog->ID);

    if (!query.exec()) {
        qDebug() << "findFilesWithoutMetadata: Query failed:" << query.lastError().text();
        return results;
    }

    // Collect results
    while (query.next()) {
        if (!shouldContinue()) {
            qDebug() << "findFilesWithoutMetadata: Stop requested";
            break;
        }

        QVariantList fileData;
        fileData << query.value(0);  // file_full_path
        fileData << query.value(1);  // file_size (for sorting, optional)

        results.append(fileData);
    }

    qDebug() << "findFilesWithoutMetadata: Found" << results.size() << "files needing metadata";

    return results;
}

QList<QVariantList> CatalogJobStoppable::findFilesWithoutChecksum()
{
    QList<QVariantList> results;

    if (!m_device || !m_device->catalog) {
        qDebug() << "findFilesWithoutChecksum: No device or catalog";
        return results;
    }

    Catalog* catalog = m_device->catalog;

    // Guard: If checksum disabled, return empty immediately
    if (catalog->includeChecksum == Catalog::CHECKSUM_NONE) {
        qDebug() << "findFilesWithoutChecksum: checksum disabled, returning empty";
        return results;
    }

    qDebug() << "=== findFilesWithoutChecksum: Querying files needing checksum ===";
    qDebug() << "Catalog ID:" << catalog->ID;
    qDebug() << "Checksum setting:" << catalog->includeChecksum;

    // Query files without checksum
    QString querySQL = QString(
                           "SELECT file_full_path, file_name, file_folder_path, file_size "
                           "FROM file "
                           "WHERE file_catalog_id = %1 "
                           "AND (checksum_extraction_date IS NULL OR checksum_extraction_date = '') "
                           "ORDER BY file_size ASC"
                           ).arg(catalog->ID);

    QSqlQuery query(QSqlDatabase::database(m_connectionName));

    if (!query.exec(querySQL)) {
        qDebug() << "ERROR: Failed to query files for checksum:" << query.lastError().text();
        return results;
    }

    qDebug() << "DEBUG: Query executed successfully";

    int rowCount = 0;
    while (query.next()) {
        QVariantList fileData;

        // Get each value and verify it's valid
        QVariant fullPath = query.value(0);
        QVariant fileName = query.value(1);
        QVariant folderPath = query.value(2);
        QVariant fileSize = query.value(3);

        // Debug first row in detail
        if (rowCount == 0) {
            qDebug() << "DEBUG: First row data:";
            qDebug() << "  fullPath valid:" << fullPath.isValid() << "value:" << fullPath.toString();
            qDebug() << "  fileName valid:" << fileName.isValid() << "value:" << fileName.toString();
            qDebug() << "  folderPath valid:" << folderPath.isValid() << "value:" << folderPath.toString();
            qDebug() << "  fileSize valid:" << fileSize.isValid() << "value:" << fileSize.toLongLong();
        }

        fileData << fullPath << fileName << folderPath << fileSize;

        // VERIFY we have exactly 4 elements
        if (fileData.size() != 4) {
            qDebug() << "ERROR: Row" << rowCount << "has" << fileData.size() << "elements instead of 4!";
            qDebug() << "  Data:" << fileData;
            continue; // Skip this row
        }

        results.append(fileData);
        rowCount++;

        // Debug first few rows
        if (rowCount <= 3) {
            qDebug() << "  File" << rowCount << ":" << fileData[1].toString()
            << "(" << QLocale().formattedDataSize(fileData[3].toLongLong()) << ")"
            << "- list size:" << fileData.size();
        }
    }

    qDebug() << "Found" << results.size() << "files needing checksum calculation";

    // Extra verification
    if (!results.isEmpty()) {
        qDebug() << "DEBUG: First result in list has" << results[0].size() << "elements";
    }

    return results;
}

void CatalogJobStoppable::extractChecksumsForFiles(const QList<QVariantList> &files)
{
    if (files.isEmpty()) {
        return;
    }

    qDebug() << "=== CatalogJobStoppable::extractChecksumsForFiles START ===";
    qDebug() << "Total files to process:" << files.size();
    qDebug() << "Catalog includeChecksum setting:" << m_device->catalog->includeChecksum;

    const int BATCH_SIZE = 1;
    int totalFiles = files.size();
    int processedFiles = 0;

    // Get the algorithm from catalog setting
    QCryptographicHash::Algorithm algorithm = FileChecksum::getAlgorithmFromString(m_device->catalog->includeChecksum);
    qDebug() << "Using algorithm:" << m_device->catalog->includeChecksum;

    QElapsedTimer totalTimer;
    totalTimer.start();

    // Process in batches
    for (int batchStart = 0; batchStart < totalFiles; batchStart += BATCH_SIZE) {
        if (!shouldContinue()) {
            qDebug() << "Checksum calculation STOPPED by user at file" << processedFiles;
            return;
        }

        waitIfPaused();

        int batchSize = qMin(BATCH_SIZE, totalFiles - batchStart);
        QElapsedTimer batchTimer;
        batchTimer.start();

        qDebug() << "=== Batch" << (batchStart / BATCH_SIZE + 1)
                 << "(" << (batchStart + 1) << "-" << (batchStart + batchSize)
                 << "of" << totalFiles << ") ===";

        // Collect batch data
        QStringList batchFileNames;
        QStringList batchFolderPaths;
        QStringList batchFullPaths;
        QList<qint64> batchFileSizes;

        // Collect batch data with SAFETY CHECKS
        for (int i = batchStart; i < batchStart + batchSize; ++i) {
            const QVariantList &fileData = files[i];

            // SAFETY: Verify we have 4 elements
            if (fileData.size() != 4) {
                qDebug() << "ERROR: File data at index" << i << "has" << fileData.size() << "elements!";
                qDebug() << "  Expected 4 elements. Data:" << fileData;
                continue; // Skip this file
            }

            batchFullPaths << fileData[0].toString();      // file_full_path
            batchFileNames << fileData[1].toString();      // file_name
            batchFolderPaths << fileData[2].toString();    // file_folder_path
            batchFileSizes << fileData[3].toLongLong();    // file_size
        }

        // Calculate checksums for this batch
        QStringList updateFileNames;
        QStringList updateFolderPaths;
        QStringList checksums;

        QString currentFileName;  // Track for status bar
        qint64 currentFileSize = 0;

        for (int i = 0; i < batchFullPaths.size(); ++i) {
            if (!shouldContinue()) break;

            QString filePath = batchFullPaths[i];
            QString fileName = batchFileNames[i];
            qint64 fileSize = batchFileSizes[i];

            // Store for status bar display
            currentFileName = fileName;
            currentFileSize = fileSize;

            qDebug() << "  [" << (processedFiles + i + 1) << "/" << totalFiles << "]"
                     << fileName << "(" << QLocale().formattedDataSize(fileSize) << ")";

            // Check file exists
            QFileInfo fileInfo(filePath);
            if (!fileInfo.exists()) {
                qDebug() << "    SKIP: File no longer exists";
                continue;
            }

            // Calculate checksum
            QString checksum = FileChecksum::calculateChecksum(filePath, algorithm);

            if (!checksum.isEmpty()) {
                qDebug() << "    SUCCESS: Checksum =" << checksum;
                updateFileNames << batchFileNames[i];
                updateFolderPaths << batchFolderPaths[i];
                checksums << checksum;
            } else {
                qDebug() << "    ERROR: Checksum calculation failed";
            }
        }

        // Batch update to database
        if (!updateFileNames.isEmpty()) {
            qDebug() << "  Batch updating" << updateFileNames.size() << "checksums to database...";

            bool success = FileChecksum::batchUpdateFileChecksum(
                m_connectionName,
                m_device->catalog->ID,
                updateFileNames,
                updateFolderPaths,
                checksums
                );

            if (success) {
                qDebug() << "  Batch update SUCCESS";
            } else {
                qDebug() << "  Batch update FAILED";
            }
        }

        int batchDurationMs = batchTimer.elapsed();
        processedFiles += batchSize;

        // Calculate bytes processed in this batch
        qint64 batchBytesProcessed = 0;
        for (int i = 0; i < batchFileSizes.size(); ++i) {
            batchBytesProcessed += batchFileSizes[i];
        }

        static qint64 totalBytesProcessed = 0;  // Track across all batches
        totalBytesProcessed += batchBytesProcessed;

        // Calculate time to completion based on BYTES, not file count
        QString timeToCompletionString;
        if (processedFiles > 0 && processedFiles < totalFiles) {
            qint64 elapsedMs = totalTimer.elapsed();

            // Calculate bytes/second rate
            double bytesPerSecond = static_cast<double>(totalBytesProcessed) / (elapsedMs / 1000.0);

            // Calculate remaining bytes
            qint64 remainingBytes = 0;
            for (int i = processedFiles; i < totalFiles; ++i) {
                const QVariantList &fileData = files[i];
                remainingBytes += fileData[3].toLongLong();
            }

            // Estimate remaining time
            if (bytesPerSecond > 0) {
                qint64 remainingSeconds = static_cast<qint64>(remainingBytes / bytesPerSecond);

                int hours = remainingSeconds / 3600;
                int minutes = (remainingSeconds % 3600) / 60;
                int seconds = remainingSeconds % 60;

                if (hours > 0) {
                    timeToCompletionString = QString("%1h %2m %3s").arg(hours).arg(minutes).arg(seconds);
                } else if (minutes > 0) {
                    timeToCompletionString = QString("%1m %2s").arg(minutes).arg(seconds);
                } else {
                    timeToCompletionString = QString("%1s").arg(seconds);
                }

                qDebug() << "Bytes/second:" << QLocale().formattedDataSize(bytesPerSecond) << "/s";
                qDebug() << "Remaining bytes:" << QLocale().formattedDataSize(remainingBytes);
            }
        }

        qDebug() << "Batch completed in" << batchDurationMs << "ms";
        qDebug() << "Progress:" << processedFiles << "/" << totalFiles;
        if (!timeToCompletionString.isEmpty()) {
            qDebug() << "Estimated time remaining:" << timeToCompletionString;
        }

        // Emit progress with marker for status bar
        // Emit progress with marker INCLUDING current file info
        QString fileInfo = QString("%1 (%2)")
                               .arg(currentFileName)
                               .arg(QLocale().formattedDataSize(currentFileSize));

        QString marker = QString("__CHECKSUM_CALCULATION__|%1|%2|%3|%4")
                             .arg(processedFiles)
                             .arg(totalFiles)
                             .arg(timeToCompletionString)
                             .arg(fileInfo);  // Add file info as 4th part

        qDebug() << "DEBUG: Emitting progress with file info:" << marker;

        emitProgressUpdate(processedFiles, totalFiles, marker);

        QCoreApplication::processEvents();
    }

    // Final completion marker
    QString marker = QString("__CHECKSUM_CALCULATION__|%1|%2").arg(processedFiles).arg(totalFiles);
    emitProgressUpdate(processedFiles, totalFiles, marker);

    qDebug() << "=== CatalogJobStoppable::extractChecksumsForFiles COMPLETE ===";
    qDebug() << "Total files processed:" << processedFiles;
    qDebug() << "Total time:" << (totalTimer.elapsed() / 1000.0) << "seconds";
}

void CatalogJobStoppable::scanDirectoryIntoFiletemp(const QString &directory,
                                                    Catalog *catalog,
                                                    qint64 &processedCount)
{
    if (!shouldContinue()) return;

    // Get file extensions for filtering
    QStringList extensions;
    if (catalog->fileType == "Image") {
        extensions = FileTypeMapping::getExtensionsForCataloging("image");
    } else if (catalog->fileType == "Audio") {
        extensions = FileTypeMapping::getExtensionsForCataloging("audio");
    } else if (catalog->fileType == "Video") {
        extensions = FileTypeMapping::getExtensionsForCataloging("video");
    } else if (catalog->fileType == "Text") {
        extensions = FileTypeMapping::getExtensionsForCataloging("text");
    } else if (catalog->fileType == "Other") {
        extensions = FileTypeMapping::getExtensionsForCataloging("other");
    } else if (catalog->fileType == "None") {
        extensions << "*";
    } else {
        extensions << "*";
    }

    // Setup directory iterator
    QDir::Filters filters = QDir::AllEntries | QDir::NoDotAndDotDot | QDir::Readable;
    if (catalog->includeHidden) {
        filters |= QDir::Hidden;
    }

    QDirIterator::IteratorFlags iteratorFlags = QDirIterator::Subdirectories;
    if (catalog->includeSymblinks) {
        iteratorFlags |= QDirIterator::FollowSymlinks;
    }

    QDirIterator it(directory, extensions, filters, iteratorFlags);

    // Batch arrays for efficient inserts
    QStringList fileNames, fileFolderPaths, fileFullPaths, fileDateTimes, fileCatalogs;
    QStringList fileExtensions, fileTypes, mimeTypes;
    QList<qint64> fileSizes;

    int batchSize = 1000;  // Adjust based on performance testing

    // Prepare insert query
    QSqlQuery insertQuery(QSqlDatabase::database(m_connectionName));
    insertQuery.prepare(R"(
        INSERT INTO filetemp (file_catalog_id, file_name, file_folder_path, file_full_path,
                             file_size, file_date_updated, file_catalog, file_extension, file_type, mime_type)
        VALUES (:catalog_id, :name, :folder_path, :full_path,
                :size, :date, :catalog_name, :extension, :file_type, :mime_type)
    )");

    while (it.hasNext() && shouldContinue()) {
        waitIfPaused();

        QString entryPath = it.next();
        QFileInfo entry(entryPath);

        // Skip excluded folders
        bool isExcluded = false;
        for (const QString &excludedFolder : catalog->excludedFolders) {
            if (entryPath.contains(excludedFolder)) {
                isExcluded = true;
                break;
            }
        }
        if (isExcluded) continue;

        // Only process files (skip directories for now)
        if (entry.isFile()) {
            QString extension = entry.suffix().toLower();
            QString quickFileType = FileMetadata::getFileTypeFromExtension(extension);
            QString quickMimeType = FileMetadata::getMimeTypeFromExtension(extension);

            fileNames << entry.fileName();
            fileFolderPaths << entry.path();
            fileFullPaths << entryPath;
            fileDateTimes << entry.lastModified().toString("yyyy/MM/dd hh:mm:ss");
            fileCatalogs << catalog->name;
            fileSizes << entry.size();
            fileExtensions << extension;
            fileTypes << quickFileType;
            mimeTypes << quickMimeType;

            processedCount++;

            if (processedCount % progressRefreshRate == 0) {
                emitProgressUpdate(processedCount, countedTotalFiles, entryPath);
                QCoreApplication::processEvents();
            }

            // Batch insert when batch is full
            if (fileNames.size() >= batchSize) {
                for (int i = 0; i < fileNames.size(); ++i) {
                    if (!shouldContinue()) break;

                    insertQuery.bindValue(":catalog_id", catalog->ID);
                    insertQuery.bindValue(":name", fileNames[i]);
                    insertQuery.bindValue(":folder_path", fileFolderPaths[i]);
                    insertQuery.bindValue(":full_path", fileFullPaths[i]);
                    insertQuery.bindValue(":size", fileSizes[i]);
                    insertQuery.bindValue(":date", fileDateTimes[i]);
                    insertQuery.bindValue(":catalog_name", fileCatalogs[i]);
                    insertQuery.bindValue(":extension", fileExtensions[i]);
                    insertQuery.bindValue(":file_type", fileTypes[i]);
                    insertQuery.bindValue(":mime_type", mimeTypes[i]);

                    if (!insertQuery.exec()) {
                        qDebug() << "Error inserting into filetemp:" << insertQuery.lastError().text();
                    }
                }

                // Clear arrays for next batch
                fileNames.clear();
                fileFolderPaths.clear();
                fileFullPaths.clear();
                fileDateTimes.clear();
                fileCatalogs.clear();
                fileSizes.clear();
                fileExtensions.clear();
                fileTypes.clear();
                mimeTypes.clear();
            }
        }
    }

    // Insert remaining files
    if (!fileNames.isEmpty() && shouldContinue()) {
        for (int i = 0; i < fileNames.size(); ++i) {
            insertQuery.bindValue(":catalog_id", catalog->ID);
            insertQuery.bindValue(":name", fileNames[i]);
            insertQuery.bindValue(":folder_path", fileFolderPaths[i]);
            insertQuery.bindValue(":full_path", fileFullPaths[i]);
            insertQuery.bindValue(":size", fileSizes[i]);
            insertQuery.bindValue(":date", fileDateTimes[i]);
            insertQuery.bindValue(":catalog_name", fileCatalogs[i]);
            insertQuery.bindValue(":extension", fileExtensions[i]);
            insertQuery.bindValue(":file_type", fileTypes[i]);
            insertQuery.bindValue(":mime_type", mimeTypes[i]);

            if (!insertQuery.exec()) {
                qDebug() << "Error inserting into filetemp:" << insertQuery.lastError().text();
            }
        }
    }
}

QList<QVariantList> CatalogJobStoppable::findNewFiles()
{
    QList<QVariantList> newFiles;

    // ADD DIAGNOSTIC: Check what's in filetemp
    qDebug() << "=== DIAGNOSTIC: findNewFiles() ===";
    qDebug() << "Catalog ID:" << m_device->catalog->ID;

    QSqlQuery diagQuery(QSqlDatabase::database(m_connectionName));
    diagQuery.prepare("SELECT COUNT(*) FROM filetemp WHERE file_catalog_id = :catalog_id");
    diagQuery.bindValue(":catalog_id", m_device->catalog->ID);
    if (diagQuery.exec() && diagQuery.next()) {
        qDebug() << "Files in filetemp for this catalog:" << diagQuery.value(0).toInt();
    }

    diagQuery.prepare("SELECT COUNT(*) FROM file WHERE file_catalog_id = :catalog_id");
    diagQuery.bindValue(":catalog_id", m_device->catalog->ID);
    if (diagQuery.exec() && diagQuery.next()) {
        qDebug() << "Files in file table for this catalog:" << diagQuery.value(0).toInt();
    }

    // Show first few file paths from each table
    diagQuery.prepare("SELECT file_full_path FROM filetemp WHERE file_catalog_id = :catalog_id LIMIT 3");
    diagQuery.bindValue(":catalog_id", m_device->catalog->ID);
    if (diagQuery.exec()) {
        qDebug() << "Sample paths in filetemp:";
        while (diagQuery.next()) {
            qDebug() << "  " << diagQuery.value(0).toString();
        }
    }

    diagQuery.prepare("SELECT file_full_path FROM file WHERE file_catalog_id = :catalog_id LIMIT 3");
    diagQuery.bindValue(":catalog_id", m_device->catalog->ID);
    if (diagQuery.exec()) {
        qDebug() << "Sample paths in file table:";
        while (diagQuery.next()) {
            qDebug() << "  " << diagQuery.value(0).toString();
        }
    }

    // Original query
    QSqlQuery query(QSqlDatabase::database(m_connectionName));
    query.prepare(R"(
        SELECT ft.file_full_path, ft.file_name, ft.file_folder_path,
               ft.file_size, ft.file_extension, ft.file_type, ft.mime_type
        FROM filetemp ft
        LEFT JOIN file f ON f.file_catalog_id = :catalog_id
                        AND f.file_full_path = ft.file_full_path
        WHERE ft.file_catalog_id = :catalog_id
          AND f.file_full_path IS NULL
    )");
    query.bindValue(":catalog_id", m_device->catalog->ID);

    if (!query.exec()) {
        qDebug() << "ERROR finding new files:" << query.lastError().text();
        return newFiles;
    }

    // ADD DIAGNOSTIC: Show what the query returns
    qDebug() << "New files query returned" << query.size() << "rows";

    while (query.next()) {
        QVariantList fileData;
        fileData << query.value(0)  // file_full_path
                 << query.value(1)  // file_name
                 << query.value(2)  // file_folder_path
                 << query.value(3)  // file_size
                 << query.value(4)  // file_extension
                 << query.value(5)  // file_type
                 << query.value(6); // mime_type

        // ADD DIAGNOSTIC: Show each new file found
        //qDebug() << "  New file:" << fileData[0].toString();

        newFiles.append(fileData);
    }

    qDebug() << "=== END DIAGNOSTIC: findNewFiles() found" << newFiles.size() << "new files ===";
    return newFiles;
}

QList<QVariantList> CatalogJobStoppable::findModifiedFiles()
{
    QList<QVariantList> modifiedFiles;

    QSqlQuery query(QSqlDatabase::database(m_connectionName));
    query.prepare(R"(
        SELECT ft.file_full_path, ft.file_name, ft.file_folder_path,
               ft.file_size, ft.file_date_updated, ft.file_extension, ft.file_type
        FROM filetemp ft
        INNER JOIN file f ON f.file_catalog_id = :catalog_id
                         AND f.file_full_path = ft.file_full_path
        WHERE ft.file_catalog_id = :catalog_id
          AND (f.file_size != ft.file_size
               OR f.file_date_updated != ft.file_date_updated)
    )");
    query.bindValue(":catalog_id", m_device->catalog->ID);

    if (!query.exec()) {
        qDebug() << "ERROR finding modified files:" << query.lastError().text();
        return modifiedFiles;
    }

    while (query.next()) {
        QVariantList fileData;
        fileData << query.value(0)  // file_full_path
                 << query.value(1)  // file_name
                 << query.value(2)  // file_folder_path
                 << query.value(3)  // file_size
                 << query.value(4)  // file_date_updated
                 << query.value(5)  // file_extension
                 << query.value(6); // file_type
        modifiedFiles.append(fileData);
    }

    return modifiedFiles;
}

QStringList CatalogJobStoppable::findDeletedFiles()
{
    QStringList deletedFiles;

    QSqlQuery query(QSqlDatabase::database(m_connectionName));
    query.prepare(R"(
        SELECT f.file_full_path
        FROM file f
        LEFT JOIN filetemp ft ON ft.file_catalog_id = :catalog_id
                             AND ft.file_full_path = f.file_full_path
        WHERE f.file_catalog_id = :catalog_id
          AND ft.file_full_path IS NULL
    )");
    query.bindValue(":catalog_id", m_device->catalog->ID);

    if (!query.exec()) {
        qDebug() << "ERROR finding deleted files:" << query.lastError().text();
        return deletedFiles;
    }

    while (query.next()) {
        deletedFiles << query.value(0).toString();
    }

    return deletedFiles;
}

int CatalogJobStoppable::countUnchangedFiles()
{
    QSqlQuery query(QSqlDatabase::database(m_connectionName));
    query.prepare(R"(
        SELECT COUNT(*)
        FROM filetemp ft
        INNER JOIN file f ON f.file_catalog_id = :catalog_id
                         AND f.file_full_path = ft.file_full_path
        WHERE ft.file_catalog_id = :catalog_id
          AND f.file_size = ft.file_size
          AND f.file_date_updated = ft.file_date_updated
    )");
    query.bindValue(":catalog_id", m_device->catalog->ID);

    if (!query.exec() || !query.next()) {
        qDebug() << "ERROR counting unchanged files:" << query.lastError().text();
        return 0;
    }

    return query.value(0).toInt();
}

void CatalogJobStoppable::insertNewFilesFromFiletemp(const QList<QVariantList> &newFiles)
{
    if (newFiles.isEmpty()) return;

    qDebug() << "Inserting" << newFiles.size() << "new files from filetemp";

    // Use a single INSERT SELECT statement for efficiency
    QSqlQuery query(QSqlDatabase::database(m_connectionName));

    // Build list of file paths for WHERE IN clause
    QStringList paths;
    for (const auto &fileData : newFiles) {
        QString escapedPath = fileData[0].toString();
        escapedPath.replace("'", "''");  // SQL escaping
        paths << "'" + escapedPath + "'";
    }

    QString pathList = paths.join(", ");

    QString insertSQL = QString(R"(
        INSERT INTO file (
            file_catalog_id, file_name, file_folder_path, file_full_path,
            file_size, file_date_updated, file_catalog, file_extension, file_type, mime_type
        )
        SELECT
            file_catalog_id, file_name, file_folder_path, file_full_path,
            file_size, file_date_updated, file_catalog, file_extension, file_type, mime_type
        FROM filetemp
        WHERE file_catalog_id = %1
          AND file_full_path IN (%2)
    )").arg(m_device->catalog->ID).arg(pathList);

    if (!query.exec(insertSQL)) {
        qDebug() << "ERROR: Bulk insert of new files failed:" << query.lastError().text();

        // Fallback: Insert one by one
        qDebug() << "Attempting individual inserts...";
        QSqlQuery individualQuery(QSqlDatabase::database(m_connectionName));
        individualQuery.prepare(R"(
            INSERT INTO file (file_catalog_id, file_name, file_folder_path, file_full_path,
                            file_size, file_date_updated, file_catalog, file_extension, file_type, mime_type)
            VALUES (:catalog_id, :name, :folder_path, :full_path,
                    :size, :date, :catalog_name, :extension, :file_type, :mime_type)
        )");

        int successCount = 0;
        for (const auto &fileData : newFiles) {
            if (!shouldContinue()) break;

            individualQuery.bindValue(":catalog_id", m_device->catalog->ID);
            individualQuery.bindValue(":name", fileData[1]);  // file_name
            individualQuery.bindValue(":folder_path", fileData[2]);  // file_folder_path
            individualQuery.bindValue(":full_path", fileData[0]);  // file_full_path
            individualQuery.bindValue(":size", fileData[3]);  // file_size

            // Get date_updated from filetemp
            QSqlQuery dateQuery(QSqlDatabase::database(m_connectionName));
            dateQuery.prepare("SELECT file_date_updated FROM filetemp WHERE file_full_path = ?");
            dateQuery.bindValue(0, fileData[0]);
            QString dateUpdated;
            if (dateQuery.exec() && dateQuery.next()) {
                dateUpdated = dateQuery.value(0).toString();
            }

            individualQuery.bindValue(":date", dateUpdated);
            individualQuery.bindValue(":catalog_name", m_device->catalog->name);
            individualQuery.bindValue(":extension", fileData[4]);  // file_extension
            individualQuery.bindValue(":file_type", fileData[5]);  // file_type
            individualQuery.bindValue(":mime_type", fileData[6]);  // mime_type

            if (individualQuery.exec()) {
                successCount++;
            } else {
                qDebug() << "Failed to insert file:" << fileData[0].toString()
                << "Error:" << individualQuery.lastError().text();
            }
        }
        qDebug() << "Individual insert completed:" << successCount << "of" << newFiles.size() << "files";
    } else {
        qDebug() << "Bulk insert successful:" << query.numRowsAffected() << "rows";
    }

    // Insert folder entries for new files
    QStringList uniqueFolders;
    for (const auto &fileData : newFiles) {
        QString folderPath = fileData[2].toString();  // file_folder_path
        if (!uniqueFolders.contains(folderPath)) {
            uniqueFolders << folderPath;
        }
    }

    if (!uniqueFolders.isEmpty()) {
        insertFolders(uniqueFolders, m_device->catalog);
    }
}

void CatalogJobStoppable::updateModifiedFilesFromFiletemp(const QList<QVariantList> &modifiedFiles)
{
    if (modifiedFiles.isEmpty()) return;

    qDebug() << "Updating" << modifiedFiles.size() << "modified files from filetemp";

    // Build list of file paths for WHERE IN clause
    QStringList paths;
    for (const auto &fileData : modifiedFiles) {
        QString escapedPath = fileData[0].toString();
        escapedPath.replace("'", "''");  // SQL escaping
        paths << "'" + escapedPath + "'";
    }

    QString pathList = paths.join(", ");

    // Bulk update using subqueries
    QString updateSQL = QString(R"(
        UPDATE file
        SET file_size = (SELECT ft.file_size FROM filetemp ft
                         WHERE ft.file_full_path = file.file_full_path AND ft.file_catalog_id = %1),
            file_date_updated = (SELECT ft.file_date_updated FROM filetemp ft
                                 WHERE ft.file_full_path = file.file_full_path AND ft.file_catalog_id = %1),
            file_extension = (SELECT ft.file_extension FROM filetemp ft
                              WHERE ft.file_full_path = file.file_full_path AND ft.file_catalog_id = %1),
            file_type = (SELECT ft.file_type FROM filetemp ft
                         WHERE ft.file_full_path = file.file_full_path AND ft.file_catalog_id = %1),
            mime_type = (SELECT ft.mime_type FROM filetemp ft
                         WHERE ft.file_full_path = file.file_full_path AND ft.file_catalog_id = %1),
            metadata_extraction_date = NULL
        WHERE file_catalog_id = %1
          AND file_full_path IN (%2)
    )").arg(m_device->catalog->ID).arg(pathList);

    QSqlQuery query(QSqlDatabase::database(m_connectionName));
    if (!query.exec(updateSQL)) {
        qDebug() << "ERROR: Bulk update of modified files failed:" << query.lastError().text();

        // Fallback: Update one by one
        qDebug() << "Attempting individual updates...";
        QSqlQuery individualQuery(QSqlDatabase::database(m_connectionName));
        individualQuery.prepare(R"(
            UPDATE file
            SET file_size = :size,
                file_date_updated = :date,
                file_extension = :extension,
                file_type = :file_type,
                mime_type = :mime_type,
                metadata_extraction_date = NULL
            WHERE file_catalog_id = :catalog_id
              AND file_full_path = :full_path
        )");

        int successCount = 0;
        for (const auto &fileData : modifiedFiles) {
            if (!shouldContinue()) break;
            individualQuery.bindValue(":size", fileData[3]);  // file_size
            individualQuery.bindValue(":date", fileData[4]);  // file_date_updated
            individualQuery.bindValue(":extension", fileData[5]);  // file_extension
            individualQuery.bindValue(":file_type", fileData[6]);  // file_type
            individualQuery.bindValue(":catalog_id", m_device->catalog->ID);
            individualQuery.bindValue(":full_path", fileData[0]);  // file_full_path
            individualQuery.bindValue(":mime_type", fileData[7]);  // mime_type

            if (individualQuery.exec()) {
                successCount++;
            } else {
                qDebug() << "Failed to update file:" << fileData[0].toString()
                << "Error:" << individualQuery.lastError().text();
            }
        }
        qDebug() << "Individual update completed:" << successCount << "of" << modifiedFiles.size() << "files";
    } else {
        qDebug() << "Bulk update successful:" << query.numRowsAffected() << "rows";
    }
}

void CatalogJobStoppable::deleteRemovedFiles(const QStringList &deletedFiles)
{
    if (deletedFiles.isEmpty()) return;

    qDebug() << "Deleting" << deletedFiles.size() << "removed files";

    // Build list of file paths for WHERE IN clause
    QStringList paths;
    for (const QString &path : deletedFiles) {
        QString escapedPath = path;
        escapedPath.replace("'", "''");  // SQL escaping
        paths << "'" + escapedPath + "'";
    }

    QString pathList = paths.join(", ");

    QString deleteSQL = QString(R"(
        DELETE FROM file
        WHERE file_catalog_id = %1
          AND file_full_path IN (%2)
    )").arg(m_device->catalog->ID).arg(pathList);

    QSqlQuery query(QSqlDatabase::database(m_connectionName));
    if (!query.exec(deleteSQL)) {
        qDebug() << "ERROR: Bulk delete of removed files failed:" << query.lastError().text();
    } else {
        qDebug() << "Bulk delete successful:" << query.numRowsAffected() << "rows";
    }
}

void CatalogJobStoppable::extractMetadataForChangedFiles(const QList<QVariantList> &changedFiles)
{
    if (changedFiles.isEmpty()) return;

    qDebug() << "=== Starting metadata extraction for" << changedFiles.size() << "files ===";

    // Collect files that need metadata extraction
    QStringList extractFileNames, extractFolderPaths, extractFullPaths;

    for (const auto &fileData : changedFiles) {
        QString filePath = fileData[0].toString();  // file_full_path

        if (FileMetadata::isMetadataSupported(filePath)) {
            QFileInfo fileInfo(filePath);
            extractFileNames << fileInfo.fileName();
            extractFolderPaths << fileInfo.absolutePath();
            extractFullPaths << filePath;
        }
    }

    if (extractFullPaths.isEmpty()) {
        qDebug() << "No files require metadata extraction";
        return;
    }

    int totalFiles = extractFullPaths.size();
    int processedFiles = 0;

    qDebug() << "Total files needing metadata extraction:" << totalFiles;

    // ETA tracking
    QElapsedTimer totalTimer;
    totalTimer.start();

    // Determine thread count based on system and database mode
    int optimalThreads = 4;  // Safe default
    int availableCores = QThread::idealThreadCount();
    if (availableCores > 0) {
        optimalThreads = qMin(availableCores - 1, 8);
        if (m_databaseMode != "Memory") {
            optimalThreads = qMin(optimalThreads, 4);
        }
    }

    qDebug() << "Using" << optimalThreads << "threads for parallel extraction";

    // Process in batches of files (progressRefreshRate)
    for (int batchStart = 0; batchStart < totalFiles; batchStart += progressRefreshRate) {
        // Check if stop requested between batches
        if (!shouldContinue()) {
            qDebug() << "Metadata extraction stopped by user after" << processedFiles << "files";
            QString stopMsg = QString("Metadata extraction stopped: %1/%2 files processed")
                                  .arg(processedFiles)
                                  .arg(totalFiles);
            emitProgressUpdate(processedFiles, totalFiles, stopMsg);
            return;
        }

        waitIfPaused();

        int batchEnd = qMin(batchStart + progressRefreshRate, totalFiles);
        int batchSize = batchEnd - batchStart;

        qDebug() << "Processing batch:" << batchStart << "to" << batchEnd << "(" << batchSize << "files)";

        // Extract batch slice
        QStringList batchFullPaths = extractFullPaths.mid(batchStart, batchSize);
        QStringList batchFileNames = extractFileNames.mid(batchStart, batchSize);
        QStringList batchFolderPaths = extractFolderPaths.mid(batchStart, batchSize);

        QElapsedTimer batchTimer;
        batchTimer.start();

        // Parallel extraction for this batch
        ParallelMetadataExtractor extractor;
        auto results = extractor.extractBatch(batchFullPaths, batchFileNames, batchFolderPaths,
                                              m_device->catalog->includeMetadata, optimalThreads);

        // Collect results
        QStringList updateFileNames, updateFolderPaths;
        QList<QVariantMap> updateMetadata;

        const auto& constResults = results;
        for (const auto &result : constResults) {
            if (!result.metadata.isEmpty()) {
                updateFileNames << result.fileName;
                updateFolderPaths << result.folderPath;
                updateMetadata << result.metadata;
            }
        }

        // Batch update to database
        if (!updateFileNames.isEmpty()) {
            FileMetadata::batchUpdateFileMetadata(m_connectionName, m_device->catalog->ID,
                                                  updateFileNames, updateFolderPaths,
                                                  updateMetadata);
        }

        int batchDurationMs = batchTimer.elapsed();
        processedFiles += batchSize;

        // Calculate time to completion
        QString timeToCompletionString;
        if (processedFiles > 0 && processedFiles < totalFiles) {
            qint64 elapsedMs = totalTimer.elapsed();
            double avgMsPerFile = static_cast<double>(elapsedMs) / processedFiles;
            int remainingFiles = totalFiles - processedFiles;
            qint64 remainingMs = static_cast<qint64>(avgMsPerFile * remainingFiles);

            int totalSeconds = remainingMs / 1000;
            int hours = totalSeconds / 3600;
            int minutes = (totalSeconds % 3600) / 60;
            int seconds = totalSeconds % 60;

            if (hours > 0) {
                timeToCompletionString = QString("%1h %2m %3s").arg(hours).arg(minutes).arg(seconds);
            } else if (minutes > 0) {
                timeToCompletionString = QString("%1m %2s").arg(minutes).arg(seconds);
            } else {
                timeToCompletionString = QString("%1s").arg(seconds);
            }
        }

        // Emit progress update with marker and time to completion
        QString marker = QString("__METADATA_EXTRACTION__|%1|%2|%3")
                             .arg(processedFiles)
                             .arg(totalFiles)
                             .arg(timeToCompletionString);
        emitProgressUpdate(processedFiles, totalFiles, marker);

        // Log batch performance
        qDebug() << "Batch completed:" << batchSize << "files in" << batchDurationMs << "ms"
                 << "(" << (batchDurationMs / qMax(1, batchSize)) << "ms/file)"
                 << "| Total progress:" << processedFiles << "/" << totalFiles
                 << "(" << (processedFiles * 100) / totalFiles << "%)"
                 << "| Time to completion:" << timeToCompletionString;

        // Allow UI to process events between batches
        QCoreApplication::processEvents();
    }

    // Final completion message with metadata extraction marker
    QString marker = QString("__METADATA_EXTRACTION__|%1|%2").arg(processedFiles).arg(totalFiles);
    emitProgressUpdate(processedFiles, totalFiles, marker);

    qDebug() << "=== Metadata extraction completed: %1 files processed ===" << processedFiles;

    //qDebug() << "=== Metadata extraction completed ===" << finalMsg;
}

void CatalogJobStoppable::migrateMimeTypesForExistingFiles()
{
    qDebug() << "=== Starting MIME type migration (by extension) ===";

    // Use the unified method with progress callback
    FileMetadata::migrateFileTypesForCatalog(
        m_connectionName,
        m_device->catalog->ID,
        // Progress callback
        [this](int processed, int total, QString message) {
            // Set marker for file type migration state
            QString marker = QString("__FILETYPE_MIGRATION__|%1|%2").arg(processed).arg(total);
            emitProgressUpdate(processed, total, marker);
            QCoreApplication::processEvents();
        },
        // Should continue callback
        [this]() -> bool {
            return shouldContinue();
        }
        );

    qDebug() << "=== MIME type migration completed ===";
}
