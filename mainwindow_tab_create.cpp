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

        qint64 totalFileSize = 0;

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
        QStringList fileList;
        QDirIterator iterator(directory, fileTypes, QDir::Files, QDirIterator::Subdirectories);
        while (iterator.hasNext()){

            //Get file information  (absolute path, size, datetime)
            QString filePath = iterator.next();

            qint64 fileSize;
            QFile file(filePath);
            //if (file.open(QIODevice::ReadOnly)){
            fileSize = file.size();
            //}
            //else fileSize = 999999;

            totalFileSize = totalFileSize + fileSize;

            QFileInfo fileInfo(filePath);
            QDateTime fileDate = fileInfo.lastModified();

            //add the data to the list, @@ is used as a separator for now
            fileList << filePath + "@@" + QString::number(fileSize) + "@@" + fileDate.toString("yyyy/MM/dd hh:mm:ss");
        }

        //Display and store file number
        //Count the number of files
        int catalogFilesNumber = fileList.count();
        ui->L_FilesNumber->setNum(catalogFilesNumber);

        //filelist.append("<catalogName>"+newCatalogName);
        fileList.prepend("<catalogTotalFileSize>"+QString::number(totalFileSize));
        fileList.prepend("<catalogFileCount>"+QString::number(catalogFilesNumber));
        fileList.prepend("<catalogSourcePath>"+newCatalogPath);


        //Define and populate a model and send it to the listView
        fileListModel = new QStringListModel(this);
        fileListModel->setStringList(fileList);
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

        //check if the file already exists
        QString fullCatalogPath = collectionFolder + "/" + newCatalogName + ".idx";

        QFile file(fullCatalogPath);
        if (file.exists()==true){
            KMessageBox::information(this,
                                     i18n("There is already a catalog with this name:    ")
                                        + newCatalogName
                                        + i18n("\nPlease choose a different name or go to Collection to rename, update, or delete the existing one."),
                                     i18n( "Info" ) );
            return;
        }

        //Catalog files
        if (newCatalogName!="" and newCatalogPath!="")
                CatalogDirectory(newCatalogPath);
        else KMessageBox::error(this,
                                  i18n("Please provide a name and select a path for this new catalog.\n Name: ")
                                  +newCatalogName+"\n Path: "+newCatalogPath,
                                  i18n( "Info" ) );

        //Check if no files where found, and let the user decide what to do
        // Get the catalog file list
        QStringList filelist = fileListModel->stringList();
        if (filelist.count() == 2){ //the CatalogDirectory method always adds 2 lines for the catalog info, there should be ignored
            int result = KMessageBox::warningContinueCancel(this,
                                i18n("The source folder does not contains any file.\n"
                                     "This could mean that the source is empty or the device attached is not mounted.\n"
                                     "Do you want to save it anyway (the catalog would be empty)?\n"));
            if ( result != KMessageBox::Continue){
                return;
            }
        }
        //Save the catalog to a new file
        SaveCatalog(newCatalogName);

        //Refresh the catalog list for the Collection screen
        LoadCatalogsToModel();
        //Refresh the catalog list for the combobox of the Search screen
        refreshCatalogSelectionList();

        KMessageBox::information(this,
                                          i18n("The new catalog,has been created.\n Name:   ")
                                          +newCatalogName+"\n Path:     "+newCatalogPath,
                                          i18n( "Info" ) );

        //Load files of the created catalog:
        //DISABLED as it takes a long time for voluminous catalog, letting the user click View if necessary
        //LoadCatalog( collectionFolder +"/"+ newCatalogName + ".idx");

        //Change tab to show the result of the catalog creation
        ui->tabWidget->setCurrentIndex(1); // tab 1 is the Collection tab

    }
    //----------------------------------------------------------------------
