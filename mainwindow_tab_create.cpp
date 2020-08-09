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
// File Name:   mainwindow_tab_create.cpp
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

#include <QFileDialog>
#include <QTextStream>

#include <iostream>

#include <KMessageBox>
#include <KLocalizedString>

//#include <KMessageBox>
//#include <KLocalizedString>

//TAB: Create Catalog ----------------------------------------------------------------------

    //Load file system for the treeview
    void MainWindow::LoadFileSystem(QString newCatalogPath)
    {
            newCatalogPath="/";
         // Creates a new model
            fileSystemModel = new QFileSystemModel(this);

         // Set filter to show only directories
            fileSystemModel->setFilter(QDir::NoDotAndDotDot | QDir::AllDirs);

         // QFileSystemModel requires root path
            QString rootPath ="/";
            fileSystemModel->setRootPath(rootPath);
            fileSystemModel->setRootPath(newCatalogPath);
         // Enable/Disable modifying file system
            //qfilesystemmodel->setReadOnly(true);

        // Attach the model to the view
            ui->TV_Explorer->setModel(fileSystemModel);

        // Only show the tree, hidding other columns and the header row.
            ui->TV_Explorer->setColumnWidth(0,250);
            ui->TV_Explorer->setColumnHidden(1,true);
            ui->TV_Explorer->setColumnHidden(2,true);
            ui->TV_Explorer->setColumnHidden(3,true);
            ui->TV_Explorer->setHeaderHidden(true);
            ui->TV_Explorer->expandToDepth(1);
    }
    //----------------------------------------------------------------------

    //Catalog the files of a directory
    void MainWindow::CatalogDirectory(QString newCatalogPath)
    {
        // Start animation while cataloging
        QApplication::setOverrideCursor(Qt::WaitCursor);

        // Get directory to catalog
        QString directory = newCatalogPath;
        //KMessageBox::information(this,"path:\n"+newCatalogPath);

        // Get the file type of catalog
        //DEV: replace by editable list of file type definition
        QStringList fileTypes;
        if      ( ui->RB_C_FileType_Image->isChecked() )
                fileTypes = fileType_Image;
        else if ( ui->RB_C_FileType_Audio->isChecked() )
                fileTypes = fileType_Audio;
        else if ( ui->RB_C_FileType_Video->isChecked() )
                fileTypes = fileType_Video;
        else if ( ui->RB_C_FileType_Text->isChecked() )
                fileTypes = fileType_Text;
        else    fileTypes.clear();

        //Iterate in the directory to create a list of files
        QStringList filelist;
        QDirIterator iterator(directory, fileTypes, QDir::Files, QDirIterator::Subdirectories);
        while (iterator.hasNext()){
            filelist << (iterator.next());
        }

        //Display and store file number
        //Count the number of files
        int catalogFilesNumber = filelist.count();
        ui->L_FilesNumber->setNum(catalogFilesNumber);

        //filelist.append("<catalogName>"+newCatalogName);
        filelist.prepend("<catalogFileCount>"+QString::number(catalogFilesNumber));
        filelist.prepend("<catalogSourcePath>"+newCatalogPath);
        //filelist.prepend("<catalogName>"+newCatalogName);
        //QString text = "<catalogFileCount>"+QString::number(catalogFilesNumber);
        //KMessageBox::information(this,"test:\n"+text);

        //Define and populate a model and send it to the listView
        fileListModel = new QStringListModel(this);
        fileListModel->setStringList(filelist);
        //ui->LV_FileList->setModel(fileListModel);

        //DEV   Stop animation
        QApplication::restoreOverrideCursor();
    }
    //----------------------------------------------------------------------

    //Save a catalog to a new file
    void MainWindow::SaveCatalog(QString newCatalogName)
    {
        // Get the model/data from the listview
        //QStringListModel *catalogModel = (QStringListModel*)ui->LV_FileList->model();

        // Get the file list from this model
        QStringList filelist = fileListModel->stringList();

        // Stream the list to the file
        QFile fileOut( collectionFolder +"/"+ newCatalogName + ".idx" );

        // write data

          if (fileOut.open(QFile::WriteOnly | QFile::Text)) {
            QTextStream stream(&fileOut);
            for (int i = 0; i < filelist.size(); ++i)
              stream << filelist.at(i) << '\n';
          } else {
            std::cerr << "error opening output file\n";
            //return EXIT_FAILURE;
          }
          fileOut.close();
    }

    //Send selected folder in the tree
    void MainWindow::on_TV_Explorer_activated(const QModelIndex &index)
    {//Sends the selected folder in the tree for the New Catalog Path)
        //Get the model/data from the tree
        QFileSystemModel* pathmodel = (QFileSystemModel*)ui->TV_Explorer->model();
        //get data from the selected file/directory
        QFileInfo fileInfo = pathmodel->fileInfo(index);
        //send the path to the line edit
        ui->LE_NewCatalogPath->setText(fileInfo.filePath());
    }
    //----------------------------------------------------------------------

    //Pick a directory from a dialog window
    void MainWindow::on_PB_PickPath_clicked()
    {
        //Get current selected path as default path for the dialog window
        newCatalogPath = ui->LE_NewCatalogPath->text();

        //Open a dialog for the user to select the directory to be cataloged. Only show directories.
        QString dir = QFileDialog::getExistingDirectory(this, tr("Select the directory to be cataloged in this new catalog"),
                                                        newCatalogPath,
                                                        QFileDialog::ShowDirsOnly
                                                        | QFileDialog::DontResolveSymlinks);
        //Send the selected directory to LE_NewCatalogPath (input line for the New Catalog Path)
        ui->LE_NewCatalogPath->setText(dir);

        //Select this directory in the treeview.
        LoadFileSystem(newCatalogPath);
    }
    //----------------------------------------------------------------------

    //Generate the Catalog name from the path
    void MainWindow::on_PB_GenerateFromPath_clicked()
    {
        //Just copy the Catalog path to the name
        QString newCatalogName = ui->LE_NewCatalogPath->text();
        newCatalogName.replace("/","__");
        ui->LE_NewCatalogName->setText(newCatalogName);
    }
    //----------------------------------------------------------------------

    //Launch the cataloging, save it, and show it
    void MainWindow::on_PB_CreateCatalog_clicked()
    {
        //Get inputs
        newCatalogPath = ui->LE_NewCatalogPath->text();
        newCatalogName = ui->LE_NewCatalogName->text();

        //Catalog files
        if (newCatalogName!="" and newCatalogPath!="")
                CatalogDirectory(newCatalogPath);
        else KMessageBox::error(this,
                                  i18n("Please provide a name and select a path for this new catalog.\n Name: ")
                                  +newCatalogName+"\n Path: "+newCatalogPath,
                                  i18n( "Info" ) );

        //Save the catalog to a new file
        SaveCatalog(newCatalogName);

        //Refresh the catalog list
        LoadCatalogFileList();
        LoadCatalogsToModel();
        refreshCatalogSelectionList();

        LoadCatalog( collectionFolder +"/"+ newCatalogName + ".idx");

        //Chang tab to show the result of the catalog creation
        //DEV refer to the name rather than index?
        ui->tabWidget->setCurrentIndex(0);

    }
    //----------------------------------------------------------------------
