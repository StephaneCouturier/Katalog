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
        createStorageList();
        generateCollectionFilesPaths();
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
            activeCatalog->setName(selectedDeviceName);
            activeCatalog->loadCatalogMetaData();

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
            updateSingleCatalog(selectedCatalog);
        }
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
            selectedDeviceID   = ui->Filters_treeView_Devices->model()->index(index.row(), 2, index.parent() ).data().toInt();
            selectedStorage->setID(selectedDeviceID);
            selectedStorage->loadStorageMetaData();
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
        selectedFilterStorageLocation = tr("All");
        selectedFilterStorageName     = tr("All");
        selectedFilterCatalogName     = tr("All");
        ui->Filters_label_DisplayLocation->setText(tr("All"));
        ui->Filters_label_DisplayStorage->setText(tr("All"));
        ui->Filters_label_DisplayCatalog->setText(tr("All"));
        activeCatalog->setName(tr(""));
        activeCatalog->loadCatalogMetaData();
        refreshStorageSelectionList(selectedFilterStorageLocation);
        refreshCatalogSelectionList(selectedFilterStorageLocation,selectedFilterStorageName);
        ui->Filter_pushButton_Explore->setEnabled(false);
        ui->Filter_pushButton_Update->setEnabled(false);

        //reset device tree
        setTreeExpandState(false);

        loadStorageTableToSelectionTreeModel();
        filterFromSelectedDevices();

        QSettings settings(settingsFilePath, QSettings:: IniFormat);
        settings.setValue("LastSearch/SelectedSearchLocation", selectedFilterStorageLocation);
        settings.setValue("LastSearch/SelectedSearchStorage",  selectedFilterStorageName);
        settings.setValue("LastSearch/SelectedSearchCatalog",  selectedFilterCatalogName);
        settings.setValue("Selection/SelectedDeviceType", tr("All"));
        settings.setValue("Selection/SelectedDeviceName", tr("All"));
        refreshDifferencesCatalogSelection();

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
    //----------------------------------------------------------------------
    void MainWindow::filterFromSelectedDevices()
    {
        if(selectedDeviceType=="Location"){
            ui->Filter_pushButton_Explore->setEnabled(false);
            ui->Filter_pushButton_Update->setEnabled(false);

            selectedFilterStorageLocation = selectedDeviceName;
            selectedFilterStorageName = tr("All");
            selectedFilterCatalogName = tr("All");

            refreshStorageSelectionList(selectedFilterStorageLocation);
            refreshCatalogSelectionList(selectedFilterStorageLocation,selectedFilterStorageName);
        }
        else if (selectedDeviceType=="Storage"){
            ui->Filter_pushButton_Explore->setEnabled(false);
            ui->Filter_pushButton_Update->setEnabled(false);

            selectedFilterStorageLocation = tr("All");
            selectedFilterStorageName = selectedDeviceName;
            selectedFilterCatalogName = tr("All");

            refreshStorageSelectionList(selectedFilterStorageLocation);
            refreshCatalogSelectionList(selectedFilterStorageLocation,selectedFilterStorageName);
        }
        else if (selectedDeviceType=="Catalog"){
            ui->Filter_pushButton_Explore->setEnabled(true);
            ui->Filter_pushButton_Update->setEnabled(true);

            activeCatalog->setName(selectedDeviceName);
            activeCatalog->loadCatalogMetaData();

            selectedFilterStorageLocation = tr("All");
            selectedFilterStorageName = tr("All");
            selectedFilterCatalogName = activeCatalog->name;
        }

        //Display selection values and save them
        ui->Filters_label_DisplayLocation->setText(selectedFilterStorageLocation);
        ui->Filters_label_DisplayStorage->setText(selectedFilterStorageName);
        ui->Filters_label_DisplayCatalog->setText(selectedFilterCatalogName);

        QSettings settings(settingsFilePath, QSettings:: IniFormat);
        settings.setValue("LastSearch/SelectedSearchLocation", selectedFilterStorageLocation);
        settings.setValue("LastSearch/SelectedSearchStorage",  selectedFilterStorageName);
        settings.setValue("LastSearch/SelectedSearchCatalog",  selectedFilterCatalogName);

        //Load matching Catalogs, Storage, Statistics
        //Load matching Catalogs
        loadCatalogsTableToModel();

        //Load matching Storage
        loadStorageTableToModel();
        updateStorageSelectionStatistics();

        //Load matching Statistics
        loadStatisticsData();
        loadStatisticsChart();
    }
    //----------------------------------------------------------------------
