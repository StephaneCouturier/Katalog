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
// Version:     0.8
/////////////////////////////////////////////////////////////////////////////
*/

#include "collection.h"
//#include "QTreeView"

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
    return 9;
}

QVariant Collection::data(const QModelIndex &index, int role) const
{
    if (!index.isValid() || role != Qt::DisplayRole) {
        return QVariant();
    }
    switch (index.column()){
    case 0: return QString(catalogFilePath[index.row()]);
    case 1: return QString(catalogName[index.row()]);
    case 2: return QString(catalogDateUpdated[index.row()]);
    case 3: return int(catalogFileCount[index.row()]);
    case 4: return qint64(catalogTotalFileSize[index.row()]);
    case 5: return QString(catalogSourcePath[index.row()]);
    case 6: return QString(catalogFileType[index.row()]);
    case 7: return bool(catalogSourcePathIsActive[index.row()]);
    case 8: return bool(catalogIncludeHidden[index.row()]);
    }
    return QVariant();
}

QVariant Collection::headerData(int section, Qt::Orientation orientation, int role) const
{
    if (role == Qt::DisplayRole && orientation == Qt::Horizontal) {
        switch (section){
        case 0: return QString("File path");
        case 1: return QString("Name");
        case 2: return QString("Last update");
        case 3: return QString("Files");
        case 4: return QString("Total File Size");
        case 5: return QString("Source Path");
        case 6: return QString("File Type");
        case 7: return QString("Active");
        case 8: return QString("includes Hidden");
        }
    }
    return QVariant();
}


// Create a method to populate the model with data:
void Collection::populateData(const QList<QString> &cCatalogFilePaths,
                              const QList<QString> &cNames,
                              const QList<QString> &cDateUpdated,
                              const QList<qint64>  &cFileCounts,
                              const QList<qint64>  &cTotalFileSize,
                              const QList<QString> &cSourcePaths,
                              const QList<QString> &cFileTypes,
                              const QList<bool>    &cSourcePathIsActives,
                              const QList<bool>    &catalogIncludeHiddens
                              )
{
    catalogName.clear();
    catalogName = cNames;
    catalogDateUpdated.clear();
    catalogDateUpdated = cDateUpdated;
    catalogFileCount.clear();
    catalogFileCount = cFileCounts;
    catalogFileType.clear();
    catalogFileType = cFileTypes;
    catalogSourcePath.clear();
    catalogSourcePath = cSourcePaths;
    catalogSourcePathIsActive.clear();
    catalogSourcePathIsActive = cSourcePathIsActives;
    catalogTotalFileSize.clear();
    catalogTotalFileSize = cTotalFileSize;
    catalogFilePath.clear();
    catalogFilePath = cCatalogFilePaths;
    catalogIncludeHidden.clear();
    catalogIncludeHidden = catalogIncludeHiddens;

    return;
}
