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
// File Name:   catalogmanager.cpp
// Purpose:     Manager implementation for catalog operations
// Description: Provides clean UI interface for catalog creation/update operations
//              Follows SearchManager pattern exactly for consistency
// Author:      Stephane Couturier
/////////////////////////////////////////////////////////////////////////////
*/

#include "catalogmanager.h"
#include <QDebug>

CatalogManager::CatalogManager(QObject *parent)
    : QObject(parent)
{
    qDebug() << "CatalogManager created";
}

CatalogManager::~CatalogManager()
{
    if (m_currentJob) {
        m_currentJob->kill();
        m_currentJob->deleteLater();
    }
    qDebug() << "CatalogManager destroyed";
}

void CatalogManager::startCatalogJobStoppable(CatalogJobStoppable *catalogEngine,
                                              Device *targetDevice,
                                              CatalogJobStoppable::OperationType operationType,
                                              const QString &databaseMode,
                                              const QString &collectionFolder)
{
    if (operationType == CatalogJobStoppable::UpdateCatalog) {
        m_catalogUpdateInProgress = true;
        m_updatingCatalogID = targetDevice->catalog->ID;
        qDebug() << "Starting catalog update - marking catalog" << m_updatingCatalogID << "as being updated";
    }

    if (m_currentJob) {
        qDebug() << "Catalog operation already running!";
        return;
    }

    if (!catalogEngine || !targetDevice) {
        emit catalogOperationError("Invalid catalog configuration");
        return;
    }

    if (operationType == CatalogJobStoppable::UpdateCatalog) {
        m_catalogUpdateInProgress = true;
        m_updatingCatalogID = targetDevice->catalog->ID;

        // ADD THESE LINES:
        m_gentleStopRequested.storeRelease(0);  // Reset stop flags for new operation
        m_hardStopRequested.storeRelease(0);
        qDebug() << "Reset stop flags for new catalog operation";
    }

    qDebug() << "Starting catalog operation:"
             << "Type:" << (operationType == CatalogJobStoppable::CreateCatalog ? "Create" : "Update")
             << "Catalog:" << targetDevice->catalog->name
             << "DatabaseMode:" << databaseMode;

    m_currentJob = new CatalogJob(this);
    m_currentJob->setCatalogJobStoppable(catalogEngine);
    m_currentJob->setTargetDevice(targetDevice);
    m_currentJob->setOperationType(operationType);
    m_currentJob->setDatabaseMode(databaseMode);
    m_currentJob->setCollectionFolder(collectionFolder);

    // Connect job signals with enhanced progress handling (same pattern as SearchManager)
    connect(m_currentJob, &KJob::result, this, &CatalogManager::onJobResult);

    // Enhanced progress connection that handles catalog progress updates
    connect(m_currentJob, &CatalogJob::catalogProgress, this, [this](qint64 filesProcessed, qint64 totalFiles, const QString &currentPath) {
        if (m_currentJob && m_currentJob->getCatalogEngine()) {
            CatalogJobStoppable* engine = m_currentJob->getCatalogEngine();

            // Regular progress updates
            if (filesProcessed >= 0) {
                setFilesProcessed(filesProcessed);
                setTotalFiles(totalFiles);
                setCurrentPath(currentPath);

                if (totalFiles > 0) {
                    int percent = qMin(100, static_cast<int>((filesProcessed * 100) / totalFiles));
                    setProgress(percent);
                }

                setCurrentCatalogName(engine->getCurrentCatalogName());

                // Set status based on current operation
                QString operationType = (m_currentJob->getOperationType() == CatalogJobStoppable::CreateCatalog) ? "Creating" : "Updating";
                if (totalFiles > 0) {
                    setStatus(QString("%1 catalog - %2 of %3 files processed")
                                  .arg(operationType)
                                  .arg(filesProcessed)
                                  .arg(totalFiles));
                } else {
                    setStatus(QString("%1 catalog - %2 files processed")
                                  .arg(operationType)
                                  .arg(filesProcessed));
                }

                // Emit special progress update for status bar
                emit specialProgressUpdate(filesProcessed, totalFiles, progress(), currentPath);
            }
        }
    });

    setStatus("Starting catalog operation...");
    setCatalogOperationRunning(true);
    setProgress(0);
    setFilesProcessed(0);
    setTotalFiles(0);
    setCurrentPath("");
    m_isPaused = false;

    // Start the job
    m_currentJob->startJob();
}

void CatalogManager::stopCatalogOperation()
{
    qDebug() << "=== CatalogManager::stopCatalogOperation() called ===";
    qDebug() << "Current job exists:" << (m_currentJob != nullptr);
    qDebug() << "Catalog operation running:" << m_catalogOperationRunning;

    if (!m_currentJob) {
        qDebug() << "No catalog operation to stop!";
        return;
    }

    qDebug() << "Stopping catalog operation...";
    setStatus("Catalog operation stopped");

    // Disconnect signals first to prevent double handling (same pattern as SearchManager)
    disconnect(m_currentJob, nullptr, this, nullptr);

    // Kill the job
    qDebug() << "Calling m_currentJob->kill()";
    m_currentJob->kill();

    // Force immediate cleanup since result signal might not be emitted
    qDebug() << "Force cleanup after kill...";
    setCatalogOperationRunning(false);
    setProgress(0);
    setCurrentCatalogName("");
    setFilesProcessed(0);
    setTotalFiles(0);
    setCurrentPath("");
    m_isPaused = false;

    // Clean up immediately
    if (m_currentJob) {
        m_currentJob->deleteLater();
        m_currentJob = nullptr;
        qDebug() << "Forced cleanup complete";
    }

    emit catalogOperationCancelled();
    qDebug() << "=== CatalogManager::stopCatalogOperation() complete ===";
}

void CatalogManager::pauseCatalogOperation()
{
    if (!m_currentJob || m_isPaused) {
        return;
    }

    if (m_currentJob->suspend()) {
        m_isPaused = true;
        qDebug() << "Catalog operation paused";
    }
}

void CatalogManager::resumeCatalogOperation()
{
    if (!m_currentJob || !m_isPaused) {
        return;
    }

    if (m_currentJob->resume()) {
        m_isPaused = false;
        qDebug() << "Catalog operation resumed";
    }
}

CatalogJobStoppable* CatalogManager::getCurrentCatalogEngine() const
{
    if (m_currentJob) {
        return m_currentJob->getCatalogEngine();
    }
    return nullptr;
}

void CatalogManager::requestGentleStop()
{
    qDebug() << "CatalogManager::requestGentleStop() - Gentle stop requested";
    m_gentleStopRequested.storeRelease(1);

    if (m_inBatchMode) {
        setStatus("Stopping after current catalog completes...");
        // Also stop the currently running catalog
        if (m_currentJob) {
            qDebug() << "Stopping current catalog in batch gentle stop";
            stopCatalogOperation();
        }
    } else {
        stopCatalogOperation(); // Single operations = immediate stop
    }
}

void CatalogManager::onJobResult(KJob *job)
{
    qDebug() << "=== CatalogManager::onJobResult() ENTRY ===";

    try {
        if (job->error() == KJob::KilledJobError) {
            setStatus("Catalog operation cancelled");
            emit catalogOperationCancelled();
        } else if (job->error()) {
            QString errorMsg = QString("Catalog operation failed: %1").arg(job->errorString());
            setStatus(errorMsg);
            emit catalogOperationError(errorMsg);
        } else {
            // Success, emit signals before any cleanup
            QString operationType = (m_currentJob && m_currentJob->getOperationType() == CatalogJobStoppable::CreateCatalog) ?
                                        "creation" : "update";
            setStatus(QString("Catalog %1 completed successfully!").arg(operationType));

            // Emit individual report here (before cleanup, while job is still valid)
            if (m_inBatchMode) {
                CatalogJobStoppable* catalogEngine = getCurrentCatalogEngine();
                if (catalogEngine) {
                    QList<qint64> results = catalogEngine->getResults();
                    if (results.size() >= 5 && results[0] == 1) {
                        // Get current device BEFORE any index changes
                        Device* currentDevice = (m_batchCurrentIndex < m_batchCatalogs.size()) ?
                                                    m_batchCatalogs[m_batchCurrentIndex] : nullptr;
                        if (currentDevice) {
                            // Emit individual report immediately
                            emit batchNeedsUIReport(currentDevice, results, "update");
                        }

                        // Update global statistics
                        updateGlobalStatistics(results[1], results[2], results[3], results[4]);
                        m_updatedCatalogs++;
                    } else {
                        m_skippedCatalogs++;
                    }
                } else {
                    m_skippedCatalogs++;
                }

                // Increment and continue batch
                m_batchCurrentIndex++;
            }

            emit catalogOperationCompleted();
        }

        // Batch handling - just start next or finish
        if (m_inBatchMode) {
            // ADD THIS CHECK FIRST:
            if (m_hardStopRequested.loadAcquire()) {
                qDebug() << "Hard stop requested - finishing batch operation early";
                m_hardStopRequested.storeRelease(0); // Reset flag
                finishBatchOperation();
                return;
            }

            if (m_batchCurrentIndex >= m_batchCatalogs.size()) {
                finishBatchOperation();
            } else {
                // Continue with next catalog
                QTimer::singleShot(100, this, [this]() {
                    startCurrentBatchCatalog();
                });
            }
        }

        // Clean up and reset state
        setCatalogOperationRunning(false);
        setProgress(0);
        setCurrentCatalogName("");
        setFilesProcessed(0);
        setTotalFiles(0);
        setCurrentPath("");
        m_isPaused = false;
        cleanupJob();

    } catch (const std::exception& e) {
        qDebug() << "EXCEPTION in onJobResult():" << e.what();
        setStatus("Catalog operation failed with exception");
        emit catalogOperationError("Catalog operation failed with exception");
        cleanupJob();
    }

    // Clean up update state
    if (m_catalogUpdateInProgress) {
        qDebug() << "Catalog update completed - clearing update state for catalog" << m_updatingCatalogID;
        m_catalogUpdateInProgress = false;
        m_updatingCatalogID = 0;
    }

    // Reset hard stop flag
    m_hardStopRequested.storeRelease(0);

    qDebug() << "=== CatalogManager::onJobResult() EXIT ===";

}

void CatalogManager::onJobPercent()
{
    if (m_currentJob) {
        unsigned long percent = m_currentJob->percent();
        qDebug() << "CatalogManager::onJobPercent received percent:" << percent;
        setProgress(static_cast<int>(percent));
    }
}

void CatalogManager::onJobInfoMessage(KJob *job, const QString &message)
{
    Q_UNUSED(job);
    setStatus(message);
}

void CatalogManager::setCatalogOperationRunning(bool running)
{
    if (m_catalogOperationRunning != running) {
        m_catalogOperationRunning = running;
        emit catalogOperationRunningChanged();
    }
}

void CatalogManager::setProgress(int progress)
{
    if (m_progress != progress) {
        qDebug() << "CatalogManager::setProgress updating from" << m_progress << "to" << progress;
        m_progress = progress;
        emit progressChanged();
    }
}

void CatalogManager::setStatus(const QString &status)
{
    if (m_status != status) {
        m_status = status;
        emit statusChanged();
    }
}

void CatalogManager::setCurrentCatalogName(const QString &catalogName)
{
    if (m_currentCatalogName != catalogName) {
        m_currentCatalogName = catalogName;
        emit currentCatalogNameChanged();
    }
}

void CatalogManager::setFilesProcessed(qint64 processed)
{
    if (m_filesProcessed != processed) {
        m_filesProcessed = processed;
        emit filesProcessedChanged();
    }
}

void CatalogManager::setTotalFiles(qint64 total)
{
    if (m_totalFiles != total) {
        m_totalFiles = total;
        emit totalFilesChanged();
    }
}

void CatalogManager::setCurrentPath(const QString &path)
{
    if (m_currentPath != path) {
        m_currentPath = path;
        emit currentPathChanged();
    }
}

void CatalogManager::cleanupJob()
{
    if (m_currentJob) {
        qDebug() << "Cleaning up catalog job...";
        m_currentJob->deleteLater();
        m_currentJob = nullptr;
        qDebug() << "Catalog job cleanup complete";
    }
}

void CatalogManager::handleBatchCatalogCompletion()
{
    qDebug() << "=== CatalogManager::handleBatchCatalogCompletion() ===";

    if (!m_inBatchMode) return;

    // ADD THIS CHECK FIRST - before the gentle stop check:
    if (m_hardStopRequested.loadAcquire()) {
        qDebug() << "Hard stop requested - finishing batch operation early (same as gentle stop)";

        // For batch operations, hard stop = gentle stop (finish current, then stop)
        // Calculate remaining active catalogs that were never processed
        int remainingActiveCatalogs = m_batchCatalogs.size() - m_batchCurrentIndex - 1;
        if (remainingActiveCatalogs > 0) {
            qDebug() << "Adding" << remainingActiveCatalogs << "stopped active catalogs to skipped count";
            m_skippedCatalogs += remainingActiveCatalogs;
        }

        QList<qint64> earlyStopList;
        earlyStopList << 1 << m_globalUpdateTotalFiles << m_globalUpdateDeltaFiles
                      << m_globalUpdateTotalSize << m_globalUpdateDeltaSize
                      << m_updatedCatalogs << m_skippedCatalogs;
        for (int i = 7; i < 14; ++i) earlyStopList << 0;

        emit batchNeedsUIReport(nullptr, earlyStopList, "list");
        m_hardStopRequested.storeRelease(0);  // Reset flag
        resetBatchState();
        emit batchOperationCompleted();
        return;
    }

    if (m_gentleStopRequested.loadAcquire()) {
        qDebug() << "Gentle stop requested - finishing batch operation early";

        // Calculate remaining active catalogs that were never processed
        int remainingActiveCatalogs = m_batchCatalogs.size() - m_batchCurrentIndex - 1;
        if (remainingActiveCatalogs > 0) {
            qDebug() << "Adding" << remainingActiveCatalogs << "stopped active catalogs to skipped count";
            m_skippedCatalogs += remainingActiveCatalogs;
        }

        QList<qint64> earlyStopList;
        earlyStopList << 1 << m_globalUpdateTotalFiles << m_globalUpdateDeltaFiles
                      << m_globalUpdateTotalSize << m_globalUpdateDeltaSize
                      << m_updatedCatalogs << m_skippedCatalogs;
        for (int i = 7; i < 14; ++i) earlyStopList << 0;

        emit batchNeedsUIReport(nullptr, earlyStopList, "list");
        m_gentleStopRequested.storeRelease(0);
        resetBatchState();
        emit batchOperationCompleted();
        return;
    }

    // Just handle global statistics update and batch progression
    m_updatedCatalogs++; // Assume success if we got here
    m_batchCurrentIndex++;
    startCurrentBatchCatalog();
}

// Add these methods to catalogmanager.cpp:

void CatalogManager::initializeBatchOperation(const QList<Device*>& catalogs, const QString& databaseMode, const QString& collectionFolder)
{
    qDebug() << "CatalogManager::initializeBatchOperation with" << catalogs.size() << "catalogs";
    qDebug() << "Database mode:" << databaseMode << "Collection folder:" << collectionFolder;

    // Clear existing batch state
    qDeleteAll(m_batchCatalogs);
    m_batchCatalogs.clear();

    // Reset stop flags
    m_gentleStopRequested.storeRelease(0);
    m_hardStopRequested.storeRelease(0);

    // Copy the catalog list and parameters
    m_batchCatalogs = catalogs;
    m_batchDatabaseMode = databaseMode;
    m_batchCollectionFolder = collectionFolder;
    m_batchCurrentIndex = 0;
    m_inBatchMode = true;

    // Reset global statistics
    m_globalUpdateTotalFiles = 0;
    m_globalUpdateDeltaFiles = 0;
    m_globalUpdateTotalSize = 0;
    m_globalUpdateDeltaSize = 0;
    m_updatedCatalogs = 0;
    m_skippedCatalogs = 0;
}

void CatalogManager::resetBatchState()
{
    qDebug() << "CatalogManager::resetBatchState";

    // Clean up batch catalogs
    qDeleteAll(m_batchCatalogs);
    m_batchCatalogs.clear();
    m_batchCurrentIndex = 0;
    m_inBatchMode = false;

    // Clear batch parameters
    m_batchDatabaseMode.clear();
    m_batchCollectionFolder.clear();

    // Reset global statistics
    m_globalUpdateTotalFiles = 0;
    m_globalUpdateDeltaFiles = 0;
    m_globalUpdateTotalSize = 0;
    m_globalUpdateDeltaSize = 0;
    m_updatedCatalogs = 0;
    m_skippedCatalogs = 0;
}

void CatalogManager::updateGlobalStatistics(qint64 totalFiles, qint64 deltaFiles, qint64 totalSize, qint64 deltaSize)
{
    m_globalUpdateTotalFiles += totalFiles;
    m_globalUpdateDeltaFiles += deltaFiles;
    m_globalUpdateTotalSize += totalSize;
    m_globalUpdateDeltaSize += deltaSize;
}

void CatalogManager::startCurrentBatchCatalog()
{
    qDebug() << "=== CatalogManager::startCurrentBatchCatalog() ENTRY ===";
    qDebug() << "Batch mode:" << m_inBatchMode;
    qDebug() << "Current index:" << m_batchCurrentIndex;
    qDebug() << "Total catalogs:" << m_batchCatalogs.size();

    // Safety check
    if (!m_inBatchMode) {
        qDebug() << "ERROR: Called startCurrentBatchCatalog but not in batch mode!";
        return;
    }

    // Check hard stop request first, before the gentle stop check,
    if (m_hardStopRequested.loadAcquire()) {
        qDebug() << "Hard stop requested - stopping batch operation";
        finishBatchOperation();
        return;
    }

    // Then check gentle stop request.
    if (m_gentleStopRequested.loadAcquire()) {
        qDebug() << "Gentle stop requested - not starting new catalog";
        handleBatchCatalogCompletion();
        return;
    }

    // Check if the process is done
    if (m_batchCurrentIndex >= m_batchCatalogs.size()) {
        qDebug() << "Batch complete - all catalogs processed";
        finishBatchOperation();
        return;
    }

    // CRITICAL CHECK: Make sure CatalogManager is ready
    if (catalogOperationRunning()) {
        qDebug() << "CatalogManager still running - deferring start";
        QTimer::singleShot(100, this, [this]() {
            qDebug() << "Retry timer fired - attempting to start catalog";
            startCurrentBatchCatalog();
        });
        return;
    }

    // Get current catalog
    Device* currentDevice = m_batchCatalogs[m_batchCurrentIndex];

    qDebug() << "Starting catalog" << (m_batchCurrentIndex + 1) << "of" << m_batchCatalogs.size();
    qDebug() << "Catalog name:" << currentDevice->name;
    qDebug() << "Catalog ID:" << currentDevice->ID;

    // Emit signal to notify UI about batch progress
    emit batchCatalogStarted(currentDevice, m_batchCurrentIndex + 1, m_batchCatalogs.size());

    // Create catalog job stoppable for this update operation
    CatalogJobStoppable* catalogJobStoppable = new CatalogJobStoppable(this);

    // Start the update for this catalog using existing method
    startCatalogJobStoppable(
        catalogJobStoppable,
        currentDevice,
        CatalogJobStoppable::UpdateCatalog,
        m_batchDatabaseMode,
        m_batchCollectionFolder
        );

    qDebug() << "Catalog update started for:" << currentDevice->name;
    qDebug() << "=== CatalogManager::startCurrentBatchCatalog() EXIT ===";
}

void CatalogManager::finishBatchOperation()
{
    qDebug() << "=== CatalogManager::finishBatchOperation() ENTRY ===";
    qDebug() << "Updated catalogs:" << m_updatedCatalogs;
    qDebug() << "Skipped catalogs:" << m_skippedCatalogs;
    qDebug() << "Total files:" << m_globalUpdateTotalFiles;
    qDebug() << "Total size:" << m_globalUpdateTotalSize;

    // Reset stop flags
    m_gentleStopRequested.storeRelease(0);
    m_hardStopRequested.storeRelease(0);

    // Create final global report (same format as original)
    QList<qint64> globalList;
    globalList << 1;  // Success code
    globalList << m_globalUpdateTotalFiles;
    globalList << m_globalUpdateDeltaFiles;
    globalList << m_globalUpdateTotalSize;
    globalList << m_globalUpdateDeltaSize;
    globalList << m_updatedCatalogs;
    globalList << m_skippedCatalogs;
    globalList << 0;
    globalList << 0;
    globalList << 0;
    globalList << 0;
    globalList << 0;
    globalList << 0;
    globalList << 0;
    globalList << 0;

    // Emit signal for UI to show final global report
    emit batchNeedsUIReport(nullptr, globalList, "list");

    // Clean up batch state
    resetBatchState();

    // Emit completion signal
    emit batchOperationCompleted();

    qDebug() << "=== CatalogManager::finishBatchOperation() EXIT ===";
}

// For backward compatibility - implement older method signatures but delegate to new implementation
void CatalogManager::startNextCatalogUpdate()
{
    qDebug() << "CatalogManager::startNextCatalogUpdate() - delegating to startCurrentBatchCatalog()";
    startCurrentBatchCatalog();
}

void CatalogManager::processNextCatalogUpdate()
{
    qDebug() << "CatalogManager::processNextCatalogUpdate() - incrementing and starting next";
    m_batchCurrentIndex++;
    startCurrentBatchCatalog();
}

void CatalogManager::requestHardStop()
{
    qDebug() << "CatalogManager::requestHardStop() - Hard stop requested";
    m_hardStopRequested.storeRelease(1);

    if (m_inBatchMode) {
        setStatus("Hard stopping after current catalog completes...");
        // Also stop the currently running catalog
        if (m_currentJob) {
            qDebug() << "Stopping current catalog in batch hard stop";
            m_currentJob->requestHardStop(); // Use hard stop for the individual job
        }
    } else {
        setStatus("Hard stopping catalog operation...");

        // For single operations, stop immediately
        if (m_currentJob) {
            m_currentJob->requestHardStop();
        }
    }
}

QString CatalogManager::getEffectiveCatalogID(int catalogID) const
{
    // During update, show old data (temp ID) for searches to maintain stable results
    if (m_catalogUpdateInProgress && catalogID == m_updatingCatalogID) {
        // Return temp ID if we're updating this catalog
        return QString::number(catalogID + 999999);
    }
    return QString::number(catalogID);
}

bool CatalogManager::isCatalogBeingUpdated(int catalogID) const
{
    return m_catalogUpdateInProgress && catalogID == m_updatingCatalogID;
}
