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
// File Name:   mainwindow_tab_collection.cpp
// Purpose:     methods for the scren Collection AND the screen Explore
// Description:
// Author:      Stephane Couturier
// Modified by: Stephane Couturier
// Created:     2020-07-11
// Version:     0.9
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

//#include <KMessageBox>
//#include <KLocalizedString>

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
                loadStorageModel();
                loadCatalogsToModel();
                refreshCatalogSelectionList();
            }

            //Reset selected catalog values (to avoid updating the last selected one for instance)
            //DEV: replace by button becoming enabled once catalog is selected
            selectedCatalogFile="";
            selectedCatalogName="";
            selectedCatalogPath="";
        }
        //----------------------------------------------------------------------
        void MainWindow::on_Collection_PB_Reload_clicked()
        {
            loadCatalogsToModel();
            refreshCatalogSelectionList();
            loadStorageModel();
        }


    //Catalog buttons
        void MainWindow::on_TrV_CatalogList_clicked(const QModelIndex &index)
        {
            selectedCatalogFile             = ui->TrV_CatalogList->model()->index(index.row(), 0, QModelIndex()).data().toString();
            selectedCatalogName             = ui->TrV_CatalogList->model()->index(index.row(), 1, QModelIndex()).data().toString();
            selectedCatalogFileCount        = ui->TrV_CatalogList->model()->index(index.row(), 3, QModelIndex()).data().toLongLong();
            selectedCatalogTotalFileSize    = ui->TrV_CatalogList->model()->index(index.row(), 4, QModelIndex()).data().toString();
            selectedCatalogPath             = ui->TrV_CatalogList->model()->index(index.row(), 5, QModelIndex()).data().toString();
            selectedCatalogFileType         = ui->TrV_CatalogList->model()->index(index.row(), 6, QModelIndex()).data().toString();
            selectedCatalogIncludeHidden    = ui->TrV_CatalogList->model()->index(index.row(), 8, QModelIndex()).data().toBool();
            selectedCatalogStorage          = ui->TrV_CatalogList->model()->index(index.row(), 9, QModelIndex()).data().toString();
            selectedCatalogIncludeSymblinks = ui->TrV_CatalogList->model()->index(index.row(),10, QModelIndex()).data().toBool();

            //display buttons
            ui->Collection_PB_Search->setEnabled(true);
            ui->PB_ViewCatalog->setEnabled(true);
            ui->PB_C_Rename->setEnabled(true);
            ui->PB_EditCatalogFile->setEnabled(true);
            ui->PB_UpdateCatalog->setEnabled(true);
            ui->Collection_pushButton_Convert->setEnabled(true);
            ui->PB_RecordCatalogStats->setEnabled(true);
            ui->Collection_PB_ViewCatalogStats->setEnabled(true);
            ui->PB_DeleteCatalog->setEnabled(true);
        }

        //----------------------------------------------------------------------
        void MainWindow::on_PB_C_OpenFolder_clicked()
        {
            //Open the selected collection folder
            QDesktopServices::openUrl(QUrl::fromLocalFile(collectionFolder));
        }
        //----------------------------------------------------------------------
        void MainWindow::on_Collection_PB_Search_clicked()
        {
            //Change the selected catalog in Search tab
            ui->CB_SelectCatalog->setCurrentText(selectedCatalogName);

            //Go to the Search tab
            ui->tabWidget->setCurrentIndex(0); // tab 0 is the Search tab
        }

        //----------------------------------------------------------------------
        void MainWindow::on_PB_UpdateCatalog_clicked()
        {
            //Update the Selected Catalog

            //Check if the update can be done, or inform the user otherwise
            if(selectedCatalogFile == "not recorded" or selectedCatalogName == "not recorded" or selectedCatalogPath == "not recorded"){
            QMessageBox::information(this,"Katalog","It seems this catalog was not correctly imported or has an old format.\n"
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
            QMessageBox::information(this,"Katalog","Please select a catalog first (some info is missing).\nselectedCatalogFile:"
                                     +selectedCatalogFile+"\nselectedCatalogName: "
                                     +selectedCatalogName+"\nselectedCatalogPath: "
                                     +selectedCatalogPath);
            return;
            }

            newCatalogName = selectedCatalogName;

            QStringList fileTypes;
            if      ( selectedCatalogFileType == "Image")
                                    fileTypes = fileType_Image;
            else if ( selectedCatalogFileType == "Audio")
                                    fileTypes = fileType_Audio;
            else if ( selectedCatalogFileType == "Video")
                                    fileTypes = fileType_Video;
            else if ( selectedCatalogFileType == "Text")
                                    fileTypes = fileType_Text;
            else                    fileTypes.clear();

            QDir dir (selectedCatalogPath);
            if (dir.exists()==true){
                CatalogDirectory(selectedCatalogPath, selectedCatalogIncludeHidden, selectedCatalogFileType, fileTypes, selectedCatalogStorage, selectedCatalogIncludeSymblinks);

                //Warning and choice if the result is 0 files
                QStringList filelist = fileListModel->stringList();
                if (filelist.count() == 3){ //the CatalogDirectory method always adds 2 lines for the catalog info, there should be ignored
                    int result = QMessageBox::warning(this,"Katalog",
                                        ("The source folder does not contains any file.\n"
                                             "This could mean that the source is empty indeed, or that the device attached is not mounted. \n"
                                             "Do you want to update it anyway (the catalog would then be empty)?\n"),QMessageBox::Yes | QMessageBox::Cancel);
                    if ( result != QMessageBox::Cancel){
                        return;
                    }
                }

                SaveCatalog(selectedCatalogName);
                QMessageBox::information(this,"Katalog","This catalog was updated.");
            }
            else {
                QMessageBox::information(this,"Katalog","This catalog cannot be updated.\n"
                                                "The source folder - "+selectedCatalogPath+" - was not found.\n"
                                                "Possible reasons:\n"
                                                "- the device is not connected and mounted\n"
                                                "- the folder was moved or renamed"
                                         );
            }

            if ( ui->Settings_ChBx_SaveRecordWhenUpdate->isChecked() == true )
                recordSelectedCatalogStats();

            //Refresh the collection view
            loadCatalogsToModel();
            //Reload stats file
            statsLoadChart();

        }
        //----------------------------------------------------------------------
        void MainWindow::on_PB_ViewCatalog_clicked()
        {
            //View the files of the Selected Catalog
            LoadCatalog(selectedCatalogFile);
            LoadFilesToModel();

            exploreLoadDirectories();

            //Go to the Search tab
            ui->L_E_CatalogName->setText(selectedCatalogName);
            ui->L_E_CatalogPath->setText(selectedCatalogPath);
            ui->tabWidget->setCurrentIndex(2); // tab 0 is the Explorer tab
        }
        //----------------------------------------------------------------------
        void MainWindow::on_TrV_CatalogList_doubleClicked(const QModelIndex &index)
        {
            //Get file from selected row
            selectedCatalogFile = ui->TrV_CatalogList->model()->index(index.row(), 0, QModelIndex()).data().toString();
            LoadCatalog(selectedCatalogFile);
            LoadFilesToModel();

            //Go to the Search tab
            ui->L_E_CatalogName->setText(selectedCatalogName);
            ui->L_E_CatalogPath->setText(selectedCatalogPath);
            ui->tabWidget->setCurrentIndex(2); // tab 0 is the Explorer tab
        }
        //----------------------------------------------------------------------
        void MainWindow::on_PB_C_Rename_clicked()
        {
            //Get info from the selected catalog
            QFileInfo selectedCatalogFileInfo(selectedCatalogFile);
            QString currentCatalogName = selectedCatalogFileInfo.baseName();

            //Display an input box with the current file name (without extension)
            bool ok;
            QString newCatalogName = QInputDialog::getText(this, tr("QInputDialog::getText()"),
                                                 tr("Enter new catalog name:"), QLineEdit::Normal,
                                                 currentCatalogName, &ok); //(QDir::home().dirName())
            //generate the full new name of the
            QString newCatalogFullName = selectedCatalogFileInfo.absolutePath() + "/" + newCatalogName + ".idx";

            //Rename the catalog file
            if (ok && !newCatalogName.isEmpty()){
                 QFile::rename(selectedCatalogFile, newCatalogFullName);

                 //refresh catalog lists
                    loadCatalogsToModel();
                    //LoadCatalogFileList();
                    refreshCatalogSelectionList();
            }
        }
        //----------------------------------------------------------------------
        void MainWindow::on_PB_EditCatalogFile_clicked()
        {
            QDesktopServices::openUrl(QUrl::fromLocalFile(selectedCatalogFile));
        }

        //----------------------------------------------------------------------
        void MainWindow::on_Collection_pushButton_Convert_clicked()
        {
            convertCatalog(selectedCatalogFile);
        }

        //----------------------------------------------------------------------
        void MainWindow::on_PB_RecordCatalogStats_clicked()
        {
            recordSelectedCatalogStats();
        }
        //----------------------------------------------------------------------
        void MainWindow::on_PB_DeleteCatalog_clicked()
        {
            if ( selectedCatalogFile != ""){

                int result = QMessageBox::warning(this,"Katalog",
                          ("Do you want to delete this catalog?\n")+selectedCatalogFile,QMessageBox::Yes|QMessageBox::Cancel);

                if ( result ==QMessageBox::Yes){
                    QFile file (selectedCatalogFile);
                    file.moveToTrash();
                    loadCatalogsToModel();
                    refreshCatalogSelectionList();
                }
             }
            else QMessageBox::information(this,"Katalog",("Please select a catalog above first."));
        }
        //----------------------------------------------------------------------
        //DEV
        void MainWindow::on_PB_ExportCatalog_clicked()
        {
            QString link = "https://sourceforge.net/p/katalogg/tickets/";
            QMessageBox::information(this,"Katalog","There is no export function yet.\n Please tell what you expect from it by opening a ticket on on:\n"+link);
            QDesktopServices::openUrl(QUrl("https://sourceforge.net/p/katalogg/tickets/"));
        }
        //----------------------------------------------------------------------
        void MainWindow::on_Collection_PB_ViewCatalogStats_clicked()
        {
            //Collection_PB_ViewCatalogStats
            ui->Stats_CB_SelectCatalog->setCurrentText(selectedCatalogName);
            //Go to the Search tab
            ui->tabWidget->setCurrentIndex(5); // tab 0 is the Search tab
        }

        //----------------------------------------------------------------------

    //File methods
        void MainWindow::on_TrV_FileList_clicked(const QModelIndex &index)
        {
            //Get file from selected row
            QString selectedFileName   = ui->TrV_FileList->model()->index(index.row(), 1, QModelIndex()).data().toString();
            QString selectedFileFolder = ui->TrV_FileList->model()->index(index.row(), 4, QModelIndex()).data().toString();
            QString selectedFile = selectedFileFolder+"/"+selectedFileName;

            //Open the file (fromLocalFile needed for spaces in file name)
            QDesktopServices::openUrl(QUrl::fromLocalFile(selectedFile));
        }



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
            QMessageBox::information(this,"Katalog","Please select a catalog above first.");
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

        int catalogFilesNumber = catalogModel->rowCount();
        ui->L_FilesNumber->setNum(catalogFilesNumber);

        //DEV   Stop animation
        QApplication::restoreOverrideCursor();
    }
    //----------------------------------------------------------------------

    //Load a collection (catalogs)
    void MainWindow::loadCatalogsToModel()
    {
        //Set up temporary lists
        QList<QString> cNames;
        QList<QString> cDateUpdates;
        QList<qint64>  cFileCounts;
        QList<qint64>  cTotalFileSizes;
        QList<QString> cSourcePaths;
        QList<bool>    cSourcePathIsActives;
        QList<QString> cFileTypes;
        QList<QString> cCatalogFilePaths;
        QList<bool>    cCatalogIncludeHiddens;
        QList<QString> cStorages;
        QList<bool>    cIncludeSymblinks;

        //Iterate in the directory to create a list of files and sort it
        QStringList fileTypes;
        fileTypes << "*.idx";

        QDirIterator iterator(collectionFolder, fileTypes, QDir::Files, QDirIterator::Subdirectories);
        while (iterator.hasNext()){

            //LoadCatalogInfo(file);
            // Get infos stored in the file
            QFile catalogFile(iterator.next());
            if(!catalogFile.open(QIODevice::ReadOnly)) {
                QMessageBox::information(this,"Katalog","No catalog found.");
                return;
            }

            QTextStream textStream(&catalogFile);
            //bool catalogNameProvided = false;
            bool catalogSourcePathProvided = false;
            bool catalogFileCountProvided = false;
            bool catalogTotalfileSizeProvided = false;
            bool catalogIncludeHiddenProvided = false;
            bool catalogFileTypeProvided = false;
            bool catalogStorageProvided = false;
            bool catalogIncludeProvided = false;

            QString catalogSourcePath;

            while (true)
            {
                QString line = textStream.readLine();

                if (line.left(19)=="<catalogSourcePath>"){
                    catalogSourcePath = line.right(line.size() - line.lastIndexOf(">") - 1);
                    cSourcePaths.append(catalogSourcePath);
                    catalogSourcePathProvided = true;
                }
                else if (line.left(18)=="<catalogFileCount>"){
                    QString catalogFileCountString = line.right(line.size() - line.lastIndexOf(">") - 1);
                    int catalogFileCount = catalogFileCountString.toInt();
                    cFileCounts.append(catalogFileCount);
                    catalogFileCountProvided = true;
                }
                else if (line.left(22)=="<catalogTotalFileSize>"){
                    QString catalogTotalFileSize = line.right(line.size() - line.lastIndexOf(">") - 1);
                    qint64 catalogFileCount = catalogTotalFileSize.toLongLong();
                    cTotalFileSizes.append(catalogFileCount);
                    catalogTotalfileSizeProvided = true;
                }
                else if (line.left(22)=="<catalogIncludeHidden>"){
                    QString catalogIncludeHidden = line.right(line.size() - line.lastIndexOf(">") - 1);
                    cCatalogIncludeHiddens.append(QVariant(catalogIncludeHidden).toBool());
                    catalogIncludeHiddenProvided = true;
                }
                else if (line.left(17)=="<catalogFileType>"){
                    QString catalogFileType = line.right(line.size() - line.lastIndexOf(">") - 1);
                    cFileTypes.append(catalogFileType);
                    catalogFileTypeProvided = true;
                }
                else if (line.left(16)=="<catalogStorage>"){
                    QString storage = line.right(line.size() - line.lastIndexOf(">") - 1);
                    cStorages.append(storage);
                    catalogStorageProvided = true;
                }
                else if (line.left(25)=="<catalogIncludeSymblinks>"){
                    QString catalogIncludeSymblinks = line.right(line.size() - line.lastIndexOf(">") - 1);
                    cIncludeSymblinks.append(catalogIncludeSymblinks.toInt());
                    catalogIncludeProvided = true;
                }
                else
                    break;

            }

            if(catalogSourcePathProvided==false)
                cSourcePaths.append("not recorded");
            if(catalogFileCountProvided==false)
                cFileCounts.append(0);
            if(catalogTotalfileSizeProvided==false)
                cTotalFileSizes.append(0);
            if(catalogIncludeHiddenProvided==false)
                cCatalogIncludeHiddens.append(false);
            if(catalogFileTypeProvided==false)
                cFileTypes.append("");
            if(catalogStorageProvided==false)
                cStorages.append("");
            if(catalogIncludeProvided==false)
                cIncludeSymblinks.append("");

            //Verify if path is active (drive connected)
            bool test = verifyCatalogPath(catalogSourcePath);
            cSourcePathIsActives.append(test);

            // Get infos about the file itself
            QFileInfo catalogFileInfo(catalogFile);
            cDateUpdates.append(catalogFileInfo.lastModified().toString("yyyy-MM-dd hh:mm:ss"));
            cCatalogFilePaths.append(catalogFileInfo.filePath());
            //QFile file(catalogFile);
            cNames.append(catalogFileInfo.baseName());

        }

        // Create model
        Collection *collectionModel = new Collection(this);

        // Populate model with data
        collectionModel->populateData(cCatalogFilePaths,
                                 cNames,
                                 cDateUpdates,
                                 cFileCounts,
                                 cTotalFileSizes,
                                 cSourcePaths,
                                 cFileTypes,
                                 cSourcePathIsActives,
                                 cCatalogIncludeHiddens,
                                 cStorages,
                                 cIncludeSymblinks);

        QSortFilterProxyModel *proxyCollectionModel = new QSortFilterProxyModel(this);
        proxyCollectionModel->setSourceModel(collectionModel);

        // Connect model to tree/table view
        ui->TrV_CatalogList->setModel(proxyCollectionModel);
        ui->TrV_CatalogList->QTreeView::sortByColumn(1,Qt::AscendingOrder);
        ui->TrV_CatalogList->header()->setSectionResizeMode(QHeaderView::Interactive);
        ui->TrV_CatalogList->header()->resizeSection(1, 300); //Name
        ui->TrV_CatalogList->header()->resizeSection(2, 150); //Date
        ui->TrV_CatalogList->header()->resizeSection(3, 100); //Files
        ui->TrV_CatalogList->header()->resizeSection(4, 125); //TotalFileSize
        ui->TrV_CatalogList->header()->resizeSection(5, 300); //Path
        ui->TrV_CatalogList->header()->resizeSection(6, 100); //FileType
        ui->TrV_CatalogList->header()->resizeSection(7,  50); //Active
        ui->TrV_CatalogList->header()->resizeSection(8,  50); //Storage
        ui->TrV_CatalogList->header()->resizeSection(9,  50); //Symblinks
        ui->TrV_CatalogList->header()->resizeSection(10, 50); //Symblinks
        ui->TrV_CatalogList->header()->hideSection(0); //Path
        ui->TrV_CatalogList->header()->hideSection(10); //Symblinks
        //Pass list of catalogs
            catalogFileList = cNames;
            catalogFileList.sort();

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

        // Get infos stored in the file
        QFile catalogFile(selectedCatalogFile);
        if(!catalogFile.open(QIODevice::ReadOnly)) {
            QMessageBox::information(this,"Katalog","No catalog found.");
            return;
        }

        QTextStream textStream(&catalogFile);

        while (true)
        {
            QString line = textStream.readLine();
            if (line.isNull())
                break;
            else
                if (line.left(1)!="<"){
                    //Split the string with \t into a list
                    QRegExp tagExp("\t");
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

                    //Get file informations
                    QFileInfo file(filePath);

                    //Append data to the lists
                    cfileNames.append(file.fileName());
                    cfileSizes.append(fileSize);
                    cfilePaths.append(file.path());
                    cfileDateTimes.append(fileDateTime);
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
        ui->TrV_FileList->header()->setSectionResizeMode(QHeaderView::Interactive);
        ui->TrV_FileList->header()->resizeSection(0, 600); //Name
        ui->TrV_FileList->header()->resizeSection(1, 110); //Size
        ui->TrV_FileList->header()->resizeSection(2, 140); //Date
        ui->TrV_FileList->header()->resizeSection(3, 400); //Path
    }

    //----------------------------------------------------------------------
    //Verify that the catalog path is accessible (so the related drive is mounted), returns true/false
    bool MainWindow::verifyCatalogPath(QString catalogSourcePath)
    {
        QDir dir(catalogSourcePath);
        bool status = dir.exists();
        return status;
    }
    //----------------------------------------------------------------------
    void MainWindow::recordSelectedCatalogStats()
    {
        QString statisticsFileName = "statistics.csv";

        QString catalogFileCount = QString::number(selectedCatalogFileCount);
        QDateTime nowDateTime = QDateTime::currentDateTime();

        QString statisticsLine = nowDateTime.toString("yyyy-MM-dd hh:mm:ss") + "\t"
                                + selectedCatalogName + "\t"
                                + catalogFileCount + "\t"
                                + selectedCatalogTotalFileSize;

        // Stream the list to the file
        QFile fileOut( collectionFolder + "/" + statisticsFileName );

        //KMessageBox::information(this,"test:" + statisticsLine + "\ntest:" + fileOut.fileName());

        // write data
        if (fileOut.open(QFile::WriteOnly | QIODevice::Append | QFile::Text)) {
            QTextStream stream(&fileOut);
            stream << statisticsLine << "\n";
         }
         fileOut.close();
    }

    //----------------------------------------------------------------------
    void MainWindow::convertCatalog(QString catalogSourcePath)
    {
        //select catalog file
        //QDesktopServices::openUrl(QUrl::fromLocalFile(collectionFolder));

        //verify catalog
        // is it .idx?

        //define new catalog file = existing + _new
        QString catalogNewPath = collectionFolder + "/temp.idx";

        //read catalog file
        QFile catalogFile(catalogSourcePath);
        if(!catalogFile.open(QIODevice::ReadOnly)) {
            QMessageBox::information(this,"Katalog","No catalog found.");
            return;
        }
        QFile fileOut(catalogNewPath);

        //stream line by line
        // Get infos stored in the file
        QTextStream textStream(&catalogFile);
        //QStringList lines;
        while (true)
        {
            QString line = textStream.readLine();
            if (line.isNull())
                break;

            //replace @@ by \t
            line.replace("@@", "\t");
            //lines.append(line);

            // Stream the list to the file


            //KMessageBox::information(this,"test:" + statisticsLine + "\ntest:" + fileOut.fileName());

            //append the line to the new file
            if (fileOut.open(QFile::WriteOnly | QIODevice::Append | QFile::Text)) {
                QTextStream stream(&fileOut);
                stream << line << "\n";
             }
             fileOut.close();

            }

        //rename files
        QString catalogFileName = catalogFile.fileName();
        catalogFile.rename(catalogFile.fileName() + ".bak");
        fileOut.rename(catalogFileName);

        QMessageBox::information(this,"Katalog","Conversion completed.\n");

    }
