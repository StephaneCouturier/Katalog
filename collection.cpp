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
* /////////////////////////////////////////////////////////////////////////////
// Application: Katalog
// File Name:   collection.cpp
// Purpose:
// Description:
// Author:      Stephane Couturier
// Modified by: Stephane Couturier
// Created:     2020-07-11
// Version:     0.6
/////////////////////////////////////////////////////////////////////////////
*/

#include "collection.h"
#include "QTreeView"


Collection::Collection(QObject *parent) : QAbstractTableModel(parent)
{

}

int Collection::rowCount(const QModelIndex &parent) const
{
    Q_UNUSED(parent);
    return catalogName.length();
}

int Collection::columnCount(const QModelIndex &parent) const
{
    Q_UNUSED(parent);
    return 5;
}

QVariant Collection::data(const QModelIndex &index, int role) const
{
    if (!index.isValid() || role != Qt::DisplayRole) {
        return QVariant();
    }
    switch (index.column()){
    case 0: return QString(catalogName[index.row()]);
    case 1: return QString(catalogDateUpdated[index.row()]);
    case 2: return int(catalogFileCount[index.row()]);
    case 3: return QString(catalogSourcePath[index.row()]);
    case 4: return QString(catalogFilePath[index.row()]);
    }
    return QVariant();
}

QVariant Collection::headerData(int section, Qt::Orientation orientation, int role) const
{
    if (role == Qt::DisplayRole && orientation == Qt::Horizontal) {
        switch (section){
        case 0: return QString("Name");
        case 1: return QString("Last update");
        case 2: return QString("Files");
        case 3: return QString("Source Path");
        case 4: return QString("File path");
        }
    }
    return QVariant();
}


// Create a method to populate the model with data:
void Collection::populateData(const QList<QString> &cNames,
                           const QList<QString> &cDateUpdated,
                           const QList<int> &cNums,
                           const QList<QString> &cSourcePaths,
                           const QList<QString> &cCatalogFiles)
{
    catalogName.clear();
    catalogName = cNames;
    catalogDateUpdated.clear();
    catalogDateUpdated = cDateUpdated;
    catalogFileCount.clear();
    catalogFileCount = cNums;
    catalogSourcePath.clear();
    catalogSourcePath = cSourcePaths;
    catalogFilePath.clear();
    catalogFilePath = cCatalogFiles;

    return;
}
