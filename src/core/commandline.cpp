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
#include "core/searchjobstoppable.h"
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
    , searchManager(nullptr)
    , searchEngine(nullptr)
    , selectedDevice(nullptr)
    , eventLoop(nullptr)
    , searchResult(0)
    , searchCompleted(false)
    , verbose(false)
    , outputLimit(-1)
{
    setupCommandLineParser();
}

CommandLineHandler::~CommandLineHandler()
{
    cleanup();
}

void CommandLineHandler::cleanup()
{
    // Clean up search objects
    if (searchManager) {
        searchManager->deleteLater();
        searchManager = nullptr;
    }

    if (searchEngine) {
        searchEngine->deleteLater();
        searchEngine = nullptr;
    }

    if (selectedDevice) {
        delete selectedDevice;
        selectedDevice = nullptr;
    }

    if (eventLoop) {
        delete eventLoop;
        eventLoop = nullptr;
    }

    // Collection will be cleaned up by parent
}

void CommandLineHandler::setupCommandLineParser()
{
    parser.setApplicationDescription("Katalog - File Catalog Management and Search Tool");
    parser.addHelpOption();
    parser.addVersionOption();

    // Positional arguments
    parser.addPositionalArgument("action", "The action to be performed. Choices:\n"
                                           "  list_catalogs     - List all catalogs (ID, active state, name)\n"
                                           "  update_catalog    - Update a specific catalog (deviceID)\n"
                                           "  update_all_active - Update all active catalogs\n"
                                           "  search            - Execute search using last search criteria");

    parser.addPositionalArgument("deviceID", "Device ID for update_catalog action (optional)", "[deviceID]");

    // Options
    parser.addOption(QCommandLineOption(QStringList() << "r" << "report",
                                        "Show detailed report for update operations"));

    parser.addOption(QCommandLineOption(QStringList() << "c" << "collection",
                                        "Path to collection folder",
                                        "path"));

    parser.addOption(QCommandLineOption("verbose",
                                        "Enable verbose output"));

    parser.addOption(QCommandLineOption("limit",
                                        "Limit number of files to display",
                                        "number"));
}

bool CommandLineHandler::parseArguments(const QCoreApplication &app)
{
    parser.process(app);

    // Get options
    verbose = parser.isSet("verbose");

    if (parser.isSet("collection")) {
        collectionPath = parser.value("collection");
    }

    if (parser.isSet("limit")) {
        bool ok;
        outputLimit = parser.value("limit").toInt(&ok);
        if (!ok || outputLimit < 1) {
            QTextStream stderr_stream(stderr);
            stderr_stream << "Error: --limit must be a positive number" << Qt::endl;
            return false;
        }
        if (verbose) {
            QTextStream stdout_stream(stdout);
            stdout_stream << "Output limit set to: " << outputLimit << " files" << Qt::endl;
        }
    }

    return true;
}

int CommandLineHandler::processCommandLine(QCoreApplication &app)
{
    if (!parseArguments(app)) {
        return 1;
    }

    // Store parsed values
    positionalArgs = parser.positionalArguments();
    requestedAction = positionalArgs.value(0);

    // If no command line action, return special code to continue to GUI
    if (requestedAction.isEmpty()) {
        return -1; // Special code to indicate "continue to GUI"
    }

    if (verbose) {
        QTextStream stdout_stream(stdout);
        stdout_stream << "Katalog Command Line" << Qt::endl;
        stdout_stream << "===================" << Qt::endl;
        stdout_stream << "Action selected: \"" << requestedAction << "\"" << Qt::endl;
    }

    // Initialize collection and database for any command
    if (!initializeDatabase()) {
        QTextStream stderr_stream(stderr);
        stderr_stream << "Error: Failed to initialize database" << Qt::endl;
        return 1;
    }

    // Handle different actions
    if (requestedAction == "search") {
        return cmd_search();
    }
    else if (requestedAction == "list_catalogs") {
        cmd_listGroup0Catalogs();
        return 0;
    }
    else if (requestedAction == "update_catalog") {
        bool deviceIdOk = false;
        int deviceId = positionalArgs.value(1).toInt(&deviceIdOk);
        if (!deviceIdOk) {
            QTextStream stderr_stream(stderr);
            stderr_stream << "Error: Invalid device ID: " << positionalArgs.value(1) << Qt::endl;
            return 1;
        }
        bool displayReport = parser.isSet("report");
        cmd_updateCatalog(deviceId, displayReport);
        return 0;
    }
    else if (requestedAction == "update_all_active") {
        bool displayReport = parser.isSet("report");
        cmd_updateAllActive(displayReport);
        return 0;
    }
    else {
        QTextStream stderr_stream(stderr);
        stderr_stream << "Error: Unknown action: " << requestedAction << Qt::endl;
        return 1;
    }
}

void CommandLineHandler::setCollection(Collection *coll)
{
    // This method allows external setting of collection (for compatibility)
    if (collection == nullptr) {
        collection = coll;
    }
}

bool CommandLineHandler::initializeDatabase()
{
    // Create collection object
    collection = new Collection();
    collection->appVersion = "2.6";

    // Get user home path for settings
    QStringList standardsPaths = QStandardPaths::standardLocations(QStandardPaths::HomeLocation);
    QString homePath = standardsPaths[0];
    QString applicationDirPath = QCoreApplication::applicationDirPath();

    // Define Settings file path
    collection->settingsFilePath = applicationDirPath + "/katalog_settings.ini";
    QFile settingsFile(collection->settingsFilePath);
    if (!settingsFile.exists()) {
        collection->settingsFilePath = homePath + "/.config/katalog_settings.ini";
    }

    // Auto-detect database mode if collection path provided
    QString databaseMode;

    if (!collectionPath.isEmpty()) {
        // Auto-detect mode from provided collection path
        databaseMode = autoDetectDatabaseMode(collectionPath);

        if (databaseMode.isEmpty()) {
            QTextStream stderr_stream(stderr);
            stderr_stream << "Error: Could not determine database mode for collection path: " << collectionPath << Qt::endl;
            stderr_stream << "Path should contain either:" << Qt::endl;
            stderr_stream << "  - CSV files (device.csv, storage.csv) for Memory mode" << Qt::endl;
            stderr_stream << "  - Database file (*.db) for File mode" << Qt::endl;
            return false;
        }

        if (verbose) {
            QTextStream stdout_stream(stdout);
            stdout_stream << "Auto-detected database mode: " << databaseMode << Qt::endl;
        }
    }
    else {
        // No collection path provided - use settings
        QSettings settings(collection->settingsFilePath, QSettings::IniFormat);
        databaseMode = settings.value("Settings/databaseMode").toString();

        if (databaseMode.isEmpty()) {
            databaseMode = "Memory"; // Default
        }

        if (verbose) {
            QTextStream stdout_stream(stdout);
            stdout_stream << "Using database mode from settings: " << databaseMode << Qt::endl;
        }
    }

    if (verbose) {
        QTextStream stdout_stream(stdout);
        stdout_stream << "Settings file: " << collection->settingsFilePath << Qt::endl;
        stdout_stream << "Final database mode: " << databaseMode << Qt::endl;
    }

    // Handle paths based on detected/configured database mode
    if (!collectionPath.isEmpty()) {
        // Command line override - set collection details based on detected mode
        if (databaseMode == "Memory") {
            collection->folder = collectionPath;
        }
        else if (databaseMode == "File") {
            QFileInfo pathInfo(collectionPath);
            if (pathInfo.isFile()) {
                // Path is the .db file itself
                collection->databaseFilePath = collectionPath;
                collection->folder = pathInfo.absolutePath();
            }
            else {
                // Path is directory containing .db file - find the .db file
                QDir dir(collectionPath);
                QStringList dbFiles = dir.entryList(QStringList() << "*.db", QDir::Files);
                if (dbFiles.isEmpty()) {
                    QTextStream stderr_stream(stderr);
                    stderr_stream << "Error: No .db files found in directory: " << collectionPath << Qt::endl;
                    return false;
                }
                collection->databaseFilePath = dir.absoluteFilePath(dbFiles.first());
                collection->folder = collectionPath;

                if (verbose) {
                    QTextStream stdout_stream(stdout);
                    stdout_stream << "Using database file: " << collection->databaseFilePath << Qt::endl;
                }
            }
        }
    }
    else {
        // Use settings-based paths
        QSettings settings(collection->settingsFilePath, QSettings::IniFormat);

        if (databaseMode == "Memory") {
            collection->folder = settings.value("LastCollectionFolder").toString();
            if (collection->folder.isEmpty()) {
                collection->folder = homePath + "/.local/share/katalog";
            }
        }
        else if (databaseMode == "File") {
            collection->databaseFilePath = settings.value("Settings/DatabaseFilePath").toString();
            if (collection->databaseFilePath.isEmpty()) {
                QTextStream stderr_stream(stderr);
                stderr_stream << "Error: No database file configured for File mode" << Qt::endl;
                return false;
            }
            QFileInfo dbFileInfo(collection->databaseFilePath);
            collection->folder = dbFileInfo.absolutePath();
        }
    }

    // Validate paths exist
    if (databaseMode == "Memory") {
        QDir collectionDir(collection->folder);
        if (!collectionDir.exists()) {
            QTextStream stderr_stream(stderr);
            stderr_stream << "Error: Collection folder does not exist: " << collection->folder << Qt::endl;
            return false;
        }

        QString deviceCsvPath = collection->folder + "/device.csv";
        if (!QFile::exists(deviceCsvPath)) {
            QTextStream stderr_stream(stderr);
            stderr_stream << "Error: Collection folder does not contain device.csv: " << collection->folder << Qt::endl;
            return false;
        }
    }
    else if (databaseMode == "File") {
        if (!QFile::exists(collection->databaseFilePath)) {
            QTextStream stderr_stream(stderr);
            stderr_stream << "Error: Database file does not exist: " << collection->databaseFilePath << Qt::endl;
            return false;
        }
    }

    if (verbose) {
        QTextStream stdout_stream(stdout);
        stdout_stream << "Using collection folder: " << collection->folder << Qt::endl;
        if (databaseMode == "File") {
            stdout_stream << "Using database file: " << collection->databaseFilePath << Qt::endl;
        }
    }

    // Store our auto-detected values before Database::initialize() overwrites them
    QString detectedDatabaseMode = databaseMode;
    QString detectedDatabaseFilePath = collection->databaseFilePath;
    QString detectedCollectionFolder = collection->folder;

    if (verbose) {
        QTextStream stdout_stream(stdout);
        stdout_stream << "=== Database Initialization Debug ===" << Qt::endl;
        stdout_stream << "collectionPath provided: " << (!collectionPath.isEmpty() ? "Yes" : "No") << Qt::endl;
        stdout_stream << "Before Database::initialize():" << Qt::endl;
        stdout_stream << "  detectedDatabaseMode: " << detectedDatabaseMode << Qt::endl;
        stdout_stream << "  detectedDatabaseFilePath: " << detectedDatabaseFilePath << Qt::endl;
        stdout_stream << "  detectedCollectionFolder: " << detectedCollectionFolder << Qt::endl;
    }

    // Use Database class for initialization
    QSqlError err = Database::initialize("defaultConnection", collection);
    if (err.type() != QSqlError::NoError) {
        QTextStream stderr_stream(stderr);
        stderr_stream << "Error: Failed to initialize database: " << err.text() << Qt::endl;
        return false;
    }

    if (verbose) {
        QTextStream stdout_stream(stdout);
        stdout_stream << "After Database::initialize() (before restore):" << Qt::endl;
        stdout_stream << "  collection->databaseMode: " << collection->databaseMode << Qt::endl;
        stdout_stream << "  collection->databaseFilePath: " << collection->databaseFilePath << Qt::endl;
        stdout_stream << "  collection->folder: " << collection->folder << Qt::endl;

        // Check what database is actually connected
        QSqlDatabase db = QSqlDatabase::database("defaultConnection");
        stdout_stream << "  Actual database connected: " << db.databaseName() << Qt::endl;
        stdout_stream << "  Database is open: " << (db.isOpen() ? "Yes" : "No") << Qt::endl;
    }

    // Restore our auto-detected values after Database::initialize()
    if (!collectionPath.isEmpty()) {
        // Store original database name for comparison
        QSqlDatabase originalDb = QSqlDatabase::database("defaultConnection");
        QString originalDbName = originalDb.databaseName();

        // Restore our auto-detected values
        collection->databaseMode = detectedDatabaseMode;
        collection->databaseFilePath = detectedDatabaseFilePath;
        collection->folder = detectedCollectionFolder;

        if (verbose) {
            QTextStream stdout_stream(stdout);
            stdout_stream << "Restored auto-detected values:" << Qt::endl;
            stdout_stream << "  collection->databaseMode: " << collection->databaseMode << Qt::endl;
            stdout_stream << "  collection->databaseFilePath: " << collection->databaseFilePath << Qt::endl;
            stdout_stream << "  Original database name: " << originalDbName << Qt::endl;
            stdout_stream << "  Target database name: " << detectedDatabaseFilePath << Qt::endl;
            stdout_stream << "  Paths match: " << (originalDbName == detectedDatabaseFilePath ? "Yes" : "No") << Qt::endl;
        }

        // Only re-initialize if the database path actually changed
        if (originalDbName != detectedDatabaseFilePath) {
            if (verbose) {
                QTextStream stdout_stream(stdout);
                stdout_stream << "Database path changed - re-initializing connection..." << Qt::endl;
            }

            // Close and remove the old connection more thoroughly
            if (originalDb.isOpen()) {
                originalDb.close();
                if (verbose) {
                    QTextStream stdout_stream(stdout);
                    stdout_stream << "Closed database connection" << Qt::endl;
                }
            }

            // Force removal of the connection
            QSqlDatabase::removeDatabase("defaultConnection");
            if (verbose) {
                QTextStream stdout_stream(stdout);
                stdout_stream << "Removed database connection" << Qt::endl;
            }

            // Wait a moment for cleanup (might help with connection caching)
            QThread::msleep(10);

            // Re-initialize with our correct paths
            err = Database::initialize("defaultConnection", collection);
            if (err.type() != QSqlError::NoError) {
                QTextStream stderr_stream(stderr);
                stderr_stream << "Error: Failed to re-initialize database: " << err.text() << Qt::endl;
                return false;
            }

            if (verbose) {
                QTextStream stdout_stream(stdout);
                stdout_stream << "Database re-initialized" << Qt::endl;

                // Verify the new connection
                QSqlDatabase newDb = QSqlDatabase::database("defaultConnection");
                stdout_stream << "New database connected: " << newDb.databaseName() << Qt::endl;
                stdout_stream << "New database is open: " << (newDb.isOpen() ? "Yes" : "No") << Qt::endl;
                stdout_stream << "Target path matches: " << (newDb.databaseName() == detectedDatabaseFilePath ? "Yes" : "No") << Qt::endl;

                // Additional verification: check a simple query
                QSqlQuery testQuery(newDb);
                testQuery.exec("SELECT COUNT(*) FROM device");
                if (testQuery.next()) {
                    int deviceCount = testQuery.value(0).toInt();
                    stdout_stream << "Device count in connected database: " << deviceCount << Qt::endl;
                }
            }
        } else {
            if (verbose) {
                QTextStream stdout_stream(stdout);
                stdout_stream << "Database path unchanged - no re-initialization needed" << Qt::endl;
            }
        }
    }

    // Load collection data only for Memory mode
    if (collection->databaseMode == "Memory") {
        if (verbose) {
            QTextStream stdout_stream(stdout);
            stdout_stream << "Memory mode: Loading collection data from CSV files..." << Qt::endl;
        }
        collection->load();
    } else {
        if (verbose) {
            QTextStream stdout_stream(stdout);
            stdout_stream << collection->databaseMode << " mode: Using persistent database" << Qt::endl;
        }
    }

    if (verbose) {
        QTextStream stdout_stream(stdout);
        stdout_stream << "=== End Database Initialization Debug ===" << Qt::endl;
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
    // NOTE: This method is for Step 2, but keeping it for compatibility
    // For Step 1, we don't actually load from search history

    if (verbose) {
        QTextStream stdout_stream(stdout);
        stdout_stream << "loadLastSearchCriteria() called (Step 1: not used)" << Qt::endl;
    }

    // FIXED: Use searchEngine instead of undefined 'search' variable
    if (!searchEngine) {
        if (verbose) {
            QTextStream stdout_stream(stdout);
            stdout_stream << "No search engine available for loading criteria" << Qt::endl;
        }
        return;
    }

    // Load criteria from the last search in the search history
    searchEngine->loadSearchHistoryCriteria("defaultConnection");

    // Check if any search criteria were loaded
    QSqlQuery query(QSqlDatabase::database("defaultConnection"));
    QString querySQL = QLatin1String(R"(
        SELECT date_time, text_phrase, search_catalog_checked, search_directory_checked
        FROM search
        ORDER BY date_time DESC
        LIMIT 1
    )");
    query.prepare(querySQL);
    query.exec();

    if (query.next()) {
        QString dateTime = query.value(0).toString();
        QString searchText = query.value(1).toString();
        bool catalogsChecked = query.value(2).toBool();
        bool directoriesChecked = query.value(3).toBool();

        if (verbose) {
            QTextStream stdout_stream(stdout);
            stdout_stream << "Loaded search criteria from: " << dateTime << Qt::endl;
            stdout_stream << "Search text: \"" << searchText << "\"" << Qt::endl;
            stdout_stream << "Search in catalogs: " << (catalogsChecked ? "Yes" : "No") << Qt::endl;
            stdout_stream << "Search in directories: " << (directoriesChecked ? "Yes" : "No") << Qt::endl;
        }
    } else {
        if (verbose) {
            QTextStream stdout_stream(stdout);
            stdout_stream << "No previous search found in history" << Qt::endl;
        }
    }
}

void CommandLineHandler::sendSearchParametersFromSearchHistory(Search *search)
{
    if (!search) return;

    if (verbose) {
        QTextStream stdout_stream(stdout);
        stdout_stream << "Setting search parameters for 'return all files'..." << Qt::endl;
    }

    // Clear any existing results
    search->clearResults();

    // STEP 1: Configure search to return ALL files (no filtering)
    search->searchOnFileName = true;
    search->searchText = "";  // Empty = match all files
    search->selectedTextCriteria = Search::TEXT_CRITERIA_ALL_WORDS;
    search->selectedSearchIn = Search::SEARCH_IN_FILE_NAMES;
    search->caseSensitive = false;
    search->selectedSearchExclude = "";

    // Search location - catalogs only (not directories)
    search->searchInCatalogsChecked = true;
    search->searchInConnectedChecked = false;

    // Disable all filtering criteria
    search->searchOnFileCriteria = false;
    search->searchOnSize = false;
    search->searchOnType = false;
    search->searchOnDate = false;
    search->searchOnDuplicates = false;
    search->searchOnDifferences = false;
    search->searchOnFolderCriteria = false;
    search->searchOnTags = false;

    // Set selected devices (All devices = deviceIDList from getSelectedDevice())
    search->selectedDeviceIDList.clear();
    search->selectedDeviceIDList.append(0);

    if (verbose) {
        QTextStream stdout_stream(stdout);
        stdout_stream << "Search parameters configured:" << Qt::endl;
        stdout_stream << "  searchText: \"" << search->searchText << "\" (empty = all files)" << Qt::endl;
        stdout_stream << "  searchInCatalogsChecked: " << (search->searchInCatalogsChecked ? "Yes" : "No") << Qt::endl;
        // FIXED: Format QList<int> properly for QTextStream
        stdout_stream << "  selectedDeviceIDList: " << formatDeviceIDList(search->selectedDeviceIDList) << Qt::endl;
        stdout_stream << "  All filtering disabled (return all files)" << Qt::endl;
    }
}

Device* CommandLineHandler::getSelectedDevice()
{
    Device *allDevice = new Device();
    allDevice->ID = 0;
    allDevice->type = "All";
    allDevice->name = "All Devices";

    // THE MISSING CALL - populate deviceListTable
    allDevice->loadDevice("defaultConnection");

    return allDevice;
}

int CommandLineHandler::cmd_search()
{
    if (verbose) {
        QTextStream stdout_stream(stdout);
        stdout_stream << "Executing search..." << Qt::endl;
    }

    // Create SearchManager (same as UI)
    if (!searchManager) {
        searchManager = new SearchManager(this);

        // Connect signals for synchronous execution
        connect(searchManager, &SearchManager::searchCompleted,
                this, &CommandLineHandler::onSearchCompleted);
        connect(searchManager, &SearchManager::searchCancelled,
                this, &CommandLineHandler::onSearchCancelled);
        connect(searchManager, &SearchManager::searchError,
                this, &CommandLineHandler::onSearchError);
    }

    // Create SearchJobStoppable (same as UI)
    searchEngine = new SearchJobStoppable(this);
    searchEngine->setDatabaseConnection("defaultConnection");

    // Enable memory mode if collection is in Memory mode (same as UI)
    if (collection->databaseMode == "Memory") {
        if (verbose) {
            QTextStream stdout_stream(stdout);
            stdout_stream << "Memory mode: Enabling memory mode for SearchJobStoppable" << Qt::endl;
        }
        searchEngine->setMemoryModeEnabled(true);
    }

    // Connect progress for verbose output
    if (verbose) {
        connect(searchEngine, &Search::searchProgress, this, &CommandLineHandler::handleSearchProgress);
    }

    // FIXED: Get a single catalog device instead of virtual "All" device
    selectedDevice = getSelectedDevice();

    if (!selectedDevice) {
        QTextStream stderr_stream(stderr);
        stderr_stream << "Error: No active catalog devices found" << Qt::endl;
        return 1;
    }

    if (verbose) {
        QTextStream stdout_stream(stdout);
        stdout_stream << "Selected device - ID: " << selectedDevice->ID
                      << ", Name: " << selectedDevice->name
                      << ", Type: " << selectedDevice->type << Qt::endl;
    }

    // Set search parameters
    sendSearchParametersFromSearchHistory(searchEngine);

    if (verbose) {
        QTextStream stdout_stream(stdout);
        stdout_stream << "Starting search via SearchManager..." << Qt::endl;
    }

    // Start search via SearchManager - this should now work!
    searchManager->startSearchJobStoppable(searchEngine, selectedDevice);

    // Wait for search completion (CLI-specific)
    eventLoop = new QEventLoop(this);
    eventLoop->exec();  // Blocks until onSearchCompleted/Cancelled/Error

    // Clean up
    delete eventLoop;
    eventLoop = nullptr;

    return searchResult;
}

void CommandLineHandler::onSearchCompleted()
{
    if (verbose) {
        QTextStream stdout_stream(stdout);
        stdout_stream << Qt::endl << "Search completed successfully." << Qt::endl;

        // Get results from SearchManager
        Search* currentSearch = searchManager->getCurrentSearch();
        if (currentSearch) {
            stdout_stream << "Files found: " << currentSearch->filesFoundNumber << Qt::endl;
            stdout_stream << "Total size: " << currentSearch->filesFoundTotalSize << " octets" << Qt::endl;
        }
    }

    // Output results
    outputSearchResults();

    searchResult = 0;  // Success
    searchCompleted = true;
    if (eventLoop) {
        eventLoop->quit();
    }
}

void CommandLineHandler::onSearchCancelled()
{
    if (verbose) {
        QTextStream stdout_stream(stdout);
        stdout_stream << Qt::endl << "Search was cancelled." << Qt::endl;
    }

    searchResult = 1;  // Error
    searchCompleted = true;
    if (eventLoop) {
        eventLoop->quit();
    }
}

void CommandLineHandler::onSearchError(const QString &error)
{
    QTextStream stderr_stream(stderr);
    stderr_stream << "Search error: " << error << Qt::endl;

    searchResult = 1;  // Error
    searchCompleted = true;
    if (eventLoop) {
        eventLoop->quit();
    }
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

QString CommandLineHandler::formatDeviceIDList(const QList<int> &deviceIDs)
{
    if (deviceIDs.isEmpty()) {
        return "[]";
    }

    QStringList stringList;
    for (int id : deviceIDs) {
        stringList << QString::number(id);
    }
    return "[" + stringList.join(", ") + "]";
}

void CommandLineHandler::outputSearchResults()
{
    Search* currentSearch = searchManager->getCurrentSearch();
    if (!currentSearch) {
        QTextStream stdout_stream(stdout);
        stdout_stream << "No search results available." << Qt::endl;
        return;
    }

    outputSearchResultsStdout();
}

void CommandLineHandler::outputSearchResultsStdout()
{
    Search* currentSearch = searchManager->getCurrentSearch();
    if (!currentSearch) {
        return;
    }

    QTextStream stdout_stream(stdout);

    if (currentSearch->filesFoundNumber == 0) {
        stdout_stream << Qt::endl << "No files found." << Qt::endl;
        return;
    }

    stdout_stream << Qt::endl << "Search Results:" << Qt::endl;
    stdout_stream << "===============" << Qt::endl;

    // Apply limit if specified
    int maxDisplay = currentSearch->fileNames.size();
    if (outputLimit > 0 && outputLimit < maxDisplay) {
        maxDisplay = outputLimit;
        if (verbose) {
            stdout_stream << "Showing first " << maxDisplay << " files (limited by --limit option)" << Qt::endl;
        }
    }

    // Output files up to maxDisplay
    for (int i = 0; i < maxDisplay; ++i) {
        QString catalogName = "";
        QString fileName = "";
        QString filePath = "";
        QString fullPath = "";
        QString fileSize = "";
        QString fileDate = "";

        // Get catalog name
        if (i < currentSearch->fileCatalogs.size()) {
            catalogName = currentSearch->fileCatalogs[i];
        }

        // Get file name
        if (i < currentSearch->fileNames.size()) {
            fileName = currentSearch->fileNames[i];
        }

        // Get file path and construct full path
        if (i < currentSearch->filePaths.size()) {
            filePath = currentSearch->filePaths[i];
            // Construct full path: path + "/" + filename
            if (filePath.endsWith("/")) {
                fullPath = filePath + fileName;
            } else {
                fullPath = filePath + "/" + fileName;
            }
        }

        // Get file size
        if (i < currentSearch->fileSizes.size()) {
            fileSize = QString::number(currentSearch->fileSizes[i]);
        }

        // Get file date
        if (i < currentSearch->fileDateTimes.size()) {
            fileDate = currentSearch->fileDateTimes[i];
        }

        // Output in tab-separated format: catalog_name	file_full_path	size	date
        stdout_stream << catalogName << "\t" << fullPath << "\t" << fileSize << "\t" << fileDate << Qt::endl;
    }

    // Show summary with limit info
    if (outputLimit > 0 && currentSearch->fileNames.size() > outputLimit) {
        stdout_stream << Qt::endl;
        stdout_stream << "... and " << (currentSearch->fileNames.size() - outputLimit) << " more files (use --limit 0, or remove option --limit to show all)" << Qt::endl;
    }

    stdout_stream << Qt::endl;
    stdout_stream << "Total: " << currentSearch->filesFoundNumber << " files, "
                  << currentSearch->filesFoundTotalSize << " bytes" << Qt::endl;
}

QString CommandLineHandler::generateTimestamp()
{
    return QDateTime::currentDateTime().toString("yyyyMMdd_hhmmss");
}

QString CommandLineHandler::autoDetectDatabaseMode(const QString &path)
{
    if (path.isEmpty()) {
        return "";  // Let caller handle settings fallback
    }

    QFileInfo pathInfo(path);

    if (verbose) {
        QTextStream stdout_stream(stdout);
        stdout_stream << "Auto-detecting database mode for: " << path << Qt::endl;
    }

    // Case 1: Path is a .db file -> File mode
    if (pathInfo.isFile() && pathInfo.suffix().toLower() == "db") {
        if (verbose) {
            QTextStream stdout_stream(stdout);
            stdout_stream << "Detected File mode: path is a .db file" << Qt::endl;
        }
        return "File";
    }

    // Case 2: Path is a directory -> check contents
    if (pathInfo.isDir()) {
        QDir dir(path);

        // Look for Memory mode indicators (.csv files)
        QStringList csvFiles = dir.entryList(QStringList() << "device.csv" << "storage.csv", QDir::Files);
        bool hasMemoryFiles = csvFiles.contains("device.csv") && csvFiles.contains("storage.csv");

        // Look for File mode indicators (.db files)
        QStringList dbFiles = dir.entryList(QStringList() << "*.db", QDir::Files);
        bool hasDbFiles = !dbFiles.isEmpty();

        if (verbose) {
            QTextStream stdout_stream(stdout);
            stdout_stream << "Directory analysis:" << Qt::endl;
            stdout_stream << "  Memory mode files (device.csv, storage.csv): " << (hasMemoryFiles ? "Found" : "Not found") << Qt::endl;
            stdout_stream << "  Database files (*.db): " << (hasDbFiles ? "Found" : "Not found") << Qt::endl;
        }

        // Priority: Memory mode if CSV files found
        if (hasMemoryFiles) {
            if (verbose) {
                QTextStream stdout_stream(stdout);
                stdout_stream << "Detected Memory mode: found required CSV files" << Qt::endl;
            }
            return "Memory";
        }

        // Fallback: File mode if .db files found
        if (hasDbFiles) {
            if (verbose) {
                QTextStream stdout_stream(stdout);
                stdout_stream << "Detected File mode: found .db files in directory" << Qt::endl;
            }
            return "File";
        }

        // Could add Hosted mode detection here if there's a config file pattern
        // e.g., if (dir.exists("connection.conf")) return "Hosted";
    }

    if (verbose) {
        QTextStream stdout_stream(stdout);
        stdout_stream << "Could not auto-detect database mode from path" << Qt::endl;
    }

    return "";  // Could not determine
}
