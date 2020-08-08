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


//TAB: Create Volume ----------------------------------------------------------------------

    //Load file system for the treeview
    void MainWindow::LoadFileSystem(QString newVolumePath)
    {
            newVolumePath="/";
         // Creates a new model
            fileSystemModel = new QFileSystemModel(this);

         // Set filter to show only directories
            fileSystemModel->setFilter(QDir::NoDotAndDotDot | QDir::AllDirs);

         // QFileSystemModel requires root path
            QString rootPath ="/";
            fileSystemModel->setRootPath(rootPath);
            fileSystemModel->setRootPath(newVolumePath);
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
    void MainWindow::CatalogDirectory(QString newVolumePath)
    {
        // Start animation while cataloging
        QApplication::setOverrideCursor(Qt::WaitCursor);

        // Get directory to catalog
        QString directory = newVolumePath;
        //KMessageBox::information(this,"path:\n"+newVolumePath);

        // Get the file type of catalog
        //DEV: replace by editable list of file type definition
        QStringList fileTypes;
        if ( ui->RB_C_FileType_Image->isChecked() )
            fileTypes=fileType_Image;
        else if ( ui->RB_C_FileType_Audio->isChecked() )
            fileTypes=fileType_Audio;
        else if ( ui->RB_C_FileType_Video->isChecked() )
            fileTypes=fileType_Video;
        else if ( ui->RB_C_FileType_Text->isChecked() )
            fileTypes=fileType_Text;
        else fileTypes.clear();

        //Iterate in the directory to create a list of files
        QStringList filelist;
        QDirIterator iterator(directory, fileTypes, QDir::Files, QDirIterator::Subdirectories);
        while (iterator.hasNext()){
            filelist << (iterator.next());
        }

        //Define and populate a model and send it to the listView
        fileListModel = new QStringListModel(this);
        fileListModel->setStringList(filelist);
        ui->LV_FileList->setModel(fileListModel);

        //Count the number of files
        //DEV set a function (duplicate)
        int catalogFilesNumber = fileListModel->rowCount();
        ui->L_FilesNumber->setNum(catalogFilesNumber);

        //Chang tab to show the result of the catalog creation
        //DEV refer to the name rather than index?
        ui->tabWidget->setCurrentIndex(0);

        //DEV   Stop animation
        QApplication::restoreOverrideCursor();
    }
    //----------------------------------------------------------------------
    //Save a catalog to a new file
    void MainWindow::SaveCatalog(QString newVolumeName)
    {
        // Get the model/data from the listview
        QStringListModel *catalogModel = (QStringListModel*)ui->LV_FileList->model();

        // Get the file list from this model
        QStringList filelist = catalogModel->stringList();
        /*KMessageBox::information(this,
                                          i18n("Info: 1")+collectionFolder
                                            +"\n 2 "+newVolumeName
                                            +"\n 3 "+newVolumePath+"\n 4 "+collectionFolder  +"/"+  newVolumeName + ".idx",
                                          i18n( "Info" ) );
        */
        // Stream the list to the file
        QFile fileOut( collectionFolder +"/"+ newVolumeName + ".idx" );

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
    {//Sends the selected folder in the tree for the New Volume Path)
        //Get the model/data from the tree
        QFileSystemModel* pathmodel = (QFileSystemModel*)ui->TV_Explorer->model();
        //get data from the selected file/directory
        QFileInfo fileInfo = pathmodel->fileInfo(index);
        //send the path to the line edit
        ui->LE_NewVolumePath->setText(fileInfo.filePath());
    }
    //----------------------------------------------------------------------
    //Pick a directory from a dialog window
    void MainWindow::on_PB_PickPath_clicked()
    {
        //Get current selected path as default path for the dialog window
        QString newVolumePath = ui->LE_NewVolumePath->text();

        //Open a dialog for the user to select the directory to be cataloged. Only show directories.
        QString dir = QFileDialog::getExistingDirectory(this, tr("Select the directory to be cataloged in this new volume"),
                                                        newVolumePath,
                                                        QFileDialog::ShowDirsOnly
                                                        | QFileDialog::DontResolveSymlinks);
        //Send the selected directory to LE_NewVolumePath (input line for the New Volume Path)
        ui->LE_NewVolumePath->setText(dir);

        //Select this directory in the treeview.
        LoadFileSystem(newVolumePath);
    }
    //----------------------------------------------------------------------
    //Generate the volume name from the path
    void MainWindow::on_PB_GenerateFromPath_clicked()
    {
        //Just copy the volume path to the name
        QString newVolumeName = ui->LE_NewVolumePath->text();
        newVolumeName.replace("/","__");
        ui->LE_NewVolumeName->setText(newVolumeName);
    }
    //----------------------------------------------------------------------
    //Launch the volume catalog, save it, and show it
    void MainWindow::on_PB_CreateCatalog_clicked()
    {
        //Get inputs
        QString newVolumePath = ui->LE_NewVolumePath->text();
        QString newVolumeName = ui->LE_NewVolumeName->text();

        //Catalog files
        if (newVolumeName!="" and newVolumePath!="")
                CatalogDirectory(newVolumePath);
        else KMessageBox::error(this,
                                  i18n("Please provide a name and select a path for this new volume.\n Name: ")
                                  +newVolumeName+"\n Path: "+newVolumePath,
                                  i18n( "Info" ) );

        //Save the catalog to a new file
        SaveCatalog(newVolumeName);

        //Refresh the catalog list
        LoadCatalogList();
        PopulateCatalogSelector();

    }
    //----------------------------------------------------------------------
