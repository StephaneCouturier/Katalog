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


#include <QTextStream>
#include <QDesktopServices>
#include <QFileDialog>
#include <QSortFilterProxyModel>

//UI----------------------------------------------------------------------------

        void MainWindow::on_Explore_treeView_FileList_clicked(const QModelIndex &index)
        {
            //Get file from selected row
            QString selectedFileName   = ui->Explore_treeView_FileList->model()->index(index.row(), 0, QModelIndex()).data().toString();
            QString selectedFileFolder = ui->Explore_treeView_FileList->model()->index(index.row(), 3, QModelIndex()).data().toString();
            QString selectedFile = selectedFileFolder+"/"+selectedFileName;

            //Open the file (fromLocalFile needed for spaces in file name)
            QDesktopServices::openUrl(QUrl::fromLocalFile(selectedFile));           
        }
        //----------------------------------------------------------------------
        void MainWindow::on_Explore_treeview_Directories_clicked(const QModelIndex &index)
        {
            //Get selected directory name
            selectedDirectoryName = ui->Explore_treeview_Directories->model()->index(index.row(), 0, QModelIndex()).data().toString();

            //Display selected directory name
            ui->Explore_label_CatalogDirectoryDisplay->setText(selectedDirectoryName);

            //Remember selected directory name
            QSettings settings(settingsFilePath, QSettings:: IniFormat);
            settings.setValue("Explore/lastSelectedDirectory", selectedDirectoryName);

            tempSelectedTreeviewSource = "flatlist";

            //Load directory files
            loadSelectedDirectoryFilesToExplore();
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
    //Context Menu methods
        void MainWindow::on_Explore_treeView_FileList_customContextMenuRequested(const QPoint &pos)
        {
            // for most widgets
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

            QAction *menuAction7 = new QAction(QIcon::fromTheme("user-trash"),(tr("Move file to Trash")), this);
            connect( menuAction7,&QAction::triggered, this, &MainWindow::exploreContextMoveToTrash);
            fileContextMenu.addAction(menuAction7);

            QAction *menuAction8 = new QAction(QIcon::fromTheme("delete"),(tr("Delete file")), this);
            connect( menuAction8,&QAction::triggered, this, &MainWindow::exploreContextDeleteFile);
            fileContextMenu.addAction(menuAction8);

            QAction* selectedItem = fileContextMenu.exec(globalPos);
            if (selectedItem)
            {
                //something
            }
            else
            {
                //KMessageBox::information(this,"test:\n did nothing.");
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

        void MainWindow::exploreContextMoveToTrash()
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
                                      tr("Are you sure you want to move\n%1\nto the trash?").arg(selectedFile))
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
                                      tr("Are you sure you want to <span style='color:red;'>DELETE</span><br/> %1 <br/>?").arg(selectedFile))
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
        //----------------------------------------------------------------------

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
        //ui->tabWidget->setCurrentIndex(2);

        //Remember last opened catalog
        QSettings settings(settingsFilePath, QSettings:: IniFormat);
        settings.setValue("Explore/lastSelectedCatalogFile", selectedCatalogFile);
        settings.setValue("Explore/lastSelectedCatalogName", selectedCatalogName);
        settings.setValue("Explore/lastSelectedCatalogPath", selectedCatalogPath);
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
        QSqlQueryModel *getDirectoriesQueryModel = new QSqlQueryModel;
        getDirectoriesQueryModel->setQuery(getDirectoriesQuery);
        QSortFilterProxyModel *getDirectoriesProxyModel = new QSortFilterProxyModel;
        getDirectoriesProxyModel->setSourceModel(getDirectoriesQueryModel);
        getDirectoriesProxyModel->setHeaderData(0, Qt::Horizontal, tr("Directory"));

        // Connect model to tree/table view
        ui->Explore_treeview_Directories->setModel(getDirectoriesProxyModel);
        ui->Explore_treeview_Directories->QTreeView::sortByColumn(lastExploreSortSection,Qt::SortOrder(lastExploreSortOrder));
        ui->Explore_treeview_Directories->header()->setSectionResizeMode(QHeaderView::Interactive);
        ui->Explore_treeview_Directories->header()->resizeSection(0, 600); //Directory

        //Display number of directories
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

        //TreeView DEV TESTS---------------

        //    DirectoryTreeModel *directorytreeModel = new DirectoryTreeModel();
        //    //directorytreeModel->setSelectedCatalogPath(selectedCatalogPath);
        //    ui->DEV_treeView_Directories->setModel(directorytreeModel);
        //    ui->DEV_treeView_Directories->header()->resizeSection(0,  300);
        //    ui->DEV_treeView_Directories->expandAll();

    }
    //----------------------------------------------------------------------
    void MainWindow::loadSelectedDirectoryFilesToExplore()
    {
        //Load the files of the selected directory into the file view

        // Load all files and create model
        QString selectSQL = QLatin1String(R"(
                            SELECT  fileName,
                                    fileSize,
                                    fileDateUpdated,
                                    filePath,
                                    fileCatalog
                            FROM    filesall
                            WHERE   fileCatalog =:fileCatalog
                            AND     filePath    =:filePath
                                        )");

        //  if selectedDirectoryName="" then no file is loaded

        QSqlQuery loadCatalogQuery;
        loadCatalogQuery.prepare(selectSQL);

        // fill lists depending on directory selection source
        loadCatalogQuery.bindValue(":fileCatalog",selectedCatalogName);
        if(selectedCatalogPath==selectedDirectoryName){
            loadCatalogQuery.bindValue(":filePath",selectedDirectoryName);
        }
        else{
            loadCatalogQuery.bindValue(":filePath",selectedCatalogPath+'/'+selectedDirectoryName);
        }

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

        // Connect model to tree/table view
        ui->Explore_treeView_FileList->setModel(proxyModel2);
        ui->Explore_treeView_FileList->QTreeView::sortByColumn(lastExploreSortSection,Qt::SortOrder(lastExploreSortOrder));
        ui->Explore_treeView_FileList->header()->setSectionResizeMode(QHeaderView::Interactive);
        ui->Explore_treeView_FileList->header()->resizeSection(0, 600); //Name
        ui->Explore_treeView_FileList->header()->resizeSection(1, 110); //Size
        ui->Explore_treeView_FileList->header()->resizeSection(2, 140); //Date
        ui->Explore_treeView_FileList->header()->resizeSection(3, 400); //Path

        ui->DEV_treeView_Files->setModel(proxyModel2);
        //ui->DEV_treeView_Files->QTreeView::sortByColumn(0,Qt::AscendingOrder);
        ui->DEV_treeView_Files->header()->setSectionResizeMode(QHeaderView::Interactive);
        ui->DEV_treeView_Files->header()->resizeSection(0, 600); //Name
        ui->DEV_treeView_Files->header()->resizeSection(1, 110); //Size
        ui->DEV_treeView_Files->header()->resizeSection(2, 140); //Date
        ui->DEV_treeView_Files->header()->resizeSection(3, 400); //Path

        QString countSQL = QLatin1String(R"(
                            SELECT  count (*)
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
        else
            countQuery.bindValue(":filePath",selectedCatalogPath+'/'+selectedDirectoryName);

        countQuery.exec();
        countQuery.next();

        ui->Explore_label_FilesNumberDisplay->setText(QLocale().toString(countQuery.value(0).toLongLong()));
    }

    //----------------------------------------------------------------------
    //DEV

    void MainWindow::on_DEV_treeView_Directories_activated(const QModelIndex &index)
    {
        selectedDirectoryName = ui->DEV_treeView_Directories->model()->index(index.row(), 1, QModelIndex()).data().toString();

    //    for (int i=0;i<3 ;i++ ) {
    //        QString temp = ui->DEV_treeView_Directories->model()->data(index().toString();
    //        QList<QVariant> temp = ui->DEV_treeView_Directories->model()->index(index.row(), i, QModelIndex()).data().toList();
    //
    //        QString temp = ui->DEV_treeView_Directories->model()->data(QModelIndex()).toString();
    //        QString temp = ui->DEV_treeView_Directories->model()->index(index.row(), i, QModelIndex()).data().toString();
            //QMessageBox::information(this,"Katalog","col:" + QString::number(i) + "<br/>value:" + temp[i].toString());
    //    }

        QMessageBox::information(this,"Katalog","selectedDirectoryName:" + selectedDirectoryName);
        tempSelectedTreeviewSource = "treelist";

        loadSelectedDirectoryFilesToExplore();
    }
    //----------------------------------------------------------------------

