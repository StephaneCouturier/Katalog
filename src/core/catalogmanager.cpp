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
    if (m_currentJob) {
        qDebug() << "Catalog operation already running!";
        return;
    }

    if (!catalogEngine || !targetDevice) {
        emit catalogOperationError("Invalid catalog configuration");
        return;
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

void CatalogManager::onJobResult(KJob *job)
{
    qDebug() << "=== CatalogManager::onJobResult() ENTRY ===";
    qDebug() << "Catalog job result received, error:" << job->error();

    try {
        if (job->error() == KJob::KilledJobError) {
            qDebug() << "Job was killed - emitting cancelled signal";
            setStatus("Catalog operation cancelled");
            emit catalogOperationCancelled();
        } else if (job->error()) {
            qDebug() << "Job failed with error:" << job->errorString();
            QString errorMsg = QString("Catalog operation failed: %1").arg(job->errorString());
            setStatus(errorMsg);
            emit catalogOperationError(errorMsg);
        } else {
            qDebug() << "Job completed successfully - emitting completed signal";
            QString operationType = (m_currentJob && m_currentJob->getOperationType() == CatalogJobStoppable::CreateCatalog) ?
                                        "creation" : "update";
            setStatus(QString("Catalog %1 completed successfully!").arg(operationType));

            qDebug() << "About to emit catalogOperationCompleted()";
            emit catalogOperationCompleted();
            qDebug() << "catalogOperationCompleted() emitted successfully";
        }

        qDebug() << "Setting catalog operation running to false";
        setCatalogOperationRunning(false);
        setProgress(job->error() ? 0 : 100);
        setCurrentCatalogName("");
        setCurrentPath("");
        m_isPaused = false;

        qDebug() << "About to cleanup job";
        cleanupJob();
        qDebug() << "Job cleanup completed";

    } catch (const std::exception& e) {
        qDebug() << "=== EXCEPTION in onJobResult():" << e.what() << "===";
    } catch (...) {
        qDebug() << "=== UNKNOWN EXCEPTION in onJobResult() ===";
    }

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

// Add these methods to catalogmanager.cpp:

void CatalogManager::initializeBatchOperation(const QList<Device*>& catalogs, const QString& databaseMode, const QString& collectionFolder)
{
    qDebug() << "CatalogManager::initializeBatchOperation with" << catalogs.size() << "catalogs";
    qDebug() << "Database mode:" << databaseMode << "Collection folder:" << collectionFolder;

    // Clear existing batch state
    qDeleteAll(m_batchCatalogs);
    m_batchCatalogs.clear();

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

    // Check if we're done
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
