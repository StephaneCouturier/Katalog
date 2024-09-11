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
#include "device.h"

//Main attributes--------------------------------------------------------
void Collection::updateCollectionVersion()
{
    QSqlQuery queryUpdateVersion;
    QString queryUpdateVersionSQL = QLatin1String(R"(
                                    UPDATE parameter
                                    SET parameter_value1 = :parameter_value1
                                    WHERE parameter_type =:parameter_type
                                    AND parameter_name =:parameter_name
                                )");
    queryUpdateVersion.prepare(queryUpdateVersionSQL);
    queryUpdateVersion.bindValue(":parameter_name", "version");
    queryUpdateVersion.bindValue(":parameter_type", "collection");
    queryUpdateVersion.bindValue(":parameter_value1", version);
    queryUpdateVersion.exec();
}

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
                qDebug() << "DEBUG: Failed to create Device file:" << deviceFile.errorString();
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
                qDebug() << "DEBUG: Failed to create Parameters file:" << parametersFile.errorString();
            }

            //Add the current version
            QSqlQuery insertQuery(QSqlDatabase::database("defaultConnection"));
            QString insertSQL = QLatin1String(R"(
                                        INSERT INTO parameter (
                                                    parameter_name,
                                                    parameter_type,
                                                    parameter_value1)
                                        VALUES(
                                                    :parameter_name,
                                                    :parameter_type,
                                                    :parameter_value1)
                                )");
            insertQuery.prepare(insertSQL);
            insertQuery.bindValue(":parameter_name", "version");
            insertQuery.bindValue(":parameter_type", "collection");
            insertQuery.bindValue(":parameter_value1", version);
            insertQuery.exec();

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

                //Even if empty, load it to the model
                loadStorageFileToTable();

                return;
            }
        }
    }
}

//File loading-----------------------------------------------------------
void Collection::load()
{//Load collection
    //Reset key values and clear database in "Memory" mode
    version ="";
    clearDatabaseData();

    //Check if new collection (the folder would be empty)
    QDir dir(folder);
    dir.setFilter(QDir::AllEntries | QDir::NoDotAndDotDot);
    if(dir.entryList().isEmpty()){
        version = appVersion;
        updateCollectionVersion();
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

    //Add a default storage device, to force any new catalog to have one
    insertPhysicalStorageGroup();
}
//----------------------------------------------------------------------
void Collection::clearDatabaseData()
{   //Clear database date in the context of Memory mode, prior to reloading files to tables
    if(databaseMode=="Memory"){
        QSqlQuery queryDelete(QSqlDatabase::database("defaultConnection"));
        queryDelete.exec("DELETE FROM device");
        queryDelete.exec("DELETE FROM catalog");
        queryDelete.exec("DELETE FROM storage");
        queryDelete.exec("DELETE FROM file");
        queryDelete.exec("DELETE FROM filetemp");
        queryDelete.exec("DELETE FROM folder");
        queryDelete.exec("DELETE FROM metadata");
        queryDelete.exec("DELETE FROM statistics_device");
        queryDelete.exec("DELETE FROM search");
        queryDelete.exec("DELETE FROM tag");
        queryDelete.exec("DELETE FROM parameter");

        //MIGRATION 1.22 to 2.0
        queryDelete.exec("DELETE FROM statistics_catalog");
        queryDelete.exec("DELETE FROM statistics_storage");
        queryDelete.exec("DELETE FROM virtual_storage");
        queryDelete.exec("DELETE FROM virtual_storage_catalog");
        queryDelete.exec("DELETE FROM device_catalog");
    }
}
//----------------------------------------------------------------------
void Collection::loadAllCatalogFiles()
{//Load all catalog files to memory
    if(databaseMode=="Memory"){
        QSqlQuery queryLoadAllCatalogFiles(QSqlDatabase::database("defaultConnection"));
        QString queryLoadAllCatalogFilesSQL = QLatin1String(R"(
                                        SELECT device_id
                                        FROM device
                                        WHERE device_type ='Catalog'
                                    )");
        queryLoadAllCatalogFiles.prepare(queryLoadAllCatalogFilesSQL);
        queryLoadAllCatalogFiles.exec();
        while(queryLoadAllCatalogFiles.next()){
            Device tempDevice;
            tempDevice.ID = queryLoadAllCatalogFiles.value(0).toInt();
            tempDevice.loadDevice("defaultConnection");
            QMutex tempMutex;
            bool tempStopRequested = false;
            tempDevice.catalog->loadCatalogFileListToTable("defaultConnection", tempMutex, tempStopRequested);
        }
    }
}
//----------------------------------------------------------------------
void Collection::loadDeviceFileToTable()
{
    if(databaseMode=="Memory"){
        //Clear table
        QSqlQuery query(QSqlDatabase::database("defaultConnection"));
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
                    QSqlQuery insertQuery(QSqlDatabase::database("defaultConnection"));
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
                                        :device_group_id,
                                        :device_date_updated,
                                        :device_order )
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
                        insertQuery.bindValue(":device_date_updated",fieldList[12]);
                        insertQuery.bindValue(":device_order",fieldList[13]);
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
        QSqlQuery queryDelete(QSqlDatabase::database("defaultConnection"));
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

                Catalog newCatalog;
                newCatalog.ID               = catalogValues[10].toInt(); //catalog_id
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
                newCatalog.includeMetadata  = catalogValues[8].compare("true", Qt::CaseInsensitive) == 0; //catalog_include_metadata
                newCatalog.appVersion       = catalogValues[9]; //catalog_app_version

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

        QSqlQuery queryDelete(QSqlDatabase::database("defaultConnection"));
        queryDelete.prepare( "DELETE FROM storage" );

        //Open file or return information
        if(!storageFile.open(QIODevice::ReadOnly)) {
            queryDelete.exec();
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
                                        storage_brand,
                                        storage_model,
                                        storage_serial_number,
                                        storage_build_date,
                                        storage_comment1,
                                        storage_comment2,
                                        storage_comment3)
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
                                        :storage_comment3)
                                )");

                    QSqlQuery insertQuery(QSqlDatabase::database("defaultConnection"));
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
        QSqlQuery deleteQuery(QSqlDatabase::database("defaultConnection"));
        deleteQuery.exec("DELETE FROM statistics_device");

        //Get infos stored in the file
        QFile statisticsDeviceFile(statisticsDeviceFilePath);
        if(!statisticsDeviceFile.open(QIODevice::ReadOnly)) {
            return;
        }

        QTextStream textStream(&statisticsDeviceFile);

        //Prepare query to load file info
        QSqlQuery insertQuery(QSqlDatabase::database("defaultConnection"));
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
void Collection::loadParameterFileToTable()
{// Load the contents of the storage statistics file into the database
    if(databaseMode=="Memory"){
        //Clear database table
        QSqlQuery deleteQuery(QSqlDatabase::database("defaultConnection"));
        deleteQuery.exec("DELETE FROM parameters");

        //Get data stored in the file
        QFile parametersFile(parameterFilePath);
        if(parametersFile.open(QIODevice::ReadOnly)) {
            QTextStream textStream(&parametersFile);

            //Prepare query to load file info
            QSqlQuery insertQuery(QSqlDatabase::database("defaultConnection"));
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
                    insertQuery.bindValue(":parameter_value2", fieldList[3]);
                    insertQuery.exec();
                }
            }
        }
        else{
            qDebug() << "DEBUG: Could not open parameters.csv:" << parametersFile.errorString();
        }
    }

    //Get collection version
    QSqlQuery queryVersion(QSqlDatabase::database("defaultConnection"));
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
        version = queryVersion.value(0).toString();
    }
}
//----------------------------------------------------------------------
void Collection::loadSearchHistoryFileToTable()
{
    if(databaseMode=="Memory"){

        //Define storage file and prepare stream
        QFile searchFile(searchHistoryFilePath);
        QTextStream textStream(&searchFile);

        QSqlQuery queryDelete(QSqlDatabase::database("defaultConnection"));
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
                        int  targetFieldsCount = 37;
                        int currentFiledsCount = fieldList.count();
                        int    diffFieldsCount = targetFieldsCount - currentFiledsCount;
                        if(diffFieldsCount !=0){
                            for(int i=0; i<diffFieldsCount; i++){
                                fieldList.append("");
                            }
                        }

                        QSqlQuery insertQuery(QSqlDatabase::database("defaultConnection"));
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
                                                    folder_criteria_checked
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
                                                    :folder_criteria_checked
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
                        insertQuery.exec();
                    }
            }
            searchFile.close();
        }
    }
}
//--------------------------------------------------------------------------
void Collection::loadTagFileToTable()
{
    if(databaseMode=="Memory"){

        //Define storage file and prepare stream
        QFile tagFile(tagFilePath);
        QTextStream textStream(&tagFile);

        QSqlQuery queryDelete(QSqlDatabase::database("defaultConnection"));
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
                    QSqlQuery insertQuery(QSqlDatabase::database("defaultConnection"));
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
//--------------------------------------------------------------------------
//File saving ----------------------------------------------------------
void Collection::saveDeviceTableToFile()
{
    if (databaseMode == "Memory"){
        QFile deviceFile(deviceFilePath);

        //Get data
        QSqlQuery query(QSqlDatabase::database("defaultConnection"));
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
                             << "order"    << "\t"
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
            msgBox.setText("DEBUG: Error opening output file:<br/>" + deviceFilePath);
            msgBox.setIcon(QMessageBox::Warning);
            msgBox.exec();
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
            << '\n';

        //Get data
        QSqlQuery query(QSqlDatabase::database("defaultConnection"));
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
                            storage_comment3
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
        QSqlQuery query(QSqlDatabase::database("defaultConnection"));
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
        QSqlQuery query(QSqlDatabase::database("defaultConnection"));
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
{
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
                << '\n';

            //Get data
            QSqlQuery query(QSqlDatabase::database("defaultConnection"));
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
                                            folder_criteria_checked
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
            //searchFile.close();
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
        QSqlQuery query(QSqlDatabase::database("defaultConnection"));
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

//File deleting---------------------------------------------------------
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
        QSqlQuery queryDelete(QSqlDatabase::database("defaultConnection"));
        queryDelete.exec("DELETE FROM catalog");

        //refresh catalog lists
        loadCatalogFilesToTable();
    }

}
//----------------------------------------------------------------------

//Data management ------------------------------------------------------
void Collection::insertPhysicalStorageGroup() {
    //Add the default Physical Group and a Virtual sub-device
    QSqlQuery query(QSqlDatabase::database("defaultConnection"));
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
        newDeviceItem2->name = QCoreApplication::translate("MainWindow", "Virtual device");
        newDeviceItem2->type = "Virtual";
        newDeviceItem2->externalID = 0;
        newDeviceItem2->groupID = 0;
        newDeviceItem2->insertDevice();
    }

    saveDeviceTableToFile();

    //Add a default storage device, to force any new catalog to have one
    QSqlQuery queryStorage(QSqlDatabase::database("defaultConnection"));
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

        newStorageDevice->name = QCoreApplication::translate("MainWindow", "Local disk");
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
        newStorageDevice->updateDevice("create",
                                       databaseMode,
                                       false,
                                       folder,
                                       false);

        //Save data to file
        saveDeviceTableToFile();
        saveStorageTableToFile();
    }
}
//----------------------------------------------------------------------
