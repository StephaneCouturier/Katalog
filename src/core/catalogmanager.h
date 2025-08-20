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
// File Name:   catalogmanager.h
// Purpose:     Manager for catalog operations (follows SearchManager pattern exactly)
// Description: Provides clean UI interface for catalog creation/update operations
// Author:      Stephane Couturier
/////////////////////////////////////////////////////////////////////////////
*/
#ifndef CATALOGMANAGER_H
#define CATALOGMANAGER_H

#pragma once

#include <QObject>
#include <QTimer>
#include "catalogjob.h"
#include "catalogjobstoppable.h"

/**
 * @brief The CatalogManager class
 * Manages catalog creation/update operations using KJob framework
 * Provides a clean interface for the UI to interact with catalog jobs
 * Follows the exact same pattern as SearchManager for consistency
 */
class CatalogManager : public QObject
{
    Q_OBJECT

    Q_PROPERTY(bool catalogOperationRunning READ catalogOperationRunning NOTIFY catalogOperationRunningChanged)
    Q_PROPERTY(int progress READ progress NOTIFY progressChanged)
    Q_PROPERTY(QString status READ status NOTIFY statusChanged)
    Q_PROPERTY(QString currentCatalogName READ currentCatalogName NOTIFY currentCatalogNameChanged)
    Q_PROPERTY(qint64 filesProcessed READ filesProcessed NOTIFY filesProcessedChanged)
    Q_PROPERTY(qint64 totalFiles READ totalFiles NOTIFY totalFilesChanged)
    Q_PROPERTY(QString currentPath READ currentPath NOTIFY currentPathChanged)

public:
    explicit CatalogManager(QObject *parent = nullptr);
    ~CatalogManager();

    // Properties
    bool catalogOperationRunning() const { return m_catalogOperationRunning; }
    int progress() const { return m_progress; }
    QString status() const { return m_status; }
    QString currentCatalogName() const { return m_currentCatalogName; }
    qint64 filesProcessed() const { return m_filesProcessed; }
    qint64 totalFiles() const { return m_totalFiles; }
    QString currentPath() const { return m_currentPath; }

    // Get the current catalog operation results
    CatalogJobStoppable* getCurrentCatalogEngine() const;

    QString getEffectiveCatalogID(int catalogID) const;
    bool isCatalogBeingUpdated(int catalogID) const;
    void requestGentleStop();
    void requestHardStop();
    bool isGentleStopRequested() const { return m_gentleStopRequested.loadAcquire(); }

    // Batch processing
    bool inBatchMode() const { return m_inBatchMode; }
    int batchProgress() const { return m_batchCurrentIndex; }
    int totalBatchCatalogs() const { return m_batchCatalogs.size(); }

    // Batch processing methods (moved from MainWindow)
    void startNextCatalogUpdate();
    void processNextCatalogUpdate();
    void startCurrentBatchCatalog();
    void finishBatchOperation();

    // Batch state management
    void initializeBatchOperation(const QList<Device*>& catalogs, const QString& databaseMode, const QString& collectionFolder);
    void setBatchMode(bool enabled) { m_inBatchMode = enabled; }
    void incrementBatchIndex() { m_batchCurrentIndex++; }
    void resetBatchState();

    // Global statistics management
    void updateGlobalStatistics(qint64 totalFiles, qint64 deltaFiles, qint64 totalSize, qint64 deltaSize);
    void incrementUpdatedCatalogs() { m_updatedCatalogs++; }
    void incrementSkippedCatalogs() { m_skippedCatalogs++; }

    // Getters for statistics (for UI reporting)
    qint64 globalUpdateTotalFiles() const { return m_globalUpdateTotalFiles; }
    qint64 globalUpdateDeltaFiles() const { return m_globalUpdateDeltaFiles; }
    qint64 globalUpdateTotalSize() const { return m_globalUpdateTotalSize; }
    qint64 globalUpdateDeltaSize() const { return m_globalUpdateDeltaSize; }
    int updatedCatalogs() const { return m_updatedCatalogs; }
    int skippedCatalogs() const { return m_skippedCatalogs; }
    void setInitialSkippedCount(int count) { m_skippedCatalogs = count; }

private:
    void setCatalogOperationRunning(bool running);
    void setProgress(int progress);
    void setStatus(const QString &status);
    void setCurrentCatalogName(const QString &catalogName);
    void setFilesProcessed(qint64 processed);
    void setTotalFiles(qint64 total);
    void setCurrentPath(const QString &path);
    void cleanupJob();
    void handleBatchCatalogCompletion();
    QAtomicInt m_gentleStopRequested{0};
    QAtomicInt m_hardStopRequested{0};
    int m_updatingCatalogID = 0;
    bool m_catalogUpdateInProgress = false;

    CatalogJob *m_currentJob = nullptr;
    bool m_catalogOperationRunning = false;
    int m_progress = 0;
    QString m_status = "Ready";
    QString m_currentCatalogName;
    qint64 m_filesProcessed = 0;
    qint64 m_totalFiles = 0;
    QString m_currentPath;
    bool m_isPaused = false;

    // Batch processing state
    QList<Device*> m_batchCatalogs;           // All catalogs to process in batch
    int m_batchCurrentIndex = 0;              // Current catalog being processed
    bool m_inBatchMode = false;               // Clear flag: are we in batch mode?

    // Batch operation parameters (stored during initialization)
    QString m_batchDatabaseMode;
    QString m_batchCollectionFolder;

    // Global batch statistics
    qint64 m_globalUpdateTotalFiles = 0;
    qint64 m_globalUpdateDeltaFiles = 0;
    qint64 m_globalUpdateTotalSize = 0;
    qint64 m_globalUpdateDeltaSize = 0;
    int m_updatedCatalogs = 0;
    int m_skippedCatalogs = 0;

public slots:
    void startCatalogJobStoppable(CatalogJobStoppable *catalogEngine,
                                  Device *targetDevice,
                                  CatalogJobStoppable::OperationType operationType,
                                  const QString &databaseMode,
                                  const QString &collectionFolder);
    void stopCatalogOperation();
    void pauseCatalogOperation();
    void resumeCatalogOperation();

private slots:
    void onJobResult(KJob *job);
    void onJobPercent();
    void onJobInfoMessage(KJob *job, const QString &message);

signals:
    void catalogOperationRunningChanged();
    void progressChanged();
    void statusChanged();
    void currentCatalogNameChanged();
    void filesProcessedChanged();
    void totalFilesChanged();
    void currentPathChanged();

    void catalogOperationCompleted();
    void catalogOperationCancelled();
    void catalogOperationError(const QString &error);

    // Special progress update for status bar (like SearchManager)
    void specialProgressUpdate(qint64 filesProcessed, qint64 totalFiles, int progressPercent, const QString &currentPath);

    // Batch operations (to notify MainWindow for UI updates)
    void batchOperationCompleted();
    void batchCatalogStarted(Device* device, int currentIndex, int totalCount);
    void batchNeedsUIReport(Device* device, const QList<qint64>& results, const QString& updateType);
};

#endif // CATALOGMANAGER_H
