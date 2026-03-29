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
// File Name:   collection.cpp
// Purpose:     Class/model for the collection (all contents including devices, catalogs, files)
// Description:
// Author:      Stephane Couturier
/////////////////////////////////////////////////////////////////////////////
*/

#include "collection.h"
#include "database.h"
#include "device.h"
#include "catalog.h"
#include <QMutex>

Collection::Collection(QObject *parent) : QObject(parent)
{

}

//Database Schema Version--------------------------------------------------------
QString Collection::loadDatabaseSchemaVersion()
{
    QSqlQuery queryVersion(QSqlDatabase::database(m_connectionName));
    QString queryVersionSQL = QLatin1String(R"(
        SELECT parameter_value1
        FROM parameter
        WHERE parameter_type = 'collection'
        AND parameter_name = 'version'
    )");
    queryVersion.prepare(queryVersionSQL);
    queryVersion.exec();

    if (queryVersion.next()) {
        return queryVersion.value(0).toString();
    }
    return "0"; // Your logic for missing versions
}
//----------------------------------------------------------------------
void Collection::setDatabaseSchemaVersion()
{
    // First try to update existing record
    QSqlQuery queryUpdateVersion(QSqlDatabase::database(m_connectionName));
    QString queryUpdateVersionSQL = QLatin1String(R"(
                                    UPDATE parameter
                                    SET parameter_value1 =:parameter_value1
                                    WHERE parameter_type =:parameter_type
                                    AND parameter_name =:parameter_name
                                )");
    queryUpdateVersion.prepare(queryUpdateVersionSQL);
    queryUpdateVersion.bindValue(":parameter_name", "version");
    queryUpdateVersion.bindValue(":parameter_type", "collection");
    queryUpdateVersion.bindValue(":parameter_value1", dbSchemaVersion);
    queryUpdateVersion.exec();

    // If no rows affected, insert new record
    if (queryUpdateVersion.numRowsAffected() == 0) {
        QSqlQuery insertQuery(QSqlDatabase::database(m_connectionName));
        QString insertSQL = QLatin1String(R"(
            INSERT INTO parameter (
                parameter_name,
                parameter_type,
                parameter_value1
            ) VALUES (
                :parameter_name,
                :parameter_type,
                :parameter_value1
            )
        )");
        insertQuery.prepare(insertSQL);
        insertQuery.bindValue(":parameter_name", "version");
        insertQuery.bindValue(":parameter_type", "collection");
        insertQuery.bindValue(":parameter_value1", dbSchemaVersion);
        insertQuery.exec();
    }

    // Save to file if memory mode
    if (databaseMode == "Memory") {
        saveParameterTableToFile();
    }
}
//----------------------------------------------------------------------

//----------------------------------------------------------------------
void Collection::loadImageFolderPath()
{
    QSqlQuery query(QSqlDatabase::database(m_connectionName));
    query.prepare(QLatin1String(R"(
        SELECT parameter_value1
        FROM parameter
        WHERE parameter_type = 'collection'
        AND parameter_name = 'imageFolderPath'
    )"));
    query.exec();
    if (query.next())
        imageFolderPath = query.value(0).toString();
    else
        imageFolderPath = folder + "/images";
}
//----------------------------------------------------------------------
void Collection::saveImageFolderPath()
{
    QSqlQuery updateQuery(QSqlDatabase::database(m_connectionName));
    updateQuery.prepare(QLatin1String(R"(
        UPDATE parameter
        SET parameter_value1 = :value
        WHERE parameter_type = 'collection'
        AND parameter_name = 'imageFolderPath'
    )"));
    updateQuery.bindValue(":value", imageFolderPath);
    updateQuery.exec();
    if (updateQuery.numRowsAffected() == 0) {
        QSqlQuery insertQuery(QSqlDatabase::database(m_connectionName));
        insertQuery.prepare(QLatin1String(R"(
            INSERT INTO parameter (parameter_name, parameter_type, parameter_value1)
            VALUES ('imageFolderPath', 'collection', :value)
        )"));
        insertQuery.bindValue(":value", imageFolderPath);
        insertQuery.exec();
    }
    if (databaseMode == "Memory")
        saveParameterTableToFile();
}
//----------------------------------------------------------------------

//File paths and creation -----------------------------------------------
void Collection::generateCollectionFilesPaths()
{
    searchHistoryFilePath       = folder + "/" + "search_history.csv";
    storageFilePath             = folder + "/" + "storage.csv";
    deviceFilePath              = folder + "/" + "device.csv";
    statisticsDeviceFileName    = "statistics.csv";
    statisticsDeviceFilePath    = folder + "/" + statisticsDeviceFileName;
    parameterFilePath           = folder + "/" + "parameters.csv";
    tagFilePath                 = folder + "/" + "tags.csv";
    mappingFilePath             = folder + "/" + "device_mapping.csv";
    catalogFilterFilePath       = folder + "/" + "catalog_filter.csv";

    //v1.22 files
    deviceCatalogFilePath       = folder + "/" + "device_catalog.csv";
    statisticsCatalogFileName   = "statistics_catalog.csv";
    statisticsCatalogFilePath   = folder + "/" + statisticsCatalogFileName;
    statisticsStorageFileName   = "statistics_storage.csv";
    statisticsStorageFilePath   = folder + "/" + statisticsStorageFileName;
}

void Collection::generateCollectionFiles()
{
    if(databaseMode=="Memory"){
        //Device.csv
        QFile deviceFile(deviceFilePath);
        if (!deviceFile.exists()) {
            //Create an empty CSV file
            if (deviceFile.open(QIODevice::WriteOnly | QIODevice::Text)) {
                //File opened successfully, no need to write anything
                deviceFile.close(); // Close the file after creating it
            } else {
                qWarning() << "WARNING: DEBUG: Failed to create Device file:" << deviceFile.errorString();
            }
        }

        //Parameters.csv
        QFile parametersFile(parameterFilePath);
        if (!parametersFile.exists()) {
            //Create an empty CSV file
            if (parametersFile.open(QIODevice::WriteOnly | QIODevice::Text)) {
                //File opened successfully, no need to write anything
                parametersFile.close(); // Close the file after creating it
            } else {
                qWarning() << "WARNING: DEBUG: Failed to create Parameters file:" << parametersFile.errorString();
            }

            //Save
            saveParameterTableToFile();
        }

        //Storage.csv
        QFile newStorageFile(storageFilePath);
        if(!newStorageFile.open(QIODevice::ReadOnly)) {

            if (newStorageFile.open(QFile::WriteOnly | QFile::Text)) {

                QTextStream stream(&newStorageFile);

                stream << "ID"            << "\t"
                       << "Name"          << "\t"
                       << "Type"          << "\t"
                       << "Location"      << "\t"
                       << "Path"          << "\t"
                       << "Label"         << "\t"
                       << "FileSystem"    << "\t"
                       << "Total"         << "\t"
                       << "Free"          << "\t"
                       << "Brand"    << "\t"
                       << "Model"    << "\t"
                       << "SerialNumber"  << "\t"
                       << "BuildDate"     << "\t"
                       << "ContentType"   << "\t"
                       << "Container"     << "\t"
                       << "Comment"       << "\t"
                       << '\n';

                newStorageFile.close();

                return;
            }
        }

        //Device mapping
        QFile newMappingFile(mappingFilePath);
        if(!newMappingFile.open(QIODevice::ReadOnly)) {

            if (newMappingFile.open(QFile::WriteOnly | QFile::Text)) {

                QTextStream stream(&newMappingFile);

                stream  << "ID"               << "\t"
                        << "Name"             << "\t"
                        << "Type"             << "\t"
                        << "Device source ID" << "\t"
                        << "Device target ID" << "\t"
                        << "BackUp Last Date" << "\t"
                        << "Backup Last Size" << "\t"
                        << '\n';

                newMappingFile.close();

                return;
            }
        }
    }
}

//File loading-----------------------------------------------------------
bool Collection::load()
{//Load collection
    //Reset key values and clear database in "Memory" mode
    dbSchemaVersion ="";
    clearDatabaseData();

    //Check if new collection (the folder would be empty)
    QDir dir(folder);
    dir.setFilter(QDir::AllEntries | QDir::NoDotAndDotDot);
    if(dir.entryList().isEmpty()){
        dbSchemaVersion = appVersion;
        setDatabaseSchemaVersion();
    }

    //Generate collection files paths and statistics parameters
    generateCollectionFilesPaths();
    generateCollectionFiles();

    //Load Files to database
    loadParameterFileToTable();
    loadDeviceFileToTable();
    loadStorageFileToTable();
    loadCatalogFilesToTable();
    loadSearchHistoryFileToTable();
    loadMappingFileToTable();
    loadCatalogFilterFileToTable();

    //Add a default storage device, to force any new catalog to have one
    bool defaultsCreated = insertPhysicalStorageGroup();

    return defaultsCreated;

}
//----------------------------------------------------------------------
void Collection::clearDatabaseData()
{   //Clear database date in the context of Memory mode, prior to reloading files to tables
    if(databaseMode=="Memory"){

        // MEMORY SAFETY: Ensure database connection is valid before executing queries
        QSqlDatabase db = QSqlDatabase::database(m_connectionName);
        if (!db.isValid() || !db.isOpen()) {
            qWarning() << "WARNING: Database connection invalid - skipping clearDatabaseData";
            return;
        }

        // Disable foreign key constraints temporarily to avoid constraint violations (SQLite-specific)
        QSqlQuery pragmaQuery(db);
        if (Database::getDatabaseType(m_connectionName) == Database::DatabaseType::SQLite) {
            pragmaQuery.exec("PRAGMA foreign_keys = OFF");
        }

        // Execute DELETE queries in dependency order to avoid foreign key issues
        QSqlQuery queryDelete(db);

        //QSqlQuery queryDelete(QSqlDatabase::database(m_connectionName));
        queryDelete.exec("DELETE FROM device_catalog");
        queryDelete.exec("DELETE FROM virtual_storage_catalog");
        queryDelete.exec("DELETE FROM virtual_storage");
        queryDelete.exec("DELETE FROM statistics_catalog");
        queryDelete.exec("DELETE FROM statistics_storage");
        queryDelete.exec("DELETE FROM statistics_device");
        queryDelete.exec("DELETE FROM device_mapping");
        queryDelete.exec("DELETE FROM catalog_filter");
        queryDelete.exec("DELETE FROM tag");
        queryDelete.exec("DELETE FROM search");
        queryDelete.exec("DELETE FROM folder");
        queryDelete.exec("DELETE FROM filetemp");
        queryDelete.exec("DELETE FROM file");
        queryDelete.exec("DELETE FROM parameter");
        queryDelete.exec("DELETE FROM catalog");
        queryDelete.exec("DELETE FROM storage");
        queryDelete.exec("DELETE FROM device");

        //MIGRATION 1.22 to 2.0
        queryDelete.exec("DELETE FROM statistics_catalog");
        queryDelete.exec("DELETE FROM statistics_storage");
        queryDelete.exec("DELETE FROM virtual_storage");
        queryDelete.exec("DELETE FROM virtual_storage_catalog");
        queryDelete.exec("DELETE FROM device_catalog");

        // Re-enable foreign key constraints (SQLite-specific)
        if (Database::getDatabaseType(m_connectionName) == Database::DatabaseType::SQLite) {
            pragmaQuery.exec("PRAGMA foreign_keys = ON");
        }

    }
}
//----------------------------------------------------------------------
bool Collection::loadAllCatalogFiles(std::function<bool(int, int, const QString &)> progressCallback)
{//Load all catalog files to memory
    if (databaseMode != "Memory")
        return true;

    // Get total file count for progress reporting when a callback is provided
    int totalFileCount = 0;
    if (progressCallback) {
        QSqlQuery countQuery(QSqlDatabase::database(m_connectionName));
        countQuery.prepare(QLatin1String(R"(
            SELECT SUM(device_total_file_count)
            FROM device
            WHERE device_type = 'Catalog'
            AND device_group_id = 0
        )"));
        countQuery.exec();
        if (countQuery.next())
            totalFileCount = countQuery.value(0).toInt();
    }

    QSqlQuery query(QSqlDatabase::database(m_connectionName));
    query.prepare(QLatin1String(R"(
        SELECT device_id, device_name, device_total_file_count
        FROM device
        WHERE device_type = 'Catalog'
    )"));
    query.exec();

    int filesLoaded = 0;
    while (query.next()) {
        Device tempDevice;
        tempDevice.ID = query.value(0).toInt();
        QString deviceName = query.value(1).toString();
        int deviceFileCount = query.value(2).toInt();

        tempDevice.loadDevice(m_connectionName);
        QMutex tempMutex;
        bool tempStopRequested = false;
        tempDevice.catalog->loadCatalogFileListToTable(tempMutex, tempStopRequested);

        filesLoaded += deviceFileCount;

        if (progressCallback && !progressCallback(filesLoaded, totalFileCount, deviceName))
            return false; // cancelled
    }

    return true;
}
//----------------------------------------------------------------------
void Collection::loadDeviceFileToTable()
{
    if(databaseMode=="Memory"){
        // Clear table
        QSqlQuery query(QSqlDatabase::database(m_connectionName));
        QString querySQL = QLatin1String(R"(DELETE FROM device)");
        query.prepare(querySQL);
        query.exec();

        // Define storage file and prepare stream
        QFile deviceFile(deviceFilePath);
        QTextStream textStream(&deviceFile);

        // Open file or create it
        if(!deviceFile.open(QIODevice::ReadOnly)) {
            // Create it, if it does not exist
            QFile newDeviceFile(deviceFilePath);
            if (newDeviceFile.open(QFile::WriteOnly | QFile::Text)) {
                QTextStream stream(&newDeviceFile);
                stream << "ID\tParent ID\tName\tType\tExternalID\tPath\t"
                       << "total_file_size\ttotal_file_count\ttotal_space\t"
                       << "free_space\tactive\tgroupID\tdate updated\torder\n";
                newDeviceFile.close();
            } else {
                qWarning() << "WARNING: DEBUG: Failed to create device file:" << newDeviceFile.errorString();
                // Optionally: return early or handle the error appropriately
            }
        }

        // Load Device lines to table
        while (true)
        {
            QString line = textStream.readLine();
            if (line.isNull())
                break;
            else
                if (line.left(2)!="ID"){ // Skip header line

                    // Split the string with tabulation into a list
                    QStringList fieldList = line.split('\t');
                    // Check if the line has enough fields
                    if (fieldList.size() < 14) {
                        qWarning() << "WARNING: DEBUG: Collection::loadDeviceFileToTable() / Invalid line format:" << line;
                        continue; // Skip this line if it doesn't have enough fields
                    }
                    QSqlQuery insertQuery(QSqlDatabase::database(m_connectionName));
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
                                        device_active,
                                        device_group_id,
                                        device_date_updated,
                                        device_order )
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
                                        :device_active,
                                        :device_group_id,
                                        :device_date_updated,
                                        :device_order )
                    )");
                    insertQuery.prepare(querySQL);
                    insertQuery.bindValue(":device_id", fieldList[0].toInt());
                    insertQuery.bindValue(":device_parent_id", fieldList[1].toInt());
                    insertQuery.bindValue(":device_name", fieldList[2]);
                    if(fieldList.size() > 3){ // Prevent issues with old files
                        insertQuery.bindValue(":device_type", fieldList[3]);
                        insertQuery.bindValue(":device_external_id", fieldList[4].toInt());
                        insertQuery.bindValue(":device_path", fieldList[5]);
                        insertQuery.bindValue(":device_total_file_size", fieldList[6].toLongLong());
                        insertQuery.bindValue(":device_total_file_count", fieldList[7].toLongLong());
                        insertQuery.bindValue(":device_total_space", fieldList[8].toLongLong());
                        insertQuery.bindValue(":device_free_space", fieldList[9].toLongLong());
                        if(fieldList.size() > 10) {
                            // Convert string to boolean properly
                            bool activeValue = (fieldList[10].toLower() == "true" ||
                                                fieldList[10] == "1");
                            insertQuery.bindValue(":device_active", activeValue);
                        } else {
                            insertQuery.bindValue(":device_active", false); // Default
                        }
                        insertQuery.bindValue(":device_group_id", fieldList[11].toInt());
                        insertQuery.bindValue(":device_date_updated", fieldList[12]);
                        insertQuery.bindValue(":device_order", fieldList[13].toInt());
                    }
                    insertQuery.exec();
                }
        }
        deviceFile.close();
    }
}
//----------------------------------------------------------------------
void Collection::loadCatalogFilesToTable()
{
    if(databaseMode=="Memory"){
        //Clear catalog table
        QSqlQuery queryDelete(QSqlDatabase::database(m_connectionName));
        queryDelete.prepare( "DELETE FROM catalog" );
        queryDelete.exec();

        //Iterate in the directory to create a list of files and sort it
        QStringList catalogFileExtensions;
        catalogFileExtensions << "*.idx";

        QDirIterator iterator(folder, catalogFileExtensions, QDir::Files, QDirIterator::Subdirectories);
        while (iterator.hasNext()){

            // Iterate to the next file
            QString path = iterator.next();
            QFile catalogFile(path);

            // Get file info
            QFileInfo catalogFileInfo(catalogFile);

            // Verify that the file can be opened
            if(!catalogFile.open(QIODevice::ReadOnly)) {
                qWarning() << "WARNING: DEBUG: loadCatalogFilesToTable() / Could not open catalog file:" << catalogFile.errorString();
                return;
            }

            //Prepare a textsteam for the file
            QTextStream textStreamCatalogs(&catalogFile);

            //Read the first 12 lines and put values in a stringlist
            QStringList catalogValues;
            QString line;
            QString value;
            for (int i=0; i<12; i++) {
                line = textStreamCatalogs.readLine();
                if (line !="" and QVariant(line.at(0)).toString()=="<"){
                    value = line.right(line.size() - line.indexOf(">") - 1);
                    if (value =="") catalogValues << "";
                    else catalogValues << value;
                }
            }
            if (catalogValues.count()== 7) catalogValues << "false"; //for older catalog without isFullDevice
            if (catalogValues.count()== 8) catalogValues << "false"; //for older catalog without includeMetadata (v2.7)
            if (catalogValues.count()== 9) catalogValues << "";      //for older catalog without appVersion (v2.7)
            if (catalogValues.count()==10) catalogValues << 0;       //for older catalog without ID (v2.8)
            if (catalogValues.count()==11) catalogValues.insert(9, "None");  //for older catalog without includeChecksum (v2.9)

            if(catalogValues.length()>0){
                //Insert a line in the table with available data

                Catalog newCatalog;
                newCatalog.setConnectionName(m_connectionName); // use this collection's connection
                newCatalog.ID               = catalogValues[11].toInt(); //catalog_id
                newCatalog.filePath         = path; //catalog_file_path
                newCatalog.name             = catalogFileInfo.completeBaseName(); //catalog_name
                newCatalog.dateUpdated      = catalogFileInfo.lastModified();//.toString("yyyy-MM-dd hh:mm:ss"); //catalog_date_updated
                newCatalog.sourcePath       = catalogValues[0]; //catalog_source_path
                newCatalog.fileCount        = catalogValues[1].toLongLong(); //catalog_file_count
                newCatalog.totalFileSize    = catalogValues[2].toLongLong(); //catalog_total_file_size
                newCatalog.includeHidden    = catalogValues[3].compare("true", Qt::CaseInsensitive) == 0; //catalog_include_hidden
                newCatalog.fileType         = catalogValues[4]; //catalog_file_type
                newCatalog.storageName      = catalogValues[5]; //catalog_storage
                newCatalog.includeSymblinks = catalogValues[6].compare("true", Qt::CaseInsensitive) == 0; //catalog_include_symblinks
                newCatalog.isFullDevice     = catalogValues[7].compare("true", Qt::CaseInsensitive) == 0; //catalog_is_full_device
                newCatalog.includeMetadata  = catalogValues[8];
                if (newCatalog.includeMetadata == "false") {
                    newCatalog.includeMetadata = Catalog::METADATA_NONE;
                }
                newCatalog.includeChecksum  = catalogValues[9]; //catalog_include_checksum
                if (newCatalog.includeChecksum == "false" || newCatalog.includeChecksum.isEmpty()) {
                    newCatalog.includeChecksum = Catalog::CHECKSUM_NONE;
                }
                newCatalog.appVersion       = catalogValues[10]; //catalog_app_version
                newCatalog.insertCatalog();
            }
            catalogFile.close();
        }
    }
}
//----------------------------------------------------------------------
void Collection::loadStorageFileToTable()
{//load Storage file data to its table
    if (databaseMode=="Memory"){

        //Define storage file and prepare stream
        QFile storageFile(storageFilePath);
        QTextStream textStream(&storageFile);

        QSqlQuery queryDelete(QSqlDatabase::database(m_connectionName));
        queryDelete.prepare( "DELETE FROM storage" );

        //Open file or return information
        if(!storageFile.open(QIODevice::ReadOnly)) {
            queryDelete.exec();
            return;
        }

        //Test file validity (application breaks between v0.13 and v0.14)
        QString line = textStream.readLine();
        if (line.left(2)!="ID"){
            qWarning() << "WARNING: DEBUG: loadStorageFileToTable() / Storage file is not valid, cannot load it to table: " << storageFilePath;
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
                                        storage_brand,
                                        storage_model,
                                        storage_serial_number,
                                        storage_build_date,
                                        storage_comment1,
                                        storage_comment2,
                                        storage_comment3,
                                        storage_picture_path)
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
                                        :storage_brand,
                                        :storage_model,
                                        :storage_serial_number,
                                        :storage_build_date,
                                        :storage_comment1,
                                        :storage_comment2,
                                        :storage_comment3,
                                        :storage_picture_path)
                                )");

                    QSqlQuery insertQuery(QSqlDatabase::database(m_connectionName));
                    insertQuery.prepare(querySQL);
                    insertQuery.bindValue(":storage_id",            fieldList[0].toInt());
                    insertQuery.bindValue(":storage_name",          fieldList[1]);
                    insertQuery.bindValue(":storage_type",          fieldList[2]);
                    insertQuery.bindValue(":storage_location",      fieldList[3]);
                    insertQuery.bindValue(":storage_path",          fieldList[4]);
                    insertQuery.bindValue(":storage_label",         fieldList[5]);
                    insertQuery.bindValue(":storage_file_system",   fieldList[6]);
                    insertQuery.bindValue(":storage_total_space",   fieldList[7].toLongLong());
                    insertQuery.bindValue(":storage_free_space",    fieldList[8].toLongLong());
                    insertQuery.bindValue(":storage_brand",         fieldList[9]);
                    insertQuery.bindValue(":storage_model",         fieldList[10]);
                    insertQuery.bindValue(":storage_serial_number", fieldList[11]);
                    insertQuery.bindValue(":storage_build_date",    fieldList[12]);
                    insertQuery.bindValue(":storage_comment1",      fieldList[13]);
                    insertQuery.bindValue(":storage_comment2",      fieldList[14]);
                    insertQuery.bindValue(":storage_comment3",      fieldList[15]);
                    insertQuery.bindValue(":storage_picture_path",  fieldList.size() > 16 ? fieldList[16] : "");

                    if(line!="")
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
        QSqlQuery deleteQuery(QSqlDatabase::database(m_connectionName));
        deleteQuery.exec("DELETE FROM statistics_device");

        //Get infos stored in the file
        QFile statisticsDeviceFile(statisticsDeviceFilePath);
        if(!statisticsDeviceFile.open(QIODevice::ReadOnly)) {
            return;
        }

        QTextStream textStream(&statisticsDeviceFile);

        //Prepare query to load file info
        QSqlQuery insertQuery(QSqlDatabase::database(m_connectionName));
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
                fieldList = line.split('\t');

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
void Collection::loadParameterFileToTable()
{// Load the contents of the storage statistics file into the database
    if(databaseMode=="Memory"){
        //Clear database table
        QSqlQuery deleteQuery(QSqlDatabase::database(m_connectionName));
        deleteQuery.exec("DELETE FROM parameters");

        //Get data stored in the file
        QFile parametersFile(parameterFilePath);
        if(parametersFile.open(QIODevice::ReadOnly)) {
            QTextStream textStream(&parametersFile);

            //Prepare query to load file info
            QSqlQuery insertQuery(QSqlDatabase::database(m_connectionName));
            QString insertSQL = QLatin1String(R"(
                                        INSERT INTO parameter (
                                                    parameter_name,
                                                    parameter_type,
                                                    parameter_value1,
                                                    parameter_value2)
                                        VALUES(
                                                    :parameter_name,
                                                    :parameter_type,
                                                    :parameter_value1,
                                                    :parameter_value2)
                                )");
            insertQuery.prepare(insertSQL);

            //Set temporary values
            QString     line;

            //Skip headers line
            textStream.readLine();

            //Load file to database
            while (!textStream.atEnd())
            {
                line = textStream.readLine();

                //Split the string with tabulation into a list
                QStringList fieldList = line.split('\t');
                if (line.isNull())
                    break;
                else
                {
                    //Append data to the database
                    insertQuery.bindValue(":parameter_name", fieldList[0]);
                    insertQuery.bindValue(":parameter_type", fieldList[1]);
                    insertQuery.bindValue(":parameter_value1", fieldList[2]);
                    insertQuery.bindValue(":parameter_value2", fieldList.size() > 3 ? fieldList[3] : "");
                    insertQuery.exec();
                }
            }
        }
        else{
            qWarning() << "WARNING: DEBUG: Could not open parameters.csv:" << parametersFile.errorString();
        }
    }

    //Get collection version
    QSqlQuery queryVersion(QSqlDatabase::database(m_connectionName));
    QString queryVersionSQL = QLatin1String(R"(
                                    SELECT parameter_value1
                                    FROM parameter
                                    WHERE parameter_type =:parameter_type
                                    AND parameter_name =:parameter_name
                                )");
    queryVersion.prepare(queryVersionSQL);
    queryVersion.bindValue(":parameter_type", "collection");
    queryVersion.bindValue(":parameter_name", "version");
    queryVersion.exec();

    while(queryVersion.next()){
        dbSchemaVersion = queryVersion.value(0).toString();
    }
}
//----------------------------------------------------------------------
void Collection::loadSearchHistoryFileToTable()
{
    if(databaseMode=="Memory"){

        //Define storage file and prepare stream
        QFile searchFile(searchHistoryFilePath);
        QTextStream textStream(&searchFile);

        QSqlQuery queryDelete(QSqlDatabase::database(m_connectionName));
        queryDelete.prepare( "DELETE FROM search" );

        //Open file or return information
        if(!searchFile.open(QIODevice::ReadOnly)) {
            return;
        }
        //Clear all entries of the current table
        queryDelete.exec();
        //Skip headers
        QString line = textStream.readLine();
        if(line.left(4)=="date"){
            while (true)
            {
                QString line = textStream.readLine();
                if (line.isNull())
                    break;
                else
                    if (line.left(2)!="ID"){//test the validity of the file

                        //Split the string with tabulation into a list
                        QStringList fieldList = line.split('\t');

                        //add empty values to support the addition of new fields to files from older versions
                        int  targetFieldsCount = 43;
                        int currentFiledsCount = fieldList.count();
                        int    diffFieldsCount = targetFieldsCount - currentFiledsCount;
                        if(diffFieldsCount !=0){
                            for(int i=0; i<diffFieldsCount; i++){
                                fieldList.append("");
                            }
                        }

                        QSqlQuery insertQuery(QSqlDatabase::database(m_connectionName));
                        QString insertQuerySQL = QLatin1String(R"(
                                                INSERT INTO search(
                                                    date_time,
                                                    text_checked,
                                                    text_phrase,
                                                    text_criteria,
                                                    text_search_in,
                                                    file_type,
                                                    file_size_checked,
                                                    file_size_min,
                                                    file_size_min_unit,
                                                    file_size_max,
                                                    file_size_max_unit,
                                                    date_modified_checked,
                                                    date_modified_min,
                                                    date_modified_max,
                                                    duplicates_checked,
                                                    duplicates_name,
                                                    duplicates_size,
                                                    duplicates_date_modified,
                                                    differences_checked,
                                                    differences_name,
                                                    differences_size,
                                                    differences_date_modified,
                                                    differences_catalogs,
                                                    show_folders,
                                                    tag_checked,
                                                    tag,
                                                    search_location,
                                                    search_storage,
                                                    search_catalog,
                                                    search_catalog_checked,
                                                    search_directory_checked,
                                                    selected_directory,
                                                    text_exclude,
                                                    case_sensitive,
                                                    file_type_checked,
                                                    file_criteria_checked,
                                                    folder_criteria_checked,
                                                    selected_device_ID_list,
                                                    duplicates_checksum,
                                                    duplicates_checksum_equal,
                                                    duplicates_compare_checked,
                                                    duplicates_device1_ID,
                                                    duplicates_device2_ID
                                                    )
                                                VALUES(
                                                    :date_time,
                                                    :text_checked,
                                                    :text_phrase,
                                                    :text_criteria,
                                                    :text_search_in,
                                                    :file_type,
                                                    :file_size_checked,
                                                    :file_size_min,
                                                    :file_size_min_unit,
                                                    :file_size_max,
                                                    :file_size_max_unit,
                                                    :date_modified_checked,
                                                    :date_modified_min,
                                                    :date_modified_max,
                                                    :duplicates_checked,
                                                    :duplicates_name,
                                                    :duplicates_size,
                                                    :duplicates_date_modified,
                                                    :differences_checked,
                                                    :differences_name,
                                                    :differences_size,
                                                    :differences_date_modified,
                                                    :differences_catalogs,
                                                    :show_folders,
                                                    :tag_checked,
                                                    :tag,
                                                    :search_location,
                                                    :search_storage,
                                                    :search_catalog,
                                                    :search_catalog_checked,
                                                    :search_directory_checked,
                                                    :selected_directory,
                                                    :text_exclude,
                                                    :case_sensitive,
                                                    :file_type_checked,
                                                    :file_criteria_checked,
                                                    :folder_criteria_checked,
                                                    :selected_device_ID_list,
                                                    :duplicates_checksum,
                                                    :duplicates_checksum_equal,
                                                    :duplicates_compare_checked,
                                                    :duplicates_device1_ID,
                                                    :duplicates_device2_ID
                                                    )
                                                )");

                        insertQuery.prepare(insertQuerySQL);
                        insertQuery.bindValue(":date_time",                 fieldList[0]);
                        insertQuery.bindValue(":text_checked",              fieldList[1]);
                        insertQuery.bindValue(":text_phrase",               fieldList[2]);
                        insertQuery.bindValue(":text_criteria",             fieldList[3]);
                        insertQuery.bindValue(":text_search_in",            fieldList[4]);
                        insertQuery.bindValue(":file_type",                 fieldList[5]);
                        insertQuery.bindValue(":file_size_checked",         fieldList[6]);
                        insertQuery.bindValue(":file_size_min",             fieldList[7]);
                        insertQuery.bindValue(":file_size_min_unit",        fieldList[8]);
                        insertQuery.bindValue(":file_size_max",             fieldList[9]);
                        insertQuery.bindValue(":file_size_max_unit",        fieldList[10]);
                        insertQuery.bindValue(":date_modified_checked",     fieldList[11]);
                        insertQuery.bindValue(":date_modified_min",         fieldList[12]);
                        insertQuery.bindValue(":date_modified_max",         fieldList[13]);
                        insertQuery.bindValue(":duplicates_checked",        fieldList[14]);
                        insertQuery.bindValue(":duplicates_name",           fieldList[15]);
                        insertQuery.bindValue(":duplicates_size",           fieldList[16]);
                        insertQuery.bindValue(":duplicates_date_modified",  fieldList[17]);
                        insertQuery.bindValue(":show_folders",              fieldList[18]);
                        insertQuery.bindValue(":tag_checked",               fieldList[19]);
                        insertQuery.bindValue(":tag",                       fieldList[20]);
                        insertQuery.bindValue(":search_location",           fieldList[21]);
                        insertQuery.bindValue(":search_storage",            fieldList[22]);
                        insertQuery.bindValue(":search_catalog",            fieldList[23]);
                        insertQuery.bindValue(":search_catalog_checked",    fieldList[24]);
                        insertQuery.bindValue(":search_directory_checked",  fieldList[25]);
                        insertQuery.bindValue(":selected_directory",        fieldList[26]);
                        insertQuery.bindValue(":text_exclude",              fieldList[27]);
                        insertQuery.bindValue(":case_sensitive",            fieldList[28]);
                        insertQuery.bindValue(":differences_checked",       fieldList[29]);
                        insertQuery.bindValue(":differences_name",          fieldList[30]);
                        insertQuery.bindValue(":differences_size",          fieldList[31]);
                        insertQuery.bindValue(":differences_date_modified", fieldList[32]);
                        insertQuery.bindValue(":differences_catalogs",      fieldList[33]);
                        insertQuery.bindValue(":file_type_checked",         fieldList[34]);
                        insertQuery.bindValue(":file_criteria_checked",     fieldList[35]);
                        insertQuery.bindValue(":folder_criteria_checked",   fieldList[36]);
                        insertQuery.bindValue(":selected_device_ID_list",   fieldList[37]);
                        insertQuery.bindValue(":duplicates_checksum",       fieldList[38]);
                        insertQuery.bindValue(":duplicates_checksum_equal", fieldList[39]);
                        insertQuery.bindValue(":duplicates_compare_checked",fieldList[40]);
                        insertQuery.bindValue(":duplicates_device1_ID",     fieldList[41]);
                        insertQuery.bindValue(":duplicates_device2_ID",     fieldList[42]);
                        insertQuery.exec();
                    }
            }
            searchFile.close();
        }
    }
}
//----------------------------------------------------------------------
void Collection::loadTagFileToTable()
{
    if(databaseMode=="Memory"){

        //Define storage file and prepare stream
        QFile tagFile(tagFilePath);
        QTextStream textStream(&tagFile);

        QSqlQuery queryDelete(QSqlDatabase::database(m_connectionName));
        queryDelete.prepare( "DELETE FROM tag" );

        //Open file or return information
        if(!tagFile.open(QIODevice::ReadOnly)) {
            return;
        }
        //Clear all entries of the current table
        queryDelete.exec();

        //Skip headers
        QString line = textStream.readLine();
        if(line.left(2)=="ID"){
            while (true)
            {
                line = textStream.readLine();
                if (line.isNull())
                    break;
                else
                {    //Split the string with tabulation into a list
                    QStringList fieldList = line.split('\t');
                    QSqlQuery insertQuery(QSqlDatabase::database(m_connectionName));
                    QString insertQuerySQL = QLatin1String(R"(
                                            INSERT INTO tag(
                                                ID,
                                                name,
                                                path,
                                                type,
                                                date_time)
                                            VALUES(
                                                :ID,
                                                :name,
                                                :path,
                                                :type,
                                                :date_time)
                                            )");
                    insertQuery.prepare(insertQuerySQL);
                    insertQuery.bindValue(":ID",        fieldList[0].toInt());
                    insertQuery.bindValue(":name",      fieldList[1]);
                    insertQuery.bindValue(":path",      fieldList[2]);
                    insertQuery.bindValue(":type",      fieldList[3]);
                    insertQuery.bindValue(":date_time", fieldList[4]);
                    insertQuery.exec();
                }
            }
            tagFile.close();
        }
    }
}
//----------------------------------------------------------------------
void Collection::loadMappingFileToTable()
{
    if(databaseMode=="Memory"){

        //Define storage file and prepare stream
        QFile mappingFile(mappingFilePath);
        QTextStream textStream(&mappingFile);

        QSqlQuery queryDelete(QSqlDatabase::database(m_connectionName));
        queryDelete.prepare( "DELETE FROM device_mapping" );

        //Open file or return information
        if(!mappingFile.open(QIODevice::ReadOnly)) {
            return;
        }
        //Clear all entries of the current table
        queryDelete.exec();

        //Skip headers
        QString line = textStream.readLine();

        while (true)
        {
            line = textStream.readLine();
            if (line.isNull())
                break;
            else
            {    //Split the string with tabulation into a list
                QStringList fieldList = line.split('\t');
                QSqlQuery insertQuery(QSqlDatabase::database(m_connectionName));
                QString insertQuerySQL = QLatin1String(R"(
                                        INSERT INTO device_mapping(
                                            mapping_id,
                                            mapping_name,
                                            mapping_type,
                                            mapping_device_source_id,
                                            mapping_device_target_id,
                                            mapping_backup_last_date,
                                            mapping_backup_last_size,
                                            mapping_strict_copy,
                                            mapping_conflict_mode,
                                            mapping_source_mode,
                                            mapping_source_collection
                                        )
                                        VALUES(
                                            :mapping_id,
                                            :mapping_name,
                                            :mapping_type,
                                            :mapping_device_source_id,
                                            :mapping_device_target_id,
                                            :mapping_backup_last_date,
                                            :mapping_backup_last_size,
                                            :mapping_strict_copy,
                                            :mapping_conflict_mode,
                                            :mapping_source_mode,
                                            :mapping_source_collection
                                        )
                                        )");
                insertQuery.prepare(insertQuerySQL);
                insertQuery.bindValue(":mapping_id",               fieldList[0].toInt());
                insertQuery.bindValue(":mapping_name",             fieldList[1]);
                insertQuery.bindValue(":mapping_type",             fieldList[2]);
                insertQuery.bindValue(":mapping_device_source_id", fieldList[3]);
                insertQuery.bindValue(":mapping_device_target_id", fieldList[4]);
                insertQuery.bindValue(":mapping_backup_last_date", fieldList[5]);
                insertQuery.bindValue(":mapping_backup_last_size", fieldList[6]);
                // field[7] = strict_copy (may be absent in old CSV files — default to 1)
                insertQuery.bindValue(":mapping_strict_copy",      fieldList.size() > 7 ? fieldList[7].toInt() : 1);
                // field[8] = conflict_mode as TEXT (e.g. "Skip", "RenameOldest") — defaults to RenameOldest if absent
                insertQuery.bindValue(":mapping_conflict_mode",
                    fieldList.size() > 8 ? fieldList[8] : QStringLiteral("RenameOldest"));
                // field[9] = source_mode ('Catalog'/'Drive') — defaults to 'Catalog' if absent
                insertQuery.bindValue(":mapping_source_mode",
                    fieldList.size() > 9 ? fieldList[9] : QStringLiteral("Catalog"));
                // field[10] = source_collection path — empty for BackUp mappings, populated for CollectionImport
                insertQuery.bindValue(":mapping_source_collection",
                    fieldList.size() > 10 ? fieldList[10] : QString());
                insertQuery.exec();
            }
        }
        mappingFile.close();
    }
}
//----------------------------------------------------------------------
void Collection::loadCatalogFilterFileToTable()
{
    if (databaseMode == "Memory") {
        QFile filterFile(catalogFilterFilePath);
        QTextStream textStream(&filterFile);

        QSqlQuery queryDelete(QSqlDatabase::database(m_connectionName));
        queryDelete.prepare("DELETE FROM catalog_filter");

        if (!filterFile.open(QIODevice::ReadOnly)) {
            return; // File does not exist yet — nothing to load
        }
        queryDelete.exec();

        // Skip header line
        textStream.readLine();

        while (true) {
            QString line = textStream.readLine();
            if (line.isNull())
                break;
            QStringList fields = line.split('\t');
            if (fields.size() < 3)
                continue;
            QSqlQuery insertQuery(QSqlDatabase::database(m_connectionName));
            insertQuery.prepare(QLatin1String(R"(
                INSERT OR IGNORE INTO catalog_filter (filter_catalog_id, filter_type, filter_value)
                VALUES (:catalog_id, :type, :value)
            )"));
            insertQuery.bindValue(":catalog_id", fields[0].toInt());
            insertQuery.bindValue(":type",       fields[1]);
            insertQuery.bindValue(":value",      fields[2]);
            insertQuery.exec();
        }
        filterFile.close();
    }
}
//----------------------------------------------------------------------
//File saving ----------------------------------------------------------
void Collection::saveDeviceTableToFile()
{
    if (databaseMode == "Memory"){
        QFile deviceFile(deviceFilePath);

        //Get data
        QSqlQuery query(QSqlDatabase::database(m_connectionName));
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
                                            device_date_updated        ,
                                            device_order
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
                             << "order"
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
        out << "ID"            << "\t"
            << "Name"          << "\t"
            << "Type"          << "\t"
            << "Location"      << "\t"
            << "Path"          << "\t"
            << "Label"         << "\t"
            << "FileSystem"    << "\t"
            << "Total"         << "\t"
            << "Free"          << "\t"
            << "Brand"         << "\t"
            << "Model"         << "\t"
            << "SerialNumber"  << "\t"
            << "BuildDate"     << "\t"
            << "Comment1"      << "\t"
            << "Comment2"      << "\t"
            << "Comment3"      << "\t"
            << "PicturePath"   << "\t"
            << '\n';

        //Get data
        QSqlQuery query(QSqlDatabase::database(m_connectionName));
        QString querySQL = QLatin1String(R"(
                         SELECT
                            storage_id            ,
                            storage_name          ,
                            storage_type          ,
                            storage_location      ,
                            storage_path          ,
                            storage_label         ,
                            storage_file_system   ,
                            storage_total_space   ,
                            storage_free_space    ,
                            storage_brand         ,
                            storage_model         ,
                            storage_serial_number ,
                            storage_build_date    ,
                            storage_comment1      ,
                            storage_comment2      ,
                            storage_comment3      ,
                            storage_picture_path
                        FROM storage
                                    )");
        query.prepare(querySQL);
        query.exec();

        if(storageFile.open(QIODevice::WriteOnly | QIODevice::Truncate)) {
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
        }
        storageFile.close();
    }
}
//----------------------------------------------------------------------
void Collection::saveStatiticsTableToFile()
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
        QSqlQuery query(QSqlDatabase::database(m_connectionName));
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
void Collection::saveParameterTableToFile()
{
    if(databaseMode=="Memory"){
        //Prepare export file
        QFile parameterFile(parameterFilePath);
        QTextStream out(&parameterFile);

        //Prepare header line
        out << "name"       << "\t"
            << "type"       << "\t"
            << "value1"     << "\t"
            << "value2"     << "\t"
            << '\n';

        //Get data
        QSqlQuery query(QSqlDatabase::database(m_connectionName));
        QString querySQL = QLatin1String(R"(
                                    SELECT  parameter_name,
                                            parameter_type,
                                            parameter_value1,
                                            parameter_value2
                                    FROM parameter
                                )");
        query.prepare(querySQL);
        query.exec();

        if(parameterFile.open(QFile::WriteOnly | QFile::Text)) {
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

        parameterFile.close();
    }
}
//----------------------------------------------------------------------
void Collection::saveSearchHistoryTableToFile()
{//To keep forward compatibility, new field shall be added at the end of the column list, not in the order of the table
    if(databaseMode=="Memory"){
        //Prepare export
        QFile searchFile(searchHistoryFilePath);
        if(searchFile.open(QIODevice::WriteOnly | QIODevice::Truncate)) {

            QTextStream out(&searchFile);

            //Prepare header line
            out << "date_time"                  << "\t"
                << "text_checked"               << "\t"
                << "text_phrase"                << "\t"
                << "text_criteria"              << "\t"
                << "text_search_in"             << "\t"
                << "file_type"                  << "\t"
                << "file_size_checked"          << "\t"
                << "file_size_min"              << "\t"
                << "file_size_min_unit"         << "\t"
                << "file_size_max"              << "\t"
                << "file_size_max_unit"         << "\t"
                << "date_modified_checked"      << "\t"
                << "date_modified_min"          << "\t"
                << "date_modified_max"          << "\t"
                << "duplicates_checked"         << "\t"
                << "duplicates_name"            << "\t"
                << "duplicates_size"            << "\t"
                << "duplicates_date_modified"   << "\t"
                << "show_folders"               << "\t"
                << "tag_checked"                << "\t"
                << "tag"                        << "\t"
                << "search_location"            << "\t"
                << "search_storage"             << "\t"
                << "search_catalog"             << "\t"
                << "search_catalog_checked"     << "\t"
                << "search_directory_checked"   << "\t"
                << "selected_directory"         << "\t"
                << "text_exclude"               << "\t"
                << "case_sensitive"             << "\t"
                << "differences_checked"        << "\t"
                << "differences_name"           << "\t"
                << "differences_size"           << "\t"
                << "differences_date_modified"  << "\t"
                << "differences_catalogs"       << "\t"
                << "file_type_checked"          << "\t"
                << "file_criteria_checked"      << "\t"
                << "folder_criteria_checked"    << "\t"
                << "selected_device_ID_list"    << "\t"
                << "duplicates_checksum"        << "\t"
                << "duplicates_checksum_equal"  << "\t"
                << "duplicates_compare_devices" << "\t"
                << "duplicates_device1"         << "\t"
                << "duplicates_device2"         << "\t"
                << '\n';

            //Get data
            QSqlQuery query(QSqlDatabase::database(m_connectionName));
            QString querySQL = QLatin1String(R"(
                                        SELECT
                                            date_time,
                                            text_checked,
                                            text_phrase,
                                            text_criteria,
                                            text_search_in,
                                            file_type,
                                            file_size_checked,
                                            file_size_min,
                                            file_size_min_unit,
                                            file_size_max,
                                            file_size_max_unit,
                                            date_modified_checked,
                                            date_modified_min,
                                            date_modified_max,
                                            duplicates_checked,
                                            duplicates_name,
                                            duplicates_size,
                                            duplicates_date_modified,
                                            show_folders,
                                            tag_checked,
                                            tag,
                                            search_location,
                                            search_storage,
                                            search_catalog,
                                            search_catalog_checked,
                                            search_directory_checked,
                                            selected_directory,
                                            text_exclude,
                                            case_sensitive,
                                            differences_checked,
                                            differences_name,
                                            differences_size,
                                            differences_date_modified,
                                            differences_catalogs,
                                            file_type_checked,
                                            file_criteria_checked,
                                            folder_criteria_checked,
                                            selected_device_ID_list,
                                            duplicates_checksum,
                                            duplicates_checksum_equal,
                                            duplicates_compare_checked,
                                            duplicates_device1_ID,
                                            duplicates_device2_ID
                                        FROM search
                                        ORDER BY date_time DESC
                                       )");
            query.prepare(querySQL);
            query.exec();

            //Iterate the result and write each line
            while (query.next()) {
                const QSqlRecord record = query.record();
                for (int i=0, recCount = record.count() ; i<recCount ; ++i){
                    if (i>0)
                        out << '\t';
                    out << record.value(i).toString();
                }
                out << '\n';
            }
        }
        searchFile.close();
    }
}
//----------------------------------------------------------------------
void Collection::saveTagTableToFile()
{
    if(databaseMode=="Memory"){
        //Prepare export file
        QFile tagFile(tagFilePath);
        QTextStream out(&tagFile);

        //Prepare header line
        out << "ID"         << "\t"
            << "name"       << "\t"
            << "path"       << "\t"
            << "type"       << "\t"
            << "date_time"  << "\t"
            << '\n';

        //Get data
        QSqlQuery query(QSqlDatabase::database(m_connectionName));
        QString querySQL = QLatin1String(R"(
                                    SELECT  ID,
                                            name,
                                            path,
                                            type,
                                            date_time
                                    FROM tag
                                )");
        query.prepare(querySQL);
        query.exec();

        if(tagFile.open(QFile::WriteOnly | QFile::Text)) {
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
        tagFile.close();
    }
}
//----------------------------------------------------------------------
void Collection::saveMappingTableToFile()
{
    if(databaseMode=="Memory"){
        //Prepare export file
        QFile mappingFile(mappingFilePath);
        QTextStream out(&mappingFile);

        //Prepare header line
        out << "id"                        << "\t"
            << "name"                      << "\t"
            << "type"                      << "\t"
            << "device_source_id"          << "\t"
            << "device_target_id"          << "\t"
            << "backup_last_date"          << "\t"
            << "backup_last_size"          << "\t"
            << "strict_copy"               << "\t"
            << "conflict_mode"             << "\t"
            << "source_mode"               << "\t"
            << "source_collection"         << "\t"
            << '\n';

        //Get data
        QSqlQuery query(QSqlDatabase::database(m_connectionName));
        QString querySQL = QLatin1String(R"(
                                    SELECT
                                        mapping_id,
                                        mapping_name,
                                        mapping_type,
                                        mapping_device_source_id,
                                        mapping_device_target_id,
                                        mapping_backup_last_date,
                                        mapping_backup_last_size,
                                        mapping_strict_copy,
                                        mapping_conflict_mode,
                                        mapping_source_mode,
                                        mapping_source_collection
                                    FROM device_mapping
                                )");
        query.prepare(querySQL);
        query.exec();

        if(mappingFile.open(QFile::WriteOnly | QFile::Text)) {
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
        mappingFile.close();
    }
}
//----------------------------------------------------------------------
void Collection::saveCatalogFilterTableToFile()
{
    if (databaseMode == "Memory") {
        QFile filterFile(catalogFilterFilePath);
        QTextStream out(&filterFile);

        QSqlQuery query(QSqlDatabase::database(m_connectionName));
        query.prepare(QLatin1String("SELECT filter_catalog_id, filter_type, filter_value FROM catalog_filter ORDER BY filter_catalog_id, filter_type, filter_value"));
        query.exec();

        if (filterFile.open(QFile::WriteOnly | QFile::Text)) {
            out << "catalog_id" << "\t" << "type" << "\t" << "value" << "\n";
            while (query.next()) {
                out << query.value(0).toString() << "\t"
                    << query.value(1).toString() << "\t"
                    << query.value(2).toString() << "\n";
            }
        }
        filterFile.close();
    }
}
//----------------------------------------------------------------------

//File deleting---------------------------------------------------------
Collection::DeleteCatalogResult Collection::deleteCatalogFile(Device *device) {
    if(databaseMode=="Memory"){
        // Move file to trash
        if (device->catalog->filePath != "") {
            QFile file(device->catalog->filePath);
            if (!file.moveToTrash()) {
                return DeleteFailedToMoveToTrash;
            }

            QString foldersFilePath = device->catalog->filePath;
            foldersFilePath.chop(4);
            foldersFilePath += ".folders.idx";
            QFile foldersFile(foldersFilePath);
            foldersFile.moveToTrash(); // Optional: check return value here too

            return DeleteSuccess;
        }
        else {
            return DeleteInvalidPath;
        }
    }
    return DeleteSuccess; // Success for non-Memory mode
}
//----------------------------------------------------------------------

//Exclude directory management ------------------------------------------
bool Collection::addExcludeDirectory(const QString &path)
{
    QString cleanedPath = path;
    int pathLength = cleanedPath.length();
    if (!cleanedPath.isEmpty() && cleanedPath != "/" &&
        QVariant(cleanedPath.at(pathLength - 1)).toString() == "/") {
        cleanedPath.remove(pathLength - 1, 1);
    }

    if (cleanedPath.isEmpty())
        return false;

    QSqlQuery insertQuery(QSqlDatabase::database(m_connectionName));
    QString insertSQL = QLatin1String(R"(
        INSERT INTO parameter (
            parameter_name,
            parameter_type,
            parameter_value2)
        VALUES(
            :parameter_name,
            :parameter_type,
            :parameter_value2)
    )");
    insertQuery.prepare(insertSQL);
    insertQuery.bindValue(":parameter_name", "");
    insertQuery.bindValue(":parameter_type", "exclude_directory");
    insertQuery.bindValue(":parameter_value2", cleanedPath);

    if (!insertQuery.exec()) {
        qWarning() << "WARNING: Failed to add exclude directory:" << insertQuery.lastError().text();
        return false;
    }

    saveParameterTableToFile();
    return true;
}
//----------------------------------------------------------------------
bool Collection::removeExcludeDirectory(const QString &path)
{
    QSqlQuery query(QSqlDatabase::database(m_connectionName));
    QString querySQL = QLatin1String(R"(
        DELETE FROM parameter
        WHERE parameter_type ='exclude_directory'
        AND parameter_value2=:parameter_value2
    )");
    query.prepare(querySQL);
    query.bindValue(":parameter_value2", path);

    if (!query.exec()) {
        qWarning() << "WARNING: Failed to remove exclude directory:" << query.lastError().text();
        return false;
    }

    saveParameterTableToFile();
    return true;
}
//----------------------------------------------------------------------
QStringList Collection::getExcludeDirectories()
{
    QStringList directories;
    QSqlQuery query(QSqlDatabase::database(m_connectionName));
    QString querySQL = QLatin1String(R"(
        SELECT DISTINCT parameter_value2
        FROM parameter
        WHERE parameter_type ='exclude_directory'
        ORDER BY parameter_value2
    )");

    if (!query.exec(querySQL)) {
        qWarning() << "WARNING: Failed to get exclude directories:" << query.lastError().text();
        return directories;
    }

    while (query.next()) {
        directories << query.value(0).toString();
    }
    return directories;
}
//----------------------------------------------------------------------

//Tag CRUD -------------------------------------------------------------
bool Collection::createTag(const QString &name, const QString &path, const QString &type, const QDateTime &dateTime)
{
    QSqlQuery insertQuery(QSqlDatabase::database(m_connectionName));
    QString insertQuerySQL = QLatin1String(R"(
        INSERT INTO tag(
            ID,
            name,
            path,
            type,
            date_time)
        VALUES(
            NULL,
            :name,
            :path,
            :type,
            :date_time)
    )");
    insertQuery.prepare(insertQuerySQL);
    insertQuery.bindValue(":name", name);
    insertQuery.bindValue(":path", path);
    insertQuery.bindValue(":type", type);
    insertQuery.bindValue(":date_time", dateTime.toString("yyyy/MM/dd hh:mm:ss"));

    if (!insertQuery.exec()) {
        qWarning() << "WARNING: Failed to create tag:" << insertQuery.lastError().text();
        return false;
    }

    saveTagTableToFile();
    return true;
}
//----------------------------------------------------------------------
bool Collection::deleteTag(int tagID)
{
    QSqlQuery query(QSqlDatabase::database(m_connectionName));
    QString querySQL = QLatin1String(R"(
        DELETE FROM tag
        WHERE ID=:ID
    )");
    query.prepare(querySQL);
    query.bindValue(":ID", tagID);

    if (!query.exec()) {
        qWarning() << "WARNING: Failed to delete tag:" << query.lastError().text();
        return false;
    }

    saveTagTableToFile();
    return true;
}
//----------------------------------------------------------------------

//Export operations -----------------------------------------------------
bool Collection::exportAllToMemoryMode(const QString &exportFolder)
{
    // Save original settings
    QString originalFolder = folder;
    QString originalDatabaseMode = databaseMode;

    // Set the export folder and generate paths
    folder = exportFolder;
    generateCollectionFilesPaths();

    // Temporarily set to Memory mode so save functions will run
    databaseMode = "Memory";

    // Initialize export folder structure
    generateCollectionFiles();

    // Export all tables
    saveParameterTableToFile();
    saveDeviceTableToFile();
    saveStorageTableToFile();
    saveTagTableToFile();
    saveMappingTableToFile();
    saveSearchHistoryTableToFile();
    saveStatiticsTableToFile();

    // Restore original settings
    databaseMode = originalDatabaseMode;
    folder = originalFolder;
    generateCollectionFilesPaths();

    return true;
}
//----------------------------------------------------------------------
bool Collection::exportToSQLiteFile(
    const QString &filePath,
    std::function<bool(int current, int total, const QString &tableName)> progressCallback)
{
    const QString exportConn = QStringLiteral("katalogExportSQLiteConn");

    // Remove existing file so we start fresh
    QFile existingFile(filePath);
    if (existingFile.exists())
        existingFile.remove();

    // Create and open the target SQLite database
    {
        QSqlDatabase db = QSqlDatabase::addDatabase(QStringLiteral("QSQLITE"), exportConn);
        db.setDatabaseName(filePath);
        if (!db.open()) {
            QSqlDatabase::removeDatabase(exportConn);
            return false;
        }
    }

    // Create the full schema in the target
    Database::createAllTables(exportConn);

    // Tables copied in FK-safe order; file and folder last (potentially large)
    const QStringList tables = {
        QStringLiteral("parameter"),
        QStringLiteral("device"),
        QStringLiteral("storage"),
        QStringLiteral("catalog"),
        QStringLiteral("device_mapping"),
        QStringLiteral("catalog_filter"),
        QStringLiteral("tag"),
        QStringLiteral("statistics_device"),
        QStringLiteral("search"),
        QStringLiteral("file"),
        QStringLiteral("folder")
    };

    const int totalTables = tables.size();
    bool success = true;

    for (int idx = 0; idx < totalTables; ++idx) {
        const QString &table = tables.at(idx);

        if (progressCallback && !progressCallback(idx + 1, totalTables, table)) {
            success = false;
            break;
        }

        QSqlQuery srcQ(QSqlDatabase::database(m_connectionName));
        srcQ.setForwardOnly(true);
        if (!srcQ.exec(QStringLiteral("SELECT * FROM ") + table))
            continue; // table absent in older schemas — skip silently

        if (!srcQ.next())
            continue; // empty table

        // Build INSERT from the first record's column names
        const QSqlRecord rec = srcQ.record();
        QStringList cols;
        QStringList placeholders;
        for (int i = 0; i < rec.count(); ++i) {
            cols << rec.fieldName(i);
            placeholders << QStringLiteral("?");
        }

        const QString insertSQL =
            QStringLiteral("INSERT OR IGNORE INTO %1 (%2) VALUES (%3)")
                .arg(table, cols.join(QLatin1Char(',')), placeholders.join(QLatin1Char(',')));

        QSqlDatabase targetDb = QSqlDatabase::database(exportConn);
        targetDb.transaction();
        QSqlQuery ins(targetDb);
        ins.prepare(insertSQL);

        do {
            for (int i = 0; i < rec.count(); ++i)
                ins.bindValue(i, srcQ.value(i));
            ins.exec();
        } while (srcQ.next());

        targetDb.commit();
    }

    QSqlDatabase::database(exportConn).close();
    QSqlDatabase::removeDatabase(exportConn);

    if (!success) {
        QFile(filePath).remove();
        return false;
    }
    return true;
}
//----------------------------------------------------------------------
bool Collection::exportAllCatalogFiles(const QString &outputFolder,
                                        std::function<bool(int current, int total, const QString &catalogName)> progressCallback)
{
    // Get all catalog devices
    QSqlQuery catalogDevicesQuery(QSqlDatabase::database(m_connectionName));
    QString catalogDevicesQuerySQL = QLatin1String(R"(
        SELECT d.device_id, d.device_name, d.device_path, d.device_total_file_count,
               d.device_total_file_size, d.device_external_id
        FROM device d
        WHERE d.device_type = 'Catalog'
        ORDER BY d.device_id
    )");

    if (!catalogDevicesQuery.exec(catalogDevicesQuerySQL)) {
        qWarning() << "WARNING: Catalog devices query failed:" << catalogDevicesQuery.lastError().text();
        return false;
    }

    // Count total catalogs
    QSqlQuery countQuery(QSqlDatabase::database(m_connectionName));
    countQuery.prepare("SELECT COUNT(*) FROM device WHERE device_type = 'Catalog'");
    if (!countQuery.exec() || !countQuery.next()) {
        qWarning() << "WARNING: Count query failed:" << countQuery.lastError().text();
        return false;
    }

    int totalCatalogs = countQuery.value(0).toInt();
    if (totalCatalogs == 0)
        return true;

    // Process each catalog device
    int currentCatalog = 0;
    while (catalogDevicesQuery.next()) {
        currentCatalog++;

        QString deviceName = catalogDevicesQuery.value(1).toString();
        QString devicePath = catalogDevicesQuery.value(2).toString();
        qint64 fileCount = catalogDevicesQuery.value(3).toLongLong();
        qint64 totalFileSize = catalogDevicesQuery.value(4).toLongLong();
        int catalogId = catalogDevicesQuery.value(5).toInt();

        // Notify progress
        if (progressCallback) {
            if (!progressCallback(currentCatalog, totalCatalogs, deviceName))
                return false; // Cancelled
        }

        // Get additional catalog metadata
        QSqlQuery catalogMetaQuery(QSqlDatabase::database(m_connectionName));
        QString catalogMetaQuerySQL = QLatin1String(R"(
            SELECT catalog_include_hidden, catalog_file_type, catalog_storage,
                   catalog_include_symblinks, catalog_is_full_device,
                   catalog_include_metadata, catalog_include_checksum, catalog_app_version
            FROM catalog
            WHERE catalog_id = :catalog_id
        )");
        catalogMetaQuery.prepare(catalogMetaQuerySQL);
        catalogMetaQuery.bindValue(":catalog_id", catalogId);

        if (!catalogMetaQuery.exec() || !catalogMetaQuery.next()) {
            qWarning() << "WARNING: Catalog metadata query failed for catalog" << catalogId << ":" << catalogMetaQuery.lastError().text();
            continue;
        }

        QString includeHidden = catalogMetaQuery.value(0).toString();
        QString fileType = catalogMetaQuery.value(1).toString();
        QString storageName = catalogMetaQuery.value(2).toString();
        QString includeSymblinks = catalogMetaQuery.value(3).toString();
        QString isFullDevice = catalogMetaQuery.value(4).toString();
        QString includeMetadata = catalogMetaQuery.value(5).toString();
        QString includeChecksum = catalogMetaQuery.value(6).toString();
        QString appVersion = catalogMetaQuery.value(7).toString();

        // Create the idx file
        QString idxFilePath = outputFolder + "/" + deviceName + ".idx";
        QFile idxFile(idxFilePath);

        if (!idxFile.open(QIODevice::WriteOnly | QIODevice::Text)) {
            qWarning() << "WARNING: Cannot open idx file for writing:" << idxFilePath;
            continue;
        }

        QTextStream idxStream(&idxFile);

        // Write catalog headers
        idxStream << "<catalogSourcePath>" << devicePath << "\n";
        idxStream << "<catalogFileCount>" << QString::number(fileCount) << "\n";
        idxStream << "<catalogTotalFileSize>" << QString::number(totalFileSize) << "\n";
        idxStream << "<catalogIncludeHidden>" << includeHidden << "\n";
        idxStream << "<catalogFileType>" << fileType << "\n";
        idxStream << "<catalogStorage>" << storageName << "\n";
        idxStream << "<catalogIncludeSymblinks>" << includeSymblinks << "\n";
        idxStream << "<catalogIsFullDevice>" << isFullDevice << "\n";
        idxStream << "<catalogIncludeMetadata>" << includeMetadata << "\n";
        idxStream << "<catalogIncludeChecksum>" << includeChecksum << "\n";
        idxStream << "<catalogAppVersion>" << appVersion << "\n";
        idxStream << "<catalogID>" << QString::number(catalogId) << "\n";

        // Get all files for this catalog
        QSqlQuery fileQuery(QSqlDatabase::database(m_connectionName));
        QString fileQuerySQL = QLatin1String(R"(
            SELECT file_full_path,
                   file_size,
                   file_date_updated,
                   file_extension,
                   file_type,
                   mime_type,
                   mime_verified,
                   type_mismatch,
                   image_width,
                   image_height,
                   image_orientation,
                   video_duration_seconds,
                   video_width,
                   video_height,
                   video_codec,
                   video_framerate,
                   video_bitrate,
                   audio_duration_seconds,
                   audio_artist,
                   audio_album,
                   audio_title,
                   audio_genre,
                   audio_year,
                   audio_track_number,
                   audio_bitrate,
                   audio_sample_rate,
                   metadata_extended,
                   metadata_extraction_date
            FROM file
            WHERE file_catalog_id = :catalog_id
            ORDER BY file_full_path
        )");
        fileQuery.prepare(fileQuerySQL);
        fileQuery.bindValue(":catalog_id", catalogId);

        if (!fileQuery.exec()) {
            qWarning() << "WARNING: File query failed for catalog" << catalogId << ":" << fileQuery.lastError().text();
            idxFile.close();
            continue;
        }

        // Write all file entries
        while (fileQuery.next()) {
            for (int i = 0; i < 28; ++i) {
                if (i > 0)
                    idxStream << "\t";
                idxStream << fileQuery.value(i).toString();
            }
            idxStream << "\n";
        }

        idxFile.close();

        // Export folders file
        exportSingleCatalogFoldersFile(catalogId, outputFolder + "/" + deviceName + ".folders.idx");
    }

    return true;
}
//----------------------------------------------------------------------
bool Collection::exportSingleCatalogFoldersFile(int catalogId, const QString &filePath)
{
    QSqlQuery checkQuery(QSqlDatabase::database(m_connectionName));
    checkQuery.prepare("SELECT COUNT(*) FROM folder WHERE folder_catalog_id = :catalog_id");
    checkQuery.bindValue(":catalog_id", catalogId);

    if (!checkQuery.exec() || !checkQuery.next()) {
        qWarning() << "WARNING: Folder count query failed:" << checkQuery.lastError().text();
        return false;
    }

    int folderCount = checkQuery.value(0).toInt();
    if (folderCount == 0)
        return true;

    QSqlQuery folderQuery(QSqlDatabase::database(m_connectionName));
    folderQuery.prepare("SELECT folder_catalog_id, folder_path FROM folder WHERE folder_catalog_id = :catalog_id ORDER BY folder_path");
    folderQuery.bindValue(":catalog_id", catalogId);

    if (!folderQuery.exec()) {
        qWarning() << "WARNING: Folder query failed:" << folderQuery.lastError().text();
        return false;
    }

    QFile foldersFile(filePath);
    if (!foldersFile.open(QIODevice::WriteOnly | QIODevice::Text)) {
        qWarning() << "WARNING: Cannot open folders file for writing:" << filePath;
        return false;
    }

    QTextStream foldersStream(&foldersFile);
    while (folderQuery.next()) {
        foldersStream << folderQuery.value(0).toString() << "\t"
                      << folderQuery.value(1).toString() << "\n";
    }

    foldersFile.close();
    return true;
}
//----------------------------------------------------------------------

//Data management ------------------------------------------------------
bool Collection::insertPhysicalStorageGroup() {
    //Add the default Physical Group and a Virtual sub-device
    QSqlQuery query(QSqlDatabase::database(m_connectionName));
    QString querySQL;

    querySQL = QLatin1String(R"(
                            SELECT COUNT(*)
                            FROM device
                            WHERE device_id = 1
                        )");
    query.prepare(querySQL);
    query.exec();
    query.next();

    bool defaultsCreated = false;
    int result = query.value(0).toInt();
    if(result == 0){
        Device *newDeviceItem1 = new Device();
        newDeviceItem1->ID = 1;
        newDeviceItem1->parentID = 0;
        newDeviceItem1->name = " Physical Group";
        newDeviceItem1->type = "Virtual";
        newDeviceItem1->externalID = 0;
        newDeviceItem1->groupID = 0;
        newDeviceItem1->insertDevice();

        Device *newDeviceItem2 = new Device();
        newDeviceItem2->ID = 2;
        newDeviceItem2->parentID = 1;
        newDeviceItem2->name = "Virtual device";
        newDeviceItem2->type = "Virtual";
        newDeviceItem2->externalID = 0;
        newDeviceItem2->groupID = 0;
        newDeviceItem2->insertDevice();

        defaultsCreated = true;
    }

    saveDeviceTableToFile();

    //Add a default storage device, to force any new catalog to have one
    QSqlQuery queryStorage(QSqlDatabase::database(m_connectionName));
    QString queryStorageSQL = QLatin1String(R"(
                                    SELECT COUNT(*)
                                    FROM device
                                    WHERE device_type='Storage'
                                )");
    queryStorage.prepare(queryStorageSQL);
    queryStorage.exec();
    queryStorage.next();

    if (queryStorage.value(0).toInt() == 0){
        //Create Device and related Storage under Physical group (ID=0)
        Device *newStorageDevice = new Device();
        newStorageDevice->generateDeviceID();
        newStorageDevice->parentID = 2;
        if(newStorageDevice->verifyParentDeviceExistsInPhysicalGroup()==true)
            newStorageDevice->parentID = 1;

        newStorageDevice->name = "Local disk";
        newStorageDevice->type = "Storage";
        newStorageDevice->path = "/";
        #ifdef Q_OS_WINDOWS
        newStorageDevice->path = "C:";
        #endif
        newStorageDevice->storage->path = newStorageDevice->path;
        newStorageDevice->storage->generateID();
        newStorageDevice->externalID = newStorageDevice->storage->ID;
        newStorageDevice->groupID = 0;
        newStorageDevice->insertDevice();
        newStorageDevice->storage->name = newStorageDevice->name;
        newStorageDevice->storage->insertStorage();
        newStorageDevice->saveDevice();
        newStorageDevice->updateStorageOnly("create");

        //Save data to file
        saveDeviceTableToFile();
        saveStorageTableToFile();
    }
    return defaultsCreated;
}
//----------------------------------------------------------------------
void Collection::updateAllDeviceActive()
{//Update the value Active for all Devices

    //For Storage and Catalog devices
    //Get the list of devices
    QSqlQuery query(QSqlDatabase::database(m_connectionName));
    QString querySQL = QLatin1String(R"(
                                            SELECT device_id
                                            FROM   device
                                            WHERE  device_type = 'Storage' OR device_type = 'Catalog'
                                    )");
    query.prepare(querySQL);
    query.exec();

    //Update and Save sourcePathIsActive for each catalog
    Device loopDevice;
    while (query.next()){
        loopDevice.ID = query.value(0).toInt();
        loopDevice.loadDevice(m_connectionName);
        loopDevice.updateActiveState(m_connectionName);
    }
}
//----------------------------------------------------------------------
Collection::CollectionFolderStatus Collection::validateCollectionFolder(const QString& folderPath, const QString& targetMode) const
{
    QDir dir(folderPath);
    if (!dir.exists()) {
        return INVALID_USER_DATA; // Caller should handle non-existent dirs
    }

    // Check if empty
    QStringList entries = dir.entryList(QDir::NoDotAndDotDot | QDir::AllEntries);
    if (entries.isEmpty()) {
        return VALID_EMPTY;
    }

    // Check for Memory mode indicators (csv)
    bool hasDeviceCsv = QFile::exists(folderPath + "/device.csv");
    bool hasStorageCsv = QFile::exists(folderPath + "/storage.csv");
    bool hasParametersCsv = QFile::exists(folderPath + "/parameters.csv");
    bool hasMemoryModeFiles = hasDeviceCsv || hasStorageCsv || hasParametersCsv;

    // Check for File mode indicators
    QStringList dbFiles = dir.entryList(QStringList() << "*.db", QDir::Files);
    bool hasDbFiles = !dbFiles.isEmpty();

    // Determine folder content type
    bool isMemoryCollection = hasMemoryModeFiles;
    bool isFileCollection = hasDbFiles;
    bool hasUserData = !isMemoryCollection && !isFileCollection && !entries.isEmpty();

    // Validate against target mode
    if (targetMode == "Memory") {
        if (isMemoryCollection) return VALID_MEMORY_MODE;
        if (isFileCollection) return INVALID_FILE_FILES;
        if (hasUserData) return INVALID_USER_DATA;
    }
    else if (targetMode == "File") {
        if (isFileCollection) return VALID_FILE_MODE;
        if (isMemoryCollection) return INVALID_MEMORY_FILES;
        if (hasUserData) return INVALID_USER_DATA;
    }

    return INVALID_USER_DATA;
}
//----------------------------------------------------------------------
QString Collection::getValidationMessage(CollectionFolderStatus status) const
{
    switch (status) {
    case VALID_EMPTY:
        return ""; //tr("This folder is empty and can be used for a new collection.");

    case VALID_MEMORY_MODE:
        return ""; //tr("This folder contains a valid Memory mode collection.");

    case VALID_FILE_MODE:
        return ""; //tr("This folder contains File mode collection auxiliary files.");

    case INVALID_MEMORY_FILES:
        return tr("This folder contains Memory mode collection files, but you are currently in File mode.<br/>"
                  "Switch to Memory mode or select a different folder.");

    case INVALID_FILE_FILES:
        return tr("This folder contains File mode collection files, but you are currently in Memory mode.<br/>"
                  "Switch to File mode or select a different folder.");

    // case INVALID_MIXED_DATA:
    //     return tr("This folder contains both collection and user data.<br/>"
    //               "To avoid mixing data types, please select a dedicated folder for collections.");

    case INVALID_USER_DATA:
    default:
        return tr("This folder contains user data and is not suitable for a collection.<br/>"
                  "Collections should be stored in dedicated folders to avoid mixing with personal files.");
    }
}
//----------------------------------------------------------------------
