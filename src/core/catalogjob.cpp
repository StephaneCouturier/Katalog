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
// Purpose:     KJob wrapper implementation for catalog operations
// Description: Simple KJob implementation that wraps CatalogJobStoppable
//              Follows SearchJob pattern exactly for reliability
// Author:      Stephane Couturier
/////////////////////////////////////////////////////////////////////////////
*/

#include "catalogjob.h"
#include <QDebug>

CatalogJob::CatalogJob(QObject *parent)
    : KJob(parent)
{
    setCapabilities(Killable | Suspendable);
    setTotalAmount(KJob::Files, 0); // Will be set when we know the estimate
    qDebug() << "CatalogJob created";
}

void CatalogJob::setCatalogJobStoppable(CatalogJobStoppable *catalogEngine)
{
    if (m_catalogEngine) {
        // Disconnect previous engine
        disconnect(m_catalogEngine, nullptr, this, nullptr);
    }

    m_catalogEngine = catalogEngine;

    if (m_catalogEngine) {
        // Connect to catalog progress signals
        connect(m_catalogEngine, &CatalogJobStoppable::catalogProgress,
                this, &CatalogJob::onCatalogProgress);
    }
}

void CatalogJob::setTargetDevice(Device *device)
{
    m_targetDevice = device;
}

void CatalogJob::setOperationType(CatalogJobStoppable::OperationType operationType)
{
    m_operationType = operationType;
}

void CatalogJob::setDatabaseMode(const QString &databaseMode)
{
    m_databaseMode = databaseMode;
}

void CatalogJob::setCollectionFolder(const QString &collectionFolder)
{
    m_collectionFolder = collectionFolder;
}

void CatalogJob::startJob()
{
    if (!m_targetDevice) {
        setError(UserDefinedError);
        setErrorText("No target device configured");
        emitResult();
        return;
    }

    if (!m_catalogEngine) {
        setError(UserDefinedError);
        setErrorText("No catalog engine configured");
        emitResult();
        return;
    }

    // Call the protected start() method
    start();
}

void CatalogJob::requestHardStop()
{
    qDebug() << "CatalogJob::requestHardStop() - Hard stop requested";

    // Forward the hard stop request to the catalog engine
    if (m_catalogEngine) {
        m_catalogEngine->requestHardStop();
    }
}

void CatalogJob::start()
{
    qDebug() << "Starting catalog job...";

    emit catalogStarted();

    // Configure the catalog engine
    m_catalogEngine->configureOperation(m_targetDevice,
                                        m_operationType,
                                        m_databaseMode,
                                        m_collectionFolder);

    // Set total amount if we have an estimate
    if (m_catalogEngine->getCountedTotalFiles() > 0) {
        setTotalAmount(KJob::Files, m_catalogEngine->getCountedTotalFiles());
    }

    // Use a timer to execute catalog operation in next event loop iteration
    // This ensures the KJob signals are properly emitted (same pattern as SearchJob)
    m_executeTimer = new QTimer(this);
    m_executeTimer->setSingleShot(true);
    connect(m_executeTimer, &QTimer::timeout, this, &CatalogJob::executeCatalogOperation);
    m_executeTimer->start(0);
}

void CatalogJob::executeCatalogOperation()
{
    if (!m_catalogEngine || !m_targetDevice) {
        setError(UserDefinedError);
        setErrorText("Catalog configuration invalid");
        emitResult();
        return;
    }

    try {
        qDebug() << "=== CatalogJob::executeCatalogOperation() START ===";

        // Execute the catalog operation
        qDebug() << "About to call m_catalogEngine->processCatalog()";
        m_catalogEngine->processCatalog();
        qDebug() << "m_catalogEngine->processCatalog() returned successfully";

        // Check if operation was stopped
        if (m_catalogEngine->wasStopRequested()) {
            qDebug() << "Stop was requested during catalog operation";
            setError(KilledJobError);
            setErrorText("Catalog operation was cancelled");
            emitResult();
            return;
        }

        // If we get here, catalog operation completed successfully
        qDebug() << "=== Catalog job completed successfully! ===";

        qDebug() << "About to emit catalogFinished()";
        emit catalogFinished();
        qDebug() << "catalogFinished() emitted successfully";

        qDebug() << "About to call emitResult()";
        emitResult();
        qDebug() << "emitResult() called successfully";

    } catch (const std::exception &e) {
        qDebug() << "=== EXCEPTION in executeCatalogOperation():" << e.what() << "===";
        setError(UserDefinedError);
        setErrorText(QString("Catalog operation failed: %1").arg(e.what()));
        emitResult();
    } catch (...) {
        qDebug() << "=== UNKNOWN EXCEPTION in executeCatalogOperation() ===";
        setError(UserDefinedError);
        setErrorText("Catalog operation failed with unknown error");
        emitResult();
    }

    qDebug() << "=== CatalogJob::executeCatalogOperation() END ===";
}

bool CatalogJob::doKill()
{
    qDebug() << "=== CatalogJob::doKill() called ===";

    if (m_executeTimer) {
        qDebug() << "Stopping execute timer";
        m_executeTimer->stop();
        m_executeTimer->setParent(nullptr);
        m_executeTimer->deleteLater();
        m_executeTimer = nullptr;
    }

    // Stop the catalog engine
    if (m_catalogEngine) {
        qDebug() << "Stopping CatalogJobStoppable engine";
        m_catalogEngine->stopCatalogOperation();
        qDebug() << "CatalogJobStoppable::stopCatalogOperation() called";
    }

    setError(KilledJobError);
    setErrorText("Catalog operation was cancelled by user");

    qDebug() << "=== CatalogJob::doKill() complete ===";
    return true;
}

bool CatalogJob::doSuspend()
{
    qDebug() << "Suspending catalog job...";

    QMutexLocker locker(&m_mutex);
    m_suspended = true;

    // Pause the catalog engine if available
    if (m_catalogEngine) {
        m_catalogEngine->pauseCatalogOperation();
    }

    return true;
}

bool CatalogJob::doResume()
{
    qDebug() << "Resuming catalog job...";

    QMutexLocker locker(&m_mutex);
    m_suspended = false;

    // Resume the catalog engine if available
    if (m_catalogEngine) {
        m_catalogEngine->resumeCatalogOperation();
    }

    return true;
}

void CatalogJob::onCatalogProgress(qint64 filesProcessed, qint64 totalFiles, const QString &currentPath)
{
    // qDebug() << "CatalogJob::onCatalogProgress received:"
    //          << "processed=" << filesProcessed
    //          << "total=" << totalFiles
    //          << "path=" << currentPath;

    // Emit our own signal for CatalogManager to handle
    emit catalogProgress(filesProcessed, totalFiles, currentPath);

    // Update KJob progress tracking
    if (filesProcessed >= 0) {
        //qDebug() << "CatalogJob updating progress to:" << filesProcessed;
        m_lastFilesProcessed = filesProcessed;
        setProcessedAmount(KJob::Files, filesProcessed);

        if (totalFiles > 0) {
            qint64 calculation = (static_cast<qint64>(filesProcessed) * 100) / totalFiles;
            qint64 percent = qMin(static_cast<qint64>(100), calculation);
            qDebug() << "CatalogJob emitting percent:" << percent;
            emitPercent(static_cast<unsigned long>(percent), 100);
        }
    }
}
