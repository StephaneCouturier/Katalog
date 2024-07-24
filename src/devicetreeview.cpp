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

#include <QFont>
#include <QBrush>
#include <QDebug>
#include <QFileIconProvider>

DeviceTreeView::DeviceTreeView(QObject *parent)
    : QSortFilterProxyModel(parent)
{
    initializeLists(); //to populate the lists
}


void DeviceTreeView::initializeLists()
{
    filecountColumnList << 3 << 4 << 5 << 6;
    filesizeColumnList  << 7 << 8 << 9 << 10;
    boldColumnList      << 0 << 6 << 7 << 8 << 9 << 10;
    booleanColumnList   << 2 << 15; //<<17 <<18 <<20;
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
