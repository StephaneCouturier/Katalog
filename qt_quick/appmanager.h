#ifndef APPMANAGER_H
#define APPMANAGER_H

#include <QObject>
#include <QDebug>

//QtCore
#include <QFile>
#include <QFileInfo>
#include <QLocale>
#include <QStringList>
#include <QStringListModel>
#include <QTranslator>
#include <QDateTime>
#include <QAbstractTableModel>
#include <QSortFilterProxyModel>
#include <QStorageInfo>
#include <QStandardPaths>
#include <QTextStream>
#include <QSaveFile>
#include <QSettings>
//QtGui
#include <QFileSystemModel>
#include <QClipboard>
#include <QStandardItemModel>
#include <QDesktopServices>
#include <QCloseEvent>
#include <QPixmap>
//QtSql
#include <QtSql>

//#include<QtSql/QSqlError>
//#include<QtSql/QSqlDatabase>
//#include<QtSql/QSqlQuery>
#include<QFile>

//QtMultimedia
//#include <QMediaPlayer>
//#include <QMediaMetaData>
//QtNetwork
#include <QNetworkAccessManager>
#include <QNetworkReply>
//QtCharts
//#include <QDateTimeAxis>
//#include <QtCharts/QBarSeries>
//#include <QtCharts/QBarSet>
//#include <QtCharts/QLineSeries>
//#include <QtCharts/QBarCategoryAxis>
//#include <QtCharts/QValueAxis>
//#include <QScatterSeries>
//#include <QtCharts/QLegendMarker>

//KF6
#include <KArchive>
#include <KZip>

//Katalog object classes
#include "core/collection.h"
#include "core/device.h"
#include "core/tag.h"
#include "core/deviceupdatemanager.h"
#include "core/catalogprogressmanager.h"
#include "core/collectionimporter.h"
#include "adapters/search.h"
#include "adapters/devicelistmodel.h"

class AppManager : public QObject
{
    Q_OBJECT
    Q_PROPERTY(DeviceListModel* deviceListModel READ getDeviceListModel NOTIFY deviceListModelChanged)
    Q_PROPERTY(QSortFilterProxyModel* deviceFilterModel READ getDeviceFilterModel CONSTANT)
    Q_PROPERTY(int selectedDeviceId READ getSelectedDeviceId NOTIFY selectedDeviceChanged)
    Q_PROPERTY(QString databaseMode           READ getDatabaseMode           NOTIFY databaseModeChanged)
    Q_PROPERTY(QString appReleaseDate         READ getAppReleaseDate         CONSTANT)
    Q_PROPERTY(QString databaseSchemaVersion  READ getDatabaseSchemaVersion  NOTIFY databaseModeChanged FINAL)
    Q_PROPERTY(QString imageFolderPath        READ getImageFolderPath        WRITE setImageFolderPath   NOTIFY imageFolderPathChanged)
    Q_PROPERTY(bool canExpandDevices READ canExpandDevices NOTIFY deviceExpandLevelChanged)
    Q_PROPERTY(bool canCollapseDevices READ canCollapseDevices NOTIFY deviceExpandLevelChanged)
    Q_PROPERTY(bool showDeviceInfo READ getShowDeviceInfo WRITE setShowDeviceInfo NOTIFY showDeviceInfoChanged)
    Q_PROPERTY(bool searchKeepsSelection READ getSearchKeepsSelection WRITE setSearchKeepsSelection NOTIFY searchKeepsSelectionChanged)
    Q_PROPERTY(bool checkVersionChoice READ getCheckVersionChoice WRITE setCheckVersionChoice NOTIFY checkVersionChoiceChanged)
    Q_PROPERTY(QVariantList recentCollections READ getRecentCollections NOTIFY recentCollectionsChanged)
    Q_PROPERTY(QString currentCollectionDisplayName READ getCurrentCollectionDisplayName NOTIFY recentCollectionsChanged)
    Q_PROPERTY(QString currentCollectionIconName    READ getCurrentCollectionIconName    NOTIFY recentCollectionsChanged)
    Q_PROPERTY(bool    searchIsRunning   READ getSearchIsRunning   NOTIFY searchStateChanged)
    Q_PROPERTY(QString searchStatusText  READ getSearchStatusText  NOTIFY searchStatusTextChanged)
    Q_PROPERTY(bool    catalogIsCreating READ getCatalogIsCreating NOTIFY catalogIsCreatingChanged)
    Q_PROPERTY(QString catalogStatusText READ getCatalogStatusText NOTIFY catalogStatusTextChanged)

public:
    explicit AppManager(QObject *parent = nullptr);

    //Application version
    QString currentVersion;
    QString releaseDate;
    bool checkVersionChoice;
    bool firstRun;
    bool developmentMode;
    int themeID;

    //Objects
    Collection *collection = new Collection();
    Device *selectedDevice = new Device(); //selected device from selection panel, used for operations on any screen
    Device *activeDevice   = new Device(); //active device from any screen, used for operations from that screen
    Device *catalogDevice  = new Device(); //selected catalog/device from Catalog screen
    Device *exploreDevice  = new Device(); //tempory catalog/device to be use in Exploore screen
    SearchSync *searchObject = nullptr;
    DeviceListModel *deviceListModel = nullptr;
    QSortFilterProxyModel *m_deviceFilterModel = nullptr;
    QString lastDatabaseError;
    int m_deviceExpandLevel = -1; // -1=show all; 0..N=show levels 0..N

public slots:
    //Global
    void initiateApp();
    void checkVersion();

    // Database selection methods
    void selectSQLiteDatabase();
    QString getDatabaseFilePath() const;
    void setDatabaseFilePath(const QString &path);
    bool reconnectToDatabase();
    QString getCurrentDatabaseInfo() const;
    void initializeDeviceListModel();
    void refreshAllUI();
    void refreshDeviceList();
    void refreshSearchResults();
    void refreshStatistics();

    //Database
    QString startDatabase();
    QString testQuery();

    // Collection management
    Q_INVOKABLE QString getDatabaseMode() const;
    Q_INVOKABLE QString getCollectionFolder() const;
    Q_INVOKABLE void openCollectionMemory(const QString &folder);
    Q_INVOKABLE void openCollectionHosted(const QString &hostName, const QString &dbName,
                                          int port, const QString &userName, const QString &password);

    //Useful functions
    void executeSearch();
    void setSearchObject(SearchSync *search);

    DeviceListModel* getDeviceListModel() const { return deviceListModel; }
    QSortFilterProxyModel* getDeviceFilterModel() const { return m_deviceFilterModel; }

    Q_INVOKABLE void setDeviceFilter(const QString &text);
    Q_INVOKABLE void expandDevices();
    Q_INVOKABLE void collapseDevices();
    Q_INVOKABLE void expandDevice(int deviceId);
    Q_INVOKABLE void collapseDevice(int deviceId);
    Q_INVOKABLE void openDeviceFolder(int deviceId);
    Q_INVOKABLE bool canExpandDevices() const;
    Q_INVOKABLE bool canCollapseDevices() const;
    bool getShowDeviceInfo() const;
    void setShowDeviceInfo(bool value);
    bool getSearchKeepsSelection() const;
    void setSearchKeepsSelection(bool value);
    QString getAppReleaseDate() const { return releaseDate; }
    bool getCheckVersionChoice() const;
    void setCheckVersionChoice(bool value);
    Q_INVOKABLE void openSettingsFile();
    Q_INVOKABLE QString getDatabaseSchemaVersion();
    Q_INVOKABLE void    setLastPage(const QString &pageName);
    Q_INVOKABLE QString getLastPage() const;

    void selectDeviceById(int deviceId);
    QString getSelectedDeviceName() const;
    int getSelectedDeviceId() const;

    Q_INVOKABLE QStringList getTagNames() const;

    // Search progress
    bool    getSearchIsRunning()  const { return m_searchIsRunning; }
    QString getSearchStatusText() const { return m_searchStatusText; }
    void onSearchProgress(int filesProcessed);

    // Catalog creation progress
    bool    getCatalogIsCreating()  const { return m_catalogIsCreating; }
    QString getCatalogStatusText()  const { return m_catalogStatusText; }

    // Catalog creation
    Q_INVOKABLE QString      createCatalog(const QString &name, const QString &path,
                                           int storageId,
                                           const QString &fileType, bool includeSubDir,
                                           bool includeHidden, bool includeSymlinks,
                                           bool isFullDevice,
                                           const QString &includeMetadata,
                                           const QString &includeChecksum,
                                           const QStringList &perCatalogExcludes);
    Q_INVOKABLE void         stopCatalogCreation();
    Q_INVOKABLE bool         isDirectoryEmpty(const QString &path) const;

    // Storage pre-selection for Create page (used by DeviceTreeComboBox storageOnly mode)
    Q_INVOKABLE int getDefaultStorageId() const;

    // Device edit
    Q_INVOKABLE QVariantMap  getDeviceDetails(int deviceId) const;
    Q_INVOKABLE QString      saveDeviceBasicFields(int deviceId, const QString &name, int parentId, const QString &path);
    Q_INVOKABLE QVariantMap  checkCatalogOptionChanges(int deviceId, const QString &fileType, bool includeHidden,
                                                       const QString &includeMetadata, const QString &includeChecksum,
                                                       bool isFullDevice) const;
    Q_INVOKABLE QString      saveCatalogOptions(int deviceId, const QString &fileType, bool includeHidden,
                                                const QString &includeMetadata, const QString &includeChecksum,
                                                bool isFullDevice);
    Q_INVOKABLE QString      saveStorageDetails(int deviceId, const QVariantMap &fields);
    Q_INVOKABLE void         triggerDeviceRescan(int deviceId);
    Q_INVOKABLE void         triggerStoragePathReplace(int deviceId, const QString &previousPath, const QString &newPath);
    Q_INVOKABLE QStringList  getDeviceExcludeFolders(int deviceId) const;
    Q_INVOKABLE bool         addDeviceExcludeFolder(int deviceId, const QString &path);
    Q_INVOKABLE bool         removeDeviceExcludeFolder(int deviceId, const QString &path);

    // Device delete
    Q_INVOKABLE QVariantMap  checkDeviceDeleteAllowed(int deviceId) const;
    Q_INVOKABLE QString      deleteDevice(int deviceId);

    // Storage helpers
    Q_INVOKABLE QStringList  getStoragePictureList() const;
    Q_INVOKABLE QString      getStorageImageFolderPath() const;
    Q_INVOKABLE QString      formatDataSize(qlonglong bytes) const;
    Q_INVOKABLE QVariantMap  refreshStorageFromDisk(int deviceId);

    // URL / path conversion helpers (cross-platform, wrap QUrl)
    Q_INVOKABLE QString      pathToFileUrl(const QString &path) const;
    Q_INVOKABLE QString      pathFromFileUrl(const QString &url) const;

    // Exclude directories (collection-level)
    Q_INVOKABLE QStringList  getExcludeDirectories() const;
    Q_INVOKABLE bool         addExcludeDirectory(const QString &path);
    Q_INVOKABLE void         removeExcludeDirectory(const QString &path);

    // Search history
    Q_INVOKABLE QVariantList getSearchHistory() const;
    Q_INVOKABLE QVariantMap  restoreSearchHistory(const QString &dateTime);
    Q_INVOKABLE void         clearSearchHistory();
    Q_INVOKABLE void         keepLastSearchHistory(int count);

    // File / folder operations from results
    Q_INVOKABLE void    openFile(const QString &filePath);
    Q_INVOKABLE void    openFolder(const QString &folderPath);
    Q_INVOKABLE void    copyToClipboard(const QString &text);
    Q_INVOKABLE QString exportSearchResultsToCSV();
    Q_INVOKABLE QString exportSearchResultsAsCatalog();
    Q_INVOKABLE int          batchMoveSearchResultsToTrash();
    Q_INVOKABLE int          batchDeleteSearchResults();
    Q_INVOKABLE QVariantMap  batchVerifyChecksums();
    Q_INVOKABLE QVariantMap  batchGetMetadata();

    // Single-file operations from context menu
    Q_INVOKABLE bool    moveFileToTrash(const QString &fullPath);
    Q_INVOKABLE bool    deleteSingleFile(const QString &fullPath);
    Q_INVOKABLE bool         catalogIncludesExtendedMetadata(int catalogId);
    Q_INVOKABLE QString      getFileMetadataJson(int catalogId, const QString &fileName, const QString &folderPath);
    Q_INVOKABLE QVariantList getFileMetadataParsedFields(int catalogId, const QString &fileName, const QString &folderPath);
    Q_INVOKABLE QString calculateAndSaveChecksum(const QString &filePath, const QString &fileName, const QString &folderPath, int catalogId);
    Q_INVOKABLE QString verifyFileChecksum(const QString &filePath, const QString &expectedChecksum);

    Q_INVOKABLE bool    shouldShowAlphaWarning() const;
    Q_INVOKABLE void    setAlphaWarningShown();

    // Recent collections
    QVariantList getRecentCollections() const;
    Q_INVOKABLE void openRecentCollection(const QVariantMap &entry);
    QString getCurrentCollectionDisplayName() const;
    QString getCurrentCollectionIconName() const;

    // Hosted database settings
    Q_INVOKABLE QString getHostName() const;
    Q_INVOKABLE QString getDatabaseName() const;
    Q_INVOKABLE int     getDatabasePort() const;
    Q_INVOKABLE QString getDatabaseUserName() const;
    Q_INVOKABLE QString getDatabasePassword() const;
    Q_INVOKABLE bool    getHostedAutoConnect() const;
    Q_INVOKABLE void    setHostedAutoConnect(bool value);

    // Image folder
    Q_INVOKABLE QString getImageFolderPath() const;
    Q_INVOKABLE void    setImageFolderPath(const QString &path);

    // Collection import
    Q_INVOKABLE QStringList  getImportSourcePaths() const;
    Q_INVOKABLE QVariantList openImportSource(const QString &path);
    Q_INVOKABLE QString      importDevice(int srcDeviceId);
    Q_INVOKABLE QString      updateAllImportsFromSource(const QString &path);

    // Language
    Q_INVOKABLE QVariantList getLanguageList() const;
    Q_INVOKABLE QString      getCurrentLanguage() const;
    Q_INVOKABLE void         setLanguage(const QString &languageCode);

signals:
    void searchStateChanged();
    void searchStatusTextChanged();
    void catalogIsCreatingChanged();
    void catalogStatusTextChanged();
    void catalogCreationCompleted(bool success, const QString &report);
    void excludeDirectoriesChanged();
    void deviceListModelChanged();
    void deviceExpandLevelChanged();
    void showDeviceInfoChanged();
    void searchKeepsSelectionChanged();
    void checkVersionChoiceChanged();
    void databasePathChanged(const QString &newPath);
    void databaseConnectionChanged(bool success, const QString &message);
    void deviceListRefreshed();
    void searchResultsRefreshed();
    void statisticsRefreshed();
    void uiRefreshCompleted();
    void selectedDeviceChanged(int deviceId);
    void databaseModeChanged();
    void recentCollectionsChanged();
    void languageChanged(const QString &code);
    void imageFolderPathChanged();
    void importSourceChanged();

private:
    bool    m_searchIsRunning  = false;
    QString m_searchStatusText;
    bool    m_catalogIsCreating = false;
    QString m_catalogStatusText;
    Device *m_creatingDevice   = nullptr;
    QDateTime m_catalogCreateStartTime;
    DeviceUpdateManager    *m_deviceUpdateManager    = nullptr;
    CatalogProgressManager *m_catalogProgressManager = nullptr;
    CollectionImporter     *m_importer               = nullptr;

    void setupDeviceUpdateManager();
    void onCatalogCreationCompleted(const QList<qint64> &results);
    void onCatalogCreationError(const QString &error);
    void onCatalogCreationCancelled();

    void saveToRecentCollections(const QString &mode, const QString &path,
                                 const QString &displayName,
                                 const QString &hostName = QString(),
                                 const QString &dbName   = QString(),
                                 int port = 3306,
                                 const QString &userName = QString(),
                                 const QString &password = QString());
};

#endif // APPMANAGER_H
