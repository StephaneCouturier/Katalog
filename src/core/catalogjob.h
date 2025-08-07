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
// File Name:   catalogjob.h
// Purpose:     KJob implementation for catalog operations
// Description: Handles catalog creation and update operations with progress reporting
// Author:      Stephane Couturier
/////////////////////////////////////////////////////////////////////////////
*/
#ifndef CATALOGJOB_H
#define CATALOGJOB_H


#include "device.h"

#include <KJob>

#include <QObject>
#include <QThread>
#include <QMutex>
#include <QTimer>
#pragma once

/**
 * @brief The CatalogJob class
 * KJob implementation for catalog creation and update operations
 * Provides progress reporting and will support cancellation in the future
 */
class CatalogJob : public KJob
{
    Q_OBJECT

public:
    enum OperationType {
        CreateCatalog,
        UpdateCatalog
    };

    /**
     * @brief Constructor for CatalogJob
     * @param device The device (with catalog) to operate on
     * @param operationType Whether to create or update the catalog
     * @param databaseMode Database mode (e.g., "Memory")
     * @param collectionFolder Path to the collection folder
     * @param parent Parent QObject
     */
    explicit CatalogJob(Device *device,
                        OperationType operationType,
                        const QString &databaseMode,
                        const QString &collectionFolder,
                        QObject *parent = nullptr);

    ~CatalogJob();

    /**
     * @brief Start the catalog operation
     * Reimplemented from KJob
     */
    void start() override;

    /**
     * @brief Get the device being processed
     * @return Pointer to the device
     */
    Device* getDevice() const { return m_device; }

    /**
     * @brief Get the operation type
     * @return The type of operation (create or update)
     */
    OperationType getOperationType() const { return m_operationType; }

protected:
    /**
     * @brief Kill/stop the job (from KJob interface)
     * @return true if job was successfully killed
     */
    bool doKill() override;

protected:
    /**
     * @brief Main work method executed in separate thread
     */
    void doWork();

    /**
     * @brief Update progress during file processing
     * @param filesProcessed Number of files processed so far
     * @param totalFiles Total number of files to process
     * @param currentPath Current file/directory being processed
     */
    void updateProgress(qint64 filesProcessed, qint64 totalFiles, const QString &currentPath);

    /**
     * @brief Calculate percentage progress
     * @param processed Files processed
     * @param total Total files
     * @return Percentage (0-100)
     */
    int calculateProgressPercent(qint64 processed, qint64 total) const;

private slots:
    /**
     * @brief Handle progress updates from catalog operations
     */
    void onProgressUpdate(qint64 filesProcessed, qint64 totalFiles, const QString &currentPath);

    /**
     * @brief Periodically update job percentage
     */
    void updateJobProgress();
    void createCatalogWithProgress();

private:
    Device *m_device;
    OperationType m_operationType;
    QString m_databaseMode;
    QString m_collectionFolder;

    // Progress tracking
    qint64 m_filesProcessed = 0;
    qint64 m_totalFiles = 0;
    QString m_currentPath;
    QTimer *m_progressTimer;

    // Threading
    QMutex m_progressMutex;

    // Operation state
    bool m_operationStarted = false;
    bool m_stopRequested = false;

signals:
    /**
     * @brief Signal emitted when files processed count updates
     * @param filesProcessed Number of files processed
     * @param totalFiles Total number of files
     * @param currentPath Current file/directory path
     */
    void filesProcessedUpdate(qint64 filesProcessed, qint64 totalFiles, const QString &currentPath);

    /**
     * @brief Signal for detailed progress information
     * Used by CatalogManager to update UI
     */
    void progressDetailsUpdate(qint64 filesProcessed, qint64 totalFiles, int progressPercent, const QString &currentPath);

    /**
     * @brief Internal signal to trigger work in worker thread
     */
    void startWork();

private slots:
    /**
     * @brief Slot called when worker thread encounters an error
     */
    void onWorkError(const QString &errorMessage);
};

#endif // CATALOGJOB_H
