#include "searchprogressmanager.h"
#include "searchmanager.h"
#include "search.h"

void SearchProgressManager::connectToSearchManager(SearchManager *searchManager)
{
    if (!searchManager || !m_statusBar) return;

    m_searchManager = searchManager;

    // Connect to existing signals
    connect(searchManager, &SearchManager::statusChanged,
            this, &SearchProgressManager::updateFromSearchManager);
    connect(searchManager, &SearchManager::progressChanged,
            this, &SearchProgressManager::updateFromSearchManager);
    connect(searchManager, &SearchManager::currentCatalogChanged,
            this, &SearchProgressManager::updateFromSearchManager);
    connect(searchManager, &SearchManager::searchRunningChanged,
            this, &SearchProgressManager::updateFromSearchManager);

    // Connect to new signals for special progress handling
    connect(searchManager, &SearchManager::specialProgressUpdate,
            this, &SearchProgressManager::handleSpecialProgressValue);
    connect(searchManager, &SearchManager::catalogLoadingStarted,
            this, [this](const QString &catalogName) {
                handleSpecialProgressValue(-2);
            });
    connect(searchManager, &SearchManager::catalogLoadingFinished,
            this, [this]() {
                handleSpecialProgressValue(-3);
            });
    connect(searchManager, &SearchManager::searchInterrupted,
            this, [this]() {
                handleSpecialProgressValue(-1);
            });
}

void SearchProgressManager::setCurrentSearch(Search *currentSearch)
{
    m_currentSearch = currentSearch;
}

void SearchProgressManager::updateFromSearchManager()
{
    if (!m_searchManager || !m_statusBar) return;

    QString statusMessage;

    if (m_searchManager->searchRunning()) {
        // Build status message for running search
        if (m_currentSearch && m_currentSearch->totalCatalogs > 0) {
            statusMessage = tr("Searching in Catalog %1 of %2 | ")
            .arg(m_currentSearch->currentCatalogIndex)
                .arg(m_currentSearch->totalCatalogs);
        }

        if (m_currentSearch && m_currentSearch->fileNames.size() > 0) {
            statusMessage += tr("Files found: %1")
            .arg(QLocale().toString(m_currentSearch->fileNames.size()));
        } else {
            statusMessage += tr("Files found: 0");
        }

        if (m_searchManager->progress() > 0) {
            statusMessage += tr(" (%1%)").arg(m_searchManager->progress());
        }

        // Show status bar without timeout during search
        m_statusBar->show();
        m_statusBar->showMessage(statusMessage);

        // NO SIGNAL EMISSION HERE - no timer needed during search

    } else {
        // Search completed - this is when we need timer management
        if (m_currentSearch && m_currentSearch->fileNames.size() > 0) {
            statusMessage = tr("Search completed | Files found: %1")
            .arg(QLocale().toString(m_currentSearch->fileNames.size()));
        } else {
            statusMessage = m_searchManager->status();
        }

        // Show completion message
        m_statusBar->show();
        m_statusBar->showMessage(statusMessage);

        // ONLY emit signal here - when search is complete and we need timer
        emit statusBarUpdated();
    }
}

void SearchProgressManager::showMessage(const QString &message, int timeout)
{
    if (m_statusBar) {
        m_statusBar->showMessage(message, timeout);
    }
}

void SearchProgressManager::handleSpecialProgressValue(int filesProcessed)
{
    if (!m_statusBar || !m_currentSearch) return;

    QString statusMessage;
    bool useTimer = false;  // Flag to determine if we should use timer

    switch (filesProcessed) {
    case -1: // Search interrupted - final state, use timer
        statusMessage = tr("Search interrupted | Files found: %1")
                            .arg(QLocale().toString(m_currentSearch->fileNames.size()));
        useTimer = true;
        break;

    case -2: // Catalog loading started - intermediate state, no timer
        statusMessage = tr("Loading Catalog %1 of %2 (%3) | Files found: %4")
                            .arg(m_currentSearch->currentCatalogIndex)
                            .arg(m_currentSearch->totalCatalogs)
                            .arg(m_currentSearch->currentCatalogName)
                            .arg(QLocale().toString(m_currentSearch->fileNames.size()));
        useTimer = false;
        break;

    case -3: // Catalog loading finished - intermediate state, no timer
        statusMessage = tr("Processing Catalog %1 of %2 | Files found: %3 | Processing files...")
                            .arg(m_currentSearch->currentCatalogIndex)
                            .arg(m_currentSearch->totalCatalogs)
                            .arg(QLocale().toString(m_currentSearch->fileNames.size()));
        useTimer = false;
        break;

    case -4: // Catalog loading progress - intermediate state, no timer
        statusMessage = tr("Loading Catalog %1 of %2 (%3) | Files found: %4")
                            .arg(m_currentSearch->currentCatalogIndex)
                            .arg(m_currentSearch->totalCatalogs)
                            .arg(m_currentSearch->currentCatalogName)
                            .arg(QLocale().toString(m_currentSearch->fileNames.size()));
        useTimer = false;
        break;

    default:
        return;
    }

    m_statusBar->show();
    if (useTimer) {
        m_statusBar->showMessage(statusMessage, 5000);  // Auto-hide final messages
    } else {
        m_statusBar->showMessage(statusMessage);  // Keep intermediate messages visible
    }

    emit statusBarUpdated();
}
