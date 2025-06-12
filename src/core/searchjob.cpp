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
#include "src/core/search_memory.h"
#include "searchjobstoppable.h"
#include <QDebug>

SearchJob::SearchJob(QObject *parent)
    : KJob(parent)
{
    setCapabilities(Killable | Suspendable);
    setTotalAmount(KJob::Files, 0); // Will be set when we know the estimate
}
//----------------------------------------------------------------------
void SearchJob::setSearchMemory(SearchMemory *searchEngine)
{
    if (m_searchEngine) {
        // Disconnect previous engine
        disconnect(m_searchEngine, nullptr, this, nullptr);
    }

    m_searchEngine = searchEngine;
    m_engineType = Memory;

    if (m_searchEngine) {
        // Connect to search progress signals
        connect(m_searchEngine, &Search::searchProgress, this, &SearchJob::onSearchProgress);
    }
}
//----------------------------------------------------------------------
void SearchJob::setSearchJobStoppable(SearchJobStoppable *searchEngine)
{
    if (m_searchEngine) {
        // Disconnect previous engine
        disconnect(m_searchEngine, nullptr, this, nullptr);
    }

    m_searchEngine = searchEngine;
    m_engineType = JobStoppable;

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
    if (!m_searchEngine || m_engineType == None) {
        setError(UserDefinedError);
        setErrorText("No search engine configured");
        emitResult();
        return;
    }

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
    if (!m_searchEngine || !m_targetDevice || m_engineType == None) {
        setError(UserDefinedError);
        setErrorText("Search configuration invalid");
        emitResult();
        return;
    }

    try {
        // Execute the search based on engine type
        if (m_engineType == Memory) {
            SearchMemory* searchMemory = static_cast<SearchMemory*>(m_searchEngine);
            searchMemory->searchFiles(m_targetDevice);
        } else if (m_engineType == JobStoppable) {
            SearchJobStoppable* searchJobStoppable = static_cast<SearchJobStoppable*>(m_searchEngine);
            searchJobStoppable->searchFiles(m_targetDevice);

            // Check if search was stopped
            if (searchJobStoppable->wasStopRequested()) {
                setError(KilledJobError);
                setErrorText("Search was cancelled");
                emitResult();
                return;
            }
        } else {
            setError(UserDefinedError);
            setErrorText("Unknown search engine type");
            emitResult();
            return;
        }

        // If we get here, search completed successfully
        qDebug() << "Search job completed successfully!";
        emit searchFinished();
        emitResult();

    } catch (const std::exception &e) {
        setError(UserDefinedError);
        setErrorText(QString("Search failed: %1").arg(e.what()));
        emitResult();
    } catch (...) {
        setError(UserDefinedError);
        setErrorText("Search failed with unknown error");
        emitResult();
    }
}
//----------------------------------------------------------------------
bool SearchJob::doKill()
{
    qDebug() << "Killing search job...";

    if (m_executeTimer) {
        m_executeTimer->stop();
        m_executeTimer->deleteLater();
        m_executeTimer = nullptr;
    }

    // Stop the search engine
    if (m_searchEngine && m_engineType == JobStoppable) {
        SearchJobStoppable* searchJobStoppable = static_cast<SearchJobStoppable*>(m_searchEngine);
        searchJobStoppable->stopSearch();
    }
    // Note: SearchMemory cannot be stopped once started

    setError(KilledJobError);
    setErrorText("Search was cancelled by user");

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
    qDebug() << "SearchJob::onSearchProgress received:" << filesProcessed;

    // Emit our own signal for SearchManager to handle
    emit searchProgress(filesProcessed);

    // Handle special progress values
    if (filesProcessed == -1) {
        // Search interrupted
        setError(KilledJobError);
        setErrorText("Search was interrupted");
        emitResult();
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
        // Catalog loading progress (SearchMemory specific)
        // For now, just ignore this
        return;
    }

    // Regular progress update
    if (filesProcessed >= 0) {
        qDebug() << "SearchJob updating progress to:" << filesProcessed;
        m_lastFilesProcessed = filesProcessed;
        setProcessedAmount(KJob::Files, filesProcessed);

        // Calculate percentage if we have a total
        if (m_searchEngine->estimatedTotalFiles > 0) {
            unsigned long percent = qMin(100UL,
                                         (static_cast<unsigned long>(filesProcessed) * 100) / m_searchEngine->estimatedTotalFiles);
            qDebug() << "SearchJob emitting percent:" << percent;
            emitPercent(percent, 100);
        }
    }
}
//----------------------------------------------------------------------
