#include "searchprocess.h"
#include "search.h"
#include "device.h"

#include "searchprocess.h"
#include "device.h"
#include "src/filesview.h"
#include <QApplication>
#include <QMessageBox>
#include <QSqlQuery>
#include <QStandardItemModel>
#include <QSqlQueryModel>

SearchProcess::SearchProcess(MainWindow *mainWindow, QString searchDatabaseMode, QObject *parent)
    : QThread(parent), mainWindow(mainWindow), isStopped(false)
{
    databaseMode = searchDatabaseMode;
}

void SearchProcess::run()
{
    QApplication::setOverrideCursor(Qt::WaitCursor);

    //Initialize the database connection within the SearchProcess thread
    QString connectionName = "searchConnection";

    // Initialize the database connection within the SearchProcess thread
    QSqlError dbError = mainWindow->initializeDatabase(connectionName);
    if (dbError.type() != QSqlError::NoError) {
        qWarning() << "DEBUG: SearchProcess::run() Failed to initialize database: " << dbError.text();
        emit searchStopped();
        return;
    }

    //Process the SEARCH in CATALOGS or DIRECTORY ------------------------------
        //Process the SEARCH in CATALOGS
        if (mainWindow->newSearch->searchInCatalogsChecked == true) {

            //For differences, only process the 2 selected catalogs
            if (mainWindow->newSearch->searchOnDifferences == true and stopRequested==false) {
                Device *diffDevice = new Device;
                diffDevice->ID = mainWindow->getDifferencesCatalog1ID();
                diffDevice->loadDevice("searchConnection");
                searchFilesInCatalog(diffDevice);

                diffDevice->ID = mainWindow->getDifferencesCatalog2ID();
                diffDevice->loadDevice("searchConnection");
                searchFilesInCatalog(diffDevice);
            }
            //Otherwise search in the list of catalogs in the selectedDevice
            else {
                if (mainWindow->selectedDevice->type == "Catalog" and stopRequested==false) {
                    // If type = "Catalog", there is no sub-device, use it directly
                    searchFilesInCatalog(mainWindow->selectedDevice);
                }
                else {
                    // Otherwise, loop through sub-devices and run searchFilesInCatalog for type = "Catalog"
                    foreach (const Device &tempDevice, mainWindow->selectedDevice->subDevices) {
                        if (tempDevice.type == "Catalog" and stopRequested==false) {
                            searchFilesInCatalog(&tempDevice);
                        }
                    }
                }
            }
        }

        //Process the SEARCH in SELECTED DIRECTORY
        else if (mainWindow->newSearch->searchInConnectedChecked == true) {
            QString sourceDirectory = mainWindow->newSearch->connectedDirectory;
            searchFilesInDirectory(sourceDirectory);
        }

    //Process Search results (nominal, differences or duplicates)
    processSearchResults();

    QSqlDatabase::removeDatabase("searchConnection");
    emit searchCompleted();
    QApplication::restoreOverrideCursor();
}

void SearchProcess::stop()
{
    QMutexLocker locker(&mutex);
    stopRequested = true;
    isStopped = true;
    emit searchStopped();
}

void SearchProcess::searchFilesInCatalog(const Device *device)
{//Run a search of files for the selected Catalog
    //Prepare Inputs including Regular Expression

    QFile catalogFile(device->catalog->sourcePath);

    QRegularExpressionMatch match;
    QRegularExpressionMatch foldermatch;

    //Define how to use the search text
    if(mainWindow->newSearch->selectedTextCriteria == QCoreApplication::translate("MainWindow", "Exact Phrase"))
        mainWindow->newSearch->regexSearchtext=mainWindow->newSearch->searchText; //just search for the extact text entered including spaces, as one text string.
    else if(mainWindow->newSearch->selectedTextCriteria == QCoreApplication::translate("MainWindow", "Begins With"))
        mainWindow->newSearch->regexSearchtext="(^"+mainWindow->newSearch->searchText+")";
    else if(mainWindow->newSearch->selectedTextCriteria == QCoreApplication::translate("MainWindow", "Any Word"))
        mainWindow->newSearch->regexSearchtext=mainWindow->newSearch->searchText.replace(" ","|");
    else if(mainWindow->newSearch->selectedTextCriteria == QCoreApplication::translate("MainWindow", "All Words")){
        QString searchTextToSplit = mainWindow->newSearch->searchText;
        QString groupRegEx = "";
        QRegularExpression lineSplitExp(" ");
        QStringList lineFieldList = searchTextToSplit.split(lineSplitExp);
        int numberOfSearchWords = lineFieldList.count();
        //Build regex group for one word
        for (int i=0; i<(numberOfSearchWords); i++){
            groupRegEx = groupRegEx + "(?=.*" + lineFieldList[i] + ")";
        }
        mainWindow->newSearch->regexSearchtext = groupRegEx;
    }
    else {
        mainWindow->newSearch->regexSearchtext="";
    }

    mainWindow->newSearch->regexPattern = mainWindow->newSearch->regexSearchtext;

    //Prepare the regexFileType for file types
    if( mainWindow->newSearch->searchOnFileCriteria==true and mainWindow->newSearch->searchOnType ==true and mainWindow->newSearch->selectedFileType !="All"){
        //Get the list of file extension and join it into one string
        if(mainWindow->newSearch->selectedFileType =="Audio"){
            mainWindow->newSearch->regexFileType = mainWindow->fileType_AudioS.join("|");
        }
        if(mainWindow->newSearch->selectedFileType =="Image"){
            mainWindow->newSearch->regexFileType = mainWindow->fileType_ImageS.join("|");
        }
        if(mainWindow->newSearch->selectedFileType =="Text"){
            mainWindow->newSearch->regexFileType = mainWindow->fileType_TextS.join("|");
        }
        if(mainWindow->newSearch->selectedFileType =="Video"){
            mainWindow->newSearch->regexFileType = mainWindow->fileType_VideoS.join("|");
        }

        //Replace the *. by .* needed for regex
        mainWindow->newSearch->regexFileType = mainWindow->newSearch->regexFileType.replace("*.",".*");

        //Add the file type expression to the regex
        mainWindow->newSearch->regexPattern = mainWindow->newSearch->regexSearchtext  + "(" + mainWindow->newSearch->regexFileType + ")";

    }
    else
        mainWindow->newSearch->regexPattern = mainWindow->newSearch->regexSearchtext;

    //Add the words to exclude to the Regular Expression
    if ( mainWindow->newSearch->selectedSearchExclude !=""){

        //Prepare
        QString searchTextToSplit = mainWindow->newSearch->selectedSearchExclude;
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
        mainWindow->newSearch->regexPattern = excludeGroupRegEx + mainWindow->newSearch->regexPattern;
    }

    //Initiate Regular Expression
    QRegularExpression regex(mainWindow->newSearch->regexPattern);
    if (mainWindow->newSearch->caseSensitive != true) {
        regex.setPatternOptions(QRegularExpression::CaseInsensitiveOption);
    }

    //Load the catalog file contents if not already loaded in memory
    if(databaseMode=="Memory"){
        device->catalog->loadCatalogFileListToTable("searchConnection", mutex, stopRequested);
    }
    //Search loop for all lines in the catalog file
    //Load the files of the Catalog
    QSqlQuery getFilesQuery(QSqlDatabase::database("searchConnection"));
    QString getFilesQuerySQL = QLatin1String(R"(
                                        SELECT  file_name,
                                                file_folder_path,
                                                file_size,
                                                file_date_updated
                                        FROM  file
                                        WHERE file_catalog=:file_catalog
                                                    )");

    //Add matching size range
    if (mainWindow->newSearch->searchOnFileCriteria==true and mainWindow->newSearch->searchOnSize==true){
        getFilesQuerySQL = getFilesQuerySQL+" AND file_size>=:file_size_min ";
        getFilesQuerySQL = getFilesQuerySQL+" AND file_size<=:file_size_max ";
    }
    //Add matching date range
    if (mainWindow->newSearch->searchOnFileCriteria==true and mainWindow->newSearch->searchOnDate==true){
        getFilesQuerySQL = getFilesQuerySQL+" AND file_date_updated>=:file_date_updated_min ";
        getFilesQuerySQL = getFilesQuerySQL+" AND file_date_updated<=:file_date_updated_max ";
    }
    getFilesQuery.prepare(getFilesQuerySQL);
    getFilesQuery.bindValue(":file_catalog", device->name);
    getFilesQuery.bindValue(":file_size_min", mainWindow->newSearch->selectedMinimumSize * mainWindow->newSearch->sizeMultiplierMin);
    getFilesQuery.bindValue(":file_size_max", mainWindow->newSearch->selectedMaximumSize * mainWindow->newSearch->sizeMultiplierMax);
    getFilesQuery.bindValue(":file_date_updated_min", mainWindow->newSearch->selectedDateMin.toString("yyyy/MM/dd hh:mm:ss"));
    getFilesQuery.bindValue(":file_date_updated_max", mainWindow->newSearch->selectedDateMax.toString("yyyy/MM/dd hh:mm:ss"));
    getFilesQuery.exec();


    //File by file, test if the file is matching all search criteria
    //Loop principle1: stop further verification as soon as a criteria fails to match
    //Loop principle2: start with fastest criteria, finish with more complex ones (tag, file name)
    while(getFilesQuery.next()){

        QMutexLocker locker(&mutex);
        if (stopRequested) {
            return;
        }

        QString   lineFileName     = getFilesQuery.value(0).toString();
        QString   lineFilePath     = getFilesQuery.value(1).toString();
        QString   lineFileFullPath = lineFilePath + "/" + lineFileName;
        bool      fileIsMatchingTag;

        //Continue to the next file if the current file is not matching the tags
        if (mainWindow->newSearch->searchOnFolderCriteria==true and mainWindow->newSearch->searchOnTags==true and mainWindow->newSearch->selectedTagName!=""){

            fileIsMatchingTag = false;

            //Set query to get a list of folder paths matching the selected tag
            QSqlQuery queryTag(QSqlDatabase::database("searchConnection"));
            QString queryTagSQL = QLatin1String(R"(
                                                SELECT path
                                                FROM tag
                                                WHERE name=:name
                            )");
            queryTag.prepare(queryTagSQL);
            queryTag.bindValue(":name",mainWindow->newSearch->selectedTagName);
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
        if (mainWindow->newSearch->searchOnFileName==true){
            //Depends on the "Search in" criteria,
            //Reduces the abosulte path to the required text string and matches the search text
            if(mainWindow->newSearch->selectedSearchIn == QCoreApplication::translate("MainWindow", "File names only"))
            {
                match = regex.match(lineFileName);
            }
            else if(mainWindow->newSearch->selectedSearchIn == QCoreApplication::translate("MainWindow", "Folder path only"))
            {

                //Check that the folder name matches the search text
                regex.setPattern(mainWindow->newSearch->regexSearchtext);
                foldermatch = regex.match(lineFilePath);
                //If it does, then check that the file matches the selected file type
                if (foldermatch.hasMatch() and mainWindow->newSearch->searchOnType==true){
                    regex.setPattern(mainWindow->newSearch->regexFileType);
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
                mainWindow->newSearch->filesFoundList << lineFilePath;
                mainWindow->newSearch->deviceFoundIDList.insert(0,QString::number(device->ID));

                //Populate result lists
                mainWindow->newSearch->fileNames.append(lineFileName);
                mainWindow->newSearch->filePaths.append(lineFilePath);
                mainWindow->newSearch->fileSizes.append(getFilesQuery.value(2).toLongLong());
                mainWindow->newSearch->fileDateTimes.append(getFilesQuery.value(3).toString());
                mainWindow->newSearch->fileCatalogs.append(device->name);
            }
        }
        else{
            //verify file matches the selected file type
            if (mainWindow->newSearch->searchOnType==true){
                regex.setPattern(mainWindow->newSearch->regexFileType);
            }
            match = regex.match(lineFilePath);
            if (!match.hasMatch()){
                continue;
            }

            //Add the file and its catalog to the results, excluding blank lines
            if (lineFilePath !=""){
                mainWindow->newSearch->filesFoundList << lineFilePath;
                mainWindow->newSearch->deviceFoundIDList.insert(0, QString::number(device->ID));

                //Populate result lists
                mainWindow->newSearch->fileNames.append(lineFileName);
                mainWindow->newSearch->filePaths.append(lineFilePath);
                mainWindow->newSearch->fileSizes.append(getFilesQuery.value(2).toLongLong());
                mainWindow->newSearch->fileDateTimes.append(getFilesQuery.value(3).toString());
                mainWindow->newSearch->fileCatalogs.append(device->name);
            }
        }
    }
}

void SearchProcess::searchFilesInDirectory(const QString &sourceDirectory)
{
    // Directory search logic here
    // This example does not involve database operations
    QDir dir(sourceDirectory);
    if (!dir.exists()) {
        qDebug() << "WARNING/ searchFilesInDirectory: Directory does not exist: " << sourceDirectory;
        return;
    }

    //Run a search of files for the selected Directory
    //Define how to use the search text //COMMON to searchFilesInCatalog
    if(mainWindow->newSearch->selectedTextCriteria == QCoreApplication::translate("MainWindow", "Exact Phrase"))
        mainWindow->newSearch->regexSearchtext=mainWindow->newSearch->searchText; //just search for the extact text entered including spaces, as one text string.
    else if(mainWindow->newSearch->selectedTextCriteria == QCoreApplication::translate("MainWindow", "Begins With"))
        mainWindow->newSearch->regexSearchtext="(^"+mainWindow->newSearch->searchText+")";
    else if(mainWindow->newSearch->selectedTextCriteria == QCoreApplication::translate("MainWindow", "Any Word"))
        mainWindow->newSearch->regexSearchtext=mainWindow->newSearch->searchText.replace(" ","|");
    else if(mainWindow->newSearch->selectedTextCriteria == QCoreApplication::translate("MainWindow", "All Words")){
        QString searchTextToSplit = mainWindow->newSearch->searchText;
        QString groupRegEx = "";
        QRegularExpression lineSplitExp(" ");
        QStringList lineFieldList = searchTextToSplit.split(lineSplitExp);
        int numberOfSearchWords = lineFieldList.count();
        //Build regex group for one word
        for (int i=0; i<(numberOfSearchWords); i++){
            groupRegEx = groupRegEx + "(?=.*" + lineFieldList[i] + ")";
        }
        mainWindow->newSearch->regexSearchtext = groupRegEx;
    }
    else {
        mainWindow->newSearch->regexSearchtext="";
    }

    mainWindow->newSearch->regexPattern = mainWindow->newSearch->regexSearchtext;

    //Prepare the regexFileType for file types //COMMON to searchFilesInCatalog
    if ( mainWindow->newSearch->searchOnFileCriteria==true and mainWindow->newSearch->selectedFileType !=QCoreApplication::translate("MainWindow", "All")){
        //Get the list of file extension and join it into one string
        if(mainWindow->newSearch->selectedFileType ==QCoreApplication::translate("MainWindow", "Audio")){
            mainWindow->newSearch->regexFileType = mainWindow->fileType_AudioS.join("|");
        }
        if(mainWindow->newSearch->selectedFileType ==QCoreApplication::translate("MainWindow", "Image")){
            mainWindow->newSearch->regexFileType = mainWindow->fileType_ImageS.join("|");
        }
        if(mainWindow->newSearch->selectedFileType ==QCoreApplication::translate("MainWindow", "Text")){
            mainWindow->newSearch->regexFileType = mainWindow->fileType_TextS.join("|");
        }
        if(mainWindow->newSearch->selectedFileType ==QCoreApplication::translate("MainWindow", "Video")){
            mainWindow->newSearch->regexFileType = mainWindow->fileType_VideoS.join("|");
        }

        //Replace the *. by .* needed for regex
        mainWindow->newSearch->regexFileType = mainWindow->newSearch->regexFileType.replace("*.",".*");

        //Add the file type expression to the regex
        mainWindow->newSearch->regexPattern = mainWindow->newSearch->regexSearchtext  + "(" + mainWindow->newSearch->regexFileType + ")";
    }

    //Add the words to exclude to the regex //COMMON to searchFilesInCatalog

    if ( mainWindow->newSearch->selectedSearchExclude !=""){

        //Prepare
        QString searchTextToSplit = mainWindow->newSearch->selectedSearchExclude;
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
        mainWindow->newSearch->regexPattern = excludeGroupRegEx + mainWindow->newSearch->regexPattern;
    }

    QRegularExpression regex(mainWindow->newSearch->regexPattern, QRegularExpression::CaseInsensitiveOption);

    //Filetypes
    //Get the file type for the catalog
    QStringList fileTypes;

    //Scan directory and create a list of files
    QString line;
    QString reducedLine;

    QDirIterator iterator(sourceDirectory, fileTypes, QDir::Files|QDir::Hidden, QDirIterator::Subdirectories);
    while (iterator.hasNext() and stopRequested==false){

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
        if (mainWindow->newSearch->searchOnSize==true){
            if ( !(     lineFileSize >= mainWindow->newSearch->selectedMinimumSize * mainWindow->newSearch->sizeMultiplierMin
                  and lineFileSize <= mainWindow->newSearch->selectedMaximumSize * mainWindow->newSearch->sizeMultiplierMax) ){
                continue;}
        }

        //Continue if the file is matching the date range
        if (mainWindow->newSearch->searchOnDate==true){
            if ( !(     lineFileDateTime >= mainWindow->newSearch->selectedDateMin
                  and lineFileDateTime <= mainWindow->newSearch->selectedDateMax ) ){
                continue;}
        }

        //Continue if the file is matching the tags
        if (mainWindow->newSearch->searchOnTags==true){

            bool fileIsMatchingTag = false;

            //Set query to get a list of folder paths matching the selected tag
            QSqlQuery queryTag(QSqlDatabase::database("defaultConnection"));
            QString queryTagSQL = QLatin1String(R"(
                                                SELECT path
                                                FROM tag
                                                WHERE name=:name
                            )");
            queryTag.prepare(queryTagSQL);
            queryTag.bindValue(":name",mainWindow->newSearch->selectedTagName);
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
        if (mainWindow->newSearch->searchOnFileName==true){
            //Depending on the "Search in" criteria,
            //reduce the abosulte path to the reaquired text string and match the search text
            if(mainWindow->newSearch->selectedSearchIn == QCoreApplication::translate("MainWindow", "File names only"))
            {
                // Extract the file name from the lineFilePath
                QFileInfo file(lineFilePath);
                reducedLine = file.fileName();

                match = regex.match(reducedLine);
            }
            else if(mainWindow->newSearch->selectedSearchIn == QCoreApplication::translate("MainWindow", "Folder path only"))
            {
                //Keep only the folder name, so all characters left of the last occurence of / in the path.
                reducedLine = lineFilePath.left(lineFilePath.lastIndexOf("/"));

                //Check the fodler name matches the search text
                regex.setPattern(mainWindow->newSearch->regexSearchtext);

                foldermatch = regex.match(reducedLine);

                //if it does, then check that the file matches the selected file type
                if (foldermatch.hasMatch()){
                    regex.setPattern(mainWindow->newSearch->regexFileType);

                    match = regex.match(lineFilePath);
                }
            }
            else {
                match = regex.match(lineFilePath);
            }

            //If the file is matching the criteria, add it and its catalog to the search results
            //COMMON to searchFilesInCatalog
            if (match.hasMatch()){

                mainWindow->newSearch->filesFoundList << lineFilePath;

                //COMMON to searchFilesInCatalog
                //Retrieve other file info
                QFileInfo file(lineFilePath);

                // Get the fileDateTime from the list if available
                QString lineFileDatetime;
                if (fieldListCount == 3){
                    lineFileDatetime = lineFieldList[2];}
                else lineFileDatetime = "";

                //Populate result lists
                mainWindow->newSearch->fileNames.append(file.fileName());
                mainWindow->newSearch->filePaths.append(file.path());
                mainWindow->newSearch->fileSizes.append(lineFileSize);
                mainWindow->newSearch->fileDateTimes.append(lineFileDatetime);
                mainWindow->newSearch->fileCatalogs.append(sourceDirectory);
            }
        }
        else{

            //Add the file and its catalog to the results, excluding blank lines
            if (lineFilePath !=""){
                mainWindow->newSearch->filesFoundList << lineFilePath;
                mainWindow->newSearch->deviceFoundIDList.insert(0, sourceDirectory);

                //Retrieve other file info
                QFileInfo file(lineFilePath);

                // Get the fileDateTime from the list if available
                QString lineFileDatetime;
                if (fieldListCount == 3){
                    lineFileDatetime = lineFieldList[2];}
                else lineFileDatetime = "";

                //Populate result lists
                mainWindow->newSearch->fileNames.append(file.fileName());
                mainWindow->newSearch->filePaths.append(file.path());
                mainWindow->newSearch->fileSizes.append(lineFileSize);
                mainWindow->newSearch->fileDateTimes.append(lineFileDatetime);
                mainWindow->newSearch->fileCatalogs.append(sourceDirectory);
            }
        }
    }
}

void SearchProcess::processSearchResults()
{

    if (isStopped) {
        emit searchStopped();
        return;
    }

    // Implementation of processing search results

    //Process search results: list of catalogs with results
    //Remove duplicates so the catalogs are listed only once, and sort the list
    mainWindow->newSearch->deviceFoundIDList.removeDuplicates();
    mainWindow->newSearch->deviceFoundIDList.sort();

    //Keep the catalog file name only
    foreach(QString item, mainWindow->newSearch->deviceFoundIDList){
        int index = mainWindow->newSearch->deviceFoundIDList.indexOf(item);
        QFileInfo fileInfo(item);
        mainWindow->newSearch->deviceFoundIDList[index] = fileInfo.baseName();
    }

    //Create model and load to the view
    mainWindow->newSearch->deviceFoundModel = new QStandardItemModel;
    mainWindow->newSearch->deviceFoundModel->setHorizontalHeaderLabels({ "Catalog with results", "ID" });


    if (mainWindow->selectedDevice->type == "Catalog") {
        // If type = "Catalog", there is no sub-device, use it directly
        //searchFilesInCatalog(mainWindow->selectedDevice);
        QList<QStandardItem *> items;
        items << new QStandardItem(mainWindow->selectedDevice->name);
        items << new QStandardItem(QString::number(mainWindow->selectedDevice->ID));
        mainWindow->newSearch->deviceFoundModel->appendRow(items);
    }
    else {

        //Device loopDevice;
        for (const QString &ID : mainWindow->newSearch->deviceFoundIDList) {
            foreach (const Device &tempDevice, mainWindow->selectedDevice->subDevices) {
                qDebug() << "DEBUG: tempDevice.ID, type: " << tempDevice.ID << tempDevice.type;
                if (tempDevice.ID == ID.toInt()) {
                    QList<QStandardItem *> items;
                    items << new QStandardItem(tempDevice.name);
                    items << new QStandardItem(QString::number(tempDevice.ID));
                    mainWindow->newSearch->deviceFoundModel->appendRow(items);
                }
            }
        }
    }
    //OK
        //ui->Search_treeView_CatalogsFound->setModel(mainWindow->newSearch->deviceFoundModel);
        //ui->Search_treeView_CatalogsFound->hideColumn(1);

    //Process search results: list of files
    //Prepare query model
    QSqlQueryModel *loadCatalogQueryModel = new QSqlQueryModel;
    // Prepare model to display
    //FilesView *fileViewModel = new FilesView(this);

    //Populate model with folders only if this option is selected
    if ( mainWindow->newSearch->searchOnFolderCriteria==true and mainWindow->newSearch->showFoldersOnly==true )
    {
        mainWindow->newSearch->filePaths.removeDuplicates();
        int numberOfFolders = mainWindow->newSearch->filePaths.count();
        mainWindow->newSearch->fileNames.clear();
        mainWindow->newSearch->fileSizes.clear();
        mainWindow->newSearch->fileDateTimes.clear();
        mainWindow->newSearch->fileCatalogs.clear();
        for (int i=0; i<numberOfFolders; i++)
            mainWindow->newSearch->fileNames <<"";
        for (int i=0; i<numberOfFolders; i++)
            mainWindow->newSearch->fileSizes <<0;
        for (int i=0; i<numberOfFolders; i++)
            mainWindow->newSearch->fileDateTimes <<"";
        for (int i=0; i<numberOfFolders; i++)
            mainWindow->newSearch->fileCatalogs <<"";

        // Populate model with data
        fileViewModel->setSourceModel(mainWindow->newSearch);
        fileViewModel->setHeaderData(0, Qt::Horizontal, QCoreApplication::translate("MainWindow", "Name"));
        fileViewModel->setHeaderData(1, Qt::Horizontal, QCoreApplication::translate("MainWindow", "Size"));
        fileViewModel->setHeaderData(2, Qt::Horizontal, QCoreApplication::translate("MainWindow", "Date"));
        fileViewModel->setHeaderData(3, Qt::Horizontal, QCoreApplication::translate("MainWindow", "Folder"));
        fileViewModel->setHeaderData(4, Qt::Horizontal, QCoreApplication::translate("MainWindow", "Catalog"));

        //TEST
        // Connect model to treeview and display
        // ui->Search_treeView_FilesFound->setModel(fileViewModel); //TEST
        // ui->Search_treeView_FilesFound->header()->resizeSection(3, 400); //Path //TEST
        // ui->Search_treeView_FilesFound->header()->resizeSection(4, 100); //Catalog //TEST
        // ui->Search_treeView_FilesFound->header()->hideSection(0); //TEST
        // ui->Search_treeView_FilesFound->header()->hideSection(1); //TEST
        // ui->Search_treeView_FilesFound->header()->hideSection(2); //TEST

        // ui->Search_label_FoundTitle->setText(QCoreApplication::translate("MainWindow", "Folders found"));
    }

    //Populate model with files if the folder option is not selected
    else
    {
        // Populate model with data
        fileViewModel->setSourceModel(mainWindow->newSearch);
        fileViewModel->setHeaderData(0, Qt::Horizontal, QCoreApplication::translate("MainWindow", "Name"));
        fileViewModel->setHeaderData(1, Qt::Horizontal, QCoreApplication::translate("MainWindow", "Size"));
        fileViewModel->setHeaderData(2, Qt::Horizontal, QCoreApplication::translate("MainWindow", "Date"));
        fileViewModel->setHeaderData(3, Qt::Horizontal, QCoreApplication::translate("MainWindow", "Folder"));
        fileViewModel->setHeaderData(4, Qt::Horizontal, QCoreApplication::translate("MainWindow", "Catalog"));
        if (mainWindow->newSearch->searchInConnectedChecked == true){
            fileViewModel->setHeaderData(3, Qt::Horizontal, QCoreApplication::translate("MainWindow", "Search Directory"));
        }

        //TEST
        // Connect model to tree/table view
        // ui->Search_treeView_FilesFound->setModel(fileViewModel);
        // ui->Search_treeView_FilesFound->header()->setSectionResizeMode(QHeaderView::Interactive);
        // ui->Search_treeView_FilesFound->header()->resizeSection(0, 600); //Name
        // ui->Search_treeView_FilesFound->header()->resizeSection(1, 110); //Size
        // ui->Search_treeView_FilesFound->header()->resizeSection(2, 140); //Date
        // ui->Search_treeView_FilesFound->header()->resizeSection(3, 400); //Path
        // ui->Search_treeView_FilesFound->header()->resizeSection(4, 140); //Catalog
        // ui->Search_treeView_FilesFound->header()->showSection(0);
        // ui->Search_treeView_FilesFound->header()->showSection(1);
        // ui->Search_treeView_FilesFound->header()->showSection(2);

        // ui->Search_label_FoundTitle->setText(QCoreApplication::translate("MainWindow", "Files found"));
    }

    //Process DUPLICATES -------------------------------

    //Process if enabled and criteria are provided
    if ( mainWindow->newSearch->searchOnFileCriteria == true and mainWindow->newSearch->searchOnDuplicates == true
        and (     mainWindow->newSearch->searchDuplicatesOnName == true
             or mainWindow->newSearch->searchDuplicatesOnSize == true
             or mainWindow->newSearch->searchDuplicatesOnDate == true )){

        //Load Search results into the database
        //clear database
        QSqlQuery deleteQuery(QSqlDatabase::database("searchConnection"));
        deleteQuery.exec("DELETE FROM filetemp");

        //prepare query to load file info
        QSqlQuery insertQuery(QSqlDatabase::database("searchConnection"));
        QString insertSQL = QLatin1String(R"(
                                                        INSERT INTO filetemp (
                                                                        file_catalog_id,
                                                                        file_name,
                                                                        file_folder_path,
                                                                        file_size,
                                                                        file_date_updated,
                                                                        file_catalog )
                                                        VALUES(
                                                                        :file_catalog_id,
                                                                        :file_name,
                                                                        :file_folder_path,
                                                                        :file_size,
                                                                        :file_date_updated,
                                                                        :file_catalog )
                                                                    )");
        insertQuery.prepare(insertSQL);

        //loop through the result list and populate database

        int rows = mainWindow->newSearch->rowCount();

        for (int i=0; i<rows; i++) {

            //Append data to the database
            insertQuery.bindValue(":file_catalog_id",   mainWindow->newSearch->index(i,0).data().toString());
            insertQuery.bindValue(":file_name",         mainWindow->newSearch->index(i,0).data().toString());
            insertQuery.bindValue(":file_size",         mainWindow->newSearch->index(i,1).data().toString());
            insertQuery.bindValue(":file_folder_path",  mainWindow->newSearch->index(i,3).data().toString());
            insertQuery.bindValue(":file_date_updated", mainWindow->newSearch->index(i,2).data().toString());
            insertQuery.bindValue(":file_catalog",      mainWindow->newSearch->index(i,4).data().toString());
            insertQuery.exec();
        }

        //Prepare duplicate SQL
        // Load all files and create model
        QString selectSQL;

        //Generate grouping of fields based on user selection, determining what are duplicates
        QString groupingFields; // this value should be a concatenation of fields, like "fileName||fileSize"

        //Same name
        if(mainWindow->newSearch->searchDuplicatesOnName == true){
            groupingFields = groupingFields + "file_name";
        }
        //Same size
        if(mainWindow->newSearch->searchDuplicatesOnSize == true){
            groupingFields = groupingFields + "||file_size";
        }
        //Same date modified
        if(mainWindow->newSearch->searchDuplicatesOnDate == true){
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
                                                            file_catalog
                                                FROM filetemp
                                                WHERE %1 IN
                                                    (SELECT %1
                                                    FROM filetemp
                                                    GROUP BY %1
                                                    HAVING count(%1)>1)
                                                ORDER BY %1
                                            )").arg(groupingFields);

        //Run Query and load to model
        QSqlQuery duplicatesQuery(QSqlDatabase::database("searchConnection"));
        duplicatesQuery.prepare(selectSQL);
        duplicatesQuery.exec();

        //recapture file results for Stats
        mainWindow->newSearch->fileNames.clear();
        mainWindow->newSearch->fileSizes.clear();
        mainWindow->newSearch->filePaths.clear();
        mainWindow->newSearch->fileDateTimes.clear();
        mainWindow->newSearch->fileCatalogs.clear();
        while(duplicatesQuery.next()){
            mainWindow->newSearch->fileNames.append(duplicatesQuery.value(0).toString());
            mainWindow->newSearch->fileSizes.append(duplicatesQuery.value(1).toLongLong());
            mainWindow->newSearch->fileDateTimes.append(duplicatesQuery.value(2).toString());
            mainWindow->newSearch->filePaths.append(duplicatesQuery.value(3).toString());
            mainWindow->newSearch->fileCatalogs.append(duplicatesQuery.value(4).toString());
        }

        //Load results to model
        loadCatalogQueryModel->setQuery(std::move(duplicatesQuery));

        //FilesView *fileViewModel = new FilesView(this);
        fileViewModel->setSourceModel(loadCatalogQueryModel);
        fileViewModel->setHeaderData(0, Qt::Horizontal, QCoreApplication::translate("MainWindow", "Name"));
        fileViewModel->setHeaderData(1, Qt::Horizontal, QCoreApplication::translate("MainWindow", "Size"));
        fileViewModel->setHeaderData(2, Qt::Horizontal, QCoreApplication::translate("MainWindow", "Date"));
        fileViewModel->setHeaderData(3, Qt::Horizontal, QCoreApplication::translate("MainWindow", "Folder"));
        fileViewModel->setHeaderData(4, Qt::Horizontal, QCoreApplication::translate("MainWindow", "Catalog"));

        // Connect model to tree/table view
        // ui->Search_treeView_FilesFound->setModel(fileViewModel);
        // ui->Search_treeView_FilesFound->header()->setSectionResizeMode(QHeaderView::Interactive);
        // ui->Search_treeView_FilesFound->header()->resizeSection(0, 600); //Name
        // ui->Search_treeView_FilesFound->header()->resizeSection(1, 110); //Size
        // ui->Search_treeView_FilesFound->header()->resizeSection(2, 140); //Date
        // ui->Search_treeView_FilesFound->header()->resizeSection(3, 400); //Path
        // ui->Search_treeView_FilesFound->header()->resizeSection(4, 100); //Catalog

        // ui->Search_label_FoundTitle->setText(QCoreApplication::translate("MainWindow", "Duplicates found"));
        mainWindow->newSearch->filesFoundNumber = fileViewModel->rowCount();

    }

    //Process DIFFERENCES -------------------------------

    //Process if enabled and criteria are provided
    if ( mainWindow->newSearch->searchOnFileCriteria == true and mainWindow->newSearch->searchOnDifferences == true
        and (   mainWindow->newSearch->differencesOnName == true
             or mainWindow->newSearch->differencesOnSize == true
             or mainWindow->newSearch->differencesOnDate == true)){

        //Load Search results into the database
        //Clear database
        QSqlQuery deleteQuery(QSqlDatabase::database("searchConnection"));
        deleteQuery.exec("DELETE FROM filetemp");

        //Prepare query to load file info
        QSqlQuery insertQuery(QSqlDatabase::database("searchConnection"));
        QString insertSQL = QLatin1String(R"(
                                                        INSERT INTO filetemp (
                                                                        file_name,
                                                                        file_folder_path,
                                                                        file_size,
                                                                        file_date_updated,
                                                                        file_catalog )
                                                        VALUES(
                                                                        :file_name,
                                                                        :file_folder_path,
                                                                        :file_size,
                                                                        :file_date_updated,
                                                                        :file_catalog )
                                                                    )");
        insertQuery.prepare(insertSQL);

        //Loop through the result list and populate database

        int rows = mainWindow->newSearch->rowCount();

        for (int i=0; i<rows; i++) {

            QString test = mainWindow->newSearch->index(i,0).data().toString();

            //Append data to the database
            insertQuery.bindValue(":file_name",        mainWindow->newSearch->index(i,0).data().toString());
            insertQuery.bindValue(":file_size",        mainWindow->newSearch->index(i,1).data().toString());
            insertQuery.bindValue(":file_folder_path", mainWindow->newSearch->index(i,3).data().toString());
            insertQuery.bindValue(":file_date_updated",mainWindow->newSearch->index(i,2).data().toString());
            insertQuery.bindValue(":file_catalog",     mainWindow->newSearch->index(i,4).data().toString());
            insertQuery.exec();

        }

        //Prepare difference SQL
        // Load all files and create model
        QString selectSQL;

        //Generate grouping of fields based on user selection, determining what are duplicates
        QString groupingFieldsDifferences; // this value should be a concatenation of fields, like "fileName||fileSize"

        //Same name
        if(mainWindow->newSearch->differencesOnName == true){
            groupingFieldsDifferences += "||file_name";
        }
        //Same size
        if(mainWindow->newSearch->differencesOnSize == true){
            groupingFieldsDifferences += "||file_size";
        }
        //Same date modified
        if(mainWindow->newSearch->differencesOnDate == true){
            groupingFieldsDifferences += "||file_date_updated";
        }

        //Remove the || at the start
        if (groupingFieldsDifferences.startsWith("||"))
            groupingFieldsDifferences.remove(0, 2);

        //Generate SQL based on grouping of fields
        selectSQL = QLatin1String(R"(
                                                 SELECT      file_name,
                                                             file_size,
                                                             file_date_updated,
                                                             file_folder_path,
                                                             file_catalog
                                                 FROM filetemp
                                                 WHERE file_catalog = :selectedDifferencesCatalog1
                                                 AND %1 NOT IN(
                                                     SELECT %1
                                                     FROM filetemp
                                                     WHERE file_catalog = :selectedDifferencesCatalog2
                                                     )
                                                 UNION
                                                 SELECT      file_name,
                                                             file_size,
                                                             file_date_updated,
                                                             file_folder_path,
                                                             file_catalog
                                                 FROM filetemp
                                                 WHERE file_catalog = :selectedDifferencesCatalog2
                                                 AND %1 NOT IN(
                                                     SELECT %1
                                                     FROM filetemp
                                                     WHERE file_catalog = :selectedDifferencesCatalog1
                                                     )
                                 )").arg(groupingFieldsDifferences);

        //Run Query and load to model
        QSqlQuery differencesQuery(QSqlDatabase::database("searchConnection"));
        differencesQuery.prepare(selectSQL);
        differencesQuery.bindValue(":selectedDifferencesCatalog1",mainWindow->newSearch->differencesCatalog1);
        differencesQuery.bindValue(":selectedDifferencesCatalog2",mainWindow->newSearch->differencesCatalog2);
        differencesQuery.exec();

        //recapture file results for Stats
        mainWindow->newSearch->fileNames.clear();
        mainWindow->newSearch->fileSizes.clear();
        mainWindow->newSearch->filePaths.clear();
        mainWindow->newSearch->fileDateTimes.clear();
        mainWindow->newSearch->fileCatalogs.clear();
        while(differencesQuery.next()){
            mainWindow->newSearch->fileNames.append(differencesQuery.value(0).toString());
            mainWindow->newSearch->fileSizes.append(differencesQuery.value(1).toLongLong());
            mainWindow->newSearch->fileDateTimes.append(differencesQuery.value(2).toString());
            mainWindow->newSearch->filePaths.append(differencesQuery.value(3).toString());
            mainWindow->newSearch->fileCatalogs.append(differencesQuery.value(4).toString());
        }

        loadCatalogQueryModel->setQuery(std::move(differencesQuery));

        //FilesView *fileViewModel = new FilesView(this);
        fileViewModel->setSourceModel(loadCatalogQueryModel);
        fileViewModel->setHeaderData(0, Qt::Horizontal, QCoreApplication::translate("MainWindow", "Name"));
        fileViewModel->setHeaderData(1, Qt::Horizontal, QCoreApplication::translate("MainWindow", "Size"));
        fileViewModel->setHeaderData(2, Qt::Horizontal, QCoreApplication::translate("MainWindow", "Date"));
        fileViewModel->setHeaderData(3, Qt::Horizontal, QCoreApplication::translate("MainWindow", "Folder"));
        fileViewModel->setHeaderData(4, Qt::Horizontal, QCoreApplication::translate("MainWindow", "Catalog"));

        // // Connect model to tree/table view
        // ui->Search_treeView_FilesFound->setModel(fileViewModel);
        // ui->Search_treeView_FilesFound->header()->setSectionResizeMode(QHeaderView::Interactive);
        // ui->Search_treeView_FilesFound->header()->resizeSection(0, 600); //Name
        // ui->Search_treeView_FilesFound->header()->resizeSection(1, 110); //Size
        // ui->Search_treeView_FilesFound->header()->resizeSection(2, 140); //Date
        // ui->Search_treeView_FilesFound->header()->resizeSection(3, 400); //Path
        // ui->Search_treeView_FilesFound->header()->resizeSection(4, 100); //Catalog

        // // Display count of files
        // ui->Search_label_FoundTitle->setText(QCoreApplication::translate("MainWindow", "Duplicates found"));

    }

    //Files found Statistics
    //Reset from previous search
    mainWindow->newSearch->filesFoundNumber = 0;
    mainWindow->newSearch->filesFoundTotalSize = 0;
    mainWindow->newSearch->filesFoundAverageSize = 0;
    mainWindow->newSearch->filesFoundMinSize = 0;
    mainWindow->newSearch->filesFoundMaxSize = 0;
    mainWindow->newSearch->filesFoundMinDate = "";
    mainWindow->newSearch->filesFoundMaxDate = "";

    //Number of files found
    mainWindow->newSearch->filesFoundNumber = fileViewModel->rowCount();

    //TEST
    // ui->Search_label_NumberResults->setText(QString::number(mainWindow->newSearch->filesFoundNumber));

    //Total size of files found
    qint64 sizeItem;
    mainWindow->newSearch->filesFoundTotalSize = 0;
    foreach (sizeItem, mainWindow->newSearch->fileSizes) {
        mainWindow->newSearch->filesFoundTotalSize = mainWindow->newSearch->filesFoundTotalSize + sizeItem;
    }
    // ui->Search_label_SizeResults->setText(QLocale().formattedDataSize(mainWindow->newSearch->filesFoundTotalSize));

    // //Other statistics, covering the case where no results are returned.
    // if (mainWindow->newSearch->filesFoundNumber !=0){
    //     mainWindow->newSearch->filesFoundAverageSize = mainWindow->newSearch->filesFoundTotalSize / mainWindow->newSearch->filesFoundNumber;
    //     QList<qint64> fileSizeList = mainWindow->newSearch->fileSizes;
    //     std::sort(fileSizeList.begin(), fileSizeList.end());
    //     mainWindow->newSearch->filesFoundMinSize = fileSizeList.first();
    //     mainWindow->newSearch->filesFoundMaxSize = fileSizeList.last();

    //     QList<QString> fileDateList = mainWindow->newSearch->fileDateTimes;
    //     std::sort(fileDateList.begin(), fileDateList.end());
    //     mainWindow->newSearch->filesFoundMinDate = fileDateList.first();
    //     mainWindow->newSearch->filesFoundMaxDate = fileDateList.last();

    //     // ui->Search_pushButton_FileFoundMoreStatistics->setEnabled(true);
    // }

    //TEST
    //Save the search criteria to the search history
    //mainWindow->newSearch->insertSearchHistoryToTable("searchConnection");
    //collection->saveSearchHistoryTableToFile();
    //loadSearchHistoryTableToModel();

    //QApplication::restoreOverrideCursor();

    // Emit signal when processing is complete
    emit searchResultsReady();

}

