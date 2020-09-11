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
#include "collection.h"
#include "catalog.h"

#include <QTextStream>
#include <QDesktopServices>
#include <QFileDialog>
#include <QSortFilterProxyModel>

#include <KMessageBox>
#include <KLocalizedString>

//TAB: Collection ----------------------------------------------------------------------

    //Collection selection
        void MainWindow::on_PB_SelectCollectionFolder_clicked()
        {
            //Open a dialog for the user to select the directory of the collection where catalog files are stored.
            QString dir = QFileDialog::getExistingDirectory(this, tr("Select the directory for this collection"),
                                                            collectionFolder,
                                                            QFileDialog::ShowDirsOnly
                                                            | QFileDialog::DontResolveSymlinks);
            //Unless the selection was cancelled, send the selected folder, and refresh the list of catalogs
            if ( dir !=""){
                collectionFolder=dir;
                ui->LE_CollectionFolder->setText(dir);

                //initiateSearchValues();
                saveSettings();
                LoadCatalogFileList();
                LoadCatalogsToModel();
                refreshCatalogSelectionList();
            }

            //Reset selected catalog values (to avoid updating the last selected one for instance)
            //DEV: repalce by button becoming enabled once catalog is selected
            selectedCatalogFile="";
            selectedCatalogName="";
            selectedCatalogPath="";
        }
        //----------------------------------------------------------------------

    //Catalog buttons
        void MainWindow::on_PB_C_OpenFolder_clicked()
        {
            //Open the selected collection folder
            QDesktopServices::openUrl(QUrl::fromLocalFile(collectionFolder));
        }
        //----------------------------------------------------------------------
        void MainWindow::on_TrV_CatalogList_activated(const QModelIndex &index)
        {
            selectedCatalogFile = ui->TrV_CatalogList->model()->index(index.row(), 4, QModelIndex()).data().toString();
            selectedCatalogName = ui->TrV_CatalogList->model()->index(index.row(), 0, QModelIndex()).data().toString();
            selectedCatalogPath = ui->TrV_CatalogList->model()->index(index.row(), 3, QModelIndex()).data().toString();
        }
        //----------------------------------------------------------------------
        void MainWindow::on_PB_UpdateCatalog_clicked()
        {
            //Update the Selected Catalog

            //Check if the updqte can be done, qnd inform the user otherwise
            if(selectedCatalogFile == "not recorded" or selectedCatalogName == "not recorded" or selectedCatalogPath == "not recorded"){
            KMessageBox::information(this,"It seems this catalog was imported or has an old format.\n"
                                         "Please Edit it and make sure it has the following first 2 lines:\n\n"
                                         "<catalogSourcePath>/folderpath\n"
                                         "<catalogFileCount>10000\n\n"
                                         "Copy/paste these lines at the begining of the file and modify the values after the >:\n"
                                         "- the catalogSourcePath is the folder to catalog the files from.\n"
                                         "- the catalogFileCount number does not matter as much, it can be updated.\n"
                                     );
            return;
            }
            if(selectedCatalogFile == "" or selectedCatalogName == "" or selectedCatalogPath == ""){
            KMessageBox::information(this,"Please select a catalog first (some info is missing).\nselectedCatalogFile:"
                                     +selectedCatalogFile+"\nselectedCatalogName: "
                                     +selectedCatalogName+"\nselectedCatalogPath: "
                                     +selectedCatalogPath);
            return;
            }

            newCatalogName = selectedCatalogName;

            QDir dir (selectedCatalogPath);
            if (dir.exists()==true){
                CatalogDirectory(selectedCatalogPath);
                SaveCatalog(selectedCatalogName);
                KMessageBox::information(this,"This catalog was updated.");
            }
            else {
                KMessageBox::information(this,"This catalog cannot be updated.\n"
                                                "The source folder - "+selectedCatalogPath+" - was not found.\n"
                                                "Possible reasons:\n"
                                                "- the device is not connected and mounted\n"
                                                "- the folder was moved or renamed"
                                         );

            }
            //Refresh the collection view
            LoadCatalogsToModel();
            //Load and display the updated catalog
            //LoadCatalog(selectedCatalogFile);

        }
        //----------------------------------------------------------------------
        void MainWindow::on_PB_ViewCatalog_clicked()
        {
            //View the files of the Selected Catalog
            LoadCatalog(selectedCatalogFile);
            LoadFilesToModel();
        }
        void MainWindow::on_TrV_CatalogList_doubleClicked(const QModelIndex &index)
        {
            //Get file from selected row
            selectedCatalogFile = ui->TrV_CatalogList->model()->index(index.row(), 4, QModelIndex()).data().toString();
            LoadCatalog(selectedCatalogFile);
            LoadFilesToModel();
        }
        //----------------------------------------------------------------------
        void MainWindow::on_PB_C_Rename_clicked()
        {

        }
        //----------------------------------------------------------------------
        void MainWindow::on_PB_EditCatalogFile_clicked()
        {
            QDesktopServices::openUrl(QUrl::fromLocalFile(selectedCatalogFile));
        }
        //----------------------------------------------------------------------
        void MainWindow::on_PB_ExportCatalog_clicked()
        {
            QString link = "https://sourceforge.net/p/katalogg/tickets/";
            KMessageBox::information(this,"There is no export function yet.\n Please tell what you expect from it by opening a ticket on on:\n"+link);
            QDesktopServices::openUrl(QUrl("https://sourceforge.net/p/katalogg/tickets/"));
        }
        //----------------------------------------------------------------------
        void MainWindow::on_PB_DeleteCatalog_clicked()
        {
            if ( selectedCatalogFile != ""){

                int result = KMessageBox::warningContinueCancel(this,
                          i18n("Do you want to delete this catalog?\n")+selectedCatalogFile);

                if ( result ==KMessageBox::Continue){
                    QFile file (selectedCatalogFile);
                    file.moveToTrash();
                    LoadCatalogFileList();
                    LoadCatalogsToModel();
                    refreshCatalogSelectionList();
                }
             }
            else KMessageBox::information(this,i18n("Please select a catalog above first."));
        }
        //----------------------------------------------------------------------
    //File methods
        void MainWindow::on_TrV_FileList_clicked(const QModelIndex &index)
        {
            //Get file from selected row
            QString selectedFileName = ui->TrV_FileList->model()->index(index.row(), 0, QModelIndex()).data().toString();
            QString selectedFileFolder = ui->TrV_FileList->model()->index(index.row(), 3, QModelIndex()).data().toString();
            QString selectedFile = selectedFileFolder+"/"+selectedFileName;
            //Open the file (fromLocalFile needed for spaces in file name)
            QDesktopServices::openUrl(QUrl::fromLocalFile(selectedFile));
            //KMessageBox::information(this,"test:\n did nothing."+selectedFile);

        }
        void MainWindow::on_TrV_FileList_customContextMenuRequested(const QPoint &pos)
        {
            // for most widgets
            QPoint globalPos = ui->TrV_FileList->mapToGlobal(pos);
            // for QAbstractScrollArea and derived classes, use:
            // QPoint globalPos = myWidget->viewport()->mapToGlobal(pos);
            QMenu fileContextMenu;

//            QAction *menuAction1 = new QAction(QIcon::fromTheme("document-open-data"),(tr("Open file")), this);
//            connect(menuAction1, &QAction::triggered, this, &MainWindow::contextOpenFile);
//            fileContextMenu.addAction(menuAction1);

//            QAction *menuAction2 = new QAction(QIcon::fromTheme("document-open-data"),(tr("Open folder")), this);
//            connect(menuAction2, &QAction::triggered, this, &MainWindow::contextOpenFolder);
//            fileContextMenu.addAction(menuAction2);

//            fileContextMenu.addSeparator();

            QAction *menuAction3 = new QAction(QIcon::fromTheme("edit-copy"),(tr("Copy absolute path")), this);
            connect( menuAction3,&QAction::triggered, this, &MainWindow::context2CopyAbsolutePath);
            fileContextMenu.addAction(menuAction3);

//            QAction *menuAction4 = new QAction(QIcon::fromTheme("edit-copy"),(tr("Copy file name with extension")), this);
//            connect( menuAction4,&QAction::triggered, this, &MainWindow::contextCopyFileNameWithExtension);
//            fileContextMenu.addAction(menuAction4);

//            QAction *menuAction5 = new QAction(QIcon::fromTheme("edit-copy"),(tr("Copy file name without extension")), this);
//            connect( menuAction5,&QAction::triggered, this, &MainWindow::contextCopyFileNameWithoutExtension);
//            fileContextMenu.addAction(menuAction5);

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
        void MainWindow::context2CopyAbsolutePath()
        {
            QModelIndex index=ui->TrV_FileList->currentIndex();
            QString selectedFileName = ui->TrV_FileList->model()->index(index.row(), 0, QModelIndex()).data().toString();
            QString selectedFileFolder = ui->TrV_FileList->model()->index(index.row(), 3, QModelIndex()).data().toString();
            QString selectedFileAbsolutePath = selectedFileFolder+"/"+selectedFileName;
            QClipboard *clipboard = QGuiApplication::clipboard();
            QString originalText = clipboard->text();
            clipboard->setText(selectedFileAbsolutePath);
        }
        //----------------------------------------------------------------------

    //Load a catalog to view the files
    void MainWindow::LoadCatalog(QString fileName)
    {
        // Create model
        QStringListModel *catalogModel;
        catalogModel = new QStringListModel(this);

        // open the file
        //DEV: replace by kmb
        QFile catalogFile;
        catalogFile.setFileName(fileName);
        if(!catalogFile.open(QIODevice::ReadOnly)) {
            KMessageBox::information(this,"Please select a catalog above first.");
            return;
        }
        // Start animation while oprning
        QApplication::setOverrideCursor(Qt::WaitCursor);

        // stream to read from file
        QStringList stringList;
        QTextStream textStream(&catalogFile);
        while (true)
        {
            QString line = textStream.readLine();
            if (line.isNull())
                break;
            else
                if (line.left(1)!="<")
                    stringList.append(line); // populate the stringlist
        }

        // Populate the model
        catalogModel->setStringList(stringList);

        // Send model to the listview and view together
        //ui->TrV_FileList->setModel(catalogModel);

        int catalogFilesNumber = catalogModel->rowCount();
        ui->L_FilesNumber->setNum(catalogFilesNumber);

        //DEV   Stop animation
        QApplication::restoreOverrideCursor();
    }
    //----------------------------------------------------------------------

    //Load a collection (catalogs)
    void MainWindow::LoadCatalogsToModel()
    {
        //Set up temporary lists
        QList<QString> cNames;
        QList<QString> cDateUpdates;
        QList<qint64> cNums;
        QList<QString> cSourcePaths;
        QList<QString> cCatalogFiles;

        //Iterate in the directory to create a list of files and sort it
        QStringList fileTypes;
        fileTypes << "*.idx";

        QDirIterator iterator(collectionFolder, fileTypes, QDir::Files, QDirIterator::Subdirectories);
        while (iterator.hasNext()){

            //LoadCatalogInfo(file);
            // Get infos stored in the file
            QFile catalogFile(iterator.next());
            if(!catalogFile.open(QIODevice::ReadOnly)) {
                KMessageBox::information(this,"No catalog found.");
                return;
            }

            QTextStream textStream(&catalogFile);
            //bool catalogNameProvided = false;
            bool catalogSourcePathProvided = false;
            bool catalogFileCountProvided = false;

            while (true)
            {
                QString line = textStream.readLine();
//          commented out as the catalog name is the catalog file name
//                if (line.left(13)=="<catalogName>"){
//                   // QString catalogName = line.right(line.size() - line.lastIndexOf(">") - 1);
//                   // cNames.append(catalogName);
//                    catalogNameProvided = true;
//                }
//                else
                if (line.left(19)=="<catalogSourcePath>"){
                    QString catalogSourcePath = line.right(line.size() - line.lastIndexOf(">") - 1);
                    cSourcePaths.append(catalogSourcePath);
                    catalogSourcePathProvided = true;
                }
                else if (line.left(18)=="<catalogFileCount>"){
                    QString catalogFileCountString = line.right(line.size() - line.lastIndexOf(">") - 1);
                    int catalogFileCount = catalogFileCountString.toInt();
                    cNums.append(catalogFileCount);
                    catalogFileCountProvided = true;
                }
                //else if (line.left(1)=="/")
                    //break;
                else
                    break;

            }

//            if(catalogNameProvided==false)
//                cNames.append("not recorded");
            if(catalogSourcePathProvided==false)
                cSourcePaths.append("not recorded");
            if(catalogFileCountProvided==false)
                cNums.append(0);

            // Get infos about the file itself
            QFileInfo catalogFileInfo(catalogFile);
            cDateUpdates.append(catalogFileInfo.lastModified().toString("yyyy-MM-dd hh:mm:ss"));
            cCatalogFiles.append(catalogFileInfo.filePath());
            //QFile file(catalogFile);
            cNames.append(catalogFileInfo.baseName());
        }

        // Create model
        Collection *collection = new Collection(this);

        // Populate model with data
        collection->populateData(cNames, cDateUpdates, cNums, cSourcePaths, cCatalogFiles);

        QSortFilterProxyModel *proxyModel = new QSortFilterProxyModel(this);
        proxyModel->setSourceModel(collection);

        // Connect model to tree/table view
        ui->TrV_CatalogList->setModel(proxyModel);
        ui->TrV_CatalogList->QTreeView::sortByColumn(0,Qt::AscendingOrder);
        ui->TrV_CatalogList->header()->setSectionResizeMode(QHeaderView::ResizeToContents);


    }
    //----------------------------------------------------------------------

    //Load a catalog (files)
    void MainWindow::LoadFilesToModel()
    {
        //Set up temporary lists
        QList<QString> cfileNames;
        QList<qint64> cfileSizes;
        QList<QString> cfilePaths;
        QList<QString> cfileDateTimes;

         //Iterate in the directory to create a list of files and sort it
        //QStringList fileTypes;
        //fileTypes << "*.idx";

        //QDirIterator iterator(collectionFolder, fileTypes, QDir::Files, QDirIterator::Subdirectories);
        //while (iterator.hasNext()){

            // Get infos stored in the file
            QFile catalogFile(selectedCatalogFile);
            if(!catalogFile.open(QIODevice::ReadOnly)) {
                KMessageBox::information(this,"No catalog found.");
                return;
            }

            QTextStream textStream(&catalogFile);
            //bool catalogNameProvided = false;

            while (true)
            {
                QString line = textStream.readLine();
                if (line.isNull())
                    break;
                else
                    if (line.left(1)!="<"){
                        //Reminder: the double @ separates the filepath, size, and datetime
                        //Split the string with @@ into a list
                        QRegExp tagExp("@@");
                        QStringList fieldList = line.split(tagExp);

                        int fieldListCount = fieldList.count();

                        // Get the filePath from the list:
                        QString filePath        = fieldList[0];

                        // Get the fileSize from the list if available
                        qint64 fileSize;
                        if (fieldListCount == 3){
                                fileSize = fieldList[1].toLongLong();}
                        else fileSize = 0;

                        // Get the fileDateTime from the list if available
                        QString fileDateTime;
                        if (fieldListCount == 3){
                                fileDateTime = fieldList[2];}
                        else fileDateTime = "";

                        //KMessageBox::information(this,"filedatetime\n"+filedatetime);

                        //Get file informations
                        QFileInfo file(filePath);

                        //Append data to the lists
                        cfileNames.append(file.fileName());
                        cfileSizes.append(fileSize);
                        cfilePaths.append(file.path());
                        cfileDateTimes.append(fileDateTime); //selectedCatalogName

                        //if (fieldListCount != 3)
                            //KMessageBox::information(this,"This catalog was made with a previous version of Katalog that did not record size and date. \n If you can, update this catalog to get this information");

                    }
            }

        // Create model
        Catalog *catalog1 = new Catalog(this);

        // Populate model with data
        catalog1->populateFileData(cfileNames, cfileSizes, cfilePaths, cfileDateTimes);

        QSortFilterProxyModel *proxyModel = new QSortFilterProxyModel(this);
        proxyModel->setSourceModel(catalog1);

        // Connect model to tree/table view
        ui->TrV_FileList->setModel(proxyModel);
        ui->TrV_FileList->QTreeView::sortByColumn(0,Qt::AscendingOrder);
        ui->TrV_FileList->header()->setSectionResizeMode(QHeaderView::ResizeToContents);

        //ui->TrV_FileList->setModel(proxyModel);
        //ui->LV_Files->QTreeView::sortByColumn(0,Qt::AscendingOrder);
        //ui->LV_Files->header()->setSectionResizeMode(QHeaderView::ResizeToContents);
    }
