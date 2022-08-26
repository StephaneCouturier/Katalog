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
// File Name:   mainwindow_tab_storage.cpp
// Purpose:     methods for the screen Storage
// Description: https://github.com/StephaneCouturier/Katalog/wiki/Storage
// Author:      Stephane Couturier
/////////////////////////////////////////////////////////////////////////////
*/

#include "mainwindow.h"
#include "ui_mainwindow.h"
#include "database.h"

#include "storageview.h"
#include "storagetreemodel.h"
#include "directorytreemodel.h"

#include <QDesktopServices>

//UI----------------------------------------------------------------------------
    //Full list ----------------------------------------------------------------
    void MainWindow::on_Storage_pushButton_CreateList_clicked()
    {
        createStorageList();
    }
    //--------------------------------------------------------------------------
    void MainWindow::on_Storage_pushButton_Reload_clicked()
    {
        loadStorageFileToTable();
        loadStorageTableToModel();
        updateStorageSelectionStatistics();
    }
    //--------------------------------------------------------------------------
    void MainWindow::on_Storage_pushButton_EditAll_clicked()
    {
        QDesktopServices::openUrl(QUrl::fromLocalFile(storageFilePath));
    }
    //--------------------------------------------------------------------------
    void MainWindow::on_Storage_pushButton_SaveAll_clicked()
    {
        saveStorageData();
        refreshLocationSelectionList();
    }
    //--------------------------------------------------------------------------
    void MainWindow::on_Storage_treeView_StorageList_clicked(const QModelIndex &index)
    {
        selectedStorageID       = ui->Storage_treeView_StorageList->model()->index(index.row(), 0, QModelIndex()).data().toInt();
        selectedStorageName     = ui->Storage_treeView_StorageList->model()->index(index.row(), 1, QModelIndex()).data().toString();
        selectedStorageLocation = ui->Storage_treeView_StorageList->model()->index(index.row(), 3, QModelIndex()).data().toString();
        selectedStoragePath     = ui->Storage_treeView_StorageList->model()->index(index.row(), 4, QModelIndex()).data().toString();

        //display buttons
        ui->Storage_pushButton_SearchStorage->setEnabled(true);
        ui->Storage_pushButton_SearchLocation->setEnabled(true);
        ui->Storage_pushButton_CreateCatalog->setEnabled(true);
        ui->Storage_pushButton_Update->setEnabled(true);
        ui->Storage_pushButton_Delete->setEnabled(true);
        ui->Storage_pushButton_OpenFilelight->setEnabled(true);

        selectedStorageIndexRow = index.row();
    }

    //With seleted storage -----------------------------------------------------
    void MainWindow::on_Storage_pushButton_New_clicked()
    {
        addStorageDevice(tr("Storage"));
    }
    //--------------------------------------------------------------------------
    void MainWindow::on_Storage_pushButton_SearchStorage_clicked()
    {
        //Change tab to show the Search screen
        ui->tabWidget->setCurrentIndex(0); // tab 0 is the Search tab

        ui->Filters_label_DisplayStorage->setText(selectedStorageName);
    }
    //--------------------------------------------------------------------------
    void MainWindow::on_Storage_pushButton_SearchLocation_clicked()
    {
        //Change tab to show the Search screen
        ui->tabWidget->setCurrentIndex(0); // tab 0 is the Search tab

        ui->Filters_label_DisplayLocation->setText(selectedStorageLocation);
    }
    //--------------------------------------------------------------------------
    void MainWindow::on_Storage_pushButton_CreateCatalog_clicked()
    {
        //Send the selected directory to LE_NewCatalogPath (input line for the New Catalog Path)
        ui->Create_lineEdit_NewCatalogPath->setText(selectedStoragePath);
        ui->Create_comboBox_StorageSelection->setCurrentText(selectedStorageName);
        ui->Create_lineEdit_NewCatalogName->setText(selectedStorageName);

        //Select this directory in the treeview.
        loadFileSystem(selectedStoragePath);

        //Change tab to show the result of the catalog creation
        ui->tabWidget->setCurrentIndex(3); // tab 3 is the Create catalog tab

    }
    //--------------------------------------------------------------------------
    void MainWindow::on_Storage_pushButton_OpenFilelight_clicked()
    {
        #ifdef Q_OS_LINUX
                QProcess::startDetached("filelight", QStringList() << selectedStoragePath);
        #else
                QProcess::startDetached("filelight", QStringList() << selectedStoragePath);
        #endif
    }
    //--------------------------------------------------------------------------
    void MainWindow::on_Storage_pushButton_Update_clicked()
    {
        updateStorageInfo(selectedStorageID, selectedStoragePath);
    }
    //--------------------------------------------------------------------------
    void MainWindow::on_Storage_pushButton_Delete_clicked()
    {
        int result = QMessageBox::warning(this,"Katalog",
                   tr("Do you want to <span style='color: red';>delete</span> this Storage device?"
                   "<table>"
                   "<tr><td>ID:   </td><td><b> %1 </td></tr>"
                   "<tr><td>Name: </td><td><b> %2 </td></tr>"
                   "</table>").arg(QString::number(selectedStorageID),selectedStorageName)
                  ,QMessageBox::Yes|QMessageBox::Cancel);

        if ( result ==QMessageBox::Yes){

            //Delete from the table
            QSqlQuery queryDeviceNumber;
            queryDeviceNumber.prepare( "DELETE FROM storage WHERE storageID = " + QString::number(selectedStorageID) );
            queryDeviceNumber.exec();

            //Reload data to model
            loadStorageTableToModel();

            //Save model data to file
            saveStorageData();

            //Refresh storage statistics
            updateStorageSelectionStatistics();
        }
    }
    //----------------------------------------------------------------------
    void MainWindow::on_StorageTreeViewStorageListHeaderSortOrderChanged(){

        QSettings settings(settingsFilePath, QSettings:: IniFormat);
        QHeaderView *storageTreeHeader = ui->Storage_treeView_StorageList->header();

        lastStorageSortSection = storageTreeHeader->sortIndicatorSection();
        lastStorageSortOrder   = storageTreeHeader->sortIndicatorOrder();

        settings.setValue("Storage/lastStorageSortSection", QString::number(lastStorageSortSection));
        settings.setValue("Storage/lastStorageSortOrder",   QString::number(lastStorageSortOrder));
    }

//Methods-----------------------------------------------------------------------

    void MainWindow::createStorageList()
    {
        // Create it, if it does not exist
        QFile newStorageFile(storageFilePath);
        if(!newStorageFile.open(QIODevice::ReadOnly)) {

            if (newStorageFile.open(QFile::WriteOnly | QFile::Text)) {

                  QTextStream stream(&newStorageFile);

                  stream << "ID"            << "\t"
                         << "Name"          << "\t"
                         << "Type"          << "\t"
                         << "Location"      << "\t"
                         << "Path"          << "\t"
                         << "Label"         << "\t"
                         << "FileSystem"    << "\t"
                         << "Total"         << "\t"
                         << "Free"          << "\t"
                         << "BrandModel"    << "\t"
                         << "SerialNumber"  << "\t"
                         << "BuildDate"     << "\t"
                         << "ContentType"   << "\t"
                         << "Container"     << "\t"
                         << "Comment"       << "\t"
                         << '\n';

                  newStorageFile.close();

                  ui->Storage_pushButton_Reload->setEnabled(true);
                  ui->Storage_pushButton_EditAll->setEnabled(true);
                  //ui->Storage_pushButton_SaveAll->setEnabled(true);

//                  QMessageBox::information(this,"Katalog",tr("A storage file was created")+":\n" + newStorageFile.fileName()
//                                           + "\n"+tr("You can edit it now")+".");

                  //Disable button so it cannot be overwritten
                  ui->Storage_pushButton_CreateList->setEnabled(false);
                  ui->Storage_pushButton_SaveAll->setEnabled(true);

                  //Even if empty, load it to the model
                  loadStorageFileToTable();
                  loadStorageTableToModel();
                  updateStorageSelectionStatistics();
                  addStorageDevice("");

            return;
            }
        }

    }
    //--------------------------------------------------------------------------
    void MainWindow::addStorageDevice(QString deviceName)
    {
        //Get inputs
            //max ID
            QSqlQuery queryDeviceNumber;
            queryDeviceNumber.prepare( "SELECT MAX (storageID) FROM storage" );
            queryDeviceNumber.exec();
            queryDeviceNumber.next();
            int maxID = queryDeviceNumber.value(0).toInt();
            int newID = maxID + 1;

            //location
            QString newLocation;
            if(selectedStorageLocation != tr("All")){
                newLocation = selectedStorageLocation;
            }
            else
                newLocation = "";

        //Insert new device with default values
        QString querySQL = QLatin1String(R"(
            insert into storage(
                            storageID,
                            storageName,
                            storageType,
                            storageLocation,
                            storagePath,
                            storageLabel,
                            storageFileSystem,
                            storageTotalSpace,
                            storageFreeSpace,
                            storageBrandModel,
                            storageSerialNumber,
                            storageBuildDate,
                            storageContentType,
                            storageContainer,
                            storageComment)
                      values(
                            :newID,
                            :storageName,
                            "",
                            :newLocation,
                            "",
                            "",
                            "",
                            0,
                            0,
                            "",
                            "",
                            "",
                            "",
                            "",
                            "")
                    )");

        QSqlQuery insertQuery;
        insertQuery.prepare(querySQL);
        insertQuery.bindValue(":newID",newID);
        insertQuery.bindValue(":newLocation",deviceName);
        insertQuery.bindValue(":newLocation",newLocation);
        insertQuery.exec();

        //load table to model
        loadStorageTableToModel();
        saveStorageData();

        //enable save button
        ui->Storage_pushButton_New->setEnabled(true);

        //Refresh Location list
        refreshLocationSelectionList();

    }
    //--------------------------------------------------------------------------
    void MainWindow::loadStorageFileToTable()
    {
        //Define storage file and prepare stream
        QFile storageFile(storageFilePath);
        QTextStream textStream(&storageFile);

        QSqlQuery queryDelete;
        queryDelete.prepare( "DELETE FROM storage" );

        //Open file or return information
        if(!storageFile.open(QIODevice::ReadOnly)) {
            //if there is no storage file, reset data and buttons
            //QMessageBox::information(this,"Katalog","No storage file was found in the current collection folder."
            //                             "\nPlease create one with the button 'Create list'\n");

            queryDelete.exec();

            //Disable all buttons, enable create list
            ui->Storage_pushButton_Reload->setEnabled(false);
            ui->Storage_pushButton_EditAll->setEnabled(false);
            ui->Storage_pushButton_SaveAll->setEnabled(false);
            ui->Storage_pushButton_New->setEnabled(false);
            ui->Storage_pushButton_CreateList->setEnabled(true);

            return;
        }

        //test file validity (application breaks between v0.13 and v0.14)
        QString line = textStream.readLine();
        if (line.left(2)!="ID"){
               QMessageBox::warning(this,"Katalog",
                                    tr("A storage.csv file was found, but could not be loaded.\n"
                                    "Likely, it was made with an older version of Katalog.\n"
                                    "The file can be fixed manually, please visit the wiki page:\n"
                                    "<a href='https://github.com/StephaneCouturier/Katalog/wiki/Storage#fixing-for-new-versions'>Storage/fixing-for-new-versions</a>")
                                    );
               return;
        }

        //Clear all entries of the current table
        queryDelete.exec();


        //Prepare query
        QSqlQuery query;
        if (!query.exec(SQL_CREATE_STORAGE)){
            QMessageBox::information(this,"Katalog","pb1.");
            return;}

        //Load storage device lines to table
        while (true)
        {

            QString line = textStream.readLine();
            if (line.isNull())
                break;
            else
                if (line.left(2)!="ID"){//skip the first line with headers

                    //Split the string with tabulation into a list
                    QStringList fieldList = line.split('\t');

                    QString querySQL = QLatin1String(R"(
                        insert into storage(
                                        storageID,
                                        storageName,
                                        storageType,
                                        storageLocation,
                                        storagePath,
                                        storageLabel,
                                        storageFileSystem,
                                        storageTotalSpace,
                                        storageFreeSpace,
                                        storageBrandModel,
                                        storageSerialNumber,
                                        storageBuildDate,
                                        storageContentType,
                                        storageContainer,
                                        storageComment)
                                  values(
                                        :storageID,
                                        :storageName,
                                        :storageType,
                                        :storageLocation,
                                        :storagePath,
                                        :storageLabel,
                                        :storageFileSystem,
                                        :storageTotalSpace,
                                        :storageFreeSpace,
                                        :storageBrandModel,
                                        :storageSerialNumber,
                                        :storageBuildDate,
                                        :storageContentType,
                                        :storageContainer,
                                        :storageComment)
                                )");

                    QSqlQuery insertQuery;
                    insertQuery.prepare(querySQL);
                    insertQuery.bindValue(":storageID",fieldList[0].toInt());
                    insertQuery.bindValue(":storageName",fieldList[1]);
                    insertQuery.bindValue(":storageType",fieldList[2]);
                    insertQuery.bindValue(":storageLocation",fieldList[3]);
                    insertQuery.bindValue(":storagePath",fieldList[4]);
                    insertQuery.bindValue(":storageLabel",fieldList[5]);
                    insertQuery.bindValue(":storageFileSystem",fieldList[6]);
                    insertQuery.bindValue(":storageTotalSpace",fieldList[7].toLongLong());
                    insertQuery.bindValue(":storageFreeSpace",fieldList[8].toLongLong());
                    insertQuery.bindValue(":storageBrandModel",fieldList[9]);
                    insertQuery.bindValue(":storageSerialNumber",fieldList[10]);
                    insertQuery.bindValue(":storageBuildDate",fieldList[11]);
                    insertQuery.bindValue(":storageContentType",fieldList[12]);
                    insertQuery.bindValue(":storageContainer",fieldList[13]);
                    insertQuery.bindValue(":storageComment", fieldList[14]);

                    insertQuery.exec();

                }
        }
        storageFile.close();

        //Enable buttons
        ui->Storage_pushButton_Reload->setEnabled(true);
        ui->Storage_pushButton_EditAll->setEnabled(true);
        //ui->Storage_pushButton_SaveAll->setEnabled(true);
        ui->Storage_pushButton_New->setEnabled(true);

        //Disable create button so it cannot be overwritten
        ui->Storage_pushButton_CreateList->setEnabled(false);

    }
    //--------------------------------------------------------------------------
    void MainWindow::loadStorageTableToModel()
    {
        storageModel->setTable("storage");

        if ( selectedDeviceType == "Location" ){
            QString tableFilter = "storageLocation = '" + selectedDeviceName + "'";
            storageModel->setFilter(tableFilter);
        }
        else if ( selectedDeviceType == "Storage" ){
            QString tableFilter = "storageName = '" + selectedDeviceName + "'";
            storageModel->setFilter(tableFilter);
        }
        else if ( selectedDeviceType == "Catalog" ){
            QString tableFilter = "storageName = '" + activeCatalog->storageName + "'";
            storageModel->setFilter(tableFilter);
        }
        storageModel->setSort(1, Qt::AscendingOrder);

        // Set the localized header captions:
        storageModel->setHeaderData(0, Qt::Horizontal, tr("ID"));
        storageModel->setHeaderData(1, Qt::Horizontal, tr("Name"));
        storageModel->setHeaderData(2, Qt::Horizontal, tr("Type"));
        storageModel->setHeaderData(3, Qt::Horizontal, tr("Location"));
        storageModel->setHeaderData(4, Qt::Horizontal, tr("Path"));
        storageModel->setHeaderData(5, Qt::Horizontal, tr("Label"));
        storageModel->setHeaderData(6, Qt::Horizontal, tr("FileSystem"));
        storageModel->setHeaderData(7, Qt::Horizontal, tr("Total"));
        storageModel->setHeaderData(8, Qt::Horizontal, tr("Free"));
        storageModel->setHeaderData(9, Qt::Horizontal, tr("Brand/Model"));
        storageModel->setHeaderData(10, Qt::Horizontal, tr("Serial Number"));
        storageModel->setHeaderData(11, Qt::Horizontal, tr("Build Date"));
        storageModel->setHeaderData(12, Qt::Horizontal, tr("Content Type"));
        storageModel->setHeaderData(13, Qt::Horizontal, tr("Container"));
        storageModel->setHeaderData(14, Qt::Horizontal, tr("Comment"));

        // Populate the storageModel:
        if (!storageModel->select()) {
            //showError(model->lastError());
            return;
        }

        StorageView *proxyStorageModel2 = new StorageView(this);
        proxyStorageModel2->setSourceModel(storageModel);//storageModel  loadCatalogQueryModel

        ui->Storage_treeView_StorageList->setModel(proxyStorageModel2);

        // Connect model to tree/table view
        ui->Storage_treeView_StorageList->QTreeView::sortByColumn(1,Qt::AscendingOrder);
        ui->Storage_treeView_StorageList->header()->setSectionResizeMode(QHeaderView::Interactive);
        ui->Storage_treeView_StorageList->header()->resizeSection(0,  50); //ID
        ui->Storage_treeView_StorageList->header()->resizeSection(1, 225); //Name
        ui->Storage_treeView_StorageList->header()->resizeSection(2, 125); //Type
        ui->Storage_treeView_StorageList->header()->resizeSection(3, 150); //Location
        ui->Storage_treeView_StorageList->header()->resizeSection(4, 250); //Path
        ui->Storage_treeView_StorageList->header()->resizeSection(5, 125); //Label
        ui->Storage_treeView_StorageList->header()->resizeSection(6,  75); //FS
        ui->Storage_treeView_StorageList->header()->resizeSection(7,  85); //Total
        ui->Storage_treeView_StorageList->header()->resizeSection(8,  85); //Free
        ui->Storage_treeView_StorageList->header()->resizeSection(9, 150); //Brand
        ui->Storage_treeView_StorageList->header()->resizeSection(10,150); //Serial
        ui->Storage_treeView_StorageList->header()->resizeSection(11, 75); //Build date
        ui->Storage_treeView_StorageList->header()->resizeSection(12, 75); //Content
        ui->Storage_treeView_StorageList->header()->resizeSection(13,125); //Container
        ui->Storage_treeView_StorageList->header()->resizeSection(14, 50); //Comment
        //ui->Storage_treeView_StorageList->header()->hideSection(1); //Path


        //Get the list of device names for the Create screen
        QSqlQuery query;
        QString querySQL = QLatin1String(R"(
                           SELECT storageName
                           FROM storage
                           WHERE storageName !=''
                                        )");  // ORDER BY storageName

        if ( selectedStorageLocation != tr("All") ){
            //AND storageLocation ='DK/Portable'  :storageLocation
            //QMessageBox::information(this,"Katalog","Ok.");

            querySQL = querySQL + " AND storageLocation ='" + selectedStorageLocation + "'";
        }
    //        if ( selectedSearchStorage != "All" )
    //            querySQL = querySQL + " AND catalogStorage = '"+selectedSearchStorage+"' ";

        querySQL = querySQL + " ORDER BY storageName ";
        query.bindValue("storageLocation", selectedStorageLocation);
        query.prepare(querySQL);
        query.exec();
        storageNameList.clear();
            while(query.next())
            {
                storageNameList<<query.value(0).toString();
            }
        loadStorageList();
    }

    //--------------------------------------------------------------------------
    void MainWindow::updateStorageInfo(int storageID, QString storagePath)
    {
        //Get current values for comparison later
            QString getStorageInfoSQL = QLatin1String(R"(
                                        SELECT  storageID,
                                                storageName,
                                                storageLocation,
                                                storageFreeSpace,
                                                storageTotalSpace
                                        FROM storage
                                        WHERE storageID =:storageID
                                        )");

            QSqlQuery getStorageInfoQuery;
            getStorageInfoQuery.prepare(getStorageInfoSQL);
            getStorageInfoQuery.bindValue(":storageID", storageID);
            getStorageInfoQuery.exec();
            getStorageInfoQuery.next();

            qint64 previousStorageFreeSpace  = getStorageInfoQuery.value(3).toLongLong();
            qint64 previousStorageTotalSpace = getStorageInfoQuery.value(4).toLongLong();
            qint64 previousStorageUsedSpace  = previousStorageTotalSpace - previousStorageFreeSpace;
            selectedStorageName = getStorageInfoQuery.value(1).toString();

        //verify if path is available / not empty
        QDir dir (storagePath);

            //Warning if no Path is provided
            if ( storagePath=="" ){
                QMessageBox::warning(this,tr("No path provided"),tr("No Path was provided. \n"
                                              "Modify the device to provide one and try again.\n")
                                              );
                return;
            }

            ///Warning and choice if the result is 0 files
            if(dir.entryInfoList(QDir::NoDotAndDotDot|QDir::AllEntries).count() == 0)
            {
                int result = QMessageBox::warning(this,tr("Directory is empty"),tr("The source folder does not contain any file.\n"
                                              "This could mean that the source is empty or the device is not mounted to this folder.\n")
                                              +tr("Katalog is going try to get values anyhow."));
                //return;
                if ( result == QMessageBox::Cancel){
                    return;
                }
            }

        //Get device information
            QStorageInfo storage;
            storage.setPath(storagePath);
            if (storage.isReadOnly())
                qDebug() << "isReadOnly:" << storage.isReadOnly();

            qint64 sizeTotal = storage.bytesTotal();
            qint64 sizeAvailable = storage.bytesAvailable();
            QString storageName = storage.name();
            QString storageFS = storage.fileSystemType();

        //get confirmation for the update
            if (sizeTotal == -1 ){
                QMessageBox::warning(this,tr("Katalog"),tr("Katalog could not get values. <br/> Check the source folder, or that the device is mounted to the source folder."));
                return;
            }

        //SQL updates
            QSqlQuery queryTotalSpace;
            queryTotalSpace.prepare( "UPDATE storage "
                                     "SET storageTotalSpace = " + QString::number(sizeTotal) + ","
                                      "storageFreeSpace = " + QString::number(sizeAvailable) + ","
                                      "storageLabel = '" + storageName +"',"
                                      "storageFileSystem = '" + storageFS +"'"
                                  + " WHERE storageID = " + QString::number(storageID) );
            queryTotalSpace.exec();

        //Add values to statistics
            QDateTime nowDateTime = QDateTime::currentDateTime();

            QString statisticsLine = nowDateTime.toString("yyyy-MM-dd hh:mm:ss") + "\t"
                                    + selectedStorageName + "\t"
                                    + QString::number(sizeAvailable) + "\t"
                                    + QString::number(sizeTotal) + "\t"
                                    + "Storage" ;

            // Stream the list to the file
            QFile fileOut( collectionFolder + "/" + statisticsFileName );

            // Write data
            if (fileOut.open(QFile::WriteOnly | QIODevice::Append | QFile::Text)) {
                QTextStream stream(&fileOut);
                stream << statisticsLine << "\n";
             }
             fileOut.close();

        //Reload new data
             QSqlQuery query;
             QString querySQL = QLatin1String(R"(
                             SELECT  storageFreeSpace, storageTotalSpace
                             FROM storage
                             WHERE storageID =:storageID
                             )");
             query.prepare(querySQL);
             query.bindValue(":storageID", storageID);
             query.exec();
             query.next();


        //Prepare to report changes to the storage
             qint64 newStorageFreeSpace    = query.value(0).toLongLong();
             qint64 deltaStorageFreeSpace  = newStorageFreeSpace - previousStorageFreeSpace;
             qint64 newStorageTotalSpace   = query.value(1).toLongLong();
             qint64 deltaStorageTotalSpace = newStorageTotalSpace - previousStorageTotalSpace;
             qint64 newStorageUsedSpace    = newStorageTotalSpace - newStorageFreeSpace;
             qint64 deltaStorageUsedSpace  = newStorageUsedSpace - previousStorageUsedSpace;

             //Inform user about the update
             if(skipCatalogUpdateSummary !=true){
             QMessageBox::information(this,"Katalog",tr("<br/>The storage device <b> %1 </b> was updated:<br/> "
                                      "<table>"
                                              "<tr><td> Used Space: </td><td><b> %2 </b></td><td>  (added: <b> %3 </b>)</td></tr>"
                                              "<tr><td> Free Space: </td><td><b> %4 </b></td><td>  (added: <b> %5 </b>)</td></tr>"
                                              "<tr><td>Total Space: </td><td><b> %6 </b></td><td>  (added: <b> %7 </b>)</td></tr>"
                                      "</table>"
                                      ).arg(selectedStorageName,
                                            QLocale().formattedDataSize(newStorageUsedSpace),
                                            QLocale().formattedDataSize(deltaStorageUsedSpace),
                                            QLocale().formattedDataSize(newStorageFreeSpace),
                                            QLocale().formattedDataSize(deltaStorageFreeSpace),
                                            QLocale().formattedDataSize(newStorageTotalSpace),
                                            QLocale().formattedDataSize(deltaStorageTotalSpace),
                                            selectedStorageName)
                                      ,Qt::TextFormat(Qt::RichText));
             }

        //reload data to model
        loadStorageTableToModel();

        //save model data to file
        saveStorageData();

        //refresh storage statistics
        updateStorageSelectionStatistics();

    }
    //--------------------------------------------------------------------------
    void MainWindow::saveStorageData()
    {
        //Save model data to Storage file
        saveStorageModelToFile();

        //load Storage file data to table
        loadStorageFileToTable();

        //load table to model
        loadStorageTableToModel();

        //refresh stats
        updateStorageSelectionStatistics();

        //refresh selection tree
        loadStorageTableToSelectionTreeModel();

    }
    //--------------------------------------------------------------------------
    void MainWindow::saveStorageModelToFile()
    {
        //Prepare export file name
        //Define storage file
        storageFilePath = collectionFolder + "/" + "storage.csv";

        QFile storageFile(storageFilePath);

        //QFile exportFile(collectionFolder+"/file.txt");
    //    QString textData;
    //    int rows = storageModel->rowCount();
    //    int columns = storageModel->columnCount();

        QTextStream out(&storageFile);
        //DEV ADD HEADER LINE
        out  << "ID"            << "\t"
             << "Name"          << "\t"
             << "Type"          << "\t"
             << "Location"      << "\t"
             << "Path"          << "\t"
             << "Label"         << "\t"
             << "FileSystem"    << "\t"
             << "Total"         << "\t"
             << "Free"          << "\t"
             << "BrandModel"    << "\t"
             << "SerialNumber"  << "\t"
             << "BuildDate"     << "\t"
             << "ContentType"   << "\t"
             << "Container"     << "\t"
             << "Comment"       << "\t"
             << '\n';

        //save from model
    //    for (int i = 0; i < rows; i++) {
    //        for (int j = 0; j < columns; j++) {
    //                textData += storageModel->data(storageModel->index(i,j)).toString();
    //                textData += "\t";      // for .csv file format
    //        }
    //        textData += "\n";             // (optional: for new line segmentation)
    //    }

        //save from query
    //    Open your output file using QFile
    //    Run a select * query on your database table
        QSqlQuery query;
        QString querySQL = QLatin1String(R"(
                             SELECT * FROM storage
                                        )");
        query.prepare(querySQL);
        query.exec();

        //    Iterate the result
        //    -- Make a QStringList containing the output of each field
        QStringList fieldList;
        while (query.next()) {

            const QSqlRecord record = query.record();
            for (int i=0, recCount = record.count() ; i<recCount ; ++i){
                if (i>0)
                out << '\t';
                out << record.value(i).toString();
            }
     //    -- Write the result in the file
             //out << line;
             out << '\n';

        }

        if(storageFile.open(QIODevice::WriteOnly | QIODevice::Truncate)) {

            //out << textData;
    //    Close the file
            //storageFile.close();
        }

        //QMessageBox::information(this,"Katalog","Results exported to the collection folder:\n"+storageFile.fileName());
        storageFile.close();
    }
    //--------------------------------------------------------------------------
    void MainWindow::updateStorageSelectionStatistics()
    {
        //Get storage statistics
        QSqlQuery query;
        QString querySQL = QLatin1String(R"(
                            SELECT  COUNT (storageID),
                                    SUM(storageFreeSpace),
                                    SUM(storageTotalSpace)
                            FROM storage
                            WHERE storageName !=''
                                        )");

        if ( selectedStorageLocation !=tr("All")){
            querySQL = querySQL + " AND storageLocation =:storageLocation";
        }

        query.prepare(querySQL);
        query.bindValue(":storageLocation", selectedStorageLocation);
        query.exec();
        query.next();

        //Get the number of devices
        int deviceNumber = query.value(0).toInt();
        ui->Storage_label_CountValue->setText(QString::number(deviceNumber));
        //Get the sum of free space
        qint64 freeSpaceTotal = query.value(1).toLongLong();
        ui->Storage_label_SpaceFreeValue->setText(QLocale().formattedDataSize(freeSpaceTotal));
        //Get the sum of total space
        qint64 totalSpace = query.value(2).toLongLong();
        ui->Storage_label_SpaceTotalValue->setText(QLocale().formattedDataSize(totalSpace));
        //Calculate used space
        qint64 usedSpace = totalSpace - freeSpaceTotal;
        ui->Storage_label_SpaceUsedValue->setText(QLocale().formattedDataSize(usedSpace));

        //Get the percent of free space
        if ( totalSpace !=0){
        float freepercent = (float)freeSpaceTotal / (float)totalSpace * 100;
        ui->Storage_label_PercentFree->setText(QString::number(round(freepercent))+"%");}
        else ui->Storage_label_PercentFree->setText("");
    }
    //--------------------------------------------------------------------------
