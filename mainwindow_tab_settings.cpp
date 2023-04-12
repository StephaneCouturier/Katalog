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

//SETTINGS / Data management
    void MainWindow::on_Settings_comboBox_DatabaseMode_currentTextChanged()
    {
        QString newDatabaseMode = ui->Settings_comboBox_DatabaseMode->itemData(ui->Settings_comboBox_DatabaseMode->currentIndex(),Qt::UserRole).toString();
        QSettings settings(settingsFilePath, QSettings:: IniFormat);
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
        loadCollection();
    }
    //----------------------------------------------------------------------
    void MainWindow::on_Settings_pushButton_SelectFolder_clicked()
    {
        //Open a dialog for the user to select the directory of the collection where catalog files are stored.
        QString dir = QFileDialog::getExistingDirectory(this, tr("Select the directory for this collection"),
                                                        collectionFolder,
                                                        QFileDialog::ShowDirsOnly
                                                        | QFileDialog::DontResolveSymlinks);

        //Unless the selection was cancelled, set the new collection folder, and refresh all data
        if ( dir !=""){

            collectionFolder = dir;

            //Set the new path in Settings tab
            ui->Settings_lineEdit_CollectionFolder->setText(collectionFolder);

            //Redefine the path of the Storage file
            storageFilePath = collectionFolder + "/" + "storage.csv";

            //Save Settings for the new collection folder value;
            saveSettings();

            //Load the collection from this new folder;
                //Clear database if mode is Memory
                if(databaseMode=="Memory"){
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
    void MainWindow::on_Settings_pushButton_OpenFolder_clicked()
    {
        //Open the selected collection folder
        QDesktopServices::openUrl(QUrl::fromLocalFile(collectionFolder));
    }
    //----------------------------------------------------------------------
    void MainWindow::on_Settings_checkBox_KeepOneBackUp_stateChanged()
    {
        QSettings settings(settingsFilePath, QSettings:: IniFormat);
        settings.setValue("Settings/KeepOneBackUp", ui->Settings_checkBox_KeepOneBackUp->isChecked());
    }
    //----------------------------------------------------------------------
    void MainWindow::on_Settings_checkBox_PreloadCatalogs_stateChanged(int arg1)
    {
        QSettings settings(settingsFilePath, QSettings:: IniFormat);
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
        databaseFilePath = ui->Settings_lineEdit_DatabaseFilePath->text();
        QDesktopServices::openUrl(QUrl::fromLocalFile(databaseFilePath));
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
        QSettings settings(settingsFilePath, QSettings:: IniFormat);
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
        QSettings settings(settingsFilePath, QSettings:: IniFormat);
        settings.setValue("Settings/Language", selectedLanguage);
    }
    //----------------------------------------------------------------------
    void MainWindow::on_Settings_comboBox_Theme_currentIndexChanged(int index)
    {
        QSettings settings(settingsFilePath, QSettings:: IniFormat);
        settings.setValue("Settings/Theme", index);
    }
    //----------------------------------------------------------------------
    void MainWindow::on_Settings_checkBox_SaveRecordWhenUpdate_stateChanged()
    {
        QSettings settings(settingsFilePath, QSettings:: IniFormat);
        settings.setValue("Settings/AutoSaveRecordWhenUpdate", ui->Settings_checkBox_SaveRecordWhenUpdate->isChecked());
    }
    //----------------------------------------------------------------------
    void MainWindow::on_Settings_checkBox_LoadLastCatalog_stateChanged(int arg1)
    {
        QSettings settings(settingsFilePath, QSettings:: IniFormat);
        settings.setValue("Settings/LoadLastCatalog", arg1);
    }
    //----------------------------------------------------------------------
    void MainWindow::on_Settings_pushButton_OpenSettingsFile_clicked()
    {
        //Open the selected collection folder
        QDesktopServices::openUrl(QUrl::fromLocalFile(settingsFilePath));
    }
    //----------------------------------------------------------------------
    void MainWindow::on_Settings_checkBox_BiggerIconSize_stateChanged(int arg1)
    {
        QSettings settings(settingsFilePath, QSettings:: IniFormat);
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
        ui->Catalogs_treeView_CatalogList->setIconSize(size);
        ui->Explore_treeview_Directories->setIconSize(size);
        ui->Explore_treeView_FileList->setIconSize(size);
        ui->Create_treeView_Explorer->setIconSize(size);
        ui->Storage_treeView_StorageList->setIconSize(size);
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
        QSettings settings(settingsFilePath, QSettings:: IniFormat);
        settings.setValue("Settings/CheckVersion", ui->Settings_checkBox_CheckVersion->isChecked());
    }
    //----------------------------------------------------------------------


//SETTINGS / data methods --------------------------------------------------
    void MainWindow::loadCollection()
    {
        //Generate collection files paths and statistics parameters
        generateCollectionFilesPaths();

        //Create a Storage list (if none exists) + conversions
        if(databaseMode=="Memory"){
            createStorageList();
            convertStatistics();

            //Clear database
            QSqlQuery queryDelete;
            queryDelete.exec("DELETE FROM catalog");
            queryDelete.exec("DELETE FROM storage");
            queryDelete.exec("DELETE FROM file");
            queryDelete.exec("DELETE FROM filetemp");
            queryDelete.exec("DELETE FROM folder");
            queryDelete.exec("DELETE FROM statistics");
            queryDelete.exec("DELETE FROM search");
            queryDelete.exec("DELETE FROM tag");

            //Load Files to database
            loadSearchHistoryFileToTable();
            loadCatalogFilesToTable();
            if(databaseMode=="Memory")
                loadStorageFileToTable();
        }

        //Load data from tables and update display
        loadSearchHistoryTableToModel();
        loadStorageTableToModel();
        updateStorageSelectionStatistics();
        loadCatalogsTableToModel();

        //Load Storage list
        refreshLocationSelectionList();
        refreshStorageSelectionList(selectedFilterStorageLocation);
        refreshCatalogSelectionList(selectedFilterStorageLocation, selectedFilterStorageName);
        loadStorageTableToSelectionTreeModel();

        //Add a storage device for catalogs without one

        QSqlQuery queryCatalog;
        QString queryCatalogSQL = QLatin1String(R"(
                                    SELECT count(*)
                                    FROM catalog
                                    WHERE catalog_storage=""
                                            )");
        queryCatalog.prepare(queryCatalogSQL);
        queryCatalog.exec();
        queryCatalog.next();

        QSqlQuery queryStorage;
        QString queryStorageSQL = QLatin1String(R"(
                                    SELECT count(*)
                                    FROM storage
                                    WHERE storage_name=""
                                )");
        queryStorage.prepare(queryStorageSQL);
        queryStorage.exec();
        queryStorage.next();

        if (queryCatalog.value(0).toInt() >0 and queryStorage.value(0).toInt() == 0){
            addStorageDevice(tr(""));
        }

        //Load Statistics
        loadStatisticsDataTypes();
        if(databaseMode=="Memory"){
            loadStatisticsCatalogFileToTable();
            loadStatisticsStorageFileToTable();
        }
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
                    selectedCatalog->loadCatalogFileListToTable();
        }
    }
    //----------------------------------------------------------------------
    void MainWindow::selectDatabaseFilePath()
    {
        //Open a dialog for the user to select the directory of the collection where catalog files are stored.
        QString newDatabaseFilePath = QFileDialog::getOpenFileName(this, tr("Select the database to open:"),
                                                                   collectionFolder,"*.db");


        //Unless the selection was cancelled, set the new collection folder, and refresh all data
        if ( newDatabaseFilePath !=""){

                    databaseFilePath = newDatabaseFilePath;

                    //Save Settings for the new collection folder value;
                    QSettings settings(settingsFilePath, QSettings:: IniFormat);
                    settings.setValue("Settings/DatabaseFilePath", databaseFilePath);
                    //QMessageBox msgBox; msgBox.setWindowTitle("Katalog"); msgBox.setText("newDatabaseFilePath:<br/>"+QVariant(newDatabaseFilePath).toString()); msgBox.setIcon(QMessageBox::Information); msgBox.exec();

                    //Set the new path in Settings tab
                    ui->Settings_lineEdit_DatabaseFilePath->setText(databaseFilePath);

                    createStorageList();
                    if(databaseMode=="Memory")
                        generateCollectionFilesPaths();
                    loadCollection();
        }

        //Reset selected values (to avoid actions on the last selected ones)
        resetSelection();
    }
    //----------------------------------------------------------------------
    void MainWindow::selectNewDatabaseFolderPath()
    {
        //QString newDir;
        //Open a dialog for the user to select the directory of the collection where catalog files are stored.
        QString newDatabaseFilePath = QFileDialog::getSaveFileName(this, tr("Select the database to open:"),
                                                                   collectionFolder+"/newKatalogFile.db","*.db");

        //Unless the selection was cancelled, set the new collection folder, and refresh all data
        if ( newDatabaseFilePath !=""){

                    databaseFilePath = newDatabaseFilePath;

                    QFile fileOut( databaseFilePath );
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
                    QSettings settings(settingsFilePath, QSettings:: IniFormat);
                    settings.setValue("Settings/DatabaseFilePath", databaseFilePath);
                    //QMessageBox msgBox; msgBox.setWindowTitle("Katalog"); msgBox.setText("newDatabaseFilePath:<br/>"+QVariant(newDatabaseFilePath).toString()); msgBox.setIcon(QMessageBox::Information); msgBox.exec();

                    //Set the new path in Settings tab
                    ui->Settings_lineEdit_DatabaseFilePath->setText(databaseFilePath);

                    createStorageList();
                    generateCollectionFilesPaths();
                    loadCollection();
        }

        //Reset selected values (to avoid actions on the last selected ones)
        resetSelection();
    }
