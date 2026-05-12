#ifndef DEVICELISTMODEL_H
#define DEVICELISTMODEL_H

#include <QAbstractListModel>
#include <QSet>
#include <QSqlQuery>
#include <QSqlDatabase>
#include <qdatetime.h>

class DeviceListModel : public QAbstractListModel
{
    Q_OBJECT

public:
    enum DeviceRoles {
        TypeRole = Qt::UserRole + 1,
        NameRole,
        DescriptionRole,
        IsActiveRole,
        DeviceIdRole,
        LevelRole,
        HasChildrenRole,
        IsCollapsedRole
    };

    explicit DeviceListModel(QObject *parent = nullptr);

    // QAbstractListModel interface
    int rowCount(const QModelIndex &parent = QModelIndex()) const override;
    QVariant data(const QModelIndex &index, int role = Qt::DisplayRole) const override;
    QHash<int, QByteArray> roleNames() const override;

public slots:
    void refreshData();
    void refreshDataSilently(); // Refresh without UI notifications
    void setMaxLevel(int level); // -1 = all, 0 = top-level only
    void collapseDevice(int deviceId);
    void expandDevice(int deviceId);
    void clearCollapsedUpToLevel(int maxLevel); // -1 = clear all
    bool hasData() const;
    QString getRefreshStatus() const;

    // Named-connection support (for import source)
    void loadFromConnection(const QString &connectionName);
    void setIncludeCollectionRoot(bool value); // prepends synthetic id=0 "Collection" root
    void clear();

private:
    struct DeviceItem {
        int id;
        int level = 0;
        QString type;
        QString name;
        QString description;
        bool isActive;
        bool hasChildren  = false;
        bool isCollapsed  = false;
        qint64 totalFileSize  = 0;
        qint64 totalFileCount = 0;
        qint64 totalSpace     = 0;
        qint64 freeSpace      = 0;
    };

    QList<DeviceItem> m_devices;
    void loadDevicesFromDatabase();
    QString formatDescription(const DeviceItem &device) const;

signals:
    void refreshStarted();
    void refreshCompleted(int deviceCount);
    void refreshFailed(const QString &error);

private:
    QString m_lastError;
    QDateTime m_lastRefresh;
    int m_maxLevel = -1; // -1 = show all levels
    QSet<int> m_collapsedIds;  // device IDs explicitly collapsed (children hidden)
    QSet<int> m_expandedIds;   // device IDs explicitly expanded (override level filter)
    QString m_connectionName;          // empty = default Qt SQL connection
    bool m_includeCollectionRoot = false; // prepend synthetic id=0 "Collection" entry
};

#endif // DEVICELISTMODEL_H
