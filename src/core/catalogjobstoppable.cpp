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

void CatalogJobStoppable::processCatalog(Device *selectedDevice)
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
            createCatalogWithProgress(m_device);
        } else {
            updateCatalogWithProgress(m_device);
        }

        if (shouldContinue()) {
            qDebug() << "=== Catalog operation completed successfully ===";
            emit catalogOperationFinished();
        }

    } catch (const std::exception &e) {
        qDebug() << "=== EXCEPTION in processCatalog():" << e.what() << "===";
        emit catalogOperationError(QString("Catalog operation failed: %1").arg(e.what()));
    } catch (...) {
        qDebug() << "=== UNKNOWN EXCEPTION in processCatalog() ===";
        emit catalogOperationError("Unknown error during catalog operation");
    }

    qDebug() << "=== CatalogJobStoppable::processCatalog() END ===";
}

void CatalogJobStoppable::createCatalogWithProgress(Device *device)
{
    qDebug() << "=== Creating catalog with progress ===";

    if (!device || !device->catalog) {
        throw std::runtime_error("Invalid device or catalog");
    }

    Catalog *catalog = device->catalog;

    // Validate source path
    QDir sourceDir(catalog->sourcePath);
    if (!sourceDir.exists()) {
        throw std::runtime_error("Source directory does not exist: " + catalog->sourcePath.toStdString());
    }

    // Check if directory is empty
    if (sourceDir.entryInfoList(QDir::NoDotAndDotDot | QDir::AllEntries).count() == 0) {
        throw std::runtime_error("Source directory is empty: " + catalog->sourcePath.toStdString());
    }

    // Get file extensions to scan for and load excluded folders
    catalog->getFileExtensions();
    catalog->loadExcludedFolders();

    // Estimate total files for progress calculation
    qDebug() << "Estimating total files...";
    emitProgressUpdate(0, 0, "Estimating total files...");
    estimatedTotalFiles = countTotalFiles(catalog->sourcePath, catalog);
    qDebug() << "Estimated total files:" << estimatedTotalFiles;

    if (!shouldContinue()) return;

    // Initialize database transaction for efficiency
    QSqlDatabase db = QSqlDatabase::database(m_connectionName);
    if (!db.exec("BEGIN TRANSACTION").isActive()) {
        qDebug() << "Warning: Could not start transaction:" << db.lastError().text();
    }

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

    // Handle Memory mode file saving
    if (m_databaseMode == "Memory") {
        emitProgressUpdate(processedCount, estimatedTotalFiles, "Saving catalog files...");

        // Save catalog to .idx file
        if (!catalog->saveCatalogToFile(m_databaseMode, m_collectionFolder)) {
            qDebug() << "Warning: Failed to save catalog to file";
        }

        // Save folders to .folders.idx file
        if (!catalog->saveFoldersToFile(m_databaseMode, m_collectionFolder)) {
            qDebug() << "Warning: Failed to save folders to file";
        }
    }

    // Final progress update
    emitProgressUpdate(processedCount, estimatedTotalFiles, "Catalog creation completed");
    qDebug() << "Catalog creation completed. Files processed:" << processedCount;
}

void CatalogJobStoppable::updateCatalogWithProgress(Device *device)
{
    qDebug() << "=== Updating catalog with progress ===";

    if (!device || !device->catalog) {
        throw std::runtime_error("Invalid device or catalog");
    }

    emitProgressUpdate(0, 0, "Starting catalog update...");

    // Use the existing updateCatalogFiles method but with progress monitoring
    QList<qint64> updateResults = device->catalog->updateCatalogFiles(m_databaseMode, m_collectionFolder, false);

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

    QStringList fileNames, filePaths, fileDateTimes, fileCatalogs;
    QList<qint64> fileSizes;

    int batchSize = 1000; // Process files in batches for efficiency
    int batchCount = 0;

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

        // Collect file information
        fileNames.append(fileInfo.fileName());
        filePaths.append(fileInfo.absolutePath());
        fileSizes.append(fileInfo.size());
        fileDateTimes.append(fileInfo.lastModified().toString("yyyy-MM-dd hh:mm:ss"));
        fileCatalogs.append(catalog->name);

        processedCount++;
        batchCount++;

        // Process batch when full or emit progress periodically
        if (batchCount >= batchSize) {
            // Insert batch into database
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

                // Insert folder entries into folder table
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

                // Clear batch
                fileNames.clear();
                filePaths.clear();
                fileSizes.clear();
                fileDateTimes.clear();
                fileCatalogs.clear();
                batchCount = 0;
            }

            // Emit progress update
            if (processedCount % progressRefreshRate == 0 || processedCount == estimatedTotalFiles) {
                emitProgressUpdate(processedCount, estimatedTotalFiles, fileInfo.absoluteFilePath());
            }
        }
    }

    // Process remaining files in last batch
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
    }
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
    if (!m_objectValid.loadAcquire()) return;

    filesProcessed = processed;
    emit catalogProgress(processed, total, currentPath);
}
