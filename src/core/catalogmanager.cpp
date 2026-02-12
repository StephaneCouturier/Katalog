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
#include <QCoreApplication>

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
    // Save for cancelled/completed messages
    m_lastOperationType = operationType;
    m_lastCatalogName = targetDevice->catalog->name;

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
    m_currentOperationType = operationType;
    m_currentJob->setDatabaseMode(databaseMode);
    m_currentJob->setCollectionFolder(collectionFolder);

    // Connect job signals with enhanced progress handling (same pattern as SearchManager)
    connect(m_currentJob, &KJob::result, this, &CatalogManager::onJobResult);

    setOperationPhase(PHASE_IDLE);

    // Enhanced progress connection that handles catalog progress updates
    connect(m_currentJob, &CatalogJob::catalogProgress,
            this, [this](qint64 filesProcessed, qint64 totalFiles, const QString &currentPath) {
                if (m_currentJob && m_currentJob->getCatalogEngine()) {
                    CatalogJobStoppable* engine = m_currentJob->getCatalogEngine();

                    // Phase detection
                    if (currentPath.startsWith("__CATALOG_LOADING__|")) {
                        setOperationPhase(PHASE_LOADING);
                    } else if (currentPath.startsWith("__COUNTING_STATE__|")) {
                        setOperationPhase(PHASE_COUNTING);
                    } else if (currentPath.startsWith("Found ") && currentPath.endsWith(" files.")) {
                        // Counting finished - stay in COUNTING phase
                        if (m_currentPhase != PHASE_COUNTING) {
                            setOperationPhase(PHASE_COUNTING);
                        }
                    } else if (currentPath.startsWith("__SAVING__|")) {
                        setOperationPhase(PHASE_SAVING);
                    } else if (currentPath.startsWith("__FILETYPE_MIGRATION__|")) {
                        setOperationPhase(PHASE_MIGRATING);
                    } else if (currentPath.startsWith("__METADATA_EXTRACTION__|")) {
                        setOperationPhase(PHASE_METADATA_EXTRACTION);
                    } else if (totalFiles > 0 && !currentPath.startsWith("__") &&
                               !currentPath.startsWith("Found ") && currentPath.contains("/")) {
                        setOperationPhase(PHASE_INDEXING);
                    }

                    // Update progress
                    if (filesProcessed >= 0) {
                        setFilesProcessed(filesProcessed);
                        setTotalFiles(totalFiles);
                        setCurrentPath(currentPath);

                        if (totalFiles > 0) {
                            int percent = qMin(100, static_cast<int>((filesProcessed * 100) / totalFiles));
                            setProgress(percent);
                        }

                        setCurrentCatalogName(engine->getCurrentCatalogName());

                        QString operationType = (m_currentJob->getOperationType() == CatalogJobStoppable::CreateCatalog) ?
                                                    "Creating" : "Updating";
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
                    }
                }
            });

    setCatalogOperationRunning(true);
    setProgress(0);
    setFilesProcessed(0);
    setTotalFiles(0);
    setCurrentPath("");
    m_isPaused = false;

    // Start the job
    m_currentJob->startJob();
}

// Replace this method in src/core/catalogmanager.cpp

void CatalogManager::stopCatalogOperation()
{
    qDebug() << "=== CatalogManager::stopCatalogOperation() called ===";

    if (!m_currentJob || !m_catalogOperationRunning) {
        qDebug() << "No operation to stop";
        return;
    }

    qDebug() << "Stopping catalog operation...";

    // SAVE STATE for cancelled message (before reset)
    CatalogJobStoppable::OperationType savedOperationType = m_currentOperationType;
    QString savedCatalogName = m_currentCatalogName;
    qint64 savedFilesProcessed = m_filesProcessed;
    qint64 savedTotalFiles = m_totalFiles;

    setStatus("Catalog operation stopped");
    disconnect(m_currentJob, nullptr, this, nullptr);

    qDebug() << "Calling m_currentJob->kill()";
    m_currentJob->kill();

    // Force immediate cleanup
    qDebug() << "Force cleanup after kill...";
    setCatalogOperationRunning(false);
    setProgress(0);
    setCurrentCatalogName("");
    setFilesProcessed(0);
    setTotalFiles(0);
    setCurrentPath("");
    m_isPaused = false;

    if (m_currentJob) {
        m_currentJob->deleteLater();
        m_currentJob = nullptr;
        qDebug() << "Forced cleanup complete";
    }

    // RESTORE state temporarily so signal handlers can read it
    m_currentOperationType = savedOperationType;
    m_currentCatalogName = savedCatalogName;
    m_filesProcessed = savedFilesProcessed;
    m_totalFiles = savedTotalFiles;

    emit catalogOperationCancelled();

    setOperationPhase(PHASE_IDLE);

    // NOW clear the restored state
    m_currentCatalogName = "";
    m_filesProcessed = 0;
    m_totalFiles = 0;

    // Reset stop flags
    m_gentleStopRequested.storeRelease(0);
    m_hardStopRequested.storeRelease(0);

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

CatalogJobStoppable::OperationType CatalogManager::currentOperationType() const
{
    // if (m_currentJob) {
    //     return m_currentJob->getOperationType();
    // }
    // return CatalogJobStoppable::UpdateCatalog;  // Default
    return m_currentOperationType;
}

void CatalogManager::requestGentleStop()
{
    qDebug() << "CatalogManager::requestGentleStop() - Gentle stop requested";
    m_gentleStopRequested.storeRelease(1);
    stopCatalogOperation(); // Single operations = immediate stop
}

void CatalogManager::onJobResult(KJob *job)
{
    qDebug() << "=== DIAGNOSTIC: CatalogManager::onJobResult() ENTRY ===";
    qDebug() << "=== DIAGNOSTIC: Job error code:" << job->error();
    qDebug() << "=== DIAGNOSTIC: KilledJobError constant:" << KJob::KilledJobError;

    try {
        if (job->error() == KJob::KilledJobError) {
            // Phase is already set correctly from where cancel was triggered
            // Don't change it - just emit signal so handlers can read lastPhase
            qDebug() << "Job cancelled - lastPhase:" << m_lastPhase << "currentPhase:" << m_currentPhase;
            emit catalogOperationCancelled();

        } else if (job->error()) {
            QString errorMsg = QString("Catalog operation failed: %1").arg(job->errorString());
            emit catalogOperationError(errorMsg);

        } else {
            // SUCCESS - save final state BEFORE emitting signals

            // Get final counts from job results
            if (m_currentJob && m_currentJob->getCatalogEngine()) {
                CatalogJobStoppable* engine = m_currentJob->getCatalogEngine();
                QList<qint64> results = engine->getResults();

                if (results.size() >= 2 && results[1] > 0) {
                    m_lastFilesProcessed = results[1];
                    m_lastTotalFiles = results[1];
                }
            }

            // Fallback to current values if results unavailable
            if (m_lastFilesProcessed == 0) {
                m_lastFilesProcessed = m_filesProcessed;
                m_lastTotalFiles = m_totalFiles;
            }

            m_lastCurrentPath = m_currentPath;
            setOperationPhase(PHASE_COMPLETING);

            qDebug() << "Saved final state:" << m_lastFilesProcessed << "/" << m_lastTotalFiles;

            // Handle batch mode
            if (m_inBatchMode) {
                CatalogJobStoppable* catalogEngine = getCurrentCatalogEngine();
                if (catalogEngine) {
                    QList<qint64> results = catalogEngine->getResults();
                    if (results.size() >= 5 && results[0] == 1) {
                        m_updatedCatalogs++;
                    } else {
                        m_skippedCatalogs++;
                    }
                } else {
                    m_skippedCatalogs++;
                }

                m_batchCurrentIndex++;
            }

            emit catalogOperationCompleted();
        }

        // Clean up and reset state
        setCatalogOperationRunning(false);
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

    // Reset hard stop flag and phase
    m_hardStopRequested.storeRelease(0);
    setOperationPhase(PHASE_IDLE);  // Reset phase after all handlers have run

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
        m_lastFilesProcessed = processed;  // Keep last value
        emit filesProcessedChanged();
    }
}

void CatalogManager::setTotalFiles(qint64 total)
{
    if (m_totalFiles != total) {
        m_totalFiles = total;
        m_lastTotalFiles = total;  // Keep last value
        emit totalFilesChanged();
    }
}

void CatalogManager::setCurrentPath(const QString &path)
{
    if (m_currentPath != path) {
        m_currentPath = path;
        m_lastCurrentPath = path;
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

    // Check hard stop before the gentle stop
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

        m_hardStopRequested.storeRelease(0);  // Reset flag
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

        m_gentleStopRequested.storeRelease(0);
        return;
    }

    // Just handle global statistics update and batch progression
    m_updatedCatalogs++; // Assume success if we got here
    m_batchCurrentIndex++;
}

void CatalogManager::requestHardStop()
{
    qDebug() << "CatalogManager::requestHardStop() - Hard stop requested";
    m_hardStopRequested.storeRelease(1);
    setStatus("Hard stopping catalog operation...");
    if (m_currentJob) {
        m_currentJob->requestHardStop();
    }
}

void CatalogManager::setOperationPhase(OperationPhase phase)
{
    if (m_currentPhase != phase) {
        m_lastPhase = m_currentPhase;  // Remember previous phase
        m_currentPhase = phase;

        // Debug output with readable names - WITH BOUNDS CHECKING
        QStringList phaseNames = {"IDLE", "LOADING", "COUNTING", "INDEXING", "MIGRATING", "METADATA_EXTRACTION", "COMPLETING"};

        // Bounds check to prevent crash
        int lastPhaseIndex = static_cast<int>(m_lastPhase);
        int currentPhaseIndex = static_cast<int>(m_currentPhase);

        QString lastPhaseName = (lastPhaseIndex >= 0 && lastPhaseIndex < phaseNames.size())
                                    ? phaseNames[lastPhaseIndex]
                                    : QString("UNKNOWN(%1)").arg(lastPhaseIndex);
        QString currentPhaseName = (currentPhaseIndex >= 0 && currentPhaseIndex < phaseNames.size())
                                       ? phaseNames[currentPhaseIndex]
                                       : QString("UNKNOWN(%1)").arg(currentPhaseIndex);

        qDebug() << "Operation phase changed:" << lastPhaseName << "->" << currentPhaseName;
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
