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
    qDebug() << "";
    qDebug() << "DEBUG: SearchProcess::run()" << isStopped << stopRequested << mainWindow->selectedDevice->name;
    QApplication::setOverrideCursor(Qt::WaitCursor);

    //Initialize the database connection within the SearchProcess thread
    QString connectionName = "searchConnection";
    //QString connectionName = "searchConnection" + QString::number(reinterpret_cast<quintptr>(QThread::currentThreadId()));
    //qDebug() << "DEBUG: connectionName" << connectionName;

    // Initialize the database connection within the SearchProcess thread
    QSqlError dbError = mainWindow->initializeDatabase(connectionName);
    if (dbError.type() != QSqlError::NoError) {
        qWarning() << "DEBUG: SearchProcess::run() Failed to initialize database: " << dbError.text();
        emit searchStopped();
        return;
    }

    //Searching "Begin With" for "File name or Folder name" is not supported yet
    if (    mainWindow->newSearch->selectedTextCriteria==tr("Begins With")
        and mainWindow->newSearch->selectedSearchIn !=tr("File names only")){
        emit searchStopped();
        QApplication::restoreOverrideCursor(); //Stop animation
        QMessageBox::information(mainWindow,"Katalog",tr("The option 'Begin With' can only be used with 'File names only'.\nUse a different combinaison."));
        return;
    }

    qWarning() << "DEBUG: SearchProcess::run() device: " << mainWindow->selectedDevice->name;

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
                //qDebug() << "DEBUG: search in the list of catalogs in the selectedDevice.";
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
    //qDebug() << "DEBUG: emit searchCompleted()";
}

void SearchProcess::stop()
{
    QMutexLocker locker(&mutex);
    stopRequested = true;
    isStopped = true;
    emit searchStopped();
    //qDebug() << "DEBUG: emit searchStopped()";
}

void SearchProcess::searchFilesInCatalog(const Device *device)
{//Run a search of files for the selected Catalog
    //Prepare Inputs including Regular Expression

    QFile catalogFile(device->catalog->sourcePath);

    QRegularExpressionMatch match;
    QRegularExpressionMatch foldermatch;

    //Define how to use the search text
    if(mainWindow->newSearch->selectedTextCriteria == tr("Exact Phrase"))
        mainWindow->newSearch->regexSearchtext=mainWindow->newSearch->searchText; //just search for the extact text entered including spaces, as one text string.
    else if(mainWindow->newSearch->selectedTextCriteria == tr("Begins With"))
        mainWindow->newSearch->regexSearchtext="(^"+mainWindow->newSearch->searchText+")";
    else if(mainWindow->newSearch->selectedTextCriteria == tr("Any Word"))
        mainWindow->newSearch->regexSearchtext=mainWindow->newSearch->searchText.replace(" ","|");
    else if(mainWindow->newSearch->selectedTextCriteria == tr("All Words")){
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

//qDebug()<<"Search in catalog prepared";
    //Load the catalog file contents if not already loaded in memory
    //device->catalog->loadCatalogFileListToTable("searchConnection", mutex, stopRequested);
    if(databaseMode=="Memory"){
        qDebug()<<"DEBUG: loadCatalogFileListToTable starting on: " << device->name;
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
qDebug()<<"DEBUG: stopRequested: " << stopRequested;
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
            qDebug()<<"DEBUG: test Tag query: " << queryTag.lastError();

            //Test if the FilePath contains a path from the list of folders matching the selected tag name
            // a slash "/" is added at the end of both values to ensure no result from a tag "AB" is returned when a tag "A" is selected
            while(queryTag.next()){
                qDebug()<<"DEBUG: test Tag: " << queryTag.value(0).toString();

                if ( (lineFilePath+"/").contains( queryTag.value(0).toString()+"/" )==true){
                    fileIsMatchingTag = true;
                    qDebug()<<"DEBUG: test Tag: " << fileIsMatchingTag;

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
            if(mainWindow->newSearch->selectedSearchIn == tr("File names only"))
            {
                match = regex.match(lineFileName);
            }
            else if(mainWindow->newSearch->selectedSearchIn == tr("Folder path only"))
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
                    match = foldermatch; //selectedSearchIn == tr("Files and Folder paths")
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
    qDebug() << "DEBUG: Searching: searchFilesInCatalog executed.";
}

void SearchProcess::searchFilesInDirectory(const QString &directory)
{
    // Directory search logic here
    // This example does not involve database operations
    QDir dir(directory);
    if (!dir.exists()) {
        qWarning() << "Directory does not exist: " << directory;
        return;
    }

    QFileInfoList fileList = dir.entryInfoList(QDir::Files | QDir::NoSymLinks);
    foreach (const QFileInfo &fileInfo, fileList) {
        if (isStopped) {
            break;
        }
        QString fileName = fileInfo.fileName();
        qDebug() << "Found file in directory: " << fileName;
        // Add the file to your search results
    }
}

void SearchProcess::processSearchResults()
{

    if (isStopped) {
        emit searchStopped();
        return;
    }

    qDebug() << "DEBUG: Searching: processSearchResults starting.";

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
        fileViewModel->setHeaderData(0, Qt::Horizontal, tr("Name"));
        fileViewModel->setHeaderData(1, Qt::Horizontal, tr("Size"));
        fileViewModel->setHeaderData(2, Qt::Horizontal, tr("Date"));
        fileViewModel->setHeaderData(3, Qt::Horizontal, tr("Folder"));
        fileViewModel->setHeaderData(4, Qt::Horizontal, tr("Catalog"));

        //TEST
        // Connect model to treeview and display
        // ui->Search_treeView_FilesFound->setModel(fileViewModel); //TEST
        // ui->Search_treeView_FilesFound->header()->resizeSection(3, 400); //Path //TEST
        // ui->Search_treeView_FilesFound->header()->resizeSection(4, 100); //Catalog //TEST
        // ui->Search_treeView_FilesFound->header()->hideSection(0); //TEST
        // ui->Search_treeView_FilesFound->header()->hideSection(1); //TEST
        // ui->Search_treeView_FilesFound->header()->hideSection(2); //TEST

        // ui->Search_label_FoundTitle->setText(tr("Folders found"));
    }

    //Populate model with files if the folder option is not selected
    else
    {
        // Populate model with data
        fileViewModel->setSourceModel(mainWindow->newSearch);
        fileViewModel->setHeaderData(0, Qt::Horizontal, tr("Name"));
        fileViewModel->setHeaderData(1, Qt::Horizontal, tr("Size"));
        fileViewModel->setHeaderData(2, Qt::Horizontal, tr("Date"));
        fileViewModel->setHeaderData(3, Qt::Horizontal, tr("Folder"));
        fileViewModel->setHeaderData(4, Qt::Horizontal, tr("Catalog"));
        if (mainWindow->newSearch->searchInConnectedChecked == true){
            fileViewModel->setHeaderData(3, Qt::Horizontal, tr("Search Directory"));
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

        // ui->Search_label_FoundTitle->setText(tr("Files found"));
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
        fileViewModel->setHeaderData(0, Qt::Horizontal, tr("Name"));
        fileViewModel->setHeaderData(1, Qt::Horizontal, tr("Size"));
        fileViewModel->setHeaderData(2, Qt::Horizontal, tr("Date"));
        fileViewModel->setHeaderData(3, Qt::Horizontal, tr("Folder"));
        fileViewModel->setHeaderData(4, Qt::Horizontal, tr("Catalog"));

        // Connect model to tree/table view
        // ui->Search_treeView_FilesFound->setModel(fileViewModel);
        // ui->Search_treeView_FilesFound->header()->setSectionResizeMode(QHeaderView::Interactive);
        // ui->Search_treeView_FilesFound->header()->resizeSection(0, 600); //Name
        // ui->Search_treeView_FilesFound->header()->resizeSection(1, 110); //Size
        // ui->Search_treeView_FilesFound->header()->resizeSection(2, 140); //Date
        // ui->Search_treeView_FilesFound->header()->resizeSection(3, 400); //Path
        // ui->Search_treeView_FilesFound->header()->resizeSection(4, 100); //Catalog

        // ui->Search_label_FoundTitle->setText(tr("Duplicates found"));
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
        fileViewModel->setHeaderData(0, Qt::Horizontal, tr("Name"));
        fileViewModel->setHeaderData(1, Qt::Horizontal, tr("Size"));
        fileViewModel->setHeaderData(2, Qt::Horizontal, tr("Date"));
        fileViewModel->setHeaderData(3, Qt::Horizontal, tr("Folder"));
        fileViewModel->setHeaderData(4, Qt::Horizontal, tr("Catalog"));

        // // Connect model to tree/table view
        // ui->Search_treeView_FilesFound->setModel(fileViewModel);
        // ui->Search_treeView_FilesFound->header()->setSectionResizeMode(QHeaderView::Interactive);
        // ui->Search_treeView_FilesFound->header()->resizeSection(0, 600); //Name
        // ui->Search_treeView_FilesFound->header()->resizeSection(1, 110); //Size
        // ui->Search_treeView_FilesFound->header()->resizeSection(2, 140); //Date
        // ui->Search_treeView_FilesFound->header()->resizeSection(3, 400); //Path
        // ui->Search_treeView_FilesFound->header()->resizeSection(4, 100); //Catalog

        // // Display count of files
        // ui->Search_label_FoundTitle->setText(tr("Duplicates found"));

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

