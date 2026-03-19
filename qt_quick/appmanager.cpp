#include "appmanager.h"
#include "core/catalog.h"
#include "core/database.h"
#include "core/databasemanager.h"
#include "core/device.h"
#include "core/filechecksum.h"
#include "core/filemetadata.h"
#include <QJsonDocument>
#include <QJsonObject>
#include "version.h"
#include <QGuiApplication>
#include <QCryptographicHash>

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
QStringList AppManager::getTagNames() const
{
    Tag tag;
    tag.loadFromDatabase(QSqlDatabase::defaultConnection);
    QList<QString> names = tag.tagNames();
    QStringList result;
    //result << tr("All");
    for (const QString &name : names)
        result << name;
    return result;
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
        entry["iconName"]    = settings.value(QString("Recent/%1/iconName").arg(i)).toString();
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
    if (mode == "File")   return "server-database";
    return "network-server";
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
                     : (mode == "File")   ? "server-database"
                                          : "network-server";
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
