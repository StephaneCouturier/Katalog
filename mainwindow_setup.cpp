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
#include "database.h"

//Database -----------------------------------------------------------------
    void MainWindow::startDatabase()
    {
        //Check Sqlite driver
        if (!QSqlDatabase::drivers().contains("QSQLITE")){
            QMessageBox msgBox;
            msgBox.setWindowTitle("Katalog");
            msgBox.setText(tr("Unable to load database.<br/>The SQLite driver was not loaded."));
            msgBox.setIcon(QMessageBox::Critical);
            msgBox.exec();
        }

        if (!QSqlDatabase::drivers().contains("QPSQL")){
            QMessageBox msgBox;
            msgBox.setWindowTitle("Katalog");
            msgBox.setText(tr("Unable to load database.<br/>The Postgres driver was not loaded."));
            msgBox.setIcon(QMessageBox::Critical);
            msgBox.exec();
        }

        // Initialize the database:
        QSqlError err = initializeDatabase();
        if (err.type() != QSqlError::NoError) {
            QMessageBox msgBox;
            msgBox.setWindowTitle("Katalog");
            msgBox.setText("could not Initialize db:<br/>" + err.databaseText());
            msgBox.setIcon(QMessageBox::Critical);
            msgBox.exec();
            return;
        }
    }
    //----------------------------------------------------------------------
    QSqlError MainWindow::initializeDatabase()
    {

        //Get database mode ("Memory", "File", or "Hosted") and fields
        QSettings settings(collection->settingsFilePath, QSettings:: IniFormat);
        collection->databaseMode     = settings.value("Settings/databaseMode").toString();
        collection->databaseFilePath = settings.value("Settings/DatabaseFilePath").toString();
        collection->databaseHostName = settings.value("Settings/databaseHostName").toString();
        collection->databaseName     = settings.value("Settings/databaseName").toString();
        collection->databasePort     = settings.value("Settings/databasePort").toInt();
        collection->databaseUserName = settings.value("Settings/databaseUserName").toString();
        collection->databasePassword = settings.value("Settings/databasePassword").toString();

        if(collection->databaseMode=="")
            collection->databaseMode="Memory";

        else if(collection->databaseMode=="Memory"){

            QSqlDatabase db = QSqlDatabase::addDatabase("QSQLITE");

            db.setDatabaseName(":memory:");
            if (!db.open())
                return db.lastError();
        }
        else if(collection->databaseMode=="File"){
            QFile databaseFile(collection->databaseFilePath);
            if (!databaseFile.exists()){
                QMessageBox msgBox;
                msgBox.setWindowTitle("Katalog");
                msgBox.setText(tr("The Database file cannot be found:<br/>") + collection->databaseFilePath);
                msgBox.setIcon(QMessageBox::Information);
                msgBox.exec();
                selectDatabaseFilePath();
            }

            QSqlDatabase db = QSqlDatabase::addDatabase("QSQLITE");
            db.setDatabaseName(collection->databaseFilePath);
            if (!db.open())
                return db.lastError();
        }
        else if(collection->databaseMode=="Hosted"){

            QSqlDatabase db = QSqlDatabase::addDatabase("QPSQL");
            db.setHostName(collection->databaseHostName);
            db.setDatabaseName(collection->databaseName);
            db.setPort(collection->databasePort);
            db.setUserName(collection->databaseUserName);
            db.setPassword(collection->databasePassword);

            if (!db.open()){
                QSqlError error = db.lastError();
                QMessageBox msgBox;
                msgBox.setText("could not open db:<br/>" + error.databaseText());
                msgBox.exec();
            }
            else {
                QMessageBox msgBox;
                msgBox.setText("Hosted db connected!");
                msgBox.exec();
            }
        }

        //Create tables
        QSqlQuery q;
        if (!q.exec(SQL_CREATE_CATALOG))
            return q.lastError();

        if (!q.exec(SQL_CREATE_STORAGE))
            return q.lastError();

        if (!q.exec(SQL_CREATE_DEVICE))
            return q.lastError();

        if (!q.exec(SQL_CREATE_DEVICE_CATALOG))
            return q.lastError();

        if (!q.exec(SQL_CREATE_FILE))
            return q.lastError();

        if (!q.exec(SQL_CREATE_FILETEMP))
            return q.lastError();

        if (!q.exec(SQL_CREATE_FOLDER))
            return q.lastError();

        if (!q.exec(SQL_CREATE_METADATA))
            return q.lastError();

        if (!q.exec(SQL_CREATE_STATISTICS_DEVICE))
            return q.lastError();

        if (!q.exec(SQL_CREATE_STATISTICS_CATALOG))
            return q.lastError();

        if (!q.exec(SQL_CREATE_STATISTICS_STORAGE))
            return q.lastError();

        if (!q.exec(SQL_CREATE_SEARCH))
            return q.lastError();

        if (!q.exec(SQL_CREATE_TAG))
            return q.lastError();

        if (!q.exec(SQL_CREATE_EXCLUDE))
            return q.lastError();

        return QSqlError();
    }
    //----------------------------------------------------------------------
    void MainWindow::clearDatabaseData()
    {   //Clear database date in the context of Memory mode, prior to reloading files to tables
        if(collection->databaseMode=="Memory"){
            QSqlQuery queryDelete;
            queryDelete.exec("DELETE FROM catalog");
            queryDelete.exec("DELETE FROM storage");
            queryDelete.exec("DELETE FROM device");
            queryDelete.exec("DELETE FROM file");
            queryDelete.exec("DELETE FROM filetemp");
            queryDelete.exec("DELETE FROM folder");
            queryDelete.exec("DELETE FROM statistics");
            queryDelete.exec("DELETE FROM search");
            queryDelete.exec("DELETE FROM tag");
        }
    }
//Set up -------------------------------------------------------------------
    void MainWindow::setupFileContextMenus(){
        ui->Search_treeView_FilesFound->setContextMenuPolicy(Qt::CustomContextMenu);
        ui->Explore_treeView_FileList->setContextMenuPolicy(Qt::CustomContextMenu);
        ui->Explore_treeview_Directories->setContextMenuPolicy(Qt::CustomContextMenu);
        ui->Devices_treeView_DeviceList->setContextMenuPolicy(Qt::CustomContextMenu);
        ui->Filters_treeView_Devices->setContextMenuPolicy(Qt::CustomContextMenu);
    }
    //----------------------------------------------------------------------
    void MainWindow::loadSettings()
    {
        //Check if a settings file already exists. If not, it is considered first use and one gets generated
            QFile settingsFile(collection->settingsFilePath);
            int themeID = 1; //default value for the theme.
            selectedTab = 3; //default value for the first launch. Create screen
            QSettings settings(collection->settingsFilePath, QSettings:: IniFormat);

            firstRun =false;

            if (!settingsFile.exists())
                firstRun =true;

            collection->collectionFolder = settings.value("LastCollectionFolder").toString();

            if (collection->collectionFolder == "")
                firstRun =true;

            if (firstRun == true){
                //Create a file, with default values
                settings.setValue("LastCollectionFolder", QApplication::applicationDirPath());

                //Set Language and theme
                QString userLanguage = QLocale::system().name();
                settings.setValue("Settings/Language", userLanguage);

                QString themeName = tr("Katalog Colors (light)");

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

                //Collection folder choice
                    //Open a dialog for the user to select the directory of the collection where catalog files are stored.
                    collection->collectionFolder = QFileDialog::getExistingDirectory(this, tr("Select the directory for this collection"),
                                                                collection->collectionFolder,
                                                                QFileDialog::ShowDirsOnly
                                                                | QFileDialog::DontResolveSymlinks);

                    //set the location of the application as a default value if a folder was not provided
                    if (collection->collectionFolder =="")
                        collection->collectionFolder = QApplication::applicationDirPath();

                    //save setting
                    settings.setValue("LastCollectionFolder", collection->collectionFolder);

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
                collection->collectionFolder = settings.value("LastCollectionFolder").toString();
            }

            selectedDevice->ID   = settings.value("Selection/SelectedDeviceID").toInt();
            selectedDevice->loadDevice();

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
            deviceTreeExpandState = settings.value("Settings/deviceTreeExpandState").toInt();

            //General settings
            ui->Settings_checkBox_SaveRecordWhenUpdate->setChecked(settings.value("Settings/AutoSaveRecordWhenUpdate", true).toBool());
            QString themeText = settings.value("Settings/Theme").toString();
            if (themeText==""){
                //fallback on default theme
                themeID=1;
            }
            else
                themeID = settings.value("Settings/Theme").toInt();

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

            //last tab selected
            selectedTab = settings.value("Settings/selectedTab").toInt();
            ui->tabWidget->setCurrentIndex(selectedTab);

            //Restore Statistics settings
            ui->Statistics_lineEdit_GraphicStartDate->setText(graphicStartDate.toString("yyyy-mm-dd"));

            //Restore last sort order for the catalogs and storage
            lastCatalogsSortSection       = settings.value("Catalogs/lastCatlogsSortSection").toInt();
            lastCatalogsSortOrder         = settings.value("Catalogs/lastCatlogsSortOrder").toInt();
            lastStorageSortSection        = settings.value("Storage/lastStorageSortSection").toInt();
            lastStorageSortOrder          = settings.value("Storage/lastStorageSortOrder").toInt();
            lastExploreSortSection        = settings.value("Explore/lastExploreSortSection").toInt();
            lastExploreSortOrder          = settings.value("Explore/lastExploreSortOrder").toInt();
            lastSearchSortSection         = settings.value("Search/lastSearchSortSection").toInt();
            lastSearchSortOrder           = settings.value("Search/lastSearchSortOrder").toInt();
            lastSearchHistorySortSection  = settings.value("Search/lastSearchHistorySortSection").toInt();
            lastSearchHistorySortOrder    = settings.value("Search/lastSearchHistorySortOrder").toInt();
            optionDisplayFolders          = settings.value("Explore/DisplayFolders").toBool();
            optionDisplaySubFolders       = settings.value("Explore/DisplaySubFolders").toBool();
            optionDisplayCatalogs         = settings.value("Devices/DisplayCatalogs").toBool();
            optionDisplayStorage          = settings.value("Devices/DisplayStorage").toBool();
            optionDisplayPhysicalGroupOnly= settings.value("Devices/DisplayPhysicalGroupOnly").toBool();
            optionDisplayAllExceptPhysicalGroup= settings.value("Devices/DisplayAllExceptPhysicalGroup").toBool();
            optionDisplayFullTable        = settings.value("Devices/DisplayFullTable").toBool();

            //Restore DEV Settings
            if(developmentMode==true){
                ui->Settings_comboBox_DatabaseMode->setCurrentText(tr(collection->databaseMode.toStdString().c_str()));
            }
    }
    //----------------------------------------------------------------------
    void MainWindow::setFileTypes()
    {
        //Filetypes for cataloging
        fileType_Image<< "*.png" << "*.jpg" << "*.gif" << "*.xcf" << "*.tif" << "*.bmp";
        fileType_Audio<< "*.mp3" << "*.wav" << "*.ogg" << "*.aif";
        fileType_Video<< "*.wmv" << "*.avi" << "*.mp4" << "*.mkv" << "*.flv"  << "*.webm" << "*.m4v" << "*.vob" << "*.ogv" << "*.mov";
        fileType_Text << "*.txt" << "*.pdf" << "*.odt" << "*.idx" << "*.html" << "*.rtf" << "*.doc" << "*.docx" << "*.epub";

        //filetypes for searching
        fileType_ImageS<< "*.png$" << "*.jpg$" << "*.gif$" << "*.xcf$" << "*.tif$" << "*.bmp$";
        fileType_AudioS<< "*.mp3$" << "*.wav$" << "*.ogg$" << "*.aif$";
        fileType_VideoS<< "*.wmv$" << "*.avi$" << "*.mp4$" << "*.mkv$" << "*.flv$"  << "*.webm$"<< "*.m4v$" << "*.vob$"  << "*.ogv$" << "*.mov$";
        fileType_TextS << "*.txt$" << "*.pdf$" << "*.odt$" << "*.idx$" << "*.html$" << "*.rtf$" << "*.doc$" << "*.docx$" << "*.epub$";
    }
    //----------------------------------------------------------------------
    void MainWindow::hideDevelopmentUIItems()
    {
        //Tabs
            ui->tabWidget->removeTab(9);

        //Filter

        //Search
            //hide Krename if not linux
            #ifndef Q_OS_LINUX
            ui->Search_comboBox_SelectProcess->removeItem(2);
            #endif

        //Catalogs
            //DEV: preparing catalog-device relation
            ui->Catalogs_checkBox_isFullDevice->hide();

        //Create
            //DEV: the option to include symblinks is not working yet
            ui->Create_checkBox_IncludeSymblinks->hide();
            ui->Create_checkBox_isFullDevice->hide();
            ui->Create_checkBox_IncludeMetadata->hide();

        //Settings
            //DEV: option to switch database mode between memory and file
            ui->Settings_widget_DataModeSelection->hide();
            ui->Settings_widget_DataMode_LocalSQLite->hide();
            ui->Settings_widget_DataMode_Hosted->hide();

        //TESTS
            //ui->Storage_pushButton_TestMedia->hide();
            //ui->Storage_listView_Media->hide();

    }
    //----------------------------------------------------------------------
    void MainWindow::loadCustomThemeLight()
    {       
        //Standard colors:
            //blue light	39b2e5
            //blue dark		10a2df  0D79A6
            //green light	81d41a
            //green dark	43bf0c
            //orange light	ff8000
            //orange dark	e36600
            //purple light	a1467e
            //purple dark	8b1871

        //Tab widget, including combo boxes and buttons

            if(developmentMode==true){
                QFile file(":styles/tabwidget_dev.css");
                file.open(QFile::ReadOnly);
                QString tabwidgetStyleSheet = QLatin1String(file.readAll());
                ui->tabWidget->setStyleSheet(tabwidgetStyleSheet);
            }
            else{
                QFile file(":styles/tabwidget_blue.css");
                file.open(QFile::ReadOnly);
                QString tabwidgetStyleSheet = QLatin1String(file.readAll());
                ui->tabWidget->setStyleSheet(tabwidgetStyleSheet);
            }

        //Filters widget
        ui->main_widget_ShowFilters->setStyleSheet(
            "QPushButton           { text-align: left; padding: 5px 4px; margin: 0px; border: 1px solid #ccc; border-radius: 5px;	padding: 5px;} "
            "QPushButton::hover    { background: #39b2e5; color: #fff; border: 1px solid #39b2e5; 	border-radius: 5px;	padding: 5px;}"
            "QPushButton::pressed  { background: #0D79A6; color: #fff; border: 1px solid #10a2df; 	border-radius: 5px;	padding: 5px;}"
         );

        ui->splitter_widget_Filters_Hide->setStyleSheet(
            "QPushButton           { text-align: left; padding: 5px 4px; margin: 0px; border: 1px solid #ccc; border-radius: 5px;	padding: 5px;} "
            "QPushButton::hover    { background: #39b2e5; color: #fff; border: 1px solid #39b2e5; 	border-radius: 5px;	padding: 5px;}"
            "QPushButton::pressed  { background: #0D79A6; color: #fff; border: 1px solid #10a2df; 	border-radius: 5px;	padding: 5px;}"

         );

        ui->Filters_label_Selection->setStyleSheet(
                    "color: #095676;"
                  );

        ui->Filters_widget->setStyleSheet(
            "QComboBox             { background-color: #FFF; padding-left: 6px; }"
            "QLabel                { color: #095676; }"
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
        ui->Catalogs_pushButton_UpdateCatalog->setStyleSheet(
                "QPushButton           { background-color: #ff8000; } "
                "QPushButton::hover    { background: #39b2e5; color: #fff; border: 1px solid #39b2e5; border: 1px solid #39b2e5; 	border-radius: 5px;	padding: 5px;}"
                "QPushButton::pressed  { background: #39b2e5; color: #fff; border: 1px solid #39b2e5; border: 1px solid #39b2e5; 	border-radius: 5px;	padding: 5px;}"
                "QPushButton::disabled { background-color: #BBB; border: 1px solid #AAA; border-radius: 5px;	padding: 5px;}"
              );
        ui->Catalogs_pushButton_UpdateAllActive->setStyleSheet(
                "QPushButton           { background-color: #ff8000; } "
                "QPushButton::hover    { background: #39b2e5; color: #fff; border: 1px solid #39b2e5; border: 1px solid #39b2e5; 	border-radius: 5px;	padding: 5px;}"
                "QPushButton::pressed  { background: #39b2e5; color: #fff; border: 1px solid #39b2e5; border: 1px solid #39b2e5; 	border-radius: 5px;	padding: 5px;}"
              );
        ui->Storage_pushButton_Update->setStyleSheet(
                "QPushButton           { background-color: #ff8000; } "
                "QPushButton::hover    { background: #39b2e5; color: #fff; border: 1px solid #39b2e5; border: 1px solid #39b2e5; 	border-radius: 5px;	padding: 5px;}"
                "QPushButton::pressed  { background: #39b2e5; color: #fff; border: 1px solid #39b2e5; border: 1px solid #39b2e5; 	border-radius: 5px;	padding: 5px;}"
                "QPushButton::disabled { background-color: #BBB; border: 1px solid #AAA; border-radius: 5px;	padding: 5px;}"
              );
        ui->Catalogs_pushButton_Save->setStyleSheet(
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
        ui->Search_line_SeparateResults->setStyleSheet("QFrame { color: #095676; border-top: 1px solid 095676; } ");
        ui->Explore_line_Separate->setStyleSheet("QFrame { color: #095676; border-top: 1px solid 095676;} ");
        ui->Statistics_line_Separate->setStyleSheet("QFrame { color: #095676; border-top: 1px solid 095676;} ");
        ui->Catalogs_line_SeparateSummary_02->setStyleSheet("QFrame { color: #095676; border-top: 1px solid 095676; } ");
        ui->Storage_line_Separate->setStyleSheet("QFrame { color: #095676; border-top: 1px solid 095676; } ");

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

    }
    //----------------------------------------------------------------------
    void MainWindow::loadCustomThemeDark()
    {
        //Standard colors:
            //blue light	39b2e5
            //blue dark		10a2df  0D79A6
            //green light	81d41a
            //green dark	43bf0c
            //orange light	ff8000
            //orange dark	e36600
            //purple light	a1467e
            //purple dark	8b1871

        //Tab widget, including combo boxes and buttons

            QString styleSheetText = QLatin1String(R"(
                        QTabWidget            { padding: 10px; margin: 0px; background-color: #095676; }
                        QTabWidget::tab-bar   { left: 0px; height: 38px;}

                        QTabBar               { background:  url(:images/Appname_Logo.png) no-repeat right; background-color: #0D79A6;
                                                border-top-left-radius:  3px;
                                                border-top-right-radius: 3px;
                                              }
                        QTabBar::pane         { border-bottom: 0px solid #C2C7CB; }

                        QTabBar:tab:first     { margin-left:  6px; }
                        QTabBar::tab          { background-color: #0D79A6; color: #000;
                                                padding-top: 3px; padding-bottom: 6px; padding-left:  6px; padding-right: 10px;
                                                margin-top: 6px; margin-bottom: 0px;
                                                border-top-left-radius:  3px;
                                                border-top-right-radius: 3px;
                        }

                        QTabBar::tab::hover   { background-color: #095676; color: #FFF; }

                        QTabBar::tab:selected { background-color: #2a2e32; color: #FFF;
                                                /*background: qlineargradient(x1: 0, y1: 0, x2: 0, y2: 1, stop: 0 #8b1871, stop: 1 #eff0f1); */
                        }

                        QTabBar::tab:!selected{  }

            )");

            ui->tabWidget->setStyleSheet(styleSheetText);

        //Colored buttons
        ui->Search_pushButton_Search->setStyleSheet(
                "QPushButton           { background-color: #81d41a; } "
                "QPushButton::hover    { background: #39b2e5; color: #fff; border: 1px solid #39b2e5; border-radius: 5px;	padding: 5px;}"
                "QPushButton::pressed  { background: #39b2e5; color: #fff; border: 1px solid #39b2e5; border-radius: 5px;	padding: 5px;}"
              );
        ui->Catalogs_pushButton_UpdateCatalog->setStyleSheet(
                "QPushButton           { background-color: #ff8000; } "
                "QPushButton::hover    { background: #39b2e5; color: #fff; border: 1px solid #39b2e5; border: 1px solid #39b2e5; 	border-radius: 5px;	padding: 5px;}"
                "QPushButton::pressed  { background: #39b2e5; color: #fff; border: 1px solid #39b2e5; border: 1px solid #39b2e5; 	border-radius: 5px;	padding: 5px;}"
                "QPushButton::disabled { background-color: #BBB; border: 1px solid #AAA; border-radius: 5px;	padding: 5px;}"
              );
        ui->Catalogs_pushButton_UpdateAllActive->setStyleSheet(
                "QPushButton           { background-color: #ff8000; } "
                "QPushButton::hover    { background: #39b2e5; color: #fff; border: 1px solid #39b2e5; border: 1px solid #39b2e5; 	border-radius: 5px;	padding: 5px;}"
                "QPushButton::pressed  { background: #39b2e5; color: #fff; border: 1px solid #39b2e5; border: 1px solid #39b2e5; 	border-radius: 5px;	padding: 5px;}"
              );
        ui->Storage_pushButton_Update->setStyleSheet(
                "QPushButton           { background-color: #ff8000; } "
                "QPushButton::hover    { background: #39b2e5; color: #fff; border: 1px solid #39b2e5; border: 1px solid #39b2e5; 	border-radius: 5px;	padding: 5px;}"
                "QPushButton::pressed  { background: #39b2e5; color: #fff; border: 1px solid #39b2e5; border: 1px solid #39b2e5; 	border-radius: 5px;	padding: 5px;}"
                "QPushButton::disabled { background-color: #BBB; border: 1px solid #AAA; border-radius: 5px;	padding: 5px;}"
              );
        ui->Catalogs_pushButton_Save->setStyleSheet(
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
        ui->Search_line_SeparateResults->setStyleSheet("QFrame { color: #095676; border-top: 1px solid 095676; } ");
        ui->Explore_line_Separate->setStyleSheet("QFrame { color: #095676; border-top: 1px solid 095676;} ");
        ui->Statistics_line_Separate->setStyleSheet("QFrame { color: #095676; border-top: 1px solid 095676;} ");
        ui->Catalogs_line_SeparateSummary_02->setStyleSheet("QFrame { color: #095676; border-top: 1px solid 095676; } ");
        ui->Storage_line_Separate->setStyleSheet("QFrame { color: #095676; border-top: 1px solid 095676; } ");

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
                if ( lastestVersion > currentVersion ){

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

