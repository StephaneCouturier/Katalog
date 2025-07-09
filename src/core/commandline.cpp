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
// File Name:   commandline.cpp
// Purpose:     Class/model for the command lines
// Description:
// Author:      Stephane Couturier
/////////////////////////////////////////////////////////////////////////////
*/
#include "commandline.h"
#include <QDir>
#include <QDateTime>
#include <QSqlDatabase>
#include <QSqlError>
#include <QDebug>
#include <QFileInfo>
#include <QStandardPaths>
#include <QSettings>
#include <QLocale>
#include <QFile>
#include <QProcess>

CommandLineHandler::CommandLineHandler(QObject *parent)
    : QObject(parent)
    , collection(nullptr)
    , search(nullptr)
    , selectedDevice(nullptr)
    , searchRequested(false)
    , outputCSV(false)
    , verbose(false)
{
    setupCommandLineParser();
}

void CommandLineHandler::setupCommandLineParser()
{
    parser.setApplicationDescription("Katalog - File Catalog Management and Search Tool");
    parser.addHelpOption();
    parser.addVersionOption();

    // Existing positional arguments (for compatibility)
    parser.addPositionalArgument("action", "The action to be performed. Choices:\n"
                                           "  list_catalogs     - List all catalogs (ID, active state, name)\n"
                                           "  update_catalog    - Update a specific catalog (deviceID)\n"
                                           "  update_all_active - Update all active catalogs\n"
                                           "  search            - Execute search using last search criteria\n"
                                           "  restart           - Restart the application.");
    parser.addPositionalArgument("deviceID", "Device ID on which the action is performed (required for update_catalog)");

    // Existing options (for compatibility)
    parser.addOption(QCommandLineOption("report", "Display a report of the operation"));

    // New search-specific options - Fix duplicate "v" option
    parser.addOption(QCommandLineOption(QStringList() << "s" << "search",
                                        "Execute search using last search criteria from the collection (alternative to positional 'search')"));

    // CSV output option
    parser.addOption(QCommandLineOption("csv",
                                        "Output search results to CSV file. Use with search action or --search option.",
                                        "filename"));

    // Collection path option
    parser.addOption(QCommandLineOption(QStringList() << "c" << "collection",
                                        "Path to collection folder",
                                        "path"));

    // Verbose option - Remove duplicate short option
    parser.addOption(QCommandLineOption("verbose",
                                        "Enable verbose output"));
}

bool CommandLineHandler::parseArguments(const QCoreApplication &app)
{
    parser.process(app);

    // Get positional arguments
    QStringList positionalArguments = parser.positionalArguments();
    QString action = positionalArguments.value(0);

    // Check for search request (either positional or option)
    searchRequested = (action == "search") || parser.isSet("search");

    outputCSV = parser.isSet("csv");
    verbose = parser.isSet("verbose");

    if (parser.isSet("csv")) {
        csvFilename = parser.value("csv");
        if (csvFilename.isEmpty()) {
            csvFilename = QString("search_results_%1.csv").arg(generateTimestamp());
        }
    }

    if (parser.isSet("collection")) {
        collectionPath = parser.value("collection");
    }

    // Validate combinations
    if (outputCSV && !searchRequested) {
        QTextStream stderr_stream(stderr);
        stderr_stream << "Error: --csv option requires search action or --search option" << Qt::endl;
        return false;
    }

    return true;
}

int CommandLineHandler::processCommandLine(QCoreApplication &app)
{
    if (!parseArguments(app)) {
        return 1;
    }

    // Get parsed values
    bool displayReport = parser.isSet("report");
    QStringList positionalArguments = parser.positionalArguments();
    QString action = positionalArguments.value(0);
    QString deviceIDStr = positionalArguments.value(1);

    // If no command line action/options, return special code to continue to GUI
    if (action.isEmpty() && !searchRequested) {
        return -1; // Special code to indicate "continue to GUI"
    }

    if (verbose) {
        QTextStream stdout_stream(stdout);
        stdout_stream << "Katalog Command Line" << Qt::endl;
        stdout_stream << "===================" << Qt::endl;
    }

    // Initialize collection and database for any command
    if (!initializeDatabase()) {
        QTextStream stderr_stream(stderr);
        stderr_stream << "Error: Failed to initialize database" << Qt::endl;
        return 1;
    }

    // Test collection folder and files
    if (verbose) {
        QTextStream stdout_stream(stdout);
        stdout_stream << "Testing collection folder..." << Qt::endl;

        QDir collectionDir(collection->folder);
        stdout_stream << "Collection folder exists: " << (collectionDir.exists() ? "Yes" : "No") << Qt::endl;

        // Check key CSV files
        QStringList keyFiles = {"device.csv", "storage.csv", "search_history.csv", "parameters.csv"};
        for (const QString &fileName : keyFiles) {
            QString filePath = collection->folder + "/" + fileName;
            QFile file(filePath);
            bool exists = file.exists();
            bool readable = exists && file.open(QIODevice::ReadOnly);
            if (readable) file.close();

            stdout_stream << fileName << " - exists: " << (exists ? "Yes" : "No")
                          << ", readable: " << (readable ? "Yes" : "No") << Qt::endl;
        }
    }

    loadCollection();

    // Handle actions
    if (!action.isEmpty()) {
        qDebug() << "";
        qDebug() << "Action selected:" << action;

        if (action == "update_catalog") {
            if (deviceIDStr.isEmpty()) {
                qWarning() << "Stopped. A device ID is required for update_catalog action.\n";
                return 1;
            }
            bool deviceIDIsInt;
            int deviceID = deviceIDStr.toInt(&deviceIDIsInt);
            if (!deviceIDIsInt) {
                qWarning() << "Stopped. The device ID must be an integer.\n";
                return 1;
            }
            cmd_updateCatalog(deviceID, displayReport);
            return 0;
        }
        else if (action == "list_catalogs") {
            cmd_listGroup0Catalogs();
            return 0;
        }
        else if (action == "update_all_active") {
            cmd_updateAllActive(displayReport);
            return 0;
        }
        else if (action == "search") {
            return executeSearch();
        }
        else if (action == "restart") {
            // Handle the restart action (existing functionality)
            qDebug() << "Restarting the application...";

            QStringList newArgs;
            bool skipNext = false;
            for (const QString &arg : QCoreApplication::arguments()) {
                if (skipNext) {
                    skipNext = false;
                    continue;
                }
                if (arg == "--catalogID" || arg == "--report" || arg == "--myoption") {
                    skipNext = true;
                } else if (arg.startsWith("--")) {
                    newArgs << arg;
                } else if (arg.startsWith("-")) {
                    newArgs << arg;
                } else {
                    continue;
                }
            }

            QProcess::startDetached(QCoreApplication::applicationFilePath(), newArgs);
            return 0;
        }
        else {
            qDebug() << "Incorrect action requested.\n"
                        "Valid actions: list_catalogs, update_catalog, update_all_active, search, restart.\n"
                        "For more information, use ./Katalog --help";
            qDebug() << "";
            return 1;
        }
    }

    // Handle standalone search option
    if (searchRequested) {
        return executeSearch();
    }

    return 0;
}

void CommandLineHandler::setCollection(Collection *coll)
{
    // This method is no longer needed since we create our own collection
    // Keeping for potential compatibility, but collection is created internally
    if (collection == nullptr) {
        collection = coll;
    }
}

bool CommandLineHandler::initializeDatabase()
{
    // Create collection object (similar to MainWindow constructor)
    collection = new Collection();
    collection->appVersion = "2.6";

    // Get user home path for settings
    QStringList standardsPaths = QStandardPaths::standardLocations(QStandardPaths::HomeLocation);
    QString homePath = standardsPaths[0];

    // Set collection folder path BEFORE loading settings
    if (!collectionPath.isEmpty()) {
        collection->folder = collectionPath;
    }

    // Set collection settings file path
    collection->settingsFilePath = homePath + "/.config/katalog_settings.ini";

    // Load from settings only if no command line path was provided
    if (collection->folder.isEmpty()) {
        QSettings settings(collection->settingsFilePath, QSettings::IniFormat);
        collection->folder = settings.value("Settings/collectionFolder").toString();

        // Use default if not set
        if (collection->folder.isEmpty()) {
            collection->folder = homePath + "/.local/share/katalog";
        }
    }

    if (verbose) {
        QTextStream stdout_stream(stdout);
        stdout_stream << "Using collection: " << collection->folder << Qt::endl;
    }

    // Verify collection exists and has data
    QDir collectionDir(collection->folder);
    if (!collectionDir.exists()) {
        QTextStream stderr_stream(stderr);
        stderr_stream << "Error: Collection folder does not exist: " << collection->folder << Qt::endl;
        stderr_stream << "Use --collection <path> to specify collection folder" << Qt::endl;
        return false;
    }

    // Check if collection has device.csv file (key indicator of existing collection)
    QString deviceCsvPath = collection->folder + "/device.csv";
    if (!QFile::exists(deviceCsvPath)) {
        QTextStream stderr_stream(stderr);
        stderr_stream << "Error: Collection folder does not contain device.csv: " << collection->folder << Qt::endl;
        stderr_stream << "This doesn't appear to be a valid Katalog collection" << Qt::endl;
        return false;
    }

    // Use the new Database class for initialization
    QSqlError err = Database::initialize("defaultConnection", collection);
    if (err.type() != QSqlError::NoError) {
        QTextStream stderr_stream(stderr);
        stderr_stream << "Error: Failed to initialize database: " << err.text() << Qt::endl;
        return false;
    }

    return true;
}

QSqlError CommandLineHandler::initializeDatabaseConnection(const QString &connectionName)
{
    // Load database settings (similar to MainWindow::initializeDatabase)
    QSettings settings(collection->settingsFilePath, QSettings::IniFormat);
    collection->databaseMode = settings.value("Settings/databaseMode").toString();
    collection->databaseFilePath = settings.value("Settings/DatabaseFilePath").toString();
    collection->databaseHostName = settings.value("Settings/databaseHostName").toString();
    collection->databaseName = settings.value("Settings/databaseName").toString();
    collection->databasePort = settings.value("Settings/databasePort").toInt();
    collection->databaseUserName = settings.value("Settings/databaseUserName").toString();
    collection->databasePassword = settings.value("Settings/databasePassword").toString();

    // Set defaults if values are not provided
    if (collection->databaseMode.isEmpty())
        collection->databaseMode = "Memory";

    // Prepare database based on selected mode
    QSqlDatabase db;

    if (collection->databaseMode == "Memory") {
        db = QSqlDatabase::addDatabase("QSQLITE", connectionName);
        db.setDatabaseName(":memory:");
    }
    else if (collection->databaseMode == "File") {
        QFile databaseFile(collection->databaseFilePath);
        if (!databaseFile.exists()) {
            return QSqlError("Database file not found", collection->databaseFilePath, QSqlError::ConnectionError);
        }
        else {
            db = QSqlDatabase::addDatabase("QSQLITE", connectionName);
            db.setDatabaseName(collection->databaseFilePath);
        }
    }
    else if (collection->databaseMode == "Hosted") {
        db = QSqlDatabase::addDatabase("QPSQL", connectionName);
        db.setHostName(collection->databaseHostName);
        db.setDatabaseName(collection->databaseName);
        db.setPort(collection->databasePort);
        db.setUserName(collection->databaseUserName);
        db.setPassword(collection->databasePassword);
    }

    // Open the database connection
    if (!db.open()) {
        return db.lastError();
    }

    return QSqlError();
}

void CommandLineHandler::loadCollection()
{
    if (verbose) {
        QTextStream stdout_stream(stdout);
        stdout_stream << "Loading collection..." << Qt::endl;

        // Debug: Check database connection
        QSqlDatabase db = QSqlDatabase::database("defaultConnection");
        stdout_stream << "Database is open: " << (db.isOpen() ? "Yes" : "No") << Qt::endl;
        stdout_stream << "Database type: " << db.driverName() << Qt::endl;
        stdout_stream << "Database name: " << db.databaseName() << Qt::endl;

        // Check if tables exist
        QStringList tables = db.tables();
        stdout_stream << "Available tables: " << tables.join(", ") << Qt::endl;
    }

    bool loaded = collection->load();

    if (verbose) {
        QTextStream stdout_stream(stdout);
        if (loaded) {
            stdout_stream << "Collection loaded successfully (defaults created)" << Qt::endl;
        } else {
            stdout_stream << "Collection loaded from existing data" << Qt::endl;
        }

        // Debug: Check if device table has any data
        QSqlDatabase db = QSqlDatabase::database("defaultConnection");
        if (db.isOpen()) {
            QSqlQuery debugQuery(db);
            if (debugQuery.exec("SELECT COUNT(*) FROM device")) {
                if (debugQuery.next()) {
                    int deviceCount = debugQuery.value(0).toInt();
                    stdout_stream << "Total devices in database: " << deviceCount << Qt::endl;
                } else {
                    stdout_stream << "Failed to get device count: " << debugQuery.lastError().text() << Qt::endl;
                }
            } else {
                stdout_stream << "Failed to execute device count query: " << debugQuery.lastError().text() << Qt::endl;
            }

            if (debugQuery.exec("SELECT COUNT(*) FROM device WHERE device_type = 'Catalog'")) {
                if (debugQuery.next()) {
                    int catalogCount = debugQuery.value(0).toInt();
                    stdout_stream << "Total catalogs in database: " << catalogCount << Qt::endl;
                } else {
                    stdout_stream << "Failed to get catalog count: " << debugQuery.lastError().text() << Qt::endl;
                }
            } else {
                stdout_stream << "Failed to execute catalog count query: " << debugQuery.lastError().text() << Qt::endl;
            }
        } else {
            stdout_stream << "Database is not open!" << Qt::endl;
        }
    }
}

void CommandLineHandler::cmd_updateCatalog(int deviceId, bool displayReport)
{
    Q_UNUSED(displayReport); // For now, not implementing the DeviceUIWrapper dependency

    qDebug() << "Updating device:   " << deviceId;

    selectedDevice = new Device();
    selectedDevice->ID = deviceId;
    selectedDevice->loadDevice("defaultConnection");
    selectedDevice->updateActive("defaultConnection");

    if(selectedDevice->type != "Catalog"){
        qDebug() << "The device selected must be a Catalog. Try with a different device ID";
        qDebug() << "Device ID: " << selectedDevice->ID;
        qDebug() << "Device Name: " << selectedDevice->name;
        qDebug() << "Device Type: " << selectedDevice->type;
        delete selectedDevice;
        return;
    }

    qDebug() << "-----------------------------------------------------------------------";
    qDebug() << "Catalog values prior to update:";
    qDebug() << "Catalog ID: " << selectedDevice->ID;
    qDebug() << "Catalog Name: " << selectedDevice->name;
    qDebug() << "Catalog Path: " << selectedDevice->path;
    qDebug() << "Catalog Type: " << selectedDevice->type;
    qDebug() << "Catalog Size: " << selectedDevice->totalFileSize;
    qDebug() << "Catalog Files: " << selectedDevice->totalFileCount;
    qDebug() << "Catalog update date: " << selectedDevice->dateTimeUpdated.toString();

    // Perform the update operation
    if(selectedDevice->active == true){
        // Note: DeviceUIWrapper::updateDeviceWithUI would need to be accessible
        // For command line, we'll call the core updateDevice method directly
        QList<qint64> updateResult = selectedDevice->updateDevice("update",
                                                                  collection->databaseMode,
                                                                  false,
                                                                  collection->folder,
                                                                  true,
                                                                  nullptr);

        // Save data
        collection->saveDeviceTableToFile();
        collection->saveStatiticsTableToFile();

        qDebug() << "---";
        qDebug() << "Catalog updated successfully.";
        qDebug() << "Catalog ID: " << selectedDevice->ID;
        qDebug() << "Catalog Name: " << selectedDevice->name;
        qDebug() << "Catalog Path: " << selectedDevice->path;
        qDebug() << "Catalog Type: " << selectedDevice->type;
        qDebug() << "Catalog Size: " << selectedDevice->totalFileSize;
        qDebug() << "Catalog Files: " << selectedDevice->totalFileCount;
        qDebug() << "Catalog update date: " << selectedDevice->dateTimeUpdated.toString();
    }
    else{
        qDebug() << "";
        qDebug() << "The Catalog was not updated as it is not active.";
        qDebug() << "";
    }

    delete selectedDevice;
    selectedDevice = nullptr;
}

void CommandLineHandler::cmd_listGroup0Catalogs()
{
    if (verbose) {
        QTextStream stdout_stream(stdout);
        stdout_stream << "Executing catalog list query..." << Qt::endl;
    }

    // Check database connection first
    QSqlDatabase db = QSqlDatabase::database("defaultConnection");
    if (!db.isOpen()) {
        qDebug() << "Database is not open!";
        return;
    }

    // Query the database for all devices of type catalog
    QSqlQuery query(db);
    QString querySQL = QLatin1String(R"(
                                    SELECT device_id, device_name, device_active
                                    FROM device
                                    WHERE device_type = 'Catalog'
                                    ORDER BY device_id
                                )");

    // Don't use prepare() for this simple query to avoid parameter issues
    if (!query.exec(querySQL)) {
        qDebug() << "Failed to execute catalog list query:" << query.lastError().text();
        qDebug() << "Query was:" << querySQL;
        return;
    }

    qDebug() << "-----------------------------------------------------------------------";
    qDebug() << "Catalogs";
    qDebug() << "-----------------------------------------------------------------------";
    qDebug() << "Device ID" << "    Active" << "      Device Name:";

    int catalogCount = 0;
    // Iterate through the results and print the device ID and name
    while (query.next()) {
        int deviceID = query.value(0).toInt();
        QString deviceName = query.value(1).toString();
        bool deviceActive = query.value(2).toBool();
        qDebug() << "  " << deviceID << "         " << deviceActive << "      " << deviceName;
        catalogCount++;
    }

    if (catalogCount == 0) {
        qDebug() << "  No catalogs found in the collection.";
        qDebug() << "  Check if the collection path is correct and contains catalog data.";

        // Debug: Show all devices
        if (verbose) {
            QTextStream stdout_stream(stdout);
            stdout_stream << "Debug: Listing all devices..." << Qt::endl;
            QSqlQuery allQuery(db);
            if (allQuery.exec("SELECT device_id, device_name, device_type FROM device")) {
                while (allQuery.next()) {
                    stdout_stream << "  ID: " << allQuery.value(0).toInt()
                    << ", Name: " << allQuery.value(1).toString()
                    << ", Type: " << allQuery.value(2).toString() << Qt::endl;
                }
            }
        }
    }

    qDebug() << "-----------------------------------------------------------------------";
}

void CommandLineHandler::cmd_updateAllActive(bool displayReport)
{
    // Select all active catalog devices from database
    QSqlQuery query(QSqlDatabase::database("defaultConnection"));
    QString querySQL = QLatin1String(R"(
                           SELECT device_id
                           FROM device
                           WHERE device_type = 'Catalog'
                           AND device_active = 1
                           ORDER BY device_id
                       )");
    query.prepare(querySQL);
    query.exec();

    // Update each catalog
    while (query.next()) {
        int deviceID = query.value(0).toInt();
        qDebug() << "processing:   " << deviceID;
        cmd_updateCatalog(deviceID, displayReport);
    }

    qDebug() << "-----------------------------------------------------------------------";
    qDebug() << "All active catalogs updated";
    qDebug() << "-----------------------------------------------------------------------";
}

void CommandLineHandler::loadLastSearchCriteria()
{
    // Get the most recent search from history
    QSqlQuery query(QSqlDatabase::database("defaultConnection"));
    QString querySQL = QLatin1String(R"(
        SELECT MAX(date_time)
        FROM search
    )");
    query.prepare(querySQL);
    query.exec();

    if (query.next()) {
        QString lastSearchDateTime = query.value(0).toString();

        if (!lastSearchDateTime.isEmpty()) {
            search->searchDateTime = lastSearchDateTime;
            search->loadSearchHistoryCriteria("defaultConnection");

            if (verbose) {
                QTextStream stdout_stream(stdout);
                stdout_stream << "Loaded search criteria from: " << lastSearchDateTime << Qt::endl;
                stdout_stream << "Search text: \"" << search->searchText << "\"" << Qt::endl;
                stdout_stream << "Search in catalogs: " << (search->searchInCatalogsChecked ? "Yes" : "No") << Qt::endl;
                stdout_stream << "Search in directories: " << (search->searchInConnectedChecked ? "Yes" : "No") << Qt::endl;
            }
        } else {
            if (verbose) {
                QTextStream stdout_stream(stdout);
                stdout_stream << "No previous search found, using default criteria" << Qt::endl;
            }
            // Set minimal default search criteria
            search->searchOnFileName = true;
            search->searchText = "*"; // Search for all files
            search->selectedTextCriteria = Search::TEXT_CRITERIA_ALL_WORDS;
            search->selectedSearchIn = Search::SEARCH_IN_FILE_NAMES;
            search->searchInCatalogsChecked = true;
            search->searchInConnectedChecked = false;
        }
    }
}

Device* CommandLineHandler::getSelectedDevice()
{
    // For command line, search in all devices (create a virtual "All" device)
    Device *allDevice = new Device();
    allDevice->ID = 0;
    allDevice->type = "All";
    allDevice->name = "All Devices";

    // Load the device list manually since loadSubDeviceList is private
    // Query for all active catalog devices
    QSqlQuery query(QSqlDatabase::database("defaultConnection"));
    QString querySQL = QLatin1String(R"(
        SELECT device_id, device_type
        FROM device
        WHERE device_type = 'Catalog'
        AND device_active = 1
        ORDER BY device_id
    )");
    query.prepare(querySQL);
    query.exec();

    while (query.next()) {
        int deviceId = query.value(0).toInt();
        allDevice->deviceIDList.append(deviceId);
    }

    return allDevice;
}

int CommandLineHandler::executeSearch()
{
    if (verbose) {
        QTextStream stdout_stream(stdout);
        stdout_stream << "Executing search..." << Qt::endl;
    }

    // Create search object
    search = new SearchMemory(this);

    // Connect progress signal for verbose output
    if (verbose) {
        connect(search, &Search::searchProgress, this, &CommandLineHandler::handleSearchProgress);
    }

    // Load search criteria from last search
    loadLastSearchCriteria();

    // Get device to search in
    selectedDevice = getSelectedDevice();

    if (selectedDevice->deviceIDList.isEmpty()) {
        QTextStream stderr_stream(stderr);
        stderr_stream << "Warning: No catalogs found in collection" << Qt::endl;
        if (!search->searchInConnectedChecked) {
            stderr_stream << "Error: No catalogs to search and directory search not enabled" << Qt::endl;
            return 1;
        }
    }

    // Execute the search
    search->searchFiles(selectedDevice);

    if (verbose) {
        QTextStream stdout_stream(stdout);
        stdout_stream << Qt::endl; // New line after progress
        stdout_stream << "Search completed." << Qt::endl;
        stdout_stream << "Files found: " << search->filesFoundNumber << Qt::endl;
        stdout_stream << "Total size: " << QLocale().formattedDataSize(search->filesFoundTotalSize) << Qt::endl;
    }

    // Output results
    outputSearchResults();

    // Cleanup
    delete search;
    delete selectedDevice;

    return 0;
}

void CommandLineHandler::handleSearchProgress(int filesProcessed)
{
    QTextStream stdout_stream(stdout);

    // Handle special progress values
    if (filesProcessed == -2) {
        stdout_stream << "Loading catalog..." << Qt::flush;
        return;
    } else if (filesProcessed == -3) {
        stdout_stream << "Processing..." << Qt::flush;
        return;
    } else if (filesProcessed == -4) {
        stdout_stream << "." << Qt::flush;
        return;
    }

    // Regular progress
    if (filesProcessed > 0 && filesProcessed % 1000 == 0) {
        stdout_stream << "." << Qt::flush;
    }
}

void CommandLineHandler::outputSearchResults()
{
    if (outputCSV) {
        outputSearchResultsCSV(csvFilename);
        if (verbose) {
            QTextStream stdout_stream(stdout);
            stdout_stream << "Results exported to: " << csvFilename << Qt::endl;
        }
    } else {
        outputSearchResultsStdout();
    }
}

void CommandLineHandler::outputSearchResultsCSV(const QString &filename)
{
    QFile csvFile(filename);
    if (!csvFile.open(QIODevice::WriteOnly | QIODevice::Text)) {
        QTextStream stderr_stream(stderr);
        stderr_stream << "Error: Cannot create CSV file: " << filename << Qt::endl;
        return;
    }

    QTextStream out(&csvFile);

    // Write header
    out << "Name,Size,Date,Folder,Catalog" << Qt::endl;

    // Write data
    for (int i = 0; i < search->fileNames.size(); ++i) {
        QString name = search->fileNames[i];
        QString size = QString::number(search->fileSizes[i]);
        QString date = search->fileDateTimes[i];
        QString folder = search->filePaths[i];
        QString catalog = search->fileCatalogs[i];

        // Escape CSV fields that contain commas or quotes
        auto escapeCSV = [](const QString &field) {
            if (field.contains(',') || field.contains('"') || field.contains('\n')) {
                QString escaped = field;
                escaped.replace("\"", "\"\"");
                return "\"" + escaped + "\"";
            }
            return field;
        };

        out << escapeCSV(name) << ","
            << escapeCSV(size) << ","
            << escapeCSV(date) << ","
            << escapeCSV(folder) << ","
            << escapeCSV(catalog) << Qt::endl;
    }

    csvFile.close();
}

void CommandLineHandler::outputSearchResultsStdout()
{
    QTextStream stdout_stream(stdout);

    if (search->filesFoundNumber == 0) {
        stdout_stream << "No files found." << Qt::endl;
        return;
    }

    // Output header
    stdout_stream << QString("%-40s %10s %20s %s").arg("Name", "Size", "Date", "Path") << Qt::endl;
    stdout_stream << QString(80, '-') << Qt::endl;

    // Output results
    for (int i = 0; i < search->fileNames.size(); ++i) {
        QString name = search->fileNames[i];
        if (name.length() > 37) {
            name = name.left(34) + "...";
        }

        QString size = QLocale().formattedDataSize(search->fileSizes[i]);
        QString date = search->fileDateTimes[i].left(16); // Just date part
        QString fullPath = search->filePaths[i] + "/" + search->fileNames[i];

        stdout_stream << QString("%-40s %10s %20s %s").arg(name, size, date, fullPath) << Qt::endl;
    }

    stdout_stream << QString(80, '-') << Qt::endl;
    stdout_stream << QString("Total: %1 files, %2")
                         .arg(search->filesFoundNumber)
                         .arg(QLocale().formattedDataSize(search->filesFoundTotalSize)) << Qt::endl;
}

QString CommandLineHandler::generateTimestamp()
{
    return QDateTime::currentDateTime().toString("yyyyMMdd_hhmmss");
}
