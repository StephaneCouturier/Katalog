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
#include "catalogsview.h"
#include "database.h"

#include <QTextStream>
#include <QDesktopServices>
#include <QFileDialog>
#include <QSortFilterProxyModel>

//TAB: Collection UI----------------------------------------------------------------------

    //Collection selection
        void MainWindow::on_Collection_pushButton_SelectFolder_clicked()
        {
            //Open a dialog for the user to select the directory of the collection where catalog files are stored.
            QString dir = QFileDialog::getExistingDirectory(this, tr("Select the directory for this collection"),
                                                            collectionFolder,
                                                            QFileDialog::ShowDirsOnly
                                                            | QFileDialog::DontResolveSymlinks);

            //Unless the selection was cancelled, set the new collection folder, and refresh the list of catalogs
            if ( dir !=""){

                collectionFolder = dir;

                //set the new path in Colletion tab
                ui->Collection_lineEdit_CollectionFolder->setText(collectionFolder);

                //redefine the path of the Storage file
                storageFilePath = collectionFolder + "/" + "storage.csv";

                //save Settings for the new collection folder value;
                saveSettings();

                //load the collection for this new folder;
                loadCollection();

            }

            //Reset selected catalog values (to avoid actions on the last selected one)
            selectedCatalogFile="";
            selectedCatalogName="";
            selectedCatalogPath="";
        }
        //----------------------------------------------------------------------
        void MainWindow::on_Collection_lineEdit_CollectionFolder_returnPressed()
        {
            loadCollection();
        }
        //----------------------------------------------------------------------
        void MainWindow::on_Collection_pushButton_Reload_clicked()
        {
            loadCollection();
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
            ui->Filters_comboBox_SelectCatalog->setCurrentText(selectedCatalogName);

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
                    refreshCatalogSelectionList("","");
            }
        }
        //----------------------------------------------------------------------
        void MainWindow::on_Collection_pushButton_UpdateCatalog_clicked()
        {   //Update the selected catalog

            updateCatalog(selectedCatalogName);
            loadCollection();

        }
        //----------------------------------------------------------------------
        void MainWindow::on_Collection_pushButton_UpdateAllActive_clicked()
        {
            //Get list of active catalogs
            QSqlQuery query;
            query.exec("SELECT catalogName, catalogSourcePathIsActive, catalogSourcePath FROM Catalog WHERE catalogSourcePathIsActive = 1 ");

            // Catalog each result
            while(query.next()){
                //Get catalog name
                QString catalogName = query.value(0).toString();
                //QMessageBox::information(this,"Katalog","catalogName." + catalogName );

                int isActive = verifyCatalogPath(query.value(2).toString());
                //QMessageBox::information(this,"Katalog","isActive." + QString::number(isActive));

                QDir dir (query.value(2).toString());
                if (dir.exists()==true){

                    //QMessageBox::information(this,"Katalog","dir.exists." );

                    ///Warning and choice if the result is 0 files
                    if(dir.entryInfoList(QDir::NoDotAndDotDot|QDir::AllEntries).count() == 0)
                    {
                        //QMessageBox::information(this,"Katalog","dir Empty." );

                        //return;
                    }
                    else{
                        updateCatalog(catalogName);
                    }
                }

                //QMessageBox::information(this,"Katalog","Ok." + catalogName + " _ " + QString::number(isActive));

            }
            QMessageBox::information(this,"Katalog","done.");

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
                    refreshCatalogSelectionList("","");
                }
             }
            else QMessageBox::information(this,"Katalog",("Please select a catalog above first."));
        }
        //----------------------------------------------------------------------

    // File methods
        void MainWindow::on_Collection_treeView_CatalogList_clicked(const QModelIndex &index)
        {
            selectedCatalogName             = ui->Collection_treeView_CatalogList->model()->index(index.row(), 0, QModelIndex()).data().toString();
            selectedCatalogFile             = ui->Collection_treeView_CatalogList->model()->index(index.row(), 1, QModelIndex()).data().toString();
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
            selectedCatalogFile = ui->Collection_treeView_CatalogList->model()->index(index.row(), 1, QModelIndex()).data().toString();
            loadCatalogFilesToExplore();

            // Go to the Search tab
            ui->Explore_label_CatalogNameDisplay->setText(selectedCatalogName);
            ui->Explore_label_CatalogPathDisplay->setText(selectedCatalogPath);
            ui->tabWidget->setCurrentIndex(2); // tab 0 is the Explorer tab
        }

        //----------------------------------------------------------------------

//TAB: Collection methods----------------------------------------------------------------------
    //----------------------------------------------------------------------
    void MainWindow::loadCollection()
    {
        //Load Storage list and refresh their statistics
            loadStorageFileToTable();
            loadStorageTableToModel();
            refreshStorageStatistics();

       //load Catalog list, Location list, Storage list
            loadCatalogFilesToTable();

            loadCatalogsToModel();

            refreshLocationSelectionList();
            refreshStorageSelectionList("All");
            refreshCatalogSelectionList("All", "All");

            //restore Search catalog selection
            ui->Filters_comboBox_SelectLocation->setCurrentText(selectedSearchLocation);
            ui->Filters_comboBox_SelectStorage->setCurrentText(selectedSearchStorage);
            ui->Filters_comboBox_SelectCatalog->setCurrentText(selectedSearchCatalog);

            //hide buttons to force user to select a catalog before allowing any action.
            hideCatalogButtons();



    }

    //----------------------------------------------------------------------
    void MainWindow::loadCatalogFilesToTable()
    {
        //Clear current entires of the catalog table
            QSqlQuery queryDelete;
            queryDelete.prepare( "DELETE FROM catalog" );
            queryDelete.exec();

        //Prepare table and insert query
            QSqlQuery query;
            if (!query.exec(CATALOG_SQL)){
                QMessageBox::information(this,"Katalog","problem to create the table.");
                return;}

            if (!query.prepare(INSERT_CATALOG_SQL)){
                QMessageBox::information(this,"Katalog","problem to insert rows.");
                return;}

        //Iterate in the directory to create a list of files and sort it
            QStringList fileTypes;
            fileTypes << "*.idx";

            QDirIterator iterator(collectionFolder, fileTypes, QDir::Files, QDirIterator::Subdirectories);
            while (iterator.hasNext()){

                // Iterate to the next file
                QFile catalogFile(iterator.next());

                // Get file info
                QFileInfo catalogFileInfo(catalogFile);

                // Verify that the file can be opened
                if(!catalogFile.open(QIODevice::ReadOnly)) {
                    QMessageBox::information(this,"Katalog","No catalog found.");
                    return;
                }

                //Prepare a textsteam for the file
                QTextStream textStream(&catalogFile);

                //Read the first 7 lines and put values in a stringlist
                QStringList catalogValues;
                QString line;
                QString value;

                for (int i=0; i<7; i++) {
                    line = textStream.readLine();
                    value = line.right(line.size() - line.lastIndexOf(">") - 1);
                    catalogValues << value;

                    /*
                    <catalogSourcePath>
                    <catalogFileCount>
                    <catalogTotalFileSize>
                    <catalogIncludeHidden>
                    <catalogFileType>
                    <catalogStorage>
                    <catalogIncludeSymblinks>
                    */


                }

                // Get infos about the file itself
                //QFileInfo catalogFileInfo(catalogFile);
                //QMessageBox::information(this,"Katalog","Ok." + catalogFileInfo.lastModified().toString("yyyy-MM-dd hh:mm:ss"));


                //catalogValues << catalogFileInfo.lastModified().toString("yyyy-MM-dd hh:mm:ss");
                //cCatalogFilePaths.append(catalogFileInfo.filePath());

                // Verify if path is active (drive connected)
                int isActive = verifyCatalogPath(catalogValues[0]);
                /*
                <catalogSourcePath>
                <catalogFileCount>
                <catalogTotalFileSize>
                <catalogIncludeHidden>
                <catalogFileType>
                <catalogStorage>
                <catalogIncludeSymblinks>
                */

                //Insert a line in the table with available data
                        /*
                        catalogID,
                        catalogFilePath,
                        catalogName,
                        catalogDateUpdated,
                        catalogSourcePath,
                        catalogFileCount,
                        catalogTotalFileSize,
                        catalogSourcePathIsActive,
                        catalogIncludeHidden,
                        catalogFileType,
                        catalogStorage,
                        catalogIncludeSymblinks
                        */

                QVariant catalogID = addCatalog(query,
                                catalogFileInfo.filePath(),  //catalogFilePath
                                catalogFileInfo.baseName(),  //catalogName
                                catalogFileInfo.lastModified().toString("yyyy-MM-dd hh:mm:ss"), //catalogDateUpdated
                                catalogValues[0], //catalogSourcePath
                                catalogValues[1].toInt(), //catalogFileCount
                                catalogValues[2].toLongLong(), //catalogTotalFileSize
                                isActive, //catalogSourcePathIsActive
                                catalogValues[3], //catalogIncludeHidden
                                catalogValues[4], //catalogFileType
                                catalogValues[5], //catalogStorage
                                catalogValues[6] //catalogIncludeSymblinks
                                );
                /*
                QVariant catalogID = addCatalog(query,
                                catalogFileInfo.filePath(),  //catalogFilePath
                                catalogFileInfo.baseName(),  //catalogName
                                catalogFileInfo.lastModified().toString("yyyy-MM-dd hh:mm:ss"), //catalogDateUpdated
                                catalogValues[0], //catalogSourcePath
                                catalogValues[1].toInt(), //catalogFileCount
                                catalogValues[2].toLongLong(), //catalogTotalFileSize
                                catalogValues[3], //catalogSourcePathIsActive
                                catalogValues[4], //catalogIncludeHidden
                                catalogValues[5], //catalogFileType
                                catalogValues[6], //catalogStorage
                                catalogValues[7] //catalogIncludeSymblinks
                                );
                */
               //QMessageBox::information(this,"Katalog","catalog " + catalogFileInfo.baseName()) ;

                catalogFile.close();
            }

    }

    //----------------------------------------------------------------------
    // Load a collection (catalogs)
    void MainWindow::loadCatalogsToModel()
    {
        //Generate SQL query from filters.
            QString loadCatalogSQL;

            //main part of the query
            loadCatalogSQL  = QLatin1String(R"( SELECT
                                            catalogName                 AS 'Name',
                                            catalogFilePath             AS 'File Path',
                                            catalogDateUpdated          AS 'Last Update',
                                            catalogFileCount            AS 'Files',
                                            catalogTotalFileSize        AS 'Total Size',
                                            catalogSourcePath           AS 'Source Path',
                                            catalogFileType             AS 'File Type',
                                            catalogSourcePathIsActive   AS 'Active',
                                            catalogIncludeHidden        AS 'Inc.Hidden',
                                            catalogStorage              AS 'Storage'
                                        FROM Catalog
                                        WHERE catalogName !=''
                                        )");

                //adding AND lines for the selected filters
                if ( selectedSearchLocation != "All" )
                    loadCatalogSQL = loadCatalogSQL + " AND catalogStorage = '"+selectedSearchLocation+"' ";

                if ( selectedSearchStorage != "All" )
                    loadCatalogSQL = loadCatalogSQL + " AND catalogStorage = '"+selectedSearchStorage+"' ";

                //last lines
 //               loadStockSQL = loadStockSQL + " GROUP BY s.Ticker ";
 //               loadStockSQL = loadStockSQL + " ORDER BY s.Name ASC ";

            //Execute query
            QSqlQuery loadCatalogQuery;
            loadCatalogQuery.prepare(loadCatalogSQL);
            loadCatalogQuery.exec();

            //Format and send to Treeview
            QSqlQueryModel *catalogQueryModel = new QSqlQueryModel;
            catalogQueryModel->setQuery(loadCatalogQuery);

//            QSortFilterProxyModel *proxyResultsModel = new QSortFilterProxyModel(this);
//            proxyResultsModel->setSourceModel(catalogQueryModel);

            CatalogsView *proxyResultsModel = new CatalogsView(this);
            proxyResultsModel->setSourceModel(catalogQueryModel);

            // Connect model to tree/table view
            ui->Collection_treeView_CatalogList->setModel(proxyResultsModel);
            ui->Collection_treeView_CatalogList->QTreeView::sortByColumn(1,Qt::AscendingOrder);
            ui->Collection_treeView_CatalogList->header()->setSectionResizeMode(QHeaderView::Interactive);
            ui->Collection_treeView_CatalogList->QTreeView::sortByColumn(0,Qt::SortOrder(0));

            //Columns size
            ui->Collection_treeView_CatalogList->header()->hideSection(1); //Path

            ui->Collection_treeView_CatalogList->header()->resizeSection(0, 300); //Name
            ui->Collection_treeView_CatalogList->header()->resizeSection(2, 150); //Date
            ui->Collection_treeView_CatalogList->header()->resizeSection(3, 100); //Files
            ui->Collection_treeView_CatalogList->header()->resizeSection(4, 125); //TotalFileSize
            ui->Collection_treeView_CatalogList->header()->resizeSection(5, 300); //Path
            ui->Collection_treeView_CatalogList->header()->resizeSection(6, 100); //FileType
            ui->Collection_treeView_CatalogList->header()->resizeSection(7,  50); //Active
            ui->Collection_treeView_CatalogList->header()->resizeSection(8,  50); //include
            ui->Collection_treeView_CatalogList->header()->resizeSection(9, 150); //Storage

            //Populate catalog statistics
            QSqlQuery query;
            QString querySQL = QLatin1String(R"(
                                SELECT COUNT(*),SUM(catalogTotalFileSize),SUM(catalogFileCount)
                                FROM Catalog
                                            )");
            query.prepare(querySQL);
            query.exec();
            query.next();

            ui->Collection_label_Catalogs->setText(QString::number(query.value(0).toInt()));
            ui->Collection_label_TotalSize->setText(QLocale().formattedDataSize(query.value(1).toLongLong()));
            ui->Collection_label_TotalNumber->setText(QString::number(query.value(2).toInt()));

    }
    //----------------------------------------------------------------------
    //----------------------------------------------------------------------
    // Verify that the catalog path is accessible (so the related drive is mounted), returns true/false
    int MainWindow::verifyCatalogPath(QString catalogSourcePath)
    {
        QDir dir(catalogSourcePath);
        int status = dir.exists();
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
    void MainWindow::updateCatalog(QString catalogName)
    {
        //Get data from the database

            QString updateCatalogSQL  = QLatin1String(R"(
                                        SELECT  catalogFilePath,
                                                catalogName,
                                                catalogSourcePath,
                                                catalogFileCount,
                                                catalogFileType,
                                                catalogTotalFileSize
                                        FROM Catalog
                                        WHERE catalogName =:catalogName
                                        )");

            QSqlQuery query;
            query.prepare(updateCatalogSQL);
            query.bindValue(":catalogName", catalogName);
            query.exec();
            query.next();

            QString currentCatalogFilePath      = query.value(0).toString();   //selectedCatalogFile
            QString currentCatalogName          = query.value(1).toString();   //selectedCatalogName
            QString currentCatalogSourcePath    = query.value(2).toString();   //selectedCatalogPath
            qint64  currentCatalogFileCount     = query.value(3).toLongLong(); //selectedCatalogFileCount
            QString currentCatalogFileType      = query.value(4).toString();   //selectedCatalogFileType
            qint64  currentCatalogTotalFileSize = query.value(5).toLongLong(); //selectedCatalogTotalFileSize




        //Check if the update can be done, or inform the user otherwise.
            //Deal with old versions, where necessary info may have not have been available
            if(currentCatalogFilePath == "not recorded" or currentCatalogName == "not recorded" or currentCatalogSourcePath == "not recorded"){
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
            if(currentCatalogFilePath == "" or currentCatalogName == "" or currentCatalogSourcePath == ""){
            QMessageBox::information(this,"Katalog","Please select a catalog first (some info is missing).\ncurrentCatalogFilePath:"
                                     +currentCatalogFilePath+"\ncurrentCatalogName: "
                                     +currentCatalogName+"\ncurrentCatalogSourcePath: "
                                     +currentCatalogSourcePath);
            return;
            }

        //BackUp the file before is the option is selected
            if ( ui->Settings_checkBox_KeepOneBackUp->isChecked() == true){
                backupCatalog(currentCatalogFilePath);
            }

        //
        newCatalogName = currentCatalogName;

        //Capture previous FileCount and TotalFileSize to be able to report the changes after the update
        qint64 previousFileCount = currentCatalogFileCount; //currentCatalogFileCount;
        qint64 previousTotalFileSize = currentCatalogTotalFileSize;//currentCatalogTotalFileSize;

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
        QDir dir (currentCatalogSourcePath);
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

            catalogDirectory(currentCatalogSourcePath,
                             selectedCatalogIncludeHidden,
                             selectedCatalogFileType,
                             fileTypes,
                             selectedCatalogStorage,
                             selectedCatalogIncludeSymblinks);

            saveCatalogToNewFile(currentCatalogName);


            //reload new data
            QString querySQL = QLatin1String(R"(
                            SELECT  catalogFilePath,
                                    catalogName,
                                    catalogSourcePath,
                                    catalogFileCount,
                                    catalogFileType,
                                    catalogTotalFileSize
                            FROM Catalog
                            WHERE catalogName =:catalogName
                            )");
            query.prepare(querySQL);
            query.bindValue(":catalogName", catalogName);
            query.exec();
            query.next();
            //currentCatalogFilePath      = query.value(0).toString();   //selectedCatalogFile
            //currentCatalogName          = query.value(1).toString();   //selectedCatalogName
            //currentCatalogSourcePath    = query.value(2).toString();   //selectedCatalogPath
            currentCatalogFileCount     = query.value(3).toLongLong(); //selectedCatalogFileCount
            currentCatalogFileCount     = selectedCatalogFileCount; //retrieved from catalog method
            //currentCatalogFileType      = query.value(4).toString();   //selectedCatalogFileType
            currentCatalogTotalFileSize = query.value(5).toLongLong(); //selectedCatalogTotalFileSize
            currentCatalogTotalFileSize = selectedCatalogTotalFileSize; //retrieved from catalog method

            //Prepare to report changes to the catalog
            qint64 deltaFileCount = currentCatalogFileCount - previousFileCount;
            qint64 deltaTotalFileSize = currentCatalogTotalFileSize - previousTotalFileSize;

            //Inform user about the update
            QMessageBox::information(this,"Katalog","<br/>This catalog was updated:<br/><b>" + currentCatalogName + "</b> "
                                     "<br/><table>"
                                     "<tr><td>Number of files: </td><td><b>" + QString::number(currentCatalogFileCount) + "</b></td><td>  (added: <b>" + QString::number(deltaFileCount) + "</b>)</td></tr>"
                                     "<tr><td>Total file size: </td><td><b>" + QLocale().formattedDataSize(currentCatalogTotalFileSize) + "</b>  </td><td>  (added: <b>" + QLocale().formattedDataSize(deltaTotalFileSize) + "</b>)</td></tr>"
                                     "</table>"
                                     ,Qt::TextFormat(Qt::RichText));
        }
        else {
            QMessageBox::information(this,"Katalog","The catalog " + currentCatalogName + " cannot be updated.\n"
                                            "\nThe source folder - "+currentCatalogSourcePath+" - was not found.\n"
                                            "\nPossible reasons:\n"
                                            "  - the device is not connected and mounted,\n"
                                            "  - the source folder was moved or renamed."
                                     );
        }

        if ( ui->Settings_checkBox_SaveRecordWhenUpdate->isChecked() == true )
            recordSelectedCatalogStats(currentCatalogName, currentCatalogFileCount, currentCatalogTotalFileSize);

        //Refresh the collection view
        loadCatalogsToModel();
        //Reload stats file
        loadStatisticsChart();

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

    //----------------------------------------------------------------------
    void MainWindow::refreshLocationCollectionFilter()
    {
        //Query the full list of locations
        QSqlQuery getLocationList;
        getLocationList.prepare("SELECT DISTINCT storageLocation FROM storage ORDER BY storageLocation");
        getLocationList.exec();

        //Put the results in a list
        QStringList locationList;
        while (getLocationList.next()) {
            locationList << getLocationList.value(0).toString();
        }

        //Prepare list for the Location combobox
        QStringList displayLocationList = locationList;
        //Add the "All" option at the beginning
        displayLocationList.insert(0,"All");

        //Send the list to the Search combobox model
        fileListModel = new QStringListModel(this);
        fileListModel->setStringList(displayLocationList);

    }
