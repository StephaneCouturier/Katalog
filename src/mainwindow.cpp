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
 * /////////////////////////////////////////////////////////////////////////////
// Application: Katalog
// File Name:   mainwindow.cpp
// Purpose:     Class for the main window
// Description: intiate the User Interface, load data into the internal database, recover last user position and display data
// Author:      Stephane Couturier
/////////////////////////////////////////////////////////////////////////////
*/
#include "mainwindow.h"
#include "ui_mainwindow.h"

#include "mainwindow_setup.cpp"
#include "mainwindow_tab_create.cpp"
#include "mainwindow_tab_search_ui.cpp"
#include "mainwindow_tab_search_pr.cpp"
#include "mainwindow_tab_tags.cpp"

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    searchMemory(nullptr),
    searchStoppable(nullptr),
    ui(new Ui::MainWindow),
    searchProcess(nullptr),
    isSearchRunning(false)
{
    // Only create memorySearch initially
    searchMemory = new SearchMemory(this);

    // Both currentSearch and lastSearch point to searchMemory
    currentSearch = searchMemory;
    lastSearch = searchMemory;

    //Set current version, release date, and development mode
        currentVersion  = "2.5";
        collection->appVersion = currentVersion;
        releaseDate     = "2025-05-08";
        developmentMode = false;

        //Detect if the theme is light or dark
        QPalette palette = QApplication::palette();
        bool isDarkTheme = palette.color(QPalette::Window).value() < 128;
        if (isDarkTheme) {
            themeID = 2; //default value for the theme = Katalog Colors (dark).
        }
        else {
            themeID = 1; //default value for the theme = Katalog Colors (light).
        }

        selectedTab = 3; //default value for the first launch = Create screen.

        //Set up the statusbar timer
        statusBarTimer = new QTimer(this);
        statusBarTimer->setSingleShot(true);
        connect(statusBarTimer, &QTimer::timeout, this, [this]() {
            statusBar()->hide();
        });
        // Hide status bar initially
        statusBar()->hide();

    //Prepare paths, user setting file, check version
        //Get user home path and application dir path
            QStringList standardsPaths = QStandardPaths::standardLocations(QStandardPaths::HomeLocation);
            QString homePath = standardsPaths[0];
            QString applicationDirPath = QCoreApplication::applicationDirPath();

        //Define Setting file path and name
            //For portable mode, check if there is a settings file located with the executable
            collection->settingsFilePath = applicationDirPath + "/katalog_settings.ini";
            QFile settingsFile(collection->settingsFilePath);
            if(!settingsFile.exists()) {
                //otherwise fall back to default katalog_settings path
                collection->settingsFilePath = homePath + "/.config/katalog_settings.ini";
            }
            QSettings settings(collection->settingsFilePath, QSettings:: IniFormat);

        //Check for new version
            checkVersionChoice = settings.value("Settings/CheckVersion", true).toBool();
            if ( checkVersionChoice == true)
                checkVersion();

    //Set up and start database (modes: "Memory", "File", or "Hosted")
        startDatabase();

    //Set up the interface globally
        //Set up the User Interface
            ui->setupUi(this);

            if(developmentMode==false){
                hideDevelopmentUIItems();
            }

            QButtonGroup buttonGroupDevices;
            buttonGroupDevices.addButton(ui->Devices_radioButton_DeviceTree);
            buttonGroupDevices.addButton(ui->Devices_radioButton_StorageList);
            buttonGroupDevices.addButton(ui->Devices_radioButton_CatalogList);

            QButtonGroup buttonGroupBackUp;
            buttonGroupBackUp.addButton(ui->BackUp_radioButton_Source);
            buttonGroupBackUp.addButton(ui->BackUp_radioButton_Target);

            QButtonGroup buttonGroupCreateFilesType;
            buttonGroupCreateFilesType.addButton(ui->Create_radioButton_FileType_Any);
            buttonGroupCreateFilesType.addButton(ui->Create_radioButton_FileType_Audio);
            buttonGroupCreateFilesType.addButton(ui->Create_radioButton_FileType_Image);
            buttonGroupCreateFilesType.addButton(ui->Create_radioButton_FileType_Text);
            buttonGroupCreateFilesType.addButton(ui->Create_radioButton_FileType_Video);

        //Settings screen
            ui->Settings_lineEdit_DatabaseFilePath->setText(collection->databaseFilePath);
            ui->Settings_comboBox_DatabaseMode->setItemData(0, "Memory", Qt::UserRole);
            ui->Settings_comboBox_DatabaseMode->setItemData(1, "File", Qt::UserRole);
            ui->Settings_comboBox_DatabaseMode->setItemData(2, "Hosted", Qt::UserRole);
            ui->Settings_lineEdit_DataMode_Hosted_HostName->setText(collection->databaseHostName);
            ui->Settings_lineEdit_DataMode_Hosted_DatabaseName->setText(collection->databaseName);
            ui->Settings_lineEdit_DataMode_Hosted_Port->setText(QVariant(collection->databasePort).toString());
            ui->Settings_lineEdit_DataMode_Hosted_UserName->setText(collection->databaseUserName);
            ui->Settings_lineEdit_DataMode_Hosted_Password->setText(collection->databasePassword);
            ui->Settings_label_VersionValue->setText(currentVersion);
            ui->Settings_label_DateValue->setText(releaseDate);

        //Load languages to the Settings combobox, keeping the user's selection
            QString userLanguage = settings.value("Settings/Language").toString();
            ui->Settings_comboBox_Language->addItem(QIcon(":/images/flags/de.png"),"de_DE");
            ui->Settings_comboBox_Language->addItem(QIcon(":/images/flags/cz.png"),"cz_CZ");
            ui->Settings_comboBox_Language->addItem(QIcon(":/images/flags/us.png"),"en_US");
            ui->Settings_comboBox_Language->addItem(QIcon(":/images/flags/fr.png"),"fr_FR");
            ui->Settings_comboBox_Language->setCurrentText(userLanguage);

        //Hide some widgets by default
            ui->Statistics_calendarWidget->hide();
            ui->Devices_widget_Edit->hide();
            ui->Devices_widget_ReplaceCatalogsOption->hide();
            ui->Search_widget_DifferencesDevices->hide();
            ui->Create_widget_SambaSettings->hide();

        //Hide file edtion items
            if( collection->databaseMode != "Memory"){
                ui->Devices_pushButton_EditList->hide();
                ui->Statistics_pushButton_EditDeviceStatisticsFile->hide();
                ui->Statistics_pushButton_Reload->hide();
                ui->Tags_pushButton_OpenTagsFile->hide();
            }

        //Load all other Settings and apply values
            loadSettings();

        //Load custom stylesheet
            //for windows, pick a windows common font.
            #ifdef Q_OS_WIN
            ui->tabWidget->setStyleSheet("font-family: calibri; font-size: 16px;");
            ui->splitter_widget_Filters->setStyleSheet("font-family: calibri; font-size: 16px;");
            #endif

            //load custom Katalog stylesheet instead of default theme
            if ( themeID == 1 ){
                loadCustomThemeLight();
            }
            else if ( themeID == 2 ){
                loadCustomThemeDark();
            }

    //Load Collection data
            // Only create searchMemory initially
            searchMemory = new SearchMemory(this);

            // Both currentSearch and lastSearch point to searchMemory
            currentSearch = searchMemory;
            lastSearch = searchMemory;

        //Load Collection
            loadCollection();
            selectedDevice->loadDevice("defaultConnection");
            filterFromSelectedDevice();

            //Load mapping to backup tab
            loadBackUpMapping();

        //Restore last opened catalog to Explore tab
            if(ui->Settings_checkBox_LoadLastCatalog->isChecked()==true){
                exploreDevice->ID = settings.value("Explore/lastExploreDeviceID").toInt();
                exploreDevice->loadDevice("defaultConnection");
                exploreSelectedFolderFullPath = settings.value("Explore/lastExploreSelectedFolderFullPath").toString();
                exploreSelectedDirectoryName  = settings.value("Explore/lastExploreSelectedDirectoryName").toString();

                openCatalogToExplore();
            }

    //Preload last selected catalogs contents to memory
        if(ui->Settings_checkBox_PreloadCatalogs->isChecked()==true){
            preloadCatalogs();
        }

    //Setup tabs

        //Setup tab: Create
            //Default path to scan
            ui->Create_lineEdit_NewCatalogPath->setText("/");

            //Always Load the file system for the treeview
            loadFileSystem("/");

            //Load the list of Storage devices for Create and Catalog tabs
            loadStorageList();

        //Setup tab: Tags
            //Set Default path to scan
            ui->Tags_lineEdit_FolderPath->setText("/");

            loadFileSystemTags(newTagFolderPath);
            reloadTagsData();

        //Setup tab: Settings
            //Load path of last collection used
            ui->Settings_lineEdit_CollectionFolder->setText(collection->folder);

            //Apply databaseMode in Settings tab
            QMap<QString, QString> databaseModeTranslations = {
                                                               {"Memory", tr("Memory")},
                                                               {"File",   tr("File")},
                                                               {"Hosted", tr("Hosted")},
                                                               };
            QString trMode = databaseModeTranslations.value(collection->databaseMode, collection->databaseMode);
            ui->Settings_comboBox_DatabaseMode->setCurrentText(trMode);
            applyDatabaseModeToUI();

            //Set file types
            setFileTypes();

            //Set apply button to disabled
            ui->Settings_pushButton_ApplyFolderpath->setEnabled(false);
            ui->Settings_pushButton_ApplyFilepath->setEnabled(false);

        //Setup tab: Explore
            ui->Explore_checkBox_DisplayFolders->setChecked(optionDisplayFolders);
            ui->Explore_checkBox_DisplaySubFolders->setChecked(optionDisplaySubFolders);

        //Setup tab: Storage
            unsavedChanges = false;

        //Setup tab: Devices
            ui->Devices_checkBox_DisplayCatalogs->setChecked(optionDisplayCatalogs);
            ui->Devices_checkBox_DisplayStorage->setChecked(optionDisplayStorage);
            ui->Devices_checkBox_DisplayPhysicalGroup->setChecked(optionDisplayPhysicalGroup);
            ui->Devices_checkBox_DisplayVirtualGroups->setChecked(optionDisplayVirtualGroups);
            ui->Devices_checkBox_DisplayFullTable->setChecked(optionDisplayFullDeviceTable);
            loadParentsList();

            QString displayContents = settings.value("Devices/DisplayContents").toString();
            if(displayContents=="Tree")
                ui->Devices_radioButton_DeviceTree->setChecked(true);
            if(displayContents=="Storage")
                ui->Devices_radioButton_StorageList->setChecked(true);
            if(displayContents=="Catalogs")
                ui->Devices_radioButton_CatalogList->setChecked(true);

        //Setup tab: Search
            //Default values
            initiateSearchFields();
            resetToDefaultSearchCriteria();

            //Load an empty model to display headers
            Catalog *emptyCatalog = new Catalog;
            ui->Search_treeView_FilesFound->setModel(emptyCatalog);
            QStandardItemModel *emptyQStandardItemModel = new QStandardItemModel;
            ui->Search_treeView_CatalogsFound->setModel(emptyQStandardItemModel);

            //Restore last Search values
            QSqlQuery query(QSqlDatabase::database("defaultConnection"));
            QString querySQL = QLatin1String(R"(
                                SELECT MAX(date_time)
                                FROM search
                            )");
            query.prepare(querySQL);
            query.exec();
            query.next();

            lastSearch->searchDateTime = query.value(0).toString();
            lastSearch->loadSearchHistoryCriteria("defaultConnection");
            loadSearchCriteria(lastSearch);

            //Restore last Search values
            filterFromSelectedDevice();

        //Setup tab: BackUp
            ui->BackUp_checkBox_DisplayFullTable->setChecked(optionDisplayFullMappingTable);

    //Context menu and other slots and signals
            setupFileContextMenus();

            //Header Order change
            connect(ui->Devices_treeView_DeviceList->header(), &QHeaderView::sortIndicatorChanged,
                    this, &::MainWindow::on_DevicesTreeViewDeviceListHeaderSortOrderChanged);

            connect(ui->Explore_treeView_FileList->header(), &QHeaderView::sortIndicatorChanged,
                    this, &::MainWindow::on_ExploreTreeViewFileListHeaderSortOrderChanged);

            connect(ui->Search_treeView_FilesFound->header(), &QHeaderView::sortIndicatorChanged,
                    this, &::MainWindow::on_SearchTreeViewFilesFoundHeaderSortOrderChanged);

            connect(ui->Search_treeView_History->header(), &QHeaderView::sortIndicatorChanged,
                    this, &::MainWindow::on_SearchTreeViewHistoryHeaderSortOrderChanged);

    //Restore sorting of views
            ui->Devices_treeView_DeviceList->QTreeView::sortByColumn(lastDevicesSortSection,Qt::SortOrder(lastDevicesSortOrder));
            ui->Explore_treeView_FileList->QTreeView::sortByColumn(lastExploreSortSection,Qt::SortOrder(lastExploreSortOrder));
            ui->Search_treeView_FilesFound->QTreeView::sortByColumn(lastSearchSortSection,Qt::SortOrder(lastSearchSortOrder));
            ui->Search_treeView_History->QTreeView::sortByColumn(lastSearchHistorySortSection,Qt::SortOrder(lastSearchHistorySortOrder));
}

MainWindow::~MainWindow()
{
    delete ui;
    /*
    if (searchProcess) {
        searchProcess->stop();
        searchProcess->wait();
        delete searchProcess;
    }
    delete newSearch; // Ensure newSearch is deleted
    */
}

void MainWindow::closeEvent (QCloseEvent *event)
{
    if(unsavedChanges == true){

        int result = QMessageBox::warning(this,"Katalog",
                    tr( "Some changes in the Storage device list have not been saved.<br/>"
                        "Save and exit, discard and exit, or cancel exit?"),
                    QMessageBox::Save|QMessageBox::Discard|QMessageBox::Cancel);
        if ( result ==QMessageBox::Cancel){
            event->ignore();
            return;
        }
        else if ( result ==QMessageBox::Save){
            //Save model data to Storage file
            collection->saveStorageTableToFile();

            //Reload Storage file data to table
            collection->loadStorageFileToTable();

            event->accept();
            return;
        }
        else if ( result ==QMessageBox::Discard){
            event->accept();
            return;
        }
    }
}

//DEV: Templates
/*
qDebug()<<"DEBUG value: " << value <<QDateTime::currentDateTime();

QMessageBox msgBox;
msgBox.setWindowTitle("Katalog");
msgBox.setText(tr("anyVariable")+": <br/>" + QVariant(anyVariable).toString());
msgBox.setText(QCoreApplication::translate("MainWindow",
                                    "Text<br/>"
                                    "<br/> Text with first parameter<br/><b>%1</b><br/>"
                                    "<br/> Text with first parameter<br/><b>%2</b><br/>"
                                    ).arg( parameter1, tempText.toUtf8() ));
msgBox.setIcon(QMessageBox::Information);
msgBox.exec();

        QSqlQuery query(QSqlDatabase::database("defaultConnection"));
        QString querySQL = QLatin1String(R"(
                                    SELECT *
                                    FROM table
                                    WHERE 1=1
                                )");
        query.prepare(querySQL);
        query.exec();
        qDebug()<<"DEBUG: query: "<<query.lastError();
        while(query.next()){
            qDebug()<<query.value(0).toString()<<query.value(1).toString();
        }
*/
