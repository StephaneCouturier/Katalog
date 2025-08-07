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
// Purpose:     Class/model to manage catalog operations using KJob framework
// Description: Manages catalog creation/update with progress reporting and cancellation
// Author:      Stephane Couturier
/////////////////////////////////////////////////////////////////////////////
*/

#include "catalogmanager.h"
#include "src/core/catalogjob.h"
#include <QDebug>

CatalogManager::CatalogManager(QObject *parent)
    : QObject(parent)
{
    qDebug() << "CatalogManager created";
}

CatalogManager::~CatalogManager()
{
    if (m_currentJob) {
        qDebug() << "CatalogManager destructor: cleaning up job";
        cleanupJob();
    }
    qDebug() << "CatalogManager destroyed";
}

void CatalogManager::startCreateCatalog(Device *device, const QString &databaseMode, const QString &collectionFolder)
{
    if (m_currentJob) {
        qDebug() << "CatalogManager::startCreateCatalog() called but operation already running";
        return;
    }

    if (!device || !device->catalog) {
        qDebug() << "CatalogManager::startCreateCatalog() called with null device or catalog";
        return;
    }

    qDebug() << "CatalogManager::startCreateCatalog() - Starting catalog creation for:" << device->catalog->name;

    // Create new job
    m_currentJob = new CatalogJob(device, CatalogJob::CreateCatalog, databaseMode, collectionFolder, this);

    // Connect job signals
    connect(m_currentJob, &KJob::result, this, &CatalogManager::onJobResult);
    connect(m_currentJob, &KJob::percent, this, &CatalogManager::onJobPercent);
    connect(m_currentJob, &KJob::infoMessage, this, &CatalogManager::onJobInfoMessage);
    connect(m_currentJob, &CatalogJob::filesProcessedUpdate, this, &CatalogManager::onFilesProcessedUpdate);
    connect(m_currentJob, &CatalogJob::progressDetailsUpdate, this, &CatalogManager::progressUpdate);

    // Set initial state
    setCatalogOperationRunning(true);
    setProgress(0);
    setStatus("Starting catalog creation...");
    setCurrentPath("");
    setFilesProcessed(0);
    setTotalFiles(0);

    // Start the job
    m_currentJob->start();

    qDebug() << "CatalogManager::startCreateCatalog() - Job started";
}

void CatalogManager::startUpdateCatalog(Device *device, const QString &databaseMode, const QString &collectionFolder)
{
    if (m_currentJob) {
        qDebug() << "CatalogManager::startUpdateCatalog() called but operation already running";
        return;
    }

    if (!device || !device->catalog) {
        qDebug() << "CatalogManager::startUpdateCatalog() called with null device or catalog";
        return;
    }

    qDebug() << "CatalogManager::startUpdateCatalog() - Starting catalog update for:" << device->catalog->name;

    // Create new job
    m_currentJob = new CatalogJob(device, CatalogJob::UpdateCatalog, databaseMode, collectionFolder, this);

    // Connect job signals
    connect(m_currentJob, &KJob::result, this, &CatalogManager::onJobResult);
    connect(m_currentJob, &KJob::percent, this, &CatalogManager::onJobPercent);
    connect(m_currentJob, &KJob::infoMessage, this, &CatalogManager::onJobInfoMessage);
    connect(m_currentJob, &CatalogJob::filesProcessedUpdate, this, &CatalogManager::onFilesProcessedUpdate);
    connect(m_currentJob, &CatalogJob::progressDetailsUpdate, this, &CatalogManager::progressUpdate);

    // Set initial state
    setCatalogOperationRunning(true);
    setProgress(0);
    setStatus("Starting catalog update...");
    setCurrentPath("");
    setFilesProcessed(0);
    setTotalFiles(0);

    // Start the job
    m_currentJob->start();

    qDebug() << "CatalogManager::startUpdateCatalog() - Job started";
}

void CatalogManager::stopOperation()
{
    // Placeholder for future stoppable implementation
    if (!m_currentJob) {
        qDebug() << "CatalogManager::stopOperation() called but no operation running";
        return;
    }

    qDebug() << "CatalogManager::stopOperation() - Stop operation requested (not implemented yet)";
    setStatus("Stop requested - not yet implemented");

    // TODO: Implement when adding cancellation support
    // For now, just log the request
}

void CatalogManager::pauseOperation()
{
    // Placeholder for future pausable implementation
    if (!m_currentJob || m_isPaused) {
        return;
    }

    qDebug() << "CatalogManager::pauseOperation() - Pause operation requested (not implemented yet)";
    setStatus("Pause requested - not yet implemented");

    // TODO: Implement when adding pause/resume support
}

void CatalogManager::resumeOperation()
{
    // Placeholder for future resumable implementation
    if (!m_currentJob || !m_isPaused) {
        return;
    }

    qDebug() << "CatalogManager::resumeOperation() - Resume operation requested (not implemented yet)";
    setStatus("Resume requested - not yet implemented");

    // TODO: Implement when adding pause/resume support
}

Device* CatalogManager::getCurrentDevice() const
{
    if (m_currentJob) {
        return m_currentJob->getDevice();
    }
    return nullptr;
}

void CatalogManager::onJobResult(KJob *job)
{
    qDebug() << "=== CatalogManager::onJobResult() START ===";
    qDebug() << "Job completed, error:" << job->error();

    try {
        if (job->error() == KJob::KilledJobError) {
            qDebug() << "Job was killed";
            setStatus("Catalog operation cancelled");
            emit catalogOperationCancelled();
        } else if (job->error()) {
            qDebug() << "Job had error:" << job->errorString();
            QString errorMsg = QString("Catalog operation failed: %1").arg(job->errorString());
            setStatus(errorMsg);
            emit catalogOperationError(errorMsg);
        } else {
            qDebug() << "Job completed successfully";
            // Success
            QString operationType = (m_currentJob->getOperationType() == CatalogJob::CreateCatalog) ? "creation" : "update";
            setStatus(QString("Catalog %1 completed successfully!").arg(operationType));
            qDebug() << "About to emit catalogOperationCompleted signal...";
            emit catalogOperationCompleted();
            qDebug() << "catalogOperationCompleted signal emitted";
        }

        qDebug() << "Setting operation running to false...";
        setCatalogOperationRunning(false);
        qDebug() << "Operation running set to false";

        qDebug() << "Setting progress...";
        setProgress(job->error() ? 0 : 100);
        qDebug() << "Progress set";

        qDebug() << "Clearing current path...";
        setCurrentPath("");
        qDebug() << "Current path cleared";

        qDebug() << "Setting paused to false...";
        m_isPaused = false;
        qDebug() << "Paused set to false";

        qDebug() << "About to cleanup job...";
        cleanupJob();
        qDebug() << "Job cleanup completed";

        qDebug() << "=== CatalogManager::onJobResult() COMPLETED ===";

    } catch (const std::exception& e) {
        qDebug() << "=== EXCEPTION in CatalogManager::onJobResult():" << e.what() << "===";
        //QApplication::restoreOverrideCursor();
    } catch (...) {
        qDebug() << "=== UNKNOWN EXCEPTION in CatalogManager::onJobResult() ===";
        //QApplication::restoreOverrideCursor();
    }
}
void CatalogManager::onJobPercent()
{
    if (m_currentJob) {
        unsigned long percent = m_currentJob->percent();
        qDebug() << "CatalogManager::onJobPercent() received percent:" << percent;
        setProgress(static_cast<int>(percent));
    }
}

void CatalogManager::onJobInfoMessage(KJob *job, const QString &message)
{
    Q_UNUSED(job);
    setStatus(message);
}

void CatalogManager::onFilesProcessedUpdate(qint64 processed, qint64 total, const QString &currentPath)
{
    qDebug() << "CatalogManager::onFilesProcessedUpdate() - Processed:" << processed << "Total:" << total << "Path:" << currentPath;

    setFilesProcessed(processed);
    setTotalFiles(total);
    setCurrentPath(currentPath);

    // Calculate and update progress percentage
    if (total > 0) {
        int progressPercent = static_cast<int>((processed * 100) / total);
        setProgress(progressPercent);
    }
}

// Property setters that emit change signals
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
        qDebug() << "CatalogManager::setProgress() updating from" << m_progress << "to" << progress;
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

void CatalogManager::setCurrentPath(const QString &path)
{
    if (m_currentPath != path) {
        m_currentPath = path;
        emit currentPathChanged();
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

void CatalogManager::cleanupJob()
{
    if (m_currentJob) {
        qDebug() << "CatalogManager::cleanupJob() - Cleaning up job...";
        m_currentJob->deleteLater();
        m_currentJob = nullptr;
        qDebug() << "CatalogManager::cleanupJob() - Job cleanup complete";
    }
}
