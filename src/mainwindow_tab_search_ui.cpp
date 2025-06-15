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
// Purpose:     methods for the screen SEARCH beside the search process
// Description: https://stephanecouturier.github.io/Katalog/docs/Features/Search
// Author:      Stephane Couturier
/////////////////////////////////////////////////////////////////////////////
*/

#include "mainwindow.h"
#include "ui_mainwindow.h"
#include "core/catalog.h"

//TAB: SEARCH FILES ------------------------------------------------------------

    //User interactions

        //Buttons and other changes
        void MainWindow::on_Search_lineEdit_SearchText_returnPressed()
        {
            launchSearch();
        }
        //----------------------------------------------------------------------
        // In mainwindow_tab_search_ui.cpp - modify the search button handler:

        void MainWindow::on_Search_pushButton_Search_clicked()
        {
            qDebug() << "=== Search/Pause/Resume button clicked ===";

            // Add defensive checks to prevent crashes
            if (!collection) {
                qWarning() << "Collection is null, cannot start search";
                return;
            }

            if (!selectedDevice) {
                qWarning() << "Selected device is null, cannot start search";
                return;
            }

            // Ensure searchMemory exists before using it
            if (!searchMemory) {
                qDebug() << "Creating searchMemory object";
                searchMemory = new SearchMemory(this);
            }

            // Ensure loadSearch exists before using it
            if (!loadSearch) {
                qDebug() << "Creating loadSearch object";
                loadSearch = new SearchMemory(this);
            }

            QString buttonText = ui->Search_pushButton_Search->text();

            if (buttonText == tr("Search") || buttonText == "Search") {
                // IDLE STATE: Start new search
                qDebug() << "Starting new search";

                try {
                    launchSearch();
                } catch (const std::exception& e) {
                    qWarning() << "Exception in launchSearch():" << e.what();
                    QApplication::restoreOverrideCursor();
                } catch (...) {
                    qWarning() << "Unknown exception in launchSearch()";
                    QApplication::restoreOverrideCursor();
                }
            }
            else if (buttonText == tr("Pause") || buttonText == "Pause") {
                // RUNNING STATE: Pause the search
                qDebug() << "Pausing search";
                pauseCurrentSearch();
            }
            else if (buttonText == tr("Resume") || buttonText == "Resume") {
                // PAUSED STATE: Resume the search
                qDebug() << "Resuming search";
                resumeCurrentSearch();
            }
        }
        //----------------------------------------------------------------------
        void MainWindow::on_Search_pushButton_Stop_clicked()
        {
            qDebug() << "=== Stop button clicked ===";

            // Stop any running search
            if (searchManager && searchManager->searchRunning()) {
                qDebug() << "Stopping active search via SearchManager";
                searchManager->stopSearch();
            } else {
                qDebug() << "No active search, but user clicked Stop - just reset buttons";
                setSearchStateIdle();
            }
        }
        //----------------------------------------------------------------------
        void MainWindow::on_Search_treeView_CatalogsFound_clicked(const QModelIndex &index)
        {
            //Refine the seach with the selection of one of the catalogs that have results

            //Get file from selected row
            selectedDevice->type= "Catalog";
            selectedDevice->ID = ui->Search_treeView_CatalogsFound->model()->index(index.row(), 1, QModelIndex()).data().toInt();
            selectedDevice->loadDevice("defaultConnection");
            displaySelectedDeviceName();

            //Seach again but only on the selected catalog
            launchSearch();
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
        void MainWindow::on_Search_comboBox_TextCriteria_currentIndexChanged(int index)
        {
            // "Begin With" can only be used in "File names only"
            ui->Search_comboBox_SearchIn->clear();

            if (index==2) { //"Begin With" is selected
                ui->Search_comboBox_SearchIn->addItem(tr("File names only"));
            }
            else {
                ui->Search_comboBox_SearchIn->addItem(tr("File names only"));
                ui->Search_comboBox_SearchIn->addItem(tr("File names or Folder paths"));
                ui->Search_comboBox_SearchIn->addItem(tr("Folder path only"));
            }
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

            //Ensure at least 1 checkbox is checked, by default the first one
            if(checked==1 and ui->Search_checkBox_DuplicatesName->isChecked() == false and
               ui->Search_checkBox_DuplicatesSize->isChecked() == false and
               ui->Search_checkBox_DuplicatesDateModified->isChecked() == false){
                ui->Search_checkBox_DuplicatesName->setChecked(true);
            }
        }
        void MainWindow::on_Search_checkBox_DuplicatesName_checkStateChanged(const Qt::CheckState &arg1)
        {
            //Leave it checked if size and date are unchecked
            if(arg1==Qt::Unchecked){
                if(ui->Search_checkBox_DuplicatesSize->checkState()==Qt::Unchecked && ui->Search_checkBox_DuplicatesDateModified->checkState()==Qt::Unchecked){
                    ui->Search_checkBox_DuplicatesName->setCheckState(Qt::Checked);
                }
            }
        }
        void MainWindow::on_Search_checkBox_DuplicatesSize_checkStateChanged(const Qt::CheckState &arg1)
        {
            //Leave it checked if name and date are unchecked
            if(arg1==Qt::Unchecked){
                if(ui->Search_checkBox_DuplicatesName->checkState()==Qt::Unchecked && ui->Search_checkBox_DuplicatesDateModified->checkState()==Qt::Unchecked){
                    ui->Search_checkBox_DuplicatesSize->setCheckState(Qt::Checked);
                }
            }
        }
        void MainWindow::on_Search_checkBox_DuplicatesDateModified_checkStateChanged(const Qt::CheckState &arg1)
        {
            //Leave it checked if name and size are unchecked
            if(arg1==Qt::Unchecked){
                if(ui->Search_checkBox_DuplicatesName->checkState()==Qt::Unchecked && ui->Search_checkBox_DuplicatesSize->checkState()==Qt::Unchecked){
                    ui->Search_checkBox_DuplicatesDateModified->setCheckState(Qt::Checked);
                }
            }
        }
        //----------------------------------------------------------------------
        void MainWindow::on_Search_checkBox_Differences_toggled(bool checked)
        {
            if(checked==1){
                ui->Search_checkBox_DifferencesName->setEnabled(true);
                ui->Search_checkBox_DifferencesSize->setEnabled(true);
                ui->Search_checkBox_DifferencesDateModified->setEnabled(true);
                ui->Search_widget_DifferencesDevices->setHidden(false);
                ui->Search_checkBox_ShowFolders->setChecked(false);
                ui->Search_checkBox_Duplicates->setChecked(false);
                ui->Search_checkBox_DuplicatesName->setEnabled(false);
                ui->Search_checkBox_DuplicatesSize->setEnabled(false);
                ui->Search_checkBox_DuplicatesDateModified->setEnabled(false);
                ui->Search_treeView_CatalogsFound->setEnabled(false);
            }
            else{
                ui->Search_widget_DifferencesDevices->setHidden(true);
                ui->Search_checkBox_DifferencesName->setDisabled(true);
                ui->Search_checkBox_DifferencesSize->setDisabled(true);
                ui->Search_checkBox_DifferencesDateModified->setDisabled(true);
                ui->Search_treeView_CatalogsFound->setEnabled(true);
            }

            //ensure at least 1 checkbox is checked, by default the first one
            if(checked==1 and ui->Search_checkBox_DifferencesName->isChecked() == false and
                ui->Search_checkBox_DifferencesSize->isChecked() == false and
                ui->Search_checkBox_DifferencesDateModified->isChecked() == false){
                ui->Search_checkBox_DifferencesName->setChecked(true);
            }
        }
        void MainWindow::on_Search_checkBox_DifferencesName_checkStateChanged(const Qt::CheckState &arg1)
        {
            //Leave it checked if size and date are unchecked
            if(arg1==Qt::Unchecked){
                if(ui->Search_checkBox_DifferencesSize->checkState()==Qt::Unchecked && ui->Search_checkBox_DifferencesDateModified->checkState()==Qt::Unchecked){
                    ui->Search_checkBox_DifferencesName->setCheckState(Qt::Checked);
                }
            }
        }
        void MainWindow::on_Search_checkBox_DifferencesSize_checkStateChanged(const Qt::CheckState &arg1)
        {
            //Leave it checked if name and date are unchecked
            if(arg1==Qt::Unchecked){
                if(ui->Search_checkBox_DifferencesName->checkState()==Qt::Unchecked && ui->Search_checkBox_DifferencesDateModified->checkState()==Qt::Unchecked){
                    ui->Search_checkBox_DifferencesSize->setCheckState(Qt::Checked);
                }
            }
        }
        void MainWindow::on_Search_checkBox_DifferencesDateModified_checkStateChanged(const Qt::CheckState &arg1)
        {
            //Leave it checked if name and size are unchecked
            if(arg1==Qt::Unchecked){
                if(ui->Search_checkBox_DifferencesName->checkState()==Qt::Unchecked && ui->Search_checkBox_DifferencesSize->checkState()==Qt::Unchecked){
                    ui->Search_checkBox_DifferencesDateModified->setCheckState(Qt::Checked);
                }
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
            loadSearch = new SearchMemory;
            loadSearch->searchDateTime = ui->Search_treeView_History->model()->index(index.row(), 0, QModelIndex()).data().toString();
            loadSearch->loadSearchHistoryCriteria("defaultConnection");
            loadSearchCriteria(loadSearch);
        }
        //----------------------------------------------------------------------
        void MainWindow::on_Search_pushButton_FileFoundMoreStatistics_clicked()
        {
            reportSearchStatistics();
        }
        //----------------------------------------------------------------------
        void MainWindow::on_SearchTreeViewFilesFoundHeaderSortOrderChanged(){

            QSettings settings(collection->settingsFilePath, QSettings:: IniFormat);
            QHeaderView *searchTreeHeader = ui->Search_treeView_FilesFound->header();

            lastSearchSortSection = searchTreeHeader->sortIndicatorSection();
            lastSearchSortOrder   = searchTreeHeader->sortIndicatorOrder();

            settings.setValue("Search/lastSearchSortSection", QLocale().toString(lastSearchSortSection));
            settings.setValue("Search/lastSearchSortOrder",   QLocale().toString(lastSearchSortOrder));
        }
        //----------------------------------------------------------------------
        void MainWindow::on_SearchTreeViewHistoryHeaderSortOrderChanged(){

            QSettings settings(collection->settingsFilePath, QSettings:: IniFormat);
            QHeaderView *searchHistoryTreeHeader = ui->Search_treeView_History->header();

            lastSearchHistorySortSection = searchHistoryTreeHeader->sortIndicatorSection();
            lastSearchHistorySortOrder   = searchHistoryTreeHeader->sortIndicatorOrder();

            settings.setValue("Search/lastSearchHistorySortSection", QLocale().toString(lastSearchHistorySortSection));
            settings.setValue("Search/lastSearchHistorySortOrder",   QLocale().toString(lastSearchHistorySortOrder));
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
            if(currentSearch->showFoldersOnly==false){
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
            if(currentSearch->showFoldersOnly==false){
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
        void MainWindow::initiateSearchFields()
        {
            // Add internal constants for TextCriteria:
            ui->Search_comboBox_TextCriteria->setItemData(0, Search::TEXT_CRITERIA_ALL_WORDS,    Qt::UserRole);
            ui->Search_comboBox_TextCriteria->setItemData(1, Search::TEXT_CRITERIA_EXACT_PHRASE, Qt::UserRole);
            ui->Search_comboBox_TextCriteria->setItemData(2, Search::TEXT_CRITERIA_BEGINS_WITH,  Qt::UserRole);
            ui->Search_comboBox_TextCriteria->setItemData(3, Search::TEXT_CRITERIA_ANY_WORD,     Qt::UserRole);

            // Add internal constants for SearchIn:
            ui->Search_comboBox_SearchIn->setItemData(0, Search::SEARCH_IN_FILE_NAMES,        Qt::UserRole);
            ui->Search_comboBox_SearchIn->setItemData(1, Search::SEARCH_IN_FILES_AND_FOLDERS, Qt::UserRole);
            ui->Search_comboBox_SearchIn->setItemData(2, Search::SEARCH_IN_FOLDER_PATH,       Qt::UserRole);

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
            ui->Search_comboBox_MinSizeUnit->setItemData(0, Search::SIZE_UNIT_TIB,   Qt::UserRole);
            ui->Search_comboBox_MinSizeUnit->setItemData(1, Search::SIZE_UNIT_GIB,   Qt::UserRole);
            ui->Search_comboBox_MinSizeUnit->setItemData(2, Search::SIZE_UNIT_MIB,   Qt::UserRole);
            ui->Search_comboBox_MinSizeUnit->setItemData(3, Search::SIZE_UNIT_KIB,   Qt::UserRole);
            ui->Search_comboBox_MinSizeUnit->setItemData(4, Search::SIZE_UNIT_BYTES, Qt::UserRole);

            ui->Search_comboBox_MaxSizeUnit->addItem(tr("TiB"));
            ui->Search_comboBox_MaxSizeUnit->addItem(tr("GiB"));
            ui->Search_comboBox_MaxSizeUnit->addItem(tr("MiB"));
            ui->Search_comboBox_MaxSizeUnit->addItem(tr("KiB"));
            ui->Search_comboBox_MaxSizeUnit->addItem(tr("Bytes"));
            ui->Search_comboBox_MaxSizeUnit->setItemData(0, Search::SIZE_UNIT_TIB,   Qt::UserRole);
            ui->Search_comboBox_MaxSizeUnit->setItemData(1, Search::SIZE_UNIT_GIB,   Qt::UserRole);
            ui->Search_comboBox_MaxSizeUnit->setItemData(2, Search::SIZE_UNIT_MIB,   Qt::UserRole);
            ui->Search_comboBox_MaxSizeUnit->setItemData(3, Search::SIZE_UNIT_KIB,   Qt::UserRole);
            ui->Search_comboBox_MaxSizeUnit->setItemData(4, Search::SIZE_UNIT_BYTES, Qt::UserRole);

            //Load last search values (from settings file)
            if (currentSearch->selectedMaximumSize == 0){
                 currentSearch->selectedMaximumSize = 1000;}

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
            ui->Search_lineEdit_Exclude->setText("");

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
            ui->Search_checkBox_DuplicatesName->setChecked(true);
            ui->Search_checkBox_DuplicatesSize->setChecked(false);
            ui->Search_checkBox_DuplicatesDateModified->setChecked(false);
            ui->Search_checkBox_Differences->setChecked(false);
            ui->Search_checkBox_DifferencesName->setChecked(true);
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
            ui->Search_pushButton_ProcessResults->setEnabled(false);
            ui->Search_comboBox_SelectProcess->setEnabled(false);
        }
        //----------------------------------------------------------------------
        void MainWindow::loadSearchCriteria(Search *search)
        {//Set search values

            //Selections
            ui->Filters_checkBox_SearchInCatalogs->setChecked(search->searchInCatalogsChecked);
            ui->Filters_checkBox_SearchInConnectedDrives->setChecked(search->searchInConnectedChecked);
            ui->Filters_lineEdit_SeletedDirectory->setText(search->connectedDirectory);
            ui->Filters_label_DisplayStorage->setText(search->selectedStorage);
            ui->Filters_label_DisplayCatalog->setText(search->selectedCatalog);

            //File name
            ui->Search_checkBox_FileCriteria->setChecked(search->searchOnFileCriteria);
            ui->Search_checkBox_FileName->setChecked(search->searchOnFileName);
            ui->Search_lineEdit_SearchText->setText(search->searchText);
            int textCriteriaIndex = search->mapTextCriteriaToComboBoxIndex(search->selectedTextCriteria);
            ui->Search_comboBox_TextCriteria->setCurrentIndex(textCriteriaIndex);
            int comboIndex = currentSearch->mapSearchInToComboBoxIndex(search->selectedSearchIn);
            ui->Search_comboBox_SearchIn->setCurrentIndex(comboIndex);
            ui->Search_checkBox_CaseSensitive->setChecked(search->caseSensitive);
            ui->Search_lineEdit_Exclude->setText(search->selectedSearchExclude);

            //File criteria
            ui->Search_checkBox_Size->setChecked(search->searchOnSize);
            ui->Search_spinBox_MinimumSize->setValue(search->selectedMinimumSize);
            ui->Search_spinBox_MaximumSize->setValue(search->selectedMaximumSize);
            int minSizeIndex = search->mapSizeUnitToComboBoxIndex(search->selectedMinSizeUnit);
            ui->Search_comboBox_MinSizeUnit->setCurrentIndex(minSizeIndex);
            int maxSizeIndex = search->mapSizeUnitToComboBoxIndex(search->selectedMaxSizeUnit);
            ui->Search_comboBox_MaxSizeUnit->setCurrentIndex(maxSizeIndex);
            ui->Search_checkBox_Type->setChecked(search->searchOnType);
            ui->Search_comboBox_FileType->setCurrentText(tr(search->selectedFileType.toUtf8()));
            ui->Search_checkBox_Date->setChecked(search->searchOnDate);
            ui->Search_dateTimeEdit_Min->setDateTime(search->selectedDateMin);
            ui->Search_dateTimeEdit_Max->setDateTime(search->selectedDateMax);

            ui->Search_checkBox_Duplicates->setChecked(search->searchOnDuplicates);
            ui->Search_checkBox_DuplicatesName->setChecked(search->searchDuplicatesOnName);
            ui->Search_checkBox_DuplicatesSize->setChecked(search->searchDuplicatesOnSize);
            ui->Search_checkBox_DuplicatesDateModified->setChecked(search->searchDuplicatesOnDate);
            ui->Search_checkBox_Differences->setChecked(search->searchOnDifferences);

            ui->Search_checkBox_DifferencesName->setChecked(search->differencesOnName);
            ui->Search_checkBox_DifferencesSize->setChecked(search->differencesOnSize);
            ui->Search_checkBox_DifferencesDateModified->setChecked(search->differencesOnDate);
            ui->Search_checkBox_DifferencesName->setChecked(search->differencesOnName); //Re-apply the state

            //Select the element in ui->Search_comboBox_DifferencesCatalog1 matching the differencesDeviceID1
            for (int i = 0; i < ui->Search_comboBox_DifferencesDevice1->count(); i++) {
                if (ui->Search_comboBox_DifferencesDevice1->itemData(i).toInt() == search->differencesDeviceID1) {
                    ui->Search_comboBox_DifferencesDevice1->setCurrentIndex(i);
                    break;
                }
            }
            for (int i = 0; i < ui->Search_comboBox_DifferencesDevice2->count(); i++) {
                if (ui->Search_comboBox_DifferencesDevice2->itemData(i).toInt() == search->differencesDeviceID2) {
                    ui->Search_comboBox_DifferencesDevice2->setCurrentIndex(i);
                    break;
                }
            }

            //Restore the tpye of search (in catalogs or in connected drives
            if (lastSearch->searchInCatalogsChecked == true){
                ui->Filters_checkBox_SearchInCatalogs->setChecked(true);
                ui->Filters_checkBox_SearchInConnectedDrives->setChecked(false);
                ui->Filters_lineEdit_SeletedDirectory->setDisabled(true);
            }
            else{
                ui->Filters_checkBox_SearchInCatalogs->setChecked(false);
                ui->Filters_checkBox_SearchInConnectedDrives->setChecked(true);
                ui->Filters_lineEdit_SeletedDirectory->setEnabled(true);
            }

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
                currentSearch = nullptr;
                currentSearch->searchOnFileName         = ui->Search_checkBox_FileName->isChecked();
                currentSearch->searchText               = ui->Search_lineEdit_SearchText->text();
                int textCriteriaIndex = ui->Search_comboBox_TextCriteria->currentIndex();
                switch (textCriteriaIndex) {
                    case 0: currentSearch->selectedTextCriteria = Search::TEXT_CRITERIA_ALL_WORDS; break;
                    case 1: currentSearch->selectedTextCriteria = Search::TEXT_CRITERIA_EXACT_PHRASE; break;
                    case 2: currentSearch->selectedTextCriteria = Search::TEXT_CRITERIA_BEGINS_WITH; break;
                    case 3: currentSearch->selectedTextCriteria = Search::TEXT_CRITERIA_ANY_WORD; break;
                default: currentSearch->selectedTextCriteria = Search::TEXT_CRITERIA_ALL_WORDS; break;
                }
                int searchInIndex = ui->Search_comboBox_SearchIn->currentIndex();
                switch (searchInIndex) {
                    case 0: currentSearch->selectedSearchIn = Search::SEARCH_IN_FILE_NAMES; break;
                    case 1: currentSearch->selectedSearchIn = Search::SEARCH_IN_FILES_AND_FOLDERS; break;
                    case 2: currentSearch->selectedSearchIn = Search::SEARCH_IN_FOLDER_PATH; break;
                    default: currentSearch->selectedSearchIn = Search::SEARCH_IN_FILES_AND_FOLDERS; break;
                }
                currentSearch->caseSensitive            = ui->Search_checkBox_CaseSensitive->isChecked();
                currentSearch->selectedSearchExclude    = ui->Search_lineEdit_Exclude->text();

                currentSearch->searchOnFileCriteria     = ui->Search_checkBox_FileCriteria->isChecked();
                currentSearch->searchOnSize             = ui->Search_checkBox_Size->isChecked();
                currentSearch->selectedMinimumSize      = ui->Search_spinBox_MinimumSize->value();
                currentSearch->selectedMaximumSize      = ui->Search_spinBox_MaximumSize->value();
                int minSizeUnitIndex = ui->Search_comboBox_MinSizeUnit->currentIndex();
                switch (minSizeUnitIndex) {
                    case 0: currentSearch->selectedMinSizeUnit = Search::SIZE_UNIT_BYTES; break;
                    case 1: currentSearch->selectedMinSizeUnit = Search::SIZE_UNIT_KIB; break;
                    case 2: currentSearch->selectedMinSizeUnit = Search::SIZE_UNIT_MIB; break;
                    case 3: currentSearch->selectedMinSizeUnit = Search::SIZE_UNIT_GIB; break;
                    case 4: currentSearch->selectedMinSizeUnit = Search::SIZE_UNIT_TIB; break;
                    default: currentSearch->selectedMinSizeUnit = Search::SIZE_UNIT_BYTES; break;
                }

                int maxSizeUnitIndex = ui->Search_comboBox_MaxSizeUnit->currentIndex();
                switch (maxSizeUnitIndex) {
                    case 0: currentSearch->selectedMaxSizeUnit = Search::SIZE_UNIT_BYTES; break;
                    case 1: currentSearch->selectedMaxSizeUnit = Search::SIZE_UNIT_KIB; break;
                    case 2: currentSearch->selectedMaxSizeUnit = Search::SIZE_UNIT_MIB; break;
                    case 3: currentSearch->selectedMaxSizeUnit = Search::SIZE_UNIT_GIB; break;
                    case 4: currentSearch->selectedMaxSizeUnit = Search::SIZE_UNIT_TIB; break;
                    default: currentSearch->selectedMaxSizeUnit = Search::SIZE_UNIT_BYTES; break;
                }
                currentSearch->setMultipliers();
                currentSearch->searchOnType             = ui->Search_checkBox_Type->isChecked();
                currentSearch->selectedFileType         = ui->Search_comboBox_FileType->itemData(ui->Search_comboBox_FileType->currentIndex(),Qt::UserRole).toString();
                currentSearch->searchOnDate             = ui->Search_checkBox_Date->isChecked();
                currentSearch->selectedDateMin          = ui->Search_dateTimeEdit_Min->dateTime();
                currentSearch->selectedDateMax          = ui->Search_dateTimeEdit_Max->dateTime();
                currentSearch->searchOnDuplicates       = ui->Search_checkBox_Duplicates->isChecked();
                currentSearch->searchDuplicatesOnName   = ui->Search_checkBox_DuplicatesName->isChecked();
                currentSearch->searchDuplicatesOnSize   = ui->Search_checkBox_DuplicatesSize->isChecked();
                currentSearch->searchDuplicatesOnDate   = ui->Search_checkBox_DuplicatesDateModified->isChecked();
                currentSearch->searchOnDifferences      = ui->Search_checkBox_Differences->isChecked();
                currentSearch->differencesOnName        = ui->Search_checkBox_DifferencesName->checkState();
                currentSearch->differencesOnSize        = ui->Search_checkBox_DifferencesSize->checkState();
                currentSearch->differencesOnDate        = ui->Search_checkBox_DifferencesDateModified->checkState();
                currentSearch->differencesDeviceID1     = ui->Search_comboBox_DifferencesDevice1->currentData().toInt();
                currentSearch->differencesDeviceID2     = ui->Search_comboBox_DifferencesDevice2->currentData().toInt();
                currentSearch->differencesDevices << QString::number(currentSearch->differencesDeviceID1) << QString::number(currentSearch->differencesDeviceID2);

                currentSearch->searchOnFolderCriteria   = ui->Search_checkBox_FolderCriteria->isChecked();
                currentSearch->showFoldersOnly          = ui->Search_checkBox_ShowFolders->isChecked();
                currentSearch->searchOnTags             = ui->Search_checkBox_Tags->isChecked();
                currentSearch->selectedTagName          = ui->Search_comboBox_Tags->currentText();

                currentSearch->selectedStorage          = ui->Filters_label_DisplayStorage->text();
                currentSearch->selectedCatalog          = ui->Filters_label_DisplayCatalog->text();
                currentSearch->searchInCatalogsChecked  = ui->Filters_checkBox_SearchInCatalogs->isChecked();
                currentSearch->searchInConnectedChecked = ui->Filters_checkBox_SearchInConnectedDrives->isChecked();
                currentSearch->connectedDirectory       = ui->Filters_lineEdit_SeletedDirectory->text();

                currentSearch->fileType_AudioS = fileType_AudioS;
                currentSearch->fileType_ImageS = fileType_ImageS;
                currentSearch->fileType_TextS  = fileType_TextS;
                currentSearch->fileType_VideoS = fileType_VideoS;

                currentSearch->diffDevice1->ID = ui->Search_comboBox_DifferencesDevice1->currentData().toInt();
                currentSearch->diffDevice2->ID = ui->Search_comboBox_DifferencesDevice2->currentData().toInt();
        }
        //----------------------------------------------------------------------
        void MainWindow::refreshDifferencesCatalogSelection(){
            ui->Search_comboBox_DifferencesDevice1->clear();
            ui->Search_comboBox_DifferencesDevice2->clear();

            Device loopDevice;
            foreach(int ID, selectedDevice->deviceIDList)
            {
                loopDevice.ID = ID;
                loopDevice.loadDevice("defaultConnection");
                ui->Search_comboBox_DifferencesDevice1->addItem(loopDevice.name,loopDevice.ID);
                ui->Search_comboBox_DifferencesDevice2->addItem(loopDevice.name,loopDevice.ID);
            }
        }
        //----------------------------------------------------------------------
        void MainWindow::batchProcessSearchResults()
        {//Process all results according to user's choice

            //user process choice
            QString selectedProcess = ui->Search_comboBox_SelectProcess->currentText();

            //Generate list of full file path (directory path + file name)
            QStringList resultsFilesList;
            for (int i = 0; i < currentSearch->fileNames.size(); ++i)
            {
                QString fileFullPath = currentSearch->filePaths[i] + "/" + currentSearch->fileNames[i];
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

                if(currentSearch->showFoldersOnly==false){
                    QString trashPath;
                    QString fileFullPath;
                    qint64 movedFiles = 0;
                    if (QMessageBox::warning(this,
                                             tr("Confirmation"),
                                             "<span style='color:orange;font-weight: bold;'>"+tr("MOVE")+"</span><br/>"
                                                 +tr("Move all %1 files (%2) from these results to trash?").arg(QLocale().toString(currentSearch->filesFoundNumber), QLocale().formattedDataSize(currentSearch->filesFoundTotalSize)),
                                             QMessageBox::Yes|QMessageBox::Cancel)
                        == QMessageBox::Yes) {
                            for (int i = 0; i < currentSearch->fileNames.size(); ++i)
                            {
                            fileFullPath = moveFileToTrash(currentSearch->filePaths[i] + "/" + currentSearch->fileNames[i]);
                            trashPath = moveFileToTrash(fileFullPath);
                            if(trashPath==""){
                         QMessageBox::information(this,"Katalog",tr("Problem moving file: ")
                                                                       +"<br/>file-<a href='"+fileFullPath+"'>"+fileFullPath+"</a>");
                            }
                            else
                         movedFiles+=1;
                            }
                            QMessageBox::information(this,"Katalog",tr("%1 files were moved to trash, out of %2 files from the results.").arg(QLocale().toString(movedFiles), QLocale().toString(currentSearch->filesFoundNumber)));
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

                if(currentSearch->showFoldersOnly==false){
                    qint64 deletedFiles = 0;
                    QString result;

                    if (QMessageBox::critical(this,
                                              tr("Confirmation"),
                                              "<span style='color:red;font-weight: bold;'>"+tr("DELETE")+"</span><br/>"
                                              +tr("Delete permanently all %1 files (%2) from these results?").arg(QLocale().toString(currentSearch->filesFoundNumber), QLocale().formattedDataSize(currentSearch->filesFoundTotalSize)),
                                              QMessageBox::Yes|QMessageBox::Cancel)
                        == QMessageBox::Yes) {
                        for (int i = 0; i < currentSearch->fileNames.size(); ++i)
                        {
                            result = deleteFile(currentSearch->filePaths[i] + "/" + currentSearch->fileNames[i]);
                            if (result!=""){
                                deletedFiles +=1;
                            }
                            result="";
                        }
                        QMessageBox::information(this,"Katalog",tr("%1 files were deleted, out of %2 files from the results.").arg(QLocale().toString(deletedFiles), QLocale().toString(currentSearch->filesFoundNumber)));
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

                //Export to new Device/Catalog
                if ( result ==QMessageBox::Yes){

                    fileExtension="idx";

                    //Verify if a virtual device name "Search Results" exists, or create one as parent for search results
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

                    //Add Device entry and populate values
                    newDevice->generateDeviceID();
                    newDevice->type = "Catalog";
                    newDevice->name = fileNameWithoutExtension;
                        //newDevice->parentID = searchResultsHolder.ID;
                    newDevice->catalog->generateID();
                    newDevice->externalID = newDevice->catalog->ID;
                    newDevice->groupID = 1;
                    newDevice->path = "EXPORT"; //there is not 1 path for a given search that can be multi-catalog
                    newDevice->insertDevice();

                    //Get inputs and set values of the new Catalog
                    newDevice->catalog->sourcePath = currentSearch->searchDateTime; //passing a date instead of a path, as there is not 1 path for a given search that can be multi-catalog
                    newDevice->catalog->appVersion = currentVersion;
                    newDevice->catalog->sourcePath = "EXPORT";
                    //Save new catalog
                    newDevice->catalog->insertCatalog();
                    collection->saveDeviceTableToFile();

                    catalogMetadata.prepend("<catalogID>" + QLocale().toString(newDevice->catalog->ID));
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

                //Export search results to file table
                if ( result ==QMessageBox::Yes){
                    //Export list of files to the "file" table
                    for (int i = 0; i < currentSearch->fileNames.size(); ++i)
                    {
                        //Prepare insert query for file
                        QSqlQuery insertFileQuery(QSqlDatabase::database("defaultConnection"));
                        QString insertFileSQL = QLatin1String(R"(
                                                    INSERT INTO file (
                                                                    file_catalog_id,
                                                                    file_name,
                                                                    file_folder_path,
                                                                    file_size,
                                                                    file_date_updated,
                                                                    file_catalog,
                                                                    file_full_path
                                                                    )
                                                    VALUES(
                                                                    :file_catalog_id,
                                                                    :file_name,
                                                                    :file_folder_path,
                                                                    :file_size,
                                                                    :file_date_updated,
                                                                    :file_catalog,
                                                                    :file_full_path )
                                                    )");
                        insertFileQuery.prepare(insertFileSQL);

                        //Prepare insert query for folder
                        QSqlQuery insertFolderQuery(QSqlDatabase::database("defaultConnection"));
                        QString insertFolderSQL = QLatin1String(R"(
                                                    INSERT OR IGNORE INTO folder(
                                                        folder_catalog_id,
                                                        folder_path
                                                     )
                                                    VALUES(
                                                        :folder_catalog_id,
                                                        :folder_path)
                                                    )");
                        insertFolderQuery.prepare(insertFolderSQL);

                        //Insert root folder (so that it is displayed even when there are no sub-folders)
                        insertFolderQuery.prepare(insertFolderSQL);
                        insertFolderQuery.bindValue(":folder_catalog_id", newDevice->catalog->ID);
                        insertFolderQuery.bindValue(":folder_path", newDevice->catalog->sourcePath);
                        insertFolderQuery.exec();

                        //Insert dirs
                            insertFolderQuery.prepare(insertFolderSQL);
                            insertFolderQuery.bindValue(":folder_catalog_id", newDevice->catalog->ID);
                            insertFolderQuery.bindValue(":folder_path", currentSearch->filePaths[i]);
                            insertFolderQuery.exec();

                        //Insert files
                            //QFile file(currentSearch->filePaths[i]);
                            insertFileQuery.bindValue(":file_catalog_id",   newDevice->catalog->ID);
                            insertFileQuery.bindValue(":file_name",         currentSearch->fileNames[i]);
                            insertFileQuery.bindValue(":file_size",         QLocale().toString(currentSearch->fileSizes[i]));
                            insertFileQuery.bindValue(":file_folder_path",  currentSearch->filePaths[i]);
                            insertFileQuery.bindValue(":file_date_updated", currentSearch->fileDateTimes[i]);
                            insertFileQuery.bindValue(":file_catalog",      currentSearch->fileCatalogs[i]);
                            insertFileQuery.bindValue(":file_full_path",    currentSearch->filePaths[i]);
                            insertFileQuery.exec();

                            //Media File Metadata
                            //DEV: includeMetadata
                            //     if(includeMetadata == true){
                            //         setMediaFile(entryPath);
                            //     }
                    }
                }

                //Export search results to file table and the export file
                if (exportFile.open(QFile::WriteOnly | QFile::Text)) {

                    QTextStream stream(&exportFile);

                    //Export file metadata
                    for (int i = 0; i < catalogMetadata.size(); ++i)
                    {
                        stream << catalogMetadata[i] << '\n';
                    }

                    //Export list of files
                    for (int i = 0; i < currentSearch->fileNames.size(); ++i)
                    {
                        //Export to file
                        QString line = currentSearch->filePaths[i] + "/" + currentSearch->fileNames[i] + "\t"
                                       + QLocale().toString(currentSearch->fileSizes[i]) + "\t"
                                       + currentSearch->fileDateTimes[i] + "\t"
                                       + currentSearch->fileCatalogs[i];
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
        //--------------------------------------------------------------------------
        void MainWindow::clearSearchResults()
        {
            Catalog *empty = new Catalog(this);
            ui->Search_treeView_FilesFound->setModel(empty);
            QStandardItemModel *emptyQStandardItemModel = new QStandardItemModel;
            emptyQStandardItemModel->setHorizontalHeaderLabels({ tr("Catalog with results")});
            ui->Search_treeView_CatalogsFound->setModel(emptyQStandardItemModel);
            ui->Search_treeView_CatalogsFound->hideColumn(1);
        }
        //--------------------------------------------------------------------------
        void MainWindow::resetSearchButton()
        {
            qDebug() << "resetSearchButton() called";
            ui->Search_pushButton_Search->setText("Search");
            ui->Search_pushButton_Search->setIcon(QIcon::fromTheme("edit-find"));
            ui->Search_pushButton_Search->setStyleSheet("QPushButton{ background-color: #81d41a; }");
        }
