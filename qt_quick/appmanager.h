#ifndef APPMANAGER_H
#define APPMANAGER_H

#include <QObject>
#include <QDebug>

//QtCore
#include <QFile>
#include <QFileInfo>
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
#include "adapters/search.h"
#include "adapters/devicelistmodel.h"

class AppManager : public QObject
{
    Q_OBJECT
    Q_PROPERTY(DeviceListModel* deviceListModel READ getDeviceListModel NOTIFY deviceListModelChanged)
    Q_PROPERTY(QSortFilterProxyModel* deviceFilterModel READ getDeviceFilterModel CONSTANT)
    Q_PROPERTY(int selectedDeviceId READ getSelectedDeviceId NOTIFY selectedDeviceChanged)
    Q_PROPERTY(QString databaseMode           READ getDatabaseMode           NOTIFY databaseModeChanged)
    Q_PROPERTY(QString databaseSchemaVersion  READ getDatabaseSchemaVersion  NOTIFY databaseModeChanged FINAL)
    Q_PROPERTY(bool canExpandDevices READ canExpandDevices NOTIFY deviceExpandLevelChanged)
    Q_PROPERTY(bool canCollapseDevices READ canCollapseDevices NOTIFY deviceExpandLevelChanged)
    Q_PROPERTY(bool showDeviceInfo READ getShowDeviceInfo WRITE setShowDeviceInfo NOTIFY showDeviceInfoChanged)
    Q_PROPERTY(bool searchKeepsSelection READ getSearchKeepsSelection WRITE setSearchKeepsSelection NOTIFY searchKeepsSelectionChanged)
    Q_PROPERTY(bool checkVersionChoice READ getCheckVersionChoice WRITE setCheckVersionChoice NOTIFY checkVersionChoiceChanged)
    Q_PROPERTY(QVariantList recentCollections READ getRecentCollections NOTIFY recentCollectionsChanged)
    Q_PROPERTY(QString currentCollectionDisplayName READ getCurrentCollectionDisplayName NOTIFY recentCollectionsChanged)
    Q_PROPERTY(QString currentCollectionIconName    READ getCurrentCollectionIconName    NOTIFY recentCollectionsChanged)

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
    bool getCheckVersionChoice() const;
    void setCheckVersionChoice(bool value);
    Q_INVOKABLE void openSettingsFile();
    Q_INVOKABLE QString getDatabaseSchemaVersion();

    void selectDeviceById(int deviceId);
    QString getSelectedDeviceName() const;
    int getSelectedDeviceId() const;

    Q_INVOKABLE QStringList getTagNames() const;

    // File / folder operations from results
    Q_INVOKABLE void    openFile(const QString &filePath);
    Q_INVOKABLE void    openFolder(const QString &folderPath);
    Q_INVOKABLE void    copyToClipboard(const QString &text);
    Q_INVOKABLE QString exportSearchResultsToCSV();
    Q_INVOKABLE QString exportSearchResultsAsCatalog();
    Q_INVOKABLE int     batchMoveSearchResultsToTrash();
    Q_INVOKABLE int     batchDeleteSearchResults();

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

signals:
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

private:
    void saveToRecentCollections(const QString &mode, const QString &path,
                                 const QString &displayName,
                                 const QString &hostName = QString(),
                                 const QString &dbName   = QString(),
                                 int port = 3306,
                                 const QString &userName = QString(),
                                 const QString &password = QString());
};

#endif // APPMANAGER_H
