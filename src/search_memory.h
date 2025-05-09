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
// File Name:   search_memory.h
// Purpose:     header for the search class that can not be stopped
// Description: https://stephanecouturier.github.io/Katalog/docs/Features/Search
// Author:      Stephane Couturier
/////////////////////////////////////////////////////////////////////////////
*///

#ifndef SEARCH_MEMORY_H
#define SEARCH_MEMORY_H

#include "search.h"

/**
 * @brief The SearchMemory class
 * Implementation of Search for memory-based (CSV files) operation
 */
class SearchMemory : public Search
{
    Q_OBJECT

public:
    /**
     * @brief SearchMemory constructor
     * @param parent Parent QObject
     */
    explicit SearchMemory(QObject *parent = nullptr);

    /**
     * @brief Main search method that coordinates the search process
     * @param selectedDevice The device to search in
     */
    void searchFiles(Device *selectedDevice);

    /**
     * @brief Load search history criteria from database
     */
    void loadSearchHistoryCriteria(const QString &connectionName);
    int currentCatalogFilesLoaded = 0;
    int currentCatalogTotalFiles = 0;

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

    /**
     * @brief Save search history to the database
     * @param connectionName Database connection name
     */
    void saveSearchHistoryToTable(const QString &connectionName) override;

};

#endif // SEARCH_MEMORY_H
