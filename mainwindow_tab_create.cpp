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
// File Name:   mainwindow_tab_create.cpp
// Purpose:     methods for the screen CREATE
// Description: https://github.com/StephaneCouturier/Katalog/wiki/Create
// Author:      Stephane Couturier
/////////////////////////////////////////////////////////////////////////////
*/

#include "mainwindow.h"
#include "ui_mainwindow.h"

//UI----------------------------------------------------------------------------

    void MainWindow::on_Create_treeView_Explorer_clicked(const QModelIndex &index)
    {//Get the selected folder from the tree to the new Catalog path

        //Get the model/data from the tree
        QFileSystemModel* pathmodel = (QFileSystemModel*)ui->Create_treeView_Explorer->model();
        //get data from the selected file/directory
        QFileInfo fileInfo = pathmodel->fileInfo(index);
        //send the path to the line edit
        ui->Create_lineEdit_NewCatalogPath->setText(fileInfo.filePath());
    }
    //--------------------------------------------------------------------------
    void MainWindow::on_Create_pushButton_PickPath_clicked()
    {//Pick a directory from a dialog window

        //Get current selected path as default path for the dialog window
        QString newSelectedPath = ui->Create_lineEdit_NewCatalogPath->text();
        //newDevice->catalog->setSourcePath(ui->Create_lineEdit_NewCatalogPath->text());

        //Open a dialog for the user to select the directory to be cataloged. Only show directories.
        QString dir = QFileDialog::getExistingDirectory(this, tr("Select the directory to be cataloged in this new catalog"),
                                                        newSelectedPath,
                                                        QFileDialog::ShowDirsOnly
                                                        | QFileDialog::DontResolveSymlinks);
        //Save selected directory, and update input line for the source path
        //newDevice->catalog->setSourcePath(dir);
        ui->Create_lineEdit_NewCatalogPath->setText(newSelectedPath);

        //Select this directory in the treeview.
        loadFileSystem(newSelectedPath);
    }
    //--------------------------------------------------------------------------
    void MainWindow::on_Create_pushButton_EditExcludeList_clicked()
    {//Edit Exclusion list

        //Verify if a folder exclusion list exists
        QFile excludeFile(excludeFilePath);
        if ( excludeFile.exists()){
            QDesktopServices::openUrl(QUrl::fromLocalFile(excludeFilePath));
        }
        else{
            //if not, propose to create it
            int result = QMessageBox::warning(this,tr("Update"),tr("No list found, create one?"),
                                              QMessageBox::Yes | QMessageBox::Cancel);

            if ( result == QMessageBox::Cancel){
                return;
            }
            else{
                // Create it, if it does not exist
                if(!excludeFile.open(QIODevice::ReadOnly)) {

                    if (excludeFile.open(QFile::WriteOnly | QFile::Text)) {

                          QTextStream stream(&excludeFile);

                          //insert one line as an example
                          stream << tr("folder/path/without/slash_at_the_end");

                          excludeFile.close();

                          //and open it for edition
                          QDesktopServices::openUrl(QUrl::fromLocalFile(excludeFilePath));
                    }
                }
            }
        }
    }
    //--------------------------------------------------------------------------
    void MainWindow::on_Create_pushButton_AddStorage_clicked()
    {
        //Change tab to show the screen to add a storage
        ui->tabWidget->setCurrentIndex(4); // tab 1 is the Collection tab
    }
    //--------------------------------------------------------------------------
    void MainWindow::on_Create_pushButton_GenerateFromPath_clicked()
    {//Generate the Catalog name from the path

        QString newCatalogName = ui->Create_lineEdit_NewCatalogPath->text();
        newCatalogName.replace("/","_");
        newCatalogName.replace(":","_");
        ui->Create_lineEdit_NewCatalogName->setText(newCatalogName);
    }
    //--------------------------------------------------------------------------
    void MainWindow::on_Create_pushButton_CreateCatalog_clicked()
    {
        createCatalog();
    }
    //--------------------------------------------------------------------------

//Methods-----------------------------------------------------------------------
    void MainWindow::loadFileSystem(QString newCatalogPath)
    {//Load file system to the Create and the Filter for connected devices treeviews

        //Create a new model, only directories, and set root path
        fileSystemModel = new QFileSystemModel(this);
        fileSystemModel->setFilter(QDir::NoDotAndDotDot | QDir::AllDirs);
        fileSystemModel->setRootPath(newCatalogPath);
        fileSystemModel->sort(0,Qt::AscendingOrder);

        //Load File System to the Create and the Filter treeviews
            //Load File System to the Filter connected device treeview
            ui->Create_treeView_Explorer->setModel(fileSystemModel);
            // Only show the tree, hidding other columns and the header row.
            ui->Create_treeView_Explorer->setColumnWidth(0,250);
            ui->Create_treeView_Explorer->setColumnHidden(1,true);
            ui->Create_treeView_Explorer->setColumnHidden(2,true);
            ui->Create_treeView_Explorer->setColumnHidden(3,true);
            ui->Create_treeView_Explorer->setHeaderHidden(true);
            ui->Create_treeView_Explorer->expandToDepth(10);

            //Load File System to the Filter tab treeview
            ui->Filters_treeView_Directory->setModel(fileSystemModel);
            // Only show the tree, hidding other columns and the header row.
            ui->Filters_treeView_Directory->setColumnWidth(0,250);
            ui->Filters_treeView_Directory->setColumnHidden(1,true);
            ui->Filters_treeView_Directory->setColumnHidden(2,true);
            ui->Filters_treeView_Directory->setColumnHidden(3,true);
            ui->Filters_treeView_Directory->setHeaderHidden(true);
            ui->Filters_treeView_Directory->expandToDepth(1);
    }
    //--------------------------------------------------------------------------
    void MainWindow::createCatalog()
    {//Create a new catalog, launch the cataloging and save, and refresh data and UI

        //Check if mandatory inputs are provided
        if (ui->Create_lineEdit_NewCatalogName->text() == ""){
            QMessageBox msgBox;
            msgBox.setWindowTitle("Katalog");
            msgBox.setText(tr("Provide a name for this new catalog.<br/>"));
            msgBox.setIcon(QMessageBox::Warning);
            msgBox.exec();
            return;
        }
        if (ui->Create_lineEdit_NewCatalogPath->text() == ""){
            QMessageBox msgBox;
            msgBox.setWindowTitle("Katalog");
            msgBox.setText(tr("Provide a path for this new catalog.<br/>"));
            msgBox.setIcon(QMessageBox::Warning);
            msgBox.exec();
            return;
        }

        //Create a new device and catalog

            //Add Device entry and associate to the new catalog and its parent
            Device *newDevice = new Device();
            newDevice->generateDeviceID();
            newDevice->type = "Catalog";
            newDevice->name = ui->Create_lineEdit_NewCatalogName->text();

            //Check if the catalog name (so the csv file name) already exists
            if (newDevice->catalog->catalogNameExists()){
                QMessageBox msgBox;
                msgBox.setWindowTitle("Katalog");
                msgBox.setText( tr("There is already a catalog with this name:<br/><b>")
                               + newDevice->catalog->name
                               + "</b><br/><br/>"+tr("Choose a different name and try again."));
                msgBox.setIcon(QMessageBox::Critical);
                msgBox.exec();
                return;
            }

            //Continue populating values
            newDevice->parentID = ui->Create_comboBox_StorageSelection->currentData().toInt();
            newDevice->catalog->generateID();
            newDevice->externalID = newDevice->catalog->ID;
            newDevice->groupID = 0; //ADAPT
            newDevice->insertDevice();

            //Get inputs and set values of the newCatalog
            newDevice->catalog->name = newDevice->name; //REMOVE
            newDevice->catalog->filePath = collectionFolder + "/" + newDevice->catalog->name + ".idx";
            newDevice->catalog->sourcePath = ui->Create_lineEdit_NewCatalogPath->text();
            newDevice->catalog->includeHidden = ui->Create_checkBox_IncludeHidden->isChecked();
            newDevice->catalog->storageName = ui->Create_comboBox_StorageSelection->currentText(); //REMOVE
            newDevice->catalog->includeSymblinks = ui->Create_checkBox_IncludeSymblinks->isChecked();
            newDevice->catalog->isFullDevice = ui->Create_checkBox_isFullDevice->isChecked();
            newDevice->catalog->includeMetadata = ui->Create_checkBox_IncludeMetadata->isChecked();
            newDevice->catalog->appVersion = currentVersion;

            //Get the file type for the catalog
            if      ( ui->Create_radioButton_FileType_Image->isChecked() ){
                    newDevice->catalog->fileType = "Image";}
            else if ( ui->Create_radioButton_FileType_Audio->isChecked() ){
                    newDevice->catalog->fileType = "Audio";}
            else if ( ui->Create_radioButton_FileType_Video->isChecked() ){
                    newDevice->catalog->fileType = "Video";}
            else if ( ui->Create_radioButton_FileType_Text->isChecked() ){
                    newDevice->catalog->fileType = "Text";}
            else
                    newDevice->catalog->fileType = "All";

            //Save new catalog
            newDevice->catalog->insertCatalog();

            //Save data to file
            saveDeviceTableToFile(deviceFilePath);

            //Reload
            loadDeviceTableToTreeModel();
            loadStorageList();

        //Launch the scan and cataloging of files
            requestSource = "create";
            updateSingleCatalog(newDevice->catalog, true);

            //Check if no files where found, and let the user decide what to do
            // Get the catalog file list
            QStringList filelist = fileListModel->stringList();
            if (filelist.count() == 5){ //the CatalogDirectory method always adds lines for the catalog metadata, they should be ignored
                int result = QMessageBox::warning(this, "Katalog - Warning",
                                    tr("The source folder does not contain any file.\n"
                                         "This could mean that the source is empty or the device is not mounted to this folder.\n")
                                         +tr("Do you want to save it anyway (the catalog would be empty)?\n"), QMessageBox::Yes
                                                  | QMessageBox::Cancel);
                if ( result != QMessageBox::Cancel){
                    return;
                }
            }

            //Update the new catalog loadedversion to indicate that files are already in memory
            QDateTime emptyDateTime = *new QDateTime;
            newDevice->catalog->setDateLoaded(emptyDateTime);

        //Refresh data and UI
            //Refresh the catalog list for the Search screen
            loadCatalogFilesToTable();

            //Refresh the catalog list for the combobox of the Search screen
            refreshDifferencesCatalogSelection();

            //Refresh Catalogs list
                updateAllCatalogPathIsActive();
                synchCatalogAndStorageValues();
                loadCatalogsTableToModel();
                loadDeviceTableToTreeModel();

            //Restore selected catalog
            ui->Filters_label_DisplayCatalog->setText(ui->Filters_label_DisplayCatalog->text());

            //Refresh filter tree
            loadDeviceFileToTable();
            loadDeviceTableToTreeModel();

            //Change tab to show the result of the catalog creation
            ui->tabWidget->setCurrentIndex(1); // tab 1 is the Collection tab

            //Disable buttons to force user to select a catalog
            ui->Catalogs_pushButton_Search->setEnabled(false);
            ui->Catalogs_pushButton_ExploreCatalog->setEnabled(false);
            ui->Catalogs_pushButton_EditCatalogFile->setEnabled(false);
            ui->Catalogs_pushButton_UpdateCatalog->setEnabled(false);
            ui->Catalogs_pushButton_ViewCatalogStats->setEnabled(false);
            ui->Catalogs_pushButton_DeleteCatalog->setEnabled(false);
    }
    //--------------------------------------------------------------------------
    void MainWindow::catalogDirectory(Catalog *catalog)
    {
        //Catalog the files of a directory and add catalog meta-data
            // Start animation while cataloging
            QApplication::setOverrideCursor(Qt::WaitCursor);

        //Prepare inputs
            //Define the extensions of files to be included
            QStringList fileExtensions;
            if      ( catalog->fileType == "Image")
                                    fileExtensions = fileType_Image;
            else if ( catalog->fileType == "Audio")
                                    fileExtensions = fileType_Audio;
            else if ( catalog->fileType == "Video")
                                    fileExtensions = fileType_Video;
            else if ( catalog->fileType == "Text")
                                    fileExtensions = fileType_Text;

            // Get directories to exclude
            QStringList excludedFolders;
            QFile excludeFile(excludeFilePath);
            if(excludeFile.open(QIODevice::ReadOnly)) {
                QTextStream textStream(&excludeFile);
                QString line;
                while (true)
                {
                    line = textStream.readLine();
                    if (line.isNull())
                    break;
                    else
                    excludedFolders << line;
                }
                excludeFile.close();
            }

        //Prepare database and queries

            //Remove any former files from db for older catalog with same name
            QSqlQuery deleteFileQuery;
            QString deleteFileQuerySQL = QLatin1String(R"(
                                            DELETE FROM file
                                            WHERE file_catalog=:file_catalog
                                        )");
            deleteFileQuery.prepare(deleteFileQuerySQL);
            deleteFileQuery.bindValue(":file_catalog",catalog->name);
            deleteFileQuery.exec();

            QSqlQuery deleteFolderQuery;
            QString deleteFolderQuerySQL = QLatin1String(R"(
                                            DELETE FROM folder
                                            WHERE folder_catalog_name=:folder_catalog_name
                                        )");
            deleteFolderQuery.prepare(deleteFolderQuerySQL);
            deleteFolderQuery.bindValue(":folder_catalog_name",catalog->name);
            deleteFolderQuery.exec();

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
            insertFileQuery.prepare(insertFileSQL);

            //prepare insert query for folder
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
            insertFolderQuery.bindValue(":folder_catalog_name", catalog->name);
            insertFolderQuery.bindValue(":folder_path",         catalog->sourcePath);
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
            if (catalog->includeHidden == true){
                QDirIterator iterator(catalog->sourcePath+"/", fileExtensions, QDir::AllEntries|QDir::NoDotAndDotDot|QDir::Hidden, QDirIterator::Subdirectories);
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
                              insertFolderQuery.bindValue(":folder_catalog_name", catalog->name);
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
                              insertFileQuery.bindValue(":file_catalog",      catalog->name);
                              insertFileQuery.bindValue(":file_full_path",    entryPath);
                              insertFileQuery.exec();

                              //Media File Metadata
                              if(developmentMode==true){
                                  if(catalog->includeMetadata == true){
                                      setMediaFile(entryPath);
                                  }
                              }
                        }
                    }
                }
            }
            else{
                QDirIterator iterator(catalog->sourcePath+"/", fileExtensions, QDir::AllEntries|QDir::NoDotAndDotDot, QDirIterator::Subdirectories);
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
                            insertFolderQuery.bindValue(":folder_catalog_name", catalog->name);
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
                            insertFileQuery.bindValue(":file_catalog",      catalog->name);
                            insertFileQuery.bindValue(":file_full_path",    entryPath);
                            insertFileQuery.exec();

                            //Media File Metadata
                            if(developmentMode==true){
                              if(catalog->includeMetadata == true){
                                  setMediaFile(entryPath);
                              }
                           }
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
                catalog->updateFileCount();
                catalog->updateTotalFileSize();

        //Populate model with lines for csv files
        if(databaseMode=="Memory"){
            //Save data to file
            QStringList fileList;

            QSqlQuery query;
            QString querySQL = QLatin1String(R"(
                        SELECT file_full_path, file_size, file_date_updated
                        FROM file
                        WHERE file_catalog=:file_catalog
                    )");
            query.prepare(querySQL);
            query.bindValue(":file_catalog",catalog->name);
            query.exec();

            while(query.next()){
                    fileList << query.value(0).toString() + "\t" + query.value(1).toString() + "\t" + query.value(2).toString();
            };

            //Prepare the catalog file data, adding first the catalog metadata at the beginning
            fileList.prepend("<catalogID>"              + QString::number(catalog->ID));
            fileList.prepend("<catalogAppVersion>"      + currentVersion);
            fileList.prepend("<catalogIncludeMetadata>" + QVariant(catalog->includeMetadata).toString());
            fileList.prepend("<catalogIsFullDevice>"    + QVariant(catalog->isFullDevice).toString());
            fileList.prepend("<catalogIncludeSymblinks>"+ QVariant(catalog->includeSymblinks).toString());
            fileList.prepend("<catalogStorage>"         + catalog->storageName);
            fileList.prepend("<catalogFileType>"        + catalog->fileType);
            fileList.prepend("<catalogIncludeHidden>"   + QVariant(catalog->includeHidden).toString());
            fileList.prepend("<catalogTotalFileSize>"   + QString::number(catalog->totalFileSize));
            fileList.prepend("<catalogFileCount>"       + QString::number(catalog->fileCount));
            fileList.prepend("<catalogSourcePath>"      + catalog->sourcePath);

            //Define and populate a model
            fileListModel = new QStringListModel(this);
            fileListModel->setStringList(fileList);
        }

        //Update catalog in db
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
        query.bindValue(":catalog_include_symblinks", catalog->includeSymblinks);
        query.bindValue(":catalog_file_count", catalog->fileCount);
        query.bindValue(":catalog_total_file_size", catalog->totalFileSize);
        query.bindValue(":catalog_app_version", currentVersion);
        query.bindValue(":catalog_name", catalog->name);
        query.exec();

        loadCatalogsTableToModel();

        //Update catalog date loaded and updated
        QDateTime emptyDateTime = *new QDateTime;
        catalog->setDateUpdated(emptyDateTime);
        catalog->setDateLoaded(emptyDateTime);

        //Stop animation
        QApplication::restoreOverrideCursor();
    }
    //--------------------------------------------------------------------------
    void MainWindow::saveCatalogToNewFile(QString newCatalogName)
    {
        if(databaseMode=="Memory"){

            //Save a catalog to a new file

            // Get the file list from this model
            QStringList filelist = fileListModel->stringList();

            // Stream the list to the file
            QFile fileOut( collectionFolder +"/"+ newCatalogName + ".idx" );

            // write data

            if (fileOut.open(QFile::WriteOnly | QFile::Text)) {
                QTextStream stream(&fileOut);
                for (int i = 0; i < filelist.size(); ++i)
                  stream << filelist.at(i) << '\n';
            }
            else {
                QMessageBox msgBox;
                msgBox.setWindowTitle("Katalog");
                msgBox.setText(tr("Error opening output file."));
                msgBox.setIcon(QMessageBox::Warning);
                msgBox.exec();
            }
            fileOut.close();
        }
    }
    //--------------------------------------------------------------------------
    void MainWindow::saveFoldersToNewFile(QString newCatalogName)
    {
        //Save a catalog to a new file

        // Get the folder list from database
        QSqlQuery query;
        QString querySQL = QLatin1String(R"(
                                SELECT
                                    folder_catalog_name,
                                    folder_path
                                FROM folder
                                WHERE folder_catalog_name=:folder_catalog_name
                                        )");
        query.prepare(querySQL);
        query.bindValue(":folder_catalog_name",newCatalogName);
        query.exec();

        // Stream the list to the file
        QFile fileOut( collectionFolder +"/"+ newCatalogName + ".folders.idx" );

        // write data

          if (fileOut.open(QFile::WriteOnly | QFile::Text)) {
            QTextStream stream(&fileOut);
            while(query.next()){
                stream << query.value(0).toString() << '\t';
                stream << query.value(1).toString() << '\n';
            }

          } else {
              QMessageBox msgBox;
              msgBox.setWindowTitle("Katalog");
              msgBox.setText(tr("Error opening output file."));
              msgBox.setIcon(QMessageBox::Warning);
              msgBox.exec();
            //return EXIT_FAILURE;
          }
          fileOut.close();
    }
    //--------------------------------------------------------------------------
//DEV --------------------------------------------------------------------------

    void MainWindow::setMediaFile(QString filePath)
    {
        QFile mediaFile(filePath);
        if(mediaFile.exists()==true){
            m_player = new QMediaPlayer(this);
//            connect(m_player, SIGNAL(mediaStatusChanged(QMediaPlayer::MediaStatus)), this, SLOT(onMediaStatusChanged(QMediaPlayer::MediaStatus)));
//            m_player->setSource(QUrl::fromLocalFile(filePath));

             QMediaPlayer *player = new QMediaPlayer(this);
//            QMediaPlayer player;
 //            player->setMedia(QUrl::fromLocalFile(filePath));

            // Wait for the media to be loaded
//            if (player->mediaStatus() != QMediaPlayer::LoadedMedia) {
//                QObject::connect(&player, &QMediaPlayer::mediaStatusChanged, &app, [&player, &app](QMediaPlayer::MediaStatus status) {
//                    if (status == QMediaPlayer::LoadedMedia) {
//                        app.quit();
//                    }
//                });

//                return a.exec();
//            }

            // Retrieve the video's metadata
//            QVariant resolution = player->metaData(QMediaMetaData::Resolution);

//            if (resolution.isValid()) {
//                QSize videoResolution = resolution.toSize();
//                qDebug() << "Video resolution:" << videoResolution.width() << "x" << videoResolution.height();
//            } else {
//                qDebug() << "Failed to retrieve video resolution.";
//            }
        }
    }
    //--------------------------------------------------------------------------
    void MainWindow::onMediaStatusChanged(QMediaPlayer::MediaStatus status)
    {
        if (status == QMediaPlayer::LoadedMedia)
            getMetaData(m_player);
    }   
    //--------------------------------------------------------------------------
    void MainWindow::getMetaData(QMediaPlayer *player)
    {
        QMediaMetaData metaData = player->metaData();

        QVariant Resolution = metaData.value(QMediaMetaData::Resolution);
        QMessageBox msgBox;
        msgBox.setWindowTitle("Katalog");
        msgBox.setText(tr("Resolution")+": <br/>" + Resolution.toString());
        msgBox.setIcon(QMessageBox::Information);
        msgBox.exec();
        //QVideoFrame frame = player->currentFrame();

     }
    //--------------------------------------------------------------------------

    /*
    void MainWindow::getMetaData(QMediaPlayer *player)
    {
        QMessageBox::information(this,"Katalog","getMetaData");

        QMediaMetaData metaData = player->metaData();

        QVariant Resolution = metaData.value(QMediaMetaData::Resolution);
        QMessageBox::information(this,"Katalog","Resolution:<br/>" + Resolution.toString());

        QVariant Duration = player->metaData()[QMediaMetaData::Duration];
        QMessageBox::information(this,"Katalog","Duration:<br/>" + Duration.toString());

        QVariant MediaType = player->metaData()[QMediaMetaData::MediaType];
        QMessageBox::information(this,"Katalog","MediaType:<br/>" + MediaType.toString());
     }


     QString filePath = "/home/stephane/Vidéos/COPY/test8.mkv";

    */
