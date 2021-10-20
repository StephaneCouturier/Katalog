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
// Purpose:     methods for the screen Search
// Description:
// Author:      Stephane Couturier
// Version:     1.02
/////////////////////////////////////////////////////////////////////////////
*/

#include "mainwindow.h"
#include "ui_mainwindow.h"
#include "catalog.h"
#include "database.h"
#include "filesview.h"

#include <QTextStream>
#include <QDesktopServices>
#include <QSaveFile>
#include <QFileDialog>
#include <QMessageBox>

//TAB: SEARCH FILES ----------------------------------------------------------------------

    //----------------------------------------------------------------------
    //User interactions
        //Buttons and other changes
        void MainWindow::on_Search_kcombobox_SearchText_returnPressed()
        {
            searchFiles();
        }
        void MainWindow::on_Search_lineEdit_SearchText_returnPressed()
        {
            searchFiles();
        }

        //----------------------------------------------------------------------
        void MainWindow::on_Search_pushButton_PasteFromClipboard_clicked()
        {
            QClipboard *clipboard = QGuiApplication::clipboard();
            QString originalText = clipboard->text();
            //clipboard->setText(selectedFile);
            #ifdef Q_OS_LINUX
                ui->Search_kcombobox_SearchText->setCurrentText(originalText);
            #else
                ui->Search_lineEdit_SearchText->setText(originalText);
            #endif
        }
        //----------------------------------------------------------------------
        void MainWindow::on_Search_pushButton_Search_clicked()
        {
            searchFiles();
        }
        //----------------------------------------------------------------------
        void MainWindow::on_Search_pushButton_ResetAll_clicked()
        {
            #ifdef Q_OS_LINUX
                ui->Search_kcombobox_SearchText->setCurrentText("");
            #else
                ui->Search_lineEdit_SearchText->setText("");
            #endif
            ui->Search_comboBox_TextCriteria->setCurrentText(tr("All Words"));
            ui->Search_comboBox_SearchIn->setCurrentText(tr("File names or Folder paths"));
            ui->Search_comboBox_FileType->setCurrentText(tr("All"));
            ui->Search_spinBox_FileTypeMinimumSize->setValue(0);
            ui->Search_spinBox_MaximumSize->setValue(999);
            ui->Search_comboBox_MinSizeUnit->setCurrentText(tr("Bytes"));
            ui->Search_comboBox_MaxSizeUnit->setCurrentText(tr("GiB"));
            ui->Search_checkBox_ShowFolders->setChecked(false);
            ui->Search_label_NumberResults->setText("");

            //Clear catalog and file results (load an empty model)
            Catalog *empty = new Catalog(this);
            ui->Search_treeView_FilesFound->setModel(empty);
            ui->Search_listView_CatalogsFound->setModel(empty);

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

              QMessageBox::information(this,"Katalog",tr("Results exported to the collection folder:")+"\n"+exportFile.fileName());
              exportFile.close();
        }
        //----------------------------------------------------------------------
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
        void MainWindow::on_Search_listView_CatalogsFound_clicked(const QModelIndex &index)
        {
            //Refine the seach with the selction of one of the catalogs that have results

            //Get file from selected row
            QString selectedCatalogName = ui->Search_listView_CatalogsFound->model()->index(index.row(), 0, QModelIndex()).data().toString();
            //selectedCatalogPath = collectionFolder + "/" + selectedCatalogName + ".idx";
            ui->Filters_comboBox_SelectCatalog->setCurrentText(selectedCatalogName);
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

                    QSettings settings(settingsFilePath, QSettings:: IniFormat);
                    settings.setValue("Settings/ShowHideCatalogResults", ui->Search_pushButton_ShowHideCatalogResults->text());
            }
            else{ //Show
                    ui->Search_pushButton_ShowHideCatalogResults->setText("<<");
                    ui->Search_listView_CatalogsFound->setHidden(false);
                    ui->Search_label_CatalogsWithResults->setHidden(false);

                    QSettings settings(settingsFilePath, QSettings:: IniFormat);
                    settings.setValue("Settings/ShowHideCatalogResults", ui->Search_pushButton_ShowHideCatalogResults->text());
            }

        }
        //----------------------------------------------------------------------

        //File Context Menu actions set up
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

        void MainWindow::searchContextOpenFolder()
        {
            QModelIndex index = ui->Search_treeView_FilesFound->currentIndex();
            QString selectedFileName   = ui->Search_treeView_FilesFound->model()->index(index.row(), 0, QModelIndex()).data().toString();
            QString selectedFileFolder = ui->Search_treeView_FilesFound->model()->index(index.row(), 3, QModelIndex()).data().toString();
            QString selectedFile = selectedFileFolder+"/"+selectedFileName;
            QString folderName = selectedFile.left(selectedFile.lastIndexOf("/"));
            QDesktopServices::openUrl(QUrl::fromLocalFile(folderName));
        }

        void MainWindow::searchContextCopyAbsolutePath()
        {
            QModelIndex index=ui->Search_treeView_FilesFound->currentIndex();

            QString selectedFileName =   ui->Search_treeView_FilesFound->model()->index(index.row(), 0, QModelIndex()).data().toString();
            QString selectedFileFolder = ui->Search_treeView_FilesFound->model()->index(index.row(), 3, QModelIndex()).data().toString();
            QString selectedFile = selectedFileFolder+"/"+selectedFileName;

            QClipboard *clipboard = QGuiApplication::clipboard();
            QString originalText = clipboard->text();
            clipboard->setText(selectedFile);
        }

        void MainWindow::searchContextCopyFolderPath()
        {
            QModelIndex index = ui->Search_treeView_FilesFound->currentIndex();
            QString selectedFileFolder = ui->Search_treeView_FilesFound->model()->index(index.row(), 3, QModelIndex()).data().toString();

            QClipboard *clipboard = QGuiApplication::clipboard();
            QString originalText = clipboard->text();
            clipboard->setText(selectedFileFolder);
        }

        void MainWindow::searchContextCopyFileNameWithExtension()
        {
            QModelIndex index = ui->Search_treeView_FilesFound->currentIndex();
            QString selectedFileName = ui->Search_treeView_FilesFound->model()->index(index.row(), 0, QModelIndex()).data().toString();

            QString fileNameWithExtension = selectedFileName;

            QClipboard *clipboard = QGuiApplication::clipboard();
            QString originalText = clipboard->text();
            clipboard->setText(fileNameWithExtension);
        }

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
                    #ifdef Q_OS_LINUX
                    searchText = ui->Search_kcombobox_SearchText->currentText();
                    #else
                    searchText = ui->Search_lineEdit_SearchText->text();
                    #endif

                    //Do nothing if there is no search text
                    if (searchText=="")
                        return;

                    //add searchText to a list, to retrieved it later
                    #ifdef Q_OS_LINUX
                        ui->Search_kcombobox_SearchText->addItem(searchText);
                    #else
                        //no alternative for win
                    #endif

                    //Get other search criteria
                    selectedSearchLocation = ui->Filters_comboBox_SelectLocation->currentText();
                    selectedSearchStorage = ui->Filters_comboBox_SelectStorage->currentText();
                    selectedSearchCatalog = ui->Filters_comboBox_SelectCatalog->currentText();
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
                    if (selectedTextCriteria==tr("Begins With") and selectedSearchIn ==tr("File names or Folder paths")){
                        QMessageBox::information(this,"Katalog",tr("Using 'Begin With' with 'File names or Folder names' is not supported yet.\nPlease try a different combinaison."));
                        return;;
                    }

            //List of catalogs to search from   catalogSelectedList

            //Execute the search
                //Start animation while searching
                //QApplication::setOverrideCursor(Qt::WaitCursor);

                //Search every catalog if "All" is selected
                if ( selectedSearchCatalog ==tr("All")){
                    foreach(sourceCatalog,catalogSelectedList)
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
                catalogFoundList.sort();

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

                    proxyModel->setHeaderData(0, Qt::Horizontal, tr("Name"));
                    proxyModel->setHeaderData(1, Qt::Horizontal, tr("Size"));
                    proxyModel->setHeaderData(2, Qt::Horizontal, tr("Date"));
                    proxyModel->setHeaderData(3, Qt::Horizontal, tr("Folder"));

                    // Connect model to tree/table view
                    ui->Search_treeView_FilesFound->setModel(proxyModel);
                    ui->Search_treeView_FilesFound->QTreeView::sortByColumn(0,Qt::AscendingOrder);
                    //ui->Search_treeView_FilesFound->header()->setSectionResizeMode(QHeaderView::ResizeToContents);
                    //ui->Search_treeView_FilesFound->header()->setSectionResizeMode(QHeaderView::Interactive);
                    ui->Search_treeView_FilesFound->header()->resizeSection(3, 400); //Path

                    ui->Search_treeView_FilesFound->header()->hideSection(0);
                    ui->Search_treeView_FilesFound->header()->hideSection(1);
                    ui->Search_treeView_FilesFound->header()->hideSection(2);

                    ui->Search_label_FoundTitle->setText(tr("Folders found"));
                }
                else
                {
                    // Populate model with data
                        searchResultsCatalog->populateFileData(sFileNames, sFileSizes, sFilePaths, sFileDateTimes);
                        QSortFilterProxyModel *proxyModel = new QSortFilterProxyModel(this);
                        proxyModel->setSourceModel(searchResultsCatalog);

                        FilesView *proxyModel2 = new FilesView(this);
                        proxyModel2->setSourceModel(searchResultsCatalog);

                        proxyModel2->setHeaderData(0, Qt::Horizontal, tr("Name"));
                        proxyModel2->setHeaderData(1, Qt::Horizontal, tr("Size"));
                        proxyModel2->setHeaderData(2, Qt::Horizontal, tr("Date"));
                        proxyModel2->setHeaderData(3, Qt::Horizontal, tr("Folder"));

                        // Connect model to tree/table view
                        ui->Search_treeView_FilesFound->setModel(proxyModel2);
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
                        ui->Search_label_FoundTitle->setText(tr("Files found"));
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
                if(selectedTextCriteria == tr("Exact Phrase"))
                    regexSearchtext=searchText; //just search for the extact text entered including spaces, as one text string.
                else if(selectedTextCriteria == tr("Begins With"))
                    regexSearchtext="(^"+searchText+")";
                else if(selectedTextCriteria == tr("Any Word"))
                    regexSearchtext=searchText.replace(" ","|");
                else if(selectedTextCriteria == tr("All Words")){
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
            if(selectedFileType !=tr("All")){
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
            //DEV: REPLACE BY SQL QUERY ON Catalog TABLE

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
        //------------------------------------------------------------------------------------------
        //UI methods
            //Set up
            void MainWindow::initiateSearchValues()
            {
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

                //Set values
                    ui->Search_comboBox_TextCriteria->setCurrentText(selectedTextCriteria);
                    ui->Search_comboBox_SearchIn->setCurrentText(selectedSearchIn);
                    ui->Search_comboBox_FileType->setCurrentText(selectedFileType);
                    ui->Search_spinBox_FileTypeMinimumSize->setValue(selectedMinimumSize);
                    ui->Search_spinBox_MaximumSize->setValue(selectedMaximumSize);
                    ui->Search_comboBox_MinSizeUnit->setCurrentText(selectedMinSizeUnit);
                    ui->Search_comboBox_MaxSizeUnit->setCurrentText(selectedMaxSizeUnit);
            }
            //----------------------------------------------------------------------
            void MainWindow::refreshLocationSelectionList()
            {
                //get current location
                QString currentLocation = ui->Filters_comboBox_SelectLocation->currentText();
                //Query the full list of locations
                QSqlQuery getLocationList;
                getLocationList.prepare("SELECT DISTINCT storageLocation FROM storage");
                getLocationList.exec();

                //Put the results in a list
                QStringList locationList;
                while (getLocationList.next()) {
                    locationList << getLocationList.value(0).toString();
                }


                //Prepare list for the Location combobox
                QStringList displayLocationList = locationList;
                //Add the "All" option at the beginning
                displayLocationList.insert(0,tr("All"));

                //Send the list to the Search combobox model
                fileListModel = new QStringListModel(this);
                fileListModel->setStringList(displayLocationList);
                ui->Filters_comboBox_SelectLocation->setModel(fileListModel);

                //Restore last selection
                ui->Filters_comboBox_SelectLocation->setCurrentText(currentLocation);

            }
            //----------------------------------------------------------------------
            void MainWindow::refreshStorageSelectionList(QString selectedLocation)
            {
                //get current location
                QString currentStorage = ui->Filters_comboBox_SelectStorage->currentText();

                //Query the full list of locations
                QSqlQuery getStorageList;

                QString queryText = "SELECT DISTINCT storageName FROM storage";

                if ( selectedLocation == tr("All")){
                        getStorageList.prepare(queryText);
                }
                else{
                        queryText = queryText + " WHERE storageLocation ='" + selectedLocation + "'";
                        getStorageList.prepare(queryText);
                }
                getStorageList.exec();

                //Put the results in a list
                QStringList locationList;
                while (getStorageList.next()) {
                    locationList << getStorageList.value(0).toString();
                }


                //Prepare list for the Location combobox
                QStringList displayLocationList = locationList;
                //Add the "All" option at the beginning
                displayLocationList.insert(0,tr("All"));

                //Send the list to the Search combobox model
                fileListModel = new QStringListModel(this);
                fileListModel->setStringList(displayLocationList);
                ui->Filters_comboBox_SelectStorage->setModel(fileListModel);

                //Restore last selection
                ui->Filters_comboBox_SelectStorage->setCurrentText(currentStorage);

            }
            //----------------------------------------------------------------------
            void MainWindow::refreshCatalogSelectionList(QString selectedLocation, QString selectedStorage)
            {
                //get current location
                QString currentCatalog = ui->Filters_comboBox_SelectCatalog->currentText();

                //Query the full list of locations
                QSqlQuery getCatalogSelectionList;

                //Prepare and run the query
                    //common
                    QString queryText = "SELECT c.catalogName FROM catalog AS c"
                                        " left JOIN storage AS s ON c.catalogStorage = s.storageName";

                    //filter depending on location and selection.
                        //Default: ( selectedLocation == "All" and selectedStorage == "All")

                        if      ( selectedLocation == tr("All") and selectedStorage != tr("All"))
                        {
                                queryText = queryText + " WHERE c.catalogStorage ='" + selectedStorage + "'";
                        }
                        else if ( selectedLocation != tr("All") and selectedStorage == tr("All"))
                        {
                                queryText = queryText + " WHERE storageLocation ='" + selectedLocation + "'";
                        }
                        else if ( selectedLocation != tr("All") and selectedStorage != tr("All"))
                        {
                                queryText = queryText + " WHERE c.catalogStorage ='" + selectedStorage + "' "
                                                        " AND storageLocation ='" + selectedLocation + "'";
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
                    fileListModel = new QStringListModel(this);
                    fileListModel->setStringList(displayCatalogList);
                    ui->Filters_comboBox_SelectCatalog->setModel(fileListModel);

                //Send list to the Statistics combobox (without All or Selected storage options)
                    fileListModel = new QStringListModel(this);
                    fileListModel->setStringList(catalogSelectedList);

                    //Get last value
                    QSettings settings(settingsFilePath, QSettings:: IniFormat);
                    QString lastValue = settings.value("Statistics/SelectedCatalog").toString();

                    //Generate list of values
                    ui->Statistics_comboBox_SelectCatalog->setModel(fileListModel);

                    //Restore last selection value or default
                    if (lastValue=="")
                        ui->Statistics_comboBox_SelectCatalog->setCurrentText(typeOfData[0]);
                    else
                        ui->Statistics_comboBox_SelectCatalog->setCurrentText(lastValue);

                //Restore last selection
                    ui->Filters_comboBox_SelectCatalog->setCurrentText(currentCatalog);

            }
