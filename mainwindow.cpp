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

//Include class headers
    #include "mainwindow.h"
    #include "ui_mainwindow.h"

//Include other mainwindow methods

    //Setup and main object classes
    #include "mainwindow_setup.cpp"
    //#include "collection.cpp"
    //#include "storage.cpp"

    //Application tabs
    #include "mainwindow_tab_search.cpp"
    #include "mainwindow_tab_create.cpp"
    #include "mainwindow_tab_catalogs.cpp"
    #include "mainwindow_tab_storage.cpp"
    #include "mainwindow_tab_tags.cpp"

#ifdef Q_OS_LINUX
MainWindow::MainWindow(QWidget *parent) : KXmlGuiWindow(parent)
#else
MainWindow::MainWindow(QWidget *parent) : QMainWindow(parent)
#endif
   , ui(new Ui::MainWindow)
{
    //setup: start database
            startDatabase();

    //Set up the interface globally
        //Set up the User Interface
            ui->setupUi(this);

            //Set current version and date
            currentVersion = "1.12";
            releaseDate = "2022-05-01";
            ui->Settings_label_VersionValue->setText(currentVersion);
            ui->Settings_label_DateValue->setText(releaseDate);

            //Check if new version is available
                     checkVersionChoice = ui->Settings_checkBox_CheckVersion->isChecked();
                     if ( checkVersionChoice == true)
                         checkVersion();

            //Set Development mode
            developmentMode = false;

            //Hide Development UI items that are not ready for use
            if(developmentMode==false){
                hideDevelopmentUIItems();
            }

            //Load languages to the Settings combobox
            ui->Settings_comboBox_Language->addItem(QIcon(":/images/flags/de.png"),"de_DE");
            ui->Settings_comboBox_Language->addItem(QIcon(":/images/flags/cz.png"),"cz_CZ");
            ui->Settings_comboBox_Language->addItem(QIcon(":/images/flags/us.png"),"en_US");
            ui->Settings_comboBox_Language->addItem(QIcon(":/images/flags/fr.png"),"fr_FR");

            //Hide some widget by default
            ui->Catalogs_widget_EditCatalog->hide();

            //For Linux, use KDE libs
            #ifdef Q_OS_LINUX
                //Set up KDE Menu/Icon actions
                setupActions();
                //Hide the lineEdit used for Windows as the Linux version uses a KDE library that is not implement in the windows version
                ui->Search_lineEdit_SearchText->hide();
            #endif

        //Load user settings
            //Get user home path and application dir path
            QStringList standardsPaths = QStandardPaths::standardLocations(QStandardPaths::HomeLocation);
            QString homePath = standardsPaths[0];
            QString applicationDirPath = QCoreApplication::applicationDirPath();

            //Define Setting file path and name
                //For portable mode, check if there is a settings file located with the executable
                settingsFilePath = applicationDirPath + "/katalog_settings.ini";
                QFile settingsFile(settingsFilePath);
                if(!settingsFile.exists()) {
                    //otherwise fall back to default katalog_settings path
                    settingsFilePath = homePath + "/.config/katalog_settings.ini";
                }

            //Load Settings and intiate values
            loadSettings();

            //Memorize filter selection from setting before refreshing
            QString firstSelectedLocation = selectedSearchLocation;
            QString firstSelectedStorage  = selectedSearchStorage;
            QString firstSelectedCatalog  = selectedSearchCatalog;

        //Load custom stylesheet
            //for windows, pick a windows common font.
            #ifdef Q_OS_WIN
            ui->tabWidget->setStyleSheet("font-family: calibri; font-size: 16px;");
            ui->splitter_widget_Filters_tabWidget->setStyleSheet("font-family: calibri; font-size: 16px;");
            #endif

            //load custom Katalog stylesheet instead of default theme
            if ( ui->Settings_comboBox_Theme->currentText() == tr("Katalog Colors (light)") ){
                loadCustomThemeLight();
            }


    //Load Collection data
            //Load Collection data from csv files
                loadCollection();

            //Preload last selected catalogs contents to memory
                if(ui->Settings_checkBox_PreloadCatalogs->isChecked()==true){
                    preloadCatalogs();
                }
            //Load last opened catalog to Explore tab
                ui->Explore_label_CatalogDirectoryDisplay->setText(selectedDirectoryName);
                if (selectedCatalogFile != ""){
                    openCatalogToExplore();
                }


    //Setup tabs

        //Setup tab: Create
            //Default path to scan
                ui->Create_lineEdit_NewCatalogPath->setText("/");

            //Define path of file containing folders to exclude when cataloging
                excludeFilePath = collectionFolder +"/"+ "exclude.csv";

            //Always Load the file system for the treeview
                loadFileSystem("/");

            //Load the list of Storage devices for Create and Catalog tabs
                loadStorageList();

        //Setup tab: Tags
                //Default path to scan
                ui->Tags_lineEdit_FolderPath->setText("/");
                loadFileSystemTags(newTagFolderPath);
                reloadTagsData();

        //Setup tab: Settings
            //Load path of last collection used
                ui->Collection_lineEdit_CollectionFolder->setText(collectionFolder);

            //Set file types
                setFileTypes();

            //Restore filters
                ui->Filters_comboBox_SelectLocation->setCurrentText(firstSelectedLocation);
                ui->Filters_comboBox_SelectStorage->setCurrentText(firstSelectedStorage);
                ui->Filters_comboBox_SelectCatalog->setCurrentText(firstSelectedCatalog);

        //Setup tab: Search
                //Load an empty model to display headers
                Catalog *empty = new Catalog(this);
                ui->Search_treeView_FilesFound->setModel(empty);
                ui->Search_listView_CatalogsFound->setModel(empty);

                //Initiate and restore Search values
                initiateSearchValues();

    //Context menu and other slots and signals
            setupFileContextMenu();

            //Header Order change
            connect(ui->Catalogs_treeView_CatalogList->header(), SIGNAL(sortIndicatorChanged(int,Qt::SortOrder)),
                    this , SLOT(on_Catalogs_treeView_CatalogList_HeaderSortOrderChanged()) );
            connect(ui->Storage_treeView_StorageList->header(), SIGNAL(sortIndicatorChanged(int,Qt::SortOrder)),
                    this , SLOT(on_Storage_treeView_StorageList_HeaderSortOrderChanged()) );
            connect(ui->Explore_treeView_FileList->header(), SIGNAL(sortIndicatorChanged(int,Qt::SortOrder)),
                    this , SLOT(on_Explore_treeView_FileList_HeaderSortOrderChanged()) );
            connect(ui->Search_treeView_FilesFound->header(), SIGNAL(sortIndicatorChanged(int,Qt::SortOrder)),
                    this , SLOT(on_Search_treeView_FilesFound_HeaderSortOrderChanged()) );
            connect(ui->Search_treeView_History->header(), SIGNAL(sortIndicatorChanged(int,Qt::SortOrder)),
                    this , SLOT(on_Search_treeView_History_HeaderSortOrderChanged()) );

    //Restore sorting of views
            ui->Catalogs_treeView_CatalogList->QTreeView::sortByColumn(lastCatalogsSortSection,Qt::SortOrder(lastCatalogsSortOrder));
            ui->Storage_treeView_StorageList->QTreeView::sortByColumn(lastStorageSortSection,Qt::SortOrder(lastStorageSortOrder));
            ui->Explore_treeView_FileList->QTreeView::sortByColumn(lastExploreSortSection,Qt::SortOrder(lastExploreSortOrder));
            ui->Search_treeView_FilesFound->QTreeView::sortByColumn(lastSearchSortSection,Qt::SortOrder(lastSearchSortOrder));
            ui->Search_treeView_History->QTreeView::sortByColumn(lastSearchHistorySortSection,Qt::SortOrder(lastSearchHistorySortOrder));

}

MainWindow::~MainWindow()
{
      delete ui;
}

//DEV Templates
/*
QMessageBox::information(this,"Katalog","Ok.");
QMessageBox::information(this,"Katalog","stringVariable: <br/>" + stringVariable);
QMessageBox::information(this,"Katalog","anyVariable: <br/>" + QVariant(anyVariable).toString());

QSqlQuery query;
QString querySQL = QLatin1String(R"(

                                )");
query.prepare(querySQL);
query.exec();
*/
