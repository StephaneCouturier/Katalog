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
// File Name:   storage.cpp
// Purpose:
// Description:
// Author:      Stephane Couturier
// Modified by: Stephane Couturier
// Created:     2020-07-11
// Version:     0.9
/////////////////////////////////////////////////////////////////////////////
*/

#include "storage.h"
//#include "mainwindow.h"
//#include "ui_mainwindow.h"
//#include <QSortFilterProxyModel>

Storage::Storage(QObject *parent) : QAbstractTableModel(parent)
{

}

// To replace existing by tree structure------------

//Storage::Storage(QObject *parent): QStandardItemModel(parent)   , treeView(new QTreeView(this))
//, standardModel(new QStandardItemModel(this))
//{
//    QList<QStandardItem *> preparedRow = prepareRow("2' External Drives", "", "2000000");
//    QStandardItem *item = standardModel->invisibleRootItem();
//    // adding a row to the invisible root item produces a root element
//    item->appendRow(preparedRow);

//    QList<QStandardItem *> secondRow = prepareRow("Maxtor_1Tb1", "32", "1000000");
//    // adding a row to an item starts a subtree
//    preparedRow.first()->appendRow(secondRow);

//    QList<QStandardItem *> thirdRow = prepareRow("Maxtor_1Tb2", "33", "1000000");
//    // adding a row to an item starts a subtree
//    preparedRow.first()->appendRow(thirdRow);

//    //ui->TrV_Storage->setModel(standardModel);
//    //treeView->expandAll();
//}


int Storage::rowCount(const QModelIndex &parent) const
{
    Q_UNUSED(parent);
    return storageName.length();
}

int Storage::columnCount(const QModelIndex &parent) const
{
    Q_UNUSED(parent);
    return 10;
}

QVariant Storage::data(const QModelIndex &index, int role) const
{
    if (!index.isValid() || role != Qt::DisplayRole) {
        return QVariant();
    }
        switch (index.column()){
        case 0: return QString(storageLocation[index.row()]);
        case 1: return QIcon(storageIcon[index.row()]);
        case 2: return QString(storageName[index.row()]);
        case 3: return int(storageID[index.row()]);
        case 4: return QString(storageType[index.row()]);
        case 5: return QString(storagePath[index.row()]);
        case 6: return QString(storageLabel[index.row()]);
        case 7: return QString(storageFileSystemType[index.row()]);
        case 8: return qint64(storageBytesTotal[index.row()]);
        case 9: return qint64(storageBytesFree[index.row()]);
    }
    return QVariant();
}

QVariant Storage::headerData(int section, Qt::Orientation orientation, int role) const
{
    if (role == Qt::DisplayRole && orientation == Qt::Horizontal) {
        switch (section){
        case 0: return QString("Location");
        case 1: return QString("Icon");
        case 2: return QString("Name");
        case 3: return QString("ID");
        case 4: return QString("Type");
        case 5: return QString("Path");
        case 6: return QString("Label");
        case 7: return QString("FileSystem");
        case 8: return QString("Total");
        case 9: return QString("Free");
        }
    }
    return QVariant();
}

// Create a method to populate the model with data:
void Storage::populateStorageData(  const QList<QString> &sNames,
                                    const QList<int> &sIDs,
                                    const QList<QString> &sTypes,
                                    const QList<QString> &sLocations,
                                    const QList<QString> &sPaths,
                                    const QList<QString> &sLabels,
                                    const QList<QString> &sFileSystemTypes,
                                    const QList<qint64>  &sBytesTotals,
                                    const QList<qint64>  &sBytesFrees,
                                    const QList<QIcon>   &sIcons
                                    )
{
    storageName.clear();
    storageName = sNames;
    storageIcon.clear();
    storageIcon = sIcons;
    storageID.clear();
    storageID = sIDs;
    storageType.clear();
    storageType = sTypes;
    storageLocation.clear();
    storageLocation = sLocations;
    storagePath.clear();
    storagePath = sPaths;
    storageLabel.clear();
    storageLabel = sLabels;
    storageFileSystemType.clear();
    storageFileSystemType = sFileSystemTypes;
    storageBytesTotal.clear();
    storageBytesTotal = sBytesTotals;
    storageBytesFree.clear();
    storageBytesFree = sBytesFrees;

    return;
}
