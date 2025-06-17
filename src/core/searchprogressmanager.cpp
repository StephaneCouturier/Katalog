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

    QString message;

    if (m_searchManager->searchRunning()) {
        // Build message format:
        // "Searching in catalog catalogname | catalog 2 of 5 | Total files/folders found: 13 | Total files processed: 20000 (9%)"

        if (m_currentSearch && m_currentSearch->searchInConnectedChecked) {
            message = tr("Searching in directory %1").arg(m_currentSearch->connectedDirectory);
        }
        else if (m_currentSearch && m_currentSearch->searchInCatalogsChecked) {
            // Handle catalog search display
            if (m_currentSearch->totalCatalogs > 0) {
                // Start with catalog name
                if (!m_searchManager->currentCatalogName().isEmpty()) {
                    message = tr("Searching in catalog %1").arg(m_searchManager->currentCatalogName());
                } else {
                    message = tr("Searching in catalog");
                }

                // Add catalog position if multiple catalogs
                if (m_currentSearch->totalCatalogs > 1) {
                    message += tr(" | Catalog %1 of %2")
                    .arg(m_currentSearch->currentCatalogIndex)
                        .arg(m_currentSearch->totalCatalogs);
                }
            } else {
                // Single catalog or no catalog info
                if (!m_searchManager->currentCatalogName().isEmpty()) {
                    message = tr("Searching in catalog %1").arg(m_searchManager->currentCatalogName());
                } else {
                    message = tr("Searching");
                }
            }
        }
        else {
            message = tr("Searching");
        }

        // Add total files/folders found - check showFoldersOnly flag
        if (m_currentSearch && m_currentSearch->fileNames.size() > 0) {
            if (m_currentSearch->showFoldersOnly) {
                message += tr(" | Total folders found: %1")
                .arg(QLocale().toString(m_currentSearch->fileNames.size()));
            } else {
                message += tr(" | Total files found: %1")
                .arg(QLocale().toString(m_currentSearch->fileNames.size()));
            }
        } else {
            if (m_currentSearch && m_currentSearch->showFoldersOnly) {
                message += tr(" | Total folders found: 0");
            } else {
                message += tr(" | Total files found: 0");
            }
        }

        // Add total files processed with percentage (using actual count from SearchManager)
        int actualFilesProcessed = m_currentSearch ? m_currentSearch->totalFilesProcessed : 0;
        if (actualFilesProcessed > 0) {
            message += tr(" | Total files processed: %1")
            .arg(QLocale().toString(actualFilesProcessed));

            if (m_searchManager->progress() > 0) {
                message += tr(" (%1%)").arg(m_searchManager->progress());
            }
        }

        // Show status bar without timeout during search
        m_statusBar->show();
        m_statusBar->showMessage(message);

        // Stop any existing timer during search
        if (m_statusBarTimer) {
            m_statusBarTimer->stop();
        }

    } else {
        // Search completed or ready
        if (m_currentSearch && m_currentSearch->fileNames.size() > 0) {
            if (m_currentSearch->showFoldersOnly) {
                message = tr("Search completed | Total folders found: %1")
                .arg(QLocale().toString(m_currentSearch->fileNames.size()));
            } else {
                message = tr("Search completed | Total files found: %1")
                .arg(QLocale().toString(m_currentSearch->fileNames.size()));
            }
        } else {
            message = m_searchManager->status();
        }

        // Show completion message
        m_statusBar->show();
        m_statusBar->showMessage(message);

        // Start timer ONLY when search is complete
        if (m_statusBarTimer) {
            m_statusBarTimer->start(5000);
        }
    }
}
void SearchProgressManager::showMessage(const QString &message, int timeout)
{
    if (m_statusBar) {
        m_statusBar->showMessage(message, timeout);
    }
}
