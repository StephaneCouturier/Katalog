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
            QString currentMessage = m_statusBar->currentMessage();
            QString cleanMessage = currentMessage;
            cleanMessage.remove(tr(" | SEARCH PAUSED"));
            cleanMessage.remove(tr(" | CATALOG LOADING PAUSED"));

            bool isLoadingCatalog = searchJobStoppable->memoryModeEnabled &&
                                    searchJobStoppable->currentCatalogFilesLoaded > 0 &&
                                    searchJobStoppable->currentCatalogFilesLoaded < searchJobStoppable->currentCatalogTotalFiles;

            QString pauseIndicator = isLoadingCatalog ?
                                         tr(" | CATALOG LOADING PAUSED") : tr(" | SEARCH PAUSED");

            m_statusBar->showMessage(cleanMessage + pauseIndicator);
            m_statusBarTimer->stop();
            return;
        }

        // For SearchJobStoppable, let MainWindow::updateSearchProgress handle it
        if (searchJobStoppable) {
            return;
        }

        // BUILD MESSAGE USING StatusBarMessageBuilder
        StatusBarMessageBuilder builder;
        builder.setOperation(tr("SEARCH"));

        // Handle different search contexts
        if (m_currentSearch && m_currentSearch->searchInConnectedChecked) {
            // Searching in directory - no device context
            builder.setProcess(tr("Searching in directory"), 0, 0);
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
        }

        // Add files/folders found
        if (m_currentSearch && m_currentSearch->fileNames.size() > 0) {
            QString resultTitle = m_currentSearch->showFoldersOnly ?
                                      tr("Folders found") : tr("Files found");
            builder.setResult(resultTitle, m_currentSearch->fileNames.size());
        }

        // Add files processed with percentage
        int actualFilesProcessed = m_currentSearch ?
                                       m_currentSearch->totalFilesProcessed : 0;

        if (actualFilesProcessed > 0) {
            if (m_searchManager->progress() > 0) {
                builder.setProcess(tr("Processed"),
                                   actualFilesProcessed,
                                   100 * actualFilesProcessed / m_searchManager->progress());
            }
        }

        m_statusBar->show();
        m_statusBar->showMessage(builder.build());
        m_statusBarTimer->stop();

    } else {
        // Search completed or ready
        StatusBarMessageBuilder builder;

        if (m_currentSearch && m_currentSearch->fileNames.size() > 0) {
            QString resultTitle = m_currentSearch->showFoldersOnly ?
                                      tr("Folders found") : tr("Files found");

            builder.setOperation(tr("Search completed"))
                .setResult(resultTitle, m_currentSearch->fileNames.size());
        } else {
            // Just show "Ready"
            m_statusBar->showMessage(tr("Ready"));
            m_statusBarTimer->start(5000);
            return;
        }

        m_statusBar->show();
        m_statusBar->showMessage(builder.build());
        m_statusBarTimer->start(5000);
    }
}

void SearchProgressManager::showMessage(const QString &message, int timeout)
{
    if (m_statusBar) {
        m_statusBar->showMessage(message, timeout);
    }
}
