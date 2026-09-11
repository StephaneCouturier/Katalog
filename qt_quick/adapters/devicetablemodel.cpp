/*LICENCE
    This file is part of Katalog

    Copyright (C) 2020, the Katalog Development team

    Author: Stephane Couturier (Symbioxy)

    Katalog is free software; you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation; either version 2 of the License, or
    (at your option) any later version.

    Katalog is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with Katalog; if not, write to the Free Software
    Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA
*/
/*FILE DESCRIPTION
/////////////////////////////////////////////////////////////////////////////
// Application: Katalog
// File Name:   devicetablemodel.cpp
// Purpose:     QAbstractTableModel for the Devices page Table display mode
// Description: See devicetablemodel.h.
// Author:      Stephane Couturier
/////////////////////////////////////////////////////////////////////////////
*/
#include "devicetablemodel.h"
#include "../../core/catalog.h"

#include <QCoreApplication>
#include <QLocale>

// Column tables. The order is K2's, so a user who knows the K2 Devices tab finds
// the same columns in the same places; the Full Table option reveals the ones K2
// hides rather than appending them at the end
// (qt_widgets/mainwindow_tab_device_pr.cpp:1596-1650 and :1845-1922).
// "header" holds K2's own source text byte-for-byte, wrapped in
// QT_TRANSLATE_NOOP so lupdate extracts it from the table and each one resolves
// to a translation that already exists in all 30 languages (DVP-C5).
static const QList<DeviceTableModel::Column> &storageColumns();
static const QList<DeviceTableModel::Column> &catalogColumns();

DeviceTableModel::DeviceTableModel(QObject *parent)
    : QAbstractTableModel(parent)
{
}

void DeviceTableModel::setView(const QString &view)
{
    if (m_view == view)
        return;
    beginResetModel();
    m_view = view;
    rebuildVisibleColumns();
    endResetModel();
}

void DeviceTableModel::setFullTable(bool fullTable)
{
    if (m_fullTable == fullTable)
        return;
    beginResetModel();
    m_fullTable = fullTable;
    rebuildVisibleColumns();
    endResetModel();
}

void DeviceTableModel::setMemoryMode(bool memoryMode)
{
    if (m_memoryMode == memoryMode)
        return;
    beginResetModel();
    m_memoryMode = memoryMode;
    rebuildVisibleColumns();
    endResetModel();
}

void DeviceTableModel::populate(const QVariantList &rows)
{
    beginResetModel();
    m_rows = rows;
    rebuildVisibleColumns();
    endResetModel();
}

void DeviceTableModel::clear()
{
    beginResetModel();
    m_rows.clear();
    endResetModel();
}

const QList<DeviceTableModel::Column> &DeviceTableModel::columnsForView() const
{
    if (m_view == QLatin1String("Storage"))
        return storageColumns();
    return catalogColumns();
}

void DeviceTableModel::rebuildVisibleColumns()
{
    m_visible.clear();
    const QList<Column> &all = columnsForView();
    for (int i = 0; i < all.size(); ++i) {
        const Column &c = all.at(i);
        if (c.fullOnly && !m_fullTable)
            continue;
        // Date Loaded and File Path describe the in-memory copy of a catalog and
        // are meaningless in File and Hosted mode, where the data is already in
        // the database - K2 hides them the same way.
        if (c.memoryOnly && !m_memoryMode)
            continue;
        m_visible.append(i);
    }
}

int DeviceTableModel::rowCount(const QModelIndex &parent) const
{
    if (parent.isValid())
        return 0;
    return m_rows.size();
}

int DeviceTableModel::columnCount(const QModelIndex &parent) const
{
    if (parent.isValid())
        return 0;
    return m_visible.size();
}

int DeviceTableModel::columnWidth(int column) const
{
    if (column < 0 || column >= m_visible.size())
        return 100;
    return columnsForView().at(m_visible.at(column)).width;
}

QVariant DeviceTableModel::data(const QModelIndex &index, int role) const
{
    if (!index.isValid() || index.row() < 0 || index.row() >= m_rows.size())
        return {};
    if (index.column() < 0 || index.column() >= m_visible.size())
        return {};

    const QVariantMap row = m_rows.at(index.row()).toMap();
    const Column &col     = columnsForView().at(m_visible.at(index.column()));
    const QVariant value  = row.value(QString::fromLatin1(col.key));
    const QString  type   = row.value(QStringLiteral("type")).toString();

    switch (role) {

    case Qt::DisplayRole:
        switch (col.kind) {
        case Kind::Name:
        case Kind::Text:
            return value.toString();
        case Kind::Number:
            return QLocale().toString(value.toLongLong());
        case Kind::Size:
            return QLocale().formattedDataSize(value.toLongLong());
        case Kind::Boolean:
            // Drawn as a tick by the delegate, so the cell carries no text.
            return QString();
        case Kind::Metadata:
            // The database stores the level as an identifier; the user reads the
            // same wording the device editor offers (PageDeviceEditForm.qml).
            {
                const QString level = value.toString();
                if (level == Catalog::METADATA_NONE)           return tr("None");
                if (level == Catalog::METADATA_MEDIA_BASIC)    return tr("Media Basic");
                if (level == Catalog::METADATA_MEDIA_EXTENDED) return tr("Media Extended");
                if (level == Catalog::METADATA_FULL)           return tr("Full Extended");
                return level;
            }
        }
        return {};

    case SortRole:
        // Numbers and sizes sort by their stored value, never by the formatted
        // text, so 1000 sorts above 999 and a GB above a MB (DVP-F2).
        if (col.kind == Kind::Number || col.kind == Kind::Size)
            return value.toLongLong();
        if (col.kind == Kind::Boolean)
            return value.toBool();
        return data(index, Qt::DisplayRole);

    case AlignRole:
        return (col.kind == Kind::Number || col.kind == Kind::Size)
                   ? QStringLiteral("right") : QStringLiteral("left");

    case IconNameRole:
        if (col.kind != Kind::Name)
            return QString();
        if (type == QLatin1String("Storage"))
            return QStringLiteral("drive-harddisk");
        if (type == QLatin1String("Catalog"))
            return row.value(QStringLiteral("active")).toBool()
                       ? QStringLiteral("media-optical-blu-ray")
                       : QStringLiteral("media-optical");
        return QStringLiteral("drive-multidisk");

    case IsBooleanRole:
        return col.kind == Kind::Boolean;

    case TickRole:
        return col.kind == Kind::Boolean && value.toBool();

    case BoldRole:
        // K2 weights only the name and the numeric columns, and only for Storage
        // and Virtual devices (devicetreeview.cpp boldColumnList).
        return (type == QLatin1String("Storage") || type == QLatin1String("Virtual"))
               && (col.kind == Kind::Name || col.kind == Kind::Number || col.kind == Kind::Size);

    case ItalicRole:
        return type == QLatin1String("Virtual")
               && (col.kind == Kind::Name || col.kind == Kind::Number || col.kind == Kind::Size);

    case DimmedRole:
        return type == QLatin1String("Virtual") || type == QLatin1String("Storage");

    case DeviceIdRole:
        return row.value(QStringLiteral("deviceId"));

    case RowDataRole:
        return row;
    }

    return {};
}

QVariant DeviceTableModel::headerData(int section, Qt::Orientation orientation, int role) const
{
    if (orientation != Qt::Horizontal || role != Qt::DisplayRole)
        return {};
    if (section < 0 || section >= m_visible.size())
        return {};
    // Marked for extraction by QT_TRANSLATE_NOOP in the column tables below.
    return QCoreApplication::translate("DeviceTableModel",
                                       columnsForView().at(m_visible.at(section)).header);
}

QHash<int, QByteArray> DeviceTableModel::roleNames() const
{
    return {
        { Qt::DisplayRole, "display"   },
        { SortRole,        "sortValue" },
        { AlignRole,       "alignment" },
        { IconNameRole,    "iconName"  },
        { IsBooleanRole,   "isBoolean" },
        { TickRole,        "tick"      },
        { BoldRole,        "bold"      },
        { ItalicRole,      "italic"    },
        { DimmedRole,      "dimmed"    },
        { DeviceIdRole,    "deviceId"  },
        { RowDataRole,     "rowData"   },
    };
}

//--------------------------------------------------------------------------
using Kind = DeviceTableModel::Kind;

static const QList<DeviceTableModel::Column> &storageColumns()
{
    static const QList<DeviceTableModel::Column> columns = {
        //  key                    header             kind          full   memory width
        { "name",               QT_TRANSLATE_NOOP("DeviceTableModel", "Name"),           Kind::Name,    false, false, 220 },
        { "active",             QT_TRANSLATE_NOOP("DeviceTableModel", "Active"),         Kind::Boolean, true,  false,  60 },
        { "externalId",         QT_TRANSLATE_NOOP("DeviceTableModel", "Storage ID"),     Kind::Number,  false, false,  80 },
        { "fileCount",          QT_TRANSLATE_NOOP("DeviceTableModel", "Number of files"),Kind::Number,  false, false, 110 },
        { "totalFileSize",      QT_TRANSLATE_NOOP("DeviceTableModel", "Total Size"),     Kind::Size,    false, false, 100 },
        { "usedSpace",          QT_TRANSLATE_NOOP("DeviceTableModel", "Used space"),     Kind::Size,    false, false, 100 },
        { "freeSpace",          QT_TRANSLATE_NOOP("DeviceTableModel", "Free space"),     Kind::Size,    false, false, 100 },
        { "totalSpace",         QT_TRANSLATE_NOOP("DeviceTableModel", "Total space"),    Kind::Size,    false, false, 100 },
        { "dateUpdated",        QT_TRANSLATE_NOOP("DeviceTableModel", "Date updated"),   Kind::Text,    false, false, 150 },
        { "path",               QT_TRANSLATE_NOOP("DeviceTableModel", "Path"),           Kind::Text,    false, false, 300 },
        { "storageType",        QT_TRANSLATE_NOOP("DeviceTableModel", "Type"),           Kind::Text,    true,  false, 120 },
        { "storageLabel",       QT_TRANSLATE_NOOP("DeviceTableModel", "Label"),          Kind::Text,    false, false, 120 },
        { "storageFileSystem",  QT_TRANSLATE_NOOP("DeviceTableModel", "FileSystem"),     Kind::Text,    false, false, 100 },
        { "storageBrand",       QT_TRANSLATE_NOOP("DeviceTableModel", "Brand"),          Kind::Text,    true,  false, 150 },
        { "storageModel",       QT_TRANSLATE_NOOP("DeviceTableModel", "Model"),          Kind::Text,    true,  false, 150 },
        { "storageSerialNumber", QT_TRANSLATE_NOOP("DeviceTableModel", "Serial Number"),  Kind::Text,    true,  false, 150 },
        { "storageBuildDate",   QT_TRANSLATE_NOOP("DeviceTableModel", "Build Date"),     Kind::Text,    true,  false, 100 },
        { "storageComment1",    QT_TRANSLATE_NOOP("DeviceTableModel", "Comment 1"),      Kind::Text,    true,  false, 150 },
        { "storageComment2",    QT_TRANSLATE_NOOP("DeviceTableModel", "Comment 2"),      Kind::Text,    true,  false, 150 },
        { "storageComment3",    QT_TRANSLATE_NOOP("DeviceTableModel", "Comment 3"),      Kind::Text,    true,  false, 150 },
    };
    return columns;
}

static const QList<DeviceTableModel::Column> &catalogColumns()
{
    static const QList<DeviceTableModel::Column> columns = {
        //  key                       header             kind           full   memory width
        { "name",                  QT_TRANSLATE_NOOP("DeviceTableModel", "Name"),           Kind::Name,     false, false, 220 },
        { "active",                QT_TRANSLATE_NOOP("DeviceTableModel", "Active"),         Kind::Boolean,  true,  false,  60 },
        { "externalId",            QT_TRANSLATE_NOOP("DeviceTableModel", "Catalog ID"),     Kind::Number,   true,  false,  80 },
        { "fileCount",             QT_TRANSLATE_NOOP("DeviceTableModel", "Number of files"),Kind::Number,   false, false, 110 },
        { "totalFileSize",         QT_TRANSLATE_NOOP("DeviceTableModel", "Total Size"),     Kind::Size,     false, false, 100 },
        { "dateUpdated",           QT_TRANSLATE_NOOP("DeviceTableModel", "Date updated"),   Kind::Text,     false, false, 150 },
        { "path",                  QT_TRANSLATE_NOOP("DeviceTableModel", "Path"),           Kind::Text,     false, false, 300 },
        { "catalogFileType",       QT_TRANSLATE_NOOP("DeviceTableModel", "File Type"),      Kind::Text,     false, false, 120 },
        { "catalogIncludeHidden",  QT_TRANSLATE_NOOP("DeviceTableModel", "Hidden"),         Kind::Boolean,  false, false,  75 },
        { "catalogIncludeMetadata", QT_TRANSLATE_NOOP("DeviceTableModel", "Metadata"),       Kind::Metadata, false, false, 130 },
        { "catalogIncludeChecksum", QT_TRANSLATE_NOOP("DeviceTableModel", "Checksum"),       Kind::Text,     false, false, 100 },
        // K2 lists this column in its boolean set, which blanks the storage name
        // it actually holds and leaves the column permanently empty
        // (devicetreeview.cpp:66). K3 shows the name, on the user's decision of
        // 2026-09-11; K2 itself is left alone (DVP-C3).
        { "catalogParentStorage",  QT_TRANSLATE_NOOP("DeviceTableModel", "Parent storage"), Kind::Text,     false, false, 150 },
        { "catalogDateLoaded",     QT_TRANSLATE_NOOP("DeviceTableModel", "Date Loaded"),    Kind::Text,     true,  true,  150 },
        { "catalogAppVersion",     QT_TRANSLATE_NOOP("DeviceTableModel", "App Version"),    Kind::Text,     true,  false,  90 },
        { "catalogFilePath",       QT_TRANSLATE_NOOP("DeviceTableModel", "File Path"),      Kind::Text,     true,  true,  300 },
    };
    return columns;
}
