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
// File Name:   mainwindow_tab_virtual.cpp
// Purpose:     methods for the screen Virtual
// Description: https://github.com/StephaneCouturier/Katalog/wiki/Virtual
// Author:      Stephane Couturier
/////////////////////////////////////////////////////////////////////////////
*/

#include "mainwindow.h"
#include "ui_mainwindow.h"
#include "devicetreeview.h"
#include "device.h"

//TAB: VIRTUAL -------------------------------------------------------------

//--- UI -------------------------------------------------------------------
//--------------------------------------------------------------------------
void MainWindow::on_Devices_pushButton_InsertRootLevel_clicked()
{
    Device *newDevice = new Device();
    newDevice->generateDeviceID();
    newDevice->type = "Virtual";
    newDevice->name = tr("Virtual Top Item") + "_" + QString::number(newDevice->ID);
    newDevice->parentID = 0;
    newDevice->externalID = 0;
    newDevice->groupID = 1; //only DeviceID 1 can be a top item in group 0 (Pyhsical group)
    newDevice->insertDevice();

    //Save data to file
    collection->saveDeviceTableToFile();

    //Reload
    loadDeviceTableToTreeModel();
    loadParentsList();
    loadDeviceTableToTreeModel();
}
//--------------------------------------------------------------------------
void MainWindow::on_Devices_pushButton_AddVirtual_clicked()
{
    if(activeDevice->type =="Virtual")
        addDeviceVirtual();
    else{
        QMessageBox msgBox;
        msgBox.setWindowTitle("Katalog");
        msgBox.setText(QCoreApplication::translate("MainWindow",
                                                   "A Virtual device can only be added to another virtual device.<br/>"
                                                   ));
        msgBox.setIcon(QMessageBox::Warning);
        msgBox.exec();
    }
}
//--------------------------------------------------------------------------
void MainWindow::on_Devices_pushButton_AddStorage_clicked()
{
    if(activeDevice->type =="Virtual")
        addDeviceStorage();
    else{
        QMessageBox msgBox;
        msgBox.setWindowTitle("Katalog");
        msgBox.setText(QCoreApplication::translate("MainWindow",
                                                   "A Storage device can only be added to a virtual device.<br/>"
                                                   ));
        msgBox.setIcon(QMessageBox::Warning);
        msgBox.exec();
    }
}
//--------------------------------------------------------------------------
void MainWindow::on_Devices_pushButton_AssignCatalog_clicked()
{
    assignCatalogToDevice(selectedDevice, activeDevice);
}
//--------------------------------------------------------------------------
void MainWindow::on_Devices_pushButton_AssignStorage_clicked()
{
    assignStorageToDevice(selectedDevice->storage->ID, selectedDevice->ID);
}
//--------------------------------------------------------------------------
void MainWindow::on_Devices_pushButton_DeleteItem_clicked()
{
    deleteDeviceItem();
}
//--------------------------------------------------------------------------
void MainWindow::on_Devices_pushButton_Edit_clicked()
{
    editDevice();
}
//--------------------------------------------------------------------------
void MainWindow::on_Devices_pushButton_EditList_clicked()
{
    QDesktopServices::openUrl(QUrl::fromLocalFile(collection->deviceFilePath));
}
//--------------------------------------------------------------------------
void MainWindow::on_Devices_pushButton_Snapshot_clicked()
{
    recordDevicesSnapshot();
}
//--------------------------------------------------------------------------
void MainWindow::on_Devices_pushButton_Save_clicked()
{
    saveDeviceForm();
}
//--------------------------------------------------------------------------
void MainWindow::on_Devices_pushButton_Cancel_clicked()
{
    ui->Devices_widget_Edit->hide();
}
//--------------------------------------------------------------------------
void MainWindow::on_Devices_pushButton_TreeExpandCollapse_clicked()
{
    setDeviceTreeExpandState(true);
}
//--------------------------------------------------------------------------
void MainWindow::on_Devices_checkBox_DisplayStorage_stateChanged(int arg1)
{
    QSettings settings(collection->settingsFilePath, QSettings:: IniFormat);
    settings.setValue("Devices/DisplayStorage", arg1);
    loadDeviceTableToTreeModel();
}
//--------------------------------------------------------------------------
void MainWindow::on_Devices_checkBox_DisplayCatalogs_stateChanged(int arg1)
{
    QSettings settings(collection->settingsFilePath, QSettings:: IniFormat);
    settings.setValue("Devices/DisplayCatalogs", arg1);
    loadDeviceTableToTreeModel();
}
//--------------------------------------------------------------------------
void MainWindow::on_Devices_checkBox_DisplayPhysicalGroupOnly_stateChanged(int arg1)
{
    QSettings settings(collection->settingsFilePath, QSettings:: IniFormat);
    settings.setValue("Devices/DisplayPhysicalGroupOnly", arg1);
    if(arg1>0)
        ui->Devices_checkBox_DisplayAllExceptPhysicalGroup->setChecked(false);
    loadDeviceTableToTreeModel();
}
//--------------------------------------------------------------------------
void MainWindow::on_Devices_checkBox_DisplayAllExceptPhysicalGroup_stateChanged(int arg1)
{
    QSettings settings(collection->settingsFilePath, QSettings:: IniFormat);
    settings.setValue("Devices/DisplayAllExceptPhysicalGroup", arg1);
    if(arg1>0)
        ui->Devices_checkBox_DisplayPhysicalGroupOnly->setChecked(false);
    loadDeviceTableToTreeModel();
}
//--------------------------------------------------------------------------
void MainWindow::on_Devices_checkBox_DisplayFullTable_stateChanged(int arg1)
{
    QSettings settings(collection->settingsFilePath, QSettings:: IniFormat);
    settings.setValue("Devices/DisplayFullTable", arg1);
    loadDeviceTableToTreeModel();
}
//--------------------------------------------------------------------------
void MainWindow::on_Devices_pushButton_verifStorage_clicked()
{
    verifyStorageWithOutDevice();
}
//--------------------------------------------------------------------------
void MainWindow::on_Devices_treeView_DeviceList_clicked(const QModelIndex &index)
{
    //Get selection data
    activeDevice->ID = ui->Devices_treeView_DeviceList->model()->index(index.row(), 3, index.parent() ).data().toInt();
    activeDevice->loadDevice();

    //Adapt buttons to selection
    ui->Devices_pushButton_Edit->setEnabled(true);
    //TESTING

    if(activeDevice->type=="Virtual"){
        ui->Devices_pushButton_Edit->setEnabled(true);
        if(activeDevice->catalog->name!=""){
            ui->Devices_pushButton_AssignCatalog->setEnabled(true);
        }
        ui->Devices_pushButton_DeleteItem->setEnabled(true);
        ui->Devices_label_SelectedDeviceVirtual->setText(activeDevice->name);
    }
    else if(activeDevice->type=="Catalog"){
        ui->Devices_pushButton_Edit->setEnabled(false);
        ui->Devices_pushButton_AssignCatalog->setEnabled(false);
        ui->Devices_pushButton_DeleteItem->setEnabled(false);
        ui->Devices_label_SelectedDeviceVirtual->setText("");
    }
}
//--------------------------------------------------------------------------
void MainWindow::on_Devices_treeView_DeviceList_customContextMenuRequested(const QPoint &pos)
{
    //Get selection data
    QModelIndex index=ui->Devices_treeView_DeviceList->currentIndex();
    activeDevice->ID   = ui->Devices_treeView_DeviceList->model()->index(index.row(), 3, index.parent() ).data().toInt();
    activeDevice->loadDevice();

    Device *tempParentDevice = new Device();
    tempParentDevice->ID = activeDevice->parentID;
    tempParentDevice->loadDevice();

    //Set actions for catalogs
    if(activeDevice->type=="Catalog"){
        QPoint globalPos = ui->Devices_treeView_DeviceList->mapToGlobal(pos);
        QMenu deviceContextMenu;

        QString deviceName = activeDevice->name;

        QAction *menuDeviceAction1 = new QAction(QIcon::fromTheme("media-playlist-repeat"), tr("Update"), this);
        deviceContextMenu.addAction(menuDeviceAction1);

        connect(menuDeviceAction1, &QAction::triggered, this, [this, deviceName]() {
            //BackUp the file before, if the option is selected
            if ( ui->Settings_checkBox_KeepOneBackUp->isChecked() == true){ backupCatalogFile(activeDevice->catalog->filePath); }
            reportAllUpdates(activeDevice, activeDevice->updateDevice("update",collection->databaseMode,false,collection->collectionFolder),"update");
            collection->saveDeviceTableToFile();
            collection->saveStatiticsToFile();
            loadDeviceTableToTreeModel();
            loadCatalogsTableToModel();
        });

        QAction *menuDeviceAction2 = new QAction(QIcon::fromTheme("document-edit-sign"), tr("Edit"), this);
        deviceContextMenu.addAction(menuDeviceAction2);

        connect(menuDeviceAction2, &QAction::triggered, this, [this, deviceName]() {
            editDevice();
        });

        deviceContextMenu.addSeparator();

        if(activeDevice->groupID !=0){
            QAction *menuDeviceAction3 = new QAction(QIcon::fromTheme("edit-cut"), tr("Unassign this catalog"), this);
            deviceContextMenu.addAction(menuDeviceAction3);

            connect(menuDeviceAction3, &QAction::triggered, this, [this, deviceName]() {
                unassignPhysicalFromDevice(activeDevice->ID, activeDevice->parentID);
            });
        }


        QAction *menuDeviceAction4 = new QAction(QIcon::fromTheme("edit-delete"), tr("Delete this catalog"), this);
        deviceContextMenu.addAction(menuDeviceAction4);

        connect(menuDeviceAction4, &QAction::triggered, this, [this, deviceName]() {


            deleteDeviceItem();

        });

        deviceContextMenu.exec(globalPos);
    }
    else if(activeDevice->type=="Storage"){
        QPoint globalPos = ui->Devices_treeView_DeviceList->mapToGlobal(pos);
        QMenu deviceContextMenu;

        QString deviceName = activeDevice->name;

        QAction *menuDeviceAction1 = new QAction(QIcon::fromTheme("media-playlist-repeat"), tr("Update"), this);
        deviceContextMenu.addAction(menuDeviceAction1);

        connect(menuDeviceAction1, &QAction::triggered, this, [this, deviceName]() {
            reportAllUpdates(activeDevice, activeDevice->updateDevice("update", collection->databaseMode, true,collection->collectionFolder), "list");
            collection->saveDeviceTableToFile();
            collection->saveStatiticsToFile();

            loadDeviceTableToTreeModel();
            loadCatalogsTableToModel();
        });

        QAction *menuDeviceAction2 = new QAction(QIcon::fromTheme("document-edit-sign"), tr("Edit"), this);
        deviceContextMenu.addAction(menuDeviceAction2);

        connect(menuDeviceAction2, &QAction::triggered, this, [this, deviceName]() {
            editDevice();
        });

        deviceContextMenu.addSeparator();

        QAction *menuDeviceAction3 = new QAction(QIcon::fromTheme("edit-cut"), tr("Unassign this storage"), this);
        deviceContextMenu.addAction(menuDeviceAction3);

        connect(menuDeviceAction3, &QAction::triggered, this, [this, deviceName]() {
            unassignPhysicalFromDevice(activeDevice->ID, activeDevice->parentID);
        });

        QAction *menuDeviceAction4 = new QAction(QIcon::fromTheme("edit-delete"), tr("Delete this storage"), this);
        deviceContextMenu.addAction(menuDeviceAction4);

        connect(menuDeviceAction4, &QAction::triggered, this, [this, deviceName]() {
            deleteDeviceItem();
        });

        deviceContextMenu.exec(globalPos);
    }
    else{
        QPoint globalPos = ui->Devices_treeView_DeviceList->mapToGlobal(pos);
        QMenu deviceContextMenu;

        QString deviceName = activeDevice->name;

        QAction *menuDeviceAction1 = new QAction(QIcon::fromTheme("document-new"), tr("Add Virtual device"), this);
        deviceContextMenu.addAction(menuDeviceAction1);

        connect(menuDeviceAction1, &QAction::triggered, this, [this, deviceName]() {
            addDeviceVirtual();
        });

        QAction *menuDeviceAction6 = new QAction(QIcon::fromTheme("document-new"), tr("Add Storage device"), this);
        deviceContextMenu.addAction(menuDeviceAction6);

        connect(menuDeviceAction6, &QAction::triggered, this, [this, deviceName]() {
            addDeviceStorage();
        });

        QAction *menuDeviceAction5 = new QAction(QIcon::fromTheme("document-new"), tr("Assign selected storage"), this);
        deviceContextMenu.addAction(menuDeviceAction5);

        connect(menuDeviceAction5, &QAction::triggered, this, [this, deviceName]() {
            assignStorageToDevice(selectedDevice->storage->ID, activeDevice->ID);
        });

        QAction *menuDeviceAction2 = new QAction(QIcon::fromTheme("document-edit-sign"), tr("Edit"), this);
        deviceContextMenu.addAction(menuDeviceAction2);

        connect(menuDeviceAction2, &QAction::triggered, this, [this, deviceName]() {
            editDevice();
        });

        QAction *menuDeviceAction3 = new QAction(QIcon::fromTheme("media-playlist-repeat"), tr("Update"), this);
        deviceContextMenu.addAction(menuDeviceAction3);

        connect(menuDeviceAction3, &QAction::triggered, this, [this, deviceName]() {
            reportAllUpdates(activeDevice, activeDevice->updateDevice("update",collection->databaseMode,true,collection->collectionFolder), "update");
            collection->saveDeviceTableToFile();
            collection->saveStatiticsToFile();
            loadDeviceTableToTreeModel();
            loadCatalogsTableToModel();
        });

        deviceContextMenu.addSeparator();

        QAction *menuDeviceAction4 = new QAction(QIcon::fromTheme("edit-delete"), tr("Delete"), this);
        deviceContextMenu.addAction(menuDeviceAction4);

        connect(menuDeviceAction4, &QAction::triggered, this, [this, deviceName]() {
            deleteDeviceItem();
        });

        deviceContextMenu.exec(globalPos);
    }
}
//--------------------------------------------------------------------------
void MainWindow::on_Devices_pushButton_SelectPath_clicked()
{
    //Get current selected path as default path for the dialog window
    QString newDevicePath = ui->Devices_lineEdit_Path->text();

    //Open a dialog for the user to select the directory to be cataloged. Only show directories.
    QString dir = QFileDialog::getExistingDirectory(this, tr("Select the directory to be cataloged in this new catalog"),
                                                    newDevicePath,
                                                    QFileDialog::ShowDirsOnly
                                                        | QFileDialog::DontResolveSymlinks);
    //Send the selected directory to LE_NewCatalogPath (input line for the New Catalog Path)
    ui->Devices_lineEdit_Path->setText(dir);
}
//--------------------------------------------------------------------------

//--- UI / Methods temp v1.x -----------------------------------------------
void MainWindow::on_Devices_pushButton_ImportS_clicked()
{
    convertDeviceCatalogFile();
    importStorageCatalogLinks();
}
//--------------------------------------------------------------------------
void MainWindow::convertDeviceCatalogFile() {

    QSqlQuery query;
    QString querySQL;

    //Update Device type
    //save table to update columns and reload
    collection->saveDeviceTableToFile();
    collection->loadDeviceFileToTable();

    //update type
    querySQL = QLatin1String(R"(
                            UPDATE device
                            SET device_type="Virtual"
                            WHERE device_type=""
                        )");
    query.prepare(querySQL);
    query.exec();

    collection->saveDeviceTableToFile();
    collection->loadDeviceFileToTable();
    loadDeviceTableToTreeModel();

    //Insert Catalog assignments from device_catalog
    querySQL = QLatin1String(R"(
                        INSERT INTO device(
                                        device_id,
                                        device_parent_id,
                                        device_name,
                                        device_type,
                                        device_external_id )
                        VALUES(         :device_id,
                                        :device_parent_id,
                                        :device_name,
                                        :device_type,
                                        :device_external_id )
                    )");
    query.prepare(querySQL);

    //Define storage file and prepare stream
    QFile deviceCatalogFile(collection->deviceCatalogFilePath);
    QTextStream textStream(&deviceCatalogFile);

    //Open file or return information
    if(!deviceCatalogFile.open(QIODevice::ReadOnly)) {
        // Create it, if it does not exist
        QFile newDeviceCatalogFile(collection->deviceCatalogFilePath);
        if(!newDeviceCatalogFile.open(QIODevice::ReadOnly)) {
            if (newDeviceCatalogFile.open(QFile::WriteOnly | QFile::Text)) {
                QTextStream stream(&newDeviceCatalogFile);
                stream << "ID"            << "\t"
                       << "Catalog Name"          << "\t"
                       << "Directory Path"          << "\t"
                       << '\n';
                newDeviceCatalogFile.close();
            }
        }
    }
    //Load Device device lines to table
    while (true)
    {
        QString line = textStream.readLine();
        if (line.isNull())
            break;
        else if (line.left(2)!="ID"){//skip the first line with headers

            //Generate new ID
            QSqlQuery queryID;
            QString queryIDSQL = QLatin1String(R"(
                            SELECT MAX(device_id)
                            FROM device
                        )");
            queryID.prepare(queryIDSQL);
            queryID.exec();
            queryID.next();
            int newID=queryID.value(0).toInt()+1;

            //Split the string with tabulation into a list
            QStringList fieldList = line.split('\t');

            query.bindValue(":device_id",newID);
            query.bindValue(":device_parent_id",fieldList[0]);
            query.bindValue(":device_name",fieldList[1]);
            query.bindValue(":device_type","Catalog");
            query.bindValue(":device_external_id",fieldList[1]);
            query.exec();
        }
    }
    deviceCatalogFile.close();
    //Save file
    collection->saveDeviceTableToFile();
    loadDeviceTableToTreeModel();

}
//--------------------------------------------------------------------------
void MainWindow::importStorageCatalogLinks() {

    QSqlQuery query;
    QString querySQL;
    querySQL = QLatin1String(R"(
                            SELECT catalog_storage,
                                    catalog_name,
                                    storage.storage_id,
                                    device.device_id
                            FROM catalog
                            JOIN storage ON storage.storage_name = catalog.catalog_storage
                            JOIN device ON storage.storage_id = device.device_external_id
                         )");
    query.prepare(querySQL);
    query.exec();

    while (query.next()) {
        QString storage_name = query.value(0).toString();
        QString catalog_name = query.value(1).toString();
        QString storage_id   = query.value(2).toString();
        int device_id   = query.value(3).toInt();

        activeDevice->catalog->name = catalog_name;
        activeDevice->catalog->loadCatalog();

       // assignCatalogToDevice(catalog_name,device_id);

        collection->saveDeviceTableToFile();
        loadDeviceTableToTreeModel();
    }
}
//--------------------------------------------------------------------------
QList<int> MainWindow::verifyStorageWithOutDevice()
{
    QList<int> gaps;
    QSqlQuery query;
    QString querySQL;
    querySQL = QLatin1String(R"(
                            SELECT storage_id
                            FROM storage
                            WHERE storage_id NOT IN(
                                SELECT device_external_id
                                FROM device
                                WHERE device_type = 'Storage'
                            )
                         )");

    query.prepare(querySQL);
    query.exec();

    while (query.next()) {
        gaps << query.value(0).toInt();
        qDebug()<< "verifyStorageWithOutDevice - gap:" << query.value(0).toInt();
    }

    return gaps;
}
//--------------------------------------------------------------------------

//--- Methods --------------------------------------------------------------
//--------------------------------------------------------------------------
void MainWindow::assignCatalogToDevice(Device *catalogDevice, Device *parentDevice)
{
    if( parentDevice->ID!=0 and catalogDevice->ID !=0){

        //Generate new ID
        QSqlQuery queryID;
        QString queryIDSQL = QLatin1String(R"(
                            SELECT MAX(device_id)
                            FROM device
                        )");
        queryID.prepare(queryIDSQL);
        queryID.exec();
        queryID.next();
        int newID = queryID.value(0).toInt()+1;

        //Insert catalog
        QSqlQuery query;
        QString querySQL = QLatin1String(R"(
                            INSERT INTO device(
                                        device_id,
                                        device_parent_id,
                                        device_name,
                                        device_type,
                                        device_external_id,
                                        device_path,
                                        device_total_file_size,
                                        device_total_file_count,
                                        device_total_space,
                                        device_free_space,
                                        device_active,
                                        device_group_id,
                                        device_date_updated)
                            VALUES(
                                        :device_id,
                                        :device_parent_id,
                                        :device_name,
                                        :device_type,
                                        :device_external_id,
                                        :device_path,
                                        :device_total_file_size,
                                        :device_total_file_count,
                                        :device_total_space,
                                        :device_free_space,
                                        :device_active,
                                        :device_group_id,
                                        :device_date_updated)
                        )");
        query.prepare(querySQL);
        query.bindValue(":device_id", newID);
        query.bindValue(":device_parent_id", parentDevice->ID);
        query.bindValue(":device_name", catalogDevice->catalog->name);
        query.bindValue(":device_type", "Catalog");
        query.bindValue(":device_external_id", catalogDevice->catalog->ID);
        query.bindValue(":device_path", catalogDevice->catalog->sourcePath);
        query.bindValue(":device_total_file_size", catalogDevice->catalog->totalFileSize);
        query.bindValue(":device_total_file_count", catalogDevice->catalog->fileCount);
        query.bindValue(":device_total_space", 0);
        query.bindValue(":device_free_space", 0);
        query.bindValue(":device_active", catalogDevice->active);
        query.bindValue(":device_group_id", parentDevice->groupID);
        query.bindValue(":device_date_updated", catalogDevice->dateTimeUpdated);
        query.exec();

        //Save data to file
        if (collection->databaseMode == "Memory"){
            //Save file
            collection->saveDeviceTableToFile();
        }

        //Reload
        loadDeviceTableToTreeModel();
        loadDeviceTableToTreeModel();
    }
}
//--------------------------------------------------------------------------
void MainWindow::assignStorageToDevice(int storageID,int deviceID)
{
    if( deviceID!=0 and storageID!=0){
        //Generate new ID
        QSqlQuery queryID;
        QString queryIDSQL = QLatin1String(R"(
                            SELECT MAX(device_id)
                            FROM device
                        )");
        queryID.prepare(queryIDSQL);
        queryID.exec();
        queryID.next();
        int newID = queryID.value(0).toInt()+1;

        //Insert storage
        QSqlQuery query;
        QString querySQL = QLatin1String(R"(
                            INSERT INTO device(
                                        device_id,
                                        device_parent_id,
                                        device_name,
                                        device_type,
                                        device_external_id,
                                        device_path,
                                        device_total_file_size,
                                        device_total_file_count,
                                        device_total_space,
                                        device_free_space)
                            VALUES(
                                        :device_id,
                                        :device_parent_id,
                                        :device_name,
                                        :device_type,
                                        :device_external_id,
                                        :device_path,
                                        :device_total_file_size,
                                        :device_total_file_count,
                                        :device_total_space,
                                        :device_free_space)
                        )");
        query.prepare(querySQL);
        query.bindValue(":device_id", newID);
        query.bindValue(":device_parent_id", deviceID);
        query.bindValue(":device_name", selectedDevice->storage->name);
        query.bindValue(":device_type", "Storage");
        query.bindValue(":device_external_id", selectedDevice->storage->ID);
        query.bindValue(":device_path", selectedDevice->storage->path);
        query.bindValue(":device_total_file_size", 0);
        query.bindValue(":device_total_file_count", 0);
        query.bindValue(":device_total_space", selectedDevice->storage->totalSpace);
        query.bindValue(":device_free_space", selectedDevice->storage->freeSpace);
        query.exec();

        //Save data to file
        if (collection->databaseMode == "Memory"){
            //Save file
            collection->saveDeviceTableToFile();
        }

        //Reload
        loadDeviceTableToTreeModel();
    }
}
//--------------------------------------------------------------------------
void MainWindow::unassignPhysicalFromDevice(int deviceID, int deviceParentID)
{
    int result = QMessageBox::warning(this,"Katalog",
                                      tr("Do you want to remove this storage or catalog from this virtual device?"),QMessageBox::Yes|QMessageBox::Cancel);

    if ( result ==QMessageBox::Yes){

        if( deviceID!=0 and deviceParentID!=0){
            //Insert catalog
            QSqlQuery query;
            QString querySQL = QLatin1String(R"(
                                        DELETE FROM device
                                        WHERE device_id=:device_id
                                        AND   device_parent_id=:device_parent_id
                                    )");
            query.prepare(querySQL);
            query.bindValue(":device_id",deviceID);
            query.bindValue(":device_parent_id",deviceParentID);
            query.exec();

            //Save data to file
            if (collection->databaseMode == "Memory"){
                //Save file
                collection->saveDeviceTableToFile();
            }

            //Reload
            loadDeviceTableToTreeModel();
        }
    }
}
//--------------------------------------------------------------------------
void MainWindow::deleteDeviceItem()
{
    activeDevice->deleteDevice(true);

    //Save data to files
    collection->saveDeviceTableToFile();
    collection->saveStorageTableToFile();
    if(activeDevice->type =="Catalog")
        collection->deleteCatalogFile(activeDevice);

    //Reload data to models
    loadCatalogsTableToModel();
    loadDeviceTableToTreeModel();
    loadStorageTableToModel();
    updateStorageSelectionStatistics();
    loadDeviceTableToTreeModel();
}
//--------------------------------------------------------------------------
void MainWindow::recordDevicesSnapshot()
{
    //Get the current total values
    QSqlQuery queryLastCatalog;
    QString queryLastCatalogSQL = QLatin1String(R"(
                                    SELECT SUM(device_file_count), SUM(device_total_file_size), SUM(device_free_space), SUM(device_total_space)
                                    FROM statistics_device
                                    WHERE date_time = (SELECT MAX(date_time)
                                                        FROM statistics_device
                                                        WHERE record_type = "snapshot")
                                    AND device_type ="Catalog"
                                    GROUP BY date_time
                                )");
    queryLastCatalog.prepare(queryLastCatalogSQL);
    queryLastCatalog.exec();
    queryLastCatalog.next();
    qint64 lastCatalogTotalFileNumber = queryLastCatalog.value(0).toLongLong();
    qint64 lastCatalogTotalFileSize   = queryLastCatalog.value(1).toLongLong();

    QSqlQuery queryLastStorage;
    QString queryLastStorageSQL = QLatin1String(R"(
                                    SELECT SUM(device_file_count), SUM(device_total_file_size), SUM(device_free_space), SUM(device_total_space)
                                    FROM statistics_device
                                    WHERE date_time = (SELECT MAX(date_time)
                                                        FROM statistics_device
                                                        WHERE record_type = "snapshot")
                                    AND device_type ="Storage"
                                    GROUP BY date_time
                                )");
    queryLastStorage.prepare(queryLastStorageSQL);
    queryLastStorage.exec();
    queryLastStorage.next();
    qint64 lastStorageFreeSpace  = queryLastStorage.value(2).toLongLong();
    qint64 lastStorageTotalSpace = queryLastStorage.value(3).toLongLong();

    //Record current catalogs and storage devices values
    QDateTime nowDateTime = QDateTime::currentDateTime();
    recordAllDeviceStats(nowDateTime);

    //Get the new total values
    QSqlQuery queryNew;
    QString queryNewSQL = QLatin1String(R"(
                                    SELECT SUM(device_file_count), SUM(device_total_file_size), SUM(device_free_space), SUM(device_total_space)
                                    FROM statistics_device
                                    WHERE date_time = (SELECT MAX(date_time)
                                                        FROM statistics_device
                                                        WHERE record_type = "snapshot")
                                    AND device_type ="Catalog"
                                    GROUP BY date_time
                            )");
    queryNew.prepare(queryNewSQL);
    queryNew.exec();
    queryNew.next();
    qint64 newTotalFileCount  = queryNew.value(0).toLongLong();
    qint64 newTotalFileSize   = queryNew.value(1).toLongLong();

    QSqlQuery queryNewStorage;
    QString queryNewStorageSQL = QLatin1String(R"(
                                    SELECT SUM(device_file_count), SUM(device_total_file_size), SUM(device_free_space), SUM(device_total_space)
                                    FROM statistics_device
                                    WHERE date_time = (SELECT MAX(date_time)
                                                        FROM statistics_device
                                                        WHERE record_type = "snapshot")
                                    AND device_type ="Storage"
                                    GROUP BY date_time
                                )");
    queryNewStorage.prepare(queryNewStorageSQL);
    queryNewStorage.exec();
    queryNewStorage.next();
    qint64 newStorageFreeSpace  = queryNewStorage.value(2).toLongLong();
    qint64 newStorageTotalSpace = queryNewStorage.value(3).toLongLong();

    //Calculate and inform
    qint64 deltaCatalogTotalFileSize   = newTotalFileSize  - lastCatalogTotalFileSize;
    qint64 deltaCatalogTotalFileNumber = newTotalFileCount - lastCatalogTotalFileNumber;
    qint64 deltaStorageFreeSpace       = newStorageFreeSpace  - lastStorageFreeSpace;
    qint64 deltaStorageTotalSpace      = newStorageTotalSpace - lastStorageTotalSpace;

    QMessageBox msgBox;
    msgBox.setWindowTitle("Katalog");
    msgBox.setText(tr(  "<br/>A snapshot of this collection was recorded:"
                      "<table>"
                      "<tr><td><br/><b>Catalogs</b></td><td></td><td></td></tr>"
                      "<tr><td>Number of files: </td><td style='text-align: right;'><b> %1 </b></td><td>  (added: <b> %2 </b>)</td></tr>"
                      "<tr><td>Total file size: </td><td style='text-align: right;'><b> %3 </b></td><td>  (added: <b> %4 </b>)</td></tr>"
                      "<tr><td><br/><b>Storage</b></td><td></td><td></td></tr>"
                      "<tr><td>Storage free space: </td><td style='text-align: right;'><b> %5 </b></td><td>  (added: <b> %6 </b>)</td></tr>"
                      "<tr><td>Storage total space: </td><td style='text-align: right;'><b> %7 </b></td><td>  (added: <b> %8 </b>)</td></tr>"
                      "</table>"
                      ).arg(QLocale().toString(newTotalFileCount),
                            QLocale().toString(deltaCatalogTotalFileNumber),
                            QLocale().formattedDataSize(newTotalFileSize),
                            QLocale().formattedDataSize(deltaCatalogTotalFileSize),
                            QLocale().formattedDataSize(newStorageFreeSpace),
                            QLocale().formattedDataSize(deltaStorageFreeSpace),
                            QLocale().formattedDataSize(newStorageTotalSpace),
                            QLocale().formattedDataSize(deltaStorageTotalSpace)
                            ));
    msgBox.setIcon(QMessageBox::Information);
    msgBox.exec();

}
//--------------------------------------------------------------------------

//--------------------------------------------------------------------------
void MainWindow::loadDeviceTableToTreeModel()
{
    //Refresh active state
    updateAllDeviceActive();

    //Retrieve device hierarchy
    QSqlQuery query;
    QString querySQL;

    querySQL = QLatin1String(R"(
                    SELECT  device_id,
                            device_parent_id,
                            device_name,
                            device_type,
                            device_external_id,
                            device_path,
                            device_total_file_size,
                            device_total_file_count,
                            device_total_space,
                            device_free_space,
                            device_active,
                            device_group_id
                    FROM  device
                )");

    if (ui->Devices_checkBox_DisplayPhysicalGroupOnly->isChecked() == true) {
        querySQL = QLatin1String(R"(

                    WITH RECURSIVE device_tree AS (
                      SELECT
                        device_id,
                        device_parent_id,
                        device_name,
                        device_type,
                        device_external_id,
                        device_path,
                        device_total_file_size,
                        device_total_file_count,
                        device_total_space,
                        device_free_space,
                        device_active,
                        device_group_id
                      FROM device
                      WHERE device_id = 1

                      UNION ALL

                      SELECT
                        child.device_id,
                        child.device_parent_id,
                        child.device_name,
                        child.device_type,
                        child.device_external_id,
                        child.device_path,
                        child.device_total_file_size,
                        child.device_total_file_count,
                        child.device_total_space,
                        child.device_free_space,
                        child.device_active,
                        child.device_group_id
                      FROM device_tree parent
                      JOIN device child ON child.device_parent_id = parent.device_id
                    )
                    SELECT
                        device_id,
                        device_parent_id,
                        device_name,
                        device_type,
                        device_external_id,
                        device_path,
                        device_total_file_size,
                        device_total_file_count,
                        device_total_space,
                        device_free_space,
                        device_active,
                        device_group_id
                    FROM device_tree
                )");
    }
    else if (ui->Devices_checkBox_DisplayAllExceptPhysicalGroup->isChecked() == true) {
        querySQL = QLatin1String(R"(
                    WITH RECURSIVE device_tree AS (
                      SELECT
                        device_id,
                        device_parent_id,
                        device_name,
                        device_type,
                        device_external_id,
                        device_path,
                        device_total_file_size,
                        device_total_file_count,
                        device_total_space,
                        device_free_space,
                        device_active,
                        device_group_id
                      FROM device
                      WHERE device_id <> 1

                      UNION ALL

                      SELECT
                        child.device_id,
                        child.device_parent_id,
                        child.device_name,
                        child.device_type,
                        child.device_external_id,
                        child.device_path,
                        child.device_total_file_size,
                        child.device_total_file_count,
                        child.device_total_space,
                        child.device_free_space,
                        child.device_active,
                        child.device_group_id
                      FROM device_tree parent
                      JOIN device child ON child.device_parent_id = parent.device_id
                      WHERE parent.device_id <> 1
                    )
                    SELECT DISTINCT -- Add DISTINCT to remove duplicates
                        device_id,
                        device_parent_id,
                        device_name,
                        device_type,
                        device_external_id,
                        device_path,
                        device_total_file_size,
                        device_total_file_count,
                        device_total_space,
                        device_free_space,
                        device_active,
                        device_group_id
                    FROM device_tree
                )");
    }

    //Add an always true WHERE close to add AND statements after more easily
    querySQL += QLatin1String(R"(
                    WHERE 1=1
                )");

    if (ui->Devices_checkBox_DisplayCatalogs->isChecked() == false) {
        querySQL += QLatin1String(R"(
                    AND device_type !='Catalog'
                )");
    }

    if (ui->Devices_checkBox_DisplayStorage->isChecked() == false) {
        querySQL += QLatin1String(R"(
                    AND device_type !='Storage'
                    AND device_type !='Catalog'
                )");
    }

    querySQL +=" ORDER BY device_type DESC, device_parent_id ASC, device_id ASC ";
    query.prepare(querySQL);
    query.exec();

    //Prepare the tree model: headers
    deviceTreeModel->clear();
    deviceTreeModel->setHorizontalHeaderLabels({
                                                tr("Name"),
                                                tr("Device Type"),
                                                tr("Active"),
                                                tr("ID"),
                                                tr("Parent ID"),
                                                tr("External ID"),
                                                tr("Number of files"),
                                                tr("Total Size"),
                                                tr("Used space"),
                                                tr("Free space"),
                                                tr("Total space"),
                                                tr("Path"),
                                                tr("group ID"),
                                                "" });

    //Create a map to store items by ID for easy access
    QMap<int, QStandardItem*> itemMap;

    //Populate model
    while (query.next()) {

        //Get data forthe item
        int id = query.value(0).toInt();
        int parentId = query.value(1).toInt();
        QString name = query.value(2).toString();
        QString type = query.value(3).toString();
        int externalId = query.value(4).toInt();
        QString path = query.value(5).toString();
        qint64 size = query.value(6).toLongLong();
        qint64 number = query.value(7).toLongLong();
        qint64 total_space = query.value(8).toLongLong();
        qint64 free_space = query.value(9).toLongLong();
        qint64 used_space = total_space - free_space;
        bool isActive = query.value(10).toBool();
        int groupID = query.value(11).toBool();

        //Create the item for this row
        QList<QStandardItem*> rowItems;
        rowItems << new QStandardItem(name);
        rowItems << new QStandardItem(type);
        rowItems << new QStandardItem(QString::number(isActive));
        rowItems << new QStandardItem(QString::number(id));
        rowItems << new QStandardItem(QString::number(parentId));
        rowItems << new QStandardItem(QString::number(externalId));
        rowItems << new QStandardItem(QString::number(number));
        rowItems << new QStandardItem(QString::number(size));
        rowItems << new QStandardItem(QString::number(used_space));
        rowItems << new QStandardItem(QString::number(free_space));
        rowItems << new QStandardItem(QString::number(total_space));
        rowItems << new QStandardItem(path);
        rowItems << new QStandardItem(QString::number(groupID));

        //Get the item representing the name, and map the parent ID
        QStandardItem* item = rowItems.at(0);
        QStandardItem* parentItem = itemMap.value(parentId);

        //Add top-level items directly to the model
        if (parentId == 0) {
            deviceTreeModel->appendRow(rowItems);
        }
        //else append the row to the parent item
        else{
            if (parentItem) {
                parentItem->appendRow(rowItems);
            }
            else if(id!=0){
                // Skip this row and proceed to the next one
                qDebug() << "loadDeviceTableToTreeModel - Parent item not found for ID:" << id;
                continue;
            }
        }

        // Store the item in the map for future reference
        itemMap.insert(id, item);
    }

    //Load Model to treeview (Virtual tab)
        DeviceTreeView *deviceTreeViewForDeviceTab = new DeviceTreeView(this);
        deviceTreeViewForDeviceTab->setSourceModel(deviceTreeModel);
        ui->Devices_treeView_DeviceList->setModel(deviceTreeViewForDeviceTab);

        //Customize tree display
        ui->Devices_treeView_DeviceList->QTreeView::sortByColumn(0,Qt::AscendingOrder);
        ui->Devices_treeView_DeviceList->header()->setSectionResizeMode(QHeaderView::Interactive);
        ui->Devices_treeView_DeviceList->header()->setSectionResizeMode(0, QHeaderView::ResizeToContents); //Name
        ui->Devices_treeView_DeviceList->header()->resizeSection( 1, 100); //Type
        ui->Devices_treeView_DeviceList->header()->resizeSection( 2,  25); //Active
        ui->Devices_treeView_DeviceList->header()->resizeSection( 3,  25); //ID
        ui->Devices_treeView_DeviceList->header()->resizeSection( 4,  25); //Parent ID
        ui->Devices_treeView_DeviceList->header()->resizeSection( 5,  25); //External ID
        ui->Devices_treeView_DeviceList->header()->resizeSection( 6, 100); //Number of Files
        ui->Devices_treeView_DeviceList->header()->resizeSection( 7, 100); //Total File Size
        ui->Devices_treeView_DeviceList->header()->resizeSection( 8, 100); //Used space
        ui->Devices_treeView_DeviceList->header()->resizeSection( 9, 100); //Free space
        ui->Devices_treeView_DeviceList->header()->resizeSection(10, 100); //Total space
        ui->Devices_treeView_DeviceList->header()->setSectionResizeMode(11, QHeaderView::ResizeToContents); //Path
        ui->Devices_treeView_DeviceList->header()->resizeSection(12,  25); //Group ID

        if (ui->Devices_checkBox_DisplayFullTable->isChecked()) {
            ui->Devices_treeView_DeviceList->header()->showSection(1); //Type
            ui->Devices_treeView_DeviceList->header()->showSection(2); //Active
            ui->Devices_treeView_DeviceList->header()->showSection(3); //ID
            ui->Devices_treeView_DeviceList->header()->showSection(4); //Parent ID
            ui->Devices_treeView_DeviceList->header()->showSection(5); //External ID
            ui->Devices_treeView_DeviceList->header()->showSection(12); //Group ID
        } else {
            ui->Devices_treeView_DeviceList->header()->hideSection(1); //Type
            ui->Devices_treeView_DeviceList->header()->hideSection(2); //Active
            ui->Devices_treeView_DeviceList->header()->hideSection(3); //ID
            ui->Devices_treeView_DeviceList->header()->hideSection(4); //Parent ID
            ui->Devices_treeView_DeviceList->header()->hideSection(5); //External ID
            ui->Devices_treeView_DeviceList->header()->hideSection(12); //Group ID
        }

        ui->Devices_treeView_DeviceList->expandAll();

    //Load Model to treeview (Filters/Device tree)
        DeviceTreeView *deviceTreeViewForSelectionPanel = new DeviceTreeView(this);

        ui->Filters_treeView_Devices->setModel(deviceTreeViewForSelectionPanel);
        deviceTreeViewForSelectionPanel->setSourceModel(deviceTreeModel);
        ui->Filters_treeView_Devices->header()->setSectionResizeMode(QHeaderView::ResizeToContents);
        ui->Filters_treeView_Devices->sortByColumn(0,Qt::AscendingOrder);
        ui->Filters_treeView_Devices->hideColumn(1);
        ui->Filters_treeView_Devices->hideColumn(2);
        ui->Filters_treeView_Devices->hideColumn(3);
        ui->Filters_treeView_Devices->setColumnWidth(2,0);
        ui->Filters_treeView_Devices->collapseAll();
        ui->Filters_treeView_Devices->header()->hide();

        for (int var = 1; var < deviceTreeViewForSelectionPanel->columnCount(); ++var) {
            ui->Filters_treeView_Devices->header()->hideSection(var);
        }

        //Restore Expand or Collapse Device Tree
        setTreeExpandState(false);
        deviceTreeViewForSelectionPanel->boldColumnList.clear();
        ui->Filters_treeView_Devices->setModel(deviceTreeViewForSelectionPanel);
        ui->Filters_treeView_Devices->expandAll();
}
//--------------------------------------------------------------------------
void MainWindow::updateNumbers() {

    activeDevice->updateNumbersFromChildren();

    collection->saveDeviceTableToFile();
    loadDeviceTableToTreeModel();
}
//--------------------------------------------------------------------------
void MainWindow::updateAllNumbers()
{
    activeDevice->updateParentsNumbers();

    collection->saveDeviceTableToFile();
    loadDeviceTableToTreeModel();
}
//--------------------------------------------------------------------------
void MainWindow::setDeviceTreeExpandState(bool toggle)
{
    //optionDeviceTreeExpandState values:  collapseAll or 2 =collapse / 0=exp.level0 / 1=exp.level1
    QString iconName = ui->Filters_pushButton_TreeExpandCollapse->icon().name();
    QSettings settings(collection->settingsFilePath, QSettings:: IniFormat);

    if (toggle==true){

        if ( optionDeviceTreeExpandState == 2 ){
            //collapsed > expand first level
            ui->Devices_pushButton_TreeExpandCollapse->setIcon(QIcon::fromTheme("expand-all"));
            optionDeviceTreeExpandState = 0;
            ui->Devices_treeView_DeviceList->expandToDepth(0);
            settings.setValue("Virtual/optionDeviceTreeExpandState", optionDeviceTreeExpandState);
        }
        else if ( optionDeviceTreeExpandState == 0 ){
            //expanded first level > expand to second level
            ui->Devices_pushButton_TreeExpandCollapse->setIcon(QIcon::fromTheme("collapse-all"));
            optionDeviceTreeExpandState = 1;
            ui->Devices_treeView_DeviceList->expandToDepth(1);
            settings.setValue("Virtual/optionDeviceTreeExpandState", optionDeviceTreeExpandState);
        }
        else if ( optionDeviceTreeExpandState == 1 ){
            //expanded second level > collapse
            ui->Devices_pushButton_TreeExpandCollapse->setIcon(QIcon::fromTheme("expand-all"));
            optionDeviceTreeExpandState = 2;
            ui->Devices_treeView_DeviceList->collapseAll();
            settings.setValue("Virtual/optionDeviceTreeExpandState", optionDeviceTreeExpandState);
        }
    }
    else
    {
        if ( optionDeviceTreeExpandState == 0 ){
            ui->Filters_pushButton_TreeExpandCollapse->setIcon(QIcon::fromTheme("expand-all"));
            ui->Filters_treeView_Devices->collapseAll();
            ui->Filters_treeView_Devices->expandToDepth(optionDeviceTreeExpandState);
        }
        else if ( optionDeviceTreeExpandState == 1 ){
            ui->Filters_pushButton_TreeExpandCollapse->setIcon(QIcon::fromTheme("collapse-all"));
            ui->Filters_treeView_Devices->collapseAll();
            ui->Filters_treeView_Devices->expandToDepth(optionDeviceTreeExpandState);
        }
        else{
            ui->Filters_pushButton_TreeExpandCollapse->setIcon(QIcon::fromTheme("expand-all"));
            ui->Filters_treeView_Devices->collapseAll();
        }
    }
}
//--------------------------------------------------------------------------
void MainWindow::shiftIDsInDeviceTable(int shiftAmount)
{
    QSqlQuery query;

    // First, update the rows with parentID = 0 to keep them unchanged
    QString sql = "UPDATE device SET device_id = device_id + :shiftAmount "
                  "WHERE device_parent_id = 0";
    query.prepare(sql);
    query.bindValue(":shiftAmount", shiftAmount);
    if (!query.exec()) {
        qDebug() << "shiftIDsInDeviceTable - Error updating device table:" << query.lastError().text();
        return;
    }

    // Next, update the rows with parentID != 0 to shift their IDs
    sql = "UPDATE device SET device_id = device_id + :shiftAmount, "
          "device_parent_id = device_parent_id + :shiftAmount "
          "WHERE device_parent_id != 0";
    query.prepare(sql);
    query.bindValue(":shiftAmount", shiftAmount);
    if (!query.exec()) {
        qDebug() << "Error updating device table:" << query.lastError().text();
        return;
    }

    qDebug() << "shiftIDsInDeviceTable - IDs shifted successfully by" << shiftAmount;
}
//--------------------------------------------------------------------------
void MainWindow::loadParentsList()
{//Load valid list of parents to the panel comboBox. It enables a selection to change the parent of a device



    //Get data
    QSqlQuery query;
    QString querySQL = QLatin1String(R"(
                                SELECT v.device_name, v.device_id
                                FROM device v

                                WHERE device_id !=0
                                AND device_id !=:selected_device_id
                            )"); //                                AND device_type NOT IN ("Catalog","Storage")


    if(activeDevice->type == "Catalog"){
        querySQL += QLatin1String(R"(AND device_type NOT IN ("Catalog"))");
    }
    else //if(activeDevice->type == "Virtual")
    {
        querySQL += QLatin1String(R"(AND device_type NOT IN ("Catalog","Storage"))");
    }

    //querySQL += " ORDER BY device_name ";
    query.prepare(querySQL);
    query.bindValue(":selected_device_id", activeDevice->ID);
    query.exec();

    //Load to comboboxes
    ui->Devices_comboBox_Parent->clear();
    ui->Devices_comboBox_Parent->addItem("Top level",query.value(1).toInt());

    while(query.next())
    {
        ui->Devices_comboBox_Parent->addItem(query.value(0).toString()+" ("+query.value(1).toString()+")",query.value(1).toInt());
    }
}
//--------------------------------------------------------------------------
void MainWindow::addDeviceVirtual()
{   //Create a new virtual device and add it to the selected Device

    Device *newDevice = new Device();
    newDevice->generateDeviceID();
    newDevice->parentID = activeDevice->ID;
    newDevice->name = tr("Virtual") + "_" + QString::number(newDevice->ID);
    newDevice->type = "Virtual";
    newDevice->externalID = 0;
    newDevice->groupID = activeDevice->groupID;
    newDevice->insertDevice();

    //Save data to file
    collection->saveDeviceTableToFile();

    //Reload
    loadDeviceTableToTreeModel();
    loadParentsList();

    //Make it the activeDevice and edit
    activeDevice->ID = newDevice->ID;
    activeDevice->loadDevice();
    editDevice();
}
//--------------------------------------------------------------------------
void MainWindow::addDeviceStorage()
{//Create a new storage device, and add it to the selected Device

    //Create Device and related Storage under Physical group (ID=0)
    Device *newDevice = new Device();
    newDevice->generateDeviceID();
    newDevice->parentID = activeDevice->ID;
    newDevice->name = tr("Storage") + "_" + QString::number(newDevice->ID);
    newDevice->type = "Storage";
    newDevice->storage->generateID();
    newDevice->externalID = newDevice->storage->ID;
    newDevice->groupID = activeDevice->groupID;
    newDevice->insertDevice();
    newDevice->storage->name = newDevice->name; //REMOVE
    newDevice->storage->insertStorage();

    //Save data to file
    collection->saveDeviceTableToFile();

    //Reload
    loadDeviceTableToTreeModel();
    loadParentsList();

    //Load table to model
    loadStorageTableToModel();

    //Save data to file and reload
    collection->saveStorageTableToFile();
    collection->loadStorageFileToTable();

    //Refresh
    loadStorageTableToModel();
    updateStorageSelectionStatistics();
    loadParentsList();

    //Make it the activeDevice and edit
    activeDevice->ID = newDevice->ID;
    activeDevice->loadDevice();
    editDevice();
}
//--------------------------------------------------------------------------
void MainWindow::editDevice()
{   //Display a panel to edit the device values

    //Refresh parent combobox
    loadParentsList();

    //Load panel and values
    ui->Devices_widget_Edit->setVisible(true);
    ui->Devices_lineEdit_Name->setText(activeDevice->name);
    ui->Devices_label_ItemDeviceTypeValue->setText(activeDevice->type);
    ui->Devices_label_ItemDeviceIDValue->setText(QString::number(activeDevice->ID));
    if(activeDevice->type !="Virtual"){
        ui->Devices_widget_EditCommon->show();
        ui->Devices_lineEdit_Path->setText(activeDevice->path);
        ui->Devices_pushButton_SelectPath->show();
    }
    else{
        ui->Devices_widget_EditCommon->hide();
    }

    //Get parent and selected it the combobox
    Device *newDeviceItem = new Device();
    newDeviceItem->ID = activeDevice->parentID;
    newDeviceItem->loadDevice();
    ui->Devices_comboBox_Parent->setCurrentText(newDeviceItem->name+" ("+QString::number(newDeviceItem->ID)+")");
}
//--------------------------------------------------------------------------
void MainWindow::saveDeviceForm()
{//Save the device values from the edit panel
//DEV: move to object
    //Get the ID of the selected parent
    QVariant selectedData = ui->Devices_comboBox_Parent->currentData();
    activeDevice->parentID = selectedData.toInt();

    activeDevice->name = ui->Devices_lineEdit_Name->text();
    activeDevice->path = ui->Devices_lineEdit_Path->text();

    Device *parentDevice = new Device();
    parentDevice->ID = activeDevice->parentID;
    parentDevice->loadDevice();

    int newGroupID = parentDevice->groupID;
    if(parentDevice->ID == 0) //If the new parent is root, the group_id should be 1 (0 is reserved for the Physical group)
        newGroupID=1;

    //Save device
    activeDevice->saveDevice();

    //Also change the group_id of sub-devices
    Device *loopDevice = new Device();
    if(activeDevice->groupID != newGroupID){
        for(int i=0; i<activeDevice->deviceIDList.count(); i++) {
            loopDevice->ID = activeDevice->deviceIDList[i];
            loopDevice->loadDevice();
            loopDevice->groupID = newGroupID;
            loopDevice->saveDevice();
        }
    }

    //If device is a catalog, rename in catalog table
    if(activeDevice->type == "Catalog"){

        //Update Catalog name
        QString querySQL = QLatin1String(R"(
                                    UPDATE catalog
                                    SET catalog_name =:catalog_name
                                    WHERE catalog_id =:catalog_id
                                )");

        QSqlQuery updateQuery;
        updateQuery.prepare(querySQL);
        updateQuery.bindValue(":catalog_name", activeDevice->name);
        updateQuery.bindValue(":catalog_id",   activeDevice->externalID);
        updateQuery.exec();

        loadCatalogsTableToModel();
        updateCatalogsScreenStatistics();

        //Save data to file
        if (collection->databaseMode=="Memory"){
            activeDevice->catalog->saveCatalog();
            activeDevice->catalog->updateCatalogFileHeaders(collection->databaseMode);
        }

        activeDevice->catalog->renameCatalogFile(activeDevice->name);
    }

    //If device is Storage, rename in storage table
    if(activeDevice->type == "Storage"){

        QString currentStorageName = activeDevice->name;
        QString newStorageName     = ui->Devices_lineEdit_Name->text();

        //Update Storage name
        QString queryUpdateStorageSQL = QLatin1String(R"(
                                    UPDATE storage
                                    SET storage_name =:storage_name
                                    WHERE storage_id =:storage_id
                                )");

        QSqlQuery updateQuery;
        updateQuery.prepare(queryUpdateStorageSQL);
        updateQuery.bindValue(":storage_name", activeDevice->name);
        updateQuery.bindValue(":storage_id",   activeDevice->externalID);
        updateQuery.exec();

        loadStorageTableToModel();
        updateStorageSelectionStatistics();

        //Save data to file
        if (collection->databaseMode=="Memory"){
            collection->saveStorageTableToFile();
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
            updateNameQuery.bindValue(":storage_id", selectedDevice->storage->ID);
            updateNameQuery.exec();

            if (collection->databaseMode=="Memory"){
                collection->saveStatiticsToFile();
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
            if (collection->databaseMode=="Memory"){

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
                Device loopCatalog;
                while (listCatalogQuery.next()){
                    loopCatalog.catalog = new Catalog;
                    loopCatalog.catalog->name = listCatalogQuery.value(0).toString();
                    loopCatalog.catalog->loadCatalog();
                    loopCatalog.catalog->storageName = newStorageName;
                    loopCatalog.catalog->updateCatalogFileHeaders(collection->databaseMode);
                }

                //Refresh
                if(collection->databaseMode=="Memory")
                    collection->loadCatalogFilesToTable();

                loadCatalogsTableToModel();
                loadCatalogsTableToModel();
            }
        }
    }

    //Finalize
    ui->Devices_widget_Edit->hide();

    //Save data to file
    collection->saveDeviceTableToFile();

    //Reload
    loadDeviceTableToTreeModel();
    ui->Devices_pushButton_Edit->setEnabled(false);
}
//--------------------------------------------------------------------------
void MainWindow::recordAllDeviceStats(QDateTime dateTime)
{// Save the values (free space and total space) of all storage devices, completing a snapshop of the collection.

    //Get the list of storage devices
    QSqlQuery query;
    QString querySQL = QLatin1String(R"(
                                        SELECT
                                            device_id,
                                            device_name,
                                            device_total_file_size,
                                            device_total_file_count,
                                            device_total_space,
                                            device_free_space
                                        FROM device
                                    )");
    query.prepare(querySQL);
    query.exec();

    //Save values for each storage device
    Device loopDevice;
    while(query.next()){
        loopDevice.ID = query.value(0).toInt();
        loopDevice.loadDevice();
        loopDevice.saveStatistics(dateTime,"snapshot");
    }
    collection->saveStatiticsToFile();

    //Refresh
    collection->loadStatisticsDeviceFileToTable();
    loadStatisticsChart();
}
//--------------------------------------------------------------------------
void MainWindow::updateAllDeviceActive()
{//Update the value Active for all Devices

    //Storage and Catalog devices
        //Get the list of devices
        QSqlQuery query;
        QString querySQL = QLatin1String(R"(
                                            SELECT device_id
                                            FROM   device
                                    )");
        //WHERE  device_type = 'Storage' OR device_type = 'Catalog'
        //WHERE device_type IN ('Storage','Catalog')
        query.prepare(querySQL);
        query.exec();

        //Update and Save sourcePathIsActive for each catalog
        Device loopDevice;
        while (query.next()){
            loopDevice.ID = query.value(0).toInt();
            loopDevice.loadDevice();
            loopDevice.updateActive();
        }


}

//--------------------------------------------------------------------------
// DEV: migration
//--------------------------------------------------------------------------
//--- Migration 1.22 to 2.0
//-------------- UI ------------------------------------------------------------

void MainWindow::on_TEST_pushButton_GenerateMissingIDs_clicked()
{
    generateAndAssociateCatalogMissingIDs();
}

void MainWindow::on_TEST_pushButton_importStorageCatalogPathsToDevice_clicked()
{
    importStorageCatalogPathsToDevice();
}

void MainWindow::on_TEST_pushButton_ImporStatistics_clicked()
{
    importStatistics();
}

//-------------- Methods -----------------------------------------------------------

void MainWindow::generateAndAssociateCatalogMissingIDs()
{
    // Start animation while opening
    QApplication::setOverrideCursor(Qt::WaitCursor);

    //Get catalogs with missing ID
    QSqlQuery query;
    QString querySQL = QLatin1String(R"(
                                    SELECT device_id, catalog_name
                                    FROM catalog
                                    LEFT JOIN device ON device_name = catalog_name

                                )");//WHERE device_id IS NULL or catalog_id = '' or catalog_id = 0
    query.prepare(querySQL);
    query.exec();

    //Loop and generate an ID
    while(query.next()){
        Device device;
        device.ID = query.value(0).toInt();
        device.loadDevice();
        if (device.catalog->ID == 0){
            device.catalog->generateID();
        }

        device.externalID = device.catalog->ID;
        device.saveDevice();
        device.catalog->name = device.name;
        device.catalog->saveCatalog();
        device.catalog->updateCatalogFileHeaders(collection->databaseMode);
    }

    querySQL = QLatin1String(R"(
                                    UPDATE device
                                    SET device_external_id = (SELECT catalog_id FROM catalog WHERE device_name = catalog_name)
                                    WHERE device_type = 'Catalog'
                                )");
    query.prepare(querySQL);
    query.exec();


    collection->saveDeviceTableToFile();
    loadDeviceTableToTreeModel();
    loadCatalogsTableToModel();

    //Stop animation
    QApplication::restoreOverrideCursor();

    QMessageBox msgBox;
    msgBox.setWindowTitle("Katalog");
    msgBox.setText(tr("generateCatalogMissingIDs"));
    msgBox.setIcon(QMessageBox::Information);
    msgBox.exec();
}

void MainWindow::importStorageCatalogPathsToDevice()
{//Move Storage or Catalog Path to Device table

    //Retrieve device hierarchy
    QSqlQuery query;
    QString querySQL;

    querySQL = QLatin1String(R"(
                    SELECT  device_id
                    FROM  device
                )");

    query.prepare(querySQL);
    query.exec();

    //Update each device
    while (query.next()) {
        Device tempDevice;
        tempDevice.ID = query.value(0).toInt();
        tempDevice.loadDevice();

        //Load storage values
        if(tempDevice.type == "Storage"){
            tempDevice.path = tempDevice.storage->path;
            tempDevice.saveDevice();
        }

        //Load catalog values
        if(tempDevice.type == "Catalog"){
            //Get path from file
            QFile catalogFile(tempDevice.catalog->filePath);
            // Get file info
            QFileInfo catalogFileInfo(catalogFile);

            // Verify that the file can be opened
            if(catalogFile.open(QIODevice::ReadOnly)) {
                QTextStream textStreamCatalogs(&catalogFile);
                QString line = textStreamCatalogs.readLine();
                tempDevice.path = line.right(line.size() - line.indexOf(">") - 1);;
                tempDevice.saveDevice();
            }
        }
    }

    //save new data
    collection->saveDeviceTableToFile();

    //reLoad
    loadDeviceTableToTreeModel();

}

void MainWindow::importStatistics()
{
    qDebug()<<"importStatistics()";
}
//----------------------------------------------------------------------
