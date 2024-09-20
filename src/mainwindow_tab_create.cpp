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
// Description: https://stephanecouturier.github.io/Katalog/docs/Features/Create
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
    void MainWindow::on_Create_pushButton_AddDirectoryToExclude_clicked()
    {//Add fodler to the exclusion list
        QString newFolderToExclude = ui->Create_lineEdit_FolderToExclude->text();
        int pathLength = newFolderToExclude.length();
        if (newFolderToExclude !="" and newFolderToExclude !="/" and QVariant(newFolderToExclude.at(pathLength-1)).toString()=="/") {
            newFolderToExclude.remove(pathLength-1,1);
        }

        if(newFolderToExclude!=""){
            //Insert new entry
            QSqlQuery insertQuery(QSqlDatabase::database("defaultConnection"));
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
            insertQuery.bindValue(":parameter_value2", newFolderToExclude);
            insertQuery.exec();
            qDebug()<<"DEBUG: query: "<<insertQuery.lastError();

            //Save
            collection->saveParameterTableToFile();

            //Reload to list view
            QSqlQuery queryLoad(QSqlDatabase::database("defaultConnection"));
            QString queryLoadSQL = QLatin1String(R"(
                                        SELECT DISTINCT parameter_value2
                                        FROM parameter
                                        WHERE parameter_type ='exclude_directory'
                                        ORDER BY parameter_value2
                                )");
            if (!queryLoad.exec(queryLoadSQL)) {
                qDebug() << "Failed to execute query";
                return;
            }

            QSqlQueryModel *model = new QSqlQueryModel;
            model->setQuery(std::move(queryLoad));

            QSortFilterProxyModel *proxyModel = new QSortFilterProxyModel(this);
            proxyModel->setSourceModel(model);
            proxyModel->setDynamicSortFilter(true);
            ui->Create_treeView_Excluded->setModel(proxyModel);
        }
    }
    //--------------------------------------------------------------------------
    void MainWindow::on_Create_pushButton_AddStorage_clicked()
    {
        //Change tab to show the screen to add a storage
        ui->tabWidget->setCurrentIndex(1); // tab 1 is the Devices tab
        ui->Devices_radioButton_DeviceTree->setChecked(true); // the tree view is required to add a storage
        loadDevicesView(); // refresh the view
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
    void MainWindow::on_Create_treeView_Excluded_customContextMenuRequested(const QPoint &pos)
    {
        //Get selection data
        QModelIndex index=ui->Create_treeView_Excluded->currentIndex();
        QString selectedDirectory = ui->Create_treeView_Excluded->model()->index(index.row(), 0, index.parent() ).data().toString();

        //Set actions
        QPoint globalPos = ui->Create_treeView_Excluded->mapToGlobal(pos);
        QMenu excludeContextMenu;

        QAction *menuDeviceAction1 = new QAction(QIcon::fromTheme("edit-delete"), tr("Remove this directory"), this);
        excludeContextMenu.addAction(menuDeviceAction1);
        connect(menuDeviceAction1, &QAction::triggered, this, [ selectedDirectory, this]() {
            //Delete
            QSqlQuery query(QSqlDatabase::database("defaultConnection"));
            QString querySQL = QLatin1String(R"(
                                    DELETE FROM parameter
                                    WHERE parameter_type ='exclude_directory'
                                    AND parameter_value2=:parameter_value2
                                )");
            query.prepare(querySQL);
            query.bindValue(":parameter_value2", selectedDirectory);
            query.exec();

            //Reload
            QSqlQuery queryLoad(QSqlDatabase::database("defaultConnection"));
            QString queryLoadSQL = QLatin1String(R"(
                                        SELECT DISTINCT parameter_value2
                                        FROM parameter
                                        WHERE parameter_type ='exclude_directory'
                                        ORDER BY parameter_value2
                                )");
            if (!queryLoad.exec(queryLoadSQL)) {
                qDebug() << "Failed to execute query";
                return;
            }

            QSqlQueryModel *model = new QSqlQueryModel;
            model->setQuery(std::move(queryLoad));

            QSortFilterProxyModel *proxyModel = new QSortFilterProxyModel(this);
            proxyModel->setSourceModel(model);
            proxyModel->setDynamicSortFilter(true);
            ui->Create_treeView_Excluded->setModel(proxyModel);
        });

        excludeContextMenu.exec(globalPos);
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
            newDevice->catalog->filePath = collection->folder + "/" + newDevice->name + ".idx";
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
            parentStorageDevice.loadDevice("defaultConnection");
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
                                                                         collection->folder,
                                                                         true),
                                                 "create");

            if (updateResult==true){
                newDevice->saveDevice();

                //Save data to files
                collection->saveDeviceTableToFile();
                newDevice->catalog->saveCatalogToFile(collection->databaseMode, collection->folder);
                newDevice->catalog->saveFoldersToFile(collection->databaseMode, collection->folder);

                //Update the new catalog loadedversion to indicate that files are already in memory
                QDateTime emptyDateTime = *new QDateTime; //Using an empty date as the function will manage creating one if needed
                newDevice->catalog->setDateLoaded(emptyDateTime, "defaultConnection");

                //Save statistics
                collection->saveStatiticsToFile();

                //Refresh data and UI
                    //Refresh the catalog list for the combobox of the Search screen
                    refreshDifferencesCatalogSelection();

                    //Refresh Catalogs list
                    updateAllDeviceActive();
                    loadDevicesView();

                    //Restore selected catalog
                    ui->Filters_label_DisplayCatalog->setText(newDevice->name);
                    selectedDevice->ID = newDevice->ID;
                    selectedDevice->loadDevice("defaultConnection");

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
