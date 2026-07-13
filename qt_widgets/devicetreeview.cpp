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
// File Name:   DeviceTreeView.cpp
// Purpose:     Class/model to display a tree of devices
// Description:
// Author:      Stephane Couturier
/////////////////////////////////////////////////////////////////////////////
*/
#include "devicetreeview.h"
#include "core/device.h"
#include "core/catalog.h"

#include <QFont>
#include <QBrush>
#include <QDebug>
#include <QFileIconProvider>
#include <qapplication.h>
#include <qpalette.h>

DeviceTreeView::DeviceTreeView(QObject *parent)
    : QSortFilterProxyModel(parent)
{
    initializeLists(); //Populate the lists of data types
}

void DeviceTreeView::initializeLists()
{
    //Assign fields to data types.
    //   1 list per data type except for text used as default (no customization)
    // + 1 list for bold text
    using namespace DeviceTreeColumns;

    //Device fields
    filecountColumnList << DEVICE_ID << PARENT_ID << EXTERNAL_ID << FILE_COUNT;
    filesizeColumnList  << FILE_SIZE << USED_SPACE << FREE_SPACE << TOTAL_SPACE;
    booleanColumnList   << IS_ACTIVE;
    boldColumnList      << NAME << FILE_COUNT << FILE_SIZE << USED_SPACE << FREE_SPACE << TOTAL_SPACE;

    //Storage fields
    //All text

    //Catalog fields
    booleanColumnList   << CATALOG_HIDDEN << CATALOG_PARENT_STORAGE;
}

void DeviceTreeView::setColorizeFullRow(bool fullRow)
{
    m_colorizeFullRow = fullRow;
}

void DeviceTreeView::setKatalogTheme(bool katalogTheme)
{
    m_katalogTheme = katalogTheme;
}

bool DeviceTreeView::lessThan(const QModelIndex &source_left, const QModelIndex &source_right) const
{
    // Root groups are ordered by device_group_id first (0 = Physical group before
    // 1 = Virtual groups), so the Physical group stays on top regardless of its name.
    // Only the full device-tree model carries the Group ID column; narrower models
    // (parent picker, search filter) have fewer columns and fall back to name sort.
    if (sourceModel() && sourceModel()->columnCount() > DeviceTreeColumns::GROUP_ID) {
        const int leftGroup  = source_left.siblingAtColumn(DeviceTreeColumns::GROUP_ID).data().toInt();
        const int rightGroup = source_right.siblingAtColumn(DeviceTreeColumns::GROUP_ID).data().toInt();
        if (leftGroup != rightGroup)
            return leftGroup < rightGroup;
    }
    return QSortFilterProxyModel::lessThan(source_left, source_right);
}

QVariant DeviceTreeView::data(const QModelIndex &index, int role) const
{
    switch ( role )
         {
            case Qt::DisplayRole:
            {
                //File size columns
                if( filesizeColumnList.contains(index.column()) ){
                    return QVariant( QLocale().formattedDataSize(QSortFilterProxyModel::data(index, role).toLongLong()) + "  ");
                }

                //Numbers columns (without units)
                else if( filecountColumnList.contains(index.column()) ){
                    return QVariant(QLocale().toString(QSortFilterProxyModel::data(index, role).toDouble(), 'f', 0)  + "  ");
                }

                //Percent columns
                else if( percentColumnList.contains(index.column()) ){
                    if ( QSortFilterProxyModel::data(index, role).toDouble() < 0 )
                        return QVariant(QLocale().toString(QSortFilterProxyModel::data(index, role).toDouble(), 'f', 2) + " %");
                    else if( percentColumnList.contains(index.column()) && QSortFilterProxyModel::data(index, role).toDouble() >= 0)
                        return QVariant("+" + QLocale().toString(QSortFilterProxyModel::data(index, role).toDouble(), 'f', 2) + " %");

                }
                //BooleanColumnList columns (display tick icon or nothing)
                else if( booleanColumnList.contains(index.column()) ){
                    return QVariant("");
                }
                // Metadata column
                else if (index.column() == DeviceTreeColumns::CATALOG_METADATA) {
                    QString internalValue = QSortFilterProxyModel::data(index, role).toString();
                    QString displayName = Catalog::metadataLevelDisplayName(internalValue);
                    return QVariant(QApplication::translate("MainWindow", displayName.toUtf8().constData()));
                }

                else QSortFilterProxyModel::data(index, role) ;

                break;
            }

            case Qt::FontRole:
            {
                if( boldColumnList.contains(index.column()) ){
                    QModelIndex idx = index.sibling(index.row(), 1);
                    QString type = QSortFilterProxyModel::data(idx, Qt::DisplayRole).toString();
                    if( type =="Virtual" ){
                        QFont boldItalicFont;
                        boldItalicFont.setBold(true);
                        boldItalicFont.setItalic(true);
                        return boldItalicFont;
                    }
                    else if( type =="Storage" ){
                        QFont boldFont;
                        boldFont.setBold(true);
                        return boldFont;
                    }
                }
                break;
            }

            case Qt::TextAlignmentRole:
            {
               if ( filecountColumnList.contains(index.column()) )
                   return QVariant ( Qt::AlignVCenter | Qt::AlignRight );

               if ( filesizeColumnList.contains(index.column()) )
                   return QVariant ( Qt::AlignVCenter | Qt::AlignRight );

               break;
            }

            case Qt::DecorationRole:
            {
                //Icon for tree items
                if( index.column()==0 ){

                    QModelIndex idx = index.sibling(index.row(), 1);
                    QString type = QSortFilterProxyModel::data(idx, Qt::DisplayRole).toString();

                    if( type=="Virtual" ){
                        return QIcon(QIcon::fromTheme("drive-multidisk"));
                    }
                    else if( type=="Storage" ){
                        return QIcon(QIcon::fromTheme("drive-harddisk"));
                    }
                    else if( type=="Catalog" ){
                        QModelIndex idx = index.sibling(index.row(), 2);
                        if( QSortFilterProxyModel::data(idx, Qt::DisplayRole).toBool()==true ){
                            return QIcon(QIcon::fromTheme("media-optical-blu-ray"));
                        }
                        else
                            return QIcon(QIcon::fromTheme("media-optical"));
                    }
                    else if( type=="" ){
                        return QIcon(QIcon::fromTheme("drive-multidisk"));
                    }
                }
                //Icon for boolean items
                else if ( booleanColumnList.contains(index.column()) ){
                    if( QSortFilterProxyModel::data(index, Qt::DisplayRole).toBool() == true ){
                        return QIcon(QIcon::fromTheme("dialog-ok-apply"));
                    }
                }
                break;
            }

            case Qt::ForegroundRole:
            {
                // Get the device type from column 1
                QModelIndex typeIndex = index.sibling(index.row(), 1);
                QString type = QSortFilterProxyModel::data(typeIndex, Qt::DisplayRole).toString();
                bool isDark = QApplication::palette().color(QPalette::Window).lightness() < 128;

                // Apply grey color to Virtual devices
                if (m_katalogTheme && type == "Virtual") {
                    // Check if we should color full row or only device name column
                    if (m_colorizeFullRow || index.column() == 0){
                        return isDark ? QColor("#999") : QColor("#666");
                    }
                }
                else if (m_katalogTheme && type == "Storage") {
                    // Check if we should color full row or only device name column
                    if (m_colorizeFullRow || index.column() == 0){
                        return isDark ? QColor("#CCC") : QColor("#444");
                    }
                }
                break;
            }
        }

    return QSortFilterProxyModel::data(index, role);
}

QVariant DeviceTreeView::headerData(int section, Qt::Orientation orientation, int role) const
{
    QList<int> grayColumnList;
    grayColumnList    <<7 <<8 <<9 <<10 <<11;


    switch ( role )
         {
            case Qt::DisplayRole:
            {
                return QSortFilterProxyModel::headerData( section, orientation, role) ;
            }
        }
        return QVariant();

}

void DeviceTreeView::setSelectedDeviceInfo(QString selectedName, QString selectedType)
{
    m_selectedDeviceName = selectedName;
    m_selectedDeviceType = selectedType;
}
