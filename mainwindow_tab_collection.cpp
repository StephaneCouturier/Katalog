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

#include <QTextStream>
#include <QDesktopServices>
#include <QFileDialog>

#include <KMessageBox>
#include <KLocalizedString>

//TAB: Collection ----------------------------------------------------------------------

    // UI set up

        //Load the list of catalog files
        void MainWindow::LoadCatalogList()
        {
            catalogList.clear();
            QStringList fileTypes;
            fileTypes << "*.idx";
            //Iterate in the directory to create a list of files and sort it
            //list the file names only
            QDirIterator iterator(collectionFolder, fileTypes, QDir::Files, QDirIterator::Subdirectories);
            while (iterator.hasNext()){
                //catalogList << (iterator.next());

                QFile file(iterator.next());
                //file.open(QIODevice::ReadOnly);
                catalogList << file.fileName();

            }
            catalogList.sort();

            //Define and populate a model and send it to the listView
            fileListModel = new QStringListModel(this);
            fileListModel->setStringList(catalogList);
            ui->TV_CatalogList->setModel(fileListModel);

        }

        //Catalog buttons methods
        void MainWindow::on_PB_UpdateCatalog_clicked()
        {
            //Get catalog name
            QFileInfo file(selectedExploreCatalog);
            QString catalogName = file.baseName();
            //Generate back the catalog folder path from the catalog file.
            //WARNING: this will only work if there was no _ in the original path.
            QString catalogPath = selectedExploreCatalog.right(selectedExploreCatalog.size() - selectedExploreCatalog.lastIndexOf("/") - 1);
            catalogPath = catalogPath.replace("__","/");
            catalogPath = catalogPath.replace(".idx","");


            QDir dir (catalogPath);
            if (dir.exists()==true){
                //KMessageBox::information(this,"path exists:\n"+catalogPath+"name:\n"+catalogName);

                CatalogDirectory(catalogPath);
                SaveCatalog(catalogName);
                KMessageBox::information(this,"This catalog was updated.");
            }
            else {
                KMessageBox::information(this,"This catalog cannot be updated. The source folder  "+catalogPath+"  does not seem to exist anymore, was renamed, or contained originally at least one _ (not supported yet).");

            }

            //Refresh catalog model
            LoadCatalogsToModel();

        }
        void MainWindow::on_PB_ViewCatalog_clicked()
        {
            //Start mouse cursor animation
            //QApplication::setOverrideCursor(Qt::WaitCursor);
            //QStringListModel* listModel= qobject_cast<QStringListModel*>(ui->TV_CatalogList->model());
            //selectedExploreCatalog = listModel->stringList().at(index.row());
            //KMessageBox::information(this,"test:\n"+selectedExploreCatalog);

            LoadCatalog(selectedExploreCatalog);
            //End mouse cursor animation
            //QApplication::restoreOverrideCursor();
        }
        void MainWindow::on_PB_ExportCatalog_clicked()
        {
            QString link = "https://sourceforge.net/p/katalogg/tickets/";
            KMessageBox::information(this,"There is no export function yet.\n Please tell what you expect from it by opening a ticket on on:\n"+link);
            QDesktopServices::openUrl(QUrl("https://sourceforge.net/p/katalogg/tickets/"));
        }
        void MainWindow::on_PB_DeleteCatalog_clicked()
        {
            if ( selectedExploreCatalog != ""){

                int result = KMessageBox::warningContinueCancel(this,
                          i18n("Do you want to delete this catalog?\n")+selectedExploreCatalog);

                if ( result ==KMessageBox::Continue){
                    QFile file (selectedExploreCatalog);
                    file.moveToTrash();
                    LoadCatalogList();
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
                LoadCatalogList();
                PopulateCatalogSelector();
                saveSettings();
            }
        }
        //----------------------------------------------------------------------
    //Load a catalog to view the files
    void MainWindow::LoadCatalog(QString fileName)
    {
        //DEV rename LoadCatalogFiles
        // Start animation while cataloging
        QApplication::setOverrideCursor(Qt::WaitCursor);


        // Create model
        QStringListModel *catalogModel;
        catalogModel = new QStringListModel(this);

        // open the file
        //DEV: replace by kmb
        QFile catalogFile;
        catalogFile.setFileName(fileName);
        if(!catalogFile.open(QIODevice::ReadOnly)) {
            KMessageBox::information(this,"Please select a catalog above first.");
        }

        // stream to read from file
        QStringList stringList;
        QTextStream textStream(&catalogFile);
        while (true)
        {
            QString line = textStream.readLine();
            if (line.isNull())
                break;
            else
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
    //Buttons
    void MainWindow::on_TV_CatalogList_activated(const QModelIndex &index)
    {
        QStringListModel* listModel= qobject_cast<QStringListModel*>(ui->TV_CatalogList->model());
        selectedExploreCatalog = listModel->stringList().at(index.row());
    }
    //----------------------------------------------------------------------
    //Open file when clicking on the listview
    //DEV replace by function and param
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
