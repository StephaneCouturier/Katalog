/*LICENCE
    This file is part of Katalog

    Copyright (C) 2021, the Katalog Development team

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
// File Name:   searchprogressmanager.cpp
// Purpose:     header for the class that handles search progress reporting
// Description: https://stephanecouturier.github.io/Katalog/docs/Features/Search
// Author:      Stephane Couturier
/////////////////////////////////////////////////////////////////////////////
*///

#include "searchprogressmanager.h"
#include "searchmanager.h"
#include "search.h"
#include "searchjobstoppable.h"
#include "statusbarmessagebuilder.h"
#include <QCoreApplication>

void SearchProgressManager::connectToSearchManager(SearchManager *searchManager)
{
    if (!searchManager || !m_statusBar) return;

    // Store reference
    m_searchManager = searchManager;

    // Connect all SearchManager signals to our update method
    connect(searchManager, &SearchManager::statusChanged,
            this, &SearchProgressManager::updateFromSearchManager);
    connect(searchManager, &SearchManager::progressChanged,
            this, &SearchProgressManager::updateFromSearchManager);
    connect(searchManager, &SearchManager::currentCatalogChanged,
            this, &SearchProgressManager::updateFromSearchManager);
    connect(searchManager, &SearchManager::searchRunningChanged,
            this, &SearchProgressManager::updateFromSearchManager);
}

void SearchProgressManager::setCurrentSearch(Search *currentSearch)
{
    m_currentSearch = currentSearch;
}

void SearchProgressManager::updateFromSearchManager()
{
    if (!m_searchManager || !m_statusBar) return;

    if (m_searchManager->searchRunning()) {
        SearchJobStoppable* searchJobStoppable = nullptr;
        if (m_currentSearch) {
            searchJobStoppable = dynamic_cast<SearchJobStoppable*>(m_currentSearch);
        }

        // Handle paused state
        if (searchJobStoppable && searchJobStoppable->isPaused()) {
            // Rebuild message with paused status
            StatusBarMessageBuilder builder;
            builder.setOperation(QCoreApplication::translate("MainWindow","Search"));
            builder.setStatus(QCoreApplication::translate("MainWindow","In Progress"));

            // Determine pause type, Checking if catalog loading mode (loading not yet complete)
            bool isLoadingCatalog = searchJobStoppable->memoryModeEnabled &&
                                    searchJobStoppable->currentCatalogFilesLoaded > 0 &&
                                    searchJobStoppable->currentCatalogTotalFiles > 0 &&
                                    searchJobStoppable->currentCatalogFilesLoaded < searchJobStoppable->currentCatalogTotalFiles;

            builder.setStatus(QCoreApplication::translate("MainWindow","Paused"));
            builder.setDeviceContext(
                    m_currentSearch->currentCatalogIndex,
                    m_currentSearch->totalCatalogs,
                    m_searchManager->currentCatalogName()
                    );

            // Add files found if available
            QString resultTitle = m_currentSearch->showFoldersOnly ?
                                      QCoreApplication::translate("MainWindow","Folders found") : QCoreApplication::translate("MainWindow","Files found");
            builder.setResult(resultTitle, m_currentSearch->fileNames.size());

            // Check if we're in catalog loading mode
            isLoadingCatalog = searchJobStoppable->memoryModeEnabled &&
                                    searchJobStoppable->currentCatalogFilesLoaded > 0 &&
                                    searchJobStoppable->currentCatalogTotalFiles > 0;

            if (isLoadingCatalog) {
                // Show catalog loading progress
                builder.setProcess(searchJobStoppable->currentOperationVerb,  // "Loading" or similar
                                   searchJobStoppable->currentCatalogFilesLoaded,
                                   searchJobStoppable->currentCatalogTotalFiles);
            } else {
                // Show search processing progress
                int actualFilesProcessed = m_currentSearch ?
                                               m_currentSearch->totalFilesProcessed : 0;
                if (actualFilesProcessed > 0 && m_searchManager->progress() > 0) {
                    builder.setProcess(QCoreApplication::translate("MainWindow","Evaluated"),
                                       actualFilesProcessed,
                                       100 * actualFilesProcessed / m_searchManager->progress());
                }
            }

            if (m_statusBarLabel) {
                m_statusBarLabel->setText(builder.build());
            }
            m_statusBar->show();
            m_statusBarTimer->stop();
            return;
        }

        // For SearchJobStoppable, let MainWindow::updateSearchProgress handle it
        if (searchJobStoppable) {
            return;
        }

        // BUILD MESSAGE USING StatusBarMessageBuilder
        StatusBarMessageBuilder builder;
        builder.setOperation(QCoreApplication::translate("MainWindow","Search"));

        // Handle different search contexts
        if (m_currentSearch && m_currentSearch->searchInConnectedChecked) {
            // Searching in directory - no device context, show processed count
            int actualFilesProcessed = m_currentSearch->totalFilesProcessed;
            if (actualFilesProcessed > 0) {
                builder.setProcess(QCoreApplication::translate("MainWindow","Evaluated"), actualFilesProcessed, 0);  // No total for directory
            }
        }
        else if (m_currentSearch && m_currentSearch->searchInCatalogsChecked) {
            // Searching in catalogs
            if (m_currentSearch->totalCatalogs > 0) {
                if (!m_searchManager->currentCatalogName().isEmpty()) {
                    builder.setDeviceContext(
                        m_currentSearch->currentCatalogIndex,
                        m_currentSearch->totalCatalogs,
                        m_searchManager->currentCatalogName()
                        );
                }
            }

            // Add files processed with percentage for catalogs
            int actualFilesProcessed = m_currentSearch->totalFilesProcessed;
            if (actualFilesProcessed > 0 && m_searchManager->progress() > 0) {
                builder.setProcess(QCoreApplication::translate("MainWindow","Evaluated"),
                                   actualFilesProcessed,
                                   100 * actualFilesProcessed / m_searchManager->progress());
            }
        }

        // Add files/folders found
        if (m_currentSearch && m_currentSearch->fileNames.size() > 0) {
            QString resultTitle = m_currentSearch->showFoldersOnly ?
                                      QCoreApplication::translate("MainWindow","Folders found") : QCoreApplication::translate("MainWindow","Files found");
            builder.setResult(resultTitle, m_currentSearch->fileNames.size());
        }

        m_statusBar->show();
        m_statusBarLabel->setText(builder.build());
        m_statusBarTimer->stop();

    } else {
        // Search completed or ready
        StatusBarMessageBuilder builder;

        if (m_currentSearch && m_currentSearch->fileNames.size() > 0) {
            builder.setOperation(QCoreApplication::translate("MainWindow", "Search"))
            .setStatus(QCoreApplication::translate("MainWindow", "Completed"));

            // Add device context if we have catalog info
            if (m_currentSearch->totalCatalogs > 0 && !m_currentSearch->currentCatalogName.isEmpty()) {
                builder.setDeviceContext(
                    m_currentSearch->currentCatalogIndex,
                    m_currentSearch->totalCatalogs,
                    m_currentSearch->currentCatalogName
                    );
            }

            // Add files evaluated
            if (m_currentSearch->totalFilesProcessed > 0) {
                builder.setProcess(QCoreApplication::translate("MainWindow", "Evaluated"),
                                   m_currentSearch->totalFilesProcessed);
            }

            // Add results
            QString resultTitle = m_currentSearch->showFoldersOnly ?
                                      QCoreApplication::translate("MainWindow", "Folders found") :
                                      QCoreApplication::translate("MainWindow", "Files found");
            builder.setResult(resultTitle, m_currentSearch->fileNames.size());
        }

        m_statusBar->show();
        m_statusBarLabel->setText(builder.build());
        m_statusBarTimer->start(5000);
    }
}

void SearchProgressManager::showMessage(const QString &message, int timeout)
{
    if (m_statusBarLabel) {
        m_statusBarLabel->setText(message);
        m_statusBar->show();

        if (timeout > 0 && m_statusBarTimer) {
            m_statusBarTimer->start(timeout);
        } else if (m_statusBarTimer) {
            m_statusBarTimer->stop();
        }
    }
}
