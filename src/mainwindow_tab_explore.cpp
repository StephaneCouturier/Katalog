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
// Description: https://stephanecouturier.github.io/Katalog/docs/Features/Explore
// Author:      Stephane Couturier
/////////////////////////////////////////////////////////////////////////////
*/

#include "mainwindow.h"
#include "ui_mainwindow.h"

#include "catalog.h"
#include "filesview.h"
#include "exploretreemodel.h"
#include "exploretreeview.h"

//UI----------------------------------------------------------------------------
    void MainWindow::on_Explore_treeview_Directories_clicked(const QModelIndex &index)
    {
        //Get selected directory name
        QString fullPath = ui->Explore_treeview_Directories->model()->index(index.row(), 2, index.parent() ).data().toString();
        exploreSelectedFolderFullPath = fullPath;
        exploreSelectedDirectoryName  = fullPath.remove(exploreDevice->path + "/");

        //Display selected directory name
        ui->Explore_label_CatalogDirectoryDisplay->setText(exploreSelectedDirectoryName);

        //Remember selected directory name
        QSettings settings(collection->settingsFilePath, QSettings:: IniFormat);
        settings.setValue("Explore/lastExploreSelectedFolderFullPath", exploreSelectedFolderFullPath);
        settings.setValue("Explore/lastExploreSelectedDirectoryName", exploreSelectedDirectoryName);

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
            exploreSelectedFolderFullPath = selectedFileFolder;
            exploreSelectedDirectoryName = selectedFileFolder.remove(exploreDevice->path + "/");

            //Remember selected directory name
            QSettings settings(collection->settingsFilePath, QSettings:: IniFormat);
            settings.setValue("Explore/lastExploreSelectedFolderFullPath", exploreSelectedFolderFullPath);
            settings.setValue("Explore/lastExploreSelectedDirectoryName", exploreSelectedDirectoryName);

            //Reload
            loadSelectedDirectoryFilesToExplore();
        }
    }
    //----------------------------------------------------------------------
    void MainWindow::on_ExploreTreeViewFileListHeaderSortOrderChanged(){

        QSettings settings(collection->settingsFilePath, QSettings:: IniFormat);
        QHeaderView *exploreTreeHeader = ui->Explore_treeView_FileList->header();

        lastExploreSortSection = exploreTreeHeader->sortIndicatorSection();
        lastExploreSortOrder   = exploreTreeHeader->sortIndicatorOrder();

        settings.setValue("Explore/lastExploreSortSection", QString::number(lastExploreSortSection));
        settings.setValue("Explore/lastExploreSortOrder",   QString::number(lastExploreSortOrder));
    }
    //----------------------------------------------------------------------
    void MainWindow::on_Explore_splitter_splitterMoved()
    {
        QSettings settings(collection->settingsFilePath, QSettings:: IniFormat);
        settings.setValue("Explore/ExploreSplitterWidget1Size", ui->Explore_splitter_widget_Directory->size());
        settings.setValue("Explore/ExploreSplitterWidget2Size", ui->Explore_splitter_widget_Files->size());
    }
    //----------------------------------------------------------------------
    void MainWindow::on_ExplorePushButtonLoadClicked()
    {
        openCatalogToExplore();
    }
    //----------------------------------------------------------------------
    void MainWindow::on_Explore_checkBox_DisplayFolders_toggled(bool checked)
    {
        optionDisplayFolders = checked;
        QSettings settings(collection->settingsFilePath, QSettings:: IniFormat);
        settings.setValue("Explore/DisplayFolders", optionDisplayFolders);
        loadSelectedDirectoryFilesToExplore();
        if(checked==true){
            ui->Explore_checkBox_DisplaySubFolders->setEnabled(true);
            ui->Explore_pushButton_OrderFoldersFirst->setEnabled(true);
        }
        else {
            ui->Explore_checkBox_DisplaySubFolders->setEnabled(false);
            ui->Explore_pushButton_OrderFoldersFirst->setEnabled(false);
        }
    }
    //----------------------------------------------------------------------
    void MainWindow::on_Explore_checkBox_DisplaySubFolders_toggled(bool checked)
    {
        optionDisplaySubFolders = checked;
        QSettings settings(collection->settingsFilePath, QSettings:: IniFormat);
        settings.setValue("Explore/DisplaySubFolders", optionDisplaySubFolders);
        loadSelectedDirectoryFilesToExplore();
    }
    //----------------------------------------------------------------------
    void MainWindow::on_Explore_pushButton_OrderFoldersFirst_clicked()
    {
        ui->Explore_treeView_FileList->QTreeView::sortByColumn(6,Qt::AscendingOrder);
    }
    //----------------------------------------------------------------------

//Context Menu - directories
    void MainWindow::on_Explore_treeview_Directories_customContextMenuRequested(const QPoint &pos)
    {
        QPoint globalPos = ui->Explore_treeview_Directories->mapToGlobal(pos);
        QMenu directoryContextMenu;

        QAction *menuDirectoryAction1 = new QAction(QIcon::fromTheme("tag"),(tr("Tag this folder")), this);
        connect(menuDirectoryAction1, &QAction::triggered, this, &MainWindow::exploreContextTagFolder);
        directoryContextMenu.addAction(menuDirectoryAction1);

        directoryContextMenu.exec(globalPos);
    }
    //--------------------------------------------------------------------------
    void MainWindow::exploreContextTagFolder()
    {
        QModelIndex index = ui->Explore_treeview_Directories->currentIndex();

        QString selectedFolder = ui->Explore_treeview_Directories->model()->index(index.row(), 2, QModelIndex()).data().toString();

        ui->Tags_lineEdit_FolderPath->setText(selectedFolder);
        ui->tabWidget->setCurrentIndex(6);
    }
    //--------------------------------------------------------------------------

//Context Menu - files
    void MainWindow::on_Explore_treeView_FileList_customContextMenuRequested(const QPoint &pos)
    {
        //Get tiem type (file or folder)
        QPoint globalPos = ui->Explore_treeView_FileList->mapToGlobal(pos);
        QModelIndex tempindex = ui->Explore_treeView_FileList->indexAt(pos);
        QString selectedType = ui->Explore_treeView_FileList->model()->index(tempindex.row(), 5, QModelIndex()).data().toString();

        QMenu fileContextMenu;

        if (selectedType == "file")
        {
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
        }
        else if (selectedType == "folder")
        {
            QAction *menuAction1 = new QAction(QIcon::fromTheme("document-open-data"),(tr("Open folder")), this);
            connect(menuAction1, &QAction::triggered, this, &MainWindow::exploreContextOpenFolder);
            fileContextMenu.addAction(menuAction1);

            fileContextMenu.addSeparator();

            QAction *menuAction3 = new QAction(QIcon::fromTheme("edit-copy"),(tr("Copy folder path")), this);
            connect( menuAction3,&QAction::triggered, this, &MainWindow::exploreContextCopyFolderPath);
            fileContextMenu.addAction(menuAction3);

            QAction *menuAction5 = new QAction(QIcon::fromTheme("edit-copy"),(tr("Copy folder name")), this);
            connect( menuAction5,&QAction::triggered, this, &MainWindow::exploreContextCopyFileNameWithExtension);
            fileContextMenu.addAction(menuAction5);

            fileContextMenu.addSeparator();

            //DEV
            if (developmentMode == true){
                QAction *menuAction7 = new QAction(QIcon::fromTheme("document-export"),(tr("Move file to other folder")), this);
                connect( menuAction7,&QAction::triggered, this, &MainWindow::searchContextMoveFileToFolder);
                fileContextMenu.addAction(menuAction7);

                QAction *menuAction9 = new QAction(QIcon::fromTheme("edit-delete"),(tr("Delete folder")), this);
                connect( menuAction9,&QAction::triggered, this, &MainWindow::exploreContextDeleteFile);
                fileContextMenu.addAction(menuAction9);
            }

            QAction *menuAction8 = new QAction(QIcon::fromTheme("user-trash"),(tr("Move folder to Trash")), this);
            connect( menuAction8,&QAction::triggered, this, &MainWindow::exploreContextMoveFileToTrash);
            fileContextMenu.addAction(menuAction8);
        }

        fileContextMenu.exec(globalPos);

    }
    //--------------------------------------------------------------------------
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
    //--------------------------------------------------------------------------
    void MainWindow::exploreContextOpenFolder()
    {
        QModelIndex index = ui->Explore_treeView_FileList->currentIndex();
        QString selectedFileName   = ui->Explore_treeView_FileList->model()->index(index.row(), 0, QModelIndex()).data().toString();
        QString selectedFileFolder = ui->Explore_treeView_FileList->model()->index(index.row(), 3, QModelIndex()).data().toString();
        QString selectedFile = selectedFileFolder+"/"+selectedFileName;
        QString folderName = selectedFile.left(selectedFile.lastIndexOf("/"));
        QDesktopServices::openUrl(QUrl::fromLocalFile(folderName));
    }
    //--------------------------------------------------------------------------
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
    //--------------------------------------------------------------------------
    void MainWindow::exploreContextCopyFolderPath()
    {
        QModelIndex index = ui->Explore_treeView_FileList->currentIndex();
        QString selectedFileFolder = ui->Explore_treeView_FileList->model()->index(index.row(), 3, QModelIndex()).data().toString();

        QClipboard *clipboard = QGuiApplication::clipboard();
        QString originalText = clipboard->text();
        clipboard->setText(selectedFileFolder);
    }
    //--------------------------------------------------------------------------
    void MainWindow::exploreContextCopyFileNameWithExtension()
    {
        QModelIndex index = ui->Explore_treeView_FileList->currentIndex();
        QString selectedFileName = ui->Explore_treeView_FileList->model()->index(index.row(), 0, QModelIndex()).data().toString();

        QString fileNameWithExtension = selectedFileName;

        QClipboard *clipboard = QGuiApplication::clipboard();
        QString originalText = clipboard->text();
        clipboard->setText(fileNameWithExtension);
    }
    //--------------------------------------------------------------------------
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
    //--------------------------------------------------------------------------
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
                                                                collection->folder,
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
    //--------------------------------------------------------------------------
    void MainWindow::exploreContextMoveFileToTrash()
    {
        //Get selected data
        QModelIndex index=ui->Explore_treeView_FileList->currentIndex();
        QString selectedFileDirectory = ui->Explore_treeView_FileList->model()->index(index.row(), 3, QModelIndex()).data().toString();
        QString selectedType          = ui->Explore_treeView_FileList->model()->index(index.row(), 5, QModelIndex()).data().toString();
        QString selectedFilePath      = ui->Explore_treeView_FileList->model()->index(index.row(), 7, QModelIndex()).data().toString();

        //Define what to move to trash
        QString pathInTrash;
        QString pathToMove;
        if (selectedType=="file")
            pathToMove= selectedFilePath;
        else pathToMove = selectedFileDirectory;

        if (selectedFilePath.isEmpty()) {
            return;
        }

        //confirm and move
        if (QMessageBox::question(this,
                                  tr("Confirmation"),
                                  tr("Move\n%1\nto the trash?").arg(pathToMove))
            == QMessageBox::Yes) {
            if (QFile::moveToTrash(pathToMove, &pathInTrash)) {
                QMessageBox::warning(this, tr("Warning"), tr("Moved to trash:<br/>") + pathInTrash);

            } else {
                QMessageBox::warning(this, tr("Warning"), tr("Move to trash failed."));
            }
        }
    }
    //--------------------------------------------------------------------------
    void MainWindow::exploreContextDeleteFile()
    {
        //Get selected data
        QModelIndex index=ui->Explore_treeView_FileList->currentIndex();
        QString selectedFileDirectory = ui->Explore_treeView_FileList->model()->index(index.row(), 3, QModelIndex()).data().toString();
        QString selectedType          = ui->Explore_treeView_FileList->model()->index(index.row(), 5, QModelIndex()).data().toString();
        QString selectedFilePath      = ui->Explore_treeView_FileList->model()->index(index.row(), 7, QModelIndex()).data().toString();

        //Define what to move to trash
        QString pathInTrash;
        QString pathToDelete;
        if (selectedType=="file")
            pathToDelete= selectedFilePath;
        else pathToDelete = selectedFileDirectory;

        if (selectedFilePath.isEmpty()) {
            return;
        }

        //confirm and delete
        if (QMessageBox::question(this,
                                  tr("Confirmation"),
                                  tr("<span style='color:red;'>DELETE</span><br/> %1 <br/>?").arg(pathToDelete))
            == QMessageBox::Yes) {

            QFile file(pathToDelete);
            if (file.exists()) {

                file.remove();

                QMessageBox::warning(this, tr("Warning"), tr("Deleted.") );

            } else {
                QMessageBox::warning(this, tr("Warning"), tr("Failed to delete."));
            }
        }
    }
    //--------------------------------------------------------------------------

//Methods-----------------------------------------------------------------------

    void MainWindow::openCatalogToExplore()
    {//Load the contents of a catalog to display folders in a tree and files in a list for direct browsing

        // Start animation while opening
        QApplication::setOverrideCursor(Qt::WaitCursor);

        //Check catalog's number of files and confirm load if too big
        if( collection->databaseMode == "Memory"
            and (exploreDevice->catalog->dateLoaded < exploreDevice->catalog->dateUpdated)){
            QSqlQuery query(QSqlDatabase::database("defaultConnection"));
            QString querySQL = QLatin1String(R"(
                                    SELECT device_total_file_count
                                    FROM device
                                    WHERE device_name=:device_name
                                    AND device_type ='Catalog'
                                )");
            query.prepare(querySQL);
            query.bindValue(":device_name",exploreDevice->name);
            query.exec();
            query.next();
            int selectedcatalogFileCount = query.value(0).toInt();
            int numberOfFilesWarningThreshold = 200000;
            if (selectedcatalogFileCount > numberOfFilesWarningThreshold){
                    int result = QMessageBox::warning(this,"Katalog",
                              tr("The selected catalog contains more than %1 files.<br/>"
                                 "It may take several minutes to open.<br/>"
                                 "Continue?").arg(QLocale().toString(numberOfFilesWarningThreshold)),QMessageBox::Yes|QMessageBox::Cancel);
                    if ( result ==QMessageBox::Cancel){
                        //Stop animation
                        QApplication::restoreOverrideCursor();
                        return;
                    }
            }
        }

        //Load folders of the Selected Catalog
            if( collection->databaseMode == "Memory")
                exploreDevice->catalog->loadFoldersToTable();

            loadCatalogDirectoriesToExplore();

        //Load the files of the Selected Catalog
            if( collection->databaseMode == "Memory"){
                QMutex tempMutex;
                bool tempStopRequested = false;
                exploreDevice->catalog->loadCatalogFileListToTable("defaultConnection", tempMutex, tempStopRequested);

            }
            loadSelectedDirectoryFilesToExplore();

        //Go to the Explorer tab
        ui->Explore_label_CatalogNameDisplay->setText(exploreDevice->name);
        ui->Explore_label_CatalogPathDisplay->setText(exploreDevice->path);

        //Remember last opened catalog
        QSettings settings(collection->settingsFilePath, QSettings:: IniFormat);
        settings.setValue("Explore/lastExploreDeviceID",    exploreDevice->ID);
        settings.setValue("Explore/lastExploreSelectedFolderFullPath", exploreSelectedFolderFullPath);
        settings.setValue("Explore/lastExploreSelectedDirectoryName",  exploreSelectedDirectoryName);

        //Stop animation
        QApplication::restoreOverrideCursor();
    }
    //----------------------------------------------------------------------
    void MainWindow::loadCatalogDirectoriesToExplore()
    {
        //Load the catalog's directories and display them

        //Prepare model
            ExploreTreeModel *exploreTreeModel = new ExploreTreeModel();
            exploreTreeModel->setCatalog(exploreDevice->externalID, exploreDevice->path);
            exploreTreeModel->setupModelData(exploreTreeModel->rootItem);

            ExploreTreeView *exploreProxyModel = new ExploreTreeView();
            exploreProxyModel->setSourceModel(exploreTreeModel);
            exploreProxyModel->caseSensitive = fileSortCaseSensitive;

        // Connect model to treeview
            ui->Explore_treeview_Directories->setSortingEnabled(true);
            ui->Explore_treeview_Directories->setModel(exploreProxyModel);
            ui->Explore_treeview_Directories->QTreeView::sortByColumn(0,Qt::SortOrder(0));

            ui->Explore_treeview_Directories->header()->resizeSection(0,  300);
            ui->Explore_treeview_Directories->hideColumn(1);//NoOfItems
            ui->Explore_treeview_Directories->hideColumn(2);//FullPath
            ui->Explore_treeview_Directories->expandToDepth(0);

        //Display number of directories and total size
            QString countSQL = QLatin1String(R"(
                                SELECT COUNT (DISTINCT (folder_path))
                                FROM folder
                                WHERE folder_catalog_id =:folder_catalog_id
                               )");
            QSqlQuery countQuery(QSqlDatabase::database("defaultConnection"));
            countQuery.prepare(countSQL);
            countQuery.bindValue(":folder_catalog_id", exploreDevice->externalID);
            countQuery.exec();
            countQuery.next();
            ui->Explore_label_DirectoryNumberDisplay->setText(QLocale().toString(countQuery.value(0).toLongLong()));
    }
    //----------------------------------------------------------------------
    void MainWindow::loadSelectedDirectoryFilesToExplore()
    {//Load the files of the exploreDevice and selected directory into the file view

        //Load all files and create model
        QString selectSQL;

        //Select folders based on selected options
        if(optionDisplayFolders==true){

            selectSQL = QLatin1String(R"(
                                    SELECT (REPLACE(folder_path, :selected_directory_full_path||"/", '')) AS file_name,
                                                NULL                  AS file_size,
                                                ""                    AS file_date_updated,
                                                folder_path           AS file_folder_path,
                                                folder_catalog_id     AS file_catalog,
                                                "folder"              AS entry_type,
                                                "1"||folder_path      AS order_value,
                                                folder_path
                                    FROM  folder
                                    WHERE folder_catalog_id=:folder_catalog_id
                            )");

            if(optionDisplaySubFolders != true){
                selectSQL = selectSQL + QLatin1String(R"(
                                    AND     (REPLACE(folder_path, :selected_directory_full_path||'/', ''))  NOT like "%/%"
                )");
            }
            else{
                selectSQL = selectSQL + QLatin1String(R"(
                                    AND     folder_path LIKE :selected_directory_full_path||'/%'
                )");
            }

            selectSQL = selectSQL + QLatin1String(R"(

                                    UNION

                                )");
        }

        //select files
        selectSQL += QLatin1String(R"(
                                    SELECT  file_name,
                                            file_size,
                                            file_date_updated,
                                            file_folder_path,
                                            file_catalog,
                                            "file" AS entry_type,
                                            "2"||file_name AS order_value,
                                            file_full_path
                                    FROM    file
                                    WHERE   file_catalog =:file_catalog
                                    AND     file_folder_path =:file_folder_path

                                    ORDER BY order_value ASC
                                )");

        if( exploreDevice->path == "EXPORT" ){
            exploreSelectedFolderFullPath.remove("EXPORT");
        }

        QSqlQuery loadCatalogQuery(QSqlDatabase::database("defaultConnection"));
        loadCatalogQuery.prepare(selectSQL);
        loadCatalogQuery.bindValue(":folder_catalog_id", exploreDevice->externalID);

        // fill lists depending on directory selection source
        loadCatalogQuery.bindValue(":file_catalog", exploreDevice->name);

        if(exploreDevice->path == exploreSelectedDirectoryName){
            loadCatalogQuery.bindValue(":file_folder_path",exploreSelectedDirectoryName);
            loadCatalogQuery.bindValue(":selectedCatalogPath", exploreSelectedDirectoryName);
            //loadCatalogQuery.bindValue(":selected_directory_full_path",selectedDirectoryFullPath);
        }
        else{
            loadCatalogQuery.bindValue(":file_folder_path", exploreSelectedFolderFullPath);
            loadCatalogQuery.bindValue(":selectedCatalogPath", exploreSelectedFolderFullPath);
            //loadCatalogQuery.bindValue(":selected_directory_full_path",selectedDirectoryFullPath);
        }

        loadCatalogQuery.bindValue(":selected_directory_full_path", exploreSelectedFolderFullPath);
        loadCatalogQuery.exec();

        QSqlQueryModel *loadCatalogQueryModel = new QSqlQueryModel;
        loadCatalogQueryModel->setQuery(std::move(loadCatalogQuery));

        FilesView *proxyModel2 = new FilesView(this);
        proxyModel2->caseSensitive = fileSortCaseSensitive;
        proxyModel2->setSourceModel(loadCatalogQueryModel);

        proxyModel2->setHeaderData(0, Qt::Horizontal, tr("Name"));
        proxyModel2->setHeaderData(1, Qt::Horizontal, tr("Size"));
        proxyModel2->setHeaderData(2, Qt::Horizontal, tr("Date"));
        proxyModel2->setHeaderData(3, Qt::Horizontal, tr("Directory"));
        proxyModel2->setHeaderData(4, Qt::Horizontal, tr("Catalog"));
        proxyModel2->setHeaderData(5, Qt::Horizontal, tr("Type"));
        proxyModel2->setHeaderData(6, Qt::Horizontal, tr("orderValue"));
        proxyModel2->setHeaderData(7, Qt::Horizontal, tr("Path"));

        // Connect model to tree/table view
        ui->Explore_treeView_FileList->setModel(proxyModel2);
        if (lastExploreSortSection !=6)
            ui->Explore_treeView_FileList->QTreeView::sortByColumn(lastExploreSortSection,Qt::SortOrder(lastExploreSortOrder));
        else
            ui->Explore_treeView_FileList->QTreeView::sortByColumn(6,Qt::AscendingOrder);

        ui->Explore_treeView_FileList->header()->setSectionResizeMode(QHeaderView::Interactive);
        ui->Explore_treeView_FileList->header()->resizeSection(0, 600); //Name
        ui->Explore_treeView_FileList->header()->resizeSection(1, 110); //Size
        ui->Explore_treeView_FileList->header()->resizeSection(2, 140); //Date
        ui->Explore_treeView_FileList->header()->resizeSection(3, 400); //Directory
        ui->Explore_treeView_FileList->hideColumn(3); //Directory
        ui->Explore_treeView_FileList->hideColumn(4); //Catalog
        ui->Explore_treeView_FileList->hideColumn(5); //Type
        ui->Explore_treeView_FileList->hideColumn(6); //orderValue
        ui->Explore_treeView_FileList->hideColumn(7); //Path

        //Display count of files and total size
        QString countSQL = QLatin1String(R"(
                                SELECT  count (*), sum(file_size)
                                FROM    file
                                WHERE   file_catalog_id =:file_catalog_id
                           )");

        if (exploreSelectedDirectoryName!=""){
            countSQL = countSQL + " AND file_folder_path =:file_folder_path";
        }

        QSqlQuery countQuery(QSqlDatabase::database("defaultConnection"));
        countQuery.prepare(countSQL);
        countQuery.bindValue(":file_catalog_id", exploreDevice->externalID);

        if(exploreDevice->path == exploreSelectedDirectoryName){
            countQuery.bindValue(":file_folder_path",exploreSelectedDirectoryName);
        }
        else{
            countQuery.bindValue(":file_folder_path", exploreSelectedFolderFullPath);
        }

        countQuery.exec();
        countQuery.next();

        ui->Explore_label_FilesNumberDisplay->setText(QLocale().toString(countQuery.value(0).toLongLong()));
        ui->Explore_label_TotalSizeLabelDisplay->setText(QLocale().formattedDataSize(countQuery.value(1).toLongLong()));
        ui->Explore_label_CatalogDirectoryDisplay->setText(exploreSelectedDirectoryName);
    }
    //----------------------------------------------------------------------
