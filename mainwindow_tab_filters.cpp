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
// File Name:   mainwindow_tab_filters.cpp
// Purpose:     methods for the Fitlers panel
// Description:
// Author:      Stephane Couturier
/////////////////////////////////////////////////////////////////////////////
*/

#include "mainwindow.h"
#include "ui_mainwindow.h"

#include "storagetreemodel.h"
#include "devicetreeview.h"
#include "catalog.h"

//FILTERS / Global ---------------------------------------------------------

    void MainWindow::on_Filters_pushButton_Filters_Hide_clicked()
    {
        ui->splitter_widget_Filters->setHidden(true);
        ui->main_widget_ShowFilters->setHidden(false);

        QSettings settings(settingsFilePath, QSettings:: IniFormat);
        settings.setValue("Settings/ShowHideFilters", "go-next");
    }
    //----------------------------------------------------------------------
    void MainWindow::on_Filters_pushButton_Filters_Show_clicked()
    {
        ui->splitter_widget_Filters->setHidden(false);
        ui->main_widget_ShowFilters->setHidden(true);

        QSettings settings(settingsFilePath, QSettings:: IniFormat);
        settings.setValue("Settings/ShowHideFilters", "go-previous");
    }
    //----------------------------------------------------------------------
    void MainWindow::on_Filters_pushButton_ResetGlobal_clicked()
    {
        resetSelection();
    }
    //----------------------------------------------------------------------
    void MainWindow::on_Filters_pushButton_ReloadCollection_clicked()
    {
        if(databaseMode=="Memory"){
            createStorageList();
        }
        generateCollectionFilesPaths();
        updateAllCatalogPathIsActive();
        loadCollection();
    }
    //----------------------------------------------------------------------
    void MainWindow::on_Filters_checkBox_SearchInCatalogs_toggled(bool checked)
    {
        QSettings settings(settingsFilePath, QSettings:: IniFormat);
        settings.setValue("LastSearch/searchInFileCatalogsChecked", checked);

        if(checked==1){
            //Enable Catalogs selection
            ui->Filters_widget_CatalogSelectionBox->setEnabled(true);
            ui->Filters_widget_CatalogSelectionTree->setEnabled(true);
            ui->Filters_widget_ConnectedDrives->setDisabled(true);
            ui->Filters_checkBox_SearchInConnectedDrives->setChecked(false);
        }
        else if(ui->Filters_checkBox_SearchInConnectedDrives->isChecked()==true){
            //Disable Catalogs selection
            ui->Filters_widget_CatalogSelectionBox->setDisabled(true);
            ui->Filters_widget_CatalogSelectionTree->setDisabled(true);
        }
        else{
            //Prevent uncheck if SearchInConnectedDrives is also unchecked
            ui->Filters_checkBox_SearchInCatalogs->setChecked(true);
        }
    }
    //----------------------------------------------------------------------
    void MainWindow::on_Filters_checkBox_SearchInConnectedDrives_toggled(bool checked)
    {
        QSettings settings(settingsFilePath, QSettings:: IniFormat);
        settings.setValue("LastSearch/searchInConnectedDriveChecked", checked);

        if(checked==1){
            //Enable Directory selection
            ui->Filters_widget_ConnectedDrives->setEnabled(true);
            ui->Filters_widget_CatalogSelectionBox->setDisabled(true);
            ui->Filters_widget_CatalogSelectionTree->setDisabled(true);
            ui->Filters_checkBox_SearchInCatalogs->setChecked(false);
            ui->Filters_widget_CatalogSelectionBox->hide();
            ui->Filters_widget_CatalogSelectionTree->hide();
            ui->Filters_widget_ConnectedDrives->show();
        }
        else if(ui->Filters_checkBox_SearchInCatalogs->isChecked()==true){
            //Disable Directory selection
            ui->Filters_widget_ConnectedDrives->setDisabled(true);
            ui->Filters_widget_CatalogSelectionBox->show();
            ui->Filters_widget_CatalogSelectionTree->show();
            ui->Filters_widget_ConnectedDrives->hide();
        }
        else{
            //Prevent uncheck if SearchInCatalogs is also unchecked
            ui->Filters_checkBox_SearchInConnectedDrives->setChecked(true);
        }
    }
    //----------------------------------------------------------------------

//FILTERS / Device tree ----------------------------------------------------

    // Top buttons ---------------------------------------------------------
    void MainWindow::on_Filter_pushButton_Search_clicked()
    {
        //Go to search tab
        ui->tabWidget->setCurrentIndex(0);
    }
    //----------------------------------------------------------------------
    void MainWindow::on_Filter_pushButton_Explore_clicked()
    {
        //reloads catalog to explore at root level
        if (selectedDeviceType=="Catalog"){
            selectedCatalog->setName(selectedDeviceName);
            selectedCatalog->loadCatalogMetaData();

            openCatalogToExplore();

            //Go to explore tab
            ui->tabWidget->setCurrentIndex(2);
        }
    }
    //----------------------------------------------------------------------
    void MainWindow::on_Filter_pushButton_Update_clicked()
    {
        //reloads catalog to explore at root level
        if (selectedDeviceType=="Catalog"){
            skipCatalogUpdateSummary= false;
            requestSource ="update";
            updateSingleCatalog(selectedCatalog, true);
        }
    }
    //----------------------------------------------------------------------
    void MainWindow::on_Filter_comboBox_TreeType_currentTextChanged(const QString &arg1)
    {
        selectedTreeType = arg1;
        if (selectedTreeType==tr("Location / Storage / Catatog")){
            loadStorageTableToSelectionTreeModel();
        }
        else if (selectedTreeType==tr("Virtual Storage / Catalog")){
            loadVirtualStorageTableToSelectionTreeModel();
        }

        QSettings settings(settingsFilePath, QSettings:: IniFormat);
        settings.setValue("Filters/LastTreeType", arg1);
    }
    //----------------------------------------------------------------------
    void MainWindow::on_Filters_pushButton_TreeExpandCollapse_clicked()
    {
        setTreeExpandState(true);
    }
    //----------------------------------------------------------------------

    // Device tree ---------------------------------------------------------
    void MainWindow::on_Filters_treeView_Devices_clicked(const QModelIndex &index)
    {
        selectedDeviceName = ui->Filters_treeView_Devices->model()->index(index.row(), 0, index.parent() ).data().toString();
        selectedDeviceType = ui->Filters_treeView_Devices->model()->index(index.row(), 1, index.parent() ).data().toString();

        if (selectedDeviceType=="Storage"){
            selectedDeviceID = ui->Filters_treeView_Devices->model()->index(index.row(), 2, index.parent() ).data().toInt();
            selectedStorage->setID(selectedDeviceID);
            selectedStorage->loadStorageMetaData();
        }
        else if (selectedDeviceType=="VirtualStorage"){
            selectedFilterVirtualStorageName = ui->Filters_treeView_Devices->model()->index(index.row(), 0, index.parent() ).data().toString();
            selectedFilterVirtualStorageID   = ui->Filters_treeView_Devices->model()->index(index.row(), 2, index.parent() ).data().toString();
        }
        else if (selectedDeviceType=="Catalog"){
            selectedCatalog->setName(selectedDeviceName);
            selectedCatalog->loadCatalogMetaData();
        }

        QSettings settings(settingsFilePath, QSettings:: IniFormat);
        settings.setValue("Selection/SelectedDeviceType", selectedDeviceType);
        settings.setValue("Selection/SelectedDeviceName", selectedDeviceName);
        settings.setValue("Selection/SelectedDeviceID",   selectedDeviceID);

        filterFromSelectedDevices();

        refreshDifferencesCatalogSelection();

        if(selectedDeviceType=="Catalog"){
            if(selectedVirtualStorageName!=""){
                ui->Virtual_pushButton_AssignCatalog->setEnabled(true);
            }
            ui->Virtual_label_SelectedCatalogDisplay->setText(selectedDeviceName);
        }
        else{
            ui->Virtual_pushButton_AssignCatalog->setEnabled(false);
            ui->Virtual_label_SelectedCatalogDisplay->setText("");
        }
    }
    //----------------------------------------------------------------------
    void MainWindow::on_Filters_treeView_Devices_customContextMenuRequested(const QPoint &pos)
    {
        //Get selection data
        QModelIndex index=ui->Filters_treeView_Devices->currentIndex();
        selectedDeviceName = ui->Filters_treeView_Devices->model()->index(index.row(), 0, index.parent() ).data().toString();
        selectedDeviceType = ui->Filters_treeView_Devices->model()->index(index.row(), 1, index.parent() ).data().toString();
//        selectedVirtualStorageName = ui->Filters_treeView_Devices->model()->index(index.row(), 0, index.parent() ).data().toString();
//        selectedVirtualStorageType = ui->Filters_treeView_Devices->model()->index(index.row(), 1, index.parent() ).data().toString();
//        selectedVirtualStorageID   = ui->Filters_treeView_Devices->model()->index(index.row(), 3, index.parent() ).data().toInt();
//        QModelIndex parentIndex = index.parent();
//        selectedVirtualStorageParentID = parentIndex.sibling(parentIndex.row(), 3).data().toInt();
//        QString selectedVirtualStorageParentName = parentIndex.sibling(parentIndex.row(), 0).data().toString();

        //Physical Storage Tree
        if (selectedTreeType==tr("Location / Storage / Catatog")){
            //loadStorageTableToSelectionTreeModel();
        }
        //Virtual Storage Tree
        else if (selectedTreeType==tr("Virtual Storage / Catalog")){
            if (selectedDeviceType=="Storage"){
                //Empty
            }
            else if (selectedDeviceType=="VirtualStorage"){
                //Empty
            }
            else if (selectedDeviceType=="Catalog"){
                /*
                QPoint globalPos = ui->Filters_treeView_Devices->mapToGlobal(pos);
                QMenu virtualStorageContextMenu;

                QString virtualStorageName = selectedVirtualStorageName;

                QAction *menuVirtualStorageAction1 = new QAction(QIcon::fromTheme("document-new"), tr("Assign this catalog to a Virtual Storage device"), this);
                virtualStorageContextMenu.addAction(menuVirtualStorageAction1);

                connect(menuVirtualStorageAction1, &QAction::triggered, this, [this, virtualStorageName, index]() {
                    on_Filters_treeView_Devices_clicked(index);
                });

                virtualStorageContextMenu.exec(globalPos);
                ui->tabWidget->setCurrentIndex(5);
                */
            }
        }

        //Set actions for catalogs
        if(selectedVirtualStorageType=="Catalog"){

        }

    }
    //----------------------------------------------------------------------

//FILTERS / Connected drives -----------------------------------------------

    void MainWindow::on_Filters_treeView_Directory_clicked(const QModelIndex &index)
    {
        //Sends the selected folder in the tree for the Filter

        //Get the model/data from the tree
        QFileSystemModel* pathmodel = (QFileSystemModel*)ui->Create_treeView_Explorer->model();
        //get data from the selected file/directory
        QFileInfo fileInfo = pathmodel->fileInfo(index);
        //send the path to the line edit
        ui->Filters_lineEdit_SeletedDirectory->setText(fileInfo.filePath());
    }
    //----------------------------------------------------------------------
    void MainWindow::on_Filter_pushButton_PickPath_clicked()
    {
        //Pick a directory from a dialog window

        //Get current selected path as default path for the dialog window
        selectedConnectedDrivePath = ui->Filters_lineEdit_SeletedDirectory->text();

        //Open a dialog for the user to select the directory to be cataloged. Only show directories.
        QString dir = QFileDialog::getExistingDirectory(this, tr("Select the directory to be cataloged in this new catalog"),
                                                        selectedConnectedDrivePath,
                                                        QFileDialog::ShowDirsOnly
                                                            | QFileDialog::DontResolveSymlinks);
        //Send the selected directory to LE_NewCatalogPath (input line for the New Catalog Path)
        ui->Filters_lineEdit_SeletedDirectory->setText(dir);

        //Select this directory in the treeview.
        loadFileSystem(selectedConnectedDrivePath);
    }
    //----------------------------------------------------------------------

//FILTERS / data methods ---------------------------------------------------

    void MainWindow::resetSelection()
    {
        //reset selected values
        selectedDeviceType = tr("All");
        selectedDeviceName = tr("All");
        selectedDeviceID   = 0;
        selectedFilterStorageLocation = tr("All");
        selectedFilterStorageName     = tr("All");
        selectedFilterCatalogName     = tr("All");
        selectedFilterVirtualStorageName = tr("All");
        ui->Filters_label_DisplayLocation->setText(tr("All"));
        ui->Filters_label_DisplayStorage->setText(tr("All"));
        ui->Filters_label_DisplayCatalog->setText(tr("All"));
        ui->Filters_label_DisplayVirtualStorage->setText(tr("All"));
        selectedCatalog->setName(tr(""));
        selectedCatalog->loadCatalogMetaData();
        refreshStorageSelectionList(selectedFilterStorageLocation);
        refreshCatalogSelectionList(selectedFilterStorageLocation,selectedFilterStorageName,selectedFilterVirtualStorageName);
        ui->Filter_pushButton_Explore->setEnabled(false);
        ui->Filter_pushButton_Update->setEnabled(false);
        ui->Filter_comboBox_TreeType->setCurrentText(selectedTreeType);
        ui->Virtual_label_SelectedCatalogDisplay->setText("");

        //reset device tree
        setTreeExpandState(false);

        if (ui->Filter_comboBox_TreeType->currentText()==tr("Location / Storage / Catatog")){
            loadStorageTableToSelectionTreeModel();
        }
        else if (ui->Filter_comboBox_TreeType->currentText()==tr("Virtual Storage / Catalog")){
            loadVirtualStorageFileToTable();
            loadVirtualStorageCatalogFileToTable();
            loadVirtualStorageTableToSelectionTreeModel();
        }
        filterFromSelectedDevices();

        QSettings settings(settingsFilePath, QSettings:: IniFormat);
        settings.setValue("Selection/SelectedDeviceType", tr("All"));
        settings.setValue("Selection/SelectedDeviceName", tr("All"));
        settings.setValue("Selection/SelectedDeviceID", 0);
        refreshDifferencesCatalogSelection();

    }
    //----------------------------------------------------------------------
    void MainWindow::filterFromSelectedDevices()
    {
        if(selectedDeviceType=="Location"){
            ui->Filter_pushButton_Explore->setEnabled(false);
            ui->Filter_pushButton_Update->setEnabled(false);

            selectedFilterStorageLocation = selectedDeviceName;
            selectedFilterStorageName = tr("All");
            selectedFilterCatalogName = tr("All");
            selectedFilterVirtualStorageName = tr("All");

            refreshCatalogSelectionList(selectedFilterStorageLocation, selectedFilterStorageName, selectedFilterVirtualStorageName);
            refreshStorageSelectionList(selectedFilterStorageLocation);
        }
        else if (selectedDeviceType=="Storage"){
            ui->Filter_pushButton_Explore->setEnabled(false);
            ui->Filter_pushButton_Update->setEnabled(false);

            selectedFilterStorageLocation = tr("All");
            selectedFilterStorageName = selectedDeviceName;
            selectedFilterCatalogName = tr("All");
            selectedFilterVirtualStorageName = tr("All");

            refreshCatalogSelectionList(selectedFilterStorageLocation, selectedFilterStorageName, selectedFilterVirtualStorageName);
            refreshStorageSelectionList(selectedFilterStorageLocation);
        }
        else if (selectedDeviceType=="Catalog"){
            ui->Filter_pushButton_Explore->setEnabled(true);
            ui->Filter_pushButton_Update->setEnabled(true);

            selectedCatalog->setName(selectedDeviceName);
            selectedCatalog->loadCatalogMetaData();

            selectedFilterStorageLocation = tr("All");
            selectedFilterStorageName = tr("All");
            selectedFilterVirtualStorageName = tr("All");
            selectedFilterCatalogName = selectedCatalog->name;
        }
        else if (selectedDeviceType=="VirtualStorage"){
            ui->Filter_pushButton_Explore->setEnabled(false);
            ui->Filter_pushButton_Update->setEnabled(false);

            selectedFilterStorageLocation = tr("All");
            selectedFilterStorageName = tr("All");
            selectedFilterCatalogName = tr("All");
            selectedFilterVirtualStorageName = selectedDeviceName;

            refreshCatalogSelectionList(selectedFilterStorageLocation, selectedFilterStorageName, selectedFilterVirtualStorageName);
            refreshStorageSelectionList(selectedFilterStorageLocation);
        }

        //Display selection values and save them
        ui->Filters_label_DisplayLocation->setText(selectedFilterStorageLocation);
        ui->Filters_label_DisplayStorage->setText(selectedFilterStorageName);
        ui->Filters_label_DisplayCatalog->setText(selectedFilterCatalogName);
        ui->Filters_label_DisplayVirtualStorage->setText(selectedFilterVirtualStorageName);

        //Load matching Catalogs, Storage, and Statistics
        //Load matching Catalogs
        loadCatalogsTableToModel();

        //Load matching Storage
        loadStorageTableToModel();
        updateStorageSelectionStatistics();

        //Load matching Statistics
        if(databaseMode=="Memory"){
            loadStatisticsCatalogFileToTable();
            loadStatisticsStorageFileToTable();
        }
        loadStatisticsChart();
    }
    //----------------------------------------------------------------------
    void MainWindow::setTreeExpandState(bool toggle)
    {
        //deviceTreeExpandState values:  collapseAll or 2 =collapse / 0=exp.level0 / 1=exp.level1
        QString iconName = ui->Filters_pushButton_TreeExpandCollapse->icon().name();
        QSettings settings(settingsFilePath, QSettings:: IniFormat);

        if (toggle==true){

            if ( deviceTreeExpandState == 2 ){
                //collapsed > expand first level
                ui->Filters_pushButton_TreeExpandCollapse->setIcon(QIcon::fromTheme("expand-all"));
                deviceTreeExpandState = 0;
                ui->Filters_treeView_Devices->expandToDepth(0);
                settings.setValue("Settings/deviceTreeExpandState", deviceTreeExpandState);
            }
            else if ( deviceTreeExpandState == 0 ){
                //expanded first level > expand to second level
                ui->Filters_pushButton_TreeExpandCollapse->setIcon(QIcon::fromTheme("collapse-all"));
                deviceTreeExpandState = 1;
                ui->Filters_treeView_Devices->expandToDepth(1);
                settings.setValue("Settings/deviceTreeExpandState", deviceTreeExpandState);
            }
            else if ( deviceTreeExpandState == 1 ){
                //expanded second level > collapse
                ui->Filters_pushButton_TreeExpandCollapse->setIcon(QIcon::fromTheme("expand-all"));
                deviceTreeExpandState = 2;
                ui->Filters_treeView_Devices->collapseAll();
                settings.setValue("Settings/deviceTreeExpandState", deviceTreeExpandState);
            }
        }
        else
        {
            if ( deviceTreeExpandState == 0 ){
                ui->Filters_pushButton_TreeExpandCollapse->setIcon(QIcon::fromTheme("expand-all"));
                ui->Filters_treeView_Devices->collapseAll();
                ui->Filters_treeView_Devices->expandToDepth(deviceTreeExpandState);
            }
            else if ( deviceTreeExpandState == 1 ){
                ui->Filters_pushButton_TreeExpandCollapse->setIcon(QIcon::fromTheme("collapse-all"));
                ui->Filters_treeView_Devices->collapseAll();
                ui->Filters_treeView_Devices->expandToDepth(deviceTreeExpandState);
            }
            else{
                ui->Filters_pushButton_TreeExpandCollapse->setIcon(QIcon::fromTheme("expand-all"));
                ui->Filters_treeView_Devices->collapseAll();
            }
        }
    }
    //----------------------------------------------------------------------
    void MainWindow::loadStorageTableToSelectionTreeModel()
    {
        const QStringList headers({tr("Location / Storage / Catalog"),tr("Type"),tr("Empty")});
        StorageTreeModel *storageTreeModel = new StorageTreeModel(headers);

        DeviceTreeView *deviceTreeProxyModel = new DeviceTreeView();
        deviceTreeProxyModel->setSourceModel(storageTreeModel);

        //LoadModel
        ui->Filters_treeView_Devices->setModel(deviceTreeProxyModel);
        ui->Filters_treeView_Devices->header()->setSectionResizeMode(QHeaderView::ResizeToContents);
        ui->Filters_treeView_Devices->sortByColumn(0,Qt::AscendingOrder);
        ui->Filters_treeView_Devices->hideColumn(1);
        ui->Filters_treeView_Devices->hideColumn(2);
        ui->Filters_treeView_Devices->setColumnWidth(2,0);
        ui->Filters_treeView_Devices->collapseAll();
        ui->Filters_treeView_Devices->header()->hide();

        //Restore Expand or Collapse Device Tree
        setTreeExpandState(false);
    }
    //--------------------------------------------------------------------------
    void MainWindow::loadVirtualStorageTableToSelectionTreeModel()
    {
        //Prepare query for catalogs
        QSqlQuery queryCatalog;
        QString queryCatalogSQL;
        queryCatalogSQL = QLatin1String(R"(
                        SELECT vsc.catalog_name,'Catalog', c.catalog_source_path_is_active
                        FROM  virtual_storage_catalog vsc
                        INNER JOIN catalog c ON vsc.catalog_name = c.catalog_name
                        WHERE vsc.virtual_storage_id =:virtual_storage_id
                        ORDER BY vsc.catalog_name
                    )");
        queryCatalog.prepare(queryCatalogSQL);

        // Create the tree model
        QStandardItemModel *model = new QStandardItemModel();
        model->setHorizontalHeaderLabels({ tr("Name"), tr("Device Type"), tr("ID"), tr("Parent ID") });

        // Retrieve data from the database
        QSqlQuery query;
        QString querySQL;
        querySQL = QLatin1String(R"(
                    SELECT  virtual_storage_id,
                            virtual_storage_parent_id,
                            virtual_storage_name
                    FROM  virtual_storage
                )");
        query.prepare(querySQL);
        query.exec();

        //Populate Model
        // Create a map to store items by ID for easy access
        QMap<int, QStandardItem*> itemMap;

        while (query.next()) {
            int id = query.value(0).toInt();
            int parentId = query.value(1).toInt();
            QString name = query.value(2).toString();

            // Create the item for this row
            QList<QStandardItem*> rowItems;
            rowItems << new QStandardItem(name);
            rowItems << new QStandardItem("VirtualStorage");
            rowItems << new QStandardItem(QString::number(id));
            rowItems << new QStandardItem(QString::number(parentId));

            QStandardItem* item = rowItems.at(0); // Get the item representing the name
            QStandardItem* parentItem = itemMap.value(parentId);

            if (parentId == 0) {
                model->appendRow(rowItems); // Add top-level items directly to the model
            }
            else{
                if (parentItem) {
                    parentItem->appendRow(rowItems); // Append the row to the parent item
                }
                else{
                    qDebug() << "Parent item not found for ID:" << id;
                    continue; // Skip this row and proceed to the next one
                }
            }

            // Store the item in the map for future reference
            itemMap.insert(id, item);

            //Add Catalogs
            queryCatalog.bindValue(":virtual_storage_id",id);
            queryCatalog.exec();
            while(queryCatalog.next()){
                //Get data
                QList<QStandardItem*> rowCatalogItems;
                rowCatalogItems << new QStandardItem(queryCatalog.value(0).toString());
                rowCatalogItems << new QStandardItem(queryCatalog.value(1).toString());
                rowCatalogItems << new QStandardItem(queryCatalog.value(2).toString());
                rowCatalogItems << new QStandardItem("");

                //Add items
                item->appendRow(rowCatalogItems);
            }
        }

        //Load Model to treeview
        DeviceTreeView *deviceTreeProxyModel = new DeviceTreeView();
        deviceTreeProxyModel->setSourceModel(model);
        ui->Filters_treeView_Devices->setModel(deviceTreeProxyModel);
        ui->Filters_treeView_Devices->header()->setSectionResizeMode(QHeaderView::ResizeToContents);
        ui->Filters_treeView_Devices->sortByColumn(0,Qt::AscendingOrder);
        ui->Filters_treeView_Devices->hideColumn(1);
        ui->Filters_treeView_Devices->hideColumn(2);
        ui->Filters_treeView_Devices->hideColumn(3);
        ui->Filters_treeView_Devices->setColumnWidth(2,0);
        ui->Filters_treeView_Devices->collapseAll();
        ui->Filters_treeView_Devices->header()->hide();

        //Restore Expand or Collapse Device Tree
        setTreeExpandState(false);

        ui->Filters_treeView_Devices->setModel(deviceTreeProxyModel);
        ui->Filters_treeView_Devices->expandAll();       
    }
    //----------------------------------------------------------------------
