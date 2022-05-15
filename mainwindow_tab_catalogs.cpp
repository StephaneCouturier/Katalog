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
// File Name:   mainwindow_tab_catalogs.cpp
// Purpose:     methods for the screen Catalogs
// Description: https://github.com/StephaneCouturier/Katalog/wiki/Catalogs
// Author:      Stephane Couturier
/////////////////////////////////////////////////////////////////////////////
*/

#include "mainwindow.h"
#include "ui_mainwindow.h"

#include "catalog.h"
#include "catalogsview.h"
#include "database.h"
#include "filesview.h"

#include <QTextStream>
#include <QDesktopServices>
#include <QFileDialog>
#include <QSortFilterProxyModel>

//UI----------------------------------------------------------------------------

    //Catalog operations
        void MainWindow::on_Catalogs_pushButton_Search_clicked()
        {
            //Change the selected catalog in Search tab
            ui->Filters_comboBox_SelectCatalog->setCurrentText(selectedCatalogName);

            //Go to the Search tab
            ui->tabWidget->setCurrentIndex(0); // tab 0 is the Search tab
        }
        //----------------------------------------------------------------------
        void MainWindow::on_Catalogs_pushButton_ViewCatalog_clicked()
        {
                //Start at the root folder of the catalog
                selectedDirectoryName     = selectedCatalogPath;
                selectedDirectoryFullPath = selectedCatalogPath;

                //Load
                openCatalogToExplore();

                //Go to explore tab
                ui->tabWidget->setCurrentIndex(2);
        }
        //----------------------------------------------------------------------
        void MainWindow::on_Catalogs_pushButton_Cancel_clicked()
        {
            ui->Catalogs_lineEdit_Name->setText(selectedCatalogName);
            ui->Catalogs_lineEdit_SourcePath->setText(selectedCatalogPath);
            ui->Catalogs_comboBox_FileType->setCurrentText(selectedCatalogFileType);
            ui->Catalogs_comboBox_Storage->setCurrentText(selectedCatalogStorage);
            ui->Catalogs_checkBox_IncludeHidden->setChecked(selectedCatalogIncludeHidden);

            ui->Catalogs_widget_EditCatalog->hide();

        }
        //----------------------------------------------------------------------
        void MainWindow::on_Catalogs_pushButton_UpdateCatalog_clicked()
        {   //Update the selected catalog
            updateSingleCatalog();
        }
        //----------------------------------------------------------------------
        void MainWindow::on_Catalogs_pushButton_UpdateAllActive_clicked()
        {
            //user to confirm running this global update

                int confirm = QMessageBox::warning(this, "Katalog",
                                    tr("Are you sure you want to update ALL catalogs that are currently filtered and identified as active?")
                                         , QMessageBox::Yes
                                                  | QMessageBox::Cancel);
                if ( confirm == QMessageBox::Cancel){
                    return;
                }

            //user to choose showing or skipping summary for each catalog update
                skipCatalogUpdateSummary = false;
                int result = QMessageBox::warning(this, "Katalog",
                                    tr("Do you want to view the summary of updates at the end of each catalog update?")
                                         , QMessageBox::Yes
                                                  | QMessageBox::No);
                if ( result == QMessageBox::No){
                    skipCatalogUpdateSummary= true;
                }

            //Get list of displayed and active catalogs
                //prepare the main part of the query
                QString querySQL  = QLatin1String(R"(
                                            SELECT catalogName, catalogSourcePathIsActive, catalogSourcePath, catalogStorage
                                            FROM catalog
                                            LEFT JOIN storage ON catalogStorage = storageName
                                            WHERE catalogSourcePathIsActive = 1
                                            )");

                    //add AND conditions for the selected filters
                    if ( selectedSearchLocation != tr("All") )
                        querySQL = querySQL + " AND storageLocation = '"+selectedSearchLocation+"' ";

                    if ( selectedSearchStorage != tr("All") )
                        querySQL = querySQL + " AND catalogStorage = '"+selectedSearchStorage+"' ";

                //run the query
                QSqlQuery query;
                query.prepare(querySQL);
                query.exec();

            // Catalog each result
            while(query.next()){
                //Get catalog name
                QString catalogName = query.value(0).toString();

                QDir dir (query.value(2).toString());
                if (dir.exists()==true){

                    ///Warning and choice if the result is 0 files
                    if(dir.entryInfoList(QDir::NoDotAndDotDot|QDir::AllEntries).count() == 0)
                    {
                        //QMessageBox::information(this,"Katalog","dir Empty." );

                        //return;
                    }
                    else{
                        //Update catalog
                        updateCatalog(catalogName);

                       //Update storage
                        selectedCatalogStorage = query.value(3).toString();

                        //Update the related storage
                        if ( selectedCatalogStorage != ""){
                           //get Storage ID
                           QSqlQuery query;
                           QString querySQL = QLatin1String(R"(
                                               SELECT storageID, storagePath FROM storage WHERE storageName =:storageName
                                                           )");
                           query.prepare(querySQL);
                           query.bindValue(":storageName",selectedCatalogStorage);
                           query.exec();
                           query.next();
                           int selectedCatalogStorageID = query.value(0).toInt();
                           QString selectedCatalogStoragePath =  query.value(1).toString();

                           //Update storage
                           if ( selectedCatalogStoragePath!="")
                               updateStorageInfo(selectedCatalogStorageID,selectedCatalogStoragePath);
                           else
                               QMessageBox::information(this,"Katalog",tr("The storage device name may not be correct:\n %1 ").arg(selectedCatalogStorage));

                       }

                       //refresh catalog lists
                          loadCatalogFilesToTable();
                          loadCatalogsToModel();
                    }
                }
            }
            QMessageBox::information(this,"Katalog",tr("Update of displayed and active catalogs completed."));
            skipCatalogUpdateSummary= false;

        }
        //----------------------------------------------------------------------
        void MainWindow::on_Catalogs_pushButton_EditCatalogFile_clicked()
        {
            //save catalogs
            int result = QMessageBox::warning(this, "Katalog",
                                tr("You are about to edit the catalog file directly.<br/><br/>"
"It generally recommended to Create a new catalog with the right initial settings (source path, file type, include Hidden Files, storage), rather than modify the catalog file directly.<br/><br/>"
"Check the Wiki page <a href='https://github.com/StephaneCouturier/Katalog/wiki/Catalogs#edit'>Catalogs/Edit</a> to understand the impact of changing this file directly.<br/><br/>"
"Do you want to continue anyway?")
                                     , QMessageBox::Yes
                                              | QMessageBox::Cancel);
            if ( result == QMessageBox::Cancel){
                return;
            }
            else
                ui->Catalogs_widget_EditCatalog->show();

        }
        //----------------------------------------------------------------------
        void MainWindow::on_Catalogs_pushButton_ViewCatalogStats_clicked()
        {
            ui->Statistics_comboBox_SelectSource->setCurrentText("Updates only");
            //ui->Statistics_comboBox_SelectCatalog->setCurrentText(selectedCatalogName);
            //Go to the Search tab
            ui->tabWidget->setCurrentIndex(5); // tab 0 is the Search tab
            //Select the type of display "selected catalog"
            ui->Statistics_comboBox_SelectSource->setCurrentText(tr("selected catalog"));
        }
        //----------------------------------------------------------------------
        void MainWindow::on_Catalogs_pushButton_Import_clicked()
        {
                importFromVVV();
        }
        //----------------------------------------------------------------------
        void MainWindow::on_Catalogs_pushButton_DeleteCatalog_clicked()
        {
            if ( selectedCatalogFile != ""){

                int result = QMessageBox::warning(this,"Katalog",
                          tr("Do you want to delete this catalog?")+"\n"+selectedCatalogFile,QMessageBox::Yes|QMessageBox::Cancel);

                if ( result ==QMessageBox::Yes){
                    QFile file (selectedCatalogFile);
                    file.moveToTrash();
                    loadCatalogsToModel();
                    refreshCatalogSelectionList("","");
                }
             }
            else QMessageBox::information(this,"Katalog",tr("Select a catalog above first."));

            //refresh catalog lists
               loadCatalogFilesToTable();
               loadCatalogsToModel();
        }
        //----------------------------------------------------------------------
        void MainWindow::on_Catalogs_pushButton_Open_clicked()
        {
            //save catalogs
            int result = QMessageBox::warning(this, "Katalog",
                                tr("You are about to edit the catalog file directly.<br/><br/>"
"It generally recommended to Create a new catalog with the right initial settings (source path, file type, include Hidden Files, storage), rather than modify the catalog file directly.<br/><br/>"
"Check the Wiki page <a href='https://github.com/StephaneCouturier/Katalog/wiki/Catalogs#edit'>Catalogs/Edit</a> to understand the impact of changing this file directly.<br/><br/>"
"Do you want to continue anyway?")
                                     , QMessageBox::Yes
                                              | QMessageBox::Cancel);
            if ( result == QMessageBox::Cancel){
                return;
            }
            else
                QDesktopServices::openUrl(QUrl::fromLocalFile(selectedCatalogFile));
        }
        //----------------------------------------------------------------------
        void MainWindow::on_Catalogs_pushButton_SelectPath_clicked()
        {
            //Get current selected path as default path for the dialog window
            newCatalogPath = ui->Catalogs_lineEdit_SourcePath->text();

            //Open a dialog for the user to select the directory to be cataloged. Only show directories.
            QString dir = QFileDialog::getExistingDirectory(this, tr("Select the directory to be cataloged in this new catalog"),
                                                            newCatalogPath,
                                                            QFileDialog::ShowDirsOnly
                                                            | QFileDialog::DontResolveSymlinks);
            //Send the selected directory to LE_NewCatalogPath (input line for the New Catalog Path)
            ui->Catalogs_lineEdit_SourcePath->setText(dir);

            //Select this directory in the treeview.
            loadFileSystem(newCatalogPath);
        }
        //----------------------------------------------------------------------
        void MainWindow::on_Catalogs_pushButton_Save_clicked()
        {
            saveCatalogChanges();
        }
        //----------------------------------------------------------------------

    //File methods
        //----------------------------------------------------------------------
        void MainWindow::on_Catalogs_treeView_CatalogList_clicked(const QModelIndex &index)
        {
            selectedCatalogName             = ui->Catalogs_treeView_CatalogList->model()->index(index.row(), 0, QModelIndex()).data().toString();
            selectedCatalogFile             = ui->Catalogs_treeView_CatalogList->model()->index(index.row(), 1, QModelIndex()).data().toString();
            selectedCatalogDateTime         = ui->Catalogs_treeView_CatalogList->model()->index(index.row(), 2, QModelIndex()).data().toString();
            selectedCatalogFileCount        = ui->Catalogs_treeView_CatalogList->model()->index(index.row(), 3, QModelIndex()).data().toLongLong();
            selectedCatalogTotalFileSize    = ui->Catalogs_treeView_CatalogList->model()->index(index.row(), 4, QModelIndex()).data().toLongLong();
            selectedCatalogPath             = ui->Catalogs_treeView_CatalogList->model()->index(index.row(), 5, QModelIndex()).data().toString();
            selectedCatalogFileType         = ui->Catalogs_treeView_CatalogList->model()->index(index.row(), 6, QModelIndex()).data().toString();
            selectedCatalogIncludeHidden    = ui->Catalogs_treeView_CatalogList->model()->index(index.row(), 8, QModelIndex()).data().toBool();
            selectedCatalogStorage          = ui->Catalogs_treeView_CatalogList->model()->index(index.row(), 9, QModelIndex()).data().toString();
            selectedCatalogIncludeSymblinks = ui->Catalogs_treeView_CatalogList->model()->index(index.row(),10, QModelIndex()).data().toBool();
            //DEV: selectedCatalogIsFullDevice     = ui->Catalogs_treeView_CatalogList->model()->index(index.row(),11, QModelIndex()).data().toBool();

            if (selectedCatalogFileType=="") selectedCatalogFileType = tr("All");

            // Display buttons
            ui->Catalogs_pushButton_Search->setEnabled(true);
            ui->Catalogs_pushButton_ViewCatalog->setEnabled(true);
            ui->Catalogs_pushButton_Open->setEnabled(true);
            ui->Catalogs_pushButton_EditCatalogFile->setEnabled(true);
            ui->Catalogs_pushButton_UpdateCatalog->setEnabled(true);
            ui->Catalogs_pushButton_ViewCatalogStats->setEnabled(true);
            ui->Catalogs_pushButton_DeleteCatalog->setEnabled(true);

            //Load catalog values to the Edit area
            ui->Catalogs_lineEdit_Name->setText(selectedCatalogName);
            ui->Catalogs_lineEdit_SourcePath->setText(selectedCatalogPath);
            ui->Catalogs_comboBox_FileType->setCurrentText(selectedCatalogFileType);
            ui->Catalogs_comboBox_Storage->setCurrentText(selectedCatalogStorage);
            ui->Catalogs_checkBox_IncludeHidden->setChecked(selectedCatalogIncludeHidden);
            //DEV: ui->Catalogs_checkBox_isFullDevice->setChecked(selectedCatalogIsFullDevice);

        }
        //----------------------------------------------------------------------
        void MainWindow::on_Catalogs_treeView_CatalogList_doubleClicked()
        {
            openCatalogToExplore();
            //Go to explore tab
            ui->tabWidget->setCurrentIndex(2);
        }
        //----------------------------------------------------------------------
        void MainWindow::on_Catalogs_pushButton_Snapshot_clicked()
        {
            recordCollectionStats();
        }
        //----------------------------------------------------------------------
        void MainWindow::on_Catalogs_treeView_CatalogList_HeaderSortOrderChanged(){

            QSettings settings(settingsFilePath, QSettings:: IniFormat);

            QHeaderView *catalogsTreeHeader = ui->Catalogs_treeView_CatalogList->header();

            lastCatalogsSortSection = catalogsTreeHeader->sortIndicatorSection();
            lastCatalogsSortOrder   = catalogsTreeHeader->sortIndicatorOrder();

            settings.setValue("Catalogs/lastCatlogsSortSection", QString::number(lastCatalogsSortSection));
            settings.setValue("Catalogs/lastCatlogsSortOrder",   QString::number(lastCatalogsSortOrder));
        }
        //----------------------------------------------------------------------
/*        void MainWindow::on_Catalogs_treeView_CatalogList_HeaderSizeChanged(int section){

//            QList<int> headerSectionsSize;

//            QSettings settings(settingsFilePath, QSettings:: IniFormat);

//            QHeaderView *catalogsTreeHeader = ui->Catalogs_treeView_CatalogList->header();

//            lastCatalogsHeaderSection = section;
//            lastCatalogsHeaderSize    = catalogsTreeHeader->sectionSize(0);

            //headerSectionsSize[lastCatalogsHeaderSection] = lastCatalogsHeaderSize;

//            settings.setValue("Catalogs/lastCatalogsHeaderSection", QString::number(lastCatalogsHeaderSection));
//            settings.setValue("Catalogs/lastCatalogsHeaderSize"   , QString::number(lastCatalogsHeaderSize));
            //settings.setValue("Catalogs/lastCatalogsHeaderSectionSizes"   , ui->Catalogs_treeView_CatalogList->header()->saveState());

//            settings.beginWriteArray("headerSectionSizes");
//            for (int i = 0; i < headerSectionsSize.size(); ++i) {
//                settings.setArrayIndex(i);
//                settings.setValue("lastCatalogsHeaderSection", i);
//                settings.setValue("lastCatalogsHeaderSize", headerSectionsSize[i]);
//            }
//            settings.endArray();



            //            lastCatalogsHeaderSection = 0;
            //            int currentSize = ui->Catalogs_treeView_CatalogList->header()->sectionSize(0);
            //            lastCatalogsHeaderSize = 200;
            //            QMessageBox::information(this,"Katalog","lastCatlogsSortSection: \n" + QVariant(lastCatalogsHeaderSection).toString()
            //                    +"\ncurrentSize: \n" + QVariant(currentSize).toString()
            //                    +"\nlastCatalogsHeaderSize: \n" + QVariant(lastCatalogsHeaderSize).toString());
//            ui->Catalogs_treeView_CatalogList->header()->resizeSection(lastCatalogsHeaderSection,lastCatalogsHeaderSize);

        }
*/

//Methods-----------------------------------------------------------------------

    //--------------------------------------------------------------------------
    void MainWindow::loadCatalogFilesToTable()
    {
        //Clear current entires of the catalog table
            QSqlQuery queryDelete;
            queryDelete.prepare( "DELETE FROM catalog" );
            queryDelete.exec();

        //Prepare table and insert query
            QSqlQuery query;
            if (!query.exec(SQL_CREATE_CATALOG)){
                QMessageBox::information(this,"Katalog","problem to create the table.");
                return;}

            if (!query.prepare(SQL_INSERT_CATALOG)){
                QMessageBox::information(this,"Katalog","problem to insert rows.");
                return;}

        //Iterate in the directory to create a list of files and sort it
            QStringList fileTypes;
            fileTypes << "*.idx";

            QDirIterator iterator(collectionFolder, fileTypes, QDir::Files, QDirIterator::Subdirectories);
            while (iterator.hasNext()){

                // Iterate to the next file
                QString path = iterator.next();
                QFile catalogFile(path);

                // Get file info
                QFileInfo catalogFileInfo(catalogFile);

                // Verify that the file can be opened
                if(!catalogFile.open(QIODevice::ReadOnly)) {
                    QMessageBox::information(this,"Katalog",tr("No catalog found."));
                    return;
                }

                //Prepare a textsteam for the file
                QTextStream textStream(&catalogFile);

                //Read the first 8 lines and put values in a stringlist
                QStringList catalogValues;
                QString line;
                QString value;
                for (int i=0; i<8; i++) {
                    line = textStream.readLine();
                    if (line.at(0)=="<"){
                        value = line.right(line.size() - line.lastIndexOf(">") - 1);
                        if (value =="") catalogValues << "";
                        else catalogValues << value;
                    }
                }
                if (catalogValues.count()==7) catalogValues << "false"; //for older catalog without isFullDevice

                // Verify if path is active (drive connected)
                int isActive = verifyCatalogPath(catalogValues[0]);

                //Insert a line in the table with available data
                QVariant catalogID = addCatalog(query,
                                catalogFileInfo.filePath(),  //catalogFilePath
                                catalogFileInfo.completeBaseName(),  //catalogName
                                catalogFileInfo.lastModified().toString("yyyy-MM-dd hh:mm:ss"), //catalogDateUpdated
                                catalogValues[0], //catalogSourcePath
                                catalogValues[1].toInt(), //catalogFileCount
                                catalogValues[2].toLongLong(), //catalogTotalFileSize
                                isActive,         //catalogSourcePathIsActive
                                catalogValues[3], //catalogIncludeHidden
                                catalogValues[4], //catalogFileType
                                catalogValues[5], //catalogStorage
                                catalogValues[6], //catalogIncludeSymblinks
                                catalogValues[7],  //catalogIsFullDevice
                                ""  //catalogLoadedVersion
                                );

                catalogFile.close();
            }

    }
    //--------------------------------------------------------------------------
    void MainWindow::loadCatalogsToModel()
    {
        //Generate SQL query from filters.
            QString loadCatalogSQL;

            //prepare the main part of the query
            loadCatalogSQL  = QLatin1String(R"(
                                        SELECT
                                            catalogName                 ,
                                            catalogFilePath             ,
                                            catalogDateUpdated          ,
                                            catalogFileCount            ,
                                            catalogTotalFileSize        ,
                                            catalogSourcePath           ,
                                            catalogFileType             ,
                                            catalogSourcePathIsActive   ,
                                            catalogIncludeHidden        ,
                                            catalogStorage              ,
                                            storageLocation             ,
                                            catalogIsFullDevice         ,
                                            catalogLoadedVersion
                                        FROM catalog
                                        LEFT JOIN storage ON catalogStorage = storageName
                                        WHERE catalogName !=''
                                        )");

                //add AND conditions for the selected filters
//                if ( selectedSearchLocation != tr("All") )
//                    loadCatalogSQL = loadCatalogSQL + " AND storageLocation = '"+selectedSearchLocation+"' ";

//                if ( selectedSearchStorage != tr("All") )
//                    loadCatalogSQL = loadCatalogSQL + " AND catalogStorage = '"+selectedSearchStorage+"' ";

            if ( selectedDeviceType == "Location" )
                loadCatalogSQL = loadCatalogSQL + " AND storage.storageLocation = '"+ selectedDeviceName +"' ";

            else if ( selectedDeviceType == "Storage" )
                loadCatalogSQL = loadCatalogSQL + " AND catalogStorage = '"+ selectedDeviceName +"' ";

            else if ( selectedDeviceType == "Catalog" )
                loadCatalogSQL = loadCatalogSQL + " AND catalogName = '"+ selectedDeviceName +"' ";


            //Execute query
            QSqlQuery loadCatalogQuery;
            loadCatalogQuery.prepare(loadCatalogSQL);
            loadCatalogQuery.exec();

            //Format and send to Treeview
            QSqlQueryModel *catalogQueryModel = new QSqlQueryModel;
            catalogQueryModel->setQuery(loadCatalogQuery);

            CatalogsView *proxyResultsModel = new CatalogsView(this);
            proxyResultsModel->setSourceModel(catalogQueryModel);

            proxyResultsModel->setHeaderData(0, Qt::Horizontal, tr("Name"));
            proxyResultsModel->setHeaderData(2, Qt::Horizontal, tr("Last Update"));
            proxyResultsModel->setHeaderData(3, Qt::Horizontal, tr("Files"));
            proxyResultsModel->setHeaderData(4, Qt::Horizontal, tr("Total Size"));
            proxyResultsModel->setHeaderData(5, Qt::Horizontal, tr("Source Path"));
            proxyResultsModel->setHeaderData(6, Qt::Horizontal, tr("File Type"));
            proxyResultsModel->setHeaderData(7, Qt::Horizontal, tr("Active"));
            proxyResultsModel->setHeaderData(8, Qt::Horizontal, tr("Inc.Hidden"));
            proxyResultsModel->setHeaderData(9, Qt::Horizontal, tr("Storage"));
            proxyResultsModel->setHeaderData(10,Qt::Horizontal, tr("Location"));
            proxyResultsModel->setHeaderData(11,Qt::Horizontal, tr("Full Device"));
            proxyResultsModel->setHeaderData(12,Qt::Horizontal, tr("Loaded Version"));

            //Connect model to tree/table view
            ui->Catalogs_treeView_CatalogList->setModel(proxyResultsModel);
            //ui->Catalogs_treeView_CatalogList->QTreeView::sortByColumn(1,Qt::AscendingOrder);
            ui->Catalogs_treeView_CatalogList->header()->setSectionResizeMode(QHeaderView::Interactive);

            //Hide column with file path
            ui->Catalogs_treeView_CatalogList->header()->hideSection(1); //Path

            //Change columns size
            ui->Catalogs_treeView_CatalogList->header()->resizeSection(0, 450); //Name
            ui->Catalogs_treeView_CatalogList->header()->resizeSection(2, 150); //Date
            ui->Catalogs_treeView_CatalogList->header()->resizeSection(3,  80); //Files
            ui->Catalogs_treeView_CatalogList->header()->resizeSection(4, 100); //TotalFileSize
            ui->Catalogs_treeView_CatalogList->header()->resizeSection(5, 300); //Path
            ui->Catalogs_treeView_CatalogList->header()->resizeSection(6, 100); //FileType
            ui->Catalogs_treeView_CatalogList->header()->resizeSection(7,  50); //Active
            ui->Catalogs_treeView_CatalogList->header()->resizeSection(8,  50); //include
            ui->Catalogs_treeView_CatalogList->header()->resizeSection(9, 150); //Storage
            ui->Catalogs_treeView_CatalogList->header()->resizeSection(10,150); //Location
            ui->Catalogs_treeView_CatalogList->header()->resizeSection(11, 50); //FullDevice
            ui->Catalogs_treeView_CatalogList->header()->resizeSection(12,150); //Loaded Version

            //Hide columns
            ui->Catalogs_treeView_CatalogList->hideColumn(11); //FullDevice
            ui->Catalogs_treeView_CatalogList->hideColumn(12); //Loaded Version

            //Populate catalogs statistics
            QSqlQuery query;
            QString querySQL = QLatin1String(R"(
                                SELECT COUNT(catalogName),SUM(catalogTotalFileSize),SUM(catalogFileCount)
                                FROM catalog
                                LEFT JOIN storage ON catalogStorage = storageName
                                WHERE catalogName !=''
                                            )");

            if ( selectedSearchLocation !=tr("All")){
                querySQL = querySQL + " AND storageLocation =:storageLocation";
            }
            if ( selectedSearchStorage !=tr("All")){
                querySQL = querySQL + " AND catalogStorage =:catalogStorage";
            }

            query.prepare(querySQL);
            query.bindValue(":storageLocation",selectedSearchLocation);
            query.bindValue(":catalogStorage", selectedSearchStorage);
            query.exec();
            query.next();

            ui->Catalogs_label_Catalogs->setText(QString::number(query.value(0).toInt()));
            ui->Catalogs_label_TotalSize->setText(QLocale().formattedDataSize(query.value(1).toLongLong()));
            ui->Catalogs_label_TotalNumber->setText(QLocale().toString(query.value(2).toInt()));

    }
    //--------------------------------------------------------------------------
    int MainWindow::verifyCatalogPath(QString catalogSourcePath)
    {
        // Verify that the catalog path is accessible (so the related drive is mounted), returns true/false

        QDir dir(catalogSourcePath);
        int status = dir.exists();
        return status;
    }
    //--------------------------------------------------------------------------
    void MainWindow::recordSelectedCatalogStats(QString selectedCatalogName, int selectedCatalogFileCount, qint64 selectedCatalogTotalFileSize)
    {
        QDateTime nowDateTime = QDateTime::currentDateTime();

        QString statisticsLine = nowDateTime.toString("yyyy-MM-dd hh:mm:ss") + "\t"
                                + selectedCatalogName + "\t"
                                + QString::number(selectedCatalogFileCount) + "\t"
                                + QString::number(selectedCatalogTotalFileSize) + "\t"
                                + "Update" ;

        // Stream the list to the file
        QFile fileOut( collectionFolder + "/" + statisticsFileName );

        // Write data
        if (fileOut.open(QFile::WriteOnly | QIODevice::Append | QFile::Text)) {
            QTextStream stream(&fileOut);
            stream << statisticsLine << "\n";
         }
         fileOut.close();
    }

    //--------------------------------------------------------------------------
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
    //--------------------------------------------------------------------------
    void MainWindow::updateSingleCatalog()
    {
        updateCatalog(selectedCatalogName);

        //Update the related storage
        if ( selectedCatalogStorage != ""){
            //get Storage ID
            QSqlQuery query;
            QString querySQL = QLatin1String(R"(
                                SELECT storageID, storagePath FROM storage WHERE storageName =:storageName
                                            )");
            query.prepare(querySQL);
            query.bindValue(":storageName",selectedCatalogStorage);
            query.exec();
            query.next();
            int selectedCatalogStorageID = query.value(0).toInt();
            QString selectedCatalogStoragePath =  query.value(1).toString();

            //Update storage
            if ( selectedCatalogStoragePath!="")
                updateStorageInfo(selectedCatalogStorageID,selectedCatalogStoragePath);
            else
                QMessageBox::information(this,"Katalog",tr("The storage device name may not be correct:\n %1 ").arg(selectedCatalogStorage));

        }

        //refresh catalog lists
           loadCatalogFilesToTable();
           loadCatalogsToModel();
    }
    //--------------------------------------------------------------------------
    void MainWindow::updateCatalog(QString catalogName)
    {
        //Get data about the catalog from the database

            QString updateCatalogSQL  = QLatin1String(R"(
                                        SELECT  catalogFilePath,
                                                catalogName,
                                                catalogSourcePath,
                                                catalogFileCount,
                                                catalogIncludeHidden,
                                                catalogFileType,
                                                catalogTotalFileSize,
                                                catalogStorage,
                                                catalogIncludeSymblinks,
                                                catalogIsFullDevice
                                        FROM catalog
                                        WHERE catalogName =:catalogName
                                        )");

            QSqlQuery query;
            query.prepare(updateCatalogSQL);
            query.bindValue(":catalogName", catalogName);
            query.exec();
            query.next();

            QString currentCatalogFilePath       = query.value(0).toString();   //selectedCatalogFile
            QString currentCatalogName           = query.value(1).toString();   //selectedCatalogName
            QString currentCatalogSourcePath     = query.value(2).toString();   //selectedCatalogPath
            qint64  currentCatalogFileCount      = query.value(3).toLongLong(); //selectedCatalogFileCount
            bool currentCatalogIncludeHidden     = query.value(4).toBool();    //selectedCatalogIncludeHidden
            QString currentCatalogFileType       = query.value(5).toString();   //selectedCatalogFileType
            qint64  currentCatalogTotalFileSize  = query.value(6).toLongLong(); //selectedCatalogTotalFileSize
            QString currentCatalogStorage        = query.value(7).toString();   //catalogStorage
            bool currentCatalogIncludeSymblinks  = query.value(8).toBool();     //catalogIncludeSymblinks
            bool isFullDevice                    = query.value(9).toBool();     //catalogIsFullDevice

        //Check if the update can be done, inform the user otherwise.
            //Deal with old versions, where necessary info may have not have been available
            if(currentCatalogFilePath == "not recorded" or currentCatalogName == "not recorded" or currentCatalogSourcePath == "not recorded"){
            QMessageBox::information(this,"Katalog",tr("It seems this catalog was not correctly imported or has an old format.\n"
                                         "Edit it and make sure it has the following first 2 lines:\n\n"
                                         "<catalogSourcePath>/folderpath\n"
                                         "<catalogFileCount>10000\n\n"
                                         "Copy/paste these lines at the begining of the file and modify the values after the >:\n"
                                         "- the catalogSourcePath is the folder to catalog the files from.\n"
                                         "- the catalogFileCount number does not matter as much, it can be updated.\n")
                                     );
            return;
            }

            //Deal with other cases where some input information is missing
            if(currentCatalogFilePath == "" or currentCatalogName == "" or currentCatalogSourcePath == ""){
            QMessageBox::information(this,"Katalog",tr("Select a catalog first (some info is missing).\n currentCatalogFilePath: %1 \n currentCatalogName: %2 \n currentCatalogSourcePath: %3").arg(
                                     currentCatalogFilePath, currentCatalogName, currentCatalogSourcePath));
            return;
            }

        //BackUp the file before is the option is selected
            if ( ui->Settings_checkBox_KeepOneBackUp->isChecked() == true){
                backupCatalog(currentCatalogFilePath);
            }


        //Capture previous FileCount and TotalFileSize to be able to report the changes after the update
            qint64 previousFileCount = currentCatalogFileCount; //currentCatalogFileCount;
            qint64 previousTotalFileSize = currentCatalogTotalFileSize;//currentCatalogTotalFileSize;

        //Define the type of files to be included
            QStringList fileTypes;
            if      ( currentCatalogFileType == "Image")
                                    fileTypes = fileType_Image;
            else if ( currentCatalogFileType == "Audio")
                                    fileTypes = fileType_Audio;
            else if ( currentCatalogFileType == "Video")
                                    fileTypes = fileType_Video;
            else if ( currentCatalogFileType == "Text")
                                    fileTypes = fileType_Text;
            else                    fileTypes.clear();

        //
        QDir dir (currentCatalogSourcePath);
        if (dir.exists()==true){
            ///Warning and choice if the result is 0 files
            if(dir.entryInfoList(QDir::NoDotAndDotDot|QDir::AllEntries).count() == 0)
            {
                int result = QMessageBox::warning(this, "Katalog - Warning",
                                    tr("The source folder does not contain any file.\n"
                                         "This could mean that the source is empty or the device is not mounted to this folder.\n")
                                         +tr("Do you want to save it anyway (the catalog would be empty)?\n"), QMessageBox::Yes
                                                  | QMessageBox::Cancel);
                if ( result == QMessageBox::Cancel){
                    return;
                }
            }

            //catalog the directory
            catalogDirectory(currentCatalogSourcePath,
                             currentCatalogIncludeHidden,
                             currentCatalogFileType,
                             fileTypes,
                             currentCatalogStorage,
                             currentCatalogIncludeSymblinks,
                             isFullDevice);

            saveCatalogToNewFile(currentCatalogName);


            //reload new data
            QString querySQL = QLatin1String(R"(
                            SELECT  catalogFilePath,
                                    catalogName,
                                    catalogSourcePath,
                                    catalogFileCount,
                                    catalogFileType,
                                    catalogTotalFileSize
                            FROM catalog
                            WHERE catalogName =:catalogName
                            )");
            query.prepare(querySQL);
            query.bindValue(":catalogName", catalogName);
            query.exec();
            query.next();
            //currentCatalogFilePath      = query.value(0).toString();   //selectedCatalogFile
            //currentCatalogName          = query.value(1).toString();   //selectedCatalogName
            //currentCatalogSourcePath    = query.value(2).toString();   //selectedCatalogPath
            currentCatalogFileCount       = query.value(3).toLongLong(); //selectedCatalogFileCount
            currentCatalogFileCount       = selectedCatalogFileCount; //retrieved from catalog method
            //currentCatalogFileType      = query.value(4).toString();   //selectedCatalogFileType
            currentCatalogTotalFileSize   = query.value(5).toLongLong(); //selectedCatalogTotalFileSize
            currentCatalogTotalFileSize   = selectedCatalogTotalFileSize; //retrieved from catalog method

            //Prepare to report changes to the catalog
            qint64 deltaFileCount = currentCatalogFileCount - previousFileCount;
            qint64 deltaTotalFileSize = currentCatalogTotalFileSize - previousTotalFileSize;

            //Inform user about the update
            if(skipCatalogUpdateSummary !=true){
            QMessageBox::information(this,"Katalog",tr("<br/>This catalog was updated:<br/><b> %1 </b> <br/>"
                                     "<table> <tr><td>Number of files: </td><td><b> %2 </b></td><td>  (added: <b> %3 </b>)</td></tr>"
                                     "<tr><td>Total file size: </td><td><b> %4 </b>  </td><td>  (added: <b> %5 </b>)</td></tr></table>"
                                     ).arg(currentCatalogName,
                                           QString::number(currentCatalogFileCount),
                                           QString::number(deltaFileCount),
                                           QLocale().formattedDataSize(currentCatalogTotalFileSize),
                                           QLocale().formattedDataSize(deltaTotalFileSize))
                                     ,Qt::TextFormat(Qt::RichText));
            }

        }
        else {
            QMessageBox::information(this,"Katalog",tr("The catalog %1 cannot be updated.\n"
                                            "\n The source folder - %2 - was not found.\n"
                                            "\n Possible reasons:\n"
                                            "    - the device is not connected and mounted,\n"
                                            "    - the source folder was moved or renamed.")
                                            .arg(currentCatalogName,
                                                   currentCatalogSourcePath)
                                     );
        }

        if ( ui->Settings_checkBox_SaveRecordWhenUpdate->isChecked() == true )
            recordSelectedCatalogStats(currentCatalogName, currentCatalogFileCount, currentCatalogTotalFileSize);

        //Refresh the collection view
        loadCatalogsToModel();
        //Reload stats file
        loadStatisticsChart();

    }
    //--------------------------------------------------------------------------
    void MainWindow::hideCatalogButtons()
    {
        //Hide buttons
        ui->Catalogs_pushButton_Search->setEnabled(false);
        ui->Catalogs_pushButton_ViewCatalog->setEnabled(false);
        ui->Catalogs_pushButton_EditCatalogFile->setEnabled(false);
        ui->Catalogs_pushButton_UpdateCatalog->setEnabled(false);
        ui->Catalogs_pushButton_ViewCatalogStats->setEnabled(false);
        ui->Catalogs_pushButton_DeleteCatalog->setEnabled(false);
    }
    //--------------------------------------------------------------------------
    void MainWindow::importFromVVV()
    {
        //Select file

            //Get user home path
            QStringList standardsPaths = QStandardPaths::standardLocations(QStandardPaths::HomeLocation);
            QString homePath = standardsPaths[0];

            //Get path of the file to import
            QString sourceFilePath = QFileDialog::getOpenFileName(this, tr("Select the csv file to be imported"),homePath);

            //Stop if no path is selected
            if ( sourceFilePath=="" ) return;

            //Define file
            QFile sourceFile(sourceFilePath);

        //Open the source file and load all data into the database

            // Start animation while cataloging
            QApplication::setOverrideCursor(Qt::WaitCursor);

            //clear database
                QSqlQuery deleteQuery;
                deleteQuery.exec("DELETE FROM file");

            //prepare query to load file info
                QSqlQuery insertQuery;
                QString insertSQL = QLatin1String(R"(
                                    INSERT INTO file (
                                                    fileName,
                                                    filePath,
                                                    fileSize,
                                                    fileDateUpdated,
                                                    fileCatalog )
                                    VALUES(
                                                    :fileName,
                                                    :filePath,
                                                    :fileSize,
                                                    :fileDateUpdated,
                                                    :fileCatalog )
                                                )");
                insertQuery.prepare(insertSQL);

            //prepare file and stream

                if(!sourceFile.open(QIODevice::ReadOnly)) {
                    QMessageBox::information(this,"Katalog",tr("No catalog found."));
                    return;
                }

                QTextStream textStream(&sourceFile);
                QString     line;

            //Process and check Headers line
                line = textStream.readLine();

                //Split the line into a fieldlist
                QStringList headerList = line.split('\t');

                //Check this is the right source format
                if (line.left(6)!="Volume"){
                    QMessageBox::warning(this,"Kotation",tr("A file was found, but could not be loaded") +".\n");
                    return;
                }
                //else {
                    //int headerFieldNumber = headerList.length();
                    //QMessageBox::information(this,"Kotation","ok to import. \n number of fields: \n " + QString::number(headerFieldNumber));
                //}

            //load all files to the database

                    while (true)
                    {
                        //Read the newt line
                        line = textStream.readLine();

                        if (line !=""){
                            QStringList fieldList = line.split("\t");
                            if ( fieldList.count()==7 ){

                                //Append data to the database
                                insertQuery.bindValue(":fileName", fieldList[2].replace("\"",""));
                                insertQuery.bindValue(":filePath", fieldList[1].replace("\"",""));
                                insertQuery.bindValue(":fileSize", fieldList[3].toLongLong());
                                insertQuery.bindValue(":fileDateUpdated", fieldList[5]);
                                insertQuery.bindValue(":fileCatalog", fieldList[0].replace("\"",""));
                                insertQuery.exec();
                            }
                        }
                       else
                            break;


                }

            //close source file
            sourceFile.close();

        //Stream the list of files out to the target calog file(s)

            //Get a list of the source catalogs
                QString listCatalogSQL = QLatin1String(R"(
                                    SELECT DISTINCT fileCatalog
                                    FROM file
                                                )");
                QSqlQuery listCatalogQuery;
                listCatalogQuery.prepare(listCatalogSQL);
                listCatalogQuery.exec();

            //Iterate in the list to generate a catalog file for each catalog
            while (listCatalogQuery.next()){

                //Get catalog name
                    QString formerCatalogName = listCatalogQuery.value(0).toString();

                //Generate a name for the file itself, without specical characters
                    newCatalogName = formerCatalogName;
                    newCatalogName.replace("/","_");
                    newCatalogName.replace("\\","_");

                //Prepare the catalog file path
                    QFile fileOut( collectionFolder +"/"+ newCatalogName + ".idx" );

                //Get statistics of the files for the list
                    QString listCatalogSQL = QLatin1String(R"(
                                        SELECT COUNT(*), SUM(fileSize)
                                        FROM file
                                        WHERE fileCatalog =:fileCatalog
                                                    )");
                    QSqlQuery listCatalogQuery;
                    listCatalogQuery.prepare(listCatalogSQL);
                    listCatalogQuery.bindValue(":fileCatalog",formerCatalogName);
                    listCatalogQuery.exec();
                    listCatalogQuery.next();

                    qint64 totalFiles = listCatalogQuery.value(0).toLongLong();
                    qint64 totalSize  = listCatalogQuery.value(1).toLongLong();

                //Prepare the stream and file headers
                    QTextStream out(&fileOut);
                    if(fileOut.open(QIODevice::WriteOnly | QIODevice::Truncate)) {

                        //append catalog definition("<catalogName>"+newCatalogName);
                        out  << "<catalogSourcePath>imported"  << "\n"
                             << "<catalogFileCount>"+QString::number(totalFiles)           << "\n"
                             << "<catalogTotalFileSize>"+QString::number(totalSize)       << "\n"
                             << "<catalogIncludeHidden>"       << "\n"
                             << "<catalogFileType>"            << "\n"
                             << "<catalogStorage>"             << "\n"
                             << "<catalogIncludeSymblinks>"    << "\n"
                             << "<catalogIsFullDevice>"     << "\n";
                    }

                //Get the list of file to add
                QString listFilesSQL = QLatin1String(R"(
                                    SELECT *
                                    FROM file
                                    WHERE fileCatalog =:fileCatalog
                                                )");
                QSqlQuery listFilesQuery;
                listFilesQuery.prepare(listFilesSQL);
                listFilesQuery.bindValue(":fileCatalog",formerCatalogName);
                listFilesQuery.exec();

                //Write the results in the file
                while (listFilesQuery.next()) {

                        out << listFilesQuery.value(1).toString() + "/" + listFilesQuery.value(0).toString();
                        out << '\t';
                        out << listFilesQuery.value(2).toString();
                        out << '\t';
                        out << listFilesQuery.value(3).toString();
                        out << '\n';
                }
            }

            //Stop animation
            QApplication::restoreOverrideCursor();

        loadCollection();

    }
    //--------------------------------------------------------------------------
    void MainWindow::saveCatalogChanges()
    {

        //Get new values
            //newCatalogSourcePath: remove the / at the end if any
            QString newCatalogSourcePath = ui->Catalogs_lineEdit_SourcePath->text();
            int pathLength = newCatalogSourcePath.length();
            if (newCatalogSourcePath.at(pathLength-1)=="/") {
                newCatalogSourcePath.remove(pathLength-1,1);
            }

            bool newCatalogIncludeHidden = ui->Catalogs_checkBox_IncludeHidden->checkState();
            QString newCatalogFileType   = ui->Catalogs_comboBox_FileType->currentText();
            QString newCatalogStorage    = ui->Catalogs_comboBox_Storage->currentText();
            bool isFullDevice            = ui->Catalogs_checkBox_isFullDevice->checkState();
            //QString newIncludeSymblinks  = ui->Catalogs_checkBox_IncludeSymblinks->currentText();

            //save catalogs
            int result = QMessageBox::warning(this, "Katalog",
                                tr("Save changes to the definition of the catalog?\n")
                                     +tr("(The catalog must be updated to reflect these changes)"), QMessageBox::Yes
                                              | QMessageBox::Cancel);
            if ( result == QMessageBox::Cancel){
                return;
            }

        //Write changes to catalog file
            // open file
            QFile file(selectedCatalogFile);
            if(file.open(QIODevice::ReadWrite | QIODevice::Text))
            {
                QString fullFileText;
                QTextStream textStream(&file);
//                int lineNumber = 0;
                while(!textStream.atEnd())
                {
                    QString line = textStream.readLine();
                    //lineNumber = lineNumber + 1;
//                    bool addedIsFullDevice = false;

                    //add file data line
                    if(!line.startsWith("<catalogSourcePath")
                            and !line.startsWith("<catalogIncludeHidden")
                            and !line.startsWith("<catalogFileType")
                            and !line.startsWith("<catalogStorage")
                            and !line.startsWith("<catalogIsFullDevice"))
                        fullFileText.append(line + "\n");
                    else{
                        //add meta-data. The ifs must be in the correct order of the meta-data lines
                        if(line.startsWith("<catalogSourcePath>"))
                                fullFileText.append("<catalogSourcePath>" + newCatalogSourcePath +"\n");

                        if(line.startsWith("<catalogIncludeHidden>"))
                                fullFileText.append("<catalogIncludeHidden>" + QVariant(newCatalogIncludeHidden).toString() +"\n");

                        if(line.startsWith("<catalogFileType>"))
                                fullFileText.append("<catalogFileType>" + newCatalogFileType +"\n");

                        if(line.startsWith("<catalogStorage>"))
                                fullFileText.append("<catalogStorage>" + newCatalogStorage +"\n");

                        if(line.startsWith("<catalogIsFullDevice>")){
                                fullFileText.append("<catalogIsFullDevice>" + QVariant(isFullDevice).toString() +"\n");
//                                addedIsFullDevice = true;
                        }

                    }
//                    if(addedIsFullDevice ==false){
//                        //add missing line
//                        fullFileText.prepend("<catalogIsFullDevice>" + QVariant(isFullDevice).toString() +"\n");
//                    }
                }
                file.resize(0);
                textStream << fullFileText;
                file.close();
            }
            else {
                QMessageBox::information(this,"Katalog",tr("Could not open file."));
            }

        //Rename catalog
            //Get info from the selected catalog
            QFileInfo selectedCatalogFileInfo(selectedCatalogFile);
            QString currentCatalogName = selectedCatalogFileInfo.baseName();
            QString newCatalogName = ui->Catalogs_lineEdit_Name->text();


            //Rename the catalog file
            if (newCatalogName != currentCatalogName){

                //generate the full new name of the
                QString newCatalogFullName = selectedCatalogFileInfo.absolutePath() + "/" + newCatalogName + ".idx";

                QFile::rename(selectedCatalogFile, newCatalogFullName);

                 //refresh catalog lists
                    loadCatalogFilesToTable();
                    loadCatalogsToModel();
                    //LoadCatalogFileList();
                    refreshCatalogSelectionList("","");

                //Rename in statistics

                    int renameChoice = QMessageBox::warning(this, "Katalog", tr("Apply the change in the statistics file?\n")
                                             , QMessageBox::Yes | QMessageBox::No);

                    if (renameChoice == QMessageBox::Yes){
                        QFile f(statisticsFilePath);
                        if(f.open(QIODevice::ReadWrite | QIODevice::Text))
                        {
                            QString s;
                            QTextStream t(&f);
                            while(!t.atEnd())
                            {
                                QString line = t.readLine();
                                QStringList lineParts = line.split("\t");
                                if (lineParts[1]==currentCatalogName){
                                    lineParts[1]= newCatalogName;
                                    line = lineParts.join("\t");
                                }
                                s.append(line + "\n");
                            }
                            f.resize(0);
                            t << s;
                            f.close();
                        }
                    }
            }

        //Refresh data
            loadCatalogFilesToTable();

        //Launch update if catalog is active and changes impact the contents (path, type, hidden)
            if (       newCatalogSourcePath    != selectedCatalogPath
                    or newCatalogIncludeHidden != selectedCatalogIncludeHidden
                    or newCatalogFileType      != selectedCatalogFileType){

                int updatechoice = QMessageBox::warning(this, "Katalog",
                                    tr("Update the catalog content with the new criteria?\n")
                                         , QMessageBox::Yes
                                                  | QMessageBox::Cancel);
                if ( updatechoice == QMessageBox::Yes){
                    //DEV: reselct the catalog to get the right values in the next function
                    // or improve the function to get it from db
                    //loadCatalogFilesToTable();
                    updateCatalog(newCatalogName);
                    //QMessageBox::information(this,"Katalog",tr("Updated."));
                }
            }

        ui->Catalogs_widget_EditCatalog->hide();

        loadCollection();
    }
//--------------------------------------------------------------------------
void MainWindow::recordCollectionStats(){
        //recordSelectedCatalogStats(selectedCatalogName, selectedCatalogFileCount, selectedCatalogTotalFileSize);

        //Get last total file size and number
        qint64 lastTotalFileSize;
        qint64 lastTotalFileNumber;

        //get the list of catalogs and data
            QSqlQuery queryLast;
            QString queryLastSQL = QLatin1String(R"(
                                        SELECT SUM(catalogTotalFileSize), SUM(catalogFileCount)
                                        FROM statistics
                                        WHERE dateTime = (SELECT MAX(dateTime)
                                                            FROM statistics
                                                            WHERE recordType="Snapshot")
                                        GROUP BY dateTime
                                        )");
            queryLast.prepare(queryLastSQL);
            queryLast.exec();
            queryLast.next();
            lastTotalFileSize   = queryLast.value(0).toLongLong();
            lastTotalFileNumber = queryLast.value(1).toLongLong();

        //record all current catalog value
        recordAllCatalogStats();


        //Get new total file size and number
        qint64 newTotalFileSize;
        qint64 newTotalFileNumber;

        //get the list of catalogs and data
            QSqlQuery queryNew;
            QString queryNewSQL = QLatin1String(R"(
                                        SELECT SUM(catalogTotalFileSize), SUM(catalogFileCount)
                                        FROM statistics
                                        WHERE dateTime = (SELECT MAX(dateTime)
                                                            FROM statistics
                                                            WHERE recordType="Snapshot")
                                        GROUP BY dateTime
                                        )");
            queryNew.prepare(queryNewSQL);
            queryNew.exec();
            queryNew.next();
            newTotalFileSize   = queryNew.value(0).toLongLong();
            newTotalFileNumber = queryNew.value(1).toLongLong();

        //Calculate and inform
        qint64 deltaTotalFileSize = newTotalFileSize - lastTotalFileSize;
        qint64 deltaTotalFileNumber = newTotalFileNumber - lastTotalFileNumber;

        QMessageBox::information(this,"Katalog",tr("<br/>A snapshot of this collection was recorded:<br/><br/>"
                                 "<table> <tr><td>Number of files: </td><td><b> %1 </b></td><td>  (added: <b> %2 </b>)</td></tr>"
                                 "<tr><td>Total file size: </td><td><b> %3 </b>  </td><td>  (added: <b> %4 </b>)</td></tr></table>"
                                 ).arg(QString::number(newTotalFileNumber),
                                       QString::number(deltaTotalFileNumber),
                                       QLocale().formattedDataSize(newTotalFileSize),
                                       QLocale().formattedDataSize(deltaTotalFileSize)
                                       )
                                 ,Qt::TextFormat(Qt::RichText));
}
//--------------------------------------------------------------------------
void MainWindow::recordAllCatalogStats()
{
    // Save the values (size and number of files) of all catalogs to the statistics file, creating a snapshop of the collection.

    QDateTime nowDateTime = QDateTime::currentDateTime();
    QString nowDateTimeFormatted = nowDateTime.toString("yyyy-MM-dd hh:mm:ss");
    //QMessageBox::information(this,"Katalog","Ok." + nowDateTimeFormatted);

    //get the list of catalogs and data
        QSqlQuery query;
        QString querySQL = QLatin1String(R"(
                                    SELECT
                                        catalogName                 ,
                                        catalogFileCount            ,
                                        catalogTotalFileSize
                                    FROM catalog
                                    )");
        query.prepare(querySQL);
        query.exec();


    //create a new record for each catalog
        QString recordCatalogName;
        QString recordCatalogFileCount;
        QString recordCatalogTotalFileSize;

        while (query.next()){
                recordCatalogName = query.value(0).toString();
                recordCatalogFileCount = query.value(1).toString();
                recordCatalogTotalFileSize = query.value(2).toString();

                QString statisticsLine = nowDateTimeFormatted + "\t"
                                        + recordCatalogName + "\t"
                                        + recordCatalogFileCount + "\t"
                                        + recordCatalogTotalFileSize + "\t"
                                        + "Snapshot" ;

                //QMessageBox::information(this,"Katalog","Ok." + statisticsLine);

                // Stream the values to the file
                    QFile fileOut( collectionFolder + "/" + statisticsFileName );

                    // Write data
                    if (fileOut.open(QFile::WriteOnly | QIODevice::Append | QFile::Text)) {
                        QTextStream stream(&fileOut);
                        stream << statisticsLine << "\n";
                     }
                     fileOut.close();

        }

        //refresh graphs
        loadStatisticsData();
        loadStatisticsChart();

}
