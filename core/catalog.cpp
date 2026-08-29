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
#include <QCoreApplication>
#include <QDir>
#include <QSqlError>
#include <QVersionNumber>

const QString Catalog::METADATA_NONE = "None";
const QString Catalog::METADATA_MEDIA_BASIC = "MediaBasic";
const QString Catalog::METADATA_MEDIA_EXTENDED = "MediaExtended";
const QString Catalog::METADATA_FULL = "FullExtended";
const QString Catalog::CHECKSUM_NONE = "None";
const QString Catalog::CHECKSUM_SHA256 = "SHA256";

QString Catalog::metadataLevelDisplayName(const QString &internalValue)
{// Maps internal DB metadata level values to their English display names
    if (internalValue == METADATA_NONE)           return "None";
    if (internalValue == "MimeOnly")              return "MIME Type Only";
    if (internalValue == METADATA_MEDIA_BASIC)    return "Media Basic";
    if (internalValue == METADATA_MEDIA_EXTENDED) return "Extended Custom";
    if (internalValue == "ExtendedCustom")        return "Extended Custom";
    if (internalValue == METADATA_FULL)           return "Extended Full";
    if (internalValue == "ExtendedFull")          return "Extended Full";
    return internalValue; // unknown value, display as-is
}

Catalog::Catalog(QObject *parent) : QAbstractTableModel(parent), workerThread(nullptr) {
    includeMetadata = METADATA_NONE;
    includeChecksum = CHECKSUM_NONE;
    includeSubDir   = true;
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
    if (!index.isValid()) return QVariant();

    if (role == Qt::DisplayRole) {
        switch (index.column()){
        case 0: return QString(fileNames[index.row()]);
        case 1: return qint64 (fileSizes[index.row()]);
        case 2: return QString(fileDateTimes[index.row()]);
        case 3: return QString(filePaths[index.row()]);
        case 4: return QString(fileCatalogs[index.row()]);
        }
        return QVariant();
    }

    // Named roles for QML
    const int row = index.row();
    switch (role) {
    case FileNameRole:   return QString(fileNames[row]);
    case FileSizeRole:   return qint64(fileSizes[row]);
    case FileDateRole:   return QString(fileDateTimes[row]);
    case FolderPathRole: return QString(filePaths[row]);
    case CatalogRole:    return QString(fileCatalogs[row]);
    }
    return QVariant();
}

QHash<int, QByteArray> Catalog::roleNames() const
{
    QHash<int, QByteArray> roles;
    roles[FileNameRole]   = "fileName";
    roles[FileSizeRole]   = "fileSize";
    roles[FileDateRole]   = "fileDate";
    roles[FolderPathRole] = "folderPath";
    roles[CatalogRole]    = "catalogName";
    return roles;
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
QString Catalog::normalizeSourcePath(const QString &selectedSourcePath)
{
    QString normalizedPath = selectedSourcePath;

    if (normalizedPath.isEmpty())
        return normalizedPath;

    //Keep filesystem roots as they are: "/" (Linux) and "X:/" (Windows)
    if (normalizedPath == "/")
        return normalizedPath;
    if (normalizedPath.length() == 3 and normalizedPath.at(1) == ':' and normalizedPath.at(2) == '/')
        return normalizedPath;

    //Remove the / at the end if any
    if (normalizedPath.endsWith('/'))
        normalizedPath.chop(1);

    return normalizedPath;
}
void Catalog::setSourcePath(QString selectedSourcePath)
{
    sourcePath = normalizeSourcePath(selectedSourcePath);
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
        catalogQuery.bindValue(":catalog_id", ID);
        catalogQuery.exec();
    }
    else
        dateUpdated = dateTime;
}

//catalog files data operation
void Catalog::generateID()
{
    int maxID = 0;
    QSqlQuery queryCatalogID(QSqlDatabase::database(m_connectionName));
    QString queryCatalogIDSQL = QLatin1String(R"(
                            SELECT MAX(catalog_id)
                            FROM catalog
                        )");
    queryCatalogID.prepare(queryCatalogIDSQL);

    if (!queryCatalogID.exec()) {
        qWarning() << "WARNING: generateID query failed:" << queryCatalogID.lastError().text();
        ID = 1;  // ✓ Fallback to 1 on error
        return;
    }

    if(queryCatalogID.next()){
        QVariant value = queryCatalogID.value(0);
        if (value.isNull()) {
            maxID = 0;  // Empty table
        } else {
            maxID = value.toInt();
        }
        ID = maxID + 1;
    } else {
        // No rows returned (shouldn't happen with MAX, but handle it)
        ID = 1;  // ✓ Default to 1
    }

}

void Catalog::insertCatalog()
{
    QSqlQuery insertCatalogQuery(QSqlDatabase::database(m_connectionName));
    QString insertCatalogQuerySQL = QLatin1String(R"(
                                INSERT INTO catalog(
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
                                                        catalog_include_checksum,
                                                        catalog_app_version,
                                                        catalog_include_sub_dir
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
                                                        :catalog_include_checksum,
                                                        :catalog_app_version,
                                                        :catalog_include_sub_dir )
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
    insertCatalogQuery.bindValue(":catalog_include_checksum", includeChecksum);
    insertCatalogQuery.bindValue(":catalog_app_version", appVersion);
    insertCatalogQuery.bindValue(":catalog_include_sub_dir", includeSubDir);
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
    query.bindValue(":file_catalog_id", ID);
    query.exec();

    querySQL = QLatin1String(R"(
                        DELETE FROM folder
                        WHERE folder_catalog_id =:folder_catalog_id
                    )");
    query.prepare(querySQL);
    query.bindValue(":folder_catalog_id", ID);
    query.exec();
}

void Catalog::saveCatalog()
{
    QSqlQuery query(QSqlDatabase::database(m_connectionName));
    QString querySQL = QLatin1String(R"(
        UPDATE catalog
        SET catalog_name              =:catalog_name,
            catalog_source_path       =:catalog_source_path,
            catalog_storage           =:catalog_storage,
            catalog_file_type         =:catalog_file_type,
            catalog_include_hidden    =:catalog_include_hidden,
            catalog_include_metadata  =:catalog_include_metadata,
            catalog_include_checksum  =:catalog_include_checksum,
            catalog_include_symblinks =:catalog_include_symblinks,
            catalog_app_version       =:catalog_app_version,
            catalog_include_sub_dir   =:catalog_include_sub_dir
        WHERE catalog_id=:catalog_id
    )");
    query.prepare(querySQL);
    query.bindValue(":catalog_id", ID);
    query.bindValue(":catalog_name", name);
    query.bindValue(":catalog_source_path", sourcePath);
    query.bindValue(":catalog_storage", storageName);
    query.bindValue(":catalog_file_type", fileType);
    query.bindValue(":catalog_include_hidden", includeHidden);
    query.bindValue(":catalog_include_metadata", includeMetadata);
    query.bindValue(":catalog_include_checksum", includeChecksum);
    query.bindValue(":catalog_include_symblinks", includeSymblinks);
    query.bindValue(":catalog_app_version", appVersion);
    query.bindValue(":catalog_include_sub_dir", includeSubDir);
    query.exec();
}

void Catalog::clearCatalogData()
{

    // Clear files from database (use catalog ID, not name)
    QString deleteFilesSQL = "DELETE FROM file WHERE file_catalog_id = :file_catalog_id";
    QSqlQuery query(QSqlDatabase::database(m_connectionName));
    query.prepare(deleteFilesSQL);
    query.bindValue(":file_catalog_id", ID);  // Use ID, not name

    if (!query.exec()) {
        qWarning() << "WARNING: Error clearing catalog files:" << query.lastError().text();
    } else {
    }

    // Clear folders from database (use catalog ID, not name)
    QString deleteFoldersSQL = "DELETE FROM folder WHERE folder_catalog_id = :folder_catalog_id";
    query.prepare(deleteFoldersSQL);
    query.bindValue(":folder_catalog_id", ID);  // Use ID, not name

    if (!query.exec()) {
        qWarning() << "WARNING: Error clearing catalog folders:" << query.lastError().text();
    } else {
    }

    // Reset counters
    fileCount = 0;
    totalFileSize = 0;
}

bool Catalog::updateCatalogFileHeaders(QString databaseMode)
{
    if(databaseMode == "Memory") {

        QFile catalogFile(filePath);
        if(!catalogFile.open(QIODevice::ReadWrite | QIODevice::Text)) {
            return false;
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
        fullFileText.append("<catalogIncludeSubDir>"   + QVariant(includeSubDir).toString()   +"\n");
        fullFileText.append("<catalogIsFullDevice>" + QVariant(isFullDevice).toString() +"\n");
        fullFileText.append("<catalogIncludeMetadata>" + QVariant(includeMetadata).toString() +"\n");
        fullFileText.append("<catalogIncludeChecksum>" + QVariant(includeChecksum).toString() +"\n");  // ADD THIS LINE
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
    else {
        return false;
    }
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
                                catalog_include_checksum     ,
                                catalog_app_version          ,
                                catalog_include_sub_dir
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
        includeChecksum    = query.value(14).toString();
        appVersion         = query.value(15).toString();
        includeSubDir      = query.value(16).isNull() ? true : query.value(16).toBool();
    }
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

    // Keep denormalized file_catalog display copy in sync
    QSqlQuery syncQuery(QSqlDatabase::database(m_connectionName));
    syncQuery.prepare(QLatin1String(R"(
                                UPDATE file
                                SET   file_catalog=:new_catalog_name
                                WHERE file_catalog_id=:catalog_id
                            )"));
    syncQuery.bindValue(":new_catalog_name", newCatalogName);
    syncQuery.bindValue(":catalog_id", ID);
    syncQuery.exec();

    //Rename value of current object
    name = newCatalogName;

}

void Catalog::renameCatalogFile(QString newCatalogName)
{
    QFileInfo catalogFileInfo(filePath);

    //Rename folders file
    QString currentFolderFilePath = filePath;

    if (currentFolderFilePath.right(4)==".idx"){
        currentFolderFilePath = currentFolderFilePath.chopped(4); //remove the .idx extension for now
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
                bool needsMigration = (QVersionNumber::fromString(appVersion) < QVersionNumber::fromString("2.8"));
                bool catalogWasMigrated = false;

                if (needsMigration) {
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
                                            metadata_extraction_date,
                                            checksum_sha256         ,
                                            checksum_extraction_date
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
                                            :file_type              ,
                                            :mime_type              ,
                                            :mime_verified          ,
                                            :type_mismatch          ,
                                            :image_width            ,
                                            :image_height           ,
                                            :image_orientation      ,
                                            :video_duration_seconds ,
                                            :video_width            ,
                                            :video_height           ,
                                            :video_codec            ,
                                            :video_framerate        ,
                                            :video_bitrate          ,
                                            :audio_duration_seconds ,
                                            :audio_artist           ,
                                            :audio_album            ,
                                            :audio_title            ,
                                            :audio_genre            ,
                                            :audio_year             ,
                                            :audio_track_number     ,
                                            :audio_bitrate          ,
                                            :audio_sample_rate      ,
                                            :metadata_extended      ,
                                            :metadata_extraction_date,
                                            :checksum_sha256        ,
                                            :checksum_extraction_date
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
                        insertFileQuery.bindValue(":checksum_sha256", QVariant());
                        insertFileQuery.bindValue(":checksum_extraction_date", QVariant());
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
                        insertFileQuery.bindValue(":metadata_extraction_date", fieldListCount > 27 ? lineFieldList[27] : QVariant());
                        insertFileQuery.bindValue(":checksum_sha256", fieldListCount > 28 ? lineFieldList[28] : QVariant());
                        insertFileQuery.bindValue(":checksum_extraction_date", fieldListCount > 29 ? lineFieldList[29] : QVariant());
                    }
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
                    appVersion = "2.9";

                    QFileInfo catalogFileInfo(filePath);
                    QString collectionFolder = catalogFileInfo.absolutePath();

                    if (saveCatalogToFile("Memory", collectionFolder)) {
                    }

                    // After saving, check if migration is 100% complete
                    if (!hasFilesNeedingMigration() && QVersionNumber::fromString(appVersion) < QVersionNumber::fromString("2.9")) {
                        appVersion = "2.9";
                        saveCatalog();  // Update database
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

void Catalog::populateFileTypes(QMutex &mutex, bool &stopRequested)
{
    Q_UNUSED(mutex);  // Using callbacks instead


    // Use the unified method with progress callback
    FileMetadata::migrateFileTypesForCatalog(
        m_connectionName,
        ID,
        // Progress callback
        [this](int processed, int total, QString message) {
            emit loadProgress(processed, total);
            QCoreApplication::processEvents();
        },
        // Should continue callback
        [&stopRequested]() -> bool {
            return !stopRequested;
        }
        );

}

void Catalog::loadExcludedFolders()
{
    excludedFolders.clear();

    // Global exclusions (apply to all catalogs)
    QSqlQuery globalQuery(QSqlDatabase::database(m_connectionName));
    globalQuery.prepare(QLatin1String(R"(
        SELECT DISTINCT parameter_value2
        FROM parameter
        WHERE parameter_type = 'exclude_directory'
        ORDER BY parameter_value2
    )"));
    globalQuery.exec();
    while (globalQuery.next()) {
        QString folder = globalQuery.value(0).toString();
        while (folder.endsWith('/') || folder.endsWith('\\'))
            folder.chop(1);
        if (!folder.isEmpty())
            excludedFolders << folder;
    }

    // Per-catalog exclusions
    if (ID > 0) {
        const QStringList perCatalog = getExcludeFolders();
        for (const QString &folder : perCatalog) {
            if (!excludedFolders.contains(folder))
                excludedFolders << folder;
        }
    }
}

QStringList Catalog::getExcludeFolders() const
{
    QStringList result;
    QSqlQuery query(QSqlDatabase::database(m_connectionName));
    query.prepare(QLatin1String(R"(
        SELECT filter_value
        FROM catalog_filter
        WHERE filter_catalog_id = :id
          AND filter_type = 'exclude_folder'
        ORDER BY filter_value
    )"));
    query.bindValue(":id", ID);
    query.exec();
    while (query.next()) {
        QString folder = query.value(0).toString();
        while (folder.endsWith('/') || folder.endsWith('\\'))
            folder.chop(1);
        if (!folder.isEmpty())
            result << folder;
    }
    return result;
}

bool Catalog::addExcludeFolder(const QString &path)
{
    QString folder = path;
    while (folder.endsWith('/') || folder.endsWith('\\'))
        folder.chop(1);
    if (folder.isEmpty() || ID <= 0)
        return false;

    QSqlQuery query(QSqlDatabase::database(m_connectionName));
    query.prepare(QLatin1String(R"(
        INSERT OR IGNORE INTO catalog_filter (filter_catalog_id, filter_type, filter_value)
        VALUES (:id, 'exclude_folder', :value)
    )"));
    query.bindValue(":id", ID);
    query.bindValue(":value", folder);
    return query.exec();
}

bool Catalog::removeExcludeFolder(const QString &path)
{
    QString folder = path;
    while (folder.endsWith('/') || folder.endsWith('\\'))
        folder.chop(1);
    if (folder.isEmpty() || ID <= 0)
        return false;

    QSqlQuery query(QSqlDatabase::database(m_connectionName));
    query.prepare(QLatin1String(R"(
        DELETE FROM catalog_filter
        WHERE filter_catalog_id = :id
          AND filter_type = 'exclude_folder'
          AND filter_value = :value
    )"));
    query.bindValue(":id", ID);
    query.bindValue(":value", folder);
    return query.exec();
}

bool Catalog::saveCatalogToFile(QString databaseMode, QString collectionFolder)
{
    if(databaseMode != "Memory") {
        return true; // Nothing to do for non-Memory mode
    }


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
                                metadata_extraction_date,
                                checksum_sha256         ,
                                checksum_extraction_date
                        FROM file
                        WHERE file_catalog_id = :file_catalog_id
                        ORDER BY file_full_path
                    )");
        queryFileList.prepare(queryFileListSQL);
        queryFileList.bindValue(":file_catalog_id", ID);

        if (!queryFileList.exec()) {
            qWarning() << "WARNING: Failed to query file list:" << queryFileList.lastError().text();
            return false;
        }


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
                                queryFileList.value(27).toString() + "\t" +  // metadata_extraction_date
                                queryFileList.value(28).toString() + "\t" +  // checksum_sha256
                                queryFileList.value(29).toString() + "\t";   // checksum_extraction_date
            fileList << fileEntry;
        }


        // Prepare the catalog file data, adding headers at the beginning (same as original)
        fileList.prepend("<catalogID>"              + QString::number(ID));
        fileList.prepend("<catalogAppVersion>"      + appVersion);
        fileList.prepend("<catalogIncludeChecksum>" + QVariant(includeChecksum).toString());
        fileList.prepend("<catalogIncludeMetadata>" + QVariant(includeMetadata).toString());
        fileList.prepend("<catalogIsFullDevice>"    + QVariant(isFullDevice).toString());
        fileList.prepend("<catalogIncludeSubDir>"   + QVariant(includeSubDir).toString());
        fileList.prepend("<catalogIncludeSymblinks>"+ QVariant(includeSymblinks).toString());
        fileList.prepend("<catalogStorage>"         + storageName);
        fileList.prepend("<catalogFileType>"        + fileType);
        fileList.prepend("<catalogIncludeHidden>"   + QVariant(includeHidden).toString());
        fileList.prepend("<catalogTotalFileSize>"   + QString::number(totalFileSize));
        fileList.prepend("<catalogFileCount>"       + QString::number(fileCount));
        fileList.prepend("<catalogSourcePath>"      + sourcePath);


        // Write to file
        filePath = collectionFolder + "/" + name + ".idx";

        QFile fileOut(filePath);
        if (!fileOut.open(QFile::WriteOnly | QFile::Text)) {
            qWarning() << "WARNING: Failed to open file for writing:" << filePath;
            return false;
        }

        QTextStream stream(&fileOut);
        for (int i = 0; i < fileList.size(); ++i) {
            stream << fileList.at(i) << '\n';
        }

        fileOut.close();

        return true;

    } catch (const std::exception& e) {
        qWarning() << "WARNING: === EXCEPTION in saveCatalogToFile():" << e.what() << "===";
        return false;
    } catch (...) {
        qWarning() << "WARNING: === UNKNOWN EXCEPTION in saveCatalogToFile() ===";
        return false;
    }
}

bool Catalog::saveFoldersToFile(QString databaseMode, QString collectionFolder)
{
    if(databaseMode != "Memory") {
        return true; // Nothing to do for non-Memory mode
    }


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
            qWarning() << "WARNING: Failed to query folders:" << query.lastError().text();
            return false;
        }

        // Build folder list
        QStringList folderList;
        while (query.next()) {
            folderList << query.value(0).toString();
        }


        // Write to file
        QString foldersFilePath = collectionFolder + "/" + name + ".folders.idx";
        QFile fileOut(foldersFilePath);
        if (!fileOut.open(QFile::WriteOnly | QFile::Text)) {
            qWarning() << "WARNING: Failed to open folders file:" << foldersFilePath;
            return false;
        }

        QTextStream stream(&fileOut);
        for (const QString &folder : folderList) {
            stream << ID << '\t' << folder << '\n';
        }

        fileOut.close();

        return true;

    } catch (const std::exception& e) {
        qWarning() << "WARNING: === EXCEPTION in saveFoldersToFile():" << e.what() << "===";
        return false;
    } catch (...) {
        qWarning() << "WARNING: === UNKNOWN EXCEPTION in saveFoldersToFile() ===";
        return false;
    }
}

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
}

void Catalog::moveFilesToTempID()
{
    if (m_tempID == 0) {
        qWarning() << "WARNING: Temp ID not generated before moving files";
        return;
    }


    // Move files to temp ID
    QSqlQuery moveFilesQuery(QSqlDatabase::database(m_connectionName));
    QString moveFilesSQL = "UPDATE file SET file_catalog_id = :temp_id WHERE file_catalog_id = :catalog_id";
    moveFilesQuery.prepare(moveFilesSQL);
    moveFilesQuery.bindValue(":temp_id", m_tempID);
    moveFilesQuery.bindValue(":catalog_id", ID);

    if (!moveFilesQuery.exec()) {
        qWarning() << "WARNING: Error moving files to temp ID:" << moveFilesQuery.lastError().text();
        return;
    }


    // Move folders to temp ID
    QSqlQuery moveFoldersQuery(QSqlDatabase::database(m_connectionName));
    QString moveFoldersSQL = "UPDATE folder SET folder_catalog_id = :temp_id WHERE folder_catalog_id = :catalog_id";
    moveFoldersQuery.prepare(moveFoldersSQL);
    moveFoldersQuery.bindValue(":temp_id", m_tempID);
    moveFoldersQuery.bindValue(":catalog_id", ID);

    if (!moveFoldersQuery.exec()) {
        qWarning() << "WARNING: Error moving folders to temp ID:" << moveFoldersQuery.lastError().text();
        return;
    }

}

void Catalog::restoreFromTempID()
{
    if (m_tempID == 0) {
        return;
    }


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
        qWarning() << "WARNING: Error restoring files from temp ID:" << restoreFilesQuery.lastError().text();
        return;
    }


    // Restore folders from temp ID
    QSqlQuery restoreFoldersQuery(QSqlDatabase::database(m_connectionName));
    QString restoreFoldersSQL = "UPDATE folder SET folder_catalog_id = :catalog_id WHERE folder_catalog_id = :temp_id";
    restoreFoldersQuery.prepare(restoreFoldersSQL);
    restoreFoldersQuery.bindValue(":catalog_id", ID);
    restoreFoldersQuery.bindValue(":temp_id", m_tempID);

    if (!restoreFoldersQuery.exec()) {
        qWarning() << "WARNING: Error restoring folders from temp ID:" << restoreFoldersQuery.lastError().text();
        return;
    }


    m_tempID = 0; // Reset temp ID
}

void Catalog::cleanupTempID()
{
    if (m_tempID == 0) {
        return;
    }


    // Delete temp files
    QSqlQuery deleteTempQuery(QSqlDatabase::database(m_connectionName));
    QString deleteTempSQL = "DELETE FROM file WHERE file_catalog_id = :temp_id";
    deleteTempQuery.prepare(deleteTempSQL);
    deleteTempQuery.bindValue(":temp_id", m_tempID);

    if (!deleteTempQuery.exec()) {
        qWarning() << "WARNING: Error deleting temp files:" << deleteTempQuery.lastError().text();
        return;
    }


    // Delete temp folders
    QSqlQuery deleteTempFoldersQuery(QSqlDatabase::database(m_connectionName));
    QString deleteTempFoldersSQL = "DELETE FROM folder WHERE folder_catalog_id = :temp_id";
    deleteTempFoldersQuery.prepare(deleteTempFoldersSQL);
    deleteTempFoldersQuery.bindValue(":temp_id", m_tempID);

    if (!deleteTempFoldersQuery.exec()) {
        qWarning() << "WARNING: Error deleting temp folders:" << deleteTempFoldersQuery.lastError().text();
        return;
    }


    m_tempID = 0; // Reset temp ID
}

int Catalog::getTempID() const
{
    return m_tempID;
}

//----------------------------------------------------------------------
// Split operations
//----------------------------------------------------------------------
QStringList Catalog::listImmediateSubdirectories() const
{
    QSqlQuery query(QSqlDatabase::database(m_connectionName));
    query.prepare(QLatin1String(R"(
        SELECT DISTINCT folder_path
        FROM folder
        WHERE folder_catalog_id = :id
        ORDER BY folder_path
    )"));
    query.bindValue(":id", ID);
    if (!query.exec()) return {};

    QString prefix = sourcePath.endsWith('/') ? sourcePath : sourcePath + '/';

    QStringList result;
    while (query.next()) {
        QString path = query.value(0).toString();
        if (!path.startsWith(prefix)) continue;
        QString relative = path.mid(prefix.length());
        if (!relative.isEmpty() && !relative.contains('/'))
            result << path;
    }
    return result;
}

QList<Catalog*> Catalog::executeSplitBySubDirectory(const QString &databaseMode,
                                                      const QString &collectionFolder)
{
    QStringList subDirs = listImmediateSubdirectories();
    if (subDirs.isEmpty())
        return {};

    QList<Catalog*> created;

    // Find a unique catalog name (append _2, _3, ... on collision)
    auto makeUniqueName = [&](const QString &baseName) -> QString {
        QString candidate = baseName;
        int suffix = 2;
        while (true) {
            QSqlQuery chk(QSqlDatabase::database(m_connectionName));
            chk.prepare("SELECT COUNT(*) FROM catalog WHERE catalog_name = :n");
            chk.bindValue(":n", candidate);
            if (chk.exec() && chk.next() && chk.value(0).toInt() == 0)
                return candidate;
            candidate = baseName + "_" + QString::number(suffix++);
        }
    };

    // Create and insert a new catalog inheriting all settings from this one
    auto createSplitCatalog = [&](const QString &newName,
                                   const QString &newSourcePath,
                                   bool newIncludeSubDir) -> Catalog* {
        Catalog *c = new Catalog();
        c->setConnectionName(m_connectionName);
        c->name             = newName;
        c->sourcePath       = newSourcePath;
        c->includeSubDir    = newIncludeSubDir;
        c->includeHidden    = includeHidden;
        c->includeSymblinks = includeSymblinks;
        c->fileType         = fileType;
        c->storageName      = storageName;
        c->isFullDevice     = isFullDevice;
        c->includeMetadata  = includeMetadata;
        c->includeChecksum  = includeChecksum;
        c->appVersion       = appVersion;
        c->dateUpdated      = dateUpdated;
        c->fileCount        = 0;
        c->totalFileSize    = 0;
        c->filePath         = collectionFolder + "/" + newName + ".idx";
        c->generateID();
        c->insertCatalog();
        return c;
    };

    // Reassign file rows to a new catalog ID, filtered by folder path
    auto moveFiles = [&](int newId, const QString &newName,
                          const QString &pathFilter, bool isRoot) {
        QSqlQuery q(QSqlDatabase::database(m_connectionName));
        if (isRoot) {
            q.prepare(QLatin1String(R"(
                UPDATE file
                SET file_catalog_id = :newId, file_catalog = :newName
                WHERE file_catalog_id = :oldId AND file_folder_path = :path
            )"));
        } else {
            q.prepare(QLatin1String(R"(
                UPDATE file
                SET file_catalog_id = :newId, file_catalog = :newName
                WHERE file_catalog_id = :oldId
                  AND (file_folder_path = :path OR file_folder_path LIKE :pathPrefix)
            )"));
            q.bindValue(":pathPrefix", pathFilter + "/%");
        }
        q.bindValue(":newId",   newId);
        q.bindValue(":newName", newName);
        q.bindValue(":oldId",   ID);
        q.bindValue(":path",    pathFilter);
        q.exec();
    };

    // Reassign folder rows to a new catalog ID, filtered by folder path
    auto moveFolders = [&](int newId, const QString &pathFilter, bool isRoot) {
        QSqlQuery q(QSqlDatabase::database(m_connectionName));
        if (isRoot) {
            q.prepare(QLatin1String(R"(
                UPDATE folder
                SET folder_catalog_id = :newId
                WHERE folder_catalog_id = :oldId AND folder_path = :path
            )"));
        } else {
            q.prepare(QLatin1String(R"(
                UPDATE folder
                SET folder_catalog_id = :newId
                WHERE folder_catalog_id = :oldId
                  AND (folder_path = :path OR folder_path LIKE :pathPrefix)
            )"));
            q.bindValue(":pathPrefix", pathFilter + "/%");
        }
        q.bindValue(":newId",  newId);
        q.bindValue(":oldId",  ID);
        q.bindValue(":path",   pathFilter);
        q.exec();
    };

    // Update the catalog row's file count and total size from the actual file data
    auto updateCounts = [&](Catalog *c) {
        QSqlQuery q(QSqlDatabase::database(m_connectionName));
        q.prepare(QLatin1String(R"(
            UPDATE catalog
            SET catalog_file_count      = (SELECT COUNT(*)                    FROM file WHERE file_catalog_id = :id),
                catalog_total_file_size = (SELECT COALESCE(SUM(file_size), 0) FROM file WHERE file_catalog_id = :id2)
            WHERE catalog_id = :id3
        )"));
        q.bindValue(":id",  c->ID);
        q.bindValue(":id2", c->ID);
        q.bindValue(":id3", c->ID);
        q.exec();
        c->updateFileCount();
        c->updateTotalFileSize();
    };

    // 1. Root catalog — files located directly at sourcePath (not in any subdirectory)
    {
        QString rootName = makeUniqueName(name + "_(" + tr("root") + ")");
        Catalog *c = createSplitCatalog(rootName, sourcePath, false);
        moveFiles(c->ID, c->name, sourcePath, true);
        moveFolders(c->ID, sourcePath, true);
        updateCounts(c);
        if (databaseMode == "Memory") {
            c->saveCatalogToFile(databaseMode, collectionFolder);
            c->saveFoldersToFile(databaseMode, collectionFolder);
        }
        created << c;
    }

    // 2. One catalog per immediate subdirectory — all descendants included
    for (const QString &subPath : std::as_const(subDirs)) {
        QString dirName = QDir(subPath).dirName();
        QString newName = makeUniqueName(name + "_" + dirName);
        Catalog *c = createSplitCatalog(newName, subPath, true);
        moveFiles(c->ID, c->name, subPath, false);
        moveFolders(c->ID, subPath, false);
        updateCounts(c);
        if (databaseMode == "Memory") {
            c->saveCatalogToFile(databaseMode, collectionFolder);
            c->saveFoldersToFile(databaseMode, collectionFolder);
        }
        created << c;
    }

    return created;
}

QList<Catalog*> Catalog::executeSplitByFileType(const QString &databaseMode,
                                                  const QString &collectionFolder)
{
    // Fixed groups: Audio, Images, Videos, Text, Other (Other absorbs None)
    struct Group {
        QString suffix;     // catalog name suffix
        QString fileType;   // catalog.fileType value used by the scanner
        QString sqlFilter;  // WHERE fragment matching file rows for this group
    };

    const QString noneFilter = FileTypeMapping::getSqlFilter(FileTypeMapping::NONE);
    const QString otherFilter= FileTypeMapping::getSqlFilter(FileTypeMapping::OTHER);

    const QList<Group> groups = {
        { "_(" + tr("Audio")  + ")", "Audio", FileTypeMapping::getSqlFilter(FileTypeMapping::AUDIO) },
        { "_(" + tr("Images") + ")", "Image", FileTypeMapping::getSqlFilter(FileTypeMapping::IMAGE) },
        { "_(" + tr("Videos") + ")", "Video", FileTypeMapping::getSqlFilter(FileTypeMapping::VIDEO) },
        { "_(" + tr("Text")   + ")", "Text",  FileTypeMapping::getSqlFilter(FileTypeMapping::TEXT)  },
        { "_(" + tr("Other")  + ")", "Other", "(" + otherFilter + ") OR (" + noneFilter + ")"       },
    };

    QList<Catalog*> created;

    // Find a unique catalog name (append _2, _3, ... on collision)
    auto makeUniqueName = [&](const QString &baseName) -> QString {
        QString candidate = baseName;
        int suffix = 2;
        while (true) {
            QSqlQuery chk(QSqlDatabase::database(m_connectionName));
            chk.prepare("SELECT COUNT(*) FROM catalog WHERE catalog_name = :n");
            chk.bindValue(":n", candidate);
            if (chk.exec() && chk.next() && chk.value(0).toInt() == 0)
                return candidate;
            candidate = baseName + "_" + QString::number(suffix++);
        }
    };

    // Update the catalog row's file count and total size from the actual file data
    auto updateCounts = [&](Catalog *c) {
        QSqlQuery q(QSqlDatabase::database(m_connectionName));
        q.prepare(QLatin1String(R"(
            UPDATE catalog
            SET catalog_file_count      = (SELECT COUNT(*)                    FROM file WHERE file_catalog_id = :id),
                catalog_total_file_size = (SELECT COALESCE(SUM(file_size), 0) FROM file WHERE file_catalog_id = :id2)
            WHERE catalog_id = :id3
        )"));
        q.bindValue(":id",  c->ID);
        q.bindValue(":id2", c->ID);
        q.bindValue(":id3", c->ID);
        q.exec();
        c->updateFileCount();
        c->updateTotalFileSize();
    };

    for (const Group &g : groups) {
        QString newName = makeUniqueName(name + g.suffix);

        Catalog *c = new Catalog();
        c->setConnectionName(m_connectionName);
        c->name             = newName;
        c->sourcePath       = sourcePath;   // same root for all file-type catalogs
        c->includeSubDir    = includeSubDir;
        c->includeHidden    = includeHidden;
        c->includeSymblinks = includeSymblinks;
        c->fileType         = g.fileType;   // scanner will filter by this on next update
        c->storageName      = storageName;
        c->isFullDevice     = isFullDevice;
        c->includeMetadata  = includeMetadata;
        c->includeChecksum  = includeChecksum;
        c->appVersion       = appVersion;
        c->dateUpdated      = dateUpdated;
        c->fileCount        = 0;
        c->totalFileSize    = 0;
        c->filePath         = collectionFolder + "/" + newName + ".idx";
        c->generateID();
        c->insertCatalog();

        // Move matching file rows to this catalog
        if (!g.sqlFilter.isEmpty()) {
            QSqlQuery qf(QSqlDatabase::database(m_connectionName));
            qf.prepare(QString(R"(
                UPDATE file
                SET file_catalog_id = :newId, file_catalog = :newName
                WHERE file_catalog_id = :oldId AND (%1)
            )").arg(g.sqlFilter));
            qf.bindValue(":newId",   c->ID);
            qf.bindValue(":newName", c->name);
            qf.bindValue(":oldId",   ID);
            qf.exec();
        }

        // Copy all folder rows (all type-split catalogs share the same folder tree)
        QSqlQuery qfold(QSqlDatabase::database(m_connectionName));
        qfold.prepare(QLatin1String(R"(
            INSERT INTO folder (folder_catalog_id, folder_path)
            SELECT :newId, folder_path
            FROM folder
            WHERE folder_catalog_id = :oldId
        )"));
        qfold.bindValue(":newId",  c->ID);
        qfold.bindValue(":oldId",  ID);
        qfold.exec();

        updateCounts(c);

        if (databaseMode == "Memory") {
            c->saveCatalogToFile(databaseMode, collectionFolder);
            c->saveFoldersToFile(databaseMode, collectionFolder);
        }

        created << c;
    }

    return created;
}

QString Catalog::getFileChecksum(const QString &fileName, const QString &folderPath) const
{// Retrieve the stored SHA-256 checksum for a specific file in this catalog
    QSqlQuery query(QSqlDatabase::database(m_connectionName));
    query.prepare(QLatin1String(R"(
        SELECT checksum_sha256
        FROM file
        WHERE file_catalog_id = :catalog_id
          AND file_name        = :file_name
          AND file_folder_path = :folder_path
    )"));
    query.bindValue(":catalog_id",   ID);
    query.bindValue(":file_name",    fileName);
    query.bindValue(":folder_path",  folderPath);

    if (query.exec() && query.next())
        return query.value(0).toString();
    return QString();
}
//----------------------------------------------------------------------
// Metadata fields management for tranistions in this catalog
bool Catalog::clearMetadataBasicFields()
{

    QSqlQuery query(QSqlDatabase::database(m_connectionName));
    QString querySQL = QLatin1String(R"(
        UPDATE file
        SET image_width = NULL,
            image_height = NULL,
            image_orientation = NULL,
            video_duration_seconds = NULL,
            video_width = NULL,
            video_height = NULL,
            video_codec = NULL,
            video_framerate = NULL,
            video_bitrate = NULL,
            audio_duration_seconds = NULL,
            audio_artist = NULL,
            audio_album = NULL,
            audio_title = NULL,
            audio_genre = NULL,
            audio_year = NULL,
            audio_track_number = NULL,
            audio_bitrate = NULL,
            audio_sample_rate = NULL
        WHERE file_catalog_id = :catalog_id
    )");

    query.prepare(querySQL);
    query.bindValue(":catalog_id", ID);

    if (!query.exec()) {
        qWarning() << "WARNING: Failed to clear metadata fields:" << query.lastError().text();
        return false;
    }

    return true;
}

bool Catalog::clearMetadataExtendedField()
{

    QSqlQuery query(QSqlDatabase::database(m_connectionName));
    QString querySQL = QLatin1String(R"(
        UPDATE file
        SET metadata_extended = NULL
        WHERE file_catalog_id = :catalog_id
    )");

    query.prepare(querySQL);
    query.bindValue(":catalog_id", ID);

    if (!query.exec()) {
        qWarning() << "WARNING: Failed to clear metadata fields:" << query.lastError().text();
        return false;
    }

    return true;
}

bool Catalog::clearMetadataExtractionDate()
{

    QSqlQuery query(QSqlDatabase::database(m_connectionName));
    QString querySQL = QLatin1String(R"(
        UPDATE file
        SET metadata_extraction_date = NULL
        WHERE file_catalog_id = :catalog_id
    )");

    query.prepare(querySQL);
    query.bindValue(":catalog_id", ID);

    if (!query.exec()) {
        qWarning() << "WARNING: Failed to clear metadata_extraction_date:" << query.lastError().text();
        return false;
    }

    return true;
}

bool Catalog::clearMetadataExtractionDateForNonMedia()
{

    QSqlQuery query(QSqlDatabase::database(m_connectionName));
    QString querySQL = QLatin1String(R"(
        UPDATE file
        SET metadata_extraction_date = NULL
        WHERE file_catalog_id = :catalog_id
          AND file_type NOT IN ('image', 'audio', 'video')
    )");

    query.prepare(querySQL);
    query.bindValue(":catalog_id", ID);

    if (!query.exec()) {
        qWarning() << "WARNING: Failed to clear metadata_extraction_date for non-media:" << query.lastError().text();
        return false;
    }

    return true;
}

void Catalog::handleMetadataTransition(const QString& previousIncludeMetadata,
                                       const QString& newIncludeMetadata)
{
    // No transition - nothing to do
    if (previousIncludeMetadata == newIncludeMetadata) {
        return;
    }


    // Scenario 2.1: None → any other value
    // Files already have NULL metadata_extraction_date
    if (previousIncludeMetadata == METADATA_NONE) {
        return;
    }

    // Scenario 2.2: Media_Basic → Media_Extended
    // Re-extract all media with extended metadata
    if (previousIncludeMetadata == METADATA_MEDIA_BASIC &&
        newIncludeMetadata == METADATA_MEDIA_EXTENDED) {
        clearMetadataExtractionDate();
        return;
    }

    // Scenario 2.3: Media_Extended → Media_Basic
    // Keep basic metadata, discard extended
    if (previousIncludeMetadata == METADATA_MEDIA_EXTENDED &&
        newIncludeMetadata == METADATA_MEDIA_BASIC) {
        clearMetadataExtendedField();
        return;
    }

    // Scenario 2.5: Media_Basic → Full_Extended
    // Re-extract media with extended + extract non-media
    if (previousIncludeMetadata == METADATA_MEDIA_BASIC &&
        newIncludeMetadata == METADATA_FULL) {
        clearMetadataExtractionDate();
        return;
    }

    // Scenario 2.4: Media_Extended → Full_Extended
    // Media already have extended, non-media will be found automatically
    if (previousIncludeMetadata == METADATA_MEDIA_EXTENDED &&
        newIncludeMetadata == METADATA_FULL) {
        return;
    }

    // Scenario 2.6: Full_Extended → Media_Basic
    // Clear extended field + clear extraction date for non-media only
    if (previousIncludeMetadata == METADATA_FULL &&
        newIncludeMetadata == METADATA_MEDIA_BASIC) {
        clearMetadataExtendedField();
        clearMetadataExtractionDateForNonMedia();
        return;
    }

    // Scenario 2.7: Full_Extended → Media_Extended
    // Clear extraction date for non-media only
    if (previousIncludeMetadata == METADATA_FULL &&
        newIncludeMetadata == METADATA_MEDIA_EXTENDED) {
        clearMetadataExtractionDateForNonMedia();
        return;
    }

    // Scenario 2.8: Any → None
    // Clear all metadata fields to reduce DB size
    if (newIncludeMetadata == METADATA_NONE) {
        clearMetadataBasicFields();
        clearMetadataExtendedField();
        clearMetadataExtractionDate();
        return;
    }

    // Log unhandled transition (for future-proofing)
    qWarning() << "WARNING: Unhandled metadata transition - no action taken";
}

bool Catalog::hasFilesNeedingMigration() const
{
    QSqlQuery checkQuery(QSqlDatabase::database(m_connectionName));
    checkQuery.prepare(R"(
        SELECT COUNT(*)
        FROM file
        WHERE file_catalog_id = :catalog_id
        AND file_type IS NULL OR (file_type IS NOT NULL AND mime_type IS NULL)
    )");
    checkQuery.bindValue(":catalog_id", ID);

    if (!checkQuery.exec() || !checkQuery.next()) {
        qWarning() << "WARNING: Failed to check migration status:" << checkQuery.lastError().text();
        return true;  // Assume needs migration if query fails (safe default)
    }

    int filesNeedingMigration = checkQuery.value(0).toInt();
    return (filesNeedingMigration > 0);
}
//----------------------------------------------------------------------
QString Catalog::getFileMetadataJson(int catalogId, const QString &fileName, const QString &folderPath, const QString &connectionName)
{
    QSqlQuery query(QSqlDatabase::database(connectionName));
    query.prepare(QLatin1String(R"(
        SELECT metadata_extended
        FROM file
        WHERE file_name = :file_name
          AND file_folder_path = :folder_path
          AND file_catalog_id = :catalog_id
          AND metadata_extended IS NOT NULL
          AND metadata_extended != ''
    )"));
    query.bindValue(":file_name",   fileName);
    query.bindValue(":folder_path", folderPath);
    query.bindValue(":catalog_id",  catalogId);

    if (query.exec() && query.next())
        return query.value(0).toString();
    return QString();
}
//----------------------------------------------------------------------
QList<Catalog::ExploreFileEntry> Catalog::getExploreEntries(
    const QString &connectionName,
    int catalogId,
    const QString &folderPath,
    bool showFolders,
    bool showSubFolders)
{
    QList<ExploreFileEntry> result;
    QSqlDatabase db = QSqlDatabase::database(connectionName);
    if (!db.isOpen()) return result;

    QString sql;

    if (showFolders) {
        sql = QLatin1String(R"(
            SELECT REPLACE(folder_path, :folderPath || '/', '') AS name,
                   NULL AS file_size,
                   '' AS file_date_updated,
                   folder_path AS file_folder_path,
                   'folder' AS entry_type,
                   '1' || folder_path AS order_val,
                   folder_path AS file_full_path,
                   NULL AS file_type, NULL AS mime_type,
                   0.0 AS video_duration_seconds,
                   NULL AS audio_artist, NULL AS audio_album, NULL AS audio_title,
                   NULL AS checksum_sha256
            FROM folder
            WHERE folder_catalog_id = :catalogId
            AND folder_path LIKE :folderPath || '/%'
        )");

        if (!showSubFolders) {
            sql += QLatin1String(R"(
            AND REPLACE(folder_path, :folderPath || '/', '') NOT LIKE '%/%'
            )");
        }

        sql += QLatin1String(R"(
            UNION
        )");
    }

    sql += QLatin1String(R"(
        SELECT file_name, file_size, file_date_updated, file_folder_path,
               'file' AS entry_type,
               '2' || file_name AS order_val,
               file_full_path,
               file_type, mime_type,
               video_duration_seconds,
               audio_artist, audio_album, audio_title,
               checksum_sha256
        FROM file
        WHERE file_catalog_id = :catalogId
        AND file_folder_path = :folderPath
        ORDER BY order_val ASC
    )");

    QSqlQuery q(db);
    q.prepare(sql);
    q.bindValue(":catalogId",  catalogId);
    q.bindValue(":folderPath", folderPath);

    if (!q.exec()) {
        qWarning() << "Catalog::getExploreEntries failed:" << q.lastError().text();
        return result;
    }

    while (q.next()) {
        ExploreFileEntry e;
        e.name                 = q.value(0).toString();
        e.size                 = q.value(1).toLongLong();
        e.dateUpdated          = q.value(2).toString();
        e.folderPath           = q.value(3).toString();
        e.entryType            = q.value(4).toString();
        // col 5 = order_val (unused)
        e.fullPath             = q.value(6).toString();
        e.fileType             = q.value(7).toString();
        e.mimeType             = q.value(8).toString();
        e.videoDurationSeconds = q.value(9).toDouble();
        e.audioArtist          = q.value(10).toString();
        e.audioAlbum           = q.value(11).toString();
        e.audioTitle           = q.value(12).toString();
        e.checksumSha256       = q.value(13).toString();
        result.append(e);
    }

    // Folder rows arrive with a NULL size. Fill each with the RECURSIVE total of
    // the files beneath it (SpecExplore.md EXP-F10).
    //
    // One grouped statement for the whole listing, then a single pass that adds
    // each group into every listed folder containing it. A correlated subquery
    // would read as "one statement" but would still execute once per row, which
    // is the shape EXP-C6 forbids.
    if (showFolders) {
        QHash<QString, int> folderRowByPath;   // folder full path -> index in result
        for (int i = 0; i < result.size(); ++i) {
            if (result[i].entryType == QLatin1String("folder")) {
                result[i].size = 0;            // a folder with nothing in it reads 0, never blank
                folderRowByPath.insert(result[i].folderPath, i);
            }
        }

        if (!folderRowByPath.isEmpty()) {
            // Range comparison rather than LIKE, so idx_file_catalog_folder is
            // used on every backend (EXP-C7): SQLite only turns a prefix LIKE
            // into a range scan under particular collation settings, and MariaDB
            // differs again. '0' is the next character after '/'.
            const QString lowerBound = folderPath + QLatin1Char('/');
            const QString upperBound = folderPath + QLatin1Char('0');

            QSqlQuery agg(db);
            agg.prepare(QLatin1String(
                "SELECT file_folder_path, SUM(file_size) FROM file "
                "WHERE file_catalog_id = :catalogId "
                "AND file_folder_path >= :lowerBound AND file_folder_path < :upperBound "
                "GROUP BY file_folder_path"));
            agg.bindValue(":catalogId",  catalogId);
            agg.bindValue(":lowerBound", lowerBound);
            agg.bindValue(":upperBound", upperBound);

            if (!agg.exec()) {
                qWarning() << "Catalog::getExploreEntries folder size aggregate failed:"
                           << agg.lastError().text();
            } else {
                while (agg.next()) {
                    const QString groupPath = agg.value(0).toString();
                    const qint64  groupSize = agg.value(1).toLongLong();

                    // Walk up from the group's folder to the open folder, adding
                    // into every listed ancestor. With "and all sub-folders" on
                    // several listed rows legitimately contain the same files, so
                    // the column overlaps by design and does not partition
                    // (EXP-F12).
                    QString walker = groupPath;
                    while (walker.size() > folderPath.size()) {
                        const auto it = folderRowByPath.constFind(walker);
                        if (it != folderRowByPath.constEnd())
                            result[it.value()].size += groupSize;
                        const int cut = walker.lastIndexOf(QLatin1Char('/'));
                        if (cut < 0)
                            break;
                        walker.truncate(cut);
                    }
                }
            }
        }
    }

    return result;
}
//----------------------------------------------------------------------
int Catalog::getExploreFolderCount(const QString &connectionName, int catalogId)
{
    QSqlDatabase db = QSqlDatabase::database(connectionName);
    if (!db.isOpen()) return 0;
    QSqlQuery q(db);
    q.prepare(QLatin1String("SELECT COUNT(DISTINCT folder_path) FROM folder WHERE folder_catalog_id = :catalogId"));
    q.bindValue(":catalogId", catalogId);
    if (!q.exec() || !q.next()) return 0;
    return q.value(0).toInt();
}

Catalog::ExploreFolderStats Catalog::getExploreFolderStats(
    const QString &connectionName, int catalogId, const QString &folderPath)
{
    ExploreFolderStats stats;
    QSqlDatabase db = QSqlDatabase::database(connectionName);
    if (!db.isOpen()) return stats;
    // An empty folder path means the whole catalog rather than one folder,
    // mirroring K2's conditional in mainwindow_tab_explore.cpp (SpecExplore.md).
    // Computed live rather than read from Device::totalFileSize, which is only
    // refreshed at catalog update and could disagree with the file records.
    QString sql = QLatin1String("SELECT COUNT(*), SUM(file_size) FROM file "
                                "WHERE file_catalog_id = :catalogId");
    if (!folderPath.isEmpty())
        sql += QLatin1String(" AND file_folder_path = :folderPath");

    QSqlQuery q(db);
    q.prepare(sql);
    q.bindValue(":catalogId",  catalogId);
    if (!folderPath.isEmpty())
        q.bindValue(":folderPath", folderPath);
    if (q.exec() && q.next()) {
        stats.fileCount  = q.value(0).toLongLong();
        stats.totalSize  = q.value(1).toLongLong();
    }
    return stats;
}
