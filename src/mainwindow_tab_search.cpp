/*LICENCE
    This file is part of Katalog

    Copyright (C) 2021, the Katalog Development team

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
// File Name:   mainwindow_tab_search.cpp
// Purpose:     methods for the screen SEARCH
// Description: https://stephanecouturier.github.io/Katalog/docs/Features/Search
// Author:      Stephane Couturier
/////////////////////////////////////////////////////////////////////////////
*/

#include "mainwindow.h"
#include "ui_mainwindow.h"
#include "catalog.h"
#include "filesview.h"
#include "searchprocess.h"

//TAB: SEARCH FILES ------------------------------------------------------------

    //User interactions

        //Buttons and other changes
        void MainWindow::on_Search_lineEdit_SearchText_returnPressed()
        {
            if(collection->databaseMode !="Memory"){
                searchFilesStoppable();
            }
            else
                searchFiles();
        }
        //----------------------------------------------------------------------
        void MainWindow::on_Search_pushButton_Search_clicked()
        {
            if(collection->databaseMode !="Memory"){
                searchFilesStoppable();
            }
            else
                searchFiles();
        }
        //----------------------------------------------------------------------
        void MainWindow::on_Search_pushButton_PasteFromClipboard_clicked()
        {
            QClipboard *clipboard = QGuiApplication::clipboard();
            QString originalText = clipboard->text();
            ui->Search_lineEdit_SearchText->setText(originalText);
        }
        //----------------------------------------------------------------------

        void MainWindow::on_Search_pushButton_CleanSearchText_clicked()
        {
            QString cleanedSearchText = ui->Search_lineEdit_SearchText->text();
            cleanedSearchText.replace("."," ");
            cleanedSearchText.replace(","," ");
            cleanedSearchText.replace("_"," ");
            cleanedSearchText.replace("-"," ");
            cleanedSearchText.replace("("," ");
            cleanedSearchText.replace(")"," ");
            cleanedSearchText.replace("["," ");
            cleanedSearchText.replace("]"," ");
            cleanedSearchText.replace("{"," ");
            cleanedSearchText.replace("}"," ");
            cleanedSearchText.replace("/"," ");
            cleanedSearchText.replace("\\"," ");
            cleanedSearchText.replace("'"," ");
            cleanedSearchText.replace("\""," ");

            ui->Search_lineEdit_SearchText->setText(cleanedSearchText);

        }
        //----------------------------------------------------------------------
        void MainWindow::on_Search_pushButton_ResetAll_clicked()
        {
            resetToDefaultSearchCriteria();
        }
        //----------------------------------------------------------------------
        void MainWindow::on_Search_pushButton_ProcessResults_clicked()
        {
            batchProcessSearchResults();
        }
        //----------------------------------------------------------------------
        void MainWindow::on_Search_treeView_FilesFound_clicked(const QModelIndex &index)
        {
            //Get file from selected row
            QString selectedFileName   = ui->Search_treeView_FilesFound->model()->index(index.row(), 0, QModelIndex()).data().toString();
            QString selectedFileFolder = ui->Search_treeView_FilesFound->model()->index(index.row(), 3, QModelIndex()).data().toString();
            QString selectedFile = selectedFileFolder+"/"+selectedFileName;
            //Open the file (fromLocalFile needed for spaces in file name)
            QDesktopServices::openUrl(QUrl::fromLocalFile(selectedFile));
        }
        //----------------------------------------------------------------------
        void MainWindow::on_Search_treeView_CatalogsFound_clicked(const QModelIndex &index)
        {
            //Refine the seach with the selction of one of the catalogs that have results

            //Get file from selected row
            selectedDevice->type= "Catalog";
            selectedDevice->ID = ui->Search_treeView_CatalogsFound->model()->index(index.row(), 1, QModelIndex()).data().toInt();
            selectedDevice->loadDevice("defaultConnection");
            displaySelectedDeviceName();

            //Seach again but only on the selected catalog
            searchFiles();
        }
        //----------------------------------------------------------------------
        void MainWindow::on_Search_pushButton_ShowHideSearchCriteria_clicked()
        {
            QString iconName = ui->Search_pushButton_ShowHideSearchCriteria->icon().name();
            if ( iconName == "go-up"){ //Hide
                    ui->Search_pushButton_ShowHideSearchCriteria->setIcon(QIcon::fromTheme("go-down"));
                    ui->Search_widget_SearchCriteria->setHidden(true);

                    QSettings settings(collection->settingsFilePath, QSettings:: IniFormat);
                    settings.setValue("Settings/ShowHideSearchCriteria", "go-down");
            }
            else{ //Show
                    ui->Search_pushButton_ShowHideSearchCriteria->setIcon(QIcon::fromTheme("go-up"));
                    ui->Search_widget_SearchCriteria->setHidden(false);

                    QSettings settings(collection->settingsFilePath, QSettings:: IniFormat);
                    settings.setValue("Settings/ShowHideSearchCriteria", "go-up");
            }
        }
        //----------------------------------------------------------------------
        void MainWindow::on_Search_pushButton_ShowHideCatalogResults_clicked()
        {
            QString iconName = ui->Search_pushButton_ShowHideCatalogResults->icon().name();

            if ( iconName == "go-previous"){ //Hide
                    ui->Search_pushButton_ShowHideCatalogResults->setIcon(QIcon::fromTheme("go-next"));
                    ui->Search_widget_ResultsCatalogs->setHidden(true);
                    ui->Search_label_CatalogsWithResults->setHidden(true);

                    QSettings settings(collection->settingsFilePath, QSettings:: IniFormat);
                    settings.setValue("Settings/ShowHideCatalogResults", "go-next");
            }
            else{ //Show
                    ui->Search_pushButton_ShowHideCatalogResults->setIcon(QIcon::fromTheme("go-previous"));

                    ui->Search_widget_ResultsCatalogs->setHidden(false);
                    ui->Search_label_CatalogsWithResults->setHidden(false);

                    QSettings settings(collection->settingsFilePath, QSettings:: IniFormat);
                    settings.setValue("Settings/ShowHideCatalogResults", "go-previous");
            }

        }
        //----------------------------------------------------------------------
        void MainWindow::on_Search_pushButton_ShowHideSearchHistory_clicked()
        {
            QString iconName = ui->Search_pushButton_ShowHideSearchHistory->icon().name();

            if ( iconName == "go-down"){ //Hide
                    ui->Search_pushButton_ShowHideSearchHistory->setIcon(QIcon::fromTheme("go-up"));
                    ui->Search_treeView_History->setHidden(true);

                    QSettings settings(collection->settingsFilePath, QSettings:: IniFormat);
                    settings.setValue("Settings/ShowHideSearchHistory", "go-up");
            }
            else{ //Show
                    ui->Search_pushButton_ShowHideSearchHistory->setIcon(QIcon::fromTheme("go-down"));
                    ui->Search_treeView_History->setHidden(false);

                    QSettings settings(collection->settingsFilePath, QSettings:: IniFormat);
                    settings.setValue("Settings/ShowHideSearchHistory", "go-down");
            }
        }
        //----------------------------------------------------------------------
        void MainWindow::on_Search_checkBox_ShowFolders_toggled(bool checked)
        {
            if(checked==1){
                ui->Search_checkBox_Duplicates->setChecked(false);
                ui->Search_checkBox_Differences->setChecked(false);
            }
        }
        //----------------------------------------------------------------------
        void MainWindow::on_Search_checkBox_Duplicates_toggled(bool checked)
        {
            if(checked==1){
                ui->Search_checkBox_DuplicatesName->setEnabled(true);
                ui->Search_checkBox_DuplicatesSize->setEnabled(true);
                ui->Search_checkBox_DuplicatesDateModified->setEnabled(true);
                ui->Search_checkBox_ShowFolders->setChecked(false);
                ui->Search_checkBox_Differences->setChecked(false);
                ui->Search_checkBox_DifferencesName->setEnabled(false);
                ui->Search_checkBox_DifferencesSize->setEnabled(false);
                ui->Search_checkBox_DifferencesDateModified->setEnabled(false);
            }
            else{
                ui->Search_checkBox_DuplicatesName->setDisabled(true);
                ui->Search_checkBox_DuplicatesSize->setDisabled(true);
                ui->Search_checkBox_DuplicatesDateModified->setDisabled(true);
            }
        }
        //----------------------------------------------------------------------
        void MainWindow::on_Search_checkBox_Differences_toggled(bool checked)
        {
            if(checked==1){
                ui->Search_checkBox_DifferencesName->setEnabled(true);
                ui->Search_checkBox_DifferencesSize->setEnabled(true);
                ui->Search_checkBox_DifferencesDateModified->setEnabled(true);
                ui->Search_widget_DifferencesCatalogs->setHidden(false);
                ui->Search_checkBox_ShowFolders->setChecked(false);
                ui->Search_checkBox_Duplicates->setChecked(false);
                ui->Search_checkBox_DuplicatesName->setEnabled(false);
                ui->Search_checkBox_DuplicatesSize->setEnabled(false);
                ui->Search_checkBox_DuplicatesDateModified->setEnabled(false);
                ui->Search_treeView_CatalogsFound->setEnabled(false);
            }
            else{
                ui->Search_widget_DifferencesCatalogs->setHidden(true);
                ui->Search_checkBox_DifferencesName->setDisabled(true);
                ui->Search_checkBox_DifferencesSize->setDisabled(true);
                ui->Search_checkBox_DifferencesDateModified->setDisabled(true);
                ui->Search_treeView_CatalogsFound->setEnabled(true);
            }
        }
        //----------------------------------------------------------------------
        void MainWindow::on_Search_checkBox_Size_toggled(bool checked)
        {
            if(checked==1){
                ui->Search_spinBox_MinimumSize->setEnabled(true);
                ui->Search_comboBox_MinSizeUnit->setEnabled(true);
                ui->Search_spinBox_MaximumSize->setEnabled(true);
                ui->Search_comboBox_MaxSizeUnit->setEnabled(true);
            }
            else{
                ui->Search_spinBox_MinimumSize->setDisabled(true);
                ui->Search_comboBox_MinSizeUnit->setDisabled(true);
                ui->Search_spinBox_MaximumSize->setDisabled(true);
                ui->Search_comboBox_MaxSizeUnit->setDisabled(true);
            }
        }
        //----------------------------------------------------------------------
        void MainWindow::on_Search_checkBox_Date_toggled(bool checked)
        {
            if(checked==1){
                ui->Search_dateTimeEdit_Min->setEnabled(true);
                ui->Search_dateTimeEdit_Max->setEnabled(true);
            }
            else{
                ui->Search_dateTimeEdit_Min->setDisabled(true);
                ui->Search_dateTimeEdit_Max->setDisabled(true);
            }
        }
        //----------------------------------------------------------------------
        void MainWindow::on_Search_checkBox_Tags_toggled(bool checked)
        {
            if(checked==1){
                ui->Search_comboBox_Tags->setEnabled(true);
            }
            else{
                ui->Search_comboBox_Tags->setDisabled(true);
            }
        }
        //----------------------------------------------------------------------
        void MainWindow::on_Search_checkBox_FileName_toggled(bool checked)
        {
            if(checked==true){
                ui->Search_widget_FileNameCriteria->setHidden(false);
                ui->Search_lineEdit_SearchText->setEnabled(true);
            }
            else{
                ui->Search_widget_FileNameCriteria->setHidden(true);
                ui->Search_lineEdit_SearchText->setDisabled(true);
            }
        }
        //----------------------------------------------------------------------
        void MainWindow::on_Search_checkBox_FileCriteria_toggled(bool checked)
        {
            if(checked==true){
                ui->Search_widget_FileCriteria->setHidden(false);
            }
            else{
                ui->Search_widget_FileCriteria->setHidden(true);
            }
        }
        //----------------------------------------------------------------------
        void MainWindow::on_Search_checkBox_Type_toggled(bool checked)
        {
            if(checked==1){
                ui->Search_comboBox_FileType->setEnabled(true);
            }
            else{
                ui->Search_comboBox_FileType->setDisabled(true);
            }
        }
        //----------------------------------------------------------------------
        void MainWindow::on_Search_checkBox_FolderCriteria_toggled(bool checked)
        {
            if(checked==true){
                ui->Search_widget_FolderCriteria->setHidden(false);
            }
            else{
                ui->Search_widget_FolderCriteria->setHidden(true);
            }
        }
        //----------------------------------------------------------------------
        void MainWindow::on_Search_treeView_History_activated(const QModelIndex &index)
        {//Load and restore the criteria of the selected search history
            loadSearch = new Search;
            loadSearch->searchDateTime = ui->Search_treeView_History->model()->index(index.row(), 0, QModelIndex()).data().toString();
            loadSearch->loadSearchHistoryCriteria();
            loadSearchCriteria(loadSearch);
        }
        //----------------------------------------------------------------------
        void MainWindow::on_Search_pushButton_FileFoundMoreStatistics_clicked()
        {
            QMessageBox msgBox;
            msgBox.setWindowTitle("Katalog");
            msgBox.setText(tr("<br/><b>Files Found Statistics</b><br/>"
                              "<table> <tr><td>Files found:  </td><td><b> %1 </b> <br/> </td></tr>"
                              "<tr><td>Total size:   </td><td><b> %2 </b>  </td></tr>"
                              "<tr><td>Average size: </td><td><b> %3 </b>  </td></tr>"
                              "<tr><td>Min size:     </td><td><b> %4 </b>  </td></tr>"
                              "<tr><td>Max size:     </td><td><b> %5 </b>  <br/></td></tr>"
                              "<tr><td>Min Date:     </td><td><b> %6 </b>  </td></tr>"
                              "<tr><td>Max Date:     </td><td><b> %7 </b>  </td></tr>"
                              "</table>"
                              ).arg(
                                   QString::number(newSearch->filesFoundNumber),
                                   QLocale().formattedDataSize(newSearch->filesFoundTotalSize),
                                   QLocale().formattedDataSize(newSearch->filesFoundAverageSize),
                                   QLocale().formattedDataSize(newSearch->filesFoundMinSize),
                                   QLocale().formattedDataSize(newSearch->filesFoundMaxSize),
                                   newSearch->filesFoundMinDate,
                                   newSearch->filesFoundMaxDate));
            msgBox.setIcon(QMessageBox::Information);
            msgBox.exec();
        }
        //----------------------------------------------------------------------
        void MainWindow::on_SearchTreeViewFilesFoundHeaderSortOrderChanged(){

            QSettings settings(collection->settingsFilePath, QSettings:: IniFormat);
            QHeaderView *searchTreeHeader = ui->Search_treeView_FilesFound->header();

            lastSearchSortSection = searchTreeHeader->sortIndicatorSection();
            lastSearchSortOrder   = searchTreeHeader->sortIndicatorOrder();

            settings.setValue("Search/lastSearchSortSection", QString::number(lastSearchSortSection));
            settings.setValue("Search/lastSearchSortOrder",   QString::number(lastSearchSortOrder));
        }
        //----------------------------------------------------------------------
        void MainWindow::on_SearchTreeViewHistoryHeaderSortOrderChanged(){

            QSettings settings(collection->settingsFilePath, QSettings:: IniFormat);
            QHeaderView *searchHistoryTreeHeader = ui->Search_treeView_History->header();

            lastSearchHistorySortSection = searchHistoryTreeHeader->sortIndicatorSection();
            lastSearchHistorySortOrder   = searchHistoryTreeHeader->sortIndicatorOrder();

            settings.setValue("Search/lastSearchHistorySortSection", QString::number(lastSearchHistorySortSection));
            settings.setValue("Search/lastSearchHistorySortOrder",   QString::number(lastSearchHistorySortOrder));
        }
        //----------------------------------------------------------------------
        void MainWindow::on_Search_splitter_Results_splitterMoved()
        {
            QSettings settings(collection->settingsFilePath, QSettings:: IniFormat);
            settings.setValue("Search/ResultsSplitterWidget1Size", ui->Search_widget_ResultsCatalogs->size());
            settings.setValue("Search/ResultsSplitterWidget2Size", ui->Search_widget_ResultsFiles->size());
        }
        //----------------------------------------------------------------------

        //Context Menu methods
        void MainWindow::on_Search_treeView_FilesFound_customContextMenuRequested(const QPoint &pos)
        {
            QPoint globalPos = ui->Search_treeView_FilesFound->mapToGlobal(pos);
            QMenu fileContextMenu;

            QAction *menuAction1 = new QAction(QIcon::fromTheme("document-open"),(tr("Open file")), this);
            connect(menuAction1, &QAction::triggered, this, &MainWindow::searchContextOpenFile);
            fileContextMenu.addAction(menuAction1);

            QAction *menuAction2 = new QAction(QIcon::fromTheme("document-open"),(tr("Open folder")), this);
            connect(menuAction2, &QAction::triggered, this, &MainWindow::searchContextOpenFolder);
            fileContextMenu.addAction(menuAction2);

            QAction *menuAction10 = new QAction(QIcon::fromTheme("document-open"),(tr("Explore folder")), this);
            connect(menuAction10, &QAction::triggered, this, &MainWindow::searchContextOpenExplore);
            fileContextMenu.addAction(menuAction10);

            fileContextMenu.addSeparator();

            QAction *menuAction3 = new QAction(QIcon::fromTheme("edit-copy"),(tr("Copy folder path")), this);
            connect( menuAction3,&QAction::triggered, this, &MainWindow::searchContextCopyFolderPath);
            fileContextMenu.addAction(menuAction3);

            QAction *menuAction4 = new QAction(QIcon::fromTheme("edit-copy"),(tr("Copy file absolute path")), this);
            connect( menuAction4,&QAction::triggered, this, &MainWindow::searchContextCopyAbsolutePath);
            fileContextMenu.addAction(menuAction4);

            QAction *menuAction5 = new QAction(QIcon::fromTheme("edit-copy"),(tr("Copy file name with extension")), this);
            connect( menuAction5,&QAction::triggered, this, &MainWindow::searchContextCopyFileNameWithExtension);
            fileContextMenu.addAction(menuAction5);

            QAction *menuAction6 = new QAction(QIcon::fromTheme("edit-copy"),(tr("Copy file name without extension")), this);
            connect( menuAction6,&QAction::triggered, this, &MainWindow::searchContextCopyFileNameWithoutExtension);
            fileContextMenu.addAction(menuAction6);

            fileContextMenu.addSeparator();

            //DEV
            if (developmentMode == true){
                QAction *menuAction7 = new QAction(QIcon::fromTheme("document-export"),(tr("Move file to other folder")), this);
                connect( menuAction7,&QAction::triggered, this, &MainWindow::searchContextMoveFileToFolder);
                fileContextMenu.addAction(menuAction7);
            }

            QAction *menuAction8 = new QAction(QIcon::fromTheme("user-trash"),(tr("Move to Trash")), this);
            connect( menuAction8,&QAction::triggered, this, &MainWindow::searchContextMoveFileToTrash);
            fileContextMenu.addAction(menuAction8);

            QAction *menuAction9 = new QAction(QIcon::fromTheme("edit-delete"),(tr("Delete file")), this);
            connect( menuAction9,&QAction::triggered, this, &MainWindow::searchContextDeleteFile);
            fileContextMenu.addAction(menuAction9);

            QAction* selectedItem = fileContextMenu.exec(globalPos);
            if (selectedItem)
            {
                //something
            }
            else
            {
                //did nothing
            }
        }
        //----------------------------------------------------------------------
        void MainWindow::searchContextOpenFile()
        {
            QModelIndex index=ui->Search_treeView_FilesFound->currentIndex();
            //Get filepath from selected row
            QString selectedFileName   = ui->Search_treeView_FilesFound->model()->index(index.row(), 0, QModelIndex()).data().toString();
            QString selectedFileFolder = ui->Search_treeView_FilesFound->model()->index(index.row(), 3, QModelIndex()).data().toString();
            QString selectedFile = selectedFileFolder+"/"+selectedFileName;
            //Open the file (fromLocalFile needed for spaces in file name)
            QDesktopServices::openUrl(QUrl::fromLocalFile(selectedFile));
        }
        //----------------------------------------------------------------------
        void MainWindow::searchContextOpenFolder()
        {
            QModelIndex index = ui->Search_treeView_FilesFound->currentIndex();
            QString selectedFileName   = ui->Search_treeView_FilesFound->model()->index(index.row(), 0, QModelIndex()).data().toString();
            QString selectedFileFolder = ui->Search_treeView_FilesFound->model()->index(index.row(), 3, QModelIndex()).data().toString();
            QString selectedFile = selectedFileFolder+"/"+selectedFileName;
            QString folderName = selectedFile.left(selectedFile.lastIndexOf("/"));
            QDesktopServices::openUrl(QUrl::fromLocalFile(folderName));
        }
        //----------------------------------------------------------------------
        void MainWindow::searchContextOpenExplore()
        {
            //Get values from selection
            QModelIndex index = ui->Search_treeView_FilesFound->currentIndex();
            exploreSelectedFolderFullPath  = ui->Search_treeView_FilesFound->model()->index(index.row(), 3, QModelIndex()).data().toString();
            QString selectedResultFileCatalog = ui->Search_treeView_FilesFound->model()->index(index.row(), 4, QModelIndex()).data().toString();

            //Prepare inputs for the Explore tab

            //Get catalog id
            QSqlQuery query(QSqlDatabase::database("defaultConnection"));
            QString querySQL = QLatin1String(R"(
                                    SELECT device_id
                                    FROM device
                                    WHERE device_name =:device_name
                                    AND device_type = 'Catalog'
                                )");
            query.prepare(querySQL);
            query.bindValue(":device_name", selectedResultFileCatalog);
            query.exec();
            query.next();

            //load device to be used in explore
            exploreDevice->ID = query.value(0).toInt();
            exploreDevice->loadDevice("defaultConnection");

            //Pass selected directory name
            exploreSelectedDirectoryName = exploreSelectedFolderFullPath;
            exploreSelectedDirectoryName.remove(exploreDevice->path + "/");

            //Open the catalog into the Explore tab and display selected directory contents
            openCatalogToExplore();
            ui->tabWidget->setCurrentIndex(2);
        }
        //----------------------------------------------------------------------
        void MainWindow::searchContextCopyAbsolutePath()
        {
            QModelIndex index=ui->Search_treeView_FilesFound->currentIndex();

            QString selectedFileName =   ui->Search_treeView_FilesFound->model()->index(index.row(), 0, QModelIndex()).data().toString();
            QString selectedFileFolder = ui->Search_treeView_FilesFound->model()->index(index.row(), 3, QModelIndex()).data().toString();
            QString selectedFile = selectedFileFolder+"/"+selectedFileName;

            QClipboard *clipboard = QGuiApplication::clipboard();
            clipboard->setText(selectedFile);
        }
        //----------------------------------------------------------------------
        void MainWindow::searchContextCopyFolderPath()
        {
            QModelIndex index = ui->Search_treeView_FilesFound->currentIndex();
            QString selectedFileFolder = ui->Search_treeView_FilesFound->model()->index(index.row(), 3, QModelIndex()).data().toString();

            QClipboard *clipboard = QGuiApplication::clipboard();
            clipboard->setText(selectedFileFolder);
        }
        //----------------------------------------------------------------------
        void MainWindow::searchContextCopyFileNameWithExtension()
        {
            QModelIndex index = ui->Search_treeView_FilesFound->currentIndex();
            QString selectedFileName = ui->Search_treeView_FilesFound->model()->index(index.row(), 0, QModelIndex()).data().toString();

            QString fileNameWithExtension = selectedFileName;

            QClipboard *clipboard = QGuiApplication::clipboard();
            clipboard->setText(fileNameWithExtension);
        }
        //----------------------------------------------------------------------
        void MainWindow::searchContextCopyFileNameWithoutExtension()
        {
            QModelIndex index=ui->Search_treeView_FilesFound->currentIndex();

            QString selectedFileName   = ui->Search_treeView_FilesFound->model()->index(index.row(), 0, QModelIndex()).data().toString();
            QString selectedFileFolder = ui->Search_treeView_FilesFound->model()->index(index.row(), 3, QModelIndex()).data().toString();
            QString selectedFile = selectedFileFolder+"/"+selectedFileName;

            QFileInfo fileName;
            fileName.setFile(selectedFile);
            QString fileNameWithoutExtension = fileName.completeBaseName();

            QClipboard *clipboard = QGuiApplication::clipboard();
            clipboard->setText(fileNameWithoutExtension);
        }
        //----------------------------------------------------------------------
        void MainWindow::searchContextMoveFileToFolder()
        {
            QModelIndex index=ui->Search_treeView_FilesFound->currentIndex();

            QString selectedFileName   = ui->Search_treeView_FilesFound->model()->index(index.row(), 0, QModelIndex()).data().toString();
            QString selectedFileFolder = ui->Search_treeView_FilesFound->model()->index(index.row(), 3, QModelIndex()).data().toString();
            QString selectedFile = selectedFileFolder+"/"+selectedFileName;           

            if (selectedFileName.isEmpty()) {
                return;
            }

            if (QMessageBox::question(this,
                                      tr("Confirmation"),
                                      tr("Move\n%1\nto another folder?").arg(selectedFile))
                == QMessageBox::Yes) {
                QFile file(selectedFile);
                if (file.exists()) {
                    //Open a dialog for the user to select the target folder
                    QString dir = QFileDialog::getExistingDirectory(this, tr("Select the folder to move this file"),
                                                                    collection->folder,
                                                                    QFileDialog::ShowDirsOnly
                                                                    | QFileDialog::DontResolveSymlinks);

                    //Check and move the file
                    if ( dir !=""){
                        //verify file exists and decide to overwrite
                        QString targetFilePath = dir + "/" + file.fileName();
                        QFile targetFile(targetFilePath);
                        if (file.exists()) {
                            if (QMessageBox::question(this,
                                                      tr("Confirmation"),
                                                      tr("A file %& already exists. Overwrite it?").arg(targetFilePath))
                                == QMessageBox::Yes) {
                                //overwrite
                            }
                            else
                                 QMessageBox::warning(this, tr("Warning"), tr("Cancelled move to folder."));
                            return;
                        }
                        //remove exisiting

                        //copy

                        //move file
                        QMessageBox::warning(this, tr("Warning"), tr("Moved to folder:<br/>") + dir);
                    }

                } else {
                    QMessageBox::warning(this, tr("Warning"), tr("This file cannot be moved (offline or not existing)."));
                }
            }
        }
        //----------------------------------------------------------------------
        void MainWindow::searchContextMoveFileToTrash()
        {
            if(newSearch->showFoldersOnly==false){
                QModelIndex index=ui->Search_treeView_FilesFound->currentIndex();
                QString selectedFileName     = ui->Search_treeView_FilesFound->model()->index(index.row(), 0, QModelIndex()).data().toString();
                QString selectedFileFolder   = ui->Search_treeView_FilesFound->model()->index(index.row(), 3, QModelIndex()).data().toString();
                QString selectedFileFullPath = selectedFileFolder+"/"+selectedFileName;
                QString pathInTrash;

                if (QMessageBox::warning(this,
                                         tr("Confirmation"),
                                         "<span style='color:orange;font-weight: bold;'>"+tr("MOVE")+"</span><br/>"
                                             +tr("Move this file to the trash?")+QString("<br/><span style='font-style: italic;'>%1</span><br/>").arg(selectedFileFullPath),
                                         QMessageBox::Yes|QMessageBox::Cancel)
                    == QMessageBox::Yes){

                    pathInTrash = moveFileToTrash(selectedFileFullPath);

                    if (pathInTrash!=""){
                        QMessageBox::information(this, tr("Information"), tr("Moved to trash:<br/>") + pathInTrash);
                    }
                    else{
                        QMessageBox::warning(this, tr("Warning"), tr("Move to trash failed."));
                    }
                }
            }
            else{
                QMessageBox msgBox;
                msgBox.setWindowTitle("Katalog");
                msgBox.setText(tr("Moving a folder to Trash is not available."));
                msgBox.setIcon(QMessageBox::Information);
                msgBox.exec();
            }
        }
        //----------------------------------------------------------------------
        QString MainWindow::moveFileToTrash(QString fileFullPath)
        {
            QString pathInTrash;
            QFile::moveToTrash(fileFullPath, &pathInTrash);
            return pathInTrash;
        }
        //----------------------------------------------------------------------
        void MainWindow::searchContextDeleteFile()
        {
            if(newSearch->showFoldersOnly==false){
                QModelIndex index=ui->Search_treeView_FilesFound->currentIndex();
                QString selectedFileName     = ui->Search_treeView_FilesFound->model()->index(index.row(), 0, QModelIndex()).data().toString();
                QString selectedFileFolder   = ui->Search_treeView_FilesFound->model()->index(index.row(), 3, QModelIndex()).data().toString();
                QString selectedFileFullPath = selectedFileFolder+"/"+selectedFileName;
                QString result;

                if (QMessageBox::critical(this,
                                          tr("Confirmation"),
                                          "<span style='color:red;font-weight: bold;'>"+tr("DELETE")+"</span><br/>"
                                              +tr("Delete this file?")+QString("<br/><span style='font-style: italic;'>%1").arg(selectedFileFullPath),
                                          QMessageBox::Yes|QMessageBox::Cancel)
                    == QMessageBox::Yes) {

                    result = deleteFile(selectedFileFullPath);

                    if (result!=""){
                        QMessageBox::information(this, tr("Information"), tr("Deleted.") );
                    }
                    else{
                        QMessageBox::warning(this, tr("Warning"), tr("Failed to delete."));
                    }
                }
            }
            else{
                QMessageBox msgBox;
                msgBox.setWindowTitle("Katalog");
                msgBox.setText(tr("Deleting a folder is not available."));
                msgBox.setIcon(QMessageBox::Information);
                msgBox.exec();
            }
        }
        //----------------------------------------------------------------------
        QString MainWindow::deleteFile(QString fileFullPath)
        {
                QFile file(fileFullPath);
                if (file.exists()) {
                    file.remove();
                    return "deleted";
                }
                else{
                    return "";
                }
        }

    //Methods-----------------------------------------------------------------------

        //Search methods
        void MainWindow::searchFiles()
        {//Run a search of files in each selected catalog based on user inputs

            // Start animation while opening
            QApplication::setOverrideCursor(Qt::WaitCursor);

            //Prepare the SEARCH -------------------------------

                //Clear the object newSearch and get new search criteria
                    getSearchCriteria();

                // Searching "Begin With" for "File name or Folder name" is not supported yet
                    if (newSearch->selectedTextCriteria==tr("Begins With") and newSearch->selectedSearchIn !=tr("File names only")){
                        QApplication::restoreOverrideCursor(); //Stop animation
                        QMessageBox::information(this,"Katalog",tr("The option 'Begin With' can only be used with 'File names only'.\nUse a different combinaison."));
                        return;
                    }

            //Process the SEARCH in CATALOGS or DIRECTORY ------------------------------
                //Process the SEARCH in CATALOGS
                    if (newSearch->searchInCatalogsChecked == true){

                        //For differences, only process the 2 selected catalogs
                        if (ui->Search_checkBox_Differences->isChecked() == true){
                            Device *diffDevice = new Device;
                            diffDevice->ID = ui->Search_comboBox_DifferencesCatalog1->currentData().toInt();
                            diffDevice->loadDevice("defaultConnection");
                            searchFilesInCatalog(diffDevice);

                            diffDevice->ID = ui->Search_comboBox_DifferencesCatalog2->currentData().toInt();
                            diffDevice->loadDevice("defaultConnection");
                            searchFilesInCatalog(diffDevice);
                        }
                        //Otherwise search in the list of catalogs in the selectedDevice
                        else{
                            if(selectedDevice->type == "Catalog") {
                                searchFilesInCatalog(selectedDevice);
                            }
                            else{
                                foreach (const Device::deviceListRow &row, selectedDevice->deviceListTable) {
                                    if(row.type == "Catalog"){
                                        Device *device = new Device;
                                        device->ID = row.ID;
                                        device->loadDevice("defaultConnection");
                                        searchFilesInCatalog(device);
                                    }
                                }
                            }
                        }
                    }
                //Process the SEARCH in SELECTED DIRECTORY
                    else if (newSearch->searchInConnectedChecked == true){
                            QString sourceDirectory = ui->Filters_lineEdit_SeletedDirectory->text();
                            searchFilesInDirectory(sourceDirectory);
                    }

                //Process search results: list of catalogs with results
                    //Remove duplicates so the catalogs are listed only once, and sort the list
                    newSearch->deviceFoundIDList.removeDuplicates();
                    newSearch->deviceFoundIDList.sort();

                    //Keep the catalog file name only
                    foreach(QString item, newSearch->deviceFoundIDList){
                            int index = newSearch->deviceFoundIDList.indexOf(item);
                            QFileInfo fileInfo(item);
                            newSearch->deviceFoundIDList[index] = fileInfo.baseName();
                    }

                    //Create model and load to the view
                    newSearch->deviceFoundModel = new QStandardItemModel;
                    newSearch->deviceFoundModel->setHorizontalHeaderLabels({ "Catalog with results", "ID" });

                    Device loopDevice;
                    for (const QString &ID : newSearch->deviceFoundIDList) {
                        loopDevice.ID = ID.toInt();
                        loopDevice.loadDevice("defaultConnection");
                        QList<QStandardItem *> items;
                        items << new QStandardItem(loopDevice.name);
                        items << new QStandardItem(QString::number(loopDevice.ID));
                        newSearch->deviceFoundModel->appendRow(items);
                    }

                    ui->Search_treeView_CatalogsFound->setModel(newSearch->deviceFoundModel);
                    ui->Search_treeView_CatalogsFound->hideColumn(1);

                //Process search results: list of files

                    //Prepare query model
                    QSqlQueryModel *loadCatalogQueryModel = new QSqlQueryModel;
                    // Prepare model to display
                    FilesView *fileViewModel = new FilesView(this);

                    //Populate model with folders only if this option is selected
                    if ( newSearch->searchOnFolderCriteria==true and ui->Search_checkBox_ShowFolders->isChecked()==true )
                    {
                        newSearch->filePaths.removeDuplicates();
                        int numberOfFolders = newSearch->filePaths.count();
                        newSearch->fileNames.clear();
                        newSearch->fileSizes.clear();
                        newSearch->fileDateTimes.clear();
                        newSearch->fileCatalogs.clear();
                        for (int i=0; i<numberOfFolders; i++)
                            newSearch->fileNames <<"";
                        for (int i=0; i<numberOfFolders; i++)
                            newSearch->fileSizes <<0;
                        for (int i=0; i<numberOfFolders; i++)
                            newSearch->fileDateTimes <<"";
                        for (int i=0; i<numberOfFolders; i++)
                            newSearch->fileCatalogs <<"";

                        // Populate model with data
                        fileViewModel->setSourceModel(newSearch);
                        fileViewModel->setHeaderData(0, Qt::Horizontal, tr("Name"));
                        fileViewModel->setHeaderData(1, Qt::Horizontal, tr("Size"));
                        fileViewModel->setHeaderData(2, Qt::Horizontal, tr("Date"));
                        fileViewModel->setHeaderData(3, Qt::Horizontal, tr("Folder"));
                        fileViewModel->setHeaderData(4, Qt::Horizontal, tr("Catalog"));

                        // Connect model to treeview and display
                        ui->Search_treeView_FilesFound->setModel(fileViewModel);
                        ui->Search_treeView_FilesFound->header()->resizeSection(3, 400); //Path
                        ui->Search_treeView_FilesFound->header()->resizeSection(4, 100); //Catalog
                        ui->Search_treeView_FilesFound->header()->hideSection(0);
                        ui->Search_treeView_FilesFound->header()->hideSection(1);
                        ui->Search_treeView_FilesFound->header()->hideSection(2);

                        ui->Search_label_FoundTitle->setText(tr("Folders found"));
                    }

                    //Populate model with files if the folder option is not selected
                    else
                    {
                        // Populate model with data
                        fileViewModel->setSourceModel(newSearch);
                        fileViewModel->setHeaderData(0, Qt::Horizontal, tr("Name"));
                        fileViewModel->setHeaderData(1, Qt::Horizontal, tr("Size"));
                        fileViewModel->setHeaderData(2, Qt::Horizontal, tr("Date"));
                        fileViewModel->setHeaderData(3, Qt::Horizontal, tr("Folder"));
                        fileViewModel->setHeaderData(4, Qt::Horizontal, tr("Catalog"));
                        if (newSearch->searchInConnectedChecked == true){
                            fileViewModel->setHeaderData(3, Qt::Horizontal, tr("Search Directory"));
                        }

                        // Connect model to tree/table view
                        ui->Search_treeView_FilesFound->setModel(fileViewModel);
                        ui->Search_treeView_FilesFound->header()->setSectionResizeMode(QHeaderView::Interactive);
                        ui->Search_treeView_FilesFound->header()->resizeSection(0, 600); //Name
                        ui->Search_treeView_FilesFound->header()->resizeSection(1, 110); //Size
                        ui->Search_treeView_FilesFound->header()->resizeSection(2, 140); //Date
                        ui->Search_treeView_FilesFound->header()->resizeSection(3, 400); //Path
                        ui->Search_treeView_FilesFound->header()->resizeSection(4, 140); //Catalog
                        ui->Search_treeView_FilesFound->header()->showSection(0);
                        ui->Search_treeView_FilesFound->header()->showSection(1);
                        ui->Search_treeView_FilesFound->header()->showSection(2);

                        ui->Search_label_FoundTitle->setText(tr("Files found"));
                    }

                //Process DUPLICATES -------------------------------

                    //Process if enabled and criteria are provided
                        if ( newSearch->searchOnFileCriteria == true and ui->Search_checkBox_Duplicates->isChecked() == true
                             and (     newSearch->searchDuplicatesOnName == true
                                    or newSearch->searchDuplicatesOnSize == true
                                    or newSearch->searchDuplicatesOnDate == true )){

							//Load Search results into the database
                                //clear database
                                    QSqlQuery deleteQuery(QSqlDatabase::database("defaultConnection"));
                                    deleteQuery.exec("DELETE FROM filetemp");

                                //prepare query to load file info
                                    QSqlQuery insertQuery(QSqlDatabase::database("defaultConnection"));
                                    QString insertSQL = QLatin1String(R"(
                                                        INSERT INTO filetemp (
                                                                        file_catalog_id,
                                                                        file_name,
                                                                        file_folder_path,
                                                                        file_size,
                                                                        file_date_updated,
                                                                        file_catalog )
                                                        VALUES(
                                                                        :file_catalog_id,
                                                                        :file_name,
                                                                        :file_folder_path,
                                                                        :file_size,
                                                                        :file_date_updated,
                                                                        :file_catalog )
                                                                    )");
                                    insertQuery.prepare(insertSQL);

                                //loop through the result list and populate database

                                    int rows = newSearch->rowCount();

                                    for (int i=0; i<rows; i++) {

                                            //Append data to the database
                                            insertQuery.bindValue(":file_catalog_id",   newSearch->index(i,0).data().toString());
                                            insertQuery.bindValue(":file_name",         newSearch->index(i,0).data().toString());
                                            insertQuery.bindValue(":file_size",         newSearch->index(i,1).data().toString());
                                            insertQuery.bindValue(":file_folder_path",  newSearch->index(i,3).data().toString());
                                            insertQuery.bindValue(":file_date_updated", newSearch->index(i,2).data().toString());
                                            insertQuery.bindValue(":file_catalog",      newSearch->index(i,4).data().toString());
                                            insertQuery.exec();
                                    }

                            //Prepare duplicate SQL
                                // Load all files and create model
                                QString selectSQL;

                                //Generate grouping of fields based on user selection, determining what are duplicates
                                QString groupingFields; // this value should be a concatenation of fields, like "fileName||fileSize"

                                    //Same name
                                    if(newSearch->searchDuplicatesOnName == true){
                                        groupingFields = groupingFields + "file_name";
                                    }
                                    //Same size
                                    if(newSearch->searchDuplicatesOnSize == true){
                                        groupingFields = groupingFields + "||file_size";
                                    }
                                    //Same date modified
                                    if(newSearch->searchDuplicatesOnDate == true){
                                        groupingFields = groupingFields + "||file_date_updated";
                                    }

                                    //Remove starting || if any
                                    if (groupingFields.startsWith("||"))
                                        groupingFields.remove(0, 2);

                                //Generate SQL based on grouping of fields
                                selectSQL = QLatin1String(R"(
                                                SELECT      file_name,
                                                            file_size,
                                                            file_date_updated,
                                                            file_folder_path,
                                                            file_catalog
                                                FROM filetemp
                                                WHERE %1 IN
                                                    (SELECT %1
                                                    FROM filetemp
                                                    GROUP BY %1
                                                    HAVING count(%1)>1)
                                                ORDER BY %1
                                            )").arg(groupingFields);

                                //Run Query and load to model
                                QSqlQuery duplicatesQuery(QSqlDatabase::database("defaultConnection"));
                                duplicatesQuery.prepare(selectSQL);
                                duplicatesQuery.exec();

                                //recapture file results for Stats
                                newSearch->fileNames.clear();
                                newSearch->fileSizes.clear();
                                newSearch->filePaths.clear();
                                newSearch->fileDateTimes.clear();
                                newSearch->fileCatalogs.clear();
                                while(duplicatesQuery.next()){
                                    newSearch->fileNames.append(duplicatesQuery.value(0).toString());
                                    newSearch->fileSizes.append(duplicatesQuery.value(1).toLongLong());
                                    newSearch->fileDateTimes.append(duplicatesQuery.value(2).toString());
                                    newSearch->filePaths.append(duplicatesQuery.value(3).toString());
                                    newSearch->fileCatalogs.append(duplicatesQuery.value(4).toString());
                                }

                                //Load results to model
                                loadCatalogQueryModel->setQuery(std::move(duplicatesQuery));

                                //FilesView *fileViewModel = new FilesView(this);
                                fileViewModel->setSourceModel(loadCatalogQueryModel);
                                fileViewModel->setHeaderData(0, Qt::Horizontal, tr("Name"));
                                fileViewModel->setHeaderData(1, Qt::Horizontal, tr("Size"));
                                fileViewModel->setHeaderData(2, Qt::Horizontal, tr("Date"));
                                fileViewModel->setHeaderData(3, Qt::Horizontal, tr("Folder"));
                                fileViewModel->setHeaderData(4, Qt::Horizontal, tr("Catalog"));

                                // Connect model to tree/table view
                                ui->Search_treeView_FilesFound->setModel(fileViewModel);
                                ui->Search_treeView_FilesFound->header()->setSectionResizeMode(QHeaderView::Interactive);
                                ui->Search_treeView_FilesFound->header()->resizeSection(0, 600); //Name
                                ui->Search_treeView_FilesFound->header()->resizeSection(1, 110); //Size
                                ui->Search_treeView_FilesFound->header()->resizeSection(2, 140); //Date
                                ui->Search_treeView_FilesFound->header()->resizeSection(3, 400); //Path
                                ui->Search_treeView_FilesFound->header()->resizeSection(4, 100); //Catalog

                                ui->Search_label_FoundTitle->setText(tr("Duplicates found"));
                                newSearch->filesFoundNumber = fileViewModel->rowCount();

                        }

                //Process DIFFERENCES -------------------------------

                    //Process if enabled and criteria are provided
                        if ( newSearch->searchOnFileCriteria == true and ui->Search_checkBox_Differences->isChecked() == true
                             and (     newSearch->differencesOnName == true
                                    or newSearch->differencesOnSize == true
                                    or newSearch->differencesOnDate == true)){

							//Load Search results into the database
                                //Clear database
                                    QSqlQuery deleteQuery(QSqlDatabase::database("defaultConnection"));
                                    deleteQuery.exec("DELETE FROM filetemp");

                                //Prepare query to load file info
                                    QSqlQuery insertQuery(QSqlDatabase::database("defaultConnection"));
                                    QString insertSQL = QLatin1String(R"(
                                                        INSERT INTO filetemp (
                                                                        file_name,
                                                                        file_folder_path,
                                                                        file_size,
                                                                        file_date_updated,
                                                                        file_catalog )
                                                        VALUES(
                                                                        :file_name,
                                                                        :file_folder_path,
                                                                        :file_size,
                                                                        :file_date_updated,
                                                                        :file_catalog )
                                                                    )");
                                    insertQuery.prepare(insertSQL);

                                //Loop through the result list and populate database

                                    int rows = newSearch->rowCount();

                                    for (int i=0; i<rows; i++) {

                                            QString test = newSearch->index(i,0).data().toString();

                                            //Append data to the database
                                            insertQuery.bindValue(":file_name",        newSearch->index(i,0).data().toString());
                                            insertQuery.bindValue(":file_size",        newSearch->index(i,1).data().toString());
                                            insertQuery.bindValue(":file_folder_path", newSearch->index(i,3).data().toString());
                                            insertQuery.bindValue(":file_date_updated",newSearch->index(i,2).data().toString());
                                            insertQuery.bindValue(":file_catalog",     newSearch->index(i,4).data().toString());
                                            insertQuery.exec();

                                    }

                            //Prepare difference SQL
                                // Load all files and create model
                                QString selectSQL;

                                //Generate grouping of fields based on user selection, determining what are duplicates
                                QString groupingFieldsDifferences; // this value should be a concatenation of fields, like "fileName||fileSize"

                                    //Same name
                                    if(newSearch->differencesOnName == true){
                                        groupingFieldsDifferences += "||file_name";
                                    }
                                    //Same size
                                    if(newSearch->differencesOnSize == true){
                                        groupingFieldsDifferences += "||file_size";
                                    }
                                    //Same date modified
                                    if(newSearch->differencesOnDate == true){
                                        groupingFieldsDifferences += "||file_date_updated";
                                    }

                                    //Remove the || at the start
                                    if (groupingFieldsDifferences.startsWith("||"))
                                        groupingFieldsDifferences.remove(0, 2);

                                //Generate SQL based on grouping of fields
                                selectSQL = QLatin1String(R"(
                                                 SELECT      file_name,
                                                             file_size,
                                                             file_date_updated,
                                                             file_folder_path,
                                                             file_catalog
                                                 FROM filetemp
                                                 WHERE file_catalog = :selectedDifferencesCatalog1
                                                 AND %1 NOT IN(
                                                     SELECT %1
                                                     FROM filetemp
                                                     WHERE file_catalog = :selectedDifferencesCatalog2
                                                     )
                                                 UNION
                                                 SELECT      file_name,
                                                             file_size,
                                                             file_date_updated,
                                                             file_folder_path,
                                                             file_catalog
                                                 FROM filetemp
                                                 WHERE file_catalog = :selectedDifferencesCatalog2
                                                 AND %1 NOT IN(
                                                     SELECT %1
                                                     FROM filetemp
                                                     WHERE file_catalog = :selectedDifferencesCatalog1
                                                     )
                                 )").arg(groupingFieldsDifferences);

                                //Run Query and load to model
                                QSqlQuery differencesQuery(QSqlDatabase::database("defaultConnection"));
                                differencesQuery.prepare(selectSQL);
                                differencesQuery.bindValue(":selectedDifferencesCatalog1",newSearch->differencesCatalog1);
                                differencesQuery.bindValue(":selectedDifferencesCatalog2",newSearch->differencesCatalog2);
                                differencesQuery.exec();

                                //recapture file results for Stats
                                newSearch->fileNames.clear();
                                newSearch->fileSizes.clear();
                                newSearch->filePaths.clear();
                                newSearch->fileDateTimes.clear();
                                newSearch->fileCatalogs.clear();
                                while(differencesQuery.next()){
                                    newSearch->fileNames.append(differencesQuery.value(0).toString());
                                    newSearch->fileSizes.append(differencesQuery.value(1).toLongLong());
                                    newSearch->fileDateTimes.append(differencesQuery.value(2).toString());
                                    newSearch->filePaths.append(differencesQuery.value(3).toString());
                                    newSearch->fileCatalogs.append(differencesQuery.value(4).toString());
                                }

                                loadCatalogQueryModel->setQuery(std::move(differencesQuery));

                                //FilesView *fileViewModel = new FilesView(this);
                                fileViewModel->setSourceModel(loadCatalogQueryModel);
                                fileViewModel->setHeaderData(0, Qt::Horizontal, tr("Name"));
                                fileViewModel->setHeaderData(1, Qt::Horizontal, tr("Size"));
                                fileViewModel->setHeaderData(2, Qt::Horizontal, tr("Date"));
                                fileViewModel->setHeaderData(3, Qt::Horizontal, tr("Folder"));
                                fileViewModel->setHeaderData(4, Qt::Horizontal, tr("Catalog"));

                                // Connect model to tree/table view
                                ui->Search_treeView_FilesFound->setModel(fileViewModel);
                                ui->Search_treeView_FilesFound->header()->setSectionResizeMode(QHeaderView::Interactive);
                                ui->Search_treeView_FilesFound->header()->resizeSection(0, 600); //Name
                                ui->Search_treeView_FilesFound->header()->resizeSection(1, 110); //Size
                                ui->Search_treeView_FilesFound->header()->resizeSection(2, 140); //Date
                                ui->Search_treeView_FilesFound->header()->resizeSection(3, 400); //Path
                                ui->Search_treeView_FilesFound->header()->resizeSection(4, 100); //Catalog

                                // Display count of files
                                ui->Search_label_FoundTitle->setText(tr("Duplicates found"));

                        }

                //Files found Statistics
                    //Reset from previous search
                        newSearch->filesFoundNumber = 0;
                        newSearch->filesFoundTotalSize = 0;
                        newSearch->filesFoundAverageSize = 0;
                        newSearch->filesFoundMinSize = 0;
                        newSearch->filesFoundMaxSize = 0;
                        newSearch->filesFoundMinDate = "";
                        newSearch->filesFoundMaxDate = "";

                    //Number of files found
                    newSearch->filesFoundNumber = fileViewModel->rowCount();
                    ui->Search_label_NumberResults->setText(QString::number(newSearch->filesFoundNumber));

                    //Total size of files found
                    qint64 sizeItem;
                    newSearch->filesFoundTotalSize = 0;
                    foreach (sizeItem, newSearch->fileSizes) {
                        newSearch->filesFoundTotalSize = newSearch->filesFoundTotalSize + sizeItem;
                    }
                    ui->Search_label_SizeResults->setText(QLocale().formattedDataSize(newSearch->filesFoundTotalSize));

                    //Other statistics, covering the case where no results are returned.
                    if (newSearch->filesFoundNumber !=0){
                        newSearch->filesFoundAverageSize = newSearch->filesFoundTotalSize / newSearch->filesFoundNumber;
                        QList<qint64> fileSizeList = newSearch->fileSizes;
                        std::sort(fileSizeList.begin(), fileSizeList.end());
                        newSearch->filesFoundMinSize = fileSizeList.first();
                        newSearch->filesFoundMaxSize = fileSizeList.last();

                        QList<QString> fileDateList = newSearch->fileDateTimes;
                        std::sort(fileDateList.begin(), fileDateList.end());
                        newSearch->filesFoundMinDate = fileDateList.first();
                        newSearch->filesFoundMaxDate = fileDateList.last();

                        ui->Search_pushButton_FileFoundMoreStatistics->setEnabled(true);
                    }

            //Save the search criteria to the search history
            newSearch->insertSearchHistoryToTable("defaultConnection");
            collection->saveSearchHistoryTableToFile();
            loadSearchHistoryTableToModel();

            //Stop animation
            QApplication::restoreOverrideCursor();

            //Enable Export
            ui->Search_pushButton_ProcessResults->setEnabled(true);
            ui->Search_comboBox_SelectProcess->setEnabled(true);
        }
        //----------------------------------------------------------------------
        void MainWindow::searchFilesInCatalog(Device *device)
        {//Run a search of files for the selected Catalog
            //Prepare Inputs including Regular Expression
            QFile catalogFile(device->catalog->sourcePath);

            QRegularExpressionMatch match;
            QRegularExpressionMatch foldermatch;

            //Define how to use the search text
            if(newSearch->selectedTextCriteria == tr("Exact Phrase"))
                newSearch->regexSearchtext=newSearch->searchText; //just search for the extact text entered including spaces, as one text string.
            else if(newSearch->selectedTextCriteria == tr("Begins With"))
                newSearch->regexSearchtext="(^"+newSearch->searchText+")";
            else if(newSearch->selectedTextCriteria == tr("Any Word"))
                newSearch->regexSearchtext=newSearch->searchText.replace(" ","|");
            else if(newSearch->selectedTextCriteria == tr("All Words")){
                QString searchTextToSplit = newSearch->searchText;
                QString groupRegEx = "";
                QRegularExpression lineSplitExp(" ");
                QStringList lineFieldList = searchTextToSplit.split(lineSplitExp);
                int numberOfSearchWords = lineFieldList.count();
                //Build regex group for one word
                for (int i=0; i<(numberOfSearchWords); i++){
                    groupRegEx = groupRegEx + "(?=.*" + lineFieldList[i] + ")";
                }
                newSearch->regexSearchtext = groupRegEx;
            }
            else {
                newSearch->regexSearchtext="";
            }

            newSearch->regexPattern = newSearch->regexSearchtext;

            //Prepare the regexFileType for file types
            if( newSearch->searchOnFileCriteria==true and newSearch->searchOnType ==true and newSearch->selectedFileType !="All"){
                //Get the list of file extension and join it into one string
                if(newSearch->selectedFileType =="Audio"){
                    newSearch->regexFileType = fileType_AudioS.join("|");
                }
                if(newSearch->selectedFileType =="Image"){
                    newSearch->regexFileType = fileType_ImageS.join("|");
                }
                if(newSearch->selectedFileType =="Text"){
                    newSearch->regexFileType = fileType_TextS.join("|");
                }
                if(newSearch->selectedFileType =="Video"){
                    newSearch->regexFileType = fileType_VideoS.join("|");
                }

                //Replace the *. by .* needed for regex
                newSearch->regexFileType = newSearch->regexFileType.replace("*.",".*");

                //Add the file type expression to the regex
                newSearch->regexPattern = newSearch->regexSearchtext  + "(" + newSearch->regexFileType + ")";

            }
            else
                newSearch->regexPattern = newSearch->regexSearchtext;

            //Add the words to exclude to the Regular Expression
            if ( newSearch->selectedSearchExclude !=""){

                //Prepare
                QString searchTextToSplit = newSearch->selectedSearchExclude;
                QString excludeGroupRegEx = "";
                QRegularExpression lineSplitExp(" ");
                QStringList lineFieldList = searchTextToSplit.split(lineSplitExp);
                int numberOfSearchWords = lineFieldList.count();

                //Build regex group to exclude all words
                //Genereate first part = first characters + the first word
                excludeGroupRegEx = "^(?!.*(" + lineFieldList[0];
                //Add more words
                for (int i=1; i<(numberOfSearchWords); i++){
                    excludeGroupRegEx = excludeGroupRegEx + "|" + lineFieldList[i];
                }
                //last part
                excludeGroupRegEx = excludeGroupRegEx + "))";

                //Add regex group to exclude to the global regexPattern
                newSearch->regexPattern = excludeGroupRegEx + newSearch->regexPattern;
            }

            //Initiate Regular Expression
            QRegularExpression regex(newSearch->regexPattern);
            if (newSearch->caseSensitive != true) {
                regex.setPatternOptions(QRegularExpression::CaseInsensitiveOption);
            }

            //Load the catalog file contents if not already loaded in memory
            QMutex tempMutex;
            bool tempStopRequested = false;
            if(collection->databaseMode=="Memory")
                device->catalog->loadCatalogFileListToTable("defaultConnection", tempMutex, tempStopRequested);

            //Search loop for all lines in the catalog file
            //Load the files of the Catalog
            QSqlQuery getFilesQuery(QSqlDatabase::database("defaultConnection"));
            QString getFilesQuerySQL = QLatin1String(R"(
                                        SELECT  file_name,
                                                file_folder_path,
                                                file_size,
                                                file_date_updated
                                        FROM  file
                                        WHERE file_catalog=:file_catalog
                                                    )");

            //Add matching size range
            if (newSearch->searchOnFileCriteria==true and newSearch->searchOnSize==true){
                getFilesQuerySQL = getFilesQuerySQL+" AND file_size>=:file_size_min ";
                getFilesQuerySQL = getFilesQuerySQL+" AND file_size<=:file_size_max ";
            }
            //Add matching date range
            if (newSearch->searchOnFileCriteria==true and newSearch->searchOnDate==true){
                getFilesQuerySQL = getFilesQuerySQL+" AND file_date_updated>=:file_date_updated_min ";
                getFilesQuerySQL = getFilesQuerySQL+" AND file_date_updated<=:file_date_updated_max ";
            }
            getFilesQuery.prepare(getFilesQuerySQL);
            getFilesQuery.bindValue(":file_catalog", device->name);
            getFilesQuery.bindValue(":file_size_min", newSearch->selectedMinimumSize * newSearch->sizeMultiplierMin);
            getFilesQuery.bindValue(":file_size_max", newSearch->selectedMaximumSize * newSearch->sizeMultiplierMax);
            getFilesQuery.bindValue(":file_date_updated_min", newSearch->selectedDateMin.toString("yyyy/MM/dd hh:mm:ss"));
            getFilesQuery.bindValue(":file_date_updated_max", newSearch->selectedDateMax.toString("yyyy/MM/dd hh:mm:ss"));
            getFilesQuery.exec();

            //File by file, test if the file is matching all search criteria
            //Loop principle1: stop further verification as soon as a criteria fails to match
            //Loop principle2: start with fastest criteria, finish with more complex ones (tag, file name)

            while(getFilesQuery.next()){

                QString   lineFileName     = getFilesQuery.value(0).toString();
                QString   lineFilePath     = getFilesQuery.value(1).toString();
                QString   lineFileFullPath = lineFilePath + "/" + lineFileName;
                bool      fileIsMatchingTag;

                //Continue to the next file if the current file is not matching the tags
                if (newSearch->searchOnFolderCriteria==true and newSearch->searchOnTags==true and newSearch->selectedTagName!=""){

                    fileIsMatchingTag = false;

                    //Set query to get a list of folder paths matching the selected tag
                    QSqlQuery queryTag(QSqlDatabase::database("defaultConnection"));
                    QString queryTagSQL = QLatin1String(R"(
                                                SELECT path
                                                FROM tag
                                                WHERE name=:name
                            )");
                    queryTag.prepare(queryTagSQL);
                    queryTag.bindValue(":name",newSearch->selectedTagName);
                    queryTag.exec();

                    //Test if the FilePath contains a path from the list of folders matching the selected tag name
                    // a slash "/" is added at the end of both values to ensure no result from a tag "AB" is returned when a tag "A" is selected
                    while(queryTag.next()){
                        if ( (lineFilePath+"/").contains( queryTag.value(0).toString()+"/" )==true){
                            fileIsMatchingTag = true;
                            break;
                        }
                    }

                    //If the file is not matching any of the paths, process the next file
                    if ( !fileIsMatchingTag==true ){
                        continue;}
                }

                //Finally, verify the text search criteria
                if (newSearch->searchOnFileName==true){
                    //Depends on the "Search in" criteria,
                    //Reduces the abosulte path to the required text string and matches the search text
                    if(newSearch->selectedSearchIn == tr("File names only"))
                    {
                        match = regex.match(lineFileName);
                    }
                    else if(newSearch->selectedSearchIn == tr("Folder path only"))
                    {

                        //Check that the folder name matches the search text
                        regex.setPattern(newSearch->regexSearchtext);
                        foldermatch = regex.match(lineFilePath);
                        //If it does, then check that the file matches the selected file type
                        if (foldermatch.hasMatch() and newSearch->searchOnType==true){
                            regex.setPattern(newSearch->regexFileType);
                            match = regex.match(lineFileName);
                        }
                        else
                            match = foldermatch; //selectedSearchIn == tr("Files and Folder paths")
                    }
                    else {
                        match = regex.match(lineFileFullPath);
                    }
                    //If the file is matching the criteria, add it and its catalog to the search results
                    if (match.hasMatch()){
                        newSearch->filesFoundList << lineFilePath;
                        newSearch->deviceFoundIDList.insert(0,QString::number(device->ID));

                        //Populate result lists
                        newSearch->fileNames.append(lineFileName);
                        newSearch->filePaths.append(lineFilePath);
                        newSearch->fileSizes.append(getFilesQuery.value(2).toLongLong());
                        newSearch->fileDateTimes.append(getFilesQuery.value(3).toString());
                        newSearch->fileCatalogs.append(device->name);
                    }
                }
                else{
                    //verify file matches the selected file type
                    if (newSearch->searchOnType==true){
                        regex.setPattern(newSearch->regexFileType);
                    }
                    match = regex.match(lineFilePath);
                    if (!match.hasMatch()){
                        continue;
                    }

                    //Add the file and its catalog to the results, excluding blank lines
                    if (lineFilePath !=""){
                        newSearch->filesFoundList << lineFilePath;
                        newSearch->deviceFoundIDList.insert(0, QString::number(device->ID));

                        //Populate result lists
                        newSearch->fileNames.append(lineFileName);
                        newSearch->filePaths.append(lineFilePath);
                        newSearch->fileSizes.append(getFilesQuery.value(2).toLongLong());
                        newSearch->fileDateTimes.append(getFilesQuery.value(3).toString());
                        newSearch->fileCatalogs.append(device->name);
                    }
                }
            }
        }
        //----------------------------------------------------------------------

        void MainWindow::searchFilesInDirectory(const QString &sourceDirectory)
        {//Run a search of files for the selected Directory
            //Define how to use the search text //COMMON to searchFilesInCatalog
                if(newSearch->selectedTextCriteria == tr("Exact Phrase"))
                    newSearch->regexSearchtext=newSearch->searchText; //just search for the extact text entered including spaces, as one text string.
                else if(newSearch->selectedTextCriteria == tr("Begins With"))
                    newSearch->regexSearchtext="(^"+newSearch->searchText+")";
                else if(newSearch->selectedTextCriteria == tr("Any Word"))
                    newSearch->regexSearchtext=newSearch->searchText.replace(" ","|");
                else if(newSearch->selectedTextCriteria == tr("All Words")){
                    QString searchTextToSplit = newSearch->searchText;
                    QString groupRegEx = "";
                    QRegularExpression lineSplitExp(" ");
                    QStringList lineFieldList = searchTextToSplit.split(lineSplitExp);
                    int numberOfSearchWords = lineFieldList.count();
                    //Build regex group for one word
                    for (int i=0; i<(numberOfSearchWords); i++){
                        groupRegEx = groupRegEx + "(?=.*" + lineFieldList[i] + ")";
                    }
                    newSearch->regexSearchtext = groupRegEx;
                }
                else {
                    newSearch->regexSearchtext="";
                     }

                newSearch->regexPattern = newSearch->regexSearchtext;

            //Prepare the regexFileType for file types //COMMON to searchFilesInCatalog
            if ( newSearch->searchOnFileCriteria==true and newSearch->selectedFileType !=tr("All")){
                //Get the list of file extension and join it into one string
                if(newSearch->selectedFileType ==tr("Audio")){
                            newSearch->regexFileType = fileType_AudioS.join("|");
                }
                if(newSearch->selectedFileType ==tr("Image")){
                            newSearch->regexFileType = fileType_ImageS.join("|");
                }
                if(newSearch->selectedFileType ==tr("Text")){
                            newSearch->regexFileType = fileType_TextS.join("|");
                }
                if(newSearch->selectedFileType ==tr("Video")){
                            newSearch->regexFileType = fileType_VideoS.join("|");
                }

                //Replace the *. by .* needed for regex
                newSearch->regexFileType = newSearch->regexFileType.replace("*.",".*");

                //Add the file type expression to the regex
                newSearch->regexPattern = newSearch->regexSearchtext  + "(" + newSearch->regexFileType + ")";
             }

            //Add the words to exclude to the regex //COMMON to searchFilesInCatalog

            if ( newSearch->selectedSearchExclude !=""){

                //Prepare
                QString searchTextToSplit = newSearch->selectedSearchExclude;
                QString excludeGroupRegEx = "";
                QRegularExpression lineSplitExp(" ");
                QStringList lineFieldList = searchTextToSplit.split(lineSplitExp);
                int numberOfSearchWords = lineFieldList.count();

                //Build regex group to exclude all words
                    //Genereate first part = first characters + the first word
                    excludeGroupRegEx = "^(?!.*(" + lineFieldList[0];
                    //Add more words
                    for (int i=1; i<(numberOfSearchWords); i++){
                        excludeGroupRegEx = excludeGroupRegEx + "|" + lineFieldList[i];
                    }
                    //Last part
                    excludeGroupRegEx = excludeGroupRegEx + "))";

                //Add regex group to exclude to the global regexPattern
                newSearch->regexPattern = excludeGroupRegEx + newSearch->regexPattern;
            }

            QRegularExpression regex(newSearch->regexPattern, QRegularExpression::CaseInsensitiveOption);

            //Filetypes
                    //Get the file type for the catalog
                    QStringList fileTypes;

            //Scan directory and create a list of files
                QString line;
                QString reducedLine;


            QDirIterator iterator(sourceDirectory, fileTypes, QDir::Files|QDir::Hidden, QDirIterator::Subdirectories);
            while (iterator.hasNext()){

                 //Get file information  (absolute path, size, datetime)
                QString filePath = iterator.next();
                QFileInfo fileInfo(filePath);
                QDateTime fileDate = fileInfo.lastModified();

            line = fileInfo.absoluteFilePath() + "\t" + QString::number(fileInfo.size()) + "\t" + fileDate.toString("yyyy/MM/dd hh:mm:ss");

            //COMMON to searchFilesInCatalog
                        QRegularExpressionMatch match;
                        QRegularExpressionMatch foldermatch;
                        //QRegularExpressionMatch matchFileType;

                        //Split the line text with tabulations into a list
                        QRegularExpression     lineSplitExp("\t");
                        QStringList lineFieldList  = line.split(lineSplitExp);
                        int         fieldListCount = lineFieldList.count();

                        //Get the file absolute path from this list
                        QString     lineFilePath   = lineFieldList[0];

                        //Get the FileSize from the list if available
                        qint64      lineFileSize;
                        if (fieldListCount == 3){lineFileSize = lineFieldList[1].toLongLong();}
                        else lineFileSize = 0;

                        //Get the File DateTime from the list if available
                        QDateTime   lineFileDateTime;
                        if (fieldListCount == 3){lineFileDateTime = QDateTime::fromString(lineFieldList[2],"yyyy/MM/dd hh:mm:ss");}
                        else lineFileDateTime = QDateTime::fromString("0001/01/01 00:00:00","yyyy/MM/dd hh:mm:ss");

                    //Exclude catalog metadata lines which are starting with the character <
                         if (lineFilePath.left(1)=="<"){continue;}

                    //Continue if the file is matching the size range
                        if (newSearch->searchOnSize==true){
                            if ( !(     lineFileSize >= newSearch->selectedMinimumSize * newSearch->sizeMultiplierMin
                                    and lineFileSize <= newSearch->selectedMaximumSize * newSearch->sizeMultiplierMax) ){
                                        continue;}
                        }

                    //Continue if the file is matching the date range
                        if (newSearch->searchOnDate==true){
                            if ( !(     lineFileDateTime >= newSearch->selectedDateMin
                                    and lineFileDateTime <= newSearch->selectedDateMax ) ){
                                        continue;}
                        }

                    //Continue if the file is matching the tags
                        if (newSearch->searchOnTags==true){

                            bool fileIsMatchingTag = false;

                            //Set query to get a list of folder paths matching the selected tag
                            QSqlQuery queryTag(QSqlDatabase::database("defaultConnection"));
                            QString queryTagSQL = QLatin1String(R"(
                                                SELECT path
                                                FROM tag
                                                WHERE name=:name
                            )");
                            queryTag.prepare(queryTagSQL);
                            queryTag.bindValue(":name",newSearch->selectedTagName);
                            queryTag.exec();

                            //Test if the FilePath contains a path from the list of folders matching the selected tag name
                            while(queryTag.next()){
                                if ( lineFilePath.contains(queryTag.value(0).toString())==true){
                                    fileIsMatchingTag = true;
                                    break;
                                }
                                //else tagIsMatching==false
                            }

                            //If the file is not matching any of the paths, process the next file
                            if ( !fileIsMatchingTag==true ){
                                    continue;}
                        }

                    //Finally, verify the text search criteria
                    if (newSearch->searchOnFileName==true){
                        //Depending on the "Search in" criteria,
                        //reduce the abosulte path to the reaquired text string and match the search text
                        if(newSearch->selectedSearchIn == tr("File names only"))
                        {
                            // Extract the file name from the lineFilePath
                            QFileInfo file(lineFilePath);
                            reducedLine = file.fileName();

                            match = regex.match(reducedLine);
                        }
                        else if(newSearch->selectedSearchIn == tr("Folder path only"))
                        {
                            //Keep only the folder name, so all characters left of the last occurence of / in the path.
                            reducedLine = lineFilePath.left(lineFilePath.lastIndexOf("/"));

                            //Check the fodler name matches the search text
                            regex.setPattern(newSearch->regexSearchtext);

                            foldermatch = regex.match(reducedLine);

                            //if it does, then check that the file matches the selected file type
                            if (foldermatch.hasMatch()){
                                regex.setPattern(newSearch->regexFileType);

                                match = regex.match(lineFilePath);
                            }
                        }
                        else {
                            match = regex.match(lineFilePath);
                        }

                        //If the file is matching the criteria, add it and its catalog to the search results
                        //COMMON to searchFilesInCatalog
                        if (match.hasMatch()){

                            newSearch->filesFoundList << lineFilePath;

                            //COMMON to searchFilesInCatalog
                            //Retrieve other file info
                            QFileInfo file(lineFilePath);

                            // Get the fileDateTime from the list if available
                            QString lineFileDatetime;
                            if (fieldListCount == 3){
                                    lineFileDatetime = lineFieldList[2];}
                            else lineFileDatetime = "";

                            //Populate result lists
                            newSearch->fileNames.append(file.fileName());
                            newSearch->filePaths.append(file.path());
                            newSearch->fileSizes.append(lineFileSize);
                            newSearch->fileDateTimes.append(lineFileDatetime);
                            newSearch->fileCatalogs.append(sourceDirectory);
                        }
                    }
                    else{

                        //Add the file and its catalog to the results, excluding blank lines
                        if (lineFilePath !=""){
                            newSearch->filesFoundList << lineFilePath;
                            newSearch->deviceFoundIDList.insert(0, sourceDirectory);

                            //Retrieve other file info
                            QFileInfo file(lineFilePath);

                            // Get the fileDateTime from the list if available
                            QString lineFileDatetime;
                            if (fieldListCount == 3){
                                    lineFileDatetime = lineFieldList[2];}
                            else lineFileDatetime = "";

                            //Populate result lists
                            newSearch->fileNames.append(file.fileName());
                            newSearch->filePaths.append(file.path());
                            newSearch->fileSizes.append(lineFileSize);
                            newSearch->fileDateTimes.append(lineFileDatetime);
                            newSearch->fileCatalogs.append(sourceDirectory);
                        }
                    }
            }
        }
        //----------------------------------------------------------------------

        //UI methods
        void MainWindow::initiateSearchFields()
        {
            //Add filetype English value additionally to the displayed/translated value
            ui->Search_comboBox_FileType->setItemData(0, "All",   Qt::UserRole);
            ui->Search_comboBox_FileType->setItemData(1, "Audio", Qt::UserRole);
            ui->Search_comboBox_FileType->setItemData(2, "Image", Qt::UserRole);
            ui->Search_comboBox_FileType->setItemData(3, "Text",  Qt::UserRole);
            ui->Search_comboBox_FileType->setItemData(4, "Video", Qt::UserRole);

            ui->Catalogs_comboBox_FileType->setItemData(0, "All",   Qt::UserRole);
            ui->Catalogs_comboBox_FileType->setItemData(1, "Audio", Qt::UserRole);
            ui->Catalogs_comboBox_FileType->setItemData(2, "Image", Qt::UserRole);
            ui->Catalogs_comboBox_FileType->setItemData(3, "Text",  Qt::UserRole);
            ui->Catalogs_comboBox_FileType->setItemData(4, "Video", Qt::UserRole);

            ui->Search_comboBox_SelectProcess->setItemData(0, "Select...",   Qt::UserRole);
            ui->Search_comboBox_SelectProcess->setItemData(1, "Export Results", Qt::UserRole);
            ui->Search_comboBox_SelectProcess->setItemData(2, "Rename (KRename)", Qt::UserRole);

            //Prepare list of size units for the Catalog selection combobox
            //The first line is the one displayed by default
            ui->Search_comboBox_MinSizeUnit->addItem(tr("TiB"));
            ui->Search_comboBox_MinSizeUnit->addItem(tr("GiB"));
            ui->Search_comboBox_MinSizeUnit->addItem(tr("MiB"));
            ui->Search_comboBox_MinSizeUnit->addItem(tr("KiB"));
            ui->Search_comboBox_MinSizeUnit->addItem(tr("Bytes"));

            ui->Search_comboBox_MaxSizeUnit->addItem(tr("TiB"));
            ui->Search_comboBox_MaxSizeUnit->addItem(tr("GiB"));
            ui->Search_comboBox_MaxSizeUnit->addItem(tr("MiB"));
            ui->Search_comboBox_MaxSizeUnit->addItem(tr("KiB"));
            ui->Search_comboBox_MaxSizeUnit->addItem(tr("Bytes"));

            //Load last search values (from settings file)
            if (newSearch->selectedMaximumSize ==0)
                newSearch->selectedMaximumSize = 1000;

            //Populate Differences combo boxes with selected catalogs
            refreshDifferencesCatalogSelection();
        }
        //----------------------------------------------------------------------
        void MainWindow::resetToDefaultSearchCriteria()
        {//Reset criteria to default search values
            //File name
            ui->Search_checkBox_FileName->setChecked(true);
            ui->Search_lineEdit_SearchText->setText("");
            ui->Search_comboBox_TextCriteria->setCurrentText(tr("All Words"));
            ui->Search_comboBox_SearchIn->setCurrentText(tr("File names only"));
            ui->Search_checkBox_CaseSensitive->setChecked(false);
            ui->Search_lineEdit_Exclude->setText(tr(""));

            //File criteria
            ui->Search_checkBox_FileCriteria->setChecked(false);
            ui->Search_checkBox_Size->setChecked(false);
            ui->Search_spinBox_MinimumSize->setValue(0);
            ui->Search_comboBox_MinSizeUnit->setCurrentText(tr("Bytes"));
            ui->Search_spinBox_MaximumSize->setValue(1000);
            ui->Search_comboBox_MaxSizeUnit->setCurrentText(tr("GiB"));
            ui->Search_checkBox_Date->setChecked(false);
            ui->Search_dateTimeEdit_Min->setDateTime(QDateTime::fromString("1970-01-01 00:00:00","yyyy-MM-dd hh:mm:ss"));
            ui->Search_dateTimeEdit_Max->setDateTime(QDateTime::fromString("2030-01-01 00:00:00","yyyy-MM-dd hh:mm:ss"));
            ui->Search_comboBox_FileType->setCurrentText(tr("All"));
            ui->Search_checkBox_Duplicates->setChecked(false);
            ui->Search_checkBox_DuplicatesName->setChecked(false);
            ui->Search_checkBox_DuplicatesSize->setChecked(false);
            ui->Search_checkBox_DuplicatesDateModified->setChecked(false);
            ui->Search_checkBox_Differences->setChecked(false);
            ui->Search_checkBox_DifferencesName->setChecked(false);
            ui->Search_checkBox_DifferencesSize->setChecked(false);
            ui->Search_checkBox_DifferencesDateModified->setChecked(false);

            //Folder criteria
            ui->Search_checkBox_FolderCriteria->setChecked(false);
            ui->Search_checkBox_ShowFolders->setChecked(false);
            ui->Search_checkBox_Tags->setChecked(false);
            ui->Search_comboBox_Tags->setCurrentText("");

            //Results
            ui->Search_label_NumberResults->setText("");
            ui->Search_label_SizeResults->setText("");
            ui->Search_pushButton_FileFoundMoreStatistics->setDisabled(true);

            //Clear catalog and file results (load an empty model)
            clearSearchResults();

            //Clear results and disable export
            newSearch->filesFoundList.clear();
            ui->Search_pushButton_ProcessResults->setEnabled(false);
            ui->Search_comboBox_SelectProcess->setEnabled(false);
        }
        //----------------------------------------------------------------------
        void MainWindow::loadSearchCriteria(Search *search)
        {//Set search values

            //Selections
            ui->Filters_lineEdit_SeletedDirectory->setText(search->connectedDirectory);
            ui->Filters_label_DisplayStorage->setText(search->selectedStorage);
            ui->Filters_label_DisplayCatalog->setText(search->selectedCatalog);

                //File name
                ui->Search_checkBox_FileCriteria->setChecked(search->searchOnFileCriteria);
                ui->Search_checkBox_FileName->setChecked(search->searchOnFileName);
                ui->Search_lineEdit_SearchText->setText(search->searchText);
                ui->Search_comboBox_TextCriteria->setCurrentText(search->selectedTextCriteria);
                ui->Search_comboBox_SearchIn->setCurrentText(search->selectedSearchIn);
                ui->Search_checkBox_CaseSensitive->setChecked(search->caseSensitive);
                ui->Search_lineEdit_Exclude->setText(search->selectedSearchExclude);

                //File criteria
                ui->Search_checkBox_Size->setChecked(search->searchOnSize);
                ui->Search_spinBox_MinimumSize->setValue(search->selectedMinimumSize);
                ui->Search_comboBox_MinSizeUnit->setCurrentText(search->selectedMinSizeUnit);
                ui->Search_spinBox_MaximumSize->setValue(search->selectedMaximumSize);
                ui->Search_comboBox_MaxSizeUnit->setCurrentText(search->selectedMaxSizeUnit);
                ui->Search_checkBox_Type->setChecked(search->searchOnType);
                ui->Search_comboBox_FileType->setCurrentText(tr(search->selectedFileType.toUtf8()));
                ui->Search_checkBox_Date->setChecked(search->searchOnDate);
                ui->Search_dateTimeEdit_Min->setDateTime(search->selectedDateMin);
                ui->Search_dateTimeEdit_Max->setDateTime(search->selectedDateMax);
                ui->Search_checkBox_Duplicates->setChecked(search->searchOnDuplicates);
                ui->Search_checkBox_DuplicatesName->setChecked(search->searchDuplicatesOnName);
                ui->Search_checkBox_DuplicatesSize->setChecked(search->searchDuplicatesOnSize);
                ui->Search_checkBox_DuplicatesDateModified->setChecked(search->searchDuplicatesOnDate);
                ui->Search_widget_DifferencesCatalogs->setHidden(true);
                ui->Search_checkBox_Differences->setChecked(search->searchOnDifferences);
                ui->Search_checkBox_DifferencesName->setChecked(search->differencesOnName);
                ui->Search_checkBox_DifferencesSize->setChecked(search->differencesOnSize);
                ui->Search_checkBox_DifferencesDateModified->setChecked(search->differencesOnDate);
                ui->Search_comboBox_DifferencesCatalog1->setCurrentText(search->differencesCatalog1);
                ui->Search_comboBox_DifferencesCatalog2->setCurrentText(search->differencesCatalog2);

                //Folder criteria
                ui->Search_checkBox_FolderCriteria->setChecked(search->searchOnFolderCriteria);
                ui->Search_checkBox_ShowFolders->setChecked(search->showFoldersOnly);
                ui->Search_checkBox_Tags->setChecked(search->searchOnTags);
                ui->Search_comboBox_Tags->setCurrentText(search->selectedTagName);

                //Clear previous results (load an empty model)
                clearSearchResults();

        }
        //----------------------------------------------------------------------
        void MainWindow::getSearchCriteria()
        {//Get all new criteria

                //Clear the temporary search
                newSearch = new Search;

                //searchDateTime;
                newSearch->searchOnFileName         = ui->Search_checkBox_FileName->isChecked();
                newSearch->searchText               = ui->Search_lineEdit_SearchText->text();
                newSearch->selectedTextCriteria     = ui->Search_comboBox_TextCriteria->currentText();
                newSearch->selectedSearchIn         = ui->Search_comboBox_SearchIn->currentText();
                newSearch->caseSensitive            = ui->Search_checkBox_CaseSensitive->isChecked();
                newSearch->selectedSearchExclude    = ui->Search_lineEdit_Exclude->text();

                newSearch->searchOnFileCriteria     = ui->Search_checkBox_FileCriteria->isChecked();
                newSearch->searchOnSize             = ui->Search_checkBox_Size->isChecked();
                newSearch->selectedMinimumSize      = ui->Search_spinBox_MinimumSize->value();
                newSearch->selectedMaximumSize      = ui->Search_spinBox_MaximumSize->value();
                newSearch->selectedMinSizeUnit      = ui->Search_comboBox_MinSizeUnit->currentText();
                newSearch->selectedMaxSizeUnit      = ui->Search_comboBox_MaxSizeUnit->currentText();
                newSearch->setMultipliers();
                newSearch->searchOnType             = ui->Search_checkBox_Type->isChecked();
                newSearch->selectedFileType         = ui->Search_comboBox_FileType->itemData(ui->Search_comboBox_FileType->currentIndex(),Qt::UserRole).toString();
                newSearch->searchOnDate             = ui->Search_checkBox_Date->isChecked();
                newSearch->selectedDateMin          = ui->Search_dateTimeEdit_Min->dateTime();
                newSearch->selectedDateMax          = ui->Search_dateTimeEdit_Max->dateTime();
                newSearch->searchOnDuplicates       = ui->Search_checkBox_Duplicates->isChecked();
                newSearch->searchDuplicatesOnName   = ui->Search_checkBox_DuplicatesName->isChecked();
                newSearch->searchDuplicatesOnSize   = ui->Search_checkBox_DuplicatesSize->isChecked();
                newSearch->searchDuplicatesOnDate   = ui->Search_checkBox_DuplicatesDateModified->isChecked();
                newSearch->searchOnDifferences      = ui->Search_checkBox_Differences->isChecked();
                newSearch->differencesOnName        = ui->Search_checkBox_DifferencesName->checkState();
                newSearch->differencesOnSize        = ui->Search_checkBox_DifferencesSize->checkState();
                newSearch->differencesOnDate        = ui->Search_checkBox_DifferencesDateModified->checkState();
                newSearch->differencesCatalog1      = ui->Search_comboBox_DifferencesCatalog1->currentText();
                newSearch->differencesCatalog2      = ui->Search_comboBox_DifferencesCatalog2->currentText();
                newSearch->differencesCatalogs  << newSearch->differencesCatalog1 << newSearch->differencesCatalog2;

                newSearch->searchOnFolderCriteria   = ui->Search_checkBox_FolderCriteria->isChecked();
                newSearch->showFoldersOnly          = ui->Search_checkBox_ShowFolders->isChecked();
                newSearch->searchOnTags             = ui->Search_checkBox_Tags->isChecked();
                newSearch->selectedTagName          = ui->Search_comboBox_Tags->currentText();

                newSearch->selectedStorage          = ui->Filters_label_DisplayStorage->text();
                newSearch->selectedCatalog          = ui->Filters_label_DisplayCatalog->text();
                newSearch->searchInCatalogsChecked  = ui->Filters_checkBox_SearchInCatalogs->isChecked();
                newSearch->searchInConnectedChecked = ui->Filters_checkBox_SearchInConnectedDrives->isChecked();
                newSearch->connectedDirectory       = ui->Filters_lineEdit_SeletedDirectory->text();

        }
        //----------------------------------------------------------------------
        void MainWindow::refreshDifferencesCatalogSelection(){
            ui->Search_comboBox_DifferencesCatalog1->clear();
            ui->Search_comboBox_DifferencesCatalog2->clear();

            Device loopDevice;
            foreach(int ID, selectedDevice->deviceIDList)
            {
                loopDevice.ID = ID;
                loopDevice.loadDevice("defaultConnection");
                if(loopDevice.type == "Catalog"){
                    ui->Search_comboBox_DifferencesCatalog1->addItem(loopDevice.name,loopDevice.ID);
                    ui->Search_comboBox_DifferencesCatalog2->addItem(loopDevice.name,loopDevice.ID);
                }
            }
        }
        //----------------------------------------------------------------------
        void MainWindow::batchProcessSearchResults()
        {//Process all results according to user's choice

            //user process choice
            QString selectedProcess = ui->Search_comboBox_SelectProcess->currentText();

            //Generate list of full file path (directory path + file name)
            QStringList resultsFilesList;
            for (int i = 0; i < newSearch->fileNames.size(); ++i)
            {
                QString fileFullPath = newSearch->filePaths[i] + "/" + newSearch->fileNames[i];
                resultsFilesList << fileFullPath;
            }

            //No selection
            if(selectedProcess==tr("Select...")){
                        QMessageBox::information(this,"Katalog",tr("Select first a process to be applied to all results below."));
                        return;
            }

            //Export Results
            else if(selectedProcess==tr("Export Results")){
                QString exportFileName = exportSearchResults();
                QFileInfo fileInfo(exportFileName);
                QString  fileSuffix = fileInfo.suffix();
                if (exportFileName !=""){
                    QMessageBox msgBox;
                    msgBox.setWindowTitle("Katalog");
                    msgBox.setTextFormat(Qt::RichText);
                    if(collection->databaseMode=="Memory" or fileSuffix=="csv"){
                        exportFileName = "file://" + exportFileName;
                        msgBox.setText(tr("Results exported to the collection folder:")
                                       +"<br/><a href='"+exportFileName+"'>"+exportFileName+"</a>");
                    }
                    else{
                        msgBox.setText(tr("Results exported a new Catalog:")
                                       +"<br/>"+exportFileName);
                    }
                    msgBox.setIcon(QMessageBox::Information);
                    msgBox.exec();
                }
            }

            //KRename
            else if(selectedProcess==tr("Rename (KRename)")){
                        QProcess process;
                        process.startDetached("krename",resultsFilesList);
            }

            //Move to trash
            else if(selectedProcess==tr("Move to Trash")){

                if(newSearch->showFoldersOnly==false){
                    QString trashPath;
                    QString fileFullPath;
                    qint64 movedFiles = 0;
                    if (QMessageBox::warning(this,
                                             tr("Confirmation"),
                                             "<span style='color:orange;font-weight: bold;'>"+tr("MOVE")+"</span><br/>"
                                                 +tr("Move all %1 files (%2) from these results to trash?").arg(QString::number(newSearch->filesFoundNumber), QLocale().formattedDataSize(newSearch->filesFoundTotalSize)),
                                             QMessageBox::Yes|QMessageBox::Cancel)
                        == QMessageBox::Yes) {
                            for (int i = 0; i < newSearch->fileNames.size(); ++i)
                            {
                            fileFullPath = moveFileToTrash(newSearch->filePaths[i] + "/" + newSearch->fileNames[i]);
                            trashPath = moveFileToTrash(fileFullPath);
                            if(trashPath==""){
                         QMessageBox::information(this,"Katalog",tr("Problem moving file: ")
                                                                       +"<br/>file-<a href='"+fileFullPath+"'>"+fileFullPath+"</a>");
                            }
                            else
                         movedFiles+=1;
                            }
                            QMessageBox::information(this,"Katalog",tr("%1 files were moved to trash, out of %2 files from the results.").arg(QString::number(movedFiles), QString::number(newSearch->filesFoundNumber)));
                    }

                    //Reset process selection to reduce risk of running it by mistake
                    ui->Search_comboBox_SelectProcess->setCurrentIndex(0);
                }
                else{
                    QMessageBox msgBox;
                    msgBox.setWindowTitle("Katalog");
                    msgBox.setText(tr("Moving a list of folders to Trash is not available."));
                    msgBox.setIcon(QMessageBox::Information);
                    msgBox.exec();
                }
            }

            //Delete
            else if(selectedProcess==tr("Delete")){

                if(newSearch->showFoldersOnly==false){
                    qint64 deletedFiles = 0;
                    QString result;

                    if (QMessageBox::critical(this,
                                              tr("Confirmation"),
                                              "<span style='color:red;font-weight: bold;'>"+tr("DELETE")+"</span><br/>"
                                              +tr("Delete permanently all %1 files (%2) from these results?").arg(QString::number(newSearch->filesFoundNumber), QLocale().formattedDataSize(newSearch->filesFoundTotalSize)),
                                              QMessageBox::Yes|QMessageBox::Cancel)
                        == QMessageBox::Yes) {
                        for (int i = 0; i < newSearch->fileNames.size(); ++i)
                        {
                            result = deleteFile(newSearch->filePaths[i] + "/" + newSearch->fileNames[i]);
                            if (result!=""){
                                deletedFiles +=1;
                            }
                            result="";
                        }
                        QMessageBox::information(this,"Katalog",tr("%1 files were deleted, out of %2 files from the results.").arg(QString::number(deletedFiles), QString::number(newSearch->filesFoundNumber)));
                    }

                    //Reset process selection to reduce risk of running it by mistake
                    ui->Search_comboBox_SelectProcess->setCurrentIndex(0);
                }
                else{
                    QMessageBox msgBox;
                    msgBox.setWindowTitle("Katalog");
                    msgBox.setText(tr("Deleting a list of folders is not available."));
                    msgBox.setIcon(QMessageBox::Information);
                    msgBox.exec();
                }
            }
        }
        //----------------------------------------------------------------------
        QString MainWindow::exportSearchResults()
        {//Export search results to file, returns fullFileName
            QString fileExtension;
            QStringList catalogMetadata;
            QString fullFileName;

            int result = QMessageBox::question(this,"Katalog",
                      tr("Create a catalog from these results?"
                         "<br/>- Yes: create a <b>Catalog</b> to store the results and use it to refine your search,"
                         "<br/>- No:  export results to a <b>csv file</b>."),
                                              QMessageBox::Yes|QMessageBox::No|QMessageBox::Cancel);

            if ( result !=QMessageBox::Cancel){

                //Prepare export file name
                QDateTime now = QDateTime::currentDateTime();
                QString timestamp = now.toString(QLatin1String("yyyyMMdd-hhmmss"));
                QString fileNameWithoutExtension = tr("search_results") + "_" + timestamp;
                Device *newDevice = new Device();

                if ( result ==QMessageBox::Yes){

                    fileExtension="idx";

                    //Verify if a device name "Search Results" exist or create one as parent for search results
                    Device searchResultsHolder;
                    searchResultsHolder.name = tr("Search Results");
                    searchResultsHolder.getIDFromDeviceName();

                    if(searchResultsHolder.ID>0)
                        newDevice->parentID = searchResultsHolder.ID;
                    else{
                        searchResultsHolder.generateDeviceID();
                        searchResultsHolder.type = "Virtual";
                        searchResultsHolder.groupID = 1;
                        searchResultsHolder.parentID = 0;
                        searchResultsHolder.insertDevice();
                        newDevice->parentID = searchResultsHolder.ID;
                    }

                    //Add Device entry
                    newDevice->generateDeviceID();
                    newDevice->type = "Catalog";
                    newDevice->name = fileNameWithoutExtension;

                    //Continue populating values

                    newDevice->parentID = searchResultsHolder.ID;
                    newDevice->catalog->generateID();
                    newDevice->externalID = newDevice->catalog->ID;
                    newDevice->groupID = 1;
                    newDevice->path = newSearch->searchDateTime;
                    newDevice->insertDevice();

                    //Get inputs and set values of the newCatalog
                    newDevice->catalog->sourcePath = newSearch->searchDateTime;//passing a date instead oo a path, as there is no path for a given search that can be multi-catalog
                    newDevice->catalog->appVersion = currentVersion;

                    //Save new catalog
                    newDevice->catalog->insertCatalog();
                    collection->saveDeviceTableToFile();

                    catalogMetadata.prepend("<catalogID>" + QString::number(newDevice->catalog->ID));
                    catalogMetadata.prepend("<catalogAppVersion>");
                    catalogMetadata.prepend("<catalogIncludeMetadata>");
                    catalogMetadata.prepend("<catalogIsFullDevice>");
                    catalogMetadata.prepend("<catalogIncludeSymblinks>");
                    catalogMetadata.prepend("<catalogStorage>EXPORT");
                    catalogMetadata.prepend("<catalogFileType>EXPORT");
                    catalogMetadata.prepend("<catalogIncludeHidden>false");
                    catalogMetadata.prepend("<catalogTotalFileSize>0");
                    catalogMetadata.prepend("<catalogFileCount>0");
                    catalogMetadata.prepend("<catalogSourcePath>EXPORT");

                    selectedDevice->ID = newDevice->ID;
                    selectedDevice->loadDevice("defaultConnection");
                    QSettings settings(collection->settingsFilePath, QSettings:: IniFormat);
                    settings.setValue("Selection/SelectedDeviceID",   selectedDevice->ID);
                    filterFromSelectedDevice();
                }
                else if( result ==QMessageBox::No){
                    fileExtension="csv";
                }

                //Complete file name based on databaseMode
                if(collection->databaseMode=="Memory"){//Use collection folder
                    QString fileNameWithExtension = fileNameWithoutExtension + "." + fileExtension;
                    fullFileName = collection->folder + "/" + fileNameWithExtension;
                }
                else if(collection->databaseMode=="File"){//Use .db file folder
                    if(fileExtension=="csv"){//for csv
                        QFileInfo fileInfo(collection->databaseFilePath);
                        QString fileNameWithExtension = fileNameWithoutExtension + "." + fileExtension;
                        fullFileName = fileInfo.absolutePath() + "/" + fileNameWithExtension;
                    }
                    else //for catalog
                        fullFileName = newDevice->name;

                }
                else if(collection->databaseMode=="Host"){//Use user's home folder
                    QStringList standardsPaths = QStandardPaths::standardLocations(QStandardPaths::HomeLocation);
                    QString homePath = standardsPaths[0];
                    QString fileNameWithExtension = fileNameWithoutExtension + "." + fileExtension;
                    fullFileName = homePath + "/" + fileNameWithExtension;
                }
                selectedDevice->catalog->filePath = fullFileName;
                selectedDevice->catalog->name = selectedDevice->name;
                QFile exportFile(fullFileName);

                //Export search results to file
                if (exportFile.open(QFile::WriteOnly | QFile::Text)) {

                    QTextStream stream(&exportFile);

                    for (int i = 0; i < catalogMetadata.size(); ++i)
                    {
                                    stream << catalogMetadata[i] << '\n';
                    }
                    for (int i = 0; i < newSearch->fileNames.size(); ++i)
                    {
                                    QString line = newSearch->filePaths[i] + "/" + newSearch->fileNames[i] + "\t"
                                                   + QString::number(newSearch->fileSizes[i]) + "\t"
                                                   + newSearch->fileDateTimes[i] + "\t"
                                                   + newSearch->fileCatalogs[i];
                                    stream << line << '\n';
                    }
                }
                exportFile.close();

                //Load files
                QDateTime emptyDateTime = *new QDateTime;
                selectedDevice->catalog->setDateLoaded(emptyDateTime, "defaultConnection");
                selectedDevice->catalog->setDateUpdated(QDateTime::currentDateTime().addMSecs(100));
                QMutex tempMutex;
                bool tempStopRequested = false;
                if(collection->databaseMode=="Memory")
                    selectedDevice->catalog->loadCatalogFileListToTable("defaultConnection", tempMutex, tempStopRequested);
                //Refresh catalogs
                loadCollection();
                loadStorageList();

                //Select new catalog with results
                ui->Filters_label_DisplayCatalog->setText(fileNameWithoutExtension);
            }

            return fullFileName;
        }
        //--------------------------------------------------------------------------
        void MainWindow::loadSearchHistoryTableToModel()
        {
            QSqlQuery querySearchHistory(QSqlDatabase::database("defaultConnection"));
            QString querySearchHistorySQL = QLatin1String(R"(
                                                SELECT
                                                    date_time,
                                                    text_checked,
                                                    text_phrase,
                                                    text_criteria,
                                                    text_search_in,
                                                    case_sensitive,
                                                    text_exclude,
                                                    file_criteria_checked,
                                                    file_type_checked,
                                                    file_type,
                                                    file_size_checked,
                                                    file_size_min,
                                                    file_size_min_unit,
                                                    file_size_max,
                                                    file_size_max_unit,
                                                    date_modified_checked,
                                                    date_modified_min,
                                                    date_modified_max,
                                                    duplicates_checked,
                                                    duplicates_name,
                                                    duplicates_size,
                                                    duplicates_date_modified,
                                                    differences_checked,
                                                    differences_name,
                                                    differences_size,
                                                    differences_date_modified,
                                                    differences_catalogs,
                                                    folder_criteria_checked,
                                                    show_folders,
                                                    tag_checked,
                                                    tag,
                                                    search_location,
                                                    search_storage,
                                                    search_catalog,
                                                    search_catalog_checked,
                                                    search_directory_checked,
                                                    selected_directory
                                                FROM search
                                                ORDER BY date_time DESC
                                            )");
            querySearchHistory.prepare(querySearchHistorySQL);
            querySearchHistory.exec();

            QSqlQueryModel *queryModel = new QSqlQueryModel();
            queryModel->setQuery(std::move(querySearchHistory));

            queryModel->setHeaderData( 0, Qt::Horizontal, tr("Date"));
            queryModel->setHeaderData( 1, Qt::Horizontal, tr("Text Phrase selected"));
            queryModel->setHeaderData( 2, Qt::Horizontal, tr("Text Phrase"));
            queryModel->setHeaderData( 3, Qt::Horizontal, tr("Text Criteria"));
            queryModel->setHeaderData( 4, Qt::Horizontal, tr("Text Search In"));
            queryModel->setHeaderData( 5, Qt::Horizontal, tr("Case Sensitive"));
            queryModel->setHeaderData( 6, Qt::Horizontal, tr("Text Exclude"));
            queryModel->setHeaderData( 7, Qt::Horizontal, tr("File Criteria selected"));
            queryModel->setHeaderData( 8, Qt::Horizontal, tr("File Type selected"));
            queryModel->setHeaderData( 9, Qt::Horizontal, tr("File Type"));
            queryModel->setHeaderData(10, Qt::Horizontal, tr("File Size selected"));
            queryModel->setHeaderData(11, Qt::Horizontal, tr("File Size Min"));
            queryModel->setHeaderData(12, Qt::Horizontal, tr("File Size Min Unit"));
            queryModel->setHeaderData(13, Qt::Horizontal, tr("File Size Max"));
            queryModel->setHeaderData(14, Qt::Horizontal, tr("File Size Max Unit"));
            queryModel->setHeaderData(15, Qt::Horizontal, tr("Date Modified selected"));
            queryModel->setHeaderData(16, Qt::Horizontal, tr("Date Modified Min"));
            queryModel->setHeaderData(17, Qt::Horizontal, tr("Date Modified Max"));
            queryModel->setHeaderData(18, Qt::Horizontal, tr("Duplicates selected"));
            queryModel->setHeaderData(19, Qt::Horizontal, tr("Duplicates Name"));
            queryModel->setHeaderData(20, Qt::Horizontal, tr("Duplicates Size"));
            queryModel->setHeaderData(21, Qt::Horizontal, tr("Duplicates Date Modified"));
            queryModel->setHeaderData(22, Qt::Horizontal, tr("Differences selected"));
            queryModel->setHeaderData(23, Qt::Horizontal, tr("Differences Name"));
            queryModel->setHeaderData(24, Qt::Horizontal, tr("Differences Size"));
            queryModel->setHeaderData(25, Qt::Horizontal, tr("Differences Date Modified"));
            queryModel->setHeaderData(26, Qt::Horizontal, tr("Differences Catalogs"));
            queryModel->setHeaderData(27, Qt::Horizontal, tr("Folders selected"));
            queryModel->setHeaderData(28, Qt::Horizontal, tr("Show Folders"));
            queryModel->setHeaderData(29, Qt::Horizontal, tr("Tag selected"));
            queryModel->setHeaderData(30, Qt::Horizontal, tr("Tag"));
            queryModel->setHeaderData(31, Qt::Horizontal, tr("Selected Location"));
            queryModel->setHeaderData(32, Qt::Horizontal, tr("Selected Storage"));
            queryModel->setHeaderData(33, Qt::Horizontal, tr("Selected Catalog"));
            queryModel->setHeaderData(34, Qt::Horizontal, tr("Search Catalog selected"));
            queryModel->setHeaderData(35, Qt::Horizontal, tr("Search Directory selected"));
            queryModel->setHeaderData(36, Qt::Horizontal, tr("Selected Directory"));

            QSortFilterProxyModel *searchHistoryProxyModel = new QSortFilterProxyModel;
            searchHistoryProxyModel->setSourceModel(queryModel);
            ui->Search_treeView_History->setModel(searchHistoryProxyModel);
            ui->Search_treeView_History->header()->setSectionResizeMode(QHeaderView::Interactive);
            ui->Search_treeView_History->header()->resizeSection(0, 150); //Date
        }

        void MainWindow::clearSearchResults()
        {
            Catalog *empty = new Catalog(this);
            ui->Search_treeView_FilesFound->setModel(empty);
            QStandardItemModel *emptyQStandardItemModel = new QStandardItemModel;
            emptyQStandardItemModel->setHorizontalHeaderLabels({ "Catalog with results", "ID" });
            ui->Search_treeView_CatalogsFound->setModel(emptyQStandardItemModel);
            ui->Search_treeView_CatalogsFound->hideColumn(1);
        }

        //--------------------------------------------------------------------------
        //Stoppable Search process
        void MainWindow::searchFilesStoppable(){
            if (isSearchRunning) {
                if (searchProcess) {
                    searchProcess->stop();
                    searchProcess->wait();
                    delete searchProcess;
                    searchProcess = nullptr;
                }
                isSearchRunning = false;
                QApplication::restoreOverrideCursor();

                ui->Search_pushButton_Search->setText("Search");
                ui->Search_pushButton_Search->setIcon(QIcon::fromTheme("edit-find"));
                ui->Search_pushButton_Search->setStyleSheet("QPushButton{ background-color: #81d41a; }");

            } else {
                getSearchCriteria();

                searchProcess = new SearchProcess(this, collection->databaseMode);
                connect(searchProcess, &SearchProcess::searchCompleted, this, &MainWindow::handleSearchCompleted);
                connect(searchProcess, &SearchProcess::searchStopped, this, &MainWindow::handleSearchStopped);
                connect(searchProcess, &SearchProcess::searchResultsReady, this, &MainWindow::updateSearchResults);
                searchProcess->start();
                isSearchRunning = true;

                ui->Search_pushButton_Search->setText("Stop");
                ui->Search_pushButton_Search->setIcon(QIcon::fromTheme("process-stop"));
                ui->Search_pushButton_Search->setStyleSheet("QPushButton{ background-color: #ff8000; }");
            }
        }
        //----------------------------------------------------------------------
        void MainWindow::updateSearchResults()
        {
            ui->Search_treeView_CatalogsFound->setModel(newSearch->deviceFoundModel);
            ui->Search_treeView_CatalogsFound->hideColumn(1);

            //Process REGULAR SEARCH (no DUPLICATES, no DIFFERENCES)
                //Populate model with folders only if this option is selected
                if ( newSearch->searchOnFolderCriteria==true and newSearch->showFoldersOnly==true )
                {
                    // Connect model to treeview and display
                    ui->Search_treeView_FilesFound->setModel(searchProcess->fileViewModel); //TEST
                    ui->Search_treeView_FilesFound->header()->resizeSection(3, 400); //Path //TEST
                    ui->Search_treeView_FilesFound->header()->resizeSection(4, 100); //Catalog //TEST
                    ui->Search_treeView_FilesFound->header()->hideSection(0); //TEST
                    ui->Search_treeView_FilesFound->header()->hideSection(1); //TEST
                    ui->Search_treeView_FilesFound->header()->hideSection(2); //TEST

                    ui->Search_label_FoundTitle->setText(tr("Folders found"));
                }

                //Populate model with files if the folder option is not selected
                else
                {
                    //Connect model to tree/table view
                    ui->Search_treeView_FilesFound->setModel(searchProcess->fileViewModel);
                    ui->Search_treeView_FilesFound->header()->setSectionResizeMode(QHeaderView::Interactive);
                    ui->Search_treeView_FilesFound->header()->resizeSection(0, 600); //Name
                    ui->Search_treeView_FilesFound->header()->resizeSection(1, 110); //Size
                    ui->Search_treeView_FilesFound->header()->resizeSection(2, 140); //Date
                    ui->Search_treeView_FilesFound->header()->resizeSection(3, 400); //Path
                    ui->Search_treeView_FilesFound->header()->resizeSection(4, 140); //Catalog
                    ui->Search_treeView_FilesFound->header()->showSection(0);
                    ui->Search_treeView_FilesFound->header()->showSection(1);
                    ui->Search_treeView_FilesFound->header()->showSection(2);

                    ui->Search_label_FoundTitle->setText(tr("Files found"));
                }

            //Process DUPLICATES
            if ( newSearch->searchOnFileCriteria == true and ui->Search_checkBox_Duplicates->isChecked() == true
                and (     newSearch->searchDuplicatesOnName == true
                     or newSearch->searchDuplicatesOnSize == true
                     or newSearch->searchDuplicatesOnDate == true )){

                    // Connect model to tree/table view
                    ui->Search_treeView_FilesFound->setModel(searchProcess->fileViewModel);
                    ui->Search_treeView_FilesFound->header()->setSectionResizeMode(QHeaderView::Interactive);
                    ui->Search_treeView_FilesFound->header()->resizeSection(0, 600); //Name
                    ui->Search_treeView_FilesFound->header()->resizeSection(1, 110); //Size
                    ui->Search_treeView_FilesFound->header()->resizeSection(2, 140); //Date
                    ui->Search_treeView_FilesFound->header()->resizeSection(3, 400); //Path
                    ui->Search_treeView_FilesFound->header()->resizeSection(4, 100); //Catalog
            }

            //Process DIFFERENCES -------------------------------
            //Process if enabled and criteria are provided
            if ( newSearch->searchOnFileCriteria == true and ui->Search_checkBox_Differences->isChecked() == true
                and (     newSearch->differencesOnName == true
                     or newSearch->differencesOnSize == true
                     or newSearch->differencesOnDate == true)){

                    // Connect model to tree/table view
                    ui->Search_treeView_FilesFound->setModel(searchProcess->fileViewModel);
                    ui->Search_treeView_FilesFound->header()->setSectionResizeMode(QHeaderView::Interactive);
                    ui->Search_treeView_FilesFound->header()->resizeSection(0, 600); //Name
                    ui->Search_treeView_FilesFound->header()->resizeSection(1, 110); //Size
                    ui->Search_treeView_FilesFound->header()->resizeSection(2, 140); //Date
                    ui->Search_treeView_FilesFound->header()->resizeSection(3, 400); //Path
                    ui->Search_treeView_FilesFound->header()->resizeSection(4, 100); //Catalog

                    // Display count of files
                    ui->Search_label_FoundTitle->setText(tr("Duplicates found"));
            }

            ui->Search_label_NumberResults->setText(QString::number(newSearch->filesFoundNumber));
            ui->Search_label_SizeResults->setText(QLocale().formattedDataSize(newSearch->filesFoundTotalSize));

            //Other statistics, covering the case where no results are returned.
            if (newSearch->filesFoundNumber !=0){
                newSearch->filesFoundAverageSize = newSearch->filesFoundTotalSize / newSearch->filesFoundNumber;
                QList<qint64> fileSizeList = newSearch->fileSizes;
                std::sort(fileSizeList.begin(), fileSizeList.end());
                newSearch->filesFoundMinSize = fileSizeList.first();
                newSearch->filesFoundMaxSize = fileSizeList.last();

                QList<QString> fileDateList = newSearch->fileDateTimes;
                std::sort(fileDateList.begin(), fileDateList.end());
                newSearch->filesFoundMinDate = fileDateList.first();
                newSearch->filesFoundMaxDate = fileDateList.last();

                ui->Search_pushButton_FileFoundMoreStatistics->setEnabled(true);
            }

            //Save the search criteria to the search history
            newSearch->insertSearchHistoryToTable("defaultConnection");
            collection->saveSearchHistoryTableToFile();
            loadSearchHistoryTableToModel();

            QApplication::restoreOverrideCursor();
        }
        //----------------------------------------------------------------------
        void MainWindow::handleSearchCompleted()
        {
            isSearchRunning = false;
            ui->Search_pushButton_Search->setText("Search");
            ui->Search_pushButton_Search->setIcon(QIcon::fromTheme("edit-find"));
            ui->Search_pushButton_Search->setStyleSheet("QPushButton{ background-color: #81d41a; }");
            //searchProcess->stop();

            QSqlQuery query(QSqlDatabase::database("defaultConnection"));
            QString querySQL = QLatin1String(R"(
                                            SELECT catalog_date_loaded
                                            FROM catalog
                                            WHERE catalog_id=:catalog_id
                                        )");
            query.prepare(querySQL);
            query.bindValue(":catalog_id", 2);
            query.exec();
            query.next();
        }
        //----------------------------------------------------------------------
        void MainWindow::handleSearchStopped()
        {
            isSearchRunning = false;
            ui->Search_pushButton_Search->setText("Search");
            ui->Search_pushButton_Search->setIcon(QIcon::fromTheme("edit-find"));
            ui->Search_pushButton_Search->setStyleSheet("QPushButton{ background-color: #81d41a; }");
        }
        //----------------------------------------------------------------------

        // Implement the getter methods
        int MainWindow::getDifferencesCatalog1ID() const {
            return ui->Search_comboBox_DifferencesCatalog1->currentData().toInt();
        }

        int MainWindow::getDifferencesCatalog2ID() const {
            return ui->Search_comboBox_DifferencesCatalog2->currentData().toInt();
        }
