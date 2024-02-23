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

#include "collection.h"
#include "device.h"

void Collection::generateCollectionFilesPaths()
{
    searchHistoryFilePath       = collectionFolder + "/" + "search_history.csv";
    storageFilePath             = collectionFolder + "/" + "storage.csv";
    deviceFilePath              = collectionFolder + "/" + "device.csv";
    deviceCatalogFilePath       = collectionFolder + "/" + "device_catalog.csv";
    statisticsCatalogFileName   = "statistics_catalog.csv";
    statisticsCatalogFilePath   = collectionFolder + "/" + statisticsCatalogFileName;
    statisticsStorageFileName   = "statistics_storage.csv";
    statisticsStorageFilePath   = collectionFolder + "/" + statisticsStorageFileName;
    statisticsDeviceFileName    = "statistics_device.csv";
    statisticsDeviceFilePath    = collectionFolder + "/" + statisticsDeviceFileName;
    excludeFilePath             = collectionFolder + "/" + "exclude.csv";
}
//----------------------------------------------------------------------
void Collection::saveDeviceTableToFile()
{
    if (databaseMode == "Memory"){
        QFile deviceFile(deviceFilePath);

        //Get data
        QSqlQuery query;
        QString querySQL = QLatin1String(R"(
                                    SELECT
                                            device_id                  ,
                                            device_parent_id           ,
                                            device_name                ,
                                            device_type                ,
                                            device_external_id         ,
                                            device_path                ,
                                            device_total_file_size     ,
                                            device_total_file_count    ,
                                            device_total_space         ,
                                            device_free_space          ,
                                            device_active              ,
                                            device_group_id            ,
                                            device_date_updated
                                    FROM device
                            )");
        query.prepare(querySQL);
        query.exec();

        //Write data
        if (deviceFile.open(QFile::WriteOnly | QFile::Text)) {

            QTextStream textStreamToFile(&deviceFile);

            //Prepare header line
            textStreamToFile << "ID"         << "\t"
                             << "Parent ID"  << "\t"
                             << "Name"       << "\t"
                             << "Type"       << "\t"
                             << "ExternalID" << "\t"
                             << "Path"       << "\t"
                             << "total_file_size" << "\t"
                             << "total_file_count"<< "\t"
                             << "total_space"<< "\t"
                             << "free_space" << "\t"
                             << "active"     << "\t"
                             << "groupID"    << "\t"
                             << "date updated"    << "\t"
                             << '\n';

            //Iterate the records and generate lines
            while (query.next()) {
                const QSqlRecord record = query.record();
                for (int i=0, recCount = record.count() ; i<recCount ; ++i){
                    if (i>0)
                        textStreamToFile << '\t';
                    textStreamToFile << record.value(i).toString();
                }
                //Write the result in the file
                textStreamToFile << '\n';
            }
            deviceFile.close();
        }
        else{
            QMessageBox msgBox;
            msgBox.setWindowTitle("Katalog");
            msgBox.setText(QCoreApplication::translate("MainWindow", "Error opening output file:<br/>%1").arg(deviceFilePath));
            msgBox.setIcon(QMessageBox::Warning);
            msgBox.exec();
        }
    }
}
//----------------------------------------------------------------------
void Collection::loadDeviceFileToTable()
{
    if(databaseMode=="Memory"){
        //Clear table
        QSqlQuery query;
        QString querySQL;
        querySQL = QLatin1String(R"(
                        DELETE FROM device
                    )");
        query.prepare(querySQL);
        query.exec();

        //Define storage file and prepare stream
        QFile deviceFile(deviceFilePath);
        QTextStream textStream(&deviceFile);

        //Open file or create it
        if(!deviceFile.open(QIODevice::ReadOnly)) {
            // Create it, if it does not exist
            QFile newDeviceFile(deviceFilePath);
            newDeviceFile.open(QFile::WriteOnly | QFile::Text);
            QTextStream stream(&newDeviceFile);
            stream << "ID"            << "\t"
                   << "Parent ID"     << "\t"
                   << "Name"          << "\t"
                   << '\n';
            newDeviceFile.close();
        }

        //Load Device device lines to table
        while (true)
        {
            QString line = textStream.readLine();
            if (line.isNull())
                break;
            else
                if (line.left(2)!="ID"){//skip the first line with headers

                    //Split the string with tabulation into a list
                    QStringList fieldList = line.split('\t');
                    QSqlQuery insertQuery;
                    querySQL = QLatin1String(R"(
                        INSERT INTO device (
                                        device_id,
                                        device_parent_id,
                                        device_name,
                                        device_type,
                                        device_external_id,
                                        device_path,
                                        device_total_file_size,
                                        device_total_file_count,
                                        device_total_space,
                                        device_free_space,
                                        device_group_id )
                        VALUES(
                                        :device_id,
                                        :device_parent_id,
                                        :device_name,
                                        :device_type,
                                        :device_external_id,
                                        :device_path,
                                        :device_total_file_size,
                                        :device_total_file_count,
                                        :device_total_space,
                                        :device_free_space,
                                        :device_group_id )
                    )");
                    insertQuery.prepare(querySQL);
                    insertQuery.bindValue(":device_id",fieldList[0].toInt());
                    insertQuery.bindValue(":device_parent_id",fieldList[1]);
                    insertQuery.bindValue(":device_name",fieldList[2]);
                    if(fieldList.size()>3){//prevent issues with files created in v1.22
                        insertQuery.bindValue(":device_type",fieldList[3]);
                        insertQuery.bindValue(":device_external_id",fieldList[4]);
                        insertQuery.bindValue(":device_path",fieldList[5]);
                        insertQuery.bindValue(":device_total_file_size",fieldList[6]);
                        insertQuery.bindValue(":device_total_file_count",fieldList[7]);
                        insertQuery.bindValue(":device_total_space",fieldList[8]);
                        insertQuery.bindValue(":device_free_space",fieldList[9]);
                        insertQuery.bindValue(":device_group_id",fieldList[11]);
                    }
                    insertQuery.exec();
                }
        }
        deviceFile.close();

        insertPhysicalStorageGroup();
    }
}
//----------------------------------------------------------------------
void Collection::loadCatalogFilesToTable()
{
    if(databaseMode=="Memory"){
        //Clear catalog table
        QSqlQuery queryDelete;
        queryDelete.prepare( "DELETE FROM catalog" );
        queryDelete.exec();

        //Iterate in the directory to create a list of files and sort it
        QStringList catalogFileExtensions;
        catalogFileExtensions << "*.idx";

        QDirIterator iterator(collectionFolder, catalogFileExtensions, QDir::Files, QDirIterator::Subdirectories);
        while (iterator.hasNext()){

            // Iterate to the next file
            QString path = iterator.next();
            QFile catalogFile(path);

            // Get file info
            QFileInfo catalogFileInfo(catalogFile);

            // Verify that the file can be opened
            if(!catalogFile.open(QIODevice::ReadOnly)) {
                QMessageBox msgBox;
                msgBox.setWindowTitle("Katalog");
                msgBox.setText(QCoreApplication::translate("MainWindow",
                                                           "No catalog found."
                                                           ));
                msgBox.setIcon(QMessageBox::Warning);
                msgBox.exec();
                return;
            }

            //Prepare a textsteam for the file
            QTextStream textStreamCatalogs(&catalogFile);

            //Read the first 10 lines and put values in a stringlist
            QStringList catalogValues;
            QString line;
            QString value;
            for (int i=0; i<11; i++) {
                line = textStreamCatalogs.readLine();
                if (line !="" and QVariant(line.at(0)).toString()=="<"){
                    value = line.right(line.size() - line.indexOf(">") - 1);
                    if (value =="") catalogValues << "";
                    else catalogValues << value;
                }
            }
            if (catalogValues.count()== 7) catalogValues << "false"; //for older catalog without isFullDevice
            if (catalogValues.count()== 8) catalogValues << "false"; //for older catalog without includeMetadata
            if (catalogValues.count()== 9) catalogValues << "";      //for older catalog without appVersion
            if (catalogValues.count()==10) catalogValues << 0;       //for older catalog without ID

            if(catalogValues.length()>0){
                //Insert a line in the table with available data

                //Prepare insert query for filesall
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
                                VALUES(
                                                :catalog_id,
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
                insertCatalogQuery.bindValue(":catalog_id",                 catalogValues[10]);
                insertCatalogQuery.bindValue(":catalog_file_path",          catalogFileInfo.filePath());
                insertCatalogQuery.bindValue(":catalog_name",               catalogFileInfo.completeBaseName());
                insertCatalogQuery.bindValue(":catalog_date_updated",       catalogFileInfo.lastModified().toString("yyyy-MM-dd hh:mm:ss"));
                insertCatalogQuery.bindValue(":catalog_source_path",        catalogValues[0]);
                insertCatalogQuery.bindValue(":catalog_file_count",         catalogValues[1].toInt());
                insertCatalogQuery.bindValue(":catalog_total_file_size",    catalogValues[2].toLongLong());
                insertCatalogQuery.bindValue(":catalog_include_hidden",     catalogValues[3]);
                insertCatalogQuery.bindValue(":catalog_file_type",          catalogValues[4]);
                insertCatalogQuery.bindValue(":catalog_storage",            catalogValues[5]);
                insertCatalogQuery.bindValue(":catalog_include_symblinks",  catalogValues[6]);
                insertCatalogQuery.bindValue(":catalog_is_full_device",     catalogValues[7]);
                insertCatalogQuery.bindValue(":catalog_date_loaded","");
                insertCatalogQuery.bindValue(":catalog_include_metadata",   catalogValues[8]);
                insertCatalogQuery.bindValue(":catalog_app_version",        catalogValues[9]);
                insertCatalogQuery.exec();
            }
            catalogFile.close();
        }
    }
}
//--------------------------------------------------------------------------
void Collection::loadStorageFileToTable()
{//load Storage file data to its table
    if (databaseMode=="Memory"){

        //Define storage file and prepare stream
        QFile storageFile(storageFilePath);
        QTextStream textStream(&storageFile);

        QSqlQuery queryDelete;
        queryDelete.prepare( "DELETE FROM storage" );

        //Open file or return information
        if(!storageFile.open(QIODevice::ReadOnly)) {

            queryDelete.exec();

            //Disable all buttons, enable create list
            // ui->Storage_pushButton_Reload->setEnabled(false);
            // ui->Storage_pushButton_EditAll->setEnabled(false);
            // ui->Storage_pushButton_SaveAll->setEnabled(false);
            // ui->Storage_pushButton_New->setEnabled(false);
            // ui->Storage_pushButton_CreateList->setEnabled(true);

            return;
        }

        //Test file validity (application breaks between v0.13 and v0.14)
        QString line = textStream.readLine();
        if (line.left(2)!="ID"){

            QMessageBox msgBox;
            msgBox.setWindowTitle("Katalog");
            msgBox.setText(QCoreApplication::translate("MainWindow",
                                                       "A storage.csv file was found, but could not be loaded.\n"
                                                       "Likely, it was made with an older version of Katalog.\n"
                                                       "The file can be fixed manually, please visit the wiki page:\n"
                                                       "<a href='https://github.com/StephaneCouturier/Katalog/wiki/Storage#fixing-for-new-versions'>"
                                                       "Storage/fixing-for-new-versions</a>"
                                                       ));
            msgBox.setIcon(QMessageBox::Information);
            msgBox.exec();

            return;
        }

        //Clear all entries of the current table
        queryDelete.exec();

        //Load storage device lines to table
        while (true)
        {

            QString line = textStream.readLine();
            if (line.isNull())
                break;
            else
                if (line.left(2)!="ID"){//skip the first line with headers

                    //Split the string with tabulation into a list
                    QStringList fieldList = line.split('\t');

                    QString querySQL = QLatin1String(R"(
                        INSERT INTO storage(
                                        storage_id,
                                        storage_name,
                                        storage_type,
                                        storage_location,
                                        storage_path,
                                        storage_label,
                                        storage_file_system,
                                        storage_total_space,
                                        storage_free_space,
                                        storage_brand_model,
                                        storage_serial_number,
                                        storage_build_date,
                                        storage_content_type,
                                        storage_container,
                                        storage_comment)
                                  values(
                                        :storage_id,
                                        :storage_name,
                                        :storage_type,
                                        :storage_location,
                                        :storage_path,
                                        :storage_label,
                                        :storage_file_system,
                                        :storage_total_space,
                                        :storage_free_space,
                                        :storage_brand_model,
                                        :storage_serial_number,
                                        :storage_build_date,
                                        :storage_content_type,
                                        :storage_container,
                                        :storage_comment)
                                )");

                    QSqlQuery insertQuery;
                    insertQuery.prepare(querySQL);
                    insertQuery.bindValue(":storage_id",fieldList[0].toInt());
                    insertQuery.bindValue(":storage_name",fieldList[1]);
                    insertQuery.bindValue(":storage_type",fieldList[2]);
                    insertQuery.bindValue(":storage_location",fieldList[3]);
                    insertQuery.bindValue(":storage_path",fieldList[4]);
                    insertQuery.bindValue(":storage_label",fieldList[5]);
                    insertQuery.bindValue(":storage_file_system",fieldList[6]);
                    insertQuery.bindValue(":storage_total_space",fieldList[7].toLongLong());
                    insertQuery.bindValue(":storage_free_space",fieldList[8].toLongLong());
                    insertQuery.bindValue(":storage_brand_model",fieldList[9]);
                    insertQuery.bindValue(":storage_serial_number",fieldList[10]);
                    insertQuery.bindValue(":storage_build_date",fieldList[11]);
                    insertQuery.bindValue(":storage_content_type",fieldList[12]);
                    insertQuery.bindValue(":storage_container",fieldList[13]);
                    insertQuery.bindValue(":storage_comment", fieldList[14]);

                    insertQuery.exec();

                }
        }
        storageFile.close();
    }
}
//----------------------------------------------------------------------
void Collection::loadStatisticsDeviceFileToTable()
{// Load the contents of the storage statistics file into the database
    if(databaseMode=="Memory"){
        //Clear database table
        QSqlQuery deleteQuery;
        deleteQuery.exec("DELETE FROM statistics_device");

        //Get infos stored in the file
        QFile statisticsDeviceFile(statisticsDeviceFilePath);
        if(!statisticsDeviceFile.open(QIODevice::ReadOnly)) {
            return;
        }

        QTextStream textStream(&statisticsDeviceFile);

        //Prepare query to load file info
        QSqlQuery insertQuery;
        QString insertSQL = QLatin1String(R"(
                                    INSERT INTO statistics_device (
                                                date_time               ,
                                                device_id               ,
                                                device_name             ,
                                                device_type             ,
                                                device_file_count       ,
                                                device_total_file_size  ,
                                                device_free_space       ,
                                                device_total_space      ,
                                                record_type             )
                                    VALUES(
                                                :date_time              ,
                                                :device_id              ,
                                                :device_name            ,
                                                :device_type            ,
                                                :device_file_count      ,
                                                :device_total_file_size ,
                                                :device_free_space      ,
                                                :device_total_space     ,
                                                :record_type            )
                                                )");
        insertQuery.prepare(insertSQL);

        //Set temporary values
        QString     line;
        QStringList fieldList;
        QString     dateTime;
        int         deviceID = 0;
        QString     deviceName;
        QString     deviceType;
        qint64      deviceFileCount = 0;
        qint64      deviceTotalFileSize = 0;
        qint64      deviceFreeSpace = 0;
        qint64      deviceTotalSpace = 0;
        QString     recordType;
        QRegularExpression tagExp("\t");

        //Skip first header line
        line = textStream.readLine();

        //Load file to database
        while (!textStream.atEnd())
        {
            line = textStream.readLine();
            if (line.isNull()) //stop when line is null
                break;
            else //parse the line and load to db
            {
                //Split the string with \t (tabulation) into a list
                fieldList.clear();
                fieldList = line.split(tagExp);

                if(fieldList.count()==9){
                    dateTime            = fieldList[0];
                    deviceID            = fieldList[1].toInt();
                    deviceName          = fieldList[2];
                    deviceType          = fieldList[3];
                    deviceFileCount     = fieldList[4].toLongLong();
                    deviceTotalFileSize = fieldList[5].toLongLong();
                    deviceFreeSpace     = fieldList[6].toLongLong();
                    deviceTotalSpace    = fieldList[7].toLongLong();
                    recordType          = fieldList[8];
                }

                //Append data to the database
                insertQuery.bindValue(":date_time", dateTime);
                insertQuery.bindValue(":device_id", deviceID);
                insertQuery.bindValue(":device_name", deviceName);
                insertQuery.bindValue(":device_type", deviceType);
                insertQuery.bindValue(":device_file_count", QString::number(deviceFileCount));
                insertQuery.bindValue(":device_total_file_size", QString::number(deviceTotalFileSize));
                insertQuery.bindValue(":device_free_space", QString::number(deviceFreeSpace));
                insertQuery.bindValue(":device_total_space", QString::number(deviceTotalSpace));
                insertQuery.bindValue(":record_type", recordType);
                insertQuery.exec();
            }
        }
    }
}
//----------------------------------------------------------------------
void Collection::loadExclude()
{// Load the contents of the storage statistics file into the database
    if(databaseMode=="Memory"){
        //Clear database table
        QSqlQuery deleteQuery;
        deleteQuery.exec("DELETE FROM exclude");

        //Get data stored in the file
        QFile excludeFile(excludeFilePath);
        if(excludeFile.open(QIODevice::ReadOnly)) {
            QTextStream textStream(&excludeFile);

            //Prepare query to load file info
            QSqlQuery insertQuery;
            QString insertSQL = QLatin1String(R"(
                                        INSERT INTO exclude (
                                                    exclude_path  )
                                        VALUES(
                                                    :exclude_path )
                                )");
            insertQuery.prepare(insertSQL);

            //Set temporary values
            QString     line;

            //Load file to database
            while (!textStream.atEnd())
            {
                line = textStream.readLine();
                if (line.isNull())
                    break;
                else
                {
                    //Append data to the database
                    insertQuery.bindValue(":exclude_path", line);
                    insertQuery.exec();
                }
            }
        }
    }
}
//----------------------------------------------------------------------
void Collection::saveStorageTableToFile()
{
    if (databaseMode=="Memory"){
        //Prepare export file
        QFile storageFile(storageFilePath);
        QTextStream out(&storageFile);

        //Prepare header line
        out  << "ID"            << "\t"
            << "Name"          << "\t"
            << "Type"          << "\t"
            << "Location"      << "\t"
            << "Path"          << "\t"
            << "Label"         << "\t"
            << "FileSystem"    << "\t"
            << "Total"         << "\t"
            << "Free"          << "\t"
            << "BrandModel"    << "\t"
            << "SerialNumber"  << "\t"
            << "BuildDate"     << "\t"
            << "ContentType"   << "\t"
            << "Container"     << "\t"
            << "Comment"       << "\t"
            << '\n';

        //Get data
        QSqlQuery query;
        QString querySQL = QLatin1String(R"(
                         SELECT * FROM storage
                                    )");
        query.prepare(querySQL);
        query.exec();

        //Iterate the records and generate lines
        while (query.next()) {
            const QSqlRecord record = query.record();
            for (int i=0, recCount = record.count() ; i<recCount ; ++i){
                if (i>0)
                    out << '\t';
                out << record.value(i).toString();
            }
            //-- Write the result in the file
            out << '\n';

        }

        if(storageFile.open(QIODevice::WriteOnly | QIODevice::Truncate)) {

            //out << textData;
            //Close the file
            //storageFile.close();
        }
        storageFile.close();
    }
}
//----------------------------------------------------------------------
void Collection::saveStatiticsToFile()
{
    if(databaseMode=="Memory"){
        //Prepare export file
        QFile statisticsFile(statisticsDeviceFilePath);
        QTextStream out(&statisticsFile);

        //Prepare header line
        out << "date_time"              << "\t"
            << "device_id"              << "\t"
            << "device_name"            << "\t"
            << "device_type"            << "\t"
            << "device_file_count"      << "\t"
            << "device_total_file_size" << "\t"
            << "device_free_space"      << "\t"
            << "device_total_space"     << "\t"
            << "record_type"            << "\t"
            << '\n';

        //Get data
        QSqlQuery query;
        QString querySQL = QLatin1String(R"(
                                    SELECT  date_time,
                                            device_id,
                                            device_name,
                                            device_type,
                                            device_file_count,
                                            device_total_file_size,
                                            device_free_space,
                                            device_total_space,
                                            record_type
                                    FROM statistics_device
                                )");
        query.prepare(querySQL);
        query.exec();

        if(statisticsFile.open(QIODevice::WriteOnly | QIODevice::Truncate)) {

            //Iterate the records and generate lines
            while (query.next()) {
                const QSqlRecord record = query.record();
                for (int i=0, recCount = record.count() ; i<recCount ; ++i){
                    if (i>0)
                        out << '\t';
                    out << record.value(i).toString();
                }
                //Write the result to the file
                out << '\n';
            }
        }

        statisticsFile.close();
    }
}
//----------------------------------------------------------------------
void Collection::insertPhysicalStorageGroup() {
    QSqlQuery query;
    QString querySQL;

    querySQL = QLatin1String(R"(
                            SELECT COUNT(*)
                            FROM device
                            WHERE device_id = 1
                        )");
    query.prepare(querySQL);
    query.exec();
    query.next();
    int result = query.value(0).toInt();

    if(result == 0){
        Device *newDeviceItem1 = new Device();
        newDeviceItem1->ID = 1;
        newDeviceItem1->parentID = 0;
        newDeviceItem1->name = QCoreApplication::translate("MainWindow", " Physical Group");
        newDeviceItem1->type = "Virtual";
        newDeviceItem1->externalID = 0;
        newDeviceItem1->groupID = 0;
        newDeviceItem1->insertDevice();

        Device *newDeviceItem2 = new Device();
        newDeviceItem2->ID = 2;
        newDeviceItem2->parentID = 1;
        newDeviceItem2->name = QCoreApplication::translate("MainWindow", "Default Virtual group");
        newDeviceItem2->type = "Virtual";
        newDeviceItem2->externalID = 0;
        newDeviceItem2->groupID = 0;
        newDeviceItem2->insertDevice();
    }

    saveDeviceTableToFile();

}
//--------------------------------------------------------------------------

void Collection::deleteCatalogFile(Device *device) {
    if(databaseMode=="Memory"){
        //Move file to trash
        if ( device->catalog->filePath != ""){

            QFile file (device->catalog->filePath);
            file.moveToTrash();

            QString foldersFilePath = device->catalog->filePath;
            foldersFilePath.chop(4);
            foldersFilePath +=".folders.idx";
            QFile foldersFile (foldersFilePath);
            foldersFile.moveToTrash();
        }
        else{
            QMessageBox msgBox;
            msgBox.setWindowTitle("Katalog");
            msgBox.setText(QCoreApplication::translate("MainWindow",
                                                       "Select a catalog with a valid path."
                                                       ));
            msgBox.setIcon(QMessageBox::Information);
            msgBox.exec();
        }
        //Clear current entires from the table
        QSqlQuery queryDelete;
        queryDelete.exec("DELETE FROM catalog");

        //refresh catalog lists
        loadCatalogFilesToTable();
    }

}
//--------------------------------------------------------------------------

