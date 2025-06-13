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
// File Name:   searchmanager.h
// Purpose:     Class/model to manage search operations using KJob framework
// Description:
// Author:      Stephane Couturier
/////////////////////////////////////////////////////////////////////////////
*/
#ifndef SEARCHMANAGER_H
#define SEARCHMANAGER_H

#pragma once

#include <QObject>
#include <QTimer>
#include "src/core/searchjob.h"
#include "src/core/search_memory.h"
#include "src/core/searchjobstoppable.h"

/**
 * @brief The SearchManager class
 * Manages search operations using KJob framework
 * Provides a clean interface for the UI to interact with search jobs
 */
class SearchManager : public QObject
{
    Q_OBJECT

    Q_PROPERTY(bool searchRunning READ searchRunning NOTIFY searchRunningChanged)
    Q_PROPERTY(int progress READ progress NOTIFY progressChanged)
    Q_PROPERTY(QString status READ status NOTIFY statusChanged)
    Q_PROPERTY(QString currentCatalog READ currentCatalog NOTIFY currentCatalogChanged)
    Q_PROPERTY(int filesProcessed READ filesProcessed NOTIFY filesProcessedChanged)

public:
    explicit SearchManager(QObject *parent = nullptr);
    ~SearchManager();

    // Properties
    bool searchRunning() const { return m_searchRunning; }
    int progress() const { return m_progress; }
    QString status() const { return m_status; }
    QString currentCatalog() const { return m_currentCatalog; }
    int filesProcessed() const { return m_filesProcessed; }

    // Get the current search results
    Search* getCurrentSearch() const;

private:
    void setSearchRunning(bool running);
    void setProgress(int progress);
    void setStatus(const QString &status);
    void setCurrentCatalog(const QString &catalog);
    void setFilesProcessed(int filesProcessed);
    void cleanupJob();
    int m_filesProcessed = 0;

    SearchJob *m_currentJob = nullptr;
    bool m_searchRunning = false;
    int m_progress = 0;
    QString m_status = "Ready";
    QString m_currentCatalog;
    bool m_isPaused = false;

public slots:
    void startSearchMemory(SearchMemory *searchEngine, Device *targetDevice);
    void startSearchJobStoppable(SearchJobStoppable *searchEngine, Device *targetDevice);
    void stopSearch();
    void pauseSearch();
    void resumeSearch();

private slots:
    void onJobResult(KJob *job);
    void onJobPercent();
    void onJobInfoMessage(KJob *job, const QString &message);
    void onCatalogLoadingStarted(const QString &catalogName);
    void onCatalogLoadingFinished();

signals:
    void searchRunningChanged();
    void progressChanged();
    void statusChanged();
    void currentCatalogChanged();
    void filesProcessedChanged();
    void searchCompleted();
    void searchCancelled();
    void searchError(const QString &error);

    void specialProgressUpdate(int specialValue);  // For -1, -2, -3, -4 values
    void catalogLoadingStarted(const QString &catalogName);
    void catalogLoadingFinished();
    void searchInterrupted();
};

#endif // SEARCHMANAGER_H
