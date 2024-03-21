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
        QFile excludeFile(collection->excludeFilePath);
        if ( excludeFile.exists()){
            QDesktopServices::openUrl(QUrl::fromLocalFile(collection->excludeFilePath));
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
                          QDesktopServices::openUrl(QUrl::fromLocalFile(collection->excludeFilePath));
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
        if (ui->Create_comboBox_StorageSelection->currentText() == ""){
            QMessageBox msgBox;
            msgBox.setWindowTitle("Katalog");
            msgBox.setText(tr("Select a Storage for this new catalog.<br/>(Selection panel on the left and dropdown list)"));
            msgBox.setIcon(QMessageBox::Warning);
            msgBox.exec();
            return;
        }

        //Create a new device and catalog

            //Initiate Device entry
            Device *newDevice = new Device();
            newDevice->generateDeviceID();
            newDevice->type = "Catalog";
            newDevice->name = ui->Create_lineEdit_NewCatalogName->text();

            //Check if the catalog name (so the csv file name) already exists
            if (newDevice->verifyDeviceNameExists()){
                QMessageBox msgBox;
                msgBox.setWindowTitle("Katalog");
                msgBox.setText( tr("There is already a catalog with this name:<br/><b>")
                               + newDevice->name
                               + "</b><br/><br/>"+tr("Choose a different name and try again."));
                msgBox.setIcon(QMessageBox::Warning);
                msgBox.exec();
                return;
            }

            //Continue populating values and add device
            newDevice->parentID = ui->Create_comboBox_StorageSelection->currentData().toInt();
            newDevice->catalog->generateID();
            newDevice->externalID = newDevice->catalog->ID;
            newDevice->groupID = 0;
            newDevice->path = ui->Create_lineEdit_NewCatalogPath->text();
            newDevice->insertDevice();

            //Get inputs and set values of the newCatalog
            newDevice->catalog->filePath = collection->collectionFolder + "/" + newDevice->name + ".idx";
            newDevice->catalog->sourcePath = ui->Create_lineEdit_NewCatalogPath->text();
            newDevice->catalog->includeHidden = ui->Create_checkBox_IncludeHidden->isChecked();
            newDevice->catalog->storageName = ui->Create_comboBox_StorageSelection->currentText();
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

            //Add path to parent Storage device if empty
            Device parentStorageDevice;
            parentStorageDevice.ID = newDevice->parentID;
            parentStorageDevice.loadDevice();
            if(parentStorageDevice.path == ""){
                parentStorageDevice.path = newDevice->path;
                parentStorageDevice.saveDevice();
                collection->saveStorageTableToFile();
            }

            //Reload
            loadDevicesView();
            loadStorageList();

        //Launch the scan and cataloging of files, including statistics
            bool updateResult = reportAllUpdates(newDevice,
                                                 newDevice->updateDevice("create",
                                                                         collection->databaseMode,
                                                                         false,
                                                                         collection->collectionFolder,
                                                                         true),
                                                 "create");

            if (updateResult==true){
                newDevice->saveDevice();

                //Save data to files
                collection->saveDeviceTableToFile();
                newDevice->catalog->saveCatalogToFile(collection->databaseMode, collection->collectionFolder);
                newDevice->catalog->saveFoldersToFile(collection->databaseMode, collection->collectionFolder);

                //Update the new catalog loadedversion to indicate that files are already in memory
                QDateTime emptyDateTime = *new QDateTime;
                newDevice->catalog->setDateLoaded(emptyDateTime);

                //Save statistics
                collection->saveStatiticsToFile();

                //Refresh data and UI
                //Refresh the catalog list for the Search screen
                collection->loadCatalogFilesToTable();

                //Refresh the catalog list for the combobox of the Search screen
                refreshDifferencesCatalogSelection();

                //Refresh Catalogs list
                updateAllDeviceActive();
                loadDevicesView();

                //Restore selected catalog
                ui->Filters_label_DisplayCatalog->setText(ui->Filters_label_DisplayCatalog->text());

                //Refresh filter tree
                collection->loadDeviceFileToTable();
                loadDevicesTreeToModel("Filters");
                loadDevicesView();

                //Change tab to show the result of the catalog creation
                ui->tabWidget->setCurrentIndex(1); // tab 1 is the Collection tab

                //Disable buttons
                ui->Catalogs_pushButton_UpdateCatalog->setEnabled(false);
            }
            else{
                newDevice->deleteDevice(false);
                loadDevicesView();
            }
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

//             QMediaPlayer *player = new QMediaPlayer(this);
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
