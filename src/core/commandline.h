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
// File Name:   commandline.h
// Purpose:     Class/model for the command lines
// Description:
// Author:      Stephane Couturier
/////////////////////////////////////////////////////////////////////////////
*/
// src/core/commandline.h
#ifndef COMMANDLINE_H
#define COMMANDLINE_H

#include <QObject>
#include <QCommandLineParser>
#include <QCoreApplication>
#include <QTextStream>
#include <QSqlQuery>
#include <QSqlDatabase>
#include <QSqlError>
#include "collection.h"
#include "search_memory.h"
#include "device.h"
#include "database.h"

class CommandLineHandler : public QObject
{
    Q_OBJECT

public:
    explicit CommandLineHandler(QObject *parent = nullptr);

    // Main entry point for command line processing
    int processCommandLine(QCoreApplication &app);

    // Optional: Set external collection (for compatibility)
    void setCollection(Collection *collection);

    // Command line catalog operations (moved from MainWindow)
    void cmd_updateCatalog(int deviceId, bool displayReport);
    void cmd_listGroup0Catalogs();
    void cmd_updateAllActive(bool displayReport);

private slots:
    void handleSearchProgress(int filesProcessed);

private:
    // Command line parsing
    void setupCommandLineParser();
    bool parseArguments(const QCoreApplication &app);

    // Search functionality
    int executeSearch();
    void loadLastSearchCriteria();
    void outputSearchResults();
    void outputSearchResultsCSV(const QString &filename);
    void outputSearchResultsStdout();

    // Helper methods
    bool initializeDatabase();
    QSqlError initializeDatabaseConnection(const QString &connectionName);
    void loadCollection();
    Device* getSelectedDevice();
    QString generateTimestamp();

    // Members
    QCommandLineParser parser;
    Collection *collection;
    SearchMemory *search;
    Device *selectedDevice;

    // Command line options
    bool searchRequested;
    bool outputCSV;
    QString csvFilename;
    bool verbose;
    QString collectionPath;
};

#endif // COMMANDLINE_H
