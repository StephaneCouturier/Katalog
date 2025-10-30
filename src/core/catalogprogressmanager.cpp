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
#include "core/statusbarmessagebuilder.h"
#include <QStatusBar>
#include <QLocale>
#include <QDebug>
#include <qapplication.h>

CatalogProgressManager::CatalogProgressManager(QStatusBar *statusBar, QTimer *timer, QLabel *statusLabel, QObject *parent)
    : QObject(parent), m_statusBar(statusBar), m_statusBarTimer(timer), m_statusBarLabel(statusLabel)
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
                        m_statusBarLabel->setText(QApplication::translate("MainWindow", "Operation cancelled"));
                        if (m_statusBarTimer) {
                            m_statusBarTimer->start(5000);
                        }
                    }
                });
    }
}

void CatalogProgressManager::setCurrentCatalogEngine(CatalogJobStoppable *currentCatalogEngine)
{
    m_currentCatalogEngine = currentCatalogEngine;
}

void CatalogProgressManager::updateFromCatalogManager()
{
    if (!m_catalogManager || !m_statusBar) return;

    if (m_catalogManager->catalogOperationRunning()) {
        StatusBarMessageBuilder builder;
        builder.setOperation(QApplication::translate("MainWindow", "Update"));

        // COUNTING FILES state
        if (m_catalogManager->totalFiles() == 0 &&
            !m_catalogManager->currentPath().isEmpty() &&
            m_catalogManager->currentPath().startsWith("__COUNTING_STATE__|")) {

            QString pathData = m_catalogManager->currentPath();
            QStringList parts = pathData.split("|");
            if (parts.size() >= 2) {
                int fileCount = parts[1].toInt();
                builder.setProcess(QApplication::translate("MainWindow", "Counting files"), fileCount);
            }

        } else if (m_catalogManager->totalFiles() > 0) {
            // PROCESSING FILES state
            builder.setProcess(
                QApplication::translate("MainWindow", "Processed"),
                m_catalogManager->filesProcessed(),
                m_catalogManager->totalFiles()
                );

            // Add current file path
            if (!m_catalogManager->currentPath().isEmpty() &&
                !m_catalogManager->currentPath().contains("__COUNTING_STATE__|")) {
                builder.setCurrentItem(m_catalogManager->currentPath());
            }
        }

        // Add catalog name prefix
        if (!m_catalogManager->currentCatalogName().isEmpty()) {
            // For single catalog, show name without "1 of 1"
            builder.setDeviceContext(1, 1, m_catalogManager->currentCatalogName());
        }

        m_statusBar->show();
        m_statusBarLabel->setText(builder.build());

        if (m_statusBarTimer) {
            m_statusBarTimer->stop();
        }

    } else {
        // OPERATION NOT RUNNING
        QString status = m_catalogManager->status();

        if (status.contains("stopped", Qt::CaseInsensitive) ||
            status.contains("cancelled", Qt::CaseInsensitive)) {
            m_statusBarLabel->setText(QApplication::translate("MainWindow", "Operation cancelled"));
        }
        else if (status.contains("completed successfully", Qt::CaseInsensitive)) {
            QString message = status;
            if (m_catalogManager->filesProcessed() > 0) {
                message += QString(" | %1 files processed")
                .arg(QLocale().toString(m_catalogManager->filesProcessed()));
            }
            m_statusBarLabel->setText(message);
        }
        else {
            m_statusBarLabel->setText(status);
        }

        if (m_statusBarTimer) {
            m_statusBarTimer->start(5000);
        }
    }
}

void CatalogProgressManager::showMessage(const QString &message, int timeout)
{
    if (m_statusBar) {
        m_statusBarLabel->setText(message);
    }
}
