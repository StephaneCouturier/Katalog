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
/////////////////////////////////////////////////////////////////////////////
// Application: Katalog
// File Name:   catalog.cpp
// Purpose:     class to create catalogs (list of files and their attributes)
// Description:
// Author:      Stephane Couturier
/////////////////////////////////////////////////////////////////////////////
*/

#include "catalog.h"

#include <QApplication>
#include <QDir>
#include <QSqlError>

Catalog::Catalog(QObject *parent) : QAbstractTableModel(parent), workerThread(nullptr) {

}

Catalog::~Catalog() {
    if (workerThread) {
        workerThread->quit();
        workerThread->wait();
        delete workerThread;
    }
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

    //Remove the / at the end if any, except for root path (Linux)
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
    QSqlQuery query(QSqlDatabase::database("defaultConnection"));
    QString querySQL = QLatin1String(R"(
                            SELECT COUNT(file_name)
                            FROM file
                            WHERE file_catalog_id =:file_catalog_id
                        )");
    query.prepare(querySQL);
    query.bindValue(":file_catalog_id", ID);
    query.exec();
    query.next();
    fileCount = query.value(0).toLongLong();
}
void Catalog::updateTotalFileSize()
{
    QSqlQuery query(QSqlDatabase::database("defaultConnection"));
    QString querySQL = QLatin1String(R"(
                            SELECT SUM(file_size)
                            FROM file
                            WHERE file_catalog_id =:file_catalog_id
                    )");
    query.prepare(querySQL);
    query.bindValue(":file_catalog_id",ID);
    query.exec();
    query.next();
    totalFileSize = query.value(0).toLongLong();
}
void Catalog::setDateLoaded(QDateTime dateTime, QString connectionName)
{
    //Only needed in "Memory" mode, used to avoid reloading a catalog already in memory.
    //Define date
    if(dateTime.isNull()){
        dateLoaded = QDateTime::currentDateTime();
        QSqlQuery catalogQuery(QSqlDatabase::database(connectionName));
        QString catalogQuerySQL = QLatin1String(R"(
                                        UPDATE catalog
                                        SET catalog_date_loaded =:catalog_date_loaded
                                        WHERE catalog_id =:catalog_id
                                      )");
        catalogQuery.prepare(catalogQuerySQL);
        catalogQuery.bindValue(":catalog_date_loaded", dateLoaded.toString("yyyy-MM-dd hh:mm:ss"));
        catalogQuery.bindValue(":catalog_id", ID);
        catalogQuery.exec();
    }
    else{
        dateLoaded = dateTime;
    }
}
void Catalog::setDateUpdated(QDateTime dateTime)
{
    if(dateTime.isNull()){
        dateUpdated = QDateTime::currentDateTime();

        QSqlQuery catalogQuery(QSqlDatabase::database("defaultConnection"));
        QString catalogQuerySQL = QLatin1String(R"(
                                            UPDATE catalog
                                            SET catalog_date_updated =:catalog_date_updated
                                            WHERE catalog_name =:catalog_name
                                          )");
        catalogQuery.prepare(catalogQuerySQL);
        catalogQuery.bindValue(":catalog_date_updated", dateUpdated.toString("yyyy-MM-dd hh:mm:ss"));
        catalogQuery.bindValue(":catalog_name",        name);
        catalogQuery.exec();
    }
    else
        dateUpdated = dateTime;
}

//catalog files data operation
void Catalog::generateID()
{//Generate ID
    int maxID = 0;
    QSqlQuery queryCatalogID(QSqlDatabase::database("defaultConnection"));
    QString queryCatalogIDSQL = QLatin1String(R"(
                                    SELECT MAX (catalog_id)
                                    FROM catalog
                                )");
    queryCatalogID.prepare(queryCatalogIDSQL);
    queryCatalogID.exec();
    if(queryCatalogID.next()){
        maxID = queryCatalogID.value(0).toInt();
        ID = maxID + 1;
    }
}

void Catalog::insertCatalog()
{//Insert new catalog entry
    QSqlQuery insertCatalogQuery(QSqlDatabase::database("defaultConnection"));
    QString insertCatalogQuerySQL = QLatin1String(R"(
                                        INSERT OR IGNORE INTO catalog (
                                                        catalog_id,
                                                        catalog_file_path,
                                                        catalog_name,
                                                        catalog_date_updated,
                                                        catalog_source_path,
                                                        catalog_file_count,
                                                        catalog_total_file_size,
                                                        catalog_include_hidden,
                                                        catalog_file_type,
                                                        catalog_storage,
                                                        catalog_include_symblinks,
                                                        catalog_is_full_device,
                                                        catalog_date_loaded,
                                                        catalog_include_metadata,
                                                        catalog_app_version
                                                        )
                                        VALUES(         :catalog_id,
                                                        :catalog_file_path,
                                                        :catalog_name,
                                                        :catalog_date_updated,
                                                        :catalog_source_path,
                                                        :catalog_file_count,
                                                        :catalog_total_file_size,
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
    insertCatalogQuery.bindValue(":catalog_id", ID);
    insertCatalogQuery.bindValue(":catalog_file_path", filePath);
    insertCatalogQuery.bindValue(":catalog_name", name);
    insertCatalogQuery.bindValue(":catalog_date_updated", dateUpdated);
    insertCatalogQuery.bindValue(":catalog_source_path", sourcePath);
    insertCatalogQuery.bindValue(":catalog_file_count", fileCount);
    insertCatalogQuery.bindValue(":catalog_total_file_size", totalFileSize);
    insertCatalogQuery.bindValue(":catalog_include_hidden", includeHidden);
    insertCatalogQuery.bindValue(":catalog_file_type", fileType);
    insertCatalogQuery.bindValue(":catalog_storage", storageName);
    insertCatalogQuery.bindValue(":catalog_include_symblinks", includeSymblinks);
    insertCatalogQuery.bindValue(":catalog_is_full_device", isFullDevice);
    insertCatalogQuery.bindValue(":catalog_date_loaded", dateLoaded);
    insertCatalogQuery.bindValue(":catalog_include_metadata", includeMetadata);
    insertCatalogQuery.bindValue(":catalog_app_version", appVersion);
    insertCatalogQuery.exec();
}

void Catalog::deleteCatalog()
{
    QSqlQuery query(QSqlDatabase::database("defaultConnection"));
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

void Catalog::saveCatalog()
{//Update database with catalog values
    QSqlQuery query(QSqlDatabase::database("defaultConnection"));
    QString querySQL = QLatin1String(R"(
                            UPDATE catalog
                            SET     catalog_name             =:catalog_name,
                                    catalog_source_path      =:catalog_source_path,
                                    catalog_storage          =:catalog_storage,
                                    catalog_file_type        =:catalog_file_type,
                                    catalog_include_hidden   =:catalog_include_hidden,
                                    catalog_include_metadata =:catalog_include_metadata,
                                    catalog_include_metadata =:catalog_include_metadata
                            WHERE  catalog_id=:catalog_id
                                )");
    query.prepare(querySQL);
    query.bindValue(":catalog_id", ID);
    query.bindValue(":catalog_name", name);
    query.bindValue(":catalog_source_path", sourcePath);
    query.bindValue(":catalog_storage", storageName);
    query.bindValue(":catalog_file_type", fileType);
    query.bindValue(":catalog_include_hidden", includeHidden);
    query.bindValue(":catalog_include_symblinks", includeSymblinks);
    query.exec();
}

void Catalog::clearCatalogData()
{
    qDebug() << "Clearing existing catalog data for update";

    // Clear files from database (use catalog ID, not name)
    QString deleteFilesSQL = "DELETE FROM file WHERE file_catalog_id = :file_catalog_id";
    QSqlQuery query(QSqlDatabase::database("defaultConnection"));
    query.prepare(deleteFilesSQL);
    query.bindValue(":file_catalog_id", ID);  // Use ID, not name

    if (!query.exec()) {
        qDebug() << "Error clearing catalog files:" << query.lastError().text();
    } else {
        qDebug() << "Cleared" << query.numRowsAffected() << "files from catalog";
    }

    // Clear folders from database (use catalog ID, not name)
    QString deleteFoldersSQL = "DELETE FROM folder WHERE folder_catalog_id = :folder_catalog_id";
    query.prepare(deleteFoldersSQL);
    query.bindValue(":folder_catalog_id", ID);  // Use ID, not name

    if (!query.exec()) {
        qDebug() << "Error clearing catalog folders:" << query.lastError().text();
    } else {
        qDebug() << "Cleared" << query.numRowsAffected() << "folders from catalog";
    }

    // Reset counters
    fileCount = 0;
    totalFileSize = 0;
}

bool Catalog::updateCatalogFileHeaders(QString databaseMode)
{
    if(databaseMode != "Memory") {
        return true; // Nothing to do for non-Memory mode
    }

    QFile catalogFile(filePath);
    if(!catalogFile.open(QIODevice::ReadWrite | QIODevice::Text)) {
        return false; // Return false to indicate failure
    }

    QString fullFileText;
    QTextStream textStream(&catalogFile);

    // Build the header content
    fullFileText.append("<catalogSourcePath>" + sourcePath +"\n");
    fullFileText.append("<catalogFileCount>" + QVariant(fileCount).toString() +"\n");
    fullFileText.append("<catalogTotalFileSize>" + QVariant(totalFileSize).toString() +"\n");
    fullFileText.append("<catalogIncludeHidden>" + QVariant(includeHidden).toString() +"\n");
    fullFileText.append("<catalogFileType>" + fileType +"\n");
    fullFileText.append("<catalogStorage>" + storageName +"\n");
    fullFileText.append("<catalogIncludeSymblinks>" + QVariant(includeSymblinks).toString() +"\n");
    fullFileText.append("<catalogIsFullDevice>" + QVariant(isFullDevice).toString() +"\n");
    fullFileText.append("<catalogIncludeMetadata>" + QVariant(includeMetadata).toString() +"\n");
    fullFileText.append("<catalogAppVersion>" + QVariant(appVersion).toString() +"\n");
    fullFileText.append("<catalogID>" + QVariant(ID).toString() +"\n");

    // Copy existing file data (non-header lines)
    while(!textStream.atEnd())
    {
        QString line = textStream.readLine();
        if(!line.startsWith("<catalog")) {
            fullFileText.append(line + "\n");
        }
    }

    // Rewrite the file with updated headers
    catalogFile.resize(0);
    textStream << fullFileText;
    catalogFile.close();

    return true;
}

QList<qint64> Catalog::updateCatalogFiles(QString databaseMode, QString collectionFolder, bool reportCannotUpdate)
{
    QList<qint64> list;
    getFileExtensions(); // Update the list of file extensions to scan

    qDebug() << "updateCatalogFiles: databaseMode=" << databaseMode << "sourcePath=" << sourcePath;

    // Basic validation - return failure codes instead of showing UI
    if (databaseMode == "Memory") {
        // Check for old format issues
        if (filePath == "not recorded" || name == "not recorded" || sourcePath == "not recorded") {
            list.append(-3); // Code for old format error
            list.append(0); list.append(0); list.append(0); list.append(0);
            return list;
        }

        // Check for missing input information
        if (filePath.isEmpty() || sourcePath.isEmpty()) {
            list.append(-4); // Code for missing info error
            list.append(0); list.append(0); list.append(0); list.append(0);
            return list;
        }
    }

    // Capture previous values for delta calculation
    qint64 previousFileCount = fileCount;
    qint64 previousTotalFileSize = totalFileSize;

    // Check if directory exists
    QDir dir(sourcePath);
    if (dir.exists()) {
        // Check if directory is empty
        if (dir.entryInfoList(QDir::NoDotAndDotDot|QDir::AllEntries).count() == 0) {
            list.append(-1); // Code for empty directory (UI can handle user choice)
            list.append(0); list.append(0); list.append(0); list.append(0);
            return list;
        }

        // Directory exists and has content - proceed with cataloging
        catalogDirectory(databaseMode, collectionFolder);

        // Calculate and return results
        qint64 newFileCount = fileCount;
        qint64 deltaFileCount = newFileCount - previousFileCount;
        qint64 newTotalFileSize = totalFileSize;
        qint64 deltaTotalFileSize = newTotalFileSize - previousTotalFileSize;

        list.append(1); // Success
        list.append(newFileCount);
        list.append(deltaFileCount);
        list.append(newTotalFileSize);
        list.append(deltaTotalFileSize);
    }
    else {
        // Directory doesn't exist
        list.append(-2); // Code for directory not found
        list.append(0); list.append(0); list.append(0); list.append(0);
    }
    qDebug() << "updateCatalogFiles: returning list with" << list.size() << "elements:" << list;
    return list;
}
void Catalog::loadCatalog(QString connectionName)
{
    QSqlQuery query(QSqlDatabase::database(connectionName));
    QString querySQL = QLatin1String(R"(
                            SELECT
                                catalog_id                   ,
                                catalog_file_path            ,
                                catalog_name                 ,
                                catalog_date_updated         ,
                                catalog_source_path          ,
                                catalog_file_count           ,
                                catalog_total_file_size      ,
                                catalog_include_hidden       ,
                                catalog_file_type            ,
                                catalog_storage              ,
                                catalog_include_symblinks    ,
                                catalog_is_full_device       ,
                                catalog_date_loaded          ,
                                catalog_include_metadata     ,
                                catalog_app_version
                            FROM catalog
                            WHERE catalog_id=:catalog_id
                        )");

    query.prepare(querySQL);
    query.bindValue(":catalog_id",ID);
    query.exec();

    if (query.next()){
        ID                 = query.value(0).toInt();
        filePath           = query.value(1).toString();
        name               = query.value(2).toString();
        dateUpdated        = query.value(3).toDateTime();
        sourcePath         = query.value(4).toString();
        fileCount          = query.value(5).toLongLong();
        totalFileSize      = query.value(6).toLongLong();
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

    //Update db
    QSqlQuery query(QSqlDatabase::database("defaultConnection"));
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

    //Rename value of current object
    name = newCatalogName;

}

void Catalog::renameCatalogFile(QString newCatalogName)
{
    QFileInfo catalogFileInfo(filePath);

    //Rename folders file
    QString currentFolderFilePath = filePath;

    if (currentFolderFilePath.right(4)==".idx"){
        currentFolderFilePath = currentFolderFilePath.chopped(4); //remove the .idx extension
        currentFolderFilePath +=".folders.idx"; //add the .folder.idx one for the folders file
        QString newFoldersFilePath = catalogFileInfo.absolutePath() + "/" + newCatalogName + ".folders.idx";
        QFile::rename(currentFolderFilePath, newFoldersFilePath);
    }

    //Rename catalog file
    QString newCatalogFilePath = catalogFileInfo.absolutePath() + "/" + newCatalogName + ".idx";
    QFile::rename(filePath, newCatalogFilePath);

    //Update the file path of the catalog with new value
    filePath = newCatalogFilePath;
}

void Catalog::loadCatalogFileListToTable(QString connectionName, QMutex &mutex, bool &stopRequested)
{
    //Load catalog files from file, if latest version is not already in memory
    if ( dateLoaded < dateUpdated ){

        // Count total lines first for progress reporting
        QFile countFile(filePath);
        if (countFile.open(QIODevice::ReadOnly | QIODevice::Text)) {
            int totalLines = 0;
            QTextStream countStream(&countFile);
            while (!countStream.atEnd()) {
                countStream.readLine();
                totalLines++;
            }
            countFile.close();

            //Inputs
            QFile catalogFile(filePath);
            if (catalogFile.open(QIODevice::ReadOnly|QIODevice::Text)) {

                //Set up a text stream from the file's data
                QTextStream streamCatalogFile(&catalogFile);
                QString lineCatalogFile;

                //Prepare database and queries
                //Clear database from old version of catalog
                QSqlQuery deleteQuery(QSqlDatabase::database(connectionName));
                QString deleteQuerySQL = QLatin1String(R"(
                                    DELETE FROM file
                                    WHERE file_catalog_id=:file_catalog_id
                                                )");
                deleteQuery.prepare(deleteQuerySQL);
                deleteQuery.bindValue(":file_catalog_id", ID);
                deleteQuery.exec();
                //Prepare insert query for file
                QSqlQuery insertFileQuery(QSqlDatabase::database(connectionName));
                QString insertFileSQL = QLatin1String(R"(
                                        INSERT INTO file (
                                                file_catalog_id,
                                                file_name,
                                                file_folder_path,
                                                file_size,
                                                file_date_updated,
                                                file_catalog,
                                                file_full_path
                                                )
                                        VALUES(
                                                :file_catalog_id,
                                                :file_name,
                                                :file_folder_path,
                                                :file_size,
                                                :file_date_updated,
                                                :file_catalog,
                                                :file_full_path )
                                        )");

                //Prepare insert query for folder
                QSqlQuery insertFolderQuery(QSqlDatabase::database(connectionName));
                QString insertFolderSQL = QLatin1String(R"(
                                        INSERT INTO folder(
                                                folder_catalog_id,
                                                folder_path
                                                            )
                                        VALUES(
                                                :folder_catalog_id,
                                                :folder_path)
                                        )");

                //Process each line of the file
                // Process each line of the file
                int linesProcessed = 0;

                while (true){
                    QMutexLocker locker(&mutex);
                    if (stopRequested) {
                        return;
                    }
                    locker.unlock(); // Unlock the mutex while processing to allow stop requests

                    lineCatalogFile = streamCatalogFile.readLine();
                    if (lineCatalogFile.isNull())
                        break;

                    //Exclude catalog meta data
                    if (lineCatalogFile.left(1)=="<"){continue;}

                    //Split the line text with tabulations into a list
                    QStringList lineFieldList  = lineCatalogFile.split("\t");
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

                    //Get the fileDateTime from the list if available
                    QString lineFileDatetime;
                    if (fieldListCount >= 3){
                        lineFileDatetime = lineFieldList[2];}
                    else lineFileDatetime = "";

                    QString folder = fileInfo.path();

                    //Load folder into the database
                    insertFolderQuery.prepare(insertFolderSQL);
                    insertFolderQuery.bindValue(":folder_catalog_id", ID);
                    insertFolderQuery.bindValue(":folder_path",       folder);
                    insertFolderQuery.exec();

                    //Load file into the database
                    insertFileQuery.prepare(insertFileSQL);
                    insertFileQuery.bindValue(":file_catalog_id",  ID);
                    insertFileQuery.bindValue(":file_name",        fileInfo.fileName());
                    insertFileQuery.bindValue(":file_size",        lineFileSize);
                    insertFileQuery.bindValue(":file_folder_path", folder );
                    insertFileQuery.bindValue(":file_date_updated",lineFileDatetime);
                    insertFileQuery.bindValue(":file_catalog",     name);
                    insertFileQuery.bindValue(":file_full_path",   lineFilePath);
                    insertFileQuery.exec();

                    // Progress reporting using configurable rate
                    linesProcessed++;
                    if (linesProcessed % catalogRefreshRate == 0) {
                        emit loadProgress(linesProcessed, totalLines);
                    }
                }

                // Final progress update
                emit loadProgress(linesProcessed, totalLines);

                //Update catalog loaded version
                QDateTime emptyDateTime = *new QDateTime;
                setDateLoaded(emptyDateTime, connectionName);

                //Close file
                catalogFile.close();
            }
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

        QSqlQuery insertFolderQuery(QSqlDatabase::database("defaultConnection"));
        QString insertFolderSQL = QLatin1String(R"(
                                        INSERT INTO folder(
                                                folder_catalog_id,
                                                folder_path
                                                            )
                                        VALUES(
                                                :folder_catalog_id,
                                                :folder_path)
                                        )");

        //Inputs
        QFile folderFile(folderFilePath);
        if (folderFile.open(QIODevice::ReadOnly|QIODevice::Text)) {

            //Set up a text stream from the file's data
            QTextStream streamFolderFile(&folderFile);
            QString lineFolderFile;

            //Clear database from old version of catalog
            QSqlQuery deleteQuery(QSqlDatabase::database("defaultConnection"));
            QString deleteQuerySQL = QLatin1String(R"(
                                DELETE FROM folder
                                WHERE folder_catalog_id=:folder_catalog_id
                                            )");
            deleteQuery.prepare(deleteQuerySQL);
            deleteQuery.bindValue(":folder_catalog_id", ID);
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
                    insertFolderQuery.bindValue(":folder_catalog_id", lineFieldList[0]);
                    insertFolderQuery.bindValue(":folder_path",       lineFieldList[1]);
                    insertFolderQuery.exec();
            }

            //Close file
                folderFile.close();
        }
        else{ //If no folder file is found, fall back on generating the list from the files themselves
            //Load files first
            QMutex tempMutex;
            bool tempStopRequested = false;
            loadCatalogFileListToTable("defaultConnection", tempMutex, tempStopRequested);

            //Get list of folders
            QSqlQuery selectFoldersQuery(QSqlDatabase::database("defaultConnection"));
            QString selectFoldersQuerySQL = QLatin1String(R"(
                                                SELECT DISTINCT file_folder_path
                                                FROM file
                                                WHERE file_catalog_id=:file_catalog_id
                                            )");
            selectFoldersQuery.prepare(selectFoldersQuerySQL);
            selectFoldersQuery.bindValue(":file_catalog_id", ID);
            selectFoldersQuery.exec();

            //Add each line to the folder table
            QString folderPath;
            while (selectFoldersQuery.next()){
                    folderPath = selectFoldersQuery.value(0).toString();
                    //Load folder into the database
                    insertFolderQuery.prepare(insertFolderSQL);
                    insertFolderQuery.bindValue(":folder_catalog_id", ID);
                    insertFolderQuery.bindValue(":folder_path", folderPath);
                    insertFolderQuery.exec();
            }
        }
    }
}

void Catalog::saveStatistics(QDateTime dateTime)
{
    QSqlQuery querySaveStatistics(QSqlDatabase::database("defaultConnection"));
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

bool Catalog::catalogNameExists()
{
    QSqlQuery query(QSqlDatabase::database("defaultConnection"));
    QString querySQL = QLatin1String(R"(
                                    SELECT COUNT(*)
                                    FROM   catalog
                                    WHERE  catalog_name = :catalog_name
                                )");

    query.prepare(querySQL);
    query.bindValue(":catalog_name", name);

    if (!query.exec()) {
        // Handle SQL error
        return false;
    }

    query.next();
    return query.value(0).toInt() > 0;
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

void Catalog::getFileExtensions()
{//Populate file type list

    QStringList fileType_Image;
    QStringList fileType_Audio;
    QStringList fileType_Video;
    QStringList fileType_Text;

    fileType_Image<< "*.png" << "*.jpg" << "*.gif" << "*.xcf" << "*.tif" << "*.bmp";
    fileType_Audio<< "*.mp3" << "*.wav" << "*.ogg" << "*.aif";
    fileType_Video<< "*.wmv" << "*.avi" << "*.mp4" << "*.mkv" << "*.flv"  << "*.webm" << "*.m4v" << "*.vob" << "*.ogv" << "*.mov";
    fileType_Text << "*.txt" << "*.pdf" << "*.odt" << "*.idx" << "*.html" << "*.rtf" << "*.doc" << "*.docx" << "*.epub";

    if      ( fileType == "Image")
        fileExtensions = fileType_Image;
    else if ( fileType == "Audio")
        fileExtensions = fileType_Audio;
    else if ( fileType == "Video")
        fileExtensions = fileType_Video;
    else if ( fileType == "Text")
        fileExtensions = fileType_Text;
}

void Catalog::loadExcludedFolders()
{
    QSqlQuery query(QSqlDatabase::database("defaultConnection"));
    QString querySQL = QLatin1String(R"(
                                    SELECT DISTINCT parameter_value2
                                    FROM parameter
                                    WHERE parameter_type ='exclude_directory'
                                    ORDER BY parameter_value2
                                )");
    query.prepare(querySQL);
    query.exec();

    while(query.next()){
        excludedFolders<<query.value(0).toString();
    }
}

void Catalog::catalogDirectory(QString databaseMode, QString collectionFolder)
{//Catalog the files of a directory and update catalog attributes

    //Prepare database, queries, and inputs

        //Remove any former files from db for older catalog with same name
        QSqlQuery deleteFileQuery(QSqlDatabase::database("defaultConnection"));
        QString deleteFileQuerySQL = QLatin1String(R"(
                                                DELETE FROM file
                                                WHERE file_catalog_id=:file_catalog_id
                                            )");
        deleteFileQuery.prepare(deleteFileQuerySQL);
        deleteFileQuery.bindValue(":file_catalog_id", ID);
        deleteFileQuery.exec();

        QSqlQuery deleteFolderQuery(QSqlDatabase::database("defaultConnection"));
        QString deleteFolderQuerySQL = QLatin1String(R"(
                                                DELETE FROM folder
                                                WHERE folder_catalog_id=:folder_catalog_id
                                            )");
        deleteFolderQuery.prepare(deleteFolderQuerySQL);
        deleteFolderQuery.bindValue(":folder_catalog_id", ID);
        deleteFolderQuery.exec();

        //Prepare insert query for file
        QSqlQuery insertFileQuery(QSqlDatabase::database("defaultConnection"));
        QString insertFileSQL = QLatin1String(R"(
                                            INSERT INTO file (
                                                            file_catalog_id,
                                                            file_name,
                                                            file_folder_path,
                                                            file_size,
                                                            file_date_updated,
                                                            file_catalog,
                                                            file_full_path
                                                            )
                                            VALUES(
                                                            :file_catalog_id,
                                                            :file_name,
                                                            :file_folder_path,
                                                            :file_size,
                                                            :file_date_updated,
                                                            :file_catalog,
                                                            :file_full_path )
                                            )");
        insertFileQuery.prepare(insertFileSQL);

        //Prepare insert query for folder
        QSqlQuery insertFolderQuery(QSqlDatabase::database("defaultConnection"));
        QString insertFolderSQL = QLatin1String(R"(
                                            INSERT OR IGNORE INTO folder(
                                                folder_catalog_id,
                                                folder_path
                                             )
                                            VALUES(
                                                :folder_catalog_id,
                                                :folder_path)
                                            )");
        insertFolderQuery.prepare(insertFolderSQL);

        //Insert root folder (so that it is displayed even when there are no sub-folders)
        insertFolderQuery.prepare(insertFolderSQL);
        insertFolderQuery.bindValue(":folder_catalog_id", ID);
        insertFolderQuery.bindValue(":folder_path", sourcePath);
        insertFolderQuery.exec();

    //Scan entries with iterator
        QString entryPath;
        //Start a transaction to save all inserts at once in the db
        QSqlQuery beginQuery(QSqlDatabase::database("defaultConnection"));
        QString beginQuerySQL = QLatin1String(R"(
                                            BEGIN
                                            )");
        beginQuery.prepare(beginQuerySQL);
        beginQuery.exec();

        loadExcludedFolders();

    //Iterator
    if (includeHidden == true){
        QDirIterator iterator(sourcePath + "/", fileExtensions, QDir::AllEntries|QDir::NoDotAndDotDot|QDir::Hidden, QDirIterator::Subdirectories);
        while (iterator.hasNext()){
            entryPath = iterator.next();
            QFileInfo entry(entryPath);

            //exclude if the folder is part of excluded directories and their sub-directories
            bool exclude = false;
            for(int i=0; i<excludedFolders.count(); i++){
                if( entryPath.contains(excludedFolders[i]) ){
                    exclude = true;
                }
            }

            if(exclude == false){
                //Insert dirs
                if (entry.isDir()) {
                    insertFolderQuery.prepare(insertFolderSQL);
                    insertFolderQuery.bindValue(":folder_catalog_id", ID);
                    insertFolderQuery.bindValue(":folder_path", entryPath);
                    insertFolderQuery.exec();
                }

                //Insert files
                else if (entry.isFile()) {

                    QFile file(entryPath);
                    insertFileQuery.bindValue(":file_catalog_id",   ID);
                    insertFileQuery.bindValue(":file_name",         entry.fileName());
                    insertFileQuery.bindValue(":file_size",         file.size());
                    insertFileQuery.bindValue(":file_folder_path",  entry.absolutePath());
                    insertFileQuery.bindValue(":file_date_updated", entry.lastModified().toString("yyyy/MM/dd hh:mm:ss"));
                    insertFileQuery.bindValue(":file_catalog",      name);
                    insertFileQuery.bindValue(":file_full_path",    entryPath);
                    insertFileQuery.exec();

                    //Media File Metadata
                    //DEV: includeMetadata
                    //     if(includeMetadata == true){
                    //         setMediaFile(entryPath);
                    //     }
                }
            }
        }
    }
    else{
        QDirIterator iterator(sourcePath + "/", fileExtensions, QDir::AllEntries|QDir::NoDotAndDotDot, QDirIterator::Subdirectories);
        while (iterator.hasNext()){
            entryPath = iterator.next();
            QFileInfo entry(entryPath);

            //exclude if the folder is part of excluded directories and their sub-directories
            bool exclude = false;
            for(int i=0; i<excludedFolders.count(); i++){
                if( entryPath.startsWith(excludedFolders[i]) ){
                    exclude = true;
                }
            }

            if(exclude == false){

                //Insert dirs
                if (entry.isDir()) {
                    insertFolderQuery.bindValue(":folder_catalog_id", ID);
                    insertFolderQuery.bindValue(":folder_path", entryPath);
                    insertFolderQuery.exec();
                }

                //Insert files
                else if (entry.isFile()) {

                    QFile file(entryPath);
                    insertFileQuery.bindValue(":file_catalog_id",   ID);
                    insertFileQuery.bindValue(":file_name",         entry.fileName());
                    insertFileQuery.bindValue(":file_size",         file.size());
                    insertFileQuery.bindValue(":file_folder_path",  entry.absolutePath());
                    insertFileQuery.bindValue(":file_date_updated", entry.lastModified().toString("yyyy/MM/dd hh:mm:ss"));
                    insertFileQuery.bindValue(":file_catalog",      name);
                    insertFileQuery.bindValue(":file_full_path",    entryPath);
                    insertFileQuery.exec();

                    //Media File Metadata
                    //DEV: includeMetadata
                    // if(developmentMode==true){
                    //     if(includeMetadata == true){
                    //         setMediaFile(entryPath);
                    //     }
                    // }
                }
            }
        }
    }

    //Commit the transaction to save all inserts at once in the db
    QSqlQuery commitQuery(QSqlDatabase::database("defaultConnection"));
    QString commitQuerySQL = QLatin1String(R"(
                                        COMMIT
                                        )");
    commitQuery.prepare(commitQuerySQL);
    commitQuery.exec();

    //update Catalog metadata
    updateFileCount();
    updateTotalFileSize();

    //Populate model with lines for csv files
    if(databaseMode=="Memory"){
        //Save data to file
        QStringList fileList;

        QSqlQuery queryFileList(QSqlDatabase::database("defaultConnection"));
        QString queryFileListSQL = QLatin1String(R"(
                        SELECT file_full_path, file_size, file_date_updated
                        FROM file
                        WHERE file_catalog_id =:file_catalog_id
                    )");
        queryFileList.prepare(queryFileListSQL);
        queryFileList.bindValue(":file_catalog_id", ID);
        queryFileList.exec();

        while(queryFileList.next()){
            fileList << queryFileList.value(0).toString() + "\t" + queryFileList.value(1).toString() + "\t" + queryFileList.value(2).toString();
        };

        //Prepare the catalog file data, adding first the catalog headers at the beginning
        fileList.prepend("<catalogID>"              + QString::number(ID));
        fileList.prepend("<catalogAppVersion>"      + appVersion);
        fileList.prepend("<catalogIncludeMetadata>" + QVariant(includeMetadata).toString());
        fileList.prepend("<catalogIsFullDevice>"    + QVariant(isFullDevice).toString());
        fileList.prepend("<catalogIncludeSymblinks>"+ QVariant(includeSymblinks).toString());
        fileList.prepend("<catalogStorage>"         + storageName);
        fileList.prepend("<catalogFileType>"        + fileType);
        fileList.prepend("<catalogIncludeHidden>"   + QVariant(includeHidden).toString());
        fileList.prepend("<catalogTotalFileSize>"   + QString::number(totalFileSize));
        fileList.prepend("<catalogFileCount>"       + QString::number(fileCount));
        fileList.prepend("<catalogSourcePath>"      + sourcePath);

        //Define and populate a model
        fileListModel = new QStringListModel(this);
        fileListModel->setStringList(fileList);

        //Write to file

        //Save data to files
        saveCatalogToFile(databaseMode, collectionFolder);
        saveFoldersToFile(databaseMode, collectionFolder);
    }

    //Update catalog in db
    //DEV: duplicate of saveCatalog?
    QSqlQuery query(QSqlDatabase::database("defaultConnection"));
    QString querySQL = QLatin1String(R"(
                                UPDATE catalog
                                SET catalog_include_symblinks =:catalog_include_symblinks,
                                    catalog_file_count =:catalog_file_count,
                                    catalog_total_file_size =:catalog_total_file_size,
                                    catalog_app_version =:catalog_app_version
                                WHERE catalog_name =:catalog_name
                            )");
    query.prepare(querySQL);
    query.bindValue(":catalog_include_symblinks", includeSymblinks);
    query.bindValue(":catalog_file_count", fileCount);
    query.bindValue(":catalog_total_file_size", totalFileSize);
    query.bindValue(":catalog_app_version", appVersion);
    query.bindValue(":catalog_name", name);
    query.exec();

    //Update catalog date loaded and updated
    QDateTime emptyDateTime = *new QDateTime;
    setDateUpdated(emptyDateTime);
    setDateLoaded(emptyDateTime, "defaultConnection");
}

//--------------------------------------------------------------------------
bool Catalog::saveCatalogToFile(QString databaseMode, QString collectionFolder)
{
    if(databaseMode != "Memory") {
        return true; // Nothing to do for non-Memory mode
    }

    qDebug() << "=== Catalog::saveCatalogToFile() START ===";
    qDebug() << "Building file list directly from database (independent of fileListModel)";

    try {
        // Build file list directly from database (don't depend on fileListModel)
        QStringList fileList;

        // Get file data from database
        QSqlQuery queryFileList(QSqlDatabase::database("defaultConnection"));
        QString queryFileListSQL = QLatin1String(R"(
                        SELECT file_full_path, file_size, file_date_updated
                        FROM file
                        WHERE file_catalog_id = :file_catalog_id
                        ORDER BY file_full_path
                    )");
        queryFileList.prepare(queryFileListSQL);
        queryFileList.bindValue(":file_catalog_id", ID);

        if (!queryFileList.exec()) {
            qDebug() << "ERROR: Failed to query file list:" << queryFileList.lastError().text();
            return false;
        }

        qDebug() << "Queried files from database successfully";

        // Build the file list (same format as original)
        while(queryFileList.next()){
            QString fileEntry = queryFileList.value(0).toString() + "\t" +
                                queryFileList.value(1).toString() + "\t" +
                                queryFileList.value(2).toString();
            fileList << fileEntry;
        }

        qDebug() << "Built file list with" << fileList.size() << "entries";

        // Prepare the catalog file data, adding headers at the beginning (same as original)
        fileList.prepend("<catalogID>"              + QString::number(ID));
        fileList.prepend("<catalogAppVersion>"      + appVersion);
        fileList.prepend("<catalogIncludeMetadata>" + QVariant(includeMetadata).toString());
        fileList.prepend("<catalogIsFullDevice>"    + QVariant(isFullDevice).toString());
        fileList.prepend("<catalogIncludeSymblinks>"+ QVariant(includeSymblinks).toString());
        fileList.prepend("<catalogStorage>"         + storageName);
        fileList.prepend("<catalogFileType>"        + fileType);
        fileList.prepend("<catalogIncludeHidden>"   + QVariant(includeHidden).toString());
        fileList.prepend("<catalogTotalFileSize>"   + QString::number(totalFileSize));
        fileList.prepend("<catalogFileCount>"       + QString::number(fileCount));
        fileList.prepend("<catalogSourcePath>"      + sourcePath);

        qDebug() << "Added catalog headers, total lines:" << fileList.size();

        // Write to file
        filePath = collectionFolder + "/" + name + ".idx";
        qDebug() << "Writing to file:" << filePath;

        QFile fileOut(filePath);
        if (!fileOut.open(QFile::WriteOnly | QFile::Text)) {
            qDebug() << "ERROR: Failed to open file for writing:" << filePath;
            return false;
        }

        QTextStream stream(&fileOut);
        for (int i = 0; i < fileList.size(); ++i) {
            stream << fileList.at(i) << '\n';
        }

        fileOut.close();

        qDebug() << "File written successfully:" << filePath;
        qDebug() << "=== Catalog::saveCatalogToFile() SUCCESS ===";
        return true;

    } catch (const std::exception& e) {
        qDebug() << "=== EXCEPTION in saveCatalogToFile():" << e.what() << "===";
        return false;
    } catch (...) {
        qDebug() << "=== UNKNOWN EXCEPTION in saveCatalogToFile() ===";
        return false;
    }
}
//--------------------------------------------------------------------------
bool Catalog::saveFoldersToFile(QString databaseMode, QString collectionFolder)
{
    if(databaseMode != "Memory") {
        return true; // Nothing to do for non-Memory mode
    }

    qDebug() << "=== Catalog::saveFoldersToFile() START ===";

    try {
        // Get the folder list from database
        QSqlQuery query(QSqlDatabase::database("defaultConnection"));
        QString querySQL = QLatin1String(R"(
                                    SELECT folder_path
                                    FROM folder
                                    WHERE folder_catalog_id = :folder_catalog_id
                                    ORDER BY folder_path
                                )");
        query.prepare(querySQL);
        query.bindValue(":folder_catalog_id", ID);

        if (!query.exec()) {
            qDebug() << "ERROR: Failed to query folders:" << query.lastError().text();
            return false;
        }

        // Build folder list
        QStringList folderList;
        while (query.next()) {
            folderList << query.value(0).toString();
        }

        qDebug() << "Found" << folderList.size() << "folders to save";

        // Write to file
        QString foldersFilePath = collectionFolder + "/" + name + ".folders.idx";
        QFile fileOut(foldersFilePath);
        if (!fileOut.open(QFile::WriteOnly | QFile::Text)) {
            qDebug() << "ERROR: Failed to open folders file:" << foldersFilePath;
            return false;
        }

        QTextStream stream(&fileOut);
        for (const QString &folder : folderList) {
            stream << ID << '\t' << folder << '\n';
        }

        fileOut.close();

        qDebug() << "Folders file written successfully:" << foldersFilePath;
        qDebug() << "=== Catalog::saveFoldersToFile() SUCCESS ===";
        return true;

    } catch (const std::exception& e) {
        qDebug() << "=== EXCEPTION in saveFoldersToFile():" << e.what() << "===";
        return false;
    } catch (...) {
        qDebug() << "=== UNKNOWN EXCEPTION in saveFoldersToFile() ===";
        return false;
    }
}

//--------------------------------------------------------------------------
int Catalog::countFileLines(const QString &filePath)
{//For counting lines in a file
    QFile file(filePath);
    if (file.open(QIODevice::ReadOnly | QIODevice::Text)) {
        int lines = 0;
        QTextStream in(&file);
        while (!in.atEnd()) {
            in.readLine();
            lines++;
        }
        file.close();
        return lines;
    }
    return 0;
}
