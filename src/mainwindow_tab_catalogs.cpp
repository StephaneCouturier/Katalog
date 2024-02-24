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
// File Name:   mainwindow_tab_catalogs.cpp
// Purpose:     methods for the screen Catalogs
// Description: https://github.com/StephaneCouturier/Katalog/wiki/Catalogs
// Author:      Stephane Couturier
/////////////////////////////////////////////////////////////////////////////
*/

#include "mainwindow.h"
#include "ui_mainwindow.h"

#include "catalog.h"
#include "catalogsview.h"

//UI----------------------------------------------------------------------------
//------------------------------------------------------------------------------
    //Catalog UI display
        void MainWindow::on_CatalogsTreeViewCatalogListHeaderSortOrderChanged(){

            QSettings settings(collection->settingsFilePath, QSettings:: IniFormat);

            QHeaderView *catalogsTreeHeader = ui->Catalogs_treeView_CatalogList->header();

            lastCatalogsSortSection = catalogsTreeHeader->sortIndicatorSection();
            lastCatalogsSortOrder   = catalogsTreeHeader->sortIndicatorOrder();

            settings.setValue("Catalogs/lastCatlogsSortSection", lastCatalogsSortSection);
            settings.setValue("Catalogs/lastCatlogsSortOrder",   lastCatalogsSortOrder);
        }
        //----------------------------------------------------------------------
    //Catalog operations
        void MainWindow::on_Catalogs_treeView_CatalogList_clicked(const QModelIndex &index)
        {
            activeDevice->ID = ui->Catalogs_treeView_CatalogList->model()->index(index.row(), 15, QModelIndex()).data().toInt();
            activeDevice->loadDevice();

            // Display buttons
            ui->Catalogs_pushButton_Search->setEnabled(true);
            ui->Catalogs_pushButton_ExploreCatalog->setEnabled(true);
            ui->Catalogs_pushButton_Open->setEnabled(true);
            ui->Catalogs_pushButton_EditCatalogFile->setEnabled(true);
            ui->Catalogs_pushButton_UpdateCatalog->setEnabled(true);
            ui->Catalogs_pushButton_ViewCatalogStats->setEnabled(true);
            ui->Catalogs_pushButton_DeleteCatalog->setEnabled(true);
            ui->Devices_pushButton_AssignCatalog->setEnabled(true);

            //Load catalog values to the Edit area
            ui->Catalogs_label_NameDisplay->setText(activeDevice->catalog->name);
            ui->Catalogs_label_Path->setText(activeDevice->catalog->sourcePath);
            ui->Catalogs_comboBox_FileType->setCurrentText(activeDevice->catalog->fileType);
            ui->Catalogs_label_StorageDisplay->setText(activeDevice->catalog->storageName);
            ui->Catalogs_checkBox_IncludeHidden->setChecked(activeDevice->catalog->includeHidden);
            ui->Catalogs_checkBox_IncludeMetadata->setChecked(activeDevice->catalog->includeMetadata);
            //DEV: ui->Catalogs_checkBox_isFullDevice->setChecked(selectedCatalogIsFullDevice);
            ui->Devices_label_SelectedCatalogDisplay->setText(activeDevice->catalog->name);
        }
        //----------------------------------------------------------------------
        void MainWindow::on_Catalogs_treeView_CatalogList_doubleClicked()
        {
            //The activeDevice becomes the selectedDevice and exploreDevice
            displaySelectedDeviceName();
            selectedDevice->ID = activeDevice->ID;
            selectedDevice->loadDevice();
            exploreDevice->ID = activeDevice->ID;
            exploreDevice->loadDevice();

            exploreSelectedFolderFullPath = exploreDevice->path;
            exploreSelectedDirectoryName  = exploreDevice->path;

            openCatalogToExplore();

            //Go to explore tab
            ui->tabWidget->setCurrentIndex(2);


            //Load
            openCatalogToExplore();

            //Go to explore tab
            ui->tabWidget->setCurrentIndex(2);
        }
        //----------------------------------------------------------------------
        void MainWindow::on_Catalogs_pushButton_Search_clicked()
        {//Change the selected device and catalog, and go to Search tab

            //Make the catalog the selected device
            selectedDevice->ID = activeDevice->ID;
            selectedDevice->loadDevice();

            //Update the displayed name
            displaySelectedDeviceName();

            QSettings settings(collection->settingsFilePath, QSettings:: IniFormat);
            settings.setValue("Selection/SelectedDeviceID", selectedDevice->ID);

            filterFromSelectedDevice();

            //Go to the Search tab
            ui->tabWidget->setCurrentIndex(0); // tab 0 is the Search tab
        }
        //----------------------------------------------------------------------
        void MainWindow::on_Catalogs_pushButton_ExploreCatalog_clicked()
        {
                //The activeDevice becomes the selectedDevice
                selectedDevice->ID = activeDevice->ID;
                selectedDevice->loadDevice();
                displaySelectedDeviceName();

                //Load
                openCatalogToExplore();

                //Go to explore tab
                ui->tabWidget->setCurrentIndex(2);
        }
        //----------------------------------------------------------------------
        void MainWindow::on_Catalogs_pushButton_Cancel_clicked()
        {
            ui->Catalogs_label_NameDisplay->setText(selectedDevice->catalog->name);
            ui->Catalogs_comboBox_FileType->setCurrentText(selectedDevice->catalog->fileType);
            ui->Catalogs_label_StorageDisplay->setText(selectedDevice->catalog->storageName);
            ui->Catalogs_checkBox_IncludeHidden->setChecked(selectedDevice->catalog->includeHidden);

            ui->Catalogs_widget_EditCatalog->hide();
        }
        //----------------------------------------------------------------------
        void MainWindow::on_Catalogs_pushButton_UpdateCatalog_clicked()
        {
            reportAllUpdates(activeDevice,
                             activeDevice->updateDevice("update",
                                                        collection->databaseMode,
                                                        false,
                                                        collection->collectionFolder,
                                                        true),
                             "update");
            collection->saveDeviceTableToFile();
            collection->saveStatiticsToFile();

            loadDeviceTableToTreeModel();
            loadCatalogsTableToModel();
            loadStatisticsChart();
        }
        //----------------------------------------------------------------------
        void MainWindow::on_Catalogs_pushButton_UpdateAllActive_clicked()
        {
            globalUpdateTotalFiles = 0;
            globalUpdateDeltaFiles = 0;
            globalUpdateTotalSize  = 0;
            globalUpdateDeltaSize  = 0;

            //User to choose showing or skipping summary for each catalog update
                bool showEachCatalogUpdateSummary = false;

                QMessageBox msgBox;
                msgBox.setWindowTitle("Katalog");
                msgBox.setText(tr("Update all active catalogs")+"<br/><br/>"+tr("Do you want a the summary of updates for each catalog?"));
                msgBox.setIcon(QMessageBox::Question);
                msgBox.setStandardButtons(QMessageBox::Yes|QMessageBox::No | QMessageBox::Cancel);
                int result = msgBox.exec();

                if ( result == QMessageBox::Yes){
                    showEachCatalogUpdateSummary = true;
                }
                else if ( result == QMessageBox::Cancel){
                    return;
                }

                int updatedCatalogs = 0;
                int skippedCatalogs = 0;

            // Loop through each row of the displayed model
                Device loopDevice;
                for (int row = 0; row < ui->Catalogs_treeView_CatalogList->model()->rowCount(); ++row) {
                    // Get the index for the "Active" field in the current row
                    QModelIndex activeIndex = ui->Catalogs_treeView_CatalogList->model()->index(row, 7);

                    // Retrieve the data for the "Active" field (assuming it contains an icon)
                    QIcon activeIcon = qvariant_cast<QIcon>(ui->Catalogs_treeView_CatalogList->model()->data(activeIndex, Qt::DecorationRole));

                    // Check if the icon is set to "dialog-ok-apply"
                    if (activeIcon.name() == QIcon::fromTheme("dialog-ok-apply").name()) {
                        updatedCatalogs +=1;
                        loopDevice.ID = ui->Catalogs_treeView_CatalogList->model()->data(ui->Catalogs_treeView_CatalogList->model()->index(row, 15)).toInt();
                        loopDevice.loadDevice();

                        QList<qint64> list = loopDevice.updateDevice("update",
                                                                     collection->databaseMode,
                                                                     false,
                                                                     collection->collectionFolder,
                                                                     true);
                        if ( showEachCatalogUpdateSummary == true ){
                            reportAllUpdates(&loopDevice, list, "update");
                        }

                        if(list.count()>0){
                            globalUpdateTotalFiles += list[1];
                            globalUpdateDeltaFiles += list[2];
                            globalUpdateTotalSize  += list[3];
                            globalUpdateDeltaSize  += list[4];
                        }
                    }
                    else
                        skippedCatalogs +=1;
                }

            QList<qint64> globalList;
            globalList <<1;
            globalList <<globalUpdateTotalFiles;
            globalList <<globalUpdateDeltaFiles;
            globalList <<globalUpdateTotalSize;
            globalList <<globalUpdateDeltaSize;
            globalList <<updatedCatalogs;
            globalList <<skippedCatalogs;
            globalList <<0;
            globalList <<0;
            globalList <<0;
            globalList <<0;
            globalList <<0;
            globalList <<0;
            globalList <<0;

            reportAllUpdates(selectedDevice, globalList, "list");

            collection->saveDeviceTableToFile();
            collection->saveStatiticsToFile();

            loadDeviceTableToTreeModel();
            loadCatalogsTableToModel();
        }
        //----------------------------------------------------------------------
        void MainWindow::on_Catalogs_pushButton_EditCatalogFile_clicked()
        {
            //save catalogs
            int result = QMessageBox::warning(this, "Katalog",
                                tr("You are about to edit the catalog file directly.<br/><br/>"
                                "It generally recommended to Create a new catalog with the right initial settings (source path, file type, include Hidden Files, storage), rather than modify the catalog file directly.<br/><br/>"
                                "Check the Wiki page <a href='https://github.com/StephaneCouturier/Katalog/wiki/Catalogs#edit'>Catalogs/Edit</a> to understand the impact of changing this file directly.<br/><br/>"
                                "Do you want to continue anyway?")
                                     , QMessageBox::Yes
                                              | QMessageBox::Cancel);
            if ( result == QMessageBox::Cancel){
                return;
            }
            else
                ui->Catalogs_widget_EditCatalog->show();
        }
        //----------------------------------------------------------------------
        void MainWindow::on_Catalogs_pushButton_ViewCatalogStats_clicked()
        {
            //The active catalog becomes the selected catalog
            selectedDevice->ID = activeDevice->ID;
            selectedDevice->loadDevice();

            //Update the displayed name
            ui->Filters_label_DisplayCatalog->setText(selectedDevice->name);

            QSettings settings(collection->settingsFilePath, QSettings:: IniFormat);
            settings.setValue("Selection/SelectedDeviceID", selectedDevice->ID);

            filterFromSelectedDevice();

            //Go to the Search tab
            ui->tabWidget->setCurrentIndex(6); // tab 6 is the Statitics tab

            //Select the type of display "selected catalog"
            ui->Statistics_comboBox_SelectSource->setCurrentText(tr("selected catalog"));

            //load the graph
            loadStatisticsChart();
        }
        //----------------------------------------------------------------------
        void MainWindow::on_Catalogs_pushButton_Import_clicked()
        {
                importFromVVV();
        }
        //----------------------------------------------------------------------
        void MainWindow::on_Catalogs_pushButton_DeleteCatalog_clicked()
        {
            int result = QMessageBox::warning(this,"Katalog",
                                                  tr("Do you want to delete this catalog?")+"\n"+ selectedDevice->catalog->name,QMessageBox::Yes|QMessageBox::Cancel);

            if ( result ==QMessageBox::Yes){

                activeDevice->deleteDevice(true);

                collection->deleteCatalogFile(activeDevice);

                loadCatalogsTableToModel();
            }
        }
        //----------------------------------------------------------------------
        void MainWindow::on_Catalogs_pushButton_Open_clicked()
        {
            //save catalogs
            int result = QMessageBox::warning(this, "Katalog",
                                tr("You are about to edit the catalog file directly.<br/><br/>"
                                "It generally recommended to Create a new catalog with the right initial settings (source path, file type, include Hidden Files, storage), rather than modify the catalog file directly.<br/><br/>"
                                "Check the Wiki page <a href='https://github.com/StephaneCouturier/Katalog/wiki/Catalogs#edit'>Catalogs/Edit</a> to understand the impact of changing this file directly.<br/><br/>"
                                "Do you want to continue anyway?")
                                     , QMessageBox::Yes
                                              | QMessageBox::Cancel);
            if ( result == QMessageBox::Cancel){
                return;
            }
            else
                QDesktopServices::openUrl(QUrl::fromLocalFile(selectedDevice->catalog->filePath));
        }
        //----------------------------------------------------------------------
        void MainWindow::on_Catalogs_pushButton_Save_clicked()
        {
            saveCatalogChanges();
        }
        //----------------------------------------------------------------------
        void MainWindow::on_Catalogs_pushButton_Snapshot_clicked()
        {
            recordDevicesSnapshot();
        }
        //----------------------------------------------------------------------

//Methods-----------------------------------------------------------------------
    void MainWindow::hideCatalogButtons()
    {
        //Hide buttons
        ui->Catalogs_pushButton_Search->setEnabled(false);
        ui->Catalogs_pushButton_ExploreCatalog->setEnabled(false);
        ui->Catalogs_pushButton_EditCatalogFile->setEnabled(false);
        ui->Catalogs_pushButton_UpdateCatalog->setEnabled(false);
        ui->Catalogs_pushButton_ViewCatalogStats->setEnabled(false);
        ui->Catalogs_pushButton_DeleteCatalog->setEnabled(false);
    }
    //--------------------------------------------------------------------------
    void MainWindow::loadCatalogsTableToModel()
    {
        //Refresh active state
        updateAllDeviceActive();

        //Get catalog data based on filters
            //Generate SQL query from filters
            QSqlQuery loadCatalogQuery;
            QString loadCatalogQuerySQL;

            //Prepare the main part of the query
            loadCatalogQuerySQL  = QLatin1String(R"(
                                        SELECT
                                            d.device_name                  ,
                                            c.catalog_file_path            ,
                                            c.catalog_date_updated         ,
                                            d.device_total_file_count      ,
                                            d.device_total_file_size       ,
                                            d.device_path                  ,
                                            c.catalog_file_type            ,
                                            d.device_active                ,
                                            c.catalog_include_hidden       ,
                                            c.catalog_include_metadata     ,
                                            (SELECT e.device_name FROM device e WHERE e.device_id = d.device_parent_id),
                                            c.catalog_is_full_device       ,
                                            c.catalog_date_loaded          ,
                                            c.catalog_app_version          ,
                                            c.catalog_id                   ,
                                            d.device_id
                                        FROM device d
                                        JOIN catalog c ON d.device_external_id = c.catalog_id
                                    )");

            if (      selectedDevice->type == "Storage" ){
                loadCatalogQuerySQL += " WHERE device_parent_id =:device_parent_id ";
            }
            else if ( selectedDevice->type == "Catalog" ){
                loadCatalogQuerySQL += " WHERE device_id =:device_id ";
            }
            else if ( selectedDevice->type == "Virtual" ){
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
                loadCatalogQuerySQL += prepareSQL;
            }

            loadCatalogQuerySQL += " AND device_type = 'Catalog' ";

            //Execute query
            loadCatalogQuery.prepare(loadCatalogQuerySQL);
            loadCatalogQuery.bindValue(":device_id",        selectedDevice->ID);
            loadCatalogQuery.bindValue(":device_parent_id", selectedDevice->ID);
            loadCatalogQuery.exec();

        //Put the results in a list
            //Clear the list of selected catalogs
            catalogSelectedList.clear();

            //Populate from the query results
            while (loadCatalogQuery.next()) {
                catalogSelectedList << loadCatalogQuery.value(0).toString();
            }

            //Send list to the Statistics combobox
            QStringListModel *catalogListModelForStats = new QStringListModel(this);
            catalogListModelForStats->setStringList(catalogSelectedList);

        //Format and send to Treeview
            QSqlQueryModel *catalogQueryModel = new QSqlQueryModel;
            catalogQueryModel->setQuery(std::move(loadCatalogQuery));

            CatalogsView *proxyResultsModel = new CatalogsView(this);
            proxyResultsModel->setSourceModel(catalogQueryModel);

            proxyResultsModel->setHeaderData(0, Qt::Horizontal, tr("Name"));
            proxyResultsModel->setHeaderData(1, Qt::Horizontal, tr("File path"));
            proxyResultsModel->setHeaderData(2, Qt::Horizontal, tr("Last Update"));
            proxyResultsModel->setHeaderData(3, Qt::Horizontal, tr("Files"));
            proxyResultsModel->setHeaderData(4, Qt::Horizontal, tr("Total Size"));
            proxyResultsModel->setHeaderData(5, Qt::Horizontal, tr("Source Path"));
            proxyResultsModel->setHeaderData(6, Qt::Horizontal, tr("File Type"));
            proxyResultsModel->setHeaderData(7, Qt::Horizontal, tr("Active"));
            proxyResultsModel->setHeaderData(8, Qt::Horizontal, tr("Inc.Hidden"));
            proxyResultsModel->setHeaderData(9, Qt::Horizontal, tr("Inc.Metadata"));
            proxyResultsModel->setHeaderData(10,Qt::Horizontal, tr("Storage"));
            proxyResultsModel->setHeaderData(11,Qt::Horizontal, tr("Full Device"));
            proxyResultsModel->setHeaderData(12,Qt::Horizontal, tr("Last Loaded"));
            proxyResultsModel->setHeaderData(13,Qt::Horizontal, tr("App Version"));
            proxyResultsModel->setHeaderData(14,Qt::Horizontal, tr("Catalog ID"));
            proxyResultsModel->setHeaderData(15,Qt::Horizontal, tr("Device ID"));

            //Connect model to tree/table view
            ui->Catalogs_treeView_CatalogList->setModel(proxyResultsModel);
            //ui->Catalogs_treeView_CatalogList->QTreeView::sortByColumn(1,Qt::AscendingOrder);
            ui->Catalogs_treeView_CatalogList->header()->setSectionResizeMode(QHeaderView::Interactive);

            //Hide column with file path
            ui->Catalogs_treeView_CatalogList->header()->hideSection(1); //Path

            //Change columns size
            ui->Catalogs_treeView_CatalogList->header()->resizeSection(0, 450); //Name
            ui->Catalogs_treeView_CatalogList->header()->resizeSection(2, 150); //Date
            ui->Catalogs_treeView_CatalogList->header()->resizeSection(3,  80); //Files
            ui->Catalogs_treeView_CatalogList->header()->resizeSection(4, 100); //TotalFileSize
            ui->Catalogs_treeView_CatalogList->header()->setSectionResizeMode(5, QHeaderView::ResizeToContents);
            ui->Catalogs_treeView_CatalogList->header()->resizeSection(6, 100); //FileType
            ui->Catalogs_treeView_CatalogList->header()->resizeSection(7,  50); //Active
            ui->Catalogs_treeView_CatalogList->header()->resizeSection(8,  50); //includeHidden
            ui->Catalogs_treeView_CatalogList->header()->resizeSection(9,  50); //includeMetadata
            ui->Catalogs_treeView_CatalogList->header()->resizeSection(10,150); //Storage
            ui->Catalogs_treeView_CatalogList->header()->resizeSection(11, 50); //FullDevice
            ui->Catalogs_treeView_CatalogList->header()->resizeSection(12,150); //Last Loaded
            ui->Catalogs_treeView_CatalogList->header()->resizeSection(13,150); //App Version
            ui->Catalogs_treeView_CatalogList->header()->resizeSection(14, 50); //Catalog ID
            ui->Catalogs_treeView_CatalogList->header()->resizeSection(15, 50); //Device ID

            //Hide columns
            if(developmentMode==false){
                ui->Catalogs_treeView_CatalogList->hideColumn( 9); //includeMetadata
                ui->Catalogs_treeView_CatalogList->hideColumn(13); //date Loaded
                ui->Catalogs_treeView_CatalogList->hideColumn(12); //isFullDevice
            }

        //Update Catalogs screen statistics
            updateCatalogsScreenStatistics();

    }
    //--------------------------------------------------------------------------
    void MainWindow::updateCatalogsScreenStatistics()
    {
        QSqlQuery querySumCatalogValues;

        //Prepare the query
        QString querySumCatalogValuesSQL  = QLatin1String(R"(
                                        SELECT  COUNT(device_id),
                                                SUM(device_total_file_size),
                                                SUM(device_total_file_count)
                                        FROM device d
                                    )");

        if (      selectedDevice->type == "Storage" ){
            querySumCatalogValuesSQL += " WHERE device_parent_id =:device_parent_id ";
        }
        else if ( selectedDevice->type == "Catalog" ){
            querySumCatalogValuesSQL += " WHERE device_id =:device_id ";
        }
        else if ( selectedDevice->type == "Virtual" ){
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
            querySumCatalogValuesSQL += prepareSQL;

        }
        querySumCatalogValuesSQL += " AND device_type = 'Catalog' ";

        //Execute and use results
        querySumCatalogValues.prepare(querySumCatalogValuesSQL);
        querySumCatalogValues.bindValue(":device_id", selectedDevice->ID);
        querySumCatalogValues.bindValue(":device_parent_id", selectedDevice->ID);
        querySumCatalogValues.exec();
        querySumCatalogValues.next();

        ui->Catalogs_label_Catalogs->setText(QString::number(querySumCatalogValues.value(0).toInt()));
        ui->Catalogs_label_TotalSize->setText(QLocale().formattedDataSize(querySumCatalogValues.value(1).toLongLong()));
        ui->Catalogs_label_TotalNumber->setText(QLocale().toString(querySumCatalogValues.value(2).toInt()));
    }
    //--------------------------------------------------------------------------
    void MainWindow::backupFile(QString filePath)
    {//Copy the file to the same location, adding .bak for the new file name.
        QString targetFilePath = filePath + ".bak";

        //Verify if a bak up file already exist and remove it.
        if (QFile::exists(targetFilePath))
        {
            QFile::remove(targetFilePath);
        }

        //Copy
        QFile::copy(filePath, targetFilePath);
    }
    //--------------------------------------------------------------------------
    void MainWindow::importFromVVV()
    {
        //Select file

            //Get path of the file to import
            QString sourceFilePath = QFileDialog::getOpenFileName(this, tr("Select the csv file to be imported"), collection->collectionFolder);

            //Stop if no path is selected
            if ( sourceFilePath=="" ) return;

            //Define file
            QFile sourceFile(sourceFilePath);

            //Prepare a dateTime to add to device or catalog anmes and avoid duplicates
            QString dateTimeForCatalogName = "_" + QDateTime::currentDateTime().toString("yy-MM-ss hh-mm-ss");


        //Open the source file and load all data into the database

            // Start animation while cataloging
            QApplication::setOverrideCursor(Qt::WaitCursor);

            //clear database
                QSqlQuery deleteQuery;
                deleteQuery.exec("DELETE FROM file");

            //prepare query to load file info
                QSqlQuery insertQuery;
                QString insertSQL = QLatin1String(R"(
                                    INSERT INTO file (
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

            //Prepare insert query for folder
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


            //prepare file and stream

                if(!sourceFile.open(QIODevice::ReadOnly)) {
                    QMessageBox::information(this,"Katalog",tr("No catalog found."));
                    return;
                }

                QTextStream textStream(&sourceFile);
                QString     line;

            //Process and check Headers line
                line = textStream.readLine();

                //Check this is the right source format
                if (line.left(6)!="Volume"){
                    QApplication::restoreOverrideCursor();
                    QMessageBox::warning(this,"Kotation",tr("A file was found, but could not be loaded") +".\n");
                    return;
                }

            //load all files to the database


                    while (true)
                    {
                        //Read the new line
                        line = textStream.readLine();

                        if (line !=""){
                            QStringList fieldList = line.split("\t");
                            if ( fieldList.count()==7 ){

                                //Append file data to the database  ( removing " characters)
                                insertQuery.bindValue(":file_name", fieldList[2].remove("\""));
                                insertQuery.bindValue(":file_folder_path", fieldList[1].remove("\""));
                                insertQuery.bindValue(":file_size", fieldList[3].toLongLong());
                                insertQuery.bindValue(":file_date_updated", fieldList[5]);
                                insertQuery.bindValue(":file_catalog", fieldList[0].remove("\"").replace("/","_") + dateTimeForCatalogName);
                                insertQuery.exec();

                                //Append folder data to the database
                                insertFolderQuery.bindValue(":folder_catalog_name", fieldList[0].remove("\"").replace("/","_") + dateTimeForCatalogName);
                                insertFolderQuery.bindValue(":folder_path",         fieldList[1].remove("\""));
                                insertFolderQuery.exec();
                            }
                        }
                       else
                            break;
                    }

                    //complete table for missing folders
                    createMissingParentDirectories();

            //close source file
            sourceFile.close();

        //Stream the list of files and folders out to the target catalog file(s)
            //Define a root folder, compensating for the fact that VVV export does not contain one

            QString virtualCatalogFolder = "/import";

            //Get a list of the source catalogs
                QString listCatalogSQL = QLatin1String(R"(
                                    SELECT DISTINCT file_catalog
                                    FROM file
                                                )");
                QSqlQuery listCatalogQuery;
                listCatalogQuery.prepare(listCatalogSQL);
                listCatalogQuery.exec();

            //Create a virtual device to host the new catalogs
                Device importVirtualDevice;
                importVirtualDevice.generateDeviceID();
                importVirtualDevice.type ="Virtual";
                importVirtualDevice.name = "imports from VVV " + dateTimeForCatalogName;
                importVirtualDevice.parentID = 0;
                importVirtualDevice.groupID = 1;
                importVirtualDevice.path = virtualCatalogFolder;
                importVirtualDevice.dateTimeUpdated = QDateTime::currentDateTime();
                importVirtualDevice.insertDevice();
                collection->saveDeviceTableToFile();

            //Iterate each catalog to generate related files
                while (listCatalogQuery.next()){

                    //Create Device
                    Device importedDevice;
                    importedDevice.generateDeviceID();
                    importedDevice.name = listCatalogQuery.value(0).toString();
                    importedDevice.type ="Catalog";
                    importedDevice.parentID = importVirtualDevice.ID;
                    importedDevice.groupID = 1;
                    importedDevice.path = virtualCatalogFolder;
                    importedDevice.catalog->generateID();
                    importedDevice.externalID = importedDevice.catalog->ID;
                    importedDevice.dateTimeUpdated = QDateTime::currentDateTime();
                    importedDevice.insertDevice();

                    //Get info for the new catalog
                        importedDevice.catalog->name = importedDevice.name;
                        importedDevice.catalog->filePath = collection->collectionFolder + "/" + importedDevice.name + ".idx";
                        importedDevice.catalog->sourcePath = virtualCatalogFolder;
                        importedDevice.catalog->includeHidden = 1;
                        importedDevice.catalog->includeSymblinks = 0;
                        importedDevice.catalog->isFullDevice = 0;
                        importedDevice.catalog->includeMetadata = 0;
                        importedDevice.catalog->appVersion = currentVersion;
                        importedDevice.catalog->insertCatalog();

                //Update total number and size of the files
                    QString listCatalogSQL = QLatin1String(R"(
                                                    SELECT COUNT(*), SUM(file_size)
                                                    FROM file
                                                    WHERE file_catalog =:file_catalog
                                            )");
                    QSqlQuery listCatalogQuery;
                    listCatalogQuery.prepare(listCatalogSQL);
                    listCatalogQuery.bindValue(":file_catalog", importedDevice.name);
                    listCatalogQuery.exec();
                    listCatalogQuery.next();

                    importedDevice.totalFileCount = listCatalogQuery.value(0).toLongLong();
                    importedDevice.totalFileSize  = listCatalogQuery.value(1).toLongLong();
                    importedDevice.saveDevice();
                    collection->saveDeviceTableToFile();

                    //Save device
                    collection->saveDeviceTableToFile();


                //Export the catalog file

                    //Prepare the catalog file path
                        QFile fileOut(importedDevice.catalog->filePath);

                    //Prepare the stream and file headers
                        QTextStream out(&fileOut);
                        if(fileOut.open(QIODevice::WriteOnly | QIODevice::Truncate)) {

                            out  << "<catalogSourcePath>" + virtualCatalogFolder << "\n"
                                << "<catalogFileCount>" + QString::number(importedDevice.totalFileCount)    << "\n"
                                << "<catalogTotalFileSize>" + QString::number(importedDevice.totalFileSize) << "\n"
                                 << "<catalogIncludeHidden>"       << "\n"
                                 << "<catalogFileType>"            << "\n"
                                 << "<catalogStorage>"             << "\n"
                                 << "<catalogIncludeSymblinks>"    << "\n"
                                 << "<catalogIsFullDevice>"        << "\n"
                                 << "<catalogIncludeMetadata>"     << "\n"
                                 << "<catalogAppVersion>" + currentVersion << "\n"
                                << "<catalogID>" + QString::number(importedDevice.externalID) << "\n";
                        }

                    //Get the list of file to add
                    QString listFilesSQL = QLatin1String(R"(
                                                SELECT
                                                    file_folder_path,
                                                    file_name,
                                                    file_size,
                                                    file_date_updated
                                                FROM file
                                                WHERE file_catalog =:file_catalog
                                            )");
                    QSqlQuery listFilesQuery;
                    listFilesQuery.prepare(listFilesSQL);
                    listFilesQuery.bindValue(":file_catalog", importedDevice.name);
                    listFilesQuery.exec();

                    //Write the results in the file
                    while (listFilesQuery.next()) {
                        out << virtualCatalogFolder + listFilesQuery.value(0).toString() + "/" + listFilesQuery.value(1).toString();
                        out << '\t';
                        out << listFilesQuery.value(2).toString();
                        out << '\t';
                        out << listFilesQuery.value(3).toString();
                        out << '\n';
                    }

                    fileOut.close();

                    //Refresh catalogs
                        collection->loadCatalogFilesToTable();


                //Export the folder file

                    //Prepare the fodlers file path
                    QFile fileFolderOut(collection->collectionFolder + "/" + importedDevice.name + ".folders.idx");

                    //Prepare the stream and file headers
                    QTextStream folderOut(&fileFolderOut);
                    if(fileFolderOut.open(QIODevice::WriteOnly | QIODevice::Truncate)) {

                        //Get the list of file to add
                        QString listFoldersSQL = QLatin1String(R"(
                                            SELECT
                                                folder_catalog_name,
                                                folder_path
                                            FROM folder
                                            WHERE folder_catalog_name =:folder_catalog_name
                                        )");
                        QSqlQuery listFoldersQuery;
                        listFoldersQuery.prepare(listFoldersSQL);
                        listFoldersQuery.bindValue(":folder_catalog_name", importedDevice.name);
                        listFoldersQuery.exec();

                        //Write the results in the file
                        while (listFoldersQuery.next()) {
                            folderOut << listFoldersQuery.value(0).toString();
                            folderOut << '\t';
                            folderOut << virtualCatalogFolder + listFoldersQuery.value(1).toString();
                            folderOut << '\n';
                        }
                    }

                    fileFolderOut.close();
            }

            //update virtual device
            importVirtualDevice.updateNumbersFromChildren();
            collection->saveDeviceTableToFile();

            //Stop animation
            QApplication::restoreOverrideCursor();

        loadCollection();
    }
    //--------------------------------------------------------------------------
    void MainWindow::saveCatalogChanges()
    {
        Device previousCatalog;
        previousCatalog.ID = activeDevice->ID;
        previousCatalog.loadDevice();

        //Get new values
            //Other values
            activeDevice->catalog->fileType         = ui->Catalogs_comboBox_FileType->itemData(ui->Catalogs_comboBox_FileType->currentIndex(),Qt::UserRole).toString();
            activeDevice->catalog->includeHidden    = ui->Catalogs_checkBox_IncludeHidden->isChecked();
            activeDevice->catalog->includeMetadata  = ui->Catalogs_checkBox_IncludeMetadata->isChecked();
            activeDevice->catalog->isFullDevice     = ui->Catalogs_checkBox_isFullDevice->checkState();
            //DEV:QString newIncludeSymblinks  = ui->Catalogs_checkBox_IncludeSymblinks->currentText();

        //Confirm save changes
            QString message = tr("Save changes to the definition of the catalog?<br/>");
            message = message + "<table> <tr><td width=155><i>" + tr("field") + "</i></td><td width=125><i>" + tr("previous value") + "</i></td><td width=200><i>" + tr("new value") + "</i></td>";

           if(activeDevice->catalog->fileType       !=previousCatalog.catalog->fileType)
                message = message + "<tr><td>" + tr("File Type")    + "</td><td>" + previousCatalog.catalog->fileType     + "</td><td><b>" + activeDevice->catalog->fileType      + "</b></td></tr>";
            if(activeDevice->catalog->includeHidden  != previousCatalog.catalog->includeHidden)
                message = message + "<tr><td>" + tr("Include Hidden")   + "</td><td>" + QVariant(previousCatalog.catalog->includeHidden).toString()   + "</td><td><b>" + QVariant(activeDevice->catalog->includeHidden).toString()   + "</b></td></tr>";
            if(activeDevice->catalog->includeMetadata  != previousCatalog.catalog->includeMetadata)
                message = message + "<tr><td>" + tr("Include Metadata") + "</td><td>" + QVariant(previousCatalog.catalog->includeMetadata).toString() + "</td><td><b>" + QVariant(activeDevice->catalog->includeMetadata).toString() + "</b></td></tr>";
            if(activeDevice->catalog->isFullDevice  != previousCatalog.catalog->isFullDevice)
                message = message + "<tr><td>" + tr("Is Full Device") + "</td><td>" + QVariant(previousCatalog.catalog->isFullDevice).toString() + "</td><td><b>" + QVariant(activeDevice->catalog->isFullDevice).toString() + "</b></td></tr>";

            message = message + "</table>";

            if(    (activeDevice->catalog->sourcePath       !=previousCatalog.catalog->sourcePath)
                or (activeDevice->catalog->fileType         !=previousCatalog.catalog->fileType)
                or (activeDevice->catalog->includeHidden    !=previousCatalog.catalog->includeHidden)
                or (activeDevice->catalog->includeMetadata  !=previousCatalog.catalog->includeMetadata))
            {
                    message = message + + "<br/><br/>" + tr("(The catalog must be updated to reflect these changes)");
            }

            int result = QMessageBox::warning(this, "Katalog", message, QMessageBox::Yes | QMessageBox::Cancel);
            if ( result == QMessageBox::Cancel){
                return;
            }

        //Write all changes to database (except change of name)
            activeDevice->catalog->saveCatalog();

        //Write changes to catalog file (update headers only)
            activeDevice->catalog->updateCatalogFileHeaders(collection->databaseMode);

        //Refresh display
        loadCatalogsTableToModel();

        //Update the list of files if the changes impact the contents (i.e. path, file type, hidden)
            if (       activeDevice->catalog->sourcePath      != previousCatalog.catalog->sourcePath
                    or activeDevice->catalog->includeHidden   != previousCatalog.catalog->includeHidden
                    or activeDevice->catalog->includeMetadata != previousCatalog.catalog->includeMetadata
                    or activeDevice->catalog->fileType        != previousCatalog.catalog->fileType)
            {
                int updatechoice = QMessageBox::warning(this, "Katalog",
                                    tr("Update the catalog content with the new criteria?\n")
                                         , QMessageBox::Yes
                                                  | QMessageBox::No);
                if ( updatechoice == QMessageBox::Yes){
                    activeDevice->catalog->loadCatalog();
                    reportAllUpdates(activeDevice,
                                     activeDevice->updateDevice("update",
                                                                collection->databaseMode,
                                                                true,
                                                                collection->collectionFolder,
                                                                true),
                                     "update");
                }
            }

        //Refresh
            if(collection->databaseMode=="Memory")
                collection->loadCatalogFilesToTable();

            loadCatalogsTableToModel();
            loadCatalogsTableToModel();

        //Hide edition section
            ui->Catalogs_widget_EditCatalog->hide();

    }
    //--------------------------------------------------------------------------
    bool MainWindow::reportAllUpdates(Device *device, QList<qint64> list, QString updateType)
    {//Provide a report for any combinaison of updates (updateType = create, single, or list) and devices

        if(list[0]==0)
            return false;
        else{


        QMessageBox msgBox;
        QString message;

        //Catalog updates
        if (device->type=="Catalog" and updateType=="update"){

            if(list[0]==1){//Catalog updated
                message = QString(tr("<br/>Catalog updated:&nbsp;<b>%1</b><br/>")).arg(device->name);
                message += QString(tr("path:&nbsp;<b>%1</b><br/>")).arg(device->path);
                message += QString("<table>"
                                   "<tr><td>Number of files: </td><td align='center'><b> %1 </b></td><td>&nbsp; &nbsp; (added: </td><td align='right'><b> %2 </b>)&nbsp; &nbsp; </td></tr>"
                                   "<tr><td>Total file size: </td><td align='right'> <b> %3 </b></td><td>&nbsp; &nbsp; (added: </td><td align='right'><b> %4 </b>)&nbsp; &nbsp; </td></tr>"
                                   ).arg(QString::number(list[1]),
                                    QString::number(list[2]),
                                    QLocale().formattedDataSize(list[3]),
                                    QLocale().formattedDataSize(list[4]));
            }

            if(list[7]==1){//Parent storage updated
                Device parentDevice;
                parentDevice.ID = device->parentID;
                parentDevice.loadDevice();

                message += (tr("<br/>"
                               "<tr><td colspan=4>Storage updated:&nbsp; <b>%1</b></td></tr>"
                               "<tr><td colspan=4>path:&nbsp; <b> %8 </b> <br/></td></tr>"
                               "<tr><td> Used Space: </td><td align='right'><b> %2 </b></td><td>&nbsp; &nbsp; (added: </td><td align='right'><b> %3 </b>)</td></tr>"
                               "<tr><td> Free Space: </td><td align='right'><b> %4 </b></td><td>&nbsp; &nbsp; (added: </td><td align='right'><b> %5 </b>)</td></tr>"
                               "<tr><td>Total Space: </td><td align='right'><b> %6 </b></td><td>&nbsp; &nbsp; (added: </td><td align='right'><b> %7 </b>)</td></tr>"
                               "</table>"
                               ).arg(parentDevice.name,
                                     QLocale().formattedDataSize(list[8]),
                                     QLocale().formattedDataSize(list[9]),
                                     QLocale().formattedDataSize(list[10]),
                                     QLocale().formattedDataSize(list[11]),
                                     QLocale().formattedDataSize(list[12]),
                                     QLocale().formattedDataSize(list[13]),
                                     parentDevice.path
                                     ));
            }

            if(list[0]==1 or list[7]==1){
                msgBox.setWindowTitle("Katalog");
                msgBox.setText(message);
                msgBox.setIcon(QMessageBox::Information);
                msgBox.exec();
            }
        }
        if (device->type=="Catalog" and updateType=="create"){
            if(list[0]==1){//Catalog updated
                    message = QString(tr("<br/>Catalog created:&nbsp;<b>%1</b><br/>")).arg(device->name);
                    message += QString(tr("path:&nbsp;<b>%1</b><br/>")).arg(device->path);

                message += QString("<table>"
                                   "<tr><td>Number of files: </td><td align='center'><b> %1 </b></td><td></tr>"
                                   "<tr><td>Total file size: </td><td align='right'> <b> %2 </b></td><td></tr>"
                                   ).arg(QString::number(list[1]),
                                    QLocale().formattedDataSize(list[3])
                                    );
            }

            if(list[7]==1){//Parent storage updated
                Device parentDevice;
                parentDevice.ID = device->parentID;
                parentDevice.loadDevice();

                message += (tr("<br/>"
                               "<tr><td colspan=4>Storage updated:&nbsp; <b>%1</b></td></tr>"
                               "<tr><td colspan=4>path:&nbsp; <b> %8 </b> <br/></td></tr>"
                               "<tr><td> Used Space: </td><td align='right'><b> %2 </b></td><td>&nbsp; &nbsp; (added: </td><td align='right'><b> %3 </b>)</td></tr>"
                               "<tr><td> Free Space: </td><td align='right'><b> %4 </b></td><td>&nbsp; &nbsp; (added: </td><td align='right'><b> %5 </b>)</td></tr>"
                               "<tr><td>Total Space: </td><td align='right'><b> %6 </b></td><td>&nbsp; &nbsp; (added: </td><td align='right'><b> %7 </b>)</td></tr>"
                               "</table>"
                               ).arg(parentDevice.name,
                                     QLocale().formattedDataSize(list[8]),
                                     QLocale().formattedDataSize(list[9]),
                                     QLocale().formattedDataSize(list[10]),
                                     QLocale().formattedDataSize(list[11]),
                                     QLocale().formattedDataSize(list[12]),
                                     QLocale().formattedDataSize(list[13]),
                                     parentDevice.path
                                     ));
            }

            if(list[0]==1 or list[7]==1){
                msgBox.setWindowTitle("Katalog");
                msgBox.setText(message);
                msgBox.setIcon(QMessageBox::Information);
                msgBox.exec();
            }
        }
        //Storage updates
        if (device->type=="Storage" and updateType=="update"){
            message.clear();
            message += (tr("<table>"
                           "<tr><td colspan=4>Storage updated:&nbsp; <b>%1</b></td></tr>"
                           "<tr><td colspan=4>path:&nbsp; <b> %8 </b> <br/></td></tr>"
                           "<tr><td> Used Space: </td><td align='right'><b> %2 </b></td><td>&nbsp; &nbsp; (added: </td><td align='right'><b> %3 </b>)</td></tr>"
                           "<tr><td> Free Space: </td><td align='right'><b> %4 </b></td><td>&nbsp; &nbsp; (added: </td><td align='right'><b> %5 </b>)</td></tr>"
                           "<tr><td>Total Space: </td><td align='right'><b> %6 </b></td><td>&nbsp; &nbsp; (added: </td><td align='right'><b> %7 </b>)</td></tr>"
                           "</table>"
                               ).arg(activeDevice->name,
                                 QLocale().formattedDataSize(list[8]),
                                 QLocale().formattedDataSize(list[9]),
                                 QLocale().formattedDataSize(list[10]),
                                 QLocale().formattedDataSize(list[11]),
                                 QLocale().formattedDataSize(list[12]),
                                 QLocale().formattedDataSize(list[13]),
                                 activeDevice->path
                                 ));
            msgBox.setWindowTitle("Katalog");
            msgBox.setText(message);
            msgBox.setIcon(QMessageBox::Information);
            msgBox.exec();
        }
        if (device->type !="Catalog" and updateType=="list"){
            if(list[0]==1){//Catalog updated
                message = QString(tr("<table>"
                                     "<br/>Selected active catalogs from <b>%1</b> are updated.&nbsp;<br/>")).arg(device->name);
                message += QString(
                                   "<tr><td>Number of files: </td><td align='center'><b> %1 </b></td><td>&nbsp; &nbsp; (added: </td><td align='right'><b> %2 </b>)&nbsp; &nbsp; </td></tr>"
                                   "<tr><td>Total file size: </td><td align='right'> <b> %3 </b></td><td>&nbsp; &nbsp; (added: </td><td align='right'><b> %4 </b>)&nbsp; &nbsp; </td></tr>"
                                   ).arg(QString::number(list[1]),
                                    QString::number(list[2]),
                                    QLocale().formattedDataSize(list[3]),
                                    QLocale().formattedDataSize(list[4]));

                message += "</table>" + QString(tr("<br/><br/> %1 updated Catalogs (active), %2 skipped Catalogs (inactive)")).arg(QString::number(list[5]),QString::number(list[6]));
            }

            if(list[7]==1){//Storage updated
                message += (tr("<tr><td colspan=4></td></tr>"
                               "<tr><td colspan=4></td></tr>"
                               "<tr><td colspan=4>Storage updated:&nbsp; <b>%1</b></td></tr>"
                               "<tr><td colspan=4>path:&nbsp; <b> %8 </b> <br/></td></tr>"
                               "<tr><td> Used Space: </td><td align='right'><b> %2 </b></td><td>&nbsp; &nbsp; (added: </td><td align='right'><b> %3 </b>)</td></tr>"
                               "<tr><td> Free Space: </td><td align='right'><b> %4 </b></td><td>&nbsp; &nbsp; (added: </td><td align='right'><b> %5 </b>)</td></tr>"
                               "<tr><td>Total Space: </td><td align='right'><b> %6 </b></td><td>&nbsp; &nbsp; (added: </td><td align='right'><b> %7 </b>)</td></tr>"
                               "</table>"
                               ).arg(device->name,
                                     QLocale().formattedDataSize(list[8]),
                                     QLocale().formattedDataSize(list[9]),
                                     QLocale().formattedDataSize(list[10]),
                                     QLocale().formattedDataSize(list[11]),
                                     QLocale().formattedDataSize(list[12]),
                                     QLocale().formattedDataSize(list[13]),
                                     device->path
                                     ));
            }

            msgBox.setWindowTitle("Katalog");
            msgBox.setText(message);
            msgBox.setIcon(QMessageBox::Information);
            msgBox.exec();
        }
        //Virtual updates
        if (device->type =="Virtual"){
            /*
                message = QString(tr("<table>"
                                     "<br/>Selected active catalogs from <b>%1</b> are updated.&nbsp;<br/>")).arg(device->name);
                message += QString(
                               "<tr><td>Number of files: </td><td align='center'><b> %1 </b></td><td>&nbsp; &nbsp; (added: </td><td align='right'><b> %2 </b>)&nbsp; &nbsp; </td></tr>"
                               "<tr><td>Total file size: </td><td align='right'> <b> %3 </b></td><td>&nbsp; &nbsp; (added: </td><td align='right'><b> %4 </b>)&nbsp; &nbsp; </td></tr>"
                               ).arg(QString::number(list[0]));

            msgBox.setWindowTitle("Katalog");
            msgBox.setText(message);
            msgBox.setIcon(QMessageBox::Information);
            msgBox.exec();
            */
        }


            return true;

        }
    }
    //--------------------------------------------------------------------------
    void MainWindow::createMissingParentDirectories() {
        QSqlQuery query;

        // Select distinct folder paths
        query.exec("SELECT DISTINCT folder_catalog_name, folder_path FROM folder");

        // Iterate through the result set
        while (query.next()) {
            QString folderCatalogName = query.value(0).toString();
            QString folderPath = query.value(1).toString();

            // Split the folder path into components
            QStringList folders = folderPath.split('/', Qt::SkipEmptyParts);
            QString currentPath;

            // Iterate through the components and insert missing parent directories
            for (const QString& folder : folders) {
                currentPath += '/' + folder;

                // Check if the current path exists in the table
                QSqlQuery checkQuery;
                checkQuery.prepare("SELECT 1 FROM folder WHERE folder_catalog_name = :catalog AND folder_path = :path");
                checkQuery.bindValue(":catalog", folderCatalogName);
                checkQuery.bindValue(":path", currentPath);

                if (!checkQuery.exec()) {
                    qDebug() << "Error checking path:" << checkQuery.lastError().text();
                }

                // If the current path doesn't exist, insert it
                if (!checkQuery.next()) {
                    QSqlQuery insertQuery;
                    insertQuery.prepare("INSERT INTO folder (folder_catalog_name, folder_path) VALUES (:catalog, :path)");
                    insertQuery.bindValue(":catalog", folderCatalogName);
                    insertQuery.bindValue(":path", currentPath);

                    if (!insertQuery.exec()) {
                        qDebug() << "Error inserting path:" << insertQuery.lastError().text();
                    }
                }
            }
        }
    }
