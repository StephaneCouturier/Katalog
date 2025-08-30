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
// Purpose:     methods for the SELECTION panel
// Description: https://stephanecouturier.github.io/Katalog/docs/Features/Selection
// Author:      Stephane Couturier
/////////////////////////////////////////////////////////////////////////////
*/

#include "mainwindow.h"
#include "ui_mainwindow.h"
#include "mainwindow_ui_wrapper_device.h"

//FILTERS / Global ---------------------------------------------------------

    void MainWindow::on_Filters_pushButton_Filters_Hide_clicked()
    {
        ui->splitter_widget_Filters->setHidden(true);
        ui->main_widget_ShowFilters->setHidden(false);

        QSettings settings(collection->settingsFilePath, QSettings:: IniFormat);
        settings.setValue("Settings/ShowHideFilters", "go-next");
    }
    //----------------------------------------------------------------------
    void MainWindow::on_Filters_pushButton_Filters_Show_clicked()
    {
        ui->splitter_widget_Filters->setHidden(false);
        ui->main_widget_ShowFilters->setHidden(true);

        QSettings settings(collection->settingsFilePath, QSettings:: IniFormat);
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
        loadCollection();
    }
    //----------------------------------------------------------------------
    void MainWindow::on_Filters_checkBox_SearchInCatalogs_toggled(bool checked)
    {
        QSettings settings(collection->settingsFilePath, QSettings:: IniFormat);
        settings.setValue("Search/searchInFileCatalogsChecked", checked);

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

        ui->Search_checkBox_Differences->setEnabled(true);
    }
    //----------------------------------------------------------------------
    void MainWindow::on_Filters_checkBox_SearchInConnectedDrives_toggled(bool checked)
    {
        QSettings settings(collection->settingsFilePath, QSettings:: IniFormat);
        settings.setValue("Search/searchInConnectedDriveChecked", checked);

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

        //Hide elements for search on Differences
        ui->Search_checkBox_Differences->setChecked(false);
        ui->Search_checkBox_DifferencesName->setChecked(false);
        ui->Search_checkBox_DifferencesSize->setChecked(false);
        ui->Search_checkBox_DifferencesDateModified->setChecked(false);
        ui->Search_checkBox_Differences->setEnabled(false);
    }
    //----------------------------------------------------------------------

//FILTERS / Device tree ----------------------------------------------------

    // Top buttons ---------------------------------------------------------
    //----------------------------------------------------------------------
    void MainWindow::on_Filters_pushButton_TreeCollapse_clicked()
    {
        changeFiltersTreeExpandLevel(-1); //Collapse tree 1 level down
    }
    //----------------------------------------------------------------------
    void MainWindow::on_Filters_pushButton_TreeExpand_clicked()
    {
        changeFiltersTreeExpandLevel(1); //Expand tree 1 level down
    }
    //----------------------------------------------------------------------

    // Device tree ---------------------------------------------------------
    void MainWindow::on_Filters_treeView_Devices_clicked(const QModelIndex &index)
    {//Get selected device data
        //Load selected device data
        selectedDevice->ID = ui->Filters_treeView_Devices->model()->index(index.row(), 3, index.parent() ).data().toInt();
        selectedDevice->loadDevice("defaultConnection");
        QSettings settings(collection->settingsFilePath, QSettings:: IniFormat);
        settings.setValue("Selection/SelectedDeviceID", selectedDevice->ID);

        filterFromSelectedDevice();

        refreshDifferencesCatalogSelection();

        ui->Catalogs_pushButton_UpdateActiveDevice->setEnabled(false);

        //Use device's path as default to create a new catalog
        ui->Create_lineEdit_NewCatalogPath->setText(selectedDevice->path);
    }
    //----------------------------------------------------------------------
    void MainWindow::on_Filters_treeView_Devices_customContextMenuRequested(const QPoint &pos)
    {
        //Get selection data
        QModelIndex index=ui->Filters_treeView_Devices->currentIndex();
        selectedDevice->ID = ui->Filters_treeView_Devices->model()->index(index.row(), 3, index.parent() ).data().toInt();
        selectedDevice->loadDevice("defaultConnection");

        on_Filters_treeView_Devices_clicked(index);

        if (selectedDevice->type=="Storage"){
            QPoint globalPos = ui->Filters_treeView_Devices->mapToGlobal(pos);
            QMenu deviceContextMenu;

            QString deviceName = selectedDevice->name;

            if(ui->tabWidget->currentIndex() != 0){
                QAction *menuDeviceAction1 = new QAction(QIcon::fromTheme("edit-find"), tr("Search"), this);
                deviceContextMenu.addAction(menuDeviceAction1);

                connect(menuDeviceAction1, &QAction::triggered, this, [this, deviceName]() {
                    ui->tabWidget->setCurrentIndex(0);
                });
            }

            // Storage update action
            QAction *menuDeviceAction3 = new QAction(QIcon::fromTheme("media-playlist-repeat"), tr("Update"), this);
            deviceContextMenu.addAction(menuDeviceAction3);
            connect(menuDeviceAction3, &QAction::triggered, this, [this, deviceName]() {
                for (int i = 0; i < selectedDevice->subDevices.size(); ++i) {
                    qDebug() << "  Existing child" << i << ":" << selectedDevice->subDevices[i].name << "ID:" << selectedDevice->subDevices[i].ID;
                }
                for (int i = 0; i < selectedDevice->deviceIDList.size(); ++i) {
                    qDebug() << "  Child ID" << i << ":" << selectedDevice->deviceIDList[i];
                }

                // Use unified DeviceUpdateManager for Storage devices
                deviceUpdateManager->updateDeviceHierarchy(
                    selectedDevice,
                    collection->databaseMode,
                    collection->folder
                    );
                qDebug() << "Device operation started using DeviceUpdateManager";
            });

            deviceContextMenu.exec(globalPos);
        }
        else if (selectedDevice->type=="Virtual"){
            QPoint globalPos = ui->Filters_treeView_Devices->mapToGlobal(pos);
            QMenu deviceContextMenu;

            QString deviceName = selectedDevice->name;

            if(ui->tabWidget->currentIndex() != 0){
                QAction *menuDeviceAction1 = new QAction(QIcon::fromTheme("edit-find"), tr("Search"), this);
                deviceContextMenu.addAction(menuDeviceAction1);

                connect(menuDeviceAction1, &QAction::triggered, this, [this, deviceName]() {
                    ui->tabWidget->setCurrentIndex(0);
                });
            }

            QAction *menuDeviceAction3 = new QAction(QIcon::fromTheme("media-playlist-repeat"), tr("Update"), this);
            deviceContextMenu.addAction(menuDeviceAction3);
            connect(menuDeviceAction3, &QAction::triggered, this, [this, deviceName]() {
                if (!deviceUpdateManager) {
                    setupDeviceUpdateManager();
                }

                // Check if already running
                if (deviceUpdateManager->operationRunning()) {
                    QMessageBox::information(this, "Katalog", tr("A device operation is already running."));
                    return;
                }

                // Set UI state for catalog operation
                setCatalogUpdateUIState(true);

                // This ensures consistent behavior across all update paths
                deviceUpdateManager->updateDeviceHierarchy(selectedDevice,
                                                           collection->databaseMode,
                                                           collection->folder);
            });

            deviceContextMenu.exec(globalPos);
        }
        else if (selectedDevice->type=="Catalog"){
            QPoint globalPos = ui->Filters_treeView_Devices->mapToGlobal(pos);
            QMenu deviceContextMenu;

            QString deviceName = selectedDevice->name;

            if(ui->tabWidget->currentIndex() != 0){
                QAction *menuDeviceAction1 = new QAction(QIcon::fromTheme("edit-find"), tr("Search"), this);
                deviceContextMenu.addAction(menuDeviceAction1);

                connect(menuDeviceAction1, &QAction::triggered, this, [this, deviceName]() {
                    ui->tabWidget->setCurrentIndex(0);
                });
            }

            QAction *menuDeviceAction3 = new QAction(QIcon::fromTheme("media-playlist-repeat"), tr("Update"), this);
            deviceContextMenu.addAction(menuDeviceAction3);
            connect(menuDeviceAction3, &QAction::triggered, this, [this, deviceName]() {
                qDebug() << "=== Filters Context Menu Update (DeviceUpdateManager) CATALOG ===";

                if (!selectedDevice || selectedDevice->type != "Catalog") {
                    qDebug() << "Filters context menu update - invalid device";
                    return;
                }

                if (!selectedDevice->active) {
                    QMessageBox::information(this, "Katalog", tr("The catalog is not active (path not available)."));
                    return;
                }

                if (!deviceUpdateManager) {
                    qDebug() << "DeviceUpdateManager not available - setting up now";
                    setupDeviceUpdateManager();
                }

                // Check if already running
                if (deviceUpdateManager->operationRunning()) {
                    QMessageBox::information(this, "Katalog", tr("A device operation is already running."));
                    return;
                }

                qDebug() << "Filters context menu catalog update for:" << selectedDevice->name;

                // Set UI state for catalog operation
                setCatalogUpdateUIState(true);

                // FIXED: Use DeviceUpdateManager instead of old CatalogManager
                // This ensures consistent behavior across all update paths
                deviceUpdateManager->updateDeviceHierarchy(selectedDevice,
                                                           collection->databaseMode,
                                                           collection->folder);

                qDebug() << "Filters context menu - DeviceUpdateManager operation started";
            });

            QAction *menuDeviceAction2 = new QAction(QIcon::fromTheme("document-new"), tr("Explore"), this);
            deviceContextMenu.addAction(menuDeviceAction2);
            connect(menuDeviceAction2, &QAction::triggered, this, [this, deviceName]() {
                exploreDevice->ID = selectedDevice->ID;
                exploreDevice->loadDevice("defaultConnection");

                exploreSelectedFolderFullPath = exploreDevice->path;
                exploreSelectedDirectoryName  = exploreDevice->path;

                openCatalogToExplore();

                //Go to explore tab
                ui->tabWidget->setCurrentIndex(2);
            });

            deviceContextMenu.exec(globalPos);
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
        selectedDevice->type = "All";
        selectedDevice->ID = 0;
        selectedDevice->loadDevice("defaultConnection");

        //Reset displayed values
        ui->Filters_label_DisplayStorage->setText(tr("All"));
        ui->Filters_label_DisplayCatalog->setText(tr("All"));
        ui->Filters_label_DisplayDevice->setText(tr("All"));

        //Reset device tree
        collection->loadDeviceFileToTable();
        loadDevicesTreeToModel("Filters");

        filterFromSelectedDevice();

        QSettings settings(collection->settingsFilePath, QSettings:: IniFormat);
        settings.setValue("Selection/SelectedDeviceType", tr("All"));
        settings.setValue("Selection/SelectedDeviceName", tr("All"));
        settings.setValue("Selection/SelectedDeviceID", 0);
        refreshDifferencesCatalogSelection();
    }
    //----------------------------------------------------------------------
    void MainWindow::filterFromSelectedDevice()
    {
        //Display selected values
        displaySelectedDeviceName();

        //Load matching Catalogs, Storage, and Statistics
        loadDevicesView("Filters");
        updateCatalogsScreenStatistics();
        updateStorageSelectionStatistics();
        loadStorageList();

        //Statistics
        loadStatisticsChart();
    }
    //----------------------------------------------------------------------
    void MainWindow::changeFiltersTreeExpandLevel(int levelChange)
    {
        filtersTreeExpandState = filtersTreeExpandState + levelChange;

        // Get max levels once and cache it
        static int maxTreeLevels = -1;
        if (maxTreeLevels == -1) {
            QSqlQuery query(QSqlDatabase::database("defaultConnection"));
            QString querySQL = QLatin1String(R"(
                    WITH RECURSIVE device_tree AS (
                      SELECT device_id, device_parent_id, 0 AS level
                      FROM device
                      WHERE device_parent_id = 0
                      UNION ALL
                      SELECT child.device_id, child.device_parent_id, parent.level + 1 AS level
                      FROM device_tree parent
                      JOIN device child ON child.device_parent_id = parent.device_id
                    )
                    SELECT MAX(level) AS total_levels FROM device_tree;
        )");
            query.prepare(querySQL);
            query.exec();
            if (query.next()) {
                maxTreeLevels = query.value(0).toInt();
            }
            if (maxTreeLevels == 0) maxTreeLevels = 3; // fallback
        }

        // Bounds checking
        if (filtersTreeExpandState < -1) filtersTreeExpandState = -1; // collapse all
        if (filtersTreeExpandState >= maxTreeLevels) filtersTreeExpandState = maxTreeLevels - 1; // max level

        QSettings settings(collection->settingsFilePath, QSettings::IniFormat);
        settings.setValue("Selection/filtersTreeExpandState", filtersTreeExpandState);

        if (filtersTreeExpandState == -1) {
            ui->Filters_treeView_Devices->collapseAll();
        } else {
            ui->Filters_treeView_Devices->expandToDepth(filtersTreeExpandState);
        }

        // Enable/disable buttons based on current state
        ui->Filters_pushButton_TreeCollapse->setEnabled(filtersTreeExpandState > -1);
        ui->Filters_pushButton_TreeExpand->setEnabled(filtersTreeExpandState < maxTreeLevels - 1);

        // Set focus to tree view when buttons are disabled to avoid focus glow
        if (!ui->Filters_pushButton_TreeExpand->isEnabled()) {
            ui->Filters_treeView_Devices->setFocus();
        }
    }
    //----------------------------------------------------------------------
    void MainWindow::displaySelectedDeviceName(){
        if ( selectedDevice->type == "" or selectedDevice->type == tr("All") )
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
