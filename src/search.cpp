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
// File Name:   search.cpp
// Purpose:     class to manage search criteria and results
// Description:
// Author:      Stephane Couturier
/////////////////////////////////////////////////////////////////////////////
*/

#include "search.h"
#include <QSqlError>

Search::Search(QObject *parent) : QAbstractTableModel(parent)
{

}

//file list model
int Search::rowCount(const QModelIndex &parent) const
{
    Q_UNUSED(parent);
    return fileNames.length();
}

int Search::columnCount(const QModelIndex &parent) const
{
    Q_UNUSED(parent);
    return 6;
}

QVariant Search::data(const QModelIndex &index, int role) const
{
    if (!index.isValid() || role != Qt::DisplayRole) {
        return QVariant();
    }
    switch (index.column()){
        case 0: return QString(fileNames[index.row()]);
        case 1: return qint64 (fileSizes[index.row()]);
        case 2: return QString(fileDateTimes[index.row()]);
        case 3: return QString(filePaths[index.row()]);
        case 4: return QString(fileCatalogs[index.row()]);
        case 5: return int(fileCatalogIDs[index.row()]);
    }
    return QVariant();
}

QVariant Search::headerData(int section, Qt::Orientation orientation, int role) const
{
    if (role == Qt::DisplayRole && orientation == Qt::Horizontal) {
        switch (section){
            case 0: return QString(tr("Name"));
            case 1: return QString(tr("Size"));
            case 2: return QString(tr("Date"));
            case 3: return QString(tr("Folder"));
            case 4: return QString(tr("Catalog Name"));
            case 5: return QString(tr("Catalog ID"));
        }
    }
    return QVariant();
}

void Search::searchFiles(Device *selectedDevice)
{
    //Run a search of files in each selected catalog based on user inputs

    //Process the SEARCH in CATALOGS or DIRECTORY ------------------------------
    //Process the SEARCH in CATALOGS
    if (searchInCatalogsChecked == true){

        //For differences, only process the 2 selected catalogs

        if (searchOnDifferences == true){
            //Load diffDevice1 files
            diffDevice1->loadDevice("defaultConnection");

            if(diffDevice1->type == "Catalog") {
                searchFilesInCatalog(diffDevice1);
            }
            else{
                foreach (const Device::deviceListRow &row, diffDevice1->deviceListTable) {
                    if(row.type == "Catalog"){
                        Device *device = new Device;
                        device->ID = row.ID;
                        device->loadDevice("defaultConnection");
                        searchFilesInCatalog(device);
                    }
                }
            }

            //Load diffDevice2 files
            diffDevice2->loadDevice("defaultConnection");

            if(diffDevice2->type == "Catalog") {
                searchFilesInCatalog(diffDevice2);
            }
            else{
                foreach (const Device::deviceListRow &row, diffDevice2->deviceListTable) {
                    if(row.type == "Catalog"){
                        Device *device = new Device;
                        device->ID = row.ID;
                        device->loadDevice("defaultConnection");
                        searchFilesInCatalog(device);
                    }
                }
            }
        }

        //Otherwise (not a "difference" search), search in the list of catalogs in the selectedDevice
        else{
            if(selectedDevice->type == "Catalog") {
                searchFilesInCatalog(selectedDevice);
            }
            else{
                foreach (const Device::deviceListRow &row, selectedDevice->deviceListTable) {
                    if(row.type == "Catalog"){
                        Device *device = new Device;
                        device->ID = row.ID;
                        device->loadDevice("defaultConnection");
                        searchFilesInCatalog(device);
                    }
                }
            }
        }
    }

    //Process the SEARCH in SELECTED DIRECTORY
    else if (searchInConnectedChecked == true){
        searchFilesInDirectory(connectedDirectory);
    }

    //Process search results: list of catalogs with results
    //Remove duplicates so the catalogs are listed only once, and sort the list
    deviceFoundIDList.removeDuplicates();
    deviceFoundIDList.sort();

    //Keep the catalog file name only
    foreach(QString item, deviceFoundIDList){
        int index = deviceFoundIDList.indexOf(item);
        QFileInfo fileInfo(item);
        deviceFoundIDList[index] = fileInfo.baseName();
    }

    //Set model headers
    deviceFoundModel->setHorizontalHeaderLabels({ QCoreApplication::translate("MainWindow", "Catalog with results"), QCoreApplication::translate("MainWindow", "ID") });

    Device loopDevice;
    for (const QString &ID : deviceFoundIDList) {
        loopDevice.ID = ID.toInt();
        loopDevice.loadDevice("defaultConnection");
        QList<QStandardItem *> items;
        items << new QStandardItem(loopDevice.name);
        items << new QStandardItem(QString::number(loopDevice.ID));
        deviceFoundModel->appendRow(items);
    }

    //Process search results: list of files

    //Populate model with folders only if this option is selected
    if ( searchOnFolderCriteria==true and showFoldersOnly==true )
    {
        QMap<QString, QPair<QString, int>> uniqueFilePaths;

        for (int i = 0; i < filePaths.size(); ++i) {
            QString filePath = filePaths.at(i);
            QString fileCatalog = fileCatalogs.at(i);
            int fileCatalogID = fileCatalogIDs.at(i);

            if (!uniqueFilePaths.contains(filePath)) {
                uniqueFilePaths.insert(filePath, qMakePair(fileCatalog, fileCatalogID));
            }
        }

        filePaths.clear();
        fileCatalogs.clear();
        fileCatalogIDs.clear();

        for (auto it = uniqueFilePaths.begin(); it != uniqueFilePaths.end(); ++it) {
            filePaths.append(it.key());
            fileCatalogs.append(it.value().first);
            fileCatalogIDs.append(it.value().second);
        }

        fileNames.clear();
        fileSizes.clear();
        fileDateTimes.clear();

        for (int i = 0; i < filePaths.size(); ++i) {
            fileNames.append("");
            fileSizes.append(0);
            fileDateTimes.append("");
        }
    }

    //Process DUPLICATES -------------------------------

    //Process if enabled and criteria are provided
    if ( searchOnFileCriteria == true and searchOnDuplicates == true
        and (   searchDuplicatesOnName == true
             or searchDuplicatesOnSize == true
             or searchDuplicatesOnDate == true )){

            //Load Search results into the database
            //clear database
        QSqlQuery deleteQuery(QSqlDatabase::database("defaultConnection"));
        deleteQuery.exec("DELETE FROM filetemp");

        //prepare query to load file info
        QSqlQuery insertQuery(QSqlDatabase::database("defaultConnection"));
        QString insertSQL = QLatin1String(R"(
                                                        INSERT INTO filetemp (
                                                                        file_name,
                                                                        file_folder_path,
                                                                        file_size,
                                                                        file_date_updated,
                                                                        file_catalog,
                                                                        file_catalog_id )
                                                        VALUES(

                                                                        :file_name,
                                                                        :file_folder_path,
                                                                        :file_size,
                                                                        :file_date_updated,
                                                                        :file_catalog,
                                                                        :file_catalog_id )
                                                                    )");
        insertQuery.prepare(insertSQL);

        //loop through the result list and populate database

        int rows = rowCount();

        for (int i=0; i<rows; i++) {

            //Append data to the database
            insertQuery.bindValue(":file_name",         index(i,0).data().toString());
            insertQuery.bindValue(":file_size",         index(i,1).data().toString());
            insertQuery.bindValue(":file_folder_path",  index(i,3).data().toString());
            insertQuery.bindValue(":file_date_updated", index(i,2).data().toString());
            insertQuery.bindValue(":file_catalog",      index(i,4).data().toString());
            insertQuery.bindValue(":file_catalog_id",   index(i,5).data().toString());
            insertQuery.exec();
        }

        //Prepare duplicate SQL
        // Load all files and create model
        QString selectSQL;

        //Generate grouping of fields based on user selection, determining what are duplicates
        QString groupingFields; // this value should be a concatenation of fields, like "fileName||fileSize"

        //Same name
        if(searchDuplicatesOnName == true){
            groupingFields = groupingFields + "file_name";
        }
        //Same size
        if(searchDuplicatesOnSize == true){
            groupingFields = groupingFields + "||file_size";
        }
        //Same date modified
        if(searchDuplicatesOnDate == true){
            groupingFields = groupingFields + "||file_date_updated";
        }

        //Remove starting || if any
        if (groupingFields.startsWith("||"))
            groupingFields.remove(0, 2);

        //Generate SQL based on grouping of fields
        selectSQL = QLatin1String(R"(
                                                SELECT      file_name,
                                                            file_size,
                                                            file_date_updated,
                                                            file_folder_path,
                                                            file_catalog,
                                                            file_catalog_id
                                                FROM filetemp
                                                WHERE %1 IN
                                                    (SELECT %1
                                                    FROM filetemp
                                                    GROUP BY %1
                                                    HAVING count(%1)>1)
                                                ORDER BY %1
                                            )").arg(groupingFields);

        //Run Query and load to model
        QSqlQuery duplicatesQuery(QSqlDatabase::database("defaultConnection"));
        duplicatesQuery.prepare(selectSQL);
        duplicatesQuery.exec();

        //recapture file results for Stats
        fileNames.clear();
        fileSizes.clear();
        filePaths.clear();
        fileDateTimes.clear();
        fileCatalogs.clear();
        fileCatalogIDs.clear();

        while(duplicatesQuery.next()){
            fileNames.append(duplicatesQuery.value(0).toString());
            fileSizes.append(duplicatesQuery.value(1).toLongLong());
            fileDateTimes.append(duplicatesQuery.value(2).toString());
            filePaths.append(duplicatesQuery.value(3).toString());
            fileCatalogs.append(duplicatesQuery.value(4).toString());
            fileCatalogIDs.append(duplicatesQuery.value(5).toInt());
        }
    }

    //Process DIFFERENCES -------------------------------

    //Process if enabled and criteria are provided
    if ( searchOnFileCriteria == true and searchOnDifferences == true
        and (   differencesOnName == true
             or differencesOnSize == true
             or differencesOnDate == true)){

            //Load Search results into the database
            //Clear database
        QSqlQuery deleteQuery(QSqlDatabase::database("defaultConnection"));
        deleteQuery.exec("DELETE FROM filetemp");

        //Prepare query to load file info
        QSqlQuery insertQuery(QSqlDatabase::database("defaultConnection"));
        QString insertSQL = QLatin1String(R"(
                                                        INSERT INTO filetemp (
                                                                        file_name,
                                                                        file_folder_path,
                                                                        file_size,
                                                                        file_date_updated,
                                                                        file_catalog,
                                                                        file_catalog_id )
                                                        VALUES(

                                                                        :file_name,
                                                                        :file_folder_path,
                                                                        :file_size,
                                                                        :file_date_updated,
                                                                        :file_catalog,
                                                                        :file_catalog_id )
                                                                    )");
        insertQuery.prepare(insertSQL);

        //Loop through the result list and populate database

        int rows = rowCount();
        for (int i=0; i<rows; i++) {
            //Append data to the database
            insertQuery.bindValue(":file_name",         index(i,0).data().toString());
            insertQuery.bindValue(":file_size",         index(i,1).data().toString());
            insertQuery.bindValue(":file_folder_path",  index(i,2).data().toString());
            insertQuery.bindValue(":file_date_updated", index(i,3).data().toString());
            insertQuery.bindValue(":file_catalog",      index(i,4).data().toString());
            insertQuery.bindValue(":file_catalog_id",   index(i,5).data().toString());
            insertQuery.exec();
        }

        //Prepare difference SQL
        // Load all files and create model
        QString selectSQL;

        //Generate grouping of fields based on user selection, determining what are duplicates
        QString groupingFieldsDifferences; // this value should be a concatenation of fields, like "fileName||fileSize"

        //Same name
        if(differencesOnName == true){
            groupingFieldsDifferences += "||file_name";
        }
        //Same size
        if(differencesOnSize == true){
            groupingFieldsDifferences += "||file_size";
        }
        //Same date modified
        if(differencesOnDate == true){
            groupingFieldsDifferences += "||file_date_updated";
        }

        //Remove the || at the start
        if (groupingFieldsDifferences.startsWith("||"))
            groupingFieldsDifferences.remove(0, 2);

        // Populate listOfCatalogDeviceIDs1
        QString listOfCatalogDeviceIDs1;
        Device *diffDevice1 = new Device;
        diffDevice1->ID = differencesDeviceID1;
        diffDevice1->loadDevice("defaultConnection");
        if(diffDevice1->type =="Catalog") {
            listOfCatalogDeviceIDs1 = listOfCatalogDeviceIDs1 + QString::number(diffDevice1->ID) + ",";
        } else {
            for (const auto& row : diffDevice1->deviceListTable) {
                if (row.type == "Catalog") {
                    listOfCatalogDeviceIDs1 = listOfCatalogDeviceIDs1 + QString::number(row.ID) + ",";
                }
            }
        }
        if (listOfCatalogDeviceIDs1.endsWith(","))
            listOfCatalogDeviceIDs1.remove(listOfCatalogDeviceIDs1.length()-1, 1);

        // Populate listOfCatalogDeviceIDs2
        QString listOfCatalogDeviceIDs2;
        Device *diffDevice2 = new Device;
        diffDevice2->ID = differencesDeviceID2;
        diffDevice2->loadDevice("defaultConnection");
        if(diffDevice2->type =="Catalog") {
            listOfCatalogDeviceIDs2 = listOfCatalogDeviceIDs2 + QString::number(diffDevice2->ID) + ",";
        } else {
            for (const auto& row : diffDevice2->deviceListTable) {
                if (row.type == "Catalog") {
                    listOfCatalogDeviceIDs2 = listOfCatalogDeviceIDs2 + QString::number(row.ID) + ",";
                }
            }
        }
        if (listOfCatalogDeviceIDs2.endsWith(","))
            listOfCatalogDeviceIDs2.remove(listOfCatalogDeviceIDs2.length()-1, 1);

        //Generate SQL based on grouping of fields
        selectSQL = QString(R"(
                                                    SELECT      file_name,
                                                                file_size,
                                                                file_date_updated,
                                                                file_folder_path,
                                                                file_catalog,
                                                                file_catalog_id
                                                    FROM filetemp
                                                    WHERE file_catalog_id IN(
                                                            SELECT device_external_id
                                                            FROM device
                                                            WHERE device_id IN(%2)
                                                            AND device_type ='Catalog'
                                                    )
                                                    AND %1 NOT IN(
                                                        SELECT %1
                                                        FROM filetemp
                                                        WHERE file_catalog_id IN(
                                                            SELECT device_external_id
                                                            FROM device
                                                            WHERE device_id IN(%3)
                                                            AND device_type ='Catalog'
                                                        )
                                                    )
                                                    UNION
                                                    SELECT      file_name,
                                                                file_size,
                                                                file_date_updated,
                                                                file_folder_path,
                                                                file_catalog,
                                                                file_catalog_id
                                                    FROM filetemp
                                                    WHERE file_catalog_id IN(
                                                            SELECT device_external_id
                                                            FROM device
                                                            WHERE device_id IN(%3)
                                                            AND device_type ='Catalog'
                                                    )
                                                    AND %1 NOT IN(
                                                        SELECT %1
                                                        FROM filetemp
                                                        WHERE file_catalog_id IN(
                                                            SELECT device_external_id
                                                            FROM device
                                                            WHERE device_id IN(%2)
                                                            AND device_type ='Catalog'
                                                        )
                                                    )
                                                )").arg(groupingFieldsDifferences,
                             listOfCatalogDeviceIDs1,
                             listOfCatalogDeviceIDs2);

        //Prepare the query
        QSqlQuery differencesQuery(QSqlDatabase::database("defaultConnection"));
        differencesQuery.prepare(selectSQL);

        //Execute the query
        if (!differencesQuery.exec())
            qDebug() << "DEBUG: differencesQuery failed:" << differencesQuery.lastError();

        //Recapture file results for Stats
        fileNames.clear();
        fileSizes.clear();
        filePaths.clear();
        fileDateTimes.clear();
        fileCatalogs.clear();
        while(differencesQuery.next()){
            fileNames.append(differencesQuery.value(0).toString());
            fileSizes.append(differencesQuery.value(1).toLongLong());
            fileDateTimes.append(differencesQuery.value(2).toString());
            filePaths.append(differencesQuery.value(3).toString());
            fileCatalogs.append(differencesQuery.value(4).toString());
            fileCatalogIDs.append(differencesQuery.value(5).toInt());
        }
    }

    //Files found Statistics
    //Reset from previous search
    filesFoundNumber = 0;
    filesFoundTotalSize = 0;
    filesFoundAverageSize = 0;
    filesFoundMinSize = 0;
    filesFoundMaxSize = 0;
    filesFoundMinDate = "";
    filesFoundMaxDate = "";

    //Number of files found
    filesFoundNumber = fileNames.count();

    //Total size of files found
    qint64 sizeItem;
    filesFoundTotalSize = 0;
    foreach (sizeItem, fileSizes) {
        filesFoundTotalSize = filesFoundTotalSize + sizeItem;
    }

    //Other statistics, covering the case where no results are returned.
    if (filesFoundNumber !=0){
        filesFoundAverageSize = filesFoundTotalSize / filesFoundNumber;
        QList<qint64> fileSizeList = fileSizes;
        std::sort(fileSizeList.begin(), fileSizeList.end());
        filesFoundMinSize = fileSizeList.first();
        filesFoundMaxSize = fileSizeList.last();

        QList<QString> fileDateList = fileDateTimes;
        std::sort(fileDateList.begin(), fileDateList.end());
        filesFoundMinDate = fileDateList.first();
        filesFoundMaxDate = fileDateList.last();
    }

    //Save the search criteria to the search history
    insertSearchHistoryToTable("defaultConnection");

}
//-----------------

void Search::searchFilesInCatalog(Device *device)
{
    //Run a search of files for the selected Catalog
    //Prepare Inputs including Regular Expression
    QFile catalogFile(device->catalog->sourcePath);

    QRegularExpressionMatch match;
    QRegularExpressionMatch foldermatch;

    //Define how to use the search text
    if(selectedTextCriteria == QCoreApplication::translate("MainWindow", "Exact Phrase"))
        regexSearchtext=searchText; //just search for the extact text entered including spaces, as one text string.
    else if(selectedTextCriteria == QCoreApplication::translate("MainWindow", "Begins With"))
        regexSearchtext="(^"+searchText+")";
    else if(selectedTextCriteria == QCoreApplication::translate("MainWindow", "Any Word"))
        regexSearchtext=searchText.replace(" ","|");
    else if(selectedTextCriteria == QCoreApplication::translate("MainWindow", "All Words")){
        QString searchTextToSplit = searchText;
        QString groupRegEx = "";
        QRegularExpression lineSplitExp(" ");
        QStringList lineFieldList = searchTextToSplit.split(lineSplitExp);
        int numberOfSearchWords = lineFieldList.count();
        //Build regex group for one word
        for (int i=0; i<(numberOfSearchWords); i++){
            groupRegEx = groupRegEx + "(?=.*" + lineFieldList[i] + ")";
        }
        regexSearchtext = groupRegEx;
    }
    else {
        regexSearchtext="";
    }

    regexPattern = regexSearchtext;

    //Prepare the regexFileType for file types
    if( searchOnFileCriteria==true and searchOnType ==true and selectedFileType !="All"){
        //Get the list of file extension and join it into one string
        if(selectedFileType =="Audio"){
            regexFileType = fileType_AudioS.join("|");
        }
        if(selectedFileType =="Image"){
            regexFileType = fileType_ImageS.join("|");
        }
        if(selectedFileType =="Text"){
            regexFileType = fileType_TextS.join("|");
        }
        if(selectedFileType =="Video"){
            regexFileType = fileType_VideoS.join("|");
        }

        //Replace the *. by .* needed for regex
        regexFileType = regexFileType.replace("*.",".*");

        //Add the file type expression to the regex
        regexPattern = regexSearchtext  + "(" + regexFileType + ")";

    }
    else
        regexPattern = regexSearchtext;

    //Add the words to exclude to the Regular Expression
    if ( selectedSearchExclude !=""){

        //Prepare
        QString searchTextToSplit = selectedSearchExclude;
        QString excludeGroupRegEx = "";
        QRegularExpression lineSplitExp(" ");
        QStringList lineFieldList = searchTextToSplit.split(lineSplitExp);
        int numberOfSearchWords = lineFieldList.count();

        //Build regex group to exclude all words
        //Genereate first part = first characters + the first word
        excludeGroupRegEx = "^(?!.*(" + lineFieldList[0];
        //Add more words
        for (int i=1; i<(numberOfSearchWords); i++){
            excludeGroupRegEx = excludeGroupRegEx + "|" + lineFieldList[i];
        }
        //last part
        excludeGroupRegEx = excludeGroupRegEx + "))";

        //Add regex group to exclude to the global regexPattern
        regexPattern = excludeGroupRegEx + regexPattern;
    }

    //Initiate Regular Expression
    QRegularExpression regex(regexPattern);
    if (caseSensitive != true) {
        regex.setPatternOptions(QRegularExpression::CaseInsensitiveOption);
    }

    //Load the catalog file contents if not already loaded in memory
    QMutex tempMutex;
    bool tempStopRequested = false;

    //Applicable for databaseMode=="Memory"
    device->catalog->loadCatalogFileListToTable("defaultConnection", tempMutex, tempStopRequested);

    //Search loop for all lines in the catalog file
    //Load the files of the Catalog
    QSqlQuery getFilesQuery(QSqlDatabase::database("defaultConnection"));
    QString getFilesQuerySQL = QLatin1String(R"(
                                            SELECT  file_name,
                                                    file_folder_path,
                                                    file_size,
                                                    file_date_updated
                                            FROM  file
                                            WHERE file_catalog_id =:file_catalog_id
                                        )");

    //Add matching size range
    if (searchOnFileCriteria==true and searchOnSize==true){
        getFilesQuerySQL = getFilesQuerySQL+" AND file_size>=:file_size_min ";
        getFilesQuerySQL = getFilesQuerySQL+" AND file_size<=:file_size_max ";
    }
    //Add matching date range
    if (searchOnFileCriteria==true and searchOnDate==true){
        getFilesQuerySQL = getFilesQuerySQL+" AND file_date_updated>=:file_date_updated_min ";
        getFilesQuerySQL = getFilesQuerySQL+" AND file_date_updated<=:file_date_updated_max ";
    }
    getFilesQuery.prepare(getFilesQuerySQL);
    getFilesQuery.bindValue(":file_catalog_id", device->externalID);
    getFilesQuery.bindValue(":file_size_min", selectedMinimumSize * sizeMultiplierMin);
    getFilesQuery.bindValue(":file_size_max", selectedMaximumSize * sizeMultiplierMax);
    getFilesQuery.bindValue(":file_date_updated_min", selectedDateMin.toString("yyyy/MM/dd hh:mm:ss"));
    getFilesQuery.bindValue(":file_date_updated_max", selectedDateMax.toString("yyyy/MM/dd hh:mm:ss"));
    getFilesQuery.exec();

    //File by file, test if the file is matching all search criteria
    //Loop principle1: stop further verification as soon as a criteria fails to match
    //Loop principle2: start with fastest criteria, finish with more complex ones (tag, file name)
    while(getFilesQuery.next()){

        QString   lineFileName       = getFilesQuery.value(0).toString();
        QString   lineFileFolderPath = getFilesQuery.value(1).toString();
        QString   lineFileFullPath   = lineFileFolderPath + "/" + lineFileName;
        bool      fileIsMatchingTag;

        //Continue to the next file if the current file is not matching the tags
        if (searchOnFolderCriteria==true and searchOnTags==true and selectedTagName!=""){

            fileIsMatchingTag = false;

            //Set query to get a list of folder paths matching the selected tag
            QSqlQuery queryTag(QSqlDatabase::database("defaultConnection"));
            QString queryTagSQL = QLatin1String(R"(
                                                SELECT path
                                                FROM tag
                                                WHERE name=:name
                            )");
            queryTag.prepare(queryTagSQL);
            queryTag.bindValue(":name",selectedTagName);
            queryTag.exec();

            //Test if the FilePath contains a path from the list of folders matching the selected tag name
            // a slash "/" is added at the end of both values to ensure no result from a tag "AB" is returned when a tag "A" is selected
            while(queryTag.next()){
                if ( (lineFileFolderPath+"/").contains( queryTag.value(0).toString()+"/" )==true){
                    fileIsMatchingTag = true;
                    break;
                }
            }

            //If the file is not matching any of the paths, process the next file
            if ( !fileIsMatchingTag==true ){
                continue;}
        }

        //Finally, verify the text search criteria
        if (searchOnFileName==true){
            //Depends on the "Search in" criteria,
            //Reduces the abosulte path to the required text string and matches the search text
            if(selectedSearchIn == QCoreApplication::translate("MainWindow", "File names only"))
            {
                match = regex.match(lineFileName);
            }
            else if(selectedSearchIn == QCoreApplication::translate("MainWindow", "Folder path only"))
            {

                //Check that the folder name matches the search text
                regex.setPattern(regexSearchtext);
                foldermatch = regex.match(lineFileFolderPath);
                //If it does, then check that the file matches the selected file type
                if (foldermatch.hasMatch() and searchOnType==true){
                    regex.setPattern(regexFileType);
                    match = regex.match(lineFileName);
                }
                else
                    match = foldermatch; //selectedSearchIn == QCoreApplication::translate("MainWindow", "Files and Folder paths")
            }
            else {
                match = regex.match(lineFileFullPath);
            }
            //If the file is matching the criteria, add it and its catalog to the search results
            if (match.hasMatch()){
                filesFoundList << lineFileFolderPath;
                deviceFoundIDList.insert(0,QString::number(device->ID));

                //Populate result lists
                fileNames.append(lineFileName);
                filePaths.append(lineFileFolderPath);
                fileSizes.append(getFilesQuery.value(2).toLongLong());
                fileDateTimes.append(getFilesQuery.value(3).toString());
                fileCatalogs.append(device->name);
                fileCatalogIDs.append(device->externalID);
            }
        }
        else{
            //verify file matches the selected file type
            if (searchOnType==true){
                regex.setPattern(regexFileType);
            }
            match = regex.match(lineFileFolderPath);
            if (!match.hasMatch()){
                continue;
            }

            //Add the file and its catalog to the results, excluding blank lines
            if (lineFileFolderPath !=""){
                filesFoundList << lineFileFolderPath;
                deviceFoundIDList.insert(0, QString::number(device->ID));

                //Populate result lists
                fileNames.append(lineFileName);
                filePaths.append(lineFileFolderPath);
                fileSizes.append(getFilesQuery.value(2).toLongLong());
                fileDateTimes.append(getFilesQuery.value(3).toString());
                fileCatalogs.append(device->name);
                fileCatalogIDs.append(device->externalID);
            }
        }
    }
}

//-----------------
void Search::searchFilesInDirectory(const QString &sourceDirectory)
{
    //Run a search of files for the selected Directory

    //Define how to use the search text //COMMON to searchFilesInCatalog
    if(selectedTextCriteria == QCoreApplication::translate("MainWindow", "Exact Phrase"))
        regexSearchtext=searchText; //just search for the extact text entered including spaces, as one text string.
    else if(selectedTextCriteria == QCoreApplication::translate("MainWindow", "Begins With"))
        regexSearchtext="(^"+searchText+")";
    else if(selectedTextCriteria == QCoreApplication::translate("MainWindow", "Any Word"))
        regexSearchtext=searchText.replace(" ","|");
    else if(selectedTextCriteria == QCoreApplication::translate("MainWindow", "All Words")){
        QString searchTextToSplit = searchText;
        QString groupRegEx = "";
        QRegularExpression lineSplitExp(" ");
        QStringList lineFieldList = searchTextToSplit.split(lineSplitExp);
        int numberOfSearchWords = lineFieldList.count();
        //Build regex group for one word
        for (int i=0; i<(numberOfSearchWords); i++){
            groupRegEx = groupRegEx + "(?=.*" + lineFieldList[i] + ")";
        }
        regexSearchtext = groupRegEx;
    }
    else {
        regexSearchtext="";
    }
    regexPattern = regexSearchtext;

    //Prepare the regexFileType for file types //COMMON to searchFilesInCatalog
    if ( searchOnFileCriteria==true and selectedFileType !=QCoreApplication::translate("MainWindow", "All")){
        //Get the list of file extension and join it into one string
        if(selectedFileType =="Audio"){
            regexFileType = fileType_AudioS.join("|");
        }
        if(selectedFileType == "Image"){
            regexFileType = fileType_ImageS.join("|");
        }
        if(selectedFileType == "Text"){
            regexFileType = fileType_TextS.join("|");
        }
        if(selectedFileType == "Video"){
            regexFileType = fileType_VideoS.join("|");
        }

        //Replace the *. by .* needed for regex
        regexFileType = regexFileType.replace("*.",".*");

        //Add the file type expression to the regex
        regexPattern = regexSearchtext  + "(" + regexFileType + ")";
    }

    //Add the words to exclude to the regex //COMMON to searchFilesInCatalog
    if ( selectedSearchExclude !=""){

        //Prepare
        QString searchTextToSplit = selectedSearchExclude;
        QString excludeGroupRegEx = "";
        QRegularExpression lineSplitExp(" ");
        QStringList lineFieldList = searchTextToSplit.split(lineSplitExp);
        int numberOfSearchWords = lineFieldList.count();

        //Build regex group to exclude all words
        //Genereate first part = first characters + the first word
        excludeGroupRegEx = "^(?!.*(" + lineFieldList[0];
        //Add more words
        for (int i=1; i<(numberOfSearchWords); i++){
            excludeGroupRegEx = excludeGroupRegEx + "|" + lineFieldList[i];
        }
        //Last part
        excludeGroupRegEx = excludeGroupRegEx + "))";

        //Add regex group to exclude to the global regexPattern
        regexPattern = excludeGroupRegEx + regexPattern;
    }

    QRegularExpression regex(regexPattern, QRegularExpression::CaseInsensitiveOption);

    //Filetypes
    //Get the file type for the catalog
    QStringList fileTypes;

    //Scan directory and create a list of files
    QString line;
    QString reducedLine;


    QDirIterator iterator(sourceDirectory, fileTypes, QDir::Files|QDir::Hidden, QDirIterator::Subdirectories);
    while (iterator.hasNext()){

        //Get file information  (absolute path, size, datetime)
        QString filePath = iterator.next();
        QFileInfo fileInfo(filePath);
        QDateTime fileDate = fileInfo.lastModified();

        line = fileInfo.absoluteFilePath() + "\t" + QString::number(fileInfo.size()) + "\t" + fileDate.toString("yyyy/MM/dd hh:mm:ss");

        //COMMON to searchFilesInCatalog
        QRegularExpressionMatch match;
        QRegularExpressionMatch foldermatch;
        //QRegularExpressionMatch matchFileType;

        //Split the line text with tabulations into a list
        QRegularExpression     lineSplitExp("\t");
        QStringList lineFieldList  = line.split(lineSplitExp);
        int         fieldListCount = lineFieldList.count();

        //Get the file absolute path from this list
        QString     lineFilePath   = lineFieldList[0];

        //Get the FileSize from the list if available
        qint64      lineFileSize;
        if (fieldListCount == 3){lineFileSize = lineFieldList[1].toLongLong();}
        else lineFileSize = 0;

        //Get the File DateTime from the list if available
        QDateTime   lineFileDateTime;
        if (fieldListCount == 3){lineFileDateTime = QDateTime::fromString(lineFieldList[2],"yyyy/MM/dd hh:mm:ss");}
        else lineFileDateTime = QDateTime::fromString("0001/01/01 00:00:00","yyyy/MM/dd hh:mm:ss");

        //Exclude catalog metadata lines which are starting with the character <
        if (lineFilePath.left(1)=="<"){continue;}

        //Continue if the file is matching the size range
        if (searchOnSize==true){
            if ( !(     lineFileSize >= selectedMinimumSize * sizeMultiplierMin
                  and lineFileSize <= selectedMaximumSize * sizeMultiplierMax) ){
                continue;}
        }

        //Continue if the file is matching the date range
        if (searchOnDate==true){
            if ( !(     lineFileDateTime >= selectedDateMin
                  and lineFileDateTime <= selectedDateMax ) ){
                continue;}
        }

        //Continue if the file is matching the tags
        if (searchOnTags==true){

            bool fileIsMatchingTag = false;

            //Set query to get a list of folder paths matching the selected tag
            QSqlQuery queryTag(QSqlDatabase::database("defaultConnection"));
            QString queryTagSQL = QLatin1String(R"(
                                                SELECT path
                                                FROM tag
                                                WHERE name=:name
                            )");
            queryTag.prepare(queryTagSQL);
            queryTag.bindValue(":name",selectedTagName);
            queryTag.exec();

            //Test if the FilePath contains a path from the list of folders matching the selected tag name
            while(queryTag.next()){
                if ( lineFilePath.contains(queryTag.value(0).toString())==true){
                    fileIsMatchingTag = true;
                    break;
                }
                //else tagIsMatching==false
            }

            //If the file is not matching any of the paths, process the next file
            if ( !fileIsMatchingTag==true ){
                continue;}
        }

        //Finally, verify the text search criteria
        if (searchOnFileName==true){
            //Depending on the "Search in" criteria,
            //reduce the abosulte path to the reaquired text string and match the search text
            if(selectedSearchIn == QCoreApplication::translate("MainWindow", "File names only"))
            {
                //Extract the file name from the lineFilePath
                QFileInfo file(lineFilePath);
                reducedLine = file.fileName();

                match = regex.match(reducedLine);
            }
            else if(selectedSearchIn == QCoreApplication::translate("MainWindow", "Folder path only"))
            {
                //Keep only the folder name, so all characters left of the last occurence of / in the path.
                reducedLine = lineFilePath.left(lineFilePath.lastIndexOf("/"));

                //Check the folder name matches the search text
                regex.setPattern(regexSearchtext);

                foldermatch = regex.match(reducedLine);

                //if it does, then check that the file matches the selected file type
                if (foldermatch.hasMatch()){
                    regex.setPattern(regexFileType);

                    match = regex.match(lineFilePath);
                }
            }
            else {
                match = regex.match(lineFilePath);
            }

            //If the file is matching the criteria, add it and its catalog to the search results
            //COMMON to searchFilesInCatalog
            if (match.hasMatch()){

                filesFoundList << lineFilePath;

                //COMMON to searchFilesInCatalog
                //Retrieve other file info
                QFileInfo file(lineFilePath);

                // Get the fileDateTime from the list if available
                QString lineFileDatetime;
                if (fieldListCount == 3){
                    lineFileDatetime = lineFieldList[2];}
                else lineFileDatetime = "";

                //Populate result lists
                fileNames.append(file.fileName());
                fileSizes.append(lineFileSize);
                fileDateTimes.append(lineFileDatetime);
                filePaths.append(file.path());
                fileCatalogs.append(sourceDirectory);
                fileCatalogIDs.append(0);
            }
        }
        else{

            //Add the file and its catalog to the results, excluding blank lines
            if (lineFilePath !=""){
                filesFoundList << lineFilePath;
                deviceFoundIDList.insert(0, sourceDirectory);

                //Retrieve other file info
                QFileInfo file(lineFilePath);

                // Get the fileDateTime from the list if available
                QString lineFileDatetime;
                if (fieldListCount == 3){
                    lineFileDatetime = lineFieldList[2];}
                else lineFileDatetime = "";

                //Populate result lists
                fileNames.append(file.fileName());
                fileSizes.append(lineFileSize);
                fileDateTimes.append(lineFileDatetime);
                filePaths.append(file.path());
                fileCatalogs.append(sourceDirectory);
                fileCatalogIDs.append(0);
            }
        }
    }
}

//methods
void Search::loadSearchHistoryCriteria()
{
    //Query
    QSqlQuery query(QSqlDatabase::database("defaultConnection"));
    QString querySQL = QLatin1String(R"(
                                SELECT
                                    date_time,
                                    text_checked,
                                    text_phrase,
                                    text_criteria,
                                    text_search_in,
                                    case_sensitive,
                                    text_exclude,
                                    file_criteria_checked,
                                    file_size_checked,
                                    file_size_min,
                                    file_size_min_unit,
                                    file_size_max,
                                    file_size_max_unit,
                                    file_type_checked,
                                    file_type,
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
                                    folder_criteria_checked,
                                    show_folders,
                                    tag_checked,
                                    tag,
                                    search_location,
                                    search_storage,
                                    search_catalog,
                                    search_catalog_checked,
                                    search_directory_checked,
                                    selected_directory
                                FROM search
                                WHERE date_time =:date_time
                               )");
    query.prepare(querySQL);
    query.bindValue(":date_time", searchDateTime);
    query.exec();

    if (query.next()){
        searchOnFileName       = query.value(1).toBool();
        searchText             = query.value(2).toString();
        selectedTextCriteria   = query.value(3).toString();
        selectedSearchIn       = query.value(4).toString();
        caseSensitive          = query.value(5).toBool();
        selectedSearchExclude  = query.value(6).toString();
        searchOnFileCriteria   = query.value(7).toBool();
        searchOnSize           = query.value(8).toBool();
        selectedMinimumSize    = query.value(9).toLongLong();
        selectedMinSizeUnit    = query.value(10).toString();
        selectedMaximumSize    = query.value(11).toLongLong();
        selectedMaxSizeUnit    = query.value(12).toString();
        searchOnType           = query.value(13).toBool();
        selectedFileType       = query.value(14).toString();
        searchOnDate           = query.value(15).toBool();
        selectedDateMin        = query.value(16).toDateTime();
        selectedDateMax        = query.value(17).toDateTime();
        searchOnDuplicates     = query.value(18).toBool();
        searchDuplicatesOnName = query.value(19).toBool();
        searchDuplicatesOnSize = query.value(20).toBool();
        searchDuplicatesOnDate = query.value(21).toBool();
        searchOnDifferences    = query.value(22).toBool();
        differencesOnName      = query.value(23).toBool();
        differencesOnSize      = query.value(24).toBool();
        differencesOnDate      = query.value(25).toBool();
        differencesDevices    = query.value(26).toString().split("||");
        if (differencesDevices.length()>1){
            differencesDeviceID1 = differencesDevices[0].toInt();
            differencesDeviceID2 = differencesDevices[1].toInt();
        }
        searchOnFolderCriteria  = query.value(27).toBool();
        showFoldersOnly         = query.value(28).toBool();
        searchOnTags            = query.value(29).toBool();
        selectedTagName         = query.value(30).toString();
        selectedStorage         = query.value(32).toString();
        selectedCatalog         = query.value(33).toString();
        searchInCatalogsChecked = query.value(34).toBool();
        searchInConnectedChecked= query.value(35).toBool();
        connectedDirectory      = query.value(36).toString();
    }
}

void Search::setMultipliers()
{//Define a size multiplier depending on the size unit selected
    sizeMultiplierMin=1;
    if      (selectedMinSizeUnit == QCoreApplication::translate("MainWindow", "KiB"))
            sizeMultiplierMin = sizeMultiplierMin *1024;
    else if (selectedMinSizeUnit == QCoreApplication::translate("MainWindow", "MiB"))
            sizeMultiplierMin = sizeMultiplierMin *1024*1024;
    else if (selectedMinSizeUnit == QCoreApplication::translate("MainWindow", "GiB"))
            sizeMultiplierMin = sizeMultiplierMin *1024*1024*1024;
    else if (selectedMinSizeUnit == QCoreApplication::translate("MainWindow", "TiB"))
            sizeMultiplierMin = sizeMultiplierMin *1024*1024*1024*1024;
    sizeMultiplierMax=1;
    if      (selectedMaxSizeUnit == QCoreApplication::translate("MainWindow", "KiB"))
            sizeMultiplierMax = sizeMultiplierMax *1024;
    else if (selectedMaxSizeUnit == QCoreApplication::translate("MainWindow", "MiB"))
            sizeMultiplierMax = sizeMultiplierMax *1024*1024;
    else if (selectedMaxSizeUnit == QCoreApplication::translate("MainWindow", "GiB"))
            sizeMultiplierMax = sizeMultiplierMax *1024*1024*1024;
    else if (selectedMaxSizeUnit == QCoreApplication::translate("MainWindow", "TiB"))
            sizeMultiplierMax = sizeMultiplierMax *1024*1024*1024*1024;
}

//----------------------------------------------------------------------
void Search::insertSearchHistoryToTable(QString connectionName)
{//Save Search to db
    QDateTime nowDateTime = QDateTime::currentDateTime();
    QString searchDateTime = nowDateTime.toString("yyyy-MM-dd hh:mm:ss");

    QSqlQuery query(QSqlDatabase::database(connectionName));
    QString querySQL = QLatin1String(R"(
                                INSERT INTO search(
                                    date_time,
                                    text_checked,
                                    text_phrase,
                                    text_criteria,
                                    text_search_in,
                                    file_criteria_checked,
                                    file_type_checked,
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
                                    folder_criteria_checked,
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
                                    case_sensitive
                                )
                                VALUES(
                                    :date_time,
                                    :text_checked,
                                    :text_phrase,
                                    :text_criteria,
                                    :text_search_in,
                                    :file_criteria_checked,
                                    :file_type_checked,
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
                                    :folder_criteria_checked,
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
                                    :case_sensitive
                                )
                )");

    query.prepare(querySQL);
    query.bindValue(":date_time",                 searchDateTime);
    query.bindValue(":text_checked",              searchOnFileName);
    query.bindValue(":text_phrase",               searchText);
    query.bindValue(":text_criteria",             selectedTextCriteria);
    query.bindValue(":text_search_in",            selectedSearchIn);
    query.bindValue(":file_criteria_checked",     searchOnFileCriteria);
    query.bindValue(":file_type_checked",         searchOnType);
    query.bindValue(":file_type",                 selectedFileType);
    query.bindValue(":file_size_checked",         searchOnSize);
    query.bindValue(":file_size_min",             selectedMinimumSize);
    query.bindValue(":file_size_min_unit",        selectedMinSizeUnit);
    query.bindValue(":file_size_max",             selectedMaximumSize);
    query.bindValue(":file_size_max_unit",        selectedMaxSizeUnit);
    query.bindValue(":date_modified_checked",     searchOnDate);
    query.bindValue(":date_modified_min",         selectedDateMin);
    query.bindValue(":date_modified_max",         selectedDateMax);
    query.bindValue(":duplicates_checked",        searchOnDuplicates);
    query.bindValue(":duplicates_name",           searchDuplicatesOnName);
    query.bindValue(":duplicates_size",           searchDuplicatesOnSize);
    query.bindValue(":duplicates_date_modified",  searchDuplicatesOnDate);
    query.bindValue(":differences_checked",       searchOnDifferences);
    query.bindValue(":differences_name",          differencesOnName);
    query.bindValue(":differences_size",          differencesOnSize);
    query.bindValue(":differences_date_modified", differencesOnDate);
    query.bindValue(":differences_catalogs",      QString::number(differencesDeviceID1) + "||" + QString::number(differencesDeviceID2));
    query.bindValue(":folder_criteria_checked",   searchOnFolderCriteria);
    query.bindValue(":show_folders",              showFoldersOnly);
    query.bindValue(":tag_checked",               searchOnTags);
    query.bindValue(":tag",                       selectedTagName);
    query.bindValue(":search_storage",            selectedStorage);
    query.bindValue(":search_catalog",            selectedCatalog);
    query.bindValue(":search_catalog_checked",    searchInCatalogsChecked);
    query.bindValue(":search_directory_checked",  searchInConnectedChecked);
    query.bindValue(":selected_directory",        connectedDirectory);
    query.bindValue(":text_exclude",              selectedSearchExclude);
    query.bindValue(":case_sensitive",            caseSensitive);
    query.exec();
}
