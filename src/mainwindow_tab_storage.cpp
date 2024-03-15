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
// File Name:   mainwindow_tab_storage.cpp
// Purpose:     methods for the screen Storage
// Description: https://github.com/StephaneCouturier/Katalog/wiki/Storage
// Author:      Stephane Couturier
/////////////////////////////////////////////////////////////////////////////
*/

#include "mainwindow.h"
#include "ui_mainwindow.h"
#include "storageview.h"
#include "storage.h"

//UI----------------------------------------------------------------------------
    //Full list ----------------------------------------------------------------
    // void MainWindow::on_Storage_pushButton_CreateList_clicked()
    // {
    //     createStorageFile();
    // }
    //--------------------------------------------------------------------------
    // void MainWindow::on_Storage_pushButton_Reload_clicked()
    // {
    //     collection->loadStorageFileToTable();
    //     loadStorageTableToModel();
    //     updateStorageSelectionStatistics();
    // }
    //--------------------------------------------------------------------------
    // void MainWindow::on_Storage_pushButton_Edit_clicked()
    // {
    //     //ui->Storage_widget_Panel->show();
    //     //ui->Storage_widget_PanelForm->setEnabled(true);
    //     loadStorageToPanel();
    // }
    //--------------------------------------------------------------------------
    // void MainWindow::on_Storage_pushButton_EditAll_clicked()
    // {
    //     QDesktopServices::openUrl(QUrl::fromLocalFile(collection->storageFilePath));
    // }
    //--------------------------------------------------------------------------
    // void MainWindow::on_Storage_pushButton_SaveAll_clicked()
    // {
    //     //Save data to file and reload
    //     if (collection->databaseMode=="Memory"){
    //         //Save model data to Storage file
    //         collection->saveStorageTableToFile();

    //         //Reload Storage file data to table
    //         collection->loadStorageFileToTable();
    //     }

    //     //refresh
    //     loadStorageTableToModel();
    //     updateStorageSelectionStatistics();
    //     unsavedChanges = false;
    //     ui->Storage_pushButton_SaveAll->setStyleSheet("color: black");

    //     loadStorageToPanel();
    // }
    //--------------------------------------------------------------------------
    // void MainWindow::on_Storage_treeView_StorageList_clicked(const QModelIndex &index)
    // {
    //     activeDevice->ID = ui->Storage_treeView_StorageList->model()->index(index.row(), 16, QModelIndex()).data().toInt();
    //     activeDevice->loadDevice();

    //     //display buttons
    //     // ui->Storage_pushButton_Edit->setEnabled(true);
    //     // ui->Storage_pushButton_SearchStorage->setEnabled(true);
    //     // ui->Storage_pushButton_SearchLocation->setEnabled(true);
    //     // ui->Storage_pushButton_CreateCatalog->setEnabled(true);
    //     // ui->Storage_pushButton_Update->setEnabled(true);
    //     // ui->Storage_pushButton_Delete->setEnabled(true);
    //     // ui->Storage_pushButton_OpenFilelight->setEnabled(true);

    //     selectedStorageIndexRow = index.row();

    //     loadStorageToPanel();
    // }
    //--------------------------------------------------------------------------

    //With seleted storage -----------------------------------------------------
    // void MainWindow::on_Storage_pushButton_New_clicked()
    // {
    //     addStorageDevice(tr("Storage"));
    // }
    //--------------------------------------------------------------------------
    // void MainWindow::on_Storage_pushButton_SearchStorage_clicked()
    // {
    //     //Change tab to show the Search screen
    //     ui->tabWidget->setCurrentIndex(0); // tab 0 is the Search tab

    //     ui->Filters_label_DisplayStorage->setText(selectedDevice->storage->name);
    // }
    //--------------------------------------------------------------------------

    //--------------------------------------------------------------------------
    // void MainWindow::on_Storage_pushButton_CreateCatalog_clicked()
    // {
    //     //Send selection to Create screen
    //     ui->Create_lineEdit_NewCatalogPath->setText(selectedDevice->storage->path);
    //     ui->Create_comboBox_StorageSelection->setCurrentText(selectedDevice->storage->name);
    //     ui->Create_lineEdit_NewCatalogName->setText(selectedDevice->storage->name);

    //     //Select this directory in the treeview.
    //     loadFileSystem(selectedDevice->storage->path);

    //     //Change tab to show the result of the catalog creation
    //     ui->tabWidget->setCurrentIndex(3); // tab 3 is the Create catalog tab

    // }
    //--------------------------------------------------------------------------
    // void MainWindow::on_Storage_pushButton_OpenFilelight_clicked()
    // {
    //     QProcess::startDetached("filelight", QStringList() << selectedDevice->storage->path);
    // }
    //--------------------------------------------------------------------------
    // void MainWindow::on_Storage_pushButton_Update_clicked()
    // {
    //     reportAllUpdates(activeDevice,
    //                      activeDevice->updateDevice("update",
    //                                                 collection->databaseMode,
    //                                                 true,
    //                                                 collection->collectionFolder,
    //                                                 true),
    //                      "update");
    //     collection->saveDeviceTableToFile();
    //     collection->saveStorageTableToFile();
    //     collection->saveStatiticsToFile();
    //     loadDevicesView();
    //     loadCatalogsTableToModel();
    //     loadStorageTableToModel();
    //     loadStorageToPanel();
    //     loadStatisticsChart();
    // }
    //--------------------------------------------------------------------------
    // void MainWindow::on_Storage_pushButton_Delete_clicked()
    // {
    //         deleteDeviceItem();

    //         //Disable buttons to force new selection
    //         // ui->Storage_pushButton_SearchLocation->setEnabled(false);
    //         // ui->Storage_pushButton_SearchStorage->setEnabled(false);
    //         // ui->Storage_pushButton_CreateCatalog->setEnabled(false);
    //         // ui->Storage_pushButton_OpenFilelight->setEnabled(false);
    //         // ui->Storage_pushButton_Delete->setEnabled(false);

    //         //Refresh storage screen statistics
    //         updateStorageSelectionStatistics();
    // }
    //--------------------------------------------------------------------------
    // void MainWindow::on_StorageTreeViewStorageListHeaderSortOrderChanged(){

    //     QSettings settings(collection->settingsFilePath, QSettings:: IniFormat);
    //     QHeaderView *storageTreeHeader = ui->Storage_treeView_StorageList->header();

    //     lastStorageSortSection = storageTreeHeader->sortIndicatorSection();
    //     lastStorageSortOrder   = storageTreeHeader->sortIndicatorOrder();

    //     settings.setValue("Storage/lastStorageSortSection", lastStorageSortSection);
    //     settings.setValue("Storage/lastStorageSortOrder",   lastStorageSortOrder);
    // }
    //--------------------------------------------------------------------------
    // void MainWindow::on_Storage_treeView_StorageList_doubleClicked()
    // {
    //     unsavedChanges = true;
    //     ui->Storage_pushButton_SaveAll->setStyleSheet("color: orange");
    // }
    //--------------------------------------------------------------------------

    //Panel --------------------------------------------------------------------
    // void MainWindow::on_Storage_pushButton_PanelSave_clicked()
    // {
    //     saveStorageFromPanel();
    // }
    //--------------------------------------------------------------------------
    // void MainWindow::on_Storage_pushButton_PanelCancel_clicked()
    // {
    //     loadStorageToPanel();
    //     //ui->Storage_widget_Panel->hide();
    // }
    //--------------------------------------------------------------------------

//Methods-----------------------------------------------------------------------

    //--------------------------------------------------------------------------
/*
    void MainWindow::addStorageDevice(QString deviceName)
    {
        //Create Storage entry
        Storage *tempStorage = new Storage;
            tempStorage->name = deviceName;
            tempStorage->generateID();
            tempStorage->insertStorage();

        //Load table to model
        //loadStorageTableToModel();

        //Save data to file and reload
        collection->saveStorageTableToFile();
        collection->loadStorageFileToTable();

        //Refresh
        //loadStorageTableToModel();
        updateStorageSelectionStatistics();

        //Enable save button
        // ui->Storage_pushButton_New->setEnabled(true);

        //Create virtual storage under Physical group (ID=0) / Default device (ID=2)
        Device *newDeviceItem = new Device();
        newDeviceItem->generateDeviceID();
        newDeviceItem->parentID = 2;
        newDeviceItem->name = tempStorage->name;
        newDeviceItem->type = "Storage";
        newDeviceItem->groupID = 0;
        newDeviceItem->externalID = tempStorage->ID;
        newDeviceItem->insertDevice();

        //Save data to file
        collection->saveDeviceTableToFile();

        //Reload
        loadDevicesView();
        //loadStorageTableToModel();
    }
*/
    //--------------------------------------------------------------------------
    /*
    void MainWindow::loadStorageTableToModel()
    {
        //Generate SQL query from filters
        QSqlQuery loadStorageQuery;
        QString loadStorageQuerySQL;

        //Prepare the main part of the query
        loadStorageQuerySQL  = QLatin1String(R"(
                                        SELECT
                                            storage_id            ,
                                            device_name           ,
                                            storage_type          ,
                                            storage_location      ,
                                            device_path           ,
                                            storage_label         ,
                                            storage_file_system   ,
                                            device_total_space    ,
                                            device_free_space     ,
                                            storage_brand_model   ,
                                            storage_serial_number ,
                                            storage_build_date    ,
                                            storage_content_type  ,
                                            storage_container     ,
                                            storage_comment       ,
                                            storage_date_updated  ,
                                            device_id
                                        FROM storage s
                                        JOIN device d ON d.device_external_id = s.storage_id
                            )");

        if ( selectedDevice->ID == 0 ){
            //No filter
        }
        else if ( selectedDevice->type == "Catalog" ){
            loadStorageQuerySQL += " WHERE device_id =:device_parent_id";
        }
        else{
            QString prepareSQL = QLatin1String(R"(
                                    WHERE d.device_id IN (
                                    WITH RECURSIVE hierarchy AS (
                                         SELECT device_id, device_parent_id, device_name
                                         FROM device
                                         WHERE device_id = :device_id
                                         UNION ALL
                                         SELECT t.device_id, t.device_parent_id, t.device_name
                                         FROM device t
                                         JOIN hierarchy h ON t.device_parent_id = h.device_id
                                    )
                                    SELECT device_id
                                    FROM hierarchy)
                                                    )");
            loadStorageQuerySQL += prepareSQL;
        }

        loadStorageQuerySQL += " AND device_type ='Storage' ";

        //Execute query
        loadStorageQuery.prepare(loadStorageQuerySQL);
        loadStorageQuery.bindValue(":device_id", selectedDevice->ID);
        loadStorageQuery.bindValue(":device_parent_id", selectedDevice->parentID);
        loadStorageQuery.exec();
        loadStorageQuery.next();

        // Connect model to tree/table view
        QSqlQueryModel *storageQueryModel = new QSqlQueryModel();
        storageQueryModel->setQuery(std::move(loadStorageQuery));

        StorageView *proxyStorageModel = new StorageView(this);
        proxyStorageModel->setSourceModel(storageQueryModel);
        proxyStorageModel->setHeaderData(0, Qt::Horizontal, tr("ID"));
        proxyStorageModel->setHeaderData(1, Qt::Horizontal, tr("Name"));
        proxyStorageModel->setHeaderData(2, Qt::Horizontal, tr("Type"));
        proxyStorageModel->setHeaderData(3, Qt::Horizontal, tr("Location"));
        proxyStorageModel->setHeaderData(4, Qt::Horizontal, tr("Path"));
        proxyStorageModel->setHeaderData(5, Qt::Horizontal, tr("Label"));
        proxyStorageModel->setHeaderData(6, Qt::Horizontal, tr("FileSystem"));
        proxyStorageModel->setHeaderData(7, Qt::Horizontal, tr("Total"));
        proxyStorageModel->setHeaderData(8, Qt::Horizontal, tr("Free"));
        proxyStorageModel->setHeaderData(9, Qt::Horizontal, tr("Brand/Model"));
        proxyStorageModel->setHeaderData(10, Qt::Horizontal, tr("Serial Number"));
        proxyStorageModel->setHeaderData(11, Qt::Horizontal, tr("Build Date"));
        proxyStorageModel->setHeaderData(12, Qt::Horizontal, tr("Content Type"));
        proxyStorageModel->setHeaderData(13, Qt::Horizontal, tr("Container"));
        proxyStorageModel->setHeaderData(14, Qt::Horizontal, tr("Comment"));
        proxyStorageModel->setHeaderData(15, Qt::Horizontal, tr("Last Update"));
        proxyStorageModel->setHeaderData(16, Qt::Horizontal, tr("Device ID"));

        ui->Storage_treeView_StorageList->setModel(proxyStorageModel);

        ui->Storage_treeView_StorageList->header()->setSectionResizeMode(QHeaderView::Interactive);
        ui->Storage_treeView_StorageList->header()->resizeSection(0,  50); //ID
        ui->Storage_treeView_StorageList->header()->resizeSection(1, 225); //Name
        ui->Storage_treeView_StorageList->header()->resizeSection(2, 125); //Type
        ui->Storage_treeView_StorageList->header()->resizeSection(3, 150); //Location
        ui->Storage_treeView_StorageList->header()->resizeSection(4, 250); //Path
        ui->Storage_treeView_StorageList->header()->resizeSection(5, 125); //Label
        ui->Storage_treeView_StorageList->header()->resizeSection(6,  75); //FS
        ui->Storage_treeView_StorageList->header()->resizeSection(7,  85); //Total
        ui->Storage_treeView_StorageList->header()->resizeSection(8,  85); //Free
        ui->Storage_treeView_StorageList->header()->resizeSection(9, 150); //BrandModel
        ui->Storage_treeView_StorageList->header()->resizeSection(10,150); //Serial
        ui->Storage_treeView_StorageList->header()->resizeSection(11, 75); //Build date
        ui->Storage_treeView_StorageList->header()->resizeSection(12,125); //Content
        ui->Storage_treeView_StorageList->header()->resizeSection(13,125); //Container
        ui->Storage_treeView_StorageList->header()->resizeSection(14, 50); //Comment
        ui->Storage_treeView_StorageList->header()->resizeSection(15,100); //Last Update
        ui->Storage_treeView_StorageList->header()->resizeSection(16, 50); //Device ID

        ui->Storage_treeView_StorageList->header()->hideSection( 3); //Location
        ui->Storage_treeView_StorageList->header()->hideSection(15); //Last Update

        //Get the list of device names for the Create screen
        loadStorageList();

        //If a storage is selected, use it for the Create screen
        if ( selectedDevice->type == "Storage" ){
            ui->Create_comboBox_StorageSelection->setCurrentText(selectedDevice->name);
        }
        else if ( selectedDevice->type == "Catalog" ){
            ui->Create_comboBox_StorageSelection->setCurrentText(selectedDevice->catalog->storageName);
            ui->Create_lineEdit_NewCatalogPath->setText(selectedDevice->catalog->sourcePath);
        }

        //Enable buttons
            //ui->Storage_pushButton_Reload->setEnabled(true);
            ui->Storage_pushButton_EditAll->setEnabled(true);
            //ui->Storage_pushButton_SaveAll->setEnabled(true);
            // ui->Storage_pushButton_New->setEnabled(true);

            //Disable create button so it cannot be overwritten
            //ui->Storage_pushButton_CreateList->setEnabled(false);

        //Update Storage screen statistics
            updateStorageSelectionStatistics();

    }
*/





    //--------------------------------------------------------------------------
    /*
    void MainWindow::loadStorageToPanel()
    {//Load selected Storage device to the edition panel
        ui->Storage_lineEdit_Panel_ID->setText(QString::number(activeDevice->storage->ID));
        ui->Storage_label_NameDisplay->setText(activeDevice->storage->name);
        ui->Storage_lineEdit_Panel_Type->setText(activeDevice->storage->type);
        ui->Storage_label_Panel_Path->setText(activeDevice->storage->path);
        ui->Storage_lineEdit_Panel_Label->setText(activeDevice->storage->label);
        ui->Storage_lineEdit_Panel_FileSystem->setText(activeDevice->storage->fileSystem);

        ui->Storage_lineEdit_Panel_Total->setText(QString::number(activeDevice->storage->totalSpace));
        ui->Storage_lineEdit_Panel_Free->setText(QString::number(activeDevice->storage->freeSpace));
        ui->Storage_label_Panel_TotalSpace->setText(QLocale().formattedDataSize(activeDevice->storage->totalSpace));
        ui->Storage_label_Panel_FreeSpace->setText(QLocale().formattedDataSize(activeDevice->storage->freeSpace));

        ui->Storage_lineEdit_Panel_BrandModel->setText(activeDevice->storage->brand_model);
        ui->Storage_lineEdit_Panel_SerialNumber->setText(activeDevice->storage->serialNumber);
        ui->Storage_lineEdit_Panel_BuildDate->setText(activeDevice->storage->buildDate);
        ui->Storage_lineEdit_Panel_ContentType->setText(activeDevice->storage->contentType);
        ui->Storage_lineEdit_Panel_Container->setText(activeDevice->storage->container);
        ui->Storage_lineEdit_Panel_Comment->setText(activeDevice->storage->comment);

        displayStoragePicture();
    }
*/
    //--------------------------------------------------------------------------
/*    void MainWindow::saveStorageFromPanel()
    {//Save changes to selected Storage device from the edition panel

        //Update
        QString querySQL = QLatin1String(R"(
                                    UPDATE storage
                                    SET storage_id = :new_storage_id,
                                        storage_type =:storage_type,
                                        storage_location =:storage_location,
                                        storage_label =:storage_label,
                                        storage_file_system =:storage_file_system,
                                        storage_total_space =:storage_total_space,
                                        storage_free_space =:storage_free_space,
                                        storage_brand_model =:storage_brand_model,
                                        storage_serial_number =:storage_serial_number,
                                        storage_build_date =:storage_build_date,
                                        storage_content_type =:storage_content_type,
                                        storage_container =:storage_container,
                                        storage_comment = :storage_comment
                                    WHERE storage_id =:storage_id
                                )");

        QSqlQuery updateQuery;
        updateQuery.prepare(querySQL);
        updateQuery.bindValue(":new_storage_id",        ui->Storage_lineEdit_Panel_ID->text());
        updateQuery.bindValue(":storage_type",          ui->Storage_lineEdit_Panel_Type->text());
        updateQuery.bindValue(":storage_label",         ui->Storage_lineEdit_Panel_Label->text());
        updateQuery.bindValue(":storage_file_system",   ui->Storage_lineEdit_Panel_FileSystem->text());
        updateQuery.bindValue(":storage_total_space",   ui->Storage_lineEdit_Panel_Total->text());
        updateQuery.bindValue(":storage_free_space",    ui->Storage_lineEdit_Panel_Free->text());
        updateQuery.bindValue(":storage_brand_model",   ui->Storage_lineEdit_Panel_BrandModel->text());
        updateQuery.bindValue(":storage_serial_number", ui->Storage_lineEdit_Panel_SerialNumber->text());
        updateQuery.bindValue(":storage_build_date",    ui->Storage_lineEdit_Panel_BuildDate->text());
        updateQuery.bindValue(":storage_content_type",  ui->Storage_lineEdit_Panel_ContentType->text());
        updateQuery.bindValue(":storage_container",     ui->Storage_lineEdit_Panel_Container->text());
        updateQuery.bindValue(":storage_comment",       ui->Storage_lineEdit_Panel_Comment->text());
        updateQuery.bindValue(":storage_id",            activeDevice->storage->ID);
        updateQuery.exec();

        //loadStorageTableToModel();
        updateStorageSelectionStatistics();

        //Save data to file
        collection->saveStorageTableToFile();
    }
    //--------------------------------------------------------------------------
*/
