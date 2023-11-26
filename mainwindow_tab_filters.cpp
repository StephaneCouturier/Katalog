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

//#include "storagetreemodel.h"
//#include "devicetreeview.h"
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
        createStorageFile();
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
        if (selectedDevice->type=="Catalog"){
            selectedCatalog->name = selectedDevice->name;
            selectedCatalog->loadCatalog();

            openCatalogToExplore();

            //Go to explore tab
            ui->tabWidget->setCurrentIndex(2);
        }
    }
    //----------------------------------------------------------------------
    void MainWindow::on_Filter_pushButton_Update_clicked()
    {
        //reloads catalog to explore at root level
        if (selectedDevice->type=="Catalog"){
            skipCatalogUpdateSummary= false;
            requestSource ="update";
            updateSingleCatalog(selectedCatalog, true);
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
    {//Get selected device data

        //Load selected device data
        selectedDevice->ID = ui->Filters_treeView_Devices->model()->index(index.row(), 3, index.parent() ).data().toInt();;
        selectedDevice->loadDevice();

        //Adapt UI based on device type
        if (selectedDevice->type=="Storage"){
            selectedStorage->ID = selectedDevice->externalID;
            selectedStorage->loadStorage();
            ui->Devices_pushButton_AssignCatalog->setEnabled(false);
            ui->Devices_pushButton_AssignStorage->setEnabled(true);
            ui->Devices_label_SelectedCatalogDisplay->setText("");
            ui->Devices_label_SelectedStorageDisplay->setText(selectedDevice->name);
        }
        else if (selectedDevice->type=="Virtual"){
            ui->Devices_pushButton_AssignCatalog->setEnabled(false);
            ui->Devices_label_SelectedCatalogDisplay->setText("");
        }
        else if (selectedDevice->type=="Catalog"){
            selectedCatalog->name = selectedDevice->name;
            selectedCatalog->loadCatalog();
            ui->Devices_pushButton_AssignCatalog->setEnabled(true);
            ui->Devices_pushButton_AssignStorage->setEnabled(false);
            ui->Devices_label_SelectedStorageDisplay->setText("");
            ui->Devices_label_SelectedCatalogDisplay->setText(selectedDevice->name);
            if(selectedDevice->name!=""){
                ui->Devices_pushButton_AssignCatalog->setEnabled(true);
            }
            ui->Devices_label_SelectedCatalogDisplay->setText(selectedDevice->name);
        }

        QSettings settings(settingsFilePath, QSettings:: IniFormat);
        settings.setValue("Selection/SelectedDeviceID",   selectedDevice->ID);

        filterFromSelectedDevices();

        refreshDifferencesCatalogSelection();
    }
    //----------------------------------------------------------------------
    void MainWindow::on_Filters_treeView_Devices_customContextMenuRequested(const QPoint &pos)
    {
        //Get selection data
        QModelIndex index=ui->Filters_treeView_Devices->currentIndex();
        selectedDevice->ID = ui->Filters_treeView_Devices->model()->index(index.row(), 3, index.parent() ).data().toInt();
        selectedDevice->loadDevice();

        if (selectedDevice->type=="Storage"){
            //Empty
        }
        else if (selectedDevice->type=="Virtual"){
            //Empty
        }
        else if (selectedDevice->type=="Catalog"){
            /*
            QPoint globalPos = ui->Filters_treeView_Devices->mapToGlobal(pos);
            QMenu deviceContextMenu;

            QString deviceName = selectedDeviceName;

            QAction *menuDeviceAction1 = new QAction(QIcon::fromTheme("document-new"), tr("Assign this catalog to a Virtual Storage device"), this);
            deviceContextMenu.addAction(menuDeviceAction1);

            connect(menuDeviceAction1, &QAction::triggered, this, [this, deviceName, index]() {
                on_Filters_treeView_Devices_clicked(index);
            });

            deviceContextMenu.exec(globalPos);
            ui->tabWidget->setCurrentIndex(5);
            */
        }


        //Set actions for catalogs
        if(selectedDevice->type=="Catalog"){

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
        //Reset selected values
        selectedDevice = new Device();
        selectedDevice->type = tr("All");
        selectedStorage = new Storage();
        selectedCatalog = new Catalog();

        //Reset displayed values
        ui->Filters_label_DisplayStorage->setText(tr("All"));
        ui->Filters_label_DisplayCatalog->setText(tr("All"));
        ui->Filters_label_DisplayDevice->setText(tr("All"));
        ui->Filter_pushButton_Explore->setEnabled(false);
        ui->Filter_pushButton_Update->setEnabled(false);
        ui->Devices_label_SelectedCatalogDisplay->setText("");

        //Reload data
        loadCatalogsTableToModel();

        //Reset device tree
        setTreeExpandState(false);
        loadDeviceFileToTable();
        loadDeviceTableToTreeModel();

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
        if (selectedDevice->type=="Storage"){
            ui->Filter_pushButton_Explore->setEnabled(false);
            ui->Filter_pushButton_Update->setEnabled(false);
        }
        else if (selectedDevice->type=="Catalog"){
            ui->Filter_pushButton_Explore->setEnabled(true);
            ui->Filter_pushButton_Update->setEnabled(true);
            selectedCatalog->name = selectedDevice->name;
            selectedCatalog->loadCatalog();
        }
        else if (selectedDevice->type=="Virtual"){
            ui->Filter_pushButton_Explore->setEnabled(false);
            ui->Filter_pushButton_Update->setEnabled(false);
        }

        //Display selection values
        displaySelectedDeviceName();

        //Load matching Catalogs, Storage, and Statistics
            //Catalogs
            loadCatalogsTableToModel();

            //Storage
            loadStorageTableToModel();
            updateCatalogsScreenStatistics();
            updateStorageSelectionStatistics();

            //Statistics
            loadStatisticsCatalogFileToTable();
            loadStatisticsStorageFileToTable();
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
    void MainWindow::displaySelectedDeviceName(){
        if ( selectedDevice->type == "" )
        {
            ui->Filters_label_DisplayDevice->setText(tr("All"));
            ui->Filters_label_DisplayStorage->setText(tr("All"));
            ui->Filters_label_DisplayCatalog->setText(tr("All"));
        }
        else if ( selectedDevice->type == "Storage" ){
            ui->Filters_label_DisplayStorage->setText(selectedDevice->name);
            ui->Filters_label_DisplayDevice->setText(tr("All"));
            ui->Filters_label_DisplayCatalog->setText(tr("All"));
        }
        else if ( selectedDevice->type == "Catalog" ){
            ui->Filters_label_DisplayCatalog->setText(selectedDevice->name);
            ui->Filters_label_DisplayDevice->setText(tr("All"));
            ui->Filters_label_DisplayStorage->setText(tr("All"));
        }
        else if ( selectedDevice->type == "Virtual" ){
            ui->Filters_label_DisplayDevice->setText(selectedDevice->name);
            ui->Filters_label_DisplayStorage->setText(tr("All"));
            ui->Filters_label_DisplayCatalog->setText(tr("All"));
        }
    }
