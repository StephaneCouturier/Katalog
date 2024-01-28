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
            qDebug()<<"filePath: "<<activeDevice->catalog->filePath;

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
            ui->Catalogs_lineEdit_SourcePath->setText(activeDevice->catalog->sourcePath);
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
            ui->Catalogs_lineEdit_SourcePath->setText(selectedDevice->catalog->sourcePath);
            ui->Catalogs_comboBox_FileType->setCurrentText(selectedDevice->catalog->fileType);
            ui->Catalogs_label_StorageDisplay->setText(selectedDevice->catalog->storageName);
            ui->Catalogs_checkBox_IncludeHidden->setChecked(selectedDevice->catalog->includeHidden);

            ui->Catalogs_widget_EditCatalog->hide();
        }
        //----------------------------------------------------------------------
        void MainWindow::on_Catalogs_pushButton_UpdateCatalog_clicked()
        {
            reportAllUpdates(activeDevice, activeDevice->updateDevice("update",collection->databaseMode,false,collection->collectionFolder), "update");
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

                        QList<qint64> list = loopDevice.updateDevice("update", collection->databaseMode, false, collection->collectionFolder);
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

                activeDevice->deleteDevice();

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
        void MainWindow::on_Catalogs_pushButton_SelectPath_clicked()
        {
            //Get current selected path as default path for the dialog window
            QString newCatalogPath = ui->Catalogs_lineEdit_SourcePath->text();

            //Open a dialog for the user to select the directory to be cataloged. Only show directories.
            QString dir = QFileDialog::getExistingDirectory(this, tr("Select the directory to be cataloged in this new catalog"),
                                                            newCatalogPath,
                                                            QFileDialog::ShowDirsOnly
                                                            | QFileDialog::DontResolveSymlinks);
            //Send the selected directory to LE_NewCatalogPath (input line for the New Catalog Path)
            ui->Catalogs_lineEdit_SourcePath->setText(dir);

            //Select this directory in the treeview.
            loadFileSystem(newCatalogPath);
        }
        //----------------------------------------------------------------------
        void MainWindow::on_Catalogs_pushButton_Save_clicked()
        {
            saveCatalogChanges(activeDevice->catalog);
        }
        //----------------------------------------------------------------------
        void MainWindow::on_Catalogs_pushButton_Snapshot_clicked()
        {
            recordCollectionStats();
        }

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
                                            (SELECT e.device_name FROM device e WHERE e.device_id = d.device_parent_id) ,
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
        QString querySumCatalogValuesSQL = QLatin1String(R"(
                                SELECT  COUNT(agg.catalog_name),
                                        SUM(agg.catalog_total_file_size),
                                        SUM(agg.catalog_file_count)
                                FROM (
                                    SELECT  c.catalog_name,
                                            c.catalog_total_file_size,
                                            c.catalog_file_count
                                    FROM catalog c
                                    LEFT JOIN device d ON d.device_name = c.catalog_name

                            )"); //LEFT JOIN storage s ON c.catalog_storage = s.storage_name

        if (      selectedDevice->type == "Storage" ){
            QString prepareSQL2 = QLatin1String(R"(
                                    WHERE c.catalog_storage =:catalog_storage
                                    GROUP BY c.catalog_name, c.catalog_total_file_size, c.catalog_file_count
                                ) agg
                                                    )");
            querySumCatalogValuesSQL += prepareSQL2;
        }
        else if ( selectedDevice->type == "Catalog" ){
            QString prepareSQL2 = QLatin1String(R"(
                                    WHERE catalog_name =:catalog_name
                                    GROUP BY c.catalog_name, c.catalog_total_file_size, c.catalog_file_count
                                ) agg
                                                    )");
            querySumCatalogValuesSQL += prepareSQL2;
        }
        else if ( selectedDevice->type == "Virtual" ){
            QString prepareSQL2 = QLatin1String(R"(

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
                                        GROUP BY c.catalog_name, c.catalog_total_file_size, c.catalog_file_count
                                ) agg
                                                    )");
            querySumCatalogValuesSQL += prepareSQL2;
        }
        else{
            QString prepareSQL2 = QLatin1String(R"(
                                    GROUP BY c.catalog_name, c.catalog_total_file_size, c.catalog_file_count
                                ) agg
                                                    )");
            querySumCatalogValuesSQL += prepareSQL2;
        }

        querySumCatalogValues.prepare(querySumCatalogValuesSQL);
        querySumCatalogValues.bindValue(":catalog_storage", selectedDevice->name);
        querySumCatalogValues.bindValue(":catalog_name", selectedDevice->name);
        querySumCatalogValues.bindValue(":device_id", selectedDevice->ID);
        querySumCatalogValues.exec();
        querySumCatalogValues.next();

        ui->Catalogs_label_Catalogs->setText(QString::number(querySumCatalogValues.value(0).toInt()));
        ui->Catalogs_label_TotalSize->setText(QLocale().formattedDataSize(querySumCatalogValues.value(1).toLongLong()));
        ui->Catalogs_label_TotalNumber->setText(QLocale().toString(querySumCatalogValues.value(2).toInt()));
    }
    //--------------------------------------------------------------------------
    void MainWindow::backupCatalogFile(QString catalogSourcePath)
    {
        QString catalogBackUpSourcePath = catalogSourcePath + ".bak";

        //Verify if a bak up file already exist and remove it.
        if (QFile::exists(catalogBackUpSourcePath))
        {
            QFile::remove(catalogBackUpSourcePath);
        }

        //Copy the file to the same location, adding .bak for the new file name.
        QFile::copy(catalogSourcePath, catalogBackUpSourcePath);
    }
    //--------------------------------------------------------------------------
    void MainWindow::importFromVVV()
    {
        //Select file

            //Get user home path
            QStringList standardsPaths = QStandardPaths::standardLocations(QStandardPaths::HomeLocation);
            QString homePath = standardsPaths[0];

            //Get path of the file to import
            QString sourceFilePath = QFileDialog::getOpenFileName(this, tr("Select the csv file to be imported"),homePath);

            //Stop if no path is selected
            if ( sourceFilePath=="" ) return;

            //Define file
            QFile sourceFile(sourceFilePath);

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

            //prepare file and stream

                if(!sourceFile.open(QIODevice::ReadOnly)) {
                    QMessageBox::information(this,"Katalog",tr("No catalog found."));
                    return;
                }

                QTextStream textStream(&sourceFile);
                QString     line;

            //Process and check Headers line
                line = textStream.readLine();

                //Split the line into a fieldlist
                //QStringList headerList = line.split('\t');

                //Check this is the right source format
                if (line.left(6)!="Volume"){
                    QMessageBox::warning(this,"Kotation",tr("A file was found, but could not be loaded") +".\n");
                    return;
                }
                //else {
                    //int headerFieldNumber = headerList.length();
                    //QMessageBox::information(this,"Kotation","ok to import. \n number of fields: \n " + QString::number(headerFieldNumber));
                //}

            //load all files to the database

                    while (true)
                    {
                        //Read the newt line
                        line = textStream.readLine();

                        if (line !=""){
                            QStringList fieldList = line.split("\t");
                            if ( fieldList.count()==7 ){

                                //Append data to the database
                                insertQuery.bindValue(":file_name", fieldList[2].replace("\"",""));
                                insertQuery.bindValue(":file_folder_path", fieldList[1].replace("\"",""));
                                insertQuery.bindValue(":file_size", fieldList[3].toLongLong());
                                insertQuery.bindValue(":file_date_updated", fieldList[5]);
                                insertQuery.bindValue(":file_catalog", fieldList[0].replace("\"",""));
                                insertQuery.exec();
                            }
                        }
                       else
                            break;


                }

            //close source file
            sourceFile.close();

        //Stream the list of files out to the target calog file(s)

            //Get a list of the source catalogs
                QString listCatalogSQL = QLatin1String(R"(
                                    SELECT DISTINCT file_catalog
                                    FROM file
                                                )");
                QSqlQuery listCatalogQuery;
                listCatalogQuery.prepare(listCatalogSQL);
                listCatalogQuery.exec();

            //Iterate in the list to generate a catalog file for each catalog
            while (listCatalogQuery.next()){

                //Get catalog name
                    QString formerCatalogName = listCatalogQuery.value(0).toString();

                //Generate a name for the file itself, without specical characters
                    QString newCatalogName;
                    newCatalogName = formerCatalogName;
                    newCatalogName.replace("/","_");
                    newCatalogName.replace("\\","_");

                //Prepare the catalog file path
                    QFile fileOut( collection->collectionFolder +"/"+ newCatalogName + ".idx" );

                //Get statistics of the files for the list
                    QString listCatalogSQL = QLatin1String(R"(
                                        SELECT COUNT(*), SUM(fileSize)
                                        FROM file
                                        WHERE file_catalog =:file_catalog
                                                    )");
                    QSqlQuery listCatalogQuery;
                    listCatalogQuery.prepare(listCatalogSQL);
                    listCatalogQuery.bindValue(":file_catalog",formerCatalogName);
                    listCatalogQuery.exec();
                    listCatalogQuery.next();

                    qint64 totalFiles = listCatalogQuery.value(0).toLongLong();
                    qint64 totalSize  = listCatalogQuery.value(1).toLongLong();

                //Prepare the stream and file headers
                    QTextStream out(&fileOut);
                    if(fileOut.open(QIODevice::WriteOnly | QIODevice::Truncate)) {

                        //append catalog definition("<catalogName>"+newCatalogName);
                        out  << "<catalogSourcePath>imported"  << "\n"
                             << "<catalogFileCount>"+QString::number(totalFiles)           << "\n"
                             << "<catalogTotalFileSize>"+QString::number(totalSize)       << "\n"
                             << "<catalogIncludeHidden>"       << "\n"
                             << "<catalogFileType>"            << "\n"
                             << "<catalogStorage>"             << "\n"
                             << "<catalogIncludeSymblinks>"    << "\n"
                             << "<catalogIsFullDevice>"     << "\n";
                    }

                //Get the list of file to add
                QString listFilesSQL = QLatin1String(R"(
                                    SELECT *
                                    FROM file
                                    WHERE file_catalog =:file_catalog
                                                )");
                QSqlQuery listFilesQuery;
                listFilesQuery.prepare(listFilesSQL);
                listFilesQuery.bindValue(":file_catalog",formerCatalogName);
                listFilesQuery.exec();

                //Write the results in the file
                while (listFilesQuery.next()) {

                        out << listFilesQuery.value(1).toString() + "/" + listFilesQuery.value(0).toString();
                        out << '\t';
                        out << listFilesQuery.value(2).toString();
                        out << '\t';
                        out << listFilesQuery.value(3).toString();
                        out << '\n';
                }
            }

            //Stop animation
            QApplication::restoreOverrideCursor();

        loadCollection();

    }
    //--------------------------------------------------------------------------
    void MainWindow::saveCatalogChanges(Catalog *previousCatalog)
    {
        Catalog newCatalog;// = previousCatalog;
        newCatalog.ID = previousCatalog->ID;
        newCatalog.loadCatalog();
        newCatalog.filePath = previousCatalog->filePath;
        newCatalog.sourcePath = previousCatalog->sourcePath;

        //Get new values
            //Get new catalog sourcePath: remove the / at the end if any, except for / alone (root directory in linux)
                newCatalog.sourcePath = ui->Catalogs_lineEdit_SourcePath->text();
                int     pathLength              = newCatalog.sourcePath.length();
                if (newCatalog.sourcePath !="" and newCatalog.sourcePath !="/" and QVariant(newCatalog.sourcePath.at(pathLength-1)).toString()=="/") {
                    newCatalog.sourcePath.remove(pathLength-1,1);
                }

            //Other values
            newCatalog.fileType         = ui->Catalogs_comboBox_FileType->itemData(ui->Catalogs_comboBox_FileType->currentIndex(),Qt::UserRole).toString();
            newCatalog.includeHidden    = ui->Catalogs_checkBox_IncludeHidden->isChecked();
            newCatalog.includeMetadata  = ui->Catalogs_checkBox_IncludeMetadata->isChecked();
            newCatalog.isFullDevice     = ui->Catalogs_checkBox_isFullDevice->checkState();
            //DEV:QString newIncludeSymblinks  = ui->Catalogs_checkBox_IncludeSymblinks->currentText();

        //Confirm save changes
            QString message = tr("Save changes to the definition of the catalog?<br/>");
            message = message + "<table> <tr><td width=155><i>" + tr("field") + "</i></td><td width=125><i>" + tr("previous value") + "</i></td><td width=200><i>" + tr("new value") + "</i></td>";

            if(newCatalog.sourcePath     != previousCatalog->sourcePath)
                message = message + "<tr><td>" + tr("Source path ") + "</td><td>" + previousCatalog->sourcePath   + "</td><td><b>" + newCatalog.sourcePath    + "</b></td></tr>";
            if(newCatalog.fileType       !=previousCatalog->fileType)
                message = message + "<tr><td>" + tr("File Type")    + "</td><td>" + previousCatalog->fileType     + "</td><td><b>" + newCatalog.fileType      + "</b></td></tr>";
            if(newCatalog.includeHidden  != previousCatalog->includeHidden)
                message = message + "<tr><td>" + tr("Include Hidden")   + "</td><td>" + QVariant(previousCatalog->includeHidden).toString()   + "</td><td><b>" + QVariant(newCatalog.includeHidden).toString()   + "</b></td></tr>";
            if(newCatalog.includeMetadata  != previousCatalog->includeMetadata)
                message = message + "<tr><td>" + tr("Include Metadata") + "</td><td>" + QVariant(previousCatalog->includeMetadata).toString() + "</td><td><b>" + QVariant(newCatalog.includeMetadata).toString() + "</b></td></tr>";
            if(newCatalog.isFullDevice  != previousCatalog->isFullDevice)
                message = message + "<tr><td>" + tr("Is Full Device") + "</td><td>" + QVariant(previousCatalog->isFullDevice).toString() + "</td><td><b>" + QVariant(newCatalog.isFullDevice).toString() + "</b></td></tr>";

            message = message + "</table>";

            if(    (newCatalog.sourcePath       !=previousCatalog->sourcePath)
                or (newCatalog.fileType         !=previousCatalog->fileType)
                or (newCatalog.includeHidden    !=previousCatalog->includeHidden)
                or (newCatalog.includeMetadata  !=previousCatalog->includeMetadata))
            {
                    message = message + + "<br/><br/>" + tr("(The catalog must be updated to reflect these changes)");
            }

            int result = QMessageBox::warning(this, "Katalog", message, QMessageBox::Yes | QMessageBox::Cancel);
            if ( result == QMessageBox::Cancel){
                return;
            }

        //Write all changes to database (except change of name)
            newCatalog.saveCatalog();

        //Write changes to catalog file (update headers only)
            newCatalog.updateCatalogFileHeaders(collection->databaseMode);

        //Refresh display
        loadCatalogsTableToModel();

        //Update the list of files if the changes impact the contents (i.e. path, file type, hidden)
            if (       newCatalog.sourcePath      != previousCatalog->sourcePath
                    or newCatalog.includeHidden   != previousCatalog->includeHidden
                    or newCatalog.includeMetadata != previousCatalog->includeMetadata
                    or newCatalog.fileType        != previousCatalog->fileType)
            {
                int updatechoice = QMessageBox::warning(this, "Katalog",
                                    tr("Update the catalog content with the new criteria?\n")
                                         , QMessageBox::Yes
                                                  | QMessageBox::No);
                if ( updatechoice == QMessageBox::Yes){
                    activeDevice->catalog->name = newCatalog.name;
                    activeDevice->catalog->loadCatalog();
                    reportAllUpdates(activeDevice, activeDevice->updateDevice("update",collection->databaseMode,true,collection->collectionFolder), "update");
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
    void MainWindow::recordCollectionStats()
    {
        //Get the current total values
        QSqlQuery queryLastCatalog;
        QString queryLastCatalogSQL = QLatin1String(R"(
                                    SELECT SUM(catalog_total_file_size), SUM(catalog_file_count)
                                    FROM statistics_catalog
                                    WHERE date_time = (SELECT MAX(date_time)
                                                        FROM statistics_catalog
                                                        WHERE record_type="snapshot")
                                    GROUP BY date_time
                                )");
        queryLastCatalog.prepare(queryLastCatalogSQL);
        queryLastCatalog.exec();
        queryLastCatalog.next();
        qint64 lastCatalogTotalFileSize   = queryLastCatalog.value(0).toLongLong();
        qint64 lastCatalogTotalFileNumber = queryLastCatalog.value(1).toLongLong();

        QSqlQuery queryLastStorage;
        QString queryLastStorageSQL = QLatin1String(R"(
                                    SELECT SUM(storage_free_space), SUM(storage_total_space)
                                    FROM statistics_storage
                                    WHERE date_time = (SELECT MAX(date_time)
                                                        FROM statistics_storage
                                                        WHERE record_type="snapshot")
                                    GROUP BY date_time
                                )");
        queryLastStorage.prepare(queryLastStorageSQL);
        queryLastStorage.exec();
        queryLastStorage.next();
        qint64 lastStorageFreeSpace  = queryLastStorage.value(0).toLongLong();
        qint64 lastStorageTotalSpace = queryLastStorage.value(1).toLongLong();

        //Record current catalogs and storage devices values
        QDateTime nowDateTime = QDateTime::currentDateTime();
        recordAllCatalogStats(nowDateTime);
        recordAllStorageStats(nowDateTime);
        recordAllDeviceStats(nowDateTime);

        //Get the new total values
        QSqlQuery queryNew;
        QString queryNewSQL = QLatin1String(R"(
                                SELECT SUM(catalog_total_file_size), SUM(catalog_file_count)
                                FROM statistics_catalog
                                WHERE date_time = (SELECT MAX(date_time)
                                                   FROM statistics_catalog
                                                   WHERE record_type="snapshot")
                                GROUP BY date_time
                            )");
        queryNew.prepare(queryNewSQL);
        queryNew.exec();
        queryNew.next();
        qint64 newTotalFileSize   = queryNew.value(0).toLongLong();
        qint64 newTotalFileCount  = queryNew.value(1).toLongLong();

        QSqlQuery queryNewStorage;
        QString queryNewStorageSQL = QLatin1String(R"(
                                    SELECT SUM(storage_free_space), SUM(storage_total_space)
                                    FROM statistics_storage
                                    WHERE date_time = (SELECT MAX(date_time)
                                                       FROM statistics_storage
                                                       WHERE record_type="snapshot")
                                    GROUP BY date_time
                                )");
        queryNewStorage.prepare(queryNewStorageSQL);
        queryNewStorage.exec();
        queryNewStorage.next();
        qint64 newStorageFreeSpace  = queryNewStorage.value(0).toLongLong();
        qint64 newStorageTotalSpace = queryNewStorage.value(1).toLongLong();

        //Calculate and inform
        qint64 deltaCatalogTotalFileSize   = newTotalFileSize  - lastCatalogTotalFileSize;
        qint64 deltaCatalogTotalFileNumber = newTotalFileCount - lastCatalogTotalFileNumber;
        qint64 deltaStorageFreeSpace       = newStorageFreeSpace  - lastStorageFreeSpace;
        qint64 deltaStorageTotalSpace      = newStorageTotalSpace - lastStorageTotalSpace;

        QMessageBox msgBox;
        msgBox.setWindowTitle("Katalog");
        msgBox.setText(tr(  "<br/>A snapshot of this collection was recorded:"
                            "<table>"
                            "<tr><td><br/><b>Catalogs</b></td><td></td><td></td></tr>"
                            "<tr><td>Number of files: </td><td style='text-align: right;'><b> %1 </b></td><td>  (added: <b> %2 </b>)</td></tr>"
                            "<tr><td>Total file size: </td><td style='text-align: right;'><b> %3 </b></td><td>  (added: <b> %4 </b>)</td></tr>"
                            "<tr><td><br/><b>Storage</b></td><td></td><td></td></tr>"
                            "<tr><td>Storage free space: </td><td style='text-align: right;'><b> %5 </b></td><td>  (added: <b> %6 </b>)</td></tr>"
                            "<tr><td>Storage total space: </td><td style='text-align: right;'><b> %7 </b></td><td>  (added: <b> %8 </b>)</td></tr>"
                            "</table>"
                          ).arg(QLocale().toString(newTotalFileCount),
                                QLocale().toString(deltaCatalogTotalFileNumber),
                                QLocale().formattedDataSize(newTotalFileSize),
                                QLocale().formattedDataSize(deltaCatalogTotalFileSize),
                                QLocale().formattedDataSize(newStorageFreeSpace),
                                QLocale().formattedDataSize(deltaStorageFreeSpace),
                                QLocale().formattedDataSize(newStorageTotalSpace),
                                QLocale().formattedDataSize(deltaStorageTotalSpace)
                                ));
        msgBox.setIcon(QMessageBox::Information);
        msgBox.exec();

    }
    //--------------------------------------------------------------------------
    void MainWindow::recordAllCatalogStats(QDateTime dateTime)
    {// Save the values (size and number of files) of all catalogs to the statistics file, creating a snapshop of the collection.

        //Get the list of catalogs and data
            QSqlQuery query;
            QString querySQL = QLatin1String(R"(
                                        SELECT
                                            catalog_name,
                                            catalog_file_count,
                                            catalog_total_file_size
                                        FROM catalog
                                        WHERE catalog_file_type !='EXPORT'
                                        )");
            query.prepare(querySQL);
            query.exec();

            QDateTime currentDateTime = QDateTime::currentDateTime();
           //Save history for each catalog
            Device loopDevice;
            while (query.next()){
                loopDevice.ID = query.value(0).toInt();
                loopDevice.loadDevice();
                loopDevice.saveStatistics(currentDateTime,"snapshot");

                if(collection->databaseMode=="Memory")
                {
                    loopDevice.catalog->saveStatisticsToFile(collection->statisticsCatalogFilePath, dateTime);
                }
            }

            //Refresh
            if(collection->databaseMode=="Memory"){
                collection->loadStatisticsCatalogFileToTable();
                collection->loadStatisticsStorageFileToTable();
            }

            loadStatisticsChart();

    }
    //--------------------------------------------------------------------------
    void MainWindow::reportAllUpdates(Device *device, QList<qint64> list, QString updateType)
    {//Provide a report for any combinaison of updates (updateType = create, single, or list) and devices

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

            //if(list[7]==1){
                msgBox.setWindowTitle("Katalog");
                msgBox.setText(message);
                msgBox.setIcon(QMessageBox::Information);
                msgBox.exec();
            //}

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
    }
    //--------------------------------------------------------------------------
