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
#include "storageview.h"
#include "storage.h"

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

        //refresh
        loadStorageTableToModel();
        updateStorageSelectionStatistics();
        loadStorageTableToSelectionTreeModel();
        refreshLocationSelectionList();
        unsavedChanges = false;
        ui->Storage_pushButton_SaveAll->setStyleSheet("color: black");

    }
    //--------------------------------------------------------------------------
    void MainWindow::on_Storage_treeView_StorageList_clicked(const QModelIndex &index)
    {
        selectedStorage->setID(ui->Storage_treeView_StorageList->model()->index(index.row(), 0, QModelIndex()).data().toInt());
        selectedStorage->loadStorageMetaData();

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

        ui->Filters_label_DisplayStorage->setText(selectedStorage->name);
    }
    //--------------------------------------------------------------------------
    void MainWindow::on_Storage_pushButton_SearchLocation_clicked()
    {
        //Change tab to show the Search screen
        ui->tabWidget->setCurrentIndex(0); // tab 0 is the Search tab

        ui->Filters_label_DisplayLocation->setText(selectedStorage->location);
    }
    //--------------------------------------------------------------------------
    void MainWindow::on_Storage_pushButton_CreateCatalog_clicked()
    {
        //Send selection to Create screen
        ui->Create_lineEdit_NewCatalogPath->setText(selectedStorage->path);
        ui->Create_comboBox_StorageSelection->setCurrentText(selectedStorage->name);
        ui->Create_lineEdit_NewCatalogName->setText(selectedStorage->name);

        //Select this directory in the treeview.
        loadFileSystem(selectedStorage->path);

        //Change tab to show the result of the catalog creation
        ui->tabWidget->setCurrentIndex(3); // tab 3 is the Create catalog tab

    }
    //--------------------------------------------------------------------------
    void MainWindow::on_Storage_pushButton_OpenFilelight_clicked()
    {
        QProcess::startDetached("filelight", QStringList() << selectedStorage->path);
    }
    //--------------------------------------------------------------------------
    void MainWindow::on_Storage_pushButton_Update_clicked()
    {
        skipCatalogUpdateSummary =false;
        updateStorageInfo(selectedStorage);
    }
    //--------------------------------------------------------------------------
    void MainWindow::on_Storage_pushButton_Delete_clicked()
    {
        int result = QMessageBox::warning(this,"Katalog",
                   tr("Do you want to <span style='color: red';>delete</span> this Storage device?"
                   "<table>"
                   "<tr><td>ID:   </td><td><b> %1 </td></tr>"
                   "<tr><td>Name: </td><td><b> %2 </td></tr>"
                   "</table>").arg(QString::number(selectedStorage->ID),selectedStorage->name)
                  ,QMessageBox::Yes|QMessageBox::Cancel);

        if ( result ==QMessageBox::Yes){

            //Delete from the table
            QSqlQuery queryDeviceNumber;
            QString queryDeviceNumberSQL = QLatin1String(R"(
                                                DELETE FROM storage
                                                WHERE storage_id = :storage_id
                                            )");
            queryDeviceNumber.prepare(queryDeviceNumberSQL);
            queryDeviceNumber.bindValue(":storage_id",selectedStorage->ID);
            queryDeviceNumber.exec();

            //Reload data to model
            loadStorageTableToModel();

            //Save model data to file
            saveStorageData();

            //refresh
            loadStorageTableToModel();
            updateStorageSelectionStatistics();
            loadStorageTableToSelectionTreeModel();

            //Disable buttons to force new selection
            ui->Storage_pushButton_SearchLocation->setEnabled(false);
            ui->Storage_pushButton_SearchStorage->setEnabled(false);
            ui->Storage_pushButton_CreateCatalog->setEnabled(false);
            ui->Storage_pushButton_OpenFilelight->setEnabled(false);
            ui->Storage_pushButton_Delete->setEnabled(false);

            //Refresh storage screen statistics
            updateStorageSelectionStatistics();
        }
    }
    //--------------------------------------------------------------------------
    void MainWindow::on_StorageTreeViewStorageListHeaderSortOrderChanged(){

        QSettings settings(settingsFilePath, QSettings:: IniFormat);
        QHeaderView *storageTreeHeader = ui->Storage_treeView_StorageList->header();

        lastStorageSortSection = storageTreeHeader->sortIndicatorSection();
        lastStorageSortOrder   = storageTreeHeader->sortIndicatorOrder();

        settings.setValue("Storage/lastStorageSortSection", lastStorageSortSection);
        settings.setValue("Storage/lastStorageSortOrder",   lastStorageSortOrder);
    }
    //--------------------------------------------------------------------------
    void MainWindow::on_Storage_treeView_StorageList_doubleClicked(const QModelIndex &index)
    {
        unsavedChanges = true;
        ui->Storage_pushButton_SaveAll->setStyleSheet("color: orange");
    }
    //--------------------------------------------------------------------------

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

                  //Enable,Disable buttons
                  ui->Storage_pushButton_Reload->setEnabled(true);
                  ui->Storage_pushButton_EditAll->setEnabled(true);

                  ui->Storage_pushButton_CreateList->setEnabled(false);
                  ui->Storage_pushButton_SaveAll->setEnabled(true);

                  //Even if empty, load it to the model
                  loadStorageFileToTable();
                  loadStorageTableToModel();
                  updateStorageSelectionStatistics();

            return;
            }
        }

    }
    //--------------------------------------------------------------------------
    void MainWindow::addStorageDevice(QString deviceName)
    {
        //Get inputs
            //Generate ID
            QSqlQuery queryDeviceNumber;
            QString queryDeviceNumberSQL = QLatin1String(R"(
                                    SELECT MAX (storage_id)
                                    FROM storage
                                )");
            queryDeviceNumber.prepare(queryDeviceNumberSQL);
            queryDeviceNumber.exec();
            queryDeviceNumber.next();
            int maxID = queryDeviceNumber.value(0).toInt();
            int newID = maxID + 1;

            //Generate Location based on current selection
            QString newLocation;
            if(selectedDeviceType == "Location"){
                newLocation = selectedDeviceName;
            }
            else if(selectedStorage->location != tr("All")){
                newLocation = selectedStorage->location;
            }
            else
                newLocation = "";

        //Insert new device with default values
        QString querySQL = QLatin1String(R"(
            INSERT INTO storage(
                            storage_id,
                            storage_name,
                            storage_type,
                            storage_location,
                            storage_path,
                            storage_label,
                            storage_file_system,
                            storage_total_space,
                            storage_free_space,
                            storage_brand_model,
                            storage_serial_number,
                            storage_build_date,
                            storage_content_type,
                            storage_container,
                            storage_comment)
                      VALUES(
                            :new_id,
                            :storage_name,
                            "",
                            :new_location,
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
        insertQuery.bindValue(":new_id",newID);
        insertQuery.bindValue(":storage_name",deviceName+"_"+QString::number(newID));
        if(deviceName=="")
            insertQuery.bindValue(":storage_name","");

        insertQuery.bindValue(":new_location",newLocation);
        insertQuery.exec();

        //load table to model
        loadStorageTableToModel();
        saveStorageData();

        //refresh
        loadStorageTableToModel();
        updateStorageSelectionStatistics();
        loadStorageTableToSelectionTreeModel();

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
                        INSERT INTO storage(
                                        storage_id,
                                        storage_name,
                                        storage_type,
                                        storage_location,
                                        storage_path,
                                        storage_label,
                                        storage_file_system,
                                        storage_total_space,
                                        storage_free_space,
                                        storage_brand_model,
                                        storage_serial_number,
                                        storage_build_date,
                                        storage_content_type,
                                        storage_container,
                                        storage_comment)
                                  values(
                                        :storage_id,
                                        :storage_name,
                                        :storage_type,
                                        :storage_location,
                                        :storage_path,
                                        :storage_label,
                                        :storage_file_system,
                                        :storage_total_space,
                                        :storage_free_space,
                                        :storage_brand_model,
                                        :storage_serial_number,
                                        :storage_build_date,
                                        :storage_content_type,
                                        :storage_container,
                                        :storage_comment)
                                )");

                    QSqlQuery insertQuery;
                    insertQuery.prepare(querySQL);
                    insertQuery.bindValue(":storage_id",fieldList[0].toInt());
                    insertQuery.bindValue(":storage_name",fieldList[1]);
                    insertQuery.bindValue(":storage_type",fieldList[2]);
                    insertQuery.bindValue(":storage_location",fieldList[3]);
                    insertQuery.bindValue(":storage_path",fieldList[4]);
                    insertQuery.bindValue(":storage_label",fieldList[5]);
                    insertQuery.bindValue(":storage_file_system",fieldList[6]);
                    insertQuery.bindValue(":storage_total_space",fieldList[7].toLongLong());
                    insertQuery.bindValue(":storage_free_space",fieldList[8].toLongLong());
                    insertQuery.bindValue(":storage_brand_model",fieldList[9]);
                    insertQuery.bindValue(":storage_serial_number",fieldList[10]);
                    insertQuery.bindValue(":storage_build_date",fieldList[11]);
                    insertQuery.bindValue(":storage_content_type",fieldList[12]);
                    insertQuery.bindValue(":storage_container",fieldList[13]);
                    insertQuery.bindValue(":storage_comment", fieldList[14]);

                    insertQuery.exec();

                }
        }
        storageFile.close();

    }
    //--------------------------------------------------------------------------
    void MainWindow::loadStorageTableToModel()
    {
        //Load, filter, and sort data model
        storageModel->setTable("storage");

        if ( selectedDeviceType == "Location" ){
            QString tableFilter = "storage_location = '" + selectedDeviceName + "'";
            storageModel->setFilter(tableFilter);
        }
        else if ( selectedDeviceType == "Storage" ){
            QString tableFilter = "storage_name = '" + selectedDeviceName + "'";
            storageModel->setFilter(tableFilter);
        }
        else if ( selectedDeviceType == "Catalog" ){
            QString tableFilter = "storage_name = '" + selectedCatalog->storageName + "'";
            storageModel->setFilter(tableFilter);
        }

        storageModel->setSort(1, Qt::AscendingOrder);

        // Populate the storageModel
        if (!storageModel->select()) {
            //showError(storageModel->lastError());
            QMessageBox::information(this,"Katalog","storageModel loading error.");
            return;
        }

        // Connect model to tree/table view
        StorageView *proxyStorageModel = new StorageView(this);
        proxyStorageModel->setSourceModel(storageModel);
        proxyStorageModel->setHeaderData(0, Qt::Horizontal, tr("ID"));
        proxyStorageModel->setHeaderData(1, Qt::Horizontal, tr("Name"));
        proxyStorageModel->setHeaderData(2, Qt::Horizontal, tr("Type"));
        proxyStorageModel->setHeaderData(3, Qt::Horizontal, tr("Location"));
        proxyStorageModel->setHeaderData(4, Qt::Horizontal, tr("Path"));
        proxyStorageModel->setHeaderData(5, Qt::Horizontal, tr("Label"));
        proxyStorageModel->setHeaderData(6, Qt::Horizontal, tr("FileSystem"));
        proxyStorageModel->setHeaderData(7, Qt::Horizontal, tr("Total"));
        proxyStorageModel->setHeaderData(8, Qt::Horizontal, tr("Free"));
        proxyStorageModel->setHeaderData(9, Qt::Horizontal, tr("Brand/Model"));
        proxyStorageModel->setHeaderData(10, Qt::Horizontal, tr("Serial Number"));
        proxyStorageModel->setHeaderData(11, Qt::Horizontal, tr("Build Date"));
        proxyStorageModel->setHeaderData(12, Qt::Horizontal, tr("Content Type"));
        proxyStorageModel->setHeaderData(13, Qt::Horizontal, tr("Container"));
        proxyStorageModel->setHeaderData(14, Qt::Horizontal, tr("Comment"));

        ui->Storage_treeView_StorageList->setModel(proxyStorageModel);

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
                           SELECT storage_name
                           FROM storage
                           WHERE storage_name !=''
                                        )");

        if ( selectedDeviceType == "Location" ){
            querySQL += QLatin1String(R"( AND storage_location ='%1' )").arg(selectedDeviceName);
        }
        else if ( selectedDeviceType == "Storage" ){
            querySQL += QLatin1String(R"( AND storage_name ='%1' )").arg(selectedDeviceName);
            ui->Create_comboBox_StorageSelection->setCurrentText(selectedDeviceName);
        }
        else if ( selectedDeviceType == "Catalog" ){
            querySQL += QLatin1String(R"( AND storage_name ='%1' )").arg(selectedCatalog->storageName);
        }

        querySQL += " ORDER BY storage_name ";
        query.prepare(querySQL);
        query.exec();
        storageNameList.clear();
            while(query.next())
            {
                storageNameList<<query.value(0).toString();
            }
        loadStorageList();

        //If a storage is selected, use it for the Create screen
        if ( selectedDeviceType == "Location" ){
            ui->Create_comboBox_StorageSelection->setCurrentText("");
        }
        else if ( selectedDeviceType == "Storage" ){
            ui->Create_comboBox_StorageSelection->setCurrentText(selectedDeviceName);
        }
        else if ( selectedDeviceType == "Catalog" ){
            ui->Create_comboBox_StorageSelection->setCurrentText(selectedCatalog->storageName);
            ui->Create_lineEdit_NewCatalogPath->setText(selectedCatalog->sourcePath);
        }

        //Enable buttons
            ui->Storage_pushButton_Reload->setEnabled(true);
            ui->Storage_pushButton_EditAll->setEnabled(true);
            //ui->Storage_pushButton_SaveAll->setEnabled(true);
            ui->Storage_pushButton_New->setEnabled(true);

            //Disable create button so it cannot be overwritten
            ui->Storage_pushButton_CreateList->setEnabled(false);

    }
    //--------------------------------------------------------------------------
    void MainWindow::updateStorageInfo(Storage* storage)
    {
        //Get current values for comparison later
            qint64 previousStorageFreeSpace  = storage->freeSpace;
            qint64 previousStorageTotalSpace = storage->totalSpace;
            qint64 previousStorageUsedSpace  = previousStorageTotalSpace - previousStorageFreeSpace;
            QDateTime lastUpdate  = storage->lastUpdated;

        //verify if path is available / not empty
        QDir dir (storage->path);

            //Warning if no Path is provided
            if ( storage->path == "" ){
                QMessageBox::warning(this,tr("No path provided"),tr("No Path was provided. \n"
                                              "Modify the device to provide one and try again.\n")
                                              );
                return;
            }

            ///Warning and choice if the result is 0 files
            if(dir.entryInfoList(QDir::NoDotAndDotDot|QDir::AllEntries).count() == 0)
            {
                int result = QMessageBox::warning(this,tr("Directory is empty"),tr("The source folder does not contain any file.<br/><br/>"
                                              "This could mean that the source is empty or the device is not mounted to this folder.<br/><br/>")
                                              +tr("The application is going try to get values anyhow."));
                //return;
                if ( result == QMessageBox::Cancel){
                    return;
                }
            }

        //Update device information
            storage->updateStorageInfo();

        //Stop if the update was not done (lastUpdate time did not change)
            if (lastUpdate == storage->lastUpdated)
                return;

        //Save statistics
            storage->saveStatistics();

            //save to file
                if(databaseMode=="Memory"){
                    storage->setStatisticsFilePath(collectionFolder + "/" + statisticsFileName);
                    storage->saveStatisticsToFile();
                }

        //Prepare to report changes to the storage
        qint64 newStorageFreeSpace    = storage->freeSpace;
        qint64 deltaStorageFreeSpace  = newStorageFreeSpace - previousStorageFreeSpace;
        qint64 newStorageTotalSpace   = storage->totalSpace;
        qint64 deltaStorageTotalSpace = newStorageTotalSpace - previousStorageTotalSpace;
        qint64 newStorageUsedSpace    = newStorageTotalSpace - newStorageFreeSpace;
        qint64 deltaStorageUsedSpace  = newStorageUsedSpace - previousStorageUsedSpace;

        //Inform user about the update
        if(skipCatalogUpdateSummary !=true){
            QMessageBox msgBox;
            msgBox.setWindowTitle("Katalog");
            msgBox.setText(tr("<br/>The storage device <b> %1 </b> was updated:<br/> "
                              "<table>"
                              "<tr><td> Used Space: </td><td><b> %2 </b></td><td>  (added: <b> %3 </b>)</td></tr>"
                              "<tr><td> Free Space: </td><td><b> %4 </b></td><td>  (added: <b> %5 </b>)</td></tr>"
                              "<tr><td>Total Space: </td><td><b> %6 </b></td><td>  (added: <b> %7 </b>)</td></tr>"
                              "</table>"
                              ).arg(storage->name,
                                    QLocale().formattedDataSize(newStorageUsedSpace),
                                    QLocale().formattedDataSize(deltaStorageUsedSpace),
                                    QLocale().formattedDataSize(newStorageFreeSpace),
                                    QLocale().formattedDataSize(deltaStorageFreeSpace),
                                    QLocale().formattedDataSize(newStorageTotalSpace),
                                    QLocale().formattedDataSize(deltaStorageTotalSpace)
                                    )
                           );
            msgBox.setIcon(QMessageBox::Information);
            msgBox.exec();
        }

        //reload data to model
        loadStorageTableToModel();

        //save model data to file
        if (databaseMode=="Memory")
            saveStorageData();

        //refresh storage screen statistics
        updateStorageSelectionStatistics();

    }
    //--------------------------------------------------------------------------
    void MainWindow::saveStorageData()
    {
        if (databaseMode=="Memory"){
            //Save model data to Storage file
            saveStorageModelToFile();

            //load Storage file data to table
            loadStorageFileToTable();
        }

    }
    //--------------------------------------------------------------------------
    void MainWindow::saveStorageModelToFile()
    {
        //Prepare export file
        storageFilePath = collectionFolder + "/" + "storage.csv";
        QFile storageFile(storageFilePath);
        QTextStream out(&storageFile);

        //Prepare header line
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

        //Get data
        QSqlQuery query;
        QString querySQL = QLatin1String(R"(
                             SELECT * FROM storage
                                        )");
        query.prepare(querySQL);
        query.exec();

        //Iterate the records and generate lines
        while (query.next()) {
            const QSqlRecord record = query.record();
            for (int i=0, recCount = record.count() ; i<recCount ; ++i){
                if (i>0)
                    out << '\t';
                out << record.value(i).toString();
            }
            //-- Write the result in the file
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
                            SELECT  COUNT (storage_id),
                                    SUM(storage_free_space),
                                    SUM(storage_total_space)
                            FROM storage
                            WHERE storage_name !=''
                                        )");

        if ( selectedDeviceType == "Location" )
            querySQL += " AND storage_location = '" + selectedDeviceName + "' ";

        if ( selectedDeviceType == "Storage" )
            querySQL += " AND storage_name = '" + selectedDeviceName + "' ";

        query.prepare(querySQL);
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
    void MainWindow::on_Storage_pushButton_TestMedia_clicked()
    {
        QStringList filePaths;
        filePaths << "/home/stephane/Vidéos/COPY/test6.mp4";
        filePaths << "/home/stephane/Vidéos/COPY/test2.mkv";
        filePaths << "/home/stephane/Vidéos/COPY/test3.mp3";
        filePaths << "/home/stephane/Vidéos/COPY/test5.mkv";

        for(int i = 0; i<filePaths.length(); i++){
            setMediaFile(filePaths[i]);
        }

        QString filePath = "/home/stephane/Vidéos/COPY/test8.mkv";
        setMediaFile(filePath);
    }
