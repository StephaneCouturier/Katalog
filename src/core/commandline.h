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

#ifndef COMMANDLINE_H
#define COMMANDLINE_H

#include <QObject>
#include <QCommandLineParser>
#include <QCoreApplication>
#include <QTextStream>
#include <QSqlQuery>
#include <QSqlDatabase>
#include <QSqlError>
#include <QEventLoop>
#include <QTimer>

#include "collection.h"
#include "device.h"
#include "database.h"
#include "search.h"
#include "searchmanager.h"
#include "searchjobstoppable.h"

class CommandLineHandler : public QObject
{
    Q_OBJECT

public:
    explicit CommandLineHandler(QObject *parent = nullptr);
    ~CommandLineHandler();

    // Main entry point for command line processing
    int processCommandLine(QCoreApplication &app);

    // Optional: Set external collection (for compatibility)
    void setCollection(Collection *collection);

    // Command line catalog operations (moved from MainWindow)
    void cmd_updateCatalog(int deviceId, bool displayReport);
    void cmd_listGroup0Catalogs();
    void cmd_updateAllActive(bool displayReport);

private slots:
    // Search completion handling
    void onSearchCompleted();
    void onSearchCancelled();
    void onSearchError(const QString &error);

    // Progress handling
    void handleSearchProgress(int filesProcessed);

private:
    QString autoDetectDatabaseMode(const QString &path);
    QString formatDeviceIDList(const QList<int> &deviceIDs);
    // Command line parsing
    void setupCommandLineParser();
    bool parseArguments(const QCoreApplication &app);

    // Search functionality
    int cmd_search();
    void sendSearchParametersFromSearchHistory(Search *search);
    void loadLastSearchCriteria();
    void outputSearchResults();
    void outputSearchResultsStdout();

    // Helper methods
    bool initializeDatabase();
    QSqlError initializeDatabaseConnection(const QString &connectionName);
    void loadCollection();
    Device* getSelectedDevice();
    QString generateTimestamp();

    // Cleanup
    void cleanup();

    // Members - Core objects
    QCommandLineParser parser;
    Collection *collection;
    SearchManager *searchManager;
    SearchJobStoppable *searchEngine;
    Device *selectedDevice;
    QEventLoop *eventLoop;

    // Search result tracking
    int searchResult;
    bool searchCompleted;

    // Command line options
    bool verbose;
    QString collectionPath;
    int outputLimit;

    // Action tracking
    QString requestedAction;
    QStringList positionalArgs;
};

#endif // COMMANDLINE_H
