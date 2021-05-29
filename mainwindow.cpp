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
// Version:     1.00
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
   //Set up interface globally
        //Set up the User Interface
            ui->setupUi(this);

            currentVersion = "1.00";
            releaseDate = "2021-05-29";
            ui->Settings_label_VersionValue->setText(currentVersion);
            ui->Settings_label_DateValue->setText(releaseDate);

            //Load languages to the Settings combobox
            ui->Settings_comboBox_Language->addItem(QIcon(":/images/flags/us.png"),"en_US");
            ui->Settings_comboBox_Language->addItem(QIcon(":/images/flags/fr.png"),"fr_FR");
            //ui->Settings_comboBox_Language->addItem(QIcon(":/images/flags/cz.png"),"cz_CZ");


        //Hide user interface items that are not ready for use (under development).
            hideDevelopmentUIItems();
            ui->Catalogs_widget_EditCatalog->hide();

        //Set up KDE Menu/Icon actions
            #ifdef Q_OS_LINUX
            setupActions();
            #endif

        //Load user settings
            //Get user home path
            QStringList standardsPaths = QStandardPaths::standardLocations(QStandardPaths::HomeLocation);
            QString homePath = standardsPaths[0];

            //Define Setting file path and name
            settingsFilePath = homePath + "/.config/katalog_settings.ini";

            //load the settings
            loadSettings();
            QString firstSelectedLocation = selectedSearchLocation;
            QString firstSelectedStorage =  selectedSearchStorage;
            QString firstSelectedCatalog = selectedSearchCatalog;

    //load custom stylesheet
            //for windows, pick a windows common font.
            #ifdef Q_OS_WIN
            ui->tabWidget->setStyleSheet("font-family: calibri; font-size: 16px;");
            ui->Global_tabWidget->setStyleSheet("font-family: calibri; font-size: 16px;");
            #endif

            //load custom Katalog stylesheet instead of default theme
            if ( ui->Settings_comboBox_Theme->currentText() == tr("Katalog Colors (light)") ){
                loadCustomThemeLight();
            }

    //setup: start database
            startDatabase();

    //setup tab: Collection
            //setup tab: Search
            //setup tab: Storage

            initiateSearchValues();

            storageFilePath = collectionFolder + "/" + "storage.csv";

            loadCollection();
            refreshLocationCollectionFilter();

    //setup tab: Create
        //Default path to scan
            ui->Create_lineEdit_NewCatalogPath->setText("/");
        //Always Load the file system for the treeview
            loadFileSystem("/");
        //Load the list of Storage devices
            loadStorageList();

    //setup tab: Tags
            //Default path to scan
            ui->Tags_lineEdit_FolderPath->setText("/");
            //Always Load the file system for the treeview
            loadFileSystemTags("/");
            loadFolderTagModel();

    //setup tab: Settings
        //Load last collection used
            //Send collection folder to the line edit
            ui->Collection_lineEdit_CollectionFolder->setText(collectionFolder);
        //Set file types
            setFileTypes();
            setupFileContextMenu();

     //Setup tap: Stats
            loadStatisticsDataTypes();
            loadStatisticsChart();

            ui->Filters_comboBox_SelectLocation->setCurrentText(firstSelectedLocation);
            ui->Filters_comboBox_SelectStorage->setCurrentText(firstSelectedStorage);
            ui->Filters_comboBox_SelectCatalog->setCurrentText(firstSelectedCatalog);

    //check if new version is available
            checkVersionChoice = ui->Settings_checkBox_CheckVersion->isChecked();
            if ( checkVersionChoice == true)
                checkVersion();

}

MainWindow::~MainWindow()
{
      delete ui;
}

//DEV
/*
QMessageBox::information(this,"Katalog","Ok.");
QMessageBox::information(this,"Katalog","Ok." + stringVariable);
QMessageBox::information(this,"Katalog","variable : \n" + QString::number(numbervariable));

QSqlQuery query;
QString querySQL = QLatin1String(R"(

                                )");
query.prepare(querySQL);
query.exec();
*/
