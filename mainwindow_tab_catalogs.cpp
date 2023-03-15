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

            QSettings settings(settingsFilePath, QSettings:: IniFormat);

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
            selectedCatalog->setName(ui->Catalogs_treeView_CatalogList->model()->index(index.row(), 0, QModelIndex()).data().toString());
            selectedCatalog->loadCatalogMetaData();

            // Display buttons
            ui->Catalogs_pushButton_Search->setEnabled(true);
            ui->Catalogs_pushButton_ExploreCatalog->setEnabled(true);
            ui->Catalogs_pushButton_Open->setEnabled(true);
            ui->Catalogs_pushButton_EditCatalogFile->setEnabled(true);
            ui->Catalogs_pushButton_UpdateCatalog->setEnabled(true);
            ui->Catalogs_pushButton_ViewCatalogStats->setEnabled(true);
            ui->Catalogs_pushButton_DeleteCatalog->setEnabled(true);

            //Load catalog values to the Edit area
            ui->Catalogs_lineEdit_Name->setText(selectedCatalog->name);
            ui->Catalogs_lineEdit_SourcePath->setText(selectedCatalog->sourcePath);
            ui->Catalogs_comboBox_FileType->setCurrentText(selectedCatalog->fileType);
            ui->Catalogs_comboBox_Storage->setCurrentText(selectedCatalog->storageName);
            ui->Catalogs_checkBox_IncludeHidden->setChecked(selectedCatalog->includeHidden);
            ui->Catalogs_checkBox_IncludeMetadata->setChecked(selectedCatalog->includeMetadata);
            //DEV: ui->Catalogs_checkBox_isFullDevice->setChecked(selectedCatalogIsFullDevice);

        }
        //----------------------------------------------------------------------
        void MainWindow::on_Catalogs_treeView_CatalogList_doubleClicked()
        {
            //Start at the root folder of the catalog
            selectedDirectoryName     = selectedCatalog->sourcePath;
            selectedDirectoryFullPath = selectedCatalog->sourcePath;
            selectedFilterCatalogName = selectedCatalog->name;


            //The selected catalog becomes the active catalog
            //activeCatalog = selectedCatalog;
            activeCatalog->setName(selectedCatalog->name);
            activeCatalog->loadCatalogMetaData();

            //Load
            openCatalogToExplore();

            //Go to explore tab
            ui->tabWidget->setCurrentIndex(2);

        }
        //----------------------------------------------------------------------
        void MainWindow::on_Catalogs_pushButton_Search_clicked()
        {
            //Change the selected catalog in Search tab
            ui->Filters_label_DisplayCatalog->setText(selectedCatalog->name);
            selectedFilterCatalogName = selectedCatalog->name;

            selectedFilterStorageLocation = tr("All");
            ui->Filters_label_DisplayLocation->setText(selectedFilterStorageLocation);

            selectedFilterStorageName = tr("All");
            ui->Filters_label_DisplayStorage->setText(selectedFilterStorageName);

            activeCatalog->setName(selectedCatalog->name);
            activeCatalog->loadCatalogMetaData();

            selectedDeviceName = selectedCatalog->name;
            selectedDeviceType = "Catalog";

            QSettings settings(settingsFilePath, QSettings:: IniFormat);
            settings.setValue("Selection/SelectedDeviceType", selectedDeviceType);
            settings.setValue("Selection/SelectedDeviceName", selectedDeviceName);

            filterFromSelectedDevices();

            //Go to the Search tab
            ui->tabWidget->setCurrentIndex(0); // tab 0 is the Search tab
        }
        //----------------------------------------------------------------------
        void MainWindow::on_Catalogs_pushButton_ExploreCatalog_clicked()
        {
                //Start at the root folder of the catalog
                selectedDirectoryName     = selectedCatalog->sourcePath;
                selectedDirectoryFullPath = selectedCatalog->sourcePath;
                selectedFilterCatalogName       = selectedCatalog->name;

                //The selected catalog becomes the active catalog
                activeCatalog->setName(selectedCatalog->name);
                activeCatalog->loadCatalogMetaData();

                //Load
                openCatalogToExplore();

                //Go to explore tab
                ui->tabWidget->setCurrentIndex(2);
        }
        //----------------------------------------------------------------------
        void MainWindow::on_Catalogs_pushButton_Cancel_clicked()
        {
            ui->Catalogs_lineEdit_Name->setText(selectedFilterCatalogName);
            ui->Catalogs_lineEdit_SourcePath->setText(selectedCatalog->sourcePath);
            ui->Catalogs_comboBox_FileType->setCurrentText(selectedCatalog->fileType);
            ui->Catalogs_comboBox_Storage->setCurrentText(selectedCatalog->storageName);
            ui->Catalogs_checkBox_IncludeHidden->setChecked(selectedCatalog->includeHidden);

            ui->Catalogs_widget_EditCatalog->hide();
        }
        //----------------------------------------------------------------------
        void MainWindow::on_Catalogs_pushButton_UpdateCatalog_clicked()
        {
            skipCatalogUpdateSummary= false;
            requestSource ="update";
            updateSingleCatalog(selectedCatalog);
        }
        //----------------------------------------------------------------------
        void MainWindow::on_Catalogs_pushButton_UpdateAllActive_clicked()
        {
            requestSource ="update";

            //user to confirm running this global update
                int confirm = QMessageBox::warning(this, "Katalog",
                                    tr("Are you sure you want to update ALL catalogs that are currently filtered and identified as active?")
                                         , QMessageBox::Yes
                                                  | QMessageBox::Cancel);
                if ( confirm == QMessageBox::Cancel){
                    return;
                }

            //user to choose showing or skipping summary for each catalog update
                skipCatalogUpdateSummary = false;
                int result = QMessageBox::warning(this, "Katalog",
                                    tr("Do you want to view the summary of updates at the end of each catalog update?")
                                         , QMessageBox::Yes
                                                  | QMessageBox::No);
                if ( result == QMessageBox::No){
                    skipCatalogUpdateSummary= true;
                }

            //Get list of displayed and active catalogs
                //prepare the main part of the query
                QString querySQL  = QLatin1String(R"(
                                            SELECT catalog_name, catalog_source_path_is_active, catalog_source_path, catalog_storage
                                            FROM catalog
                                            LEFT JOIN storage ON catalog_storage = storage_name
                                            WHERE catalog_source_path_is_active = 1
                                            )");

                    //add AND conditions for the selected filters
                    if ( selectedFilterStorageLocation != tr("All") )
                        querySQL = querySQL + " AND storage_location = '" + selectedFilterStorageLocation + "' ";

                    if ( selectedFilterStorageName != tr("All") )
                        querySQL = querySQL + " AND catalog_storage = '" + selectedFilterStorageName + "' ";

                //run the query
                QSqlQuery query;
                query.prepare(querySQL);
                query.exec();

            // Catalog each result
            while(query.next()){
                //Get catalog name
                QString catalogName = query.value(0).toString();

                QDir dir (query.value(2).toString());
                if (dir.exists()==true){

                    ///Warning and choice if the result is 0 files
                    if(dir.entryInfoList(QDir::NoDotAndDotDot|QDir::AllEntries).count() == 0)
                    {
                        //QMessageBox::information(this,"Katalog","dir Empty." );

                        //return;
                    }
                    else{
                        //Update catalog
                        tempCatalog->setName(catalogName);
                        tempCatalog->loadCatalogMetaData();
                        updateCatalogFileList(tempCatalog);

                       //Update storage
                        QString selectedCatalogStorage = query.value(3).toString();

                        //Update the related storage
                        if ( selectedCatalogStorage != ""){
                           //get Storage ID
                           QSqlQuery query;
                           QString querySQL = QLatin1String(R"(
                                               SELECT storage_id, storage_path FROM storage WHERE storage_name =:storage_name
                                                           )");
                           query.prepare(querySQL);
                           query.bindValue(":storage_name",selectedCatalogStorage);
                           query.exec();
                           query.next();
                           int selectedCatalogStorageID = query.value(0).toInt();
                           QString selectedCatalogStoragePath =  query.value(1).toString();

                           //Update storage
                           if ( selectedCatalogStoragePath!=""){
                               tempStorage->setID(selectedCatalogStorageID);
                               tempStorage->loadStorageMetaData();
                               updateStorageInfo(tempStorage);
                           }
                           else
                               QMessageBox::information(this,"Katalog",tr("The storage device name may not be correct:\n %1 ").arg(selectedCatalogStorage));

                       }

                       //refresh catalog lists
                          loadCatalogFilesToTable();
                          loadCatalogsTableToModel();
                    }
                }
            }
            QMessageBox::information(this,"Katalog",tr("Update of displayed and active catalogs completed."));
            skipCatalogUpdateSummary= false;
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
            //Change the selected catalog in Search tab
            ui->Filters_label_DisplayCatalog->setText(selectedCatalog->name);
            selectedFilterCatalogName = selectedCatalog->name;

            selectedFilterStorageLocation = tr("All");
            ui->Filters_label_DisplayLocation->setText(selectedFilterStorageLocation);

            selectedFilterStorageName = tr("All");
            ui->Filters_label_DisplayStorage->setText(selectedFilterStorageName);

            activeCatalog->setName(selectedCatalog->name);
            activeCatalog->loadCatalogMetaData();

            selectedDeviceName = selectedCatalog->name;
            selectedDeviceType = "Catalog";

            QSettings settings(settingsFilePath, QSettings:: IniFormat);
            settings.setValue("Selection/SelectedDeviceType", selectedDeviceType);
            settings.setValue("Selection/SelectedDeviceName", selectedDeviceName);

            filterFromSelectedDevices();

            //Go to the Search tab
            ui->tabWidget->setCurrentIndex(5); // tab 0 is the Search tab

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
                                                  tr("Do you want to delete this catalog?")+"\n"+selectedCatalog->filePath,QMessageBox::Yes|QMessageBox::Cancel);

            if ( result ==QMessageBox::Yes){

                if(databaseMode=="Memory"){
                    //move file to trash
                    if ( selectedCatalog->filePath != ""){


                              QFile file (selectedCatalog->filePath);
                              file.moveToTrash();

                    }
                    else QMessageBox::information(this,"Katalog",tr("Select a catalog above first."));

                    //Clear current entires from the table
                    QSqlQuery queryDelete;
                    queryDelete.exec("DELETE FROM catalog");

                    //refresh catalog lists
                    loadCatalogFilesToTable();
                }

                else if(databaseMode=="File"){
                    selectedCatalog->deleteCatalog();
                }

                loadCatalogsTableToModel();
                refreshCatalogSelectionList("","");
                loadStorageTableToSelectionTreeModel();
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
                QDesktopServices::openUrl(QUrl::fromLocalFile(selectedCatalog->filePath));
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
            requestSource = "update";
            saveCatalogChanges(selectedCatalog);
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
    void MainWindow::loadCatalogFilesToTable()
    {
        //Clear catalog table
            QSqlQuery queryDelete;
            queryDelete.prepare( "DELETE FROM catalog" );
            queryDelete.exec();

        //Iterate in the directory to create a list of files and sort it
            QStringList catalogFileExtensions;
            catalogFileExtensions << "*.idx";

            QDirIterator iterator(collectionFolder, catalogFileExtensions, QDir::Files, QDirIterator::Subdirectories);
            while (iterator.hasNext()){

                // Iterate to the next file
                QString path = iterator.next();
                QFile catalogFile(path);

                // Get file info
                QFileInfo catalogFileInfo(catalogFile);

                // Verify that the file can be opened
                if(!catalogFile.open(QIODevice::ReadOnly)) {
                    QMessageBox::information(this,"Katalog",tr("No catalog found."));
                    return;
                }

                //Prepare a textsteam for the file
                QTextStream textStreamCatalogs(&catalogFile);

                //Read the first 8 lines and put values in a stringlist
                QStringList catalogValues;
                QString line;
                QString value;
                for (int i=0; i<9; i++) {
                    line = textStreamCatalogs.readLine();
                    if (line !="" and QVariant(line.at(0)).toString()=="<"){
                        value = line.right(line.size() - line.lastIndexOf(">") - 1);
                        if (value =="") catalogValues << "";
                        else catalogValues << value;
                    }
                }
                if (catalogValues.count()==7) catalogValues << "false"; //for older catalog without isFullDevice
                if (catalogValues.count()==8) catalogValues << "false"; //for older catalog without includeMetadata

                if(catalogValues.length()>0){
                    // Verify if path is active (drive connected)
                    int isActive = verifyCatalogPath(catalogValues[0]);

                    //Insert a line in the table with available data

                    //prepare insert query for filesall
                    QSqlQuery insertCatalogQuery;
                    QString insertCatalogQuerySQL = QLatin1String(R"(
                                            INSERT OR IGNORE INTO catalog (
                                                            catalog_file_path,
                                                            catalog_name,
                                                            catalog_date_updated,
                                                            catalog_source_path,
                                                            catalog_file_count,
                                                            catalog_total_file_size,
                                                            catalog_source_path_is_active,
                                                            catalog_include_hidden,
                                                            catalog_file_type,
                                                            catalog_storage,
                                                            catalog_include_symblinks,
                                                            catalog_is_full_device,
                                                            catalog_date_loaded,
                                                            catalog_include_metadata
                                                            )
                                            VALUES(
                                                            :catalog_file_path,
                                                            :catalog_name,
                                                            :catalog_date_updated,
                                                            :catalog_source_path,
                                                            :catalog_file_count,
                                                            :catalog_total_file_size,
                                                            :catalog_source_path_is_active,
                                                            :catalog_include_hidden,
                                                            :catalog_file_type,
                                                            :catalog_storage,
                                                            :catalog_include_symblinks,
                                                            :catalog_is_full_device,
                                                            :catalog_date_loaded,
                                                            :catalog_include_metadata )
                                        )");

                    insertCatalogQuery.prepare(insertCatalogQuerySQL);
                    insertCatalogQuery.bindValue(":catalog_file_path",catalogFileInfo.filePath());
                    insertCatalogQuery.bindValue(":catalog_name",catalogFileInfo.completeBaseName());
                    insertCatalogQuery.bindValue(":catalog_date_updated",catalogFileInfo.lastModified().toString("yyyy-MM-dd hh:mm:ss"));
                    insertCatalogQuery.bindValue(":catalog_source_path",catalogValues[0]);
                    insertCatalogQuery.bindValue(":catalog_file_count",catalogValues[1].toInt());
                    insertCatalogQuery.bindValue(":catalog_total_file_size",catalogValues[2].toLongLong());
                    insertCatalogQuery.bindValue(":catalog_source_path_is_active",isActive);
                    insertCatalogQuery.bindValue(":catalog_include_hidden",catalogValues[3]);
                    insertCatalogQuery.bindValue(":catalog_file_type",catalogValues[4]);
                    insertCatalogQuery.bindValue(":catalog_storage",catalogValues[5]);
                    insertCatalogQuery.bindValue(":catalog_include_symblinks",catalogValues[6]);
                    insertCatalogQuery.bindValue(":catalog_is_full_device",catalogValues[7]);
                    insertCatalogQuery.bindValue(":catalog_date_loaded","");
                    insertCatalogQuery.bindValue(":catalog_include_metadata",catalogValues[8]);
                    insertCatalogQuery.exec();

                }
                catalogFile.close();
            }

    }
    //--------------------------------------------------------------------------
    void MainWindow::loadCatalogsTableToModel()
    {
        //Generate SQL query from filters.
            QString loadCatalogSQL;

            //prepare the main part of the query
            loadCatalogSQL  = QLatin1String(R"(
                                        SELECT
                                            catalog_name                 ,
                                            catalog_file_path            ,
                                            catalog_date_updated         ,
                                            catalog_file_count           ,
                                            catalog_total_file_size      ,
                                            catalog_source_path          ,
                                            catalog_file_type            ,
                                            catalog_source_path_is_active,
                                            catalog_include_hidden       ,
                                            catalog_include_metadata     ,
                                            catalog_storage              ,
                                            storage_location             ,
                                            catalog_is_full_device       ,
                                            catalog_date_loaded
                                        FROM catalog c
                                        LEFT JOIN storage s ON catalog_storage = storage_name
                                        WHERE catalog_name !=''
                                        )");

            if (     selectedFilterStorageLocation == tr("All")
                 and selectedFilterStorageName     == tr("All")
                 and selectedFilterCatalogName     == tr("All") )
                {//No filtering
            }
            else if ( selectedDeviceType == "Location" )
                loadCatalogSQL = loadCatalogSQL + " AND s.storage_location = '"+ selectedDeviceName +"' ";

            else if ( selectedDeviceType == "Storage" )
                loadCatalogSQL = loadCatalogSQL + " AND catalog_storage = '"+ selectedDeviceName +"' ";

            else if ( selectedDeviceType == "Catalog" )
                loadCatalogSQL = loadCatalogSQL + " AND catalog_name = '"+ selectedDeviceName +"' ";


            //Execute query
            QSqlQuery loadCatalogQuery;
            loadCatalogQuery.prepare(loadCatalogSQL);
            loadCatalogQuery.exec();

            //Format and send to Treeview
            QSqlQueryModel *catalogQueryModel = new QSqlQueryModel;
            catalogQueryModel->setQuery(std::move(loadCatalogQuery));

            CatalogsView *proxyResultsModel = new CatalogsView(this);
            proxyResultsModel->setSourceModel(catalogQueryModel);

            proxyResultsModel->setHeaderData(0, Qt::Horizontal, tr("Name"));
            proxyResultsModel->setHeaderData(2, Qt::Horizontal, tr("Last Update"));
            proxyResultsModel->setHeaderData(3, Qt::Horizontal, tr("Files"));
            proxyResultsModel->setHeaderData(4, Qt::Horizontal, tr("Total Size"));
            proxyResultsModel->setHeaderData(5, Qt::Horizontal, tr("Source Path"));
            proxyResultsModel->setHeaderData(6, Qt::Horizontal, tr("File Type"));
            proxyResultsModel->setHeaderData(7, Qt::Horizontal, tr("Active"));
            proxyResultsModel->setHeaderData(8, Qt::Horizontal, tr("Inc.Hidden"));
            proxyResultsModel->setHeaderData(9, Qt::Horizontal, tr("Inc.Metadata"));
            proxyResultsModel->setHeaderData(10,Qt::Horizontal, tr("Storage"));
            proxyResultsModel->setHeaderData(11,Qt::Horizontal, tr("Location"));
            proxyResultsModel->setHeaderData(12,Qt::Horizontal, tr("Full Device"));
            proxyResultsModel->setHeaderData(13,Qt::Horizontal, tr("Last Loaded"));

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
            ui->Catalogs_treeView_CatalogList->header()->resizeSection(5, 300); //Path
            ui->Catalogs_treeView_CatalogList->header()->resizeSection(6, 100); //FileType
            ui->Catalogs_treeView_CatalogList->header()->resizeSection(7,  50); //Active
            ui->Catalogs_treeView_CatalogList->header()->resizeSection(8,  50); //includeHidden
            ui->Catalogs_treeView_CatalogList->header()->resizeSection(9,  50); //includeMetadata
            ui->Catalogs_treeView_CatalogList->header()->resizeSection(10,150); //Storage
            ui->Catalogs_treeView_CatalogList->header()->resizeSection(11,150); //Location
            ui->Catalogs_treeView_CatalogList->header()->resizeSection(12, 50); //FullDevice
            ui->Catalogs_treeView_CatalogList->header()->resizeSection(13,150); //Last Loaded

            //Hide columns
            if(developmentMode==false){
                ui->Catalogs_treeView_CatalogList->hideColumn( 9); //includeMetadata
                ui->Catalogs_treeView_CatalogList->hideColumn(13); //date Loaded
                ui->Catalogs_treeView_CatalogList->hideColumn(12); //isFullDevice
            }
            //Populate catalogs statistics
            QSqlQuery query;
            QString querySQL = QLatin1String(R"(
                                SELECT COUNT(catalog_name),SUM(catalog_total_file_size),SUM(catalog_file_count)
                                FROM catalog
                                LEFT JOIN storage ON catalog_storage = storage_name
                                WHERE catalog_name !=''
                                            )");

            if ( selectedFilterStorageLocation !=tr("All")){
                querySQL = querySQL + " AND storage_location =:storage_location";
            }
            if ( selectedFilterStorageName !=tr("All")){
                querySQL = querySQL + " AND catalog_storage =:catalog_storage";
            }

            query.prepare(querySQL);
            query.bindValue(":storage_location",selectedFilterStorageLocation);
            query.bindValue(":catalog_storage", selectedFilterStorageName);
            query.exec();
            query.next();

            ui->Catalogs_label_Catalogs->setText(QString::number(query.value(0).toInt()));
            ui->Catalogs_label_TotalSize->setText(QLocale().formattedDataSize(query.value(1).toLongLong()));
            ui->Catalogs_label_TotalNumber->setText(QLocale().toString(query.value(2).toInt()));

    }
    //--------------------------------------------------------------------------
    int MainWindow::verifyCatalogPath(QString catalogSourcePath)
    {
        // Verify that the catalog path is accessible (so the related drive is mounted), returns true/false
        QDir dir(catalogSourcePath);
        int status = dir.exists();
        return status;
    }
    //--------------------------------------------------------------------------
    void MainWindow::recordSelectedCatalogStats(QString selectedCatalogName, int selectedCatalogFileCount, qint64 selectedCatalogTotalFileSize)
    {
        QDateTime nowDateTime = QDateTime::currentDateTime();

        QString statisticsLine = nowDateTime.toString("yyyy-MM-dd hh:mm:ss") + "\t"
                                + selectedCatalogName + "\t"
                                + QString::number(selectedCatalogFileCount) + "\t"
                                + QString::number(selectedCatalogTotalFileSize) + "\t"
                                + "Update" ;

        if(databaseMode=="Memory"){
            // Stream the list to the file
            QFile fileOut( collectionFolder + "/" + statisticsFileName );

            // Write data
            if (fileOut.open(QFile::WriteOnly | QIODevice::Append | QFile::Text)) {
                QTextStream stream(&fileOut);
                stream << statisticsLine << "\n";
             }
             fileOut.close();
        }
        else if(databaseMode=="File"){
             QSqlQuery query;
             QString querySQL = QLatin1String(R"(
                                INSERT INTO statistics(
                                                date_time,
                                                catalog_name,
                                                catalog_file_count,
                                                catalog_total_file_size,
                                                record_type
                                                )
                                VALUES(
                                                :date_time,
                                                :catalog_name,
                                                :catalog_file_count,
                                                :catalog_total_file_size,
                                                :record_type
                                                )
                                )");
             query.prepare(querySQL);
             query.bindValue(":date_time",nowDateTime.toString("yyyy-MM-dd hh:mm:ss"));
             query.bindValue(":catalog_name",selectedCatalogName);
             query.bindValue(":catalog_file_count",QString::number(selectedCatalogFileCount));
             query.bindValue(":catalog_total_file_size",QString::number(selectedCatalogTotalFileSize));
             query.bindValue(":record_type","Update");
             query.exec();
        }
    }
    //--------------------------------------------------------------------------
    void MainWindow::backupCatalog(QString catalogSourcePath)
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
    void MainWindow::updateSingleCatalog(Catalog *catalog)
    {
        //Update catalog file list      
        updateCatalogFileList(catalog);

        //Update its storage
        if ( catalog->storageName != ""){
            //get Storage ID
            QSqlQuery query;
            QString querySQL = QLatin1String(R"(
                                    SELECT storage_id, storage_path
                                    FROM storage
                                    WHERE storage_name =:storage_name
                                )");
            query.prepare(querySQL);
            query.bindValue(":storage_name",catalog->storageName);
            query.exec();
            query.next();
            int selectedCatalogStorageID = query.value(0).toInt();
            QString selectedCatalogStoragePath =  query.value(1).toString();

            //Update storage
            if ( selectedCatalogStoragePath!=""){
                tempStorage->setID(selectedCatalogStorageID);
                tempStorage->loadStorageMetaData();
                updateStorageInfo(tempStorage);
            }
        }

        //Refresh catalog lists
        if(databaseMode=="Memory")
           loadCatalogFilesToTable();

        loadCatalogsTableToModel();

    }
    //--------------------------------------------------------------------------
    void MainWindow::updateCatalogFileList(Catalog *catalog)
    {
        if(databaseMode=="Memory"){
           //Check if the update can be done, inform the user otherwise.
           //Deal with old versions, where necessary info may have not have been available
           if(catalog->filePath == "not recorded" or catalog->name == "not recorded" or catalog->sourcePath == "not recorded"){
                QMessageBox::information(this,"Katalog",tr("It seems this catalog was not correctly imported or has an old format.\n"
                                                             "Edit it and make sure it has the following first 2 lines:\n\n"
                                                             "<catalogSourcePath>/folderpath\n"
                                                             "<catalogFileCount>10000\n\n"
                                                             "Copy/paste these lines at the begining of the file and modify the values after the >:\n"
                                                             "- the catalogSourcePath is the folder to catalog the files from.\n"
                                                             "- the catalogFileCount number does not matter as much, it can be updated.\n")
                                         );
                return;
           }

           //Deal with other cases where some input information is missing
           if(catalog->filePath == "" or catalog->name == "" or catalog->sourcePath == ""){
                QMessageBox::information(this,"Katalog",tr("Select a catalog first (some info is missing).\n currentCatalogFilePath: %1 \n currentCatalogName: %2 \n currentCatalogSourcePath: %3").arg(
                                                              catalog->filePath, catalog->name, catalog->sourcePath));
                return;
           }

           //BackUp the file before, if the option is selected
           if ( ui->Settings_checkBox_KeepOneBackUp->isChecked() == true){
                backupCatalog(catalog->filePath);
           }

        }

        //Capture previous FileCount and TotalFileSize to report the changes after the update
            qint64 previousFileCount     = catalog->fileCount;
            qint64 previousTotalFileSize = catalog->totalFileSize;

        //Process if dir exists
        QDir dir (catalog->sourcePath);
        if (dir.exists()==true){
            ///Warning and choice if the result is 0 files
            if(dir.entryInfoList(QDir::NoDotAndDotDot|QDir::AllEntries).count() == 0)
            {
                int result = QMessageBox::warning(this, "Katalog - Warning",
                                    tr("The source folder does not contain any file.\n"
                                         "This could mean that the source is empty or the device is not mounted to this folder.\n")
                                         +tr("Do you want to save it anyway (the catalog would be empty)?\n"), QMessageBox::Yes
                                                  | QMessageBox::Cancel);
                if ( result == QMessageBox::Cancel){
                    return;
                }
            }

            //catalog the directory and save it to the file
            catalogDirectory(catalog);

            if(databaseMode=="Memory"){
                saveCatalogToNewFile(catalog->name);
                saveFoldersToNewFile(catalog->name);
            }

            //Prepare to report changes to the catalog
            qint64 deltaFileCount     = catalog->fileCount     - previousFileCount;
            qint64 deltaTotalFileSize = catalog->totalFileSize - previousTotalFileSize;

            //Inform user about the update
            if(skipCatalogUpdateSummary !=true){
                QMessageBox msgBox;
                QString message;
                if (requestSource=="update")
                    message = QString(tr("<br/>This catalog was updated:<br/><b> %1 </b> <br/>")).arg(catalog->name);
                else if (requestSource=="create")
                    message = QString(tr("<br/>This catalog was created:<br/><b> %1 </b> <br/>")).arg(catalog->name);

                message += QString("<table> <tr><td>Number of files: </td><td><b> %1 </b></td><td>  (added: <b> %2 </b>)</td></tr>"
                                     "<tr><td>Total file size: </td><td><b> %3 </b>  </td><td>  (added: <b> %4 </b>)</td></tr></table>"
                                     ).arg(QString::number(catalog->fileCount),
                                           QString::number(deltaFileCount),
                                           QLocale().formattedDataSize(catalog->totalFileSize),
                                           QLocale().formattedDataSize(deltaTotalFileSize));
                msgBox.setWindowTitle("Katalog");
                msgBox.setText(message);
                msgBox.setIcon(QMessageBox::Information);
                msgBox.exec();
            }
        }
        else {
            QMessageBox::information(this,"Katalog",tr("The catalog %1 cannot be updated.\n"
                                            "\n The source folder - %2 - was not found.\n"
                                            "\n Possible reasons:\n"
                                            "    - the device is not connected and mounted,\n"
                                            "    - the source folder was moved or renamed.")
                                            .arg(catalog->name,
                                                   catalog->sourcePath)
                                     );
        }

        //record catalog statistics if option is selected
        if ( ui->Settings_checkBox_SaveRecordWhenUpdate->isChecked() == true )
            recordSelectedCatalogStats(catalog->name, catalog->fileCount, catalog->totalFileSize);

        //Refresh data to UI
        if(databaseMode=="Memory")
            loadCatalogFilesToTable();

        loadCatalogsTableToModel();
        loadStatisticsChart();

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
                QStringList headerList = line.split('\t');

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
                    QFile fileOut( collectionFolder +"/"+ newCatalogName + ".idx" );

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
    void MainWindow::saveCatalogChanges(Catalog *catalog)
    {
            //Get new values
            //get new name
            QString previousCatalogName = catalog->name;
            QString newCatalogName = ui->Catalogs_lineEdit_Name->text();

            //get new catalog sourcePath: remove the / at the end if any, except for / alone (root directory in linux)
            QString newCatalogSourcePath    = ui->Catalogs_lineEdit_SourcePath->text();
            int     pathLength              = newCatalogSourcePath.length();
            if (newCatalogSourcePath !="" and newCatalogSourcePath !="/" and QVariant(newCatalogSourcePath.at(pathLength-1)).toString()=="/") {
                newCatalogSourcePath.remove(pathLength-1,1);
            }
            QString newCatalogStorage         = ui->Catalogs_comboBox_Storage->currentText();
            QString newCatalogFileType        = ui->Catalogs_comboBox_FileType->itemData(ui->Catalogs_comboBox_FileType->currentIndex(),Qt::UserRole).toString();
            QString newCatalogIncludeHidden   = QVariant(ui->Catalogs_checkBox_IncludeHidden->isChecked()).toString();
            QString newCatalogIncludeMetadata = QVariant(ui->Catalogs_checkBox_IncludeMetadata->isChecked()).toString();

            bool    isFullDevice            = ui->Catalogs_checkBox_isFullDevice->checkState();
            //DEV:QString newIncludeSymblinks  = ui->Catalogs_checkBox_IncludeSymblinks->currentText();

        //Confirm save changes
            QString message = tr("Save changes to the definition of the catalog?<br/>");
            message = message + "<table> <tr><td width=155><i>" + tr("field") + "</i></td><td width=125><i>" + tr("previous value") + "</i></td><td width=200><i>" + tr("new value") + "</i></td>";
            if(newCatalogName           !=catalog->name)
                message = message + "<tr><td>" + tr("Name")         + "</td><td>" + catalog->name         + "</td><td><b>" + newCatalogName          + "</b></td></tr>";
            if(newCatalogSourcePath     !=catalog->sourcePath)
                message = message + "<tr><td>" + tr("Source path ") + "</td><td>" + catalog->sourcePath   + "</td><td><b>" + newCatalogSourcePath    + "</b></td></tr>";
            if(newCatalogStorage        !=catalog->storageName)
                message = message + "<tr><td>" + tr("Storage name") + "</td><td>" + catalog->storageName  + "</td><td><b>" + newCatalogStorage       + "</b></td></tr>";
            if(newCatalogFileType       !=catalog->fileType)
                message = message + "<tr><td>" + tr("File Type")    + "</td><td>" + catalog->fileType     + "</td><td><b>" + newCatalogFileType      + "</b></td></tr>";
            if(newCatalogIncludeHidden  !=QVariant(catalog->includeHidden).toString())
                message = message + "<tr><td>" + tr("Include Hidden")   + "</td><td>" + QVariant(catalog->includeHidden).toString()   + "</td><td><b>" + newCatalogIncludeHidden   + "</b></td></tr>";
            if(newCatalogIncludeMetadata  !=QVariant(catalog->includeMetadata).toString())
                message = message + "<tr><td>" + tr("Include Metadata") + "</td><td>" + QVariant(catalog->includeMetadata).toString() + "</td><td><b>" + newCatalogIncludeMetadata + "</b></td></tr>";

            message = message + "</table>";

            if(    (newCatalogSourcePath     !=catalog->sourcePath)
                or (newCatalogFileType       !=catalog->fileType)
                or (newCatalogIncludeHidden  !=QVariant(catalog->includeHidden).toString())
                or (newCatalogIncludeMetadata  !=QVariant(catalog->includeMetadata).toString())
            ){
                    message = message + + "<br/><br/>" + tr("(The catalog must be updated to reflect these changes)");
            }

            int result = QMessageBox::warning(this, "Katalog", message, QMessageBox::Yes | QMessageBox::Cancel);
            if ( result == QMessageBox::Cancel){
                return;
            }

        //Write all changes to database (except change of name)
            QSqlQuery query;
            QString querySQL = QLatin1String(R"(
                                    UPDATE catalog
                                    SET catalog_source_path      =:catalog_source_path,
                                        catalog_storage          =:catalog_storage,
                                        catalog_file_type        =:catalog_file_type,
                                        catalog_include_hidden   =:catalog_include_hidden,
                                        catalog_include_metadata =:catalog_include_metadata
                                    WHERE catalog_name =:catalog_name
                                )");
            //DEV: catalogIncludeSymblinks =: newIncludeSymblinks;

            query.prepare(querySQL);
            query.bindValue(":catalog_source_path",      newCatalogSourcePath);
            query.bindValue(":catalog_storage",          newCatalogStorage);
            query.bindValue(":catalog_file_type",        newCatalogFileType);
            query.bindValue(":catalog_include_hidden",   newCatalogIncludeHidden);
            query.bindValue(":catalog_include_metadata", newCatalogIncludeMetadata);
            query.bindValue(":catalog_name",             catalog->name);
            //DEV:query.bindValue(":catalogIncludeSymblinks", catalog->includeSymblinks);
            query.exec();

            loadCatalogsTableToModel();

        //Write changes to catalog file (update headers only)
        if(databaseMode=="Memory"){
            QFile catalogFile(catalog->filePath);
            if(catalogFile.open(QIODevice::ReadWrite | QIODevice::Text))
            {
                QString fullFileText;
                QTextStream textStream(&catalogFile);

                while(!textStream.atEnd())
                {
                    QString line = textStream.readLine();
                    //DEV: bool addedIsFullDevice = false;

                    //add file data line
                    if(!line.startsWith("<catalogSourcePath")
                            and !line.startsWith("<catalogIncludeHidden")
                            and !line.startsWith("<catalogFileType")
                            and !line.startsWith("<catalogStorage")
                            and !line.startsWith("<catalogIsFullDevice")
                            and !line.startsWith("<catalogIncludeMetadata")
                            )
                    {
                        fullFileText.append(line + "\n");
                    }
                    else{
                        //add catalog meta-data. The ifs must be in the correct order of the meta-data lines
                        if(line.startsWith("<catalogSourcePath>"))
                                fullFileText.append("<catalogSourcePath>" + newCatalogSourcePath +"\n");

                        if(line.startsWith("<catalogIncludeHidden>"))
                                fullFileText.append("<catalogIncludeHidden>" + QVariant(newCatalogIncludeHidden).toString() +"\n");

                        if(line.startsWith("<catalogFileType>"))
                                fullFileText.append("<catalogFileType>" + newCatalogFileType +"\n");

                        if(line.startsWith("<catalogStorage>"))
                                fullFileText.append("<catalogStorage>" + newCatalogStorage +"\n");

                        if(line.startsWith("<catalogIsFullDevice>")){
                                fullFileText.append("<catalogIsFullDevice>" + QVariant(isFullDevice).toString() +"\n");
                            //DEV: addedIsFullDevice = true;
                        }
                        if(line.startsWith("<catalogIncludeMetadata>")){
                                fullFileText.append("<catalogIncludeMetadata>" + newCatalogIncludeMetadata +"\n");
                            //DEV: addedIsFullDevice = true;
                        }
                    }
                    //DEV: if(addedIsFullDevice ==false){
                    //DEV:      //add missing line
                    //DEV:      fullFileText.prepend("<catalogIsFullDevice>" + QVariant(isFullDevice).toString() +"\n");
                    //DEV: }
                }
                catalogFile.resize(0);
                textStream << fullFileText;
                catalogFile.close();
            }
            else {
                QMessageBox::information(this,"Katalog",tr("Could not open file."));
            }
        }

        //Rename the catalog file
        if (newCatalogName != previousCatalogName){

            //verfiy if name exists
            QSqlQuery query;
            QString querySQL = QLatin1String(R"(
                                SELECT catalog_name
                                FROM catalog
                                WHERE catalog_name=:catalog_name
                            )");
            query.prepare(querySQL);
            query.bindValue(":catalog_name",newCatalogName);
            query.exec();
            query.next();
            if (query.value(0).toString() !=""){
                QMessageBox msgBox;
                msgBox.setWindowTitle("Katalog");
                msgBox.setText( tr("There is already a catalog with this name:<br/><b>")
                               + newCatalogName
                               + "</b><br/><br/>"+tr("Choose a different name."));
                msgBox.setIcon(QMessageBox::Critical);
                msgBox.exec();
                return;
            }

            catalog->renameCatalog(newCatalogName);

            if(databaseMode=="Memory"){
                catalog->renameCatalogFile(newCatalogName);
                loadCatalogFilesToTable();
                loadCatalogsTableToModel();
            }

            //Rename in statistics
            int renameChoice = QMessageBox::warning(this, "Katalog", tr("Apply the change in the statistics file?\n")
                                                    , QMessageBox::Yes | QMessageBox::No);

            if (renameChoice == QMessageBox::Yes){
                if(databaseMode=="Memory"){
                    QFile f(statisticsFilePath);
                    if(f.open(QIODevice::ReadWrite | QIODevice::Text))
                    {
                        QString s;
                        QTextStream t(&f);
                        while(!t.atEnd())
                        {
                                QString line = t.readLine();
                                QStringList lineParts = line.split("\t");
                                if (lineParts[1]==previousCatalogName){
                                    lineParts[1]= newCatalogName;
                                    line = lineParts.join("\t");
                                }
                                s.append(line + "\n");
                        }
                        f.resize(0);
                        t << s;
                        f.close();
                    }
                }
                else if(databaseMode=="File"){
                    QSqlQuery query;
                    QString querySQL = QLatin1String(R"(
                                    UPDATE statistics
                                    SET catalog_name =:newCatalog_name
                                    WHERE catalog_name =:previousCatalogName
                                )");
                    query.prepare(querySQL);
                    query.bindValue(":newCatalog_name", newCatalogName);
                    query.bindValue(":previousCatalogName", previousCatalogName);
                    query.exec();
                }
            }

        }

        //Refresh display
        loadCatalogsTableToModel();
        refreshCatalogSelectionList("","");

        //Update the list of files if the changes impact the contents (i.e. path, file type, hidden)
            if (       newCatalogSourcePath      != catalog->sourcePath
                    or newCatalogIncludeHidden   != QVariant(catalog->includeHidden).toString()
                    or newCatalogIncludeMetadata != QVariant(catalog->includeMetadata).toString()
                    or newCatalogFileType        != catalog->fileType)
            {
                int updatechoice = QMessageBox::warning(this, "Katalog",
                                    tr("Update the catalog content with the new criteria?\n")
                                         , QMessageBox::Yes
                                                  | QMessageBox::No);
                if ( updatechoice == QMessageBox::Yes){
                    tempCatalog->setName(newCatalogName);
                    tempCatalog->loadCatalogMetaData();
                    updateCatalogFileList(tempCatalog);
                }
            }

        //Refresh
            if(databaseMode=="Memory")
                loadCatalogFilesToTable();

            loadCatalogsTableToModel();
            loadCatalogsTableToModel();
            loadStorageTableToSelectionTreeModel();

        //Hide edition section
            ui->Catalogs_widget_EditCatalog->hide();

    }
    //--------------------------------------------------------------------------
    void MainWindow::recordCollectionStats(){

            //Get last total file size and number
            qint64 lastTotalFileSize;
            qint64 lastTotalFileNumber;

            //get the list of catalogs and data
                QSqlQuery queryLast;
                QString queryLastSQL = QLatin1String(R"(
                                            SELECT SUM(catalog_total_file_size), SUM(catalog_file_count)
                                            FROM statistics
                                            WHERE date_time = (SELECT MAX(date_time)
                                                                FROM statistics
                                                                WHERE record_type="Snapshot")
                                            GROUP BY date_time
                                            )");
                queryLast.prepare(queryLastSQL);
                queryLast.exec();
                queryLast.next();
                lastTotalFileSize   = queryLast.value(0).toLongLong();
                lastTotalFileNumber = queryLast.value(1).toLongLong();

            //record all current catalog value
            recordAllCatalogStats();

            //Get new total file size and number
            qint64 newTotalFileSize;
            qint64 newTotalFileNumber;

            //get the list of catalogs and data
                QSqlQuery queryNew;
                QString queryNewSQL = QLatin1String(R"(
                                            SELECT SUM(catalog_total_file_size), SUM(catalog_file_count)
                                            FROM statistics
                                            WHERE date_time = (SELECT MAX(date_time)
                                                                FROM statistics
                                                                WHERE record_type="Snapshot")
                                            GROUP BY date_time
                                            )");
                queryNew.prepare(queryNewSQL);
                queryNew.exec();
                queryNew.next();
                newTotalFileSize   = queryNew.value(0).toLongLong();
                newTotalFileNumber = queryNew.value(1).toLongLong();

            //Calculate and inform
            qint64 deltaTotalFileSize = newTotalFileSize - lastTotalFileSize;
            qint64 deltaTotalFileNumber = newTotalFileNumber - lastTotalFileNumber;

            QMessageBox msgBox;
            msgBox.setWindowTitle("Katalog");
            msgBox.setText(tr("<br/>A snapshot of this collection was recorded:<br/><br/>"
                              "<table> <tr><td>Number of files: </td><td><b> %1 </b></td><td>  (added: <b> %2 </b>)</td></tr>"
                              "<tr><td>Total file size: </td><td><b> %3 </b>  </td><td>  (added: <b> %4 </b>)</td></tr></table>"
                              ).arg(QString::number(newTotalFileNumber),
                                    QString::number(deltaTotalFileNumber),
                                    QLocale().formattedDataSize(newTotalFileSize),
                                    QLocale().formattedDataSize(deltaTotalFileSize)
                                    ));
            msgBox.setIcon(QMessageBox::Information);
            msgBox.exec();

    }
    //--------------------------------------------------------------------------
    void MainWindow::recordAllCatalogStats()
    {
        // Save the values (size and number of files) of all catalogs to the statistics file, creating a snapshop of the collection.
        QDateTime nowDateTime = QDateTime::currentDateTime();
        QString nowDateTimeFormatted = nowDateTime.toString("yyyy-MM-dd hh:mm:ss");

        //get the list of catalogs and data
            QSqlQuery query;
            QString querySQL = QLatin1String(R"(
                                        SELECT
                                            catalog_name                 ,
                                            catalog_file_count            ,
                                            catalog_total_file_size
                                        FROM catalog
                                        )");
            query.prepare(querySQL);
            query.exec();

        //create a new record for each catalog
            QString recordCatalogName;
            QString recordCatalogFileCount;
            QString recordCatalogTotalFileSize;

            while (query.next()){
                    recordCatalogName = query.value(0).toString();
                    recordCatalogFileCount = query.value(1).toString();
                    recordCatalogTotalFileSize = query.value(2).toString();

                    QString statisticsLine = nowDateTimeFormatted + "\t"
                                            + recordCatalogName + "\t"
                                            + recordCatalogFileCount + "\t"
                                            + recordCatalogTotalFileSize + "\t"
                                            + "Snapshot" ;

                    //QMessageBox::information(this,"Katalog","Ok." + statisticsLine);

                    // Stream the values to the file
                        QFile fileOut( collectionFolder + "/" + statisticsFileName );

                        // Write data
                        if (fileOut.open(QFile::WriteOnly | QIODevice::Append | QFile::Text)) {
                            QTextStream stream(&fileOut);
                            stream << statisticsLine << "\n";
                         }
                         fileOut.close();

            }

            //refresh graphs
            loadStatisticsData();
            loadStatisticsChart();

    }
