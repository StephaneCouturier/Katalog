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
            //File name
            ui->Search_lineEdit_SearchText->setText("");
            ui->Search_checkBox_FileCriteria->setEnabled(true);
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
            ui->Search_pushButton_ExportResults->setEnabled(false);

        }
        //----------------------------------------------------------------------
        void MainWindow::on_Search_pushButton_ExportResults_clicked()
        {          
            QString exportFileName = exportSearchResults();
            QMessageBox::information(this,"Katalog",tr("Results exported to the collection folder:")
                                                    +"<br/><a href='"+exportFileName+"'>"+exportFileName+"</a>");
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
        {
            //Restore the criteria of the selected search history
            searchOnFileCriteria         = ui->Search_treeView_History->model()->index(index.row(), 1, QModelIndex()).data().toBool();

            QString TextPhrase   = ui->Search_treeView_History->model()->index(index.row(), 2, QModelIndex()).data().toString();
//            #ifdef Q_OS_LINUX
//                    ui->Search_kcombobox_SearchText->setEditText(TextPhrase);
//            #else
                    ui->Search_lineEdit_SearchText->setText(TextPhrase);
//            #endif

            selectedTextCriteria = ui->Search_treeView_History->model()->index(index.row(), 3, QModelIndex()).data().toString();
            selectedSearchIn     = ui->Search_treeView_History->model()->index(index.row(), 4, QModelIndex()).data().toString();
            caseSensitive        = ui->Search_treeView_History->model()->index(index.row(), 5, QModelIndex()).data().toBool();
            selectedSearchExclude= ui->Search_treeView_History->model()->index(index.row(), 6, QModelIndex()).data().toString();
            selectedFileType     = ui->Search_treeView_History->model()->index(index.row(), 7, QModelIndex()).data().toString();
            searchOnSize         = ui->Search_treeView_History->model()->index(index.row(), 8, QModelIndex()).data().toBool();
            selectedMinimumSize  = ui->Search_treeView_History->model()->index(index.row(), 9, QModelIndex()).data().toInt();
            selectedMinSizeUnit  = ui->Search_treeView_History->model()->index(index.row(), 10, QModelIndex()).data().toString();
            selectedMaximumSize  = ui->Search_treeView_History->model()->index(index.row(), 11, QModelIndex()).data().toInt();
            selectedMaxSizeUnit  = ui->Search_treeView_History->model()->index(index.row(), 12, QModelIndex()).data().toString();
            searchOnDate         = ui->Search_treeView_History->model()->index(index.row(), 13, QModelIndex()).data().toBool();
            selectedDateMin      = ui->Search_treeView_History->model()->index(index.row(), 14, QModelIndex()).data().toDateTime();
            selectedDateMax      = ui->Search_treeView_History->model()->index(index.row(), 15, QModelIndex()).data().toDateTime();
            searchOnDuplicates   = ui->Search_treeView_History->model()->index(index.row(), 16, QModelIndex()).data().toBool();
            hasDuplicatesOnName  = ui->Search_treeView_History->model()->index(index.row(), 17, QModelIndex()).data().toBool();
            hasDuplicatesOnSize  = ui->Search_treeView_History->model()->index(index.row(), 18, QModelIndex()).data().toBool();
            hasDuplicatesOnDateModified = ui->Search_treeView_History->model()->index(index.row(), 19, QModelIndex()).data().toBool();
            searchOnDifferences  = ui->Search_treeView_History->model()->index(index.row(), 20, QModelIndex()).data().toBool();
            hasDifferencesOnName = ui->Search_treeView_History->model()->index(index.row(), 21, QModelIndex()).data().toBool();
            hasDifferencesOnSize = ui->Search_treeView_History->model()->index(index.row(), 22, QModelIndex()).data().toBool();
            hasDifferencesOnDateModified = ui->Search_treeView_History->model()->index(index.row(), 23, QModelIndex()).data().toBool();

            QStringList selectedDifferencesCatalogs = ui->Search_treeView_History->model()->index(index.row(), 24, QModelIndex()).data().toString().split("||");
            if (selectedDifferencesCatalogs.length()>1){
                selectedDifferencesCatalog1 = selectedDifferencesCatalogs[0];
                selectedDifferencesCatalog2 = selectedDifferencesCatalogs[1];
            }

            showFoldersOnly      = ui->Search_treeView_History->model()->index(index.row(), 25, QModelIndex()).data().toBool();
            searchOnTags         = ui->Search_treeView_History->model()->index(index.row(), 26, QModelIndex()).data().toBool();
            selectedTag          = ui->Search_treeView_History->model()->index(index.row(), 27, QModelIndex()).data().toString();

            searchInFileCatalogsChecked   = ui->Search_treeView_History->model()->index(index.row(), 31, QModelIndex()).data().toBool();
            searchInConnectedDriveChecked = ui->Search_treeView_History->model()->index(index.row(), 32, QModelIndex()).data().toBool();
            selectedDirectoryName = ui->Search_treeView_History->model()->index(index.row(), 33, QModelIndex()).data().toString();

            initiateSearchValues();

            selectedFilterStorageLocation  = ui->Search_treeView_History->model()->index(index.row(), 28, QModelIndex()).data().toString();
            ui->Filters_label_DisplayLocation->setText(selectedFilterStorageLocation);

            selectedFilterStorageName   = ui->Search_treeView_History->model()->index(index.row(), 29, QModelIndex()).data().toString();
            ui->Filters_label_DisplayStorage->setText(selectedFilterStorageName);

            selectedFilterCatalogName   = ui->Search_treeView_History->model()->index(index.row(), 30, QModelIndex()).data().toString();
            ui->Filters_label_DisplayCatalog->setText(selectedFilterCatalogName);

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

            QAction *menuAction8 = new QAction(QIcon::fromTheme("user-trash"),(tr("Move file to Trash")), this);
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
                if (QFile::moveToTrash(selectedFile, &pathInTrash)) {
                    QMessageBox::warning(this, tr("Warning"), tr("Moved to trash:<br/>") + pathInTrash);

                } else {
                    QMessageBox::warning(this, tr("Warning"), tr("Move to trash failed."));
                }
            }
        }
        //----------------------------------------------------------------------
        void MainWindow::searchContextDeleteFile()
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

    //Methods-----------------------------------------------------------------------

        //Search methods
        //run a search of files in each selected catalog based on user inputs
        void MainWindow::searchFiles()
        {
            // Start animation while opening
            QApplication::setOverrideCursor(Qt::WaitCursor);           

            //Prepare the SEARCH -------------------------------

                //Search results are currently captured in the Catalog model, not the database.
                //Clear exisitng lists of results and search variables
                    filesFoundList.clear();
                    catalogFoundList.clear();

                    //Set up temporary lists
                    sFileNames.clear();
                    sFileSizes.clear();
                    sFilePaths.clear();
                    sFileDateTimes.clear();
                    sFileCatalogs.clear();

                //Get search criteria
                    searchText             = ui->Search_lineEdit_SearchText->text();
                    selectedTextCriteria   = ui->Search_comboBox_TextCriteria->currentText();
                    selectedSearchIn       = ui->Search_comboBox_SearchIn->currentText();
                    selectedSearchExclude  = ui->Search_lineEdit_Exclude->text();
                    selectedFileType       = ui->Search_comboBox_FileType->itemData(ui->Search_comboBox_FileType->currentIndex(),Qt::UserRole).toString();
                    selectedMinimumSize    = ui->Search_spinBox_MinimumSize->value();
                    selectedMaximumSize    = ui->Search_spinBox_MaximumSize->value();
                    selectedMinSizeUnit    = ui->Search_comboBox_MinSizeUnit->currentText();
                    selectedMaxSizeUnit    = ui->Search_comboBox_MaxSizeUnit->currentText();
                    selectedDateMin        = ui->Search_dateTimeEdit_Min->dateTime();
                    selectedDateMax        = ui->Search_dateTimeEdit_Max->dateTime();
                    searchOnSize           = ui->Search_checkBox_Size->isChecked();
                    searchOnDate           = ui->Search_checkBox_Date->isChecked();
                    searchOnTags           = ui->Search_checkBox_Tags->isChecked();
                    searchOnFileName       = ui->Search_checkBox_FileName->isChecked();
                    searchOnFileCriteria   = ui->Search_checkBox_FileCriteria->isChecked();
                    searchOnFolderCriteria = ui->Search_checkBox_FolderCriteria->isChecked();
                    selectedTag            = ui->Search_comboBox_Tags->currentText();
                    caseSensitive          = ui->Search_checkBox_CaseSensitive->isChecked();

                    searchInFileCatalogsChecked   = ui->Filters_checkBox_SearchInCatalogs->isChecked();
                    searchInConnectedDriveChecked = ui->Filters_checkBox_SearchInConnectedDrives->isChecked();

                    //Differences
                    hasDifferencesOnName         = ui->Search_checkBox_DifferencesName->checkState();
                    hasDifferencesOnSize         = ui->Search_checkBox_DifferencesSize->checkState();
                    hasDifferencesOnDateModified = ui->Search_checkBox_DifferencesDateModified->checkState();
                    selectedDifferencesCatalog1  = ui->Search_comboBox_DifferencesCatalog1->currentText();
                    selectedDifferencesCatalog2  = ui->Search_comboBox_DifferencesCatalog2->currentText();

                    // Get the file size min and max, from 0 to 1000.
                    // Define a size multiplier depending on the size unit selected
                    sizeMultiplierMin=1;
                    if      (selectedMinSizeUnit == tr("KiB"))
                            sizeMultiplierMin = sizeMultiplierMin *1024;
                    else if (selectedMinSizeUnit == tr("MiB"))
                            sizeMultiplierMin = sizeMultiplierMin *1024*1024;
                    else if (selectedMinSizeUnit == tr("GiB"))
                            sizeMultiplierMin = sizeMultiplierMin *1024*1024*1024;
                    else if (selectedMinSizeUnit == tr("TiB"))
                            sizeMultiplierMin = sizeMultiplierMin *1024*1024*1024*1024;
                    sizeMultiplierMax=1;
                    if      (selectedMaxSizeUnit == tr("KiB"))
                            sizeMultiplierMax = sizeMultiplierMax *1024;
                    else if (selectedMaxSizeUnit == tr("MiB"))
                            sizeMultiplierMax = sizeMultiplierMax *1024*1024;
                    else if (selectedMaxSizeUnit == tr("GiB"))
                            sizeMultiplierMax = sizeMultiplierMax *1024*1024*1024;
                    else if (selectedMaxSizeUnit == tr("TiB"))
                            sizeMultiplierMax = sizeMultiplierMax *1024*1024*1024*1024;

                 // Searching "Begin With" for File name or Folder name is not supported yet
                    if (selectedTextCriteria==tr("Begins With") and selectedSearchIn !=tr("File names only")){
                        QMessageBox::information(this,"Katalog",tr("The option 'Begin With' can only be used with 'File names only'.\nUse a different combinaison."));
                        //Stop animation
                        QApplication::restoreOverrideCursor();
                        return;;
                    }


            //Process the SEARCH in CATALOGS or DIRECTORY ------------------------------
                //Process the SEARCH in CATALOGS
                    if (searchInFileCatalogsChecked==true){
                        //List of catalogs to search from: catalogSelectedList
                            //Search every catalog if "All" is selected
                            if ( selectedFilterCatalogName ==tr("All")){
                                //For differences, only process with the selected catalogs
                                if (ui->Search_checkBox_Differences->isChecked() ==true){
                                    QStringList differenceCatalogs;
                                    differenceCatalogs << selectedDifferencesCatalog1;
                                    differenceCatalogs << selectedDifferencesCatalog2;
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
                            }
                    }
                //Process the SEARCH in SELECTED DIRECTORY
                    else if (searchInConnectedDriveChecked==true){
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
                    // Use a virtual catalog to store results
                    Catalog *searchResultsCatalog = new Catalog(this);
                    // Prepare query model
                    QSqlQueryModel *loadCatalogQueryModel = new QSqlQueryModel;
                    // Prepare model to display
                    FilesView *fileViewModel = new FilesView(this);

                    // Populate model with folders only if this option is selected
                    if ( searchOnFolderCriteria==true and ui->Search_checkBox_ShowFolders->isChecked()==true )
                    {
                        sFilePaths.removeDuplicates();
                        int numberOfFolders = sFilePaths.count();
                        sFileNames.clear();
                        sFileSizes.clear();
                        sFileDateTimes.clear();
                        sFileCatalogs.clear();
                        for (int i=0; i<numberOfFolders; i++)
                            sFileNames <<"";
                        for (int i=0; i<numberOfFolders; i++)
                            sFileSizes <<0;
                        for (int i=0; i<numberOfFolders; i++)
                            sFileDateTimes <<"";
                        for (int i=0; i<numberOfFolders; i++)
                            sFileCatalogs <<"";

                        // Populate model with data
                        searchResultsCatalog->populateFileData(sFileNames, sFileSizes, sFilePaths, sFileDateTimes,sFileCatalogs);
                        fileViewModel->setSourceModel(searchResultsCatalog);
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
                        searchResultsCatalog->populateFileData(sFileNames, sFileSizes, sFilePaths, sFileDateTimes, sFileCatalogs);
                        fileViewModel->setSourceModel(searchResultsCatalog);
                        fileViewModel->setHeaderData(0, Qt::Horizontal, tr("Name"));
                        fileViewModel->setHeaderData(1, Qt::Horizontal, tr("Size"));
                        fileViewModel->setHeaderData(2, Qt::Horizontal, tr("Date"));
                        fileViewModel->setHeaderData(3, Qt::Horizontal, tr("Folder"));
                        fileViewModel->setHeaderData(4, Qt::Horizontal, tr("Catalog"));
                        if (searchInConnectedDriveChecked==true){
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
                    //Get inputs
                        hasDuplicatesOnName         = ui->Search_checkBox_DuplicatesName->checkState();
                        hasDuplicatesOnSize         = ui->Search_checkBox_DuplicatesSize->checkState();
                        hasDuplicatesOnDateModified = ui->Search_checkBox_DuplicatesDateModified->checkState();
                    //Process if enabled and criteria are provided
                        if ( searchOnFileCriteria==true and ui->Search_checkBox_Duplicates->isChecked() ==true
                             and (     hasDuplicatesOnName==true
                                    or hasDuplicatesOnSize==true
                                    or hasDuplicatesOnDateModified==true)){

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

                                    int rows = searchResultsCatalog->rowCount();

                                    for (int i=0; i<rows; i++) {

                                            //QString test = searchResultsCatalog->index(i,0).data().toString();

                                            //Append data to the database
                                            insertQuery.bindValue(":file_name",         searchResultsCatalog->index(i,0).data().toString());
                                            insertQuery.bindValue(":file_size",         searchResultsCatalog->index(i,1).data().toString());
                                            insertQuery.bindValue(":file_folder_path",  searchResultsCatalog->index(i,3).data().toString());
                                            insertQuery.bindValue(":file_date_updated", searchResultsCatalog->index(i,2).data().toString());
                                            insertQuery.bindValue(":file_catalog",      searchResultsCatalog->index(i,4).data().toString());
                                            insertQuery.exec();
                                    }

                            //Prepare duplicate SQL
                                // Load all files and create model
                                QString selectSQL;

                                //Generate grouping of fields based on user selection, determining what are duplicates
                                QString groupingFields; // this value should be a concatenation of fields, like "fileName||fileSize"

                                    //same name
                                    if(hasDuplicatesOnName == true){
                                        groupingFields = groupingFields + "file_name";
                                    }
                                    //same size
                                    if(hasDuplicatesOnSize == true){
                                        groupingFields = groupingFields + "||file_size";
                                    }
                                    //same date modified
                                    if(hasDuplicatesOnDateModified == true){
                                        groupingFields = groupingFields + "||file_date_updated";
                                    }

                                    //remove starting || if any
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
                                sFileNames.clear();
                                sFileSizes.clear();
                                sFilePaths.clear();
                                sFileDateTimes.clear();
                                sFileCatalogs.clear();
                                while(duplicatesQuery.next()){
                                        sFileNames.append(duplicatesQuery.value(0).toString());
                                        sFileSizes.append(duplicatesQuery.value(1).toLongLong());
                                        sFileDateTimes.append(duplicatesQuery.value(2).toString());
                                        sFilePaths.append(duplicatesQuery.value(3).toString());
                                        sFileCatalogs.append(duplicatesQuery.value(4).toString());
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
                        if ( searchOnFileCriteria==true and ui->Search_checkBox_Differences->isChecked() ==true
                             and (     hasDifferencesOnName==true
                                    or hasDifferencesOnSize==true
                                    or hasDifferencesOnDateModified==true)){

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

                                    int rows = searchResultsCatalog->rowCount();

                                    for (int i=0; i<rows; i++) {

                                            QString test = searchResultsCatalog->index(i,0).data().toString();

                                            //Append data to the database
                                            insertQuery.bindValue(":file_name",        searchResultsCatalog->index(i,0).data().toString());
                                            insertQuery.bindValue(":file_size",        searchResultsCatalog->index(i,1).data().toString());
                                            insertQuery.bindValue(":file_folder_path", searchResultsCatalog->index(i,3).data().toString());
                                            insertQuery.bindValue(":file_date_updated",searchResultsCatalog->index(i,2).data().toString());
                                            insertQuery.bindValue(":file_catalog",     searchResultsCatalog->index(i,4).data().toString());
                                            insertQuery.exec();

                                    }

                            //Prepare difference SQL
                                // Load all files and create model
                                QString selectSQL;

                                //Generate grouping of fields based on user selection, determining what are duplicates
                                QString groupingFieldsDifferences; // this value should be a concatenation of fields, like "fileName||fileSize"

                                    //same name
                                    if(hasDifferencesOnName == true){
                                        groupingFieldsDifferences += "||file_name";
                                    }
                                    //same size
                                    if(hasDifferencesOnSize == true){
                                        groupingFieldsDifferences += "||file_size";
                                    }
                                    //same date modified
                                    if(hasDifferencesOnDateModified == true){
                                        groupingFieldsDifferences += "||file_date_updated";
                                    }

                                    //remove the || at the start
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
                                qDebug()<<differencesQuery.prepare(selectSQL);
                                differencesQuery.bindValue(":selectedDifferencesCatalog1",selectedDifferencesCatalog1);
                                differencesQuery.bindValue(":selectedDifferencesCatalog2",selectedDifferencesCatalog2);
                                differencesQuery.exec();

                                //recapture file results for Stats
                                sFileNames.clear();
                                sFileSizes.clear();
                                sFilePaths.clear();
                                sFileDateTimes.clear();
                                sFileCatalogs.clear();
                                while(differencesQuery.next()){
                                        sFileNames.append(differencesQuery.value(0).toString());
                                        sFileSizes.append(differencesQuery.value(1).toLongLong());
                                        sFileDateTimes.append(differencesQuery.value(2).toString());
                                        sFilePaths.append(differencesQuery.value(3).toString());
                                        sFileCatalogs.append(differencesQuery.value(4).toString());
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
                    foreach (sizeItem, sFileSizes) {
                                filesFoundTotalSize = filesFoundTotalSize + sizeItem;
                    }
                    ui->Search_label_SizeResults->setText(QLocale().formattedDataSize(filesFoundTotalSize));

                    //Other statistics, covering the case where no results are returned.
                    if (filesFoundNumber !=0){
                                filesFoundAverageSize = filesFoundTotalSize / filesFoundNumber;
                                QList<qint64> fileSizeList = sFileSizes;
                                std::sort(fileSizeList.begin(), fileSizeList.end());
                                filesFoundMinSize = fileSizeList.first();
                                filesFoundMaxSize = fileSizeList.last();

                                QList<QString> fileDateList = sFileDateTimes;
                                std::sort(fileDateList.begin(), fileDateList.end());
                                filesFoundMinDate = fileDateList.first();
                                filesFoundMaxDate = fileDateList.last();

                                ui->Search_pushButton_FileFoundMoreStatistics->setEnabled(true);
                    }

            //Save the search parameters to the settings file
            saveSettings();
            insertSearchHistoryToTable();
            if(databaseMode=="Memory")
                saveSearchHistoryTableToFile();
            loadSearchHistoryTableToModel();

            //Stop animation
            QApplication::restoreOverrideCursor();

            //Enable Export
            ui->Search_pushButton_ExportResults->setEnabled(true);

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
                    if(selectedTextCriteria == tr("Exact Phrase"))
                        regexSearchtext=searchText; //just search for the extact text entered including spaces, as one text string.
                    else if(selectedTextCriteria == tr("Begins With"))
                        regexSearchtext="(^"+searchText+")";
                    else if(selectedTextCriteria == tr("Any Word"))
                        regexSearchtext=searchText.replace(" ","|");
                    else if(selectedTextCriteria == tr("All Words")){
                        QString searchTextToSplit = searchText;
                        QString groupRegEx = "";
                        QRegularExpression lineSplitExp(" ");
                        QStringList lineFieldList = searchTextToSplit.split(lineSplitExp);
                        int numberOfSearchWords = lineFieldList.count();
                        //build regex group for one word
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
                if( searchOnFileCriteria==true and selectedFileType !="All"){
                    //Get the list of file extension and join it into one string
                    if(selectedFileType =="Audio"){
                                regexFileType = fileType_AudioS.join("|");
                    }
                    if(selectedFileType =="Image"){
                                regexFileType = fileType_ImageS.join("|");
                    }
                    if(selectedFileType =="Text"){
                                regexFileType = fileType_TextS.join("|");
                    }
                    if(selectedFileType =="Video"){
                                regexFileType = fileType_VideoS.join("|");
                    }

                    //Replace the *. by .* needed for regex
                    regexFileType = regexFileType.replace("*.",".*");

                    //Add the file type expression to the regex
                    regexPattern = regexSearchtext  + "(" + regexFileType + ")";

                 }

                //Add the words to exclude to the regex
                if ( selectedSearchExclude !=""){

                    //Prepare
                    QString searchTextToSplit = selectedSearchExclude;
                    QString excludeGroupRegEx = "";
                    QRegularExpression lineSplitExp(" ");
                    QStringList lineFieldList = searchTextToSplit.split(lineSplitExp);
                    int numberOfSearchWords = lineFieldList.count();

                    //Build regex group to exclude all words
                        //Genereate first part = first characters + the first word
                        excludeGroupRegEx = "^(?!.*(" + lineFieldList[0];
                        //add more words
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
                if (caseSensitive != true) {
                    regex.setPatternOptions(QRegularExpression::CaseInsensitiveOption);
                }

            //Load the catalog file contents if not already loaded in memory
                tempCatalog->loadCatalogFileListToTable();

            //Search loop for all lines in the catalog file
                //load the files of the Catalog
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
                    if (searchOnFileCriteria==true and searchOnSize==true){
                        getFilesQuerySQL = getFilesQuerySQL+" AND file_size>=:file_size_min ";
                        getFilesQuerySQL = getFilesQuerySQL+" AND file_size<=:file_size_max ";
                    }
                    //Add matching date range
                    if (searchOnFileCriteria==true and searchOnDate==true){
                        getFilesQuerySQL = getFilesQuerySQL+" AND file_date_updated>=:file_date_updated_min ";
                        getFilesQuerySQL = getFilesQuerySQL+" AND file_date_updated<=:file_date_updated_max ";
                    }
                    getFilesQuery.prepare(getFilesQuerySQL);
                    getFilesQuery.bindValue(":file_catalog",sourceCatalogName);
                    getFilesQuery.bindValue(":file_size_min",selectedMinimumSize * sizeMultiplierMin);
                    getFilesQuery.bindValue(":file_size_max",selectedMaximumSize * sizeMultiplierMax);
                    getFilesQuery.bindValue(":file_date_updated_min",selectedDateMin.toString("yyyy/MM/dd hh:mm:ss"));
                    getFilesQuery.bindValue(":file_date_updated_max",selectedDateMax.toString("yyyy/MM/dd hh:mm:ss"));
                    getFilesQuery.exec();

                //File by file, test if the file is matching all search criteria
                //Loop principle1: stop further verification as soon as a criteria fails to match
                //Loop principle2: start with fastest criteria, finish with more complex ones (tag, file name)

                while(getFilesQuery.next()){

                    QString   lineFileName     = getFilesQuery.value(0).toString();
                    QString   lineFilePath     = getFilesQuery.value(1).toString();
                    QString   lineFileFullPath = lineFilePath + "/" + lineFileName;
                    //qint64    lineFileSize     = getFilesQuery.value(2).toLongLong();
                    //QDateTime lineFileDateTime = QDateTime::fromString(getFilesQuery.value(3).toString(),"yyyy/MM/dd hh:mm:ss");
                    bool      fileIsMatchingTag;

                    //Continue if the file is matching the tags
                        if (searchOnFolderCriteria==true and searchOnTags==true and selectedTag!=""){

                            fileIsMatchingTag = false;

                            //Set query to get a list of folder paths matching the selected tag
                            QSqlQuery queryTag;
                            QString queryTagSQL = QLatin1String(R"(
                                                SELECT path
                                                FROM tag
                                                WHERE name=:name
                            )");
                            queryTag.prepare(queryTagSQL);
                            queryTag.bindValue(":name",selectedTag);
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
                        if (searchOnFileName==true){
                            //Depends on the "Search in" criteria,
                            //Reduces the abosulte path to the required text string and matches the search text
                            if(selectedSearchIn == tr("File names only"))
                            {
                                match = regex.match(lineFileName);
                            }
                            else if(selectedSearchIn == tr("Folder path only"))
                            {
                                //Keep only the folder name, so all characters left of the last occurence of / in the path.
                                //reducedLine = lineFilePath.left(lineFilePath.lastIndexOf("/"));

                                //Check that the folder name matches the search text
                                regex.setPattern(regexSearchtext);
                                foldermatch = regex.match(lineFilePath);

                                //if it does, then check that the file matches the selected file type
                                if (foldermatch.hasMatch()){
                                    regex.setPattern(regexFileType);
                                    match = regex.match(lineFilePath);
                                }
                            }
                            else {
                                match = regex.match(lineFileFullPath);
                            }

                            //If the file is matching the criteria, add it and its catalog to the search results
                            if (match.hasMatch()){
                                filesFoundList << lineFilePath;
                                catalogFoundList.insert(0,sourceCatalogName);

                                //Retrieve other file info
                                //QFileInfo file(lineFileFullPath);

                                //Populate result lists
                                sFileNames.append(lineFileName);
                                sFilePaths.append(lineFilePath);
                                sFileSizes.append(getFilesQuery.value(2).toLongLong());
                                sFileDateTimes.append(getFilesQuery.value(3).toString());
                                sFileCatalogs.append(sourceCatalogName);
                            }
                        }
                        else{
                            //verify file matches the selected file type
                            regex.setPattern(regexFileType);
                            match = regex.match(lineFilePath);
                            if (!match.hasMatch()){
                                continue;
                            }

                            //Add the file and its catalog to the results, excluding blank lines
                            if (lineFilePath !=""){
                                filesFoundList << lineFilePath;
                                catalogFoundList.insert(0,sourceCatalogName);

                                //Retrieve other file info
                                //QFileInfo file(lineFilePath);

                                //Populate result lists
                                sFileNames.append(lineFileName);
                                sFilePaths.append(lineFilePath);
                                sFileSizes.append(getFilesQuery.value(2).toLongLong());
                                sFileDateTimes.append(getFilesQuery.value(3).toString());
                                sFileCatalogs.append(sourceCatalogName);
                            }
                        }
                }
      }
        //----------------------------------------------------------------------
        //run a search of files for the selected Directory
        void MainWindow::searchFilesInDirectory(const QString &sourceDirectory)
        {
            //Define how to use the search text //COMMON to searchFilesInCatalog
                if(selectedTextCriteria == tr("Exact Phrase"))
                    regexSearchtext=searchText; //just search for the extact text entered including spaces, as one text string.
                else if(selectedTextCriteria == tr("Begins With"))
                    regexSearchtext="(^"+searchText+")";
                else if(selectedTextCriteria == tr("Any Word"))
                    regexSearchtext=searchText.replace(" ","|");
                else if(selectedTextCriteria == tr("All Words")){
                    QString searchTextToSplit = searchText;
                    QString groupRegEx = "";
                    QRegularExpression lineSplitExp(" ");
                    QStringList lineFieldList = searchTextToSplit.split(lineSplitExp);
                    int numberOfSearchWords = lineFieldList.count();
                    //build regex group for one word
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
            if ( searchOnFileCriteria==true and selectedFileType !=tr("All")){
                //Get the list of file extension and join it into one string
                if(selectedFileType ==tr("Audio")){
                            regexFileType = fileType_AudioS.join("|");
                }
                if(selectedFileType ==tr("Image")){
                            regexFileType = fileType_ImageS.join("|");
                }
                if(selectedFileType ==tr("Text")){
                            regexFileType = fileType_TextS.join("|");
                }
                if(selectedFileType ==tr("Video")){
                            regexFileType = fileType_VideoS.join("|");
                }

                //Replace the *. by .* needed for regex
                regexFileType = regexFileType.replace("*.",".*");

                //Add the file type expression to the regex
                regexPattern = regexSearchtext  + "(" + regexFileType + ")";
             }

            //Add the words to exclude to the regex //COMMON to searchFilesInCatalog

            if ( selectedSearchExclude !=""){

                //Prepare
                QString searchTextToSplit = selectedSearchExclude;
                QString excludeGroupRegEx = "";
                QRegularExpression lineSplitExp(" ");
                QStringList lineFieldList = searchTextToSplit.split(lineSplitExp);
                int numberOfSearchWords = lineFieldList.count();

                //Build regex group to exclude all words
                    //Genereate first part = first characters + the first word
                    excludeGroupRegEx = "^(?!.*(" + lineFieldList[0];
                    //add more words
                    for (int i=1; i<(numberOfSearchWords); i++){
                        excludeGroupRegEx = excludeGroupRegEx + "|" + lineFieldList[i];
                    }
                    //last part
                    excludeGroupRegEx = excludeGroupRegEx + "))";

                //Add regex group to exclude to the global regexPattern
                regexPattern = excludeGroupRegEx + regexPattern;
            }

            QRegularExpression regex(regexPattern, QRegularExpression::CaseInsensitiveOption);

            //filetypes
                    // Get the file type for the catalog
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
                    if (searchOnSize==true){
                            if ( !(     lineFileSize >= selectedMinimumSize * sizeMultiplierMin
                                    and lineFileSize <= selectedMaximumSize * sizeMultiplierMax) ){
                                        continue;}
                        }
                    //Continue if the file is matching the date range
                        if (searchOnDate==true){
                            if ( !(     lineFileDateTime >= selectedDateMin
                                    and lineFileDateTime <= selectedDateMax ) ){
                                        continue;}
                        }

                    //Continue if the file is matching the tags
                        if (searchOnTags==true){

                            bool fileIsMatchingTag = false;

                            //Set query to get a list of folder paths matching the selected tag
                            QSqlQuery queryTag;
                            QString queryTagSQL = QLatin1String(R"(
                                                SELECT path
                                                FROM tag
                                                WHERE name=:name
                            )");
                            queryTag.prepare(queryTagSQL);
                            queryTag.bindValue(":name",selectedTag);
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
                    if (searchOnFileName==true){
                        //Depending on the "Search in" criteria,
                        //reduce the abosulte path to the reaquired text string and match the search text
                        if(selectedSearchIn == tr("File names only"))
                        {
                            // Extract the file name from the lineFilePath
                            QFileInfo file(lineFilePath);
                            reducedLine = file.fileName();

                            match = regex.match(reducedLine);
                        }
                        else if(selectedSearchIn == tr("Folder path only"))
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
                            sFileNames.append(file.fileName());
                            sFilePaths.append(file.path());
                            sFileSizes.append(lineFileSize);
                            sFileDateTimes.append(lineFileDatetime);
                            sFileCatalogs.append(sourceDirectory);
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
                            sFileNames.append(file.fileName());
                            sFilePaths.append(file.path());
                            sFileSizes.append(lineFileSize);
                            sFileDateTimes.append(lineFileDatetime);
                            sFileCatalogs.append(sourceDirectory);
                        }
                    }
            }
        }
        //----------------------------------------------------------------------
        QString MainWindow::getCatalogStorageName(QString catalogFilePath)
        {
            //DEV: REPLACE BY SQL QUERY ON Catalog TABLE

//            QString catalogStorageName;
            //LoadCatalogInfo(file);
            // Get infos stored in the file
            QFile catalogFile(catalogFilePath);
            if(!catalogFile.open(QIODevice::ReadOnly)) {
                return "";
            }

            QTextStream textStream(&catalogFile);

            while (true)
            {
                QString line = textStream.readLine();

                if (line.left(19)=="<catalogSourcePath>"){

                }
                else if (line.left(18)=="<catalogFileCount>"){

                }
                else if (line.left(22)=="<catalogTotalFileSize>"){

                }
                else if (line.left(22)=="<catalogIncludeHidden>"){

                }
                else if (line.left(17)=="<catalogFileType>"){

                }
                else if (line.left(16)=="<catalogStorage>"){
                    QString catalogStorageName = line.right(line.size() - line.lastIndexOf(">") - 1);
                    return catalogStorageName;
                }
                else
                    break;
            }
            return "";
        }
        //----------------------------------------------------------------------
        void MainWindow::getLocationCatalogList(QString location)
        {
            //DEV: REPLACE BY SQL QUERY ON Storage and Catalog TABLE
            // get all catalog for storage with matching location
            /*
            QSqlQuery queryDeviceNumber;
            queryDeviceNumber.prepare(
                        "SELECT * FROM storage, catalog "
                        "WHERE location = " + selectedStorageLocation );
            queryDeviceNumber.exec();

            "SELECT * FROM storage, catalog
            WHERE location = " + location + '"'
            */

            //QString catalogStorageName;

            //Define storage file
            QFile storageFile(storageFilePath);

            //Open file
            if(!storageFile.open(QIODevice::ReadOnly)) {
                return;
            }

            // Get list of storage matching the location
            QTextStream textStream(&storageFile);
            QStringList locationStorageList;
            while (true)
            {
                QString line = textStream.readLine();
                if (line.isNull())
                    break;

                //Split the string by tabulations into a list
                QStringList fieldList = line.split('\t');

                //Add to the list if the location is matching
                if ( fieldList[3] == location){
                    locationStorageList << fieldList[0];
                }
            }

            //Get list of catalogs matching these storages
            QString sourceStorage;
            foreach(sourceStorage,locationStorageList)
                    {

                    foreach(sourceCatalog,catalogFileList)
                        {

                        //get catalog storage name
                        QString sourceCatalogStorageName;
                        QString currentCatalogFilePath = collectionFolder + "/" + sourceCatalog + ".idx";
                        sourceCatalogStorageName = getCatalogStorageName(currentCatalogFilePath);

                        if  ( sourceCatalogStorageName == sourceStorage )
                            {
                                    locationCatalogList << sourceCatalog;
                            }
                        }
                    }
        }
        //----------------------------------------------------------------------

        //UI methods
        void MainWindow::initiateSearchValues()
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
                if (selectedMaximumSize ==0)
                    selectedMaximumSize = 1000;

            //Populate Differences combo boxes with selected catalogs
                refreshDifferencesCatalogSelection();

            //Set values
                ui->Search_checkBox_FileName->setChecked(searchOnFileName);
                ui->Search_checkBox_FileCriteria->setChecked(searchOnFileCriteria);
                ui->Search_checkBox_FolderCriteria->setChecked(searchOnFolderCriteria);
                ui->Search_comboBox_TextCriteria->setCurrentText(selectedTextCriteria);
                ui->Search_comboBox_SearchIn->setCurrentText(selectedSearchIn);
                ui->Search_lineEdit_Exclude->setText(selectedSearchExclude);
                ui->Search_comboBox_FileType->setCurrentText(tr(selectedFileType.toUtf8()));
                ui->Search_spinBox_MinimumSize->setValue(selectedMinimumSize);
                ui->Search_spinBox_MaximumSize->setValue(selectedMaximumSize);
                ui->Search_comboBox_MinSizeUnit->setCurrentText(selectedMinSizeUnit);
                ui->Search_comboBox_MaxSizeUnit->setCurrentText(selectedMaxSizeUnit);
                ui->Search_dateTimeEdit_Min->setDateTime(selectedDateMin);
                ui->Search_dateTimeEdit_Max->setDateTime(selectedDateMax);
                ui->Search_checkBox_Size->setChecked(searchOnSize);
                ui->Search_checkBox_Date->setChecked(searchOnDate);
                ui->Search_checkBox_Tags->setChecked(searchOnTags);
                ui->Search_comboBox_Tags->setCurrentText(selectedTag);
                ui->Search_checkBox_Duplicates->setChecked(searchOnDuplicates);
                ui->Search_checkBox_DuplicatesName->setChecked(hasDuplicatesOnName);
                ui->Search_checkBox_DuplicatesSize->setChecked(hasDuplicatesOnSize);
                ui->Search_checkBox_DuplicatesDateModified->setChecked(hasDuplicatesOnDateModified);
                ui->Search_widget_DifferencesCatalogs->setHidden(true);
                ui->Search_checkBox_Differences->setChecked(searchOnDifferences);
                ui->Search_checkBox_DifferencesName->setChecked(hasDifferencesOnName);
                ui->Search_checkBox_DifferencesSize->setChecked(hasDifferencesOnSize);
                ui->Search_checkBox_DifferencesDateModified->setChecked(hasDifferencesOnDateModified);
                ui->Search_comboBox_DifferencesCatalog1->setCurrentText(selectedDifferencesCatalog1);
                ui->Search_comboBox_DifferencesCatalog2->setCurrentText(selectedDifferencesCatalog2);
                ui->Search_checkBox_ShowFolders->setChecked(showFoldersOnly);
                ui->Search_checkBox_CaseSensitive->setChecked(caseSensitive);
                ui->Filters_lineEdit_SeletedDirectory->setText(selectedConnectedDrivePath);
                ui->Filters_checkBox_SearchInCatalogs->setChecked(searchInFileCatalogsChecked);
                ui->Filters_checkBox_SearchInConnectedDrives->setChecked(searchInConnectedDriveChecked);

        }
        //----------------------------------------------------------------------
        void MainWindow::refreshLocationSelectionList()
        {
            //Get current location
//            QString currentLocation = ui->Filters_label_DisplayLocation->text();
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

            //Send the list to the Search combobox model
//            QStringListModel *locationListModel = new QStringListModel();
//            locationListModel->setStringList(displayLocationList);
//            ui->Filters_label_DisplayLocation->setModel(locationListModel);

            //Restore last selection
//            ui->Filters_comboBox_SelectLocation->setCurrentText(currentLocation);

        }
        //----------------------------------------------------------------------
        void MainWindow::refreshStorageSelectionList(QString selectedLocation)
        {
            //get current location
//            QString currentStorage = ui->Filters_label_DisplayStorage->text();

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

            //Send the list to the Search combobox model
//            QStringListModel *storageListModel = new QStringListModel();
//            storageListModel->setStringList(displayStorageList);
//            ui->Filters_label_DisplayStorage->setModel(storageListModel);

//            Restore last selection
//            ui->Filters_comboBox_SelectStorage->setCurrentText(currentStorage);

        }
        //----------------------------------------------------------------------
        void MainWindow::refreshCatalogSelectionList(QString selectedLocation, QString selectedStorage)
        {
            //get current location
            //QString currentCatalog = ui->Filters_label_DisplayCatalog->text();

            //Query the full list of locations
            QSqlQuery getCatalogSelectionList;

            //Prepare and run the query
                //common
                QString queryText = "SELECT c.catalog_name FROM catalog AS c"
                                    " left JOIN storage AS s ON c.catalog_storage = s.storage_name";

                //filter depending on location and selection.
                    //Default: ( selectedLocation == "All" and selectedStorage == "All")

                    if      ( selectedLocation == tr("All") and selectedStorage != tr("All"))
                    {
                            queryText = queryText + " WHERE c.catalog_storage ='" + selectedStorage + "'";
                    }
                    else if ( selectedLocation != tr("All") and selectedStorage == tr("All"))
                    {
                            queryText = queryText + " WHERE storage_location ='" + selectedLocation + "'";
                    }
                    else if ( selectedLocation != tr("All") and selectedStorage != tr("All"))
                    {
                            queryText = queryText + " WHERE c.catalog_storage ='" + selectedStorage + "' "
                                                    " AND storage_location ='" + selectedLocation + "'";
                    }
                //common
                //queryText = queryText + " ORDER BY catalogName ASC";

                //run the query
                getCatalogSelectionList.prepare(queryText);
                getCatalogSelectionList.exec();

            //Put the results in a list
                //clear the list of selected catalogs
                catalogSelectedList.clear();

                //populate from the query results
                while (getCatalogSelectionList.next()) {
                    catalogSelectedList << getCatalogSelectionList.value(0).toString();
                }
                catalogSelectedList.sort(); //DEV: because the SQL "order by" does not work
            //Prepare the list for the Location combobox
                QStringList displayCatalogList = catalogSelectedList;
                //Add the "All" option at the beginning
                displayCatalogList.insert(0,tr("All"));

            //Send the list to the Search combobox model
                //QStringListModel *catalogListModel = new QStringListModel();
                //catalogListModel->setStringList(displayCatalogList);
                //ui->Filters_comboBox_SelectCatalog->setModel(catalogListModel);

            //Send list to the Statistics combobox (without All or Selected storage options)
                QStringListModel *catalogListModelForStats = new QStringListModel(this);
                catalogListModelForStats->setStringList(catalogSelectedList);

                //Get last value
//                QSettings settings(settingsFilePath, QSettings:: IniFormat);
//                QString lastValue = settings.value("Statistics/SelectedCatalog").toString();

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
        QString MainWindow::exportSearchResults()
        {
            QString fileExtension;
            QStringList catalogMetadata;

            int result = QMessageBox::warning(this,"Katalog",
                      tr("Create a catalog from these results?"
                         "<br/>- Yes: create an idx file and use it to refine your search,"
                         "<br/>- No:  simply export results to a csv file."),
                                              QMessageBox::Yes|QMessageBox::No);

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
            else{
                fileExtension="csv";
            }

            //Prepare export file name
            QDateTime now = QDateTime::currentDateTime();
            QString timestamp = now.toString(QLatin1String("yyyyMMdd-hhmmss"));
            QString fileNameWithoutExtension = QString::fromLatin1("search_results_%1").arg(timestamp);
            QString fileNameWithExtension = fileNameWithoutExtension + "." + fileExtension;
            QString fullFileName=collectionFolder+"/"+fileNameWithExtension;
            QFile exportFile(fullFileName);

            //Export search results to file
            if (exportFile.open(QFile::WriteOnly | QFile::Text)) {

                QTextStream stream(&exportFile);

                for (int i = 0; i < catalogMetadata.size(); ++i)
                {
                    stream << catalogMetadata[i] << '\n';
                }

                for (int i = 0; i < filesFoundList.size(); ++i)
                {
                    QString line = sFilePaths[i] + "/" + sFileNames[i] + "\t"
                                 + QString::number(sFileSizes[i]) + "\t"
                                 + sFileDateTimes[i] + "\t"
                                 + sFileCatalogs[i];
                    stream << line << '\n';
                }
            }
            exportFile.close();

            //Refresh catalogs
            loadCollection();

            //Select new catalog with results
            ui->Filters_label_DisplayCatalog->setText(fileNameWithoutExtension);

            return fullFileName;
        }
        //----------------------------------------------------------------------
        void MainWindow::insertSearchHistoryToTable()
        {
            //Save Search to db
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
                                    case_sensitive
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
                                    :case_sensitive
                                )
                )");

            query.prepare(querySQL);
            query.bindValue(":date_time",             searchDateTime);
            query.bindValue(":text_checked",          ui->Search_checkBox_FileCriteria->isChecked());

//            #ifdef Q_OS_LINUX
//            query.bindValue(":TextPhrase",           ui->Search_kcombobox_SearchText->currentText());
//            #else
            query.bindValue(":text_phrase",           ui->Search_lineEdit_SearchText->text());
//            #endif

            query.bindValue(":text_criteria",           selectedTextCriteria);
            query.bindValue(":text_search_in",          selectedSearchIn);
            query.bindValue(":file_type",               selectedFileType);
            query.bindValue(":file_size_checked",       ui->Search_checkBox_Size->isChecked());
            query.bindValue(":file_size_min",           selectedMinimumSize);
            query.bindValue(":file_size_min_unit",      selectedMinSizeUnit);
            query.bindValue(":file_size_max",           selectedMaximumSize);
            query.bindValue(":file_size_max_unit",      selectedMaxSizeUnit);
            query.bindValue(":date_modified_checked",   ui->Search_checkBox_Date->isChecked());
            query.bindValue(":date_modified_min",       ui->Search_dateTimeEdit_Min->dateTime().toString("yyyy/MM/dd hh:mm:ss"));
            query.bindValue(":date_modified_max",       ui->Search_dateTimeEdit_Max->dateTime().toString("yyyy/MM/dd hh:mm:ss"));
            query.bindValue(":duplicates_checked",      ui->Search_checkBox_Duplicates->isChecked());
            query.bindValue(":duplicates_name",         ui->Search_checkBox_DuplicatesName->isChecked());
            query.bindValue(":duplicates_size",         ui->Search_checkBox_DuplicatesSize->isChecked());
            query.bindValue(":duplicates_date_modified",ui->Search_checkBox_DuplicatesDateModified->isChecked());
            query.bindValue(":differences_checked",     ui->Search_checkBox_Differences->isChecked());
            query.bindValue(":differences_name",        ui->Search_checkBox_DifferencesName->isChecked());
            query.bindValue(":differences_size",        ui->Search_checkBox_DifferencesSize->isChecked());
            query.bindValue(":differences_date_modified", ui->Search_checkBox_DifferencesDateModified->isChecked());
            query.bindValue(":differences_catalogs",    selectedDifferencesCatalog1+"||"+selectedDifferencesCatalog2);
            query.bindValue(":show_folders",            ui->Search_checkBox_ShowFolders->isChecked());
            query.bindValue(":tag_checked",             ui->Search_checkBox_Tags->isChecked());
            query.bindValue(":tag",                     ui->Search_comboBox_Tags->currentText());
            query.bindValue(":search_location",         selectedFilterStorageLocation);
            query.bindValue(":search_storage",          selectedFilterStorageName);
            query.bindValue(":search_catalog",          selectedFilterCatalogName);
            query.bindValue(":search_catalog_checked",  ui->Filters_checkBox_SearchInCatalogs->isChecked());
            query.bindValue(":search_directory_checked",ui->Filters_checkBox_SearchInConnectedDrives->isChecked());
            query.bindValue(":selected_directory",      ui->Filters_lineEdit_SeletedDirectory->text());
            query.bindValue(":text_exclude",            ui->Search_lineEdit_Exclude->text());
            query.bindValue(":case_sensitive",          ui->Search_checkBox_CaseSensitive->isChecked());
            query.exec();
        }
        //----------------------------------------------------------------------
        void MainWindow::saveSearchHistoryTableToFile()
        {
            //Prepare export
            QFile searchFile(searchHistoryFilePath);
            //Create if not exist
            //QFile exportFile(collectionFolder+"/file.txt");

            QTextStream out(&searchFile);

            //Query
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
                                    differences_catalogs
                                FROM search
                                ORDER BY date_time DESC
                               )");
            query.prepare(querySQL);
            query.exec();

            //    Iterate the result
            //    -- Make a QStringList containing the output of each field
            while (query.next()) {

                const QSqlRecord record = query.record();
                for (int i=0, recCount = record.count() ; i<recCount ; ++i){
                    if (i>0)
                    out << '\t';
                    out << record.value(i).toString();
                }
         //    -- Write the result in the file
                 //out << line;
                 out << '\n';

            }

            if(searchFile.open(QIODevice::WriteOnly | QIODevice::Truncate)) {

                //out << textData;
        //    Close the file
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

                        //add empty values to support the addition of new fields for files from older versions
                        if (fieldList.count()<29){
                            fieldList.append("");
                            fieldList.append("");
                            fieldList.append("");
                            fieldList.append("");
                            fieldList.append("");
                        }
                        if (fieldList.count()<34){
                            fieldList.append("");
                            fieldList.append("");
                            fieldList.append("");
                            fieldList.append("");
                            fieldList.append("");
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
                                            case_sensitive
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
                                            :case_sensitive
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
                                                    selected_directory
                                                FROM search
                                                ORDER BY date_time DESC
                                            )");
            querySearchHistory.prepare(querySearchHistorySQL);
            querySearchHistory.exec();

            QSqlQueryModel *queryModel = new QSqlQueryModel();
            queryModel->setQuery(std::move(querySearchHistory));

//            queryModel->setHeaderData( 0, Qt::Horizontal, tr("dateTime"));
//            queryModel->setHeaderData( 1, Qt::Horizontal, tr("TextChecked"));
//            queryModel->setHeaderData( 2, Qt::Horizontal, tr("TextPhrase"));
//            queryModel->setHeaderData( 3, Qt::Horizontal, tr("TextCriteria"));
//            queryModel->setHeaderData( 4, Qt::Horizontal, tr("TextSearchIn"));
//            queryModel->setHeaderData( 5, Qt::Horizontal, tr("FileType"));
//            queryModel->setHeaderData( 6, Qt::Horizontal, tr("FileSizeChecked"));
//            queryModel->setHeaderData( 7, Qt::Horizontal, tr("FileSizeMin"));
//            queryModel->setHeaderData( 8, Qt::Horizontal, tr("FileSizeMinUnit"));
//            queryModel->setHeaderData( 9, Qt::Horizontal, tr("FileSizeMax"));
//            queryModel->setHeaderData(10, Qt::Horizontal, tr("FileSizeMaxUnit"));
//            queryModel->setHeaderData(11, Qt::Horizontal, tr("DateModifiedChecked"));
//            queryModel->setHeaderData(12, Qt::Horizontal, tr("DateModifiedMin"));
//            queryModel->setHeaderData(13, Qt::Horizontal, tr("DateModifiedMax"));
//            queryModel->setHeaderData(14, Qt::Horizontal, tr("DuplicatesChecked"));
//            queryModel->setHeaderData(15, Qt::Horizontal, tr("DuplicatesName"));
//            queryModel->setHeaderData(16, Qt::Horizontal, tr("DuplicatesSize"));
//            queryModel->setHeaderData(17, Qt::Horizontal, tr("DuplicatesDateModified"));
//            queryModel->setHeaderData(18, Qt::Horizontal, tr("ShowFolders"));
//            queryModel->setHeaderData(19, Qt::Horizontal, tr("TagChecked"));
//            queryModel->setHeaderData(20, Qt::Horizontal, tr("Tag"));
//            queryModel->setHeaderData(21, Qt::Horizontal, tr("searchLocation"));
//            queryModel->setHeaderData(22, Qt::Horizontal, tr("searchStorage"));
//            queryModel->setHeaderData(23, Qt::Horizontal, tr("searchCatalog"));

            QSortFilterProxyModel *searchHistoryProxyModel = new QSortFilterProxyModel;
            searchHistoryProxyModel->setSourceModel(queryModel);
            ui->Search_treeView_History->setModel(searchHistoryProxyModel);
            ui->Search_treeView_History->header()->setSectionResizeMode(QHeaderView::Interactive);
            ui->Search_treeView_History->header()->resizeSection(0, 150); //Date
        }
