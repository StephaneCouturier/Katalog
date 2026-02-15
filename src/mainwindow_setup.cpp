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
// File Name:   mainwindow_setup.cpp
// Purpose:     methods for the mainwindow
// Description: initiate additions to the interface including theme and loading previous settings
// Author:      Stephane Couturier
/////////////////////////////////////////////////////////////////////////////
*/

#include "mainwindow.h"
#include "ui_mainwindow.h"
#include "core/database.h"
#include <QTimer>
#include <QVersionNumber>

//Set up -------------------------------------------------------------------
    void MainWindow::setupFileContextMenus(){
        ui->Search_treeView_FilesFound->setContextMenuPolicy(Qt::CustomContextMenu);
        ui->Explore_treeView_FileList->setContextMenuPolicy(Qt::CustomContextMenu);
        ui->Explore_treeview_Directories->setContextMenuPolicy(Qt::CustomContextMenu);
        ui->Devices_treeView_DeviceList->setContextMenuPolicy(Qt::CustomContextMenu);
        ui->Filters_treeView_Devices->setContextMenuPolicy(Qt::CustomContextMenu);
        ui->Create_treeView_Excluded->setContextMenuPolicy(Qt::CustomContextMenu);
        ui->Tags_treeView_FolderTags->setContextMenuPolicy(Qt::CustomContextMenu);
    }
    //----------------------------------------------------------------------
    void MainWindow::loadSettings()
    {
        //Check if a settings file already exists. If not, it is considered first use and one gets generated
            QFile settingsFile(collection->settingsFilePath);
            QSettings settings(collection->settingsFilePath, QSettings:: IniFormat);

            firstRun =false;

            if (!settingsFile.exists())
                firstRun =true;

            collection->folder = settings.value("LastCollectionFolder").toString();

            if (collection->folder == "")
                firstRun =true;

            if (firstRun == true){
                //Create a file, with default values
                settings.setValue("LastCollectionFolder", QApplication::applicationDirPath());

                //Set Language and theme
                QString userLanguage = QLocale::system().name();
                settings.setValue("Settings/Language", userLanguage);

                ui->Settings_comboBox_Theme->setCurrentIndex(themeID); //Default theme is "Katalog Colors"

                //Get the translate theme name from combobox
                QString themeName = ui->Settings_comboBox_Theme->currentText();

                QMessageBox msgBox;
                msgBox.setWindowTitle("Katalog");
                msgBox.setText(tr("<br/><b>Welcome to Katalog!</b><br/><br/>"
                                  "It seems this is the first run.<br/><br/>"
                                  "The following Settings have been applied:<br/>"
                                  " - Language: <b>%1</b><br/> - Theme: <b>%2</b><br/><br/>You can change these in the tab %3.").arg(userLanguage,themeName,tr("Settings"))
                + tr("<br/><br/>On the next screen, pick an existing Collection folder or create a new one."));
                msgBox.setIcon(QMessageBox::Information);
                msgBox.exec();

                //Language
                ui->Settings_comboBox_Language->setCurrentText(userLanguage);

                //Collection folder choice - with validation
                bool folderSelected = false;
                while (!folderSelected) {
                    //Open a dialog for the user to select the directory of the collection where catalog files are stored.
                    QString selectedFolder = QFileDialog::getExistingDirectory(this, tr("Select the directory for this collection"),
                                                                               collection->folder,
                                                                               QFileDialog::ShowDirsOnly
                                                                                   | QFileDialog::DontResolveSymlinks);

                    //Handle user cancellation
                    if (selectedFolder.isEmpty()) {
                        //set the location of the application as a default value if a folder was not provided
                        selectedFolder = QApplication::applicationDirPath();
                    }

                    // Validate the selected folder
                    Collection::CollectionFolderStatus status = collection->validateCollectionFolder(selectedFolder, collection->databaseMode);

                    switch (status) {
                    case Collection::VALID_EMPTY:
                    case Collection::VALID_MEMORY_MODE:
                    case Collection::VALID_FILE_MODE:
                        // Valid folder - accept and continue
                        collection->folder = selectedFolder;
                        folderSelected = true;
                        break;

                    case Collection::INVALID_MEMORY_FILES:
                    case Collection::INVALID_FILE_FILES:
                    case Collection::INVALID_USER_DATA:
                    case Collection::INVALID_MIXED_DATA: {
                        // Invalid folder - show options to user
                        InvalidFolderAction action = showInvalidFolderDialog(selectedFolder, status, true);

                        switch (action) {
                        case ACTION_CREATE_SUBFOLDER: {
                            // Create new collection in subfolder
                            QString newCollectionPath = selectedFolder + "/Katalog_Collection_" +
                                                        QDateTime::currentDateTime().toString("yyyyMMdd");
                            if (QDir().mkpath(newCollectionPath)) {
                                collection->folder = newCollectionPath;
                                folderSelected = true;
                            }
                            // If creation failed, continue loop
                            break;
                        }
                        case ACTION_SELECT_DIFFERENT:
                            // Continue loop to let user select again
                            break;

                        case ACTION_USE_DEFAULT:
                            // Use application folder as fallback
                            collection->folder = QApplication::applicationDirPath();
                            folderSelected = true;
                            break;
                        case ACTION_CANCEL:
                            break;
                        }
                        break;
                    }
                    }
                }

                    //save setting
                    settings.setValue("LastCollectionFolder", collection->folder);

                //Go to Create screen
                QMessageBox msgBox2;
                msgBox2.setWindowTitle("Katalog");
                msgBox2.setText(tr("<br/><b>Ready to create a file catalog:</b><br/><br/>")
                                   + tr("1- Select an entire drive or directory, <br/>2- select options, and <br/>3- click 'Create'<br/>"));
                msgBox2.setIcon(QMessageBox::Information);
                msgBox2.exec();

                ui->tabWidget->setCurrentIndex(selectedTab);
            }

        //Load the settings to application variables

            //Collection folder
            if (firstRun != true){
                collection->folder = settings.value("LastCollectionFolder").toString();
            }

            selectedDevice->ID   = settings.value("Selection/SelectedDeviceID").toInt();
            if(selectedDevice->ID == 0) selectedDevice->type = "All";
            selectedDevice->loadDevice(m_connectionName);

            graphicStartDate = QDateTime::fromString(settings.value("Statistics/graphStartDate").toString(),"yyyy-mm-dd");

            //Restore Splitters
            if (settings.value("Settings/SplitterWidget1Size").toSize().width() !=-1 and settings.value("Settings/SplitterWidget2Size").toSize().width() !=-1){
                ui->splitter->setSizes(QList<int>() << settings.value("Settings/SplitterWidget1Size").toSize().width() << settings.value("Settings/SplitterWidget2Size").toSize().width());
            }
            if (settings.value("Search/ResultsSplitterWidget1Size").toSize().width() !=-1 and settings.value("Search/ResultsSplitterWidget2Size").toSize().width() !=-1){
                ui->Search_splitter_Results->setSizes(QList<int>() << settings.value("Search/ResultsSplitterWidget1Size").toSize().width() << settings.value("Search/ResultsSplitterWidget2Size").toSize().width());
            }
            if (settings.value("Explore/ExploreSplitterWidget1Size").toSize().width() !=-1 and settings.value("Explore/ExploreSplitterWidget2Size").toSize().width() !=-1){
                ui->Explore_splitter->setSizes(QList<int>() << settings.value("Explore/ExploreSplitterWidget1Size").toSize().width() << settings.value("Explore/ExploreSplitterWidget2Size").toSize().width());
            }

            //Filters selection
                //by default, SearchInCatalogs is enabled
                ui->Filters_checkBox_SearchInCatalogs->setChecked(true);
                ui->Filters_widget_ConnectedDrives->hide();

            //Filters selection for search Search/searchInFileCatalogsChecked
                if ( settings.value("Search/searchInFileCatalogsChecked") != ""){
                    //ui->Filters_checkBox_SearchInConnectedDrives->setChecked(false);
                    if ( settings.value("Search/searchInConnectedDriveChecked") == true){
                        ui->Filters_checkBox_SearchInConnectedDrives->setChecked(1);
                    }
                    else
                        ui->Filters_checkBox_SearchInCatalogs->setChecked(1);
                }
                else
                    ui->Filters_checkBox_SearchInCatalogs->setChecked(1);

            //Show or Hide ShowHideSearchCriteria
            if ( settings.value("Settings/ShowHideSearchCriteria") == "go-down"){ //Hide
                    ui->Search_pushButton_ShowHideSearchCriteria->setIcon(QIcon::fromTheme("go-down"));
                    ui->Search_widget_SearchCriteria->setHidden(true);
            }

            //Show or Hide ShowHideCatalogResults
            if ( settings.value("Settings/ShowHideCatalogResults") == "go-next"){ //Hide
                    ui->Search_pushButton_ShowHideCatalogResults->setIcon(QIcon::fromTheme("go-next"));
                    ui->Search_widget_ResultsCatalogs->setHidden(true);
                    ui->Search_label_CatalogsWithResults->setHidden(true);
            }

            //Show or Hide ShowHideGlobal
            if ( settings.value("Settings/ShowHideFilters") == "go-next"){ //Hide
                    ui->splitter_widget_Filters->setHidden(true);
                    ui->main_widget_ShowFilters->setHidden(false);
            }
            else{ // Show
                ui->splitter_widget_Filters->setHidden(false);
                ui->main_widget_ShowFilters->setHidden(true);
            }
            //Show or Hide ShowHideSearchHistory
            if ( settings.value("Settings/ShowHideSearchHistory") == "go-up"){ //Hide
                    ui->Search_pushButton_ShowHideSearchHistory->setIcon(QIcon::fromTheme("go-up"));
                    ui->Search_treeView_History->setHidden(true);
            }

            //Expand/Collapse device selection tree
            filtersTreeExpandState = settings.value("Selection/filtersTreeExpandState").toInt();
            deviceTreeExpandState = settings.value("Devices/deviceTreeExpandState").toInt();

            //General settings
            themeID = settings.value("Settings/Theme").toInt();
            if (themeID !=0 and themeID !=1){
                //fallback on default theme
                themeID=1;
            }

            ui->Settings_checkBox_BiggerIconSize->setChecked(settings.value("Settings/ThemeBiggerIconSize", 0).toBool());
            ui->Settings_checkBox_KeepOneBackUp->setChecked(settings.value("Settings/KeepOneBackUp", true).toBool());
            ui->Settings_comboBox_Language->setCurrentText(settings.value("Settings/Language").toString());
            ui->Settings_checkBox_CheckVersion->setChecked(settings.value("Settings/CheckVersion", true).toBool());
            ui->Settings_checkBox_PreloadCatalogs->setChecked(settings.value("Settings/PreloadCatalogs", false).toBool());
            ui->Settings_checkBox_LoadLastCatalog->setChecked(settings.value("Settings/LoadLastCatalog", false).toBool());
            ui->Settings_comboBox_Theme->setCurrentIndex(themeID);

            //Restore last statistics values
            ui->Statistics_comboBox_SelectSource->setCurrentText(settings.value("Statistics/SelectedSource").toString());
            ui->Statistics_comboBox_TypeOfData->setCurrentText(settings.value("Statistics/TypeOfData").toString());
            ui->Statistics_checkBox_DisplayEachValue->setChecked(settings.value("Statistics/DisplayEachValue").toBool());

            //last tab selected
            if(selectedTab==96)
                selectedTab = 6;
            else
                selectedTab = settings.value("Settings/selectedTab").toInt();
            ui->tabWidget->setCurrentIndex(selectedTab);

            //Restore Statistics settings
            ui->Statistics_lineEdit_GraphicStartDate->setText(graphicStartDate.toString("yyyy-mm-dd"));

            // Duplicates: default to "Within selected device", hide device selection
            ui->Search_radioButton_DuplicatesWithinSelectedDevice->setChecked(true);
            ui->Search_widget_DuplicatesDevices->setHidden(true);

            //Restore last sort order for the catalogs and storage
            lastDevicesSortSection        = settings.value("Devices/lastDevicesSortSection").toInt();
            lastDevicesSortOrder          = settings.value("Devices/lastDevicesSortOrder").toInt();
            lastStorageSortSection        = settings.value("Storage/lastStorageSortSection").toInt();
            lastStorageSortOrder          = settings.value("Storage/lastStorageSortOrder").toInt();
            lastExploreSortSection        = settings.value("Explore/lastExploreSortSection").toInt();
            lastExploreSortOrder          = settings.value("Explore/lastExploreSortOrder").toInt();
            lastSearchSortSection         = settings.value("Search/lastSearchSortSection").toInt();
            lastSearchSortOrder           = settings.value("Search/lastSearchSortOrder").toInt();
            lastSearchHistorySortSection  = settings.value("Search/lastSearchHistorySortSection").toInt();
            lastSearchHistorySortOrder    = settings.value("Search/lastSearchHistorySortOrder").toInt();

            optionDisplayPhysicalGroup    = settings.value("Devices/DisplayPhysicalGroup", true).toBool();
            optionDisplayVirtualGroups    = settings.value("Devices/DisplayVirtualGroups", true).toBool();
            optionDisplayStorage          = settings.value("Devices/DisplayStorage", true).toBool();
            optionDisplayCatalogs         = settings.value("Devices/DisplayCatalogs", true).toBool();
            optionDisplayFullDeviceTable  = settings.value("Devices/DisplayFullDeviceTable", false).toBool();
            optionDisplayFullMappingTable = settings.value("BackUp/DisplayFullMappingTable", false).toBool();

            optionDisplayFolders          = settings.value("Explore/DisplayFolders").toBool();
            optionDisplaySubFolders       = settings.value("Explore/DisplaySubFolders").toBool();

            //Restore other settings
            fileSortCaseSensitive       = settings.value("Settings/FileCaseSensitiveSort").toBool();
            ui->Settings_checkBox_SettingsFileCaseSensitiveSort->setChecked(fileSortCaseSensitive);

            //BackUp Tab
            QString filterMappingTable = settings.value("BackUp/FilterMappingTable", "Source").toString();
            if(filterMappingTable=="Target"){
                ui->BackUp_radioButton_Target->setChecked(true);
            }
            ui->BackUp_checkBox_OnlySelectedLinks->setChecked(
                settings.value("BackUp/OnlySelectedLinks", false).toBool()
                );
    }
    //----------------------------------------------------------------------
    void MainWindow::hideDevelopmentUIItems()
    {
        //Devices
            //Catalogs
                //DEV: preparing catalog-device relation
                ui->Catalogs_checkBox_isFullDevice->hide();

        //Create
            //DEV: the option to include symblinks is not working yet
            ui->Create_checkBox_IncludeSymblinks->hide();
            ui->Create_checkBox_isFullDevice->hide();

            //DEV: Samba is not implemented yet
            ui->Create_label_TypeOfSource->hide();
            ui->Create_comboBox_SourceType->hide();
            ui->Create_label_Path->hide();
    }
    //----------------------------------------------------------------------
    void MainWindow::checkVersion()
    {
        //Get the number of the lastest Version
        QString lastestVersion;
        QString htmlPage;
        QString downloadAddress = "https://github.com/StephaneCouturier/Katalog/releases/latest";
        //NOTES:
        // github will redirect this address to the actual lastest release page.
        // The event will return a message containing this latest release exact address;
        // This address contains the release tag, which is the number to get.

        //Get html message
        QNetworkAccessManager manager;
        QNetworkReply *response = manager.get(QNetworkRequest(QUrl(downloadAddress)));
        QEventLoop event;
        connect(response,SIGNAL(finished()),&event,SLOT(quit()));
        event.exec();
        htmlPage = response->readAll();

        //Parse html text, search, and return the release number
        QString searchString1 = "/Katalog/releases/tag/v";
        QStringList lineValues;
        QTextStream stream(&htmlPage);
        while (!stream.atEnd())
        {
            //Read the next line
            QString line = stream.readLine();

            //Verify it contains the search string
            if (line.contains(searchString1, Qt::CaseSensitive)) {

                //get value
                lineValues = line.split(searchString1);
                lineValues = lineValues[1].split("\"");
                lastestVersion = lineValues[0];

                QString releaseNotesAddress = "https://github.com/StephaneCouturier/Katalog/releases/tag/v" + lastestVersion;

                //inform user if new version is available, and give the choice to download it
                if ( QVersionNumber::fromString(lastestVersion) > QVersionNumber::fromString(currentVersion) ){

                    int result = QMessageBox::information(this,"Katalog",
                                    tr("This is version: v%1 <br/><br/>A new version is available: <b>v%2</b> <br/> "
                                    "Find the list of new features in the <a href='%3'>Release Notes</a><br/><br/>"
                                    "Do you want to download it?")
                                    .arg(currentVersion,lastestVersion, releaseNotesAddress),
                                    QMessageBox::Yes|QMessageBox::Cancel);

                    if ( result ==QMessageBox::Yes){
                        QDesktopServices::openUrl(QUrl("https://sourceforge.net/projects/katalogg/files/latest/download"));
                    }
                }
                return;
            }
        }
    }
    //----------------------------------------------------------------------

    //--- Database management -----------------------------------------------
    void MainWindow::runDatabaseMigrations()
    {
        QString currentSchemaVersion = collection->loadDatabaseSchemaVersion();
        qDebug() << "Current database schema version:" << currentSchemaVersion;
        qDebug() << "App version:" << collection->appVersion;

        //Run in ascending version order
            QVersionNumber schemaVersion = QVersionNumber::fromString(currentSchemaVersion);

            if (schemaVersion < QVersionNumber::fromString("2.6")) {
                qDebug() << "Running database migration to 2.6...";
                collection->dbSchemaVersion = "2.6";

                QSqlError migrationError = Database::runMigration_2_6(m_connectionName);
                if (migrationError.type() == QSqlError::NoError) {
                    collection->setDatabaseSchemaVersion();
                    qDebug() << "Database migration to 2.6 completed";
                    migrateExistingSearchDeviceData_2_6();
                } else {
                    qDebug() << "Database migration to 2.6 failed:" << migrationError.text();
                    return;
                }
            }

            if (schemaVersion < QVersionNumber::fromString("2.8")) {
                qDebug() << "Running SAFE database migration to 2.8...";
                collection->dbSchemaVersion = "2.8";

                // Use the safe migration method
                QApplication::setOverrideCursor(Qt::BusyCursor);
                QSqlError migrationError = Database::runMigration_2_8(m_connectionName);
                QApplication::restoreOverrideCursor();
                if (migrationError.type() == QSqlError::NoError) {
                    collection->setDatabaseSchemaVersion();
                    qDebug() << "SAFE database migration to 2.8 completed";
                } else {
                    qDebug() << "Database migration to 2.8 failed:" << migrationError.text();
                    QMessageBox::critical(this, "Migration Failed",
                                          QString("Database migration failed: %1\n\n"
                                                  "Your original database backup is safe.\n"
                                                  "Contact support for assistance.").arg(migrationError.text()));
                    return;
                }
            }

            if (schemaVersion < QVersionNumber::fromString("2.9")) {
                qDebug() << "Running database migration to 2.9...";
                collection->dbSchemaVersion = "2.9";

                QApplication::setOverrideCursor(Qt::BusyCursor);
                QSqlError migrationError = Database::runMigration_2_9(m_connectionName);
                QApplication::restoreOverrideCursor();
                if (migrationError.type() == QSqlError::NoError) {
                    collection->setDatabaseSchemaVersion();
                    qDebug() << "Database migration to 2.9 completed";
                } else {
                    qDebug() << "Database migration to 2.9 failed:" << migrationError.text();
                    QMessageBox::critical(this, "Migration Failed",
                                          QString("Database migration to 2.9 failed: %1\n\n"
                                                  "Please check the logs and contact support if needed.")
                                              .arg(migrationError.text()));
                    return;
                }
            }

        // Refresh display
        loadSearchHistoryTableToModel();
    }
   //----------------------------------------------------------------------
    void MainWindow::migrateExistingSearchDeviceData_2_6()
    {
        qDebug() << "Migrating existing search history device data...";

        QSqlQuery updateQuery(QSqlDatabase::database(m_connectionName));
        QSqlQuery selectQuery(QSqlDatabase::database(m_connectionName));

        // Get all search records that need migration
        selectQuery.exec(R"(
        SELECT date_time, search_catalog, search_storage, search_catalog_checked
        FROM search
        WHERE selected_device_ID_list IS NULL OR selected_device_ID_list = ''
    )");

        while (selectQuery.next()) {
            QString dateTime = selectQuery.value(0).toString();
            QString catalogName = selectQuery.value(1).toString();
            QString storageName = selectQuery.value(2).toString();
            bool searchInCatalogs = selectQuery.value(3).toBool();

            QString deviceIdList = "";

            if (searchInCatalogs) {
                // Find device ID by catalog or storage name
                QSqlQuery deviceQuery(QSqlDatabase::database(m_connectionName));

                if (catalogName != tr("All") && !catalogName.isEmpty()) {
                    // Look for specific catalog (only Catalog type)
                    deviceQuery.prepare(R"(
                    SELECT device_id FROM device
                    WHERE device_name = :name AND device_type = 'Catalog'
                )");
                    deviceQuery.bindValue(":name", catalogName);
                }
                else if (storageName != tr("All") && !storageName.isEmpty()) {
                    // Look for storage device (only Storage type)
                    deviceQuery.prepare(R"(
                    SELECT device_id FROM device
                    WHERE device_name = :name AND device_type = 'Storage'
                )");
                    deviceQuery.bindValue(":name", storageName);
                }
                else {
                    // "All" devices - use device ID 0 as convention
                    deviceIdList = "0";
                }

                if (deviceIdList.isEmpty()) {
                    if (deviceQuery.exec() && deviceQuery.next()) {
                        deviceIdList = QString::number(deviceQuery.value(0).toInt());
                    }
                    else {
                        // Device not found, use 0 as fallback for "All"
                        deviceIdList = "0";
                        qDebug() << "Warning: Could not find device for catalog:" << catalogName << "storage:" << storageName;
                    }
                }
            }
            else {
                // Directory search - no specific device, use 0
                deviceIdList = "0";
            }

            // Update the record with the device ID
            updateQuery.prepare(R"(
            UPDATE search
            SET selected_device_ID_list = :device_list
            WHERE date_time = :date_time
        )");
            updateQuery.bindValue(":device_list", deviceIdList);
            updateQuery.bindValue(":date_time", dateTime);

            if (!updateQuery.exec()) {
                qDebug() << "Failed to update search record:" << updateQuery.lastError().text();
            }
        }

        qDebug() << "Search history device data migration completed";
    }
    //----------------------------------------------------------------------
    bool MainWindow::backupMemoryDatabaseToFile(
        const QString &memoryConnectionName, const QString &filePath)
    {
        // Open the file-based database (destination)
        QSqlDatabase fileDb = QSqlDatabase::addDatabase("QSQLITE", "file_connection");
        fileDb.setDatabaseName(filePath);

        if (!fileDb.open()) {
            QMessageBox::warning(nullptr, "Database Error", "Unable to open file-based database: " + fileDb.lastError().text());
            return false;
        }

        // Get the in-memory database
        QSqlDatabase memoryDb = QSqlDatabase::database(memoryConnectionName, false);

        if (!memoryDb.isOpen()) {
            QMessageBox::warning(nullptr, "Database Error", "In-memory database is not open.");
            return false;
        }

        // Dump schema and data from in-memory database
        // Note: sqlite_master is appropriate here as this is a SQLite-to-SQLite backup operation
        QSqlQuery queryTableList(memoryDb);
        if (!queryTableList.exec("SELECT name, sql FROM sqlite_master WHERE type='table'")) {
            QMessageBox::warning(nullptr, "Database Error", "Error retrieving schema from in-memory database: " + queryTableList.lastError().text());
            return false;
        }

        //Display progress
        //Prepare temporary variables

        // Get the total number of files for all devices
        QSqlQuery tableCountQuery(QSqlDatabase::database(m_connectionName));
        QString tableCountQuerySQL = QLatin1String(R"(
                        SELECT COUNT(*) FROM sqlite_master WHERE type='table'
                    )");
        tableCountQuery.prepare(tableCountQuerySQL);
        tableCountQuery.exec();
        tableCountQuery.next();
        qint64 totalTableCount = tableCountQuery.value(0).toInt();

        QProgressDialog progress("Dumping Tables...", "Cancel", 0, totalTableCount, this);
        progress.setWindowModality(Qt::WindowModal);
        progress.setMinimumDuration(0); // This will make the dialog appear immediately
        progress.setValue(0);
        qint64 tablesDumped = 0;

        // Apply schema and data to the file-based database
        QSqlQuery fileDbQuery(fileDb);
        while (queryTableList.next()) {
            QString tableName = queryTableList.value(0).toString();
            QString createTableSQL = queryTableList.value(1).toString();

            progress.setLabelText(QString("Dumping table <br/> %1 <br/><br/> %2 table(s) dumped out of %3" ).arg(tableName, QLocale().toString(tablesDumped), QLocale().toString(totalTableCount)) );
            progress.setValue(tablesDumped);

            QCoreApplication::processEvents();

            //Process table copy, except for the system table sqlite_sequence
            if (tableName != "sqlite_sequence")
            {
                qDebug()<<tablesDumped;
                // Create table in file-based database
                if (!fileDbQuery.exec(createTableSQL)) {
                    QMessageBox::warning(nullptr, "Database Error", "Error creating table in file-based database: " + fileDbQuery.lastError().text());
                    return false;
                }

                // Copy data from in-memory to file-based database
                QSqlQuery dataQuery(memoryDb);
                QString selectDataSQL = QString("SELECT * FROM %1").arg(tableName);
                if (!dataQuery.exec(selectDataSQL)) {
                    QMessageBox::warning(nullptr, "Database Error", "Error retrieving data from in-memory database: " + dataQuery.lastError().text());
                    return false;
                }

                // Prepare insert query for file-based database
                QString insertSQL = QString("INSERT INTO %1 VALUES (").arg(tableName);
                for (int i = 0; i < dataQuery.record().count(); ++i) {
                    if (i > 0) insertSQL += ", ";
                    insertSQL += "?";
                }
                insertSQL += ")";

                QSqlQuery insertQuery(fileDb);

                fileDbQuery.exec("BEGIN TRANSACTION");
                while (dataQuery.next()) {
                    insertQuery.prepare(insertSQL);
                    for (int i = 0; i < dataQuery.record().count(); ++i) {
                        insertQuery.addBindValue(dataQuery.value(i));
                    }

                    if (!insertQuery.exec()) {
                        QMessageBox::warning(nullptr, "Database Error", "Error inserting data: " + insertQuery.lastError().text());
                        return false;
                    }
                }
                fileDbQuery.exec("COMMIT");
            }

            if (progress.wasCanceled())
                return false;

            tablesDumped += 1;
            progress.setValue(tablesDumped);
        }

        fileDb.close();
        return true;
    }
    //----------------------------------------------------------------------

    //--- Theme management -------------------------------------------------
    void MainWindow::loadCustomThemeLight()
    {
        //Standard colors:
        //blue light	39b2e5
        //blue dark		10a2df  0D79A6
        //blue lightest e9f7fc
        //green light	81d41a
        //green dark	43bf0c
        //orange light	ff8000
        //orange dark	e36600
        //purple light	a1467e
        //purple dark	8b1871

        //Tab widget, including combo boxes and buttons
        themeColor="blue";
        if(themeColor=="dev"){
            QFile file(":styles/tabwidget_dev_light.css");
            if (file.open(QFile::ReadOnly)) {
                QString tabwidgetStyleSheet = QLatin1String(file.readAll());
                ui->tabWidget->setStyleSheet(tabwidgetStyleSheet);
                file.close();
            }
        }
        else if(themeColor=="blue"){
            QFile file(":styles/tabwidget_blue_light.css");
            if (file.open(QFile::ReadOnly)) {
                QString tabwidgetStyleSheet = QLatin1String(file.readAll());
                ui->tabWidget->setStyleSheet(tabwidgetStyleSheet);
                file.close();
            }
        }

        //Filters widget
        ui->main_widget_ShowFilters->setStyleSheet(
            "QPushButton           { text-align: left; padding: 5px 4px; margin: 0px; border: 1px solid #ccc; border-radius: 5px;	padding: 5px;} "
            "QPushButton::hover    { background: #39b2e5; color: #fff; border: 1px solid #39b2e5; 	border-radius: 5px;	padding: 5px;}"
            "QPushButton::pressed  { background: #0D79A6; color: #fff; border: 1px solid #10a2df; 	border-radius: 5px;	padding: 5px;}"
            );

        ui->Filters_widget_Hide->setStyleSheet(
            "QPushButton           { text-align: left; padding: 5px 4px; margin: 0px; border: 1px solid #ccc; border-radius: 5px;	padding: 5px;} "
            "QPushButton::hover    { background: #39b2e5; color: #fff; border: 1px solid #39b2e5; 	border-radius: 5px;	padding: 5px;}"
            "QPushButton::pressed  { background: #0D79A6; color: #fff; border: 1px solid #10a2df; 	border-radius: 5px;	padding: 5px;}"

            );

        ui->Filters_label_Selection->setStyleSheet(
            "color: #095676;"
            );

        ui->Filters_widget->setStyleSheet(
            "QComboBox             { background-color: #333; padding-left: 6px; }"
            "QLabel                { color: #095676; }"        // Title labels (blue)
            "QLabel[objectName*=\"Display\"] { color: #333; }"
            "QTabBar::tab          { height: 30px; }"
            "QTabWidget::tab-bar   { left: 0px; }"
            "QTabWidget            { padding: 0px; margin: 0px; }"

            "QPushButton           { text-align: left; padding: 5px 4px; margin: 0px; border: 1px solid #ccc; border-radius: 5px;	padding: 5px;} "
            "QPushButton::hover    { background: #39b2e5; color: #fff; border: 1px solid #39b2e5; 	border-radius: 5px;	padding: 5px;}"
            "QPushButton::pressed  { background: #0D79A6; color: #fff; border: 1px solid #10a2df; 	border-radius: 5px;	padding: 5px;}"
            );

        //Colored buttons
        ui->Search_pushButton_Search->setStyleSheet(
            "QPushButton           { background-color: #81d41a; } "
            "QPushButton::hover    { background: #39b2e5; color: #fff; border: 1px solid #39b2e5; border-radius: 5px;	padding: 5px;}"
            "QPushButton::pressed  { background: #39b2e5; color: #fff; border: 1px solid #39b2e5; border-radius: 5px;	padding: 5px;}"
            );
        ui->Catalogs_pushButton_UpdateActiveDevice->setStyleSheet(
            "QPushButton           { background-color: #ff8000; } "
            "QPushButton::hover    { background: #39b2e5; color: #fff; border: 1px solid #39b2e5; border: 1px solid #39b2e5; 	border-radius: 5px;	padding: 5px;}"
            "QPushButton::pressed  { background: #39b2e5; color: #fff; border: 1px solid #39b2e5; border: 1px solid #39b2e5; 	border-radius: 5px;	padding: 5px;}"
            "QPushButton::disabled { background-color: #BBB; border: 1px solid #AAA; border-radius: 5px;	padding: 5px;}"
            );
        ui->Catalogs_pushButton_UpdateAllActive->setStyleSheet(
            "QPushButton           { background-color: #ff8000; } "
            "QPushButton::hover    { background: #39b2e5; color: #fff; border: 1px solid #39b2e5; border: 1px solid #39b2e5; 	border-radius: 5px;	padding: 5px;}"
            "QPushButton::pressed  { background: #39b2e5; color: #fff; border: 1px solid #39b2e5; border: 1px solid #39b2e5; 	border-radius: 5px;	padding: 5px;}"
            "QPushButton::disabled { background-color: #BBB; border: 1px solid #AAA; border-radius: 5px;	padding: 5px;}"
            );
        ui->Create_pushButton_CreateCatalog->setStyleSheet(
            "QPushButton           { background-color: #81d41a; padding-right: 20px; } "
            "QPushButton::hover    { background: #39b2e5; color: #fff; border: 1px solid #39b2e5; border: 1px solid #39b2e5; 	border-radius: 5px;	padding: 5px;}"
            "QPushButton::pressed  { background: #39b2e5; color: #fff; border: 1px solid #39b2e5; border: 1px solid #39b2e5; 	border-radius: 5px;	padding: 5px;}"
            );
        ui->Settings_pushButton_DatabaseModeApplyAndRestart->setStyleSheet(
            "QPushButton           { background-color: #ff8000; } "
            "QPushButton::hover    { background: #39b2e5; color: #fff; border: 1px solid #39b2e5; border: 1px solid #39b2e5; 	border-radius: 5px;	padding: 5px;}"
            "QPushButton::pressed  { background: #39b2e5; color: #fff; border: 1px solid #39b2e5; border: 1px solid #39b2e5; 	border-radius: 5px;	padding: 5px;}"
            "QPushButton::disabled { background-color: #BBB; border: 1px solid #AAA; border-radius: 5px;	padding: 5px;}"
            );

        //Lines
        ui->Filters_line_2->setStyleSheet("QFrame { color: #39b2e5; border-top: 1px solid #39b2e5;} ");
        ui->Search_line_SeparateResults->setStyleSheet("QFrame { color: #39b2e5; border-top: 1px solid #39b2e5;} ");
        ui->Devices_line_SeparateTop->setStyleSheet("QFrame { color: #39b2e5; border-top: 1px solid #39b2e5;} ");
        ui->Devices_line_separateButtons->setStyleSheet("QFrame { color: #39b2e5; border-top: 1px solid #39b2e5;} ");
        ui->Explore_line_1->setStyleSheet("QFrame { color: #39b2e5; border-top: 1px solid #39b2e5;} ");
        ui->Create_hline_01->setStyleSheet("QFrame { color: #39b2e5; border-top: 1px solid #39b2e5;} ");
        ui->Create_hline_02->setStyleSheet("QFrame { color: #39b2e5; border-top: 1px solid #39b2e5;} ");
        ui->Create_hline_03->setStyleSheet("QFrame { color: #39b2e5; border-top: 1px solid #39b2e5;} ");
        ui->Statistics_line_Separate->setStyleSheet("QFrame { color: #39b2e5; border-top: 1px solid #39b2e5;} ");
        ui->Settings_line_1->setStyleSheet("QFrame { color: #39b2e5; border-top: 1px solid #39b2e5; } ");
        ui->Settings_line_2->setStyleSheet("QFrame { color: #39b2e5; border-top: 1px solid #39b2e5;} ");
        ui->Create_hline_01->setStyleSheet("QFrame { color: #39b2e5; border-top: 1px solid #39b2e5;} ");

        //Doted lines on Search screen
        ui->Search_label_LinkImage01->setStyleSheet("QLabel { background: url(:/images/link_blue/link-tree-mid.png) repeat-y left; } ");
        ui->Search_label_LinkImage02->setStyleSheet("QLabel { background: url(:/images/link_blue/link-v.png) repeat-y left; } ");
        ui->Search_label_LinkImage03->setStyleSheet("QLabel { background: url(:/images/link_blue/link-tree-mid.png) repeat-y left; } ");
        ui->Search_label_LinkImage04->setStyleSheet("QLabel { background: url(:/images/link_blue/link-h.png) repeat-x left; } ");
        ui->Search_label_LinkImage05->setStyleSheet("QLabel { background: url(:/images/link_blue/link-v.png) repeat-y left; } ");
        ui->Search_label_LinkImage06->setStyleSheet("QLabel { background: url(:/images/link_blue/link-tree-mid.png) repeat-y left; } ");
        ui->Search_label_LinkImage07->setStyleSheet("QLabel { background: url(:/images/link_blue/link-tree-mid.png) repeat-y left; } ");
        ui->Search_label_LinkImage08->setStyleSheet("QLabel { background: url(:/images/link_blue/link-tree-end.png) no-repeat left; } ");
        ui->Search_label_LinkImage09->setStyleSheet("QLabel { background: url(:/images/link_blue/link-tree-end.png) repeat-x left; } ");
        ui->Search_label_LinkImage10->setStyleSheet("QLabel { background: url(:/images/link_blue/link-h.png) repeat-x left; } ");
        ui->Search_label_LinkImage11->setStyleSheet("QLabel { background: url(:/images/link_blue/link-tree-mid.png) repeat-y left; } ");
        ui->Search_label_LinkImage12->setStyleSheet("QLabel { background: url(:/images/link_blue/link-h.png) repeat-x left; } ");
        ui->Search_label_LinkImage13->setStyleSheet("QLabel { background: url(:/images/link_blue/link-tree-mid.png) repeat-y left; } ");
        ui->Search_label_LinkImage14->setStyleSheet("QLabel { background: url(:/images/link_blue/link-h.png) repeat-x left; } ");
        ui->Search_label_LinkImage15->setStyleSheet("QLabel { background: url(:/images/link_blue/link-v.png) repeat-y left; } ");
        ui->Search_label_LinkImage16->setStyleSheet("QLabel { background: url(:/images/link_blue/link-v.png) repeat-y left; } ");
        ui->Search_label_LinkImage17->setStyleSheet("QLabel { background: url(:/images/link_blue/link-tree-end.png) no-repeat left; } ");
        ui->Search_label_LinkImage18->setStyleSheet("QLabel { background: url(:/images/link_blue/link-v.png) repeat-y left; } ");
        ui->Search_label_LinkImage19->setStyleSheet("QLabel { background: url(:/images/link_blue/link-v.png) repeat-y left; } ");
        ui->Search_label_LinkImage20->setStyleSheet("QLabel { background: url(:/images/link_blue/link-tree-end.png) no-repeat left; } ");
        ui->Search_label_LinkImage21->setStyleSheet("QLabel { background: url(:/images/link_blue/link-v.png) repeat-y left; } ");
        ui->Search_label_LinkImage22->setStyleSheet("QLabel { background: url(:/images/link_blue/link-v.png) repeat-y left; } ");
        ui->Search_label_LinkImage23->setStyleSheet("QLabel { background: url(:/images/link_blue/link-v.png) repeat-y left; } ");
        ui->Search_label_LinkImage24->setStyleSheet("QLabel { background: url(:/images/link_blue/link-tree-mid.png) repeat-y left; } ");
        ui->Search_label_LinkImage30->setStyleSheet("QLabel { background: url(:/images/link_blue/link-v.png) repeat-y left; } ");
        ui->Search_label_LinkImage31->setStyleSheet("QLabel { background: url(:/images/link_blue/link-v.png) repeat-y left; } ");
        ui->Search_label_LinkImage32->setStyleSheet("QLabel { background: url(:/images/link_blue/link-v.png) repeat-y left; } ");
        ui->Search_label_LinkImage33->setStyleSheet("QLabel { background: url(:/images/link_blue/link-v.png) repeat-y left; } ");
        ui->Search_label_LinkImage16_7->setStyleSheet("QLabel { background: url(:/images/link_blue/link-v.png) repeat-y left; } ");
        ui->Search_label_LinkImage16_8->setStyleSheet("QLabel { background: url(:/images/link_blue/link-v.png) repeat-y left; } ");
        ui->Search_label_LinkImage16_3->setStyleSheet("QLabel { background: url(:/images/link_blue/link-v.png) repeat-y left; } ");
        ui->Search_label_LinkImage16_9->setStyleSheet("QLabel { background: url(:/images/link_blue/link-v.png) repeat-y left; } ");
        ui->Search_label_LinkImage16_2->setStyleSheet("QLabel { background: url(:/images/link_blue/link-v.png) repeat-y left; } ");
        ui->Search_label_LinkImage16_15->setStyleSheet("QLabel { background: url(:/images/link_blue/link-v.png) repeat-y left; } ");
        ui->Search_label_LinkImage26->setStyleSheet("QLabel { background: url(:/images/link_blue/link-tree-mid.png) repeat-y left; } ");
        ui->Search_label_LinkImage27->setStyleSheet("QLabel { background: url(:/images/link_blue/link-tree-mid.png) repeat-y left; } ");
        ui->Search_label_LinkImage28->setStyleSheet("QLabel { background: url(:/images/link_blue/link-tree-end.png) no-repeat left; } ");

        //Change the alternate color of treeview lines
        ui->Filters_treeView_Devices->setStyleSheet(
            "QTreeView { alternate-background-color: #e9f7fc;}"
            );
        //Search
        ui->Search_treeView_FilesFound->setStyleSheet(
            "QTreeView { alternate-background-color: #e9f7fc;}"
            );
        ui->Search_treeView_CatalogsFound->setStyleSheet(
            "QTreeView { alternate-background-color: #e9f7fc;}"
            );
        ui->Search_treeView_History->setStyleSheet(
            "QTreeView { alternate-background-color: #e9f7fc;}"
            );

        //Devices
        ui->Devices_treeView_DeviceList->setStyleSheet(
            "QTreeView { alternate-background-color: #e9f7fc;}"
            );

        //Explore
        ui->Explore_treeview_Directories->setStyleSheet(
            "QTreeView { alternate-background-color: #e9f7fc;}"
            );
        ui->Explore_treeView_FileList->setStyleSheet(
            "QTreeView { alternate-background-color: #e9f7fc;}"
            );
        //Create
        ui->Create_treeView_Explorer->setStyleSheet(
            "QTreeView { alternate-background-color: #e9f7fc;}"
            );
        ui->Create_treeView_Excluded->setStyleSheet(
            "QTreeView { alternate-background-color: #e9f7fc;}"
            );
        //Tags
        ui->Tags_treeview_Explorer->setStyleSheet(
            "QTreeView { alternate-background-color: #e9f7fc;}"
            );
        ui->Tags_listView_ExistingTags->setStyleSheet(
            "QListView { alternate-background-color: #e9f7fc;}"
            );
        ui->Tags_treeView_FolderTags->setStyleSheet(
            "QTreeView { alternate-background-color: #e9f7fc;}"
            );
        //BackUp
        ui->BackUp_tableView_CurrentMappings->setStyleSheet(
            "QTreeView { alternate-background-color: #e9f7fc;}"
            );
        ui->BackUp_treeView_List1->setStyleSheet(
            "QTreeView { alternate-background-color: #e9f7fc;}"
            );
        ui->BackUp_treeView_List2->setStyleSheet(
            "QTreeView { alternate-background-color: #e9f7fc;}"
            );

        QString additionalStyles = ui->tabWidget->styleSheet() +
                                   "QTabWidget QLabel { color: #095676; }"
                                   "QTabWidget QLabel[objectName*=\"Value\"], "
                                   "QTabWidget QLabel[objectName*=\"Display\"] { color: #333; }";
        ui->tabWidget->setStyleSheet(additionalStyles);
    }
    //----------------------------------------------------------------------
    void MainWindow::loadCustomThemeDark()
    {
        //Standard colors:
        //blue light	39b2e5
        //blue dark		10a2df  0D79A6
        //blue alternate 161b1d
        //green light	81d41a
        //green dark	43bf0c
        //orange light	ff8000
        //orange dark	e36600
        //purple light	a1467e
        //purple dark	8b1871

        //Tab widget, including combo boxes and buttons
        themeColor="blue";
        if(themeColor=="dev"){
            QFile file(":styles/tabwidget_dev_light.css");
            if (file.open(QFile::ReadOnly)) {
                QString tabwidgetStyleSheet = QLatin1String(file.readAll());
                ui->tabWidget->setStyleSheet(tabwidgetStyleSheet);
                file.close();
            }
        }
        else if(themeColor=="blue"){
            QFile file(":styles/tabwidget_blue_dark.css");
            if (file.open(QFile::ReadOnly)) {
                QString tabwidgetStyleSheet = QLatin1String(file.readAll());
                ui->tabWidget->setStyleSheet(tabwidgetStyleSheet);
                file.close();
            }
        }

        //Filters widget
        ui->main_widget_ShowFilters->setStyleSheet(
            "QPushButton           { text-align: left; padding: 5px 4px; margin: 0px; border: 1px solid #ccc; border-radius: 5px;	padding: 5px;} "
            "QPushButton::hover    { background: #39b2e5; color: #fff; border: 1px solid #39b2e5; 	border-radius: 5px;	padding: 5px;}"
            "QPushButton::pressed  { background: #0D79A6; color: #fff; border: 1px solid #10a2df; 	border-radius: 5px;	padding: 5px;}"
            );

        ui->Filters_widget_Hide->setStyleSheet(
            "QPushButton           { text-align: left; padding: 5px 4px; margin: 0px; border: 1px solid #ccc; border-radius: 5px;	padding: 5px;} "
            "QPushButton::hover    { background: #39b2e5; color: #fff; border: 1px solid #39b2e5; 	border-radius: 5px;	padding: 5px;}"
            "QPushButton::pressed  { background: #0D79A6; color: #fff; border: 1px solid #10a2df; 	border-radius: 5px;	padding: 5px;}"

            );

        ui->Filters_label_Selection->setStyleSheet(
            "color: #39b2e5;"
            );

        ui->Filters_widget->setStyleSheet(
            "QComboBox             { background-color: #FFF; padding-left: 6px; }"
            "QLabel                { color: #39b2e5; }"        // Title labels (blue)
            "QLabel[objectName*=\"Display\"] { color: #FFF; }"

            "QTabBar::tab          { height: 30px; }"
            "QTabWidget::tab-bar   { left: 0px; }"
            "QTabWidget            { padding: 0px; margin: 0px; }"

            "QPushButton           { text-align: left; padding: 5px 4px; margin: 0px; border: 1px solid #ccc; border-radius: 5px;	padding: 5px;} "
            "QPushButton::hover    { background: #39b2e5; color: #fff; border: 1px solid #39b2e5; 	border-radius: 5px;	padding: 5px;}"
            "QPushButton::pressed  { background: #0D79A6; color: #fff; border: 1px solid #10a2df; 	border-radius: 5px;	padding: 5px;}"
            );

        //Colored buttons
        ui->Search_pushButton_Search->setStyleSheet(
            "QPushButton           { background-color: #81d41a; } "
            "QPushButton::hover    { background: #39b2e5; color: #fff; border: 1px solid #39b2e5; border-radius: 5px;	padding: 5px;}"
            "QPushButton::pressed  { background: #39b2e5; color: #fff; border: 1px solid #39b2e5; border-radius: 5px;	padding: 5px;}"
            );
        ui->Catalogs_pushButton_UpdateActiveDevice->setStyleSheet(
            "QPushButton           { background-color: #ff8000; } "
            "QPushButton::hover    { background: #39b2e5; color: #fff; border: 1px solid #39b2e5; border: 1px solid #39b2e5; 	border-radius: 5px;	padding: 5px;}"
            "QPushButton::pressed  { background: #39b2e5; color: #fff; border: 1px solid #39b2e5; border: 1px solid #39b2e5; 	border-radius: 5px;	padding: 5px;}"
            "QPushButton::disabled { background-color: #BBB; border: 1px solid #AAA; border-radius: 5px;	padding: 5px;}"
            );
        ui->Catalogs_pushButton_UpdateAllActive->setStyleSheet(
            "QPushButton           { background-color: #ff8000; } "
            "QPushButton::hover    { background: #39b2e5; color: #fff; border: 1px solid #39b2e5; border: 1px solid #39b2e5; 	border-radius: 5px;	padding: 5px;}"
            "QPushButton::pressed  { background: #39b2e5; color: #fff; border: 1px solid #39b2e5; border: 1px solid #39b2e5; 	border-radius: 5px;	padding: 5px;}"
            "QPushButton::disabled { background-color: #BBB; border: 1px solid #AAA; border-radius: 5px;	padding: 5px;}"
            );
        ui->Create_pushButton_CreateCatalog->setStyleSheet(
            "QPushButton           { background-color: #81d41a; padding-right: 20px; } "
            "QPushButton::hover    { background: #39b2e5; color: #fff; border: 1px solid #39b2e5; border: 1px solid #39b2e5; 	border-radius: 5px;	padding: 5px;}"
            "QPushButton::pressed  { background: #39b2e5; color: #fff; border: 1px solid #39b2e5; border: 1px solid #39b2e5; 	border-radius: 5px;	padding: 5px;}"
            );

        //Lines
        ui->Search_line_SeparateResults->setStyleSheet("QFrame { color: #095676; } ");
        ui->Devices_line_SeparateTop->setStyleSheet("QFrame { color: #095676; } "); // border-top: 1px solid #095676;
        ui->Statistics_line_Separate->setStyleSheet("QFrame { color: #095676; } ");

        //Doted lines on Search screen
        ui->Search_label_LinkImage01->setStyleSheet("QLabel { background: url(:/images/link_blue/link-tree-mid.png) repeat-y left; } ");
        ui->Search_label_LinkImage02->setStyleSheet("QLabel { background: url(:/images/link_blue/link-v.png) repeat-y left; } ");
        ui->Search_label_LinkImage03->setStyleSheet("QLabel { background: url(:/images/link_blue/link-tree-mid.png) repeat-y left; } ");
        ui->Search_label_LinkImage04->setStyleSheet("QLabel { background: url(:/images/link_blue/link-h.png) repeat-x left; } ");
        ui->Search_label_LinkImage05->setStyleSheet("QLabel { background: url(:/images/link_blue/link-v.png) repeat-y left; } ");
        ui->Search_label_LinkImage06->setStyleSheet("QLabel { background: url(:/images/link_blue/link-tree-mid.png) repeat-y left; } ");
        ui->Search_label_LinkImage07->setStyleSheet("QLabel { background: url(:/images/link_blue/link-tree-mid.png) repeat-y left; } ");
        ui->Search_label_LinkImage08->setStyleSheet("QLabel { background: url(:/images/link_blue/link-tree-end.png) no-repeat left; } ");
        ui->Search_label_LinkImage09->setStyleSheet("QLabel { background: url(:/images/link_blue/link-tree-end.png) repeat-x left; } ");
        ui->Search_label_LinkImage10->setStyleSheet("QLabel { background: url(:/images/link_blue/link-h.png) repeat-x left; } ");
        ui->Search_label_LinkImage11->setStyleSheet("QLabel { background: url(:/images/link_blue/link-tree-mid.png) repeat-y left; } ");
        ui->Search_label_LinkImage12->setStyleSheet("QLabel { background: url(:/images/link_blue/link-h.png) repeat-x left; } ");
        ui->Search_label_LinkImage13->setStyleSheet("QLabel { background: url(:/images/link_blue/link-tree-mid.png) repeat-y left; } ");
        ui->Search_label_LinkImage14->setStyleSheet("QLabel { background: url(:/images/link_blue/link-h.png) repeat-x left; } ");
        ui->Search_label_LinkImage15->setStyleSheet("QLabel { background: url(:/images/link_blue/link-v.png) repeat-y left; } ");
        ui->Search_label_LinkImage16->setStyleSheet("QLabel { background: url(:/images/link_blue/link-v.png) repeat-y left; } ");
        ui->Search_label_LinkImage17->setStyleSheet("QLabel { background: url(:/images/link_blue/link-tree-end.png) no-repeat left; } ");
        ui->Search_label_LinkImage18->setStyleSheet("QLabel { background: url(:/images/link_blue/link-v.png) repeat-y left; } ");
        ui->Search_label_LinkImage19->setStyleSheet("QLabel { background: url(:/images/link_blue/link-v.png) repeat-y left; } ");
        ui->Search_label_LinkImage20->setStyleSheet("QLabel { background: url(:/images/link_blue/link-tree-end.png) no-repeat left; } ");
        ui->Search_label_LinkImage21->setStyleSheet("QLabel { background: url(:/images/link_blue/link-v.png) repeat-y left; } ");
        ui->Search_label_LinkImage22->setStyleSheet("QLabel { background: url(:/images/link_blue/link-v.png) repeat-y left; } ");
        ui->Search_label_LinkImage23->setStyleSheet("QLabel { background: url(:/images/link_blue/link-v.png) repeat-y left; } ");
        ui->Search_label_LinkImage24->setStyleSheet("QLabel { background: url(:/images/link_blue/link-tree-mid.png) repeat-y left; } ");
        ui->Search_label_LinkImage30->setStyleSheet("QLabel { background: url(:/images/link_blue/link-v.png) repeat-y left; } ");
        ui->Search_label_LinkImage31->setStyleSheet("QLabel { background: url(:/images/link_blue/link-v.png) repeat-y left; } ");
        ui->Search_label_LinkImage32->setStyleSheet("QLabel { background: url(:/images/link_blue/link-v.png) repeat-y left; } ");
        ui->Search_label_LinkImage33->setStyleSheet("QLabel { background: url(:/images/link_blue/link-v.png) repeat-y left; } ");
        ui->Search_label_LinkImage16_7->setStyleSheet("QLabel { background: url(:/images/link_blue/link-v.png) repeat-y left; } ");
        ui->Search_label_LinkImage16_8->setStyleSheet("QLabel { background: url(:/images/link_blue/link-v.png) repeat-y left; } ");
        ui->Search_label_LinkImage16_3->setStyleSheet("QLabel { background: url(:/images/link_blue/link-v.png) repeat-y left; } ");
        ui->Search_label_LinkImage16_9->setStyleSheet("QLabel { background: url(:/images/link_blue/link-v.png) repeat-y left; } ");
        ui->Search_label_LinkImage16_2->setStyleSheet("QLabel { background: url(:/images/link_blue/link-v.png) repeat-y left; } ");
        ui->Search_label_LinkImage16_15->setStyleSheet("QLabel { background: url(:/images/link_blue/link-v.png) repeat-y left; } ");
        ui->Search_label_LinkImage26->setStyleSheet("QLabel { background: url(:/images/link_blue/link-tree-mid.png) repeat-y left; } ");
        ui->Search_label_LinkImage27->setStyleSheet("QLabel { background: url(:/images/link_blue/link-tree-mid.png) repeat-y left; } ");
        ui->Search_label_LinkImage28->setStyleSheet("QLabel { background: url(:/images/link_blue/link-tree-end.png) no-repeat left; } ");


        //Change the alternate color of treeview lines
        ui->Filters_treeView_Devices->setStyleSheet(
            "QTreeView { alternate-background-color: #161b1d; }"
            );

        ui->Filters_label_Selection->setStyleSheet(
            "color: #39b2e5;"
            );

        ui->Filters_widget->setStyleSheet(
            "QComboBox             { background-color: #FFF; padding-left: 6px; }"
            "QLabel                { color: #FFF; }"
            "QTabBar::tab          { height: 30px; }"
            "QTabWidget::tab-bar   { left: 0px; }"
            "QTabWidget            { padding: 0px; margin: 0px; }"

            "QPushButton           { text-align: left; padding: 5px 4px; margin: 0px; border: 1px solid #ccc; border-radius: 5px;	padding: 5px;} "
            "QPushButton::hover    { background: #39b2e5; color: #fff; border: 1px solid #39b2e5; 	border-radius: 5px;	padding: 5px;}"
            "QPushButton::pressed  { background: #0D79A6; color: #fff; border: 1px solid #10a2df; 	border-radius: 5px;	padding: 5px;}"
            );

        //Search
        ui->Search_treeView_FilesFound->setStyleSheet(
            "QTreeView { alternate-background-color: #161b1d; }"
            );
        ui->Search_treeView_CatalogsFound->setStyleSheet(
            "QTreeView { alternate-background-color: #161b1d; }"
            );
        ui->Search_treeView_History->setStyleSheet(
            "QTreeView { alternate-background-color: #161b1d; }"
            );

        //Devices
        ui->Devices_treeView_DeviceList->setStyleSheet(
            "QTreeView { alternate-background-color: #161b1d; }"
            );

        //Explore
        ui->Explore_treeview_Directories->setStyleSheet(
            "QTreeView { alternate-background-color: #161b1d; }"
            );
        ui->Explore_treeView_FileList->setStyleSheet(
            "QTreeView { alternate-background-color: #161b1d; }"
            );
        //Create
        ui->Create_treeView_Explorer->setStyleSheet(
            "QTreeView { alternate-background-color: #161b1d; }"
            );
        ui->Create_treeView_Excluded->setStyleSheet(
            "QTreeView { alternate-background-color: #161b1d; }"
            );
        //Tags
        ui->Tags_treeview_Explorer->setStyleSheet(
            "QTreeView { alternate-background-color: #161b1d; }"
            );
        ui->Tags_listView_ExistingTags->setStyleSheet(
            "QListView { alternate-background-color: #161b1d; }"
            );
        ui->Tags_treeView_FolderTags->setStyleSheet(
            "QTreeView { alternate-background-color: #161b1d; }"
            );
        //BackUp
        ui->BackUp_tableView_CurrentMappings->setStyleSheet(
            "QTreeView { alternate-background-color: #161b1d; }"
            );
        ui->BackUp_treeView_List1->setStyleSheet(
            "QTreeView { alternate-background-color: #161b1d; }"
            );
        ui->BackUp_treeView_List2->setStyleSheet(
            "QTreeView { alternate-background-color: #161b1d; }"
            );

        QString additionalStyles = ui->tabWidget->styleSheet() +
                                   "QTabWidget QLabel { color: #39b2e5; }"
                                   "QTabWidget QLabel[objectName*=\"Value\"], "
                                   "QTabWidget QLabel[objectName*=\"Display\"] { color: #fff; }";
        ui->tabWidget->setStyleSheet(additionalStyles);
    }
    //----------------------------------------------------------------------
    bool MainWindow::isDarkTheme() const
    {
        // Use the same logic as the existing code in mainwindow.cpp
        //QPalette palette = QApplication::palette();
        //return palette.color(QPalette::Window).lightness() < 128;

        // First, detect the desktop environment to choose the best detection method
        QString xdgCurrentDesktop = qEnvironmentVariable("XDG_CURRENT_DESKTOP");
        QString sessionDesktop = qEnvironmentVariable("DESKTOP_SESSION");

        qDebug() << "Theme detection - XDG_CURRENT_DESKTOP:" << xdgCurrentDesktop
                 << "DESKTOP_SESSION:" << sessionDesktop;

        // KDE Detection: Use Qt palette method as it works well on KDE
        if (xdgCurrentDesktop.contains("KDE", Qt::CaseInsensitive) ||
            sessionDesktop.contains("plasma", Qt::CaseInsensitive) ||
            qEnvironmentVariable("KDE_SESSION_VERSION").length() > 0) {

            QPalette palette = QApplication::palette();
            bool kdeDark = palette.color(QPalette::Window).lightness() < 128;
            qDebug() << "KDE detected - using Qt palette detection. Window lightness:"
                     << palette.color(QPalette::Window).lightness() << "- Dark theme:" << kdeDark;
            return kdeDark;
        }

        // For non-KDE environments, use enhanced detection
        qDebug() << "Non-KDE environment detected - using enhanced detection";

        // Method 1: Check GTK_THEME environment variable (most reliable when set)
        QString gtkTheme = qEnvironmentVariable("GTK_THEME");
        if (!gtkTheme.isEmpty()) {
            bool gtkDark = gtkTheme.contains("dark", Qt::CaseInsensitive);
            qDebug() << "GTK_THEME found:" << gtkTheme << "- Dark theme:" << gtkDark;
            return gtkDark;
        }

        // Method 2: Desktop environment specific detection
        bool desktopSpecificResult = false;
        bool desktopSpecificFound = false;

        if (xdgCurrentDesktop.contains("GNOME", Qt::CaseInsensitive) ||
            xdgCurrentDesktop.contains("ubuntu", Qt::CaseInsensitive)) {
            desktopSpecificResult = checkGnomeTheme();
            desktopSpecificFound = true;
            qDebug() << "GNOME/Ubuntu theme detection result:" << desktopSpecificResult;
        }
        else if (xdgCurrentDesktop.contains("XFCE", Qt::CaseInsensitive)) {
            desktopSpecificResult = checkXfceTheme();
            desktopSpecificFound = true;
            qDebug() << "XFCE theme detection result:" << desktopSpecificResult;
        }
        else if (xdgCurrentDesktop.contains("X-Cinnamon", Qt::CaseInsensitive) ||
                 xdgCurrentDesktop.contains("Cinnamon", Qt::CaseInsensitive)) {
            desktopSpecificResult = checkCinnamonTheme();
            desktopSpecificFound = true;
            qDebug() << "Cinnamon theme detection result:" << desktopSpecificResult;
        }

        // If desktop-specific detection worked, use it
        if (desktopSpecificFound) {
            return desktopSpecificResult;
        }

        // Method 3: Fallback to Qt palette with additional checks
        QPalette palette = QApplication::palette();
        bool qtPaletteDark = palette.color(QPalette::Window).lightness() < 128;

        // Method 4: Additional validation using text color
        QColor textColor = palette.color(QPalette::WindowText);
        bool textColorIndicatesDark = textColor.lightness() > 128; // Light text suggests dark theme

        qDebug() << "Fallback detection - Window lightness:" << palette.color(QPalette::Window).lightness()
                 << "Text lightness:" << textColor.lightness()
                 << "Qt palette says dark:" << qtPaletteDark
                 << "Text color suggests dark:" << textColorIndicatesDark;

        // If both indicators agree, use that result
        if (qtPaletteDark == textColorIndicatesDark) {
            qDebug() << "Both indicators agree - using result:" << qtPaletteDark;
            return qtPaletteDark;
        }

        // If they disagree, prefer text color indication (often more reliable on non-KDE)
        qDebug() << "Indicators disagree - preferring text color indication:" << textColorIndicatesDark;
        return textColorIndicatesDark;
    }
    bool MainWindow::checkGnomeTheme() const
    {
        // First try the newer color-scheme setting (GNOME 42+)
        QProcess process;
        process.start("gsettings", QStringList() << "get" << "org.gnome.desktop.interface" << "color-scheme");
        process.waitForFinished(1000);

        if (process.exitCode() == 0) {
            QString colorScheme = process.readAllStandardOutput().trimmed();
            colorScheme.remove(QChar('\''));  // Remove quotes
            qDebug() << "GNOME color-scheme:" << colorScheme;
            if (colorScheme.contains("dark", Qt::CaseInsensitive) || colorScheme.contains("prefer-dark")) {
                return true;
            }
            if (colorScheme.contains("light", Qt::CaseInsensitive) || colorScheme.contains("default")) {
                return false;
            }
        }

        // Fallback to gtk-theme setting
        process.start("gsettings", QStringList() << "get" << "org.gnome.desktop.interface" << "gtk-theme");
        process.waitForFinished(1000);

        if (process.exitCode() == 0) {
            QString gtkTheme = process.readAllStandardOutput().trimmed();
            gtkTheme.remove(QChar('\''));  // Remove quotes
            qDebug() << "GNOME gtk-theme:" << gtkTheme;
            return gtkTheme.contains("dark", Qt::CaseInsensitive);
        }

        return false;
    }

    // Helper method to check XFCE theme
    bool MainWindow::checkXfceTheme() const
    {
        QProcess process;
        process.start("xfconf-query", QStringList() << "-c" << "xsettings" << "-p" << "/Net/ThemeName");
        process.waitForFinished(1000);

        if (process.exitCode() == 0) {
            QString themeName = process.readAllStandardOutput().trimmed();
            qDebug() << "XFCE theme name:" << themeName;
            return themeName.contains("dark", Qt::CaseInsensitive);
        }

        return false;
    }

    // Helper method to check Cinnamon theme
    bool MainWindow::checkCinnamonTheme() const
    {
        QProcess process;
        process.start("gsettings", QStringList() << "get" << "org.cinnamon.desktop.interface" << "gtk-theme");
        process.waitForFinished(1000);

        if (process.exitCode() == 0) {
            QString gtkTheme = process.readAllStandardOutput().trimmed();
            gtkTheme.remove(QChar('\''));  // Remove quotes
            qDebug() << "Cinnamon gtk-theme:" << gtkTheme;
            return gtkTheme.contains("dark", Qt::CaseInsensitive);
        }

        return false;
    }

    //----------------------------------------------------------------------
    void MainWindow::setupIconTheme()
    {
#ifdef Q_OS_LINUX
        setupLinuxIconTheme();
#elif defined(Q_OS_WIN)
        // Windows: Use the original Qt approach that was working
        setupWindowsIconTheme();
#else
        // macOS or other: Use Windows approach as fallback
        // setupWindowsIconTheme();
#endif
    }
    //----------------------------------------------------------------------
#ifdef Q_OS_LINUX
    void MainWindow::setupLinuxIconTheme()
    {
        qDebug() << "Setting up Linux icon theme...";

        // Disable KDE icon integration to restore Qt-only behavior
        // This forces Qt to use fallback paths like the old portable versions

        // Force Qt to ignore KDE integration for icon loading
        //QApplication::setDesktopSettingsAware(false);

        // Clear any KDE theme to force fallback usage
        QIcon::setThemeName("");

        // Set up fallback paths like the old working versions
        bool darkTheme = isDarkTheme();
        QStringList fallbackPaths;

        if (darkTheme) {
            fallbackPaths << ":/fallback-icons-dark";  // White icons for dark backgrounds
            qDebug() << "Linux dark theme: Using white icons from :/fallback-icons-dark";
        } else {
            fallbackPaths << ":/fallback-icons"; // Dark icons for light backgrounds
            qDebug() << "Linux light theme: Using dark icons from :/fallback-icons-light";
        }

        QIcon::setFallbackSearchPaths(fallbackPaths);

        qDebug() << "Linux icon setup completed. Fallback paths:" << QIcon::fallbackSearchPaths();

        // Test icon loading
        QIcon testIcon = QIcon::fromTheme("folder");
        qDebug() << "Test folder icon loaded successfully:" << !testIcon.isNull();
    }
#endif
    //----------------------------------------------------------------------
#ifdef Q_OS_WIN
    void MainWindow::setupWindowsIconTheme()
    {
        // Windows: Keep the exact same logic that was working before
        qDebug() << "Setting up Windows icon theme...";

        // Use the original approach from main.cpp
        bool darkTheme = isDarkTheme();

        // Set breeze theme (works fine on Windows)
        QIcon::setThemeName("breeze");

        // Set fallback paths
        QStringList fallbackPaths = QIcon::fallbackSearchPaths();

        if (darkTheme) {
            fallbackPaths << ":/fallback-icons-dark";
            qDebug() << "Windows dark theme: Using fallback-icons-dark";
        } else {
            fallbackPaths << ":/fallback-icons";
            qDebug() << "Windows light theme: Using fallback-icons-light";
        }

        QIcon::setFallbackSearchPaths(fallbackPaths);

        qDebug() << "Windows icon setup completed. Theme:" << QIcon::themeName();
        qDebug() << "Fallback paths:" << QIcon::fallbackSearchPaths();
    }
#endif
    //----------------------------------------------------------------------
    void MainWindow::debugIconSetup()
    {
        qDebug() << "=== ICON SETUP DEBUG ===";
        qDebug() << "Platform:"
#ifdef Q_OS_LINUX
                 << "Linux"
#elif defined(Q_OS_WIN)
                 << "Windows"
#elif defined(Q_OS_MAC)
                 << "macOS"
#else
                 << "Other"
#endif
            ;

        qDebug() << "Icon theme name:" << QIcon::themeName();
        qDebug() << "Fallback search paths:" << QIcon::fallbackSearchPaths();
        qDebug() << "Theme search paths:" << QIcon::themeSearchPaths();
        qDebug() << "Desktop settings aware:" << QApplication::desktopSettingsAware();
        qDebug() << "Dark theme detected:" << isDarkTheme();

        // Test key icons
        QStringList testIcons = {"folder", "edit-find", "image-jpeg"};
        for (const QString &iconName : testIcons) {
            QIcon icon = QIcon::fromTheme(iconName);
            qDebug() << "Icon" << iconName << "- null:" << icon.isNull()
                     << "sizes:" << icon.availableSizes().size();
        }

        qDebug() << "=== END ICON DEBUG ===";
    }
    //----------------------------------------------------------------------
    void MainWindow::refreshAllIcons()
    {
        qDebug() << "Refreshing all UI icons after theme change...";

        // Force refresh of tree view icons by triggering model updates
        if (ui->Search_treeView_FilesFound && ui->Search_treeView_FilesFound->model()) {
            ui->Search_treeView_FilesFound->model()->layoutChanged();
        }

        if (ui->Explore_treeView_FileList && ui->Explore_treeView_FileList->model()) {
            ui->Explore_treeView_FileList->model()->layoutChanged();
        }

        if (ui->Explore_treeview_Directories && ui->Explore_treeview_Directories->model()) {
            ui->Explore_treeview_Directories->model()->layoutChanged();
        }

        if (ui->Filters_treeView_Devices && ui->Filters_treeView_Devices->model()) {
            ui->Filters_treeView_Devices->model()->layoutChanged();
        }

        if (ui->Devices_treeView_DeviceList && ui->Devices_treeView_DeviceList->model()) {
            ui->Devices_treeView_DeviceList->model()->layoutChanged();
        }

        // Refresh button icons that use QIcon::fromTheme
        if (ui->Search_pushButton_Search) {
            ui->Search_pushButton_Search->setIcon(QIcon::fromTheme("edit-find"));
        }

        // Force repaint of the entire UI
        update();

        qDebug() << "Icon refresh completed";
    }
    //----------------------------------------------------------------------
    void MainWindow::changeEvent(QEvent *event)
    {
        if (event->type() == QEvent::PaletteChange ||
            event->type() == QEvent::StyleChange ||
            event->type() == QEvent::ThemeChange) {

            qDebug() << "Desktop theme change detected, refreshing icons...";

            // Small delay to ensure the palette change is fully applied
            QTimer::singleShot(100, this, [this]() {
                // Reapply icon theme with new desktop theme detection
                setupIconTheme();

                // Refresh all existing icons in the UI
                refreshAllIcons();

                // Optionally reapply custom theme colors if using Katalog theme
                if (themeID == 1) {
                    if (isDarkTheme()) {
                        loadCustomThemeDark();
                    } else {
                        loadCustomThemeLight();
                    }
                }

                qDebug() << "Theme change handling completed";
            });
        }

        // Call parent implementation
        KXmlGuiWindow::changeEvent(event);
    }
    //----------------------------------------------------------------------

    // Progress Reporting
    //----------------------------------------------------------------------
    void MainWindow::updateStatusBarMessage(const QString& htmlMessage, int timeout)
    {
        if (statusBarLabel) {
            statusBarLabel->setText(htmlMessage);
            statusBar()->show();

            if (timeout > 0 && statusBarTimer) {
                statusBarTimer->start(timeout);
            } else if (statusBarTimer) {
                statusBarTimer->stop();
            }
        }
    }
    //----------------------------------------------------------------------

    // Icon & Theme testing
    //----------------------------------------------------------------------
    void MainWindow::debugIconLoadingDetailed()
    {
        qDebug() << "=== DETAILED ICON LOADING ANALYSIS ===";
        qDebug() << "Platform:"
#ifdef Q_OS_LINUX
                 << "Linux"
#elif defined(Q_OS_WIN)
                 << "Windows"
#elif defined(Q_OS_MAC)
                 << "macOS"
#else
                 << "Other"
#endif
            ;

        // Basic icon theme info
        qDebug() << "Icon theme name:" << QIcon::themeName();
        qDebug() << "Fallback search paths:" << QIcon::fallbackSearchPaths();
        qDebug() << "Theme search paths:" << QIcon::themeSearchPaths();
        qDebug() << "Desktop settings aware:" << QApplication::desktopSettingsAware();
        qDebug() << "Dark theme detected:" << isDarkTheme();

        // Check if KF6BreezeIcons is available
        qDebug() << "";
        qDebug() << "=== KF6 Integration Check ===";

        // Try to load a test icon with different approaches
        QStringList testIcons = {"folder", "edit-find", "media-optical", "dialog-ok-apply"};

        for (const QString &iconName : testIcons) {
            qDebug() << "";
            qDebug() << "Testing icon:" << iconName;

            // Method 1: Standard QIcon::fromTheme (current approach)
            QIcon standardIcon = QIcon::fromTheme(iconName);
            qDebug() << "  Standard fromTheme - null:" << standardIcon.isNull()
                     << "sizes:" << standardIcon.availableSizes().size();
            if (!standardIcon.isNull()) {
                qDebug() << "  Available sizes:" << standardIcon.availableSizes();
                // Try to get the actual file path being used
                QPixmap pixmap = standardIcon.pixmap(22, 22);
                qDebug() << "  22x22 pixmap null:" << pixmap.isNull();
            }

            // Method 2: Test if KF6 theme loading works by temporarily clearing fallbacks
            QStringList originalFallbacks = QIcon::fallbackSearchPaths();
            QIcon::setFallbackSearchPaths(QStringList()); // Clear fallbacks temporarily

            // Try with breeze theme name
            QIcon::setThemeName("breeze");
            QIcon kf6Icon = QIcon::fromTheme(iconName);
            qDebug() << "  KF6/Breeze without fallbacks - null:" << kf6Icon.isNull()
                     << "sizes:" << kf6Icon.availableSizes().size();

            // Try with system theme
            QIcon::setThemeName(""); // Let system decide
            QIcon systemIcon = QIcon::fromTheme(iconName);
            qDebug() << "  System theme without fallbacks - null:" << systemIcon.isNull()
                     << "sizes:" << systemIcon.availableSizes().size();

            // Restore original settings
            QIcon::setFallbackSearchPaths(originalFallbacks);
            setupIconTheme(); // Restore your current setup

            // Method 3: Direct resource check
            QString lightResource = QString(":/fallback-icons/%1.png").arg(iconName);
            QString darkResource = QString(":/fallback-icons-dark/%1.png").arg(iconName);
            qDebug() << "  Light resource exists:" << QFile::exists(lightResource);
            qDebug() << "  Dark resource exists:" << QFile::exists(darkResource);
        }

        qDebug() << "";
        qDebug() << "=== Icon Search Path Analysis ===";
        QStringList themePaths = QIcon::themeSearchPaths();
        for (const QString &path : themePaths) {
            qDebug() << "Theme path:" << path;
            QDir dir(path);
            if (dir.exists()) {
                QStringList themes = dir.entryList(QDir::Dirs | QDir::NoDotAndDotDot);
                qDebug() << "  Available themes:" << themes;

                // Check if breeze theme exists
                if (themes.contains("breeze")) {
                    QString breezePath = path + "/breeze";
                    QDir breezeDir(breezePath);
                    if (breezeDir.exists()) {
                        QStringList categories = breezeDir.entryList(QDir::Dirs | QDir::NoDotAndDotDot);
                        qDebug() << "  Breeze categories:" << categories;
                    }
                }
            } else {
                qDebug() << "  Path does not exist";
            }
        }

        qDebug() << "";
        qDebug() << "=== Environment Variables ===";
        qDebug() << "XDG_DATA_DIRS:" << qgetenv("XDG_DATA_DIRS");
        qDebug() << "XDG_CONFIG_HOME:" << qgetenv("XDG_CONFIG_HOME");
        qDebug() << "XDG_DATA_HOME:" << qgetenv("XDG_DATA_HOME");

        qDebug() << "=== END DETAILED ANALYSIS ===";
        qDebug() << "";
    }

    // Also add this method to track which actual files are being loaded
    void MainWindow::testIconSourceTracking()
    {
        qDebug() << "=== ICON SOURCE TRACKING TEST ===";

        // Create a custom icon engine to track loading
        QStringList testIcons = {"folder", "edit-find", "document-save"};

        for (const QString &iconName : testIcons) {
            QIcon icon = QIcon::fromTheme(iconName);
            if (!icon.isNull()) {
                // Get pixmaps at different sizes to see what's being loaded
                QPixmap pm16 = icon.pixmap(16, 16);
                QPixmap pm22 = icon.pixmap(22, 22);
                QPixmap pm32 = icon.pixmap(32, 32);

                qDebug() << "Icon" << iconName << ":";
                qDebug() << "  16x16 available:" << !pm16.isNull();
                qDebug() << "  22x22 available:" << !pm22.isNull();
                qDebug() << "  32x32 available:" << !pm32.isNull();
                qDebug() << "  Cachekey:" << QString::number(icon.cacheKey());

                // Try to determine if it's coming from resources
                // This is a bit hacky but can help identify the source
                QIcon resourceIcon = QIcon(QString(":/fallback-icons/%1.png").arg(iconName));
                QIcon resourceIconDark = QIcon(QString(":/fallback-icons-dark/%1.png").arg(iconName));

                bool matchesLight = (icon.cacheKey() == resourceIcon.cacheKey());
                bool matchesDark = (icon.cacheKey() == resourceIconDark.cacheKey());

                qDebug() << "  Matches light resource:" << matchesLight;
                qDebug() << "  Matches dark resource:" << matchesDark;

                if (!matchesLight && !matchesDark) {
                    qDebug() << "  *** LIKELY FROM SYSTEM/KF6 ***";
                } else {
                    qDebug() << "  *** FROM EMBEDDED RESOURCES ***";
                }
            }
        }

        qDebug() << "=== END SOURCE TRACKING ===";
    }

    // Modified setupIconTheme methods to test KF6 icon loading
    // Add these as test methods to MainWindow class
    void MainWindow::setupIconThemeWithKF6Test()
    {
        qDebug() << "=== TESTING KF6 ICON LOADING ===";

#ifdef Q_OS_LINUX
        setupLinuxIconThemeKF6Test();
#elif defined(Q_OS_WIN)
        setupWindowsIconThemeKF6Test();
#else
        //setupWindowsIconThemeKF6Test();
#endif

        // Run detailed diagnostics after setup
        debugIconLoadingDetailed();
        testIconSourceTracking();
    }

#ifdef Q_OS_LINUX
    void MainWindow::setupLinuxIconThemeKF6Test()
    {
        qDebug() << "Setting up Linux icon theme with KF6 integration test...";

        // ENABLE KDE icon integration (opposite of current approach)
        QApplication::setDesktopSettingsAware(true);

        // Let KF6 handle the theme name (don't force empty)
        // Try different approaches:

        // Approach 1: Let system decide theme
        qDebug() << "Test 1: System-decided theme";
        QIcon::setThemeName(""); // Let system decide

        // Clear fallback paths to force KF6/system usage
        QIcon::setFallbackSearchPaths(QStringList());

        qDebug() << "After clearing fallbacks:";
        qDebug() << "  Theme name:" << QIcon::themeName();
        qDebug() << "  Theme paths:" << QIcon::themeSearchPaths();

        // Test if icons load
        QIcon testIcon1 = QIcon::fromTheme("folder");
        qDebug() << "  System folder icon loaded:" << !testIcon1.isNull();

        // Approach 2: Explicitly set breeze theme
        qDebug() << "";
        qDebug() << "Test 2: Explicit breeze theme";
        QIcon::setThemeName("breeze");

        QIcon testIcon2 = QIcon::fromTheme("folder");
        qDebug() << "  Breeze folder icon loaded:" << !testIcon2.isNull();

        // Approach 3: Add minimal fallback only if KF6 fails
        qDebug() << "";
        qDebug() << "Test 3: KF6 with minimal fallback";

        if (testIcon2.isNull()) {
            qDebug() << "  KF6 icons not available, adding fallback";
            QStringList fallbackPaths;
            bool darkTheme = isDarkTheme();

            if (darkTheme) {
                fallbackPaths << ":/fallback-icons-dark";
            } else {
                fallbackPaths << ":/fallback-icons";
            }

            QIcon::setFallbackSearchPaths(fallbackPaths);
            qDebug() << "  Added fallback paths:" << fallbackPaths;
        } else {
            qDebug() << "  KF6 icons working, no fallback needed";
        }

        qDebug() << "Linux KF6 test setup completed";
    }
#endif

#ifdef Q_OS_WIN
    void MainWindow::setupWindowsIconThemeKF6Test()
    {
        qDebug() << "Setting up Windows icon theme with KF6 integration test...";

        // Test if KF6BreezeIcons works on Windows
        QApplication::setDesktopSettingsAware(true);

        // Try KF6 approach first
        QIcon::setThemeName("breeze");
        QIcon::setFallbackSearchPaths(QStringList()); // Clear fallbacks

        qDebug() << "Testing KF6 on Windows:";
        qDebug() << "  Theme name:" << QIcon::themeName();
        qDebug() << "  Theme paths:" << QIcon::themeSearchPaths();

        // Test critical icons
        QStringList testIcons = {"folder", "edit-find", "document-save"};
        int successCount = 0;

        for (const QString &iconName : testIcons) {
            QIcon icon = QIcon::fromTheme(iconName);
            bool loaded = !icon.isNull();
            if (loaded) successCount++;
            qDebug() << "  " << iconName << "loaded:" << loaded;
        }

        qDebug() << "KF6 success rate:" << successCount << "/" << testIcons.size();

        // If KF6 doesn't work well, add fallbacks
        if (successCount < testIcons.size()) {
            qDebug() << "Adding fallback paths for Windows";

            QStringList fallbackPaths = QIcon::fallbackSearchPaths();
            bool darkTheme = isDarkTheme();

            if (darkTheme) {
                fallbackPaths << ":/fallback-icons-dark";
            } else {
                fallbackPaths << ":/fallback-icons";
            }

            QIcon::setFallbackSearchPaths(fallbackPaths);
            qDebug() << "Windows KF6 test with fallback completed";
        } else {
            qDebug() << "Windows KF6 test - no fallback needed!";
        }
    }
#endif

    // Safe method to test removing fallback resources
    void MainWindow::testWithoutFallbackResources()
    {
        qDebug() << "=== TESTING WITHOUT FALLBACK RESOURCES ===";

        // Store original setup
        QString originalTheme = QIcon::themeName();
        QStringList originalFallbacks = QIcon::fallbackSearchPaths();
        bool originalDesktopAware = QApplication::desktopSettingsAware();

        // Test KF6-only setup
        QApplication::setDesktopSettingsAware(true);
        QIcon::setThemeName("breeze");
        QIcon::setFallbackSearchPaths(QStringList()); // Remove all fallbacks

        qDebug() << "Testing icon loading without any fallback resources:";

        // Test all your commonly used icons
        QStringList criticalIcons = {
            "folder", "edit-find", "document-save", "document-open", "edit-copy",
            "edit-paste", "edit-delete", "media-optical", "drive-harddisk",
            "dialog-ok-apply", "go-up", "go-down", "go-next", "go-previous"
        };

        int successCount = 0;
        QStringList failedIcons;

        for (const QString &iconName : criticalIcons) {
            QIcon icon = QIcon::fromTheme(iconName);
            bool loaded = !icon.isNull() && !icon.availableSizes().isEmpty();

            if (loaded) {
                successCount++;
                qDebug() << "  ✅" << iconName << "- sizes:" << icon.availableSizes();
            } else {
                failedIcons << iconName;
                qDebug() << "  ❌" << iconName << "- FAILED";
            }
        }

        qDebug() << "";
        qDebug() << "RESULTS:";
        qDebug() << "  Success rate:" << successCount << "/" << criticalIcons.size()
                 << "(" << (100 * successCount / criticalIcons.size()) << "%)";

        if (!failedIcons.isEmpty()) {
            qDebug() << "  Failed icons:" << failedIcons;
            qDebug() << "  These would need fallback resources";
        }

        if (successCount == criticalIcons.size()) {
            qDebug() << "  🎉 ALL ICONS LOADED FROM KF6! Safe to remove fallback resources.";
        } else if (successCount >= criticalIcons.size() * 0.8) {
            qDebug() << "  ⚠️ Most icons work, consider keeping minimal fallback for:" << failedIcons;
        } else {
            qDebug() << "  ❌ KF6 icon loading insufficient, keep fallback resources";
        }

        // Restore original setup
        QApplication::setDesktopSettingsAware(originalDesktopAware);
        QIcon::setThemeName(originalTheme);
        QIcon::setFallbackSearchPaths(originalFallbacks);

        qDebug() << "=== FALLBACK TEST COMPLETED, ORIGINAL SETUP RESTORED ===";
    }
