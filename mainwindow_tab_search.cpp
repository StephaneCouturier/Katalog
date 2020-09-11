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
// Version:     0.1
/////////////////////////////////////////////////////////////////////////////
*/

#include "mainwindow.h"
#include "ui_mainwindow.h"
#include "catalog.h"

#include <QTextStream>
#include <QDesktopServices>
#include <QSaveFile>
#include <QFileDialog>

#include <KMessageBox>
#include <KLocalizedString>

//TAB: SEARCH FILES ----------------------------------------------------------------------

    //UI methods
        //Set up
        void MainWindow::initiateSearchValues()
        {
            //Prepare list for the Catalog selection combobox


//                QStringList displaycatalogList = catalogList;

//                //Add the option All at the beginning
//                displaycatalogList.insert(0,"All");
//                //Send list to the combobox
//                fileListModel = new QStringListModel(this);
//                fileListModel->setStringList(displaycatalogList);
//                ui->CB_SelectCatalog->setModel(fileListModel);

            //Load last search values (from settings file)
                ui->CB_SelectCatalog->setCurrentText(selectedSearchCatalog);
                ui->CB_S_TextCriteria->setCurrentText(selectedTextCriteria);
                ui->CB_S_SearchIn->setCurrentText(selectedSearchIn);
                ui->CB_S_FileType->setCurrentText(selectedFileType);
        }
        void MainWindow::refreshCatalogSelectionList()
        {
            //Prepare list for the Catalog selection combobox
                QStringList displaycatalogList = catalogFileList;

                //Add the option All at the beginning
                displaycatalogList.insert(0,"All");
                //Send list to the combobox
                fileListModel = new QStringListModel(this);
                fileListModel->setStringList(displaycatalogList);
                ui->CB_SelectCatalog->setModel(fileListModel);

        }

        //User interactions
        void MainWindow::on_PB_S_ResetAll_clicked()
        {
            ui->KCB_SearchText->setCurrentText("");
            ui->CB_SelectCatalog->setCurrentText("All");
            ui->CB_S_TextCriteria->setCurrentText("All Words");
            ui->CB_S_SearchIn->setCurrentText("File or Folder names");
            ui->CB_S_FileType->setCurrentText("Any");

        }
        void MainWindow::on_PB_Search_clicked()
        {
            SearchFiles();
        }
        void MainWindow::on_KCB_SearchText_returnPressed()
        {
            SearchFiles();
        }

        void MainWindow::on_TrV_FilesFound_clicked(const QModelIndex &index)
        {
            //Get file from selected row
            QString selectedFileName = ui->TrV_FilesFound->model()->index(index.row(), 0, QModelIndex()).data().toString();
            QString selectedFileFolder = ui->TrV_FilesFound->model()->index(index.row(), 3, QModelIndex()).data().toString();
            QString selectedFile = selectedFileFolder+"/"+selectedFileName;
            //Open the file (fromLocalFile needed for spaces in file name)
            QDesktopServices::openUrl(QUrl::fromLocalFile(selectedFile));
            //KMessageBox::information(this,"test:\n did nothing."+selectedFile);
        }

        void MainWindow::on_PB_ExportResults_clicked()
        {
            //Prepare export file name
            QDateTime now = QDateTime::currentDateTime();
            QString timestamp = now.toString(QLatin1String("yyyyMMdd-hhmmss"));
            QString filename = QString::fromLatin1("/search_results_%1.txt").arg(timestamp);
            filename=collectionFolder+filename;
            QFile exportFile(filename);

            //QFile exportFile(collectionFolder+"/file.txt");

              if (exportFile.open(QFile::WriteOnly | QFile::Text)) {

                  QTextStream stream(&exportFile);
                  for (int i = 0; i < filesFoundList.size(); ++i)
                    stream << filesFoundList.at(i) << '\n';
                    //} else {
                  //std::cerr << "error opening output file\n";
                  //return EXIT_FAILURE;
                }
              //else {
                //std::cerr << "error opening output file\n";
                //return EXIT_FAILURE;
              //}
              KMessageBox::information(this,"Results exported to the collection folder:\n"+exportFile.fileName());
              exportFile.close();
        }

        void MainWindow::on_TR_CatalogFoundList_clicked(const QModelIndex &index)
        {
            //Get file from selected row
            QString selectedCatalogName = ui->TR_CatalogFoundList->model()->index(index.row(), 0, QModelIndex()).data().toString();
            selectedCatalogName = collectionFolder + "/" + selectedCatalogName;
            ui->CB_SelectCatalog->setCurrentText(selectedCatalogName);
            SearchFiles();
            //->model()->index(index.row(), 3, QModelIndex()).data().toString();
            //QString selectedFile = selectedFileFolder+"/"+selectedFileName;
            //Open the file (fromLocalFile needed for spaces in file name)
            //QDesktopServices::openUrl(QUrl::fromLocalFile(selectedFile));
            //KMessageBox::information(this,"test:\n did nothing."+selectedFile);
            //KMessageBox::information(this,"Results exported to the collection folder:\n"+selectedCatalogName);

        }

        //File Context Menu actions set up
        void MainWindow::on_TrV_FilesFound_customContextMenuRequested(const QPoint &pos)
        {
            //KMessageBox::information(this,"test.");

            // for most widgets
            QPoint globalPos = ui->TrV_FilesFound->mapToGlobal(pos);
            // for QAbstractScrollArea and derived classes you would use:
            //QPoint globalPos = myWidget->viewport()->mapToGlobal(pos);
            QMenu fileContextMenu;

            QAction *menuAction1 = new QAction(QIcon::fromTheme("document-open-data"),(tr("Open file")), this);
            connect(menuAction1, &QAction::triggered, this, &MainWindow::contextOpenFile);
            fileContextMenu.addAction(menuAction1);

            QAction *menuAction2 = new QAction(QIcon::fromTheme("document-open-data"),(tr("Open folder")), this);
            connect(menuAction2, &QAction::triggered, this, &MainWindow::contextOpenFolder);
            fileContextMenu.addAction(menuAction2);

            fileContextMenu.addSeparator();

            QAction *menuAction3 = new QAction(QIcon::fromTheme("edit-copy"),(tr("Copy folder path")), this);
            connect( menuAction3,&QAction::triggered, this, &MainWindow::contextCopyFolderPath);
            fileContextMenu.addAction(menuAction3);

            QAction *menuAction4 = new QAction(QIcon::fromTheme("edit-copy"),(tr("Copy absolute path")), this);
            connect( menuAction4,&QAction::triggered, this, &MainWindow::contextCopyAbsolutePath);
            fileContextMenu.addAction(menuAction4);

            QAction *menuAction5 = new QAction(QIcon::fromTheme("edit-copy"),(tr("Copy file name with extension")), this);
            connect( menuAction5,&QAction::triggered, this, &MainWindow::contextCopyFileNameWithExtension);
            fileContextMenu.addAction(menuAction5);

            QAction *menuAction6 = new QAction(QIcon::fromTheme("edit-copy"),(tr("Copy file name without extension")), this);
            connect( menuAction6,&QAction::triggered, this, &MainWindow::contextCopyFileNameWithoutExtension);
            fileContextMenu.addAction(menuAction6);

            //fileContextMenu.addSeparator();

            // TEST:  copy file to..., cut file, move file to..., trash, delete, the full Dolphin menu!
            //QAction *menuAction30 = new QAction(QIcon::fromTheme("edit-copy"),(tr("TEST")), this);
            //fileContextMenu.addAction(menuAction30);

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

        //Context Menu methods
        void MainWindow::contextOpenFile()
        {
            QModelIndex index=ui->TrV_FilesFound->currentIndex();          
            //Get filepath from selected row
            QString selectedFileName = ui->TrV_FilesFound->model()->index(index.row(), 0, QModelIndex()).data().toString();
            QString selectedFileFolder = ui->TrV_FilesFound->model()->index(index.row(), 3, QModelIndex()).data().toString();
            QString selectedFile = selectedFileFolder+"/"+selectedFileName;
            //Open the file (fromLocalFile needed for spaces in file name)
            QDesktopServices::openUrl(QUrl::fromLocalFile(selectedFile));
        }
        void MainWindow::contextOpenFolder()
        {
            QModelIndex index=ui->TrV_FilesFound->currentIndex();
            QString selectedFileName = ui->TrV_FilesFound->model()->index(index.row(), 0, QModelIndex()).data().toString();
            QString selectedFileFolder = ui->TrV_FilesFound->model()->index(index.row(), 3, QModelIndex()).data().toString();
            QString selectedFile = selectedFileFolder+"/"+selectedFileName;
            QString folderName = selectedFile.left(selectedFile.lastIndexOf("/"));
            QDesktopServices::openUrl(QUrl::fromLocalFile(folderName));
        }
        void MainWindow::contextCopyAbsolutePath()
        {
            QModelIndex index=ui->TrV_FilesFound->currentIndex();

            QString selectedFileName = ui->TrV_FilesFound->model()->index(index.row(), 0, QModelIndex()).data().toString();
            QString selectedFileFolder = ui->TrV_FilesFound->model()->index(index.row(), 3, QModelIndex()).data().toString();
            QString selectedFile = selectedFileFolder+"/"+selectedFileName;

            QClipboard *clipboard = QGuiApplication::clipboard();
            QString originalText = clipboard->text();
            clipboard->setText(selectedFile);
        }
        void MainWindow::contextCopyFolderPath()
        {
            QModelIndex index=ui->TrV_FilesFound->currentIndex();
            QString selectedFileFolder = ui->TrV_FilesFound->model()->index(index.row(), 3, QModelIndex()).data().toString();

            QClipboard *clipboard = QGuiApplication::clipboard();
            QString originalText = clipboard->text();
            clipboard->setText(selectedFileFolder);
        }
        void MainWindow::contextCopyFileNameWithExtension()
        {
            QModelIndex index=ui->TrV_FilesFound->currentIndex();
            QString selectedFileName = ui->TrV_FilesFound->model()->index(index.row(), 0, QModelIndex()).data().toString();

            QString fileNameWithExtension = selectedFileName;

            QClipboard *clipboard = QGuiApplication::clipboard();
            QString originalText = clipboard->text();
            clipboard->setText(fileNameWithExtension);
        }
        void MainWindow::contextCopyFileNameWithoutExtension()
        {
            QModelIndex index=ui->TrV_FilesFound->currentIndex();

            QString selectedFileName = ui->TrV_FilesFound->model()->index(index.row(), 0, QModelIndex()).data().toString();
            QString selectedFileFolder = ui->TrV_FilesFound->model()->index(index.row(), 3, QModelIndex()).data().toString();
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
        void MainWindow::SearchFiles()
        {
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
                    searchText = ui->KCB_SearchText->currentText();

                    //Do nothing if there is no search text
                    //DEV: message box: No text entered, do you want to list all files based on the other criteria?
                    if (searchText=="")
                        return;

                    //add searchText to a list, to retrieved it later
                    //searchTextList.insert(0,searchText);
                    ui->KCB_SearchText->addItem(searchText);

                    //Get other search criteria
                    selectedSearchCatalog = ui->CB_SelectCatalog->currentText();
                    selectedTextCriteria = ui->CB_S_TextCriteria->currentText();
                    selectedSearchIn = ui->CB_S_SearchIn->currentText();
                    selectedFileType = ui->CB_S_FileType->currentText();

                 // Searching "Begin With" for File name or Folder name is not supported yet
                    if (selectedTextCriteria=="Begins With" and selectedSearchIn =="File names or Folder paths"){
                        KMessageBox::information(this,"Using 'Begin With' with 'File names or Folder names' is not supported yet.\nPlease try a different combinaison.");
                        return;;
                    }

            //Execute the search
                //Start animation while searching
                //QApplication::setOverrideCursor(Qt::WaitCursor);

                //Search every catalog if "All" is selected
                if ( selectedSearchCatalog =="All"){
                    foreach(sourceCatalog,catalogFileList)
                            {
                                SearchFilesInCatalog(sourceCatalog);
                            }
                    }
                //Otherwise just search files in the selected catalog
                else{
                    SearchFilesInCatalog(selectedSearchCatalog);
                    }

            //Process search results: list of catalogs
                //Remove duplicates so the catalogs are listed only once
                catalogFoundList.removeDuplicates();

                //Keep the catalog file name only
                foreach(QString item, catalogFoundList){
                        int index = catalogFoundList.indexOf(item);
                        QDir dir(item);
                        catalogFoundList[index] = dir.dirName();
                        }

                //Load list of catalogs in which files where found
                catalogFoundListModel = new QStringListModel(this);
                catalogFoundListModel->setStringList(catalogFoundList);
                ui->TR_CatalogFoundList->setModel(catalogFoundListModel);


            //Process search results: list of files
                // Create model
                Catalog *searchResultsCatalog = new Catalog(this);

                // Populate model with data
                searchResultsCatalog->populateFileData(sFileNames, sFileSizes, sFilePaths, sFileDateTimes);
                QSortFilterProxyModel *proxyModel = new QSortFilterProxyModel(this);
                proxyModel->setSourceModel(searchResultsCatalog);

                // Connect model to tree/table view
                ui->TrV_FilesFound->setModel(proxyModel);
                ui->TrV_FilesFound->QTreeView::sortByColumn(0,Qt::AscendingOrder);
                //ui->TrV_FilesFound->header()->setSectionResizeMode(QHeaderView::ResizeToContents);
                ui->TrV_FilesFound->header()->setSectionResizeMode(QHeaderView::Interactive);
                ui->TrV_FilesFound->header()->resizeSection(0, 600); //Name
                ui->TrV_FilesFound->header()->resizeSection(1, 110); //Size
                ui->TrV_FilesFound->header()->resizeSection(2, 140); //Date
                ui->TrV_FilesFound->header()->resizeSection(3, 400); //Path

                //Count and display the number of files found
                int numberFilesResult = searchResultsCatalog->rowCount();
                ui->L_NumberFilesResult->setNum(numberFilesResult);

                //Save the search parameters to the seetings file
                saveSettings();

            //Stop cursor animation
            //QApplication::restoreOverrideCursor();
        }
        //----------------------------------------------------------------------
        void MainWindow::SearchFilesInCatalog(const QString &sourceCatalog)
        {
            QFile catalogFile(sourceCatalog);

            //Define how to use the search text
                if(selectedTextCriteria == "Exact Phrase")
                    regexSearchtext=searchText; //just search for the extact text entered including spaces, as one text string.
                else if(selectedTextCriteria == "Begins With")
                    regexSearchtext="(^"+searchText+")";
                else if(selectedTextCriteria == "Any Word")
                    regexSearchtext=searchText.replace(" ","|");
                else if(selectedTextCriteria == "All Words"){
                    //regexSearchtext="(.*"+searchText.replace(" ","*)(.*")+"*)";

                    QString searchTextToSplit = searchText;
                    QString groupRegEx = "";
                    QRegExp lineSplitExp(" ");
                    QStringList lineFieldList = searchTextToSplit.split(lineSplitExp);
                    int numberOfSearchWords = lineFieldList.count();

                    // ^(?=.*\bjack\b)(?=.*\bjames\b).*$

                    //build regex group for one word
                    for (int i=0; i<(numberOfSearchWords); i++){
                        groupRegEx = groupRegEx + "(?=.*" + lineFieldList[i] + ")";

                    //regexSearchtext="(.*"+searchText.replace(" ","*)(.*")+"*)";
                    //(was|created|and)+.*(was|created|and)+.*(was|created|and)+.*
                    }
                    //groupRegEx = groupRegEx + lineFieldList[numberOfSearchWords-1] + ")";

                    //repeat group for each word
                    //QString fullRegex;
//                    for (int i=0; i<(numberOfSearchWords); i++){
//                        fullRegex = fullRegex + groupRegEx + "+.*";
//                    }

                    regexSearchtext = groupRegEx;// + ".*";
                    //regexSearchtext = "^(?=.*je)(?=.*war)(?=.*star).*$";

                    //regexSearchtext = "(?=.*je)(?=.*war)(?=.*star).*";
                }
                else {
                    regexSearchtext="";
                     }

                regexPattern = regexSearchtext;

            //Prepare the regexFileType for file types
            if(selectedFileType !="Any"){
                //Get the list of file extension and join it into one string
                if(selectedFileType =="Audio"){
                            regexFileType = fileType_Audio.join("|");
                }
                if(selectedFileType =="Image"){
                            regexFileType = fileType_Image.join("|");
                }
                if(selectedFileType =="Text"){
                            regexFileType = fileType_Text.join("|");
                }
                if(selectedFileType =="Video"){
                            regexFileType = fileType_Video.join("|");
                }

                //Replace the *. by .* needed for regex
                regexFileType = regexFileType.replace("*.",".*");
                //Add the file type expression to the regex
                regexPattern = regexSearchtext  + "(" + regexFileType + ")";

             }

            //KMessageBox::information(this,"regexPattern:\n"+regexPattern);
            //return;

            ui->L_Regex->setText(regexPattern);
            QRegularExpression regex(regexPattern, QRegularExpression::CaseInsensitiveOption);
            //QRegularExpression regexFType(regexFileType, QRegularExpression::CaseInsensitiveOption);

            //Search loop for all lines in the catalog file
            if (catalogFile.open(QIODevice::ReadOnly|QIODevice::Text)) {

                //Set up a stream from the file's data
                QTextStream stream(&catalogFile);
                QString line;
                QString reducedLine;

                //Line by Line, test if the file is matching all search criteria
                // It tests first the "search in" file/path criteria, then adapts for searchtext and filetype criteria
                do {
                    //Get line text
                    line = stream.readLine();
                    QRegularExpressionMatch match;
                    //QRegularExpressionMatch matchFileType;
                    QRegularExpressionMatch foldermatch;

                    //Split the line text with @@ into a list
                    QRegExp     lineSplitExp("@@");
                    QStringList lineFieldList   = line.split(lineSplitExp);
                    int         fieldListCount  = lineFieldList.count();

                    // Get the file path from the list:
                    QString     lineFilePath    = lineFieldList[0];

                    //Start by excluding lines starting with < (catalog infos)
                    if (lineFilePath.left(1)!="<"){

                        //reduce it depending on the "Search in" criteria
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
                            //else return;
                            //fileTypeJoin
                        }
                        else {
                            match = regex.match(lineFilePath);
                        }

                        //QRegularExpressionMatch match = regex.match(line);

                        //If the file is matching the criteria, add it and its catalog to the search results
                        if (match.hasMatch()){
                            filesFoundList << lineFilePath;
                            catalogFoundList.insert(0,sourceCatalog);

                            //Retrieve other file info
                            QFileInfo file(lineFilePath);

                            qint64 lineFileSize;
                            if (fieldListCount == 3){
                                    lineFileSize = lineFieldList[1].toLongLong();}
                            else lineFileSize = 0;

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
                    }
                } while(!line.isNull());
            }
            else return;

            }
        //----------------------------------------------------------------------
        //DEV: to be replaces using the collection model as source
        void MainWindow::LoadCatalogFileList()
        {
            catalogFileList.clear();
            QStringList fileTypes;
            fileTypes << "*.idx";
            //Iterate in the directory to create a list of files and sort it
            //list the file names only
            QDirIterator iterator(collectionFolder, fileTypes, QDir::Files, QDirIterator::Subdirectories);
            while (iterator.hasNext()){
                //catalogList << (iterator.next());

                QFile file(iterator.next());
                //file.open(QIODevice::ReadOnly);
                catalogFileList << file.fileName();

            }
            catalogFileList.sort();

            //Define and populate a model and send it to the listView
            //fileListModel = new QStringListModel(this);
            //fileListModel->setStringList(catalogFileList);
            //ui->TrV_CatalogList->setModel(fileListModel);

        }
        //----------------------------------------------------------------------
//------------------------------------------------------------------------------------------
