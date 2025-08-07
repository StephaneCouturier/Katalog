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
// File Name:   catalogjob.cpp
// Purpose:     KJob implementation for catalog operations
// Description: Handles catalog creation and update operations with progress reporting
// Author:      Stephane Couturier
/////////////////////////////////////////////////////////////////////////////
*/

#include "catalogjob.h"
#include <QDebug>
#include <QTimer>
#include <QCoreApplication>
#include <QDir>
#include <QDirIterator>
#include <qsqlerror.h>

CatalogJob::CatalogJob(Device *device,
                       OperationType operationType,
                       const QString &databaseMode,
                       const QString &collectionFolder,
                       QObject *parent)
    : KJob(parent)
    , m_device(device)
    , m_operationType(operationType)
    , m_databaseMode(databaseMode)
    , m_collectionFolder(collectionFolder)
    , m_progressTimer(new QTimer(this))
{
    setCapabilities(KJob::Killable); // Now supports cancellation!

    // Setup progress timer for periodic updates
    m_progressTimer->setInterval(500); // Update every 500ms, similar to SearchJob
    connect(m_progressTimer, &QTimer::timeout, this, &CatalogJob::updateJobProgress);
}

CatalogJob::~CatalogJob()
{
    if (m_progressTimer) {
        m_progressTimer->stop();
    }
}

void CatalogJob::start()
{
    if (m_operationStarted) {
        qDebug() << "CatalogJob::start() called but operation already started";
        return;
    }

    if (!m_device || !m_device->catalog) {
        setError(KJob::UserDefinedError);
        setErrorText("No device or catalog provided for operation");
        emitResult();
        return;
    }

    qDebug() << "CatalogJob::start() - Starting" << (m_operationType == CreateCatalog ? "create" : "update") << "operation for catalog:" << m_device->catalog->name;

    m_operationStarted = true;

    // Set initial status message
    QString operationName = (m_operationType == CreateCatalog) ? "Creating" : "Updating";
    emit infoMessage(this, QString("%1 catalog %2...").arg(operationName, m_device->catalog->name));

    // Start progress timer
    m_progressTimer->start();

    // Execute work directly (no separate thread for now - can be added later if needed)
    QTimer::singleShot(0, this, [this]() {
        doWork();
    });
}

void CatalogJob::doWork()
{
    qDebug() << "CatalogJob::doWork() - Starting work in thread";

    try {
        // CHECK FOR STOP REQUEST AT START
        if (m_stopRequested) {
            qDebug() << "CatalogJob::doWork() - Stop was requested, aborting";
            return; // emitResult() already called in doKill()
        }

        if (m_operationType == CreateCatalog) {
            emit infoMessage(this, QString("Creating catalog %1...").arg(m_device->catalog->name));
            createCatalogWithProgress();
        } else {
            emit infoMessage(this, QString("Updating catalog %1...").arg(m_device->catalog->name));

            // For update operations, we need to check stop request periodically
            // since updateCatalogFiles() might be a long-running operation
            QList<qint64> updateResults = m_device->catalog->updateCatalogFiles(m_databaseMode, m_collectionFolder, false);

            // CHECK FOR STOP REQUEST AFTER UPDATE
            if (m_stopRequested) {
                qDebug() << "CatalogJob::doWork() - Stop requested during update";
                return; // emitResult() already called in doKill()
            }

            // Process results...
            if (!updateResults.isEmpty() && updateResults[0] == 1) {
                // Success handling code...
                qint64 newFileCount = updateResults[1];
                qint64 newTotalSize = updateResults[3];

                {
                    QMutexLocker locker(&m_progressMutex);
                    m_filesProcessed = newFileCount;
                    m_totalFiles = newFileCount;
                    m_currentPath = QString("Update completed");
                }

                emit infoMessage(this, QString("Catalog updated successfully. Files: %1, Size: %2 bytes")
                                           .arg(newFileCount).arg(newTotalSize));
            } else {
                // Handle error cases...
                QString errorMsg = "Catalog update failed";
                if (!updateResults.isEmpty()) {
                    switch (updateResults[0]) {
                    case -1: errorMsg = "Catalog path not accessible"; break;
                    case -2: errorMsg = "Catalog path is empty"; break;
                    case -3: errorMsg = "Old catalog format - needs migration"; break;
                    case -4: errorMsg = "Missing catalog information"; break;
                    default: errorMsg = "Unknown update error"; break;
                    }
                }
                emit onWorkError(errorMsg);
                return;
            }
        }

        // Final check before completion
        if (m_stopRequested) {
            qDebug() << "CatalogJob::doWork() - Stop requested before completion";
            return; // emitResult() already called in doKill()
        }

        // Operation completed successfully
        qDebug() << "=== Catalog operation completed successfully ===";
        emit infoMessage(this, QString("Catalog operation completed successfully"));

        m_progressTimer->stop();
        setPercent(100);
        emitResult(); // Signal successful completion

    } catch (const std::exception& e) {
        qDebug() << "=== EXCEPTION in doWork():" << e.what() << "===";
        setError(KJob::UserDefinedError);
        setErrorText(QString("Exception during catalog operation: %1").arg(e.what()));
        emitResult();
        return;
    } catch (...) {
        qDebug() << "=== UNKNOWN EXCEPTION in doWork() ===";
        setError(KJob::UserDefinedError);
        setErrorText("Unknown exception during catalog operation");
        emitResult();
        return;
    }
}

void CatalogJob::createCatalogWithProgress()
{
    // This method creates the catalog with complete progress reporting
    // We implement our own file processing instead of calling catalogDirectory()

    emit infoMessage(this, QString("Processing files in %1...").arg(m_device->catalog->sourcePath));

    // Check if directory exists and is not empty
    QDir dir(m_device->catalog->sourcePath);
    if (!dir.exists()) {
        throw std::runtime_error("Source directory does not exist");
    }

    if (dir.entryInfoList(QDir::NoDotAndDotDot|QDir::AllEntries).count() == 0) {
        throw std::runtime_error("Source directory is empty");
    }

    // Get file extensions to scan for
    m_device->catalog->getFileExtensions();

    // Use the same iterator logic as catalogDirectory but with our own processing
    QStringList fileExtensions;
    if (m_device->catalog->fileType == "Image") {
        fileExtensions << "*.png" << "*.jpg" << "*.gif" << "*.xcf" << "*.tif" << "*.bmp";
    } else if (m_device->catalog->fileType == "Audio") {
        fileExtensions << "*.mp3" << "*.wav" << "*.ogg" << "*.aif";
    } else if (m_device->catalog->fileType == "Video") {
        fileExtensions << "*.wmv" << "*.avi" << "*.mp4" << "*.mkv" << "*.flv" << "*.webm" << "*.m4v" << "*.vob" << "*.ogv" << "*.mov";
    } else if (m_device->catalog->fileType == "Text") {
        fileExtensions << "*.txt" << "*.pdf" << "*.odt" << "*.idx" << "*.html" << "*.rtf" << "*.doc" << "*.docx" << "*.epub";
    } else {
        // "All" - no filter
        fileExtensions.clear();
    }

    // *** STEP 1: Count total files first for accurate progress ***
    emit infoMessage(this, QString("Counting files..."));

    // Load excluded folders first
    m_device->catalog->loadExcludedFolders();

    QDirIterator::IteratorFlags flags = QDirIterator::Subdirectories;
    if (m_device->catalog->includeHidden) {
        flags |= QDirIterator::FollowSymlinks;
    }

    // Create iterator for counting
    QDirIterator* countIterator;
    if (fileExtensions.isEmpty()) {
        countIterator = new QDirIterator(m_device->catalog->sourcePath,
                                         QDir::Files | QDir::NoDotAndDotDot | (m_device->catalog->includeHidden ? QDir::Hidden : QDir::NoFilter),
                                         flags);
    } else {
        countIterator = new QDirIterator(m_device->catalog->sourcePath,
                                         fileExtensions,
                                         QDir::Files | QDir::NoDotAndDotDot | (m_device->catalog->includeHidden ? QDir::Hidden : QDir::NoFilter),
                                         flags);
    }

    qint64 totalFiles = 0;
    while (countIterator->hasNext()) {
        // Check for stop request
        if (m_stopRequested) {
            delete countIterator;
            qDebug() << "Stop requested during file counting";
            return;
        }

        QString entryPath = countIterator->next();

        // Check for excluded directories
        bool exclude = false;
        for(const QString& excludedFolder : m_device->catalog->excludedFolders) {
            if(entryPath.contains(excludedFolder)) {
                exclude = true;
                break;
            }
        }

        if (!exclude) {
            totalFiles++;
        }
    }
    delete countIterator;

    // Set total files for progress calculation
    {
        QMutexLocker locker(&m_progressMutex);
        m_totalFiles = totalFiles;
        m_filesProcessed = 0;
    }

    emit infoMessage(this, QString("Found %1 files. Processing...").arg(totalFiles));
    emit filesProcessedUpdate(0, totalFiles, QString("Starting file processing"));

    // *** STEP 2: Setup database transaction and queries ***
    QSqlQuery beginQuery(QSqlDatabase::database("defaultConnection"));
    beginQuery.prepare("BEGIN");
    if (!beginQuery.exec()) {
        qDebug() << "ERROR: Failed to begin transaction:" << beginQuery.lastError().text();
        throw std::runtime_error(QString("Failed to begin transaction: %1").arg(beginQuery.lastError().text()).toStdString());
    }
    qDebug() << "Transaction started successfully";

    QSqlQuery insertFileQuery(QSqlDatabase::database("defaultConnection"));
    QString insertFileSQL = R"(
        INSERT INTO file(
            file_catalog_id,
            file_name,
            file_size,
            file_folder_path,
            file_date_updated,
            file_catalog,
            file_full_path
        ) VALUES(
            :file_catalog_id,
            :file_name,
            :file_size,
            :file_folder_path,
            :file_date_updated,
            :file_catalog,
            :file_full_path
        )
    )";
    insertFileQuery.prepare(insertFileSQL);

    QSqlQuery insertFolderQuery(QSqlDatabase::database("defaultConnection"));
    QString insertFolderSQL = R"(
        INSERT OR IGNORE INTO folder(
            folder_catalog_id,
            folder_path
        ) VALUES(
            :folder_catalog_id,
            :folder_path
        )
    )";
    insertFolderQuery.prepare(insertFolderSQL);

    // Insert root folder
    insertFolderQuery.bindValue(":folder_catalog_id", m_device->catalog->ID);
    insertFolderQuery.bindValue(":folder_path", m_device->catalog->sourcePath);
    insertFolderQuery.exec();

    // *** STEP 3: Process files with progress reporting ***
    //QSqlQuery beginQuery(QSqlDatabase::database("defaultConnection"));
    beginQuery.prepare("BEGIN");
    beginQuery.exec();

    // Create iterator for processing (with both files and directories)
    QDirIterator* processIterator;
    if (fileExtensions.isEmpty()) {
        processIterator = new QDirIterator(m_device->catalog->sourcePath,
                                           QDir::AllEntries | QDir::NoDotAndDotDot | (m_device->catalog->includeHidden ? QDir::Hidden : QDir::NoFilter),
                                           flags);
    } else {
        processIterator = new QDirIterator(m_device->catalog->sourcePath,
                                           fileExtensions,
                                           QDir::AllEntries | QDir::NoDotAndDotDot | (m_device->catalog->includeHidden ? QDir::Hidden : QDir::NoFilter),
                                           flags);
    }

    qint64 filesProcessed = 0;
    bool transactionRollback = false;

    // Process files with progress reporting and stop checking
    while (processIterator->hasNext()) {
        // *** CHECK FOR STOP REQUEST - CRITICAL ***
        if (m_stopRequested) {
            qDebug() << "Stop requested during file processing - rolling back transaction";
            transactionRollback = true;
            break;
        }

        QString entryPath = processIterator->next();
        QFileInfo entry(entryPath);

        // Check for excluded directories
        bool exclude = false;
        for(const QString& excludedFolder : m_device->catalog->excludedFolders) {
            if(entryPath.contains(excludedFolder)) {
                exclude = true;
                break;
            }
        }

        if (!exclude) {
            if (entry.isDir()) {
                // Insert directory
                insertFolderQuery.bindValue(":folder_catalog_id", m_device->catalog->ID);
                insertFolderQuery.bindValue(":folder_path", entryPath);
                insertFolderQuery.exec();
            } else if (entry.isFile()) {
                // Insert file
                insertFileQuery.bindValue(":file_catalog_id", m_device->catalog->ID);
                insertFileQuery.bindValue(":file_name", entry.fileName());
                insertFileQuery.bindValue(":file_size", entry.size());
                insertFileQuery.bindValue(":file_folder_path", entry.path());
                insertFileQuery.bindValue(":file_date_updated", entry.lastModified().toString("yyyy/MM/dd hh:mm:ss"));
                insertFileQuery.bindValue(":file_catalog", m_device->catalog->name);
                insertFileQuery.bindValue(":file_full_path", entryPath);
                insertFileQuery.exec();

                filesProcessed++;

                // Update progress every 250 files to avoid overwhelming the UI
                if (filesProcessed % 250 == 0) {
                    {
                        QMutexLocker locker(&m_progressMutex);
                        m_filesProcessed = filesProcessed;
                        m_currentPath = entry.absoluteFilePath();
                    }

                    // Emit progress update with proper totals
                    emit filesProcessedUpdate(filesProcessed, totalFiles, entry.absoluteFilePath());

                    // *** ADDITIONAL STOP CHECK DURING PROGRESS UPDATES ***
                    if (m_stopRequested) {
                        qDebug() << "Stop requested during progress update - rolling back transaction";
                        transactionRollback = true;
                        break;
                    }
                }
            }
        }
    }

    // qDebug() << "Final counts - Files:" << m_device->catalog->fileCount << "Size:" << m_device->catalog->totalFileSize;
    delete processIterator;

    // *** ADD THIS CRITICAL CHECK HERE ***
    if (transactionRollback || m_stopRequested) {
        qDebug() << "=== STOP DETECTED: Rolling back transaction and returning early ===";

        // Rollback the transaction
        QSqlQuery rollbackQuery(QSqlDatabase::database("defaultConnection"));
        rollbackQuery.prepare("ROLLBACK");
        rollbackQuery.exec();

        qDebug() << "=== EARLY RETURN: Stop processing completed ===";
        return; // ← CRITICAL: Don't continue with commit/save!
    }

    // Only continue if NOT stopped
    qDebug() << "File processing completed. About to commit transaction...";

    // Commit the transaction with proper error checking
    QSqlQuery commitQuery(QSqlDatabase::database("defaultConnection"));
    commitQuery.prepare("COMMIT");
    if (!commitQuery.exec()) {
        qDebug() << "ERROR: Failed to commit transaction:" << commitQuery.lastError().text();
        qDebug() << "Attempting to rollback...";

        // Try to rollback on commit failure
        QSqlQuery rollbackQuery(QSqlDatabase::database("defaultConnection"));
        rollbackQuery.prepare("ROLLBACK");
        rollbackQuery.exec();

        throw std::runtime_error(QString("Failed to commit transaction: %1").arg(commitQuery.lastError().text()).toStdString());
    }
    qDebug() << "Transaction committed successfully";

    // *** CALCULATE METADATA OURSELVES - AVOID CALLING EXTERNAL METHODS ***
    // Calculate total size from our inserts (no transaction issues)
    qint64 totalSize = 0;
    QSqlQuery sizeQuery(QSqlDatabase::database("defaultConnection"));
    sizeQuery.prepare("SELECT SUM(file_size) FROM file WHERE file_catalog_id = :catalog_id");
    sizeQuery.bindValue(":catalog_id", m_device->catalog->ID);
    if (sizeQuery.exec() && sizeQuery.next()) {
        totalSize = sizeQuery.value(0).toLongLong();
    }

    // Update catalog object directly (no database calls, no transaction issues)
    m_device->catalog->fileCount = filesProcessed;
    m_device->catalog->totalFileSize = totalSize;

    qDebug() << "Final counts - Files:" << m_device->catalog->fileCount << "Size:" << m_device->catalog->totalFileSize;

    {
        QMutexLocker locker(&m_progressMutex);
        m_filesProcessed = m_device->catalog->fileCount;
        m_totalFiles = m_device->catalog->fileCount;
        m_currentPath = QString("Catalog creation completed");
    }

    // Final progress update
    qDebug() << "About to send final progress update...";
    emit filesProcessedUpdate(m_device->catalog->fileCount, m_device->catalog->fileCount, QString("Completed"));
    qDebug() << "Final progress update sent";

    // *** SAVE TO FILES IF IN MEMORY MODE ***
    if (m_databaseMode == "Memory") {
        qDebug() << "Memory mode detected - preparing file saves...";

        try {
            // *** SAVE CATALOG FILE (.idx) ***
            qDebug() << "Building file list for catalog file...";
            QStringList fileList;

            // Query the database for all files we just inserted
            QSqlQuery queryFileList(QSqlDatabase::database("defaultConnection"));
            queryFileList.prepare(R"(
                SELECT file_full_path, file_size, file_date_updated
                FROM file
                WHERE file_catalog_id = :file_catalog_id
                ORDER BY file_full_path
            )");
            queryFileList.bindValue(":file_catalog_id", m_device->catalog->ID);

            if (!queryFileList.exec()) {
                qDebug() << "WARNING: Failed to query file list for .idx file:" << queryFileList.lastError().text();
            } else {
                // Build the file list (path + size + date separated by tabs)
                while (queryFileList.next()) {
                    QString filePath = queryFileList.value(0).toString();
                    QString fileSize = queryFileList.value(1).toString();
                    QString fileDate = queryFileList.value(2).toString();
                    fileList << filePath + "\t" + fileSize + "\t" + fileDate;
                }
                qDebug() << "Added" << fileList.size() << "file entries to catalog";
            }

            // Prepend catalog headers (in reverse order since we're prepending)
            fileList.prepend("<catalogID>"              + QString::number(m_device->catalog->ID));
            fileList.prepend("<catalogAppVersion>"      + m_device->catalog->appVersion);
            fileList.prepend("<catalogIncludeMetadata>" + QVariant(m_device->catalog->includeMetadata).toString());
            fileList.prepend("<catalogIsFullDevice>"    + QVariant(m_device->catalog->isFullDevice).toString());
            fileList.prepend("<catalogIncludeSymblinks>"+ QVariant(m_device->catalog->includeSymblinks).toString());
            fileList.prepend("<catalogStorage>"         + m_device->catalog->storageName);
            fileList.prepend("<catalogFileType>"        + m_device->catalog->fileType);
            fileList.prepend("<catalogIncludeHidden>"   + QVariant(m_device->catalog->includeHidden).toString());
            fileList.prepend("<catalogTotalFileSize>"   + QString::number(m_device->catalog->totalFileSize));
            fileList.prepend("<catalogFileCount>"       + QString::number(m_device->catalog->fileCount));
            fileList.prepend("<catalogSourcePath>"      + m_device->catalog->sourcePath);

            // Write catalog file directly (avoid fileListModel complexity)
            QString catalogFilePath = m_collectionFolder + "/" + m_device->catalog->name + ".idx";
            QFile catalogFile(catalogFilePath);
            if (catalogFile.open(QFile::WriteOnly | QFile::Text)) {
                QTextStream stream(&catalogFile);
                for (const QString &line : fileList) {
                    stream << line << '\n';
                }
                catalogFile.close();
                qDebug() << "Catalog file saved successfully:" << catalogFilePath;
            } else {
                qDebug() << "WARNING: Failed to open catalog file for writing:" << catalogFilePath;
            }

            // *** SAVE FOLDERS FILE (.folders.idx) ***
            qDebug() << "Building folder list for folders file...";
            QSqlQuery queryFolderList(QSqlDatabase::database("defaultConnection"));
            queryFolderList.prepare(R"(
                SELECT folder_catalog_id, folder_path
                FROM folder
                WHERE folder_catalog_id = :folder_catalog_id
                ORDER BY folder_path
            )");
            queryFolderList.bindValue(":folder_catalog_id", m_device->catalog->ID);

            if (!queryFolderList.exec()) {
                qDebug() << "WARNING: Failed to query folder list for .folders.idx file:" << queryFolderList.lastError().text();
            } else {
                QString foldersFilePath = m_collectionFolder + "/" + m_device->catalog->name + ".folders.idx";
                QFile foldersFile(foldersFilePath);
                if (foldersFile.open(QFile::WriteOnly | QFile::Text)) {
                    QTextStream stream(&foldersFile);
                    while (queryFolderList.next()) {
                        stream << queryFolderList.value(0).toString() << '\t';
                        stream << queryFolderList.value(1).toString() << '\n';
                    }
                    foldersFile.close();
                    qDebug() << "Folders file saved successfully:" << foldersFilePath;
                } else {
                    qDebug() << "WARNING: Failed to open folders file for writing:" << foldersFilePath;
                }
            }

            qDebug() << "Memory mode file saving completed successfully";

        } catch (const std::exception& e) {
            qDebug() << "EXCEPTION during Memory mode file saving:" << e.what();
            qDebug() << "Continuing - database has the data, files can be regenerated";
        } catch (...) {
            qDebug() << "UNKNOWN EXCEPTION during Memory mode file saving";
            qDebug() << "Continuing - database has the data, files can be regenerated";
        }
    } else {
        qDebug() << "Not in Memory mode - skipping file saves";
    }

    // *** UPDATE CATALOG RECORD IN DATABASE ***
    // This is a single UPDATE with no transaction conflicts
    qDebug() << "Updating catalog metadata in database...";
    QSqlQuery updateCatalogQuery(QSqlDatabase::database("defaultConnection"));
    updateCatalogQuery.prepare(R"(
        UPDATE catalog
        SET catalog_file_count = :file_count,
            catalog_total_file_size = :total_size,
            catalog_date_updated = :date_updated
        WHERE catalog_id = :catalog_id
    )");
    updateCatalogQuery.bindValue(":file_count", m_device->catalog->fileCount);
    updateCatalogQuery.bindValue(":total_size", m_device->catalog->totalFileSize);
    updateCatalogQuery.bindValue(":date_updated", QDateTime::currentDateTime().toString("yyyy/MM/dd hh:mm:ss"));
    updateCatalogQuery.bindValue(":catalog_id", m_device->catalog->ID);

    if (!updateCatalogQuery.exec()) {
        qDebug() << "WARNING: Failed to update catalog metadata:" << updateCatalogQuery.lastError().text();
    } else {
        qDebug() << "Catalog metadata updated successfully in database";
    }

    qDebug() << "About to emit final info message...";
    emit infoMessage(this, QString("Catalog created successfully. Files: %1, Size: %2 bytes")
                               .arg(m_device->catalog->fileCount).arg(m_device->catalog->totalFileSize));
    qDebug() << "Final info message emitted successfully";
}

void CatalogJob::updateProgress(qint64 filesProcessed, qint64 totalFiles, const QString &currentPath)
{
    QMutexLocker locker(&m_progressMutex);
    m_filesProcessed = filesProcessed;
    m_totalFiles = totalFiles;
    m_currentPath = currentPath;

    // Emit the detailed progress signal for CatalogManager
    int progressPercent = calculateProgressPercent(filesProcessed, totalFiles);
    emit progressDetailsUpdate(filesProcessed, totalFiles, progressPercent, currentPath);
}

int CatalogJob::calculateProgressPercent(qint64 processed, qint64 total) const
{
    if (total <= 0) return 0;
    return static_cast<int>((processed * 100) / total);
}

void CatalogJob::onProgressUpdate(qint64 filesProcessed, qint64 totalFiles, const QString &currentPath)
{
    updateProgress(filesProcessed, totalFiles, currentPath);

    // Emit the update signal
    emit filesProcessedUpdate(filesProcessed, totalFiles, currentPath);
}

void CatalogJob::updateJobProgress()
{
    QMutexLocker locker(&m_progressMutex);

    // Update KJob progress percentage
    int progressPercent = calculateProgressPercent(m_filesProcessed, m_totalFiles);
    setPercent(progressPercent);

    // Send info message with current progress
    if (!m_currentPath.isEmpty()) {
        QString progressMsg = QString("Files processed: %1 | Total files: %2 | Progress: %3% | %4")
        .arg(m_filesProcessed)
            .arg(m_totalFiles)
            .arg(progressPercent)
            .arg(m_currentPath);
        emit infoMessage(this, progressMsg);
    }
}

void CatalogJob::onWorkError(const QString &errorMessage)
{
    qDebug() << "CatalogJob::onWorkError() - Error:" << errorMessage;

    m_progressTimer->stop();

    setError(KJob::UserDefinedError);
    setErrorText(errorMessage);

    emit infoMessage(this, QString("Error: %1").arg(errorMessage));

    emitResult();
}

bool CatalogJob::doKill()
{
    qDebug() << "CatalogJob::doKill() - Stop requested";

    m_stopRequested = true;
    m_progressTimer->stop();

    setError(KJob::KilledJobError);
    setErrorText("Catalog operation was cancelled by user");

    emit infoMessage(this, "Catalog operation cancelled");

    // ADD THIS LINE - This is the only fix needed!
    emitResult();

    return true;
}
