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
    return fileNames.length();
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
    case 0: return QString(fileNames[index.row()]);
    case 1: return qint64 (fileSizes[index.row()]);
    case 3: return QString(filePaths[index.row()]);
    case 2: return QString(fileDateTimes[index.row()]);
    case 4: return QString(fileCatalogs[index.row()]);
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
    name = selectedCatalogName;
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
    query.bindValue(":catalogName",name);
    query.exec();
    query.next();

    ID                 = query.value(0).toString();
    filePath           = query.value(1).toString();
    name               = query.value(2).toString();
    dateUpdated        = query.value(3).toString();
    sourcePath         = query.value(4).toString();
    fileCount          = query.value(5).toString();
    totalFileSize      = query.value(6).toString();
    sourcePathIsActive = query.value(7).toString();
    includeHidden      = query.value(8).toString();
    fileType           = query.value(9).toString();
    storage            = query.value(10).toString();
    includeSymblinks   = query.value(11).toString();
    isFullDevice       = query.value(12).toString();
    loadedVersion      = query.value(13).toString();
}

void Catalog::populateFileData( const QList<QString> &cfileName,
                                const QList<qint64>  &cfileSize,
                                const QList<QString> &cfilePath,
                                const QList<QString> &cfileDateTime,
                                const QList<QString> &cfileCatalog)
{
    fileNames.clear();
    fileNames = cfileName;
    fileSizes.clear();
    fileSizes = cfileSize;
    filePaths.clear();
    filePaths = cfilePath;
    fileDateTimes.clear();
    fileDateTimes = cfileDateTime;
    fileCatalogs.clear();
    fileCatalogs = cfileCatalog;

    return;
}
