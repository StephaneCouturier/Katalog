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
// Description:
// Author:      Stephane Couturier
/////////////////////////////////////////////////////////////////////////////
*/

#include "mainwindow.h"
#include "ui_mainwindow.h"
#include "catalog.h"
#include "filesview.h"

//TAB: SEARCH FILES ------------------------------------------------------------

    //User interactions

        //Buttons and other changes
        void MainWindow::on_Search_lineEdit_SearchText_returnPressed()
        {
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
        void MainWindow::on_Search_pushButton_Search_clicked()
        {
            searchFiles();
        }
        //----------------------------------------------------------------------
        void MainWindow::on_Search_pushButton_CleanSearchText_clicked()
        {
            QString originalText;
            originalText = ui->Search_lineEdit_SearchText->text();

            originalText.replace("."," ");
            originalText.replace(","," ");
            originalText.replace("_"," ");
            originalText.replace("-"," ");
            originalText.replace("("," ");
            originalText.replace(")"," ");
            originalText.replace("["," ");
            originalText.replace("]"," ");
            originalText.replace("{"," ");
            originalText.replace("}"," ");
            originalText.replace("/"," ");
            originalText.replace("\\"," ");
            originalText.replace("'"," ");
            originalText.replace("\""," ");

            ui->Search_lineEdit_SearchText->setText(originalText);

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
        void MainWindow::on_Search_listView_CatalogsFound_clicked(const QModelIndex &index)
        {
            //Refine the seach with the selction of one of the catalogs that have results

            //Get file from selected row
            selectedFilterCatalogName = ui->Search_listView_CatalogsFound->model()->index(index.row(), 0, QModelIndex()).data().toString();
            ui->Filters_label_DisplayCatalog->setText(selectedFilterCatalogName);

            selectedFilterStorageLocation = tr("All");
            ui->Filters_label_DisplayLocation->setText(selectedFilterStorageLocation);

            selectedFilterStorageName = tr("All");
            ui->Filters_label_DisplayStorage->setText(selectedFilterStorageName);

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

                    QSettings settings(settingsFilePath, QSettings:: IniFormat);
                    settings.setValue("Settings/ShowHideSearchCriteria", "go-down");
            }
            else{ //Show
                    ui->Search_pushButton_ShowHideSearchCriteria->setIcon(QIcon::fromTheme("go-up"));
                    ui->Search_widget_SearchCriteria->setHidden(false);

                    QSettings settings(settingsFilePath, QSettings:: IniFormat);
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

                    QSettings settings(settingsFilePath, QSettings:: IniFormat);
                    settings.setValue("Settings/ShowHideCatalogResults", "go-next");
            }
            else{ //Show
                    ui->Search_pushButton_ShowHideCatalogResults->setIcon(QIcon::fromTheme("go-previous"));

                    ui->Search_widget_ResultsCatalogs->setHidden(false);
                    ui->Search_label_CatalogsWithResults->setHidden(false);

                    QSettings settings(settingsFilePath, QSettings:: IniFormat);
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

                    QSettings settings(settingsFilePath, QSettings:: IniFormat);
                    settings.setValue("Settings/ShowHideSearchHistory", "go-up");
            }
            else{ //Show
                    ui->Search_pushButton_ShowHideSearchHistory->setIcon(QIcon::fromTheme("go-down"));
                    ui->Search_treeView_History->setHidden(false);

                    QSettings settings(settingsFilePath, QSettings:: IniFormat);
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
                ui->Search_listView_CatalogsFound->setEnabled(false);
            }
            else{
                ui->Search_widget_DifferencesCatalogs->setHidden(true);
                ui->Search_checkBox_DifferencesName->setDisabled(true);
                ui->Search_checkBox_DifferencesSize->setDisabled(true);
                ui->Search_checkBox_DifferencesDateModified->setDisabled(true);
                ui->Search_listView_CatalogsFound->setEnabled(true);
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
                                   QString::number(filesFoundNumber),
                                   QLocale().formattedDataSize(filesFoundTotalSize),
                                   QLocale().formattedDataSize(filesFoundAverageSize),
                                   QLocale().formattedDataSize(filesFoundMinSize),
                                   QLocale().formattedDataSize(filesFoundMaxSize),
                                   filesFoundMinDate,
                                   filesFoundMaxDate));
            msgBox.setIcon(QMessageBox::Information);
            msgBox.exec();
        }
        //----------------------------------------------------------------------
        void MainWindow::on_SearchTreeViewFilesFoundHeaderSortOrderChanged(){

            QSettings settings(settingsFilePath, QSettings:: IniFormat);
            QHeaderView *searchTreeHeader = ui->Search_treeView_FilesFound->header();

            lastSearchSortSection = searchTreeHeader->sortIndicatorSection();
            lastSearchSortOrder   = searchTreeHeader->sortIndicatorOrder();

            settings.setValue("Search/lastSearchSortSection", QString::number(lastSearchSortSection));
            settings.setValue("Search/lastSearchSortOrder",   QString::number(lastSearchSortOrder));
        }
        //----------------------------------------------------------------------
        void MainWindow::on_SearchTreeViewHistoryHeaderSortOrderChanged(){

            QSettings settings(settingsFilePath, QSettings:: IniFormat);
            QHeaderView *searchHistoryTreeHeader = ui->Search_treeView_History->header();

            lastSearchHistorySortSection = searchHistoryTreeHeader->sortIndicatorSection();
            lastSearchHistorySortOrder   = searchHistoryTreeHeader->sortIndicatorOrder();

            settings.setValue("Search/lastSearchHistorySortSection", QString::number(lastSearchHistorySortSection));
            settings.setValue("Search/lastSearchHistorySortOrder",   QString::number(lastSearchHistorySortOrder));
        }
        //----------------------------------------------------------------------
        void MainWindow::on_Search_splitter_Results_splitterMoved()
        {
            QSettings settings(settingsFilePath, QSettings:: IniFormat);
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
            QString selectedFileFolder  = ui->Search_treeView_FilesFound->model()->index(index.row(), 3, QModelIndex()).data().toString();
            QString selectedFileCatalog = ui->Search_treeView_FilesFound->model()->index(index.row(), 4, QModelIndex()).data().toString();

            //Prepare inputs for the Explore
            selectedCatalog->setName(selectedFileCatalog);
            selectedCatalog->loadCatalogMetaData();
            selectedDirectoryName = selectedFileFolder.remove(selectedCatalog->sourcePath + "/");

            //Open the catalog into the Explore
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
                                                                    collectionFolder,
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
        //run a search of files in each selected catalog based on user inputs
        void MainWindow::searchFiles()
        {
            // Start animation while opening
            QApplication::setOverrideCursor(Qt::WaitCursor);

            //Prepare the SEARCH -------------------------------
                //Clear exisiting lists of results and search variables
                    filesFoundList.clear();
                    catalogFoundList.clear();

                //Clear the temporary search and get search criteria
                    getSearchCriteria();

                // Searching "Begin With" for File name or Folder name is not supported yet
                    //Stop animation
                    if (newSearch->selectedTextCriteria==tr("Begins With") and newSearch->selectedSearchIn !=tr("File names only")){
                        QApplication::restoreOverrideCursor();
                        QMessageBox::information(this,"Katalog",tr("The option 'Begin With' can only be used with 'File names only'.\nUse a different combinaison."));
                        return;;
                    }

            //Process the SEARCH in CATALOGS or DIRECTORY ------------------------------
                //Process the SEARCH in CATALOGS
                    if (newSearch->searchInCatalogsChecked == true){

                        //List of catalogs to search from: catalogSelectedList
                            //Search every catalog if "All" is selected
                            if ( selectedFilterCatalogName ==tr("All")){
                                //For differences, only process with the selected catalogs
                                if (ui->Search_checkBox_Differences->isChecked() == true){
                                    QStringList differenceCatalogs;
                                    differenceCatalogs << newSearch->differencesCatalog1;
                                    differenceCatalogs << newSearch->differencesCatalog2;
                                    foreach(sourceCatalog,differenceCatalogs)
                                        {
                                            searchFilesInCatalog(sourceCatalog);
                                        }
                                }
                                //Otherwise process all selected globally
                                else foreach(sourceCatalog,catalogSelectedList)
                                        {
                                            searchFilesInCatalog(sourceCatalog);
                                        }
                                }

                            //Otherwise just search files in the selected catalog
                            else{
                                searchFilesInCatalog(selectedFilterCatalogName);

                                //but also load the second catalog for Differences
                                if ( newSearch->searchOnFileCriteria == true and ui->Search_checkBox_Differences->isChecked() == true
                                    and (   newSearch->differencesOnName == true
                                         or newSearch->differencesOnSize == true
                                         or newSearch->differencesOnDate == true)){

                                        if(ui->Search_comboBox_DifferencesCatalog1->currentText()!=selectedFilterCatalogName)
                                            searchFilesInCatalog(ui->Search_comboBox_DifferencesCatalog1->currentText());

                                        if(ui->Search_comboBox_DifferencesCatalog2->currentText()!=selectedFilterCatalogName)
                                            searchFilesInCatalog(ui->Search_comboBox_DifferencesCatalog2->currentText());
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
                    catalogFoundList.removeDuplicates();
                    catalogFoundList.sort();

                    //Keep the catalog file name only
                    foreach(QString item, catalogFoundList){
                            int index = catalogFoundList.indexOf(item);
                            //QDir dir(item);
                            QFileInfo fileInfo(item);
                            catalogFoundList[index] = fileInfo.baseName();
                    }

                    //Load list of catalogs in which files where found
                    catalogFoundListModel = new QStringListModel(this);
                    catalogFoundListModel->setStringList(catalogFoundList);
                    ui->Search_listView_CatalogsFound->setModel(catalogFoundListModel);

                //Process search results: list of files

                    // Prepare query model
                    QSqlQueryModel *loadCatalogQueryModel = new QSqlQueryModel;
                    // Prepare model to display
                    FilesView *fileViewModel = new FilesView(this);

                    // Populate model with folders only if this option is selected
                    if ( newSearch->searchOnFolderCriteria==true and ui->Search_checkBox_ShowFolders->isChecked()==true )
                    {
                        newSearch->sFilePaths.removeDuplicates();
                        int numberOfFolders = newSearch->sFilePaths.count();
                        newSearch->sFileNames.clear();
                        newSearch->sFileSizes.clear();
                        newSearch->sFileDateTimes.clear();
                        newSearch->sFileCatalogs.clear();
                        for (int i=0; i<numberOfFolders; i++)
                            newSearch->sFileNames <<"";
                        for (int i=0; i<numberOfFolders; i++)
                            newSearch->sFileSizes <<0;
                        for (int i=0; i<numberOfFolders; i++)
                            newSearch->sFileDateTimes <<"";
                        for (int i=0; i<numberOfFolders; i++)
                            newSearch->sFileCatalogs <<"";

                        // Populate model with data
                        newSearch->populateFileData(
                            newSearch->sFileNames,
                            newSearch->sFileSizes,
                            newSearch->sFilePaths,
                            newSearch->sFileDateTimes,
                            newSearch->sFileCatalogs);
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

                    // Populate model with files if the folder option is not selected
                    else
                    {
                        // Populate model with data
                        newSearch->populateFileData(
                            newSearch->sFileNames,
                            newSearch->sFileSizes,
                            newSearch->sFilePaths,
                            newSearch->sFileDateTimes,
                            newSearch->sFileCatalogs);
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
                                    QSqlQuery deleteQuery;
                                    deleteQuery.exec("DELETE FROM filetemp");

                                //prepare query to load file info
                                    QSqlQuery insertQuery;
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

                                //loop through the result list and populate database

                                    int rows = newSearch->rowCount();

                                    for (int i=0; i<rows; i++) {

                                            //Append data to the database
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
                                QSqlQuery duplicatesQuery;
                                duplicatesQuery.prepare(selectSQL);
                                duplicatesQuery.exec();

                                //recapture file results for Stats
                                newSearch->sFileNames.clear();
                                newSearch->sFileSizes.clear();
                                newSearch->sFilePaths.clear();
                                newSearch->sFileDateTimes.clear();
                                newSearch->sFileCatalogs.clear();
                                while(duplicatesQuery.next()){
                                        newSearch->sFileNames.append(duplicatesQuery.value(0).toString());
                                        newSearch->sFileSizes.append(duplicatesQuery.value(1).toLongLong());
                                        newSearch->sFileDateTimes.append(duplicatesQuery.value(2).toString());
                                        newSearch->sFilePaths.append(duplicatesQuery.value(3).toString());
                                        newSearch->sFileCatalogs.append(duplicatesQuery.value(4).toString());
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
                                filesFoundNumber = fileViewModel->rowCount();

                        }

                //Process DIFFERENCES -------------------------------

                    //Process if enabled and criteria are provided
                        if ( newSearch->searchOnFileCriteria == true and ui->Search_checkBox_Differences->isChecked() == true
                             and (     newSearch->differencesOnName == true
                                    or newSearch->differencesOnSize == true
                                    or newSearch->differencesOnDate == true)){

							//Load Search results into the database
                                //Clear database
                                    QSqlQuery deleteQuery;
                                    deleteQuery.exec("DELETE FROM filetemp");

                                //Prepare query to load file info
                                    QSqlQuery insertQuery;
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
                                QSqlQuery differencesQuery;
                                differencesQuery.prepare(selectSQL);
                                differencesQuery.bindValue(":selectedDifferencesCatalog1",newSearch->differencesCatalog1);
                                differencesQuery.bindValue(":selectedDifferencesCatalog2",newSearch->differencesCatalog2);
                                differencesQuery.exec();

                                //recapture file results for Stats
                                newSearch->sFileNames.clear();
                                newSearch->sFileSizes.clear();
                                newSearch->sFilePaths.clear();
                                newSearch->sFileDateTimes.clear();
                                newSearch->sFileCatalogs.clear();
                                while(differencesQuery.next()){
                                        newSearch->sFileNames.append(differencesQuery.value(0).toString());
                                        newSearch->sFileSizes.append(differencesQuery.value(1).toLongLong());
                                        newSearch->sFileDateTimes.append(differencesQuery.value(2).toString());
                                        newSearch->sFilePaths.append(differencesQuery.value(3).toString());
                                        newSearch->sFileCatalogs.append(differencesQuery.value(4).toString());
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
                        filesFoundNumber = 0;
                        filesFoundTotalSize = 0;
                        filesFoundAverageSize = 0;
                        filesFoundMinSize = 0;
                        filesFoundMaxSize = 0;
                        filesFoundMinDate = "";
                        filesFoundMaxDate = "";

                    //Number of files found
                    filesFoundNumber = fileViewModel->rowCount();
                    ui->Search_label_NumberResults->setText(QString::number(filesFoundNumber));

                    //Total size of files found
                    qint64 sizeItem;
                    filesFoundTotalSize = 0;
                    foreach (sizeItem, newSearch->sFileSizes) {
                                filesFoundTotalSize = filesFoundTotalSize + sizeItem;
                    }
                    ui->Search_label_SizeResults->setText(QLocale().formattedDataSize(filesFoundTotalSize));

                    //Other statistics, covering the case where no results are returned.
                    if (filesFoundNumber !=0){
                                filesFoundAverageSize = filesFoundTotalSize / filesFoundNumber;
                                QList<qint64> fileSizeList = newSearch->sFileSizes;
                                std::sort(fileSizeList.begin(), fileSizeList.end());
                                filesFoundMinSize = fileSizeList.first();
                                filesFoundMaxSize = fileSizeList.last();

                                QList<QString> fileDateList = newSearch->sFileDateTimes;
                                std::sort(fileDateList.begin(), fileDateList.end());
                                filesFoundMinDate = fileDateList.first();
                                filesFoundMaxDate = fileDateList.last();

                                ui->Search_pushButton_FileFoundMoreStatistics->setEnabled(true);
                    }

            //Save the search criteria to the search history
            insertSearchHistoryToTable();
            if(databaseMode=="Memory")
                saveSearchHistoryTableToFile();
            loadSearchHistoryTableToModel();

            //Stop animation
            QApplication::restoreOverrideCursor();

            //Enable Export
            ui->Search_pushButton_ProcessResults->setEnabled(true);
            ui->Search_comboBox_SelectProcess->setEnabled(true);
        }
        //----------------------------------------------------------------------
        //run a search of files for the selected Catalog
        void MainWindow::searchFilesInCatalog(const QString &sourceCatalogName)
        {
            tempCatalog->setName(sourceCatalogName);
            tempCatalog->loadCatalogMetaData();

            //Prepare Inputs
                QFile catalogFile(tempCatalog->sourcePath);

                QRegularExpressionMatch match;
                QRegularExpressionMatch foldermatch;

                //Define how to use the search text
                    if(newSearch->selectedTextCriteria == tr("Exact Phrase"))
                        regexSearchtext=newSearch->searchText; //just search for the extact text entered including spaces, as one text string.
                    else if(newSearch->selectedTextCriteria == tr("Begins With"))
                        regexSearchtext="(^"+newSearch->searchText+")";
                    else if(newSearch->selectedTextCriteria == tr("Any Word"))
                        regexSearchtext=newSearch->searchText.replace(" ","|");
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
                        regexSearchtext = groupRegEx;
                    }
                    else {
                        regexSearchtext="";
                         }

                    regexPattern = regexSearchtext;

                //Prepare the regexFileType for file types
                if( newSearch->searchOnFileCriteria==true and newSearch->searchOnType ==true and newSearch->selectedFileType !="All"){
                    //Get the list of file extension and join it into one string
                    if(newSearch->selectedFileType =="Audio"){
                                regexFileType = fileType_AudioS.join("|");
                    }
                    if(newSearch->selectedFileType =="Image"){
                                regexFileType = fileType_ImageS.join("|");
                    }
                    if(newSearch->selectedFileType =="Text"){
                                regexFileType = fileType_TextS.join("|");
                    }
                    if(newSearch->selectedFileType =="Video"){
                                regexFileType = fileType_VideoS.join("|");
                    }

                    //Replace the *. by .* needed for regex
                    regexFileType = regexFileType.replace("*.",".*");

                    //Add the file type expression to the regex
                    regexPattern = regexSearchtext  + "(" + regexFileType + ")";

                 }
                 else
                    regexPattern = regexSearchtext;

                //Add the words to exclude to the regex
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
                    regexPattern = excludeGroupRegEx + regexPattern;
                }

                //Initiate Regular Expression
                QRegularExpression regex(regexPattern);
                if (newSearch->caseSensitive != true) {
                    regex.setPatternOptions(QRegularExpression::CaseInsensitiveOption);
                }

            //Load the catalog file contents if not already loaded in memory
                tempCatalog->loadCatalogFileListToTable();

            //Search loop for all lines in the catalog file
                //Load the files of the Catalog
                    QSqlQuery getFilesQuery;
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
                    getFilesQuery.bindValue(":file_catalog",sourceCatalogName);
                    getFilesQuery.bindValue(":file_size_min",newSearch->selectedMinimumSize * newSearch->sizeMultiplierMin);
                    getFilesQuery.bindValue(":file_size_max",newSearch->selectedMaximumSize * newSearch->sizeMultiplierMax);
                    getFilesQuery.bindValue(":file_date_updated_min",newSearch->selectedDateMin.toString("yyyy/MM/dd hh:mm:ss"));
                    getFilesQuery.bindValue(":file_date_updated_max",newSearch->selectedDateMax.toString("yyyy/MM/dd hh:mm:ss"));
                    getFilesQuery.exec();

                //File by file, test if the file is matching all search criteria
                //Loop principle1: stop further verification as soon as a criteria fails to match
                //Loop principle2: start with fastest criteria, finish with more complex ones (tag, file name)

                while(getFilesQuery.next()){

                    QString   lineFileName     = getFilesQuery.value(0).toString();
                    QString   lineFilePath     = getFilesQuery.value(1).toString();
                    QString   lineFileFullPath = lineFilePath + "/" + lineFileName;
                    bool      fileIsMatchingTag;

                    //Continue if the file is matching the tags
                        if (newSearch->searchOnFolderCriteria==true and newSearch->searchOnTags==true and newSearch->selectedTagName!=""){

                            fileIsMatchingTag = false;

                            //Set query to get a list of folder paths matching the selected tag
                            QSqlQuery queryTag;
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
                                regex.setPattern(regexSearchtext);
                                foldermatch = regex.match(lineFilePath);
                                //If it does, then check that the file matches the selected file type
                                if (foldermatch.hasMatch() and newSearch->searchOnType==true){
                                    regex.setPattern(regexFileType);
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
                                filesFoundList << lineFilePath;
                                catalogFoundList.insert(0,sourceCatalogName);

                                //Populate result lists
                                newSearch->sFileNames.append(lineFileName);
                                newSearch->sFilePaths.append(lineFilePath);
                                newSearch->sFileSizes.append(getFilesQuery.value(2).toLongLong());
                                newSearch->sFileDateTimes.append(getFilesQuery.value(3).toString());
                                newSearch->sFileCatalogs.append(sourceCatalogName);
                            }
                        }
                        else{
                            //verify file matches the selected file type
                            if (newSearch->searchOnType==true){
                                regex.setPattern(regexFileType);
                            }
                            match = regex.match(lineFilePath);
                            if (!match.hasMatch()){
                                continue;
                            }

                            //Add the file and its catalog to the results, excluding blank lines
                            if (lineFilePath !=""){
                                filesFoundList << lineFilePath;
                                catalogFoundList.insert(0,sourceCatalogName);

                                //Populate result lists
                                newSearch->sFileNames.append(lineFileName);
                                newSearch->sFilePaths.append(lineFilePath);
                                newSearch->sFileSizes.append(getFilesQuery.value(2).toLongLong());
                                newSearch->sFileDateTimes.append(getFilesQuery.value(3).toString());
                                newSearch->sFileCatalogs.append(sourceCatalogName);
                            }
                        }
                }
        }
        //----------------------------------------------------------------------
        //run a search of files for the selected Directory
        void MainWindow::searchFilesInDirectory(const QString &sourceDirectory)
        {
            //Define how to use the search text //COMMON to searchFilesInCatalog
                if(newSearch->selectedTextCriteria == tr("Exact Phrase"))
                    regexSearchtext=newSearch->searchText; //just search for the extact text entered including spaces, as one text string.
                else if(newSearch->selectedTextCriteria == tr("Begins With"))
                    regexSearchtext="(^"+newSearch->searchText+")";
                else if(newSearch->selectedTextCriteria == tr("Any Word"))
                    regexSearchtext=newSearch->searchText.replace(" ","|");
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
                    regexSearchtext = groupRegEx;
                }
                else {
                    regexSearchtext="";
                     }

                regexPattern = regexSearchtext;

            //Prepare the regexFileType for file types //COMMON to searchFilesInCatalog
            if ( newSearch->searchOnFileCriteria==true and newSearch->selectedFileType !=tr("All")){
                //Get the list of file extension and join it into one string
                if(newSearch->selectedFileType ==tr("Audio")){
                            regexFileType = fileType_AudioS.join("|");
                }
                if(newSearch->selectedFileType ==tr("Image")){
                            regexFileType = fileType_ImageS.join("|");
                }
                if(newSearch->selectedFileType ==tr("Text")){
                            regexFileType = fileType_TextS.join("|");
                }
                if(newSearch->selectedFileType ==tr("Video")){
                            regexFileType = fileType_VideoS.join("|");
                }

                //Replace the *. by .* needed for regex
                regexFileType = regexFileType.replace("*.",".*");

                //Add the file type expression to the regex
                regexPattern = regexSearchtext  + "(" + regexFileType + ")";
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
                regexPattern = excludeGroupRegEx + regexPattern;
            }

            QRegularExpression regex(regexPattern, QRegularExpression::CaseInsensitiveOption);

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
                            QSqlQuery queryTag;
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
                            regex.setPattern(regexSearchtext);

                            foldermatch = regex.match(reducedLine);

                            //if it does, then check that the file matches the selected file type
                            if (foldermatch.hasMatch()){
                                regex.setPattern(regexFileType);

                                match = regex.match(lineFilePath);
                            }

                        }
                        else {
                            match = regex.match(lineFilePath);
                        }

                        //If the file is matching the criteria, add it and its catalog to the search results
                        //COMMON to searchFilesInCatalog
                        if (match.hasMatch()){

                            filesFoundList << lineFilePath;

                            //COMMON to searchFilesInCatalog
                            //Retrieve other file info
                            QFileInfo file(lineFilePath);

                            // Get the fileDateTime from the list if available
                            QString lineFileDatetime;
                            if (fieldListCount == 3){
                                    lineFileDatetime = lineFieldList[2];}
                            else lineFileDatetime = "";

                            //Populate result lists
                            newSearch->sFileNames.append(file.fileName());
                            newSearch->sFilePaths.append(file.path());
                            newSearch->sFileSizes.append(lineFileSize);
                            newSearch->sFileDateTimes.append(lineFileDatetime);
                            newSearch->sFileCatalogs.append(sourceDirectory);
                        }
                    }
                    else{

                        //Add the file and its catalog to the results, excluding blank lines
                        if (lineFilePath !=""){
                            filesFoundList << lineFilePath;
                            catalogFoundList.insert(0,sourceDirectory);

                            //Retrieve other file info
                            QFileInfo file(lineFilePath);

                            // Get the fileDateTime from the list if available
                            QString lineFileDatetime;
                            if (fieldListCount == 3){
                                    lineFileDatetime = lineFieldList[2];}
                            else lineFileDatetime = "";

                            //Populate result lists
                            newSearch->sFileNames.append(file.fileName());
                            newSearch->sFilePaths.append(file.path());
                            newSearch->sFileSizes.append(lineFileSize);
                            newSearch->sFileDateTimes.append(lineFileDatetime);
                            newSearch->sFileCatalogs.append(sourceDirectory);
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
                    // the first line is the one displayed by default
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
            Catalog *empty = new Catalog(this);
            ui->Search_treeView_FilesFound->setModel(empty);
            ui->Search_listView_CatalogsFound->setModel(empty);

            //Clear results and disable export
            filesFoundList.clear();
            ui->Search_pushButton_ProcessResults->setEnabled(false);
            ui->Search_comboBox_SelectProcess->setEnabled(false);
        }
        //----------------------------------------------------------------------
        void MainWindow::loadSearchCriteria(Search *search)
        {//Set search values

            //Selections
            if(search->searchInCatalogsChecked == 1){
                        ui->Filters_checkBox_SearchInCatalogs->setChecked(search->searchInCatalogsChecked);
            }
            else if(search->searchInConnectedChecked== 1){
                        ui->Filters_checkBox_SearchInConnectedDrives->setChecked(search->searchInConnectedChecked);
            }

            ui->Filters_lineEdit_SeletedDirectory->setText(search->connectedDirectory);

            selectedFilterStorageLocation = search->selectedLocation;
            selectedFilterStorageName = search->selectedStorage;
            selectedFilterCatalogName = search->selectedCatalog;
            ui->Filters_label_DisplayLocation->setText(selectedFilterStorageLocation);
            ui->Filters_label_DisplayStorage->setText(selectedFilterStorageName);
            ui->Filters_label_DisplayCatalog->setText(selectedFilterCatalogName);

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
                Catalog *empty = new Catalog(this);
                ui->Search_treeView_FilesFound->setModel(empty);
                ui->Search_listView_CatalogsFound->setModel(empty);


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

                newSearch->selectedLocation         = ui->Filters_label_DisplayLocation->text();
                newSearch->selectedStorage          = ui->Filters_label_DisplayStorage->text();
                newSearch->selectedCatalog          = ui->Filters_label_DisplayCatalog->text();
                newSearch->searchInCatalogsChecked  = ui->Filters_checkBox_SearchInCatalogs->isChecked();
                newSearch->searchInConnectedChecked = ui->Filters_checkBox_SearchInConnectedDrives->isChecked();
                newSearch->connectedDirectory       = ui->Filters_lineEdit_SeletedDirectory->text();

        }
        //----------------------------------------------------------------------
        void MainWindow::refreshLocationSelectionList()
        {
            //Query the full list of locations
            QSqlQuery getLocationList;
            QString getLocationListSQL = QLatin1String(R"(
                                            SELECT DISTINCT storage_location
                                            FROM storage
                                            ORDER BY storage_location
                            )");
            getLocationList.prepare(getLocationListSQL);
            getLocationList.exec();

            //Put the results in a list
            QStringList locationList;
            while (getLocationList.next()) {
                locationList << getLocationList.value(0).toString();
            }

            QStringList displayLocationList = locationList;
            //Add the "All" option at the beginning
            displayLocationList.insert(0,tr("All"));
        }
        //----------------------------------------------------------------------
        void MainWindow::refreshStorageSelectionList(QString selectedLocation)
        {
            //Query the full list of locations
            QSqlQuery getStorageList;

            QString queryText = "SELECT storage_name FROM storage ";

            if ( selectedLocation == tr("All")){
                    queryText = queryText + " ORDER BY storage_name ";
                    getStorageList.prepare(queryText);
            }
            else{
                    queryText = queryText + " WHERE storage_location ='" + selectedLocation + "'";
                    queryText = queryText + " ORDER BY storage_name ";
                    getStorageList.prepare(queryText);
            }
            getStorageList.exec();

            //Put the results in a list
            QStringList storageList;
            while (getStorageList.next()) {
                storageList << getStorageList.value(0).toString();
            }

            //Prepare list for the Location combobox
            QStringList displayStorageList = storageList;
            //Add the "All" option at the beginning
            displayStorageList.insert(0,tr("All"));
        }
        //----------------------------------------------------------------------
        void MainWindow::refreshCatalogSelectionList(QString selectedLocation, QString selectedStorage, QString selectedVirtualStorage)
        {//Update the list of selected catalogs, based on the selection of Location or Storage device

            //Prepare and run the query
                QSqlQuery getCatalogSelectionList;
                QString getCatalogSelectionListSQL = QLatin1String(R"(
                                                        SELECT c.catalog_name
                                                        FROM catalog c
                                                        LEFT JOIN storage s ON c.catalog_storage = s.storage_name
                                                    )");

                //Filter depending on location and selection.
                if      ( selectedLocation == tr("All") and selectedStorage != tr("All"))
                {
                        getCatalogSelectionListSQL += " WHERE c.catalog_storage =:selectedStorage ";
                }
                else if ( selectedLocation != tr("All") and selectedStorage == tr("All"))
                {
                        getCatalogSelectionListSQL += " WHERE storage_location =:selectedLocation ";
                }
                else if ( selectedLocation == tr("All") and selectedStorage == tr("All") and selectedVirtualStorage == tr("All"))
                {
                        getCatalogSelectionListSQL += " WHERE c.catalog_storage =:selectedStorage"
                                                      " AND storage_location =:selectedLocation ";
                } else
               if ( selectedLocation == tr("All") and selectedStorage == tr("All") and selectedVirtualStorage != tr("All"))
                {
                        getCatalogSelectionListSQL += " INNER JOIN virtual_storage_catalog vsc ON c.catalog_name = vsc.catalog_name ";
                        getCatalogSelectionListSQL += " INNER JOIN virtual_storage         vs  ON vs.virtual_storage_id = vsc.virtual_storage_id ";
                        getCatalogSelectionListSQL +=   " WHERE vsc.virtual_storage_id IN ( "
                                                        " WITH RECURSIVE hierarchy_cte AS ( "
                                                        "       SELECT virtual_storage_id, virtual_storage_parent_id, virtual_storage_name "
                                                        "       FROM virtual_storage "
                                                        "       WHERE virtual_storage_id = :virtual_storage_id "
                                                        "       UNION ALL "
                                                        "       SELECT t.virtual_storage_id, t.virtual_storage_parent_id, t.virtual_storage_name "
                                                        "       FROM virtual_storage t "
                                                        "       JOIN hierarchy_cte cte ON t.virtual_storage_parent_id = cte.virtual_storage_id "
                                                        "  ) "
                                                        "  SELECT virtual_storage_id "
                                                        "  FROM hierarchy_cte) ";
                }

                //Order
                getCatalogSelectionListSQL += " ORDER BY c.catalog_name ASC ";

                //Run the query
                getCatalogSelectionList.prepare(getCatalogSelectionListSQL);
                getCatalogSelectionList.bindValue(":selectedLocation", selectedLocation);
                getCatalogSelectionList.bindValue(":selectedStorage",  selectedStorage);
                getCatalogSelectionList.bindValue(":virtual_storage_id", selectedFilterVirtualStorageID);
                getCatalogSelectionList.exec();

            //Put the results in a list
                //Clear the list of selected catalogs
                catalogSelectedList.clear();

                //Populate from the query results
                while (getCatalogSelectionList.next()) {
                    catalogSelectedList << getCatalogSelectionList.value(0).toString();
                }

            //Send list to the Statistics combobox
                QStringListModel *catalogListModelForStats = new QStringListModel(this);
                catalogListModelForStats->setStringList(catalogSelectedList);
        }
        //--------------------------------------------------------------------------
        void MainWindow::refreshDifferencesCatalogSelection(){
            ui->Search_comboBox_DifferencesCatalog1->clear();
            ui->Search_comboBox_DifferencesCatalog2->clear();
            foreach(sourceCatalog,catalogSelectedList)
                    {
                        ui->Search_comboBox_DifferencesCatalog1->addItem(sourceCatalog);
                        ui->Search_comboBox_DifferencesCatalog2->addItem(sourceCatalog);
                    }
        }
        //----------------------------------------------------------------------
        void MainWindow::batchProcessSearchResults()
        {//Process all results according to user's choice

            //user process choice
            QString selectedProcess = ui->Search_comboBox_SelectProcess->currentText();
            QString result;

            //Generate list of full file path (directory path + file name)
            QStringList resultsFilesList;
            for (int i = 0; i < newSearch->sFileNames.size(); ++i)
            {
                QString fileFullPath = newSearch->sFilePaths[i] + "/" + newSearch->sFileNames[i];
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
                        if (exportFileName !=""){
                            QMessageBox msgBox;
                            msgBox.setWindowTitle("Katalog");
                            msgBox.setTextFormat(Qt::RichText);
                            exportFileName = "file://" + exportFileName;
                            msgBox.setText(tr("Results exported to the collection folder:")
                                           +"<br/><a href='"+exportFileName+"'>"+exportFileName+"</a>");
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
                                                 +tr("Move all %1 files (%2) from these results to trash?").arg(QString::number(filesFoundNumber), QLocale().formattedDataSize(filesFoundTotalSize)),
                                             QMessageBox::Yes|QMessageBox::Cancel)
                        == QMessageBox::Yes) {
                            for (int i = 0; i < newSearch->sFileNames.size(); ++i)
                            {
                            fileFullPath = moveFileToTrash(newSearch->sFilePaths[i] + "/" + newSearch->sFileNames[i]);
                            trashPath = moveFileToTrash(fileFullPath);
                            if(trashPath==""){
                         QMessageBox::information(this,"Katalog",tr("Problem moving file: ")
                                                                       +"<br/>file-<a href='"+fileFullPath+"'>"+fileFullPath+"</a>");
                            }
                            else
                         movedFiles+=1;
                            }
                            QMessageBox::information(this,"Katalog",tr("%1 files were moved to trash, out of %2 files from the results.").arg(QString::number(movedFiles), QString::number(filesFoundNumber)));
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
                    if (QMessageBox::critical(this,
                                              tr("Confirmation"),
                                              "<span style='color:red;font-weight: bold;'>"+tr("DELETE")+"</span><br/>"
                                              +tr("Delete permanently all %1 files (%2) from these results?").arg(QString::number(filesFoundNumber), QLocale().formattedDataSize(filesFoundTotalSize)),
                                              QMessageBox::Yes|QMessageBox::Cancel)
                        == QMessageBox::Yes) {
                        for (int i = 0; i < newSearch->sFileNames.size(); ++i)
                        {
                            result = deleteFile(newSearch->sFilePaths[i] + "/" + newSearch->sFileNames[i]);
                            if (result!=""){
                                deletedFiles +=1;
                            }
                            result="";
                        }
                        QMessageBox::information(this,"Katalog",tr("%1 files were deleted, out of %2 files from the results.").arg(QString::number(deletedFiles), QString::number(filesFoundNumber)));
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
        {
            QString fileExtension;
            QStringList catalogMetadata;
            QString fullFileName;

            int result = QMessageBox::question(this,"Katalog",
                      tr("Create a catalog from these results?"
                         "<br/>- Yes: create an idx file and use it to refine your search,"
                         "<br/>- No:  simply export results to a csv file."),
                                              QMessageBox::Yes|QMessageBox::No|QMessageBox::Cancel);

            if ( result !=QMessageBox::Cancel){

                if ( result ==QMessageBox::Yes){
                    fileExtension="idx";

                    catalogMetadata.prepend("<catalogIsFullDevice>");
                    catalogMetadata.prepend("<catalogIncludeSymblinks>");
                    catalogMetadata.prepend("<catalogStorage>EXPORT");
                    catalogMetadata.prepend("<catalogFileType>EXPORT");
                    catalogMetadata.prepend("<catalogIncludeHidden>false");
                    catalogMetadata.prepend("<catalogTotalFileSize>0");
                    catalogMetadata.prepend("<catalogFileCount>0");
                    catalogMetadata.prepend("<catalogSourcePath>EXPORT");
                }
                else if( result ==QMessageBox::No){
                    fileExtension="csv";
                }

                //Prepare export file name
                QDateTime now = QDateTime::currentDateTime();
                QString timestamp = now.toString(QLatin1String("yyyyMMdd-hhmmss"));
                QString fileNameWithoutExtension = QString::fromLatin1("search_results_%1").arg(timestamp);
                QString fileNameWithExtension = fileNameWithoutExtension + "." + fileExtension;
                fullFileName = collectionFolder+"/"+fileNameWithExtension;
                QFile exportFile(fullFileName);

                //Export search results to file
                if (exportFile.open(QFile::WriteOnly | QFile::Text)) {

                    QTextStream stream(&exportFile);

                    for (int i = 0; i < catalogMetadata.size(); ++i)
                    {
                                    stream << catalogMetadata[i] << '\n';
                    }
                    for (int i = 0; i < newSearch->sFileNames.size(); ++i)
                    {
                                    QString line = newSearch->sFilePaths[i] + "/" + newSearch->sFileNames[i] + "\t"
                                                   + QString::number(newSearch->sFileSizes[i]) + "\t"
                                                   + newSearch->sFileDateTimes[i] + "\t"
                                                   + newSearch->sFileCatalogs[i];
                                    stream << line << '\n';
                    }
                }
                exportFile.close();

                //Refresh catalogs
                loadCollection();

                //Select new catalog with results
                ui->Filters_label_DisplayCatalog->setText(fileNameWithoutExtension);
            }
            return fullFileName;
        }
        //----------------------------------------------------------------------
        void MainWindow::insertSearchHistoryToTable()
        {//Save Search to db

            QDateTime nowDateTime = QDateTime::currentDateTime();
            QString searchDateTime = nowDateTime.toString("yyyy-MM-dd hh:mm:ss");

            QSqlQuery query;
            QString querySQL = QLatin1String(R"(
                                INSERT INTO search(
                                    date_time,
                                    text_checked,
                                    text_phrase,
                                    text_criteria,
                                    text_search_in,
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
                                    selected_directory,
                                    text_exclude,
                                    case_sensitive
                                )
                                VALUES(
                                    :date_time,
                                    :text_checked,
                                    :text_phrase,
                                    :text_criteria,
                                    :text_search_in,
                                    :file_criteria_checked,
                                    :file_type_checked,
                                    :file_type,
                                    :file_size_checked,
                                    :file_size_min,
                                    :file_size_min_unit,
                                    :file_size_max,
                                    :file_size_max_unit,
                                    :date_modified_checked,
                                    :date_modified_min,
                                    :date_modified_max,
                                    :duplicates_checked,
                                    :duplicates_name,
                                    :duplicates_size,
                                    :duplicates_date_modified,
                                    :differences_checked,
                                    :differences_name,
                                    :differences_size,
                                    :differences_date_modified,
                                    :differences_catalogs,
                                    :folder_criteria_checked,
                                    :show_folders,
                                    :tag_checked,
                                    :tag,
                                    :search_location,
                                    :search_storage,
                                    :search_catalog,
                                    :search_catalog_checked,
                                    :search_directory_checked,
                                    :selected_directory,
                                    :text_exclude,
                                    :case_sensitive
                                )
                )");

            query.prepare(querySQL);
            query.bindValue(":date_time",                 searchDateTime);
            query.bindValue(":text_checked",              ui->Search_checkBox_FileName->isChecked());
            query.bindValue(":text_phrase",               ui->Search_lineEdit_SearchText->text());
            query.bindValue(":text_criteria",             newSearch->selectedTextCriteria);
            query.bindValue(":text_search_in",            newSearch->selectedSearchIn);
            query.bindValue(":file_criteria_checked",     ui->Search_checkBox_FileCriteria->isChecked());
            query.bindValue(":file_type_checked",         ui->Search_checkBox_Type->isChecked());
            query.bindValue(":file_type",                 newSearch->selectedFileType);
            query.bindValue(":file_size_checked",         ui->Search_checkBox_Size->isChecked());
            query.bindValue(":file_size_min",             newSearch->selectedMinimumSize);
            query.bindValue(":file_size_min_unit",        newSearch->selectedMinSizeUnit);
            query.bindValue(":file_size_max",             newSearch->selectedMaximumSize);
            query.bindValue(":file_size_max_unit",        newSearch->selectedMaxSizeUnit);
            query.bindValue(":date_modified_checked",     ui->Search_checkBox_Date->isChecked());
            query.bindValue(":date_modified_min",         ui->Search_dateTimeEdit_Min->dateTime().toString("yyyy/MM/dd hh:mm:ss"));
            query.bindValue(":date_modified_max",         ui->Search_dateTimeEdit_Max->dateTime().toString("yyyy/MM/dd hh:mm:ss"));
            query.bindValue(":duplicates_checked",        ui->Search_checkBox_Duplicates->isChecked());
            query.bindValue(":duplicates_name",           ui->Search_checkBox_DuplicatesName->isChecked());
            query.bindValue(":duplicates_size",           ui->Search_checkBox_DuplicatesSize->isChecked());
            query.bindValue(":duplicates_date_modified",  ui->Search_checkBox_DuplicatesDateModified->isChecked());
            query.bindValue(":differences_checked",       ui->Search_checkBox_Differences->isChecked());
            query.bindValue(":differences_name",          ui->Search_checkBox_DifferencesName->isChecked());
            query.bindValue(":differences_size",          ui->Search_checkBox_DifferencesSize->isChecked());
            query.bindValue(":differences_date_modified", ui->Search_checkBox_DifferencesDateModified->isChecked());
            query.bindValue(":differences_catalogs",      newSearch->differencesCatalog1+"||"+newSearch->differencesCatalog2);
            query.bindValue(":folder_criteria_checked",   ui->Search_checkBox_FolderCriteria->isChecked());
            query.bindValue(":show_folders",              ui->Search_checkBox_ShowFolders->isChecked());
            query.bindValue(":tag_checked",               ui->Search_checkBox_Tags->isChecked());
            query.bindValue(":tag",                       ui->Search_comboBox_Tags->currentText());
            query.bindValue(":search_location",           selectedFilterStorageLocation);
            query.bindValue(":search_storage",            selectedFilterStorageName);
            query.bindValue(":search_catalog",            selectedFilterCatalogName);
            query.bindValue(":search_catalog_checked",    ui->Filters_checkBox_SearchInCatalogs->isChecked());
            query.bindValue(":search_directory_checked",  ui->Filters_checkBox_SearchInConnectedDrives->isChecked());
            query.bindValue(":selected_directory",        ui->Filters_lineEdit_SeletedDirectory->text());
            query.bindValue(":text_exclude",              ui->Search_lineEdit_Exclude->text());
            query.bindValue(":case_sensitive",            ui->Search_checkBox_CaseSensitive->isChecked());
            query.exec();
        }
        //----------------------------------------------------------------------
        void MainWindow::saveSearchHistoryTableToFile()
        {
            //Prepare export
            QFile searchFile(searchHistoryFilePath);
            if(searchFile.open(QIODevice::WriteOnly | QIODevice::Truncate)) {

                QTextStream out(&searchFile);

                //Get data
                QSqlQuery query;
                QString querySQL = QLatin1String(R"(
                                    SELECT
                                        date_time,
                                        text_checked,
                                        text_phrase,
                                        text_criteria,
                                        text_search_in,
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
                                        show_folders,
                                        tag_checked,
                                        tag,
                                        search_location,
                                        search_storage,
                                        search_catalog,
                                        search_catalog_checked,
                                        search_directory_checked,
                                        selected_directory,
                                        text_exclude,
                                        case_sensitive,
                                        differences_checked,
                                        differences_name,
                                        differences_size,
                                        differences_date_modified,
                                        differences_catalogs,
                                        file_type_checked,
                                        file_criteria_checked,
                                        folder_criteria_checked
                                    FROM search
                                    ORDER BY date_time DESC
                                   )");
                query.prepare(querySQL);
                query.exec();

                //Iterate the result and write each line
                while (query.next()) {
                    const QSqlRecord record = query.record();
                    for (int i=0, recCount = record.count() ; i<recCount ; ++i){
                        if (i>0)
                        out << '\t';
                        out << record.value(i).toString();
                    }
                    out << '\n';
                }
            //searchFile.close();
            }
            searchFile.close();
        }
        //--------------------------------------------------------------------------
        void MainWindow::loadSearchHistoryFileToTable()
        {
            //Define storage file and prepare stream
            QFile searchFile(searchHistoryFilePath);
            QTextStream textStream(&searchFile);

            QSqlQuery queryDelete;
            queryDelete.prepare( "DELETE FROM search" );

            //Open file or return information
            if(!searchFile.open(QIODevice::ReadOnly)) {
                return;
            }
            //Clear all entries of the current table
            queryDelete.exec();

            while (true)
            {
                QString line = textStream.readLine();
                if (line.isNull())
                    break;
                else
                    if (line.left(2)!="ID"){//test the validity of the file

                        //Split the string with tabulation into a list
                        QStringList fieldList = line.split('\t');

                        //add empty values to support the addition of new fields to files from older versions
                        int  targetFieldsCount = 37;
                        int currentFiledsCount = fieldList.count();
                        int    diffFieldsCount = targetFieldsCount - currentFiledsCount;
                        if(diffFieldsCount !=0){
                            for(int i=0; i<diffFieldsCount; i++){
                                fieldList.append("");
                            }
                        }

                        QSqlQuery insertQuery;
                        QString insertQuerySQL = QLatin1String(R"(
                                        INSERT INTO search(
                                            date_time,
                                            text_checked,
                                            text_phrase,
                                            text_criteria,
                                            text_search_in,
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
                                            show_folders,
                                            tag_checked,
                                            tag,
                                            search_location,
                                            search_storage,
                                            search_catalog,
                                            search_catalog_checked,
                                            search_directory_checked,
                                            selected_directory,
                                            text_exclude,
                                            case_sensitive,
                                            file_type_checked,
                                            file_criteria_checked,
                                            folder_criteria_checked
                                            )
                                        VALUES(
                                            :date_time,
                                            :text_checked,
                                            :text_phrase,
                                            :text_criteria,
                                            :text_search_in,
                                            :file_type,
                                            :file_size_checked,
                                            :file_size_min,
                                            :file_size_min_unit,
                                            :file_size_max,
                                            :file_size_max_unit,
                                            :date_modified_checked,
                                            :date_modified_min,
                                            :date_modified_max,
                                            :duplicates_checked,
                                            :duplicates_name,
                                            :duplicates_size,
                                            :duplicates_date_modified,
                                            :differences_checked,
                                            :differences_name,
                                            :differences_size,
                                            :differences_date_modified,
                                            :differences_catalogs,
                                            :show_folders,
                                            :tag_checked,
                                            :tag,
                                            :search_location,
                                            :search_storage,
                                            :search_catalog,
                                            :search_catalog_checked,
                                            :search_directory_checked,
                                            :selected_directory,
                                            :text_exclude,
                                            :case_sensitive,
                                            :file_type_checked,
                                            :file_criteria_checked,
                                            :folder_criteria_checked
                                            )
                                        )");

                        insertQuery.prepare(insertQuerySQL);
                        insertQuery.bindValue(":date_time",                 fieldList[0]);
                        insertQuery.bindValue(":text_checked",              fieldList[1]);
                        insertQuery.bindValue(":text_phrase",               fieldList[2]);
                        insertQuery.bindValue(":text_criteria",             fieldList[3]);
                        insertQuery.bindValue(":text_search_in",            fieldList[4]);
                        insertQuery.bindValue(":file_type",                 fieldList[5]);
                        insertQuery.bindValue(":file_size_checked",         fieldList[6]);
                        insertQuery.bindValue(":file_size_min",             fieldList[7]);
                        insertQuery.bindValue(":file_size_min_unit",        fieldList[8]);
                        insertQuery.bindValue(":file_size_max",             fieldList[9]);
                        insertQuery.bindValue(":file_size_max_unit",        fieldList[10]);
                        insertQuery.bindValue(":date_modified_checked",     fieldList[11]);
                        insertQuery.bindValue(":date_modified_min",         fieldList[12]);
                        insertQuery.bindValue(":date_modified_max",         fieldList[13]);
                        insertQuery.bindValue(":duplicates_checked",        fieldList[14]);
                        insertQuery.bindValue(":duplicates_name",           fieldList[15]);
                        insertQuery.bindValue(":duplicates_size",           fieldList[16]);
                        insertQuery.bindValue(":duplicates_date_modified",  fieldList[17]);
                        insertQuery.bindValue(":show_folders",              fieldList[18]);
                        insertQuery.bindValue(":tag_checked",               fieldList[19]);
                        insertQuery.bindValue(":tag",                       fieldList[20]);
                        insertQuery.bindValue(":search_location",           fieldList[21]);
                        insertQuery.bindValue(":search_storage",            fieldList[22]);
                        insertQuery.bindValue(":search_catalog",            fieldList[23]);
                        insertQuery.bindValue(":search_catalog_checked",    fieldList[24]);
                        insertQuery.bindValue(":search_directory_checked",  fieldList[25]);
                        insertQuery.bindValue(":selected_directory",        fieldList[26]);
                        insertQuery.bindValue(":text_exclude",              fieldList[27]);
                        insertQuery.bindValue(":case_sensitive",            fieldList[28]);
                        insertQuery.bindValue(":differences_checked",       fieldList[29]);
                        insertQuery.bindValue(":differences_name",          fieldList[30]);
                        insertQuery.bindValue(":differences_size",          fieldList[31]);
                        insertQuery.bindValue(":differences_date_modified", fieldList[32]);
                        insertQuery.bindValue(":differences_catalogs",      fieldList[33]);
                        insertQuery.bindValue(":file_type_checked",         fieldList[34]);
                        insertQuery.bindValue(":file_criteria_checked",     fieldList[35]);
                        insertQuery.bindValue(":folder_criteria_checked",   fieldList[36]);
                        insertQuery.exec();
                    }
            }
            searchFile.close();
        }
        //--------------------------------------------------------------------------
        void MainWindow::loadSearchHistoryTableToModel()
        {
            QSqlQuery querySearchHistory;
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
