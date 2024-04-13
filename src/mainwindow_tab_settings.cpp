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
        applyDatabaseModeToUI();
    }
    //----------------------------------------------------------------------
    void MainWindow::applyDatabaseModeToUI()
    {
        QString newDatabaseMode = ui->Settings_comboBox_DatabaseMode->itemData(ui->Settings_comboBox_DatabaseMode->currentIndex(),Qt::UserRole).toString();

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

        if(newDatabaseMode != collection->databaseMode)
            ui->Settings_pushButton_DatabaseModeApplyAndRestart->setEnabled(true);
        else
            ui->Settings_pushButton_DatabaseModeApplyAndRestart->setEnabled(false);
    }
    //----------------------------------------------------------------------
    void MainWindow::on_Settings_pushButton_DatabaseModeApplyAndRestart_clicked()
    {
        //Save choice of mode
        collection->databaseMode = ui->Settings_comboBox_DatabaseMode->itemData(ui->Settings_comboBox_DatabaseMode->currentIndex(),Qt::UserRole).toString();
        QSettings settings(collection->settingsFilePath, QSettings:: IniFormat);
        settings.setValue("Settings/databaseMode", collection->databaseMode);

        //Save folder
        if(collection->databaseMode=="Memory"){
            settings.setValue("LastCollectionFolder", collection->folder);
        }
        //Save sqlite file path
        else if(collection->databaseMode=="File"){
            settings.setValue("Settings/DatabaseFilePath", collection->databaseFilePath);
        }
        //Save host parameters
        else if(collection->databaseMode=="Hosted"){
            settings.setValue("Settings/databaseHostName", ui->Settings_lineEdit_DataMode_Hosted_HostName->text());
            settings.setValue("Settings/databaseName",     ui->Settings_lineEdit_DataMode_Hosted_DatabaseName->text());
            settings.setValue("Settings/databasePort",     ui->Settings_lineEdit_DataMode_Hosted_Port->text());
            settings.setValue("Settings/databaseUserName", ui->Settings_lineEdit_DataMode_Hosted_UserName->text());
            settings.setValue("Settings/databasePassword", ui->Settings_lineEdit_DataMode_Hosted_Password->text());
        }
        //Enable and highlight restart button
        QProcess::startDetached(QApplication::applicationFilePath(), QApplication::arguments());
        QApplication::exit();
    }
    //----------------------------------------------------------------------
    void MainWindow::changeCollectionFolder()
    {
        QSettings settings(collection->settingsFilePath, QSettings:: IniFormat);
        settings.setValue("LastCollectionFolder", collection->folder);

        //Set the new path in Settings tab
        ui->Settings_lineEdit_CollectionFolder->setText(collection->folder);

        //Redefine the path of the Storage file
        collection->storageFilePath = collection->folder + "/" + "storage.csv";

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

        collection->createStorageFile();
        collection->generateCollectionFilesPaths();
        loadCollection();
    }

    //Memory ---------------------------------------------------------------
    void MainWindow::on_Settings_lineEdit_CollectionFolder_returnPressed()
    {
        QString dir = ui->Settings_lineEdit_CollectionFolder->text();

        //Unless the selection was cancelled, set the new collection folder, and refresh all data
        if ( dir !=""){
            collection->folder = dir;
            changeCollectionFolder();
        }

        //Reset selected values (to avoid actions on the last selected ones)
        resetSelection();
    }
    //----------------------------------------------------------------------
    void MainWindow::on_Settings_pushButton_SelectFolder_clicked()
    {
        //Open a dialog for the user to select the directory of the collection where catalog files are stored.
        QString dir = QFileDialog::getExistingDirectory(this, tr("Select the directory for this collection"),
                                                        collection->folder,
                                                        QFileDialog::ShowDirsOnly
                                                            | QFileDialog::DontResolveSymlinks);

        //Unless the selection was cancelled, set the new collection folder, and refresh all data
        if ( dir !=""){
            collection->folder = dir;
            changeCollectionFolder();
        }

        //Reset selected values (to avoid actions on the last selected ones)
        resetSelection();
    }
    //----------------------------------------------------------------------
    void MainWindow::on_Settings_pushButton_OpenFolder_clicked()
    {
        //Open the selected collection folder
        QDesktopServices::openUrl(QUrl::fromLocalFile(collection->folder));
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
    void MainWindow::on_Settings_lineEdit_DatabaseFilePath_returnPressed()
    {
        QString newDatabaseFile = ui->Settings_lineEdit_DatabaseFilePath->text();

        //Unless the selection was cancelled, set the new collection folder, and refresh all data
        if ( newDatabaseFile !=""){
            collection->databaseFilePath = newDatabaseFile;
            changeDatabaseFilePath();
            ui->Settings_pushButton_DatabaseModeApplyAndRestart->setEnabled(true);
        }
        else
            ui->Settings_pushButton_DatabaseModeApplyAndRestart->setEnabled(false);

        //Reset selected values (to avoid actions on the last selected ones)
        resetSelection();
    }
    //----------------------------------------------------------------------
    void MainWindow::changeDatabaseFilePath()
    {
        //Save Settings for the new collection folder value;
        QSettings settings(collection->settingsFilePath, QSettings:: IniFormat);
        settings.setValue("Settings/DatabaseFilePath", collection->databaseFilePath);

        //Set the new path in Settings tab
        ui->Settings_lineEdit_DatabaseFilePath->setText(collection->databaseFilePath);

        collection->createStorageFile();
        collection->generateCollectionFilesPaths();
    }
    //Hosted ---------------------------------------------------------------
    void MainWindow::on_Settings_lineEdit_DataMode_Hosted_HostName_textChanged(const QString &arg1)
    {
        if(arg1 !=collection->databaseHostName)
            ui->Settings_pushButton_DatabaseModeApplyAndRestart->setEnabled(true);
    }
    //----------------------------------------------------------------------
    void MainWindow::on_Settings_lineEdit_DataMode_Hosted_DatabaseName_textChanged(const QString &arg1)
    {
        if(arg1 !=collection->databaseName)
            ui->Settings_pushButton_DatabaseModeApplyAndRestart->setEnabled(true);
    }
    //----------------------------------------------------------------------
    void MainWindow::on_Settings_lineEdit_DataMode_Hosted_Port_textChanged(const QString &arg1)
    {
        int newPort = arg1.toInt();
        if(newPort !=collection->databasePort)
            ui->Settings_pushButton_DatabaseModeApplyAndRestart->setEnabled(true);
    }
    //----------------------------------------------------------------------
    void MainWindow::on_Settings_lineEdit_DataMode_Hosted_UserName_textChanged(const QString &arg1)
    {
        if(arg1 !=collection->databaseUserName)
            ui->Settings_pushButton_DatabaseModeApplyAndRestart->setEnabled(true);
    }
    //----------------------------------------------------------------------
    void MainWindow::on_Settings_lineEdit_DataMode_Hosted_Password_textChanged(const QString &arg1)
    {
        if(arg1 !=collection->databasePassword)
            ui->Settings_pushButton_DatabaseModeApplyAndRestart->setEnabled(true);
    }
    //----------------------------------------------------------------------

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
        saveStatisticsEnabled = ui->Settings_checkBox_SaveRecordWhenUpdate->isChecked();
        settings.setValue("Settings/AutoSaveRecordWhenUpdate", saveStatisticsEnabled);
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
        collection->generateCollectionFiles();

        //Load data from files to database ("Memory" database mode
            //Create a Storage list (if none exists) + conversions
            collection->createStorageFile();

            //Clear database (when in memory)
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
        collection->insertPhysicalStorageGroup();

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
            if(newDevice->verifyParentDeviceExistsInPhysicalGroup()==true)
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
        }

        //Reload models
        loadDevicesTreeToModel("Filters");
        loadDevicesView();
        loadParentsList();

        //Load Statistics
        loadStatisticsDataTypes();
        collection->loadStatisticsDeviceFileToTable();
        loadStatisticsChart();

        //Load Tags
        reloadTagsData();

        //Verify Collection version and trigger migration to v2.0
        QFile statitsticsFile(collection->statisticsDeviceFilePath);
        if (statitsticsFile.exists()){
            collection->version = "2.0";
        }
        else{
            collection->version = "1.x";

            QMessageBox msgBox;
            QString message;
            message += "<br/>";
            message += "<b>" + tr("Collection Upgrade Required") + "</b>";
            message += "<br/><br/>";
            message += tr("Katalog has detected that the selected collection was created with an earlier version.");
            message += "<br/>";
            message += tr("<br/>Current collection folder: ") + QString::fromUtf8("<br/><b>%1</b>").arg(collection->folder);
            message += "<br/>";
            message += tr("<br/>Current collection version: ") + QString::fromUtf8("<br/><b>%1</b>").arg(collection->version);
            message += "<br/><br/><br/>";
            message += tr("To utilize this collection with Katalog 2.0, it needs to be upgraded.");
            message += "<br/><br/>";
            message += tr("The upgrade process can be performed automatically, but it is strongly advised to <b>back up the collection folder/files before proceeding</b>.");
            message += "<br/><br/>";
            message += tr("What would you like to do?");
            message += "<br/><br/>";

            msgBox.setWindowTitle("Katalog");
            msgBox.setText(message);
            msgBox.setIcon(QMessageBox::Warning);
            msgBox.setStandardButtons(QMessageBox::Yes | QMessageBox::No | QMessageBox::Cancel);
            msgBox.setButtonText(QMessageBox::Yes, "Upgrade Now");
            msgBox.setButtonText(QMessageBox::No, "Choose a Different Folder");
            msgBox.setButtonText(QMessageBox::Cancel, "Exit Application");

            int result = msgBox.exec();

            if (result == QMessageBox::Yes) {
                // Trigger migration
                migrateCollection();

            } else if (result == QMessageBox::No) {
                // Select other folder
                on_Settings_pushButton_SelectFolder_clicked();
                return;

            } else if (result == QMessageBox::Cancel) {
                // Quit app
                qApp->deleteLater();
                return;
            }
        }
    }
    //----------------------------------------------------------------------
    void MainWindow::preloadCatalogs()
    {
        collection->loadAllCatalogFiles();
    }
    //----------------------------------------------------------------------
    void MainWindow::selectDatabaseFilePath()
    {
        //Open a dialog for the user to select the directory of the collection where catalog files are stored.
        QString newDatabaseFilePath = QFileDialog::getOpenFileName(this, tr("Select the database to open:"),
                                                                   collection->databaseFilePath,"*.db");

        //Unless the selection was cancelled, set the new collection folder, and refresh all data
        if ( newDatabaseFilePath !=""){

            collection->databaseFilePath = newDatabaseFilePath;
            //Save Settings for the new collection folder value;
            QSettings settings(collection->settingsFilePath, QSettings:: IniFormat);
            settings.setValue("Settings/DatabaseFilePath", collection->databaseFilePath);

            //Set the new path in Settings tab
            ui->Settings_lineEdit_DatabaseFilePath->setText(collection->databaseFilePath);

            //ui->Settings_pushButton_DatabaseModeApplyAndRestart->setEnabled(true);
            QSqlDatabase db = QSqlDatabase::addDatabase("QSQLITE");
            db.setDatabaseName(collection->databaseFilePath);
            if (!db.open())
                qDebug()<< db.lastError();

            loadCollection();
        }
        // else
        //     ui->Settings_pushButton_DatabaseModeApplyAndRestart->setEnabled(false);

    }
    //----------------------------------------------------------------------
    void MainWindow::selectNewDatabaseFolderPath()
    {
        //Open a dialog for the user to select the directory of the collection where catalog files are stored.
        QString newDatabaseFilePath = QFileDialog::getSaveFileName(this, tr("Select the database to open:"),
                                                                   collection->folder+"/newKatalogFile.db","*.db");

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

                    //ui->Settings_pushButton_DatabaseModeApplyAndRestart->setEnabled(true);
        }

        //Reset selected values (to avoid actions on the last selected ones)
        resetSelection();
    }



