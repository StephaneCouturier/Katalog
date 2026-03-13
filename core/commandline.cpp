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
#include "core/catalogjobstoppable.h"
#include "core/catalogmanager.h"
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
    , useSearchHistory(true)
    , overrideCaseSensitive(false)
    , searchCriteriaProvided(false)
{
    setupCommandLineParser();
}

CommandLineHandler::~CommandLineHandler()
{
    cleanup();
}

//--- Initialize ------------------------------------------------------------------------

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
    parser.setApplicationDescription(
        "Katalog - File Catalog Management and Search Tool\n\n"
        "Examples:\n"
        "  katalog search                                     # Use collection and device from settings\n"
        "  katalog search --selectedDeviceID 4                # Use collection from settings, device ID 4\n"
        "  katalog search --collection /path/to/collection    # Use specified collection, all devices\n"
        "  katalog search --collection /path/to/collection --selectedDeviceID 4  # Use specified collection, device ID 4\n"
        "  katalog search --collection /path/to/collection --selectedDeviceID 0  # Use specified collection, all devices (explicit)"
        );

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
    //Report option deactivated for now
    // parser.addOption(QCommandLineOption(QStringList() << "r" << "report",
    //                                     "Show detailed report for update operations"));

    parser.addOption(QCommandLineOption(QStringList() << "c" << "collection",
                                        "Path to collection folder",
                                        "path"));

    parser.addOption(QCommandLineOption("verbose",
                                        "Enable verbose output"));

    parser.addOption(QCommandLineOption("limit",
                                        "Limit number of files to display",
                                        "number"));

    parser.addOption(QCommandLineOption("selectedDeviceID",
                                        "Device ID to search in. Default: use settings file value, or 0 (All) when used with --collection",
                                        "deviceID"));

    // Search criteria override options
    parser.addOption(QCommandLineOption("text",
                                        "Search text/phrase to look for",
                                        "search-term"));

    parser.addOption(QCommandLineOption("type",
                                        "Filter by file type: all, audio, image, text, video",
                                        "file-type", "all"));

    parser.addOption(QCommandLineOption("size-min",
                                        "Minimum file size (e.g. 1MB, 5GB)",
                                        "size"));

    parser.addOption(QCommandLineOption("size-max",
                                        "Maximum file size (e.g. 100MB, 2GB)",
                                        "size"));

    parser.addOption(QCommandLineOption("date-after",
                                        "Files modified after date (YYYY-MM-DD)",
                                        "date"));

    parser.addOption(QCommandLineOption("date-before",
                                        "Files modified before date (YYYY-MM-DD)",
                                        "date"));

    parser.addOption(QCommandLineOption("case-sensitive",
                                        "Enable case-sensitive text search"));

    parser.addOption(QCommandLineOption("search-in",
                                        "Search scope: filenames, files-and-folders, folder-paths",
                                        "scope", "filenames"));

    parser.addOption(QCommandLineOption("text-criteria",
                                        "Text matching: all-words, exact-phrase, begins-with, any-word",
                                        "criteria", "all-words"));

    parser.addOption(QCommandLineOption("exclude",
                                        "Exclude files containing these terms",
                                        "exclude-terms"));

    parser.addOption(QCommandLineOption("no-history",
                                        "Start with default criteria instead of loading search history"));
}

bool CommandLineHandler::parseArguments(const QCoreApplication &app)
{
    parser.process(app);

    // Get options
    verbose = parser.isSet("verbose");

    // Parse collection option
    if (parser.isSet("collection")) {
        collectionPath = parser.value("collection");
    }

    // Parse limit option
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

    // Parse selectedDeviceID option
    selectedDeviceID = 0; // Default value
    selectedDeviceIDProvided = false;

    if (parser.isSet("selectedDeviceID")) {
        bool ok;
        selectedDeviceID = parser.value("selectedDeviceID").toInt(&ok);
        selectedDeviceIDProvided = true;

        if (!ok || selectedDeviceID < 0) {
            QTextStream stderr_stream(stderr);
            stderr_stream << "Error: --selectedDeviceID must be a non-negative number" << Qt::endl;
            return false;
        }

        if (verbose) {
            QTextStream stdout_stream(stdout);
            stdout_stream << "Command line selectedDeviceID: " << selectedDeviceID << Qt::endl;
        }
    }

    // Parse search criteria overrides
    searchCriteriaProvided = false;
    useSearchHistory = !parser.isSet("no-history");

    if (parser.isSet("text")) {
        overrideSearchText = parser.value("text");
        searchCriteriaProvided = true;
    }

    if (parser.isSet("type")) {
        overrideFileType = parser.value("type").toLower();
        QStringList validTypes = {"all", "audio", "image", "text", "video"};
        if (!validTypes.contains(overrideFileType)) {
            QTextStream stderr_stream(stderr);
            stderr_stream << "Error: --type must be one of: " << validTypes.join(", ") << Qt::endl;
            return false;
        }
        searchCriteriaProvided = true;
    }

    if (parser.isSet("size-min")) {
        overrideSizeMin = parser.value("size-min");
        if (!validateSizeFormat(overrideSizeMin)) {
            QTextStream stderr_stream(stderr);
            stderr_stream << "Error: Invalid --size-min format. Use format like: 1MB, 5GB, 100KB" << Qt::endl;
            return false;
        }
        searchCriteriaProvided = true;
    }

    if (parser.isSet("size-max")) {
        overrideSizeMax = parser.value("size-max");
        if (!validateSizeFormat(overrideSizeMax)) {
            QTextStream stderr_stream(stderr);
            stderr_stream << "Error: Invalid --size-max format. Use format like: 1MB, 5GB, 100KB" << Qt::endl;
            return false;
        }
        searchCriteriaProvided = true;
    }

    if (parser.isSet("date-after")) {
        QString dateStr = parser.value("date-after");
        if (!validateDateFormat(dateStr)) {
            QTextStream stderr_stream(stderr);
            stderr_stream << "Error: Invalid --date-after format. Use YYYY-MM-DD format" << Qt::endl;
            return false;
        }
        overrideDateAfter = parseDate(dateStr);
        searchCriteriaProvided = true;
    }

    if (parser.isSet("date-before")) {
        QString dateStr = parser.value("date-before");
        if (!validateDateFormat(dateStr)) {
            QTextStream stderr_stream(stderr);
            stderr_stream << "Error: Invalid --date-before format. Use YYYY-MM-DD format" << Qt::endl;
            return false;
        }
        overrideDateBefore = parseDate(dateStr);
        searchCriteriaProvided = true;
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
    // Initialize collection object
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
    QString databaseFilePath;

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

        // Set up collection paths based on detected mode
        if (databaseMode == "Memory") {
            collection->folder = collectionPath;
        }
        else if (databaseMode == "File") {
            QFileInfo pathInfo(collectionPath);
            if (pathInfo.isFile()) {
                // Path is the .db file itself
                databaseFilePath = collectionPath;
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
                databaseFilePath = dir.absoluteFilePath(dbFiles.first());
                collection->folder = collectionPath;

                if (verbose) {
                    QTextStream stdout_stream(stdout);
                    stdout_stream << "Using database file: " << databaseFilePath << Qt::endl;
                }
            }
        }
    }
    else {
        // No collection path provided - use settings (Database::initialize will load from settings)
        QSettings settings(collection->settingsFilePath, QSettings::IniFormat);
        databaseMode = settings.value("Settings/databaseMode").toString();

        if (verbose) {
            QTextStream stdout_stream(stdout);
            stdout_stream << "Loading settings from: " << collection->settingsFilePath << Qt::endl;
            stdout_stream << "Available settings keys: " << settings.allKeys().join(", ") << Qt::endl;
        }

        if (databaseMode.isEmpty()) {
            databaseMode = "Memory"; // Default
        }

        // Load collection paths from settings based on mode
        if (databaseMode == "Memory") {
            collection->folder = settings.value("LastCollectionFolder").toString();
            if (verbose) {
                QTextStream stdout_stream(stdout);
                stdout_stream << "Loaded LastCollectionFolder from settings: '" << collection->folder << "'" << Qt::endl;
            }
            if (collection->folder.isEmpty()) {
                collection->folder = homePath + "/.local/share/katalog";
                if (verbose) {
                    QTextStream stdout_stream(stdout);
                    stdout_stream << "LastCollectionFolder was empty, using default: " << collection->folder << Qt::endl;
                }
            }
        }
        else if (databaseMode == "File") {
            databaseFilePath = settings.value("Settings/DatabaseFilePath").toString();
            if (verbose) {
                QTextStream stdout_stream(stdout);
                stdout_stream << "Loaded DatabaseFilePath from settings: '" << databaseFilePath << "'" << Qt::endl;
            }
            if (databaseFilePath.isEmpty()) {
                QTextStream stderr_stream(stderr);
                stderr_stream << "Error: No database file configured for File mode" << Qt::endl;
                return false;
            }
            QFileInfo dbFileInfo(databaseFilePath);
            collection->folder = dbFileInfo.absolutePath();
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
        if (!QFile::exists(databaseFilePath)) {
            QTextStream stderr_stream(stderr);
            stderr_stream << "Error: Database file does not exist: " << databaseFilePath << Qt::endl;
            return false;
        }
    }

    if (verbose) {
        QTextStream stdout_stream(stdout);
        stdout_stream << "Using collection folder: " << collection->folder << Qt::endl;
        if (databaseMode == "File") {
            stdout_stream << "Using database file: " << databaseFilePath << Qt::endl;
        }
    }

    if (verbose) {
        QTextStream stdout_stream(stdout);
        stdout_stream << "=== Database Initialization Debug ===" << Qt::endl;
        stdout_stream << "collectionPath provided: " << (!collectionPath.isEmpty() ? "Yes" : "No") << Qt::endl;
        stdout_stream << "Before Database::initialize():" << Qt::endl;
        stdout_stream << "  databaseMode: " << databaseMode << Qt::endl;
        stdout_stream << "  databaseFilePath: " << databaseFilePath << Qt::endl;
        stdout_stream << "  collectionFolder: " << collection->folder << Qt::endl;
    }

    // Use Database class for initialization with optional overrides
    QSqlError err;
    if (!collectionPath.isEmpty()) {
        // Command line override - pass detected values to override settings
        err = Database::initialize(m_connectionName, collection, databaseMode, databaseFilePath);
    } else {
        // No command line override - use normal settings-based initialization
        // Note: collection->folder was already set from settings above for Memory mode
        err = Database::initialize(m_connectionName, collection);
    }

    if (err.type() != QSqlError::NoError) {
        QTextStream stderr_stream(stderr);
        stderr_stream << "Error: Failed to initialize database: " << err.text() << Qt::endl;
        return false;
    }

    if (verbose) {
        QTextStream stdout_stream(stdout);
        stdout_stream << "After Database::initialize():" << Qt::endl;
        stdout_stream << "  collection->databaseMode: " << collection->databaseMode << Qt::endl;
        stdout_stream << "  collection->databaseFilePath: " << collection->databaseFilePath << Qt::endl;
        stdout_stream << "  collection->folder: " << collection->folder << Qt::endl;

        // Check what database is actually connected
        QSqlDatabase db = QSqlDatabase::database(m_connectionName);
        stdout_stream << "  Actual database connected: " << db.databaseName() << Qt::endl;
        stdout_stream << "  Database is open: " << (db.isOpen() ? "Yes" : "No") << Qt::endl;

        // Additional verification: check a simple query
        QSqlQuery testQuery(db);
        testQuery.exec("SELECT COUNT(*) FROM device");
        if (testQuery.next()) {
            int deviceCount = testQuery.value(0).toInt();
            stdout_stream << "Device count in connected database: " << deviceCount << Qt::endl;
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
        QString driver;
        if (collection->databaseType == "PostgreSQL") {
            driver = "QPSQL";
        } else {
            driver = "QMYSQL";  // Default to MySQL
        }

        db = QSqlDatabase::addDatabase(driver, connectionName);
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
        QSqlDatabase db = QSqlDatabase::database(m_connectionName);
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
        QSqlDatabase db = QSqlDatabase::database(m_connectionName);
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
    searchEngine->loadSearchHistoryCriteria(m_connectionName);

    // Check if any search criteria were loaded
    QSqlQuery query(QSqlDatabase::database(m_connectionName));
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

    // Setting search parameters for 'return all files'
    bool tempSearchSkipCriteria = false;
    if (tempSearchSkipCriteria) {
        // ... existing code for skipping criteria ...
    }
    else {
        QTextStream stdout_stream(stdout);
        stdout_stream << "Setting search parameters from search history..." << Qt::endl;

        // Get the latest search date first and set it on the search object
        QSqlQuery query(QSqlDatabase::database(m_connectionName));
        QString querySQL = QLatin1String(R"(
            SELECT date_time
            FROM search
            ORDER BY date_time DESC
            LIMIT 1
        )");
        query.prepare(querySQL);
        query.exec();

        if (query.next()) {
            QString latestSearchDateTime = query.value(0).toString();
            search->searchDateTime = latestSearchDateTime;

            if (verbose) {
                stdout_stream << "Loading search criteria from: " << latestSearchDateTime << Qt::endl;
            }
        } else {
            if (verbose) {
                stdout_stream << "No previous search found in history" << Qt::endl;
            }
            search->searchDateTime = "";
        }

        // Load from search history (gets the ready-to-use fields)
        if (useSearchHistory) {
            search->loadSearchHistoryCriteria(m_connectionName);
        }

        // Apply command line overrides (this overrides history values)
        if (searchCriteriaProvided) {
            if (verbose) {
                stdout_stream << "Applying command line search criteria overrides..." << Qt::endl;
            }

            // Apply text search overrides
            if (parser.isSet("text")) {
                search->searchOnFileName = true;
                search->searchText = overrideSearchText;
                if (verbose) stdout_stream << "  Override search text: \"" << search->searchText << "\"" << Qt::endl;
            }

            // Apply file type override
            if (parser.isSet("type")) {
                search->searchOnType = (overrideFileType != "all");
                search->selectedFileType = (overrideFileType == "all") ? "All" :
                                               overrideFileType.left(1).toUpper() + overrideFileType.mid(1);
                if (verbose) stdout_stream << "  Override file type: " << search->selectedFileType << Qt::endl;
            }

            // Apply size overrides
            if (parser.isSet("size-min") || parser.isSet("size-max")) {
                search->searchOnSize = true;
                if (parser.isSet("size-min")) {
                    parseSizeValue(overrideSizeMin, search->selectedMinimumSize, search->selectedMinSizeUnit);
                }
                if (parser.isSet("size-max")) {
                    parseSizeValue(overrideSizeMax, search->selectedMaximumSize, search->selectedMaxSizeUnit);
                }
                search->setMultipliers();
                if (verbose) stdout_stream << "  Override size range: "
                                  << search->selectedMinimumSize << search->selectedMinSizeUnit
                                  << " - " << search->selectedMaximumSize << search->selectedMaxSizeUnit << Qt::endl;
            }

            // Apply date overrides
            if (parser.isSet("date-after") || parser.isSet("date-before")) {
                search->searchOnDate = true;

                if (parser.isSet("date-after")) {
                    search->selectedDateMin = overrideDateAfter;
                }
                if (parser.isSet("date-before")) {
                    search->selectedDateMax = overrideDateBefore;
                }

                if (verbose) {
                    stdout_stream << "  Override date range: "
                                  << search->selectedDateMin.toString("yyyy-MM-dd")
                                  << " to " << search->selectedDateMax.toString("yyyy-MM-dd") << Qt::endl;
                }
            }

            // Apply case sensitive override
            if (parser.isSet("case-sensitive")) {
                search->caseSensitive = true;
                if (verbose) stdout_stream << "  Override case sensitive: enabled" << Qt::endl;
            }
        }

        // Set selected devices (use CLI device selection)
        search->selectedDeviceIDList.clear();
        if (selectedDevice && selectedDevice->ID != 0) {
            search->selectedDeviceIDList.append(selectedDevice->ID);
        } else {
            search->selectedDeviceIDList.append(0);  // All devices
        }

        // ... rest of existing code ...
    }
}

Device* CommandLineHandler::getSelectedDevice()
{
    Device *device = new Device();

    if (selectedDeviceIDProvided) {
        // --selectedDeviceID was explicitly provided: use it regardless of collection source
        device->ID = selectedDeviceID;

        if (verbose) {
            QTextStream stdout_stream(stdout);
            stdout_stream << "Using selectedDeviceID from command line: ID=" << device->ID;
            if (collectionPath.isEmpty()) {
                stdout_stream << " (with collection from settings file)";
            } else {
                stdout_stream << " (with collection from --collection)";
            }
            stdout_stream << Qt::endl;
        }
    } else if (collectionPath.isEmpty()) {
        // No --collection and no --selectedDeviceID: use selectedDevice from settings file
        QSettings settings(collection->settingsFilePath, QSettings::IniFormat);
        device->ID = settings.value("Selection/SelectedDeviceID", 0).toInt();

        if (verbose) {
            QTextStream stdout_stream(stdout);
            stdout_stream << "Using selectedDevice from settings file: ID=" << device->ID << Qt::endl;
        }
    } else {
        // --collection specified but no --selectedDeviceID: default to 0 (All)
        device->ID = 0;

        if (verbose) {
            QTextStream stdout_stream(stdout);
            stdout_stream << "Using default selectedDeviceID=0 (All) with collection from --collection" << Qt::endl;
        }
    }

    // Load device details
    if (device->ID == 0) {
        device->type = "All";
        device->name = "All Devices";
    }

    device->loadDevice(m_connectionName);

    return device;
}

//--- Commands ---------------------------------------------------------------------------
void CommandLineHandler::cmd_updateCatalog(int deviceId, bool displayReport)
{
    Q_UNUSED(displayReport); // TODO: Implement reporting for command line

    qInfo() << "Updating device:" << deviceId;

    // Load and validate the device
    selectedDevice = new Device();
    selectedDevice->ID = deviceId;
    selectedDevice->loadDevice(m_connectionName);
    selectedDevice->updateActiveState(m_connectionName);

    if (selectedDevice->type != "Catalog") {
        qInfo() << "The device selected must be a Catalog. Try with a different device ID";
        qInfo() << "Device ID:" << selectedDevice->ID;
        qInfo() << "Device Name:" << selectedDevice->name;
        qInfo() << "Device Type:" << selectedDevice->type;
        delete selectedDevice;
        return;
    }

    if (!selectedDevice->active) {
        qInfo() << "";
        qInfo() << "The Catalog was not updated as it is not active.";
        qInfo() << "Device ID:" << selectedDevice->ID;
        qInfo() << "Device Name:" << selectedDevice->name;
        delete selectedDevice;
        return;
    }

    qInfo() << "-----------------------------------------------------------------------";
    qInfo() << "Catalog values prior to update:";
    qInfo() << "Catalog ID:" << selectedDevice->ID;
    qInfo() << "Catalog Name:" << selectedDevice->name;
    qInfo() << "Catalog Path:" << selectedDevice->path;
    qInfo() << "Catalog Type:" << selectedDevice->type;
    qInfo() << "Catalog Size:" << selectedDevice->totalFileSize;
    qInfo() << "Catalog Files:" << selectedDevice->totalFileCount;
    qInfo() << "Catalog update date:" << selectedDevice->dateTimeUpdated.toString();

    // Use the SAME approach as UI - create CatalogManager
    qInfo() << "Starting catalog update using CatalogManager (same as UI)...";

    CatalogManager* catalogManager = new CatalogManager(this);
    CatalogJobStoppable* catalogJobStoppable = new CatalogJobStoppable(this);

    // Simple event loop to wait for completion (same pattern as UI)
    QEventLoop loop;

    // Connect to the same signals as UI
    connect(catalogManager, &CatalogManager::catalogOperationCompleted, &loop, &QEventLoop::quit);
    connect(catalogManager, &CatalogManager::catalogOperationError, &loop, &QEventLoop::quit);
    connect(catalogManager, &CatalogManager::catalogOperationCancelled, &loop, &QEventLoop::quit);

    // Timeout protection
    QTimer::singleShot(60 * 60 * 1000, &loop, &QEventLoop::quit); // 1 hour timeout

    // Start catalog operation (SAME as UI)
    catalogManager->startCatalogJobStoppable(
        catalogJobStoppable,
        selectedDevice,
        CatalogJobStoppable::UpdateCatalog,
        collection->databaseMode,
        collection->folder
        );

    qInfo() << "Waiting for catalog update to complete...";
    loop.exec();

    // Check if operation was successful (simple check)
    if (catalogManager->catalogOperationRunning()) {
        // Still running = timeout
        qInfo() << "---";
        qWarning() << "WARNING: Catalog update FAILED: Operation timed out";
        catalogManager->stopCatalogOperation();
    } else {
        // Completed - assume success (same as UI behavior)
        qInfo() << "Catalog operation completed";

        // Reload device to get updated statistics
        selectedDevice->loadDevice(m_connectionName);

        // Save collection data (same as UI)
        collection->saveDeviceTableToFile();
        collection->saveStatiticsTableToFile();

        qInfo() << "---";
        qInfo() << "Catalog updated successfully.";
        qInfo() << "Catalog ID:" << selectedDevice->ID;
        qInfo() << "Catalog Name:" << selectedDevice->name;
        qInfo() << "Catalog Path:" << selectedDevice->path;
        qInfo() << "Catalog Type:" << selectedDevice->type;
        qInfo() << "Catalog Size:" << selectedDevice->totalFileSize;
        qInfo() << "Catalog Files:" << selectedDevice->totalFileCount;
        qInfo() << "Catalog update date:" << selectedDevice->dateTimeUpdated.toString();
    }

    // Clean up
    catalogManager->deleteLater();
    delete selectedDevice;
}

void CommandLineHandler::cmd_updateAllActive(bool displayReport)
{
    qInfo() << "Starting update of all active catalogs...";

    // Select all active catalog devices from database
    QSqlQuery query(QSqlDatabase::database(m_connectionName));
    QString querySQL = QLatin1String(R"(
                           SELECT device_id
                           FROM device
                           WHERE device_type = 'Catalog'
                           AND device_active = 1
                           ORDER BY device_id
                       )");
    query.prepare(querySQL);

    if (!query.exec()) {
        qWarning() << "WARNING: Error querying active catalogs:" << query.lastError().text();
        return;
    }

    // Collect all device IDs first
    QList<int> deviceIDs;
    while (query.next()) {
        deviceIDs.append(query.value(0).toInt());
    }

    if (deviceIDs.isEmpty()) {
        qInfo() << "No active catalogs found to update.";
        return;
    }

    qInfo() << QString("Found %1 active catalog(s) to update.").arg(deviceIDs.size());

    // Global statistics tracking (same as UI batch system)
    int successCount = 0;
    int errorCount = 0;
    QStringList errorCatalogs;

    // Update each catalog sequentially
    for (int i = 0; i < deviceIDs.size(); ++i) {
        int deviceID = deviceIDs[i];

        qInfo() << "-----------------------------------------------------------------------";
        qInfo() << QString("Processing catalog %1 of %2 (ID: %3)")
                        .arg(i + 1)
                        .arg(deviceIDs.size())
                        .arg(deviceID);

        // Load device info for logging
        Device tempDevice;
        tempDevice.ID = deviceID;
        tempDevice.loadDevice(m_connectionName);

        qInfo() << "Catalog name:" << tempDevice.name;

        try {
            // Use the updated single catalog method
            cmd_updateCatalog(deviceID, displayReport);
            successCount++;
            qInfo() << "✓ Catalog" << deviceID << "updated successfully";

        } catch (const std::exception& e) {
            errorCount++;
            errorCatalogs.append(QString("%1 (%2)").arg(tempDevice.name).arg(deviceID));
            qWarning() << "WARNING: ✗ Catalog" << deviceID << "failed:" << e.what();

        } catch (...) {
            errorCount++;
            errorCatalogs.append(QString("%1 (%2)").arg(tempDevice.name).arg(deviceID));
            qWarning() << "WARNING: ✗ Catalog" << deviceID << "failed with unknown error";
        }
    }

    // Final summary report
    qInfo() << "=======================================================================";
    qInfo() << "BATCH UPDATE SUMMARY";
    qInfo() << "=======================================================================";
    qInfo() << "Total catalogs processed:" << deviceIDs.size();
    qInfo() << "Successful updates:" << successCount;
    qWarning() << "WARNING: Failed updates:" << errorCount;

    if (!errorCatalogs.isEmpty()) {
        qInfo() << "";
        qWarning() << "WARNING: Failed catalogs:";
        for (const QString& errorCatalog : errorCatalogs) {
            qInfo() << "  -" << errorCatalog;
        }
    }

    qInfo() << "=======================================================================";
    qInfo() << "All active catalogs processing completed.";
    qInfo() << "=======================================================================";
}

void CommandLineHandler::cmd_listGroup0Catalogs()
{
    if (verbose) {
        QTextStream stdout_stream(stdout);
        stdout_stream << "Executing catalog list query..." << Qt::endl;
    }

    // Check database connection first
    QSqlDatabase db = QSqlDatabase::database(m_connectionName);
    if (!db.isOpen()) {
        qInfo() << "Database is not open!";
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
        qWarning() << "WARNING: Failed to execute catalog list query:" << query.lastError().text();
        qInfo() << "Query was:" << querySQL;
        return;
    }

    qInfo() << "-----------------------------------------------------------------------";
    qInfo() << "Catalogs";
    qInfo() << "-----------------------------------------------------------------------";
    qInfo() << "Device ID" << "    Active" << "      Device Name:";

    int catalogCount = 0;
    // Iterate through the results and print the device ID and name
    while (query.next()) {
        int deviceID = query.value(0).toInt();
        QString deviceName = query.value(1).toString();
        bool deviceActive = query.value(2).toBool();
        qInfo() << "  " << deviceID << "         " << deviceActive << "      " << deviceName;
        catalogCount++;
    }

    if (catalogCount == 0) {
        qInfo() << "  No catalogs found in the collection.";
        qInfo() << "  Check if the collection path is correct and contains catalog data.";

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

    qInfo() << "-----------------------------------------------------------------------";
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
    searchEngine->setDatabaseConnection(m_connectionName);

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

    if (selectedDevice->ID != 0) {
        // Validate that specific device exists
        QSqlQuery checkQuery(QSqlDatabase::database(m_connectionName));
        checkQuery.prepare("SELECT device_name, device_type FROM device WHERE device_id = :id");
        checkQuery.bindValue(":id", selectedDevice->ID);

        if (!checkQuery.exec() || !checkQuery.next()) {
            QTextStream stderr_stream(stderr);
            stderr_stream << "Error: Device ID " << selectedDevice->ID << " not found" << Qt::endl;
            return 1;
        }

        if (verbose) {
            QTextStream stdout_stream(stdout);
            stdout_stream << "Found device - ID: " << selectedDevice->ID
                          << ", Name: " << checkQuery.value(0).toString()
                          << ", Type: " << checkQuery.value(1).toString() << Qt::endl;
        }
    }

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

//--- Process ----------------------------------------------------------------------------
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

bool CommandLineHandler::validateSizeFormat(const QString &sizeStr)
{
    // Regular expression to match formats like: 1, 1B, 1KB, 1.5MB, 1GB, etc.
    QRegularExpression sizeRegex("^\\d+(\\.\\d+)?(B|KB|MB|GB|TB|KiB|MiB|GiB|TiB)?$",
                                 QRegularExpression::CaseInsensitiveOption);
    return sizeRegex.match(sizeStr).hasMatch();
}

void CommandLineHandler::parseSizeValue(const QString &sizeStr, qint64 &value, QString &unit)
{
    // Extract numeric part and unit part
    QRegularExpression regex("^(\\d+(?:\\.\\d+)?)(.*)?$", QRegularExpression::CaseInsensitiveOption);
    QRegularExpressionMatch match = regex.match(sizeStr);

    if (match.hasMatch()) {
        double numericValue = match.captured(1).toDouble();
        QString unitStr = match.captured(2).toUpper();

        // Default to bytes if no unit specified
        if (unitStr.isEmpty()) {
            unitStr = "BYTES";
        }

        // Map common units to internal constants
        if (unitStr == "B" || unitStr == "BYTES") {
            unit = Search::SIZE_UNIT_BYTES;
            value = static_cast<qint64>(numericValue);
        }
        else if (unitStr == "KB" || unitStr == "KIB") {
            unit = Search::SIZE_UNIT_KIB;
            value = static_cast<qint64>(numericValue);
        }
        else if (unitStr == "MB" || unitStr == "MIB") {
            unit = Search::SIZE_UNIT_MIB;
            value = static_cast<qint64>(numericValue);
        }
        else if (unitStr == "GB" || unitStr == "GIB") {
            unit = Search::SIZE_UNIT_GIB;
            value = static_cast<qint64>(numericValue);
        }
        else if (unitStr == "TB" || unitStr == "TIB") {
            unit = Search::SIZE_UNIT_TIB;
            value = static_cast<qint64>(numericValue);
        }
        else {
            // Default fallback
            unit = Search::SIZE_UNIT_BYTES;
            value = static_cast<qint64>(numericValue);
        }
    }
    else {
        // Fallback for invalid format
        unit = Search::SIZE_UNIT_BYTES;
        value = 0;
    }
}

bool CommandLineHandler::validateDateFormat(const QString &dateStr)
{
    // Accept YYYY-MM-DD format
    QDate date = QDate::fromString(dateStr, "yyyy-MM-dd");
    return date.isValid();
}

QDateTime CommandLineHandler::parseDate(const QString &dateStr)
{
    QDate date = QDate::fromString(dateStr, "yyyy-MM-dd");
    if (date.isValid()) {
        // Create QDateTime with the date and start of day (00:00:00)
        return QDateTime(date, QTime(0, 0, 0));
    }
    return QDateTime(); // Invalid datetime
}
