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
//#include "filesview.h"//test
#include "storagetreemodel.h"
#include "storageview.h"

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
    void MainWindow::on_Filters_pushButton_ResetGlobal_clicked()
    {
            ui->Filters_comboBox_SelectLocation->setCurrentText(tr("All"));
            ui->Filters_comboBox_SelectStorage->setCurrentText(tr("All"));
            ui->Filters_comboBox_SelectCatalog->setCurrentText(tr("All"));
    }
    //----------------------------------------------------------------------
    void MainWindow::on_Filters_comboBox_SelectLocation_currentIndexChanged(const QString &selectedLocation)
    {
        //save selection in settings file;
        QSettings settings(settingsFilePath, QSettings:: IniFormat);
        settings.setValue("LastSearch/SelectedSearchLocation", selectedLocation);

        //Load matching Storage
        refreshStorageSelectionList(selectedLocation);
        selectedSearchLocation = selectedLocation;

        //Load matching Catalog
        loadCatalogsToModel();
        loadStorageTableToModel();
        refreshStorageStatistics();

        //Load statistics
        loadStatisticsData();
        loadStatisticsChart();
    }
    //----------------------------------------------------------------------
    void MainWindow::on_Filters_comboBox_SelectStorage_currentIndexChanged(const QString &selectedStorage)
    {
        //save selection in settings file;
        QSettings settings(settingsFilePath, QSettings:: IniFormat);
        settings.setValue("LastSearch/SelectedSearchStorage", selectedStorage);

        //Get selected Location
        QString selectedLocation = ui->Filters_comboBox_SelectLocation->currentText();

        //Load matching Storage
        refreshCatalogSelectionList(selectedLocation, selectedStorage);

        selectedSearchStorage = selectedStorage;

        //Load matching Catalog
        loadCatalogsToModel();

        //Load statistics
        loadStatisticsData();
        loadStatisticsChart();
    }
    //----------------------------------------------------------------------
    void MainWindow::on_Filters_comboBox_SelectCatalog_currentIndexChanged(const QString &selectedCatalog)
    {
        //save selection in settings file;
        QSettings settings(settingsFilePath, QSettings:: IniFormat);
        settings.setValue("LastSearch/SelectedSearchCatalog", selectedCatalog);

        selectedSearchCatalog = selectedCatalog;

        //Load statistics
        loadStatisticsData();
        loadStatisticsChart();
    }
    //----------------------------------------------------------------------
    void MainWindow::on_Filters_checkBox_SearchInCatalogs_toggled(bool checked)
    {
        QSettings settings(settingsFilePath, QSettings:: IniFormat);
        settings.setValue("LastSearch/searchInFileCatalogsChecked", checked);

        if(checked==1){
            //Enable Catalogs selection
            ui->Filters_widget_CatalogSelection->setEnabled(true);
            ui->Filters_widget_ConnectedDrives->setDisabled(true);
            ui->Filters_checkBox_SearchInConnectedDrives->setChecked(false);
        }
        else if(ui->Filters_checkBox_SearchInConnectedDrives->isChecked()==true){
            //Disable Catalogs selection
            ui->Filters_widget_CatalogSelection->setDisabled(true);
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
            ui->Filters_widget_CatalogSelection->setDisabled(true);
            ui->Filters_checkBox_SearchInCatalogs->setChecked(false);
            ui->Filters_widget_CatalogSelection->hide();
            ui->Filters_widget_ConnectedDrives->show();
        }
        else if(ui->Filters_checkBox_SearchInCatalogs->isChecked()==true){
            //Disable Directory selection
            ui->Filters_widget_ConnectedDrives->setDisabled(true);
            ui->Filters_widget_CatalogSelection->show();
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
    void MainWindow::on_Filters_treeView_Devices_clicked(const QModelIndex &index)
    {
        QString clickedName = ui->Filters_treeView_Devices->model()->index(index.row(), 0, index.parent() ).data().toString();
        //ui->DEV2_label_Display->setText(clickedName);

        QString clickedType = ui->Filters_treeView_Devices->model()->index(index.row(), 1, index.parent() ).data().toString();
        //QMessageBox::information(this,"Katalog","type:" + QVariant(clickedType).toString());

        if(clickedType=="Location"){
            ui->Filters_comboBox_SelectLocation->setCurrentText(clickedName);
            ui->Filters_comboBox_SelectStorage->setCurrentText(tr("All"));
            ui->Filters_comboBox_SelectCatalog->setCurrentText(tr("All"));
        }
        else if (clickedType=="Storage"){
            ui->Filters_comboBox_SelectLocation->setCurrentText(tr("All"));
            ui->Filters_comboBox_SelectStorage->setCurrentText(clickedName);
            ui->Filters_comboBox_SelectCatalog->setCurrentText(tr("All"));
        }
        else if (clickedType=="Catalog"){
            ui->Filters_comboBox_SelectLocation->setCurrentText(tr("All"));
            ui->Filters_comboBox_SelectStorage->setCurrentText(tr("All"));
            ui->Filters_comboBox_SelectCatalog->setCurrentText(clickedName);
        }
    }
    //----------------------------------------------------------------------
    void MainWindow::loadStorageTableToFilterTree()
    {
        const QStringList headers({tr("Title"),tr("Type")});

        StorageTreeModel *storageTreeModel = new StorageTreeModel(headers);

        //LoadModel
        ui->Filters_treeView_Devices->setModel(storageTreeModel);
        ui->Filters_treeView_Devices->header()->resizeSection(0,  300);
        ui->Filters_treeView_Devices->expandAll();

        ui->Filters_treeView_Devices->hideColumn(1);
        ui->Filters_treeView_Devices->collapseAll();
        ui->Filters_treeView_Devices->header()->hide();
    }

//SETTINGS / Collection ----------------------------------------------------

    void MainWindow::on_Collection_pushButton_SelectFolder_clicked()
    {
        //Open a dialog for the user to select the directory of the collection where catalog files are stored.
        QString dir = QFileDialog::getExistingDirectory(this, tr("Select the directory for this collection"),
                                                        collectionFolder,
                                                        QFileDialog::ShowDirsOnly
                                                        | QFileDialog::DontResolveSymlinks);

        //Unless the selection was cancelled, set the new collection folder, and refresh the list of catalogs
        if ( dir !=""){

            collectionFolder = dir;

            //set the new path in Colletion tab
            ui->Collection_lineEdit_CollectionFolder->setText(collectionFolder);

            //redefine the path of the Storage file
            storageFilePath = collectionFolder + "/" + "storage.csv";

            //save Settings for the new collection folder value;
            saveSettings();

            //load the collection for this new folder;
            loadCollection();

        }

        //Reset selected catalog values (to avoid actions on the last selected one)
        selectedCatalogFile="";
        selectedCatalogName="";
        selectedCatalogPath="";
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
        //Load Storage list and refresh their statistics
            loadStorageFileToTable();
            loadStorageTableToModel();
            refreshStorageStatistics();

       //load Catalog list, Location list, Storage list, Statistics
            loadCatalogFilesToTable();
            loadCatalogsToModel();

            refreshLocationSelectionList();
            refreshStorageSelectionList(tr("All"));
            refreshCatalogSelectionList(tr("All"), tr("All"));

            //restore Search catalog selection
            //ui->Filters_comboBox_SelectLocation->setCurrentText(selectedSearchLocation);
            //ui->Filters_comboBox_SelectStorage->setCurrentText(selectedSearchStorage);
            //ui->Filters_comboBox_SelectCatalog->setCurrentText(selectedSearchCatalog);

            statisticsFilePath = collectionFolder + "/" + "statistics.csv";
            loadStatisticsData();
            loadStatisticsChart();

            //hide buttons to force user to select a catalog before allowing any action.
            hideCatalogButtons();
    }
    //----------------------------------------------------------------------
    void MainWindow::preloadCatalogs()
    {
        foreach(sourceCatalog,catalogSelectedList)
                {
                    loadCatalogFilelistToTable(sourceCatalog);
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
