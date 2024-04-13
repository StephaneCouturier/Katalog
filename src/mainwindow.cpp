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
#include "mainwindow_tab_search.cpp"
#include "mainwindow_tab_tags.cpp"

MainWindow::MainWindow(QWidget *parent) : QMainWindow(parent), ui(new Ui::MainWindow)
{
    //Set current version, release date, and development mode
        currentVersion  = "2.0";
        releaseDate     = "2024-04-13";
        developmentMode = false;

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

            QButtonGroup buttonGroup;
            buttonGroup.addButton(ui->Devices_radioButton_DeviceTree);
            buttonGroup.addButton(ui->Devices_radioButton_StorageList);
            buttonGroup.addButton(ui->Devices_radioButton_CatalogList);

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

        //Hide file edtion items
            if( collection->databaseMode != "Memory"){
                ui->Statistics_pushButton_EditDeviceStatisticsFile->hide();
                ui->Statistics_pushButton_Reload->hide();
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
            if ( ui->Settings_comboBox_Theme->currentText() == tr("Katalog Colors (light)") ){
                loadCustomThemeLight();
            }
            else if ( ui->Settings_comboBox_Theme->currentText() == tr("Katalog Colors (dark)") ){
                loadCustomThemeDark();
            }

    //Load Collection data
        //Load Collection
            loadCollection();
            selectedDevice->loadDevice();
            filterFromSelectedDevice();

        //Restore last opened catalog to Explore tab
            if(ui->Settings_checkBox_LoadLastCatalog->isChecked()==true){
                exploreDevice->ID = settings.value("Explore/lastExploreDeviceID").toInt();
                exploreDevice->loadDevice();
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

            //last tree type selected
            QString selectedTreeType = settings.value("Filters/LastTreeType").toString();

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
            ui->Devices_checkBox_DisplayFullTable->setChecked(optionDisplayFullTable);
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
            QSqlQuery query;
            QString querySQL = QLatin1String(R"(
                                SELECT MAX(date_time)
                                FROM search
                            )");
            query.prepare(querySQL);
            query.exec();
            query.next();

            lastSearch->searchDateTime = query.value(0).toString();
            lastSearch->loadSearchHistoryCriteria();
            loadSearchCriteria(lastSearch);

            //Restore last Search values
            filterFromSelectedDevice();

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
qDebug()<<"DEBUG value: " << value;

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

        QSqlQuery query;
        QString querySQL = QLatin1String(R"(
                                    SELECT *
                                    FROM table
                                    WHERE 1=1
                                )");
        query.prepare(querySQL);
        query.exec();
        qDebug()<<query.lastError();
        while(query.next()){
            qDebug()<<query.value(0).toString()<<query.value(1).toString();
        }
*/
