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
// File Name:   searchjobstoppable.h
// Purpose:     header for the search class that can be stopped (based on KJob)
// Description: https://stephanecouturier.github.io/Katalog/docs/Features/Search
// Author:      Stephane Couturier
/////////////////////////////////////////////////////////////////////////////
*///

#ifndef SEARCHJOBSTOPPABLE_H
#define SEARCHJOBSTOPPABLE_H

#include "search.h"
#include <QSqlDatabase>
#include <QMutex>
#include <QAtomicInt>
#include <QThread>
/**
 * @brief The SearchJobStoppable class
 * A clean, KJob-compatible search engine designed for database operations
 * Built with QtQuick compatibility and testability in mind
 */
class SearchJobStoppable : public Search
{
    Q_OBJECT

public:
    explicit SearchJobStoppable(QObject *parent = nullptr);
    virtual ~SearchJobStoppable();

    /**
     * @brief Set the database connection to use for search operations
     * @param connectionName Name of the QSqlDatabase connection
     */
    void setDatabaseConnection(const QString &connectionName);

    /**
     * @brief Main search method that coordinates the search process
     * @param selectedDevice The device to search in
     */
    void searchFiles(Device *selectedDevice) ;

    /**
     * @brief Stops an ongoing search
     */
    void stopSearch();

    /**
     * @brief Pauses the search (can be resumed later)
     */
    void pauseSearch();

    /**
     * @brief Resumes a paused search
     */
    void resumeSearch();

    /**
     * @brief Check if search was requested to stop
     */
    bool wasStopRequested() const override { return m_stopRequested.loadAcquire(); }

    /**
     * @brief Check if search is currently paused
     */
    bool isPaused() const { return m_paused.loadAcquire(); }

    /**
     * @brief Set the progress refresh rate (files processed per progress update)
     */
    void setProgressRefreshRate(int rate) { progressRefreshRate = rate; }

    int currentCatalogFilesLoaded = 0;
    int currentCatalogTotalFiles = 0;

    bool memoryModeEnabled = false;
    QString currentFilePath;

    /**
     * @brief Enable memory mode (loads CSV files during search)
     */
    void setMemoryModeEnabled(bool enabled) { memoryModeEnabled = enabled; }

protected:
    /**
     * @brief Search for files in a catalog
     * @param device The catalog device to search in
     * @param mutex Mutex for thread safety (unused in this implementation)
     * @param stopRequested Flag to indicate if the search should stop (unused)
     */
    void searchFilesInCatalog(Device *device, QMutex &mutex, bool &stopRequested) override;

    /**
     * @brief Search for files in a directory
     * @param sourceDirectory The directory to search in
     * @param mutex Mutex for thread safety (unused in this implementation)
     * @param stopRequested Flag to indicate if the search should stop (unused)
     */
    void searchFilesInDirectory(const QString &sourceDirectory, QMutex &mutex, bool &stopRequested) override;

    /**
     * @brief Process duplicates in search results
     * @param connectionName Database connection name
     */
    void processDuplicates(const QString &connectionName) override;
    void processDuplicatesCompareDevices(const QString &connectionName);

    /**
     * @brief Process differences between catalogs
     * @param connectionName Database connection name
     */
    void processDifferences(const QString &connectionName) override;

private:
    bool shouldContinue() const;
    void waitIfPaused();

    QString m_connectionName;
    QAtomicInt m_stopRequested{0};
    QAtomicInt m_paused{0};
    mutable QMutex m_pauseMutex;

    bool m_csvLoadingStopFlag{false};
    QAtomicInt m_objectValid{1};

    QString buildJoinCondition(const QString &alias1, const QString &alias2, const QString &fields);
    QString buildExistsCondition(const QString &alias1, const QString &alias2, const QString &fields);
};

#endif // SEARCHJOBSTOPPABLE_H
