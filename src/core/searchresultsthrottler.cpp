/*LICENCE
    This file is part of Katalog

    Copyright (C) 2025, the Katalog Development team

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
// File Name:   searchresultsthrottler.cpp
// Purpose:     Throttle search results display updates for performance
// Description: Implementation of SearchResultsThrottler class
// Author:      Stephane Couturier
/////////////////////////////////////////////////////////////////////////////
*/

#include "searchresultsthrottler.h"
#include "search.h"
#include <QDebug>

SearchResultsThrottler::SearchResultsThrottler(QObject *parent)
    : QObject(parent)
{
    setupTimer();
}

void SearchResultsThrottler::setupTimer()
{
    m_updateTimer = new QTimer(this);
    m_updateTimer->setSingleShot(true);
    connect(m_updateTimer, &QTimer::timeout, this, &SearchResultsThrottler::performPendingUpdate);
}

void SearchResultsThrottler::setUpdateInterval(int interval)
{
    if (m_updateInterval != interval && interval > 0) {
        m_updateInterval = interval;
        emit updateIntervalChanged();
    }
}

void SearchResultsThrottler::setEnabled(bool enabled)
{
    if (m_enabled != enabled) {
        m_enabled = enabled;

        if (!enabled) {
            // Stop timer and clear pending updates when disabled
            m_updateTimer->stop();
            m_pendingUpdate = false;
        }

        emit enabledChanged();
    }
}

void SearchResultsThrottler::setCurrentSearch(Search *search)
{
    m_currentSearch = search;
    reset();
}

void SearchResultsThrottler::onSearchProgress(int filesProcessed)
{
    if (!m_enabled) {
        // When disabled, pass through immediately
        emit updateDisplay();
        return;
    }

    // Track the latest progress
    m_lastFilesProcessed = filesProcessed;

    // Check if this requires immediate update
    if (shouldUpdateImmediately(filesProcessed)) {
        qDebug() << "SearchResultsThrottler: Immediate update for special progress value:" << filesProcessed;

        // Stop any pending timer and update immediately
        m_updateTimer->stop();
        m_pendingUpdate = false;
        emit updateDisplay();
        return;
    }

    // For regular progress updates, use throttling
    m_pendingUpdate = true;

    if (!m_updateTimer->isActive()) {
        qDebug() << "SearchResultsThrottler: Starting throttled update timer (" << m_updateInterval << "ms)";
        m_updateTimer->start(m_updateInterval);
    }
}

void SearchResultsThrottler::forceUpdate()
{
    m_updateTimer->stop();
    m_pendingUpdate = false;
    emit updateDisplay();
}

void SearchResultsThrottler::reset()
{
    m_updateTimer->stop();
    m_pendingUpdate = false;
    m_lastFilesProcessed = -1;
}

void SearchResultsThrottler::performPendingUpdate()
{
    if (m_pendingUpdate) {
        qDebug() << "SearchResultsThrottler: Performing throttled update (files processed:" << m_lastFilesProcessed << ")";
        m_pendingUpdate = false;
        emit updateDisplay();
    }
}

bool SearchResultsThrottler::shouldUpdateImmediately(int filesProcessed) const
{
    // Special progress values that should trigger immediate updates
    switch (filesProcessed) {
    case -1: // Search interrupted
    case -2: // Catalog loading started
    case -3: // Catalog loading finished
        return true;
    case -4: // Catalog loading progress - can be throttled
    default:
        return false;
    }
}
