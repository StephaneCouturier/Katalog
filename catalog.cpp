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
// Purpose:     class to create catalogs (list of files and their attributes)
// Description:
// Author:      Stephane Couturier
/////////////////////////////////////////////////////////////////////////////
*/

#include "catalog.h"
#include <QSqlQuery>

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
    return 5;
}

QVariant Catalog::data(const QModelIndex &index, int role) const
{
    if (!index.isValid() || role != Qt::DisplayRole) {
        return QVariant();
    }
    switch (index.column()){
    case 0: return QString(fileName[index.row()]);
    case 1: return qint64 (fileSize[index.row()]);
    case 3: return QString(filePath[index.row()]);
    case 2: return QString(fileDateTime[index.row()]);
    case 4: return QString(fileCatalog[index.row()]);
    }
    return QVariant();
}

QVariant Catalog::headerData(int section, Qt::Orientation orientation, int role) const
{
    if (role == Qt::DisplayRole && orientation == Qt::Horizontal) {
        switch (section){
        case 0: return QString(tr("Name"));
        case 1: return QString(tr("Size"));
        case 3: return QString(tr("Folder"));
        case 2: return QString(tr("Date"));
        case 4: return QString(tr("Catalog"));
        }
    }
    return QVariant();
}

void Catalog::setCatalogName(QString selectedCatalogName)
{
    catalogName = selectedCatalogName;
}

void Catalog::loadCatalogMetaData()
{
    QSqlQuery query;
    QString querySQL = QLatin1String(R"(
                                SELECT
                                    catalogID                   ,
                                    catalogFilePath             ,
                                    catalogName                 ,
                                    catalogDateUpdated          ,
                                    catalogFileCount            ,
                                    catalogTotalFileSize        ,
                                    catalogSourcePath           ,
                                    catalogFileType             ,
                                    catalogSourcePathIsActive   ,
                                    catalogIncludeHidden        ,
                                    catalogStorage              ,
                                    storageLocation             ,
                                    catalogIsFullDevice         ,
                                    catalogLoadedVersion
                                FROM catalog
                                LEFT JOIN storage ON catalogStorage = storageName
                                WHERE catalogName=:catalogName
                        )");
    query.prepare(querySQL);
    query.bindValue(":catalogName",catalogName);
    query.exec();
    query.next();

    catalogID                 = query.value(0).toString();
    catalogFilePath           = query.value(1).toString();
    catalogName               = query.value(2).toString();
    catalogDateUpdated        = query.value(3).toString();
    catalogSourcePath         = query.value(4).toString();
    catalogFileCount          = query.value(5).toString();
    catalogTotalFileSize      = query.value(6).toString();
    catalogSourcePathIsActive = query.value(7).toString();
    catalogIncludeHidden      = query.value(8).toString();
    catalogFileType           = query.value(9).toString();
    catalogStorage            = query.value(10).toString();
    catalogIncludeSymblinks   = query.value(11).toString();
    catalogIsFullDevice       = query.value(12).toString();
    catalogLoadedVersion      = query.value(13).toString();
}

void Catalog::populateFileData( const QList<QString> &cfileName,
                                const QList<qint64>  &cfileSize,
                                const QList<QString> &cfilePath,
                                const QList<QString> &cfileDateTime,
                                const QList<QString> &cfileCatalog)
{
    fileName.clear();
    fileName = cfileName;
    fileSize.clear();
    fileSize = cfileSize;
    filePath.clear();
    filePath = cfilePath;
    fileDateTime.clear();
    fileDateTime = cfileDateTime;
    fileCatalog.clear();
    fileCatalog = cfileCatalog;

    return;
}
