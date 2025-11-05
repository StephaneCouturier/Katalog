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
// File Name:   searchresultsthrottler.h
// Purpose:     Throttle search results display updates for performance
// Description: Manages when to update UI with search results to balance
//              responsiveness with performance
// Author:      Stephane Couturier
/////////////////////////////////////////////////////////////////////////////
*/

#ifndef SEARCHRESULTSTHROTTLER_H
#define SEARCHRESULTSTHROTTLER_H

#pragma once

#include <QObject>
#include <QTimer>

class Search;

/**
 * @brief The SearchResultsThrottler class
 * Throttles search results display updates to balance UI responsiveness with performance.
 * Collects search progress signals and emits display updates at controlled intervals.
 */
class SearchResultsThrottler : public QObject
{
    Q_OBJECT

    Q_PROPERTY(int updateInterval READ updateInterval WRITE setUpdateInterval NOTIFY updateIntervalChanged)
    Q_PROPERTY(bool enabled READ enabled WRITE setEnabled NOTIFY enabledChanged)

public:
    explicit SearchResultsThrottler(QObject *parent = nullptr);
    ~SearchResultsThrottler() = default;

    // Properties
    int updateInterval() const { return m_updateInterval; }
    void setUpdateInterval(int interval);

    bool enabled() const { return m_enabled; }
    void setEnabled(bool enabled);

    // Configuration
    void setCurrentSearch(Search *search);

public slots:
    /**
     * @brief Handle search progress updates
     * @param filesProcessed Number of files processed, or special values:
     *        -1: Search interrupted (immediate display update)
     *        -2: Catalog loading started (immediate display update)
     *        -3: Catalog loading finished (immediate display update)
     *        -4: Catalog loading progress (IGNORED - status bar only, no display update)
     *        >= 0: Files evaluated against criteria (throttled display updates)
     */
    void onSearchProgress(int filesProcessed);

    /**
     * @brief Force immediate display update
     */
    void forceUpdate();

    /**
     * @brief Reset throttler state
     */
    void reset();

signals:
    /**
     * @brief Signal to update the search results display
     */
    void updateDisplay();

    /**
     * @brief Properties change notifications
     */
    void updateIntervalChanged();
    void enabledChanged();

private slots:
    void performPendingUpdate();

private:
    QTimer *m_updateTimer = nullptr;
    Search *m_currentSearch = nullptr;

    bool m_pendingUpdate = false;
    bool m_enabled = true;
    int m_updateInterval = 500; // milliseconds
    int m_lastFilesProcessed = -1;

    void setupTimer();
    bool shouldUpdateImmediately(int filesProcessed) const;
};

#endif // SEARCHRESULTSTHROTTLER_H
