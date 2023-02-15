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
#include <QFileInfo>
#include <QRegularExpression>

Catalog::Catalog(QObject *parent) : QAbstractTableModel(parent)
{

}

//file list model
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


//set catalog definition
void Catalog::setName(QString selectedName)
{
    name = selectedName;
}
void Catalog::setFilePath(QString selectedFilePath)
{
    filePath = selectedFilePath;
}
void Catalog::setSourcePath(QString selectedSourcePath)
{
    sourcePath = selectedSourcePath;

    //remove the / at the end if any, except for root path
    int pathLength = sourcePath.length();
//    if (sourcePath !="/" and sourcePath.at(pathLength-1)=="/") {
//        sourcePath.remove(pathLength-1,1);
//    }
}
void Catalog::setFileCount(qint64 selectedFileCount)
{
    fileCount = selectedFileCount;
}
void Catalog::setTotalFileSize(qint64 selectedTotalFileSize)
{
    totalFileSize = selectedTotalFileSize;
}
void Catalog::setIncludeHidden(bool selectedIncludeHidden)
{
    includeHidden = selectedIncludeHidden;
}
void Catalog::setFileType(QString selectedFileType)
{
    fileType = selectedFileType;
}
void Catalog::setStorageName(QString selectedStorageName)
{
    storageName = selectedStorageName;
}
void Catalog::setIncludeSymblinks(bool selectedIncludeSymblinks)
{
    includeSymblinks = selectedIncludeSymblinks;
}
void Catalog::setIsFullDevice(bool selectedIsFullDevice)
{
    isFullDevice = selectedIsFullDevice;
}
void Catalog::setDateLoaded(QString dateTimeString)
{
    dateLoaded = dateTimeString;
}
void Catalog::setIncludeMetadata(bool selectedIncludeMetadata)
{
    includeMetadata = selectedIncludeMetadata;
}

//catalog files data operation
void Catalog::loadCatalogMetaData()
{
    QSqlQuery query;
    QString querySQL = QLatin1String(R"(
                            SELECT
                                catalogFilePath             ,
                                catalogName                 ,
                                catalogDateUpdated          ,
                                catalogSourcePath           ,
                                catalogFileCount            ,
                                catalogTotalFileSize        ,
                                catalogSourcePathIsActive   ,
                                catalogIncludeHidden        ,
                                catalogFileType             ,
                                catalogStorage              ,
                                catalogIncludeSymblinks     ,
                                catalogIsFullDevice         ,
                                catalogLoadedVersion        ,
                                catalogIncludeMetadata
                            FROM catalog
                            LEFT JOIN storage ON catalogStorage = storageName
                            WHERE catalogName=:catalogName
                        )");
    query.prepare(querySQL);
    query.bindValue(":catalogName",name);
    query.exec();
    query.next();

    filePath           = query.value(0).toString();
    name               = query.value(1).toString();
    dateUpdated        = query.value(2).toString();
    sourcePath         = query.value(3).toString();
    fileCount          = query.value(4).toLongLong();
    totalFileSize      = query.value(5).toLongLong();
    sourcePathIsActive = query.value(6).toBool();
    includeHidden      = query.value(7).toBool();
    fileType           = query.value(8).toString();
    storageName        = query.value(9).toString();
    includeSymblinks   = query.value(10).toBool();
    isFullDevice       = query.value(11).toBool();
    dateLoaded         = query.value(12).toString();
    includeMetadata    = query.value(13).toBool();
}


void Catalog::loadCatalogFileListToTable()
{
    //Verify if the lastest version of the catalog is already in memory
    QDateTime dateTime1 = QDateTime::fromString(dateLoaded, "yyyy-MM-dd hh:mm:ss");
    QDateTime dateTime2 = QDateTime::fromString(dateUpdated,"yyyy-MM-dd hh:mm:ss");

    //Load catalog files if latest version is not already in memory
    if ( dateTime1 < dateTime2){

        //Inputs
        QFile catalogFile(filePath);

        if (catalogFile.open(QIODevice::ReadOnly|QIODevice::Text)) {

            //Set up a text stream from the file's data
            QTextStream streamCatalogFile(&catalogFile);
            QString lineCatalogFile;
            QRegularExpression lineCatalogFileSplitExp("\t");

            //Prepare database and queries
                //clear database from old version of catalog
                QSqlQuery deleteQuery;
                QString deleteQuerySQL = QLatin1String(R"(
                                    DELETE FROM filesall
                                    WHERE fileCatalog=:fileCatalog
                                                )");
                deleteQuery.prepare(deleteQuerySQL);
                deleteQuery.bindValue(":fileCatalog",name);
                deleteQuery.exec();

                //prepare insert query for filesall
                QSqlQuery insertFilesallQuery;
                QString insertFilesallSQL = QLatin1String(R"(
                                        INSERT INTO filesall (
                                                        fileName,
                                                        filePath,
                                                        fileSize,
                                                        fileDateUpdated,
                                                        fileCatalog,
                                                        fileFullPath
                                                        )
                                        VALUES(
                                                        :fileName,
                                                        :filePath,
                                                        :fileSize,
                                                        :fileDateUpdated,
                                                        :fileCatalog,
                                                        :fileFullPath )
                                    )");

                //prepare insert query for folder
                                    QSqlQuery insertFolderQuery;
                                    QString insertFolderSQL = QLatin1String(R"(
                                            INSERT INTO folder(
                                                    folderHash,
                                                    folderCatalogName,
                                                    folderPath
                                                                )
                                            VALUES(
                                                    :folderHash,
                                                    :folderCatalogName,
                                                    :folderPath)
                                                        )");


            //process each line of the file
                while (true){
                    lineCatalogFile = streamCatalogFile.readLine();
                    if (lineCatalogFile.isNull())
                        break;

                    //exclude catalog meta data
                    if (lineCatalogFile.left(1)=="<"){continue;}

                    //Split the line text with tabulations into a list
                    QStringList lineFieldList  = lineCatalogFile.split(lineCatalogFileSplitExp);
                    int         fieldListCount = lineFieldList.count();

                    //Get the file absolute path from this list
                    QString     lineFilePath   = lineFieldList[0];

                    //Get the FileSize from the list if available
                    qint64      lineFileSize;
                    if (fieldListCount >= 3){lineFileSize = lineFieldList[1].toLongLong();}
                    else lineFileSize = 0;

                    //Get the File DateTime from the list if available
                    QDateTime   lineFileDateTime;
                    if (fieldListCount >= 3){lineFileDateTime = QDateTime::fromString(lineFieldList[2],"yyyy/MM/dd hh:mm:ss");}
                    else lineFileDateTime = QDateTime::fromString("0001/01/01 00:00:00","yyyy/MM/dd hh:mm:ss");

                    //Retrieve file info
                    QFileInfo fileInfo(lineFilePath);

                    // Get the fileDateTime from the list if available
                    QString lineFileDatetime;
                    if (fieldListCount >= 3){
                            lineFileDatetime = lineFieldList[2];}
                    else lineFileDatetime = "";

                    QString folder = fileInfo.path();
                    QString folderHash = QString::number(qHash(folder));

                    //Load folder into the database
                        insertFolderQuery.prepare(insertFolderSQL);
                        insertFolderQuery.bindValue(":folderHash",      folderHash);
                        insertFolderQuery.bindValue(":folderCatalogName",name);
                        insertFolderQuery.bindValue(":folderPath",      folder);
                        insertFolderQuery.exec();

                    //Load file into the database
                        insertFilesallQuery.prepare(insertFilesallSQL);
                        insertFilesallQuery.bindValue(":fileName",        fileInfo.fileName());
                        insertFilesallQuery.bindValue(":fileSize",        lineFileSize);
                        insertFilesallQuery.bindValue(":filePath",        folder ); //DEV: replace later by folderHash
                        insertFilesallQuery.bindValue(":fileDateUpdated", lineFileDatetime);
                        insertFilesallQuery.bindValue(":fileCatalog",     name);
                        insertFilesallQuery.bindValue(":fileFullPath",    lineFilePath);
                        insertFilesallQuery.exec();
                }

            //update catalog loadedversion
                QDateTime nowDateTime = QDateTime::currentDateTime();
                setDateLoaded(nowDateTime.toString("yyyy-MM-dd hh:mm:ss"));

                QSqlQuery catalogQuery;
                QString catalogQuerySQL = QLatin1String(R"(
                                            UPDATE catalog
                                            SET catalogLoadedVersion =:catalogDateLoaded
                                            WHERE catalogName =:catalogName
                                        )");
                catalogQuery.prepare(catalogQuerySQL);
                catalogQuery.bindValue(":catalogDateLoaded", dateLoaded);
                catalogQuery.bindValue(":catalogName",       name);
                catalogQuery.exec();

            //close file
                catalogFile.close();
        }
    }
}

void Catalog::renameCatalog(QString newCatalogName)
{
        //rename value of current object
        name = newCatalogName;

        //rename file
        QFileInfo catalogFileInfo(filePath);
        QString newCatalogFilePath = catalogFileInfo.absolutePath() + "/" + newCatalogName + ".idx";
        QFile::rename(filePath, newCatalogFilePath);
        filePath = newCatalogFilePath;
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
