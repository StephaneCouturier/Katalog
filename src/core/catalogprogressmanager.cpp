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
    if (!catalogManager) return;

    m_catalogManager = catalogManager;

    // Connect to progress update signals
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

    // Handle cancellation
    connect(m_catalogManager, &CatalogManager::catalogOperationCancelled,
            this, [this]() {
                qDebug() << "=== CANCELLED SIGNAL ===";
                qDebug() << "Phase:" << m_catalogManager->lastPhase();

                StatusBarMessageBuilder builder;

                // Operation type
                if (m_catalogManager->lastOperationType() == CatalogJobStoppable::CreateCatalog) {
                    builder.setOperation(QApplication::translate("MainWindow", "Create"));
                } else {
                    builder.setOperation(QApplication::translate("MainWindow", "Update"));
                }

                builder.setStatus(QApplication::translate("MainWindow", "Cancelled"));

                // Catalog index (batch context)
                if (m_batchTotalCatalogs > 1) {
                    builder.setCatalogIndex(m_batchCurrentIndex, m_batchTotalCatalogs);
                } else {
                    builder.setCatalogIndex(1, 1);
                }

                // Catalog name
                if (!m_catalogManager->lastCatalogName().isEmpty()) {
                    builder.setCatalogName(m_catalogManager->lastCatalogName());
                }

                // Process info based on PHASE
                switch (m_catalogManager->lastPhase()) {
                case CatalogManager::PHASE_LOADING:
                    builder.setProcess(
                        QApplication::translate("MainWindow", "Loaded"),
                        m_catalogManager->lastFilesProcessed(),
                        m_catalogManager->lastTotalFiles()
                        );
                    break;

                case CatalogManager::PHASE_COUNTING:
                    builder.setProcess(
                        QApplication::translate("MainWindow", "Counted"),
                        m_catalogManager->lastFilesProcessed(),
                        0  // No total for counting
                        );
                    break;

                case CatalogManager::PHASE_INDEXING:
                    builder.setProcess(
                        QApplication::translate("MainWindow", "Indexed"),
                        m_catalogManager->lastFilesProcessed(),
                        m_catalogManager->lastTotalFiles()
                        );
                    break;

                case CatalogManager::PHASE_MIGRATING:
                    builder.setProcess(
                        QApplication::translate("MainWindow", "File Types Updated"),
                        m_catalogManager->lastFilesProcessed(),
                        m_catalogManager->lastTotalFiles()
                        );
                    break;

                case CatalogManager::PHASE_METADATA_EXTRACTION:
                    builder.setProcess(
                        QApplication::translate("MainWindow", "Metadata Extracted"),
                        m_catalogManager->lastFilesProcessed(),
                        m_catalogManager->lastTotalFiles()
                        );
                    break;

                default:
                    break;
                }

                m_statusBarLabel->setText(builder.build());

                if (m_statusBarTimer) {
                    m_statusBarTimer->start(5000);
                }
            });

    // Handle completion
    connect(m_catalogManager, &CatalogManager::catalogOperationCompleted,
            this, [this]() {
                qDebug() << "=== COMPLETED SIGNAL ===";

                StatusBarMessageBuilder builder;

                // Operation type
                if (m_catalogManager->lastOperationType() == CatalogJobStoppable::CreateCatalog) {
                    builder.setOperation(QApplication::translate("MainWindow", "Create"));
                } else {
                    builder.setOperation(QApplication::translate("MainWindow", "Update"));
                }

                builder.setStatus(QApplication::translate("MainWindow", "Completed"));

                // Catalog index (batch context)
                if (m_batchTotalCatalogs > 1) {
                    builder.setCatalogIndex(m_batchCurrentIndex, m_batchTotalCatalogs);
                } else {
                    builder.setCatalogIndex(1, 1);
                }

                // Catalog name
                if (!m_catalogManager->lastCatalogName().isEmpty()) {
                    builder.setCatalogName(m_catalogManager->lastCatalogName());
                }

                // ALWAYS show final count
                qint64 finalProcessed = m_catalogManager->lastFilesProcessed();
                qint64 finalTotal = m_catalogManager->lastTotalFiles();

                // For updates with small changes, use total if processed is 0
                if (finalProcessed == 0 && finalTotal > 0) {
                    finalProcessed = finalTotal;
                }

                if (finalProcessed > 0 && finalTotal > 0) {
                    builder.setProcess(
                        QApplication::translate("MainWindow", "Indexed"),
                        finalProcessed,
                        finalTotal
                        );
                }

                m_statusBarLabel->setText(builder.build());

                if (m_statusBarTimer) {
                    m_statusBarTimer->start(5000);
                }
            });
}

void CatalogProgressManager::setCurrentCatalogEngine(CatalogJobStoppable *currentCatalogEngine)
{
    m_currentCatalogEngine = currentCatalogEngine;
}

void CatalogProgressManager::updateFromCatalogManager()
{
    if (!m_catalogManager || !m_statusBar) return;

    // ONLY handle in-progress updates
    if (m_catalogManager->catalogOperationRunning()) {
        StatusBarMessageBuilder builder;

        // Operation type
        if (m_catalogManager->currentOperationType() == CatalogJobStoppable::CreateCatalog) {
            builder.setOperation(QApplication::translate("MainWindow", "Create"));
        } else {
            builder.setOperation(QApplication::translate("MainWindow", "Update"));
        }

        builder.setStatus(QApplication::translate("MainWindow", "In Progress"));

        // Catalog index (batch context)
        if (m_batchTotalCatalogs > 1) {
            builder.setCatalogIndex(m_batchCurrentIndex, m_batchTotalCatalogs);
        } else {
            builder.setCatalogIndex(1, 1);
        }

        // Catalog name
        if (!m_catalogManager->currentCatalogName().isEmpty()) {
            builder.setCatalogName(m_catalogManager->currentCatalogName());
        }

        // Detect phase from currentPath
        QString currentPath = m_catalogManager->currentPath();

        if (currentPath.startsWith("__CATALOG_LOADING__|")) {
            // CATALOG LOADING
            QStringList parts = currentPath.split("|");
            if (parts.size() >= 3) {
                int loaded = parts[1].toInt();
                int total = parts[2].toInt();
                builder.setProcess(
                    QApplication::translate("MainWindow", "Loaded"),
                    loaded,
                    total
                    );
            }
        } else if (currentPath.startsWith("__COUNTING_STATE__|")) {
            // COUNTING
            QStringList parts = currentPath.split("|");
            if (parts.size() >= 2) {
                int fileCount = parts[1].toInt();
                builder.setProcess(
                    QApplication::translate("MainWindow", "Counted"),
                    fileCount,
                    0
                    );
            }

        } else if (currentPath.startsWith("__FILETYPE_MIGRATION__|")) {
            // FILE TYPE MIGRATION
            QStringList parts = currentPath.split("|");
            if (parts.size() >= 3) {
                int processed = parts[1].toInt();
                int total = parts[2].toInt();
                builder.setProcess(
                    QApplication::translate("MainWindow", "File Types Updated"),
                    processed,
                    total
                    );
            }

        } else if (currentPath.startsWith("__SAVING__|")) {
            // Show simple "Saving" indicator without progress numbers
            builder.setCurrentItem(QApplication::translate("MainWindow", "Saving"));

        } else if (currentPath.startsWith("__METADATA_EXTRACTION__|")) {
            // METADATA EXTRACTION
            QStringList parts = currentPath.split("|");
            if (parts.size() >= 3) {
                int processed = parts[1].toInt();
                int total = parts[2].toInt();

                builder.setProcess(
                    QApplication::translate("MainWindow", "Metadata Extracted"),
                    processed,
                    total
                    );

                // Add time to completion if present (Part 10)
                if (parts.size() >= 4 && !parts[3].isEmpty()) {
                    builder.setTimeToCompletion(parts[3]);
                }
            }

        } else if (currentPath.startsWith("__CHECKSUM_CALCULATION__|")) {
            // CHECKSUM CALCULATION
            QStringList parts = currentPath.split("|");
            if (parts.size() >= 3) {
                int processed = parts[1].toInt();
                int total = parts[2].toInt();

                builder.setProcess(
                    QApplication::translate("MainWindow", "Checksums Calculated"),
                    processed,
                    total
                    );

                // Add time to completion if present (part 3)
                if (parts.size() >= 4 && !parts[3].isEmpty()) {
                    builder.setTimeToCompletion(parts[3]);
                }

                // Add current file info if present (part 4)
                if (parts.size() >= 5 && !parts[4].isEmpty()) {
                    builder.setCurrentItem(parts[4]);
                }
            }

        } else if (m_catalogManager->totalFiles() > 0) {
            // INDEXING
            builder.setProcess(
                QApplication::translate("MainWindow", "Indexed"),
                m_catalogManager->filesProcessed(),
                m_catalogManager->totalFiles()
                );

            // Add file path if it's real
            if (!currentPath.isEmpty() &&
                !currentPath.startsWith("__") &&
                !currentPath.startsWith("Found ") &&
                !currentPath.startsWith("Analyzing ") &&
                !currentPath.startsWith("Updating ")) {
                builder.setCurrentItem(currentPath);
            }
        }

        m_statusBar->show();
        m_statusBarLabel->setText(builder.build());

        if (m_statusBarTimer) {
            m_statusBarTimer->stop();
        }
    }
    // When NOT running, do NOTHING - completion signal handles it
}

void CatalogProgressManager::showMessage(const QString &message, int timeout)
{
    if (m_statusBar) {
        m_statusBarLabel->setText(message);
    }
}

void CatalogProgressManager::setBatchContext(int currentIndex, int totalCatalogs)
{
    m_batchCurrentIndex = currentIndex;
    m_batchTotalCatalogs = totalCatalogs;
    qDebug() << "CatalogProgressManager: Batch context set to" << currentIndex << "of" << totalCatalogs;
}

void CatalogProgressManager::clearBatchContext()
{
    m_batchCurrentIndex = 0;
    m_batchTotalCatalogs = 0;
    qDebug() << "CatalogProgressManager: Batch context cleared";
}
