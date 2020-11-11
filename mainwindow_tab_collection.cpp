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
// Version:     0.13
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

//TAB: Collection UI----------------------------------------------------------------------

    //Collection selection
        void MainWindow::on_Collection_pushButton_SelectFolder_clicked()
        {
            //Open a dialog for the user to select the directory of the collection where catalog files are stored.
            QString dir = QFileDialog::getExistingDirectory(this, tr("Select the directory for this collection"),
                                                            collectionFolder,
                                                            QFileDialog::ShowDirsOnly
                                                            | QFileDialog::DontResolveSymlinks);
            //Unless the selection was cancelled, send the selected folder, and refresh the list of catalogs
            if ( dir !=""){
                collectionFolder=dir;
                ui->Collection_lineEdit_CollectionFolder->setText(dir);

                //initiateSearchValues();
                saveSettings();
                loadStorageModel();
                loadCatalogsToModel();
                refreshCatalogSelectionList();
            }

            //Reset selected catalog values (to avoid updating the last selected one for instance)
            selectedCatalogFile="";
            selectedCatalogName="";
            selectedCatalogPath="";
        }
        //----------------------------------------------------------------------
        void MainWindow::on_Collection_pushButton_Reload_clicked()
        {
            loadCatalogsToModel();
            refreshCatalogSelectionList();
            loadStorageModel();

            //hide buttons to force user to select a catalog before allowing any catalg action.
            hideCatalogButtons();
        }
        //----------------------------------------------------------------------
        void MainWindow::on_Collection_pushButton_OpenFolder_clicked()
        {
            //Open the selected collection folder
            QDesktopServices::openUrl(QUrl::fromLocalFile(collectionFolder));
        }
        //----------------------------------------------------------------------

    //Catalog buttons
        void MainWindow::on_Collection_pushButton_Search_clicked()
        {
            //Change the selected catalog in Search tab
            ui->Search_comboBox_SelectCatalog->setCurrentText(selectedCatalogName);

            //Go to the Search tab
            ui->tabWidget->setCurrentIndex(0); // tab 0 is the Search tab
        }
        //----------------------------------------------------------------------
        void MainWindow::on_Collection_pushButton_ViewCatalog_clicked()
        {
            //View the files of the Selected Catalog
            loadCatalogFilesToExplore();

            //Go to the Search tab
            ui->Explore_label_CatalogNameDisplay->setText(selectedCatalogName);
            ui->Explore_label_CatalogPathDisplay->setText(selectedCatalogPath);
            ui->tabWidget->setCurrentIndex(2); // tab 0 is the Explorer tab
        }
        //----------------------------------------------------------------------
        void MainWindow::on_Collection_pushButton_Rename_clicked()
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
        void MainWindow::on_Collection_pushButton_UpdateCatalog_clicked()
        {   //Update the selected catalog

            //Check if the update can be done, or inform the user otherwise.
                //Deal with old versions, where necessary info may have not have been available
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

                //Deal with other cases where some input information is missing
                if(selectedCatalogFile == "" or selectedCatalogName == "" or selectedCatalogPath == ""){
                QMessageBox::information(this,"Katalog","Please select a catalog first (some info is missing).\nselectedCatalogFile:"
                                         +selectedCatalogFile+"\nselectedCatalogName: "
                                         +selectedCatalogName+"\nselectedCatalogPath: "
                                         +selectedCatalogPath);
                return;
                }

            //BackUp the file before is the option is selected
                if ( ui->Settings_checkBox_KeepOneBackUp->isChecked() == true){
                    backupCatalog(selectedCatalogFile);
                }

            //
            newCatalogName = selectedCatalogName;

            //Capture previous FileCount and TotalFileSize to be able to report the changes after the update
            qint64 previousFileCount = selectedCatalogFileCount;
            qint64 previousTotalFileSize = selectedCatalogTotalFileSize;

            //Define the type of files to be included
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

            //
            QDir dir (selectedCatalogPath);
            if (dir.exists()==true){
                ///Warning and choice if the result is 0 files
                if(dir.entryInfoList(QDir::NoDotAndDotDot|QDir::AllEntries).count() == 0)
                {
                    int result = QMessageBox::warning(this,"Directory is empty","The source folder does not contains any file.\n"
                                                  "This could mean indeed that the source is empty, or that the device is not mounted to this folder. \n"
                                                  "Do you want to update it anyway (the catalog would then be empty)?\n",QMessageBox::Yes | QMessageBox::Cancel);
                    //return;
                    if ( result == QMessageBox::Cancel){
                        return;
                    }
                }

                catalogDirectory(selectedCatalogPath, selectedCatalogIncludeHidden, selectedCatalogFileType, fileTypes, selectedCatalogStorage, selectedCatalogIncludeSymblinks);

                saveCatalogToNewFile(selectedCatalogName);

                //Prepare to report changes to the catalog
                qint64 deltaFileCount = selectedCatalogFileCount - previousFileCount;
                qint64 deltaTotalFileSize = selectedCatalogTotalFileSize - previousTotalFileSize;

                //Inform user about the update
                QMessageBox::information(this,"Katalog","<br/>This catalog was updated:<br/><b>" + selectedCatalogName + "</b> "
                                         "<br/><table>"
                                         "<tr><td>Number of files: </td><td><b>" + QString::number(selectedCatalogFileCount) + "</b></td><td>  (added: <b>" + QString::number(deltaFileCount) + "</b>)</td></tr>"
                                         "<tr><td>Total file size: </td><td><b>" + QString::number(selectedCatalogTotalFileSize) + "</b></td><td>  (added: <b>" + QString::number(deltaTotalFileSize) + "</b>)</td></tr>"
                                         "</table>"
                                         ,Qt::TextFormat(Qt::RichText));
            }
            else {
                QMessageBox::information(this,"Katalog","The catalog " + selectedCatalogName + " cannot be updated.\n"
                                                "The source folder - "+selectedCatalogPath+" - was not found.\n"
                                                "Possible reasons:\n"
                                                "- the device is not connected and mounted,\n"
                                                "- the source folder was moved or renamed."
                                         );
            }

            if ( ui->Settings_checkBox_SaveRecordWhenUpdate->isChecked() == true )
                recordSelectedCatalogStats(selectedCatalogName, selectedCatalogFileCount, selectedCatalogTotalFileSize);

            //Refresh the collection view
            loadCatalogsToModel();
            //Reload stats file
            statsLoadChart();
        }
        //----------------------------------------------------------------------
        void MainWindow::on_Collection_pushButton_EditCatalogFile_clicked()
        {
            QDesktopServices::openUrl(QUrl::fromLocalFile(selectedCatalogFile));
        }
        //----------------------------------------------------------------------
        void MainWindow::on_Collection_pushButton_RecordCatalogStats_clicked()
        {
            recordSelectedCatalogStats(selectedCatalogName, selectedCatalogFileCount, selectedCatalogTotalFileSize);
        }
        //----------------------------------------------------------------------
        void MainWindow::on_Collection_pushButton_ViewCatalogStats_clicked()
        {
            ui->Statistics_comboBox_SelectCatalog->setCurrentText(selectedCatalogName);
            //Go to the Search tab
            ui->tabWidget->setCurrentIndex(5); // tab 0 is the Search tab
        }
        //----------------------------------------------------------------------
        void MainWindow::on_Collection_pushButton_Convert_clicked()
        {
            convertCatalog(selectedCatalogFile);
        }
        //----------------------------------------------------------------------
        void MainWindow::on_Collection_pushButton_DeleteCatalog_clicked()
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

    // File methods
        void MainWindow::on_Collection_treeView_CatalogList_clicked(const QModelIndex &index)
        {
            selectedCatalogFile             = ui->Collection_treeView_CatalogList->model()->index(index.row(), 0, QModelIndex()).data().toString();
            selectedCatalogName             = ui->Collection_treeView_CatalogList->model()->index(index.row(), 1, QModelIndex()).data().toString();
            selectedCatalogDateTime         = ui->Collection_treeView_CatalogList->model()->index(index.row(), 2, QModelIndex()).data().toString();
            selectedCatalogFileCount        = ui->Collection_treeView_CatalogList->model()->index(index.row(), 3, QModelIndex()).data().toLongLong();
            selectedCatalogTotalFileSize    = ui->Collection_treeView_CatalogList->model()->index(index.row(), 4, QModelIndex()).data().toLongLong();
            selectedCatalogPath             = ui->Collection_treeView_CatalogList->model()->index(index.row(), 5, QModelIndex()).data().toString();
            selectedCatalogFileType         = ui->Collection_treeView_CatalogList->model()->index(index.row(), 6, QModelIndex()).data().toString();
            selectedCatalogIncludeHidden    = ui->Collection_treeView_CatalogList->model()->index(index.row(), 8, QModelIndex()).data().toBool();
            selectedCatalogStorage          = ui->Collection_treeView_CatalogList->model()->index(index.row(), 9, QModelIndex()).data().toString();
            selectedCatalogIncludeSymblinks = ui->Collection_treeView_CatalogList->model()->index(index.row(),10, QModelIndex()).data().toBool();

            // Display buttons
            ui->Collection_pushButton_Search->setEnabled(true);
            ui->Collection_pushButton_ViewCatalog->setEnabled(true);
            ui->Collection_pushButton_Rename->setEnabled(true);
            ui->Collection_pushButton_EditCatalogFile->setEnabled(true);
            ui->Collection_pushButton_UpdateCatalog->setEnabled(true);
            ui->Collection_pushButton_Convert->setEnabled(true);
            ui->Collection_pushButton_RecordCatalogStats->setEnabled(true);
            ui->Collection_pushButton_ViewCatalogStats->setEnabled(true);
            ui->Collection_pushButton_DeleteCatalog->setEnabled(true);
        }
        //----------------------------------------------------------------------
        void MainWindow::on_Collection_treeView_CatalogList_doubleClicked(const QModelIndex &index)
        {
            // Get file from selected row
            selectedCatalogFile = ui->Collection_treeView_CatalogList->model()->index(index.row(), 0, QModelIndex()).data().toString();
            loadCatalogFilesToExplore();

            // Go to the Search tab
            ui->Explore_label_CatalogNameDisplay->setText(selectedCatalogName);
            ui->Explore_label_CatalogPathDisplay->setText(selectedCatalogPath);
            ui->tabWidget->setCurrentIndex(2); // tab 0 is the Explorer tab
        }
        //----------------------------------------------------------------------

//TAB: Collection methods----------------------------------------------------------------------

    // Load a collection (catalogs)
    void MainWindow::loadCatalogsToModel()
    {
        // Set up temporary lists
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

        // Iterate in the directory to create a list of files and sort it
        QStringList fileTypes;
        fileTypes << "*.idx";

        QDirIterator iterator(collectionFolder, fileTypes, QDir::Files, QDirIterator::Subdirectories);
        while (iterator.hasNext()){

            // Get infos stored in the file
            QFile catalogFile(iterator.next());
            if(!catalogFile.open(QIODevice::ReadOnly)) {
                QMessageBox::information(this,"Katalog","No catalog found.");
                return;
            }

            QTextStream textStream(&catalogFile);
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

            // Verify if path is active (drive connected)
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
        ui->Collection_treeView_CatalogList->setModel(proxyCollectionModel);
        ui->Collection_treeView_CatalogList->QTreeView::sortByColumn(1,Qt::AscendingOrder);
        ui->Collection_treeView_CatalogList->header()->setSectionResizeMode(QHeaderView::Interactive);
        ui->Collection_treeView_CatalogList->header()->resizeSection(1, 300); //Name
        ui->Collection_treeView_CatalogList->header()->resizeSection(2, 150); //Date
        ui->Collection_treeView_CatalogList->header()->resizeSection(3, 100); //Files
        ui->Collection_treeView_CatalogList->header()->resizeSection(4, 125); //TotalFileSize
        ui->Collection_treeView_CatalogList->header()->resizeSection(5, 300); //Path
        ui->Collection_treeView_CatalogList->header()->resizeSection(6, 100); //FileType
        ui->Collection_treeView_CatalogList->header()->resizeSection(7,  50); //Active
        ui->Collection_treeView_CatalogList->header()->resizeSection(8,  50); //Storage
        ui->Collection_treeView_CatalogList->header()->resizeSection(9,  50); //Symblinks
        ui->Collection_treeView_CatalogList->header()->resizeSection(10, 50); //Symblinks
        ui->Collection_treeView_CatalogList->header()->hideSection(0); //Path
        ui->Collection_treeView_CatalogList->header()->hideSection(10); //Symblinks
        //Pass list of catalogs
            catalogFileList = cNames;
            catalogFileList.sort();

    }
    //----------------------------------------------------------------------

    //----------------------------------------------------------------------
    // Verify that the catalog path is accessible (so the related drive is mounted), returns true/false
    bool MainWindow::verifyCatalogPath(QString catalogSourcePath)
    {
        QDir dir(catalogSourcePath);
        bool status = dir.exists();
        return status;
    }
    //----------------------------------------------------------------------
    void MainWindow::recordSelectedCatalogStats(QString selectedCatalogName, int selectedCatalogFileCount, qint64 selectedCatalogTotalFileSize)
    {
        QString statisticsFileName = "statistics.csv";

        QDateTime nowDateTime = QDateTime::currentDateTime();

        QString statisticsLine = nowDateTime.toString("yyyy-MM-dd hh:mm:ss") + "\t"
                                + selectedCatalogName + "\t"
                                + QString::number(selectedCatalogFileCount) + "\t"
                                + QString::number(selectedCatalogTotalFileSize);

        // Stream the list to the file
        QFile fileOut( collectionFolder + "/" + statisticsFileName );

        // Write data
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
    //----------------------------------------------------------------------
    void MainWindow::backupCatalog(QString catalogSourcePath)
    {
        QString catalogBackUpSourcePath = catalogSourcePath + ".bak";

        //Verify if a bak up file already exist and remove it.
        if (QFile::exists(catalogBackUpSourcePath))
        {
            QFile::remove(catalogBackUpSourcePath);
        }

        //Copy the file to the same location, adding .bak for the new file name.
        QFile::copy(catalogSourcePath, catalogBackUpSourcePath);

        //Inform user
        //QMessageBox::information(this,"Katalog","Backup done.\n");

    }
    //----------------------------------------------------------------------
    void MainWindow::hideCatalogButtons()
    {
        //Display buttons
        ui->Collection_pushButton_Search->setEnabled(false);
        ui->Collection_pushButton_ViewCatalog->setEnabled(false);
        ui->Collection_pushButton_Rename->setEnabled(false);
        ui->Collection_pushButton_EditCatalogFile->setEnabled(false);
        ui->Collection_pushButton_UpdateCatalog->setEnabled(false);
        ui->Collection_pushButton_Convert->setEnabled(false);
        ui->Collection_pushButton_RecordCatalogStats->setEnabled(false);
        ui->Collection_pushButton_ViewCatalogStats->setEnabled(false);
        ui->Collection_pushButton_DeleteCatalog->setEnabled(false);
    }
