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
// File Name:   devicemappingview.cpp
// Purpose:     Class/model to display a mapping of devices
// Description:
// Author:      Stephane Couturier
/////////////////////////////////////////////////////////////////////////////
*/
#include "devicemappingview.h"

#include <QFont>
#include <QBrush>
#include <QDebug>
#include <QFileIconProvider>
#include <QCoreApplication>

DeviceMappingView::DeviceMappingView(QObject *parent)
    : QSortFilterProxyModel(parent)
{
    initializeLists(); //Populate the lists of data types
}


void DeviceMappingView::initializeLists()
{
    //Assign fields to data types.
    //   1 list per data type except for text used as default (no customization)
    // + 1 list for bold text

    //Device fields  (cols 3,4,5 added for strict_copy/ignore_exclusions/conflict_mode → all indices +3 vs original)
    filecountColumnList << 11 << 18 << 22;
    filesizeColumnList  << 10 << 17 << 20;
    percentColumnList   << 21 << 23;
    booleanColumnList   <<  8 << 15;
    boldColumnList      <<  1; // Mapping Name
}

QVariant DeviceMappingView::data(const QModelIndex &index, int role) const
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

        //Translated enum columns: Type (2), Copy mode (3), Source mode (4), On conflict (5)
        // All lookups use the MainWindow context to keep translations centralised.
        // Corresponding tr() registrations live in mainwindow_setup.cpp (loadSettings).
        else if( index.column() >= 2 && index.column() <= 5 ){
            const QString val = QSortFilterProxyModel::data(index, role).toString();
            if (val == QLatin1String("Backup") || val == QLatin1String("BackUp"))
                return QCoreApplication::translate("MainWindow", "Backup");
            if (val == QLatin1String("Archive"))
                return QCoreApplication::translate("MainWindow", "Archive");
            if (val == QLatin1String("Strict"))
                return QCoreApplication::translate("MainWindow", "Strict");
            if (val == QLatin1String("Unique"))
                return QCoreApplication::translate("MainWindow", "Unique");
            if (val == QLatin1String("Catalog"))
                return QCoreApplication::translate("MainWindow", "Catalog");
            if (val == QLatin1String("Drive"))
                return QCoreApplication::translate("MainWindow", "Drive");
            if (val == QLatin1String("Skip"))
                return QCoreApplication::translate("MainWindow", "Skip");
            if (val == QLatin1String("Rename oldest"))
                return QCoreApplication::translate("MainWindow", "Rename oldest");
            return val; // fallback: return raw value untranslated
        }

        else QSortFilterProxyModel::data(index, role);

        break;
    }

    case Qt::FontRole:
    {
        if( boldColumnList.contains(index.column()) ){
            QFont boldFont;
            boldFont.setBold(true);
            return boldFont;
        }
        break;
    }

    case Qt::TextAlignmentRole:
    {
        if ( filecountColumnList.contains(index.column()) )
            return QVariant ( Qt::AlignVCenter | Qt::AlignRight );

        if ( filesizeColumnList.contains(index.column()) )
            return QVariant ( Qt::AlignVCenter | Qt::AlignRight );

        if ( percentColumnList.contains(index.column()) )
            return QVariant ( Qt::AlignVCenter | Qt::AlignRight );

        break;
    }

    case Qt::DecorationRole:
    {
        //Icon for source catalog
        if( index.column()==7 ){
            QModelIndex idx = index.sibling(index.row(), 8);
            if( QSortFilterProxyModel::data(idx, Qt::DisplayRole).toBool()==true ){
                return QIcon(QIcon::fromTheme("media-optical-blu-ray"));
            }
            else
                return QIcon(QIcon::fromTheme("media-optical"));
        }
        //Icon for target catalog
        if( index.column()==14 ){
            QModelIndex idx = index.sibling(index.row(), 15);
            if( QSortFilterProxyModel::data(idx, Qt::DisplayRole).toBool()==true ){
                return QIcon(QIcon::fromTheme("media-optical-blu-ray"));
            }
            else
                return QIcon(QIcon::fromTheme("media-optical"));
        }
        //Icon for boolean items
        else if ( booleanColumnList.contains(index.column()) ){
            if( QSortFilterProxyModel::data(index, Qt::DisplayRole).toBool() == true ){
                return QIcon(QIcon::fromTheme("dialog-ok-apply"));
            }
        }
        break;
    }
    }

    return QSortFilterProxyModel::data(index, role);
}

QVariant DeviceMappingView::headerData(int section, Qt::Orientation orientation, int role) const
{
    QList<int> grayColumnList;
    grayColumnList    <<10 <<11 <<12 <<13 <<14;


    switch ( role )
    {
    case Qt::DisplayRole:
    {
        return QSortFilterProxyModel::headerData( section, orientation, role) ;
    }
    }
    return QVariant();

}

void DeviceMappingView::setSelectedDeviceInfo(QString selectedName, QString selectedType)
{
    m_selectedDeviceName = selectedName;
    m_selectedDeviceType = selectedType;
}
