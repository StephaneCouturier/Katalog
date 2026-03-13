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
// File Name:   searchjob.h
// Purpose:     Class/model to execute a search using KJob framework
// Description:
// Author:      Stephane Couturier
/////////////////////////////////////////////////////////////////////////////
*/
#ifndef SEARCHJOB_H
#define SEARCHJOB_H

#pragma once
class SearchJobStoppable;

#include <KJob>
#include <QTimer>
#include <QMutex>
#include <QAtomicInt>
#include "search.h"
#include "device.h"

/**
 * @brief The SearchJob class
 * KJob implementation for search operations
 */
class SearchJob : public KJob
{
    Q_OBJECT

public:
    explicit SearchJob(QObject *parent = nullptr);

    // Configure the search job
    void setSearchJobStoppable(SearchJobStoppable *searchEngine);
    void setTargetDevice(Device *device);

    // Public method to start the job
    void startJob();

    // Get results
    Search* getSearchEngine() const { return m_searchEngine; }

private:
    Search *m_searchEngine = nullptr;
    Device *m_targetDevice = nullptr;
    QTimer *m_executeTimer = nullptr;
    QMutex m_mutex;
    bool m_suspended = false;
    int m_lastFilesProcessed = 0;
    QAtomicInt m_isKilled{0};

protected:
    void start() override;
    bool doKill() override;
    bool doSuspend() override;
    bool doResume() override;

private slots:
    void onSearchProgress(int filesProcessed);
    void executeSearch();

signals:
    void searchStarted();
    void searchFinished();
    void catalogLoadingStarted(const QString &catalogName);
    void catalogLoadingFinished();
    void searchProgress(int filesProcessed);
};

#endif // SEARCHJOB_H
