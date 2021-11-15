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
// Purpose:     methods for the screen CREATE
// Description: https://github.com/StephaneCouturier/Katalog/wiki/Create
// Author:      Stephane Couturier
/////////////////////////////////////////////////////////////////////////////
*/

#include "mainwindow.h"
#include "ui_mainwindow.h"

#include <QFileDialog>
#include <QTextStream>
#include <QDesktopServices>

//UI----------------------------------------------------------------------------

    void MainWindow::on_Create_treeView_Explorer_clicked(const QModelIndex &index)
    {
        //Sends the selected folder in the tree for the New Catalog Path

        //Get the model/data from the tree
        QFileSystemModel* pathmodel = (QFileSystemModel*)ui->Create_treeView_Explorer->model();
        //get data from the selected file/directory
        QFileInfo fileInfo = pathmodel->fileInfo(index);
        //send the path to the line edit
        ui->Create_lineEdit_NewCatalogPath->setText(fileInfo.filePath());
    }
    //--------------------------------------------------------------------------
    void MainWindow::on_Create_pushButton_PickPath_clicked()
    {
        //Pick a directory from a dialog window

        //Get current selected path as default path for the dialog window
        newCatalogPath = ui->Create_lineEdit_NewCatalogPath->text();

        //Open a dialog for the user to select the directory to be cataloged. Only show directories.
        QString dir = QFileDialog::getExistingDirectory(this, tr("Select the directory to be cataloged in this new catalog"),
                                                        newCatalogPath,
                                                        QFileDialog::ShowDirsOnly
                                                        | QFileDialog::DontResolveSymlinks);
        //Send the selected directory to LE_NewCatalogPath (input line for the New Catalog Path)
        ui->Create_lineEdit_NewCatalogPath->setText(dir);

        //Select this directory in the treeview.
        loadFileSystem(newCatalogPath);
    }
    //--------------------------------------------------------------------------
    void MainWindow::on_Create_pushButton_EditExcludeList_clicked()
    {
        //Edit Exclusion list

        //Verify if a folder exclusion list exists
        QFile excludeFile(excludeFilePath);
        if ( excludeFile.exists()){
            QDesktopServices::openUrl(QUrl::fromLocalFile(excludeFilePath));
        }
        else{
            //if not, propose to create it
            int result = QMessageBox::warning(this,tr("Update"),tr("No list found, create one?"),
                                              QMessageBox::Yes | QMessageBox::Cancel);

            if ( result == QMessageBox::Cancel){
                return;
            }
            else{
                // Create it, if it does not exist
                if(!excludeFile.open(QIODevice::ReadOnly)) {

                    if (excludeFile.open(QFile::WriteOnly | QFile::Text)) {

                          QTextStream stream(&excludeFile);

                          //insert one line as an example
                          stream << tr("folder/path/without/slash_at_the_end");

                          excludeFile.close();

                          //and open it for edition
                          QDesktopServices::openUrl(QUrl::fromLocalFile(excludeFilePath));
                    }
                }
            }
        }
    }
    //--------------------------------------------------------------------------
    void MainWindow::on_Create_pushButton_AddStorage_clicked()
    {
        //Change tab to show the screen to add a storage
        ui->tabWidget->setCurrentIndex(4); // tab 1 is the Collection tab
    }
    //--------------------------------------------------------------------------
    void MainWindow::on_Create_pushButton_GenerateFromPath_clicked()
    {
        //Generate the Catalog name from the path
        QString newCatalogName = ui->Create_lineEdit_NewCatalogPath->text();
        newCatalogName.replace("/","_");
        newCatalogName.replace(":","_");
        ui->Create_lineEdit_NewCatalogName->setText(newCatalogName);
    }
    //--------------------------------------------------------------------------
    void MainWindow::on_Create_pushButton_CreateCatalog_clicked()
    {
        //Launch the cataloging, save it, and show it

        //Get inputs
        newCatalogPath = ui->Create_lineEdit_NewCatalogPath->text();
        newCatalogName = ui->Create_lineEdit_NewCatalogName->text();
        newCatalogStorage = ui->Create_comboBox_StorageSelection->currentText();
        //get other options
        bool includeHidden = ui->Create_checkBox_IncludeHidden->isChecked();
        bool includeSymblinks = ui->Create_checkBox_IncludeSymblinks->isChecked();
        // Get the file type for the catalog
        QStringList fileTypes;
        QString selectedCreateFileType;
        if      ( ui->Create_radioButton_FileType_Image->isChecked() ){
                fileTypes = fileType_Image;
                selectedCreateFileType = "Image";}
        else if ( ui->Create_radioButton_FileType_Audio->isChecked() ){
                fileTypes = fileType_Audio;
                selectedCreateFileType = "Audio";}
         else if ( ui->Create_radioButton_FileType_Video->isChecked() ){
                fileTypes = fileType_Video;
                selectedCreateFileType = "Video";}
        else if ( ui->Create_radioButton_FileType_Text->isChecked() ){
                fileTypes = fileType_Text;
                selectedCreateFileType = "Text";}
        else    fileTypes.clear();

        //check if the file already exists
        QString fullCatalogPath = collectionFolder + "/" + newCatalogName + ".idx";
        QFile file(fullCatalogPath);
        if (file.exists()==true){
            QMessageBox::information(this, "Katalog",
                                     tr("There is already a catalog with this name:")
                                        + newCatalogName
                                        + "\n"+tr("Choose a different name."),
                                     ( tr("Ok") ) );
            return;
        }

        //Catalog files
        if (newCatalogName!="" and newCatalogPath!="")
                catalogDirectory(newCatalogPath,includeHidden, selectedCreateFileType, fileTypes, newCatalogStorage, includeSymblinks);
        else QMessageBox::warning(this, "Katalog",
                                  tr("Provide a name and select a path for this new catalog.")+"\n" +tr("Name:")
                                  +newCatalogName+"\n"+tr("Path:")+newCatalogPath,
                                  ( tr("Ok") ) );

        //Check if no files where found, and let the user decide what to do
        // Get the catalog file list
        QStringList filelist = fileListModel->stringList();
        if (filelist.count() == 5){ //the CatalogDirectory method always adds 2 lines for the catalog info, there should be ignored
            int result = QMessageBox::warning(this, "Katalog - Warning",
                                tr("The source folder does not contain any file.\n"
                                     "This could mean that the source is empty or the device is not mounted to this folder.\n")
                                     +tr("Do you want to save it anyway (the catalog would be empty)?\n"), QMessageBox::Yes
                                              | QMessageBox::Cancel);
            if ( result != QMessageBox::Cancel){
                return;
            }
        }
        //Save the catalog to a new file
        saveCatalogToNewFile(newCatalogName);

        //Refresh the catalog list for the Search screen
        loadCatalogFilesToTable();
        //Refresh the catalog list for the Collection screen
        loadCatalogsToModel();


        //Refresh the catalog list for the combobox of the Search screen
            //Get current search selection
            selectedSearchLocation = ui->Filters_comboBox_SelectLocation->currentText();
            selectedSearchStorage = ui->Filters_comboBox_SelectStorage->currentText();
            selectedSearchCatalog = ui->Filters_comboBox_SelectCatalog->currentText();

            //Refresh list
            refreshCatalogSelectionList(selectedSearchLocation,selectedSearchStorage);

            //Restore selcted catalog
            ui->Filters_comboBox_SelectCatalog->setCurrentText(selectedSearchCatalog);

        QMessageBox::information(this, "Katalog",
                                  tr("The new catalog,has been created.\n Name:   ")
                                  +newCatalogName+"\n" +tr("Source:") + newCatalogPath,
                                  ( tr("Ok") ) );

        //Add new catalog values to the statistics log, if the user has chosen this options.
            if ( ui->Settings_checkBox_SaveRecordWhenUpdate->isChecked() == true ){

                //Save values
                recordSelectedCatalogStats(newCatalogName, selectedCatalogFileCount, selectedCatalogTotalFileSize);

                //Reload stats file to refresh values
                loadStatisticsChart();
            }

        //Change tab to show the result of the catalog creation
        ui->tabWidget->setCurrentIndex(1); // tab 1 is the Collection tab

        //Disable buttons to force user to select a catalog
        ui->Collection_pushButton_Search->setEnabled(false);
        ui->Collection_pushButton_ViewCatalog->setEnabled(false);
        ui->Collection_pushButton_EditCatalogFile->setEnabled(false);
        ui->Collection_pushButton_UpdateCatalog->setEnabled(false);
        ui->Collection_pushButton_ViewCatalogStats->setEnabled(false);
        ui->Collection_pushButton_DeleteCatalog->setEnabled(false);

    }
    //--------------------------------------------------------------------------

//Methods-----------------------------------------------------------------------

    //Load file system for the treeview
    void MainWindow::loadFileSystem(QString newCatalogPath)
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
            ui->Create_treeView_Explorer->setModel(fileSystemModel);

        // Only show the tree, hidding other columns and the header row.
            ui->Create_treeView_Explorer->setColumnWidth(0,250);
            ui->Create_treeView_Explorer->setColumnHidden(1,true);
            ui->Create_treeView_Explorer->setColumnHidden(2,true);
            ui->Create_treeView_Explorer->setColumnHidden(3,true);
            ui->Create_treeView_Explorer->setHeaderHidden(true);
            ui->Create_treeView_Explorer->expandToDepth(1);
    }
    //--------------------------------------------------------------------------
    void MainWindow::loadStorageList()
    {
        //Prepare list for the Storage selection combobox
        QStringList storageNameListforCombo = storageNameList;
        storageNameListforCombo.prepend("");
        fileListModel = new QStringListModel(this);
        fileListModel->setStringList(storageNameListforCombo);
        ui->Create_comboBox_StorageSelection->setModel(fileListModel);
        ui->Catalogs_comboBox_Storage->setModel(fileListModel);
    }
    //--------------------------------------------------------------------------
    void MainWindow::catalogDirectory(QString newCatalogPath,
                                      bool includeHidden,
                                      QString fileType,
                                      QStringList fileTypes,
                                      QString newCatalogStorage,
                                      bool includeSymblinks)
    {
        //Catalog the files of a directory and add catalog meta-data

        // Start animation while cataloging
        QApplication::setOverrideCursor(Qt::WaitCursor);

        // Get directory to catalog
        QString directory = newCatalogPath;

        qint64 catalogTotalFileSize = 0;

        // Get directories to exclude
        QStringList excludedFolders;
        QFile excludeFile(excludeFilePath);
        if(!excludeFile.open(QIODevice::ReadOnly)) {
             //QMessageBox::information(this,"Katalog",tr("No exclude file found."));
             //return;
        }
        QTextStream textStream(&excludeFile);
        QString line;
        while (true)
        {
            line = textStream.readLine();
            if (line.isNull())
                break;
            else
                excludedFolders << line;
        }
        excludeFile.close();

        //Iterate in the directory to create a list of files
        QStringList fileList;

        if (includeHidden == true){
            QDirIterator iterator(directory, fileTypes, QDir::Files|QDir::Hidden, QDirIterator::Subdirectories);
            while (iterator.hasNext()){

                 //Get file information  (absolute path, size, datetime)
                QString filePath = iterator.next();

                qint64 fileSize;
                QFile file(filePath);
                //if (file.open(QIODevice::ReadOnly)){
                fileSize = file.size();
                catalogTotalFileSize = catalogTotalFileSize + fileSize;

                QFileInfo fileInfo(filePath);
                QDateTime fileDate = fileInfo.lastModified();

                //exclude if the folder is part of excluded directories
                bool excludeFile = false;
                //exclude files in /directory/lowerlevel/file when exclude fodler is in /directory
                for (int i=0; i<excludedFolders.length(); i++) {
                    if(fileInfo.absolutePath().contains(excludedFolders[i]+"/") ){
                        excludeFile = true;
                        break;
                    }
                }
                //exclude files in /directory/file when exclude fodler is in /directory
                if (excludedFolders.contains(fileInfo.absolutePath())){
                    excludeFile = true;
                }
                //add file to list if not excluded
                if(excludeFile == false){
                        fileList << filePath + "\t" + QString::number(fileSize) + "\t" + fileDate.toString("yyyy/MM/dd hh:mm:ss");
                }
            }
        }
        else{
            QDirIterator iterator(directory, fileTypes, QDir::Files, QDirIterator::Subdirectories);
            while (iterator.hasNext()){

                //Get file information  (absolute path, size, datetime)
                QString filePath = iterator.next();

                qint64 fileSize;
                QFile file(filePath);
                //if (file.open(QIODevice::ReadOnly)){
                fileSize = file.size();
                catalogTotalFileSize = catalogTotalFileSize + fileSize;

                QFileInfo fileInfo(filePath);
                QDateTime fileDate = fileInfo.lastModified();

                //exclude if the folder is part of excluded directories
                bool excludeFile = false;
                //exclude files in /directory/lowerlevel/file when exclude fodler is in /directory
                for (int i=0; i<excludedFolders.length(); i++) {
                    if(fileInfo.absolutePath().contains(excludedFolders[i]+"/") ){
                        excludeFile = true;
                        break;
                    }
                }
                //exclude files in /directory/file when exclude fodler is in /directory
                if (excludedFolders.contains(fileInfo.absolutePath())){
                    excludeFile = true;
                }
                //add file to list if not excluded
                if(excludeFile == false){
                        fileList << filePath + "\t" + QString::number(fileSize) + "\t" + fileDate.toString("yyyy/MM/dd hh:mm:ss");
                }
            }
        }

        //Display and store file number
        //Count the number of files
        int catalogFilesNumber = fileList.count();
        ui->Explore_label_FilesNumberDisplay->setNum(catalogFilesNumber);

        //filelist.append("<catalogName>"+newCatalogName);
        fileList.prepend("<catalogIncludeSymblinks>"+ QVariant(includeSymblinks).toString());
        fileList.prepend("<catalogStorage>"         + newCatalogStorage);
        fileList.prepend("<catalogFileType>"        + fileType);
        fileList.prepend("<catalogIncludeHidden>"   + QVariant(includeHidden).toString());
        fileList.prepend("<catalogTotalFileSize>"   + QString::number(catalogTotalFileSize));
        fileList.prepend("<catalogFileCount>"       + QString::number(catalogFilesNumber));
        fileList.prepend("<catalogSourcePath>"      + newCatalogPath);

        //Define and populate a model
        fileListModel = new QStringListModel(this);
        fileListModel->setStringList(fileList);

        //Set this new catalog as the selected Catalog
        selectedCatalogFileCount        = catalogFilesNumber;
        selectedCatalogTotalFileSize    = catalogTotalFileSize;

        QSqlQuery query;
        query.prepare("UPDATE Catalog "
                        "SET catalogIncludeSymblinks =:catalogIncludeSymblinks, "
                            "catalogFileCount =:catalogFileCount, "
                            "catalogTotalFileSize =:catalogTotalFileSize "
                        "WHERE catalogSourcePath =:catalogSourcePath ");
        //query.bindValue(":catalogIncludeSymblinks", QVariant(includeSymblinks).toString());
        query.bindValue(":catalogFileCount", catalogFilesNumber);
        query.bindValue(":catalogTotalFileSize", catalogTotalFileSize);
        query.bindValue(":catalogSourcePath", newCatalogPath);
        query.exec();

        loadCatalogFilesToTable();

        //Stop animation
        QApplication::restoreOverrideCursor();
    }
    //--------------------------------------------------------------------------
    void MainWindow::saveCatalogToNewFile(QString newCatalogName)
    {
        //Save a catalog to a new file

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
              QMessageBox::information(this,"Katalog","error opening output file\n");
            //return EXIT_FAILURE;
          }
          fileOut.close();
    }
    //--------------------------------------------------------------------------



