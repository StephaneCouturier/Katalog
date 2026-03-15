#ifndef DEVICELISTMODEL_H
#define DEVICELISTMODEL_H

#include <QAbstractListModel>
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
        LevelRole
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
    bool hasData() const;
    QString getRefreshStatus() const;

private:
    struct DeviceItem {
        int id;
        int level = 0;
        QString type;
        QString name;
        QString description;
        bool isActive;
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
};

#endif // DEVICELISTMODEL_H
