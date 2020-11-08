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
// Description:
// Author:      Stephane Couturier
// Modified by: Stephane Couturier
// Created:     2020-07-11
// Version:     0.9
/////////////////////////////////////////////////////////////////////////////
*/

//Include classes
    #include "mainwindow.h"
    #include "ui_mainwindow.h"
    //#include "initdb.h"

//Include other mainwindow methods

    //SETUP: Menu and Icons - Actions KDE setup
    #include "mainwindow_setup.cpp"

    //Main objects
    #include "collection.cpp"
    #include "storage.cpp"

    //TABS:
    #include "mainwindow_tab_search.cpp"
    #include "mainwindow_tab_create.cpp"
    #include "mainwindow_tab_collection.cpp"
    #include "mainwindow_tab_storage.cpp"
    #include "mainwindow_tab_tags.cpp"

MainWindow::MainWindow(QWidget *parent) : QMainWindow(parent)// KXmlGuiWindow(parent)
   , ui(new Ui::MainWindow)
{

   //Set up interface globally
        //Set up GUI
            ui->setupUi(this);

        //Set icon fallback in case theme is not available
            QIcon::setFallbackSearchPaths(QIcon::fallbackSearchPaths() << ":icons");

        //DEV: start the database
            //startSQLDB();
        //DEV: test translation
            //QMessageBox(tr("Folder"));
            //ui->label_test->setText(i18n("Folder"));


        //Hide user interface items that are not ready for use (under development).
            hideDevelopmentUIItems();

        //Set up KDE Menu/Icon actions
            //setupActions();

        //Load user settings
            //Get user home path
            QStringList standardsPaths = QStandardPaths::standardLocations(QStandardPaths::HomeLocation);
            QString homePath = standardsPaths[0];
            //Define Setting file path and name
            settingsFile = homePath + "/.config/katalog_settings.ini";
            //settingsFile = QApplication::applicationDirPath() + "/katalog_settings.ini";
            //load the settings
            loadSettings();

    //load custom stylesheet
                //if ( ui->Settings_ChBx_SaveRecordWhenUpdate->isChecked() == true )
                //if ( ui->Settings_checkBox_UseDefaultTheme->isChecked() == false ){

                //for windows, pick a windows common font.
                #ifdef Q_OS_WIN
                ui->tabWidget->setStyleSheet(font-family: calibri;
                      );
                #endif

                //load custom Katalog stylesheet instead of default theme
                if ( ui->Settings_comboBox_Theme->currentText() == "Katalog Colors (light)" ){
                    loadCustomTheme1();
                }

    //setup tab: Collection
        //Load the list of catalogs from the collection folder
            loadCatalogsToModel();

    //setup tab: Explore
            exploreLoadDirectories();

    //setup tab: Search
            initiateSearchValues();
            refreshCatalogSelectionList();

            ui->CB_SelectCatalog->setCurrentText(selectedSearchCatalog);

    //setup tab: Storage
            storageFilePath = collectionFolder + "/" + "storage.csv";
            loadStorageModel();

    //setup tab: Create
        //Default path to scan
            ui->LE_NewCatalogPath->setText("/");
        //Always Load the file system for the treeview
            LoadFileSystem("/");
        //Load list of Storage
            loadStorageList();

    //setup tab: Tags
            //Default path to scan
            ui->LE_TagFolderPath->setText("/");
            //Always Load the file system for the treeview
            loadFileSystemTags("/");
            loadFolderTagModel();

    //setup tab: Settings
        //Load last collection used
            //Send collection folder to the line edit
            ui->LE_CollectionFolder->setText(collectionFolder);
        //Set file types
            setFileTypes();
            //DEV: interface to edit
            FileTypesEditor();
            setupFileContextMenu();

     //Setup tap: Stats
            loadTypeOfData();
            statsLoadChart();
            //statsLoadChart2();
}

MainWindow::~MainWindow()
{
      delete ui;
}

//DEV
/*
QMessageBox::information(this,"Katalog","Ok.");
KMessageBox::information(this,"test:\n");
*/
