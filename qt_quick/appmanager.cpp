#include "appmanager.h"
#include "core/catalog.h"
#include "core/language.h"
#include "core/database.h"
#include <QDir>
#include "core/databasemanager.h"
#include "core/device.h"
#include "core/filechecksum.h"
#include "core/filemetadata.h"
#include "core/searchjobstoppable.h"
#include "core/statusbarmessagebuilder.h"
#include "core/catalog.h"
#include "core/backupjobstoppable.h"
#include "core/backupprofilegenerator.h"
#include "core/directoryreplicator.h"
#include "core/storage.h"
#include "core/foldertreeloader.h"
#include <QThread>
#include <QJsonDocument>
#include <QJsonObject>
#include "version.h"
#include <QGuiApplication>
#include <QCoreApplication>
#include <QCryptographicHash>
#include <QTimer>
#include <QSysInfo>
#include <QLocale>
#include <kcoreaddons_version.h>

namespace {

// How long the start of a scan is held back so the activity panel reaches the
// screen first (OPQ-F11). The scan runs on the GUI thread — CatalogJobStoppable
// pumps the event loop by hand — so once it starts, nothing is painted until it
// yields. A zero-delay timer is not enough: it can fire before the window's
// pending update request is turned into a swapped frame, which leaves the panel
// logically visible but never drawn. 50 ms is about three frames at 60 Hz, and
// is imperceptible at the start of an operation that is about to do real work.
constexpr int kStartDeferralMs = 50;

// Render a stored search phrase or exclude string for a single-line display.
// Terms are stored joined with '\n'; each is quoted separately and the quoted
// terms are joined with ", ", so nothing but punctuation is added and the
// result never contains a newline. See SpecSearchList.md (SRL-F11, SRL-C9).
QString quoteSearchTerms(const QString &stored)
{
    const QStringList terms = stored.split(QLatin1Char('\n'), Qt::SkipEmptyParts);
    QStringList quoted;
    quoted.reserve(terms.size());
    for (const QString &term : terms) {
        const QString trimmed = term.trimmed();
        if (!trimmed.isEmpty())
            quoted << QLatin1Char('"') + trimmed + QLatin1Char('"');
    }
    return quoted.join(QLatin1String(", "));
}

} // namespace

AppManager::AppManager(QObject *parent) : QObject(parent)
{
    m_importSourceDeviceModel = new DeviceListModel(this);
    m_importSourceDeviceModel->setIncludeCollectionRoot(true);
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

    // Read database mode and path so collection->folder can be derived correctly.
    // Database::initialize() reads these same keys again inside startDatabase(),
    // but we need folder set before any core code uses it.
    collection->databaseMode     = settings.value("Settings/databaseMode").toString();
    collection->databaseFilePath = settings.value("Settings/DatabaseFilePath").toString();
    if (collection->databaseMode.isEmpty())
        collection->databaseMode = "File";

    if (collection->databaseMode == "Memory") {
        collection->folder = settings.value("LastCollectionFolder").toString();
    } else if (collection->databaseMode == "File") {
        if (!collection->databaseFilePath.isEmpty())
            collection->folder = QFileInfo(collection->databaseFilePath).absolutePath();
    }

    m_firstRun = !QFile(collection->settingsFilePath).exists()
              || (collection->databaseMode == "Memory" && collection->folder.isEmpty())
              || (collection->databaseMode == "File"   && collection->databaseFilePath.isEmpty());

    selectedDevice->ID = settings.value("Selection/SelectedDeviceID", 1).toInt();
    if (selectedDevice->ID == 0) selectedDevice->type = "All";
    selectedDevice->loadDevice(m_connectionName);

    //Check for new version
    checkVersionChoice = settings.value("Settings/CheckVersion", true).toBool();
    if ( checkVersionChoice == true)
        checkVersion();

    m_fileSortCaseSensitive = settings.value("Settings/FileCaseSensitiveSort", false).toBool();

    //qDebug()<<"initiated App.";
}
//----------------------------------------------------------------------
void AppManager::setSearchObject(SearchSync *search)
{
    searchObject = search;
    search->setDatabaseConnection(m_connectionName);
    connect(search, &Search::searchProgress, this, &AppManager::onSearchProgress);

    m_searchSortModel = new FilesView(this);
    m_searchSortModel->setSourceModel(search);

    m_exploreFilesModel = new ExploreFilesModel(this);
    m_exploreSortModel  = new FilesView(this);
    m_exploreSortModel->setSourceModel(m_exploreFilesModel);

    m_backupPreviewModel = new BackupPreviewModel(this);

    m_searchSortModel->setCaseSensitive(m_fileSortCaseSensitive);
    m_exploreSortModel->setCaseSensitive(m_fileSortCaseSensitive);

    // Restore saved sort state so data is sorted correctly when it first arrives
    QSettings settings(collection->settingsFilePath, QSettings::IniFormat);
    int searchCol   = settings.value("Search/lastSearchSortSection", -1).toInt();
    int searchOrder = settings.value("Search/lastSearchSortOrder",    0).toInt();
    if (searchCol >= 0)
        m_searchSortModel->sort(searchCol, searchOrder);
    int exploreCol   = settings.value("Explore/lastExploreSortSection", -1).toInt();
    int exploreOrder = settings.value("Explore/lastExploreSortOrder",    0).toInt();
    if (exploreCol >= 0)
        m_exploreSortModel->sort(exploreCol, exploreOrder);
}
//----------------------------------------------------------------------
void AppManager::onSearchProgress(int filesProcessed)
{
    if (!searchObject) return;

    StatusBarMessageBuilder builder;
    builder.setOperation(tr("Search"));

    if (filesProcessed == -1) {
        // Search stopped/cancelled
        builder.setStatus(tr("Stopped"));
        if (!searchObject->currentCatalogName.isEmpty())
            builder.setDeviceContext(searchObject->currentCatalogIndex, searchObject->totalCatalogs, searchObject->currentCatalogName);
        if (searchObject->totalFilesProcessed > 0)
            builder.setProcess(tr("Evaluated"), searchObject->totalFilesProcessed);
        if (searchObject->fileNames.size() > 0)
            builder.setResult(searchObject->showFoldersOnly ? tr("Folders found") : tr("Files found"), searchObject->fileNames.size());
    } else if (filesProcessed == -4) {
        // Loading progress tick (Memory mode CSV loading)
        builder.setStatus(tr("In Progress"));
        SearchJobStoppable *sjs = dynamic_cast<SearchJobStoppable*>(searchObject);
        if (sjs) {
            if (searchObject->totalCatalogs > 0)
                builder.setDeviceContext(searchObject->currentCatalogIndex, searchObject->totalCatalogs, searchObject->currentCatalogName);
            builder.setProcess(sjs->currentOperationVerb, sjs->currentCatalogFilesLoaded, sjs->currentCatalogTotalFiles);
        }
    } else if (filesProcessed < 0) {
        // -2 (loading started) and -3 (loading finished)
        builder.setStatus(tr("In Progress"));
        if (searchObject->totalCatalogs > 0)
            builder.setDeviceContext(searchObject->currentCatalogIndex, searchObject->totalCatalogs, searchObject->currentCatalogName);
        if (searchObject->fileNames.size() > 0)
            builder.setResult(searchObject->showFoldersOnly ? tr("Folders found") : tr("Files found"), searchObject->fileNames.size());
    } else {
        // Regular progress (filesProcessed >= 0)
        builder.setStatus(tr("In Progress"));
        if (searchObject->searchInCatalogsChecked && searchObject->totalCatalogs > 0)
            builder.setDeviceContext(searchObject->currentCatalogIndex, searchObject->totalCatalogs, searchObject->currentCatalogName);
        builder.setProcess(tr("Evaluated"), filesProcessed, searchObject->estimatedTotalFiles);
        if (searchObject->fileNames.size() > 0)
            builder.setResult(searchObject->showFoldersOnly ? tr("Folders found") : tr("Files found"), searchObject->fileNames.size());
    }

    m_searchStatusText = builder.build();
    emit searchStatusTextChanged();
    QCoreApplication::processEvents();
}
//----------------------------------------------------------------------
void AppManager::stopSearch()
{
    if (searchObject) searchObject->stopSearch();
}
//----------------------------------------------------------------------
void AppManager::pauseSearch()
{
    if (searchObject) {
        searchObject->pauseSearch();
        m_searchIsPaused = true;
        emit searchStateChanged();
    }
}
//----------------------------------------------------------------------
void AppManager::resumeSearch()
{
    if (searchObject) {
        searchObject->resumeSearch();
        m_searchIsPaused = false;
        emit searchStateChanged();
    }
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

    // Memory mode requires CSV loading during search
    searchObject->setMemoryModeEnabled(collection->databaseMode == "Memory");

    // Capture device path at search time so Results page shows the right device
    searchObject->devicePath = Device::getDevicePath(selectedDevice->ID, m_connectionName);

    // Save criteria to history table before search (mirrors SearchManager::startSearchJobStoppable)
    searchObject->saveSearchHistoryToTable(m_connectionName);

    m_searchIsRunning = true;
    m_searchStatusText = StatusBarMessageBuilder().setOperation(tr("Search")).setStatus(tr("In Progress")).build();
    emit searchStateChanged();
    emit searchStatusTextChanged();

    // Execute search (synchronous — onSearchProgress() fires via direct connection during this call)
    searchObject->searchFiles(selectedDevice);

    m_searchIsRunning = false;
    m_searchIsPaused  = false;
    emit searchStateChanged();

    // Build completion message
    {
        StatusBarMessageBuilder builder;
        builder.setOperation(tr("Search")).setStatus(tr("Completed"));
        if (searchObject->totalCatalogs > 0 && !searchObject->currentCatalogName.isEmpty())
            builder.setDeviceContext(searchObject->currentCatalogIndex, searchObject->totalCatalogs, searchObject->currentCatalogName);
        if (searchObject->totalFilesProcessed > 0)
            builder.setProcess(tr("Evaluated"), searchObject->totalFilesProcessed);
        QString resultTitle = searchObject->showFoldersOnly ? tr("Folders found") : tr("Files found");
        builder.setResult(resultTitle, searchObject->fileNames.size());
        m_searchStatusText = builder.build();
        emit searchStatusTextChanged();
        QTimer::singleShot(5000, this, [this]() {
            m_searchStatusText.clear();
            emit searchStatusTextChanged();
        });
    }

    // Persist history to CSV file (no-op for File/Hosted modes — guarded inside the method)
    collection->saveSearchHistoryTableToFile();

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
    const QString conn = m_connectionName;

    QSqlError err = DatabaseManager::connect(conn, collection);
    if (err.type() != QSqlError::NoError) {
        qWarning() << "startDatabase failed:" << err.text();
        return "startDatabase: " + err.text();
    }

    QSqlError migErr = DatabaseManager::runMigrations(conn, collection);
    if (migErr.type() != QSqlError::NoError)
        qWarning() << "startDatabase: migrations failed (non-fatal):" << migErr.text();

    collection->insertPhysicalStorageGroup();
    initializeDeviceListModel();
    selectedDevice->loadDevice(conn);
    emit databaseModeChanged();
    qInfo() << "Katalog" << currentVersion << "– database ready:" << collection->databaseMode;
    return "startDatabase: No Error";
}
//----------------------------------------------------------------------
void AppManager::initializeDeviceListModel()
{
    if (deviceListModel) {
        delete deviceListModel;
    }
    // Recompute each device's active state from the filesystem (drive mounted?) before
    // loading the list — the stored device_active is stale when opening a collection
    // (it reflects connectivity at the time it was last saved, possibly on another machine).
    collection->updateAllDeviceActive();
    // Seed the refresh policy state so the activation that follows startup finds
    // an unchanged mount table and exits at the cheap gate.
    m_lastActiveProbe = QDateTime::currentDateTime();
    m_lastMountSignature = collection->mountSignature();
    deviceListModel = new DeviceListModel(this);
    deviceListModel->loadFromConnection(m_connectionName);
    m_deviceExpandLevel = -1; // reset to fully expanded on new connection

    if (!m_deviceFilterModel) {
        m_deviceFilterModel = new QSortFilterProxyModel(this);
        m_deviceFilterModel->setFilterRole(DeviceListModel::NameRole);
        m_deviceFilterModel->setFilterCaseSensitivity(Qt::CaseInsensitive);
    }
    m_deviceFilterModel->setSourceModel(deviceListModel);

    //qDebug() << "DeviceListModel initialized with" << deviceListModel->rowCount() << "devices";
}
//----------------------------------------------------------------------
void AppManager::openDeviceFolder(int deviceId)
{
    Device device;
    device.ID = deviceId;
    device.loadDevice(m_connectionName);
    if (!device.path.isEmpty())
        QDesktopServices::openUrl(QUrl::fromLocalFile(device.path));
}
//----------------------------------------------------------------------
void AppManager::expandDevice(int deviceId)
{
    if (deviceListModel)
        deviceListModel->expandDevice(deviceId);
}
//----------------------------------------------------------------------
void AppManager::collapseDevice(int deviceId)
{
    if (deviceListModel)
        deviceListModel->collapseDevice(deviceId);
}
//----------------------------------------------------------------------
void AppManager::setDeviceFilter(const QString &text)
{
    if (m_deviceFilterModel)
        m_deviceFilterModel->setFilterFixedString(text);
}
//----------------------------------------------------------------------
void AppManager::expandDevices()
{
    if (!deviceListModel || !canExpandDevices()) return;

    if (m_deviceExpandLevel == -1) return;

    int maxDepth = Device::getMaxHierarchyDepth(m_connectionName);
    m_deviceExpandLevel++;
    if (m_deviceExpandLevel >= maxDepth)
        m_deviceExpandLevel = -1; // show all once past max

    deviceListModel->clearCollapsedUpToLevel(m_deviceExpandLevel);
    deviceListModel->setMaxLevel(m_deviceExpandLevel);
    emit deviceExpandLevelChanged();
}
//----------------------------------------------------------------------
void AppManager::collapseDevices()
{
    if (!deviceListModel || !canCollapseDevices()) return;

    if (m_deviceExpandLevel == -1) {
        int maxDepth = Device::getMaxHierarchyDepth(m_connectionName);
        m_deviceExpandLevel = qMax(0, maxDepth - 1);
    } else {
        m_deviceExpandLevel--;
    }

    deviceListModel->setMaxLevel(m_deviceExpandLevel);
    emit deviceExpandLevelChanged();
}
//----------------------------------------------------------------------
bool AppManager::getShowDeviceInfo() const
{
    QSettings settings(collection->settingsFilePath, QSettings::IniFormat);
    return settings.value("Selection/ShowDeviceInfo", true).toBool();
}
//----------------------------------------------------------------------
void AppManager::setShowDeviceInfo(bool value)
{
    QSettings settings(collection->settingsFilePath, QSettings::IniFormat);
    settings.setValue("Selection/ShowDeviceInfo", value);
    settings.sync();
    emit showDeviceInfoChanged();
}
//----------------------------------------------------------------------
bool AppManager::getRefreshDeviceStatusOnActivation() const
{
    QSettings settings(collection->settingsFilePath, QSettings::IniFormat);
    // Opt-in: nobody acquires a background filesystem probe they did not ask for.
    return settings.value("Settings/RefreshDeviceStatusOnActivation", false).toBool();
}
//----------------------------------------------------------------------
void AppManager::setRefreshDeviceStatusOnActivation(bool value)
{
    QSettings settings(collection->settingsFilePath, QSettings::IniFormat);
    settings.setValue("Settings/RefreshDeviceStatusOnActivation", value);
    settings.sync();
    emit refreshDeviceStatusOnActivationChanged();
}
//----------------------------------------------------------------------
void AppManager::refreshDeviceActiveOnActivation()
{
    if (!getRefreshDeviceStatusOnActivation())
        return;

    // Cheap gate first: reading the mount table costs nothing and, when nothing
    // has been mounted or unmounted, spares every device path a QDir::exists()
    // that would block on an unreachable network mount.
    const QString signature = collection->mountSignature();
    if (signature == m_lastMountSignature)
        return;
    m_lastMountSignature = signature;

    // Debounce: also absorbs the activation that immediately follows startup,
    // where the collection was just opened and already probed.
    if (m_lastActiveProbe.isValid()
            && m_lastActiveProbe.secsTo(QDateTime::currentDateTime()) < 30)
        return;

    refreshDeviceList();
}
//----------------------------------------------------------------------
bool AppManager::getShowSelectionPage() const
{
    // Mirrors K2's Selection/Filters panel show/hide (mainwindow_tab_filters.cpp).
    // K2 stores "go-next" when hidden and "go-previous" when shown; default = shown.
    QSettings settings(collection->settingsFilePath, QSettings::IniFormat);
    return settings.value("Settings/ShowHideFilters", "go-previous").toString() != "go-next";
}
//----------------------------------------------------------------------
void AppManager::setShowSelectionPage(bool value)
{
    QSettings settings(collection->settingsFilePath, QSettings::IniFormat);
    settings.setValue("Settings/ShowHideFilters", value ? "go-previous" : "go-next");
    settings.sync();
    emit showSelectionPageChanged();
}
//----------------------------------------------------------------------
bool AppManager::getDeviceFilterFromSelection() const
{
    QSettings settings(collection->settingsFilePath, QSettings::IniFormat);
    return settings.value("Devices/FilterFromSelection", false).toBool();
}
//----------------------------------------------------------------------
void AppManager::setDeviceFilterFromSelection(bool value)
{
    QSettings settings(collection->settingsFilePath, QSettings::IniFormat);
    settings.setValue("Devices/FilterFromSelection", value);
    settings.sync();
    emit deviceFilterFromSelectionChanged();
}
//----------------------------------------------------------------------
bool AppManager::getSearchKeepsSelection() const
{
    QSettings settings(collection->settingsFilePath, QSettings::IniFormat);
    return settings.value("Search/KeepsSelection", true).toBool();
}
//----------------------------------------------------------------------
void AppManager::setSearchKeepsSelection(bool value)
{
    QSettings settings(collection->settingsFilePath, QSettings::IniFormat);
    settings.setValue("Search/KeepsSelection", value);
    settings.sync();
    emit searchKeepsSelectionChanged();
}
//----------------------------------------------------------------------
bool AppManager::getCheckVersionChoice() const
{
    return checkVersionChoice;
}
//----------------------------------------------------------------------
void AppManager::setCheckVersionChoice(bool value)
{
    if (checkVersionChoice == value)
        return;
    checkVersionChoice = value;
    QSettings settings(collection->settingsFilePath, QSettings::IniFormat);
    settings.setValue("Settings/CheckVersion", value);
    emit checkVersionChoiceChanged();
}
//----------------------------------------------------------------------
bool AppManager::getFileSortCaseSensitive() const
{
    return m_fileSortCaseSensitive;
}
//----------------------------------------------------------------------
void AppManager::setFileSortCaseSensitive(bool value)
{
    if (m_fileSortCaseSensitive == value)
        return;
    m_fileSortCaseSensitive = value;
    if (m_searchSortModel)  m_searchSortModel->setCaseSensitive(value);
    if (m_exploreSortModel) m_exploreSortModel->setCaseSensitive(value);
    QSettings settings(collection->settingsFilePath, QSettings::IniFormat);
    settings.setValue("Settings/FileCaseSensitiveSort", value);
    emit fileSortCaseSensitiveChanged();
}
//----------------------------------------------------------------------
void AppManager::openSettingsFile()
{
    QDesktopServices::openUrl(QUrl::fromLocalFile(collection->settingsFilePath));
}
//----------------------------------------------------------------------
QString AppManager::getSettingsFilePath() const
{
    return collection->settingsFilePath;
}
//----------------------------------------------------------------------
QString AppManager::getDatabaseSchemaVersion()
{
    return collection->loadDatabaseSchemaVersion();
}
//----------------------------------------------------------------------
void AppManager::setLastPage(const QString &pageName)
{
    QSettings settings(collection->settingsFilePath, QSettings::IniFormat);
    settings.setValue("Settings/lastPage", pageName);
}
//----------------------------------------------------------------------
QString AppManager::getLastPage() const
{
    QSettings settings(collection->settingsFilePath, QSettings::IniFormat);
    return settings.value("Settings/lastPage", "Search").toString();
}
//----------------------------------------------------------------------
bool AppManager::canExpandDevices() const
{
    return m_deviceExpandLevel != -1; // -1 means already showing all
}
//----------------------------------------------------------------------
bool AppManager::canCollapseDevices() const
{
    return m_deviceExpandLevel != 0; // 0 means only top-level visible
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
void AppManager::openDatabaseFile()
{
    QDesktopServices::openUrl(QUrl::fromLocalFile(collection->databaseFilePath));
}
//----------------------------------------------------------------------
void AppManager::setDatabaseFilePath(const QString &path)
{
    if (path.isEmpty()) {
        qWarning() << "AppManager::setDatabaseFilePath: Empty path provided";
        emit databaseConnectionChanged(false, tr("Empty database path provided"));
        return;
    }

    //qDebug() << "AppManager::setDatabaseFilePath:" << path;

    // Check if file exists
    QFile dbFile(path);
    if (!dbFile.exists()) {
        qWarning() << "AppManager::setDatabaseFilePath: Database file does not exist:" << path;
        emit databaseConnectionChanged(false, tr("Database file does not exist: %1").arg(path));
        return;
    }

    // Store old settings for rollback if needed
    QString oldPath = collection->databaseFilePath;
    QSettings settings(collection->settingsFilePath, QSettings::IniFormat);
    QString oldMode = settings.value("Settings/databaseMode").toString();
    QString oldFilePath = settings.value("Settings/DatabaseFilePath").toString();

    // Save new settings before reconnecting so Database::initialize() reads them
    settings.setValue("Settings/databaseMode",    "File");
    settings.setValue("Settings/DatabaseFilePath", path);
    settings.sync();

    if (reconnectToDatabase()) {
        qDebug() << "Successfully connected to new database:" << path;
        saveToRecentCollections("File", path, QFileInfo(path).fileName());
        emit databaseConnectionChanged(true, tr("Opened: %1").arg(QFileInfo(path).fileName()));
    } else {
        // Rollback settings and connection
        settings.setValue("Settings/databaseMode",    oldMode);
        settings.setValue("Settings/DatabaseFilePath", oldFilePath);
        settings.sync();
        collection->databaseFilePath = oldPath;
        reconnectToDatabase();
        emit databaseConnectionChanged(false, tr("Failed to open database: %1").arg(lastDatabaseError));
    }
}
//----------------------------------------------------------------------
void AppManager::createNewSQLiteCollection(const QString &path)
{
    if (path.isEmpty()) return;

    // Create the containing folder first: the file dialog returns a path without
    // creating anything, so a folder the user typed rather than picked does not
    // exist yet and the file below could not be written.
    QDir().mkpath(QFileInfo(path).absolutePath());

    // Create an empty file — SQLite will write the schema on first connection.
    QFile dbFile(path);
    if (!dbFile.open(QFile::WriteOnly)) {
        emit databaseConnectionChanged(false, tr("Could not create file: %1").arg(path));
        return;
    }
    dbFile.close();

    // Connect, run migrations (creates schema), add to recents, refresh UI.
    setDatabaseFilePath(path);

    // Seed the three default devices K2 always creates for a new collection.
    // insertPhysicalStorageGroup() is idempotent — it checks for device_id=1 first.
    bool defaultsCreated = collection->insertPhysicalStorageGroup();
    if (defaultsCreated) {
        // Translate the hard-coded English names K2 inserted into the user's locale.
        const QString conn = m_connectionName;
        auto renameDevice = [&](int id, const QString &newName) {
            Device dev;
            dev.setConnectionName(conn);
            dev.ID = id;
            dev.loadDevice(conn);
            dev.name = newName;
            dev.saveDevice();
        };
        renameDevice(1, tr(" Physical Group"));
        renameDevice(2, tr("Virtual device"));

        QSqlQuery q(QSqlDatabase::database(conn));
        q.prepare("SELECT device_id FROM device WHERE device_type='Storage' AND device_name='Local disk' LIMIT 1");
        if (q.exec() && q.next())
            renameDevice(q.value(0).toInt(), tr("Local disk"));

        refreshAllUI();
    }
}
//----------------------------------------------------------------------
QString AppManager::getCollectionBrowsePath() const
{
    // File mode   → folder containing the .db file
    if (collection->databaseMode == "File" && !collection->databaseFilePath.isEmpty())
        return QFileInfo(collection->databaseFilePath).absolutePath();
    // Memory mode → the collection folder
    if (collection->databaseMode == "Memory" && !collection->folder.isEmpty())
        return collection->folder;
    // Hosted mode (or anything unresolved) → user home folder
    return QStandardPaths::writableLocation(QStandardPaths::HomeLocation);
}
//----------------------------------------------------------------------
QString AppManager::getNewCollectionDefaultPath() const
{
    return getCollectionBrowsePath() + "/katalog.db";
}
//----------------------------------------------------------------------
void AppManager::refreshAllUI()
{
    qDebug() << "AppManager::refreshAllUI() - Starting comprehensive UI refresh";

    // Re-validate the selected device against the (possibly new) connection FIRST,
    // before refreshDeviceList() emits deviceListRefreshed. Pickers that react to
    // that signal (e.g. the Create page Storage picker, whose default is derived
    // from selectedDevice via getDefaultStorageId()) must read a device that
    // actually exists in the current collection — otherwise they resolve against
    // the previous collection's selection and never recompute.
    if (selectedDevice && selectedDevice->ID > 0) {
        selectedDevice->loadDevice(m_connectionName);

        // If device no longer exists, reset to default
        if (selectedDevice->name.isEmpty()) {
            selectedDevice->ID = 1; // Physical Group
            selectedDevice->loadDevice(m_connectionName);
            qDebug() << "Previous selected device not found, reset to Physical Group";
        }
    }

    // Refresh device list model (emits deviceListRefreshed → pickers re-resolve)
    refreshDeviceList();

    // Clear and refresh search results if search object exists
    refreshSearchResults();

    // Refresh statistics if needed
    refreshStatistics();

    // selectedDevice may have been reloaded/reset above without any other notice;
    // notify so device-dependent UI (Storage picker, selection highlight, the
    // Statistics device reminder, …) reflects the current collection's device.
    emit selectedDeviceChanged(selectedDevice ? selectedDevice->ID : 0);
    emit tagsChanged();
    emit uiRefreshCompleted();
    qDebug() << "AppManager::refreshAllUI() - Completed";
}

void AppManager::refreshDeviceList()
{
    if (deviceListModel) {
        qDebug() << "Refreshing device list model";
        // Recompute device active states from the filesystem before reloading the list,
        // so the Selection page reflects what is actually mounted now (not stale values
        // carried over when switching/opening a collection). Mirrors K2's device refresh.
        collection->updateAllDeviceActive();
        m_lastActiveProbe = QDateTime::currentDateTime();
        deviceListModel->refreshData();
        emit deviceListRefreshed();
        emit deviceListModelChanged();
        emit deviceListChanged();
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
    const QString conn = m_connectionName;

    QSqlError err = DatabaseManager::reconnect(conn, collection);
    if (err.type() != QSqlError::NoError) {
        lastDatabaseError = err.text();
        qWarning() << "AppManager::reconnectToDatabase failed:" << lastDatabaseError;
        return false;
    }

    QSqlError migErr = DatabaseManager::runMigrations(conn, collection);
    if (migErr.type() != QSqlError::NoError) {
        qWarning() << "AppManager::reconnectToDatabase: migrations failed (non-fatal):" << migErr.text();
    }

    if (collection->databaseMode == "File" && !collection->databaseFilePath.isEmpty())
        collection->folder = QFileInfo(collection->databaseFilePath).absolutePath();

    lastDatabaseError.clear();
    collection->setConnectionName(conn);
    collection->loadImageFolderPath();
    collection->insertPhysicalStorageGroup();

    // Opening a different collection: reset the selection to the default root
    // (Physical Group, guaranteed to exist by insertPhysicalStorageGroup above).
    // Device IDs are not stable across collections, so keeping the previous
    // selection would silently point at a different/nonexistent device — and the
    // Create page's Storage picker would derive a wrong or invalid parent from it.
    // refreshAllUI() below reloads this device for the new connection and notifies.
    if (selectedDevice)
        selectedDevice->ID = 1;

    refreshAllUI();
    emit databaseModeChanged();
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
//----------------------------------------------------------------------
QString AppManager::getSelectedDeviceType() const
{
    if (selectedDevice)
        return selectedDevice->type;
    return QString();
}
//----------------------------------------------------------------------
QString AppManager::getSelectedDevicePath() const
{
    if (selectedDevice)
        return selectedDevice->path;
    return QString();
}

QString AppManager::getHostName() const
{
    QSettings settings(collection->settingsFilePath, QSettings::IniFormat);
    return settings.value("Settings/databaseHostName").toString();
}
//----------------------------------------------------------------------
QString AppManager::getDatabaseName() const
{
    QSettings settings(collection->settingsFilePath, QSettings::IniFormat);
    return settings.value("Settings/databaseName").toString();
}
//----------------------------------------------------------------------
QString AppManager::getPhpMyAdminUrl() const
{
    // Base URL the Hosted "Edit" button opens; the database name is appended by
    // the caller. Editable + persisted so each machine can adjust host/port/path
    // (e.g. macOS MAMP uses :8888) without code changes.
    QSettings settings(collection->settingsFilePath, QSettings::IniFormat);
    return settings.value("Settings/PhpMyAdminUrl",
               "http://localhost/phpMyAdmin/index.php?route=/database/structure&db=").toString();
}
//----------------------------------------------------------------------
void AppManager::setPhpMyAdminUrl(const QString &url)
{
    QSettings settings(collection->settingsFilePath, QSettings::IniFormat);
    settings.setValue("Settings/PhpMyAdminUrl", url);
}
//----------------------------------------------------------------------
int AppManager::getDatabasePort() const
{
    QSettings settings(collection->settingsFilePath, QSettings::IniFormat);
    return settings.value("Settings/databasePort", 3306).toInt();
}
//----------------------------------------------------------------------
QString AppManager::getDatabaseUserName() const
{
    QSettings settings(collection->settingsFilePath, QSettings::IniFormat);
    return settings.value("Settings/databaseUserName").toString();
}
//----------------------------------------------------------------------
QString AppManager::getDatabasePassword() const
{
    QSettings settings(collection->settingsFilePath, QSettings::IniFormat);
    return settings.value("Settings/databasePassword").toString();
}
//----------------------------------------------------------------------
bool AppManager::getHostedAutoConnect() const
{
    QSettings settings(collection->settingsFilePath, QSettings::IniFormat);
    return settings.value("Settings/HostedAutoConnect", false).toBool();
}
//----------------------------------------------------------------------
void AppManager::setHostedAutoConnect(bool value)
{
    QSettings settings(collection->settingsFilePath, QSettings::IniFormat);
    settings.setValue("Settings/HostedAutoConnect", value);
    settings.sync();
}
//----------------------------------------------------------------------
QVariantList AppManager::getLanguageList() const
{
    QVariantList result;
    const QStringList codes = Language::getSupportedLanguages();
    for (const QString &code : codes) {
        QVariantMap entry;
        entry["code"]        = code;
        entry["displayName"] = Language::getDisplayName(code);
        entry["flagPath"]    = Language::getFlagPath(code);
        result.append(entry);
    }
    return result;
}
//----------------------------------------------------------------------
QString AppManager::getCurrentLanguage() const
{
    QSettings settings(collection->settingsFilePath, QSettings::IniFormat);
    return settings.value("Settings/Language", "en_US").toString();
}
//----------------------------------------------------------------------
void AppManager::setLanguage(const QString &languageCode)
{
    QSettings settings(collection->settingsFilePath, QSettings::IniFormat);
    settings.setValue("Settings/Language", languageCode);
    settings.sync();
    emit languageChanged(languageCode);
}
//----------------------------------------------------------------------
void AppManager::openFile(const QString &filePath)
{
    QDesktopServices::openUrl(QUrl::fromLocalFile(filePath));
}
//----------------------------------------------------------------------
void AppManager::openFolder(const QString &folderPath)
{
    QDesktopServices::openUrl(QUrl::fromLocalFile(folderPath));
}
//----------------------------------------------------------------------
void AppManager::copyToClipboard(const QString &text)
{
    QGuiApplication::clipboard()->setText(text);
}
//----------------------------------------------------------------------
QString AppManager::systemInformation()
{
    // Assembled for pasting into a public bug report (SpecAbout.md).
    //
    // Every field is named and read one at a time. Nothing here iterates the
    // settings or dumps a config object, so a value added elsewhere in the
    // application can never find its way into this block by accident (ABT-C2).
    //
    // Deliberately absent (ABT-C1): machine hostname, user name, every
    // filesystem path, the collection name, and the hosted database's host,
    // port, name, user and password — the password is stored in plain text in
    // the .ini, and this text is written to the clipboard to be pasted in
    // public.
    //
    // Not translated, by intent: the block is read by a maintainer triaging a
    // report, so English field names keep every report legible whatever the
    // reporter's UI language. Only the button and the confirmation notification
    // that produce it are translated.
    QStringList lines;

    lines << QStringLiteral("Katalog %1 (%2)")
                 .arg(QLatin1String(KATALOG_VERSION_STRING),
                      QLatin1String(KATALOG_RELEASE_DATE));
    lines << QStringLiteral("Qt %1 (built against %2)")
                 .arg(QLatin1String(qVersion()), QLatin1String(QT_VERSION_STR));
    lines << QStringLiteral("KDE Frameworks %1")
                 .arg(QLatin1String(KCOREADDONS_VERSION_STRING));
    lines << QStringLiteral("OS: %1").arg(QSysInfo::prettyProductName());
    lines << QStringLiteral("Kernel: %1 %2")
                 .arg(QSysInfo::kernelType(), QSysInfo::kernelVersion());
    lines << QStringLiteral("Architecture: %1").arg(QSysInfo::currentCpuArchitecture());
    lines << QStringLiteral("Language: %1").arg(QLocale().name());
    lines << QStringLiteral("Database mode: %1 (schema %2)")
                 .arg(getDatabaseMode(), getDatabaseSchemaVersion());

    return lines.join(QLatin1Char('\n'));
}
//----------------------------------------------------------------------
QString AppManager::exportSearchResultsToCSV()
{
    if (!searchObject || searchObject->fileNames.isEmpty())
        return QString();

    // Determine export path
    QString dir = QStandardPaths::writableLocation(QStandardPaths::DownloadLocation);
    if (dir.isEmpty())
        dir = QStandardPaths::writableLocation(QStandardPaths::HomeLocation);
    QString timestamp = QDateTime::currentDateTime().toString("yyyyMMdd-HHmmss");
    QString filePath  = dir + "/katalog_search_" + timestamp + ".csv";

    QSaveFile file(filePath);
    if (!file.open(QIODevice::WriteOnly | QIODevice::Text))
        return QString();

    QTextStream out(&file);
    out.setEncoding(QStringConverter::Utf8);

    // Header row
    out << "Name\tSize\tDate\tFolder\tCatalog\n";

    int count = searchObject->fileNames.size();
    for (int i = 0; i < count; ++i) {
        out << searchObject->fileNames.value(i)     << "\t"
            << searchObject->fileSizes.value(i)     << "\t"
            << searchObject->fileDateTimes.value(i) << "\t"
            << searchObject->filePaths.value(i)     << "\t"
            << searchObject->fileCatalogs.value(i)  << "\n";
    }

    if (!file.commit())
        return QString();

    return filePath;
}
//----------------------------------------------------------------------
QString AppManager::exportSearchResultsAsCatalog()
{
    if (!searchObject || searchObject->fileNames.isEmpty())
        return QString();

    const QString conn = m_connectionName;
    const QString timestamp = QDateTime::currentDateTime().toString("yyyyMMdd-hhmmss");
    const QString catalogName = tr("search_results") + "_" + timestamp;

    // Find or create the "Search Results" virtual parent device
    Device holder;
    holder.name = tr("Search Results");
    holder.getIDFromDeviceName();
    if (holder.ID <= 0) {
        holder.generateDeviceID();
        holder.type     = "Virtual";
        holder.groupID  = 1;
        holder.parentID = 0;
        holder.insertDevice();
    }

    // Create the new Device + Catalog
    Device *newDevice = new Device();
    newDevice->generateDeviceID();
    newDevice->type     = "Catalog";
    newDevice->name     = catalogName;
    newDevice->groupID  = 1;
    newDevice->parentID = holder.ID;
    newDevice->path     = "EXPORT";
    newDevice->totalFileCount = searchObject->filesFoundNumber;
    newDevice->totalFileSize  = searchObject->filesFoundTotalSize;

    newDevice->catalog->generateID();
    newDevice->externalID        = newDevice->catalog->ID;
    newDevice->catalog->name     = catalogName;
    newDevice->catalog->setDateUpdated(QDateTime());
    newDevice->catalog->setDateLoaded(QDateTime().addMSecs(100));
    newDevice->catalog->fileType = searchObject->searchOnType
                                   ? searchObject->selectedFileType : "All";
    newDevice->insertDevice();
    newDevice->catalog->sourcePath   = "EXPORT";
    newDevice->catalog->appVersion   = currentVersion;
    newDevice->catalog->insertCatalog();

    // Prepare insert queries
    const Database::DatabaseType dbType = Database::getDatabaseType(conn);
    const QString insertOrIgnore = Database::getInsertOrIgnorePrefix(dbType);

    const QString insertFolderSQL = QString(R"(
        %1 INTO folder(folder_catalog_id, folder_path)
        VALUES(:folder_catalog_id, :folder_path)
    )").arg(insertOrIgnore)
     + (dbType == Database::DatabaseType::PostgreSQL
        ? " ON CONFLICT (folder_catalog_id, folder_path) DO NOTHING" : "");

    const QString insertFileSQL = QLatin1String(R"(
        INSERT INTO file(
            file_catalog_id, file_name, file_folder_path, file_size,
            file_date_updated, file_catalog, file_full_path,
            file_extension, file_type, mime_type, mime_verified, type_mismatch,
            image_width, image_height, image_orientation,
            video_duration_seconds, video_width, video_height,
            video_codec, video_framerate, video_bitrate,
            audio_duration_seconds, audio_artist, audio_album, audio_title,
            audio_genre, audio_year, audio_track_number,
            audio_bitrate, audio_sample_rate,
            metadata_extended, metadata_extraction_date,
            checksum_sha256, checksum_extraction_date
        ) VALUES(
            :file_catalog_id, :file_name, :file_folder_path, :file_size,
            :file_date_updated, :file_catalog, :file_full_path,
            :file_extension, :file_type, :mime_type, :mime_verified, :type_mismatch,
            :image_width, :image_height, :image_orientation,
            :video_duration_seconds, :video_width, :video_height,
            :video_codec, :video_framerate, :video_bitrate,
            :audio_duration_seconds, :audio_artist, :audio_album, :audio_title,
            :audio_genre, :audio_year, :audio_track_number,
            :audio_bitrate, :audio_sample_rate,
            :metadata_extended, :metadata_extraction_date,
            :checksum_sha256, :checksum_extraction_date
        )
    )");

    QSqlQuery folderQ(QSqlDatabase::database(conn));
    QSqlQuery fileQ(QSqlDatabase::database(conn));
    folderQ.prepare(insertFolderSQL);
    fileQ.prepare(insertFileSQL);

    const int catId = newDevice->catalog->ID;
    const int n = searchObject->fileNames.size();

    for (int i = 0; i < n; ++i) {
        // Insert folder
        folderQ.bindValue(":folder_catalog_id", catId);
        folderQ.bindValue(":folder_path", searchObject->filePaths.value(i));
        folderQ.exec();

        // Insert file — all columns, null if array too short
        auto sv = [&](const QStringList &v) -> QVariant { return i < v.size() ? QVariant(v[i]) : QVariant(); };
        auto iv = [&](const QList<int>    &v) -> QVariant { return i < v.size() ? QVariant(v[i]) : QVariant(); };
        auto bv = [&](const QList<bool>   &v) -> QVariant { return i < v.size() ? QVariant(v[i]) : QVariant(); };
        auto dv = [&](const QList<double> &v) -> QVariant { return i < v.size() ? QVariant(v[i]) : QVariant(); };

        fileQ.bindValue(":file_catalog_id",            catId);
        fileQ.bindValue(":file_name",                  searchObject->fileNames.value(i));
        fileQ.bindValue(":file_folder_path",           searchObject->filePaths.value(i));
        fileQ.bindValue(":file_size",                  searchObject->fileSizes.value(i));
        fileQ.bindValue(":file_date_updated",          searchObject->fileDateTimes.value(i));
        fileQ.bindValue(":file_catalog",               searchObject->fileCatalogs.value(i));
        fileQ.bindValue(":file_full_path",             searchObject->filePaths.value(i));
        fileQ.bindValue(":file_extension",             sv(searchObject->fileExtensions));
        fileQ.bindValue(":file_type",                  sv(searchObject->fileTypes));
        fileQ.bindValue(":mime_type",                  sv(searchObject->mimeTypes));
        fileQ.bindValue(":mime_verified",              bv(searchObject->mimeVerified));
        fileQ.bindValue(":type_mismatch",              bv(searchObject->typeMismatch));
        fileQ.bindValue(":image_width",                iv(searchObject->imageWidths));
        fileQ.bindValue(":image_height",               iv(searchObject->imageHeights));
        fileQ.bindValue(":image_orientation",          iv(searchObject->imageOrientations));
        fileQ.bindValue(":video_duration_seconds",     iv(searchObject->videoDurations));
        fileQ.bindValue(":video_width",                iv(searchObject->videoWidths));
        fileQ.bindValue(":video_height",               iv(searchObject->videoHeights));
        fileQ.bindValue(":video_codec",                sv(searchObject->videoCodecs));
        fileQ.bindValue(":video_framerate",            dv(searchObject->videoFramerates));
        fileQ.bindValue(":video_bitrate",              iv(searchObject->videoBitrates));
        fileQ.bindValue(":audio_duration_seconds",     iv(searchObject->audioDurations));
        fileQ.bindValue(":audio_artist",               sv(searchObject->audioArtists));
        fileQ.bindValue(":audio_album",                sv(searchObject->audioAlbums));
        fileQ.bindValue(":audio_title",                sv(searchObject->audioTitles));
        fileQ.bindValue(":audio_genre",                sv(searchObject->audioGenres));
        fileQ.bindValue(":audio_year",                 iv(searchObject->audioYears));
        fileQ.bindValue(":audio_track_number",         iv(searchObject->audioTrackNumbers));
        fileQ.bindValue(":audio_bitrate",              iv(searchObject->audioBitrates));
        fileQ.bindValue(":audio_sample_rate",          iv(searchObject->audioSampleRates));
        fileQ.bindValue(":metadata_extended",          sv(searchObject->metadataExtendeds));
        fileQ.bindValue(":metadata_extraction_date",   sv(searchObject->metadataExtractionDates));
        fileQ.bindValue(":checksum_sha256",            sv(searchObject->checksumSha256s));
        fileQ.bindValue(":checksum_extraction_date",   sv(searchObject->checksumExtractionDates));
        fileQ.exec();
    }

    // Refresh device list so the new catalog appears in Selection
    refreshDeviceList();

    delete newDevice;
    return catalogName;
}
//----------------------------------------------------------------------
int AppManager::batchMoveSearchResultsToTrash()
{
    if (!searchObject || searchObject->fileNames.isEmpty())
        return 0;
    int count = 0;
    int total = searchObject->fileNames.size();
    for (int i = 0; i < total; ++i) {
        QString path = searchObject->filePaths.value(i) + "/" + searchObject->fileNames.value(i);
        if (QFile::moveToTrash(path))
            ++count;
    }
    return count;
}
//----------------------------------------------------------------------
int AppManager::batchDeleteSearchResults()
{
    if (!searchObject || searchObject->fileNames.isEmpty())
        return 0;
    int count = 0;
    int total = searchObject->fileNames.size();
    for (int i = 0; i < total; ++i) {
        QString path = searchObject->filePaths.value(i) + "/" + searchObject->fileNames.value(i);
        if (QFile::remove(path))
            ++count;
    }
    return count;
}
//----------------------------------------------------------------------
QVariantMap AppManager::batchVerifyChecksums()
{
    int total = 0, matched = 0, mismatched = 0, calculated = 0, errors = 0;

    if (searchObject) {
        const int n = searchObject->fileNames.size();
        for (int i = 0; i < n; ++i) {
            ++total;
            const QString &fileName  = searchObject->fileNames[i];
            const QString &folder    = searchObject->filePaths[i];
            const QString  filePath  = folder + "/" + fileName;
            const int      catalogId = searchObject->fileCatalogIDs.value(i, -1);
            const QString  stored    = searchObject->checksumSha256s.value(i);

            if (!QFileInfo::exists(filePath)) { ++errors; continue; }

            QString actual = FileChecksum::calculateChecksum(filePath, QCryptographicHash::Sha256);
            if (actual.isEmpty()) { ++errors; continue; }

            if (stored.isEmpty()) {
                if (catalogId > 0)
                    FileChecksum::updateFileChecksum(m_connectionName,
                                                    catalogId, fileName, folder, actual, "SHA256");
                ++calculated;
            } else {
                actual == stored ? ++matched : ++mismatched;
            }
        }
    }

    QVariantMap result;
    result["total"]      = total;
    result["matched"]    = matched;
    result["mismatched"] = mismatched;
    result["calculated"] = calculated;
    result["errors"]     = errors;
    return result;
}
//----------------------------------------------------------------------
QVariantMap AppManager::batchGetMetadata()
{
    int total = 0, updated = 0, skipped = 0, errors = 0;

    if (searchObject) {
        const int n = searchObject->fileNames.size();
        for (int i = 0; i < n; ++i) {
            ++total;
            const QString &fileName  = searchObject->fileNames[i];
            const QString &folder    = searchObject->filePaths[i];
            const QString  filePath  = folder + "/" + fileName;
            const int      catalogId = searchObject->fileCatalogIDs.value(i, -1);

            if (!QFileInfo::exists(filePath)) { ++errors; continue; }
            if (catalogId <= 0)               { ++skipped; continue; }

            QVariantMap metadata = FileMetadata::extractMetadata(filePath, Catalog::METADATA_MEDIA_EXTENDED);
            if (metadata.isEmpty())           { ++skipped; continue; }

            if (FileMetadata::updateFileMetadata(m_connectionName,
                                                catalogId, fileName, folder, metadata))
                ++updated;
            else
                ++errors;
        }
    }

    QVariantMap result;
    result["total"]   = total;
    result["updated"] = updated;
    result["skipped"] = skipped;
    result["errors"]  = errors;
    return result;
}
//----------------------------------------------------------------------
bool AppManager::moveFileToTrash(const QString &fullPath)
{
    return QFile::moveToTrash(fullPath);
}
//----------------------------------------------------------------------
bool AppManager::deleteSingleFile(const QString &fullPath)
{
    return QFile::remove(fullPath);
}
//----------------------------------------------------------------------
bool AppManager::catalogIncludesExtendedMetadata(int catalogId)
{
    QSqlQuery query(QSqlDatabase::database(m_connectionName));
    query.prepare(QLatin1String("SELECT catalog_include_metadata FROM catalog WHERE catalog_id = :id"));
    query.bindValue(":id", catalogId);
    if (query.exec() && query.next())
        return query.value(0).toString().contains("Extended");
    return false;
}
//----------------------------------------------------------------------
QString AppManager::getFileMetadataJson(int catalogId, const QString &fileName, const QString &folderPath)
{
    return Catalog::getFileMetadataJson(catalogId, fileName, folderPath, m_connectionName);
}
//----------------------------------------------------------------------
QVariantList AppManager::getFileMetadataParsedFields(int catalogId, const QString &fileName, const QString &folderPath)
{
    QString json = Catalog::getFileMetadataJson(catalogId, fileName, folderPath, m_connectionName);
    if (json.isEmpty())
        return {};

    QJsonParseError err;
    QJsonDocument doc = QJsonDocument::fromJson(json.toUtf8(), &err);
    if (err.error != QJsonParseError::NoError || !doc.isObject())
        return {};

    const auto pairs = FileMetadata::parseExtendedMetadataFields(doc.object());
    QVariantList result;
    result.reserve(pairs.size());
    for (const auto &p : pairs) {
        QVariantMap row;
        row["label"] = p.first;
        row["value"] = p.second;
        result.append(row);
    }
    return result;
}
//----------------------------------------------------------------------
QString AppManager::calculateAndSaveChecksum(const QString &filePath, const QString &fileName, const QString &folderPath, int catalogId)
{
    if (!QFileInfo::exists(filePath))
        return QString();
    QString checksum = FileChecksum::calculateChecksum(filePath, QCryptographicHash::Sha256);
    if (!checksum.isEmpty() && catalogId > 0)
        FileChecksum::updateFileChecksum(m_connectionName, catalogId, fileName, folderPath, checksum, "SHA256");
    return checksum;
}
//----------------------------------------------------------------------
QString AppManager::verifyFileChecksum(const QString &filePath, const QString &expectedChecksum)
{
    auto result = FileChecksum::verifyChecksum(filePath, expectedChecksum, QCryptographicHash::Sha256);
    if (!result.success)
        return "error:" + result.errorMessage;
    return result.match ? "match" : "mismatch:" + result.actualChecksum;
}
//----------------------------------------------------------------------
QVariantList AppManager::getSearchHistory() const
{
    // Column index map (matches SELECT order below):
    // 0  date_time
    // 1  text_checked         2  text_phrase      3  text_criteria
    // 4  case_sensitive       5  text_exclude
    // 6  file_criteria_checked
    // 7  file_type_checked    8  file_type
    // 9  file_size_checked    10 file_size_min     11 file_size_min_unit
    //                         12 file_size_max     13 file_size_max_unit
    // 14 date_modified_checked 15 date_modified_min 16 date_modified_max
    // 17 metadata_checked
    // 18 duplicates_checked   19 dup_name  20 dup_size  21 dup_date  22 dup_checksum
    // 23 differences_checked
    // 24 folder_criteria_checked  25 show_folders  26 tag_checked  27 tag

    QVariantList result;
    QSqlQuery query(QSqlDatabase::database(m_connectionName));
    query.prepare(QLatin1String(R"(
        SELECT date_time,
               text_checked, text_phrase, text_criteria, case_sensitive, text_exclude,
               file_criteria_checked,
               file_type_checked, file_type,
               file_size_checked, file_size_min, file_size_min_unit, file_size_max, file_size_max_unit,
               date_modified_checked, date_modified_min, date_modified_max,
               metadata_checked,
               duplicates_checked, duplicates_name, duplicates_size, duplicates_date_modified, duplicates_checksum,
               differences_checked,
               folder_criteria_checked, show_folders, tag_checked, tag
        FROM search ORDER BY date_time DESC
    )"));
    if (!query.exec())
        return result;

    while (query.next()) {
        QStringList parts;

        // ── File name ──────────────────────────────────────────────────────
        if (query.value(1).toBool()) {
            QString phrase = query.value(2).toString();
            QString criteria = query.value(3).toString();
            if (!phrase.isEmpty()) {
                // A multi-term phrase holds its terms joined with '\n'. The
                // delegate is a single line, so quote each term and join them
                // with ", " — a newline must never reach it. A single-term
                // phrase renders exactly as it always has.
                QString item = quoteSearchTerms(phrase);
                if (!criteria.isEmpty() && criteria != QLatin1String("All Words"))
                    item += QLatin1String(" (") + criteria + QLatin1Char(')');
                parts << item;
            }
            if (query.value(4).toBool())
                parts << tr("Case sensitive");
            QString exclude = query.value(5).toString();
            if (!exclude.isEmpty())
                parts << tr("Exclude: %1").arg(quoteSearchTerms(exclude));
        }

        // ── File criteria ──────────────────────────────────────────────────
        if (query.value(6).toBool()) {
            if (query.value(7).toBool())
                parts << tr("Type: %1").arg(query.value(8).toString());
            if (query.value(9).toBool())
                parts << tr("Size: %1 %2 – %3 %4")
                          .arg(query.value(10).toString(), query.value(11).toString(),
                               query.value(12).toString(), query.value(13).toString());
            if (query.value(14).toBool())
                parts << tr("Date: %1 – %2")
                          .arg(query.value(15).toString(), query.value(16).toString());
        }

        // ── Metadata ───────────────────────────────────────────────────────
        if (query.value(17).toBool())
            parts << tr("Metadata");

        // ── Duplicates ─────────────────────────────────────────────────────
        if (query.value(18).toBool()) {
            QStringList dup;
            if (query.value(19).toBool()) dup << tr("Name");
            if (query.value(20).toBool()) dup << tr("Size");
            if (query.value(21).toBool()) dup << tr("Date");
            if (query.value(22).toBool()) dup << tr("Checksum");
            parts << tr("Duplicates: %1").arg(dup.isEmpty() ? QStringLiteral("–") : dup.join(QLatin1String(", ")));
        }

        // ── Differences ────────────────────────────────────────────────────
        if (query.value(23).toBool())
            parts << tr("Differences");

        // ── Folder criteria ────────────────────────────────────────────────
        if (query.value(24).toBool()) {
            if (query.value(25).toBool()) parts << tr("Folders only");
            if (query.value(26).toBool()) {
                QString tag = query.value(27).toString();
                parts << (tag.isEmpty() ? tr("Tag") : tr("Tag: %1").arg(tag));
            }
        }

        QVariantMap entry;
        entry["dateTime"] = query.value(0).toString();
        entry["summary"]  = parts.join(QLatin1String("  |  "));
        result.append(entry);
    }
    return result;
}
//----------------------------------------------------------------------
QVariantMap AppManager::restoreSearchHistory(const QString &dateTime)
{
    if (!searchObject)
        return {};
    searchObject->searchDateTime = dateTime;
    searchObject->loadSearchHistoryCriteria(m_connectionName);
    return searchObject->properties();
}
//----------------------------------------------------------------------
void AppManager::clearSearchHistory()
{
    collection->clearSearchHistory(m_connectionName);
}
//----------------------------------------------------------------------
void AppManager::keepLastSearchHistory(int count)
{
    collection->keepLastSearchHistory(count, m_connectionName);
}
//----------------------------------------------------------------------
QStringList AppManager::getTagNames() const
{
    collection->loadTagFileToTable();
    Tag tag;
    tag.loadFromDatabase(m_connectionName);
    QStringList result;
    for (const QString &name : tag.tagNames())
        if (!result.contains(name))
            result << name;
    result.sort();
    return result;
}
//----------------------------------------------------------------------
QVariantList AppManager::getTagEntries(const QString &filterName) const
{
    collection->loadTagFileToTable();
    Tag tagModel;
    tagModel.loadFromDatabase(m_connectionName, filterName);
    QVariantList result;
    for (int i = 0; i < tagModel.rowCount(); ++i) {
        QModelIndex idx = tagModel.index(i, 0);
        QVariantMap entry;
        entry[QStringLiteral("tagId")]  = tagModel.data(idx, Tag::IdRole);
        entry[QStringLiteral("name")]   = tagModel.data(idx, Tag::NameRole);
        entry[QStringLiteral("folder")] = tagModel.data(idx, Tag::FolderRole);
        result << entry;
    }
    return result;
}
//----------------------------------------------------------------------
bool AppManager::createTag(const QString &name, const QString &path)
{
    bool ok = collection->createTag(name, path, QString(), QDateTime::currentDateTime());
    if (ok) emit tagsChanged();
    return ok;
}
//----------------------------------------------------------------------
bool AppManager::deleteTag(int tagID)
{
    bool ok = collection->deleteTag(tagID);
    if (ok) emit tagsChanged();
    return ok;
}
//----------------------------------------------------------------------
QVariantMap AppManager::getStatisticsData(const QString &source, const QString &dataType, const QString &startDate) const
{
    collection->loadStatisticsDeviceFileToTable();

    QDateTime dt;
    if (!startDate.isEmpty())
        dt = QDateTime::fromString(startDate, "yyyy-MM-dd");

    StatChartData data = Statistics::getChartData(
        m_connectionName,
        selectedDevice->ID,
        selectedDevice->type,
        source,
        dataType,
        dt
    );

    auto toVarList = [](const QList<StatPoint> &pts) {
        QVariantList list;
        list.reserve(pts.size());
        for (const auto &p : pts) {
            QVariantMap m;
            m[QStringLiteral("x")] = p.msecsSinceEpoch;
            m[QStringLiteral("y")] = p.value;
            list << m;
        }
        return list;
    };

    QVariantMap result;
    result[QStringLiteral("series1")]     = toVarList(data.series1);
    result[QStringLiteral("series2")]     = toVarList(data.series2);
    result[QStringLiteral("series3")]     = toVarList(data.series3);
    result[QStringLiteral("loadSeries1")] = data.loadSeries1;
    result[QStringLiteral("loadSeries2")] = data.loadSeries2;
    result[QStringLiteral("loadSeries3")] = data.loadSeries3;
    result[QStringLiteral("maxValue")]    = data.maxValue;
    result[QStringLiteral("unitKey")]     = data.unitKey;
    result[QStringLiteral("deviceName")]  = selectedDevice->name;
    result[QStringLiteral("hasData")]     = !data.series1.isEmpty();
    return result;
}
//----------------------------------------------------------------------
QString AppManager::getStatisticsSetting(const QString &key) const
{
    QSettings settings(collection->settingsFilePath, QSettings::IniFormat);
    return settings.value("Statistics/" + key).toString();
}
//----------------------------------------------------------------------
void AppManager::setStatisticsSetting(const QString &key, const QVariant &value)
{
    QSettings settings(collection->settingsFilePath, QSettings::IniFormat);
    settings.setValue("Statistics/" + key, value);
    settings.sync();
}
//----------------------------------------------------------------------
bool AppManager::shouldShowAlphaWarning() const
{
    QSettings settings(collection->settingsFilePath, QSettings::IniFormat);
    return settings.value("Settings/ShowBetaVersion", true).toBool();
}

void AppManager::setAlphaWarningShown()
{
    QSettings settings(collection->settingsFilePath, QSettings::IniFormat);
    settings.setValue("Settings/ShowBetaVersion", false);
    settings.sync();
}
//----------------------------------------------------------------------
void AppManager::clearFirstRun()
{
    if (m_firstRun) {
        m_firstRun = false;
        emit firstRunChanged();
    }
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

        // Load the row first (single indexed query, synchronous) so that name/path/
        // type are populated before the signal fires. Emitting earlier left bindings
        // on selectedDeviceName/Path/Type reading the *previous* device's values with
        // no later notification, i.e. always one selection behind.
        selectedDevice->loadDevice(m_connectionName);

        QSettings settings(collection->settingsFilePath, QSettings::IniFormat);
        settings.setValue("Selection/SelectedDeviceID", deviceId);

        emit selectedDeviceChanged(deviceId);

        qDebug() << "Selected device changed to ID:" << deviceId << "Name:" << selectedDevice->name;
    }
}
//----------------------------------------------------------------------
QString AppManager::getDatabaseMode() const
{
    return collection->databaseMode;
}
//----------------------------------------------------------------------
QString AppManager::getCollectionFolder() const
{
    return collection->folder;
}
//----------------------------------------------------------------------
void AppManager::openCollectionMemory(const QString &folder)
{
    collection->folder = folder;

    QSettings settings(collection->settingsFilePath, QSettings::IniFormat);
    settings.setValue("Settings/databaseMode",  "Memory");
    settings.setValue("LastCollectionFolder",   folder);
    settings.sync();

    if (reconnectToDatabase()) {
        saveToRecentCollections("Memory", folder, QFileInfo(folder).fileName());
        emit databaseConnectionChanged(true, tr("Memory collection opened: %1").arg(QFileInfo(folder).fileName()));
    } else {
        emit databaseConnectionChanged(false, tr("Failed to open Memory collection: %1").arg(lastDatabaseError));
    }
}
//----------------------------------------------------------------------
void AppManager::openCollectionHosted(const QString &hostName, const QString &dbName,
                                      int port, const QString &userName, const QString &password)
{
    QSettings settings(collection->settingsFilePath, QSettings::IniFormat);
    settings.setValue("Settings/databaseMode",    "Hosted");
    settings.setValue("Settings/databaseHostName", hostName);
    settings.setValue("Settings/databaseName",     dbName);
    settings.setValue("Settings/databasePort",     port);
    settings.setValue("Settings/databaseUserName", userName);
    settings.setValue("Settings/databasePassword", password);
    settings.sync();

    if (reconnectToDatabase()) {
        saveToRecentCollections("Hosted", hostName + "/" + dbName, hostName + "/" + dbName,
                                hostName, dbName, port, userName, password);
        emit databaseConnectionChanged(true, tr("Connected: %1/%2").arg(hostName).arg(dbName));
    } else {
        emit databaseConnectionChanged(false, tr("Hosted connection failed: %1").arg(lastDatabaseError));
    }
}
//----------------------------------------------------------------------
QVariantList AppManager::getRecentCollections() const
{
    QSettings settings(collection->settingsFilePath, QSettings::IniFormat);
    int count = qMin(settings.value("Recent/count", 0).toInt(), 5);
    QVariantList result;
    for (int i = 0; i < count; ++i) {
        QVariantMap entry;
        entry["mode"]        = settings.value(QString("Recent/%1/mode").arg(i)).toString();
        entry["path"]        = settings.value(QString("Recent/%1/path").arg(i)).toString();
        entry["displayName"] = settings.value(QString("Recent/%1/displayName").arg(i)).toString();
        QString storedIcon   = settings.value(QString("Recent/%1/iconName").arg(i)).toString();
        if      (storedIcon == "server-database") storedIcon = "network-server-database";
        else if (storedIcon == "network-server")  storedIcon = "network-workgroup";
        entry["iconName"]    = storedIcon;
        entry["hostName"]    = settings.value(QString("Recent/%1/hostName").arg(i)).toString();
        entry["dbName"]      = settings.value(QString("Recent/%1/dbName").arg(i)).toString();
        entry["port"]        = settings.value(QString("Recent/%1/port").arg(i), 3306).toInt();
        entry["userName"]    = settings.value(QString("Recent/%1/userName").arg(i)).toString();
        entry["password"]    = settings.value(QString("Recent/%1/password").arg(i)).toString();
        result.append(entry);
    }
    return result;
}
//----------------------------------------------------------------------
QString AppManager::getCurrentCollectionDisplayName() const
{
    QString mode = getDatabaseMode();
    QSettings settings(collection->settingsFilePath, QSettings::IniFormat);
    if (mode == "Memory")
        return QFileInfo(collection->folder).fileName();
    if (mode == "File")
        return QFileInfo(settings.value("Settings/DatabaseFilePath").toString()).fileName();
    if (mode == "Hosted")
        return settings.value("Settings/databaseHostName").toString()
               + "/" + settings.value("Settings/databaseName").toString();
    return QString();
}
//----------------------------------------------------------------------
QString AppManager::getCurrentCollectionIconName() const
{
    QString mode = getDatabaseMode();
    if (mode == "Memory") return "folder";
    if (mode == "File")   return "network-server-database";
    return "network-workgroup";
}
//----------------------------------------------------------------------
void AppManager::openRecentCollection(const QVariantMap &entry)
{
    QString mode = entry["mode"].toString();
    if (mode == "Memory") {
        openCollectionMemory(entry["path"].toString());
    } else if (mode == "File") {
        setDatabaseFilePath(entry["path"].toString());
    } else if (mode == "Hosted") {
        openCollectionHosted(entry["hostName"].toString(),
                             entry["dbName"].toString(),
                             entry["port"].toInt(),
                             entry["userName"].toString(),
                             entry["password"].toString());
    }
}
//----------------------------------------------------------------------
void AppManager::saveToRecentCollections(const QString &mode, const QString &path,
                                         const QString &displayName,
                                         const QString &hostName, const QString &dbName,
                                         int port, const QString &userName, const QString &password)
{
    QSettings settings(collection->settingsFilePath, QSettings::IniFormat);
    int count = qMin(settings.value("Recent/count", 0).toInt(), 5);

    // Load existing entries
    QVariantList existing;
    for (int i = 0; i < count; ++i) {
        QVariantMap e;
        e["mode"]        = settings.value(QString("Recent/%1/mode").arg(i)).toString();
        e["path"]        = settings.value(QString("Recent/%1/path").arg(i)).toString();
        e["displayName"] = settings.value(QString("Recent/%1/displayName").arg(i)).toString();
        e["iconName"]    = settings.value(QString("Recent/%1/iconName").arg(i)).toString();
        e["hostName"]    = settings.value(QString("Recent/%1/hostName").arg(i)).toString();
        e["dbName"]      = settings.value(QString("Recent/%1/dbName").arg(i)).toString();
        e["port"]        = settings.value(QString("Recent/%1/port").arg(i), 3306).toInt();
        e["userName"]    = settings.value(QString("Recent/%1/userName").arg(i)).toString();
        e["password"]    = settings.value(QString("Recent/%1/password").arg(i)).toString();
        existing.append(e);
    }

    // Remove any existing entry with the same path (dedup)
    for (int i = existing.size() - 1; i >= 0; --i) {
        if (existing[i].toMap()["path"].toString() == path)
            existing.removeAt(i);
    }

    // Build and prepend new entry
    QString iconName = (mode == "Memory") ? "folder"
                     : (mode == "File")   ? "network-server-database"
                                          : "network-workgroup";
    QVariantMap newEntry;
    newEntry["mode"]        = mode;
    newEntry["path"]        = path;
    newEntry["displayName"] = displayName;
    newEntry["iconName"]    = iconName;
    newEntry["hostName"]    = hostName;
    newEntry["dbName"]      = dbName;
    newEntry["port"]        = port;
    newEntry["userName"]    = userName;
    newEntry["password"]    = password;
    existing.prepend(newEntry);

    // Trim to 5
    while (existing.size() > 5)
        existing.removeLast();

    // Persist
    settings.setValue("Recent/count", existing.size());
    for (int i = 0; i < existing.size(); ++i) {
        QVariantMap e = existing[i].toMap();
        settings.setValue(QString("Recent/%1/mode").arg(i),        e["mode"]);
        settings.setValue(QString("Recent/%1/path").arg(i),        e["path"]);
        settings.setValue(QString("Recent/%1/displayName").arg(i), e["displayName"]);
        settings.setValue(QString("Recent/%1/iconName").arg(i),    e["iconName"]);
        settings.setValue(QString("Recent/%1/hostName").arg(i),    e["hostName"]);
        settings.setValue(QString("Recent/%1/dbName").arg(i),      e["dbName"]);
        settings.setValue(QString("Recent/%1/port").arg(i),        e["port"]);
        settings.setValue(QString("Recent/%1/userName").arg(i),    e["userName"]);
        settings.setValue(QString("Recent/%1/password").arg(i),    e["password"]);
    }
    settings.sync();

    emit recentCollectionsChanged();
}
//----------------------------------------------------------------------
// Catalog creation
//----------------------------------------------------------------------
void AppManager::setupDeviceUpdateManager()
{
    if (!m_deviceUpdateManager) {
        m_deviceUpdateManager = new DeviceUpdateManager(this);
    }
    disconnect(m_deviceUpdateManager, nullptr, this, nullptr);

    if (!m_catalogProgressManager)
        m_catalogProgressManager = new CatalogProgressManager(this);
    // Reconnected on every setup, not only on creation: the devices variant of
    // this setup points the same progress manager at m_deviceUpdateStatusText,
    // so a creation started after an update would otherwise report its progress
    // into the devices status text and leave the create text stale.
    disconnect(m_catalogProgressManager, nullptr, this, nullptr);
    connect(m_catalogProgressManager, &CatalogProgressManager::statusMessageChanged,
            this, [this](const QString &message, int /*timeout*/) {
                m_catalogStatusText = message;
                emit catalogStatusTextChanged();
            });
    m_deviceUpdateManager->setCatalogProgressManager(m_catalogProgressManager);
    m_deviceUpdateManager->setConnectionName(m_connectionName);

    connect(m_deviceUpdateManager, &DeviceUpdateManager::operationCompleted,
            this, &AppManager::onCatalogCreationCompleted);
    connect(m_deviceUpdateManager, &DeviceUpdateManager::operationError,
            this, &AppManager::onCatalogCreationError);
    connect(m_deviceUpdateManager, &DeviceUpdateManager::operationCancelled,
            this, &AppManager::onCatalogCreationCancelled);
}
//----------------------------------------------------------------------
QString AppManager::createCatalog(const QString &name, const QString &path,
                                   int storageId,
                                   const QString &fileType, bool includeSubDir,
                                   bool includeHidden, bool includeSymlinks,
                                   bool isFullDevice,
                                   const QString &includeMetadata,
                                   const QString &includeChecksum,
                                   const QStringList &perCatalogExcludes)
{
    // Validation
    if (name.trimmed().isEmpty())
        return tr("Provide a name for this new catalog.");
    if (path.trimmed().isEmpty())
        return tr("Provide a path for this new catalog.");
    if (storageId <= 0)
        return tr("Select a Storage for this new catalog.\n(Selection panel on the left and dropdown list)");
    QDir sourceDir(path);
    if (!sourceDir.exists())
        return tr("The source directory does not exist.");

    // Duplicate name check
    Device *newDevice = new Device();
    newDevice->generateDeviceID();
    newDevice->type = "Catalog";
    newDevice->name = name.trimmed();
    if (newDevice->verifyDeviceNameExists()) {
        delete newDevice;
        return tr("There is already a catalog with this name:\n%1\n\nChoose a different name and try again.").arg(name.trimmed());
    }

    // Populate device and catalog
    newDevice->parentID = storageId;
    newDevice->catalog->generateID();
    newDevice->externalID = newDevice->catalog->ID;
    newDevice->groupID    = 0;
    newDevice->path       = path;
    newDevice->insertDevice();

    // Load storage name for catalog record
    Device parentDevice;
    parentDevice.ID = storageId;
    parentDevice.loadDevice(m_connectionName);

    newDevice->catalog->name             = newDevice->name;
    newDevice->catalog->filePath         = collection->folder + "/" + newDevice->name + ".idx";
    newDevice->catalog->sourcePath       = path;
    newDevice->catalog->fileType         = fileType;
    newDevice->catalog->includeSubDir    = includeSubDir;
    newDevice->catalog->includeHidden    = includeHidden;
    newDevice->catalog->includeSymblinks = includeSymlinks;
    newDevice->catalog->isFullDevice     = isFullDevice;
    newDevice->catalog->includeMetadata  = includeMetadata;
    newDevice->catalog->includeChecksum  = includeChecksum;
    newDevice->catalog->storageName      = parentDevice.name;
    newDevice->catalog->appVersion       = currentVersion;
    newDevice->catalog->insertCatalog();

    // Per-catalog exclude folders
    for (const QString &folder : perCatalogExcludes)
        newDevice->catalog->addExcludeFolder(folder);
    collection->saveCatalogFilterTableToFile();

    // Update parent Storage path if it was empty
    if (parentDevice.path.isEmpty()) {
        parentDevice.path = path;
        parentDevice.saveDevice();
        collection->saveStorageTableToFile();
    }

    // Check for a concurrent operation BEFORE touching the manager: rewiring its
    // handlers now would hand the operation already running to the wrong set,
    // and its completion would be reported as this creation's.
    if (m_catalogIsCreating
        || m_deviceUpdateIsRunning
        || (m_deviceUpdateManager && m_deviceUpdateManager->operationRunning())) {
        // Busy: queue the scan instead of refusing. The device and catalog rows
        // above are already written, so the entry only needs the device id — and
        // the catalog is visible immediately, waiting to be filled. The handlers
        // are wired when the entry actually starts, in startNextDeviceUpdate().
        const int     queuedId   = newDevice->ID;
        const QString queuedName = newDevice->name;
        delete newDevice;
        enqueueOperation(queuedId, true, queuedName);
        emit deviceListChanged();
        refreshDeviceList();
        return QString();
    }

    setupDeviceUpdateManager();
    m_creatingDevice = newDevice;
    m_catalogIsCreating = true;
    m_catalogCreateStartTime = QDateTime::currentDateTime();
    m_catalogStatusText = StatusBarMessageBuilder().setOperation(tr("Create")).setStatus(tr("In Progress")).build();
    emit catalogIsCreatingChanged();
    emit catalogStatusTextChanged();

    // The scan runs synchronously on the GUI thread (CatalogJobStoppable pumps
    // the event loop by hand), so calling it here would block before Qt Quick
    // ever paints the activity panel — on a small catalog the panel would be
    // shown for zero frames and its indicator never seen. One turn of the event
    // loop first guarantees the panel is on screen, with the indicator running,
    // before the scan takes the thread. The busy flags above are set
    // synchronously, so the concurrency guard still closes at request time.
    Device *deviceToScan = newDevice;
    QTimer::singleShot(kStartDeferralMs, this, [this, deviceToScan]() {
        m_deviceUpdateManager->updateDeviceHierarchy(deviceToScan,
                                                     collection->databaseMode,
                                                     collection->folder,
                                                     "create");
    });
    return QString();
}
//----------------------------------------------------------------------
void AppManager::stopCatalogCreation()
{
    if (m_deviceUpdateManager && m_deviceUpdateManager->operationRunning())
        m_deviceUpdateManager->requestHardStop();
}
//----------------------------------------------------------------------
bool AppManager::isDirectoryEmpty(const QString &path) const
{
    QDir dir(path);
    return dir.exists() && dir.entryInfoList(QDir::NoDotAndDotDot | QDir::AllEntries).isEmpty();
}
//----------------------------------------------------------------------
void AppManager::onCatalogCreationCompleted(const QList<qint64> &/*results*/)
{
    collection->saveDeviceTableToFile();
    collection->saveStatiticsTableToFile();

    m_catalogIsCreating = false;
    emit catalogIsCreatingChanged();
    // Status bar message is built and emitted by CatalogProgressManager
    // via statusMessageChanged → m_catalogStatusText, matching the spec:
    // CREATE | Completed | Catalog 1 of 1 | <name> | Indexed: N of N (100%)

    refreshDeviceList();
    emit catalogCreationCompleted(true, QString());
    m_creatingDevice = nullptr;

    finishRunningOperation();
}
//----------------------------------------------------------------------
void AppManager::onCatalogCreationError(const QString &error)
{
    if (m_creatingDevice) {
        m_creatingDevice->deleteDevice(false);
        m_creatingDevice = nullptr;
    }
    m_catalogIsCreating = false;
    m_catalogStatusText.clear();
    emit catalogIsCreatingChanged();
    emit catalogStatusTextChanged();
    refreshDeviceList();
    emit catalogCreationCompleted(false, error);

    // A failure does not abandon what is queued behind it (OPQ-F9).
    finishRunningOperation();
}
//----------------------------------------------------------------------
void AppManager::onCatalogCreationCancelled()
{
    // A stop during the metadata/checksum phase happens after the file list is
    // committed. The catalog is complete as an index and is kept; only the
    // optional scan is unfinished, and the next update finishes it. Deleting the
    // device here would throw away a catalog the user just spent minutes building.
    const bool keptCatalog = m_deviceUpdateManager && m_deviceUpdateManager->lastCatalogCommitted();
    const QString incompleteMessage = m_deviceUpdateManager
                                      ? m_deviceUpdateManager->lastScanIncompleteMessage()
                                      : QString();

    if (!keptCatalog && m_creatingDevice) {
        m_creatingDevice->deleteDevice(false);
    }
    m_creatingDevice = nullptr;

    m_catalogIsCreating = false;
    m_catalogStatusText.clear();
    emit catalogIsCreatingChanged();
    emit catalogStatusTextChanged();
    refreshDeviceList();

    if (keptCatalog)
        emit catalogCreationCompleted(true, incompleteMessage);
    else
        emit catalogCreationCompleted(false, tr("Catalog creation was stopped."));

    // Stopping this creation does not abandon what is queued behind it (OPQ-F7):
    // carry on with the next entry.
    finishRunningOperation();
}
//----------------------------------------------------------------------
// Storage pre-selection for Create page (used by DeviceTreeComboBox storageOnly mode)
//----------------------------------------------------------------------
int AppManager::getDefaultStorageId() const
{
    const QString type = selectedDevice->type;
    if (type == "Storage")
        return selectedDevice->ID;
    if (type == "Catalog")
        return selectedDevice->parentID;
    if (type == "Virtual")
        return Device::getFirstStorageDescendantId(selectedDevice->ID, m_connectionName);
    return 0;
}

// Device delete
//----------------------------------------------------------------------
QVariantMap AppManager::checkDeviceDeleteAllowed(int deviceId) const
{
    const QString conn = m_connectionName;
    Device dev;
    dev.ID = deviceId;
    dev.loadDevice(conn);
    dev.verifyHasSubDevice(conn);

    QVariantMap result;
    if (dev.hasSubDevice) {
        result["allowed"]      = false;
        result["errorMessage"] = tr("The selected device cannot be deleted as long as it has sub-devices.");
    } else {
        result["allowed"]      = true;
        result["errorMessage"] = QString();
    }
    return result;
}
//----------------------------------------------------------------------
QString AppManager::deleteDevice(int deviceId)
{
    const QString conn = m_connectionName;
    Device dev;
    dev.ID = deviceId;
    dev.loadDevice(conn);

    Device::DeleteOperationResult res = dev.deleteDevice(false);

    switch (res.result) {
    case Device::DeleteHasSubDevices:
        return tr("The selected device cannot be deleted as long as it has sub-devices.");
    case Device::DeleteError:
        return res.errorMessage;
    case Device::DeleteCancelled:
        return tr("Deletion cancelled.");
    case Device::DeleteSuccess:
        break;
    }

    // Update parent numbers (mirrors K2 deleteDeviceItem)
    if (dev.parentID > 0) {
        Device parent;
        parent.ID = dev.parentID;
        parent.loadDevice(conn);
        parent.updateNumbersFromChildren();
        parent.updateParentsNumbers();
    }

    // Persist to files
    collection->saveDeviceTableToFile();
    collection->saveStorageTableToFile();

    // In Memory mode, move the catalog index files to trash
    if (dev.type == "Catalog") {
        Collection::DeleteCatalogResult fileResult = collection->deleteCatalogFile(&dev);
        if (fileResult == Collection::DeleteFailedToMoveToTrash) {
            return tr("Deletion failed");
        }
    }

    emit deviceListChanged();
    refreshAllUI();
    return QString();
}

// Devices page operations
//----------------------------------------------------------------------
QVariantList AppManager::getDeviceList(const QString &viewFilter, int scopeDeviceId) const
{
    const QString conn = m_connectionName;

    // Re-probe the filesystem before reading device_active, so opening the
    // Devices page or switching its type filter reflects what is mounted now.
    // K2 does the same inside its device-tree model loaders; without it this
    // path would display whatever was cached at the last device operation.
    collection->updateAllDeviceActive();
    m_lastActiveProbe = QDateTime::currentDateTime();

    const QList<Device::DeviceTreeNode> nodes = Device::loadDeviceTree(conn, scopeDeviceId);

    // Build id → type map so each node can resolve its parent's type without a second query.
    QHash<int, QString> typeById;
    typeById.reserve(nodes.size());
    for (const Device::DeviceTreeNode &n : nodes)
        typeById.insert(n.id, n.type);

    QVariantList result;
    result.reserve(nodes.size());

    for (const Device::DeviceTreeNode &n : nodes) {
        // Apply type filter — "All" (or any unrecognised value) passes everything through.
        if (viewFilter == QLatin1String("Catalogs") && n.type != QLatin1String("Catalog"))
            continue;
        if (viewFilter == QLatin1String("Storage") && n.type != QLatin1String("Storage"))
            continue;

        QVariantMap item;
        item[QStringLiteral("deviceId")]      = n.id;
        item[QStringLiteral("parentId")]      = n.parentId;
        item[QStringLiteral("name")]          = n.name;
        item[QStringLiteral("type")]          = n.type;
        item[QStringLiteral("path")]          = n.path;
        item[QStringLiteral("totalFileSize")] = n.totalFileSize;
        item[QStringLiteral("fileCount")]     = n.totalFileCount;
        item[QStringLiteral("totalSpace")]    = n.totalSpace;
        item[QStringLiteral("freeSpace")]     = n.freeSpace;
        item[QStringLiteral("active")]        = n.isActive;
        item[QStringLiteral("dateUpdated")]   = n.dateUpdated;
        item[QStringLiteral("level")]         = n.level;
        item[QStringLiteral("parentType")]    = typeById.value(n.parentId);
        item[QStringLiteral("groupId")]       = n.groupId;
        item[QStringLiteral("externalId")]    = n.externalId;
        item[QStringLiteral("comment")]       = n.comment;
        result.append(item);
    }
    return result;
}
//----------------------------------------------------------------------
int AppManager::addDeviceVirtual(int parentId)
{
    const QString conn = m_connectionName;
    Device parent;
    parent.ID = parentId;
    parent.loadDevice(conn);

    Device *newDevice = new Device();
    newDevice->generateDeviceID();
    newDevice->type     = QStringLiteral("Virtual");
    newDevice->name     = tr("Virtual") + "_" + QString::number(newDevice->ID);
    newDevice->parentID = parentId;
    newDevice->groupID  = (parentId == 0) ? 1 : parent.groupID;
    newDevice->externalID = 0;
    newDevice->insertDevice();

    collection->saveDeviceTableToFile();
    emit deviceListChanged();
    refreshDeviceList();

    int id = newDevice->ID;
    delete newDevice;
    return id;
}
//----------------------------------------------------------------------
int AppManager::addDeviceStorage(int parentId)
{
    const QString conn = m_connectionName;
    Device parent;
    parent.ID = parentId;
    parent.loadDevice(conn);

    Device *newDevice = new Device();
    newDevice->generateDeviceID();
    newDevice->type     = QStringLiteral("Storage");
    newDevice->name     = tr("Storage") + "_" + QString::number(newDevice->ID);
    newDevice->parentID = parentId;
    newDevice->groupID  = 0;
    newDevice->storage->generateID();
    newDevice->externalID = newDevice->storage->ID;
    newDevice->insertDevice();
    newDevice->storage->name = newDevice->name;
    newDevice->storage->insertStorage();

    collection->saveDeviceTableToFile();
    collection->saveStorageTableToFile();
    emit deviceListChanged();
    refreshDeviceList();

    int id = newDevice->ID;
    delete newDevice;
    return id;
}
//----------------------------------------------------------------------
void AppManager::setupDeviceUpdateManagerForDevices()
{
    if (!m_deviceUpdateManager)
        m_deviceUpdateManager = new DeviceUpdateManager(this);
    disconnect(m_deviceUpdateManager, nullptr, this, nullptr);

    if (!m_catalogProgressManager) {
        m_catalogProgressManager = new CatalogProgressManager(this);
    }
    disconnect(m_catalogProgressManager, nullptr, this, nullptr);
    connect(m_catalogProgressManager, &CatalogProgressManager::statusMessageChanged,
            this, [this](const QString &msg, int) {
                m_deviceUpdateStatusText = msg;
                emit deviceUpdateStatusChanged();
            });
    m_deviceUpdateManager->setCatalogProgressManager(m_catalogProgressManager);
    m_deviceUpdateManager->setConnectionName(m_connectionName);

    connect(m_deviceUpdateManager, &DeviceUpdateManager::operationCompleted,
            this, &AppManager::onDevicePageUpdateCompleted);
    connect(m_deviceUpdateManager, &DeviceUpdateManager::operationError,
            this, [this](const QString &err) {
                m_deviceUpdateStatusText = tr("Error: %1").arg(err);
                emit deviceUpdateStatusChanged();
                // A failure does not abandon the rest of the queue (OPQ-F9).
                // The running flag is cleared by startNextDeviceUpdate() once the
                // queue is found empty, so the panel does not blink out between
                // two entries.
                finishRunningOperation();
            });
    connect(m_deviceUpdateManager, &DeviceUpdateManager::operationCancelled,
            this, [this]() {
                // Stop ends the running operation only; the queue carries on
                // (OPQ-F7). Emptying the queue is the separate Clear action.
                m_isBatchUpdate = false;
                m_deviceUpdateStatusText.clear();
                emit deviceUpdateStatusChanged();
                emit deviceListChanged();
                finishRunningOperation();
            });
}
//----------------------------------------------------------------------
void AppManager::onDevicePageUpdateCompleted(const QList<qint64> &results)
{
    collection->saveDeviceTableToFile();
    collection->saveStatiticsTableToFile();
    emit deviceListChanged();
    refreshDeviceList();

    if (m_isBatchUpdate) {
        // Single operationCompleted for the whole "All Active" batch — show aggregate
        QVariantMap report = buildBatchUpdateReport(results);
        if (!report.isEmpty())
            emit deviceUpdateReportReady(report);

        m_isBatchUpdate = false;
        finishRunningOperation();
    } else {
        if (m_showEachUpdateReport) {
            QVariantMap report = buildUpdateReport(m_currentUpdateDeviceId, results);
            if (!report.isEmpty()) {
                emit deviceUpdateReportReady(report);
                if (!m_pendingDeviceUpdates.isEmpty()) {
                    // More entries waiting — the queue holds until the user
                    // acknowledges this report (OPQ-F10). The entry itself is
                    // over, so it stops being shown as running right away.
                    m_waitingForReportAck = true;
                    m_hasRunningOperation = false;
                    emitQueueChanged();
                    return;
                }
            }
        }
        finishRunningOperation();
    }
}
//----------------------------------------------------------------------
QVariantList AppManager::getOperationQueue() const
{
    // The running entry first, then the waiting ones in run order.
    QVariantList out;
    if (m_hasRunningOperation) {
        QVariantMap e;
        e["deviceId"]   = m_runningOperation.deviceId;
        e["deviceName"] = m_runningOperation.deviceName;
        e["isCreate"]   = m_runningOperation.isCreate;
        e["running"]    = true;
        out.append(e);
    }
    for (const QueuedOperation &op : m_pendingDeviceUpdates) {
        QVariantMap e;
        e["deviceId"]   = op.deviceId;
        e["deviceName"] = op.deviceName;
        e["isCreate"]   = op.isCreate;
        e["running"]    = false;
        out.append(e);
    }
    return out;
}
//----------------------------------------------------------------------
void AppManager::emitQueueChanged()
{
    emit operationQueueChanged();
}
//----------------------------------------------------------------------
bool AppManager::isDeviceQueuedOrRunning(int deviceId) const
{
    if (m_hasRunningOperation && m_runningOperation.deviceId == deviceId)
        return true;
    for (const QueuedOperation &op : m_pendingDeviceUpdates)
        if (op.deviceId == deviceId)
            return true;
    return false;
}
//----------------------------------------------------------------------
void AppManager::enqueueOperation(int deviceId, bool isCreate, const QString &deviceName)
{
    // A device can appear once: re-requesting one already queued or running
    // leaves the queue untouched rather than doubling the work.
    if (isDeviceQueuedOrRunning(deviceId))
        return;

    QueuedOperation op;
    op.deviceId   = deviceId;
    op.isCreate   = isCreate;
    op.deviceName = deviceName;
    m_pendingDeviceUpdates.append(op);
    emitQueueChanged();
}
//----------------------------------------------------------------------
void AppManager::removeQueuedOperation(int index)
{
    if (index < 0 || index >= m_pendingDeviceUpdates.size())
        return;

    const QueuedOperation op = m_pendingDeviceUpdates.takeAt(index);

    // A queued creation already has its device and catalog rows in the database
    // (they are written before the operation is queued). Dropping the entry must
    // remove them too, or an empty catalog is left behind.
    if (op.isCreate) {
        Device dev;
        dev.ID = op.deviceId;
        dev.loadDevice(m_connectionName);
        dev.deleteDevice(false);
        emit deviceListChanged();
        refreshDeviceList();
    }
    emitQueueChanged();
}
//----------------------------------------------------------------------
void AppManager::clearOperationQueue()
{
    while (!m_pendingDeviceUpdates.isEmpty())
        removeQueuedOperation(0);
}
//----------------------------------------------------------------------
void AppManager::finishRunningOperation()
{
    // The single ending for every way an operation can finish — completed,
    // stopped or failed. The running entry is released and the queue carries on
    // (OPQ-F7, OPQ-F9).
    m_hasRunningOperation = false;
    emitQueueChanged();
    scheduleNextOperation();
}
//----------------------------------------------------------------------
void AppManager::scheduleNextOperation()
{
    // The next entry must never start from inside a DeviceUpdateManager signal
    // handler, because core is not finished with the previous operation when
    // that signal arrives: CatalogManager::onJobResult() runs cleanupJob() as
    // soon as the handlers return, and onCatalogOperationCompleted() arms
    // cleanupOperation() on a short timer of its own. A job started before
    // those run gets the previous operation's teardown applied to it — deleted
    // job, manager reset to idle — and dies silently, or keeps running with no
    // owner. So: wait past core's own deferred cleanup, then confirm the
    // manager really is idle before starting anything (OPQ-C4).
    if (m_nextOperationScheduled)
        return;
    m_nextOperationScheduled = true;
    QTimer::singleShot(operationChainDelayMs, this, [this]() {
        m_nextOperationScheduled = false;
        if (!m_pendingDeviceUpdates.isEmpty()
            && m_deviceUpdateManager && m_deviceUpdateManager->operationRunning()) {
            // Still winding down — come back rather than start on top of it.
            scheduleNextOperation();
            return;
        }
        startNextDeviceUpdate();
    });
}
//----------------------------------------------------------------------
void AppManager::startNextDeviceUpdate()
{
    if (m_pendingDeviceUpdates.isEmpty()) {
        m_deviceUpdateIsRunning = false;
        m_isBatchUpdate         = false;
        m_operationsStarted     = 0;
        m_hasRunningOperation   = false;
        emitQueueChanged();
        emit deviceUpdateStateChanged();
        // Status text lingers for 5 seconds then clears (K2 behaviour)
        QTimer::singleShot(5000, this, [this]() {
            m_deviceUpdateStatusText.clear();
            emit deviceUpdateStatusChanged();
        });
        setupDeviceUpdateManager();
        return;
    }
    const QueuedOperation next = m_pendingDeviceUpdates.takeFirst();
    m_runningOperation    = next;
    m_hasRunningOperation = true;
    m_operationsStarted++;
    emitQueueChanged();

    // Claim the running flag here rather than relying on whoever queued the work:
    // this function is the only place an entry actually starts, so it owns the
    // state. Without this, a path that cleared the flag before the queue advanced
    // (a cancellation, an error) would leave a job running with the UI believing
    // nothing is happening — no progress, no Stop button, no panel.
    m_deviceUpdateIsRunning = true;
    emit deviceUpdateStateChanged();

    m_currentUpdateDeviceId = next.deviceId;
    Device *dev = new Device();
    dev->ID = next.deviceId;
    dev->loadDevice(m_connectionName);

    if (next.isCreate) {
        // Queued creation: the device and catalog rows already exist, so this
        // only runs the scan. The create-specific handlers need m_creatingDevice.
        // The handler set is wired per entry, here rather than by the caller: a
        // creation has to end in onCatalogCreationCompleted(), or m_catalogIsCreating
        // is never cleared and the activity panel stays busy for good.
        setupDeviceUpdateManager();
        m_creatingDevice         = dev;
        m_catalogIsCreating      = true;
        m_catalogCreateStartTime = QDateTime::currentDateTime();
        m_catalogStatusText      = StatusBarMessageBuilder().setOperation(tr("Create"))
                                                            .setStatus(tr("In Progress")).build();
        emit catalogIsCreatingChanged();
        emit catalogStatusTextChanged();
        // Deferred so the panel paints first (OPQ-F11). This function is not only
        // the queue's continuation — it is also where a single request from an
        // idle application starts, so the panel is not already on screen.
        QTimer::singleShot(kStartDeferralMs, this, [this, dev]() {
            m_deviceUpdateManager->updateDeviceHierarchy(dev, collection->databaseMode, collection->folder, "create");
        });
        return;
    }

    setupDeviceUpdateManagerForDevices();
    // Deferred so the panel paints first (OPQ-F11) — this is the path a single
    // update from an idle application takes, not just the queue's continuation.
    // The batch context goes with it: it must still be set AFTER
    // updateDeviceHierarchy (which internally calls cleanupOperation →
    // clearBatchContext), so the context survives into the catalog job's
    // progress signals. Leaving it here would set it before the deferred call
    // and the call would then clear it.
    QTimer::singleShot(kStartDeferralMs, this, [this, dev]() {
        m_deviceUpdateManager->updateDeviceHierarchy(dev, collection->databaseMode, collection->folder, "update");
        // Counted from the queue as it stands rather than from a total fixed when the
        // batch was requested: the queue can grow while it drains, and "3 of 5" has
        // to become "3 of 6" when it does.
        const int knownTotal = m_operationsStarted + m_pendingDeviceUpdates.size();
        if (knownTotal > 1 && m_catalogProgressManager)
            m_catalogProgressManager->setBatchContext(m_operationsStarted, knownTotal);
    });
}
//----------------------------------------------------------------------
QVariantMap AppManager::buildUpdateReport(int deviceId, const QList<qint64> &results)
{
    if (results.isEmpty() || results[0] != 1)
        return {};

    const QString conn = m_connectionName;
    Device dev;
    dev.ID = deviceId;
    dev.loadDevice(conn);

    QVariantMap r;
    r[QStringLiteral("deviceName")] = dev.name;
    r[QStringLiteral("deviceType")] = dev.type;
    r[QStringLiteral("devicePath")] = dev.path;

    if (dev.type == QStringLiteral("Catalog")) {
        if (results.size() > 4) {
            r[QStringLiteral("fileCount")]  = results[1];
            r[QStringLiteral("filesAdded")] = results[2];
            r[QStringLiteral("totalSize")]  = results[3];
            r[QStringLiteral("sizeAdded")]  = results[4];
        }
        bool storageUpdated = (results.size() > 7 && results[7] == 1);
        r[QStringLiteral("storageUpdated")] = storageUpdated;
        if (storageUpdated && results.size() > 13) {
            Device parent;
            parent.ID = dev.parentID;
            parent.loadDevice(conn);
            r[QStringLiteral("storageName")]       = parent.name;
            r[QStringLiteral("storagePath")]       = parent.path;
            r[QStringLiteral("storageUsed")]       = results[8];
            r[QStringLiteral("storageUsedAdded")]  = results[9];
            r[QStringLiteral("storageFree")]       = results[10];
            r[QStringLiteral("storageFreeAdded")]  = results[11];
            r[QStringLiteral("storageTotal")]      = results[12];
            r[QStringLiteral("storageTotalAdded")] = results[13];
        }
    } else if (dev.type == QStringLiteral("Storage")) {
        bool storageUpdated = (results.size() > 7 && results[7] == 1);
        r[QStringLiteral("storageUpdated")] = storageUpdated;
        if (results.size() > 13) {
            r[QStringLiteral("storageUsed")]       = results[8];
            r[QStringLiteral("storageUsedAdded")]  = results[9];
            r[QStringLiteral("storageFree")]       = results[10];
            r[QStringLiteral("storageFreeAdded")]  = results[11];
            r[QStringLiteral("storageTotal")]      = results[12];
            r[QStringLiteral("storageTotalAdded")] = results[13];
        }
    } else if (dev.type == QStringLiteral("Virtual")) {
        if (results.size() > 4) {
            r[QStringLiteral("fileCount")]       = results[1];
            r[QStringLiteral("filesAdded")]      = results[2];
            r[QStringLiteral("totalSize")]       = results[3];
            r[QStringLiteral("sizeAdded")]       = results[4];
        }
        if (results.size() > 6) {
            r[QStringLiteral("catalogsUpdated")] = results[5];
            r[QStringLiteral("catalogsSkipped")] = results[6];
        }
        bool storageUpdated = (results.size() > 7 && results[7] == 1);
        r[QStringLiteral("storageUpdated")] = storageUpdated;
        if (storageUpdated && results.size() > 13) {
            r[QStringLiteral("storageUsed")]       = results[8];
            r[QStringLiteral("storageUsedAdded")]  = results[9];
            r[QStringLiteral("storageFree")]       = results[10];
            r[QStringLiteral("storageFreeAdded")]  = results[11];
            r[QStringLiteral("storageTotal")]      = results[12];
            r[QStringLiteral("storageTotalAdded")] = results[13];
        }
    }
    return r;
}
//----------------------------------------------------------------------
QVariantMap AppManager::buildBatchUpdateReport(const QList<qint64> &results)
{
    if (results.isEmpty() || results[0] != 1)
        return {};

    QVariantMap r;
    r[QStringLiteral("deviceType")] = QStringLiteral("list");
    if (results.size() > 4) {
        r[QStringLiteral("fileCount")]       = results[1];
        r[QStringLiteral("filesAdded")]      = results[2];
        r[QStringLiteral("totalSize")]       = results[3];
        r[QStringLiteral("sizeAdded")]       = results[4];
    }
    if (results.size() > 6) {
        r[QStringLiteral("catalogsUpdated")] = results[5];
        r[QStringLiteral("catalogsSkipped")] = results[6];
    }
    bool storageUpdated = (results.size() > 7 && results[7] == 1);
    r[QStringLiteral("storageUpdated")] = storageUpdated;
    if (storageUpdated && results.size() > 13) {
        r[QStringLiteral("storageUsed")]       = results[8];
        r[QStringLiteral("storageUsedAdded")]  = results[9];
        r[QStringLiteral("storageFree")]       = results[10];
        r[QStringLiteral("storageFreeAdded")]  = results[11];
        r[QStringLiteral("storageTotal")]      = results[12];
        r[QStringLiteral("storageTotalAdded")] = results[13];
    }
    return r;
}
//----------------------------------------------------------------------
void AppManager::updateDevice(int deviceId)
{
    Device dev;
    dev.ID = deviceId;
    dev.loadDevice(m_connectionName);

    // Already busy: queue it instead of dropping the request silently.
    if (m_deviceUpdateIsRunning || (m_deviceUpdateManager && m_deviceUpdateManager->operationRunning())) {
        enqueueOperation(deviceId, false, dev.name);
        return;
    }

    m_isBatchUpdate          = false;
    m_showEachUpdateReport   = true;  // single update always reports
    enqueueOperation(deviceId, false, dev.name);
    startNextDeviceUpdate();
}
//----------------------------------------------------------------------
void AppManager::updateAllActiveDevices(bool showEachReport)
{
    const QString conn = m_connectionName;

    // Scope: catalogs under the currently selected device (same as what the Catalogs
    // list view displays). 0 = no selection = all catalogs in the collection.
    int scopeId = selectedDevice ? selectedDevice->ID : 0;
    QList<Device *> activeCatalogs = Device::getActiveCatalogList(conn, scopeId);

    if (activeCatalogs.isEmpty())
        return;

    if (m_deviceUpdateIsRunning
        || (m_deviceUpdateManager && m_deviceUpdateManager->operationRunning())) {
        // Busy: append every active catalog as its own entry rather than drop the
        // request (OPQ-F1, OPQ-F4). The aggregate form cannot be queued — it runs
        // the whole set as a single hierarchy — so work queued this way always
        // reports per catalog. m_showEachUpdateReport is deliberately left alone:
        // changing it now would change how the running operation reports itself.
        for (Device *d : activeCatalogs)
            enqueueOperation(d->ID, false, d->name);
        qDeleteAll(activeCatalogs);
        return;
    }

    m_showEachUpdateReport  = showEachReport;
    m_currentUpdateDeviceId = -1;

    if (showEachReport) {
        // Process one catalog at a time so each report can pause for acknowledgement.
        // Appends: anything already waiting keeps its place (OPQ-F4).
        m_isBatchUpdate = false;
        for (Device *d : activeCatalogs)
            enqueueOperation(d->ID, false, d->name);
        qDeleteAll(activeCatalogs);

        startNextDeviceUpdate();
    } else {
        // Run all as a single hierarchy; show one aggregate report at the end
        m_isBatchUpdate         = true;
        m_deviceUpdateIsRunning = true;
        emit deviceUpdateStateChanged();

        setupDeviceUpdateManagerForDevices();

        Device *dummy = m_deviceUpdateManager->createDummyDeviceFromList(activeCatalogs);
        // Deferred one event-loop turn so the activity panel paints before the
        // scan blocks the GUI thread — same reason as in createCatalog().
        QTimer::singleShot(kStartDeferralMs, this, [this, dummy]() {
            m_deviceUpdateManager->updateDeviceHierarchy(dummy, collection->databaseMode, collection->folder, "update");
        });
        // activeCatalogs intentionally not deleted here — createDummyDeviceFromList copies Device
        // values but the copies share raw catalog*/storage* pointers with the originals.
        // Deleting the originals would create dangling pointers in dummy->subDevices (same pattern as K2).
    }
}
//----------------------------------------------------------------------
void AppManager::stopDeviceUpdate()
{
    m_waitingForReportAck = false;
    if (m_deviceUpdateManager && m_deviceUpdateManager->operationRunning()) {
        // The queue is left intact: the cancellation handler starts the next
        // entry. Use Clear to empty the queue.
        m_deviceUpdateManager->requestHardStop();
    }
}
//----------------------------------------------------------------------
void AppManager::acknowledgeUpdateReport()
{
    if (!m_waitingForReportAck) return;
    m_waitingForReportAck = false;
    scheduleNextOperation();
}
//----------------------------------------------------------------------
void AppManager::gentleStopDeviceUpdate()
{
    if (m_deviceUpdateManager && m_deviceUpdateManager->operationRunning())
        m_deviceUpdateManager->requestGentleStop();
}
//----------------------------------------------------------------------
QString AppManager::assignCatalogToDevice(int catalogDeviceId, int virtualDeviceId)
{
    const QString conn = m_connectionName;

    Device catalogDev;
    catalogDev.ID = catalogDeviceId;
    catalogDev.loadDevice(conn);
    if (catalogDev.type != QStringLiteral("Catalog"))
        return tr("The selected device is not a catalog.");

    Device virtualDev;
    virtualDev.ID = virtualDeviceId;
    virtualDev.loadDevice(conn);

    if (Device::isCatalogAssigned(catalogDev.externalID, virtualDeviceId, conn))
        return tr("The catalog is already assigned to this Virtual device.");

    if (!Device::assignCatalogToDevice(&catalogDev, &virtualDev, conn))
        return tr("Failed to assign catalog to device.");

    collection->saveDeviceTableToFile();
    emit deviceListChanged();
    return QString();
}
//----------------------------------------------------------------------
void AppManager::launchFilelight(int deviceId)
{
    Device dev;
    dev.ID = deviceId;
    dev.loadDevice(m_connectionName);
    QString path = (dev.type == QStringLiteral("Catalog") && dev.catalog)
                   ? dev.catalog->sourcePath : dev.path;
    if (!path.isEmpty())
        QProcess::startDetached(QStringLiteral("filelight"), {path});
}
//----------------------------------------------------------------------
QVariantMap AppManager::recordDevicesSnapshot()
{
    const QString conn = m_connectionName;

    // In Memory mode the statistics_device table is empty until loaded from
    // file; without this the "previous" totals below read an empty table and
    // every delta equals the full total. No-op in File/Hosted mode. Mirrors
    // K2's recordAllDeviceStats().
    collection->loadStatisticsDeviceFileToTable();

    auto sumQuery = [&](const QString &type) -> QList<qint64> {
        QSqlQuery q(QSqlDatabase::database(conn));
        q.prepare(QStringLiteral(
            "SELECT SUM(device_file_count),SUM(device_total_file_size),"
            "       SUM(device_free_space),SUM(device_total_space)"
            " FROM statistics_device"
            " WHERE date_time=(SELECT MAX(date_time) FROM statistics_device WHERE record_type='snapshot')"
            "   AND device_type=:t GROUP BY date_time"));
        q.bindValue(":t", type);
        q.exec(); q.next();
        return {q.value(0).toLongLong(), q.value(1).toLongLong(),
                q.value(2).toLongLong(), q.value(3).toLongLong()};
    };

    auto prevCat = sumQuery("Catalog");
    auto prevSto = sumQuery("Storage");

    // Record current values
    QDateTime now = QDateTime::currentDateTime();
    QSqlQuery all(QSqlDatabase::database(conn));
    all.exec("SELECT device_id FROM device");
    while (all.next()) {
        Device dev;
        dev.ID = all.value(0).toInt();
        dev.loadDevice(conn);
        dev.saveStatistics(now, "snapshot");
    }
    collection->saveStatiticsTableToFile();

    auto newCat = sumQuery("Catalog");
    auto newSto = sumQuery("Storage");

    QVariantMap result;
    result["newCatalogFileCount"]  = newCat.value(0, 0);
    result["newCatalogFileSize"]   = newCat.value(1, 0);
    result["deltaCatalogFileCount"]= newCat.value(0, 0) - prevCat.value(0, 0);
    result["deltaCatalogFileSize"] = newCat.value(1, 0) - prevCat.value(1, 0);
    result["newStorageFreeSpace"]  = newSto.value(2, 0);
    result["newStorageTotalSpace"] = newSto.value(3, 0);
    result["deltaStorageFree"]     = newSto.value(2, 0) - prevSto.value(2, 0);
    result["deltaStorageTotal"]    = newSto.value(3, 0) - prevSto.value(3, 0);
    return result;
}
//----------------------------------------------------------------------
QString AppManager::splitCatalogBySubDirectory(int deviceId)
{
    Device dev;
    dev.ID = deviceId;
    dev.loadDevice(m_connectionName);

    if (collection->databaseMode == "Memory")
        dev.catalog->loadFoldersToTable();

    if (!collection->executeSplitBySubDirectory(&dev))
        return tr("Split failed: no catalogs were created.");

    collection->saveDeviceTableToFile();
    emit deviceListChanged();
    refreshDeviceList();
    return QString();
}
//----------------------------------------------------------------------
void AppManager::splitCatalogByFileType(int deviceId, bool verifyFirst)
{
    Device *dev = new Device();
    dev->ID = deviceId;
    dev->loadDevice(m_connectionName);

    auto performSplit = [this, dev]() {
        if (!collection->executeSplitByFileType(dev)) {
            emit splitCompleted(false, tr("Split failed: no catalogs were created."));
        } else {
            collection->saveDeviceTableToFile();
            emit deviceListChanged();
            refreshDeviceList();
            emit splitCompleted(true, QString());
        }
        delete dev;
    };

    if (!verifyFirst) {
        performSplit();
        return;
    }

    // Verify MIME types first, then split
    setupDeviceUpdateManagerForDevices();
    m_deviceUpdateIsRunning = true;
    m_deviceUpdateStatusText = tr("Verifying MIME types for %1…").arg(dev->name);
    emit deviceUpdateStateChanged();
    emit deviceUpdateStatusChanged();

    // Disconnect generic handler, wire split-specific completion
    disconnect(m_deviceUpdateManager, &DeviceUpdateManager::operationCompleted,
               this, &AppManager::onDevicePageUpdateCompleted);
    connect(m_deviceUpdateManager, &DeviceUpdateManager::operationCompleted,
            this, [this, performSplit](const QList<qint64> &) {
                m_deviceUpdateIsRunning = false;
                m_deviceUpdateStatusText.clear();
                emit deviceUpdateStateChanged();
                emit deviceUpdateStatusChanged();
                setupDeviceUpdateManager();
                performSplit();
            });

    // Run MIME verification via DeviceUpdateManager (uses same CatalogJobStoppable internally)
    // DeviceUpdateManager handles VerifyMimeTypes through its catalog job.
    // Deferred one event-loop turn so the activity panel paints before the scan
    // blocks the GUI thread — same reason as in createCatalog().
    QTimer::singleShot(kStartDeferralMs, this, [this, dev]() {
        m_deviceUpdateManager->updateDeviceHierarchy(dev, collection->databaseMode, collection->folder, "update");
    });
}
//----------------------------------------------------------------------
QVariantMap AppManager::verifyDeviceChecksums(int deviceId)
{
    Device dev;
    dev.ID = deviceId;
    dev.loadDevice(m_connectionName);

    if (!dev.catalog) return QVariantMap{{ "error", tr("Device has no catalog.") }};

    if (collection->databaseMode == "Memory") {
        bool stop = false;
        QMutex mutex;
        dev.catalog->loadCatalogFileListToTable(mutex, stop);
    }

    int total = FileChecksum::countFilesWithChecksum(m_connectionName, dev.externalID);
    if (total == 0)
        return QVariantMap{{ "noChecksums", true }, { "verified", 0 }, { "mismatches", 0 }, { "missing", 0 }};

    bool continueFlag = true;
    auto shouldContinue = [&]() { return continueFlag; };
    auto progressCb = [this](int current, int total, const QString &) {
        m_deviceUpdateStatusText = tr("Verifying checksums: %1 / %2").arg(current).arg(total);
        emit deviceUpdateStatusChanged();
        QCoreApplication::processEvents();
    };

    auto res = FileChecksum::verifyCatalogChecksums(
        m_connectionName,
        dev.externalID, dev.catalog->sourcePath,
        shouldContinue, progressCb);

    m_deviceUpdateStatusText.clear();
    emit deviceUpdateStatusChanged();

    QVariantMap result;
    result["noChecksums"] = false;
    result["total"]       = total;
    result["verified"]    = res.verified;
    result["mismatches"]  = res.mismatches;
    result["missing"]     = res.missing;
    QStringList mm; for (const auto &f : res.mismatchedFiles) mm << f;
    QStringList ms; for (const auto &f : res.missingFiles)    ms << f;
    result["mismatchedFiles"] = mm;
    result["missingFiles"]    = ms;
    return result;
}
//----------------------------------------------------------------------
QString AppManager::unassignDevice(int deviceId, int parentId)
{
    const QString conn = m_connectionName;
    if (!Device::unassignFromDevice(deviceId, parentId, conn))
        return tr("Failed to unassign device.");
    if (collection->databaseMode == "Memory")
        collection->saveDeviceTableToFile();
    emit deviceListChanged();
    refreshDeviceList();
    return QString();
}
//----------------------------------------------------------------------
QString AppManager::importFromVVV(const QString &path)
{
    const QString conn = m_connectionName;
    QFile sourceFile(path);

    if (!sourceFile.open(QIODevice::ReadOnly))
        return tr("Could not open file.");

    QTextStream ts(&sourceFile);
    QString line = ts.readLine();
    if (!line.startsWith("Volume")) {
        sourceFile.close();
        return tr("File format not recognized (expected VVV CSV).");
    }

    QString dt = "_" + QDateTime::currentDateTime().toString("yy-MM-dd hh-mm-ss");
    QSet<QString> uniqueNames;
    while (!ts.atEnd()) {
        line = ts.readLine();
        if (line.isEmpty()) continue;
        QStringList f = line.split('\t');
        if (f.count() == 7) {
            QString n = QString(f[0]).remove('"').replace('/', '_') + dt;
            uniqueNames.insert(n);
        }
    }
    sourceFile.close();

    if (uniqueNames.isEmpty())
        return tr("No catalog data found in file.");

    // Create virtual device to host imported catalogs
    Device importVirtual;
    importVirtual.generateDeviceID();
    importVirtual.type     = "Virtual";
    importVirtual.name     = "imports from VVV" + dt;
    importVirtual.parentID = 0;
    importVirtual.groupID  = 1;
    importVirtual.path     = "/import";
    importVirtual.dateTimeUpdated = QDateTime::currentDateTime();
    importVirtual.insertDevice();
    collection->saveDeviceTableToFile();

    QMap<QString, qint64>   nameToId;
    QMap<QString, Device*>  nameToDev;

    for (const QString &catName : uniqueNames) {
        Device *d = new Device();
        d->generateDeviceID();
        d->name     = catName;
        d->type     = "Catalog";
        d->parentID = importVirtual.ID;
        d->groupID  = 1;
        d->path     = "/import";
        d->catalog->generateID();
        d->externalID = d->catalog->ID;
        d->dateTimeUpdated = QDateTime::currentDateTime();
        d->insertDevice();
        d->catalog->name        = d->name;
        d->catalog->filePath    = collection->folder + "/" + d->name + ".idx";
        d->catalog->sourcePath  = "/import";
        d->catalog->includeHidden     = 1;
        d->catalog->includeSymblinks  = 0;
        d->catalog->isFullDevice      = 0;
        d->catalog->includeMetadata   = Catalog::METADATA_NONE;
        d->catalog->appVersion        = currentVersion;
        d->catalog->insertCatalog();
        nameToId[catName]  = d->catalog->ID;
        nameToDev[catName] = d;
    }

    QSqlQuery delQ(QSqlDatabase::database(conn));
    delQ.exec("DELETE FROM file");
    delQ.exec("DELETE FROM folder");

    QSqlQuery insFile(QSqlDatabase::database(conn));
    insFile.prepare("INSERT INTO file(file_catalog_id,file_name,file_folder_path,file_size,file_date_updated,file_catalog)"
                    " VALUES(:cid,:name,:path,:size,:date,:cat)");

    QSqlQuery insFolder(QSqlDatabase::database(conn));
    insFolder.prepare("INSERT OR IGNORE INTO folder(folder_catalog_id,folder_path) VALUES(:cid,:path)");

    if (!sourceFile.open(QIODevice::ReadOnly))
        return tr("Could not open file: %1").arg(path);
    ts.setDevice(&sourceFile);
    ts.readLine(); // skip header
    while (!ts.atEnd()) {
        line = ts.readLine();
        if (line.isEmpty()) continue;
        QStringList f = line.split('\t');
        if (f.count() != 7) continue;
        QString catName  = QString(f[0]).remove('"').replace('/', '_') + dt;
        qint64 catId     = nameToId.value(catName, 0);
        QString folder   = "/import" + QString(f[1]).remove('"');
        insFile.bindValue(":cid",  catId);
        insFile.bindValue(":name", QString(f[2]).remove('"'));
        insFile.bindValue(":path", folder);
        insFile.bindValue(":size", f[3].toLongLong());
        insFile.bindValue(":date", f[5]);
        insFile.bindValue(":cat",  catName);
        insFile.exec();
        insFolder.bindValue(":cid",  catId);
        insFolder.bindValue(":path", folder);
        insFolder.exec();
    }
    sourceFile.close();

    // Root folders
    for (auto it = nameToId.begin(); it != nameToId.end(); ++it) {
        insFolder.bindValue(":cid",  it.value());
        insFolder.bindValue(":path", "/import");
        insFolder.exec();
    }

    // Export catalog files and update stats
    for (auto it = nameToDev.begin(); it != nameToDev.end(); ++it) {
        Device *d = it.value();
        QSqlQuery stats(QSqlDatabase::database(conn));
        stats.prepare("SELECT COUNT(*),SUM(file_size) FROM file WHERE file_catalog_id=:id");
        stats.bindValue(":id", d->externalID);
        stats.exec(); stats.next();
        d->totalFileCount = stats.value(0).toLongLong();
        d->totalFileSize  = stats.value(1).toLongLong();
        d->saveDevice();

        QFile out(d->catalog->filePath);
        if (out.open(QIODevice::WriteOnly | QIODevice::Truncate)) {
            QTextStream s(&out);
            s << "<catalogSourcePath>/import\n"
              << "<catalogFileCount>" << d->totalFileCount << "\n"
              << "<catalogTotalFileSize>" << d->totalFileSize << "\n"
              << "<catalogIncludeHidden>\n<catalogFileType>\n<catalogStorage>\n"
              << "<catalogIncludeSymblinks>\n<catalogIsFullDevice>\n<catalogIncludeMetadata>\n"
              << "<catalogAppVersion>" << currentVersion << "\n"
              << "<catalogID>" << d->externalID << "\n";
            QSqlQuery files(QSqlDatabase::database(conn));
            files.prepare("SELECT file_folder_path,file_name,file_size,file_date_updated FROM file WHERE file_catalog_id=:id");
            files.bindValue(":id", d->externalID);
            files.exec();
            while (files.next())
                s << files.value(0).toString() << "/" << files.value(1).toString()
                  << "\t" << files.value(2).toString() << "\t" << files.value(3).toString() << "\n";
            out.close();
        }

        QFile fout(collection->folder + "/" + d->name + ".folders.idx");
        if (fout.open(QIODevice::WriteOnly | QIODevice::Truncate)) {
            QTextStream s(&fout);
            QSqlQuery folders(QSqlDatabase::database(conn));
            folders.prepare("SELECT folder_catalog_id,folder_path FROM folder WHERE folder_catalog_id=:id");
            folders.bindValue(":id", d->externalID);
            folders.exec();
            while (folders.next())
                s << folders.value(0).toString() << "\t" << folders.value(1).toString() << "\n";
            fout.close();
        }
    }

    qDeleteAll(nameToDev);
    importVirtual.updateNumbersFromChildren();
    collection->saveDeviceTableToFile();
    if (collection->databaseMode == "Memory")
        collection->loadCatalogFilesToTable();

    refreshAllUI();
    return QString();
}
//----------------------------------------------------------------------

// Device edit
//----------------------------------------------------------------------
QVariantMap AppManager::getDeviceDetails(int deviceId) const
{
    const QString conn = m_connectionName;
    Device dev;
    dev.ID = deviceId;
    dev.loadDevice(conn);

    QVariantMap r;
    r["type"]     = dev.type;
    r["name"]     = dev.name;
    r["parentId"] = dev.parentID;
    r["path"]     = dev.path;
    r["deviceId"] = dev.ID;
    r["groupId"]  = dev.groupID;
    r["comment"]  = dev.comment;

    if (dev.type == "Catalog") {
        r["fileType"]        = dev.catalog->fileType;
        r["includeSubDir"]   = dev.catalog->includeSubDir;
        r["includeHidden"]   = dev.catalog->includeHidden;
        r["includeMetadata"] = dev.catalog->includeMetadata;
        r["includeChecksum"] = dev.catalog->includeChecksum;
        r["isFullDevice"]    = dev.catalog->isFullDevice;
        r["excludeFolders"]  = dev.catalog->getExcludeFolders();
    } else if (dev.type == "Storage") {
        r["storageExtId"]         = dev.externalID;
        r["storageType"]          = dev.storage->type;
        r["storageLabel"]         = dev.storage->label;
        r["storageFileSystem"]    = dev.storage->fileSystem;
        r["totalSpace"]           = (qlonglong)dev.totalSpace;
        r["freeSpace"]            = (qlonglong)dev.freeSpace;
        r["storageBrand"]         = dev.storage->brand;
        r["storageModel"]         = dev.storage->model;
        r["storageSerialNumber"]  = dev.storage->serialNumber;
        r["storageBuildDate"]     = dev.storage->buildDate;
        r["storageComment1"]      = dev.storage->comment1;
        r["storageComment2"]      = dev.storage->comment2;
        r["storageComment3"]      = dev.storage->comment3;
        r["storagePicturePath"]   = dev.storage->picturePath;
    }
    return r;
}
//----------------------------------------------------------------------
QString AppManager::saveDeviceBasicFields(int deviceId, const QString &name, int parentId, const QString &path, const QString &comment)
{
    const QString conn = m_connectionName;
    Device dev;
    dev.ID = deviceId;
    dev.loadDevice(conn);

    // Remember the parent the device had before this edit, so a re-parent can
    // refresh the chain it was moved away from (see end of method).
    const int previousParentId = dev.parentID;

    // Device id 1 is the reserved " Physical Group" root: its parent is fixed at
    // root (which keeps it in the Physical group, id 0). Force it here so no UI
    // path — including a stale parent picker — can ever re-parent it.
    if (deviceId == 1)
        parentId = 0;

    // Name uniqueness for Catalog
    if (dev.type == "Catalog" && name != dev.name) {
        Device check;
        check.name = name;
        if (check.verifyDeviceNameExists())
            return tr("There is already a catalog with this name: %1\nChoose a different name.").arg(name);
    }

    dev.name = name;
    // loadDevice() above populated the comment, so the full-row UPDATE in
    // saveDevice() cannot blank it (SpecDeviceComment.md DCM-C2).
    dev.comment = comment;
    dev.path = Catalog::normalizeSourcePath(path);
    dev.catalog->sourcePath = dev.path;

    // Parent + groupId cascade
    Device newParent;
    newParent.ID = parentId;
    if (parentId > 0)
        newParent.loadDevice(conn);

    if (dev.type == "Catalog" && dev.groupID == 0 && parentId > 0
        && newParent.type != "Storage") {
        return tr("A Catalog in the Physical group can only be set under a Storage device.");
    }

    // At root (parentId 0) keep the device's current group. K2 forced group 1
    // here ("root == Virtual"), but the seeded " Physical Group" (device id 1)
    // is a group-0 device that lives at root — forcing 1 would cascade-flip the
    // whole physical hierarchy into the Virtual group. Combined with the
    // same-group parent filter, the group can now never change on edit.
    int newGroupId = (parentId == 0) ? dev.groupID : newParent.groupID;
    if (dev.groupID != newGroupId) {
        dev.loadDevice(conn); // reload to get deviceIDList
        Device sub;
        for (int id : dev.deviceIDList) {
            sub.ID = id;
            sub.loadDevice(conn);
            sub.groupID = newGroupId;
            sub.saveDevice();
        }
    }

    dev.parentID = parentId;
    dev.groupID  = newGroupId;
    dev.saveDevice();

    // On a re-parent, recompute the file count / size / space of the parent the
    // device left and the parent it joined, then propagate up to their Virtual
    // ancestors — otherwise Storage and upper Virtual devices keep stale totals.
    // Mirrors K2 saveDeviceForm, which updates both the previous and new parent
    // chains (updateNumbersFromChildren + updateParentsNumbers on each).
    if (previousParentId != parentId) {
        if (previousParentId > 0) {
            Device oldParent;
            oldParent.ID = previousParentId;
            oldParent.loadDevice(conn);
            oldParent.updateNumbersFromChildren();
            oldParent.updateParentsNumbers();
        }
        if (parentId > 0) {
            Device newParentChain;
            newParentChain.ID = parentId;
            newParentChain.loadDevice(conn);
            newParentChain.updateNumbersFromChildren();
            newParentChain.updateParentsNumbers();
        }
    }

    collection->saveDeviceTableToFile();
    return {};
}
//----------------------------------------------------------------------
QVariantMap AppManager::checkCatalogOptionChanges(int deviceId, const QString &fileType, bool includeSubDir,
                                                   bool includeHidden,
                                                   const QString &includeMetadata, const QString &includeChecksum,
                                                   bool isFullDevice) const
{
    const QString conn = m_connectionName;
    Device prev;
    prev.ID = deviceId;
    prev.loadDevice(conn);

    QStringList changedFields;
    bool rescanNeeded = false;

    if (fileType != prev.catalog->fileType) {
        changedFields << tr("File type: %1 → %2").arg(prev.catalog->fileType, fileType);
        rescanNeeded = true;
    }
    if (includeSubDir != prev.catalog->includeSubDir) {
        changedFields << tr("Include subdirectories: %1 → %2")
                             .arg(prev.catalog->includeSubDir ? tr("yes") : tr("no"),
                                  includeSubDir               ? tr("yes") : tr("no"));
        rescanNeeded = true;
    }
    if (includeHidden != prev.catalog->includeHidden) {
        changedFields << tr("Include hidden: %1 → %2")
                             .arg(prev.catalog->includeHidden ? tr("All") : tr("None"),
                                  includeHidden              ? tr("All") : tr("None"));
        rescanNeeded = true;
    }
    if (includeMetadata != prev.catalog->includeMetadata) {
        changedFields << tr("Include metadata: %1 → %2").arg(prev.catalog->includeMetadata, includeMetadata);
        rescanNeeded = true;
    }
    if (includeChecksum != prev.catalog->includeChecksum) {
        changedFields << tr("Include checksum: %1 → %2").arg(prev.catalog->includeChecksum, includeChecksum);
        bool toNone = (prev.catalog->includeChecksum != Catalog::CHECKSUM_NONE
                       && includeChecksum == Catalog::CHECKSUM_NONE);
        if (!toNone) rescanNeeded = true;
    }
    if (isFullDevice != prev.catalog->isFullDevice) {
        changedFields << tr("Is full device: %1 → %2")
                             .arg(prev.catalog->isFullDevice ? tr("yes") : tr("no"),
                                  isFullDevice               ? tr("yes") : tr("no"));
        rescanNeeded = true;
    }

    QVariantMap result;
    result["needsConfirmation"] = !changedFields.isEmpty();
    result["message"]           = changedFields.join('\n');
    result["rescanNeeded"]      = rescanNeeded;
    return result;
}
//----------------------------------------------------------------------
QString AppManager::saveCatalogOptions(int deviceId, const QString &fileType, bool includeSubDir,
                                        bool includeHidden,
                                        const QString &includeMetadata, const QString &includeChecksum,
                                        bool isFullDevice)
{
    const QString conn = m_connectionName;
    Device dev;
    dev.ID = deviceId;
    dev.loadDevice(conn);

    const QString prevMetadata = dev.catalog->includeMetadata;

    dev.catalog->fileType        = fileType;
    dev.catalog->includeSubDir   = includeSubDir;
    dev.catalog->includeHidden   = includeHidden;
    dev.catalog->includeMetadata = includeMetadata;
    dev.catalog->includeChecksum = includeChecksum;
    dev.catalog->isFullDevice    = isFullDevice;

    dev.catalog->saveCatalog();
    dev.catalog->updateCatalogFileHeaders(collection->databaseMode);
    dev.catalog->renameCatalogFile(dev.name);

    if (prevMetadata != includeMetadata)
        dev.catalog->handleMetadataTransition(prevMetadata, includeMetadata);

    return {};
}
//----------------------------------------------------------------------
QString AppManager::saveStorageDetails(int deviceId, const QVariantMap &fields)
{
    const QString conn = m_connectionName;
    Device dev;
    dev.ID = deviceId;
    dev.loadDevice(conn);

    // Check Storage external ID uniqueness if changed
    int newExtId = fields.value("storageExtId", dev.externalID).toInt();
    if (newExtId != dev.externalID) {
        Device check;
        check.externalID = newExtId;
        if (check.verifyStorageExternalIDExists())
            return tr("There is already a Storage with this ID. Choose a different ID.");
        dev.externalID = newExtId;
    }

    dev.totalSpace = fields.value("totalSpace", (qlonglong)dev.totalSpace).toLongLong();
    dev.freeSpace  = fields.value("freeSpace",  (qlonglong)dev.freeSpace).toLongLong();
    dev.saveDevice();

    dev.storage->type         = fields.value("storageType",         dev.storage->type).toString();
    dev.storage->label        = fields.value("storageLabel",        dev.storage->label).toString();
    dev.storage->fileSystem   = fields.value("storageFileSystem",   dev.storage->fileSystem).toString();
    dev.storage->brand        = fields.value("storageBrand",        dev.storage->brand).toString();
    dev.storage->model        = fields.value("storageModel",        dev.storage->model).toString();
    dev.storage->serialNumber = fields.value("storageSerialNumber", dev.storage->serialNumber).toString();
    dev.storage->buildDate    = fields.value("storageBuildDate",    dev.storage->buildDate).toString();
    dev.storage->comment1     = fields.value("storageComment1",     dev.storage->comment1).toString();
    dev.storage->comment2     = fields.value("storageComment2",     dev.storage->comment2).toString();
    dev.storage->comment3     = fields.value("storageComment3",     dev.storage->comment3).toString();
    dev.storage->picturePath  = fields.value("storagePicturePath",  dev.storage->picturePath).toString();

    // Persist via SQL (mirrors K2 saveDeviceForm Storage branch)
    QSqlQuery q(QSqlDatabase::database(conn));
    q.prepare(QLatin1String(R"(
        UPDATE storage
        SET storage_id           = :storage_id,
            storage_type         = :type,
            storage_label        = :label,
            storage_file_system  = :fs,
            storage_total_space  = :total,
            storage_free_space   = :free,
            storage_brand        = :brand,
            storage_model        = :model,
            storage_serial_number= :serial,
            storage_build_date   = :build,
            storage_comment1     = :c1,
            storage_comment2     = :c2,
            storage_comment3     = :c3,
            storage_picture_path = :pic
        WHERE storage_id = :old_id
    )"));
    q.bindValue(":storage_id", newExtId);
    q.bindValue(":type",   dev.storage->type);
    q.bindValue(":label",  dev.storage->label);
    q.bindValue(":fs",     dev.storage->fileSystem);
    q.bindValue(":total",  dev.totalSpace);
    q.bindValue(":free",   dev.freeSpace);
    q.bindValue(":brand",  dev.storage->brand);
    q.bindValue(":model",  dev.storage->model);
    q.bindValue(":serial", dev.storage->serialNumber);
    q.bindValue(":build",  dev.storage->buildDate);
    q.bindValue(":c1",     dev.storage->comment1);
    q.bindValue(":c2",     dev.storage->comment2);
    q.bindValue(":c3",     dev.storage->comment3);
    q.bindValue(":pic",    dev.storage->picturePath);
    q.bindValue(":old_id", dev.externalID);
    if (!q.exec())
        return q.lastError().text();

    collection->saveStorageTableToFile();
    return {};
}
//----------------------------------------------------------------------
void AppManager::triggerDeviceRescan(int deviceId)
{
    Device *dev = new Device();
    dev->ID = deviceId;
    dev->loadDevice(m_connectionName);
    if (!m_deviceUpdateManager)
        setupDeviceUpdateManager();
    m_deviceUpdateManager->updateDeviceHierarchy(dev, collection->databaseMode, collection->folder, "update");
}
//----------------------------------------------------------------------
void AppManager::triggerStoragePathReplace(int deviceId, const QString &previousPath, const QString &newPath)
{
    Device *dev = new Device();
    dev->ID = deviceId;
    dev->loadDevice(m_connectionName);
    if (!m_deviceUpdateManager)
        setupDeviceUpdateManager();
    m_deviceUpdateManager->replaceStorageRoot(dev, previousPath, newPath,
                                              collection->databaseMode, collection->folder);
}
//----------------------------------------------------------------------
QStringList AppManager::getDeviceExcludeFolders(int deviceId) const
{
    Device dev;
    dev.ID = deviceId;
    dev.loadDevice(m_connectionName);
    if (dev.type != "Catalog") return {};
    return dev.catalog->getExcludeFolders();
}
//----------------------------------------------------------------------
bool AppManager::addDeviceExcludeFolder(int deviceId, const QString &path)
{
    Device dev;
    dev.ID = deviceId;
    dev.loadDevice(m_connectionName);
    if (dev.type != "Catalog") return false;
    bool ok = dev.catalog->addExcludeFolder(path.trimmed());
    if (ok) collection->saveCatalogFilterTableToFile();
    return ok;
}
//----------------------------------------------------------------------
bool AppManager::removeDeviceExcludeFolder(int deviceId, const QString &path)
{
    Device dev;
    dev.ID = deviceId;
    dev.loadDevice(m_connectionName);
    if (dev.type != "Catalog") return false;
    bool ok = dev.catalog->removeExcludeFolder(path);
    if (ok) collection->saveCatalogFilterTableToFile();
    return ok;
}

// Explore page
//----------------------------------------------------------------------
QVariantMap AppManager::exploreOpenCatalog(int deviceId)
{
    QVariantMap result;
    const QString conn = m_connectionName;

    exploreDevice->catalog->setConnectionName(conn);
    exploreDevice->setConnectionName(conn);
    exploreDevice->ID = deviceId;
    exploreDevice->loadDevice(conn);

    if (exploreDevice->type != QLatin1String("Catalog")) {
        result[QStringLiteral("success")] = false;
        result[QStringLiteral("message")] = tr("Selected device is not a catalog");
        return result;
    }

    if (collection->databaseMode == QLatin1String("Memory")) {
        exploreDevice->catalog->loadFoldersToTable();
        bool stopRequested = false;
        QMutex mutex;
        exploreDevice->catalog->loadCatalogFileListToTable(mutex, stopRequested);
    }

    int folderCount = Catalog::getExploreFolderCount(conn, exploreDevice->externalID);

    result[QStringLiteral("success")]    = true;
    result[QStringLiteral("name")]       = exploreDevice->name;
    result[QStringLiteral("path")]       = exploreDevice->path;
    result[QStringLiteral("externalId")] = exploreDevice->externalID;
    result[QStringLiteral("folderCount")] = folderCount;
    return result;
}
//----------------------------------------------------------------------
QVariantList AppManager::getExploreFolders()
{
    QVariantList result;
    if (exploreDevice->externalID == 0) return result;

    const QString conn = m_connectionName;
    const QString catalogPath = exploreDevice->path;
    int pos = catalogPath.lastIndexOf(QLatin1Char('/'));
    const QString pathRoot = (pos >= 0) ? catalogPath.left(pos) : QString();

    QList<FolderNode*> nodes = FolderTreeLoader::loadExploreTree(conn, exploreDevice->externalID, pathRoot);

    std::function<void(const QList<FolderNode*>&, int)> flatten;
    flatten = [&](const QList<FolderNode*> &list, int level) {
        for (FolderNode *node : list) {
            QVariantMap item;
            item[QStringLiteral("name")]      = node->name;
            item[QStringLiteral("fullPath")]  = node->fullPath;
            item[QStringLiteral("level")]     = level;
            item[QStringLiteral("fileCount")] = node->fileCount;
            result.append(item);
            if (!node->children.isEmpty())
                flatten(node->children, level + 1);
        }
    };
    flatten(nodes, 0);
    qDeleteAll(nodes);

    return result;
}
//----------------------------------------------------------------------
QVariantList AppManager::getExploreEntries(const QString &folderPath, bool showFolders, bool showSubFolders)
{
    QVariantList result;
    if (exploreDevice->externalID == 0) return result;

    const QString conn = m_connectionName;
    const QList<Catalog::ExploreFileEntry> entries = Catalog::getExploreEntries(
        conn, exploreDevice->externalID, folderPath, showFolders, showSubFolders);

    for (const Catalog::ExploreFileEntry &e : entries) {
        QVariantMap item;
        item[QStringLiteral("name")]                 = e.name;
        item[QStringLiteral("size")]                 = e.size;
        item[QStringLiteral("dateUpdated")]          = e.dateUpdated;
        item[QStringLiteral("folderPath")]           = e.folderPath;
        item[QStringLiteral("fullPath")]             = e.fullPath;
        item[QStringLiteral("entryType")]            = e.entryType;
        item[QStringLiteral("fileType")]             = e.fileType;
        item[QStringLiteral("mimeType")]             = e.mimeType;
        item[QStringLiteral("videoDurationSeconds")] = e.videoDurationSeconds;
        item[QStringLiteral("audioArtist")]          = e.audioArtist;
        item[QStringLiteral("audioAlbum")]           = e.audioAlbum;
        item[QStringLiteral("audioTitle")]           = e.audioTitle;
        item[QStringLiteral("checksumSha256")]       = e.checksumSha256;
        result.append(item);
    }
    return result;
}
//----------------------------------------------------------------------
void AppManager::loadExploreEntries(const QString &folderPath, bool showFolders, bool showSubFolders)
{
    if (!m_exploreFilesModel) return;
    if (exploreDevice->externalID == 0) {
        m_exploreFilesModel->clear();
        return;
    }
    const QList<Catalog::ExploreFileEntry> entries = Catalog::getExploreEntries(
        m_connectionName, exploreDevice->externalID, folderPath, showFolders, showSubFolders);
    m_exploreFilesModel->populate(entries);
}
//----------------------------------------------------------------------
void AppManager::exploreRemoveRow(int proxyRow)
{
    if (!m_exploreSortModel || !m_exploreFilesModel) return;
    QModelIndex proxyIdx  = m_exploreSortModel->index(proxyRow, 0);
    QModelIndex sourceIdx = m_exploreSortModel->mapToSource(proxyIdx);
    if (sourceIdx.isValid())
        m_exploreFilesModel->removeEntry(sourceIdx.row());
}
//----------------------------------------------------------------------
void AppManager::sortSearch(int column, int order)
{
    if (!m_searchSortModel) return;
    m_searchSortModel->sort(column, order);
    QSettings settings(collection->settingsFilePath, QSettings::IniFormat);
    settings.setValue("Search/lastSearchSortSection", column);
    settings.setValue("Search/lastSearchSortOrder",   order);
}
//----------------------------------------------------------------------
void AppManager::sortExplore(int column, int order)
{
    if (!m_exploreSortModel) return;
    m_exploreSortModel->sort(column, order);
    QSettings settings(collection->settingsFilePath, QSettings::IniFormat);
    settings.setValue("Explore/lastExploreSortSection", column);
    settings.setValue("Explore/lastExploreSortOrder",   order);
}
//----------------------------------------------------------------------
int AppManager::getSearchSortColumn() const
{
    return m_searchSortModel ? m_searchSortModel->sortColumn() : -1;
}
int AppManager::getSearchSortOrder() const
{
    return m_searchSortModel ? (int)m_searchSortModel->sortOrder() : 0;
}
int AppManager::getExploreSortColumn() const
{
    return m_exploreSortModel ? m_exploreSortModel->sortColumn() : -1;
}
int AppManager::getExploreSortOrder() const
{
    return m_exploreSortModel ? (int)m_exploreSortModel->sortOrder() : 0;
}
//----------------------------------------------------------------------
QString AppManager::exploreGetChecksum(const QString &fileName, const QString &folderPath)
{
    if (exploreDevice->externalID == 0) return QString();
    return exploreDevice->catalog->getFileChecksum(fileName, folderPath);
}
//----------------------------------------------------------------------
QVariantMap AppManager::getExploreFolderStats(const QString &folderPath)
{
    QVariantMap result;
    result["fileCount"] = 0;
    result["totalSize"] = 0LL;
    if (exploreDevice->externalID == 0) return result;
    const QString conn = exploreDevice->catalog->connectionName();
    Catalog::ExploreFolderStats stats = Catalog::getExploreFolderStats(conn, exploreDevice->externalID, folderPath);
    result["fileCount"] = stats.fileCount;
    result["totalSize"] = stats.totalSize;
    return result;
}
//----------------------------------------------------------------------

// Storage helpers
//----------------------------------------------------------------------
QStringList AppManager::getStoragePictureList() const
{
    QStringList result;
    result << QString();  // empty = no picture
    QDir dir(collection->imageFolderPath);
    if (dir.exists()) {
        const QStringList filters = {"*.png","*.jpg","*.jpeg","*.bmp","*.gif","*.webp"};
        result << dir.entryList(filters, QDir::Files, QDir::Name);
    }
    return result;
}
//----------------------------------------------------------------------
QString AppManager::getStorageImageFolderPath() const
{
    return collection->imageFolderPath;
}
//----------------------------------------------------------------------
QString AppManager::getImageFolderPath() const
{
    return collection->imageFolderPath;
}
//----------------------------------------------------------------------
void AppManager::setImageFolderPath(const QString &path)
{
    if (collection->imageFolderPath == path)
        return;
    collection->imageFolderPath = path;
    collection->saveImageFolderPath();
    emit imageFolderPathChanged();
}

// Collection import
//----------------------------------------------------------------------
QStringList AppManager::getImportSourcePaths() const
{
    return collection->getImportSourcePaths();
}
//----------------------------------------------------------------------
void AppManager::openImportSource(const QString &path)
{
    if (!m_importer)
        m_importer = new CollectionImporter(collection, this);

    QSqlError err = m_importer->openSource(path);
    if (err.type() != QSqlError::NoError) {
        qWarning() << "AppManager::openImportSource error:" << err.text();
        m_importStatusText = tr("Error: could not open collection");
        emit importStatusTextChanged();
        m_importSourceDeviceModel->clear();
        emit importSourceChanged();
        return;
    }

    if (!m_importer->checkSchemaCompatibility()) {
        qWarning() << "AppManager::openImportSource: schema mismatch (proceeding):" << m_importer->lastError();
    }

    m_importSourceDeviceModel->loadFromConnection(m_importer->sourceConnectionName());

    m_importStatusText = QString();
    emit importStatusTextChanged();
    emit importSourceChanged();
}
//----------------------------------------------------------------------
QString AppManager::importDevice(int srcDeviceId)
{
    if (!m_importer || !m_importer->isSourceOpen())
        return tr("No source collection is open.");

    m_importIsRunning  = true;
    m_importReportText.clear();
    emit importReportTextChanged();
    m_importStatusText = StatusBarMessageBuilder()
                             .setOperation(tr("Collection Import"))
                             .setStatus(tr("In Progress"))
                             .build();
    emit importIsRunningChanged();
    emit importStatusTextChanged();

    QMetaObject::Connection progressConn = connect(
        m_importer, &CollectionImporter::fileImportProgress,
        this, [this](int catalogIndex, int totalCatalogs, const QString &catalogName,
                     qint64 done, qint64 total) {
            m_importStatusText = StatusBarMessageBuilder()
                                     .setOperation(tr("Collection Import"))
                                     .setStatus(tr("In Progress"))
                                     .setDeviceContext(catalogIndex, totalCatalogs, catalogName)
                                     .setProcess(tr("Files"), done, total)
                                     .build();
            emit importStatusTextChanged();
            QCoreApplication::processEvents(QEventLoop::ExcludeUserInputEvents);
        },
        Qt::DirectConnection);

    bool ok;
    if (srcDeviceId == 0)
        ok = (m_importer->importAllDevices() >= 0);
    else
        ok = m_importer->importDevice(srcDeviceId);

    disconnect(progressConn);
    m_importIsRunning = false;
    emit importIsRunningChanged();

    if (!ok) {
        m_importStatusText = StatusBarMessageBuilder()
                                 .setOperation(tr("Collection Import"))
                                 .setStatus(tr("Error"))
                                 .setCurrentItem(m_importer->lastError())
                                 .build();
        emit importStatusTextChanged();
        return m_importer->lastError();
    }

    // Persist to CSV files in Memory mode
    if (collection->databaseMode == "Memory") {
        collection->saveDeviceTableToFile();
        collection->saveMappingTableToFile();
        collection->saveCatalogFilterTableToFile();
        collection->saveStorageTableToFile();
        collection->saveTagTableToFile();
        collection->saveStatiticsTableToFile();
    }

    m_importStatusText = StatusBarMessageBuilder()
                             .setOperation(tr("Collection Import"))
                             .setStatus(tr("Completed"))
                             .build();
    emit importStatusTextChanged();

    refreshAllUI();
    emit importSourceChanged();
    return QString();
}
//----------------------------------------------------------------------
QString AppManager::updateAllImportsFromSource(const QString &path)
{
    if (!m_importer)
        m_importer = new CollectionImporter(collection, this);

    m_importIsRunning  = true;
    m_importReportText.clear();
    emit importReportTextChanged();
    m_importStatusText = StatusBarMessageBuilder()
                             .setOperation(tr("Collection Update"))
                             .setStatus(tr("In Progress"))
                             .build();
    emit importIsRunningChanged();
    emit importStatusTextChanged();
    QCoreApplication::processEvents(QEventLoop::ExcludeUserInputEvents);

    bool ok = m_importer->updateAllImportsFromSource(path);

    m_importIsRunning = false;
    emit importIsRunningChanged();

    if (!ok) {
        m_importStatusText = StatusBarMessageBuilder()
                                 .setOperation(tr("Collection Update"))
                                 .setStatus(tr("Error"))
                                 .setCurrentItem(m_importer->lastError())
                                 .build();
        emit importStatusTextChanged();
        return m_importer->lastError();
    }

    // Persist to CSV files in Memory mode
    if (collection->databaseMode == "Memory") {
        collection->saveDeviceTableToFile();
        collection->saveMappingTableToFile();
        collection->saveCatalogFilterTableToFile();
        collection->saveStorageTableToFile();
        collection->saveTagTableToFile();
        collection->saveStatiticsTableToFile();
    }

    m_importStatusText = StatusBarMessageBuilder()
                             .setOperation(tr("Collection Update"))
                             .setStatus(tr("Completed"))
                             .build();
    emit importStatusTextChanged();

    // Build the change report (file-count and size deltas) from the importer's
    // aggregate stats. Reuses existing translatable labels (Catalogs / Files / Size);
    // arrows, dots, signs and numbers are non-translatable.
    const CollectionImporter::ImportUpdateStats st = m_importer->lastUpdateStats();
    if (st.catalogsUpdated > 0) {
        QLocale loc;
        const qint64 dFiles = st.filesAfter - st.filesBefore;
        const qint64 dSize  = st.sizeAfter  - st.sizeBefore;
        const QString dFilesStr = (dFiles >= 0 ? QStringLiteral("+") : QStringLiteral("-"))
                                  + loc.toString(qAbs(dFiles));
        const QString dSizeStr  = (dSize  >= 0 ? QStringLiteral("+") : QStringLiteral("-"))
                                  + loc.formattedDataSize(qAbs(dSize));
        m_importReportText =
              tr("Catalogs") + ": " + loc.toString(st.catalogsUpdated)
            + "   ·   " + tr("Files") + ": " + loc.toString(st.filesBefore)
                + " → " + loc.toString(st.filesAfter) + " (" + dFilesStr + ")"
            + "   ·   " + tr("Size")  + ": " + loc.formattedDataSize(st.sizeBefore)
                + " → " + loc.formattedDataSize(st.sizeAfter) + " (" + dSizeStr + ")";
        emit importReportTextChanged();
    }

    refreshAllUI();
    return QString();
}
//----------------------------------------------------------------------
QString AppManager::formatDataSize(qlonglong bytes) const
{
    if (bytes <= 0) return QString();
    return QLocale().formattedDataSize(bytes);
}
//----------------------------------------------------------------------
QString AppManager::formatDataSizeDelta(qlonglong bytes) const
{
    return QLocale().formattedDataSize(bytes);
}
//----------------------------------------------------------------------
QVariantMap AppManager::refreshStorageFromDisk(int deviceId)
{
    QVariantMap r;
    const QString conn = m_connectionName;
    Device dev;
    dev.ID = deviceId;
    dev.loadDevice(conn);

    if (dev.type != "Storage") {
        r["error"] = tr("Not a Storage device.");
        return r;
    }

    Storage::UpdateResult sr = dev.storage->updateStorageInfo();

    if (sr.errorCode == Storage::ErrorNoPath
     || sr.errorCode == Storage::ErrorEmptyDirectory
     || sr.errorCode == Storage::ErrorCannotGetValues) {
        r["error"] = sr.errorMessage;
        return r;
    }

    collection->saveDeviceTableToFile();
    refreshDeviceList();

    r["error"]        = QString();
    r["totalSpace"]   = (qlonglong)dev.storage->totalSpace;
    r["freeSpace"]    = (qlonglong)dev.storage->freeSpace;
    r["storageType"]  = dev.storage->type;
    r["storageLabel"] = dev.storage->label;
    r["fileSystem"]   = dev.storage->fileSystem;
    return r;
}

//----------------------------------------------------------------------
QString AppManager::pathToFileUrl(const QString &path) const
{
    return QUrl::fromLocalFile(path).toString();
}
//----------------------------------------------------------------------
QString AppManager::pathFromFileUrl(const QString &url) const
{
    return QUrl(url).toLocalFile();
}
//----------------------------------------------------------------------
QString AppManager::normalizeSourcePath(const QString &path) const
{
    return Catalog::normalizeSourcePath(path.trimmed());
}

// Exclude directories (collection-level)
//----------------------------------------------------------------------
QStringList AppManager::getExcludeDirectories() const
{
    return collection->getExcludeDirectories();
}
//----------------------------------------------------------------------
bool AppManager::addExcludeDirectory(const QString &path)
{
    if (path.trimmed().isEmpty()) return false;
    bool ok = collection->addExcludeDirectory(path.trimmed());
    if (ok) emit excludeDirectoriesChanged();
    return ok;
}
//----------------------------------------------------------------------
void AppManager::removeExcludeDirectory(const QString &path)
{
    collection->removeExcludeDirectory(path);
    emit excludeDirectoriesChanged();
}

//----------------------------------------------------------------------
// Backup
//----------------------------------------------------------------------

static MappingFilter buildMappingFilter(const QString &filterType, int deviceId, const QString &mappingType)
{
    MappingFilter filter;
    filter.mappingType = mappingType;
    if (filterType == QLatin1String("Source") && deviceId > 0)
        return MappingFilter(MappingFilter::SourceDevice, deviceId, mappingType);
    if (filterType == QLatin1String("Target") && deviceId > 0)
        return MappingFilter(MappingFilter::TargetDevice, deviceId, mappingType);
    return filter;
}

QVariantList AppManager::getBackupMappings(const QString &filterType, int deviceId, const QString &mappingType) const
{
    BackupMappingManager manager(m_connectionName);
    const MappingFilter filter = buildMappingFilter(filterType, deviceId, mappingType);
    const QList<MappingInfo> mappings = manager.getFilteredMappings(filter);

    // Refresh device_active for every referenced device so the active indicator
    // reflects whether the path currently exists on disk.
    QHash<int, bool> activeByDeviceId;
    for (const MappingInfo &m : mappings) {
        for (int pass = 0; pass < 2; ++pass) {
            int devId    = (pass == 0) ? m.sourceDeviceId : m.targetDeviceId;
            QString path = (pass == 0) ? m.sourcePath     : m.targetPath;
            if (!activeByDeviceId.contains(devId)) {
                Device d;
                d.ID   = devId;
                d.path = path;
                d.updateActiveState(m_connectionName);
                activeByDeviceId[devId] = d.active;
            }
        }
    }

    QVariantList result;
    result.reserve(mappings.size());
    for (const MappingInfo &m : mappings) {
        QVariantMap map;
        map[QStringLiteral("mappingId")]          = m.mappingId;
        map[QStringLiteral("mappingName")]         = m.mappingName;
        map[QStringLiteral("mappingType")]         = m.mappingType;
        map[QStringLiteral("sourceDeviceId")]      = m.sourceDeviceId;
        map[QStringLiteral("targetDeviceId")]      = m.targetDeviceId;
        map[QStringLiteral("sourceName")]          = m.sourceName;
        map[QStringLiteral("sourcePath")]          = m.sourcePath;
        map[QStringLiteral("sourceSize")]          = m.sourceSize;
        map[QStringLiteral("sourceSizeStr")]       = QLocale().formattedDataSize(m.sourceSize);
        map[QStringLiteral("sourceFileCount")]     = m.sourceFileCount;
        map[QStringLiteral("sourceActive")]        = activeByDeviceId.value(m.sourceDeviceId, false);
        map[QStringLiteral("sourceDateUpdated")]   = m.sourceDateUpdated;
        map[QStringLiteral("targetName")]          = m.targetName;
        map[QStringLiteral("targetPath")]          = m.targetPath;
        map[QStringLiteral("targetSize")]          = m.targetSize;
        map[QStringLiteral("targetSizeStr")]       = QLocale().formattedDataSize(m.targetSize);
        map[QStringLiteral("targetFileCount")]     = m.targetFileCount;
        map[QStringLiteral("targetActive")]        = activeByDeviceId.value(m.targetDeviceId, false);
        map[QStringLiteral("targetDateUpdated")]   = m.targetDateUpdated;
        map[QStringLiteral("sizeDiff")]            = m.sourceSize - m.targetSize;
        map[QStringLiteral("sizeDiffStr")]         = QLocale().formattedDataSize(qAbs(m.sourceSize - m.targetSize));
        map[QStringLiteral("fileCountDiff")]       = m.sourceFileCount - m.targetFileCount;
        map[QStringLiteral("strictCopy")]          = m.strictCopy;
        map[QStringLiteral("conflictMode")]        = conflictModeToString(m.conflictMode);
        map[QStringLiteral("sourceDrive")]         = m.sourceDrive;
        map[QStringLiteral("lastBackupDate")]      = m.lastBackupDate;
        map[QStringLiteral("lastBackupSize")]      = m.lastBackupSize;
        map[QStringLiteral("lastBackupSizeStr")]   = QLocale().formattedDataSize(m.lastBackupSize);
        result.append(map);
    }
    return result;
}
//----------------------------------------------------------------------
QVariantMap AppManager::getBackupTotals(const QString &filterType, int deviceId, const QString &mappingType) const
{
    BackupMappingManager manager(m_connectionName);
    const MappingFilter filter  = buildMappingFilter(filterType, deviceId, mappingType);
    const MappingTotals  totals = manager.calculateTotals(filter);

    const qint64 deviceSize  = selectedDevice->totalFileSize;
    const qint64 mapped      = totals.totalSourceSize;
    const float  coveragePct = (deviceSize > 0)
        ? static_cast<float>(mapped) / static_cast<float>(deviceSize) * 100.0f
        : 0.0f;

    QVariantMap result;
    result[QStringLiteral("totalMappings")]         = totals.totalMappings;
    result[QStringLiteral("totalSourceSizeStr")]    = QLocale().formattedDataSize(totals.totalSourceSize);
    result[QStringLiteral("totalTargetSizeStr")]    = QLocale().formattedDataSize(totals.totalTargetSize);
    result[QStringLiteral("totalSizeDiffStr")]      = QLocale().formattedDataSize(qAbs(totals.totalSizeDifference));
    result[QStringLiteral("totalSourceFileCount")]  = totals.totalSourceFileCount;
    result[QStringLiteral("totalTargetFileCount")]  = totals.totalTargetFileCount;
    result[QStringLiteral("totalFileCountDiff")]    = totals.totalFileCountDifference;
    result[QStringLiteral("deviceName")]            = selectedDevice->name;
    result[QStringLiteral("deviceSizeStr")]         = QLocale().formattedDataSize(deviceSize);
    result[QStringLiteral("coveragePct")]           = QString::number(static_cast<double>(coveragePct), 'f', 1);
    return result;
}
//----------------------------------------------------------------------
QString AppManager::createBackupMapping(const QString &name, const QString &type,
    int sourceId, int targetId, bool strictCopy,
    const QString &conflictMode, bool sourceDrive)
{
    if (name.trimmed().isEmpty())
        return tr("Provide a link name.");
    if (sourceId <= 0)
        return tr("Select a source catalog first.");
    if (targetId <= 0)
        return tr("Select a target catalog first.");
    if (sourceId == targetId)
        return tr("Select a different source or target (a device shall not be mapped to itself).");

    BackupMappingManager manager(m_connectionName);
    if (!manager.createMapping(name.trimmed(), type, sourceId, targetId, strictCopy, conflictMode, sourceDrive))
        return tr("Failed to create link.");

    collection->saveMappingTableToFile();
    emit backupMappingsChanged();
    return QString();
}
//----------------------------------------------------------------------
QString AppManager::updateBackupMapping(int mappingId, const QString &name, const QString &type,
    int sourceId, int targetId, bool strictCopy,
    const QString &conflictMode, bool sourceDrive)
{
    if (name.trimmed().isEmpty())
        return tr("Provide a link name.");
    if (sourceId <= 0)
        return tr("Select a source catalog first.");
    if (targetId <= 0)
        return tr("Select a target catalog first.");
    if (sourceId == targetId)
        return tr("Select a different source or target (a device shall not be mapped to itself).");

    BackupMappingManager manager(m_connectionName);
    if (!manager.updateMapping(mappingId, name.trimmed(), type, sourceId, targetId, strictCopy, conflictMode, sourceDrive))
        return tr("Failed to update link.");

    collection->saveMappingTableToFile();
    emit backupMappingsChanged();
    return QString();
}
//----------------------------------------------------------------------
bool AppManager::deleteBackupMapping(int mappingId)
{
    BackupMappingManager manager(m_connectionName);
    if (!manager.deleteMapping(mappingId))
        return false;
    collection->saveMappingTableToFile();
    emit backupMappingsChanged();
    return true;
}
//----------------------------------------------------------------------
bool AppManager::invertBackupMapping(int mappingId)
{
    BackupMappingManager manager(m_connectionName);
    if (!manager.invertMapping(mappingId))
        return false;
    collection->saveMappingTableToFile();
    emit backupMappingsChanged();
    return true;
}
//----------------------------------------------------------------------
// compareForBackup moved to core (CatalogDifferenceEngine::compareForBackup) per
// BKP-C4/BKP-C8 so the UI layer no longer holds compare/orchestration logic.
//----------------------------------------------------------------------
// startBackupPreview: entry point from QML. Runs the pre-preview catalog update (if
// enabled) on the Backup page, then the compare — both keep the UI responsive — and
// emits backupPreviewReady so the Backup Preview page opens only when the report is
// ready (BKP-F14/BKP-F16).
void AppManager::startBackupPreview(int mappingId)
{
    if (m_backupPreviewRunning || m_catalogUpdateForBackupRunning)
        return;

    if (updateBeforeBackup()) {
        m_previewAfterUpdateMappingId = mappingId;
        prepareCatalogsForMapping(mappingId);   // threaded update; chains to runPreviewCompare on completion
        return;
    }
    runPreviewCompare(mappingId);
}
//----------------------------------------------------------------------
void AppManager::stopBackupPreview()
{
    if (m_backupPreviewRunning)
        m_previewStopRequested = true;
}
//----------------------------------------------------------------------
void AppManager::runPreviewCompare(int mappingId)
{
    if (m_backupPreviewModel) m_backupPreviewModel->clear();

    // Enter the running state so the Backup page shows compare progress + a Stop button.
    m_backupPreviewRunning    = true;
    m_previewStopRequested    = false;
    m_backupPreviewStatusText = StatusBarMessageBuilder()
                                    .setOperation(tr("Search"))
                                    .setStatus(tr("In Progress"))
                                    .build();
    emit backupPreviewRunningChanged();
    emit backupPreviewStatusChanged();
    QCoreApplication::processEvents();  // paint the running state before the compare starts

    QVariantMap result;
    result[QStringLiteral("hasData")] = false;
    result[QStringLiteral("error")]   = QString();
    m_lastPreviewSummary = result;

    // Clears the running state and notifies QML. success/cancelled decide whether the
    // Backup Preview page opens (BKP-F14/BKP-F16).
    auto finish = [&](bool success, bool cancelled) {
        m_backupPreviewRunning = false;
        m_backupPreviewStatusText.clear();
        emit backupPreviewRunningChanged();
        emit backupPreviewStatusChanged();
        emit backupPreviewReady(mappingId, success, cancelled);
    };

    BackupMappingManager manager(m_connectionName);
    MappingInfo mapping = manager.getMappingById(mappingId);
    if (mapping.mappingId < 0) {
        result[QStringLiteral("error")] = tr("Link not found.");
        m_lastPreviewSummary = result;
        emit backupNotification(tr("Link not found."), true);
        finish(false, false);
        return;
    }

    Device sourceDevice;
    sourceDevice.ID = mapping.sourceDeviceId;
    sourceDevice.loadDevice(m_connectionName);
    Device targetDevice;
    targetDevice.ID = mapping.targetDeviceId;
    targetDevice.loadDevice(m_connectionName);

    if (sourceDevice.type != QLatin1String("Catalog") || targetDevice.type != QLatin1String("Catalog")) {
        result[QStringLiteral("error")] = tr("Both source and target must be Catalog devices.");
        m_lastPreviewSummary = result;
        emit backupNotification(tr("Both source and target must be Catalog devices."), true);
        finish(false, false);
        return;
    }

    sourceDevice.updateActiveState(m_connectionName);
    targetDevice.updateActiveState(m_connectionName);

    if (collection->databaseMode == QLatin1String("Memory")) {
        QMutex mutex;
        sourceDevice.catalog->loadCatalogFileListToTable(mutex, m_previewStopRequested);
        targetDevice.catalog->loadCatalogFileListToTable(mutex, m_previewStopRequested);
    }
    if (m_previewStopRequested) { finish(false, true); return; }

    CatalogDifferenceEngine cmpEngine(m_connectionName);
    const BackupCompareResult cmp = cmpEngine.compareForBackup(
        sourceDevice, targetDevice, mapping.strictCopy, mapping.sourceDrive, &m_previewStopRequested);
    if (m_previewStopRequested) { finish(false, true); return; }

    const bool                isArchive = (mapping.mappingType == QLatin1String("Archive"));

    const BackupSpaceCheck spaceCheck = evaluateBackupSpace(
        Storage::availableSpace(targetDevice.path),
        cmp.filesToCopy, cmp.fileConflicts, mapping.conflictMode);

    QString spaceStatusStr;
    switch (spaceCheck.status) {
        case BackupSpaceStatus::OK:           spaceStatusStr = QStringLiteral("OK");           break;
        case BackupSpaceStatus::Low:          spaceStatusStr = QStringLiteral("Low");          break;
        case BackupSpaceStatus::Insufficient: spaceStatusStr = QStringLiteral("Insufficient"); break;
        default:                              spaceStatusStr = QStringLiteral("Unknown");      break;
    }

    m_lastPreviewRows.clear();
    QVariantList filesToCopy;
    for (const DifferenceFileEntry &e : cmp.filesToCopy) {
        QVariantMap row;
        row[QStringLiteral("status")]      = isArchive ? tr("Move") : tr("Copy");
        row[QStringLiteral("fileName")]    = e.fileName;
        row[QStringLiteral("folderPath")]  = e.folderPath;
        row[QStringLiteral("fileSize")]    = e.fileSize;
        row[QStringLiteral("fileSizeStr")] = QLocale().formattedDataSize(e.fileSize);
        filesToCopy.append(row);
        BackupPreviewRow pr;
        pr.status    = isArchive ? tr("Move") : tr("Copy");
        pr.fileName  = e.fileName;
        pr.folderPath= e.folderPath;
        pr.fileSize  = e.fileSize;
        m_lastPreviewRows.append(pr);
    }
    QVariantList fileConflicts;
    for (const DifferenceFileEntry &e : cmp.fileConflicts) {
        QVariantMap row;
        row[QStringLiteral("status")]      = tr("Conflict");
        row[QStringLiteral("fileName")]    = e.fileName;
        row[QStringLiteral("folderPath")]  = e.folderPath;
        row[QStringLiteral("fileSize")]    = e.fileSize;
        row[QStringLiteral("fileSizeStr")] = QLocale().formattedDataSize(e.fileSize);
        fileConflicts.append(row);
        BackupPreviewRow pr;
        pr.status    = tr("Conflict");
        pr.fileName  = e.fileName;
        pr.folderPath= e.folderPath;
        pr.fileSize  = e.fileSize;
        m_lastPreviewRows.append(pr);
    }

    // Feed the resizable-column preview table (BKP-F15): copy/move rows first, then
    // conflicts — the trailing fileConflicts rows are flagged so the table can style them.
    if (m_backupPreviewModel)
        m_backupPreviewModel->populate(m_lastPreviewRows, static_cast<int>(cmp.fileConflicts.size()));

    result[QStringLiteral("hasData")]        = true;
    result[QStringLiteral("isArchive")]      = isArchive;
    result[QStringLiteral("filesToCopy")]    = filesToCopy;
    result[QStringLiteral("fileConflicts")]  = fileConflicts;
    result[QStringLiteral("skippedCount")]   = cmp.skippedCount;
    result[QStringLiteral("spaceStatus")]    = spaceStatusStr;
    result[QStringLiteral("spaceAvailable")] = spaceCheck.available;
    result[QStringLiteral("spaceRequired")]  = spaceCheck.required;
    result[QStringLiteral("sourceName")]     = sourceDevice.name;
    result[QStringLiteral("targetName")]     = targetDevice.name;
    result[QStringLiteral("sourceActive")]   = sourceDevice.active;
    result[QStringLiteral("targetActive")]   = targetDevice.active;
    result[QStringLiteral("mappingName")]    = mapping.mappingName;

    m_lastPreviewSummary = result;
    finish(true, false);
}
//----------------------------------------------------------------------
void AppManager::runBackup(int mappingId)
{
    if (m_backupJob) return;

    // If "update before" is on, update both catalogs first then call executeBackupJob()
    if (updateBeforeBackup() && !m_catalogUpdateForBackupRunning) {
        m_pendingBackupAfterUpdate = mappingId;
        prepareCatalogsForMapping(mappingId);
        return;
    }

    executeBackupJob(mappingId);
}
//----------------------------------------------------------------------
void AppManager::executeBackupJob(int mappingId)
{
    BackupMappingManager manager(m_connectionName);
    MappingInfo mapping = manager.getMappingById(mappingId);
    if (mapping.mappingId < 0) return;

    Device sourceDevice;
    sourceDevice.ID = mapping.sourceDeviceId;
    sourceDevice.loadDevice(m_connectionName);
    Device targetDevice;
    targetDevice.ID = mapping.targetDeviceId;
    targetDevice.loadDevice(m_connectionName);

    if (sourceDevice.type != QLatin1String("Catalog") || targetDevice.type != QLatin1String("Catalog")) {
        emit backupNotification(tr("Both source and target must be Catalog devices."), true);
        return;
    }

    sourceDevice.updateActiveState(m_connectionName);
    targetDevice.updateActiveState(m_connectionName);
    if (!sourceDevice.active) {
        emit backupNotification(tr("Source not available: %1").arg(sourceDevice.name), true);
        return;
    }
    if (!targetDevice.active) {
        emit backupNotification(tr("Target not available: %1").arg(targetDevice.name), true);
        return;
    }

    m_backupSourceDevice     = sourceDevice;
    m_backupTargetDevice     = targetDevice;
    m_runningBackupMappingId = mappingId;

    if (collection->databaseMode == QLatin1String("Memory")) {
        QMutex mutex;
        bool stopRequested = false;
        sourceDevice.catalog->loadCatalogFileListToTable(mutex, stopRequested);
        targetDevice.catalog->loadCatalogFileListToTable(mutex, stopRequested);
    }

    //Replicate the full directory tree first (preserves empty directories)
    if (mapping.sourceDrive) {
        //Drive mode: walk the source filesystem directly
        DirectoryReplicator replicator(m_connectionName);
        replicator.replicateFromDrive(sourceDevice.path, targetDevice.path);
    } else {
        //Catalog mode: replicate from the folder table (includes empty directories)
        if (collection->databaseMode == QLatin1String("Memory"))
            sourceDevice.catalog->loadFoldersToTable();
        DirectoryReplicator replicator(m_connectionName);
        replicator.replicate({sourceDevice.externalID}, sourceDevice.path, targetDevice.path);
    }

    CatalogDifferenceEngine cmpEngine(m_connectionName);
    const BackupCompareResult cmp = cmpEngine.compareForBackup(sourceDevice, targetDevice, mapping.strictCopy, mapping.sourceDrive);
    if (cmp.filesToCopy.isEmpty() && cmp.fileConflicts.isEmpty()) {
        m_runningBackupMappingId = -1;
        emit backupFinished(0, 0, 0, 0, 0, 0, false);
        return;
    }

    const bool isArchive = (mapping.mappingType == QLatin1String("Archive"));
    m_backupIsArchive = isArchive;

    m_backupJob = new BackupJobStoppable();
    m_backupJob->setFiles(cmp.filesToCopy);
    m_backupJob->setConflictMode(mapping.conflictMode);
    if (mapping.conflictMode == ConflictMode::RenameOldest)
        m_backupJob->setConflictFiles(cmp.fileConflicts);
    m_backupJob->setSourcePath(sourceDevice.path);
    m_backupJob->setTargetPath(targetDevice.path);
    m_backupJob->setArchiveMode(isArchive);

    m_backupThread = new QThread();
    m_backupJob->moveToThread(m_backupThread);
    connect(m_backupThread, &QThread::started,   m_backupJob,    &BackupJobStoppable::runBackup);
    connect(m_backupJob,    &BackupJobStoppable::backupProgress, this, &AppManager::onBackupProgressInternal);
    connect(m_backupJob,    &BackupJobStoppable::backupFinished, this, &AppManager::onBackupFinishedInternal);
    connect(m_backupJob,    &BackupJobStoppable::backupFinished, m_backupThread, &QThread::quit);
    connect(m_backupThread, &QThread::finished,  m_backupJob,    &QObject::deleteLater);
    connect(m_backupThread, &QThread::finished,  m_backupThread, &QObject::deleteLater);

    m_backupTimer.start();
    m_backupThread->start();
}
//----------------------------------------------------------------------
void AppManager::stopBackup()
{
    if (m_backupJob) m_backupJob->stopBackup();
}
//----------------------------------------------------------------------
void AppManager::pauseBackup()
{
    if (m_backupJob) m_backupJob->pauseBackup();
}
//----------------------------------------------------------------------
void AppManager::resumeBackup()
{
    if (m_backupJob) m_backupJob->resumeBackup();
}
//----------------------------------------------------------------------
void AppManager::onBackupProgressInternal(int filesDone, int totalFiles,
    qint64 bytesCopied, qint64 totalBytes, const QString &currentFile)
{
    emit backupProgress(filesDone, totalFiles, bytesCopied, totalBytes, currentFile);
}
//----------------------------------------------------------------------
void AppManager::onBackupFinishedInternal(const BackupReport &report)
{
    const int finishedMappingId = m_runningBackupMappingId;
    m_backupJob              = nullptr;
    m_backupThread           = nullptr;
    m_runningBackupMappingId = -1;
    emit backupFinished(
        report.copiedCount(),
        report.movedCount(),
        report.renamedCount(),
        report.conflictCount(),
        report.errorCount(),
        report.totalBytesCopied,
        report.wasCancelled);

    // Record the completed run on the mapping (date + bytes transferred) so the card can
    // show "Last backup". Done for any completed (non-cancelled) run, including 0 files.
    if (!report.wasCancelled && finishedMappingId >= 0) {
        BackupMappingManager manager(m_connectionName);
        manager.setLastBackup(finishedMappingId,
                              QDateTime::currentDateTime().toString(QStringLiteral("yyyy-MM-dd hh:mm:ss")),
                              report.totalBytesCopied);
        collection->saveMappingTableToFile();
        emit backupMappingsChanged();
    }

    // Re-scan the affected catalogs after a successful backup so the card's totals/diff
    // refresh (same rule as K2: the "Update catalogs" checkbox controls before AND after).
    // Backup only changes the target; Archive moves files out of the source, so re-scan
    // both — source first so the visible "To move" count updates, then target.
    if (!report.wasCancelled && updateBeforeBackup() && !m_catalogUpdateForBackupRunning) {
        m_postBackupUpdateQueue.clear();
        if (m_backupIsArchive)
            m_postBackupUpdateQueue.append(m_backupSourceDevice.ID);
        m_postBackupUpdateQueue.append(m_backupTargetDevice.ID);
        // Defer past DeviceUpdateManager's ~10 ms post-operation cleanup window (same
        // singleShot guard used in onCatalogUpdateForBackupStep) so the update runs
        // against a fully idle manager rather than erroring out.
        QTimer::singleShot(60, this, [this]() { runNextPostBackupUpdate(); });
    }
}
//----------------------------------------------------------------------
void AppManager::runNextPostBackupUpdate()
{
    if (m_postBackupUpdateQueue.isEmpty()) {
        emit backupMappingsChanged();
        return;
    }
    const int id = m_postBackupUpdateQueue.takeFirst();
    setupDeviceUpdateManager();   // clean, idle manager
    disconnect(m_deviceUpdateManager, nullptr, this, nullptr);
    // Refresh after each catalog (on any outcome) and chain to the next one, deferring
    // past the cleanup window so the manager is idle before the next update.
    auto onDone = [this]() {
        setupDeviceUpdateManager();
        emit backupMappingsChanged();
        QTimer::singleShot(60, this, [this]() { runNextPostBackupUpdate(); });
    };
    connect(m_deviceUpdateManager, &DeviceUpdateManager::operationCompleted, this, [onDone]() { onDone(); });
    connect(m_deviceUpdateManager, &DeviceUpdateManager::operationError,     this, [onDone](const QString &) { onDone(); });
    connect(m_deviceUpdateManager, &DeviceUpdateManager::operationCancelled, this, [onDone]() { onDone(); });

    Device *dev = new Device();
    dev->ID = id;
    dev->loadDevice(m_connectionName);
    m_deviceUpdateManager->updateDeviceHierarchy(dev, collection->databaseMode, collection->folder, "update");
}
//----------------------------------------------------------------------
QVariantMap AppManager::replicateDirectories(int mappingId)
{
    QVariantMap result;
    BackupMappingManager manager(m_connectionName);
    MappingInfo mapping = manager.getMappingById(mappingId);
    if (mapping.mappingId < 0) {
        result[QStringLiteral("error")] = tr("Link not found.");
        return result;
    }

    Device sourceDevice;
    sourceDevice.ID = mapping.sourceDeviceId;
    sourceDevice.loadDevice(m_connectionName);
    Device targetDevice;
    targetDevice.ID = mapping.targetDeviceId;
    targetDevice.loadDevice(m_connectionName);

    if (sourceDevice.type != QLatin1String("Catalog") || targetDevice.type != QLatin1String("Catalog")) {
        result[QStringLiteral("error")] = tr("Both source and target must be Catalog devices.");
        return result;
    }
    sourceDevice.updateActiveState(m_connectionName);
    targetDevice.updateActiveState(m_connectionName);
    if (!sourceDevice.active) {
        result[QStringLiteral("error")] = tr("Source not available: %1").arg(sourceDevice.name);
        return result;
    }
    if (!targetDevice.active) {
        result[QStringLiteral("error")] = tr("Target not available: %1").arg(targetDevice.name);
        return result;
    }

    DirectoryReplicator replicator(m_connectionName);
    ReplicationResult repResult;
    if (mapping.sourceDrive) {
        repResult = replicator.replicateFromDrive(sourceDevice.path, targetDevice.path);
    } else {
        if (collection->databaseMode == QLatin1String("Memory"))
            sourceDevice.catalog->loadFoldersToTable();
        repResult = replicator.replicate({sourceDevice.externalID}, sourceDevice.path, targetDevice.path);
    }

    result[QStringLiteral("error")]   = QString();
    result[QStringLiteral("created")] = repResult.createdCount();
    result[QStringLiteral("skipped")] = repResult.skippedCount();
    result[QStringLiteral("errors")]  = repResult.errorCount();
    return result;
}
//----------------------------------------------------------------------
QString AppManager::exportLastBackupPreviewToCsv()
{
    if (m_lastPreviewRows.isEmpty())
        return tr("No preview data to export.");
    const QString filePath = BackupMappingManager::exportPreviewToCsv(m_lastPreviewRows, collection->folder);
    if (filePath.isEmpty())
        return tr("Export failed.");
    return filePath;
}
//----------------------------------------------------------------------
QString AppManager::generateLuckyBackupProfile(const QVariantList &mappingIds)
{
    QList<int> ids;
    for (const QVariant &v : mappingIds)
        ids.append(v.toInt());

    BackupProfileGenerator generator(m_connectionName);
    const BackupProfileResult result = generator.generateProfile(ids);
    if (!result.success) {
        const QString msg = tr("Failed to generate Backup profile") + ": " + result.errorMessage;
        emit backupNotification(msg, true);
        return msg;
    }
    const QString msg = tr("Backup profile created.") + " " + result.profilePath;
    emit backupNotification(msg, false);
    return msg;
}
//----------------------------------------------------------------------
QString AppManager::getBackupSetting(const QString &key) const
{
    QSettings settings(collection->settingsFilePath, QSettings::IniFormat);
    return settings.value("BackUp/" + key).toString();
}
//----------------------------------------------------------------------
void AppManager::setBackupSetting(const QString &key, const QVariant &value)
{
    QSettings settings(collection->settingsFilePath, QSettings::IniFormat);
    settings.setValue("BackUp/" + key, value);
    settings.sync();
}
//----------------------------------------------------------------------
bool AppManager::updateBeforeBackup() const
{
    return getBackupSetting(QStringLiteral("UpdateBeforeBackup")) == QLatin1String("true");
}
//----------------------------------------------------------------------
void AppManager::setUpdateBeforeBackup(bool v)
{
    setBackupSetting(QStringLiteral("UpdateBeforeBackup"), v ? QStringLiteral("true") : QStringLiteral("false"));
    emit updateBeforeBackupChanged();
}
//----------------------------------------------------------------------
// Async: updates source catalog then target catalog for the given mapping.
// Emits catalogsForMappingPrepared() when both are done (or on error).
// runBackup() uses this internally; QML preview page calls it directly.
void AppManager::prepareCatalogsForMapping(int mappingId)
{
    if (m_catalogUpdateForBackupRunning) {
        emit catalogsForMappingPrepared(mappingId, false,
            tr("A catalog update is already in progress. Please wait and try again."));
        return;
    }

    BackupMappingManager manager(m_connectionName);
    MappingInfo mapping = manager.getMappingById(mappingId);
    if (mapping.mappingId < 0) {
        emit catalogsForMappingPrepared(mappingId, false, tr("Mapping not found."));
        return;
    }

    Device sourceDevice;
    sourceDevice.ID = mapping.sourceDeviceId;
    sourceDevice.loadDevice(m_connectionName);
    Device targetDevice;
    targetDevice.ID = mapping.targetDeviceId;
    targetDevice.loadDevice(m_connectionName);

    m_pendingCatalogUpdateMappingId    = mappingId;
    m_pendingCatalogUpdateSourceDevice = sourceDevice;
    m_pendingCatalogUpdateTargetDevice = targetDevice;
    m_backupCatalogUpdatePhase         = BackupCatalogUpdatePhase::UpdatingSource;
    m_catalogUpdateForBackupRunning    = true;
    emit catalogUpdateForBackupRunningChanged();

    setupCatalogUpdateForBackupConnections();

    Device *dev = new Device();
    *dev = m_pendingCatalogUpdateSourceDevice;
    m_deviceUpdateManager->updateDeviceHierarchy(dev, collection->databaseMode, collection->folder, "update");
}
//----------------------------------------------------------------------
void AppManager::setupCatalogUpdateForBackupConnections()
{
    if (!m_deviceUpdateManager)
        setupDeviceUpdateManager();
    disconnect(m_deviceUpdateManager, nullptr, this, nullptr);

    // Route catalog-update progress to the backup-specific status text so the Backup
    // page can show the same StatusBarMessageBuilder messages as other screens. Wire
    // this explicitly (not via setupDeviceUpdateManager) so it is deterministic even
    // if a device/create update ran earlier and pointed the progress manager elsewhere.
    if (!m_catalogProgressManager)
        m_catalogProgressManager = new CatalogProgressManager(this);
    disconnect(m_catalogProgressManager, nullptr, this, nullptr);
    connect(m_catalogProgressManager, &CatalogProgressManager::statusMessageChanged,
            this, [this](const QString &message, int /*timeout*/) {
                m_catalogUpdateForBackupStatusText = message;
                emit catalogUpdateForBackupStatusChanged();
            });
    m_deviceUpdateManager->setCatalogProgressManager(m_catalogProgressManager);

    connect(m_deviceUpdateManager, &DeviceUpdateManager::operationCompleted,
            this, &AppManager::onCatalogUpdateForBackupStep);

    auto onFail = [this](const QString &error) {
        m_catalogUpdateForBackupRunning = false;
        m_catalogUpdateForBackupStatusText.clear();
        m_backupCatalogUpdatePhase      = BackupCatalogUpdatePhase::None;
        m_pendingBackupAfterUpdate      = -1;
        int id                          = m_pendingCatalogUpdateMappingId;
        m_pendingCatalogUpdateMappingId = -1;
        setupDeviceUpdateManager();
        emit catalogUpdateForBackupRunningChanged();
        emit catalogUpdateForBackupStatusChanged();
        emit catalogsForMappingPrepared(id, false, error);

        // If a preview was waiting on this update, report it as a failed preview so the
        // Backup page clears and the Preview page does not open.
        if (m_previewAfterUpdateMappingId == id) {
            m_previewAfterUpdateMappingId = -1;
            if (!error.isEmpty())
                emit backupNotification(error, true);
            emit backupPreviewReady(id, false, false);
        }
    };
    connect(m_deviceUpdateManager, &DeviceUpdateManager::operationError,     this, onFail);
    connect(m_deviceUpdateManager, &DeviceUpdateManager::operationCancelled, this,
            [onFail]() { onFail(QString()); });
}
//----------------------------------------------------------------------
void AppManager::onCatalogUpdateForBackupStep()
{
    if (m_backupCatalogUpdatePhase == BackupCatalogUpdatePhase::UpdatingSource) {
        // Source done — update target next.
        // DeviceUpdateManager defers cleanup ~10 ms after operationCompleted; delay
        // past that window so the manager is fully reset before the next operation.
        m_backupCatalogUpdatePhase = BackupCatalogUpdatePhase::UpdatingTarget;
        QTimer::singleShot(50, this, [this]() {
            Device *dev = new Device();
            *dev = m_pendingCatalogUpdateTargetDevice;
            m_deviceUpdateManager->updateDeviceHierarchy(dev, collection->databaseMode, collection->folder, "update");
        });
        return;
    }

    // Both catalogs updated — reload device data then continue
    m_backupCatalogUpdatePhase = BackupCatalogUpdatePhase::None;
    m_pendingCatalogUpdateSourceDevice.loadDevice(m_connectionName);
    m_pendingCatalogUpdateTargetDevice.loadDevice(m_connectionName);

    int id                          = m_pendingCatalogUpdateMappingId;
    m_pendingCatalogUpdateMappingId = -1;
    int pendingBackup               = m_pendingBackupAfterUpdate;
    m_pendingBackupAfterUpdate      = -1;
    m_catalogUpdateForBackupRunning = false;
    m_catalogUpdateForBackupStatusText.clear();   // backup/archive run shows its own progress next

    setupDeviceUpdateManager();  // restore normal catalog/device connections
    emit catalogUpdateForBackupRunningChanged();
    emit catalogUpdateForBackupStatusChanged();
    emit catalogsForMappingPrepared(id, true, QString());

    if (pendingBackup >= 0)
        executeBackupJob(pendingBackup);
    else if (m_previewAfterUpdateMappingId == id) {
        // Preview flow: catalogs are updated — now run the (cancellable) compare, then
        // backupPreviewReady opens the Preview page.
        m_previewAfterUpdateMappingId = -1;
        runPreviewCompare(id);
    }
}

