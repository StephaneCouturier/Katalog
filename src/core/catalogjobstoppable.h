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
        UpdateCatalog
    };

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

    QList<qint64> getResults() const;

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
    void processDirectoryWithProgress(const QString &directory,
                                      Catalog *catalog,
                                      qint64 &processedCount);

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
    QString m_connectionName;

    // State management (atomic for thread safety)
    QAtomicInt m_stopRequested{0};
    QAtomicInt m_paused{0};
    mutable QMutex m_pauseMutex;
    QAtomicInt m_objectValid{1};

    QDateTime m_lastProgressEmit;
    static const int PROGRESS_UPDATE_INTERVAL_MS = 500; // Update every 500ms max

    // Track original values for delta calculation
    qint64 m_originalFileCount = 0;
    qint64 m_originalTotalFileSize = 0;


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
};

#endif // CATALOGJOBSTOPPABLE_H
