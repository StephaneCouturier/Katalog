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
// Purpose:     class to crate catalogs (list of files and their attributes)
// Description:
// Author:      Stephane Couturier
// Modified by: Stephane Couturier
// Created:     2020-07-11
// Version:     0.6
/////////////////////////////////////////////////////////////////////////////
*/

#include "catalog.h"

Catalog::Catalog(QObject *parent) : QAbstractTableModel(parent)
{

}

int Catalog::rowCount(const QModelIndex &parent) const
{
    Q_UNUSED(parent);
    return fileName.length();
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
    case 0: return QString(fileName[index.row()]);
    case 1: return qint64(fileSize[index.row()]);
    case 3: return QString(filePath[index.row()]);
    case 2: return QString(fileDateTime[index.row()]);
    }
    return QVariant();
}

QVariant Catalog::headerData(int section, Qt::Orientation orientation, int role) const
{
    if (role == Qt::DisplayRole && orientation == Qt::Horizontal) {
        switch (section){
        case 0: return QString("Name");
        case 1: return QString("Size");
        case 3: return QString("Folder");
        case 2: return QString("Date");
        }
    }
    return QVariant();
}

// Create a method to populate the model with data:
void Catalog::populateFileData(const QList<QString> &cfileName,
                           const QList<qint64> &cfileSize,
                           const QList<QString> &cfilePath,
                           const QList<QString> &cfileDateTime)
{
    fileName.clear();
    fileName = cfileName;
    fileSize.clear();
    fileSize = cfileSize;
    filePath.clear();
    filePath = cfilePath;
    fileDateTime.clear();
    fileDateTime = cfileDateTime;

    return;
}

