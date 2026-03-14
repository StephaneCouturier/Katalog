#include "devicelistmodel.h"
#include <QDebug>
#include <QLocale>
#include <functional>
#include <qdatetime.h>
#include <qsqlerror.h>

DeviceListModel::DeviceListModel(QObject *parent)
    : QAbstractListModel(parent)
{
    loadDevicesFromDatabase();
}

int DeviceListModel::rowCount(const QModelIndex &parent) const
{
    Q_UNUSED(parent)
    return m_devices.count();
}

QVariant DeviceListModel::data(const QModelIndex &index, int role) const
{
    if (!index.isValid() || index.row() >= m_devices.count()) {
        return QVariant();
    }

    const DeviceItem &device = m_devices.at(index.row());

    switch (role) {
    case TypeRole:
        return device.type;
    case NameRole:
        return device.name;
    case DescriptionRole:
        return device.description;
    case IsActiveRole:
        return device.isActive;
    case DeviceIdRole:
        return device.id;
    case LevelRole:
        return device.level;
    default:
        return QVariant();
    }
}

QHash<int, QByteArray> DeviceListModel::roleNames() const
{
    QHash<int, QByteArray> roles;
    roles[TypeRole] = "type";
    roles[NameRole] = "name";
    roles[DescriptionRole] = "description";
    roles[IsActiveRole] = "isActive";
    roles[DeviceIdRole] = "deviceId";
    roles[LevelRole] = "level";
    return roles;
}

QString DeviceListModel::formatDescription(const DeviceItem &device) const
{
    QSqlQuery query(QSqlDatabase::database("defaultConnection"));

    if (device.type == "Virtual") {
        // For virtual devices, show total space if available
        query.prepare("SELECT device_total_space FROM device WHERE device_id = :id");
        query.bindValue(":id", device.id);
        if (query.exec() && query.next()) {
            qint64 totalSpace = query.value(0).toLongLong();
            if (totalSpace > 0) {
                return QLocale().formattedDataSize(totalSpace);
            }
        }
        return "";
    }
    else if (device.type == "Storage") {
        // For storage devices, show total space
        query.prepare("SELECT device_total_space FROM device WHERE device_id = :id");
        query.bindValue(":id", device.id);
        if (query.exec() && query.next()) {
            qint64 totalSpace = query.value(0).toLongLong();
            if (totalSpace > 0) {
                return QLocale().formattedDataSize(totalSpace);
            }
        }
        return "";
    }
    else if (device.type == "Catalog") {
        // For catalogs, show file count and total size
        query.prepare("SELECT device_total_file_count, device_total_file_size FROM device WHERE device_id = :id");
        query.bindValue(":id", device.id);
        if (query.exec() && query.next()) {
            qint64 fileCount = query.value(0).toLongLong();
            qint64 totalSize = query.value(1).toLongLong();

            if (fileCount > 0 && totalSize > 0) {
                return QString("%1, %2 files")
                    .arg(QLocale().formattedDataSize(totalSize))
                    .arg(QLocale().toString(fileCount));
            } else if (fileCount > 0) {
                return QString("%1 files").arg(QLocale().toString(fileCount));
            }
        }
        return "";
    }
    return "";
}

void DeviceListModel::refreshData()
{
    emit refreshStarted();

    int oldCount = m_devices.count();

    beginResetModel();

    // Clear existing data
    m_devices.clear();
    m_lastError.clear();

    // Reload from database
    loadDevicesFromDatabase();

    endResetModel();

    int newCount = m_devices.count();
    m_lastRefresh = QDateTime::currentDateTime();

    if (!m_lastError.isEmpty()) {
        emit refreshFailed(m_lastError);
        qWarning() << "DeviceListModel refresh failed:" << m_lastError;
    } else {
        emit refreshCompleted(newCount);
        qDebug() << "DeviceListModel refresh completed. Devices:" << oldCount << "->" << newCount;
    }
}

void DeviceListModel::refreshDataSilently()
{
    // Refresh without emitting UI notification signals
    beginResetModel();
    m_devices.clear();
    loadDevicesFromDatabase();
    endResetModel();
    m_lastRefresh = QDateTime::currentDateTime();
}

bool DeviceListModel::hasData() const
{
    return !m_devices.isEmpty();
}

QString DeviceListModel::getRefreshStatus() const
{
    if (!m_lastError.isEmpty()) {
        return QString("Error: %1").arg(m_lastError);
    }

    if (m_lastRefresh.isValid()) {
        return QString("Last updated: %1").arg(m_lastRefresh.toString("hh:mm:ss"));
    }

    return "Not loaded";
}

void DeviceListModel::loadDevicesFromDatabase()
{
    // Enhanced error handling
    QSqlDatabase db = QSqlDatabase::database();
    if (!db.isOpen()) {
        m_lastError = "Database not connected";
        qWarning() << "DeviceListModel::loadDevicesFromDatabase:" << m_lastError;
        return;
    }

    // Load all devices flat — tree order built in C++ to avoid DB-specific SQL (|| vs CONCAT, CTE support)
    QSqlQuery query(db);
    query.prepare(QLatin1String(R"(
        SELECT  device_id,
                device_parent_id,
                device_name,
                device_type,
                device_total_file_size,
                device_total_file_count,
                device_total_space,
                device_free_space,
                device_active
        FROM device
        ORDER BY device_parent_id ASC, device_order ASC, device_name ASC
    )"));

    if (!query.exec()) {
        m_lastError = QString("Query failed: %1").arg(query.lastError().text());
        qWarning() << "DeviceListModel::loadDevicesFromDatabase:" << m_lastError;
        return;
    }

    // Build parent → children map (same pattern as K2 QMap<id, item>)
    QMap<int, QList<DeviceItem>> childrenOf;
    while (query.next()) {
        DeviceItem device;
        device.id       = query.value(0).toInt();
        int parentId    = query.value(1).toInt();
        device.name     = query.value(2).toString();
        device.type     = query.value(3).toString();
        device.isActive = query.value(8).toBool();
        childrenOf[parentId].append(device);
    }

    // Depth-first traversal → flat list in correct visual order with levels
    std::function<void(int, int)> traverse = [&](int parentId, int level) {
        for (DeviceItem &dev : childrenOf[parentId]) {
            dev.level = level;
            dev.description = formatDescription(dev);
            m_devices.append(dev);
            traverse(dev.id, level + 1);
        }
    };
    traverse(0, 0);
}
