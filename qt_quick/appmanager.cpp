#include "appmanager.h"
#include "database.h"
#include "version.h"

AppManager::AppManager(QObject *parent) : QObject(parent)
{

}
//----------------------------------------------------------------------
void AppManager::initiateApp()
{
    currentVersion  = KATALOG_VERSION_STRING;
    releaseDate     = KATALOG_RELEASE_DATE;

    //Prepare paths, user setting file, check version
    //Get user home path and application dir path
    QStringList standardsPaths = QStandardPaths::standardLocations(QStandardPaths::HomeLocation);
    QString homePath = standardsPaths[0];
    QString applicationDirPath = QCoreApplication::applicationDirPath();

    //Define Setting file path and name
    //For portable mode, check if there is a settings file located with the executable
    collection->settingsFilePath = applicationDirPath + "/katalog_settings.ini";
    QFile settingsFile(collection->settingsFilePath);
    if(!settingsFile.exists()) {
        //otherwise fall back to default katalog_settings path
        collection->settingsFilePath = homePath + "/.config/katalog_settings.ini";
    }
    QSettings settings(collection->settingsFilePath, QSettings::IniFormat);

    // Load database path from settings, with fallback to default
    QString defaultDbPath = "/home/stephane/Developments/Katalog/Data/newKatalogFile.db";
    collection->databaseFilePath = settings.value("Database/FilePath", defaultDbPath).toString();

    selectedDevice->ID = settings.value("Selection/SelectedDeviceID", 1).toInt();
    if (selectedDevice->ID == 0) selectedDevice->type = "All";
    selectedDevice->loadDevice(QSqlDatabase::defaultConnection);

    // Verify the database file exists, fallback to default if not
    if (!QFile::exists(collection->databaseFilePath)) {
        qWarning() << "Database file not found:" << collection->databaseFilePath;
        collection->databaseFilePath = defaultDbPath;

        // If even the default doesn't exist, log a warning
        if (!QFile::exists(collection->databaseFilePath)) {
            qWarning() << "Default database file not found:" << collection->databaseFilePath;
        }
    }

    qDebug() << "Using database file:" << collection->databaseFilePath;

    //Check for new version
    checkVersionChoice = settings.value("Settings/CheckVersion", true).toBool();
    if ( checkVersionChoice == true)
        checkVersion();

    qDebug()<<"initiated App.";
}
//----------------------------------------------------------------------
void AppManager::setSearchObject(SearchSync *search)
{
    searchObject = search;
}
//----------------------------------------------------------------------
void AppManager::executeSearch()
{
    if (!searchObject) {
        qWarning() << "AppManager::executeSearch: Search object not set";
        return;
    }

    if (!selectedDevice) {
        qWarning() << "AppManager::executeSearch: No device selected";
        return;
    }

    // Debug: Check if criteria are set
    QVariantMap props = searchObject->properties();
    qDebug() << "=== SEARCH CRITERIA CHECK ===";
    qDebug() << "searchOnFileName:" << props.value("searchOnFileName").toBool();
    qDebug() << "searchText:" << props.value("searchText").toString();
    qDebug() << "searchInCatalogsChecked:" << props.value("searchInCatalogsChecked").toBool();
    qDebug() << "selectedFileType:" << props.value("selectedFileType").toString();
    qDebug() << "Search object pointer:" << searchObject;

    qDebug() << "Executing search with device ID:" << selectedDevice->ID
             << "Name:" << selectedDevice->name
             << "Type:" << selectedDevice->type;

    // Execute search
    searchObject->searchFiles(selectedDevice);

    // Debug: Check results
    QVariantMap resultProps = searchObject->properties();
    qDebug() << "Files found:" << resultProps.value("filesFoundNumber").toInt();
}
//----------------------------------------------------------------------
// Also add a method to get current database info for display:
QString AppManager::getCurrentDatabaseInfo() const
{
    QFileInfo dbInfo(collection->databaseFilePath);
    if (dbInfo.exists()) {
        return QString("Database: %1 (%2)")
        .arg(dbInfo.fileName())
            .arg(QLocale().formattedDataSize(dbInfo.size()));
    } else {
        return "Database: Not found";
    }
}
//----------------------------------------------------------------------
void AppManager::checkVersion()
{

}
//----------------------------------------------------------------------
//Database management --------------------------------------------------
QString AppManager::startDatabase()
{
    qDebug()<<"Collection::startDatabase()";
    //Check Sqlite driver
    if (!QSqlDatabase::drivers().contains("QSQLITE")){
        return "startDatabase: Unable to load database.<br/>The SQLite driver was not loaded.";
    }

    // Initialize the database:
    QSqlError err = initializeDatabase();
    if (err.type() != QSqlError::NoError) {
        return "startDatabase: The database could not be initialized.<br/>Create a new database or select an existing one.";
    }
    else {
        // Add this: Initialize DeviceListModel after database is ready
        initializeDeviceListModel();
        return "startDatabase: No Error";
    }
}
//----------------------------------------------------------------------
void AppManager::initializeDeviceListModel()
{
    if (deviceListModel) {
        delete deviceListModel;
    }
    deviceListModel = new DeviceListModel(this);
    qDebug() << "DeviceListModel initialized with" << deviceListModel->rowCount() << "devices";
}
//----------------------------------------------------------------------
QSqlError AppManager::initializeDatabase()
{
    QFile databaseFile(collection->databaseFilePath);
    if (!databaseFile.exists()){
        //return error
    }
    else{
        QSqlDatabase db = QSqlDatabase::addDatabase("QSQLITE");
        db.setDatabaseName(collection->databaseFilePath);
        if (!db.open())
            return db.lastError();
    }

    //Create tables if they do not exist
    QSqlQuery q;

    if (!q.exec(SQL_CREATE_DEVICE))
        return q.lastError();

    if (!q.exec(SQL_CREATE_CATALOG))
        return q.lastError();

    if (!q.exec(SQL_CREATE_STORAGE))
        return q.lastError();

    if (!q.exec(SQL_CREATE_DEVICE_CATALOG))
        return q.lastError();

    if (!q.exec(SQL_CREATE_FILE))
        return q.lastError();

    if (!q.exec(SQL_CREATE_FILETEMP))
        return q.lastError();

    if (!q.exec(SQL_CREATE_FOLDER))
        return q.lastError();

    if (!q.exec(SQL_CREATE_METADATA))
        return q.lastError();

    if (!q.exec(SQL_CREATE_STATISTICS_DEVICE))
        return q.lastError();

    if (!q.exec(SQL_CREATE_SEARCH))
        return q.lastError();

    if (!q.exec(SQL_CREATE_TAG))
        return q.lastError();

    if (!q.exec(SQL_CREATE_PARAMETER))
        return q.lastError();

    //Migration
    if (!q.exec(SQL_CREATE_STATISTICS_CATALOG))
        return q.lastError();

    if (!q.exec(SQL_CREATE_STATISTICS_STORAGE))
        return q.lastError();

    if (!q.exec(SQL_CREATE_VIRTUAL_STORAGE))
        return q.lastError();

    if (!q.exec(SQL_CREATE_VIRTUAL_STORAGE_CATALOG))
        return q.lastError();

    return QSqlError();
}
//----------------------------------------------------------------------
QString AppManager::testQuery()
{
    qDebug()<<"AppManager::testQuery ------start-------";

    QSqlQuery query;
    QString querySQL = QLatin1String(R"(
                                    SELECT device_name
                                    FROM device
                                    WHERE device_id = 2
                                )");
    query.prepare(querySQL);
    if(!query.exec())
        qDebug()<<"AppManager::testQuery: "<<query.lastError();

    query.next();
    qDebug()<<"AppManager::testQuery / query result: "<<query.value(0).toString();

    //Test using KZip, list contents
    KZip zip("/home/stephane/Developments/archive.zip");
    if (!zip.open(QIODevice::ReadOnly)) return {};
    qDebug()<<"AppManager::testQuery / test zip: " <<zip.directory()->entries();
    zip.close();

    qDebug()<<"AppManager::testQuery ------end-------";
    return query.value(0).toString();
}
//----------------------------------------------------------------------
void AppManager::selectSQLiteDatabase()
{
    qDebug() << "AppManager::selectSQLiteDatabase() called";

    // Get current directory or use a default
    QString currentDir = QFileInfo(collection->databaseFilePath).absolutePath();
    if (currentDir.isEmpty() || !QDir(currentDir).exists()) {
        QStringList standardsPaths = QStandardPaths::standardLocations(QStandardPaths::HomeLocation);
        currentDir = standardsPaths.isEmpty() ? "/" : standardsPaths[0];
    }

    // This will be handled by QML FileDialog, so we just emit a signal
    // The actual file selection will happen in QML
    emit databasePathChanged(collection->databaseFilePath);
}
//----------------------------------------------------------------------
QString AppManager::getDatabaseFilePath() const
{
    return collection->databaseFilePath;
}
//----------------------------------------------------------------------
void AppManager::setDatabaseFilePath(const QString &path)
{
    if (path.isEmpty()) {
        qWarning() << "AppManager::setDatabaseFilePath: Empty path provided";
        emit databaseConnectionChanged(false, "Empty database path provided");
        return;
    }

    //qDebug() << "AppManager::setDatabaseFilePath:" << path;

    // Check if file exists
    QFile dbFile(path);
    if (!dbFile.exists()) {
        qWarning() << "AppManager::setDatabaseFilePath: Database file does not exist:" << path;
        emit databaseConnectionChanged(false, "Database file does not exist: " + path);
        return;
    }

    // Store the old path for rollback if needed
    QString oldPath = collection->databaseFilePath;

    // Set new path
    collection->databaseFilePath = path;

    // Try to reconnect to the new database
    if (reconnectToDatabase()) {
        qDebug() << "Successfully connected to new database:" << path;
        emit databaseConnectionChanged(true, "Successfully connected to: " + QFileInfo(path).fileName());

        // Save the new path to settings
        QSettings settings(collection->settingsFilePath, QSettings::IniFormat);
        settings.setValue("Database/FilePath", path);
        settings.sync();
    } else {
        // Rollback on failure
        collection->databaseFilePath = oldPath;
        reconnectToDatabase(); // Restore old connection
        emit databaseConnectionChanged(false, "Failed to connect to database: " + path);
    }
}
//----------------------------------------------------------------------
void AppManager::refreshAllUI()
{
    qDebug() << "AppManager::refreshAllUI() - Starting comprehensive UI refresh";

    // Refresh device list model
    refreshDeviceList();

    // Clear and refresh search results if search object exists
    refreshSearchResults();

    // Refresh statistics if needed
    refreshStatistics();

    // Reload selected device to ensure it's still valid
    if (selectedDevice && selectedDevice->ID > 0) {
        selectedDevice->loadDevice(QSqlDatabase::defaultConnection);

        // If device no longer exists, reset to default
        if (selectedDevice->name.isEmpty()) {
            selectedDevice->ID = 1; // Physical Group
            selectedDevice->loadDevice(QSqlDatabase::defaultConnection);
            qDebug() << "Previous selected device not found, reset to Physical Group";
        }
    }

    emit uiRefreshCompleted();
    qDebug() << "AppManager::refreshAllUI() - Completed";
}

void AppManager::refreshDeviceList()
{
    if (deviceListModel) {
        qDebug() << "Refreshing device list model";
        deviceListModel->refreshData();
        emit deviceListRefreshed();
        emit deviceListModelChanged(); // Notify QML of model change
    }
}

void AppManager::refreshSearchResults()
{
    if (searchObject) {
        qDebug() << "Clearing search results";
        searchObject->resetSearchResults();
        emit searchResultsRefreshed();
    }
}

void AppManager::refreshStatistics()
{
    // Add any statistics refresh logic here if needed
    qDebug() << "Statistics refresh (placeholder)";
    emit statisticsRefreshed();
}

bool AppManager::reconnectToDatabase()
{
    // Close existing database connection
    {
        QSqlDatabase db = QSqlDatabase::database();
        if (db.isOpen()) {
            db.close();
            qDebug() << "Closed existing database connection";
        }
    }
    QSqlDatabase::removeDatabase(QSqlDatabase::defaultConnection);

    // Initialize with new database
    QSqlError err = initializeDatabase();
    if (err.type() != QSqlError::NoError) {
        qWarning() << "AppManager::reconnectToDatabase failed:" << err.text();
        return false;
    }

    // Refresh all UI components after successful database connection
    refreshAllUI();

    return true;
}

QString AppManager::getSelectedDeviceName() const
{
    if (selectedDevice && !selectedDevice->name.isEmpty()) {
        return selectedDevice->name;
    }
    return "No device selected";
}

int AppManager::getSelectedDeviceId() const
{
    if (selectedDevice) {
        return selectedDevice->ID;
    }
    return -1;
}

bool AppManager::shouldShowAlphaWarning() const
{
    QSettings settings(collection->settingsFilePath, QSettings::IniFormat);
    return settings.value("Settings/ShowAlphaWarning", true).toBool();
}

void AppManager::setAlphaWarningShown()
{
    QSettings settings(collection->settingsFilePath, QSettings::IniFormat);
    settings.setValue("Settings/ShowAlphaWarning", false);
    settings.sync();
}

void AppManager::selectDeviceById(int deviceId)
{
    if (!selectedDevice) {
        selectedDevice = new Device();
    }

    // Only emit signal if the device actually changes
    if (selectedDevice->ID != deviceId) {
        selectedDevice->ID = deviceId;
        if (selectedDevice->ID == 0) selectedDevice->type = "All";
        selectedDevice->loadDevice(QSqlDatabase::defaultConnection);

        QSettings settings(collection->settingsFilePath, QSettings::IniFormat);
        settings.setValue("Selection/SelectedDeviceID", deviceId);

        emit selectedDeviceChanged(deviceId);

        qDebug() << "Selected device changed to ID:" << deviceId << "Name:" << selectedDevice->name;
    }
}
