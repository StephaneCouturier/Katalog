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
// File Name:   catalog.cpp
// Purpose:
// Description:
// Author:      Stephane Couturier
// Modified by: Stephane Couturier
// Created:     2020-07-11
// Version:     0.1
/////////////////////////////////////////////////////////////////////////////
*/

#include "catalog.h"
#include "QTableView"


Catalog::Catalog(QObject *parent) : QAbstractTableModel(parent)
{
}

int Catalog::rowCount(const QModelIndex &parent) const
{
    Q_UNUSED(parent);
    return catalogName.length();
}

int Catalog::columnCount(const QModelIndex &parent) const
{
    Q_UNUSED(parent);
    return 4;
}

QVariant Catalog::data(const QModelIndex &index, int role) const
{
    if (!index.isValid() || role != Qt::DisplayRole) {
        return QVariant();
    }
    switch (index.column()){
    case 0: return QString(catalogName[index.row()]);
    case 1: return QString(catalogDateUpdated[index.row()]);
    case 2: return QString(catalogFileCount[index.row()]);
    case 3: return QString(catalogSourcePath[index.row()]);
    }
    return QVariant();
}

QVariant Catalog::headerData(int section, Qt::Orientation orientation, int role) const
{
    if (role == Qt::DisplayRole && orientation == Qt::Horizontal) {
        switch (section){
        case 0: return QString("Name");
        case 1: return QString("Last update");
        case 2: return QString("Number of Files");
        case 3: return QString("Source Path");
        }
    }
    return QVariant();
}


// Create a method to populate the model with data:
void Catalog::populateData(const QList<QString> &cNames,
                           const QList<QString> &cSourcePaths,
                           const QList<QString> &cDateUpdated,
                           const QList<QString> &cNums)
{
    catalogName.clear();
    catalogName = cNames;
    catalogSourcePath.clear();
    catalogSourcePath = cSourcePaths;
    catalogDateUpdated.clear();
    catalogDateUpdated = cDateUpdated;
    catalogFileCount.clear();
    catalogFileCount = cNums;
    return;
}


void Catalog::LoadCatalogInfo(QString catalogFilePath)
{

}
