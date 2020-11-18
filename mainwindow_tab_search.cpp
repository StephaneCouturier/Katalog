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
// File Name:   mainwindow_tab_search.cpp
// Purpose:
// Description:
// Author:      Stephane Couturier
// Modified by: Stephane Couturier
// Created:     2020-07-11
// Version:     0.8
/////////////////////////////////////////////////////////////////////////////
*/

#include "mainwindow.h"
#include "ui_mainwindow.h"
#include "catalog.h"

#include <QTextStream>
#include <QDesktopServices>
#include <QSaveFile>
#include <QFileDialog>
#include <QMessageBox>

//#include <KMessageBox>
//#include <KLocalizedString>

//TAB: SEARCH FILES ----------------------------------------------------------------------

        //----------------------------------------------------------------------
        //User interactions
        void MainWindow::on_Search_pushButton_ResetAll_clicked()
        {
            ui->Search_kcombobox_SearchText->setCurrentText("");
            ui->Search_comboBox_SelectCatalog->setCurrentText("All");
            ui->Search_comboBox_TextCriteria->setCurrentText("All Words");
            ui->Search_comboBox_SearchIn->setCurrentText("File names or Folder paths");
            ui->Search_comboBox_FileType->setCurrentText("Any");
            ui->Search_spinBox_FileTypeMinimumSize->setValue(0);
            ui->Search_spinBox_MaximumSize->setValue(1000);
            ui->Search_comboBox_MinSizeUnit->setCurrentText("Byte");
            ui->Search_comboBox_MaxSizeUnit->setCurrentText("GiB");
            ui->Search_checkBox_ShowFolders->setChecked(false);
            //ui->LE_Tags->setCurrentText("");

            //Clear catalog and file results (load an empty model)
            Catalog *empty = new Catalog(this);
            ui->Search_treeView_FilesFound->setModel(empty);
            ui->Search_listView_CatalogsFound->setModel(empty);

        }

        //----------------------------------------------------------------------
        void MainWindow::on_Search_pushButton_PasteFromClipboard_clicked()
        {
            QClipboard *clipboard = QGuiApplication::clipboard();
            QString originalText = clipboard->text();
            //clipboard->setText(selectedFile);
            ui->Search_kcombobox_SearchText->setCurrentText(originalText);
        }
        //----------------------------------------------------------------------
        void MainWindow::on_Search_pushButton_Search_clicked()
        {
            searchFiles();
        }
        //----------------------------------------------------------------------
        void MainWindow::on_Search_kcombobox_SearchText_returnPressed()
        {
            searchFiles();
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
            //KMessageBox::information(this,"test:\n did nothing."+selectedFile);
        }
        //----------------------------------------------------------------------
        void MainWindow::on_Search_pushButton_ExportResults_clicked()
        {
            //Prepare export file name
            QDateTime now = QDateTime::currentDateTime();
            QString timestamp = now.toString(QLatin1String("yyyyMMdd-hhmmss"));
            QString filename = QString::fromLatin1("/search_results_%1.csv").arg(timestamp);
            filename=collectionFolder+filename;
            QFile exportFile(filename);

            //QFile exportFile(collectionFolder+"/file.txt");

              if (exportFile.open(QFile::WriteOnly | QFile::Text)) {

                  QTextStream stream(&exportFile);

                  for (int i = 0; i < filesFoundList.size(); ++i)
                    {
                      QString line = sFilePaths[i] + "/" + sFileNames[i] + "\t" + QString::number(sFileSizes[i]) + "\t" + sFileDateTimes[i];
                      stream << line << '\n';
                    }
                }

              QMessageBox::information(this,"Katalog","Results exported to the collection folder:\n"+exportFile.fileName());
              exportFile.close();
        }
        //----------------------------------------------------------------------
        void MainWindow::on_Search_listView_CatalogsFound_clicked(const QModelIndex &index)
        {
            //Refine the seach with the selction of one of the catalogs that have results

            //Get file from selected row
            QString selectedCatalogName = ui->Search_listView_CatalogsFound->model()->index(index.row(), 0, QModelIndex()).data().toString();
            //selectedCatalogPath = collectionFolder + "/" + selectedCatalogName + ".idx";
            ui->Search_comboBox_SelectCatalog->setCurrentText(selectedCatalogName);
            //Seach again but only on the selected catalog
            searchFiles();
        }
        //----------------------------------------------------------------------
        void MainWindow::on_Search_pushButton_ShowHideCatalogResults_clicked()
        {
            QString visible = ui->Search_pushButton_ShowHideCatalogResults->text();

            if ( visible == "<<"){ //Hide
                    ui->Search_pushButton_ShowHideCatalogResults->setText(">>");
                    ui->Search_listView_CatalogsFound->setHidden(true);
                    ui->Search_label_CatalogsWithResults->setHidden(true);

                    QSettings settings(settingsFile, QSettings:: IniFormat);
                    settings.setValue("Settings/ShowHideCatalogResults", ui->Search_pushButton_ShowHideCatalogResults->text());
            }
            else{ //Show
                    ui->Search_pushButton_ShowHideCatalogResults->setText("<<");
                    ui->Search_listView_CatalogsFound->setHidden(false);
                    ui->Search_label_CatalogsWithResults->setHidden(false);

                    QSettings settings(settingsFile, QSettings:: IniFormat);
                    settings.setValue("Settings/ShowHideCatalogResults", ui->Search_pushButton_ShowHideCatalogResults->text());
            }

        }

        //----------------------------------------------------------------------
        //File Context Menu actions set up
        void MainWindow::on_Search_treeView_FilesFound_customContextMenuRequested(const QPoint &pos)
        {
            //KMessageBox::information(this,"test.");

            // for most widgets
            QPoint globalPos = ui->Search_treeView_FilesFound->mapToGlobal(pos);
            // for QAbstractScrollArea and derived classes you would use:
            //QPoint globalPos = myWidget->viewport()->mapToGlobal(pos);
            QMenu fileContextMenu;

            QAction *menuAction1 = new QAction(QIcon::fromTheme("document-open"),(tr("Open file")), this);
            connect(menuAction1, &QAction::triggered, this, &MainWindow::contextOpenFile);
            fileContextMenu.addAction(menuAction1);

            QAction *menuAction2 = new QAction(QIcon::fromTheme("document-open"),(tr("Open folder")), this);
            connect(menuAction2, &QAction::triggered, this, &MainWindow::contextOpenFolder);
            fileContextMenu.addAction(menuAction2);

            fileContextMenu.addSeparator();

            QAction *menuAction3 = new QAction(QIcon::fromTheme("edit-copy"),(tr("Copy folder path")), this);
            connect( menuAction3,&QAction::triggered, this, &MainWindow::contextCopyFolderPath);
            fileContextMenu.addAction(menuAction3);

            QAction *menuAction4 = new QAction(QIcon::fromTheme("edit-copy"),(tr("Copy file absolute path")), this);
            connect( menuAction4,&QAction::triggered, this, &MainWindow::contextCopyAbsolutePath);
            fileContextMenu.addAction(menuAction4);

            QAction *menuAction5 = new QAction(QIcon::fromTheme("edit-copy"),(tr("Copy file name with extension")), this);
            connect( menuAction5,&QAction::triggered, this, &MainWindow::contextCopyFileNameWithExtension);
            fileContextMenu.addAction(menuAction5);

            QAction *menuAction6 = new QAction(QIcon::fromTheme("edit-copy"),(tr("Copy file name without extension")), this);
            connect( menuAction6,&QAction::triggered, this, &MainWindow::contextCopyFileNameWithoutExtension);
            fileContextMenu.addAction(menuAction6);

            //fileContextMenu.addSeparator();

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

        //----------------------------------------------------------------------
        //Context Menu methods
        void MainWindow::contextOpenFile()
        {
            QModelIndex index=ui->Search_treeView_FilesFound->currentIndex();
            //Get filepath from selected row
            QString selectedFileName   = ui->Search_treeView_FilesFound->model()->index(index.row(), 0, QModelIndex()).data().toString();
            QString selectedFileFolder = ui->Search_treeView_FilesFound->model()->index(index.row(), 3, QModelIndex()).data().toString();
            QString selectedFile = selectedFileFolder+"/"+selectedFileName;
            //Open the file (fromLocalFile needed for spaces in file name)
            QDesktopServices::openUrl(QUrl::fromLocalFile(selectedFile));
        }
        void MainWindow::contextOpenFolder()
        {
            QModelIndex index = ui->Search_treeView_FilesFound->currentIndex();
            QString selectedFileName   = ui->Search_treeView_FilesFound->model()->index(index.row(), 0, QModelIndex()).data().toString();
            QString selectedFileFolder = ui->Search_treeView_FilesFound->model()->index(index.row(), 3, QModelIndex()).data().toString();
            QString selectedFile = selectedFileFolder+"/"+selectedFileName;
            QString folderName = selectedFile.left(selectedFile.lastIndexOf("/"));
            QDesktopServices::openUrl(QUrl::fromLocalFile(folderName));
        }
        void MainWindow::contextCopyAbsolutePath()
        {
            QModelIndex index=ui->Search_treeView_FilesFound->currentIndex();

            QString selectedFileName =   ui->Search_treeView_FilesFound->model()->index(index.row(), 0, QModelIndex()).data().toString();
            QString selectedFileFolder = ui->Search_treeView_FilesFound->model()->index(index.row(), 3, QModelIndex()).data().toString();
            QString selectedFile = selectedFileFolder+"/"+selectedFileName;

            QClipboard *clipboard = QGuiApplication::clipboard();
            QString originalText = clipboard->text();
            clipboard->setText(selectedFile);
        }
        void MainWindow::contextCopyFolderPath()
        {
            QModelIndex index = ui->Search_treeView_FilesFound->currentIndex();
            QString selectedFileFolder = ui->Search_treeView_FilesFound->model()->index(index.row(), 3, QModelIndex()).data().toString();

            QClipboard *clipboard = QGuiApplication::clipboard();
            QString originalText = clipboard->text();
            clipboard->setText(selectedFileFolder);
        }
        void MainWindow::contextCopyFileNameWithExtension()
        {
            QModelIndex index = ui->Search_treeView_FilesFound->currentIndex();
            QString selectedFileName = ui->Search_treeView_FilesFound->model()->index(index.row(), 0, QModelIndex()).data().toString();

            QString fileNameWithExtension = selectedFileName;

            QClipboard *clipboard = QGuiApplication::clipboard();
            QString originalText = clipboard->text();
            clipboard->setText(fileNameWithExtension);
        }
        void MainWindow::contextCopyFileNameWithoutExtension()
        {
            QModelIndex index=ui->Search_treeView_FilesFound->currentIndex();

            QString selectedFileName   = ui->Search_treeView_FilesFound->model()->index(index.row(), 0, QModelIndex()).data().toString();
            QString selectedFileFolder = ui->Search_treeView_FilesFound->model()->index(index.row(), 3, QModelIndex()).data().toString();
            QString selectedFile = selectedFileFolder+"/"+selectedFileName;

            QFileInfo fileName;
            fileName.setFile(selectedFile);
            QString fileNameWithoutExtension = fileName.baseName();

            QClipboard *clipboard = QGuiApplication::clipboard();
            QString originalText = clipboard->text();
            clipboard->setText(fileNameWithoutExtension);
        }
        //----------------------------------------------------------------------

    //Search methods
        void MainWindow::searchFiles()
        {
            // Start animation while opening
            QApplication::setOverrideCursor(Qt::WaitCursor);

            //Set up Search
                //Clear exisitng lists of results and search variables
                    filesFoundList.clear();
                    catalogFoundList.clear();

                    //Set up temporary lists
                    sFileNames.clear();
                    sFileSizes.clear();
                    sFilePaths.clear();
                    sFileDateTimes.clear();

                //Get search criteria
                    //Get text to search in file names or directories
                    searchText = ui->Search_kcombobox_SearchText->currentText();

                    //Do nothing if there is no search text
                    if (searchText=="")
                        return;

                    //add searchText to a list, to retrieved it later
                    ui->Search_kcombobox_SearchText->addItem(searchText);

                    //Get other search criteria
                    selectedSearchCatalog = ui->Search_comboBox_SelectCatalog->currentText();
                    selectedTextCriteria  = ui->Search_comboBox_TextCriteria->currentText();
                    selectedSearchIn      = ui->Search_comboBox_SearchIn->currentText();
                    selectedFileType      = ui->Search_comboBox_FileType->currentText();
                    selectedMinimumSize   = ui->Search_spinBox_FileTypeMinimumSize->value();
                    selectedMaximumSize   = ui->Search_spinBox_MaximumSize->value();
                    selectedMinSizeUnit   = ui->Search_comboBox_MinSizeUnit->currentText();
                    selectedMaxSizeUnit   = ui->Search_comboBox_MaxSizeUnit->currentText();
                    //selectedTags        = ui->LE_Tags->text();

                    // User can enter size min anx max from 0 to 1000.
                    // Define a size multiplier depending on the size unit selected
                    sizeMultiplierMin=1;
                    if      (selectedMinSizeUnit =="KiB")
                            sizeMultiplierMin = sizeMultiplierMin * 1024;
                    else if (selectedMinSizeUnit =="MiB")
                            sizeMultiplierMin = sizeMultiplierMin *1024*1024;
                    else if (selectedMinSizeUnit =="GiB")
                            sizeMultiplierMin = sizeMultiplierMin *1024*1024*1024;
                    sizeMultiplierMax=1;
                    if      (selectedMaxSizeUnit =="KiB")
                            sizeMultiplierMax = sizeMultiplierMax * 1024;
                    else if (selectedMaxSizeUnit =="MiB")
                            sizeMultiplierMax = sizeMultiplierMax *1024*1024;
                    else if (selectedMaxSizeUnit =="GiB")
                            sizeMultiplierMax = sizeMultiplierMax *1024*1024*1024;

                 // Searching "Begin With" for File name or Folder name is not supported yet
                    if (selectedTextCriteria=="Begins With" and selectedSearchIn =="File names or Folder paths"){
                        QMessageBox::information(this,"Katalog","Using 'Begin With' with 'File names or Folder names' is not supported yet.\nPlease try a different combinaison.");
                        return;;
                    }

            //Execute the search
                //Start animation while searching
                //QApplication::setOverrideCursor(Qt::WaitCursor);

                //Search every catalog if "All" is selected
                if ( selectedSearchCatalog =="All"){
                    foreach(sourceCatalog,catalogFileList)
                            {
                                searchFilesInCatalog(sourceCatalog);
                            }
                    }
                //Search catalogs matching the storage if "Selectd storage" is selected
                else if ( selectedSearchCatalog =="Selected Storage"){

                    foreach(sourceCatalog,catalogFileList)
                            {

                            //get catalog storage name
                            QString sourceCatalogStorageName;
                            QString currentCatalogFilePath = collectionFolder + "/" + sourceCatalog + ".idx";
                            sourceCatalogStorageName = getCatalogStorageName(currentCatalogFilePath);
                                if  ( selectedStorageName == sourceCatalogStorageName )
                                {
                                        searchFilesInCatalog(sourceCatalog);
                                }
                            }
                }
                else if ( selectedSearchCatalog =="Selected Location"){
                    getLocationCatalogList(selectedStorageLocation);
                    foreach(sourceCatalog,locationCatalogList)
                            {
                                searchFilesInCatalog(sourceCatalog);
                            }
                }
                //Otherwise just search files in the selected catalog
                else{
                    //QString selectedSearchCatalogPath;
                    searchFilesInCatalog(selectedSearchCatalog);
                }

            //Process search results: list of catalogs
                //Remove duplicates so the catalogs are listed only once
                catalogFoundList.removeDuplicates();

                //Keep the catalog file name only
                foreach(QString item, catalogFoundList){
                        int index = catalogFoundList.indexOf(item);
                        //QDir dir(item);
                        QFileInfo fileInfo(item);

                        //catalogFoundList[index] = dir.dirName();
                        catalogFoundList[index] = fileInfo.baseName();
                }

                //Load list of catalogs in which files where found
                catalogFoundListModel = new QStringListModel(this);
                catalogFoundListModel->setStringList(catalogFoundList);
                ui->Search_listView_CatalogsFound->setModel(catalogFoundListModel);

            //Process search results: list of files
                // Create model
                Catalog *searchResultsCatalog = new Catalog(this);              

                if ( ui->Search_checkBox_ShowFolders->isChecked()==true )
                {
                    //show folders only:

                    // Populate model with data
                    sFilePaths.removeDuplicates();
                    int numberOfFolders = sFilePaths.count();
                    sFileNames.clear();
                    sFileSizes.clear();
                    sFileDateTimes.clear();
                    for (int i=0; i<numberOfFolders; i++)
                        sFileNames <<"";
                    for (int i=0; i<numberOfFolders; i++)
                        sFileSizes <<0;
                    for (int i=0; i<numberOfFolders; i++)
                        sFileDateTimes <<"";

                    searchResultsCatalog->populateFileData(sFileNames, sFileSizes, sFilePaths, sFileDateTimes);
                    QSortFilterProxyModel *proxyModel = new QSortFilterProxyModel(this);
                    proxyModel->setSourceModel(searchResultsCatalog);

                    // Connect model to tree/table view
                    ui->Search_treeView_FilesFound->setModel(proxyModel);
                    ui->Search_treeView_FilesFound->QTreeView::sortByColumn(0,Qt::AscendingOrder);
                    //ui->TrV_FilesFound->header()->setSectionResizeMode(QHeaderView::ResizeToContents);
                    //ui->TrV_FilesFound->header()->setSectionResizeMode(QHeaderView::Interactive);
                    //ui->TrV_FilesFound->header()->resizeSection(0, 600); //Name
                    //ui->TrV_FilesFound->header()->resizeSection(1, 110); //Size
                    //ui->TrV_FilesFound->header()->resizeSection(2, 140); //Date
                    ui->Search_treeView_FilesFound->header()->resizeSection(3, 400); //Path

                    ui->Search_treeView_FilesFound->header()->hideSection(0);
                    ui->Search_treeView_FilesFound->header()->hideSection(1);
                    ui->Search_treeView_FilesFound->header()->hideSection(2);

                    ui->Search_label_FoundTitle->setText("Folders found:");
                }
                else
                {
                    // Populate model with data
                        searchResultsCatalog->populateFileData(sFileNames, sFileSizes, sFilePaths, sFileDateTimes);
                        QSortFilterProxyModel *proxyModel = new QSortFilterProxyModel(this);
                        proxyModel->setSourceModel(searchResultsCatalog);

                        // Connect model to tree/table view
                        ui->Search_treeView_FilesFound->setModel(proxyModel);
                        ui->Search_treeView_FilesFound->QTreeView::sortByColumn(0,Qt::AscendingOrder);
                        //ui->TrV_FilesFound->header()->setSectionResizeMode(QHeaderView::ResizeToContents);
                        ui->Search_treeView_FilesFound->header()->setSectionResizeMode(QHeaderView::Interactive);
                        ui->Search_treeView_FilesFound->header()->resizeSection(0, 600); //Name
                        ui->Search_treeView_FilesFound->header()->resizeSection(1, 110); //Size
                        ui->Search_treeView_FilesFound->header()->resizeSection(2, 140); //Date
                        ui->Search_treeView_FilesFound->header()->resizeSection(3, 400); //Path
                        ui->Search_treeView_FilesFound->header()->showSection(0);
                        ui->Search_treeView_FilesFound->header()->showSection(1);
                        ui->Search_treeView_FilesFound->header()->showSection(2);
                        ui->Search_label_FoundTitle->setText("Files found:");
                }

                //Count and display the number of files found
                int numberFilesResult = searchResultsCatalog->rowCount();
                ui->Search_label_NumberResults->setNum(numberFilesResult);

                //Save the search parameters to the seetings file
                saveSettings();

                //Stop animation
                QApplication::restoreOverrideCursor();

        }

        //----------------------------------------------------------------------
        void MainWindow::searchFilesInCatalog(const QString &sourceCatalogName)
        {
            QString sourceCatalogPath = collectionFolder + "/" + sourceCatalogName + ".idx";
            QFile catalogFile(sourceCatalogPath);

            //Define how to use the search text
                if(selectedTextCriteria == "Exact Phrase")
                    regexSearchtext=searchText; //just search for the extact text entered including spaces, as one text string.
                else if(selectedTextCriteria == "Begins With")
                    regexSearchtext="(^"+searchText+")";
                else if(selectedTextCriteria == "Any Word")
                    regexSearchtext=searchText.replace(" ","|");
                else if(selectedTextCriteria == "All Words"){
                    QString searchTextToSplit = searchText;
                    QString groupRegEx = "";
                    QRegExp lineSplitExp(" ");
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
            if(selectedFileType !="Any"){
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

            ui->Search_label_Regex->setText(regexPattern);
            QRegularExpression regex(regexPattern, QRegularExpression::CaseInsensitiveOption);

            //Search loop for all lines in the catalog file
            if (catalogFile.open(QIODevice::ReadOnly|QIODevice::Text)) {

                //Set up a stream from the file's data
                QTextStream stream(&catalogFile);
                QString line;
                QString reducedLine;

                //Line by Line, test if the file is matching all search criteria
                do {
                    //Get line / file data
                        line = stream.readLine();
                        QRegularExpressionMatch match;
                        QRegularExpressionMatch foldermatch;
                        //QRegularExpressionMatch matchFileType;

                        //Split the line text with tabulations into a list
                        QRegExp     lineSplitExp("\t");
                        QStringList lineFieldList  = line.split(lineSplitExp);
                        int         fieldListCount = lineFieldList.count();

                        // Get the file absolute path and the file size from this list
                        QString     lineFilePath   = lineFieldList[0];

                        // Get the FileSize from the list if available
                        qint64      lineFileSize;
                        if (fieldListCount == 3){
                                lineFileSize = lineFieldList[1].toLongLong();}
                        else lineFileSize = 0;

                    //Exclude catalog metadata lines which are starting with the character <
                         if (lineFilePath.left(1)=="<"){continue;}

                    //PREDEV: continue if the folder has a matching tag
                        //selectedTags
                        //if (selectedTags == selectedTags){continue;}

                    //continue if the file is matching the size range
                        if ( !(     lineFileSize >= selectedMinimumSize * sizeMultiplierMin
                                and lineFileSize <= selectedMaximumSize * sizeMultiplierMax) ){
                                    continue;}

                    //Finally, verify the text search criteria
                        //Depending on the "Search in" criteria,
                        //reduce the abosulte path to the reaquired text string and match the search text
                        if(selectedSearchIn == "File names only")
                        {
                            // Extract the file name from the lineFilePath
                            QFileInfo file(lineFilePath);
                            reducedLine = file.fileName();

                            match = regex.match(reducedLine);
                        }
                        else if(selectedSearchIn == "Folder path only")
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
                    if (match.hasMatch()){
                        filesFoundList << lineFilePath;
                        catalogFoundList.insert(0,sourceCatalogName);

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
                    }

                } while(!line.isNull());
            }
            else return;

        }
        //----------------------------------------------------------------------
        QString MainWindow::getCatalogStorageName(QString catalogFilePath)
        {
            QString catalogStorageName;
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
        //------------------------------------------------------------------------------------------
        void MainWindow::getLocationCatalogList(QString location)
        {
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
                    //KMessageBox::information(this,"test:\n"+ location + fieldList[0] );
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
        //------------------------------------------------------------------------------------------
        //UI methods
            //Set up
            void MainWindow::initiateSearchValues()
            {
                //Prepare list of size units for the Catalog selection combobox
                // the first line is the one displayed by default
                ui->Search_comboBox_MinSizeUnit->addItem("GiB");
                ui->Search_comboBox_MinSizeUnit->addItem("MiB");
                ui->Search_comboBox_MinSizeUnit->addItem("KiB");
                ui->Search_comboBox_MinSizeUnit->addItem("Bytes");
                ui->Search_comboBox_MaxSizeUnit->addItem("GiB");
                ui->Search_comboBox_MaxSizeUnit->addItem("MiB");
                ui->Search_comboBox_MaxSizeUnit->addItem("KiB");
                ui->Search_comboBox_MaxSizeUnit->addItem("Bytes");

                //Load last search values (from settings file)
                    if (selectedMaximumSize ==0)
                        selectedMaximumSize = 1000;

                //Set values
                    ui->Search_comboBox_SelectCatalog->setCurrentText(selectedSearchCatalog);
                    ui->Search_comboBox_TextCriteria->setCurrentText(selectedTextCriteria);
                    ui->Search_comboBox_SearchIn->setCurrentText(selectedSearchIn);
                    ui->Search_comboBox_FileType->setCurrentText(selectedFileType);
                    ui->Search_spinBox_FileTypeMinimumSize->setValue(selectedMinimumSize);
                    ui->Search_spinBox_MaximumSize->setValue(selectedMaximumSize);
                    ui->Search_comboBox_MinSizeUnit->setCurrentText(selectedMinSizeUnit);
                    ui->Search_comboBox_MaxSizeUnit->setCurrentText(selectedMaxSizeUnit);
            }
            //----------------------------------------------------------------------
            void MainWindow::refreshCatalogSelectionList()
            {
                //Send list to the Statistics combobox (without All or Selected storage options)
                fileListModel = new QStringListModel(this);
                fileListModel->setStringList(catalogFileList);
                ui->Statistics_comboBox_SelectCatalog->setModel(fileListModel);

                //Prepare list for the Catalog combobox
                QStringList displaycatalogList = catalogFileList;

                //Add the option All at the beginning
                displaycatalogList.insert(0,"All");
                displaycatalogList.insert(1,"Selected Storage");
                displaycatalogList.insert(2,"Selected Location");
                //catalogFileList = cNames;
                fileListModel->setStringList(displaycatalogList);
                //Send list to the Search combobox (with All on Slecteed storage options)
                ui->Search_comboBox_SelectCatalog->setModel(fileListModel);
            }
