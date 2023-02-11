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
// File Name:   storageview.cpp
// Purpose:
// Description:
// Author:      Stephane Couturier
/////////////////////////////////////////////////////////////////////////////
*/
#include "storageview.h"

#include <QFont>
#include <QBrush>
#include <QDebug>
#include <QFileIconProvider>

StorageView::StorageView(QObject *parent)
    : QSortFilterProxyModel(parent)
{

}

QVariant StorageView::data(const QModelIndex &index, int role) const
{
    //Define list of column per type of data
    QList<int> filesizeColumnList, filecountColumnList, percentColumnList;
      filesizeColumnList <<7 <<8;

    switch ( role )
         {

            case Qt::DisplayRole:
            {
                //Currency (Euro) columns
                if( filesizeColumnList.contains(index.column()) ){
                    return QVariant( QLocale().formattedDataSize(QSortFilterProxyModel::data(index, role).toDouble(),2,QLocale::DataSizeIecFormat));
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

                else QSortFilterProxyModel::data(index, role) ;

                break;
            }

            case Qt::FontRole:
            {
                if (index.column() == 1 ) {
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

               break;
            }

            case Qt::BackgroundRole:
            {
                if (index.column()  == 2)  //change background
                    //return QBrush(Qt::red);
                break;
            }

            case Qt::DecorationRole:
            {
                //Filename column
                if( index.column()==1 ){
                    return QIcon(QIcon::fromTheme("drive-harddisk"));
                }

                break;
            }
        }

    return QSortFilterProxyModel::data(index, role);
}

QVariant StorageView::headerData(int section, Qt::Orientation orientation, int role) const
{
    QList<int> grayColumnList;
    grayColumnList    <<7 <<8 <<9 <<10 <<11;

    switch ( role )
         {
            case Qt::DisplayRole:
            {
                return QSortFilterProxyModel::headerData( section, orientation, role) ;
            }
            case Qt::BackgroundRole:
            {
                if (grayColumnList.contains(section))
                    //return QBrush(QColor(245, 245, 245));
                break;
            }
        }
        return QVariant();

}
