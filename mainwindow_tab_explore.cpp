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
// File Name:   mainwindow_tab_explore.cpp
// Purpose:     methods for the screen EXPLORE
// Description: https://github.com/StephaneCouturier/Katalog/wiki/Explore
// Author:      Stephane Couturier
/////////////////////////////////////////////////////////////////////////////
*/

#include "mainwindow.h"
#include "ui_mainwindow.h"

#include "catalog.h"
#include "filesview.h"
#include "directorytreemodel.h"
#include "exploretreemodel.h"
#include "exploretreeview.h"

#include <QTextStream>
#include <QDesktopServices>
#include <QFileDialog>
#include <QSortFilterProxyModel>

//UI----------------------------------------------------------------------------

        //----------------------------------------------------------------------
        void MainWindow::on_Explore_treeview_Directories_clicked(const QModelIndex &index)
        {
            //Get selected directory name
            QString fullPath    = ui->Explore_treeview_Directories->model()->index(index.row(), 2, index.parent() ).data().toString();
            selectedDirectoryFullPath = fullPath;
            selectedDirectoryName = fullPath.remove(selectedCatalogPath+"/");

            //Display selected directory name
            ui->Explore_label_CatalogDirectoryDisplay->setText(selectedDirectoryName);

            //Remember selected directory name
            QSettings settings(settingsFilePath, QSettings:: IniFormat);
            settings.setValue("Explore/lastSelectedDirectory", selectedDirectoryName);

            //Load directory files
            loadSelectedDirectoryFilesToExplore();
        }
        //----------------------------------------------------------------------
        void MainWindow::on_Explore_treeView_FileList_clicked(const QModelIndex &index)
        {
            //Get file from selected row
            QString selectedFileName   = ui->Explore_treeView_FileList->model()->index(index.row(), 0, QModelIndex()).data().toString();
            QString selectedFileFolder = ui->Explore_treeView_FileList->model()->index(index.row(), 3, QModelIndex()).data().toString();
            QString selectedFile       = selectedFileFolder+"/"+selectedFileName;
            QString selectedType       = ui->Explore_treeView_FileList->model()->index(index.row(), 5, QModelIndex()).data().toString();

            if(selectedType=="file"){
                //Open the file (fromLocalFile needed for spaces in file name)
                QDesktopServices::openUrl(QUrl::fromLocalFile(selectedFile));
            }
            else{
                //openDirectory
                selectedDirectoryName     = selectedDirectoryFullPath.remove(selectedCatalogPath+"/");
                selectedDirectoryFullPath = selectedFileFolder;

                //Remember selected directory name
                QSettings settings(settingsFilePath, QSettings:: IniFormat);
                settings.setValue("Explore/lastSelectedDirectory", selectedDirectoryName);
                //Reload
                loadSelectedDirectoryFilesToExplore();
            }

        }
        //----------------------------------------------------------------------
        void MainWindow::on_Explore_treeView_FileList_HeaderSortOrderChanged(){

            QSettings settings(settingsFilePath, QSettings:: IniFormat);
            QHeaderView *exploreTreeHeader = ui->Explore_treeView_FileList->header();

            lastExploreSortSection = exploreTreeHeader->sortIndicatorSection();
            lastExploreSortOrder   = exploreTreeHeader->sortIndicatorOrder();

            settings.setValue("Explore/lastExploreSortSection", QString::number(lastExploreSortSection));
            settings.setValue("Explore/lastExploreSortOrder",   QString::number(lastExploreSortOrder));
        }
        //----------------------------------------------------------------------
        void MainWindow::on_Explore_splitter_splitterMoved()
        {
            QSettings settings(settingsFilePath, QSettings:: IniFormat);
            settings.setValue("Explore/ExploreSplitterWidget1Size", ui->Explore_splitter_widget_Directory->size());
            settings.setValue("Explore/ExploreSplitterWidget2Size", ui->Explore_splitter_widget_Files->size());
        }
        //----------------------------------------------------------------------
        void MainWindow::on_Explore_pushButton_Load_clicked()
        {
            if (selectedDeviceType=="Catalog" and selectedDeviceName !=selectedCatalogName){
                QMessageBox::information(this,"Katalog","selectedDeviceName: <br/>" + selectedDeviceName);
                selectedCatalogName = selectedDeviceName;
            }
            openCatalogToExplore();
        }
        //----------------------------------------------------------------------
        void MainWindow::on_Explore_checkBox_DisplayFolders_toggled(bool checked)
        {
            optionDisplayFolders = checked;
            QSettings settings(settingsFilePath, QSettings:: IniFormat);
            settings.setValue("Explore/DisplayFolders", optionDisplayFolders);
            loadSelectedDirectoryFilesToExplore();
        }
        //----------------------------------------------------------------------

    //Context Menu - files
        void MainWindow::on_Explore_treeView_FileList_customContextMenuRequested(const QPoint &pos)
        {
            QPoint globalPos = ui->Explore_treeView_FileList->mapToGlobal(pos);
            QMenu fileContextMenu;

            QAction *menuAction1 = new QAction(QIcon::fromTheme("document-open-data"),(tr("Open file")), this);
            connect(menuAction1, &QAction::triggered, this, &MainWindow::exploreContextOpenFile);
            fileContextMenu.addAction(menuAction1);

            QAction *menuAction2 = new QAction(QIcon::fromTheme("document-open-data"),(tr("Open folder")), this);
            connect(menuAction2, &QAction::triggered, this, &MainWindow::exploreContextOpenFolder);
            fileContextMenu.addAction(menuAction2);

            fileContextMenu.addSeparator();

            QAction *menuAction3 = new QAction(QIcon::fromTheme("edit-copy"),(tr("Copy folder path")), this);
            connect( menuAction3,&QAction::triggered, this, &MainWindow::exploreContextCopyFolderPath);
            fileContextMenu.addAction(menuAction3);

            QAction *menuAction4 = new QAction(QIcon::fromTheme("edit-copy"),(tr("Copy file absolute path")), this);
            connect( menuAction4,&QAction::triggered, this, &MainWindow::exploreContextCopyAbsolutePath);
            fileContextMenu.addAction(menuAction4);

            QAction *menuAction5 = new QAction(QIcon::fromTheme("edit-copy"),(tr("Copy file name with extension")), this);
            connect( menuAction5,&QAction::triggered, this, &MainWindow::exploreContextCopyFileNameWithExtension);
            fileContextMenu.addAction(menuAction5);

            QAction *menuAction6 = new QAction(QIcon::fromTheme("edit-copy"),(tr("Copy file name without extension")), this);
            connect( menuAction6,&QAction::triggered, this, &MainWindow::exploreContextCopyFileNameWithoutExtension);
            fileContextMenu.addAction(menuAction6);

            fileContextMenu.addSeparator();

            //DEV
            if (developmentMode == true){
                QAction *menuAction7 = new QAction(QIcon::fromTheme("document-export"),(tr("Move file to other folder")), this);
                connect( menuAction7,&QAction::triggered, this, &MainWindow::searchContextMoveFileToFolder);
                fileContextMenu.addAction(menuAction7);
            }

            QAction *menuAction8 = new QAction(QIcon::fromTheme("user-trash"),(tr("Move file to Trash")), this);
            connect( menuAction8,&QAction::triggered, this, &MainWindow::exploreContextMoveFileToTrash);
            fileContextMenu.addAction(menuAction8);

            QAction *menuAction9 = new QAction(QIcon::fromTheme("edit-delete"),(tr("Delete file")), this);
            connect( menuAction9,&QAction::triggered, this, &MainWindow::exploreContextDeleteFile);
            fileContextMenu.addAction(menuAction9);

            QAction* selectedItem = fileContextMenu.exec(globalPos);
            if (selectedItem)
            {
                //something
            }
            else
            {
                //QMessageBox::information(this,"Katalog","anyVariable: \n" + QVariant("did nothing").toString());

            }
        }

        void MainWindow::exploreContextOpenFile()
        {
            QModelIndex index=ui->Explore_treeView_FileList->currentIndex();
            //Get filepath from selected row
            QString selectedFileName   = ui->Explore_treeView_FileList->model()->index(index.row(), 0, QModelIndex()).data().toString();
            QString selectedFileFolder = ui->Explore_treeView_FileList->model()->index(index.row(), 3, QModelIndex()).data().toString();
            QString selectedFile = selectedFileFolder+"/"+selectedFileName;
            //Open the file (fromLocalFile needed for spaces in file name)
            QDesktopServices::openUrl(QUrl::fromLocalFile(selectedFile));
        }

        void MainWindow::exploreContextOpenFolder()
        {
            QModelIndex index = ui->Explore_treeView_FileList->currentIndex();
            QString selectedFileName   = ui->Explore_treeView_FileList->model()->index(index.row(), 0, QModelIndex()).data().toString();
            QString selectedFileFolder = ui->Explore_treeView_FileList->model()->index(index.row(), 3, QModelIndex()).data().toString();
            QString selectedFile = selectedFileFolder+"/"+selectedFileName;
            QString folderName = selectedFile.left(selectedFile.lastIndexOf("/"));
            QDesktopServices::openUrl(QUrl::fromLocalFile(folderName));
        }

        void MainWindow::exploreContextCopyAbsolutePath()
        {
            QModelIndex index=ui->Explore_treeView_FileList->currentIndex();

            QString selectedFileName =   ui->Explore_treeView_FileList->model()->index(index.row(), 0, QModelIndex()).data().toString();
            QString selectedFileFolder = ui->Explore_treeView_FileList->model()->index(index.row(), 3, QModelIndex()).data().toString();
            QString selectedFile = selectedFileFolder+"/"+selectedFileName;

            QClipboard *clipboard = QGuiApplication::clipboard();
            QString originalText = clipboard->text();
            clipboard->setText(selectedFile);
        }

        void MainWindow::exploreContextCopyFolderPath()
        {
            QModelIndex index = ui->Explore_treeView_FileList->currentIndex();
            QString selectedFileFolder = ui->Explore_treeView_FileList->model()->index(index.row(), 3, QModelIndex()).data().toString();

            QClipboard *clipboard = QGuiApplication::clipboard();
            QString originalText = clipboard->text();
            clipboard->setText(selectedFileFolder);
        }

        void MainWindow::exploreContextCopyFileNameWithExtension()
        {
            QModelIndex index = ui->Explore_treeView_FileList->currentIndex();
            QString selectedFileName = ui->Explore_treeView_FileList->model()->index(index.row(), 0, QModelIndex()).data().toString();

            QString fileNameWithExtension = selectedFileName;

            QClipboard *clipboard = QGuiApplication::clipboard();
            QString originalText = clipboard->text();
            clipboard->setText(fileNameWithExtension);
        }

        void MainWindow::exploreContextCopyFileNameWithoutExtension()
        {
            QModelIndex index=ui->Explore_treeView_FileList->currentIndex();

            QString selectedFileName   = ui->Explore_treeView_FileList->model()->index(index.row(), 0, QModelIndex()).data().toString();
            QString selectedFileFolder = ui->Explore_treeView_FileList->model()->index(index.row(), 3, QModelIndex()).data().toString();
            QString selectedFile = selectedFileFolder+"/"+selectedFileName;

            QFileInfo fileName;
            fileName.setFile(selectedFile);
            QString fileNameWithoutExtension = fileName.completeBaseName();

            QClipboard *clipboard = QGuiApplication::clipboard();
            QString originalText = clipboard->text();
            clipboard->setText(fileNameWithoutExtension);
        }

        void MainWindow::exploreContextMoveFileToFolder()
        {
            QModelIndex index=ui->Search_treeView_FilesFound->currentIndex();

            QString selectedFileName   = ui->Search_treeView_FilesFound->model()->index(index.row(), 0, QModelIndex()).data().toString();
            QString selectedFileFolder = ui->Search_treeView_FilesFound->model()->index(index.row(), 3, QModelIndex()).data().toString();
            QString selectedFile = selectedFileFolder+"/"+selectedFileName;

            QString pathInTrash;

            if (selectedFileName.isEmpty()) {
                return;
            }

            if (QMessageBox::question(this,
                                      tr("Confirmation"),
                                      tr("Move\n%1\nto the trash?").arg(selectedFile))
                == QMessageBox::Yes) {
                QFile file(selectedFile);
                if (file.exists()) {
                    //Open a dialog for the user to select the target folder
                    QString dir = QFileDialog::getExistingDirectory(this, tr("Select the folder to move this file"),
                                                                    collectionFolder,
                                                                    QFileDialog::ShowDirsOnly
                                                                    | QFileDialog::DontResolveSymlinks);

                    //Unless the selection was cancelled, set the new collection folder, and refresh the list of catalogs
                    if ( dir !=""){
                        //move

                    }

                    //move file
                    QMessageBox::warning(this, tr("Warning"), tr("Moved to folder:<br/>") + dir);

                } else {
                    QMessageBox::warning(this, tr("Warning"), tr("Move to folder failed."));
                }
            }
        }

        void MainWindow::exploreContextMoveFileToTrash()
        {
            QModelIndex index=ui->Explore_treeView_FileList->currentIndex();

            QString selectedFileName   = ui->Explore_treeView_FileList->model()->index(index.row(), 0, QModelIndex()).data().toString();
            QString selectedFileFolder = ui->Explore_treeView_FileList->model()->index(index.row(), 3, QModelIndex()).data().toString();
            QString selectedFile = selectedFileFolder+"/"+selectedFileName;

            QString pathInTrash;

            if (selectedFileName.isEmpty()) {
                return;
            }

            if (QMessageBox::question(this,
                                      tr("Confirmation"),
                                      tr("Move\n%1\nto the trash?").arg(selectedFile))
                == QMessageBox::Yes) {
                if (QFile::moveToTrash(selectedFile, &pathInTrash)) {
                    QMessageBox::warning(this, tr("Warning"), tr("Moved to trash:<br/>") + pathInTrash);

                } else {
                    QMessageBox::warning(this, tr("Warning"), tr("Move to trash failed."));
                }
            }
        }

        void MainWindow::exploreContextDeleteFile()
        {
            QModelIndex index=ui->Explore_treeView_FileList->currentIndex();

            QString selectedFileName   = ui->Explore_treeView_FileList->model()->index(index.row(), 0, QModelIndex()).data().toString();
            QString selectedFileFolder = ui->Explore_treeView_FileList->model()->index(index.row(), 3, QModelIndex()).data().toString();
            QString selectedFile = selectedFileFolder+"/"+selectedFileName;

            if (selectedFileName.isEmpty()) {
                return;
            }

            if (QMessageBox::question(this,
                                      tr("Confirmation"),
                                      tr("<span style='color:red;'>DELETE</span><br/> %1 <br/>?").arg(selectedFile))
                == QMessageBox::Yes) {

                QFile file(selectedFile);
                if (file.exists()) {

                    file.remove();

                    QMessageBox::warning(this, tr("Warning"), tr("Deleted.") );

                } else {
                    QMessageBox::warning(this, tr("Warning"), tr("Failed to delete."));
                }
            }
        }

    //Context Menu - directories
        void MainWindow::on_Explore_treeview_Directories_customContextMenuRequested(const QPoint &pos)
        {
            QPoint globalPos = ui->Explore_treeview_Directories->mapToGlobal(pos);
            QMenu directoryContextMenu;

            QAction *menuDirectoryAction1 = new QAction(QIcon::fromTheme("tag"),(tr("Tag this folder")), this);
            connect(menuDirectoryAction1, &QAction::triggered, this, &MainWindow::exploreContextTagFolder);
            directoryContextMenu.addAction(menuDirectoryAction1);

            QAction* selectedItem = directoryContextMenu.exec(globalPos);

            if (selectedItem)
            {
                //something
                //QMessageBox::information(this,"Katalog","anyVariable: \n" + QVariant("ok").toString());

            }
            else
            {
                //QMessageBox::information(this,"Katalog","anyVariable: \n" + QVariant("did nothing").toString());
            }
        }

        void MainWindow::exploreContextTagFolder()
        {
            QModelIndex index = ui->Explore_treeview_Directories->currentIndex();

            QString selectedFolder = ui->Explore_treeview_Directories->model()->index(index.row(), 2, QModelIndex()).data().toString();

            ui->Tags_lineEdit_FolderPath->setText(selectedFolder);
            ui->tabWidget->setCurrentIndex(6);
        }

//Methods-----------------------------------------------------------------------

    void MainWindow::openCatalogToExplore()
    {
        //Check catalog's number of files and confirm load if too big
        QSqlQuery query;
        QString querySQL = QLatin1String(R"(
                            SELECT catalogFileCount
                            FROM catalog
                            WHERE catalogName=:catalogName
                                        )");
        query.prepare(querySQL);
        query.bindValue(":catalogName",selectedCatalogName);
        query.exec();
        query.next();
        int selectedcatalogFileCount = query.value(0).toInt();
        if (selectedcatalogFileCount > 500000){
                int result = QMessageBox::warning(this,"Katalog",
                          tr("The selected catalog contains more than 500.000 files.<br/>"
                             "This could take one or several minutes to open.<br/>"
                             "Do you want to continue?"),QMessageBox::Yes|QMessageBox::Cancel);
                if ( result ==QMessageBox::Cancel){
                    return;
                }
        }

        //selectedCatalogPath: remove the / at the end if any
        int pathLength = selectedCatalogPath.length();
        if (selectedCatalogPath.at(pathLength-1)=="/") {
            selectedCatalogPath.remove(pathLength-1,1);
        }

        //Load the files of the Selected Catalog
        //loadCatalogFilesToExplore();
        loadCatalogFilelistToTable(selectedCatalogName);

        loadCatalogDirectoriesToExplore();
        loadSelectedDirectoryFilesToExplore();

        //Go to the Explorer tab
        ui->Explore_label_CatalogNameDisplay->setText(selectedCatalogName);
        ui->Explore_label_CatalogPathDisplay->setText(selectedCatalogPath);

        //Remember last opened catalog
        QSettings settings(settingsFilePath, QSettings:: IniFormat);
        settings.setValue("Explore/lastSelectedCatalogFile", selectedCatalogFile);
        settings.setValue("Explore/lastSelectedCatalogName", selectedCatalogName);
        settings.setValue("Explore/lastSelectedCatalogPath", selectedCatalogPath);
        settings.setValue("Explore/lastSelectedDirectory", selectedDirectoryName);
    }
    //----------------------------------------------------------------------
    void MainWindow::loadCatalogDirectoriesToExplore()
    {
        //Load the catalog's directories and display them

        //prepare query to load file info
            QSqlQuery getDirectoriesQuery;

            //shorten the paths as they all start with the catalog path
            QString getDirectoriesSQL = QLatin1String(R"(
                                            SELECT DISTINCT (REPLACE(filePath, :selectedCatalogPath||'/', ''))
                                            FROM filesall
                                            WHERE   fileCatalog =:fileCatalog
                                            ORDER BY filePath ASC
                                        )");
            getDirectoriesQuery.prepare(getDirectoriesSQL);
            getDirectoriesQuery.bindValue(":fileCatalog",selectedCatalogName);
            getDirectoriesQuery.bindValue(":selectedCatalogPath",selectedCatalogPath);
            getDirectoriesQuery.exec();

        //Prepare model
            ExploreTreeModel *exploreTreeModel = new ExploreTreeModel();
            exploreTreeModel->setModelCatalog(selectedCatalogName,selectedCatalogPath);

            ExploreTreeView *exploreProxyModel = new ExploreTreeView();
            exploreProxyModel->setSourceModel(exploreTreeModel);
            //ui->Explore_treeview_Directories->QTreeView::sortByColumn(lastExploreSortSection,Qt::SortOrder(lastExploreSortOrder));
            ui->Explore_treeview_Directories->QTreeView::sortByColumn(5,Qt::SortOrder(0));
            ui->Explore_treeview_Directories->header()->setSectionResizeMode(QHeaderView::Interactive);
            //ui->Explore_treeview_Directories->header()->resizeSection(0, 600); //Directory

        // Connect model to treeview
            ui->Explore_treeview_Directories->setModel(exploreProxyModel);
            ui->Explore_treeview_Directories->header()->resizeSection(0,  300);
            ui->Explore_treeview_Directories->hideColumn(1);//NoOfItems
            ui->Explore_treeview_Directories->hideColumn(2);//FullPath

        //Display number of directories and total size
            QString countSQL = QLatin1String(R"(
                                SELECT count (DISTINCT (filePath))
                                FROM filesall
                                WHERE fileCatalog =:fileCatalog
                               )");
            QSqlQuery countQuery;
            countQuery.prepare(countSQL);
            countQuery.bindValue(":fileCatalog",selectedCatalogName);
            countQuery.exec();
            countQuery.next();
            ui->Explore_label_DirectoryNumberDisplay->setText(QLocale().toString(countQuery.value(0).toLongLong()));

            //DEV DirectoryTreeModel --------------------------------
/*
        if(developmentMode==true){

            QString modelToTest = "Storage"; //

            if (modelToTest =="Storage"){

            }
            else if (modelToTest =="Storage") {
                const QStringList headers({tr("Title"),tr("Type")});
                DirectoryTreeModel *directorytreeModel = new DirectoryTreeModel(headers);
                directorytreeModel->setModelCatlog("Maxtor_2Tb","/run/media/stephane/Maxtor_2Tb");

                ui->Explore_treeview_Directories->setModel(directorytreeModel);
                ui->Explore_treeview_Directories->header()->resizeSection(0,  300);
                ui->Explore_treeview_Directories->expandAll();
            }
        }
*/
    }
    //----------------------------------------------------------------------
    void MainWindow::loadSelectedDirectoryFilesToExplore()
    {
        //Load the files of the selected directory into the file view

        // Load all files and create model
        QString selectSQL;

        if(optionDisplayFolders==true){

            selectSQL = QLatin1String(R"(
                                    SELECT  		REPLACE(filePath, :selectedDirectoryFullPath||'/', ''),
                                                    SUM(fileSize),
                                                    "",
                                                    filePath,
                                                    "",
                                                    "folder" AS Type,
                                                    "1"||filePath AS orderValue
                                    FROM    filesall
                                    WHERE   fileCatalog =:fileCatalog
                                    AND     filePath  like :folderPath

                                    GROUP BY filePath

                                    UNION

                                    SELECT  		fileName,
                                                    fileSize,
                                                    fileDateUpdated,
                                                    filePath,
                                                    fileCatalog,
                                                    "file"  AS Type,
                                                    "2"||fileName AS orderValue
                                    FROM    filesall
                                    WHERE   fileCatalog =:fileCatalog
                                    AND     filePath    =:filePath

                                    ORDER BY orderValue ASC

                                )");
            // AND     REPLACE(filePath, :selectedDirectoryFullPath||'/', ''  NOT like "/"
        }
        else{
            selectSQL = QLatin1String(R"(
                                    SELECT  		fileName,
                                                    fileSize,
                                                    fileDateUpdated,
                                                    filePath,
                                                    fileCatalog,
                                                    "file"  AS Type,
                                                    "2"||fileName AS orderValue
                                    FROM    filesall
                                    WHERE   fileCatalog =:fileCatalog
                                    AND     filePath    =:filePath

                                    ORDER BY orderValue ASC
                                )");
        }


        if( selectedCatalogPath == "EXPORT" ){
            selectedDirectoryFullPath.remove("EXPORT");
        }

        //  if selectedDirectoryName="" then no file is loaded

        QSqlQuery loadCatalogQuery;
        loadCatalogQuery.prepare(selectSQL);

        // fill lists depending on directory selection source
        loadCatalogQuery.bindValue(":fileCatalog",selectedCatalogName);

        if(selectedCatalogPath==selectedDirectoryName){
            loadCatalogQuery.bindValue(":filePath",selectedDirectoryName);
        }
        else{
            loadCatalogQuery.bindValue(":filePath",selectedDirectoryFullPath);
        }

        loadCatalogQuery.bindValue(":folderPath",selectedDirectoryFullPath+"/%");
        loadCatalogQuery.bindValue(":selectedDirectoryFullPath",selectedDirectoryFullPath);

        loadCatalogQuery.exec();

        QSqlQueryModel *loadCatalogQueryModel = new QSqlQueryModel;
        loadCatalogQueryModel->setQuery(loadCatalogQuery);

        FilesView *proxyModel2 = new FilesView(this);
        proxyModel2->setSourceModel(loadCatalogQueryModel);

        proxyModel2->setHeaderData(0, Qt::Horizontal, tr("Name"));
        proxyModel2->setHeaderData(1, Qt::Horizontal, tr("Size"));
        proxyModel2->setHeaderData(2, Qt::Horizontal, tr("Date"));
        proxyModel2->setHeaderData(3, Qt::Horizontal, tr("Path"));
        proxyModel2->setHeaderData(4, Qt::Horizontal, tr("Catalog"));
        proxyModel2->setHeaderData(5, Qt::Horizontal, tr("Type"));

        // Connect model to tree/table view
        ui->Explore_treeView_FileList->setModel(proxyModel2);
        ui->Explore_treeView_FileList->QTreeView::sortByColumn(lastExploreSortSection,Qt::SortOrder(lastExploreSortOrder));
        ui->Explore_treeView_FileList->header()->setSectionResizeMode(QHeaderView::Interactive);
        ui->Explore_treeView_FileList->header()->resizeSection(0, 600); //Name
        ui->Explore_treeView_FileList->header()->resizeSection(1, 110); //Size
        ui->Explore_treeView_FileList->header()->resizeSection(2, 140); //Date
        ui->Explore_treeView_FileList->header()->resizeSection(3, 400); //Path
        ui->Explore_treeView_FileList->hideColumn(4);
        ui->Explore_treeView_FileList->hideColumn(5);

        //Display count of files and total size
        QString countSQL = QLatin1String(R"(
                            SELECT  count (*), sum(fileSize)
                            FROM    filesall
                            WHERE   fileCatalog =:fileCatalog
                                        )");

        if (selectedDirectoryName!=""){
            countSQL = countSQL + " AND filePath =:filePath";
        }

        QSqlQuery countQuery;
        countQuery.prepare(countSQL);
        countQuery.bindValue(":fileCatalog",selectedCatalogName);

        if(selectedCatalogPath==selectedDirectoryName){
            countQuery.bindValue(":filePath",selectedDirectoryName);
        }
        else{
            countQuery.bindValue(":filePath",selectedDirectoryFullPath);
        }

        countQuery.exec();
        countQuery.next();

        ui->Explore_label_FilesNumberDisplay->setText(QLocale().toString(countQuery.value(0).toLongLong()));
        ui->Explore_label_TotalSizeLabelDisplay->setText(QLocale().formattedDataSize(countQuery.value(1).toLongLong()));
        ui->Explore_label_CatalogDirectoryDisplay->setText(selectedDirectoryName);
    }
