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
#include "mainwindow_tab_search.cpp"
#include "mainwindow_tab_create.cpp"
#include "mainwindow_tab_catalogs.cpp"
#include "mainwindow_tab_storage.cpp"
#include "mainwindow_tab_tags.cpp"

//#ifdef Q_OS_LINUX
//MainWindow::MainWindow(QWidget *parent) : KXmlGuiWindow(parent), ui(new Ui::MainWindow)
//#else
MainWindow::MainWindow(QWidget *parent) : QMainWindow(parent), ui(new Ui::MainWindow)
//#endif
{
    //Set current version and release date, and check new version
        currentVersion  = "1.17";
        releaseDate     = "2023-03-14";
        developmentMode = false;

    //Prepare paths, user setting file, check version
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
            QSettings settings(settingsFilePath, QSettings:: IniFormat);

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

            ui->Settings_lineEdit_DatabaseFilePath->setText(databaseFilePath);
            ui->Settings_comboBox_DatabaseMode->setItemData(0, "Memory", Qt::UserRole);
            ui->Settings_comboBox_DatabaseMode->setItemData(1, "File", Qt::UserRole);
            ui->Settings_comboBox_DatabaseMode->setItemData(2, "Hosted", Qt::UserRole);
            ui->Settings_lineEdit_DataMode_Hosted_HostName->setText(databaseHostName);
            ui->Settings_lineEdit_DataMode_Hosted_DatabaseName->setText(databaseName);
            ui->Settings_lineEdit_DataMode_Hosted_Port->setText(QVariant(databasePort).toString());
            ui->Settings_lineEdit_DataMode_Hosted_UserName->setText(databaseUserName);
            ui->Settings_lineEdit_DataMode_Hosted_Password->setText(databasePassword);

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
            ui->Catalogs_widget_EditCatalog->hide();
            ui->Statistics_calendarWidget->hide();

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
            //Generate collection files paths and statistics parameters
                generateCollectionFilesPaths();

            //Create a Storage list (if none exists)
                if(databaseMode=="Memory"){
                    createStorageList();
                }

            //Load Collection data from csv files
                loadCollection();

            //Preload last selected catalogs contents to memory
                if(ui->Settings_checkBox_PreloadCatalogs->isChecked()==true){
                    preloadCatalogs();
                }

            //Load last opened catalog to Explore tab
                ui->Explore_label_CatalogDirectoryDisplay->setText(selectedDirectoryName);
                if (activeCatalog->filePath != ""){
                    openCatalogToExplore();
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
                ui->Settings_lineEdit_CollectionFolder->setText(collectionFolder);

            //Set file types
                setFileTypes();

        //Setup tab: Explore
                ui->Explore_checkBox_DisplayFolders->setChecked(optionDisplayFolders);
                ui->Explore_checkBox_DisplaySubFolders->setChecked(optionDisplaySubFolders);

        //Setup tab: Search
                //Load an empty model to display headers
                Catalog *empty = new Catalog(this);
                ui->Search_treeView_FilesFound->setModel(empty);
                ui->Search_listView_CatalogsFound->setModel(empty);

                //Initiate and restore Search values
                initiateSearchValues();

        //Setup tab: Storage
                unsavedChanges = false;

    //Context menu and other slots and signals
            setupFileContextMenu();

            //Header Order change
            connect(ui->Catalogs_treeView_CatalogList->header(), &QHeaderView::sortIndicatorChanged,
                    this, &::MainWindow::on_CatalogsTreeViewCatalogListHeaderSortOrderChanged);

            connect(ui->Storage_treeView_StorageList->header(), &QHeaderView::sortIndicatorChanged,
                    this, &::MainWindow::on_StorageTreeViewStorageListHeaderSortOrderChanged);

            connect(ui->Explore_treeView_FileList->header(), &QHeaderView::sortIndicatorChanged,
                    this, &::MainWindow::on_ExploreTreeViewFileListHeaderSortOrderChanged);

            connect(ui->Search_treeView_FilesFound->header(), &QHeaderView::sortIndicatorChanged,
                    this, &::MainWindow::on_SearchTreeViewFilesFoundHeaderSortOrderChanged);

            connect(ui->Search_treeView_History->header(), &QHeaderView::sortIndicatorChanged,
                    this, &::MainWindow::on_SearchTreeViewHistoryHeaderSortOrderChanged);

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

void MainWindow::closeEvent (QCloseEvent *event)
{
    if(unsavedChanges == true){

        int result = QMessageBox::warning(this,"Katalog",
                    tr( "Some changes in Storage list have not been saved.<br/>"
                        "Are you sure to quit without saving?"),
                    QMessageBox::Yes|QMessageBox::Cancel);
        if ( result ==QMessageBox::Cancel){
            event->ignore();
            return;
        }
        else if ( result ==QMessageBox::Yes){
            event->accept();
            return;
        }
    }
}

//DEV Templates
/*
qDebug()<<"DEBUG     value:    " << value;

QMessageBox msgBox; msgBox.setWindowTitle("Katalog"); msgBox.setText("value:<br/>"+QVariant(variable).toString()); msgBox.setIcon(QMessageBox::Information); msgBox.exec();

QMessageBox msgBox;
msgBox.setWindowTitle("Katalog");
msgBox.setText(tr("anyVariable")+": <br/>" + QVariant(anyVariable).toString());
msgBox.setIcon(QMessageBox::Information);
msgBox.exec();

QSqlQuery query;
QString querySQL = QLatin1String(R"(

                                )");
query.prepare(querySQL);
query.exec();
*/
