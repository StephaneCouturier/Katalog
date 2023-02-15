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
    {
        //Sends the selected folder in the tree for the New Catalog Path

        //Get the model/data from the tree
        QFileSystemModel* pathmodel = (QFileSystemModel*)ui->Create_treeView_Explorer->model();
        //get data from the selected file/directory
        QFileInfo fileInfo = pathmodel->fileInfo(index);
        //send the path to the line edit
        ui->Create_lineEdit_NewCatalogPath->setText(fileInfo.filePath());
    }
    //--------------------------------------------------------------------------
    void MainWindow::on_Create_pushButton_PickPath_clicked()
    {
        //Pick a directory from a dialog window

        //Get current selected path as default path for the dialog window
        QString newCatalogPath = ui->Create_lineEdit_NewCatalogPath->text();

        //Open a dialog for the user to select the directory to be cataloged. Only show directories.
        QString dir = QFileDialog::getExistingDirectory(this, tr("Select the directory to be cataloged in this new catalog"),
                                                        newCatalogPath,
                                                        QFileDialog::ShowDirsOnly
                                                        | QFileDialog::DontResolveSymlinks);
        //Send the selected directory to LE_NewCatalogPath (input line for the New Catalog Path)
        ui->Create_lineEdit_NewCatalogPath->setText(dir);

        //Select this directory in the treeview.
        loadFileSystem(newCatalogPath);
    }
    //--------------------------------------------------------------------------
    void MainWindow::on_Create_pushButton_EditExcludeList_clicked()
    {
        //Edit Exclusion list

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
    {
        //Generate the Catalog name from the path
        QString newCatalogName = ui->Create_lineEdit_NewCatalogPath->text();
        newCatalogName.replace("/","_");
        newCatalogName.replace(":","_");
        ui->Create_lineEdit_NewCatalogName->setText(newCatalogName);
    }
    //--------------------------------------------------------------------------
    void MainWindow::on_Create_pushButton_CreateCatalog_clicked()
    {
        createCatalog();
        loadCollection();
    }
    //--------------------------------------------------------------------------

//Methods-----------------------------------------------------------------------

    //Load file system for the treeview
    void MainWindow::loadFileSystem(QString newCatalogPath)
    {
            newCatalogPath="/";
         // Creates a new model
            fileSystemModel = new QFileSystemModel(this);

         // Set filter to show only directories
            fileSystemModel->setFilter(QDir::NoDotAndDotDot | QDir::AllDirs);

         // QFileSystemModel requires root path
            QString rootPath ="/";
            fileSystemModel->setRootPath(rootPath);
            fileSystemModel->setRootPath(newCatalogPath);

         // Enable/Disable modifying file system
            //qfilesystemmodel->setReadOnly(true);


        //loadFileSystem in the Create screen tree view
            // Attach the model to the view
                ui->Create_treeView_Explorer->setModel(fileSystemModel);
            // Only show the tree, hidding other columns and the header row.
                ui->Create_treeView_Explorer->setColumnWidth(0,250);
                ui->Create_treeView_Explorer->setColumnHidden(1,true);
                ui->Create_treeView_Explorer->setColumnHidden(2,true);
                ui->Create_treeView_Explorer->setColumnHidden(3,true);
                ui->Create_treeView_Explorer->setHeaderHidden(true);
                ui->Create_treeView_Explorer->expandToDepth(1);

        //loadFileSystem in the Filter tab tree view
            // Attach the model to the view
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
    void MainWindow::loadStorageList()
    {
        //Prepare list for the Storage selection combobox
        QStringList storageNameListforCombo = storageNameList;
        storageNameListforCombo.prepend("");
        fileListModel = new QStringListModel(this);
        fileListModel->setStringList(storageNameListforCombo);
        ui->Create_comboBox_StorageSelection->setModel(fileListModel);
        ui->Catalogs_comboBox_Storage->setModel(fileListModel);
    }
    //--------------------------------------------------------------------------
    void MainWindow::createCatalog()
    {
        //Create a new catalog, launch the cataloging and save, and refresh data and UI
        //Create a new catalog
        Catalog *newCatalog = new Catalog();

            //Get inputs and set values of the newCatalog
            newCatalog->setName(ui->Create_lineEdit_NewCatalogName->text());
            newCatalog->setFilePath(collectionFolder + "/" + newCatalog->name + ".idx");
            newCatalog->setSourcePath(ui->Create_lineEdit_NewCatalogPath->text());
            newCatalog->setIncludeHidden(ui->Create_checkBox_IncludeHidden->isChecked());
            newCatalog->setStorageName(ui->Create_comboBox_StorageSelection->currentText());
            newCatalog->setIncludeSymblinks(ui->Create_checkBox_IncludeSymblinks->isChecked());
            newCatalog->setIsFullDevice(ui->Create_checkBox_isFullDevice->isChecked());
            newCatalog->setDateLoaded(QDateTime::currentDateTime().toString("yyyy-MM-dd hh:mm:ss"));
            newCatalog->setFileCount(0);
            newCatalog->setTotalFileSize(0);
            newCatalog->setIncludeMetadata(ui->Create_checkBox_IncludeMetadata->isChecked());

            //Get the file type for the catalog
            if      ( ui->Create_radioButton_FileType_Image->isChecked() ){
                    newCatalog->fileType = "Image";}
            else if ( ui->Create_radioButton_FileType_Audio->isChecked() ){
                    newCatalog->fileType = "Audio";}
             else if ( ui->Create_radioButton_FileType_Video->isChecked() ){
                    newCatalog->fileType = "Video";}
            else if ( ui->Create_radioButton_FileType_Text->isChecked() ){
                    newCatalog->fileType = "Text";}
            else
                    newCatalog->fileType = "All";

            //Check if the catalog (file) already exists
            QFile file(newCatalog->filePath);
            if (file.exists()==true){
                QMessageBox::information(this, "Katalog",
                                         tr("There is already a catalog with this name:")
                                            + newCatalog->name
                                            + "\n"+tr("Choose a different name."),
                                         ( tr("Ok") ) );
                return;
            }

        //Launch the scan and cataloging of files
            if (newCatalog->name!="" and newCatalog->sourcePath!="")
                    updateSingleCatalog(newCatalog);
            else QMessageBox::warning(this, "Katalog",
                                      tr("Provide a name and select a path for this new catalog.")+"\n" +tr("Name:")
                                      +newCatalog->name+"\n"+tr("Path:")+newCatalog->sourcePath,
                                      ( tr("Ok") ) );

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
            QSqlQuery queryUpdateDateLoaded;
            QString queryUpdateDateLoadedSQL = QLatin1String(R"(
                                                    UPDATE catalog
                                                    SET catalogLoadedVersion=:catalogDateLoaded
                                                    WHERE catalogName=:catalogName
                                            )");
            queryUpdateDateLoaded.prepare(queryUpdateDateLoadedSQL);
            queryUpdateDateLoaded.bindValue(":catalogDateLoaded",newCatalog->dateLoaded);
            queryUpdateDateLoaded.bindValue(":catalogName",newCatalog->name);
            queryUpdateDateLoaded.exec();

            //Add new catalog values to the statistics log, if the user has chosen this option
                if ( ui->Settings_checkBox_SaveRecordWhenUpdate->isChecked() == true ){

                    //Save values
                    recordSelectedCatalogStats(newCatalog->name, newCatalog->fileCount, newCatalog->totalFileSize);

                    //Reload stats file to refresh values
                    loadStatisticsChart();
                }

        //Refresh data and UI
            //Refresh the catalog list for the Search screen
            loadCatalogFilesToTable();

            //Refresh the catalog list for the Collection screen
            loadCatalogsTableToModel();

            //Refresh the catalog list for the combobox of the Search screen
                //Get current search selection
                selectedFilterStorageLocation = ui->Filters_label_DisplayLocation->text();
                selectedFilterStorageName     = ui->Filters_label_DisplayStorage->text();
                selectedFilterCatalogName     = ui->Filters_label_DisplayCatalog->text();

                //Refresh list
                refreshCatalogSelectionList(selectedFilterStorageLocation, selectedFilterStorageName);

                //Restore selcted catalog
                ui->Filters_label_DisplayCatalog->setText(selectedFilterCatalogName);

            //Change tab to show the result of the catalog creation
            ui->tabWidget->setCurrentIndex(1); // tab 1 is the Collection tab

            //Disable buttons to force user to select a catalog
            ui->Catalogs_pushButton_Search->setEnabled(false);
            ui->Catalogs_pushButton_ExploreCatalog->setEnabled(false);
            ui->Catalogs_pushButton_EditCatalogFile->setEnabled(false);
            ui->Catalogs_pushButton_UpdateCatalog->setEnabled(false);
            ui->Catalogs_pushButton_ViewCatalogStats->setEnabled(false);
            ui->Catalogs_pushButton_DeleteCatalog->setEnabled(false);

            //Inform user
            QMessageBox::information(this, "Katalog",
                                      tr("The new catalog,has been created.\n Name:   ")
                                      +newCatalog->name + "\n" +tr("Source:   ") + newCatalog->sourcePath,
                                      ( tr("Ok") ) );
    }
    //--------------------------------------------------------------------------
    void MainWindow::catalogDirectory(Catalog *catalog)
    {
        //Catalog the files of a directory and add catalog meta-data

        // Start animation while cataloging
        QApplication::setOverrideCursor(Qt::WaitCursor);

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
        if(!excludeFile.open(QIODevice::ReadOnly)) {
             //QMessageBox::information(this,"Katalog",tr("No exclude file found."));
             //return;
        }
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

        //Iterate in the directory to create a list of files
        QStringList fileList;
        qint64 catalogTotalFileSize = 0;

        if (catalog->includeHidden == true){
            QDirIterator iterator(catalog->sourcePath, fileExtensions, QDir::Files|QDir::Hidden, QDirIterator::Subdirectories);
            while (iterator.hasNext()){              

                QString dir = iterator.next();
                QString filePath = iterator.filePath();
                qint64 fileSize;
                QFile file(filePath);

                fileSize = file.size();
                catalogTotalFileSize = catalogTotalFileSize + fileSize;

                QFileInfo fileInfo(filePath);
                QDateTime fileDate = fileInfo.lastModified();

                //exclude if the folder is part of excluded directories
                bool excludeFile = false;
                //exclude files in /directory/lowerlevel/file when exclude folder is in /directory
                for (int i=0; i<excludedFolders.length(); i++) {
                    if(fileInfo.absolutePath().contains(excludedFolders[i]+"/") ){
                        excludeFile = true;
                        break;
                    }
                }
                //exclude files in /directory/file when exclude folder is in /directory
                if (excludedFolders.contains(fileInfo.absolutePath())){
                    excludeFile = true;
                }
                //add file to list if not excluded
                if(excludeFile == false){
                        fileList << filePath + "\t" + QString::number(fileSize) + "\t" + fileDate.toString("yyyy/MM/dd hh:mm:ss");
                }

                if(developmentMode==false){
                    //Include Media File Metadata
                                    //Include Media File Metadata
                                    if(catalog->includeMetadata == true){
                                        setMediaFile(filePath);
                                        //QMessageBox::information(this,"Katalog","Create_checkBox_IncludeMetadata: <br/>" + QVariant("ok").toString());
                                    }
                }
            }
        }
        else{
            QDirIterator iterator(catalog->sourcePath, fileExtensions, QDir::Files, QDirIterator::Subdirectories);
            while (iterator.hasNext()){

                QString dir = iterator.next();
                QString filePath = iterator.filePath();
                qint64 fileSize;
                QFile file(filePath);

                fileSize = file.size();
                catalogTotalFileSize = catalogTotalFileSize + fileSize;

                QFileInfo fileInfo(filePath);
                QDateTime fileDate = fileInfo.lastModified();

                //exclude if the folder is part of excluded directories
                bool excludeFile = false;
                //exclude files in /directory/lowerlevel/file when exclude fodler is in /directory
                for (int i=0; i<excludedFolders.length(); i++) {
                    if(fileInfo.absolutePath().contains(excludedFolders[i]+"/") ){
                        excludeFile = true;
                        break;
                    }
                }
                //exclude files in /directory/file when exclude folder is in /directory
                if (excludedFolders.contains(fileInfo.absolutePath())){
                    excludeFile = true;
                }
                //add file to list if not excluded
                if(excludeFile == false){
                    fileList << filePath + "\t" + QString::number(fileSize) + "\t" + fileDate.toString("yyyy/MM/dd hh:mm:ss");
                }

                if(developmentMode==false){
                    //Include Media File Metadata
                                    //Include Media File Metadata
                                    if(catalog->includeMetadata == true){
                                        setMediaFile(filePath);
                                        //QMessageBox::information(this,"Katalog","Create_checkBox_IncludeMetadata: <br/>" + QVariant("ok").toString());
                                    }
                }
            }
        }

        //update Catalog metadata
        catalog->setFileCount(fileList.count());
        catalog->setTotalFileSize(catalogTotalFileSize);

        //Insert catalog file list
        //DEV note: this can be optimized combining with the method loadCatalogFilelistToTable(Catalog *catalog)

            //Remove any former files from db for older catalog with same name
            QSqlQuery deleteQuery;
            QString deleteQuerySQL = QLatin1String(R"(
                                DELETE FROM filesall
                                WHERE fileCatalog=:fileCatalog
                                            )");
            deleteQuery.prepare(deleteQuerySQL);
            deleteQuery.bindValue(":fileCatalog",catalog->name);
            deleteQuery.exec();

            //prepare insert query for filesall
            QSqlQuery insertFilesallQuery;
            QString insertFilesallSQL = QLatin1String(R"(
                                    INSERT INTO filesall (
                                                    fileName,
                                                    filePath,
                                                    fileSize,
                                                    fileDateUpdated,
                                                    fileCatalog,
                                                    fileFullPath
                                                    )
                                    VALUES(
                                                    :fileName,
                                                    :filePath,
                                                    :fileSize,
                                                    :fileDateUpdated,
                                                    :fileCatalog,
                                                    :fileFullPath )
                                )");

            //prepare insert query for folder
                                   QSqlQuery insertFolderQuery;
                                   QString insertFolderSQL = QLatin1String(R"(
                                           INSERT OR IGNORE INTO folder(
                                                   folderHash,
                                                   folderCatalogName,
                                                   folderPath
                                                             )
                                           VALUES(
                                                  :folderHash,
                                                  :folderCatalogName,
                                                  :folderPath)
                                                       )");
            //Iterrate through the file list
            QRegularExpression lineCatalogFileSplitExp("\t");
            for(int i=0; i<fileList.count(); i++){
            //fileList[i]

                //Split the line text with tabulations into a list
                QStringList lineFieldList  = fileList[i].split(lineCatalogFileSplitExp);
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
                QString folderHash = QString::number(qHash(folder));

                 //Load folder into the database
                     insertFolderQuery.prepare(insertFolderSQL);
                     insertFolderQuery.bindValue(":folderHash",      folderHash);
                     insertFolderQuery.bindValue(":folderCatalogName",   catalog->name);
                     insertFolderQuery.bindValue(":folderPath",      folder);
                     insertFolderQuery.exec();

                //Load file into the database
                    insertFilesallQuery.prepare(insertFilesallSQL);
                    insertFilesallQuery.bindValue(":fileName",        fileInfo.fileName());
                    insertFilesallQuery.bindValue(":fileSize",        lineFileSize);
                    insertFilesallQuery.bindValue(":filePath",        folder ); //DEV: replace later by folderHash
                    insertFilesallQuery.bindValue(":fileDateUpdated", lineFileDatetime);
                    insertFilesallQuery.bindValue(":fileCatalog",     catalog->name);
                    insertFilesallQuery.bindValue(":fileFullPath",    lineFilePath);
                    insertFilesallQuery.exec();

        }

        //Prepare the catalog file data, adding first the catalog metadata at the beginning
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

        //Update catalog in db
        QSqlQuery query;
        QString querySQL = QLatin1String(R"(
                                UPDATE Catalog
                                SET catalogIncludeSymblinks =:catalogIncludeSymblinks,
                                    catalogFileCount =:catalogFileCount,
                                    catalogTotalFileSize =:catalogTotalFileSize
                                    WHERE catalogName =:catalogName
                            )");
        query.prepare(querySQL);
        query.bindValue(":catalogFileCount", catalog->fileCount);
        query.bindValue(":catalogTotalFileSize", catalog->totalFileSize);
        query.bindValue(":catalogName", catalog->name);
        query.exec();

        //update catalog date loaded
            QDateTime nowDateTime = QDateTime::currentDateTime();
            catalog->setDateLoaded(nowDateTime.toString("yyyy-MM-dd hh:mm:ss"));

            QSqlQuery catalogQuery;
            QString catalogQuerySQL = QLatin1String(R"(
                                        UPDATE catalog
                                        SET catalogLoadedVersion =:catalogDateLoaded
                                        WHERE catalogName =:catalogName
                                      )");
            catalogQuery.prepare(catalogQuerySQL);
            catalogQuery.bindValue(":catalogDateLoaded", catalog->dateLoaded);
            catalogQuery.bindValue(":catalogName",       catalog->name);
            catalogQuery.exec();

        //Stop animation
        QApplication::restoreOverrideCursor();
    }
    //--------------------------------------------------------------------------
    void MainWindow::saveCatalogToNewFile(QString newCatalogName)
    {
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
          } else {
              QMessageBox::information(this,"Katalog","error opening output file\n");
            //return EXIT_FAILURE;
          }
          fileOut.close();
    }
    //--------------------------------------------------------------------------

//DEV--------------------------------------------------------------------------

    void MainWindow::setMediaFile(QString filePath)
    {
        QFile mediaFile(filePath);
        if(mediaFile.exists()==true){
            m_player = new QMediaPlayer(this);
            connect(m_player, SIGNAL(mediaStatusChanged(QMediaPlayer::MediaStatus)), this, SLOT(onMediaStatusChanged(QMediaPlayer::MediaStatus)));
            m_player->setSource(QUrl::fromLocalFile(filePath));
        }
    }

    void MainWindow::onMediaStatusChanged(QMediaPlayer::MediaStatus status)
    {
        if (status == QMediaPlayer::LoadedMedia)
            getMetaData(m_player);
    }




    void MainWindow::getMetaData(QMediaPlayer *player)
    {
        QMediaMetaData metaData = player->metaData();

        QVariant Resolution = metaData.value(QMediaMetaData::Resolution);
        QMessageBox::information(this,"Katalog","Resolution:<br/>" + Resolution.toString());

        //QVideoFrame frame = player->currentFrame();
        //qDebug() << "Width: " << frame.width() << ", Height: " << frame.height();

     }

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







