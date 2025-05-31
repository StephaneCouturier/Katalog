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
// File Name:   search_stoppable.h
// Purpose:     header for the search class that can be stopped
// Description: https://stephanecouturier.github.io/Katalog/docs/Features/Search
// Author:      Stephane Couturier
/////////////////////////////////////////////////////////////////////////////
*/
/**
 * @brief The SearchStoppable class
 * Implementation of Search for SQLite File or Hosted database operation.
 * NOTE: Because all files are already in the database, this class does NOT need
 * to load catalog files from disk like SearchMemory does. Progress reporting
 * should focus on database query execution time instead.
 */

#ifndef SEARCH_STOPPABLE_H
#define SEARCH_STOPPABLE_H

#include "search.h"
#include <QThread>
#include <QMutex>

/**
 * @brief The SearchStoppable class
 * Implementation of Search for SQLite database operation
 */
class SearchStoppable : public Search
{
    Q_OBJECT

public:
    /**
     * @brief SearchStoppable constructor
     * @param parent Parent QObject
     */
    explicit SearchStoppable(QObject *parent = nullptr);

    /**
     * @brief Destructor
     */
    virtual ~SearchStoppable();

    /**
     * @brief Main search method that coordinates the search process
     * @param selectedDevice The device to search in
     */
    void searchFiles(Device *selectedDevice);

    /**
     * @brief Stops an ongoing search
     */
    void stopSearch();

    /**
     * @brief Database connection name for this search
     */
    QString connectionName;

    bool initializeDatabase();

    bool wasStopRequested() const { return stopRequested; }

    /**
     * @brief Indicates if a search has been requested to stop
     */
    bool stopRequested;
    bool useTimerForDebug = false;

protected:
    /**
     * @brief Search for files in a catalog
     * @param device The catalog device to search in
     * @param mutex Mutex for thread safety
     * @param stopRequested Flag to indicate if the search should stop
     */
    void searchFilesInCatalog(Device *device, QMutex &mutex, bool &stopRequested) override;

    /**
     * @brief Search for files in a directory
     * @param sourceDirectory The directory to search in
     * @param mutex Mutex for thread safety
     * @param stopRequested Flag to indicate if the search should stop
     */
    void searchFilesInDirectory(const QString &sourceDirectory, QMutex &mutex, bool &stopRequested) override;

    /**
     * @brief Process duplicates in search results
     * @param connectionName Database connection name
     */
    void processDuplicates(const QString &connectionName) override;

    /**
     * @brief Process differences between catalogs
     * @param connectionName Database connection name
     */
    void processDifferences(const QString &connectionName) override;

private:
    /**
     * @brief Mutex for thread safety
     */
    QMutex mutex;
};

#endif // SEARCH_STOPPABLE_H
