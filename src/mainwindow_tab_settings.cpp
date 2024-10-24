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
// Description: https://stephanecouturier.github.io/Katalog/docs/Features/Settings
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
        //Restart
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

        //Load the collection from this new folder;
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
    void MainWindow::on_Settings_pushButton_ExportToSQLitFile_clicked()
    {
        if (collection->databaseMode == "Memory") {

            QMessageBox msgBox;
            msgBox.setWindowTitle("Katalog");

            //Folder and file name selection

                //Default folder and file name
                QString backupFilePath = collection->folder + "/export.db"; // Path to the output file

                //Open a dialog for the user to select the directory of the collection where catalog files are stored.
                QString selectedBackupFilePath = QFileDialog::getSaveFileName(this, tr("Select the directory and file name for his export."),
                                                                backupFilePath);

                //Unless the selection was cancelled, set the new collection folder, and refresh all data
                if ( selectedBackupFilePath !=""){
                    QFile newBackupFile(selectedBackupFilePath);
                    if( newBackupFile.exists())
                        newBackupFile.moveToTrash();

                    backupFilePath = selectedBackupFilePath;
                }

            //Load all Catalogs indexes into memory

                //Prepare temporary variables
                Device tempCatalogDevice;
                QMutex tempMutex;
                bool tempStopRequested = false;

                // Get the total number of files for all devices
                QSqlQuery fileCountQuery(QSqlDatabase::database("defaultConnection"));
                QString fileCountQuerySQL = QLatin1String(R"(
                    SELECT SUM(device_total_file_count)
                    FROM device
                    WHERE device_type ="Catalog"
                    AND device_group_id = 0
                )");
                fileCountQuery.prepare(fileCountQuerySQL);
                fileCountQuery.exec();
                fileCountQuery.next();
                qint64 totalFileCount = fileCountQuery.value(0).toInt();

                // Create the progress dialog
                QProgressDialog progress("Loading devices...", "Cancel", 0, totalFileCount, this);
                progress.setWindowModality(Qt::WindowModal);
                qint64 filesLoaded = 0;

                // List all Catalogs indexes to be loaded into memory
                QSqlQuery query(QSqlDatabase::database("defaultConnection"));
                QString querySQL = QLatin1String(R"(
                    SELECT device_id, device_name, device_total_file_count
                    FROM device
                    WHERE device_type ="Catalog"
                )");
                query.prepare(querySQL);
                query.exec();

                while(query.next()){
                    int deviceId = query.value(0).toInt();
                    QString deviceName = query.value(1).toString();
                    qint64 deviceFileCount = query.value(2).toInt();

                    progress.setLabelText(QString("Loading all catalogs prior to export<br/> %1 <br/><br/> %2 files loaded out of %3" ).arg(deviceName, QLocale().toString(filesLoaded), QLocale().toString(totalFileCount)) );

                    tempCatalogDevice.ID = deviceId;
                    tempCatalogDevice.loadDevice("defaultConnection");
                    tempCatalogDevice.catalog->loadCatalogFileListToTable("defaultConnection", tempMutex, tempStopRequested);

                    filesLoaded += deviceFileCount;
                    progress.setValue(filesLoaded);

                    if (progress.wasCanceled())
                        return;
                }

            //Dump all the database in Memory to the sql File
            if (!backupMemoryDatabaseToFile("defaultConnection", backupFilePath)) {
                msgBox.setText(QCoreApplication::translate("MainWindow",
                                                           "Failed to export in-memory database to file.<br/>"
                                                           "<br/> Export file path: <br/><b>%1</b><br/>"
                                                           ).arg( backupFilePath ));
            } else {
                msgBox.setIcon(QMessageBox::Information);
                msgBox.setText(QCoreApplication::translate("MainWindow",
                                                           "Successful export of collection to SQLite database file.<br/>"
                                                           "<br/> Export file path: <br/><b>%1</b><br/>"
                                                           ).arg( backupFilePath ));
            }

            //Inform of end of process
            msgBox.exec();
        }
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
    void MainWindow::on_Settings_pushButton_Documentation_clicked()
    {
        QDesktopServices::openUrl(QUrl("https://stephanecouturier.github.io/Katalog/"));
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
    void MainWindow::on_Settings_checkBox_SettingsFileCaseSensitiveSort_stateChanged()
    {
        QSettings settings(collection->settingsFilePath, QSettings:: IniFormat);
        settings.setValue("Settings/FileCaseSensitiveSort", ui->Settings_checkBox_SettingsFileCaseSensitiveSort->isChecked());
        fileSortCaseSensitive = ui->Settings_checkBox_SettingsFileCaseSensitiveSort->isChecked();
    }
    //----------------------------------------------------------------------

//SETTINGS / data methods --------------------------------------------------
    void MainWindow::loadCollection()
    {
        collection->load();

        //Check active status and synch it
        updateAllDeviceActive();

        //Load data from tables to models and update display
        loadSearchHistoryTableToModel();
        filterFromSelectedDevice();

        //Reload models
        loadDevicesTreeToModel("Filters");
        loadParentsList();

        //Load Statistics
        loadStatisticsDataTypes();
        collection->loadStatisticsDeviceFileToTable();
        loadStatisticsChart();

        //Load Tags
        reloadTagsData();

        //Load directories to exclude
        QSqlQuery queryLoad(QSqlDatabase::database("defaultConnection"));
        QString queryLoadSQL = QLatin1String(R"(
                                        SELECT DISTINCT parameter_value2
                                        FROM parameter
                                        WHERE parameter_type ='exclude_directory'
                                        ORDER BY parameter_value2
                                )");
        if (!queryLoad.exec(queryLoadSQL)) {
            qDebug() << "Failed to execute queryLoad exclude_directory.";
            return;
        }
        QSqlQueryModel *model = new QSqlQueryModel;
        model->setQuery(std::move(queryLoad));
        QSortFilterProxyModel *proxyModel = new QSortFilterProxyModel(this);
        proxyModel->setSourceModel(model);
        ui->Create_treeView_Excluded->setModel(proxyModel);

        //Verify Collection version and trigger migration or updates
        if(collection->databaseMode=="Memory"){
            if ( collection->version < collection->appVersion and collection->version < "2.0"){
                //Migration 1.x > 2.0

                collection->version = "1.x";

                QString releaseNotesAddress = "https://github.com/StephaneCouturier/Katalog/releases/tag/v" + currentVersion;
                QString wikiAddress = "https://github.com/StephaneCouturier/Katalog/wiki/Major-release-2.0";

                QMessageBox msgBox;
                QString message;
                message += "<b>" + tr("Collection Upgrade Required") + "</b>";
                message += "<br/><br/><table><tr><td width=550>";
                message += tr("This application of 'Katalog' is in version: ") + "<b>" + currentVersion + "</b>";
                message += "<br/></td></tr><tr><td>";
                message += tr("Current collection version: ") + QString::fromUtf8("<b>%1</b>").arg(collection->version);
                message += "<br/></td></tr><tr><td>";
                message += tr("Collection folder: ") + QString::fromUtf8("<br/><i>%1</i>").arg(collection->folder);
                message += "<br/></td></tr><tr><td>";
                message += tr("This upgrade process can be performed automatically.");
                message += "<br/></td></tr><tr><td>";
                message += tr("Find out about the main changes and the migration in this <a href='%1'>Major release 2.0</a>.").arg(wikiAddress);
                message += "</td></tr><tr><td>";
                message += tr("Find the usual list of new features in the <a href='%1'>Release Notes</a>.").arg(releaseNotesAddress);
                message += "</td></tr><tr><td><br/>";
                message += tr("<span><b style='color:red;'>Back up the collection folder/files before upgrading!</b>");
                message += "</td></tr><tr><td>";
                message += tr("What should be done now?");
                message += "</td></tr></table><br/>";
                msgBox.setWindowTitle("Katalog");
                msgBox.setText(message);
                msgBox.setIcon(QMessageBox::Warning);
                msgBox.setStandardButtons(QMessageBox::Yes | QMessageBox::No | QMessageBox::Cancel);
                msgBox.setButtonText(QMessageBox::Yes, tr("Upgrade Now"));
                msgBox.setButtonText(QMessageBox::No, tr("Choose a Different Folder"));
                msgBox.setButtonText(QMessageBox::Cancel, tr("Exit Application"));

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

            if ( collection->version < collection->appVersion and collection->version >= "2.0"){
                //Apply db or file changes since 2.0
                //No changes yet.

                //Update collection version
                collection->version = collection->appVersion;
                collection->updateCollectionVersion();
                collection->saveParameterTableToFile();

                //Inform
                // QMessageBox msgBox;
                // msgBox.setWindowTitle("Katalog");
                // msgBox.setText(QCoreApplication::translate("MainWindow",
                //                                            "Updated collection to v2.1."
                //                                            ).arg( collection->version, collection->appVersion));
                // msgBox.setIcon(QMessageBox::Information);
                // msgBox.exec();
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
    }
    //----------------------------------------------------------------------
    void MainWindow::selectNewDatabaseFolderPath()
    {
        //Get last db file location if "File" mode, otherwise use collection folder
        QString newFileFolder;
        if(collection->databaseFilePath !=""){
            QFileInfo fileInfo(collection->databaseFilePath);
            newFileFolder = fileInfo.absolutePath();
        }
        else
            newFileFolder = collection->folder;

        //Open a dialog for the user to select the directory of the collection where catalog files are stored.
        QString newDatabaseFilePath = QFileDialog::getSaveFileName(this, tr("Select the database to create and open:"),
                                                                   newFileFolder + "/newKatalogFile.db","*.db");

        //Unless the selection was cancelled, set the new collection folder, and refresh all data
        if ( newDatabaseFilePath !=""){

                    collection->databaseFilePath = newDatabaseFilePath;

                    QFile fileOut(collection->databaseFilePath);
                    if (fileOut.open(QFile::WriteOnly | QFile::Text)) {
                        //create an empty file

                        QSqlDatabase db = QSqlDatabase::addDatabase("QSQLITE");
                        db.setDatabaseName(collection->databaseFilePath);
                        if (!db.open())
                            qDebug()<< db.lastError();

                        QSqlQuery q(QSqlDatabase::database("defaultConnection"));
                        q.exec(SQL_CREATE_DEVICE);
                        q.exec(SQL_CREATE_CATALOG);
                        q.exec(SQL_CREATE_STORAGE);
                        q.exec(SQL_CREATE_FILE);
                        q.exec(SQL_CREATE_FILETEMP);
                        q.exec(SQL_CREATE_FOLDER);
                        q.exec(SQL_CREATE_METADATA);
                        q.exec(SQL_CREATE_STATISTICS_DEVICE);
                        q.exec(SQL_CREATE_SEARCH);
                        q.exec(SQL_CREATE_TAG);
                        q.exec(SQL_CREATE_PARAMETER);
                    }
                    fileOut.close();

                    //Save Settings for the new collection folder value;
                    QSettings settings(collection->settingsFilePath, QSettings:: IniFormat);
                    settings.setValue("Settings/DatabaseFilePath", collection->databaseFilePath);

                    //Set the new path in Settings tab
                    ui->Settings_lineEdit_DatabaseFilePath->setText(collection->databaseFilePath);

                    loadCollection();
        }

        //Reset selected values (to avoid actions on the last selected ones)
        resetSelection();
    }
