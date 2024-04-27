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
// File Name:   exploretreeview.cpp
// Purpose:
// Description:
// Author:      Stephane Couturier
/////////////////////////////////////////////////////////////////////////////
*/
#include "exploretreeview.h"

#include <QFont>
#include <QBrush>
#include <QDebug>
#include <QFileIconProvider>

ExploreTreeView::ExploreTreeView(QObject *parent)
    : QSortFilterProxyModel(parent)
{

}

QVariant ExploreTreeView::data(const QModelIndex &index, int role) const
{
    switch ( role )
         {
            case Qt::DecorationRole:
            {
                //Folder column
                if( index.column()==0 ){
                    return QIcon(QIcon::fromTheme("folder"));
                }

                break;
            }

            case Qt::BackgroundRole:
            {
                if (1 == index.row() % 2)
                    return QColor(255, 255, 255);
                else
                    return QColor(247, 247, 247);

                break;
            }

        }


    return QSortFilterProxyModel::data(index, role);
}

QVariant ExploreTreeView::headerData(int section, Qt::Orientation orientation, int role) const
{
//    QList<int> grayColumnList;
//    grayColumnList    <<0 <<1;

    switch ( role )
         {
            case Qt::DisplayRole:
            {
                return QSortFilterProxyModel::headerData( section, orientation, role) ;
            }
            case Qt::BackgroundRole:
            {
//                if (grayColumnList.contains(section))  //change background
//                    return QBrush(QColor(245, 245, 245));
//                break;
            }
        }
        return QVariant();

}
