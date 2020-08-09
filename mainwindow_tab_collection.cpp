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
            //Get current selected path as default path for the dialog window
            //collectionFolder = ui->PB_SelectCollectionFolder->text();

            //Open a dialog for the user to select the directory to be cataloged. Only show directories.
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
            //Check if the updqte cqn be done, qnd inform the user otherwise
            if(selectedCatalogFile == "not recorded" or selectedCatalogName == "not recorded" or selectedCatalogPath == "not recorded"){
            KMessageBox::information(this,"It seems this catalog was imported or has an old format.\n"
                                         "Please Edit it and make sure it has the following first 3 lines:\n\n"
                                         "<catalogName>catalogName\n"
                                         "<catalogSourcePath>/home/user/folder/folder\n"
                                         "<catalogFileCount>10000\n\n"
                                         "Copy/paste these lines and modify the value after the >:\n"
                                         "- the catalogName must be equal to the filename, without the .idx\n"
                                         "- the catalogSourcePath must correspond to the folder where the files should be cataloged\n"
                                         "- the catalogFileCount number does not matter, it will be updated.\n"
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
            LoadCatalog(selectedCatalogFile);

        }
        //----------------------------------------------------------------------
        void MainWindow::on_PB_ViewCatalog_clicked()
        {
            LoadCatalog(selectedCatalogFile);
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
        void MainWindow::on_LV_FileList_customContextMenuRequested(const QPoint &pos)
        {
            // for most widgets
            QPoint globalPos = ui->LV_FileList->mapToGlobal(pos);
            // for QAbstractScrollArea and derived classes you would use:
            // QPoint globalPos = myWidget->viewport()->mapToGlobal(pos);
            QMenu myMenu;
            //myMenu.addAction("Menu Item 1");
            //myMenu.addAction("Menu Item 2");
            // ...

            QAction* selectedItem = myMenu.exec(globalPos);
            if (selectedItem)
            {
                //KMessageBox::information(this,"test:\n Copy file name to clipboard, Copy path to clipboard, Open containing folder.");
            }
            else
            {
                //KMessageBox::information(this,"test:\n nothing.");

            }
        }
        //----------------------------------------------------------------------

    //File methods
        void MainWindow::on_LV_FileList_clicked(const QModelIndex &index)
        {
            //Get file full path
            QString fileName;
            QStringListModel* listModel= qobject_cast<QStringListModel*>(ui->LV_FileList->model());
            fileName = listModel->stringList().at(index.row());
            //Open the file (fromLocalFile needed for spaces in file name)
            QDesktopServices::openUrl(QUrl::fromLocalFile(fileName));
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
        ui->LV_FileList->setModel(catalogModel);

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
        QList<int> cNums;
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
            //KMessageBox::information(this,"iterator"+iterator.fileName());

            QTextStream textStream(&catalogFile);
            //bool catalogNameProvided = false;
            bool catalogSourcePathProvided = false;
            bool catalogFileCountProvided = false;

            while (true)
            {
                QString line = textStream.readLine();
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
                else if (line.left(1)=="/")
                    break;
                else
                    KMessageBox::information(this,"iterator"+iterator.fileName());

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

