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

#include "search.h"  // qt_quick/core/search.h → inherits ../../core/search.h

SearchSync::SearchSync(QObject *parent) : Search(parent)
{
}

//file list model
int SearchSync::rowCount(const QModelIndex &parent) const
{
    Q_UNUSED(parent);
    return fileNames.length();
}

int SearchSync::columnCount(const QModelIndex &parent) const
{
    Q_UNUSED(parent);
    return 5;
}

QVariant SearchSync::data(const QModelIndex &index, int role) const
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

QVariant SearchSync::headerData(int section, Qt::Orientation orientation, int role) const
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

QHash<int, QByteArray> SearchSync::roleNames() const
{
    // Start with the base QAbstractItemModel defaults (includes "display" for Qt::DisplayRole)
    QHash<int, QByteArray> roles = QAbstractTableModel::roleNames();
    // Merge in the named roles from Search for QML access by role name
    const QHash<int, QByteArray> searchRoles = Search::roleNames();
    for (auto it = searchRoles.cbegin(); it != searchRoles.cend(); ++it) {
        roles[it.key()] = it.value();
    }
    return roles;
}

QString SearchSync::testFunction()
{
    return "testObject: " + searchInCatalogsChecked;
}

//----------------------------------------------------------------------
void SearchSync::resetSearchResults()
{
    clearResults();
    searchTextList.clear();

    emit propertiesChanged();
}
//----------------------------------------------------------------------
//Search process
void SearchSync::searchFiles(Device *selectedDevice)
{//Run a search of files in each selected catalog based on user inputs

    //Prepare the SEARCH -------------------------------
        //Clear the previous search data
        resetSearchResults();
        setFileTypes();
        setMultipliers();

    QMutex mutex;
    bool stopRequested = false;

    if (searchInCatalogsChecked == true){

            if (selectedDevice->type == "Catalog") {
                searchFilesInCatalog(selectedDevice, mutex, stopRequested);
            }
            else {
                foreach(const Device::deviceListRow & row, selectedDevice->deviceListTable) {
                    if (row.type == "Catalog") {
                        Device *device = new Device;
                        device->ID = row.ID;
                        device->loadDevice(QSqlDatabase::defaultConnection);
                        searchFilesInCatalog(device, mutex, stopRequested);
                        delete device;
                    }
                }
            }

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

    //Create model and load to the view
    deviceFoundModel = new QStandardItemModel;
    deviceFoundModel->setHorizontalHeaderLabels({ "Catalog with results", "ID" });

    Device loopDevice;
    for (const QString &ID : deviceFoundIDList) {
        loopDevice.ID = ID.toInt();
        loopDevice.loadDevice(QSqlDatabase::defaultConnection);
        QList<QStandardItem *> items;
        items << new QStandardItem(loopDevice.name);
        items << new QStandardItem(QString::number(loopDevice.ID));
        deviceFoundModel->appendRow(items);
    }

    //Files found Statistics
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

    emit dataChanged(index(0, 0), index(rowCount() - 1, columnCount() - 1));
    emit propertiesChanged();
    emit layoutChanged();

}
//----------------------------------------------------------------------
void SearchSync::searchFilesInCatalog(Device *device, QMutex &mutex, bool &stopRequested)
{//Run a search of files for the selected Catalog
    Q_UNUSED(mutex);
    Q_UNUSED(stopRequested);

    //Prepare Inputs including Regular Expression
    QRegularExpressionMatch match;
    QRegularExpressionMatch foldermatch;

    //Define how to use the search text
    if(selectedTextCriteria == TEXT_CRITERIA_EXACT_PHRASE)
        regexSearchtext=searchText;
    else if(selectedTextCriteria == TEXT_CRITERIA_BEGINS_WITH)
        regexSearchtext="(^"+searchText+")";
    else if(selectedTextCriteria == TEXT_CRITERIA_ANY_WORD)
        regexSearchtext=QString(searchText).replace(" ","|");
    else if(selectedTextCriteria == TEXT_CRITERIA_ALL_WORDS){

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
    qDebug()<<"selectedFileType: "<<selectedFileType;
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

    qDebug()<<"regexPattern: "<<regexPattern;

    //Initiate Regular Expression
    QRegularExpression regex(regexPattern);
    if (caseSensitive != true) {
        regex.setPatternOptions(QRegularExpression::CaseInsensitiveOption);
    }

    //Search loop for all lines in the catalog file
    //Load the files of the Catalog
    QSqlQuery getFilesQuery;
    QString getFilesQuerySQL = QLatin1String(R"(
                                        SELECT  file_name,
                                                file_folder_path,
                                                file_size,
                                                file_date_updated
                                        FROM  file
                                        WHERE file_catalog=:file_catalog
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
    getFilesQuery.bindValue(":file_catalog", device->name);
    getFilesQuery.bindValue(":file_size_min", selectedMinimumSize * sizeMultiplierMin);
    getFilesQuery.bindValue(":file_size_max", selectedMaximumSize * sizeMultiplierMax);
    getFilesQuery.bindValue(":file_date_updated_min", selectedDateMin.toString("yyyy/MM/dd hh:mm:ss"));
    getFilesQuery.bindValue(":file_date_updated_max", selectedDateMax.toString("yyyy/MM/dd hh:mm:ss"));
    getFilesQuery.exec();

    //File by file, test if the file is matching all search criteria
    //Loop principle1: stop further verification as soon as a criteria fails to match
    //Loop principle2: start with fastest criteria, finish with more complex ones (tag, file name)

    while(getFilesQuery.next()){

        QString   lineFileName     = getFilesQuery.value(0).toString();
        QString   lineFilePath     = getFilesQuery.value(1).toString();
        QString   lineFileFullPath = lineFilePath + "/" + lineFileName;
        bool      fileIsMatchingTag;

        //Continue to the next file if the current file is not matching the tags
        if (searchOnFolderCriteria==true and searchOnTags==true and selectedTagName!=""){

            fileIsMatchingTag = false;

            //Set query to get a list of folder paths matching the selected tag
            QSqlQuery queryTag;
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
                if ( (lineFilePath+"/").contains( queryTag.value(0).toString()+"/" )==true){
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
            //Reduces the absolute path to the required text string and matches the search text
            if(selectedSearchIn == SEARCH_IN_FILE_NAMES)
            {
                match = regex.match(lineFileName);
            }
            else if(selectedSearchIn == SEARCH_IN_FOLDER_PATH)
            {

                //Check that the folder name matches the search text
                regex.setPattern(regexSearchtext);
                foldermatch = regex.match(lineFilePath);
                //If it does, then check that the file matches the selected file type
                if (foldermatch.hasMatch() and searchOnType==true){
                    regex.setPattern(regexFileType);
                    match = regex.match(lineFileName);
                }
                else
                    match = foldermatch;
            }
            else {
                match = regex.match(lineFileFullPath);
            }
            //If the file is matching the criteria, add it and its catalog to the search results
            if (match.hasMatch()){
                filesFoundList << lineFilePath;
                deviceFoundIDList.insert(0,QString::number(device->ID));

                //Populate result lists
                fileNames.append(lineFileName);
                filePaths.append(lineFilePath);
                fileSizes.append(getFilesQuery.value(2).toLongLong());
                fileDateTimes.append(getFilesQuery.value(3).toString());
                fileCatalogs.append(device->name);
            }
        }
        else{
            //verify file matches the selected file type
            if (searchOnType==true){
                regex.setPattern(regexFileType);
            }
            match = regex.match(lineFilePath);
            if (!match.hasMatch()){
                continue;
            }

            //Add the file and its catalog to the results, excluding blank lines
            if (lineFilePath !=""){
                filesFoundList << lineFilePath;
                deviceFoundIDList.insert(0, QString::number(device->ID));

                //Populate result lists
                fileNames.append(lineFileName);
                filePaths.append(lineFilePath);
                fileSizes.append(getFilesQuery.value(2).toLongLong());
                fileDateTimes.append(getFilesQuery.value(3).toString());
                fileCatalogs.append(device->name);
            }
        }
    }
}
//----------------------------------------------------------------------
void SearchSync::searchFilesInDirectory(const QString &sourceDirectory, QMutex &mutex, bool &stopRequested)
{//Run a search of files for the selected Directory
    Q_UNUSED(mutex);
    Q_UNUSED(stopRequested);

    regexPattern = regexSearchtext;

    //Prepare the regexFileType for file types
    if ( searchOnFileCriteria==true and selectedFileType !="All"){
        //Replace the *. by .* needed for regex
        regexFileType = regexFileType.replace("*.",".*");

        //Add the file type expression to the regex
        regexPattern = regexSearchtext  + "(" + regexFileType + ")";
    }

    //Add the words to exclude to the regex
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

        QRegularExpressionMatch match;
        QRegularExpressionMatch foldermatch;

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
        qDebug() <<selectedMaximumSize<<sizeMultiplierMax<<selectedMaximumSize * sizeMultiplierMax;
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
            QSqlQuery queryTag;
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
            }

            //If the file is not matching any of the paths, process the next file
            if ( !fileIsMatchingTag==true ){
                continue;}
        }

        //Finally, verify the text search criteria
        if (searchOnFileName==true){
            //Depending on the "Search in" criteria,
            //reduce the absolute path to the required text string and match the search text
            if(selectedSearchIn == SEARCH_IN_FILE_NAMES)
            {
                // Extract the file name from the lineFilePath
                QFileInfo file(lineFilePath);
                reducedLine = file.fileName();

                match = regex.match(reducedLine);
            }
            else if(selectedSearchIn == SEARCH_IN_FOLDER_PATH)
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
            if (match.hasMatch()){

                filesFoundList << lineFilePath;

                //Retrieve other file info
                QFileInfo file(lineFilePath);

                // Get the fileDateTime from the list if available
                QString lineFileDatetime;
                if (fieldListCount == 3){
                    lineFileDatetime = lineFieldList[2];}
                else lineFileDatetime = "";

                //Populate result lists
                fileNames.append(file.fileName());
                filePaths.append(file.path());
                fileSizes.append(lineFileSize);
                fileDateTimes.append(lineFileDatetime);
                fileCatalogs.append(sourceDirectory);
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
                filePaths.append(file.path());
                fileSizes.append(lineFileSize);
                fileDateTimes.append(lineFileDatetime);
                fileCatalogs.append(sourceDirectory);
            }
        }
    }
}
//----------------------------------------------------------------------
void SearchSync::setFileTypes()
{
    //Filetypes for cataloging
    fileType_Audio<< "*.mp3" << "*.wav" << "*.ogg" << "*.aif";
    fileType_Image<< "*.png" << "*.jpg" << "*.gif" << "*.xcf" << "*.tif" << "*.bmp";
    fileType_Text << "*.txt" << "*.pdf" << "*.odt" << "*.idx" << "*.html" << "*.rtf" << "*.doc" << "*.docx" << "*.epub";
    fileType_Video<< "*.wmv" << "*.avi" << "*.mp4" << "*.mkv" << "*.flv"  << "*.webm" << "*.m4v" << "*.vob" << "*.ogv" << "*.mov";

    //filetypes for searching
    fileType_AudioS<< "*.mp3$" << "*.wav$" << "*.ogg$" << "*.aif$";
    fileType_ImageS<< "*.png$" << "*.jpg$" << "*.gif$" << "*.xcf$" << "*.tif$" << "*.bmp$";
    fileType_TextS << "*.txt$" << "*.pdf$" << "*.odt$" << "*.idx$" << "*.html$" << "*.rtf$" << "*.doc$" << "*.docx$" << "*.epub$";
    fileType_VideoS<< "*.wmv$" << "*.avi$" << "*.mp4$" << "*.mkv$" << "*.flv$"  << "*.webm$"<< "*.m4v$" << "*.vob$"  << "*.ogv$" << "*.mov$";
}
//----------------------------------------------------------------------
