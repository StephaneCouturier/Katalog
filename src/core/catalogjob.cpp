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
        if (m_operationType == CreateCatalog) {
            // For create operations, start the catalog creation directly
            emit infoMessage(this, QString("Creating catalog %1...").arg(m_device->catalog->name));

            // Call our progress-reporting catalog creation
            createCatalogWithProgress();

        } else {
            // For update operations, use existing catalog update method
            emit infoMessage(this, QString("Updating catalog %1...").arg(m_device->catalog->name));

            // Use existing updateCatalogFiles method
            QList<qint64> updateResults = m_device->catalog->updateCatalogFiles(m_databaseMode, m_collectionFolder, false);

            // Process results
            if (!updateResults.isEmpty() && updateResults[0] == 1) {
                // Success
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
                // Handle error cases
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

        // Operation completed successfully
        qDebug() << "=== Catalog operation completed successfully ===";
        emit infoMessage(this, QString("Catalog operation completed successfully"));

        qDebug() << "About to stop progress timer...";
        m_progressTimer->stop();
        qDebug() << "Progress timer stopped successfully";

        // Set final progress and complete the job
        qDebug() << "About to set final progress to 100%...";
        setPercent(100);
        qDebug() << "Final progress set to 100% successfully";

        qDebug() << "About to emit result...";
        emitResult(); // This is the proper KJob way to signal completion
        qDebug() << "Result emitted successfully";

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
    beginQuery.exec();

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

    // Process files with progress reporting
    while (processIterator->hasNext()) {
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
                }
            }
        }
    }

    delete processIterator;

    qDebug() << "File processing completed. About to commit transaction...";

    // Commit the transaction
    QSqlQuery commitQuery(QSqlDatabase::database("defaultConnection"));
    commitQuery.prepare("COMMIT");
    if (!commitQuery.exec()) {
        qDebug() << "ERROR: Failed to commit transaction:" << commitQuery.lastError().text();
        throw std::runtime_error(QString("Failed to commit transaction: %1").arg(commitQuery.lastError().text()).toStdString());
    }
    qDebug() << "Transaction committed successfully";

    // Update catalog metadata
    qDebug() << "About to update catalog metadata...";
    m_device->catalog->updateFileCount();
    qDebug() << "File count updated";

    m_device->catalog->updateTotalFileSize();
    qDebug() << "Total file size updated";

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

    // Save to files if in Memory mode
    if (m_databaseMode == "Memory") {
        qDebug() << "Memory mode detected - preparing file list model...";

        // *** POPULATE THE FILE LIST MODEL THAT saveCatalogToFile() EXPECTS ***
        // This is the missing step that was causing the crash!
        QStringList fileList;

        QSqlQuery queryFileList(QSqlDatabase::database("defaultConnection"));
        QString queryFileListSQL = R"(
                        SELECT file_full_path, file_size, file_date_updated
                        FROM file
                        WHERE file_catalog_id = :file_catalog_id
                    )";
        queryFileList.prepare(queryFileListSQL);
        queryFileList.bindValue(":file_catalog_id", m_device->catalog->ID);

        qDebug() << "Executing file list query for catalog ID:" << m_device->catalog->ID;
        if (!queryFileList.exec()) {
            qDebug() << "ERROR: Failed to execute file list query:" << queryFileList.lastError().text();
            throw std::runtime_error("Failed to query file list for model");
        }

        qDebug() << "Building file list for model...";
        while(queryFileList.next()){
            fileList << queryFileList.value(0).toString() + "\t" + queryFileList.value(1).toString() + "\t" + queryFileList.value(2).toString();
        }
        qDebug() << "File list built with" << fileList.size() << "entries";

        // Prepare the catalog file data, adding headers at the beginning (like original catalogDirectory)
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

        qDebug() << "Creating and populating fileListModel...";
        // Create and populate the model that saveCatalogToFile expects
        m_device->catalog->fileListModel = new QStringListModel(m_device->catalog);
        m_device->catalog->fileListModel->setStringList(fileList);
        qDebug() << "fileListModel created with" << fileList.size() << "total entries (including headers)";

        qDebug() << "Now calling saveCatalogToFile...";
        try {
            bool catalogSaved = m_device->catalog->saveCatalogToFile(m_databaseMode, m_collectionFolder);
            if (!catalogSaved) {
                qDebug() << "WARNING: saveCatalogToFile returned false, but continuing...";
            } else {
                qDebug() << "Catalog file saved successfully";
            }
        } catch (const std::exception& e) {
            qDebug() << "EXCEPTION during saveCatalogToFile:" << e.what();
            qDebug() << "Continuing without file save - database has the data";
        } catch (...) {
            qDebug() << "UNKNOWN EXCEPTION during saveCatalogToFile";
            qDebug() << "Continuing without file save - database has the data";
        }

        qDebug() << "Saving folders to file...";
        try {
            bool foldersSaved = m_device->catalog->saveFoldersToFile(m_databaseMode, m_collectionFolder);
            if (!foldersSaved) {
                qDebug() << "WARNING: saveFoldersToFile returned false, but continuing...";
            } else {
                qDebug() << "Folders file saved successfully";
            }
        } catch (const std::exception& e) {
            qDebug() << "EXCEPTION during saveFoldersToFile:" << e.what();
            qDebug() << "Continuing without folders file save - database has the data";
        } catch (...) {
            qDebug() << "UNKNOWN EXCEPTION during saveFoldersToFile";
            qDebug() << "Continuing without folders file save - database has the data";
        }

        qDebug() << "File saving phase completed successfully";
    } else {
        qDebug() << "Not in Memory mode - skipping file saves";
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

    return true;
}
