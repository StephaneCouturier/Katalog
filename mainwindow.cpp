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
// Version:     0.1
/////////////////////////////////////////////////////////////////////////////
*/

//Include classes
    #include "mainwindow.h"
    #include "ui_mainwindow.h"

//Include other mainwindow methods
    //SETUP: Menu and Icons - Actions KDE setup
    #include "mainwindow_setup.cpp"
    #include "catalog.cpp"
    //TAB: Search files
    #include "mainwindow_tab_search.cpp"
    //TAB: Create Catalog
    #include "mainwindow_tab_create.cpp"
    //TAB: Explore Catalogs
    #include "mainwindow_tab_collection.cpp"
    //TAB: TESTS
    //TAB: Find Duplicates
    //TAB: Statistiques

MainWindow::MainWindow(QWidget *parent) : KXmlGuiWindow(parent)
   , ui(new Ui::MainWindow)
{

   //Start up
        //Set up GUI
            ui->setupUi(this);
            HideDevelopmentUIItems();
        //Set up KDE Menu/Icon actions
            setupActions();
        //Load settings
            settingsFile = QApplication::applicationDirPath() + "/katalog_settings.ini";
            loadSettings();

    //TAB: Collection
        //Load the list of catalogs from the collection folder
            LoadCatalogList();
        //LoadCatalogList2();
            PopulateCatalogSelector();

    //TAB: Search files

    //TAB: Create Volume
        //Default path to scan
            //DEV replace by a value from the collection or settings
            ui->LE_NewVolumePath->setText("/");
        //Always Load the file system for the treeview
            LoadFileSystem("/");

    //TAB: Settings
        //Load last collection used
            //Send collection folder to the line edit
            ui->LE_CollectionFolder->setText(collectionFolder);
        //Set file types
            setFileTypes();
            //DEV: interface to edit
            FileTypesEditor();
            //ui->TV_Catalogs->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);
            ui->TV_Catalogs->horizontalHeader()->setSectionResizeMode(QHeaderView::Interactive);

            setupFileContextMenu();

    //TAB: Tests
        LoadCatalogsToModel();


}

MainWindow::~MainWindow()
{
      delete ui;
}

//DEV usefull stuff
/*
KMessageBox::information(this,"test:\n");
qDebug("test of qdebug");
*/









