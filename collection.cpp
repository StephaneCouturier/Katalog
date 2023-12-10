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
                                    SELECT *
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
void Collection::loadStatisticsCatalogFileToTable()
{// Load the contents of the statistics file into the database
    if(databaseMode=="Memory"){

        //clear database table
        QSqlQuery deleteQuery;
        deleteQuery.exec("DELETE FROM statistics_catalog");

        // Get infos stored in the file
        QFile statisticsCatalogFile(statisticsCatalogFilePath);
        if(!statisticsCatalogFile.open(QIODevice::ReadOnly)) {
            return;
        }

        QTextStream textStream(&statisticsCatalogFile);

        //prepare query to load file info
        QSqlQuery insertQuery;
        QString insertSQL = QLatin1String(R"(
                                    INSERT INTO statistics_catalog (
                                                    date_time,
                                                    catalog_name,
                                                    catalog_file_count,
                                                    catalog_total_file_size,
                                                    record_type )
                                    VALUES(
                                                    :date_time,
                                                    :catalog_name,
                                                    :catalog_file_count,
                                                    :catalog_total_file_size,
                                                    :record_type )
                                                )");
        insertQuery.prepare(insertSQL);

        //set temporary values
        QString     line;
        QStringList fieldList;

        QString     dateTime;
        QString     catalogName;
        qint64      catalogFileCount;
        qint64      catalogTotalFileSize;
        QString     recordType;
        QRegularExpression tagExp("\t");

        //Skip titles line
        line = textStream.readLine();

        //load file to database
        while (!textStream.atEnd())
        {
            line = textStream.readLine();
            if (line.isNull())
                break;
            else
            {
                //Split the string with \t (tabulation) into a list
                fieldList.clear();
                fieldList = line.split(tagExp);
                dateTime                = fieldList[0];
                catalogName             = fieldList[1];
                catalogFileCount        = fieldList[2].toLongLong();
                catalogTotalFileSize    = fieldList[3].toLongLong();
                recordType              = fieldList[4];

                //Append data to the database
                insertQuery.bindValue(":date_time", dateTime);
                insertQuery.bindValue(":catalog_name", catalogName);
                insertQuery.bindValue(":catalog_file_count", QString::number(catalogFileCount));
                insertQuery.bindValue(":catalog_total_file_size", QString::number(catalogTotalFileSize));
                insertQuery.bindValue(":record_type", recordType);
                insertQuery.exec();
            }
        }
    }
}
//----------------------------------------------------------------------
void Collection::loadStatisticsStorageFileToTable()
{// Load the contents of the storage statistics file into the database
    if(databaseMode=="Memory"){
        //clear database table
        QSqlQuery deleteQuery;
        deleteQuery.exec("DELETE FROM statistics_storage");

        // Get infos stored in the file
        QFile statisticsStorageFile(statisticsStorageFilePath);
        if(!statisticsStorageFile.open(QIODevice::ReadOnly)) {
            return;
        }

        QTextStream textStream(&statisticsStorageFile);

        //prepare query to load file info
        QSqlQuery insertQuery;
        QString insertSQL = QLatin1String(R"(
                                    INSERT INTO statistics_storage (
                                                    date_time,
                                                    storage_id,
                                                    storage_name,
                                                    storage_free_space,
                                                    storage_total_space,
                                                    record_type )
                                    VALUES(
                                                    :date_time,
                                                    :storage_id,
                                                    :storage_name,
                                                    :storage_free_space,
                                                    :storage_total_space,
                                                    :record_type )
                                                )");
        insertQuery.prepare(insertSQL);

        //set temporary values
        QString     line;
        QStringList fieldList;
        QString     dateTime;
        int         storageID;
        QString     storageName;
        qint64      storageFreeSpace;
        qint64      storageTotalSpace;
        QString     recordType;
        QRegularExpression tagExp("\t");

        //Skip titles line
        line = textStream.readLine();

        //load file to database
        while (!textStream.atEnd())
        {
            line = textStream.readLine();
            if (line.isNull())
                break;
            else
            {
                //Split the string with \t (tabulation) into a list
                fieldList.clear();
                fieldList = line.split(tagExp);
                dateTime            = fieldList[0];
                storageName         = fieldList[1];
                storageFreeSpace    = fieldList[2].toLongLong();
                storageTotalSpace   = fieldList[3].toLongLong();
                storageID           = fieldList[4].toInt();
                recordType          = fieldList[5];

                //Append data to the database
                insertQuery.bindValue(":date_time", dateTime);
                insertQuery.bindValue(":storage_id", storageID);
                insertQuery.bindValue(":storage_name", storageName);
                insertQuery.bindValue(":storage_free_space", QString::number(storageFreeSpace));
                insertQuery.bindValue(":storage_total_space", QString::number(storageTotalSpace));
                insertQuery.bindValue(":record_type", recordType);
                insertQuery.exec();
            }
        }
    }
}
//----------------------------------------------------------------------
void Collection::loadStatisticsDeviceFileToTable()
{// Load the contents of the storage statistics file into the database
    if(databaseMode=="Memory"){
        //clear database table
        QSqlQuery deleteQuery;
        deleteQuery.exec("DELETE FROM statistics_device");

        // Get infos stored in the file
        QFile statisticsDeviceFile(statisticsDeviceFilePath);
        if(!statisticsDeviceFile.open(QIODevice::ReadOnly)) {
            return;
        }

        QTextStream textStream(&statisticsDeviceFile);

        //prepare query to load file info
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

        //set temporary values
        QString     line;
        QStringList fieldList;
        QString     dateTime;
        int         deviceID;
        QString     deviceName;
        QString     deviceType;
        qint64      deviceFileCount;
        qint64      deviceTotalFileSize;
        qint64      deviceFreeSpace;
        qint64      deviceTotalSpace;
        QString     recordType;
        QRegularExpression tagExp("\t");

        //Skip titles line
        line = textStream.readLine();

        //load file to database
        while (!textStream.atEnd())
        {
            line = textStream.readLine();
            if (line.isNull())
                break;
            else
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
void Collection::saveStatiticsToFile()
{
    if(databaseMode=="Memory"){
        //Prepare export file
        QFile statisticsFile(statisticsStorageFilePath);
        QTextStream out(&statisticsFile);

        //Get data
        QSqlQuery query;
        QString querySQL = QLatin1String(R"(
                                    SELECT  date_time,
                                            storage_name,
                                            storage_free_space,
                                            storage_total_space,
                                            storage_id,
                                            record_type
                                    FROM statistics_storage
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

        if(statisticsFile.open(QIODevice::WriteOnly | QIODevice::Truncate)) {
            //out << textData;
            //Close the file
            //storageFile.close();
        }

        statisticsFile.close();
    }
}
//----------------------------------------------------------------------
