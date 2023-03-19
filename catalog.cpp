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

    //if provided, remove the / at the end if any, except for root path (unix)
    if(sourcePath!=""){
        int pathLength   = sourcePath.length();
        QString lastChar = sourcePath.at(pathLength-1);
        if (sourcePath !="/" and lastChar=="/") {
            sourcePath.remove(pathLength-1,1);
        }
    }
}
void Catalog::setFileCount()
{
    QSqlQuery query;
    QString querySQL = QLatin1String(R"(
                            SELECT COUNT(file_name)
                            FROM file
                            WHERE file_catalog =:file_catalog
                        )");
    query.prepare(querySQL);
    query.bindValue(":file_catalog",name);
    query.exec();
    query.next();
    fileCount = query.value(0).toLongLong();
}
void Catalog::setTotalFileSize()
{
    QSqlQuery query;
    QString querySQL = QLatin1String(R"(
                            SELECT SUM(file_size)
                            FROM file
                            WHERE file_catalog =:file_catalog
                        )");
    query.prepare(querySQL);
    query.bindValue(":file_catalog",name);
    query.exec();
    query.next();
    totalFileSize = query.value(0).toLongLong();
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
void Catalog::setDateLoaded()
{
    dateLoaded = QDateTime::currentDateTime().toString("yyyy-MM-dd hh:mm:ss");

    QSqlQuery catalogQuery;
    QString catalogQuerySQL = QLatin1String(R"(
                                        UPDATE catalog
                                        SET catalog_date_loaded =:catalog_date_loaded
                                        WHERE catalog_name =:catalog_name
                                      )");
    catalogQuery.prepare(catalogQuerySQL);
    catalogQuery.bindValue(":catalog_date_loaded", dateLoaded);
    catalogQuery.bindValue(":catalog_name",        name);
    catalogQuery.exec();
}
void Catalog::setDateUpdated()
{
    dateUpdated = QDateTime::currentDateTime().toString("yyyy-MM-dd hh:mm:ss");

    QSqlQuery catalogQuery;
    QString catalogQuerySQL = QLatin1String(R"(
                                        UPDATE catalog
                                        SET catalog_date_updated =:catalog_date_updated
                                        WHERE catalog_name =:catalog_name
                                      )");
    catalogQuery.prepare(catalogQuerySQL);
    catalogQuery.bindValue(":catalog_date_updated", dateUpdated);
    catalogQuery.bindValue(":catalog_name",        name);
    catalogQuery.exec();
}
void Catalog::setIncludeMetadata(bool selectedIncludeMetadata)
{
    includeMetadata = selectedIncludeMetadata;
}
void Catalog::setAppVersion(QString selectedAppVersion)
{
    appVersion = selectedAppVersion;
}

//catalog files data operation
void Catalog::createCatalog()
{
    QSqlQuery insertCatalogQuery;
    QString insertCatalogQuerySQL = QLatin1String(R"(
                                        INSERT OR IGNORE INTO catalog (
                                                        catalog_file_path,
                                                        catalog_name,
                                                        catalog_date_updated,
                                                        catalog_source_path,
                                                        catalog_file_count,
                                                        catalog_total_file_size,
                                                        catalog_source_path_is_active,
                                                        catalog_include_hidden,
                                                        catalog_file_type,
                                                        catalog_storage,
                                                        catalog_include_symblinks,
                                                        catalog_is_full_device,
                                                        catalog_date_loaded,
                                                        catalog_include_metadata,
                                                        catalog_app_version
                                                        )
                                        VALUES(         :catalog_file_path,
                                                        :catalog_name,
                                                        :catalog_date_updated,
                                                        :catalog_source_path,
                                                        :catalog_file_count,
                                                        :catalog_total_file_size,
                                                        :catalog_source_path_is_active,
                                                        :catalog_include_hidden,
                                                        :catalog_file_type,
                                                        :catalog_storage,
                                                        :catalog_include_symblinks,
                                                        :catalog_is_full_device,
                                                        :catalog_date_loaded,
                                                        :catalog_include_metadata,
                                                        :catalog_app_version )
                                    )");

    insertCatalogQuery.prepare(insertCatalogQuerySQL);
    insertCatalogQuery.bindValue(":catalog_file_path", filePath);
    insertCatalogQuery.bindValue(":catalog_name",name);
    insertCatalogQuery.bindValue(":catalog_date_updated",dateUpdated);
    insertCatalogQuery.bindValue(":catalog_source_path",sourcePath);
    insertCatalogQuery.bindValue(":catalog_file_count",fileCount);
    insertCatalogQuery.bindValue(":catalog_total_file_size",totalFileSize);
    insertCatalogQuery.bindValue(":catalog_source_path_is_active",sourcePathIsActive);
    insertCatalogQuery.bindValue(":catalog_include_hidden",includeHidden);
    insertCatalogQuery.bindValue(":catalog_file_type",fileType);
    insertCatalogQuery.bindValue(":catalog_storage",storageName);
    insertCatalogQuery.bindValue(":catalog_include_symblinks",includeSymblinks);
    insertCatalogQuery.bindValue(":catalog_is_full_device",isFullDevice);
    insertCatalogQuery.bindValue(":catalog_date_loaded",dateLoaded);
    insertCatalogQuery.bindValue(":catalog_include_metadata",includeMetadata);
    insertCatalogQuery.bindValue(":catalog_app_version",appVersion);
    insertCatalogQuery.exec();
}

void Catalog::deleteCatalog()
{
    QSqlQuery query;
    QString querySQL = QLatin1String(R"(
                            DELETE FROM catalog
                            WHERE catalog_name=:catalog_name
                        )");
    query.prepare(querySQL);
    query.bindValue(":catalog_name",name);
    query.exec();
}

void Catalog::loadCatalogMetaData()
{
    QSqlQuery query;
    QString querySQL = QLatin1String(R"(
                            SELECT
                                catalog_file_path            ,
                                catalog_name                 ,
                                catalog_date_updated         ,
                                catalog_source_path          ,
                                catalog_file_count           ,
                                catalog_total_file_size      ,
                                catalog_source_path_is_active,
                                catalog_include_hidden       ,
                                catalog_file_type            ,
                                catalog_storage              ,
                                catalog_include_symblinks    ,
                                catalog_is_full_device       ,
                                catalog_date_loaded          ,
                                catalog_include_metadata     ,
                                catalog_app_version
                            FROM catalog
                            LEFT JOIN storage ON catalog_storage = storage_name
                            WHERE catalog_name=:catalog_name
                        )");
    query.prepare(querySQL);
    query.bindValue(":catalog_name",name);
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
    appVersion         = query.value(14).toString();
}

void Catalog::renameCatalog(QString newCatalogName)
{
    //rename value of current object
    name = newCatalogName;
}

void Catalog::renameCatalogFile(QString newCatalogName)
{
    //rename file
    QFileInfo catalogFileInfo(filePath);
    QString newCatalogFilePath = catalogFileInfo.absolutePath() + "/" + newCatalogName + ".idx";
    QFile::rename(filePath, newCatalogFilePath);
    filePath = newCatalogFilePath;
}

void Catalog::loadCatalogFileListToTable()
{
    //Verify if the lastest version of the catalog is already in memory
    QDateTime dateTimeLoaded  = QDateTime::fromString(dateLoaded, "yyyy-MM-dd hh:mm:ss");
    QDateTime dateTimeUpdated = QDateTime::fromString(dateUpdated,"yyyy-MM-dd hh:mm:ss");

    //Load catalog files if latest version is not already in memory
    if ( dateTimeLoaded < dateTimeUpdated ){
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
                                    DELETE FROM file
                                    WHERE file_catalog=:file_catalog
                                                )");
                deleteQuery.prepare(deleteQuerySQL);
                deleteQuery.bindValue(":file_catalog",name);
                deleteQuery.exec();

                //prepare insert query for file
                QSqlQuery insertFileQuery;
                QString insertFileSQL = QLatin1String(R"(
                                        INSERT INTO file (
                                                file_name,
                                                file_folder_path,
                                                file_size,
                                                file_date_updated,
                                                file_catalog,
                                                file_full_path
                                                )
                                        VALUES(
                                                :file_name,
                                                :file_folder_path,
                                                :file_size,
                                                :file_date_updated,
                                                :file_catalog,
                                                :file_full_path )
                                        )");

                //prepare insert query for folder
                                QSqlQuery insertFolderQuery;
                                QString insertFolderSQL = QLatin1String(R"(
                                        INSERT INTO folder(
                                                folder_catalog_name,
                                                folder_path
                                                            )
                                        VALUES(
                                                :folder_catalog_name,
                                                :folder_path)
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

                    //Load folder into the database
                        insertFolderQuery.prepare(insertFolderSQL);
                        insertFolderQuery.bindValue(":folder_catalog_name",name);
                        insertFolderQuery.bindValue(":folder_path",      folder);
                        insertFolderQuery.exec();

                    //Load file into the database
                        insertFileQuery.prepare(insertFileSQL);
                        insertFileQuery.bindValue(":file_name",        fileInfo.fileName());
                        insertFileQuery.bindValue(":file_size",        lineFileSize);
                        insertFileQuery.bindValue(":file_folder_path", folder );
                        insertFileQuery.bindValue(":file_date_updated",lineFileDatetime);
                        insertFileQuery.bindValue(":file_catalog",     name);
                        insertFileQuery.bindValue(":file_full_path",   lineFilePath);
                        insertFileQuery.exec();
                }

            //update catalog loaded version
                setDateLoaded();

            //close file
                catalogFile.close();
        }
    }
}

void Catalog::loadFoldersToTable()
{
    //Verify if the lastest version of the catalog is already in memory
    QDateTime dateTimeLoaded   = QDateTime::fromString(dateLoaded, "yyyy-MM-dd hh:mm:ss");
    QDateTime dateTimeUploaded = QDateTime::fromString(dateUpdated,"yyyy-MM-dd hh:mm:ss");
    //Load catalog files if latest version is not already in memory
    if ( dateTimeLoaded < dateTimeUploaded ){

        //prepare inputs and insert query for folder
        QString folderFilePath = filePath;
        int pos = folderFilePath.lastIndexOf(".idx");
        folderFilePath = folderFilePath.left(pos);
        folderFilePath +=".folders.idx";

        QSqlQuery insertFolderQuery;
        QString insertFolderSQL = QLatin1String(R"(
                                        INSERT INTO folder(
                                                folder_catalog_name,
                                                folder_path
                                                            )
                                        VALUES(
                                                :folder_catalog_name,
                                                :folder_path)
                                        )");

        //Inputs
        QFile folderFile(folderFilePath);
        if (folderFile.open(QIODevice::ReadOnly|QIODevice::Text)) {

            //Set up a text stream from the file's data
            QTextStream streamFolderFile(&folderFile);
            QString lineFolderFile;
            QRegularExpression lineFolderFileSplitExp("\t");

            //Prepare database and queries
                //clear database from old version of catalog
                QSqlQuery deleteQuery;
                QString deleteQuerySQL = QLatin1String(R"(
                                    DELETE FROM folder
                                    WHERE folder_catalog_name=:folder_catalog_name
                                                )");
                deleteQuery.prepare(deleteQuerySQL);
                deleteQuery.bindValue(":folder_catalog_name",name);
                deleteQuery.exec();

            //process each line of the file
                while (true){
                    lineFolderFile = streamFolderFile.readLine();
                    if (lineFolderFile.isNull())
                        break;

                    //exclude catalog meta data
                    //if (lineFolderFile.left(1)=="<"){continue;}

                    //Split the line text with tabulations into a list
                    QStringList lineFieldList  = lineFolderFile.split("\t");

                    //Load folder into the database
                        insertFolderQuery.prepare(insertFolderSQL);
                        insertFolderQuery.bindValue(":folder_catalog_name",lineFieldList[0]);
                        insertFolderQuery.bindValue(":folder_path",        lineFieldList[1]);
                        insertFolderQuery.exec();
                }

            //close file
                folderFile.close();
        }
        else{ //if no folder file is found, fall back on generating the list from the files themselves
                //load files first
                loadCatalogFileListToTable();

                //get list of folders
                QSqlQuery selectFoldersQuery;
                QString selectFoldersQuerySQL = QLatin1String(R"(
                                                    SELECT DISTINCT file_folder_path
                                                    FROM file
                                                    WHERE file_catalog=:file_catalog
                                                )");
                selectFoldersQuery.prepare(selectFoldersQuerySQL);
                selectFoldersQuery.bindValue(":file_catalog",name);
                selectFoldersQuery.exec();

                //add each line to the folder table
                QString folderPath;
                while (selectFoldersQuery.next()){
                        folderPath = selectFoldersQuery.value(0).toString();
                        //Load folder into the database
                        insertFolderQuery.prepare(insertFolderSQL);
                        insertFolderQuery.bindValue(":folder_catalog_name",name);
                        insertFolderQuery.bindValue(":folder_path",        folderPath);
                        insertFolderQuery.exec();
                }
        }
    }
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
