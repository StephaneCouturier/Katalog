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
// Purpose:     methods for the Settings panel
// Description:
// Author:      Stephane Couturier
/////////////////////////////////////////////////////////////////////////////
*/

#include "mainwindow.h"
#include "ui_mainwindow.h"
#include "database.h"
#include "catalog.h"

//SETTINGS / GLOBAL -----------------------------------------------------------------
    void MainWindow::on_splitter_splitterMoved()
    {
        QSettings settings(collection->settingsFilePath, QSettings:: IniFormat);

        QSize widget1Size = ui->splitter_widget_Filters->size();
        QSize widget2Size = ui->splitter_widget_TabWidget->size();

        settings.setValue("Settings/SplitterWidget1Size", widget1Size);
        settings.setValue("Settings/SplitterWidget2Size", widget2Size);

    }
    //----------------------------------------------------------------------
    void MainWindow::on_tabWidget_currentChanged(int index)
    {
        selectedTab = index;
        QSettings settings(collection->settingsFilePath, QSettings:: IniFormat);
        settings.setValue("Settings/selectedTab", ui->tabWidget->currentIndex());
    }
    //----------------------------------------------------------------------

//SETTINGS / Data management
    void MainWindow::on_Settings_comboBox_DatabaseMode_currentTextChanged()
    {
        QString newDatabaseMode = ui->Settings_comboBox_DatabaseMode->itemData(ui->Settings_comboBox_DatabaseMode->currentIndex(),Qt::UserRole).toString();
        QSettings settings(collection->settingsFilePath, QSettings:: IniFormat);
        settings.setValue("Settings/databaseMode", newDatabaseMode);

        if(newDatabaseMode=="Memory"){
            ui->Settings_widget_DataMode_CSVFiles->show();
            ui->Settings_widget_DataMode_LocalSQLite->hide();
            ui->Settings_widget_DataMode_Hosted->hide();
        }
        else if(newDatabaseMode=="File"){
            ui->Settings_widget_DataMode_CSVFiles->hide();
            ui->Settings_widget_DataMode_LocalSQLite->show();
            ui->Settings_widget_DataMode_Hosted->hide();
        }
        else if(newDatabaseMode=="Hosted"){
            ui->Settings_widget_DataMode_CSVFiles->hide();
            ui->Settings_widget_DataMode_LocalSQLite->hide();
            ui->Settings_widget_DataMode_Hosted->show();
        }
    }
    //----------------------------------------------------------------------

    //Memory ---------------------------------------------------------------
    void MainWindow::on_Settings_lineEdit_CollectionFolder_returnPressed()
    {
        QSettings settings(collection->settingsFilePath, QSettings:: IniFormat);
        settings.setValue("LastCollectionFolder", collection->collectionFolder);

        loadCollection();
    }
    //----------------------------------------------------------------------
    void MainWindow::on_Settings_pushButton_SelectFolder_clicked()
    {
        //Open a dialog for the user to select the directory of the collection where catalog files are stored.
        QString dir = QFileDialog::getExistingDirectory(this, tr("Select the directory for this collection"),
                                                        collection->collectionFolder,
                                                        QFileDialog::ShowDirsOnly
                                                        | QFileDialog::DontResolveSymlinks);

        //Unless the selection was cancelled, set the new collection folder, and refresh all data
        if ( dir !=""){

            collection->collectionFolder = dir;

            QSettings settings(collection->settingsFilePath, QSettings:: IniFormat);
            settings.setValue("LastCollectionFolder", collection->collectionFolder);

            //Set the new path in Settings tab
            ui->Settings_lineEdit_CollectionFolder->setText(collection->collectionFolder);

            //Redefine the path of the Storage file
            collection->storageFilePath = collection->collectionFolder + "/" + "storage.csv";

            //Load the collection from this new folder;
                //Clear database if mode is Memory
                if(collection->databaseMode=="Memory"){
                    //Clear current entires from the tables
                        QSqlQuery queryDelete;
                        queryDelete.exec("DELETE FROM catalog");
                        queryDelete.exec("DELETE FROM storage");
                        queryDelete.exec("DELETE FROM file");
                        queryDelete.exec("DELETE FROM filetemp");
                        queryDelete.exec("DELETE FROM folder");
                        queryDelete.exec("DELETE FROM statistics");
                        queryDelete.exec("DELETE FROM search");
                        queryDelete.exec("DELETE FROM tag");
                }
                if(collection->databaseMode=="File"){
                    //Open database file
                    //DEV
                    QMessageBox::warning(this,"Katalog","Database was not changed.");
                }

            collection->createStorageFile();
            collection->generateCollectionFilesPaths();
            loadCollection();
        }

        //Reset selected values (to avoid actions on the last selected ones)
        resetSelection();
    }
    //----------------------------------------------------------------------
    void MainWindow::on_Settings_pushButton_OpenFolder_clicked()
    {
        //Open the selected collection folder
        QDesktopServices::openUrl(QUrl::fromLocalFile(collection->collectionFolder));
    }
    //----------------------------------------------------------------------
    void MainWindow::on_Settings_checkBox_KeepOneBackUp_stateChanged()
    {
        QSettings settings(collection->settingsFilePath, QSettings:: IniFormat);
        settings.setValue("Settings/KeepOneBackUp", ui->Settings_checkBox_KeepOneBackUp->isChecked());
    }
    //----------------------------------------------------------------------
    void MainWindow::on_Settings_checkBox_PreloadCatalogs_stateChanged(int arg1)
    {
        QSettings settings(collection->settingsFilePath, QSettings:: IniFormat);
        settings.setValue("Settings/PreloadCatalogs", arg1);
    }
    //----------------------------------------------------------------------

    //File -----------------------------------------------------------------
    void MainWindow::on_Settings_pushButton_SelectDatabaseFilePath_clicked()
    {
        selectDatabaseFilePath();
    }
    //----------------------------------------------------------------------
    void MainWindow::on_Settings_pushButton_EditDatabaseFile_clicked()
    {
        collection->databaseFilePath = ui->Settings_lineEdit_DatabaseFilePath->text();
        QDesktopServices::openUrl(QUrl::fromLocalFile(collection->databaseFilePath));
    }
    //----------------------------------------------------------------------
    void MainWindow::on_Settings_pushButton_NewDatabaseFile_clicked()
    {
        selectNewDatabaseFolderPath();
    }
    //----------------------------------------------------------------------

    //Hosted ---------------------------------------------------------------
    void MainWindow::on_Settings_pushButton_SaveHostedParameters_clicked()
    {
        QSettings settings(collection->settingsFilePath, QSettings:: IniFormat);
        settings.setValue("Settings/databaseHostName", ui->Settings_lineEdit_DataMode_Hosted_HostName->text());
        settings.setValue("Settings/databaseName",     ui->Settings_lineEdit_DataMode_Hosted_DatabaseName->text());
        settings.setValue("Settings/databasePort",     ui->Settings_lineEdit_DataMode_Hosted_Port->text());
        settings.setValue("Settings/databaseUserName", ui->Settings_lineEdit_DataMode_Hosted_UserName->text());
        settings.setValue("Settings/databasePassword", ui->Settings_lineEdit_DataMode_Hosted_Password->text());

        QMessageBox msgBox;
        msgBox.setWindowTitle("Katalog");
        msgBox.setText(tr("Save Database OnlineSettings.<br/>Please restart the app."));
        msgBox.setIcon(QMessageBox::Information);
        msgBox.exec();
    }

//SETTINGS / Language & Theme ----------------------------------------------
    void MainWindow::on_Settings_comboBox_Language_currentTextChanged(const QString &selectedLanguage)
    {
        QSettings settings(collection->settingsFilePath, QSettings:: IniFormat);
        settings.setValue("Settings/Language", selectedLanguage);
    }
    //----------------------------------------------------------------------
    void MainWindow::on_Settings_comboBox_Theme_currentIndexChanged(int index)
    {
        QSettings settings(collection->settingsFilePath, QSettings:: IniFormat);
        settings.setValue("Settings/Theme", index);
    }
    //----------------------------------------------------------------------
    void MainWindow::on_Settings_checkBox_SaveRecordWhenUpdate_stateChanged()
    {
        QSettings settings(collection->settingsFilePath, QSettings:: IniFormat);
        settings.setValue("Settings/AutoSaveRecordWhenUpdate", ui->Settings_checkBox_SaveRecordWhenUpdate->isChecked());       
    }
    //----------------------------------------------------------------------
    void MainWindow::on_Settings_checkBox_LoadLastCatalog_stateChanged(int arg1)
    {
        QSettings settings(collection->settingsFilePath, QSettings:: IniFormat);
        settings.setValue("Settings/LoadLastCatalog", arg1);
    }
    //----------------------------------------------------------------------
    void MainWindow::on_Settings_pushButton_OpenSettingsFile_clicked()
    {
        //Open the selected collection folder
        QDesktopServices::openUrl(QUrl::fromLocalFile(collection->settingsFilePath));
    }
    //----------------------------------------------------------------------
    void MainWindow::on_Settings_checkBox_BiggerIconSize_stateChanged(int arg1)
    {
        QSettings settings(collection->settingsFilePath, QSettings:: IniFormat);
        settings.setValue("Settings/ThemeBiggerIconSize", arg1);

        QSize size;
        if(arg1==2){
            size.setHeight(32);
            size.setWidth(32);}
        else{
            size.setHeight(22);
            size.setWidth(22);
        }
        ui->Filters_treeView_Devices->setIconSize(size);
        ui->Filters_treeView_Directory->setIconSize(size);
        ui->Search_treeView_FilesFound->setIconSize(size);
        ui->Explore_treeview_Directories->setIconSize(size);
        ui->Explore_treeView_FileList->setIconSize(size);
        ui->Create_treeView_Explorer->setIconSize(size);
        ui->Devices_treeView_DeviceList->setIconSize(size);
        ui->Tags_treeview_Explorer->setIconSize(size);
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
    void MainWindow::on_Settings_checkBox_CheckVersion_stateChanged()
    {
        QSettings settings(collection->settingsFilePath, QSettings:: IniFormat);
        settings.setValue("Settings/CheckVersion", ui->Settings_checkBox_CheckVersion->isChecked());
    }
    //----------------------------------------------------------------------


//SETTINGS / data methods --------------------------------------------------
    void MainWindow::loadCollection()
    {
        //Generate collection files paths and statistics parameters
        collection->generateCollectionFilesPaths();

        //Load data from files to database ("Memory" database mode
            //Create a Storage list (if none exists) + conversions
            collection->createStorageFile();

            //Clear database
            clearDatabaseData();

            //Load Files to database
            loadSearchHistoryFileToTable();
            collection->loadCatalogFilesToTable();
            collection->loadStorageFileToTable();
            collection->loadDeviceFileToTable();
            collection->loadExclude();

        //Check active status and synch it
            updateAllDeviceActive();

        //Load data from tables to models and update display
            loadDevicesTreeToModel("Filters");
            loadSearchHistoryTableToModel();
            filterFromSelectedDevice();

        //Add a default storage device, to force any new catalog to have one
        QSqlQuery queryStorage;
        QString queryStorageSQL = QLatin1String(R"(
                                    SELECT COUNT(*)
                                    FROM device
                                    WHERE device_type='Storage'
                                )");
        queryStorage.prepare(queryStorageSQL);
        queryStorage.exec();
        queryStorage.next();

        if (queryStorage.value(0).toInt() == 0){
            //Create Device and related Storage under Physical group (ID=0)
            Device *newDevice = new Device();
            newDevice->generateDeviceID();

            newDevice->parentID = 2;
            if(newDevice->verifyParentDeviceExistsInPhysicalGroup()==false)
                newDevice->parentID = 1;

            newDevice->name = tr("Default Storage");
            newDevice->type = "Storage";
            newDevice->storage->generateID();
            newDevice->externalID = newDevice->storage->ID;
            newDevice->groupID = 0;
            newDevice->insertDevice();
            newDevice->storage->insertStorage();

            //Save data to file
            collection->saveDeviceTableToFile();

            //Reload
            loadDevicesView();
            loadParentsList();
        }

        //Load Statistics
        loadStatisticsDataTypes();
        collection->loadStatisticsDeviceFileToTable();
        loadStatisticsChart();

        //Load Tags
        reloadTagsData();
    }
    //----------------------------------------------------------------------
    void MainWindow::preloadCatalogs()
    {
        foreach(QString sourceCatalog,catalogSelectedList)
        {
                    selectedDevice->name = sourceCatalog;
                    selectedDevice->catalog->loadCatalog();
                    selectedDevice->catalog->loadCatalogFileListToTable();
        }
    }
    //----------------------------------------------------------------------
    void MainWindow::selectDatabaseFilePath()
    {
        //Open a dialog for the user to select the directory of the collection where catalog files are stored.
        QString newDatabaseFilePath = QFileDialog::getOpenFileName(this, tr("Select the database to open:"),
                                                                   collection->collectionFolder,"*.db");

        //Unless the selection was cancelled, set the new collection folder, and refresh all data
        if ( newDatabaseFilePath !=""){

                    collection->databaseFilePath = newDatabaseFilePath;

                    //Save Settings for the new collection folder value;
                    QSettings settings(collection->settingsFilePath, QSettings:: IniFormat);
                    settings.setValue("Settings/DatabaseFilePath", collection->databaseFilePath);

                    //Set the new path in Settings tab
                    ui->Settings_lineEdit_DatabaseFilePath->setText(collection->databaseFilePath);

                    collection->createStorageFile();
                    collection->generateCollectionFilesPaths();

                    loadCollection();
        }

        //Reset selected values (to avoid actions on the last selected ones)
        resetSelection();
    }
    //----------------------------------------------------------------------
    void MainWindow::selectNewDatabaseFolderPath()
    {
        //Open a dialog for the user to select the directory of the collection where catalog files are stored.
        QString newDatabaseFilePath = QFileDialog::getSaveFileName(this, tr("Select the database to open:"),
                                                                   collection->collectionFolder+"/newKatalogFile.db","*.db");

        //Unless the selection was cancelled, set the new collection folder, and refresh all data
        if ( newDatabaseFilePath !=""){

                    collection->databaseFilePath = newDatabaseFilePath;

                    QFile fileOut(collection->databaseFilePath);
                    if (fileOut.open(QFile::WriteOnly | QFile::Text)) {
                        //create an empty file
                    }
                    fileOut.close();

                    QSqlQuery q;
                    q.exec(SQL_CREATE_CATALOG);
                    q.exec(SQL_CREATE_STORAGE);
                    q.exec(SQL_CREATE_FILE);
                    q.exec(SQL_CREATE_FILETEMP);
                    q.exec(SQL_CREATE_FOLDER);
                    q.exec(SQL_CREATE_METADATA);
                    q.exec(SQL_CREATE_SEARCH);
                    q.exec(SQL_CREATE_TAG);

                    //Save Settings for the new collection folder value;
                    QSettings settings(collection->settingsFilePath, QSettings:: IniFormat);
                    settings.setValue("Settings/DatabaseFilePath", collection->databaseFilePath);

                    //Set the new path in Settings tab
                    ui->Settings_lineEdit_DatabaseFilePath->setText(collection->databaseFilePath);

                    collection->createStorageFile();
                    collection->generateCollectionFilesPaths();
                    loadCollection();
        }

        //Reset selected values (to avoid actions on the last selected ones)
        resetSelection();
    }
