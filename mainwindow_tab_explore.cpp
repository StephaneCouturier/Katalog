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

//Methods-----------------------------------------------------------------------

    void MainWindow::openCatalogToExplore()
    {
        //Check catalog's number of files and confirm load
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

        //Load the files of the Selected Catalog
        loadCatalogFilesToExplore();
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
    void MainWindow::loadCatalogFilesToExplore()
    {
        //Load all files from a catalog into database

        // Start animation while opening
        QApplication::setOverrideCursor(Qt::WaitCursor);

        // Get infos stored in the file
        QFile catalogFile(selectedCatalogFile);
        if(!catalogFile.open(QIODevice::ReadOnly)) {
            QMessageBox::information(this,"Katalog",tr("No catalog found."));
            return;
        }

        QFileInfo catalogFileInfo(selectedCatalogFile);

        QTextStream textStream(&catalogFile);

        //clear database
        QSqlQuery deleteQuery0;
        deleteQuery0.exec("DELETE FROM file");

        //prepare query to load file info
        QSqlQuery insertQuery;
        QString insertSQL = QLatin1String(R"(
                            INSERT INTO file (
                                            fileName,
                                            filePath,
                                            fileSize,
                                            fileDateUpdated,
                                            fileCatalog )
                            VALUES(
                                            :fileName,
                                            :filePath,
                                            :fileSize,
                                            :fileDateUpdated,
                                            :fileCatalog )
                                        )");
        insertQuery.prepare(insertSQL);

        //set temporary values
            QString     line;
            QStringList fieldList;
            int         fieldListCount;
            QString     filePath;
            qint64      fileSize;
            QString     fileDateTime;


            int count=0;

        //load each file
        while (true)
        {
            line = textStream.readLine();

            count++;

            if (line.isNull())
                break;
            else
                if (line.left(1)!="<"){

                    //Split the string with \t (tabulation) into a list
                    QRegExp tagExp("\t"); //setpattern
                    fieldList.clear();
                    fieldList = line.split(tagExp);

                    fieldListCount = fieldList.count();

                    // Get the filePath from the list:
                    filePath = fieldList[0];

                    // Get the fileSize from the list if available
                    if (fieldListCount == 3){
                            fileSize = fieldList[1].toLongLong();}
                    else fileSize = 0;

                    // Get the fileDateTime from the list if available
                    if (fieldListCount == 3){
                            fileDateTime = fieldList[2];}
                    else fileDateTime = "";

                    //Get file informations
                    QFileInfo file(filePath);

                    //Append data to the database
                    insertQuery.bindValue(":fileName", file.fileName());
                    insertQuery.bindValue(":filePath", file.path());
                    insertQuery.bindValue(":fileSize", fileSize);
                    insertQuery.bindValue(":fileDateUpdated", fileDateTime);
                    insertQuery.bindValue(":fileCatalog", catalogFileInfo.baseName());
                    insertQuery.exec();

                }

        }

        catalogFile.close();

        //Stop animation
        QApplication::restoreOverrideCursor();
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
                                    FROM file
                                    ORDER BY filePath ASC
             )");

            getDirectoriesQuery.prepare(getDirectoriesSQL);
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
                            FROM file
                                        )");
        QSqlQuery countQuery;
        countQuery.prepare(countSQL);
        countQuery.exec();
        countQuery.next();
        ui->Explore_label_DirectoryNumberDisplay->setText(QLocale().toString(countQuery.value(0).toLongLong()));

        //TreeView TESTS---------------

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
                            SELECT  fileName AS Name,
                                    fileSize AS Size,
                                    fileDateUpdated AS Date,
                                    filePath AS Path,
                                    fileCatalog AS Catalog
                            FROM file
                            WHERE filePath =:filePath
                                        )");

        //  if selectedDirectoryName="" then no file is loaded

        QSqlQuery loadCatalogQuery;
        loadCatalogQuery.prepare(selectSQL);

        // fill lists depending on directory selection source
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
                            FROM file
                                        )");

        if (selectedDirectoryName!=""){
            countSQL = countSQL + " WHERE filePath =:filePath";
        }

        QSqlQuery countQuery;
        countQuery.prepare(countSQL);
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
