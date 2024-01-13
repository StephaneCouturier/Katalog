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
#include "qapplication.h"
#include "qdir.h"
#include "qsqlerror.h"

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
        catalogQuery.bindValue(":catalog_date_loaded", dateLoaded.toString("yyyy-MM-dd hh:mm:ss"));
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
    QSqlQuery queryCatalogID;
    QString queryCatalogIDSQL = QLatin1String(R"(
                                    SELECT MAX (catalog_id)
                                    FROM catalog
                                )");
    queryCatalogID.prepare(queryCatalogIDSQL);
    queryCatalogID.exec();
    queryCatalogID.next();
    int maxID = queryCatalogID.value(0).toInt();
    ID = maxID + 1;
}

void Catalog::insertCatalog()
{//Insert new catalog entry
    QSqlQuery insertCatalogQuery;
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
    insertCatalogQuery.bindValue(":catalog_name",name);
    insertCatalogQuery.bindValue(":catalog_date_updated",dateUpdated);
    insertCatalogQuery.bindValue(":catalog_source_path",sourcePath);
    insertCatalogQuery.bindValue(":catalog_file_count",fileCount);
    insertCatalogQuery.bindValue(":catalog_total_file_size",totalFileSize);
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

void Catalog::saveCatalog()
{//Update database with catalog values
    QSqlQuery query;
    QString querySQL = QLatin1String(R"(
                            UPDATE catalog
                            SET     catalog_id               =:catalog_id,
                                    catalog_source_path      =:catalog_source_path,
                                    catalog_storage          =:catalog_storage,
                                    catalog_file_type        =:catalog_file_type,
                                    catalog_include_hidden   =:catalog_include_hidden,
                                    catalog_include_metadata =:catalog_include_metadata
                            WHERE  catalog_name=:catalog_name
                                )");
    query.prepare(querySQL);
    query.bindValue(":catalog_id", ID);
    query.bindValue(":catalog_name", name);
    query.exec();
}

void Catalog::updateCatalogFileHeaders(QString databaseMode)
{//Write changes to catalog file (update headers only)
    if(databaseMode=="Memory"){
        QFile catalogFile(filePath);
        if(catalogFile.open(QIODevice::ReadWrite | QIODevice::Text))
        {
            QString fullFileText;
            QTextStream textStream(&catalogFile);

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

            while(!textStream.atEnd())
            {
                QString line = textStream.readLine();

                //add file data line
                if( !line.startsWith("<catalog") )
                {//just pass the line if it is not a header
                    fullFileText.append(line + "\n");
                }
            }

            catalogFile.resize(0);//delete file content
            textStream << fullFileText;//populate the file with the textStream
            catalogFile.close();
        }
        else {

            QMessageBox msgBox;
            msgBox.setWindowTitle("Katalog");
            msgBox.setText(QCoreApplication::translate("MainWindow",
                                                       "Could not open file."
                                                       ) );
            msgBox.setIcon(QMessageBox::Information);
            msgBox.exec();
        }
    }
}

QList<qint64> Catalog::updateCatalogFiles(QString databaseMode, QString collectionFolder)
{//Update the files of the catalog and return a list with update information
    QList<qint64> list;

    if(databaseMode=="Memory"){
        //Check if the update can be done, inform the user otherwise.
        //Deal with old versions, where necessary info may have not have been available
        if(filePath == "not recorded" or name == "not recorded" or sourcePath == "not recorded"){

            QMessageBox msgBox;
            msgBox.setWindowTitle("Katalog");
            msgBox.setText(QCoreApplication::translate("MainWindow",
                                "It seems this catalog was not correctly imported or has an old format.<br/>"
                                "Edit it and make sure it has the following first 2 lines:<br/><br/>"
                                "<catalogSourcePath>/folderpath<br/>"
                                "<catalogFileCount>10000<br/><br/>"
                                "Copy/paste these lines at the begining of the file and modify the values after the >:<br/>"
                                "- the catalogSourcePath is the folder to catalog the files from.<br/>"
                                "- the catalogFileCount number does not matter as much, it can be updated.<br/>"));
            msgBox.setIcon(QMessageBox::Information);
            msgBox.exec();

            return list;
        }

        //Deal with other cases where some input information is missing
        if(filePath == "" or name == "" or sourcePath == ""){

            QMessageBox msgBox;
            msgBox.setWindowTitle("Katalog");

            msgBox.setText(QCoreApplication::translate("MainWindow", "Select a catalog first (some info is missing).<br/> "
                                                                     "currentCatalogFilePath: %1 <br/> currentCatalogName: %2 <br/> "
                                                                     "currentCatalogSourcePath: %3").arg(
                                   filePath, name, sourcePath));
            msgBox.setIcon(QMessageBox::Information);
            msgBox.exec();

            return list;
        }
    }

    //Capture previous FileCount and TotalFileSize to report the changes after the update
    qint64 previousFileCount     = fileCount;
    qint64 previousTotalFileSize = totalFileSize;

    //If dir exists, catalog the directory (iterator)
    QDir dir (sourcePath);
    if (dir.exists() == true){
        ///Warning and choice if the result is 0 files
        if(dir.entryInfoList(QDir::NoDotAndDotDot|QDir::AllEntries).count() == 0)
        {
            QApplication::restoreOverrideCursor();
            QMessageBox msgBox;
            msgBox.setWindowTitle("Katalog");
            msgBox.setText(QCoreApplication::translate("MainWindow",
                                            "The source folder does not contain any file.<br/>"
                                            "This could mean that the source is empty or the device is not mounted to this folder.<br/>"
                                            "Do you want to save it anyway (the catalog would be empty)?."));
            msgBox.setIcon(QMessageBox::Warning);
            msgBox.setStandardButtons(QMessageBox::Yes | QMessageBox::Cancel);
            int result = msgBox.exec();

            if ( result == QMessageBox::Cancel){
                return list;
            }
            else
                QApplication::setOverrideCursor(Qt::WaitCursor);
        }

        //catalog the directory (iterator)
        catalogDirectory(databaseMode, collectionFolder);

        //Populate list to report changes
        qint64 newFileCount       = fileCount;
        qint64 deltaFileCount     = newFileCount - previousFileCount;
        qint64 newTotalFileSize   = totalFileSize;
        qint64 deltaTotalFileSize = newTotalFileSize - previousTotalFileSize;

        list.append(1);//catalog updated
        list.append(newFileCount);
        list.append(deltaFileCount);
        list.append(newTotalFileSize);
        list.append(deltaTotalFileSize);

        return list;
    }
    else {
        QApplication::restoreOverrideCursor();
        QMessageBox msgBox;
        msgBox.setWindowTitle("Katalog");
        msgBox.setText(QCoreApplication::translate("MainWindow",
                                            "The catalog <b>%1</b> cannot be updated.<br/>"
                                            "<br/> The source folder was not found.<br/><b>%2</b><br/>"
                                            "<br/> Possible reasons:<br/>"
                                            "    - the device is not connected and mounted,<br/>"
                                            "    - the source folder was moved or renamed.").arg(name,sourcePath));
        msgBox.setIcon(QMessageBox::Warning);
        msgBox.exec();

        list.append(0);//catalog not updated
        list.append(0);
        list.append(0);
        list.append(0);
        list.append(0);
        return list;
    }
}

void Catalog::loadCatalog()
{
    QSqlQuery query;
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
                            LEFT JOIN storage ON catalog_storage = storage_name
                            WHERE catalog_name=:catalog_name
                        )");
    query.prepare(querySQL);
    query.bindValue(":catalog_name",name);
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

void Catalog::loadCatalogFileListToTable()
{//Load catalog files from file, if latest version is not already in memory

    if ( dateLoaded < dateUpdated ){
        //Inputs
        QFile catalogFile(filePath);

        if (catalogFile.open(QIODevice::ReadOnly|QIODevice::Text)) {

            //Set up a text stream from the file's data
            QTextStream streamCatalogFile(&catalogFile);
            QString lineCatalogFile;

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

bool Catalog::catalogNameExists()
{
    QSqlQuery query;
    QString querySQL = QLatin1String(R"(
                                    SELECT COUNT(*)
                                    FROM   catalog
                                    WHERE  catalog_name = :catalog_name
                                )");

    query.prepare(querySQL);
    query.bindValue(":catalog_name", name);

    if (!query.exec()) {
        // Handle SQL error
        qDebug() << "Error executing catalogFileExists:" << query.lastError().text();
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

void Catalog::getFileTypes()
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
    //REPLACE BY DATABASE QUERY
    QSqlQuery query;
    QString querySQL = QLatin1String(R"(
                                    SELECT *
                                    FROM exclude
                                )");
    query.prepare(querySQL);
    query.exec();
    qDebug()<<query.lastError();
    while(query.next()){
        qDebug()<<query.value(0).toString()<<query.value(1).toString();
        excludedFolders<<query.value(0).toString();
    }


    // QString excludeFilePath = collectionFolder + "/" + "exclude.csv";
    // //QStringList excludedFolders;
    // QFile excludeFile(excludeFilePath);
    // if(excludeFile.open(QIODevice::ReadOnly)) {
    //     QTextStream textStream(&excludeFile);
    //     QString line;
    //     while (true)
    //     {
    //         line = textStream.readLine();
    //         if (line.isNull())
    //             break;
    //         else
    //             excludedFolders << line;
    //     }
    //     excludeFile.close();
    // }
}


void Catalog::catalogDirectory(QString databaseMode, QString collectionFolder)
{//Catalog the files of a directory and update catalog attributes

    //DEV: 2 Media File Metadata

    //Prepare inputs
    // QString excludeFilePath = collectionFolder + "/" + "exclude.csv";
    // Get directories to exclude
    // QStringList excludedFolders;
    // QFile excludeFile(excludeFilePath);
    // if(excludeFile.open(QIODevice::ReadOnly)) {
    //     QTextStream textStream(&excludeFile);
    //     QString line;
    //     while (true)
    //     {
    //         line = textStream.readLine();
    //         if (line.isNull())
    //             break;
    //         else
    //             excludedFolders << line;
    //     }
    //     excludeFile.close();
    // }

    //Prepare database and queries

    //Remove any former files from db for older catalog with same name
    //DEV: replace by ID
    QSqlQuery deleteFileQuery;
    QString deleteFileQuerySQL = QLatin1String(R"(
                                            DELETE FROM file
                                            WHERE file_catalog=:file_catalog
                                        )");
    deleteFileQuery.prepare(deleteFileQuerySQL);
    deleteFileQuery.bindValue(":file_catalog", name);
    deleteFileQuery.exec();

    QSqlQuery deleteFolderQuery;
    QString deleteFolderQuerySQL = QLatin1String(R"(
                                            DELETE FROM folder
                                            WHERE folder_catalog_name=:folder_catalog_name
                                        )");
    deleteFolderQuery.prepare(deleteFolderQuerySQL);
    deleteFolderQuery.bindValue(":folder_catalog_name",name);
    deleteFolderQuery.exec();

    //Prepare insert query for file
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
    insertFileQuery.prepare(insertFileSQL);

    //Prepare insert query for folder
    QSqlQuery insertFolderQuery;
    QString insertFolderSQL = QLatin1String(R"(
                                        INSERT OR IGNORE INTO folder(
                                            folder_catalog_name,
                                            folder_path
                                         )
                                        VALUES(
                                            :folder_catalog_name,
                                            :folder_path)
                                        )");
    insertFolderQuery.prepare(insertFolderSQL);

    //insert root folder (so that it is displayed even when there are no sub-folders)
    insertFolderQuery.prepare(insertFolderSQL);
    insertFolderQuery.bindValue(":folder_catalog_name", name);
    insertFolderQuery.bindValue(":folder_path",         sourcePath);
    insertFolderQuery.exec();

    //Scan entries with iterator
    QString entryPath;
    //Start a transaction to save all inserts at once in the db
    QSqlQuery beginQuery;
    QString beginQuerySQL = QLatin1String(R"(
                                        BEGIN
                                        )");
    beginQuery.prepare(beginQuerySQL);
    beginQuery.exec();

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
                    insertFolderQuery.bindValue(":folder_catalog_name", name);
                    insertFolderQuery.bindValue(":folder_path",         entryPath);
                    insertFolderQuery.exec();
                }

                //Insert files
                else if (entry.isFile()) {

                    QFile file(entryPath);
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
                    insertFolderQuery.bindValue(":folder_catalog_name", name);
                    insertFolderQuery.bindValue(":folder_path",         entryPath);
                    insertFolderQuery.exec();
                }

                //Insert files
                else if (entry.isFile()) {

                    QFile file(entryPath);
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
    QSqlQuery commitQuery;
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

        QSqlQuery queryFileList;
        QString queryFileListSQL = QLatin1String(R"(
                        SELECT file_full_path, file_size, file_date_updated
                        FROM file
                        WHERE file_catalog=:file_catalog
                    )");
        queryFileList.prepare(queryFileListSQL);
        queryFileList.bindValue(":file_catalog",name);
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

        //Check if no files where found, and let the user decide what to do
        // Get the catalog file list
        if (fileList.count() == 11){ //the CatalogDirectory method always adds lines for the catalog metadata, they should be ignored
            QMessageBox msgBox;
            msgBox.setWindowTitle("Katalog");
            msgBox.setText(QCoreApplication::translate("MainWindow",
                                                       "The source folder does not contain any file.<br/>"
                                                       "This could mean that the source is empty or the device is not mounted to this folder.<br/>"
                                                       "Do you want to save it anyway (the catalog would be empty)?<br/>"
                            ));
            msgBox.setIcon(QMessageBox::Warning);
            msgBox.setStandardButtons(QMessageBox::Yes|QMessageBox::Cancel);
            int result = msgBox.exec();

            if ( result != QMessageBox::Cancel){
                return;
            }
        }
    }

    //Update catalog in db
    //DEV: duplcate of saveCatalog?
    QSqlQuery query;
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
    setDateLoaded(emptyDateTime);
}

//--------------------------------------------------------------------------
void Catalog::saveCatalogToFile(QString databaseMode, QString collectionFolder)
{//Save a catalog's file list to a file
    if(databaseMode=="Memory"){
        //Get the file list from this model
        QStringList filelist = fileListModel->stringList();
        filePath = collectionFolder +"/"+ name + ".idx";

        //Stream the list to the file
        QFile fileOut(filePath);

        //Write data
        if (fileOut.open(QFile::WriteOnly | QFile::Text)) {
            QTextStream stream(&fileOut);
            for (int i = 0; i < filelist.size(); ++i)
                stream << filelist.at(i) << '\n';
        }
        else {
            QMessageBox msgBox;
            msgBox.setWindowTitle("Katalog");
            msgBox.setText(QCoreApplication::translate("MainWindow","Error opening output file."));
            msgBox.setIcon(QMessageBox::Warning);
            msgBox.exec();
        }
        fileOut.close();
    }
}
//--------------------------------------------------------------------------
void Catalog::saveFoldersToFile(QString databaseMode, QString collectionFolder)
{//Save a catalog's folders to a new file
    if(databaseMode=="Memory"){
        //Get the folder list from database
        QSqlQuery query;
        QString querySQL = QLatin1String(R"(
                                    SELECT
                                        folder_catalog_name,
                                        folder_path
                                    FROM folder
                                    WHERE folder_catalog_name=:folder_catalog_name
                                            )");
        query.prepare(querySQL);
        query.bindValue(":folder_catalog_name", name);
        query.exec();

        //Stream the list to the file
        QFile fileOut( collectionFolder +"/"+ name + ".folders.idx" );

        //Write data
        if (fileOut.open(QFile::WriteOnly | QFile::Text)) {
            QTextStream stream(&fileOut);
            while(query.next()){
                stream << query.value(0).toString() << '\t';
                stream << query.value(1).toString() << '\n';
            }

        } else {
            QMessageBox msgBox;
            msgBox.setWindowTitle("Katalog");
            msgBox.setText(QCoreApplication::translate("MainWindow","Error opening output file."));
            msgBox.setIcon(QMessageBox::Warning);
            msgBox.exec();
            //return EXIT_FAILURE;
        }
        fileOut.close();
    }
}
