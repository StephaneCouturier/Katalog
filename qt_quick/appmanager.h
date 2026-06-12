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
#include "core/statistics.h"
#include "core/backupmappingmanager.h"
#include "core/catalogdifferenceengine.h"
#include "adapters/search.h"
#include "adapters/devicelistmodel.h"
#include "adapters/explorefilesmodel.h"
#include "filesview.h"
#include <QElapsedTimer>

class BackupJobStoppable;
class QThread;

class AppManager : public QObject
{
    Q_OBJECT
    Q_PROPERTY(DeviceListModel* deviceListModel READ getDeviceListModel NOTIFY deviceListModelChanged)
    Q_PROPERTY(QSortFilterProxyModel* deviceFilterModel  READ getDeviceFilterModel  CONSTANT)
    Q_PROPERTY(QAbstractItemModel*    searchSortModel   READ getSearchSortModel    CONSTANT)
    Q_PROPERTY(QAbstractItemModel*    exploreSortModel  READ getExploreSortModel   CONSTANT)
    Q_PROPERTY(int     selectedDeviceId   READ getSelectedDeviceId   NOTIFY selectedDeviceChanged)
    Q_PROPERTY(QString selectedDeviceType READ getSelectedDeviceType NOTIFY selectedDeviceChanged)
    Q_PROPERTY(QString selectedDevicePath READ getSelectedDevicePath NOTIFY selectedDeviceChanged)
    Q_PROPERTY(QString databaseMode           READ getDatabaseMode           NOTIFY databaseModeChanged)
    Q_PROPERTY(QString appReleaseDate         READ getAppReleaseDate         CONSTANT)
    Q_PROPERTY(QString databaseSchemaVersion  READ getDatabaseSchemaVersion  NOTIFY databaseModeChanged FINAL)
    Q_PROPERTY(QString imageFolderPath        READ getImageFolderPath        WRITE setImageFolderPath   NOTIFY imageFolderPathChanged)
    Q_PROPERTY(bool canExpandDevices READ canExpandDevices NOTIFY deviceExpandLevelChanged)
    Q_PROPERTY(bool canCollapseDevices READ canCollapseDevices NOTIFY deviceExpandLevelChanged)
    Q_PROPERTY(bool showDeviceInfo READ getShowDeviceInfo WRITE setShowDeviceInfo NOTIFY showDeviceInfoChanged)
    Q_PROPERTY(bool showSelectionPage READ getShowSelectionPage WRITE setShowSelectionPage NOTIFY showSelectionPageChanged)
    Q_PROPERTY(bool deviceFilterFromSelection READ getDeviceFilterFromSelection WRITE setDeviceFilterFromSelection NOTIFY deviceFilterFromSelectionChanged)
    Q_PROPERTY(bool searchKeepsSelection READ getSearchKeepsSelection WRITE setSearchKeepsSelection NOTIFY searchKeepsSelectionChanged)
    Q_PROPERTY(bool checkVersionChoice READ getCheckVersionChoice WRITE setCheckVersionChoice NOTIFY checkVersionChoiceChanged)
    Q_PROPERTY(bool fileSortCaseSensitive READ getFileSortCaseSensitive WRITE setFileSortCaseSensitive NOTIFY fileSortCaseSensitiveChanged)
    Q_PROPERTY(QVariantList recentCollections READ getRecentCollections NOTIFY recentCollectionsChanged)
    Q_PROPERTY(QString currentCollectionDisplayName READ getCurrentCollectionDisplayName NOTIFY recentCollectionsChanged)
    Q_PROPERTY(QString currentCollectionIconName    READ getCurrentCollectionIconName    NOTIFY recentCollectionsChanged)
    Q_PROPERTY(bool    searchIsRunning   READ getSearchIsRunning   NOTIFY searchStateChanged)
    Q_PROPERTY(bool    searchIsPaused    READ getSearchIsPaused    NOTIFY searchStateChanged)
    Q_PROPERTY(QString searchStatusText  READ getSearchStatusText  NOTIFY searchStatusTextChanged)
    Q_PROPERTY(bool    catalogIsCreating READ getCatalogIsCreating NOTIFY catalogIsCreatingChanged)
    Q_PROPERTY(QString catalogStatusText READ getCatalogStatusText NOTIFY catalogStatusTextChanged)
    Q_PROPERTY(bool    importIsRunning   READ getImportIsRunning   NOTIFY importIsRunningChanged)
    Q_PROPERTY(QString importStatusText  READ getImportStatusText  NOTIFY importStatusTextChanged)
    Q_PROPERTY(DeviceListModel* importSourceDeviceModel READ getImportSourceDeviceModel NOTIFY importSourceChanged)
    Q_PROPERTY(bool updateBeforeBackup READ updateBeforeBackup WRITE setUpdateBeforeBackup NOTIFY updateBeforeBackupChanged)
    Q_PROPERTY(bool catalogUpdateForBackupRunning READ catalogUpdateForBackupRunning NOTIFY catalogUpdateForBackupRunningChanged)
    Q_PROPERTY(bool    deviceUpdateIsRunning  READ getDeviceUpdateIsRunning  NOTIFY deviceUpdateStateChanged)
    Q_PROPERTY(QString deviceUpdateStatusText READ getDeviceUpdateStatusText NOTIFY deviceUpdateStatusChanged)
    Q_PROPERTY(bool    isFirstRun             READ isFirstRun                NOTIFY firstRunChanged)

public:
    explicit AppManager(QObject *parent = nullptr);

    //Application version
    QString currentVersion;
    QString releaseDate;
    bool checkVersionChoice;
    bool m_fileSortCaseSensitive = false;
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
    Q_INVOKABLE void openDatabaseFile();
    Q_INVOKABLE void openCollectionMemory(const QString &folder);
    Q_INVOKABLE void openCollectionHosted(const QString &hostName, const QString &dbName,
                                          int port, const QString &userName, const QString &password);
    Q_INVOKABLE void    createNewSQLiteCollection(const QString &path);
    Q_INVOKABLE QString getNewCollectionDefaultPath() const;

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
    bool getShowSelectionPage() const;
    void setShowSelectionPage(bool value);
    bool getDeviceFilterFromSelection() const;
    void setDeviceFilterFromSelection(bool value);
    bool getSearchKeepsSelection() const;
    void setSearchKeepsSelection(bool value);
    QString getAppReleaseDate() const { return releaseDate; }
    bool getCheckVersionChoice() const;
    void setCheckVersionChoice(bool value);
    bool getFileSortCaseSensitive() const;
    void setFileSortCaseSensitive(bool value);
    QString getSelectedDevicePath() const;
    Q_INVOKABLE void    openSettingsFile();
    Q_INVOKABLE QString getSettingsFilePath() const;
    Q_INVOKABLE QString getDatabaseSchemaVersion();
    Q_INVOKABLE void    setLastPage(const QString &pageName);
    Q_INVOKABLE QString getLastPage() const;

    void selectDeviceById(int deviceId);
    QString getSelectedDeviceName() const;
    int     getSelectedDeviceId()   const;
    QString getSelectedDeviceType() const;

    Q_INVOKABLE QStringList  getTagNames() const;

    // Statistics
    Q_INVOKABLE QVariantMap getStatisticsData(const QString &source, const QString &dataType, const QString &startDate) const;
    Q_INVOKABLE QString     getStatisticsSetting(const QString &key) const;
    Q_INVOKABLE void        setStatisticsSetting(const QString &key, const QVariant &value);

    // Backup
    Q_INVOKABLE QVariantList getBackupMappings(const QString &filterType = "None", int deviceId = -1, const QString &mappingType = "All") const;
    Q_INVOKABLE QVariantMap  getBackupTotals(const QString &filterType = "None", int deviceId = -1, const QString &mappingType = "All") const;
    Q_INVOKABLE QString      createBackupMapping(const QString &name, const QString &type, int sourceId, int targetId, bool strictCopy, const QString &conflictMode, bool sourceDrive);
    Q_INVOKABLE bool         deleteBackupMapping(int mappingId);
    Q_INVOKABLE bool         invertBackupMapping(int mappingId);
    Q_INVOKABLE QVariantMap  previewBackup(int mappingId);
    Q_INVOKABLE void         runBackup(int mappingId);
    Q_INVOKABLE void         stopBackup();
    Q_INVOKABLE void         pauseBackup();
    Q_INVOKABLE void         resumeBackup();
    Q_INVOKABLE QVariantMap  replicateDirectories(int mappingId);
    Q_INVOKABLE QString      exportLastBackupPreviewToCsv();
    Q_INVOKABLE QString      generateLuckyBackupProfile(const QVariantList &mappingIds);
    Q_INVOKABLE QString      getBackupSetting(const QString &key) const;
    Q_INVOKABLE void         setBackupSetting(const QString &key, const QVariant &value);

    // Catalog-update-before-backup
    bool updateBeforeBackup() const;
    void setUpdateBeforeBackup(bool v);
    bool catalogUpdateForBackupRunning() const { return m_catalogUpdateForBackupRunning; }
    Q_INVOKABLE void prepareCatalogsForMapping(int mappingId);

    Q_INVOKABLE QVariantList getTagEntries(const QString &filterName = QString()) const;
    Q_INVOKABLE bool         createTag(const QString &name, const QString &path);
    Q_INVOKABLE bool         deleteTag(int tagID);

    // Search progress
    bool    getSearchIsRunning()  const { return m_searchIsRunning; }
    bool    getSearchIsPaused()   const { return m_searchIsPaused; }
    QString getSearchStatusText() const { return m_searchStatusText; }
    void onSearchProgress(int filesProcessed);
    Q_INVOKABLE void stopSearch();
    Q_INVOKABLE void pauseSearch();
    Q_INVOKABLE void resumeSearch();

    // Catalog creation progress
    bool    getCatalogIsCreating()  const { return m_catalogIsCreating; }
    QString getCatalogStatusText()  const { return m_catalogStatusText; }

    // Import progress
    bool    getImportIsRunning()  const { return m_importIsRunning; }
    QString getImportStatusText() const { return m_importStatusText; }

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

    // Devices page operations
    Q_INVOKABLE QVariantList getDeviceList(const QString &viewFilter = "All", int scopeDeviceId = 0) const;
    Q_INVOKABLE int          addDeviceVirtual(int parentId);
    Q_INVOKABLE int          addDeviceStorage(int parentId);
    Q_INVOKABLE void         updateDevice(int deviceId);
    Q_INVOKABLE void         updateAllActiveDevices(bool showEachReport = false);
    Q_INVOKABLE void         acknowledgeUpdateReport();
    Q_INVOKABLE void         stopDeviceUpdate();
    Q_INVOKABLE void         gentleStopDeviceUpdate();
    Q_INVOKABLE QVariantMap  recordDevicesSnapshot();
    Q_INVOKABLE QString      splitCatalogBySubDirectory(int deviceId);
    Q_INVOKABLE void         splitCatalogByFileType(int deviceId, bool verifyFirst);
    Q_INVOKABLE QVariantMap  verifyDeviceChecksums(int deviceId);
    Q_INVOKABLE QString      unassignDevice(int deviceId, int parentId);
    Q_INVOKABLE QString      assignCatalogToDevice(int catalogDeviceId, int virtualDeviceId);
    Q_INVOKABLE void         launchFilelight(int deviceId);
    Q_INVOKABLE QString      importFromVVV(const QString &path);
    bool    getDeviceUpdateIsRunning()  const { return m_deviceUpdateIsRunning; }
    QString getDeviceUpdateStatusText() const { return m_deviceUpdateStatusText; }

    // Explore page
    Q_INVOKABLE QVariantMap  exploreOpenCatalog(int deviceId);
    Q_INVOKABLE QVariantList getExploreFolders();
    Q_INVOKABLE QVariantList getExploreEntries(const QString &folderPath, bool showFolders, bool showSubFolders);
    Q_INVOKABLE void         loadExploreEntries(const QString &folderPath, bool showFolders, bool showSubFolders);
    Q_INVOKABLE void         exploreRemoveRow(int proxyRow);
    Q_INVOKABLE QVariantMap  getExploreFolderStats(const QString &folderPath);
    Q_INVOKABLE QString      exploreGetChecksum(const QString &fileName, const QString &folderPath);

    // Sort
    Q_INVOKABLE void sortSearch(int column, int order);
    Q_INVOKABLE void sortExplore(int column, int order);
    Q_INVOKABLE int  getSearchSortColumn()  const;
    Q_INVOKABLE int  getSearchSortOrder()   const;
    Q_INVOKABLE int  getExploreSortColumn() const;
    Q_INVOKABLE int  getExploreSortOrder()  const;

    // Sort model accessors (for Q_PROPERTY)
    QAbstractItemModel *getSearchSortModel()  const { return m_searchSortModel; }
    QAbstractItemModel *getExploreSortModel() const { return m_exploreSortModel; }

    // Storage helpers
    Q_INVOKABLE QStringList  getStoragePictureList() const;
    Q_INVOKABLE QString      getStorageImageFolderPath() const;
    Q_INVOKABLE QString      formatDataSize(qlonglong bytes) const;
    Q_INVOKABLE QString      formatDataSizeDelta(qlonglong bytes) const;
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
    bool                isFirstRun() const { return m_firstRun; }
    Q_INVOKABLE void    clearFirstRun();

    // Recent collections
    QVariantList getRecentCollections() const;
    Q_INVOKABLE void openRecentCollection(const QVariantMap &entry);
    QString getCurrentCollectionDisplayName() const;
    QString getCurrentCollectionIconName() const;

    // Hosted database settings
    Q_INVOKABLE QString getHostName() const;
    Q_INVOKABLE QString getDatabaseName() const;
    Q_INVOKABLE QString getPhpMyAdminUrl() const;
    Q_INVOKABLE void    setPhpMyAdminUrl(const QString &url);
    Q_INVOKABLE int     getDatabasePort() const;
    Q_INVOKABLE QString getDatabaseUserName() const;
    Q_INVOKABLE QString getDatabasePassword() const;
    Q_INVOKABLE bool    getHostedAutoConnect() const;
    Q_INVOKABLE void    setHostedAutoConnect(bool value);

    // Image folder
    Q_INVOKABLE QString getImageFolderPath() const;
    Q_INVOKABLE void    setImageFolderPath(const QString &path);

    // Collection import
    Q_INVOKABLE QStringList       getImportSourcePaths() const;
    Q_INVOKABLE void              openImportSource(const QString &path);
    Q_INVOKABLE QString           importDevice(int srcDeviceId);
    DeviceListModel              *getImportSourceDeviceModel() const { return m_importSourceDeviceModel; }
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
    void showSelectionPageChanged();
    void searchKeepsSelectionChanged();
    void deviceFilterFromSelectionChanged();
    void checkVersionChoiceChanged();
    void fileSortCaseSensitiveChanged();
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
    void importIsRunningChanged();
    void importStatusTextChanged();
    void tagsChanged();
    void backupMappingsChanged();
    void backupProgress(int filesDone, int totalFiles, qint64 bytesCopied, qint64 totalBytes, const QString &currentFile);
    void backupFinished(int copiedCount, int movedCount, int renamedCount, int conflictCount, int errorCount, qint64 totalBytesCopied, bool wasCancelled);
    void backupNotification(const QString &message, bool isError);
    void updateBeforeBackupChanged();
    void catalogUpdateForBackupRunningChanged();
    void catalogsForMappingPrepared(int mappingId, bool success, const QString &error);
    void deviceUpdateStateChanged();
    void deviceUpdateStatusChanged();
    void deviceUpdateReportReady(QVariantMap report);
    void firstRunChanged();
    void deviceListChanged();
    void checksumVerificationCompleted(QVariantMap result);
    void splitCompleted(bool success, const QString &error);

private:
    QString m_connectionName = "defaultConnection";
    bool    m_firstRun = false;
    bool    m_searchIsRunning  = false;
    bool    m_searchIsPaused   = false;
    QString m_searchStatusText;
    bool    m_catalogIsCreating = false;
    QString m_catalogStatusText;
    Device *m_creatingDevice   = nullptr;
    QDateTime m_catalogCreateStartTime;
    DeviceUpdateManager    *m_deviceUpdateManager    = nullptr;
    CatalogProgressManager *m_catalogProgressManager = nullptr;
    CollectionImporter     *m_importer               = nullptr;
    bool    m_importIsRunning  = false;
    QString m_importStatusText;
    DeviceListModel        *m_importSourceDeviceModel = nullptr;

    void setupDeviceUpdateManager();
    void onCatalogCreationCompleted(const QList<qint64> &results);
    void onCatalogCreationError(const QString &error);
    void onCatalogCreationCancelled();

    // Backup helpers
    struct BackupCompareResult {
        QList<DifferenceFileEntry> filesToCopy;
        QList<DifferenceFileEntry> fileConflicts;
        int skippedCount = 0;
    };
    BackupCompareResult compareForBackup(const Device &src, const Device &tgt, bool strictCopy, bool sourceDrive);
    void executeBackupJob(int mappingId);
    void onBackupProgressInternal(int filesDone, int totalFiles, qint64 bytesCopied, qint64 totalBytes, const QString &currentFile);
    void onBackupFinishedInternal(const BackupReport &report);

    BackupJobStoppable     *m_backupJob             = nullptr;
    QThread                *m_backupThread          = nullptr;
    bool                    m_backupIsArchive        = false;
    QElapsedTimer           m_backupTimer;
    Device                  m_backupTargetDevice;
    int                     m_runningBackupMappingId = -1;
    QList<BackupPreviewRow> m_lastPreviewRows;

    // Catalog-update-before-backup state
    enum class BackupCatalogUpdatePhase { None, UpdatingSource, UpdatingTarget };
    BackupCatalogUpdatePhase m_backupCatalogUpdatePhase       = BackupCatalogUpdatePhase::None;
    int                      m_pendingCatalogUpdateMappingId  = -1;
    int                      m_pendingBackupAfterUpdate       = -1;
    Device                   m_pendingCatalogUpdateSourceDevice;
    Device                   m_pendingCatalogUpdateTargetDevice;
    bool                     m_catalogUpdateForBackupRunning  = false;
    void setupCatalogUpdateForBackupConnections();
    void onCatalogUpdateForBackupStep();

    // Devices page state
    bool    m_deviceUpdateIsRunning  = false;
    QString m_deviceUpdateStatusText;
    QList<int> m_pendingDeviceUpdates;
    int     m_currentUpdateDeviceId  = 0;
    int     m_pendingBatchTotal      = 0;
    bool    m_isBatchUpdate          = false;
    bool    m_showEachUpdateReport   = false;
    bool    m_waitingForReportAck    = false;
    void setupDeviceUpdateManagerForDevices();
    void onDevicePageUpdateCompleted(const QList<qint64> &results);
    void startNextDeviceUpdate();
    QVariantMap buildUpdateReport(int deviceId, const QList<qint64> &results);
    QVariantMap buildBatchUpdateReport(const QList<qint64> &results);

    ExploreFilesModel *m_exploreFilesModel = nullptr;
    FilesView         *m_exploreSortModel  = nullptr;
    FilesView         *m_searchSortModel   = nullptr;

    void saveToRecentCollections(const QString &mode, const QString &path,
                                 const QString &displayName,
                                 const QString &hostName = QString(),
                                 const QString &dbName   = QString(),
                                 int port = 3306,
                                 const QString &userName = QString(),
                                 const QString &password = QString());
};

#endif // APPMANAGER_H
