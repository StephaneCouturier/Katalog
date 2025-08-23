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
// File Name:   catalogprogressmanager.cpp
// Purpose:     Implementation of catalog progress display and status bar updates
// Description: Handles UI progress updates for catalog operations
//              Follows SearchProgressManager pattern exactly
// Author:      Stephane Couturier
/////////////////////////////////////////////////////////////////////////////
*/

#include "catalogprogressmanager.h"
#include <QStatusBar>
#include <QLocale>
#include <QDebug>

CatalogProgressManager::CatalogProgressManager(QStatusBar *statusBar, QTimer *timer, QObject *parent)
    : QObject(parent), m_statusBar(statusBar), m_statusBarTimer(timer)
{
    qDebug() << "CatalogProgressManager created with statusBar and timer";
}

void CatalogProgressManager::connectToCatalogManager(CatalogManager *catalogManager)
{
    if (m_catalogManager) {
        // Disconnect from previous catalog manager
        disconnect(m_catalogManager, nullptr, this, nullptr);
    }

    m_catalogManager = catalogManager;



    if (m_catalogManager) {
        // Connect to catalog manager signals for automatic updates
        connect(m_catalogManager, &CatalogManager::catalogOperationRunningChanged,
                this, &CatalogProgressManager::updateFromCatalogManager);
        connect(m_catalogManager, &CatalogManager::progressChanged,
                this, &CatalogProgressManager::updateFromCatalogManager);
        connect(m_catalogManager, &CatalogManager::statusChanged,
                this, &CatalogProgressManager::updateFromCatalogManager);
        connect(m_catalogManager, &CatalogManager::filesProcessedChanged,
                this, &CatalogProgressManager::updateFromCatalogManager);
        connect(m_catalogManager, &CatalogManager::totalFilesChanged,
                this, &CatalogProgressManager::updateFromCatalogManager);
        connect(m_catalogManager, &CatalogManager::currentPathChanged,
                this, &CatalogProgressManager::updateFromCatalogManager);
        connect(m_catalogManager, &CatalogManager::currentCatalogNameChanged,
                this, &CatalogProgressManager::updateFromCatalogManager);
        connect(m_catalogManager, &CatalogManager::catalogOperationCancelled,
                this, [this]() {
                    qDebug() << "CatalogProgressManager: Operation cancelled - clearing status immediately";
                    if (m_statusBar) {
                        m_statusBar->showMessage("Catalog operation cancelled");
                        if (m_statusBarTimer) {
                            m_statusBarTimer->start(5000);
                        }
                    }
                });
        connect(m_catalogManager, &CatalogManager::specialProgressUpdate,
                this, [this](qint64 filesProcessed, qint64 totalFiles, int progressPercent, const QString& currentPath) {
                    if (m_statusBar && !currentPath.isEmpty()) {
                        QString progressText = tr("Processing: %1 (%2/%3)")
                        .arg(currentPath)
                            .arg(QLocale().toString(filesProcessed))
                            .arg(QLocale().toString(totalFiles));
                        m_statusBar->showMessage(progressText);
                    }
                });
    }
}

void CatalogProgressManager::setCurrentCatalogEngine(CatalogJobStoppable *currentCatalogEngine)
{
    m_currentCatalogEngine = currentCatalogEngine;
}

// Replace this method in src/core/catalogprogressmanager.cpp

void CatalogProgressManager::updateFromCatalogManager()
{
    if (!m_catalogManager || !m_statusBar) return;

    QString message;

    if (m_catalogManager->catalogOperationRunning()) {
        // COUNTING PHASE: Show estimation messages
        if (m_catalogManager->totalFiles() == 0 &&
            !m_catalogManager->currentPath().isEmpty() &&
            m_catalogManager->currentPath().contains("Counting")) {

            message = m_catalogManager->currentPath(); // "Counting... X files found"

        } else if (m_catalogManager->totalFiles() > 0) {
            // PROCESSING PHASE: Show detailed progress with file paths
            message = QString("Files processed: %1 | Total files: %2 | Progress: %3%")
                          .arg(QLocale().toString(m_catalogManager->filesProcessed()), QLocale().toString(m_catalogManager->totalFiles()))
                          .arg(m_catalogManager->progress());

            // Add current file path being processed
            if (!m_catalogManager->currentPath().isEmpty() &&
                !m_catalogManager->currentPath().contains("Counting")) {
                QString displayPath = m_catalogManager->currentPath();
                message += QString(" | %1").arg(displayPath);
            }
        } else {
            message = m_catalogManager->status();
        }

        // Add catalog name prefix
        if (!m_catalogManager->currentCatalogName().isEmpty()) {
            message = QString("Catalog: %1 | %2")
            .arg(m_catalogManager->currentCatalogName(), message);
        }

        m_statusBar->showMessage(message);
        if (m_statusBarTimer) m_statusBarTimer->stop();

    } else {
        // Operation complete - show final status
        message = (m_catalogManager->status() == "Ready") ?
                      tr("Ready for catalog operations") : m_catalogManager->status();

        if (message.contains("completed successfully") && m_catalogManager->filesProcessed() > 0) {
            message += QString(" | %1 files processed")
            .arg(QLocale().toString(m_catalogManager->filesProcessed()));
        }

        m_statusBar->showMessage(message);
        if (m_statusBarTimer) m_statusBarTimer->start(5000);
    }
}

void CatalogProgressManager::showMessage(const QString &message, int timeout)
{
    if (m_statusBar) {
        m_statusBar->showMessage(message, timeout);
    }
}
