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
        // Build dynamic status while search is running
        message = m_searchManager->status();

        if (m_searchManager->progress() > 0) {
            message += QString(" (%1%)").arg(m_searchManager->progress());
        }

        if (!m_searchManager->currentCatalog().isEmpty()) {
            message += QString(" - %1").arg(m_searchManager->currentCatalog());
        }

        // Add file count if available
        if (m_currentSearch && m_currentSearch->fileNames.size() > 0) {
            message += QString(" | Files found: %1")
            .arg(QLocale().toString(m_currentSearch->fileNames.size()));
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
            message = tr("Search completed | Files found: %1")
            .arg(QLocale().toString(m_currentSearch->fileNames.size()));
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
