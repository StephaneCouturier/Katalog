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
        createStorageFile();
    }
    //--------------------------------------------------------------------------
    void MainWindow::on_Storage_pushButton_Reload_clicked()
    {
        loadStorageFileToTable();
        loadStorageTableToModel();
        updateStorageSelectionStatistics();
    }
    //--------------------------------------------------------------------------
    void MainWindow::on_Storage_pushButton_Edit_clicked()
    {
        ui->Storage_widget_Panel->show();
        ui->Storage_widget_PanelForm->setEnabled(true);
        loadStorageToPanel();
    }
    //--------------------------------------------------------------------------
    void MainWindow::on_Storage_pushButton_EditAll_clicked()
    {
        QDesktopServices::openUrl(QUrl::fromLocalFile(storageFilePath));
    }
    //--------------------------------------------------------------------------
    void MainWindow::on_Storage_pushButton_SaveAll_clicked()
    {
        //Save data to file and reload
        if (databaseMode=="Memory"){
            //Save model data to Storage file
            saveStorageTableToFile();

            //Reload Storage file data to table
            loadStorageFileToTable();
        }

        //refresh
        loadStorageTableToModel();
        updateStorageSelectionStatistics();
        unsavedChanges = false;
        ui->Storage_pushButton_SaveAll->setStyleSheet("color: black");

        loadStorageToPanel();
    }
    //--------------------------------------------------------------------------
    void MainWindow::on_Storage_treeView_StorageList_clicked(const QModelIndex &index)
    {
        selectedStorage->ID = ui->Storage_treeView_StorageList->model()->index(index.row(), 0, QModelIndex()).data().toInt();
        selectedStorage->loadStorageMetaData();

        //display buttons
        ui->Storage_pushButton_Edit->setEnabled(true);
        ui->Storage_pushButton_SearchStorage->setEnabled(true);
        ui->Storage_pushButton_SearchLocation->setEnabled(true);
        ui->Storage_pushButton_CreateCatalog->setEnabled(true);
        ui->Storage_pushButton_Update->setEnabled(true);
        ui->Storage_pushButton_Delete->setEnabled(true);
        ui->Storage_pushButton_OpenFilelight->setEnabled(true);

        selectedStorageIndexRow = index.row();

        loadStorageToPanel();
    }
    //--------------------------------------------------------------------------

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
        saveDeviceTableToFile(deviceFilePath);
        loadDeviceTableToTreeModel();
        loadStorageToPanel();
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
            selectedStorage->deleteStorage();

            //Reload data to model
            loadStorageTableToModel();

            //Save data to file and reload
            if (databaseMode=="Memory"){
                //Save model data to Storage file
                saveStorageTableToFile();

                //Reload Storage file data to table
                loadStorageFileToTable();
            }

            //refresh
            loadStorageTableToModel();
            updateStorageSelectionStatistics();

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
    void MainWindow::on_Storage_treeView_StorageList_doubleClicked()
    {
        unsavedChanges = true;
        ui->Storage_pushButton_SaveAll->setStyleSheet("color: orange");
    }
    //--------------------------------------------------------------------------

    //Panel --------------------------------------------------------------------
    void MainWindow::on_Storage_pushButton_PanelSave_clicked()
    {
        saveStorageFromPanel();
    }
    //--------------------------------------------------------------------------
    void MainWindow::on_Storage_pushButton_PanelCancel_clicked()
    {
        loadStorageToPanel();
        ui->Storage_widget_Panel->hide();
    }
    //--------------------------------------------------------------------------

//Methods-----------------------------------------------------------------------
    void MainWindow::createStorageFile()
    {
        if(databaseMode=="Memory"){
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
   }
    //--------------------------------------------------------------------------
    void MainWindow::addStorageDevice(QString deviceName)
    {
        //Create Storage entry
            tempStorage->name = deviceName;
            tempStorage->generateID();
            tempStorage->insertStorage();

        //Load table to model
        loadStorageTableToModel();

        //Save data to file and reload
        saveStorageTableToFile();
        loadStorageFileToTable();

        //Refresh
        loadStorageTableToModel();
        updateStorageSelectionStatistics();

        //Enable save button
        ui->Storage_pushButton_New->setEnabled(true);

        //Create virtual storage under Physical group (ID=0) / Default device (ID=2)
        Device *newDeviceItem = new Device();
        newDeviceItem->generateDeviceID();
        newDeviceItem->parentID = 2;
        newDeviceItem->name = tempStorage->name;
        newDeviceItem->type = "Storage";
        newDeviceItem->groupID = 0;
        newDeviceItem->externalID = tempStorage->ID;
        newDeviceItem->insertDevice();

        //Save data to file
        saveDeviceTableToFile(deviceFilePath);

        //Reload
        loadDeviceTableToTreeModel();
        loadStorageTableToModel();
    }
    //--------------------------------------------------------------------------
    void MainWindow::loadStorageFileToTable()
    {//load Storage file data to its table
        if (databaseMode=="Memory"){

            //Define storage file and prepare stream
            QFile storageFile(storageFilePath);
            QTextStream textStream(&storageFile);

            QSqlQuery queryDelete;
            queryDelete.prepare( "DELETE FROM storage" );

            //Open file or return information
            if(!storageFile.open(QIODevice::ReadOnly)) {

                queryDelete.exec();

                //Disable all buttons, enable create list
                ui->Storage_pushButton_Reload->setEnabled(false);
                ui->Storage_pushButton_EditAll->setEnabled(false);
                ui->Storage_pushButton_SaveAll->setEnabled(false);
                ui->Storage_pushButton_New->setEnabled(false);
                ui->Storage_pushButton_CreateList->setEnabled(true);

                return;
            }

            //Test file validity (application breaks between v0.13 and v0.14)
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
    }
    //--------------------------------------------------------------------------
    void MainWindow::loadStorageTableToModel()
    {
        //Generate SQL query from filters
        QSqlQuery loadStorageQuery;
        QString loadStorageQuerySQL;

        //Prepare the main part of the query
        loadStorageQuerySQL  = QLatin1String(R"(
                                        SELECT
                                            storage_id            ,
                                            storage_name          ,
                                            storage_type          ,
                                            storage_location      ,
                                            storage_path          ,
                                            storage_label         ,
                                            storage_file_system   ,
                                            storage_total_space   ,
                                            storage_free_space    ,
                                            storage_brand_model   ,
                                            storage_serial_number ,
                                            storage_build_date    ,
                                            storage_content_type  ,
                                            storage_container     ,
                                            storage_comment       ,
                                            storage_date_updated  ,
                                            device_id
                                        FROM storage s
                                        JOIN device d ON d.device_external_id = s.storage_id
                            )");
        if (    selectedDevice->type == tr("All") ){
            //No filtering
        }
        else if ( selectedDevice->type == "Storage" ){
            loadStorageQuerySQL += " WHERE device_id =:device_id ";
        }
        else if ( selectedDevice->type == "Catalog" ){
            loadStorageQuerySQL += " WHERE storage_name =:catalog_storage";
        }
        else if ( selectedDevice->type == "Virtual" ){
            QString prepareSQL = QLatin1String(R"(
                                    WHERE d.device_id IN (
                                    WITH RECURSIVE hierarchy AS (
                                         SELECT device_id, device_parent_id, device_name
                                         FROM device
                                         WHERE device_id = :device_id
                                         UNION ALL
                                         SELECT t.device_id, t.device_parent_id, t.device_name
                                         FROM device t
                                         JOIN hierarchy h ON t.device_parent_id = h.device_id
                                    )
                                    SELECT device_id
                                    FROM hierarchy)
                                                    )");
            loadStorageQuerySQL += prepareSQL;
        }

        loadStorageQuerySQL += " AND device_type ='Storage' ";

        //Execute query
        loadStorageQuery.prepare(loadStorageQuerySQL);
        loadStorageQuery.bindValue(":storage_name", selectedDevice->name);
        loadStorageQuery.bindValue(":storage_location", selectedDevice->name);
        loadStorageQuery.bindValue(":catalog_storage", selectedDevice->name);
        loadStorageQuery.bindValue(":catalog_name", selectedDevice->name);
        loadStorageQuery.bindValue(":device_id", selectedDevice->ID);
        loadStorageQuery.exec();
        loadStorageQuery.next();

        // Connect model to tree/table view
        QSqlQueryModel *storageQueryModel = new QSqlQueryModel();
        storageQueryModel->setQuery(std::move(loadStorageQuery));

        // Check for errors in the query
        if (storageQueryModel->lastError().isValid()) {
            // Handle the error, e.g., display an error message
            qDebug() << "Error: " << storageQueryModel->lastError().text();
        }

        StorageView *proxyStorageModel = new StorageView(this);
        proxyStorageModel->setSourceModel(storageQueryModel);
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
        proxyStorageModel->setHeaderData(15, Qt::Horizontal, tr("Last Update"));

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
        ui->Storage_treeView_StorageList->header()->resizeSection(9, 150); //BrandModel
        ui->Storage_treeView_StorageList->header()->resizeSection(10,150); //Serial
        ui->Storage_treeView_StorageList->header()->resizeSection(11, 75); //Build date
        ui->Storage_treeView_StorageList->header()->resizeSection(12,125); //Content
        ui->Storage_treeView_StorageList->header()->resizeSection(13,125); //Container
        ui->Storage_treeView_StorageList->header()->resizeSection(14, 50); //Comment
        ui->Storage_treeView_StorageList->header()->hideSection( 3); //Location
        ui->Storage_treeView_StorageList->header()->hideSection(15); //Last Update

        //Get the list of device names for the Create screen
        loadStorageList();

        //If a storage is selected, use it for the Create screen
        if ( selectedDevice->type == "Location" ){
            ui->Create_comboBox_StorageSelection->setCurrentText("");
        }
        else if ( selectedDevice->type == "Storage" ){
            ui->Create_comboBox_StorageSelection->setCurrentText(selectedDevice->name);
        }
        else if ( selectedDevice->type == "Catalog" ){
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

        //Update Storage screen statistics
            updateStorageSelectionStatistics();


    }
    //----------------------------------------------------------------------
    void MainWindow::updateStorageInfo(Storage* storage)
    {
        //verify if path is available / not empty
        QDir dir (storage->path);

        //Warning if no Path is provided
        if ( storage->path == "" ){
            QMessageBox msgBox;
            msgBox.setWindowTitle("Katalog");
            msgBox.setText(QCoreApplication::translate("MainWindow", "No Path was provided. <br/>"
                                                                     "Modify the device to provide one and try again."));
            msgBox.setIcon(QMessageBox::Warning);
            msgBox.exec();

            return;
        }

        ///Warning and choice if the result is 0 files
        if(dir.entryInfoList(QDir::NoDotAndDotDot|QDir::AllEntries).count() == 0)
        {
            QMessageBox msgBox;
            msgBox.setWindowTitle("Katalog");
            msgBox.setText(QCoreApplication::translate("MainWindow", "The source folder '%1' does not contain any file.<br/><br/>"
                                                                     "This could mean that the source is empty or the device is not mounted to this folder.<br/><br/>"
                                                                     "Force trying to get values anyhow?").arg(storage->path)
                           );
            msgBox.setIcon(QMessageBox::Warning);
            msgBox.setStandardButtons(QMessageBox::Ok | QMessageBox::Cancel);
            msgBox.setDefaultButton(QMessageBox::Cancel);
            int result = msgBox.exec();

            if ( result == QMessageBox::Cancel){
                    return;
            }
        }

        //Update device information
            QList<qint64> updates = storage->updateStorageInfo();

        //Save statistics
            QDateTime dateTime = QDateTime::currentDateTime();
            storage->saveStatistics(dateTime);

            //save to file
                if(databaseMode=="Memory"){
                    QString storageStatisticsFilePath = collectionFolder + "/" + "statistics_storage.csv"; //statisticsFileName;
                    storage->saveStatisticsToFile(storageStatisticsFilePath, dateTime);
                }

        //Inform user about the update
        if(skipCatalogUpdateSummary !=true and updates.count()>0){
            QMessageBox msgBox;
            msgBox.setWindowTitle("Katalog");
            msgBox.setText(tr("<br/>The storage device <b> %1 </b> was updated:<br/> "
                              "<table>"
                              "<tr><td> Used Space: </td><td><b> %2 </b></td><td>  (added: <b> %3 </b>)</td></tr>"
                              "<tr><td> Free Space: </td><td><b> %4 </b></td><td>  (added: <b> %5 </b>)</td></tr>"
                              "<tr><td>Total Space: </td><td><b> %6 </b></td><td>  (added: <b> %7 </b>)</td></tr>"
                              "</table>"
                              ).arg(storage->name,
                                    QLocale().formattedDataSize(updates[0]),
                                    QLocale().formattedDataSize(updates[1]),
                                    QLocale().formattedDataSize(updates[2]),
                                    QLocale().formattedDataSize(updates[3]),
                                    QLocale().formattedDataSize(updates[4]),
                                    QLocale().formattedDataSize(updates[5])
                                    )
                           );
            msgBox.setIcon(QMessageBox::Information);
            msgBox.exec();
        }

        //reload data to model
        loadStorageTableToModel();

        //save model data to file
        if (databaseMode=="Memory"){
                //Save model data to Storage file
                saveStorageTableToFile();

                //Reload Storage file data to table
                loadStorageFileToTable();
        }

        //refresh storage screen statistics
        updateStorageSelectionStatistics();

    }
    //--------------------------------------------------------------------------
    void MainWindow::saveStorageTableToFile()
    {
        if (databaseMode=="Memory"){
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
                //Close the file
                //storageFile.close();
            }
            storageFile.close();
        }
    }
    //--------------------------------------------------------------------------
    void MainWindow::updateStorageSelectionStatistics()
    {
        //Get storage statistics
        QSqlQuery query;

        //Prepare the main part of the query
        QString querySQL = QLatin1String(R"(
                                        SELECT
                                            COUNT (s.storage_id),
                                            SUM(s.storage_free_space),
                                            SUM(s.storage_total_space)

                                        FROM storage s
                                        LEFT JOIN device d ON d.device_external_id = s.storage_id
                            )");
        if (    selectedDevice->type  == tr("All") )
        {
            //No filtering
        }

        else if ( selectedDevice->type == "Storage" ){
            querySQL += " WHERE device_name =:device_name ";
        }
        //         else if ( selectedDeviceType == "Catalog" ){
        //            loadStorageQuerySQL += " WHERE storage_name =:catalog_storage";
        //        }
        else if ( selectedDevice->type == "Virtual" ){
            QString prepareSQL = QLatin1String(R"(
                                    WHERE d.device_id IN (
                                    WITH RECURSIVE hierarchy AS (
                                         SELECT device_id, device_parent_id, device_name
                                         FROM device
                                         WHERE device_id = :device_id
                                         UNION ALL
                                         SELECT t.device_id, t.device_parent_id, t.device_name
                                         FROM device t
                                         JOIN hierarchy h ON t.device_parent_id = h.device_id
                                    )
                                    SELECT device_id
                                    FROM hierarchy)
                                                    )");
            querySQL += prepareSQL;
        }

        //Execute query
        query.prepare(querySQL);
        query.bindValue(":device_name", selectedDevice->name);
        //query.bindValue(":catalog_storage", selectedDevice->name);
        query.bindValue(":catalog_name", selectedDevice->name);
        query.bindValue(":device_id", selectedDevice->ID);
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
    void MainWindow::recordAllStorageStats(QDateTime dateTime)
    {// Save the values (free space and total space) of all storage devices, completing a snapshop of the collection.

        //Get the list of storage devices
        QSqlQuery query;
        QString querySQL = QLatin1String(R"(
                                        SELECT
                                            storage_id,
                                            storage_name,
                                            storage_free_space,
                                            storage_total_space
                                        FROM storage
                                        )");
        query.prepare(querySQL);
        query.exec();

        //Save values for each storage device
        while(query.next()){
            tempStorage->ID = query.value(0).toInt();
            tempStorage->loadStorageMetaData();
            tempStorage->saveStatistics(dateTime);

            if(databaseMode=="Memory")
            {
                QString filePath = collectionFolder + "/" + "statistics_storage.csv";
                tempStorage->saveStatisticsToFile(filePath, dateTime);
            }
        }

        //Refresh
        if(databaseMode=="Memory"){
            loadStatisticsCatalogFileToTable();
            loadStatisticsStorageFileToTable();
        }

        loadStatisticsChart();

    }
    //--------------------------------------------------------------------------
    void MainWindow::displayStoragePicture()
    {//Load and display the picture of the storage device
        QString picturePath = collectionFolder + "/images/" + QString::number(selectedStorage->ID) + ".jpg";
        QPixmap pic(picturePath);
        QFile file(picturePath);
        if(file.exists()){
            ui->Storage_label_Picture->setScaledContents(true);
            ui->Storage_label_Picture->setPixmap(pic.scaled(350, 300, Qt::KeepAspectRatio));
        }
        else{
            QPixmap empty("");
            ui->Storage_label_Picture->setPixmap(empty);
        }
    }
    //--------------------------------------------------------------------------
    void MainWindow::loadStorageToPanel()
    {//Load selected Storage device to the edition panel
        ui->Storage_lineEdit_Panel_ID->setText(QString::number(selectedStorage->ID));
        ui->Storage_lineEdit_Panel_Name->setText(selectedStorage->name);
        ui->Storage_lineEdit_Panel_Type->setText(selectedStorage->type);
        ui->Storage_lineEdit_Panel_Path->setText(selectedStorage->path);
        ui->Storage_lineEdit_Panel_Label->setText(selectedStorage->label);
        ui->Storage_lineEdit_Panel_FileSystem->setText(selectedStorage->fileSystem);

        ui->Storage_lineEdit_Panel_Total->setText(QString::number(selectedStorage->totalSpace));
        ui->Storage_lineEdit_Panel_Free->setText(QString::number(selectedStorage->freeSpace));
        ui->Storage_label_Panel_TotalSpace->setText(QLocale().formattedDataSize(selectedStorage->totalSpace));
        ui->Storage_label_Panel_FreeSpace->setText(QLocale().formattedDataSize(selectedStorage->freeSpace));

        ui->Storage_lineEdit_Panel_BrandModel->setText(selectedStorage->brand_model);
        ui->Storage_lineEdit_Panel_SerialNumber->setText(selectedStorage->serialNumber);
        ui->Storage_lineEdit_Panel_BuildDate->setText(selectedStorage->buildDate);
        ui->Storage_lineEdit_Panel_ContentType->setText(selectedStorage->contentType);
        ui->Storage_lineEdit_Panel_Container->setText(selectedStorage->container);
        ui->Storage_lineEdit_Panel_Comment->setText(selectedStorage->comment);

        displayStoragePicture();
    }
    //--------------------------------------------------------------------------
    void MainWindow::saveStorageFromPanel()
    {//Save changes to selected Storage device from the edition panel

        QString currentStorageName = selectedStorage->name;
        QString newStorageName     = ui->Storage_lineEdit_Panel_Name->text();

        //Update
        QString querySQL = QLatin1String(R"(
                                    UPDATE storage
                                    SET storage_id = :new_storage_id,
                                        storage_name =:storage_name,
                                        storage_type =:storage_type,
                                        storage_location =:storage_location,
                                        storage_path =:storage_path,
                                        storage_label =:storage_label,
                                        storage_file_system =:storage_file_system,
                                        storage_total_space =:storage_total_space,
                                        storage_free_space =:storage_free_space,
                                        storage_brand_model =:storage_brand_model,
                                        storage_serial_number =:storage_serial_number,
                                        storage_build_date =:storage_build_date,
                                        storage_content_type =:storage_content_type,
                                        storage_container =:storage_container,
                                        storage_comment = :storage_comment
                                    WHERE storage_id =:storage_id
                                )");

        QSqlQuery updateQuery;
        updateQuery.prepare(querySQL);
        updateQuery.bindValue(":new_storage_id",        ui->Storage_lineEdit_Panel_ID->text());
        updateQuery.bindValue(":storage_name",          ui->Storage_lineEdit_Panel_Name->text());
        updateQuery.bindValue(":storage_type",          ui->Storage_lineEdit_Panel_Type->text());
        updateQuery.bindValue(":storage_path",          ui->Storage_lineEdit_Panel_Path->text());
        updateQuery.bindValue(":storage_label",         ui->Storage_lineEdit_Panel_Label->text());
        updateQuery.bindValue(":storage_file_system",   ui->Storage_lineEdit_Panel_FileSystem->text());
        updateQuery.bindValue(":storage_total_space",   ui->Storage_lineEdit_Panel_Total->text());
        updateQuery.bindValue(":storage_free_space",    ui->Storage_lineEdit_Panel_Free->text());
        updateQuery.bindValue(":storage_brand_model",   ui->Storage_lineEdit_Panel_BrandModel->text());
        updateQuery.bindValue(":storage_serial_number", ui->Storage_lineEdit_Panel_SerialNumber->text());
        updateQuery.bindValue(":storage_build_date",    ui->Storage_lineEdit_Panel_BuildDate->text());
        updateQuery.bindValue(":storage_content_type",  ui->Storage_lineEdit_Panel_ContentType->text());
        updateQuery.bindValue(":storage_container",     ui->Storage_lineEdit_Panel_Container->text());
        updateQuery.bindValue(":storage_comment",       ui->Storage_lineEdit_Panel_Comment->text());
        updateQuery.bindValue(":storage_id",            selectedStorage->ID);
        updateQuery.exec();

        loadStorageTableToModel();
        updateStorageSelectionStatistics();

        //Save data to file
        if (databaseMode=="Memory"){
            saveStorageTableToFile();
        }

        //Update name in statistics and catalogs
        if (currentStorageName != newStorageName){
            //Update statistics
            QString updateNameQuerySQL = QLatin1String(R"(
                                    UPDATE statistics_storage
                                    SET storage_name = :new_storage_name
                                    WHERE storage_id =:storage_id
                                )");

            QSqlQuery updateNameQuery;
            updateNameQuery.prepare(updateNameQuerySQL);
            updateNameQuery.bindValue(":new_storage_name", newStorageName);
            updateNameQuery.bindValue(":storage_id", selectedStorage->ID);
            updateNameQuery.exec();

            if (databaseMode=="Memory"){
                saveStatiticsToFile();
            }

            //Update catalogs (database mode)
            QString updateCatalogQuerySQL = QLatin1String(R"(
                                    UPDATE catalog
                                    SET catalog_storage = :new_storage_name
                                    WHERE catalog_storage =:current_storage_name
                                )");

            QSqlQuery updateCatalogQuery;
            updateCatalogQuery.prepare(updateCatalogQuerySQL);
            updateCatalogQuery.bindValue(":current_storage_name", currentStorageName);
            updateCatalogQuery.bindValue(":new_storage_name", newStorageName);
            updateCatalogQuery.exec();

            //Update catalogs (memory mode)
            if (databaseMode=="Memory"){

                //List catalogs
                QString listCatalogQuerySQL = QLatin1String(R"(
                                    SELECT catalog_name
                                    FROM catalog
                                    WHERE catalog_storage =:new_storage_name
                                )");

                QSqlQuery listCatalogQuery;
                listCatalogQuery.prepare(listCatalogQuerySQL);
                listCatalogQuery.bindValue(":new_storage_name", newStorageName);
                listCatalogQuery.exec();

                //Edit and save each one
                while (listCatalogQuery.next()){
                    tempCatalog = new Catalog;
                    tempCatalog->name = listCatalogQuery.value(0).toString();
                    tempCatalog->loadCatalogMetaData();
                    tempCatalog->storageName = newStorageName;
                    tempCatalog->updateStorageNameToFile();
                }

                //Refresh
                if(databaseMode=="Memory")
                    loadCatalogFilesToTable();

                loadCatalogsTableToModel();
                loadCatalogsTableToModel();
            }
        }
    }
    //--------------------------------------------------------------------------
    void MainWindow::loadStorageList()
    {   //Load Storage selection to comboBoxes

        //Get data
        QSqlQuery query;
        QString querySQL = QLatin1String(R"(
                                SELECT s.storage_name, d.device_id
                                FROM storage s
                                LEFT JOIN device d ON s.storage_id = d.device_external_id
                                WHERE storage_name !=''
                            )");

        if ( selectedDevice->type == "Location" ){
            querySQL += QLatin1String(R"( AND storage_location ='%1' )").arg(selectedDevice->name);
        }
        else if ( selectedDevice->type == "Storage" ){
            querySQL += QLatin1String(R"( AND storage_name ='%1' )").arg(selectedDevice->name);
            ui->Create_comboBox_StorageSelection->setCurrentText(selectedDevice->name);
        }
        else if ( selectedDevice->type == "Catalog" ){
            querySQL += QLatin1String(R"( AND storage_name ='%1' )").arg(selectedCatalog->storageName);
        }

        querySQL += " ORDER BY storage_name ";
        query.prepare(querySQL);
        query.exec();

        //Load to comboboxes
        ui->Create_comboBox_StorageSelection->clear();
        ui->Catalogs_comboBox_Storage->clear();
        while(query.next())
        {
            ui->Create_comboBox_StorageSelection->addItem(query.value(0).toString(),query.value(1).toInt());
            ui->Catalogs_comboBox_Storage->addItem(query.value(0).toString(),query.value(1).toInt());
        }

    }
    //--------------------------------------------------------------------------

//--------------------------------------------------------------------------
//--------------------------------------------------------------------------
    void MainWindow::on_TEST_pushButton_TestMedia_clicked()
    {
        QStringList filePaths;
        filePaths << "/home/stephane/Vidéos/COPY/test6.mp4";
//        filePaths << "/home/stephane/Vidéos/COPY/test2.mkv";
//        filePaths << "/home/stephane/Vidéos/COPY/test3.mp3";
//        filePaths << "/home/stephane/Vidéos/COPY/test5.mkv";

        for(int i = 0; i<filePaths.length(); i++){
            setMediaFile(filePaths[i]);
        }

        QString filePath = "/home/stephane/Vidéos/COPY/test8.mkv";
        setMediaFile(filePath);
    }
