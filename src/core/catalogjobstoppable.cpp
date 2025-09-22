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
        setProgressRefreshRate(10);  // More frequent updates for slower metadata operations
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
    // Right before the batch size decision:
    qDebug() << "CatalogJobStoppable::processDirectoryWithProgress, Catalog includeMetadata value:" << catalog->includeMetadata
             << "NONE:" << Catalog::METADATA_NONE
             << "BASIC:" << Catalog::METADATA_MEDIA_BASIC
             << "Comparison result:" << (catalog->includeMetadata == Catalog::METADATA_MEDIA_BASIC);

    if (!shouldContinue()) return;

    qDebug() << "Processing directory:" << directory;

    // Get file extensions for filtering (for files only)
    QStringList extensions;
    if (catalog->fileType == "Image") {
        extensions << "*.png" << "*.jpg" << "*.jpeg" << "*.gif" << "*.xcf" << "*.tif" << "*.tiff" << "*.bmp";
    } else if (catalog->fileType == "Audio") {
        extensions << "*.mp3" << "*.wav" << "*.ogg" << "*.aif" << "*.aiff" << "*.flac";
    } else if (catalog->fileType == "Video") {
        extensions << "*.wmv" << "*.avi" << "*.mp4" << "*.mkv" << "*.flv" << "*.webm" << "*.m4v" << "*.vob" << "*.ogv" << "*.mov";
    } else if (catalog->fileType == "Text") {
        extensions << "*.txt" << "*.pdf" << "*.odt" << "*.idx" << "*.html" << "*.rtf" << "*.doc" << "*.docx" << "*.epub";
    } else if (catalog->fileType == "None") {
        extensions << "*";
    } else {
        // Default: include all files
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

    // Database batching arrays - separate arrays for folder paths and full paths
    QStringList fileNames, fileFolderPaths, fileFullPaths, fileDateTimes, fileCatalogs;
    QStringList fileExtensions;
    QStringList fileTypes;
    QList<qint64> fileSizes;
    QStringList directoryPaths; // NEW: Track directories separately

    int batchSize;
    if (!catalog->includeMetadata.isEmpty() &&
        catalog->includeMetadata != Catalog::METADATA_NONE) {
        batchSize = 10;  // Smaller batches for metadata-enabled catalogs
        qDebug() << "Using small batch size for metadata extraction:" << batchSize;
    } else {
        batchSize = 1000; // Standard large batches for fast operations
        qDebug() << "Using standard batch size:" << batchSize;
    }

    int batchCount = 0;   // Database batch counter

    // Progress tracking (separate from database batching)
    qint64 filesProcessedInThisCall = 0;

    // Emit the transition message once
    emitProgressUpdate(0, countedTotalFiles, "Processing files...");

    while (it.hasNext() && shouldContinue()) {
        waitIfPaused();

        QString entryPath = it.next();
        QFileInfo entry(entryPath);

        // Skip excluded folders (same logic as original)
        bool isExcluded = false;
        for (const QString &excludedFolder : catalog->excludedFolders) {
            if (entryPath.contains(excludedFolder)) {
                isExcluded = true;
                break;
            }
        }
        if (isExcluded) continue;

        // RESTORE ORIGINAL LOGIC: Handle directories and files separately
        if (entry.isDir()) {
            // Insert directories (including empty ones) - RESTORED FROM v2.6
            directoryPaths << entryPath;
        }
        else if (entry.isFile()) {
            // Special handling for "None" type - manually filter extensionless files
            if (catalog->fileType == "None") {
                QString extension = entry.suffix();
                if (!extension.isEmpty()) {
                    continue; // Skip files that have extensions
                }
            }

            // Get file extension and quick type
            QString extension = entry.suffix().toLower();
            QString quickFileType = FileMetadata::getFileTypeFromExtension(extension);

            // Process file - correctly separate folder path from full path
            fileNames << entry.fileName();
            fileFolderPaths << entry.path();
            fileFullPaths << entryPath;
            fileDateTimes << entry.lastModified().toString("yyyy/MM/dd hh:mm:ss");
            fileCatalogs << catalog->name;
            fileSizes << entry.size();
            fileExtensions << extension;     // Store extension
            fileTypes << quickFileType;      // Store type based on extension ONLY

            // Update counters
            processedCount++;
            filesProcessedInThisCall++;
            batchCount++;

            // Progress updates
            if (filesProcessedInThisCall % progressRefreshRate == 0) {
                emitProgressUpdate(processedCount, countedTotalFiles, entry.absoluteFilePath());
                QCoreApplication::processEvents();
            }
        }

        // Database batching (when we have enough files OR directories)
        if ((batchCount >= batchSize && !fileNames.isEmpty()) || !directoryPaths.isEmpty() && directoryPaths.size() >= 100) {
            // Insert files if we have any
            if (!fileNames.isEmpty()) {
                QSqlQuery query(QSqlDatabase::database(m_connectionName));
                query.prepare(R"(
                INSERT INTO file (file_catalog_id, file_name, file_folder_path, file_full_path,
                                file_size, file_date_updated, file_catalog,
                                file_extension, file_type)
                VALUES (:catalog_id, :name, :folder_path, :full_path,
                        :size, :date, :catalog_name,
                        :extension, :file_type)
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

                    if (!query.exec()) {
                        qDebug() << "Database insert error:" << query.lastError().text();
                    }
                }

                // Metadata extraction - ONLY for media files with metadata enabled
                if (catalog->includeMetadata != Catalog::METADATA_NONE) {
                    for (int i = 0; i < fileFullPaths.size(); ++i) {
                        if (!shouldContinue()) break;

                        const QString &filePath = fileFullPaths[i];
                        if (FileMetadata::isMetadataSupported(filePath)) {
                            FileMetadata::extractAndStore(filePath, m_connectionName,
                                                          catalog->ID, catalog->includeMetadata);
                        }
                    }
                }

                // Insert folders from files
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

                // Clear file batch arrays
                fileNames.clear();
                fileFolderPaths.clear();
                fileFullPaths.clear();
                fileDateTimes.clear();
                fileCatalogs.clear();
                fileSizes.clear();
                fileExtensions.clear();
                fileTypes.clear();
                batchCount = 0;
            }

            // Insert directories (including empty ones) - RESTORED FUNCTIONALITY
            if (!directoryPaths.isEmpty()) {
                QSqlQuery folderQuery(QSqlDatabase::database(m_connectionName));
                folderQuery.prepare(R"(
                    INSERT OR IGNORE INTO folder (folder_catalog_id, folder_path)
                    VALUES (:catalog_id, :path)
                )");

                for (const QString &dirPath : directoryPaths) {
                    if (!shouldContinue()) break;

                    folderQuery.bindValue(":catalog_id", catalog->ID);
                    folderQuery.bindValue(":path", dirPath);

                    if (!folderQuery.exec()) {
                        qDebug() << "Directory insert error:" << folderQuery.lastError().text();
                    }
                }

                directoryPaths.clear();
            }
        }
    }

    // Process remaining files and directories in final batch
    if (!fileNames.isEmpty() && shouldContinue()) {
        QSqlQuery query(QSqlDatabase::database(m_connectionName));
        query.prepare(R"(
            INSERT INTO file (file_catalog_id, file_name, file_folder_path, file_full_path, file_size, file_date_updated, file_catalog)
            VALUES (:catalog_id, :name, :folder_path, :full_path, :size, :date, :catalog_name)
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

            if (!query.exec()) {
                qDebug() << "Database insert error:" << query.lastError().text();
            }
        }

        if (catalog->includeMetadata != Catalog::METADATA_NONE) {
            for (int i = 0; i < fileFullPaths.size(); ++i) {
                if (!shouldContinue()) break;

                const QString &filePath = fileFullPaths[i];
                if (FileMetadata::isMetadataSupported(filePath)) {
                    FileMetadata::extractAndStore(filePath, m_connectionName, catalog->ID, catalog->includeMetadata);
                }
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

    // Process remaining directories in final batch
    if (!directoryPaths.isEmpty() && shouldContinue()) {
        QSqlQuery folderQuery(QSqlDatabase::database(m_connectionName));
        folderQuery.prepare(R"(
            INSERT OR IGNORE INTO folder (folder_catalog_id, folder_path)
            VALUES (:catalog_id, :path)
        )");

        for (const QString &dirPath : directoryPaths) {
            if (!shouldContinue()) break;

            folderQuery.bindValue(":catalog_id", catalog->ID);
            folderQuery.bindValue(":path", dirPath);

            if (!folderQuery.exec()) {
                qDebug() << "Directory insert error:" << folderQuery.lastError().text();
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

    emitProgressUpdate(totalFiles, totalFiles, QString("Found %1 files.").arg(totalFiles));
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
            int percentComplete = (processedFiles * 100) / totalFiles;
            QString progressMsg = QString("Extracting metadata: %1/%2 (%3%)")
                                      .arg(processedFiles)
                                      .arg(totalFiles)
                                      .arg(percentComplete);

            emitProgressUpdate(processedFiles, totalFiles, progressMsg);
            QCoreApplication::processEvents();
        }
    }

    // Final progress update
    QString finalMsg = QString("Metadata extraction completed: %1 files processed")
                           .arg(processedFiles);
    emitProgressUpdate(processedFiles, totalFiles, finalMsg);

    qDebug() << "=== Metadata extraction completed ===" << finalMsg;
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
