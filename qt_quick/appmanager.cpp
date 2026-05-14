#include "appmanager.h"
#include "core/catalog.h"
#include "core/language.h"
#include "core/database.h"
#include "core/databasemanager.h"
#include "core/device.h"
#include "core/filechecksum.h"
#include "core/filemetadata.h"
#include "core/searchjobstoppable.h"
#include "core/statusbarmessagebuilder.h"
#include "core/catalog.h"
#include <QJsonDocument>
#include <QJsonObject>
#include "version.h"
#include <QGuiApplication>
#include <QCoreApplication>
#include <QCryptographicHash>
#include <QTimer>

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

    // Load collection folder (Memory mode) from settings
    collection->folder = settings.value("LastCollectionFolder").toString();

    selectedDevice->ID = settings.value("Selection/SelectedDeviceID", 1).toInt();
    if (selectedDevice->ID == 0) selectedDevice->type = "All";
    selectedDevice->loadDevice(QSqlDatabase::defaultConnection);

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
    connect(search, &Search::searchProgress, this, &AppManager::onSearchProgress);
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

    // Memory mode requires CSV loading during search
    searchObject->setMemoryModeEnabled(collection->databaseMode == "Memory");

    // Capture device path at search time so Results page shows the right device
    searchObject->devicePath = Device::getDevicePath(selectedDevice->ID, QSqlDatabase::defaultConnection);

    // Save criteria to history table before search (mirrors SearchManager::startSearchJobStoppable)
    searchObject->saveSearchHistoryToTable(QSqlDatabase::defaultConnection);

    m_searchIsRunning = true;
    m_searchStatusText = StatusBarMessageBuilder().setOperation(tr("Search")).setStatus(tr("In Progress")).build();
    emit searchStateChanged();
    emit searchStatusTextChanged();

    // Execute search (synchronous — onSearchProgress() fires via direct connection during this call)
    searchObject->searchFiles(selectedDevice);

    m_searchIsRunning = false;
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
    const QString conn = QSqlDatabase::defaultConnection;

    QSqlError err = DatabaseManager::connect(conn, collection);
    if (err.type() != QSqlError::NoError) {
        qWarning() << "startDatabase failed:" << err.text();
        return "startDatabase: " + err.text();
    }

    initializeDeviceListModel();
    emit databaseModeChanged();
    return "startDatabase: No Error";
}
//----------------------------------------------------------------------
void AppManager::initializeDeviceListModel()
{
    if (deviceListModel) {
        delete deviceListModel;
    }
    deviceListModel = new DeviceListModel(this);
    m_deviceExpandLevel = -1; // reset to fully expanded on new connection

    if (!m_deviceFilterModel) {
        m_deviceFilterModel = new QSortFilterProxyModel(this);
        m_deviceFilterModel->setFilterRole(DeviceListModel::NameRole);
        m_deviceFilterModel->setFilterCaseSensitivity(Qt::CaseInsensitive);
    }
    m_deviceFilterModel->setSourceModel(deviceListModel);

    qDebug() << "DeviceListModel initialized with" << deviceListModel->rowCount() << "devices";
}
//----------------------------------------------------------------------
void AppManager::openDeviceFolder(int deviceId)
{
    Device device;
    device.ID = deviceId;
    device.loadDevice(QSqlDatabase::defaultConnection);
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

    int maxDepth = Device::getMaxHierarchyDepth(QSqlDatabase::defaultConnection);
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
        int maxDepth = Device::getMaxHierarchyDepth(QSqlDatabase::defaultConnection);
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
void AppManager::openSettingsFile()
{
    QDesktopServices::openUrl(QUrl::fromLocalFile(collection->settingsFilePath));
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
        emit databaseConnectionChanged(true, "Opened: " + QFileInfo(path).fileName());
    } else {
        // Rollback settings and connection
        settings.setValue("Settings/databaseMode",    oldMode);
        settings.setValue("Settings/DatabaseFilePath", oldFilePath);
        settings.sync();
        collection->databaseFilePath = oldPath;
        reconnectToDatabase();
        emit databaseConnectionChanged(false, "Failed to open database: " + lastDatabaseError);
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

    emit tagsChanged();
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
    const QString conn = QSqlDatabase::defaultConnection;

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

    lastDatabaseError.clear();
    collection->loadImageFolderPath();
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

    const QString conn = QSqlDatabase::defaultConnection;
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
                    FileChecksum::updateFileChecksum(QSqlDatabase::defaultConnection,
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

            if (FileMetadata::updateFileMetadata(QSqlDatabase::defaultConnection,
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
    QSqlQuery query(QSqlDatabase::database(QSqlDatabase::defaultConnection));
    query.prepare(QLatin1String("SELECT catalog_include_metadata FROM catalog WHERE catalog_id = :id"));
    query.bindValue(":id", catalogId);
    if (query.exec() && query.next())
        return query.value(0).toString().contains("Extended");
    return false;
}
//----------------------------------------------------------------------
QString AppManager::getFileMetadataJson(int catalogId, const QString &fileName, const QString &folderPath)
{
    return Catalog::getFileMetadataJson(catalogId, fileName, folderPath, QSqlDatabase::defaultConnection);
}
//----------------------------------------------------------------------
QVariantList AppManager::getFileMetadataParsedFields(int catalogId, const QString &fileName, const QString &folderPath)
{
    QString json = Catalog::getFileMetadataJson(catalogId, fileName, folderPath, QSqlDatabase::defaultConnection);
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
        FileChecksum::updateFileChecksum(QSqlDatabase::defaultConnection, catalogId, fileName, folderPath, checksum, "SHA256");
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
    QSqlQuery query(QSqlDatabase::database(QSqlDatabase::defaultConnection));
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
                QString item = QLatin1Char('"') + phrase + QLatin1Char('"');
                if (!criteria.isEmpty() && criteria != QLatin1String("All Words"))
                    item += QLatin1String(" (") + criteria + QLatin1Char(')');
                parts << item;
            }
            if (query.value(4).toBool())
                parts << tr("Case sensitive");
            QString exclude = query.value(5).toString();
            if (!exclude.isEmpty())
                parts << tr("Exclude: %1").arg(exclude);
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
    searchObject->loadSearchHistoryCriteria(QSqlDatabase::defaultConnection);
    return searchObject->properties();
}
//----------------------------------------------------------------------
void AppManager::clearSearchHistory()
{
    collection->clearSearchHistory(QSqlDatabase::defaultConnection);
}
//----------------------------------------------------------------------
void AppManager::keepLastSearchHistory(int count)
{
    collection->keepLastSearchHistory(count, QSqlDatabase::defaultConnection);
}
//----------------------------------------------------------------------
QStringList AppManager::getTagNames() const
{
    collection->loadTagFileToTable();
    Tag tag;
    tag.loadFromDatabase(QSqlDatabase::defaultConnection);
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
    tagModel.loadFromDatabase(QSqlDatabase::defaultConnection, filterName);
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
        emit databaseConnectionChanged(true, "Memory collection opened: " + QFileInfo(folder).fileName());
    } else {
        emit databaseConnectionChanged(false, "Failed to open Memory collection: " + lastDatabaseError);
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
        emit databaseConnectionChanged(true, "Connected: " + hostName + "/" + dbName);
    } else {
        emit databaseConnectionChanged(false, "Hosted connection failed: " + lastDatabaseError);
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

    if (!m_catalogProgressManager) {
        m_catalogProgressManager = new CatalogProgressManager(this);
        connect(m_catalogProgressManager, &CatalogProgressManager::statusMessageChanged,
                this, [this](const QString &message, int /*timeout*/) {
                    m_catalogStatusText = message;
                    emit catalogStatusTextChanged();
                });
    }
    m_deviceUpdateManager->setCatalogProgressManager(m_catalogProgressManager);

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
    parentDevice.loadDevice(QSqlDatabase::defaultConnection);

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

    // Setup manager and check for concurrent operation
    setupDeviceUpdateManager();
    if (m_deviceUpdateManager->operationRunning()) {
        delete newDevice;
        return tr("A device operation is already running.");
    }

    m_creatingDevice = newDevice;
    m_catalogIsCreating = true;
    m_catalogCreateStartTime = QDateTime::currentDateTime();
    m_catalogStatusText = StatusBarMessageBuilder().setOperation(tr("Create")).setStatus(tr("In Progress")).build();
    emit catalogIsCreatingChanged();
    emit catalogStatusTextChanged();

    m_deviceUpdateManager->updateDeviceHierarchy(newDevice,
                                                  collection->databaseMode,
                                                  collection->folder,
                                                  "create");
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

    const QDateTime endTime  = QDateTime::currentDateTime();
    const qint64 elapsedMs   = m_catalogCreateStartTime.msecsTo(endTime);
    const int totalSec       = static_cast<int>(elapsedMs / 1000);
    const QString duration   = QString("%1:%2:%3")
        .arg(totalSec / 3600,        2, 10, QLatin1Char('0'))
        .arg((totalSec % 3600) / 60, 2, 10, QLatin1Char('0'))
        .arg(totalSec % 60,          2, 10, QLatin1Char('0'));
    const QString report = tr("Indexing — Start: %1 | End: %2 | Duration: %3")
        .arg(m_catalogCreateStartTime.toString("hh:mm:ss"))
        .arg(endTime.toString("hh:mm:ss"))
        .arg(duration);

    m_catalogIsCreating = false;
    m_catalogStatusText = report;
    emit catalogIsCreatingChanged();
    emit catalogStatusTextChanged();
    QTimer::singleShot(8000, this, [this]() {
        m_catalogStatusText.clear();
        emit catalogStatusTextChanged();
    });

    refreshDeviceList();
    emit catalogCreationCompleted(true, report);
    m_creatingDevice = nullptr;
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
}
//----------------------------------------------------------------------
void AppManager::onCatalogCreationCancelled()
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
    emit catalogCreationCompleted(false, tr("Catalog creation was stopped."));
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
        return Device::getFirstStorageDescendantId(selectedDevice->ID, QSqlDatabase::defaultConnection);
    return 0;
}

// Device delete
//----------------------------------------------------------------------
QVariantMap AppManager::checkDeviceDeleteAllowed(int deviceId) const
{
    const QString conn = QSqlDatabase::defaultConnection;
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
    const QString conn = QSqlDatabase::defaultConnection;
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

    refreshAllUI();
    return QString();
}

// Device edit
//----------------------------------------------------------------------
QVariantMap AppManager::getDeviceDetails(int deviceId) const
{
    const QString conn = QSqlDatabase::defaultConnection;
    Device dev;
    dev.ID = deviceId;
    dev.loadDevice(conn);

    QVariantMap r;
    r["type"]     = dev.type;
    r["name"]     = dev.name;
    r["parentId"] = dev.parentID;
    r["path"]     = dev.path;
    r["deviceId"] = dev.ID;

    if (dev.type == "Catalog") {
        r["fileType"]        = dev.catalog->fileType;
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
QString AppManager::saveDeviceBasicFields(int deviceId, const QString &name, int parentId, const QString &path)
{
    const QString conn = QSqlDatabase::defaultConnection;
    Device dev;
    dev.ID = deviceId;
    dev.loadDevice(conn);

    // Name uniqueness for Catalog
    if (dev.type == "Catalog" && name != dev.name) {
        Device check;
        check.name = name;
        if (check.verifyDeviceNameExists())
            return tr("There is already a catalog with this name: %1\nChoose a different name.").arg(name);
    }

    dev.name = name;
    dev.path = path;

    // Trim trailing slash (except root "/")
    if (dev.path.length() > 1 && dev.path.endsWith('/'))
        dev.path.chop(1);

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

    int newGroupId = (parentId == 0) ? 1 : newParent.groupID;
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
    collection->saveDeviceTableToFile();
    return {};
}
//----------------------------------------------------------------------
QVariantMap AppManager::checkCatalogOptionChanges(int deviceId, const QString &fileType, bool includeHidden,
                                                   const QString &includeMetadata, const QString &includeChecksum,
                                                   bool isFullDevice) const
{
    const QString conn = QSqlDatabase::defaultConnection;
    Device prev;
    prev.ID = deviceId;
    prev.loadDevice(conn);

    QStringList changedFields;
    bool rescanNeeded = false;

    if (fileType != prev.catalog->fileType) {
        changedFields << tr("File type: %1 → %2").arg(prev.catalog->fileType, fileType);
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
QString AppManager::saveCatalogOptions(int deviceId, const QString &fileType, bool includeHidden,
                                        const QString &includeMetadata, const QString &includeChecksum,
                                        bool isFullDevice)
{
    const QString conn = QSqlDatabase::defaultConnection;
    Device dev;
    dev.ID = deviceId;
    dev.loadDevice(conn);

    const QString prevMetadata = dev.catalog->includeMetadata;

    dev.catalog->fileType        = fileType;
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
    const QString conn = QSqlDatabase::defaultConnection;
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
    dev->loadDevice(QSqlDatabase::defaultConnection);
    if (!m_deviceUpdateManager)
        setupDeviceUpdateManager();
    m_deviceUpdateManager->updateDeviceHierarchy(dev, collection->databaseMode, collection->folder, "update");
}
//----------------------------------------------------------------------
void AppManager::triggerStoragePathReplace(int deviceId, const QString &previousPath, const QString &newPath)
{
    Device *dev = new Device();
    dev->ID = deviceId;
    dev->loadDevice(QSqlDatabase::defaultConnection);
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
    dev.loadDevice(QSqlDatabase::defaultConnection);
    if (dev.type != "Catalog") return {};
    return dev.catalog->getExcludeFolders();
}
//----------------------------------------------------------------------
bool AppManager::addDeviceExcludeFolder(int deviceId, const QString &path)
{
    Device dev;
    dev.ID = deviceId;
    dev.loadDevice(QSqlDatabase::defaultConnection);
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
    dev.loadDevice(QSqlDatabase::defaultConnection);
    if (dev.type != "Catalog") return false;
    bool ok = dev.catalog->removeExcludeFolder(path);
    if (ok) collection->saveCatalogFilterTableToFile();
    return ok;
}

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
    m_importStatusText = tr("Collection Import") + " | " + tr("In Progress");
    emit importIsRunningChanged();
    emit importStatusTextChanged();

    QMetaObject::Connection progressConn = connect(
        m_importer, &CollectionImporter::fileImportProgress,
        this, [this](int catalogIndex, int totalCatalogs, const QString &catalogName,
                     qint64 done, qint64 total) {
            m_importStatusText =
                tr("Collection Import") + " | "
                + QString("%1/%2: %3 | ").arg(catalogIndex).arg(totalCatalogs).arg(catalogName)
                + tr("Files") + QString(": %1 / %2")
                    .arg(QLocale().toString(done))
                    .arg(QLocale().toString(total));
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
        m_importStatusText = tr("Collection Import") + " | " + tr("Error") + ": " + m_importer->lastError();
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

    m_importStatusText = tr("Collection Import") + " | " + tr("Completed");
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
    m_importStatusText = tr("Collection Update") + " | " + tr("In Progress");
    emit importIsRunningChanged();
    emit importStatusTextChanged();
    QCoreApplication::processEvents(QEventLoop::ExcludeUserInputEvents);

    bool ok = m_importer->updateAllImportsFromSource(path);

    m_importIsRunning = false;
    emit importIsRunningChanged();

    if (!ok) {
        m_importStatusText = tr("Collection Update") + " | " + tr("Error") + ": " + m_importer->lastError();
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

    m_importStatusText = tr("Collection Update") + " | " + tr("Completed");
    emit importStatusTextChanged();

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
QVariantMap AppManager::refreshStorageFromDisk(int deviceId)
{
    QVariantMap r;
    const QString conn = QSqlDatabase::defaultConnection;
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

