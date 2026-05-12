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
    case HasChildrenRole:
        return device.hasChildren;
    case IsCollapsedRole:
        return device.isCollapsed;
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
    roles[HasChildrenRole] = "hasChildren";
    roles[IsCollapsedRole] = "isCollapsed";
    return roles;
}

QString DeviceListModel::formatDescription(const DeviceItem &device) const
{
    QLocale locale;
    QStringList lines;

    // Files line — shown for all types that have file data
    if (device.totalFileCount > 0) {
        QString filesLine = QString("%1 files").arg(locale.toString(device.totalFileCount));
        if (device.totalFileSize > 0)
            filesLine += QString(" · %1").arg(locale.formattedDataSize(device.totalFileSize));
        // Space details — Storage and Virtual only
        if ((device.type == "Storage" || device.type == "Virtual") && device.totalSpace > 0) {
            qint64 usedSpace = device.totalSpace - device.freeSpace;
            filesLine += QString("  · %1 · %2")
                             .arg(locale.formattedDataSize(usedSpace))
                             //.arg(locale.formattedDataSize(device.freeSpace))
                             .arg(locale.formattedDataSize(device.totalSpace));
        }
        lines << filesLine;
    }

    return lines.join("\n");
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

void DeviceListModel::loadFromConnection(const QString &connectionName)
{
    m_connectionName = connectionName;
    refreshDataSilently();
}
//----------------------------------------------------------------------
void DeviceListModel::setIncludeCollectionRoot(bool value)
{
    m_includeCollectionRoot = value;
}
//----------------------------------------------------------------------
void DeviceListModel::clear()
{
    m_connectionName.clear();
    beginResetModel();
    m_devices.clear();
    endResetModel();
}
//----------------------------------------------------------------------
void DeviceListModel::loadDevicesFromDatabase()
{
    QSqlDatabase db = m_connectionName.isEmpty()
                      ? QSqlDatabase::database()
                      : QSqlDatabase::database(m_connectionName);
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
        device.id            = query.value(0).toInt();
        int parentId         = query.value(1).toInt();
        device.name          = query.value(2).toString();
        device.type          = query.value(3).toString();
        device.totalFileSize  = query.value(4).toLongLong();
        device.totalFileCount = query.value(5).toLongLong();
        device.totalSpace    = query.value(6).toLongLong();
        device.freeSpace     = query.value(7).toLongLong();
        device.isActive      = query.value(8).toBool();
        childrenOf[parentId].append(device);
    }

    // Depth-first traversal → flat list in correct visual order with levels.
    // Children are visited when:
    //   1. device is NOT in m_collapsedIds (explicit collapse wins over everything), AND
    //   2. device IS in m_expandedIds (explicit expand overrides level), OR level allows it
    std::function<void(int, int)> traverse = [&](int parentId, int level) {
        for (DeviceItem &dev : childrenOf[parentId]) {
            dev.level       = level;
            dev.hasChildren = childrenOf.contains(dev.id) && !childrenOf[dev.id].isEmpty();

            bool explicitlyCollapsed = m_collapsedIds.contains(dev.id);
            bool explicitlyExpanded  = m_expandedIds.contains(dev.id);
            bool levelAllows         = (m_maxLevel == -1) || (level < m_maxLevel);
            bool recurse             = !explicitlyCollapsed && (explicitlyExpanded || levelAllows);

            dev.isCollapsed = dev.hasChildren && !recurse;
            dev.description = formatDescription(dev);
            m_devices.append(dev);
            if (recurse)
                traverse(dev.id, level + 1);
        }
    };
    traverse(0, 0);

    if (m_includeCollectionRoot) {
        DeviceItem root;
        root.id           = 0;
        root.level        = 0;
        root.name         = tr("Collection");
        root.type         = "Virtual";
        root.isActive     = true;
        root.hasChildren  = !m_devices.isEmpty();
        root.isCollapsed  = false;
        m_devices.prepend(root);
    }
}
//----------------------------------------------------------------------
void DeviceListModel::setMaxLevel(int level)
{
    m_maxLevel = level;
    refreshData();
}
//----------------------------------------------------------------------
void DeviceListModel::clearCollapsedUpToLevel(int maxLevel)
{
    if (maxLevel == -1) {
        m_collapsedIds.clear();
        return;
    }
    // A device at level L has children at L+1. Remove it from m_collapsedIds
    // when the new level allows L+1, i.e. when L < maxLevel.
    for (const DeviceItem &dev : m_devices) {
        if (dev.level < maxLevel)
            m_collapsedIds.remove(dev.id);
    }
}
//----------------------------------------------------------------------
void DeviceListModel::collapseDevice(int deviceId)
{
    m_collapsedIds.insert(deviceId);
    m_expandedIds.remove(deviceId);
    refreshData();
}
//----------------------------------------------------------------------
void DeviceListModel::expandDevice(int deviceId)
{
    m_expandedIds.insert(deviceId);
    m_collapsedIds.remove(deviceId);
    refreshData();
}
