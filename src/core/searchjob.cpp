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
// File Name:   searchjob.cpp
// Purpose:     Class/model to execute a search using KJob framework
// Description:
// Author:      Stephane Couturier
/////////////////////////////////////////////////////////////////////////////
*/

#include "searchjob.h"
#include "searchjobstoppable.h"
#include <QDebug>

SearchJob::SearchJob(QObject *parent)
    : KJob(parent)
{
    setCapabilities(Killable | Suspendable);
    setTotalAmount(KJob::Files, 0); // Will be set when we know the estimate
}
//----------------------------------------------------------------------
void SearchJob::setSearchJobStoppable(SearchJobStoppable *searchEngine)
{
    if (m_searchEngine) {
        // Disconnect previous engine
        disconnect(m_searchEngine, nullptr, this, nullptr);
    }

    m_searchEngine = searchEngine;

    if (m_searchEngine) {
        // Connect to search progress signals
        connect(m_searchEngine, &Search::searchProgress, this, &SearchJob::onSearchProgress);
    }
}
//----------------------------------------------------------------------
void SearchJob::setTargetDevice(Device *device)
{
    m_targetDevice = device;
}
//----------------------------------------------------------------------
void SearchJob::startJob()
{
    if (!m_targetDevice) {
        setError(UserDefinedError);
        setErrorText("No target device configured");
        emitResult();
        return;
    }

    // Call the protected start() method
    start();
}
//----------------------------------------------------------------------
void SearchJob::start()
{
    qDebug() << "Starting search job...";

    emit searchStarted();

    // Initialize progress tracking
    m_searchEngine->initializeProgressTracking(m_targetDevice);

    // Set total amount if we have an estimate
    if (m_searchEngine->estimatedTotalFiles > 0) {
        setTotalAmount(KJob::Files, m_searchEngine->estimatedTotalFiles);
    }

    // Clear any previous results
    m_searchEngine->clearResults();

    // Setup search patterns
    m_searchEngine->prepareSearchPatterns();

    // Use a timer to execute search in next event loop iteration
    // This ensures the KJob signals are properly emitted
    m_executeTimer = new QTimer(this);
    m_executeTimer->setSingleShot(true);
    connect(m_executeTimer, &QTimer::timeout, this, &SearchJob::executeSearch);
    m_executeTimer->start(0);
}
//----------------------------------------------------------------------
void SearchJob::executeSearch()
{
    qDebug() << "=== SearchJob::executeSearch() START ===";

    // Check if search was killed before doing anything else
    if (m_isKilled.loadAcquire()) {
        qDebug() << "Job was killed before executeSearch started - exiting without emitResult()";
        return;
    }

    if (!m_searchEngine || !m_targetDevice) {
        setError(UserDefinedError);
        setErrorText("Search configuration invalid");
        // Only emit if not killed
        if (!m_isKilled.loadAcquire()) {
            emitResult();
        }
        return;
    }

    try {
        // Execute the search
        SearchJobStoppable* searchJobStoppable = static_cast<SearchJobStoppable*>(m_searchEngine);
        searchJobStoppable->searchFiles(m_targetDevice);

        // After searchFiles() returns, check if search was killed
        // If search was killed, doKill() will have already set the error and KJob will handle cleanup
        // DO NOT call emitResult() - let KJob's own machinery handle it
        if (m_isKilled.loadAcquire()) {
            qDebug() << "Job was killed during search - exiting without emitResult()";
            return;
        }

        // Check if search was stopped normally
        if (searchJobStoppable->wasStopRequested()) {
            setError(KilledJobError);
            setErrorText("Search was cancelled");
            // Only emit if not killed
            if (!m_isKilled.loadAcquire()) {
                emitResult();
            }
            return;
        }

        // If we get here, search completed successfully
        qDebug() << "Search job completed successfully!";

        // Only emit signals if not killed
        if (!m_isKilled.loadAcquire()) {
            emit searchFinished();
            emitResult();
        }

    } catch (const std::exception &e) {
        qDebug() << "Exception in executeSearch:" << e.what();
        setError(UserDefinedError);
        setErrorText(QString("Search failed: %1").arg(e.what()));
        // Only emit if not killed
        if (!m_isKilled.loadAcquire()) {
            emitResult();
        }
    } catch (...) {
        qDebug() << "Unknown exception in executeSearch";
        setError(UserDefinedError);
        setErrorText("Search failed with unknown error");
        // Only emit if not killed
        if (!m_isKilled.loadAcquire()) {
            emitResult();
        }
    }

    qDebug() << "=== SearchJob::executeSearch() END ===";
}
//----------------------------------------------------------------------
bool SearchJob::doKill()
{
    qDebug() << "=== SearchJob::doKill() called ===";

    // Set the killed flag FIRST before doing anything else
    m_isKilled.storeRelease(1);
    qDebug() << "Killed flag set to 1";

    if (m_executeTimer) {
        qDebug() << "Stopping execute timer";
        m_executeTimer->stop();
        m_executeTimer->setParent(nullptr);
        m_executeTimer->deleteLater();
        m_executeTimer = nullptr;
    }

    // Stop the search engine
    if (m_searchEngine) {
        qDebug() << "Stopping SearchJobStoppable engine";
        SearchJobStoppable* searchJobStoppable = static_cast<SearchJobStoppable*>(m_searchEngine);
        if (searchJobStoppable) {
            searchJobStoppable->stopSearch();
            qDebug() << "SearchJobStoppable::stopSearch() called";
        }
    }

    setError(KilledJobError);
    setErrorText("Search was cancelled by user");

    qDebug() << "=== SearchJob::doKill() complete ===";

    // Return true to indicate kill was successful
    // KJob will handle calling finishJob() and emitting signals
    return true;
}
//----------------------------------------------------------------------
bool SearchJob::doSuspend()
{
    qDebug() << "Suspending search job...";

    QMutexLocker locker(&m_mutex);
    m_suspended = true;

    // Note: For now, we don't have a pause mechanism in the search engines
    // This could be implemented by adding a suspend/resume mechanism to Search classes

    return true;
}
//----------------------------------------------------------------------
bool SearchJob::doResume()
{
    qDebug() << "Resuming search job...";

    QMutexLocker locker(&m_mutex);
    m_suspended = false;

    return true;
}
//----------------------------------------------------------------------
void SearchJob::onSearchProgress(int filesProcessed)
{
    // If search kill was requested, do not emit anything
    if (m_isKilled.loadAcquire()) {
        qDebug() << "SearchJob::onSearchProgress - job killed, ignoring progress update";
        return;
    }

    qDebug() << "SearchJob::onSearchProgress received:" << filesProcessed;

    // Emit our own signal for SearchManager to handle
    emit searchProgress(filesProcessed);

    // Handle special progress values
    if (filesProcessed == -1) {
        // Search interrupted
        setError(KilledJobError);
        setErrorText("Search was interrupted");
        // Only emit if not killed
        if (!m_isKilled.loadAcquire()) {
            emitResult();
        }
        return;
    }

    if (filesProcessed == -2) {
        // Catalog loading started
        emit catalogLoadingStarted(m_searchEngine->currentCatalogName);
        return;
    }

    if (filesProcessed == -3) {
        // Catalog loading finished
        emit catalogLoadingFinished();
        return;
    }

    if (filesProcessed == -4) {
        // Catalog loading progress - forward it to MainWindow
        emit searchProgress(filesProcessed);
        return;
    }

    // Regular progress update
    if (filesProcessed >= 0) {
        qDebug() << "SearchJob updating progress to:" << filesProcessed;
        m_lastFilesProcessed = filesProcessed;
        setProcessedAmount(KJob::Files, filesProcessed);

        if (m_searchEngine->estimatedTotalFiles > 0) {
            qint64 calculation = (static_cast<qint64>(filesProcessed) * 100) / m_searchEngine->estimatedTotalFiles;
            qint64 percent = qMin(static_cast<qint64>(100), calculation);
            qDebug() << "SearchJob emitting percent:" << percent;
            emitPercent(static_cast<unsigned long>(percent), 100);
        }
    }
}
//----------------------------------------------------------------------
