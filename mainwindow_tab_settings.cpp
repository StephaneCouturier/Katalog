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
// File Name:   mainwindow_tab_settings.cpp
// Purpose:     methods for the Fitlers and Settings panel
// Description:
// Author:      Stephane Couturier
/////////////////////////////////////////////////////////////////////////////
*/

#include "mainwindow.h"
#include "ui_mainwindow.h"

#include "storagetreemodel.h"
#include "storageview.h"
#include "devicetreeview.h"
#include "catalog.h"

#include <QSettings>
#include <QDesktopServices>
#include <QFileDialog>

//ALL/GLOBAL -----------------------------------------------------------------
    void MainWindow::on_splitter_splitterMoved()
    {
        QSettings settings(settingsFilePath, QSettings:: IniFormat);

        QSize widget1Size = ui->splitter_widget_Filters->size();
        QSize widget2Size = ui->splitter_widget_TabWidget->size();

        settings.setValue("Settings/SplitterWidget1Size", widget1Size);
        settings.setValue("Settings/SplitterWidget2Size", widget2Size);

    }
    //----------------------------------------------------------------------
    void MainWindow::on_tabWidget_currentChanged(int index)
    {
        selectedTab = index;
        QSettings settings(settingsFilePath, QSettings:: IniFormat);
        settings.setValue("Settings/selectedTab", ui->tabWidget->currentIndex());
    }
    //----------------------------------------------------------------------
    void MainWindow::on_Filters_tabWidget_currentChanged(int index)
    {
        int selectedFiltersTab = index;
        QSettings settings(settingsFilePath, QSettings:: IniFormat);
        settings.setValue("Settings/selectedFiltersTab", selectedFiltersTab);
    }
    //----------------------------------------------------------------------
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

//FILTERS -------------------------------------------------------------
    void MainWindow::on_Filters_treeView_Devices_clicked(const QModelIndex &index)
    {
        selectedDeviceName = ui->Filters_treeView_Devices->model()->index(index.row(), 0, index.parent() ).data().toString();
        selectedDeviceType = ui->Filters_treeView_Devices->model()->index(index.row(), 1, index.parent() ).data().toString();

        QSettings settings(settingsFilePath, QSettings:: IniFormat);
        settings.setValue("Selection/SelectedDeviceType", selectedDeviceType);
        settings.setValue("Selection/SelectedDeviceName", selectedDeviceName);

        filterFromSelectedDevices();

        refreshDifferencesCatalogSelection();

    }
    //----------------------------------------------------------------------
    void MainWindow::on_Filters_pushButton_ResetGlobal_clicked()
    {
        resetSelection();
    }
    //----------------------------------------------------------------------
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
            activeCatalog->setName(selectedDeviceName);
            activeCatalog->loadCatalogMetaData();

            updateSingleCatalog(activeCatalog);
        }
    }
    //----------------------------------------------------------------------
    void MainWindow::on_Filters_pushButton_TreeExpandCollapse_clicked()
    {
        toggleTreeExpandState();
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
        ui->Filters_treeView_Devices->setColumnWidth(2,0);
        ui->Filters_treeView_Devices->collapseAll();
        ui->Filters_treeView_Devices->header()->hide();

        //Restore Expand or Collapse Device Tree
        toggleTreeExpandState();
    }
    //----------------------------------------------------------------------
    void MainWindow::resetSelection()
    {
        //reset selected values
        selectedDeviceType = tr("All");
        selectedDeviceName = tr("All");
        selectedStorageLocation   = tr("All");
        selectedStorageName       = tr("All");
        selectedCatalogName       = tr("All");
        ui->Filters_label_DisplayLocation->setText(tr("All"));
        ui->Filters_label_DisplayStorage->setText(tr("All"));
        ui->Filters_label_DisplayCatalog->setText(tr("All"));
        activeCatalog->setName(tr(""));
        activeCatalog->loadCatalogMetaData();
        refreshStorageSelectionList(selectedStorageLocation);
        refreshCatalogSelectionList(selectedStorageLocation,selectedStorageName);
        ui->Filter_pushButton_Explore->setEnabled(false);
        ui->Filter_pushButton_Update->setEnabled(false);

        //reset device tree
        toggleTreeExpandState();
        toggleTreeExpandState();

        loadStorageTableToSelectionTreeModel();
        filterFromSelectedDevices();

        QSettings settings(settingsFilePath, QSettings:: IniFormat);
        settings.setValue("LastSearch/SelectedSearchLocation", selectedStorageLocation);
        settings.setValue("LastSearch/SelectedSearchStorage",  selectedStorageName);
        settings.setValue("LastSearch/SelectedSearchCatalog",  selectedCatalogName);

        refreshDifferencesCatalogSelection();

    }
    //----------------------------------------------------------------------
    void MainWindow::toggleTreeExpandState()
    {
        //deviceTreeExpandState values:  0=collapse / 1=exp.level0 / 2=exp.level1
        QString iconName = ui->Filters_pushButton_TreeExpandCollapse->icon().name();
        QSettings settings(settingsFilePath, QSettings:: IniFormat);

        if ( deviceTreeExpandState == 0 ){
            //collapsed > expand first level
            ui->Filters_pushButton_TreeExpandCollapse->setIcon(QIcon::fromTheme("expand-all"));
            ui->Filters_treeView_Devices->expandToDepth(deviceTreeExpandState);
            settings.setValue("Settings/deviceTreeExpandState", deviceTreeExpandState);
            deviceTreeExpandState = 1;
        }
        else if ( deviceTreeExpandState == 1 ){
            //expanded first level > expand to second level
            ui->Filters_pushButton_TreeExpandCollapse->setIcon(QIcon::fromTheme("collapse-all"));
            ui->Filters_treeView_Devices->expandAll();
            settings.setValue("Settings/deviceTreeExpandState", deviceTreeExpandState);
            deviceTreeExpandState = 2;
        }
        else if ( deviceTreeExpandState == 2 ){
            //expanded second level > collapse
            ui->Filters_pushButton_TreeExpandCollapse->setIcon(QIcon::fromTheme("expand-all"));
            ui->Filters_treeView_Devices->collapseAll();
            settings.setValue("Settings/deviceTreeExpandState", deviceTreeExpandState);
            deviceTreeExpandState = 0;
        }
    }
    //----------------------------------------------------------------------
    void MainWindow::filterFromSelectedDevices()
    {
        if(selectedDeviceType=="Location"){
            ui->Filter_pushButton_Explore->setEnabled(false);
            ui->Filter_pushButton_Update->setEnabled(false);

            selectedStorageLocation = selectedDeviceName;
            selectedStorageName = tr("All");
            selectedCatalogName = tr("All");

            refreshStorageSelectionList(selectedStorageLocation);
            refreshCatalogSelectionList(selectedStorageLocation,selectedStorageName);
        }
        else if (selectedDeviceType=="Storage"){
            ui->Filter_pushButton_Explore->setEnabled(false);
            ui->Filter_pushButton_Update->setEnabled(false);

            selectedStorageLocation = tr("All");
            selectedStorageName = selectedDeviceName;
            selectedCatalogName = tr("All");

            refreshStorageSelectionList(selectedStorageLocation);
            refreshCatalogSelectionList(selectedStorageLocation,selectedStorageName);
        }
        else if (selectedDeviceType=="Catalog"){
            ui->Filter_pushButton_Explore->setEnabled(true);
            ui->Filter_pushButton_Update->setEnabled(true);

            activeCatalog->setName(selectedDeviceName);
            activeCatalog->loadCatalogMetaData();

            selectedStorageLocation = tr("All");
            selectedStorageName = tr("All");
            selectedCatalogName = activeCatalog->name;
        }

        //Display selection values and save them
            ui->Filters_label_DisplayLocation->setText(selectedStorageLocation);
            ui->Filters_label_DisplayStorage->setText(selectedStorageName);
            ui->Filters_label_DisplayCatalog->setText(selectedCatalogName);

            QSettings settings(settingsFilePath, QSettings:: IniFormat);
            settings.setValue("LastSearch/SelectedSearchLocation", selectedStorageLocation);
            settings.setValue("LastSearch/SelectedSearchStorage",  selectedStorageName);
            settings.setValue("LastSearch/SelectedSearchCatalog",  selectedCatalogName);

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

//SETTINGS / Collection ----------------------------------------------------

    void MainWindow::on_Collection_pushButton_SelectFolder_clicked()
    {
        //Open a dialog for the user to select the directory of the collection where catalog files are stored.
        QString dir = QFileDialog::getExistingDirectory(this, tr("Select the directory for this collection"),
                                                        collectionFolder,
                                                        QFileDialog::ShowDirsOnly
                                                        | QFileDialog::DontResolveSymlinks);

        //Unless the selection was cancelled, set the new collection folder, and refresh all data
        if ( dir !=""){

            collectionFolder = dir;

            //set the new path in Colletion tab
            ui->Collection_lineEdit_CollectionFolder->setText(collectionFolder);

            //redefine the path of the Storage file
            storageFilePath = collectionFolder + "/" + "storage.csv";

            //save Settings for the new collection folder value;
            saveSettings();

            //load the collection from this new folder;
                //Clear database if mode is Memory
                if(databaseMode=="Memory"){
                    //Clear current entires from the tables
                        QSqlQuery queryDelete;
                        queryDelete.exec("DELETE FROM catalog");
                        queryDelete.exec("DELETE FROM storage");
                        queryDelete.exec("DELETE FROM filesall");
                        queryDelete.exec("DELETE FROM file");
                        queryDelete.exec("DELETE FROM folder");
                        queryDelete.exec("DELETE FROM statistics");
                        queryDelete.exec("DELETE FROM search");
                        queryDelete.exec("DELETE FROM tag");
                }
                if(databaseMode=="File"){
                    //Open database file
                    //DEV
                    QMessageBox::warning(this,"Katalog","Database was not changed.");
                }

            createStorageList();
            generateCollectionFilesPaths();
            loadCollection();

        }

        //Reset selected values (to avoid actions on the last selected ones)
        resetSelection();
    }
    //----------------------------------------------------------------------
    void MainWindow::on_Collection_lineEdit_CollectionFolder_returnPressed()
    {
        loadCollection();
    }
    //----------------------------------------------------------------------
    void MainWindow::on_Collection_pushButton_OpenFolder_clicked()
    {
        //Open the selected collection folder
        QDesktopServices::openUrl(QUrl::fromLocalFile(collectionFolder));
    }
    //----------------------------------------------------------------------
    void MainWindow::on_Collection_pushButton_Reload_clicked()
    {
        loadCollection();
    }
    //----------------------------------------------------------------------
    void MainWindow::loadCollection()
    {
        //Load search history
            loadSearchHistoryFileToTable();
            loadSearchHistoryTableToModel();

        //Load Storage list and refresh their statistics
            loadStorageFileToTable();
            loadStorageTableToModel();
            updateStorageSelectionStatistics();

       //Load Catalogs list
            loadCatalogFilesToTable();
            loadCatalogsTableToModel();

       //Load Storage list
            refreshLocationSelectionList();
            refreshStorageSelectionList(selectedStorageLocation);
            refreshCatalogSelectionList(selectedStorageLocation, selectedStorageName);
            loadStorageTableToSelectionTreeModel();

            //Add a storage device for catalogs without one
            QSqlQuery queryCatalog;
            QString queryCatalogSQL = QLatin1String(R"(
                                    SELECT count(*)
                                    FROM catalog
                                    WHERE catalogStorage=""
                                            )");
            queryCatalog.prepare(queryCatalogSQL);
            queryCatalog.exec();
            queryCatalog.next();

            QSqlQuery queryStorage;
            QString queryStorageSQL = QLatin1String(R"(
                                    SELECT count(*)
                                    FROM storage
                                    WHERE storageName=""
                                )");
            queryStorage.prepare(queryStorageSQL);
            queryStorage.exec();
            queryStorage.next();

            if (queryCatalog.value(0).toInt() >0 and queryStorage.value(0).toInt() == 0){
                addStorageDevice(tr(""));
            }

       //Load Statistics
            loadStatisticsDataTypes();
            loadStatisticsData();
            loadStatisticsChart();

       //Load Tags
            reloadTagsData();

       //Hide buttons to force user to select a catalog before allowing any action.
            hideCatalogButtons();
    }
    //----------------------------------------------------------------------
    void MainWindow::preloadCatalogs()
    {
        foreach(sourceCatalog,catalogSelectedList)
                {
                    selectedCatalog->setName(sourceCatalog);
                    selectedCatalog->loadCatalogMetaData();
                    loadCatalogFilelistToTable(selectedCatalog);
                }
    }

//SETTINGS / Settings ------------------------------------------------------

    void MainWindow::on_Settings_checkBox_SaveRecordWhenUpdate_stateChanged()
    {
        QSettings settings(settingsFilePath, QSettings:: IniFormat);
        settings.setValue("Settings/AutoSaveRecordWhenUpdate", ui->Settings_checkBox_SaveRecordWhenUpdate->isChecked());
    }
    //----------------------------------------------------------------------
    void MainWindow::on_Settings_checkBox_KeepOneBackUp_stateChanged()
    {
        QSettings settings(settingsFilePath, QSettings:: IniFormat);
        settings.setValue("Settings/KeepOneBackUp", ui->Settings_checkBox_KeepOneBackUp->isChecked());
    }
    //----------------------------------------------------------------------
    void MainWindow::on_Settings_comboBox_Theme_currentIndexChanged(int index)
    {
        QSettings settings(settingsFilePath, QSettings:: IniFormat);
        settings.setValue("Settings/Theme", index);
    }
    //----------------------------------------------------------------------
    void MainWindow::on_Settings_checkBox_CheckVersion_stateChanged()
    {
        QSettings settings(settingsFilePath, QSettings:: IniFormat);
        settings.setValue("Settings/CheckVersion", ui->Settings_checkBox_CheckVersion->isChecked());
    }
    //----------------------------------------------------------------------
    void MainWindow::on_Settings_checkBox_PreloadCatalogs_stateChanged(int arg1)
    {
        QSettings settings(settingsFilePath, QSettings:: IniFormat);
        settings.setValue("Settings/PreloadCatalogs", ui->Settings_checkBox_PreloadCatalogs->isChecked());
    }
    //----------------------------------------------------------------------
    void MainWindow::on_Collection_pushButton_OpenSettingsFile_clicked()
    {
        //Open the selected collection folder
        QDesktopServices::openUrl(QUrl::fromLocalFile(settingsFilePath));
    }
    //----------------------------------------------------------------------

//SETTINGS / About ---------------------------------------------------------

    void MainWindow::on_Settings_pushButton_Wiki_clicked()
    {
        QDesktopServices::openUrl(QUrl("https://github.com/StephaneCouturier/Katalog/wiki"));
    }
    //----------------------------------------------------------------------
    void MainWindow::on_Settings_pushButton_ReleaseNotes_clicked()
    {
        QDesktopServices::openUrl(QUrl("https://github.com/StephaneCouturier/Katalog/releases"));
    }
    //----------------------------------------------------------------------
    void MainWindow::on_Settings_comboBox_Language_currentTextChanged(const QString &selectedLanguage)
    {
        QSettings settings(settingsFilePath, QSettings:: IniFormat);
        settings.setValue("Settings/Language", selectedLanguage);
    }
    //----------------------------------------------------------------------
