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
// File Name:   catalogprogressmanager.h
// Purpose:     Manager for catalog progress display and status bar updates
// Description: Handles UI progress updates for catalog operations
//              Follows SearchProgressManager pattern exactly
// Author:      Stephane Couturier
/////////////////////////////////////////////////////////////////////////////
*/
#ifndef CATALOGPROGRESSMANAGER_H
#define CATALOGPROGRESSMANAGER_H

#pragma once

#include "catalogmanager.h"
#include "catalogjobstoppable.h"
#include <QObject>
#include <QStatusBar>
#include <QTimer>
#include <QString>
#include <QLocale>

/**
 * @brief The CatalogProgressManager class
 * Manages catalog progress display and status bar updates
 * Follows the exact same pattern as SearchProgressManager for consistency
 */
class CatalogProgressManager : public QObject
{
    Q_OBJECT

public:
    // Declare constructor - implement in cpp file
    explicit CatalogProgressManager(QStatusBar *statusBar, QTimer *timer, QObject *parent = nullptr);

    /**
     * @brief Set the catalog manager to monitor
     * @param catalogManager The catalog manager instance
     */
    void connectToCatalogManager(CatalogManager *catalogManager);

    /**
     * @brief Set the current catalog operation engine for detailed progress
     * @param currentCatalogEngine The catalog engine being monitored
     */
    void setCurrentCatalogEngine(CatalogJobStoppable *currentCatalogEngine);

public slots:
    /**
     * @brief Update progress display from catalog manager
     * Called when catalog manager state changes
     */
    void updateFromCatalogManager();

    /**
     * @brief Show message with optional timeout (matching SearchProgressManager)
     */
    void showMessage(const QString &message, int timeout = 0);

signals:
    void statusBarUpdated();  // Keep for consistency with SearchProgressManager

private:
    CatalogManager *m_catalogManager = nullptr;
    CatalogJobStoppable *m_currentCatalogEngine = nullptr;
    QStatusBar *m_statusBar = nullptr;
    QTimer *m_statusBarTimer = nullptr;
};

#endif // CATALOGPROGRESSMANAGER_H
