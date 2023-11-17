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
#include "qdir.h"

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
void Catalog::updateFileCount()
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
void Catalog::updateTotalFileSize()
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
void Catalog::setDateLoaded(QDateTime dateTime)
{
    if(dateTime.isNull()){
        dateLoaded = QDateTime::currentDateTime();

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
    else
        dateLoaded = dateTime;
}
void Catalog::setDateUpdated(QDateTime dateTime)
{
    if(dateTime.isNull()){
        dateUpdated = QDateTime::currentDateTime();

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
    else
        dateUpdated = dateTime;
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
    query.bindValue(":catalog_name", name);
    query.exec();

    querySQL = QLatin1String(R"(
                        DELETE FROM file
                        WHERE file_catalog =:file_catalog
                    )");
    query.prepare(querySQL);
    query.bindValue(":file_catalog ", name);
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
    if (query.next()){
        filePath           = query.value(0).toString();
        name               = query.value(1).toString();
        dateUpdated        = query.value(2).toDateTime();
        sourcePath         = query.value(3).toString();
        fileCount          = query.value(4).toLongLong();
        totalFileSize      = query.value(5).toLongLong();
        sourcePathIsActive = query.value(6).toBool();
        includeHidden      = query.value(7).toBool();
        fileType           = query.value(8).toString();
        storageName        = query.value(9).toString();
        includeSymblinks   = query.value(10).toBool();
        isFullDevice       = query.value(11).toBool();
        dateLoaded         = query.value(12).toDateTime();
        includeMetadata    = query.value(13).toBool();
        appVersion         = query.value(14).toString();
    }
}

void Catalog::renameCatalog(QString newCatalogName)
{

    //update db
    QSqlQuery query;
    QString querySQL = QLatin1String(R"(
                                UPDATE catalog
                                SET   catalog_name=:new_catalog_name
                                WHERE catalog_name=:catalog_name
                            )");
    query.prepare(querySQL);
    query.bindValue(":new_catalog_name",newCatalogName);
    query.bindValue(":catalog_name",name);
    query.exec();
    query.next();

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

void Catalog::updateStorageNameToFile()
{//Write changes to catalog file (update headers only)

        QFile catalogFile(filePath);
        if(catalogFile.open(QIODevice::ReadWrite | QIODevice::Text))
        {
            QString fullFileText;
            QTextStream textStream(&catalogFile);

            while(!textStream.atEnd())
            {
                QString line = textStream.readLine();

                //add file data line
                if(!line.startsWith("<catalogSourcePath")
                    and !line.startsWith("<catalogIncludeHidden")
                    and !line.startsWith("<catalogFileType")
                    and !line.startsWith("<catalogStorage")
                    and !line.startsWith("<catalogIsFullDevice")
                    and !line.startsWith("<catalogIncludeMetadata")
                    )
                {
                    fullFileText.append(line + "\n");
                }
                else{
                    //add catalog meta-data. The ifs must be in the correct order of the meta-data lines
                    if(line.startsWith("<catalogSourcePath>"))
                        fullFileText.append("<catalogSourcePath>" + sourcePath +"\n");

                    if(line.startsWith("<catalogIncludeHidden>"))
                        fullFileText.append("<catalogIncludeHidden>" + QVariant(includeHidden).toString() +"\n");

                    if(line.startsWith("<catalogFileType>"))
                        fullFileText.append("<catalogFileType>" + fileType +"\n");

                    if(line.startsWith("<catalogStorage>"))
                        fullFileText.append("<catalogStorage>" + storageName +"\n");

                    if(line.startsWith("<catalogIsFullDevice>")){
                        fullFileText.append("<catalogIsFullDevice>" + QVariant(isFullDevice).toString() +"\n");
                    }
                    if(line.startsWith("<catalogIncludeMetadata>")){
                        fullFileText.append("<catalogIncludeMetadata>" + QVariant(includeMetadata).toString() +"\n");
                    }
                }
            }
            catalogFile.resize(0);
            textStream << fullFileText;
            catalogFile.close();
        }
}

void Catalog::loadCatalogFileListToTable()
{//Load catalog files from file, if latest version is not already in memory

    if ( dateLoaded < dateUpdated ){
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
                QDateTime emptyDateTime = *new QDateTime;
                setDateLoaded(emptyDateTime);

            //close file
                catalogFile.close();
        }
    }
}

void Catalog::loadFoldersToTable()
{//Load catalog folders from file, if latest version is not already in memory

    if ( dateLoaded < dateUpdated ){

        //Prepare inputs and insert query for folder
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

            //Clear database from old version of catalog
            QSqlQuery deleteQuery;
            QString deleteQuerySQL = QLatin1String(R"(
                                DELETE FROM folder
                                WHERE folder_catalog_name=:folder_catalog_name
                                            )");
            deleteQuery.prepare(deleteQuerySQL);
            deleteQuery.bindValue(":folder_catalog_name",name);
            deleteQuery.exec();

            //Process each line of the file
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

            //Close file
                folderFile.close();
        }
        else{ //If no folder file is found, fall back on generating the list from the files themselves
            //Load files first
            loadCatalogFileListToTable();

            //Get list of folders
            QSqlQuery selectFoldersQuery;
            QString selectFoldersQuerySQL = QLatin1String(R"(
                                                SELECT DISTINCT file_folder_path
                                                FROM file
                                                WHERE file_catalog=:file_catalog
                                            )");
            selectFoldersQuery.prepare(selectFoldersQuerySQL);
            selectFoldersQuery.bindValue(":file_catalog",name);
            selectFoldersQuery.exec();

            //Add each line to the folder table
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

void Catalog::saveStatistics(QDateTime dateTime)
{
    QSqlQuery querySaveStatistics;
    QString querySaveStatisticsSQL = QLatin1String(R"(
                                        INSERT INTO statistics_catalog(
                                                date_time,
                                                catalog_name,
                                                catalog_file_count,
                                                catalog_total_file_size,
                                                record_type)
                                        VALUES(
                                                :date_time,
                                                :catalog_name,
                                                :catalog_file_count,
                                                :catalog_total_file_size,
                                                :record_type)
                                    )");
    querySaveStatistics.prepare(querySaveStatisticsSQL);
    querySaveStatistics.bindValue(":date_time", dateTime.toString("yyyy-MM-dd hh:mm:ss"));
    querySaveStatistics.bindValue(":catalog_name", name);
    querySaveStatistics.bindValue(":catalog_file_count",  fileCount);
    querySaveStatistics.bindValue(":catalog_total_file_size", totalFileSize);
    if (dateTime == dateUpdated)
        querySaveStatistics.bindValue(":record_type", "update");
    else
        querySaveStatistics.bindValue(":record_type", "snapshot");

    querySaveStatistics.exec();
}

void Catalog::saveStatisticsToFile(QString filePath, QDateTime dateTime)
{
    //Prepare file and data
    QFile fileOut(filePath);
    QString record_type;
    if (dateTime == dateUpdated)
        record_type = "update";
    else
        record_type = "snapshot";

    QString statisticsLine =   dateTime.toString("yyyy-MM-dd hh:mm:ss") + "\t"
                             + name + "\t"
                             + QString::number(fileCount) + "\t"
                             + QString::number(totalFileSize) + "\t"
                             + record_type;

    // Write data
    if (fileOut.open(QFile::WriteOnly | QIODevice::Append | QFile::Text)) {
        QTextStream stream(&fileOut);
        stream << statisticsLine << "\n";
    }
    fileOut.close();
}

void Catalog::updateSourcePathIsActive()
{
    // Verify that the catalog path is accessible (so the related drive is mounted)
    QDir dir(sourcePath);
    sourcePathIsActive = dir.exists();
    QSqlQuery query;
    QString querySQL = QLatin1String(R"(
                            UPDATE catalog
                            SET catalog_source_path_is_active=:catalog_source_path_is_active
                            WHERE catalog_name=:catalog_name
                        )");
    query.prepare(querySQL);
    query.bindValue(":catalog_source_path_is_active", sourcePathIsActive);
    query.bindValue(":catalog_name", name);
    query.exec();
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
