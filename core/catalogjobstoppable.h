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
// File Name:   catalogjobstoppable.h
// Purpose:     Class for stoppable catalog creation/update operations
// Description: Handles actual catalog work with progress reporting and stop/pause capabilities
// Author:      Stephane Couturier
/////////////////////////////////////////////////////////////////////////////
*/
#ifndef CATALOGJOBSTOPPABLE_H
#define CATALOGJOBSTOPPABLE_H

#pragma once
#include <QObject>
#include <QAtomicInt>
#include <QMutex>
#include "device.h"
#include "parallelmetadataextractor.h"

/**
 * @brief The CatalogJobStoppable class
 * Handles actual catalog creation/update work with progress reporting
 * and stop/pause capabilities. Follows the same pattern as SearchJobStoppable.
 */
class CatalogJobStoppable : public QObject
{
    Q_OBJECT

public:
    enum OperationType {
        CreateCatalog,
        UpdateCatalog,
        VerifyMimeTypes,
        ExtractMissingMetadata
    };

    // Update statistics structure
    struct UpdateStatistics {
        int newFiles = 0;
        int modifiedFiles = 0;
        int deletedFiles = 0;
        int unchangedFiles = 0;
        int metadataExtracted = 0;

        void clear() {
            newFiles = 0;
            modifiedFiles = 0;
            deletedFiles = 0;
            unchangedFiles = 0;
            metadataExtracted = 0;
        }

        int totalChanges() const {
            return newFiles + modifiedFiles + deletedFiles;
        }
    };

    // Get update statistics from last operation
    UpdateStatistics getUpdateStatistics() const { return m_updateStats; }

    explicit CatalogJobStoppable(QObject *parent = nullptr);
    ~CatalogJobStoppable();

    /**
     * @brief Configure the catalog operation
     * @param device Device containing catalog to process
     * @param operationType Create or update operation
     * @param databaseMode Database mode (e.g., "Memory")
     * @param collectionFolder Collection folder path
     * @param connectionName Database connection name
     */
    void configureOperation(Device *device,
                            OperationType operationType,
                            const QString &databaseMode,
                            const QString &collectionFolder,
                            const QString &connectionName = "defaultConnection");

    /**
     * @brief Main method to execute catalog operation
     * @param selectedDevice The device to process (usually same as configured device)
     */
    void processCatalog();

    /**
     * @brief Stop the catalog operation
     */
    void stopCatalogOperation();

    /**
     * @brief Pause the catalog operation (can be resumed later)
     */
    void pauseCatalogOperation();

    /**
     * @brief Resume a paused catalog operation
     */
    void resumeCatalogOperation();

    /**
     * @brief Check if operation was requested to stop
     */
    bool wasStopRequested() const { return m_stopRequested.loadAcquire(); }

    /**
     * @brief Check if operation is currently paused
     */
    bool isPaused() const { return m_paused.loadAcquire(); }

    /**
     * @brief Set the progress refresh rate (files processed per progress update)
     */
    void setProgressRefreshRate(int rate) { progressRefreshRate = rate; }

    /**
     * @brief Get counted total files for progress calculation
     */
    qint64 getCountedTotalFiles() const { return countedTotalFiles; }

    /**
     * @brief Get current catalog name being processed
     */
    QString getCurrentCatalogName() const { return currentCatalogName; }

    /**
     * @brief Complete catalog creation with all post-processing tasks
     * @param device The device that was processed
     */
    void completeCatalogCreation();

    // Progress tracking properties (similar to SearchJobStoppable)
    qint64 countedTotalFiles = 0;
    qint64 filesProcessed = 0;
    QString currentCatalogName;
    int progressRefreshRate = 100; // Progress update frequency

    // Shared across all batches of one catalog run. processBatch() is called
    // once per 100 files, so a local extractor would rebuild its thread pool
    // on every batch.
    ParallelMetadataExtractor metadataExtractor;

    // Set once the created catalog's file list is committed. After that the
    // catalog is kept even if the user stops during metadata/checksum scanning.
    bool m_catalogCommitted = false;

    // Non-empty when the catalog was kept but its metadata and/or checksum scan
    // did not finish; holds the ready-to-display explanation for the user.
    QString m_scanIncompleteMessage;

public:
    QString scanIncompleteMessage() const { return m_scanIncompleteMessage; }
    // True once a created catalog's file list is committed, so a stop after that
    // point must keep the catalog instead of discarding it.
    bool catalogCommitted() const { return m_catalogCommitted; }

    QList<qint64> getResults() const;

    void requestHardStop();

protected:
    /**
     * @brief Create catalog with progress reporting
     * @param device The device to create catalog for
     */
    void createCatalogWithProgress();

    /**
     * @brief Update catalog with progress reporting
     * @param device The device to update catalog for
     */
    void updateCatalogWithProgress();

    /**
     * @brief Process directory recursively with progress updates
     * @param directory Directory to process
     * @param catalog Catalog object to populate
     * @param processedCount Running count of processed files
     */
    void extracted(Catalog *&catalog, QString &fileFullPath, bool &isExcluded);
    void processDirectoryWithProgress(const QString &directory,
                                      Catalog *catalog, qint64 &processedCount);

    /**
     * @brief Count total files in directory for progress estimation
     * @param directory Directory to count
     * @param catalog Catalog with file type filters
     * @return counted total file count
     */
    qint64 countTotalFiles(const QString &directory, Catalog *catalog);

private:
    bool shouldContinue() const;
    void waitIfPaused();
    void emitProgressUpdate(qint64 processed, qint64 total, const QString &currentPath);

    // Configuration
    Device *m_device = nullptr;
    OperationType m_operationType = CreateCatalog;
    QString m_databaseMode;
    QString m_collectionFolder;
    QString m_connectionName = "defaultConnection";

    // State management (atomic for thread safety)
    QAtomicInt m_stopRequested{0};
    QAtomicInt m_hardStopRequested{0};
    QAtomicInt m_paused{0};
    mutable QMutex m_pauseMutex;
    QAtomicInt m_objectValid{1};

    QDateTime m_lastProgressEmit;
    static const int PROGRESS_UPDATE_INTERVAL_MS = 500; // Update every 500ms max

    // Track original values for delta calculation
    qint64 m_originalFileCount = 0;
    qint64 m_originalTotalFileSize = 0;

    void updateRelatedCatalogDevices();
    Storage::UpdateResult m_storageUpdateResult;
    bool m_storageWasUpdated = false;

    // MIME verification methods
    void extractMissingMetadata();
    bool shouldExtractMetadata(const QString &filePath, Catalog *catalog) const;
    void verifyMimeTypes();
    void saveMismatchReport(const QStringList &mismatches);
    int m_mismatchCount = 0;
    QString m_reportFilePath;
    QString saveMismatchReportAndReturnPath(const QStringList &mismatches);

    // Helper for fast type detection
    QString getQuickFileType(const QFileInfo &fileInfo) const;

    void processBatch(QStringList& fileNames, QStringList& fileFolderPaths,
                                           QStringList& fileFullPaths, QStringList& fileDateTimes,
                                           QStringList& fileCatalogs, QList<qint64>& fileSizes,
                                           QStringList& fileExtensions, QStringList& fileTypes,
                                           QStringList& mimeTypes,
                                           QStringList& directoryPaths, Catalog* catalog);

    void insertFolders(const QStringList& folderPaths, Catalog* catalog);

    // Incremental update methods
    void updateCatalogIncremental();
    void scanDirectoryIntoFiletemp(const QString &directory, Catalog *catalog, qint64 &processedCount);
    QList<QVariantList> findFilesWithoutMetadata();

    QList<QVariantList> findFilesWithoutChecksum();
    void extractChecksumsForFiles(const QList<QVariantList> &files);

    // SQL-based difference detection
    QList<QVariantList> findNewFiles();
    QList<QVariantList> findModifiedFiles();
    QStringList findDeletedFiles();
    int countUnchangedFiles();

    // Bulk operations
    void insertNewFilesFromFiletemp(const QList<QVariantList> &newFiles);
    void updateModifiedFilesFromFiletemp(const QList<QVariantList> &modifiedFiles);
    void deleteRemovedFiles(const QStringList &deletedFiles);
    void extractMetadataForChangedFiles(const QList<QVariantList> &changedFiles);

    // Update statistics tracking
    UpdateStatistics m_updateStats;

    void migrateMimeTypesForExistingFiles();

signals:
    /**
     * @brief Signal emitted to report progress during catalog operations
     * @param filesProcessed Number of files processed so far
     * @param totalFiles Estimated total number of files
     * @param currentPath Path of current file/directory being processed
     */
    void catalogProgress(qint64 filesProcessed, qint64 totalFiles, const QString &currentPath);

    /**
     * @brief Signal emitted when catalog operation completes successfully
     */
    void catalogOperationFinished();

    /**
     * @brief Signal emitted when catalog operation encounters an error
     * @param errorMessage Description of the error
     */
    void catalogOperationError(const QString &errorMessage);
    void mimeVerificationCompleted(int mismatchCount, const QString& reportPath);
};

#endif // CATALOGJOBSTOPPABLE_H
