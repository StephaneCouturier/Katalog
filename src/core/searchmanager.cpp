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
// File Name:   searchmanager.cpp
// Purpose:     Class/model to manage search operations using KJob framework
// Description:
// Author:      Stephane Couturier
/////////////////////////////////////////////////////////////////////////////
*/

#include "searchmanager.h"
#include <QDebug>

SearchManager::SearchManager(QObject *parent)
    : QObject(parent)
{
}

SearchManager::~SearchManager()
{
    if (m_currentJob) {
        m_currentJob->kill();
        m_currentJob->deleteLater();
    }
}
//----------------------------------------------------------------------
void SearchManager::startSearchMemory(SearchMemory *searchEngine, Device *targetDevice)
{
    if (m_currentJob) {
        qDebug() << "Search already running!";
        return;
    }

    if (!searchEngine || !targetDevice) {
        emit searchError("Invalid search configuration");
        return;
    }

    m_currentJob = new SearchJob(this);
    m_currentJob->setSearchMemory(searchEngine);
    m_currentJob->setTargetDevice(targetDevice);

    // Connect job signals
    connect(m_currentJob, &KJob::result, this, &SearchManager::onJobResult);
    connect(m_currentJob, &KJob::percent, this, &SearchManager::onJobPercent);
    // Note: infoMessage signal may not be available in all KJob versions
    // connect(m_currentJob, &KJob::infoMessage, this, &SearchManager::onJobInfoMessage);

    // Connect search-specific signals
    connect(m_currentJob, &SearchJob::catalogLoadingStarted, this, &SearchManager::onCatalogLoadingStarted);
    connect(m_currentJob, &SearchJob::catalogLoadingFinished, this, &SearchManager::onCatalogLoadingFinished);

    setStatus("Starting search...");
    setSearchRunning(true);
    setProgress(0);
    m_isPaused = false;

    // Start the job
    m_currentJob->startJob();
}
//----------------------------------------------------------------------
void SearchManager::startSearchJobStoppable(SearchJobStoppable *searchEngine, Device *targetDevice)
{
    if (m_currentJob) {
        qDebug() << "Search already running!";
        return;
    }

    if (!searchEngine || !targetDevice) {
        emit searchError("Invalid search configuration");
        return;
    }

    m_currentJob = new SearchJob(this);
    m_currentJob->setSearchJobStoppable(searchEngine);
    m_currentJob->setTargetDevice(targetDevice);

    // Connect job signals with enhanced progress handling
    connect(m_currentJob, &KJob::result, this, &SearchManager::onJobResult);

    // Enhanced progress connection that handles special values
    connect(m_currentJob, &SearchJob::searchProgress, this, [this](int filesProcessed) {
        if (m_currentJob && m_currentJob->getSearchEngine()) {
            Search* engine = m_currentJob->getSearchEngine();

            // Handle special progress values for catalog loading
            if (filesProcessed == -2) {
                // Catalog loading started - set catalog name
                QString catalogName = engine->currentCatalogName;
                if (!catalogName.isEmpty()) {
                    setCurrentCatalogName(catalogName);
                    setStatus(QString("Loading catalog: %1").arg(catalogName));
                }
                return;
            }

            if (filesProcessed == -3) {
                // Catalog loading finished - update status
                setStatus("Processing files...");
                return;
            }

            // Regular progress update
            if (filesProcessed >= 0) {
                setFilesProcessed(filesProcessed);

                if (engine->estimatedTotalFiles > 0) {
                    int percent = qMin(100, (filesProcessed * 100) / engine->estimatedTotalFiles);
                    qDebug() << "SearchManager progress update:" << percent << "% (" << filesProcessed << "/" << engine->estimatedTotalFiles << ")";
                    setProgress(percent);
                }

                // Update status with catalog info during regular processing
                QString statusMsg = "Searching";
                if (!engine->currentCatalogName.isEmpty()) {
                    setCurrentCatalogName(engine->currentCatalogName);
                }
                setStatus(statusMsg);
            }
        }
    });

    // Connect catalog-specific signals if available
    connect(m_currentJob, &SearchJob::catalogLoadingStarted, this, &SearchManager::onCatalogLoadingStarted);
    connect(m_currentJob, &SearchJob::catalogLoadingFinished, this, &SearchManager::onCatalogLoadingFinished);

    // Enhanced progress connection that captures files processed count
    connect(m_currentJob, &SearchJob::searchProgress, this, [this](int filesProcessed) {
        if (m_currentJob && m_currentJob->getSearchEngine()) {
            Search* engine = m_currentJob->getSearchEngine();

            // Update files processed count
            if (filesProcessed >= 0) {
                setFilesProcessed(filesProcessed);
            }

            if (engine->estimatedTotalFiles > 0) {
                int percent = qMin(100, (filesProcessed * 100) / engine->estimatedTotalFiles);
                qDebug() << "SearchManager progress update:" << percent << "% (" << filesProcessed << "/" << engine->estimatedTotalFiles << ")";
                setProgress(percent);
            }
        }
    });
    // Note: infoMessage signal may not be available in all KJob versions
    // connect(m_currentJob, &KJob::infoMessage, this, &SearchManager::onJobInfoMessage);

    // Connect search-specific signals
    //connect(m_currentJob, &SearchJob::catalogLoadingStarted, this, &SearchManager::onCatalogLoadingStarted);
    //connect(m_currentJob, &SearchJob::catalogLoadingFinished, this, &SearchManager::onCatalogLoadingFinished);

    setStatus("Starting search...");
    setSearchRunning(true);
    setProgress(0);
    m_isPaused = false;

    // Start the job
    m_currentJob->startJob();
}
//----------------------------------------------------------------------
void SearchManager::stopSearch()
{
    qDebug() << "=== SearchManager::stopSearch() called ===";
    qDebug() << "Current job exists:" << (m_currentJob != nullptr);
    qDebug() << "Search running:" << m_searchRunning;

    if (!m_currentJob) {
        qDebug() << "No search to stop!";
        return;
    }

    qDebug() << "Stopping search...";
    setStatus("Stopping search...");

    // Disconnect signals first to prevent double handling
    disconnect(m_currentJob, nullptr, this, nullptr);

    // Kill the job
    qDebug() << "Calling m_currentJob->kill()";
    m_currentJob->kill();

    // Force immediate cleanup since result signal might not be emitted
    qDebug() << "Force cleanup after kill...";
    setSearchRunning(false);
    setProgress(0);
    setCurrentCatalogName("");
    m_isPaused = false;

    // Clean up immediately
    if (m_currentJob) {
        m_currentJob->deleteLater();
        m_currentJob = nullptr;
        qDebug() << "Forced cleanup complete";
    }

    emit searchCancelled();
    qDebug() << "=== SearchManager::stopSearch() complete ===";
}
//----------------------------------------------------------------------
void SearchManager::pauseSearch()
{
    if (!m_currentJob || m_isPaused) {
        return;
    }

    if (m_currentJob->suspend()) {
        m_isPaused = true;
        setStatus("Search paused");
        qDebug() << "Search paused";
    }
}
//----------------------------------------------------------------------
void SearchManager::resumeSearch()
{
    if (!m_currentJob || !m_isPaused) {
        return;
    }

    if (m_currentJob->resume()) {
        m_isPaused = false;
        setStatus("Search resumed");
        qDebug() << "Search resumed";
    }
}
//----------------------------------------------------------------------
Search* SearchManager::getCurrentSearch() const
{
    if (m_currentJob) {
        return m_currentJob->getSearchEngine();
    }
    return nullptr;
}
//----------------------------------------------------------------------
void SearchManager::onJobResult(KJob *job)
{
    qDebug() << "Search job result received, error:" << job->error();

    if (job->error() == KJob::KilledJobError) {
        setStatus("Search cancelled");
        emit searchCancelled();
    } else if (job->error()) {
        QString errorMsg = QString("Search failed: %1").arg(job->errorString());
        setStatus(errorMsg);
        emit searchError(errorMsg);
    } else {
        setStatus("Search completed successfully!");
        emit searchCompleted();
    }

    setSearchRunning(false);
    setProgress(job->error() ? 0 : 100);
    setCurrentCatalogName("");
    m_isPaused = false;

    qDebug() << "About to cleanup job...";
    cleanupJob();
    qDebug() << "Job cleanup finished";
}
//----------------------------------------------------------------------
void SearchManager::onJobPercent()
{
    if (m_currentJob) {
        unsigned long percent = m_currentJob->percent();
        qDebug() << "SearchManager::onJobPercent received percent:" << percent;
        setProgress(static_cast<int>(percent));
    }
}
//----------------------------------------------------------------------
void SearchManager::onJobInfoMessage(KJob *job, const QString &message)
{
    Q_UNUSED(job);
    setStatus(message);
}
//----------------------------------------------------------------------
void SearchManager::onCatalogLoadingStarted(const QString &catalogName)
{
    setCurrentCatalogName(catalogName);
    setStatus(QString("Loading catalog: %1").arg(catalogName));
}
//----------------------------------------------------------------------
void SearchManager::onCatalogLoadingFinished()
{
    setStatus("Processing files...");
}
//----------------------------------------------------------------------
void SearchManager::setSearchRunning(bool running)
{
    if (m_searchRunning != running) {
        m_searchRunning = running;
        emit searchRunningChanged();
    }
}
//----------------------------------------------------------------------
void SearchManager::setProgress(int progress)
{
    if (m_progress != progress) {
        qDebug() << "SearchManager::setProgress updating from" << m_progress << "to" << progress;
        m_progress = progress;
        emit progressChanged();
    }
}
//----------------------------------------------------------------------
void SearchManager::setStatus(const QString &status)
{
    if (m_status != status) {
        m_status = status;
        emit statusChanged();
    }
}
//----------------------------------------------------------------------
void SearchManager::setCurrentCatalogName(const QString &catalog)
{
    if (m_currentCatalogName != catalog) {
        m_currentCatalogName = catalog;
        emit currentCatalogChanged();
    }
}
//----------------------------------------------------------------------
void SearchManager::cleanupJob()
{
    if (m_currentJob) {
        qDebug() << "Cleaning up search job...";
        m_currentJob->deleteLater();
        m_currentJob = nullptr;
        qDebug() << "Search job cleanup complete";
    }
}
//----------------------------------------------------------------------
void SearchManager::setFilesProcessed(int filesProcessed)
{
    if (m_filesProcessed != filesProcessed) {
        m_filesProcessed = filesProcessed;
        emit filesProcessedChanged();
    }
}
