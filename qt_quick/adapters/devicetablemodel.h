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
// File Name:   devicetablemodel.h
// Purpose:     QAbstractTableModel for the Devices page Table display mode
// Description: Feeds the Storage list and Catalogs list tables (SpecDevicesPage
//              DVP-F1/F2/F3/F5). The column set depends on the active view
//              ("Storage" / "Catalogs"), on the Full Table option and, for two
//              catalog columns, on Memory database mode - mirroring K2's
//              loadDevicesStorageToModel() / loadDevicesCatalogToModel()
//              (qt_widgets/mainwindow_tab_device_pr.cpp:1400 and :1652).
//              Cell rendering mirrors the K2 DeviceTreeView proxy
//              (qt_widgets/devicetreeview.cpp): locale-formatted right-aligned
//              sizes and counts, a tick for boolean columns, a device icon in
//              the Name cell, bold / bold-italic / greyed rows by device type.
// Author:      Stephane Couturier
/////////////////////////////////////////////////////////////////////////////
*/
#ifndef DEVICETABLEMODEL_H
#define DEVICETABLEMODEL_H

#include <QAbstractTableModel>
#include <QVariantList>
#include <QVariantMap>

class DeviceTableModel : public QAbstractTableModel
{
    Q_OBJECT

public:
    enum Roles {
        // The raw, still-typed cell value. The sort proxy sorts on this rather
        // than on DisplayRole, so a formatted size sorts by its byte count and
        // a file count sorts numerically (DVP-F2).
        SortRole      = Qt::UserRole + 1,
        AlignRole     = Qt::UserRole + 2,   // "left" | "right"
        IconNameRole  = Qt::UserRole + 3,   // themed icon for the Name cell, empty elsewhere
        IsBooleanRole = Qt::UserRole + 4,   // cell is drawn as a tick, not as text
        TickRole      = Qt::UserRole + 5,   // the tick is on
        BoldRole      = Qt::UserRole + 6,
        ItalicRole    = Qt::UserRole + 7,
        DimmedRole    = Qt::UserRole + 8,
        DeviceIdRole  = Qt::UserRole + 9,
        RowDataRole   = Qt::UserRole + 10,  // whole row map, for the context menu
    };

    explicit DeviceTableModel(QObject *parent = nullptr);

    /** "Storage" or "Catalogs". Any other value yields no columns. */
    void setView(const QString &view);
    void setFullTable(bool fullTable);
    /** Memory database mode gates the Date Loaded and File Path columns, as K2 does. */
    void setMemoryMode(bool memoryMode);

    void populate(const QVariantList &rows);
    void clear();

    int      rowCount(const QModelIndex &parent = QModelIndex()) const override;
    int      columnCount(const QModelIndex &parent = QModelIndex()) const override;
    QVariant data(const QModelIndex &index, int role = Qt::DisplayRole) const override;
    QVariant headerData(int section, Qt::Orientation orientation, int role = Qt::DisplayRole) const override;
    QHash<int, QByteArray> roleNames() const override;

    /** Suggested width for a visible column, used by the QML columnWidthProvider. */
    Q_INVOKABLE int columnWidth(int column) const;

    enum class Kind {
        Name,      // device name, carries the type icon
        Text,
        Number,    // locale-formatted integer, right-aligned
        Size,      // locale-formatted data size, right-aligned
        Boolean,   // drawn as a tick
        Metadata,  // internal metadata level mapped to its display name
    };

    struct Column {
        const char *key;       // key into the row map delivered by AppManager::getDeviceList
        const char *header;    // source text, translated through tr()
        Kind        kind;
        bool        fullOnly;  // revealed by the Full Table option
        bool        memoryOnly;// Memory database mode only
        int         width;
    };

private:
    const QList<Column> &columnsForView() const;
    void rebuildVisibleColumns();

    QString     m_view;
    bool        m_fullTable  = false;
    bool        m_memoryMode = false;
    QVariantList m_rows;
    QList<int>  m_visible;   // indices into columnsForView()
};

#endif // DEVICETABLEMODEL_H
