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

#include <QTextStream>
#include <QDesktopServices>
#include <QSaveFile>
#include <QFileDialog>

#include <iostream>

#include <KMessageBox>
#include <KLocalizedString>

#include <QDebug>

//TAB: SEARCH FILES ----------------------------------------------------------------------

    //UI methods
        //Set up
        void MainWindow::initiateSearchValues()
        {
            //Prepare list for the Catalog selection combobox
                QStringList displaycatalogList = catalogList;

                //Add the option All at the beginning
                displaycatalogList.insert(0,"All");
                //Send list to the combobox
                fileListModel = new QStringListModel(this);
                fileListModel->setStringList(displaycatalogList);
                ui->CB_SelectCatalog->setModel(fileListModel);

            //Load last search values (from settings file)
                ui->CB_SelectCatalog->setCurrentText(selectedSearchCatalog);
                ui->CB_S_TextCriteria->setCurrentText(selectedTextCriteria);
                ui->CB_S_SearchIn->setCurrentText(selectedSearchIn);
                ui->CB_S_FileType->setCurrentText(selectedFileType);
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
        void MainWindow::on_LV_FilesFoundList_clicked(const QModelIndex &index)
        {
            //Get file full path
            QString fileName;
            QStringListModel* listModel= qobject_cast<QStringListModel*>(ui->LV_FilesFoundList->model());
            fileName = listModel->stringList().at(index.row());
            //Open the file (fromLocalFile needed for spaces in file name)
            QDesktopServices::openUrl(QUrl::fromLocalFile(fileName));
        }
        void MainWindow::on_PB_ExportResults_clicked()
        {
            //Get list of search results
            QStringListModel *catalogModel = (QStringListModel*)ui->LV_FilesFoundList->model();
            QStringList filelist = catalogModel->stringList();

            //Get current selected path as default path for the dialog window

            //Open a dialog for the user to select the directory to be cataloged. Only show directories.

            QDateTime now = QDateTime::currentDateTime();
            QString timestamp = now.toString(QLatin1String("yyyyMMdd-hhmmss"));
            QString filename = QString::fromLatin1("/search_results_%1.txt").arg(timestamp);
            filename=collectionFolder+filename;
            QFile exportFile(filename);

            //QFile exportFile(collectionFolder+"/file.txt");

              if (exportFile.open(QFile::WriteOnly | QFile::Text)) {
                  QTextStream stream(&exportFile);
                  for (int i = 0; i < filelist.size(); ++i)
                    stream << filelist.at(i) << '\n';
                    } else {
                  std::cerr << "error opening output file\n";
                  //return EXIT_FAILURE;
                }
              //else {
                //std::cerr << "error opening output file\n";
                //return EXIT_FAILURE;
              //}
              KMessageBox::information(this,"Results exported to the collection folder:\n"+exportFile.fileName());
              exportFile.close();
        }

        //File Context Menu actions set up
        void MainWindow::on_LV_FilesFoundList_customContextMenuRequested(const QPoint &pos)
        {
            // for most widgets
            QPoint globalPos = ui->LV_FilesFoundList->mapToGlobal(pos);
            // for QAbstractScrollArea and derived classes you would use:
            // QPoint globalPos = myWidget->viewport()->mapToGlobal(pos);
            QMenu fileContextMenu;

            QAction *menuAction1 = new QAction(QIcon::fromTheme("document-open-data"),(tr("Open file")), this);
            connect(menuAction1, &QAction::triggered, this, &MainWindow::contextOpenFile);
            fileContextMenu.addAction(menuAction1);

            QAction *menuAction2 = new QAction(QIcon::fromTheme("document-open-data"),(tr("Open folder")), this);
            connect(menuAction2, &QAction::triggered, this, &MainWindow::contextOpenFolder);
            fileContextMenu.addAction(menuAction2);

            fileContextMenu.addSeparator();

            QAction *menuAction3 = new QAction(QIcon::fromTheme("edit-copy"),(tr("Copy absolute path")), this);
            connect( menuAction3,&QAction::triggered, this, &MainWindow::contextCopyAbsolutePath);
            fileContextMenu.addAction(menuAction3);

            QAction *menuAction4 = new QAction(QIcon::fromTheme("edit-copy"),(tr("Copy file name with extension")), this);
            connect( menuAction4,&QAction::triggered, this, &MainWindow::contextCopyFileNameWithExtension);
            fileContextMenu.addAction(menuAction4);

            QAction *menuAction5 = new QAction(QIcon::fromTheme("edit-copy"),(tr("Copy file name without extension")), this);
            connect( menuAction5,&QAction::triggered, this, &MainWindow::contextCopyFileNameWithoutExtension);
            fileContextMenu.addAction(menuAction5);

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
            QModelIndex index=ui->LV_FilesFoundList->currentIndex();
            QString fileName;
            QStringListModel* listModel= qobject_cast<QStringListModel*>(ui->LV_FilesFoundList->model());
            fileName = listModel->stringList().at(index.row());
            QDesktopServices::openUrl(QUrl::fromLocalFile(fileName));
        }
        void MainWindow::contextOpenFolder()
        {
            QModelIndex index=ui->LV_FilesFoundList->currentIndex();
            QString fileName;
            QStringListModel* listModel= qobject_cast<QStringListModel*>(ui->LV_FilesFoundList->model());
            fileName = listModel->stringList().at(index.row());
            QString folderName = fileName.left(fileName.lastIndexOf("/"));
            QDesktopServices::openUrl(QUrl::fromLocalFile(folderName));
        }
        void MainWindow::contextCopyAbsolutePath()
        {
            QModelIndex index=ui->LV_FilesFoundList->currentIndex();
            QString fileName;
            QStringListModel* listModel= qobject_cast<QStringListModel*>(ui->LV_FilesFoundList->model());
            fileName = listModel->stringList().at(index.row());
            QClipboard *clipboard = QGuiApplication::clipboard();
            QString originalText = clipboard->text();
            clipboard->setText(fileName);
        }
        void MainWindow::contextCopyFileNameWithExtension()
        {
            QModelIndex index=ui->LV_FilesFoundList->currentIndex();
            QString fileName;
            QStringListModel* listModel= qobject_cast<QStringListModel*>(ui->LV_FilesFoundList->model());
            fileName = listModel->stringList().at(index.row());
            QString fileNameWithExtension = fileName.right(fileName.size() - fileName.lastIndexOf("/") - 1);

            QClipboard *clipboard = QGuiApplication::clipboard();
            QString originalText = clipboard->text();
            clipboard->setText(fileNameWithExtension);
        }
        void MainWindow::contextCopyFileNameWithoutExtension()
        {
            QModelIndex index=ui->LV_FilesFoundList->currentIndex();
            QFileInfo fileName;
            QStringListModel* listModel= qobject_cast<QStringListModel*>(ui->LV_FilesFoundList->model());
            fileName.setFile(listModel->stringList().at(index.row()));
            QString fileNameWithoutExtension = fileName.baseName();
            //.base.right(fileName.size() - fileName.lastIndexOf("/") - 1);
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

                //Get search criteria
                    //Get text to search in file names or directories
                    searchText = ui->KCB_SearchText->currentText();

                    //Do nothing if there is no search text
                    //DEV: message box: No text entered, do you want to list all files based on the other criteria?
                    if (searchText=="")
                        return;

                    //add searchTest to a list, to retrieved it later
                    //searchTextList.insert(0,searchText);
                    ui->KCB_SearchText->addItem(searchText);

                    //Get other search criteria
                    selectedSearchCatalog = ui->CB_SelectCatalog->currentText();
                    selectedTextCriteria = ui->CB_S_TextCriteria->currentText();
                    selectedSearchIn = ui->CB_S_SearchIn->currentText();
                    selectedFileType = ui->CB_S_FileType->currentText();

                 // Searching "Begin With" for File name or Fodler name is not supported yet
                    if (selectedTextCriteria=="Begins With" and selectedSearchIn =="File names or Folder paths"){
                        KMessageBox::information(this,"Searching -Begin With- with -File names or Fodler names- is not supported yet.\nPlease try a different combinaison.");
                        return;;
                    }

            //Execute the search
                //Start animation while searching
                //QApplication::setOverrideCursor(Qt::WaitCursor);

                //Search every catalog if "All" is selected
                if ( selectedSearchCatalog =="All"){
                    foreach(sourceCatalog,catalogList)
                            {
                                SearchFilesInCatalog(sourceCatalog);
                            }
                    }
                //Otherwise just search the selected catalog
                else{
                    //Search files
                    SearchFilesInCatalog(selectedSearchCatalog);
                    catalogFoundList.insert(0,selectedSearchCatalog);
                    }

            //Process search results
                //Remove duplicates so the catalogs are listed only once
                catalogFoundList.removeDuplicates();
                //Keep the catalog file name only
                foreach(QString item, catalogFoundList){
                        int index = catalogFoundList.indexOf(item);
                        QDir dir(item);
                        catalogFoundList[index] = dir.dirName();
                        }

                //Return list of catalog in which files where found
                catalogFoundListModel = new QStringListModel(this);
                catalogFoundListModel->setStringList(catalogFoundList);
                ui->TR_CatalogFoundList->setModel(catalogFoundListModel);

                //Count and display the number of files found
                int numberFilesResult = fileListModel->rowCount();
                ui->L_NumberFilesResult->setNum(numberFilesResult);

                saveSettings();

            //Stop cursor animation
            //QApplication::restoreOverrideCursor();
        }
        //----------------------------------------------------------------------
        void MainWindow::SearchFilesInCatalog(const QString &sourceCatalog)
        {
            QFile catalogFile(sourceCatalog);
            //Generate RegularExpression = pattern to verify each file
            //DEV: test if file path has space
            //KMessageBox::information(this,"test:\n"+selectedSearchIn+"test:\n"+selectedTextCriteria+"test:\n"+selectedFileType);

            //Define how to use the search text
                if(selectedTextCriteria == "Exact Phrase")
                    regexSearchtext=searchText;
                else if(selectedTextCriteria == "Begins With")
                    regexSearchtext="(^"+searchText+")";
                else if(selectedTextCriteria == "Any Word")
                    regexSearchtext=searchText.replace(" ","|");
                else if(selectedTextCriteria == "All Words")
                    regexSearchtext="("+searchText.replace(" ",")(.*")+")";
                else {
                    regexSearchtext="";
                     }

                regexPattern = regexSearchtext;

            //Prepare the regexFileType
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
                regexPattern = regexSearchtext + "(" + regexFileType + ")";

             }

            //KMessageBox::information(this,"regexPattern:\n"+regexPattern);
            ui->L_Regex->setText(regexPattern);
            QRegularExpression regex(regexPattern, QRegularExpression::CaseInsensitiveOption);

            //Search loop for all RegEx

            //Open catalog file
            if (catalogFile.open(QIODevice::ReadOnly|QIODevice::Text)) {
                //Set up a stream from the file's data
                QTextStream stream(&catalogFile);
                QString line;
                QString reducedLine;
                //QRegularExpressionMatch match;


                //in file path file name criteria
                //selectedPathOnly = ui->RB_InFilePath->isChecked();
                //selectedNameOnly = ui->RB_InFileName->isChecked();
                //Line by Line, test if the file is matching all search criteria
                // This tests first the search in file/path criteria, then adapts for searchtext and tiletype criteria
                do {
                    //Get line text
                    line = stream.readLine();
                    QRegularExpressionMatch match;
                    QRegularExpressionMatch foldermatch;

                    //Start by excluding lines starting with < (catalog infos)
                    if (line.left(1)!="<"){

                        //reduce it depending on the "Search in" criteria
                        if(selectedSearchIn == "File names only")
                        {
                            //Keep only the filename, so all characters at the right of the last occurence of / in the path.
                            //to know the number of characters to take right:
                            //taking the total number of characters, substract the last index position, add 1 to avoid the /
                            reducedLine = line.right(line.size() - line.lastIndexOf("/") - 1);

                            //KMessageBox::information(this,"test:\n"+reducedLine);

                            //regexPattern = regexSearchtext + "(" + regexFileType + ")";
                            match = regex.match(reducedLine);
                        }
                        else if(selectedSearchIn == "Folder path only")
                        {
                            //Keep only the folder name, so all characters left of the last occurence of / in the path.
                            reducedLine = line.left(line.lastIndexOf("/"));

                            //Check the fodler name matches the search text
                            regex.setPattern(regexSearchtext);

                            foldermatch = regex.match(reducedLine);

                            //if it does, then check that the file matches the selected file type
                            if (foldermatch.hasMatch()){
                                regex.setPattern(regexFileType);

                                match = regex.match(line);
                            }
                            //else return;
                            //fileTypeJoin
                        }
                        else {
                            //regexPattern = regexSearchtext + "(" + regexFileType + ")";
                            //KMessageBox::information(this,"test:\n"+regexPattern);
                            match = regex.match(line);
                        }
                        //QRegularExpressionMatch match = regex.match(line);
                        //If the file is matching the criteria, add it and its catalog to the search results
                        if (match.hasMatch()){
                            filesFoundList << line;
                            catalogFoundList.insert(0,sourceCatalog);
                        }
                    }
                } while(!line.isNull());
            }
            else return;

            //populate result lists
            fileListModel = new QStringListModel(this);
            fileListModel->setStringList(filesFoundList);
            ui->LV_FilesFoundList->setModel(fileListModel);

            }
        //----------------------------------------------------------------------

//------------------------------------------------------------------------------------------
