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
#include "adapters/search.h"
#include "adapters/devicelistmodel.h"

class AppManager : public QObject
{
    Q_OBJECT
    Q_PROPERTY(DeviceListModel* deviceListModel READ getDeviceListModel NOTIFY deviceListModelChanged)
    Q_PROPERTY(QSortFilterProxyModel* deviceFilterModel READ getDeviceFilterModel CONSTANT)
    Q_PROPERTY(int selectedDeviceId READ getSelectedDeviceId NOTIFY selectedDeviceChanged)
    Q_PROPERTY(QString databaseMode READ getDatabaseMode NOTIFY databaseModeChanged)
    Q_PROPERTY(bool canExpandDevices READ canExpandDevices NOTIFY deviceExpandLevelChanged)
    Q_PROPERTY(bool canCollapseDevices READ canCollapseDevices NOTIFY deviceExpandLevelChanged)
    Q_PROPERTY(bool showDeviceInfo READ getShowDeviceInfo WRITE setShowDeviceInfo NOTIFY showDeviceInfoChanged)

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

    void selectDeviceById(int deviceId);
    QString getSelectedDeviceName() const;
    int getSelectedDeviceId() const;

    Q_INVOKABLE bool    shouldShowAlphaWarning() const;
    Q_INVOKABLE void    setAlphaWarningShown();

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
    void databasePathChanged(const QString &newPath);
    void databaseConnectionChanged(bool success, const QString &message);
    void deviceListRefreshed();
    void searchResultsRefreshed();
    void statisticsRefreshed();
    void uiRefreshCompleted();
    void selectedDeviceChanged(int deviceId);
    void databaseModeChanged();
};

#endif // APPMANAGER_H
