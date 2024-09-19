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
// Purpose:     Class/model to display a tree of directories
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

bool ExploreTreeView::lessThan(const QModelIndex &left, const QModelIndex &right) const
{
        // QString leftData = sourceModel()->data(left).toString();
        // QString rightData = sourceModel()->data(right).toString();

        // // Compare strings case-insensitively
        // return QString::compare(leftData, rightData, Qt::CaseInsensitive) < 0;

        QVariant leftData = sourceModel()->data(left);
        QVariant rightData = sourceModel()->data(right);

        if (leftData.typeId() == QMetaType::QString && rightData.typeId() == QMetaType::QString) {
            QString leftString = leftData.toString();
            QString rightString = rightData.toString();

            if (caseSensitive) {
                return leftString < rightString;
            } else {
                return QString::compare(leftString, rightString, Qt::CaseInsensitive) < 0;
            }
        }

        return QSortFilterProxyModel::lessThan(left, right);

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
        }

    return QSortFilterProxyModel::data(index, role);
}

QVariant ExploreTreeView::headerData(int section, Qt::Orientation orientation, int role) const
{
    switch ( role )
         {
            case Qt::DisplayRole:
            {
                return QSortFilterProxyModel::headerData( section, orientation, role) ;
            }
        }
        return QVariant();
}
