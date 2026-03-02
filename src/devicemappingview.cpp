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

    //Device fields  (cols 3+4 added for strict_copy/conflict_mode → all indices +2 vs original)
    filecountColumnList << 10 << 17 << 21;
    filesizeColumnList  <<  9 << 16 << 19;
    percentColumnList   << 20 << 22;
    booleanColumnList   <<  7 << 14;
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

        else QSortFilterProxyModel::data(index, role) ;

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
        if( index.column()==6 ){
            QModelIndex idx = index.sibling(index.row(), 7);
            if( QSortFilterProxyModel::data(idx, Qt::DisplayRole).toBool()==true ){
                return QIcon(QIcon::fromTheme("media-optical-blu-ray"));
            }
            else
                return QIcon(QIcon::fromTheme("media-optical"));
        }
        //Icon for target catalog
        if( index.column()==13 ){
            QModelIndex idx = index.sibling(index.row(), 14);
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
    grayColumnList    <<9 <<10 <<11 <<12 <<13;


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
