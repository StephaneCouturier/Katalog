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
#include "filetypemapping.h"
#include "filemetadata.h"
#include <QApplication>
#include <QDir>
#include <QSqlError>
#include <qmessagebox.h>

const QString Catalog::METADATA_NONE = "None";
const QString Catalog::METADATA_MEDIA_BASIC = "MediaBasic";
const QString Catalog::METADATA_MEDIA_EXTENDED = "MediaExtended";
const QString Catalog::METADATA_FULL = "FullExtended";

Catalog::Catalog(QObject *parent) : QAbstractTableModel(parent), workerThread(nullptr) {
    includeMetadata = METADATA_NONE;  // Simple default
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
    QSqlQuery query(QSqlDatabase::database(m_connectionName));
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
    QSqlQuery query(QSqlDatabase::database(m_connectionName));
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
void Catalog::setDateLoaded(QDateTime dateTime)
{
    //Only needed in "Memory" mode, used to avoid reloading a catalog already in memory.
    //Define date
    if(dateTime.isNull()){
        dateLoaded = QDateTime::currentDateTime();
        QSqlQuery catalogQuery(QSqlDatabase::database(m_connectionName));
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

        QSqlQuery catalogQuery(QSqlDatabase::database(m_connectionName));
        QString catalogQuerySQL = QLatin1String(R"(
                                            UPDATE catalog
                                            SET catalog_date_updated =:catalog_date_updated
                                            WHERE catalog_id =:catalog_id
                                          )");
        catalogQuery.prepare(catalogQuerySQL);
        catalogQuery.bindValue(":catalog_date_updated", dateUpdated.toString("yyyy-MM-dd hh:mm:ss"));
        catalogQuery.bindValue(":catalog_name", ID);
        catalogQuery.exec();
    }
    else
        dateUpdated = dateTime;
}

//catalog files data operation
void Catalog::generateID()
{//Generate ID
    int maxID = 0;
    QSqlQuery queryCatalogID(QSqlDatabase::database(m_connectionName));
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
    QSqlQuery insertCatalogQuery(QSqlDatabase::database(m_connectionName));
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
    QSqlQuery query(QSqlDatabase::database(m_connectionName));
    QString querySQL = QLatin1String(R"(
                            DELETE FROM catalog
                            WHERE catalog_id=:catalog_id
                        )");
    query.prepare(querySQL);
    query.bindValue(":catalog_id", ID);
    query.exec();

    querySQL = QLatin1String(R"(
                        DELETE FROM file
                        WHERE file_catalog_id =:file_catalog_id
                    )");
    query.prepare(querySQL);
    query.bindValue(":file_catalog_id ", ID);
    query.exec();

}

void Catalog::saveCatalog()
{//Update database with catalog values
    QSqlQuery query(QSqlDatabase::database(m_connectionName));
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
    query.bindValue(":catalog_include_metadata", includeMetadata);
    query.bindValue(":catalog_include_symblinks", includeSymblinks);
    query.exec();
}

void Catalog::clearCatalogData()
{
    qDebug() << "Clearing existing catalog data for update";

    // Clear files from database (use catalog ID, not name)
    QString deleteFilesSQL = "DELETE FROM file WHERE file_catalog_id = :file_catalog_id";
    QSqlQuery query(QSqlDatabase::database(m_connectionName));
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

void Catalog::loadCatalog()
{
    QSqlQuery query(QSqlDatabase::database(m_connectionName));
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
        includeMetadata    = query.value(13).toString();
        appVersion         = query.value(14).toString();
    }

    if (includeMetadata.isEmpty()) { includeMetadata = METADATA_NONE; }
}

void Catalog::renameCatalog(QString newCatalogName)
{

    //Update db
    QSqlQuery query(QSqlDatabase::database(m_connectionName));
    QString querySQL = QLatin1String(R"(
                                UPDATE catalog
                                SET   catalog_name=:new_catalog_name
                                WHERE catalog_id=:catalog_id
                            )");
    query.prepare(querySQL);
    query.bindValue(":new_catalog_name",newCatalogName);
    query.bindValue(":catalog_id",ID);
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

void Catalog::loadCatalogFileListToTable(QMutex &mutex, bool &stopRequested)
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

                // Detect if this is an older catalog t oversion 2.8 that needs migration
                bool needsMigration = (appVersion < "2.8");
                bool catalogWasMigrated = false;

                if (needsMigration) {
                    qDebug() << "Loading v2.6 catalog:" << name << "- will migrate to v2.8 format";
                }

                //Set up a text stream from the file's data
                QTextStream streamCatalogFile(&catalogFile);
                QString lineCatalogFile;

                //Prepare database and queries
                //Clear database from old version of catalog
                QSqlQuery deleteQuery(QSqlDatabase::database(m_connectionName));
                QString deleteQuerySQL = QLatin1String(R"(
                                    DELETE FROM file
                                    WHERE file_catalog_id=:file_catalog_id
                                                )");
                deleteQuery.prepare(deleteQuerySQL);
                deleteQuery.bindValue(":file_catalog_id", ID);
                deleteQuery.exec();
                //Prepare insert query for file
                QSqlQuery insertFileQuery(QSqlDatabase::database(m_connectionName));
                QString insertFileSQL = QLatin1String(R"(
                                    INSERT INTO file(
                                            file_catalog_id         ,
                                            file_name               ,
                                            file_folder_path        ,
                                            file_size               ,
                                            file_date_updated       ,
                                            file_catalog            ,
                                            file_full_path          ,
                                            file_extension          ,
                                            file_type               ,
                                            mime_type               ,
                                            mime_verified           ,
                                            type_mismatch           ,
                                            image_width             ,
                                            image_height            ,
                                            image_orientation       ,
                                            video_duration_seconds  ,
                                            video_width             ,
                                            video_height            ,
                                            video_codec             ,
                                            video_framerate         ,
                                            video_bitrate           ,
                                            audio_duration_seconds  ,
                                            audio_artist            ,
                                            audio_album             ,
                                            audio_title             ,
                                            audio_genre             ,
                                            audio_year              ,
                                            audio_track_number      ,
                                            audio_bitrate           ,
                                            audio_sample_rate       ,
                                            metadata_extended       ,
                                            metadata_extraction_date
                                            )
                                    VALUES(
                                            :file_catalog_id        ,
                                            :file_name              ,
                                            :file_folder_path       ,
                                            :file_size              ,
                                            :file_date_updated      ,
                                            :file_catalog           ,
                                            :file_full_path         ,
                                            :file_extension         ,
                                            :file_type               ,
                                            :mime_type               ,
                                            :mime_verified           ,
                                            :type_mismatch           ,
                                            :image_width             ,
                                            :image_height            ,
                                            :image_orientation       ,
                                            :video_duration_seconds  ,
                                            :video_width             ,
                                            :video_height            ,
                                            :video_codec             ,
                                            :video_framerate         ,
                                            :video_bitrate           ,
                                            :audio_duration_seconds  ,
                                            :audio_artist            ,
                                            :audio_album             ,
                                            :audio_title             ,
                                            :audio_genre             ,
                                            :audio_year              ,
                                            :audio_track_number      ,
                                            :audio_bitrate           ,
                                            :audio_sample_rate       ,
                                            :metadata_extended       ,
                                            :metadata_extraction_date
                                            )
                                    )");

                //Prepare insert query for folder
                QSqlQuery insertFolderQuery(QSqlDatabase::database(m_connectionName));
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
                    insertFileQuery.bindValue(":file_folder_path", folder);
                    insertFileQuery.bindValue(":file_date_updated",lineFileDatetime);
                    insertFileQuery.bindValue(":file_catalog",     name);
                    insertFileQuery.bindValue(":file_full_path",   lineFilePath);

                    // Handle v2.6 vs v2.8 format differences
                    if (needsMigration) {
                        // v2.6 format: only 3 columns (path, size, date)
                        // Generate file_extension and file_type from the filename
                        QString extension = fileInfo.suffix().toLower();
                        QString fileType = FileMetadata::getFileTypeFromExtension(extension);
                        QString mimeType = FileMetadata::getMimeTypeFromExtension(extension);

                        insertFileQuery.bindValue(":file_extension", extension);
                        insertFileQuery.bindValue(":file_type", fileType);
                        insertFileQuery.bindValue(":mime_type", mimeType);
                        catalogWasMigrated = true;

                        // All metadata fields are NULL for v2.6 files
                        insertFileQuery.bindValue(":image_width", QVariant());
                        insertFileQuery.bindValue(":image_height", QVariant());
                        insertFileQuery.bindValue(":image_orientation", QVariant());
                        insertFileQuery.bindValue(":video_duration_seconds", QVariant());
                        insertFileQuery.bindValue(":video_width", QVariant());
                        insertFileQuery.bindValue(":video_height", QVariant());
                        insertFileQuery.bindValue(":video_codec", QVariant());
                        insertFileQuery.bindValue(":video_framerate", QVariant());
                        insertFileQuery.bindValue(":video_bitrate", QVariant());
                        insertFileQuery.bindValue(":audio_duration_seconds", QVariant());
                        insertFileQuery.bindValue(":audio_artist", QVariant());
                        insertFileQuery.bindValue(":audio_album", QVariant());
                        insertFileQuery.bindValue(":audio_title", QVariant());
                        insertFileQuery.bindValue(":audio_genre", QVariant());
                        insertFileQuery.bindValue(":audio_year", QVariant());
                        insertFileQuery.bindValue(":audio_track_number", QVariant());
                        insertFileQuery.bindValue(":audio_bitrate", QVariant());
                        insertFileQuery.bindValue(":audio_sample_rate", QVariant());
                        insertFileQuery.bindValue(":metadata_extended", QVariant());
                        insertFileQuery.bindValue(":metadata_extraction_date", QVariant());
                    } else {
                        // v2.8 format: 28 columns including extension, type, and metadata
                        QString extension = fileInfo.suffix().toLower();

                        // Read file_extension from column 3
                        insertFileQuery.bindValue(":file_extension", fieldListCount > 3 ? lineFieldList[3] : extension);
                        // Read file_type from column 4
                        insertFileQuery.bindValue(":file_type", fieldListCount > 4 ? lineFieldList[4] : QVariant());
                        // Read mime_type from column 5
                        insertFileQuery.bindValue(":mime_type", fieldListCount > 5 ? lineFieldList[5] : QVariant());
                        // Read mime_verified from column 6 (NEW)
                        insertFileQuery.bindValue(":mime_verified", fieldListCount > 6 ? lineFieldList[6].toInt() : QVariant());
                        // Read type_mismatch from column 7 (NEW)
                        insertFileQuery.bindValue(":type_mismatch", fieldListCount > 7 ? lineFieldList[7].toInt() : QVariant());
                        // Metadata starts at column 8 (shifted from column 6)
                        insertFileQuery.bindValue(":image_width", fieldListCount > 8 ? lineFieldList[8].toInt() : QVariant());
                        insertFileQuery.bindValue(":image_height", fieldListCount > 9 ? lineFieldList[9].toInt() : QVariant());
                        insertFileQuery.bindValue(":image_orientation", fieldListCount > 10 ? lineFieldList[10].toInt() : QVariant());
                        insertFileQuery.bindValue(":video_duration_seconds", fieldListCount > 11 ? lineFieldList[11].toDouble() : QVariant());
                        insertFileQuery.bindValue(":video_width", fieldListCount > 12 ? lineFieldList[12].toInt() : QVariant());
                        insertFileQuery.bindValue(":video_height", fieldListCount > 13 ? lineFieldList[13].toInt() : QVariant());
                        insertFileQuery.bindValue(":video_codec", fieldListCount > 14 ? lineFieldList[14] : QVariant());
                        insertFileQuery.bindValue(":video_framerate", fieldListCount > 15 ? lineFieldList[15].toDouble() : QVariant());
                        insertFileQuery.bindValue(":video_bitrate", fieldListCount > 16 ? lineFieldList[16].toInt() : QVariant());
                        insertFileQuery.bindValue(":audio_duration_seconds", fieldListCount > 17 ? lineFieldList[17].toDouble() : QVariant());
                        insertFileQuery.bindValue(":audio_artist", fieldListCount > 18 ? lineFieldList[18] : QVariant());
                        insertFileQuery.bindValue(":audio_album", fieldListCount > 19 ? lineFieldList[19] : QVariant());
                        insertFileQuery.bindValue(":audio_title", fieldListCount > 20 ? lineFieldList[20] : QVariant());
                        insertFileQuery.bindValue(":audio_genre", fieldListCount > 21 ? lineFieldList[21] : QVariant());
                        insertFileQuery.bindValue(":audio_year", fieldListCount > 22 ? lineFieldList[22].toInt() : QVariant());
                        insertFileQuery.bindValue(":audio_track_number", fieldListCount > 23 ? lineFieldList[23].toInt() : QVariant());
                        insertFileQuery.bindValue(":audio_bitrate", fieldListCount > 24 ? lineFieldList[24].toInt() : QVariant());
                        insertFileQuery.bindValue(":audio_sample_rate", fieldListCount > 25 ? lineFieldList[25].toInt() : QVariant());
                        insertFileQuery.bindValue(":metadata_extended", fieldListCount > 26 ? lineFieldList[26] : QVariant());
                        insertFileQuery.bindValue(":metadata_extraction_date", fieldListCount > 27 ? lineFieldList[27] : QVariant());      }
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
                setDateLoaded(emptyDateTime);

                //Close file
                catalogFile.close();

                if (catalogWasMigrated) {
                    qDebug() << "Catalog migrated, saving to v2.8 format...";
                    appVersion = "2.8";

                    QFileInfo catalogFileInfo(filePath);
                    QString collectionFolder = catalogFileInfo.absolutePath();

                    if (saveCatalogToFile("Memory", collectionFolder)) {
                        qDebug() << "Catalog successfully saved in v2.8 format";
                    }
                }
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

        QSqlQuery insertFolderQuery(QSqlDatabase::database(m_connectionName));
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
            QSqlQuery deleteQuery(QSqlDatabase::database(m_connectionName));
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
            loadCatalogFileListToTable(tempMutex, tempStopRequested);

            //Get list of folders
            QSqlQuery selectFoldersQuery(QSqlDatabase::database(m_connectionName));
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
    QSqlQuery querySaveStatistics(QSqlDatabase::database(m_connectionName));
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
    QSqlQuery query(QSqlDatabase::database(m_connectionName));
    QString querySQL = QLatin1String(R"(
                                    SELECT COUNT(*)
                                    FROM   catalog
                                    WHERE  catalog_id = :catalog_id
                                )");

    query.prepare(querySQL);
    query.bindValue(":catalog_id", ID);

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
{
    if (fileType == "Image") {
        fileExtensions = FileTypeMapping::getExtensionsForCataloging("image");
    } else if (fileType == "Audio") {
        fileExtensions = FileTypeMapping::getExtensionsForCataloging("audio");
    } else if (fileType == "Video") {
        fileExtensions = FileTypeMapping::getExtensionsForCataloging("video");
    } else if (fileType == "Text") {
        fileExtensions = FileTypeMapping::getExtensionsForCataloging("text");
    } else if (fileType == "Other") {
        fileExtensions = FileTypeMapping::getExtensionsForCataloging("other");
    } else if (fileType == "None") {
        // For extensionless files, this method isn't the right approach
        // The actual filtering happens in catalogjobstoppable.cpp
        fileExtensions = QStringList(); // Empty - manual filtering used instead
    } else {
        // For "All" or any other type, include all extensions
        fileExtensions << "*"; // Include all files
    }
}

void Catalog::populateFileTypes()
{
    // Check if this catalog needs migration
    QSqlQuery checkQuery(QSqlDatabase::database(m_connectionName));
    checkQuery.prepare("SELECT COUNT(*) FROM file WHERE file_catalog_id = ? "
                       "AND (file_type IS NULL OR file_type = '' "
                       "OR file_extension IS NULL OR file_extension = '' "
                       "OR mime_type IS NULL OR mime_type = '')");
    checkQuery.bindValue(0, ID);

    if (!checkQuery.exec() || !checkQuery.next()) {
        qDebug() << "Failed to check migration status for catalog:" << name;
        return;
    }

    int filesToMigrate = checkQuery.value(0).toInt();

    if (filesToMigrate == 0) {
        return;  // Already migrated
    }

    qDebug() << "Catalog" << name << "needs migration for" << filesToMigrate << "files";

    // Emit initial progress
    emit loadProgress(0, filesToMigrate);
    QCoreApplication::processEvents();  // Keep UI responsive
qDebug() << "= loadProgress:";
    QSqlDatabase db = QSqlDatabase::database(m_connectionName);

    if (!db.transaction()) {
        qDebug() << "Failed to begin transaction for catalog migration";
        return;
    }

    // Select files needing migration
    QSqlQuery selectQuery(db);
    selectQuery.prepare(R"(
        SELECT file_name, file_full_path
        FROM file
        WHERE file_catalog_id = ?
        AND (file_type IS NULL OR file_type = ''
             OR file_extension IS NULL OR file_extension = ''
             OR mime_type IS NULL OR mime_type = '')
    )");
    selectQuery.bindValue(0, ID);

    if (!selectQuery.exec()) {
        db.rollback();
        qDebug() << "Failed to select files for catalog migration:" << selectQuery.lastError().text();
        return;
    }

    // Prepare update query
    QSqlQuery updateQuery(db);
    updateQuery.prepare(R"(
        UPDATE file
        SET file_extension = ?, file_type = ?, mime_type = ?
        WHERE file_catalog_id = ? AND file_full_path = ?
    )");

    int processed = 0;
    int progressRefreshRate = qMax(100, filesToMigrate / 100);  // Update every 1% or at least every 100 files

    while (selectQuery.next()) {
        QString fileName = selectQuery.value(0).toString();
        QString fileFullPath = selectQuery.value(1).toString();

        // Determine extension, type, and mime_type from extension
        QString nameForExtraction = fileName.isEmpty() ? fileFullPath : fileName;
        QFileInfo fileInfo(nameForExtraction);
        QString extension = fileInfo.suffix().toLower();
        QString fileType = extension.isEmpty() ? "none" : FileMetadata::getFileTypeFromExtension(extension);
        QString mimeType = extension.isEmpty() ? "" : FileMetadata::getMimeTypeFromExtension(extension);

        // Update the record
        updateQuery.bindValue(0, extension);
        updateQuery.bindValue(1, fileType);
        updateQuery.bindValue(2, mimeType);
        updateQuery.bindValue(3, ID);
        updateQuery.bindValue(4, fileFullPath);

        if (!updateQuery.exec()) {
            qDebug() << "Failed to update file:" << fileFullPath;
        }

        processed++;

        // Emit progress periodically
        if (processed % progressRefreshRate == 0) {
            emit loadProgress(processed, filesToMigrate);
            QCoreApplication::processEvents();  // Keep UI responsive
            qDebug() << &"= processed:"[processed]+filesToMigrate;
        }
    }

    // Commit transaction
    if (!db.commit()) {
        db.rollback();
        qDebug() << "Failed to commit catalog migration:" << db.lastError().text();
        return;
    }

    // Final progress update
    emit loadProgress(processed, filesToMigrate);
    QCoreApplication::processEvents();

    qDebug() << "Migrated" << processed << "files for catalog:" << name;
}

void Catalog::loadExcludedFolders()
{
    QSqlQuery query(QSqlDatabase::database(m_connectionName));
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
        QSqlQuery queryFileList(QSqlDatabase::database(m_connectionName));
        QString queryFileListSQL = QLatin1String(R"(
                        SELECT  file_full_path          ,
                                file_size               ,
                                file_date_updated       ,
                                file_extension          ,
                                file_type               ,
                                mime_type               ,
                                mime_verified           ,
                                type_mismatch           ,
                                image_width             ,
                                image_height            ,
                                image_orientation       ,
                                video_duration_seconds  ,
                                video_width             ,
                                video_height            ,
                                video_codec             ,
                                video_framerate         ,
                                video_bitrate           ,
                                audio_duration_seconds  ,
                                audio_artist            ,
                                audio_album             ,
                                audio_title             ,
                                audio_genre             ,
                                audio_year              ,
                                audio_track_number      ,
                                audio_bitrate           ,
                                audio_sample_rate       ,
                                metadata_extended       ,
                                metadata_extraction_date
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
            QString fileEntry = queryFileList.value(0).toString() + "\t" +   // file_full_path
                                queryFileList.value(1).toString() + "\t" +   // file_size
                                queryFileList.value(2).toString() + "\t" +   // file_date_updated
                                queryFileList.value(3).toString() + "\t" +   // file_extension
                                queryFileList.value(4).toString() + "\t" +   // file_type
                                queryFileList.value(5).toString() + "\t" +   // mime_type
                                queryFileList.value(6).toString() + "\t" +   // mime_verified
                                queryFileList.value(7).toString() + "\t" +   // type_mismatch
                                queryFileList.value(8).toString() + "\t" +   // image_width
                                queryFileList.value(9).toString() + "\t" +   // image_height
                                queryFileList.value(10).toString() + "\t" +  // image_orientation
                                queryFileList.value(11).toString() + "\t" +  // video_duration_seconds
                                queryFileList.value(12).toString() + "\t" +  // video_width
                                queryFileList.value(13).toString() + "\t" +  // video_height
                                queryFileList.value(14).toString() + "\t" +  // video_codec
                                queryFileList.value(15).toString() + "\t" +  // video_framerate
                                queryFileList.value(16).toString() + "\t" +  // video_bitrate
                                queryFileList.value(17).toString() + "\t" +  // audio_duration_seconds
                                queryFileList.value(18).toString() + "\t" +  // audio_artist
                                queryFileList.value(19).toString() + "\t" +  // audio_album
                                queryFileList.value(20).toString() + "\t" +  // audio_title
                                queryFileList.value(21).toString() + "\t" +  // audio_genre
                                queryFileList.value(22).toString() + "\t" +  // audio_year
                                queryFileList.value(23).toString() + "\t" +  // audio_track_number
                                queryFileList.value(24).toString() + "\t" +  // audio_bitrate
                                queryFileList.value(25).toString() + "\t" +  // audio_sample_rate
                                queryFileList.value(26).toString() + "\t" +  // metadata_extended
                                queryFileList.value(27).toString() + "\t";   // metadata_extraction_date
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
        QSqlQuery query(QSqlDatabase::database(m_connectionName));
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

void Catalog::generateTempID()
{
    m_tempID = ID + 999999;
    qDebug() << "Generated temp catalog ID:" << m_tempID << "for catalog:" << ID;
}

void Catalog::moveFilesToTempID()
{
    if (m_tempID == 0) {
        qDebug() << "ERROR: Temp ID not generated before moving files";
        return;
    }

    qDebug() << "Moving existing files from catalog ID" << ID << "to temp ID" << m_tempID;

    // Move files to temp ID
    QSqlQuery moveFilesQuery(QSqlDatabase::database(m_connectionName));
    QString moveFilesSQL = "UPDATE file SET file_catalog_id = :temp_id WHERE file_catalog_id = :catalog_id";
    moveFilesQuery.prepare(moveFilesSQL);
    moveFilesQuery.bindValue(":temp_id", m_tempID);
    moveFilesQuery.bindValue(":catalog_id", ID);

    if (!moveFilesQuery.exec()) {
        qDebug() << "Error moving files to temp ID:" << moveFilesQuery.lastError().text();
        return;
    }

    qDebug() << "Moved" << moveFilesQuery.numRowsAffected() << "files to temp ID";

    // Move folders to temp ID
    QSqlQuery moveFoldersQuery(QSqlDatabase::database(m_connectionName));
    QString moveFoldersSQL = "UPDATE folder SET folder_catalog_id = :temp_id WHERE folder_catalog_id = :catalog_id";
    moveFoldersQuery.prepare(moveFoldersSQL);
    moveFoldersQuery.bindValue(":temp_id", m_tempID);
    moveFoldersQuery.bindValue(":catalog_id", ID);

    if (!moveFoldersQuery.exec()) {
        qDebug() << "Error moving folders to temp ID:" << moveFoldersQuery.lastError().text();
        return;
    }

    qDebug() << "Moved" << moveFoldersQuery.numRowsAffected() << "folders to temp ID";
}

void Catalog::restoreFromTempID()
{
    if (m_tempID == 0) {
        qDebug() << "No temp ID to restore from";
        return;
    }

    qDebug() << "Restoring files from temp ID" << m_tempID << "to catalog ID" << ID;

    // Delete any current files first (partial new scan)
    QSqlQuery deleteCurrentQuery(QSqlDatabase::database(m_connectionName));
    QString deleteCurrentSQL = "DELETE FROM file WHERE file_catalog_id = :catalog_id";
    deleteCurrentQuery.prepare(deleteCurrentSQL);
    deleteCurrentQuery.bindValue(":catalog_id", ID);
    deleteCurrentQuery.exec();

    QSqlQuery deleteCurrentFoldersQuery(QSqlDatabase::database(m_connectionName));
    QString deleteCurrentFoldersSQL = "DELETE FROM folder WHERE folder_catalog_id = :catalog_id";
    deleteCurrentFoldersQuery.prepare(deleteCurrentFoldersSQL);
    deleteCurrentFoldersQuery.bindValue(":catalog_id", ID);
    deleteCurrentFoldersQuery.exec();

    // Restore files from temp ID
    QSqlQuery restoreFilesQuery(QSqlDatabase::database(m_connectionName));
    QString restoreFilesSQL = "UPDATE file SET file_catalog_id = :catalog_id WHERE file_catalog_id = :temp_id";
    restoreFilesQuery.prepare(restoreFilesSQL);
    restoreFilesQuery.bindValue(":catalog_id", ID);
    restoreFilesQuery.bindValue(":temp_id", m_tempID);

    if (!restoreFilesQuery.exec()) {
        qDebug() << "Error restoring files from temp ID:" << restoreFilesQuery.lastError().text();
        return;
    }

    qDebug() << "Restored" << restoreFilesQuery.numRowsAffected() << "files from temp ID";

    // Restore folders from temp ID
    QSqlQuery restoreFoldersQuery(QSqlDatabase::database(m_connectionName));
    QString restoreFoldersSQL = "UPDATE folder SET folder_catalog_id = :catalog_id WHERE folder_catalog_id = :temp_id";
    restoreFoldersQuery.prepare(restoreFoldersSQL);
    restoreFoldersQuery.bindValue(":catalog_id", ID);
    restoreFoldersQuery.bindValue(":temp_id", m_tempID);

    if (!restoreFoldersQuery.exec()) {
        qDebug() << "Error restoring folders from temp ID:" << restoreFoldersQuery.lastError().text();
        return;
    }

    qDebug() << "Restored" << restoreFoldersQuery.numRowsAffected() << "folders from temp ID";

    m_tempID = 0; // Reset temp ID
}

void Catalog::cleanupTempID()
{
    if (m_tempID == 0) {
        qDebug() << "No temp ID to cleanup";
        return;
    }

    qDebug() << "Cleaning up temp ID" << m_tempID;

    // Delete temp files
    QSqlQuery deleteTempQuery(QSqlDatabase::database(m_connectionName));
    QString deleteTempSQL = "DELETE FROM file WHERE file_catalog_id = :temp_id";
    deleteTempQuery.prepare(deleteTempSQL);
    deleteTempQuery.bindValue(":temp_id", m_tempID);

    if (!deleteTempQuery.exec()) {
        qDebug() << "Error deleting temp files:" << deleteTempQuery.lastError().text();
        return;
    }

    qDebug() << "Deleted" << deleteTempQuery.numRowsAffected() << "temp files";

    // Delete temp folders
    QSqlQuery deleteTempFoldersQuery(QSqlDatabase::database(m_connectionName));
    QString deleteTempFoldersSQL = "DELETE FROM folder WHERE folder_catalog_id = :temp_id";
    deleteTempFoldersQuery.prepare(deleteTempFoldersSQL);
    deleteTempFoldersQuery.bindValue(":temp_id", m_tempID);

    if (!deleteTempFoldersQuery.exec()) {
        qDebug() << "Error deleting temp folders:" << deleteTempFoldersQuery.lastError().text();
        return;
    }

    qDebug() << "Deleted" << deleteTempFoldersQuery.numRowsAffected() << "temp folders";

    m_tempID = 0; // Reset temp ID
}

int Catalog::getTempID() const
{
    return m_tempID;
}
