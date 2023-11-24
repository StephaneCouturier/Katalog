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
    saveDeviceTableToFile(deviceFilePath);

    //Reload
    loadDeviceTableToTreeModel();
    loadParentsList();
    loadDeviceTableToTreeModel();
}
//--------------------------------------------------------------------------
void MainWindow::on_Devices_pushButton_AddVirtual_clicked()
{
    addDeviceVirtual();
}
//--------------------------------------------------------------------------
void MainWindow::on_Devices_pushButton_AddStorage_clicked()
{
    addDeviceStorage();
}
//--------------------------------------------------------------------------
void MainWindow::on_Devices_pushButton_AssignCatalog_clicked()
{
    assignCatalogToDevice(selectedCatalog->name, selectedDevice->ID);
}
//--------------------------------------------------------------------------
void MainWindow::on_Devices_pushButton_AssignStorage_clicked()
{
    assignStorageToDevice(selectedStorage->ID, selectedDevice->ID);
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
    QDesktopServices::openUrl(QUrl::fromLocalFile(deviceFilePath));
}
//--------------------------------------------------------------------------
void MainWindow::on_Devices_pushButton_Save_clicked()
{
    saveDevice();
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
    QSettings settings(settingsFilePath, QSettings:: IniFormat);
    settings.setValue("Devices/DisplayStorage", arg1);
    loadDeviceTableToTreeModel();
}
//--------------------------------------------------------------------------
void MainWindow::on_Devices_checkBox_DisplayCatalogs_stateChanged(int arg1)
{
    QSettings settings(settingsFilePath, QSettings:: IniFormat);
    settings.setValue("Devices/DisplayCatalogs", arg1);
    loadDeviceTableToTreeModel();
}
//--------------------------------------------------------------------------
void MainWindow::on_Devices_checkBox_DisplayPhysicalGroupOnly_stateChanged(int arg1)
{
    QSettings settings(settingsFilePath, QSettings:: IniFormat);
    settings.setValue("Devices/DisplayPhysicalGroupOnly", arg1);
    if(arg1>0)
        ui->Devices_checkBox_DisplayAllExceptPhysicalGroup->setChecked(false);
    loadDeviceTableToTreeModel();
}
//--------------------------------------------------------------------------
void MainWindow::on_Devices_checkBox_DisplayAllExceptPhysicalGroup_stateChanged(int arg1)
{
    QSettings settings(settingsFilePath, QSettings:: IniFormat);
    settings.setValue("Devices/DisplayAllExceptPhysicalGroup", arg1);
    if(arg1>0)
        ui->Devices_checkBox_DisplayPhysicalGroupOnly->setChecked(false);
    loadDeviceTableToTreeModel();
}
//--------------------------------------------------------------------------
void MainWindow::on_Devices_checkBox_DisplayFullTable_stateChanged(int arg1)
{
    QSettings settings(settingsFilePath, QSettings:: IniFormat);
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
    tempDevice->ID = ui->Devices_treeView_DeviceList->model()->index(index.row(), 3, index.parent() ).data().toInt();
    tempDevice->loadDevice();

    //Adapt buttons to selection
    ui->Devices_pushButton_Edit->setEnabled(true);
    //TESTING

    if(tempDevice->type=="Virtual"){
        ui->Devices_pushButton_Edit->setEnabled(true);
        if(selectedCatalog->name!=""){
            ui->Devices_pushButton_AssignCatalog->setEnabled(true);
        }
        ui->Devices_pushButton_DeleteItem->setEnabled(true);
        ui->Devices_label_SelectedDeviceVirtual->setText(tempDevice->name);
    }
    else if(tempDevice->type=="Catalog"){
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
    tempDevice->ID   = ui->Devices_treeView_DeviceList->model()->index(index.row(), 3, index.parent() ).data().toInt();
    tempDevice->loadDevice();
    tempDevice->loadSubDeviceList();

    Device *tempParentDevice = new Device();
    tempParentDevice->ID = tempDevice->parentID;
    tempParentDevice->loadDevice();

    //Set actions for catalogs
    if(tempDevice->type=="Catalog"){
        QPoint globalPos = ui->Devices_treeView_DeviceList->mapToGlobal(pos);
        QMenu deviceContextMenu;

        QString deviceName = tempDevice->name;

        QAction *menuDeviceAction1 = new QAction(QIcon::fromTheme("media-playlist-repeat"), tr("Update"), this);
        deviceContextMenu.addAction(menuDeviceAction1);

        connect(menuDeviceAction1, &QAction::triggered, this, [this, deviceName]() {
            tempCatalog->name = tempDevice->name;
            tempCatalog->loadCatalogMetaData();
            updateSingleCatalog(tempCatalog, true);

            updateAllNumbers();
        });

        deviceContextMenu.addSeparator();

        QAction *menuDeviceAction3 = new QAction(QIcon::fromTheme("edit-cut"), tr("Unassign this catalog"), this);
        deviceContextMenu.addAction(menuDeviceAction3);

        connect(menuDeviceAction3, &QAction::triggered, this, [this, deviceName]() {
            unassignPhysicalFromDevice(tempDevice->ID, tempDevice->parentID);
        });

        deviceContextMenu.exec(globalPos);
    }
    else if(tempDevice->type=="Storage"){
        QPoint globalPos = ui->Devices_treeView_DeviceList->mapToGlobal(pos);
        QMenu deviceContextMenu;

        QString deviceName = tempDevice->name;

        QAction *menuDeviceAction1 = new QAction(QIcon::fromTheme("media-playlist-repeat"), tr("Update"), this);
        deviceContextMenu.addAction(menuDeviceAction1);

        connect(menuDeviceAction1, &QAction::triggered, this, [this, deviceName]() {
            updateAllNumbers();
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
            unassignPhysicalFromDevice(tempDevice->ID, tempDevice->parentID);
        });

        deviceContextMenu.exec(globalPos);
    }
    else{
        QPoint globalPos = ui->Devices_treeView_DeviceList->mapToGlobal(pos);
        QMenu deviceContextMenu;

        QString deviceName = tempDevice->name;

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
            assignStorageToDevice(selectedStorage->ID, tempDevice->ID);
        });

        QAction *menuDeviceAction2 = new QAction(QIcon::fromTheme("document-edit-sign"), tr("Edit"), this);
        deviceContextMenu.addAction(menuDeviceAction2);

        connect(menuDeviceAction2, &QAction::triggered, this, [this, deviceName]() {
            editDevice();
        });

        QAction *menuDeviceAction3 = new QAction(QIcon::fromTheme("media-playlist-repeat"), tr("Update"), this);
        deviceContextMenu.addAction(menuDeviceAction3);

        connect(menuDeviceAction3, &QAction::triggered, this, [this, deviceName]() {
            updateNumbers(tempDevice->ID, tempDevice->type);
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
    saveDeviceTableToFile(deviceFilePath);
    loadDeviceFileToTable();

    //update type
    querySQL = QLatin1String(R"(
                            UPDATE device
                            SET device_type="Virtual"
                            WHERE device_type=""
                        )");
    query.prepare(querySQL);
    query.exec();

    saveDeviceTableToFile(deviceFilePath);
    loadDeviceFileToTable();
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
    QFile deviceCatalogFile(deviceCatalogFilePath);
    QTextStream textStream(&deviceCatalogFile);

    //Open file or return information
    if(!deviceCatalogFile.open(QIODevice::ReadOnly)) {
        // Create it, if it does not exist
        QFile newDeviceCatalogFile(deviceCatalogFilePath);
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
    saveDeviceTableToFile(deviceFilePath);
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

        selectedCatalog->name = catalog_name;
        selectedCatalog->loadCatalogMetaData();

        assignCatalogToDevice(catalog_name,device_id);

        saveDeviceTableToFile(deviceFilePath);
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
    qDebug()<< "verifyStorageWithOutDevice:" << query.lastError();

    while (query.next()) {
        gaps << query.value(0).toInt();
        qDebug()<< "gap:" << query.value(0).toInt();
    }

    return gaps;
}
//--------------------------------------------------------------------------

//--- Methods --------------------------------------------------------------
//--------------------------------------------------------------------------
void MainWindow::assignCatalogToDevice(QString catalogName,int deviceID)
{
    if( deviceID!=0 and catalogName!=""){

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
        query.bindValue(":device_name", selectedCatalog->name);
        query.bindValue(":device_type", "Catalog");
        query.bindValue(":device_external_id", selectedCatalog->name);
        query.bindValue(":device_path", selectedCatalog->sourcePath);
        query.bindValue(":device_total_file_size", selectedCatalog->totalFileSize);
        query.bindValue(":device_total_file_count", selectedCatalog->fileCount);
        query.bindValue(":device_total_space", 0);
        query.bindValue(":device_free_space", 0);
        query.exec();

        //Save data to file
        if (databaseMode == "Memory"){
            //Save file
            saveDeviceTableToFile(deviceFilePath);
        }

        //Reload
        loadDeviceTableToTreeModel();
        loadDeviceTableToTreeModel();
    }
}
//--------------------------------------------------------------------------
void MainWindow::assignStorageToDevice(int storageID,int deviceID)
{
    qDebug()<<storageID<<deviceID;
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
        query.bindValue(":device_name", selectedStorage->name);
        query.bindValue(":device_type", "Storage");
        query.bindValue(":device_external_id", selectedStorage->ID);
        query.bindValue(":device_path", selectedStorage->path);
        query.bindValue(":device_total_file_size", 0);
        query.bindValue(":device_total_file_count", 0);
        query.bindValue(":device_total_space", selectedStorage->totalSpace);
        query.bindValue(":device_free_space", selectedStorage->freeSpace);
        query.exec();

        //Save data to file
        if (databaseMode == "Memory"){
            //Save file
            saveDeviceTableToFile(deviceFilePath);
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
            if (databaseMode == "Memory"){
                //Save file
                saveDeviceTableToFile(deviceCatalogFilePath);
            }

            //Reload
            loadDeviceTableToTreeModel();
        }
    }
}
//--------------------------------------------------------------------------
void MainWindow::deleteDeviceItem()
{
    tempDevice->verifyHasSubDevice();
    tempDevice->verifyHasCatalog();

    if ( tempDevice->hasSubDevice == false ){

        if ( tempDevice->hasCatalog == false ){

            int result = QMessageBox::warning(this,"Katalog",
                                              tr("Do you want to <span style='color: red';>delete</span> this %1 device?"
                                                 "<table>"
                                                 "<tr><td>ID:   </td><td><b> %2 </td></tr>"
                                                 "<tr><td>Name: </td><td><b> %3 </td></tr>"
                                                 "</table>").arg(tempDevice->type, QString::number(tempDevice->ID), tempDevice->name)
                                              ,QMessageBox::Yes|QMessageBox::Cancel);

            if ( result ==QMessageBox::Yes){

                //Delete selected ID
                tempDevice->deleteDevice();
                if(tempDevice->type == "Storage"){
                    tempStorage->ID = tempDevice->externalID;
                    tempStorage->deleteStorage();
                }

                //Save data to files
                saveDeviceTableToFile(deviceFilePath);
                saveStorageTableToFile();

                //Reload data to models
                loadDeviceTableToTreeModel();
                loadStorageTableToModel();
                updateStorageSelectionStatistics();
                loadDeviceTableToTreeModel();
            }
        }
        else{
            QMessageBox msgBox;
            msgBox.setWindowTitle("Katalog");
            msgBox.setText(tr("The selected device cannot be deleted as long as it has catalogs linked."));
            msgBox.setIcon(QMessageBox::Warning);
            msgBox.exec();
        }
    }
    else{
        QMessageBox msgBox;
        msgBox.setWindowTitle("Katalog");
        msgBox.setText(tr("The selected device cannot be deleted as long as it has sub-devices."));
        msgBox.setIcon(QMessageBox::Warning);
        msgBox.exec();
    }
}
//--------------------------------------------------------------------------
void MainWindow::loadDeviceFileToTable()
{
    if(databaseMode=="Memory"){
        //Clear table
        QSqlQuery query;
        QString querySQL;
        querySQL = QLatin1String(R"(
                        DELETE FROM device
                    )");
        query.prepare(querySQL);
        query.exec();

        //Define storage file and prepare stream
        QFile deviceFile(deviceFilePath);
        QTextStream textStream(&deviceFile);

        //Open file or create it
        if(!deviceFile.open(QIODevice::ReadOnly)) {
            // Create it, if it does not exist
            QFile newDeviceFile(deviceFilePath);
            newDeviceFile.open(QFile::WriteOnly | QFile::Text);
            QTextStream stream(&newDeviceFile);
            stream << "ID"            << "\t"
                   << "Parent ID"     << "\t"
                   << "Name"          << "\t"
                   << '\n';
            newDeviceFile.close();
        }

        //Load Device device lines to table
        while (true)
        {
            QString line = textStream.readLine();
            if (line.isNull())
                break;
            else
                if (line.left(2)!="ID"){//skip the first line with headers

                    //Split the string with tabulation into a list
                    QStringList fieldList = line.split('\t');
                    QSqlQuery insertQuery;
                    querySQL = QLatin1String(R"(
                        INSERT INTO device (
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
                                        device_group_id )
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
                                        :device_group_id )
                    )");
                    insertQuery.prepare(querySQL);
                    insertQuery.bindValue(":device_id",fieldList[0].toInt());
                    insertQuery.bindValue(":device_parent_id",fieldList[1]);
                    insertQuery.bindValue(":device_name",fieldList[2]);
                    if(fieldList.size()>3){//prevent issues with files created in v1.22
                        insertQuery.bindValue(":device_type",fieldList[3]);
                        insertQuery.bindValue(":device_external_id",fieldList[4]);
                        insertQuery.bindValue(":device_path",fieldList[5]);
                        insertQuery.bindValue(":device_total_file_size",fieldList[6]);
                        insertQuery.bindValue(":device_total_file_count",fieldList[7]);
                        insertQuery.bindValue(":device_total_space",fieldList[8]);
                        insertQuery.bindValue(":device_free_space",fieldList[9]);
                        insertQuery.bindValue(":device_group_id",fieldList[11]);
                    }
                    insertQuery.exec();
                }
        }
        deviceFile.close();

        insertPhysicalStorageGroup();
    }
}
//--------------------------------------------------------------------------
void MainWindow::loadDeviceTableToTreeModel()
{
    //QList<QStandardItem*> firstColumnItems;

    //Synch data to device
    synchCatalogAndStorageValues();

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
    deviceTreeModel->setHorizontalHeaderLabels({tr("Name"),
                                      tr("Device Type"),
                                      tr("Active"),
                                      tr("ID"),
                                      tr("Parent ID"),
                                      tr("External ID"),
                                      tr("Number of files"),
                                      tr("Total Size"),
                                      tr("Total space"),
                                      tr("Free space"),
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
        qint64 size = query.value(6).toLongLong();
        qint64 number = query.value(7).toLongLong();
        qint64 total_space = query.value(8).toLongLong();
        qint64 free_space = query.value(9).toLongLong();
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
        rowItems << new QStandardItem(QString::number(total_space));
        rowItems << new QStandardItem(QString::number(free_space));
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
            else{
                // Skip this row and proceed to the next one
                qDebug() << "Parent item not found for ID:" << id;
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
        ui->Devices_treeView_DeviceList->header()->resizeSection(0, 350); //Name
        ui->Devices_treeView_DeviceList->header()->resizeSection(1, 100); //Type
        ui->Devices_treeView_DeviceList->header()->resizeSection(2,  25); //Active
        ui->Devices_treeView_DeviceList->header()->resizeSection(3,  25); //ID
        ui->Devices_treeView_DeviceList->header()->resizeSection(4,  25); //Parent ID
        ui->Devices_treeView_DeviceList->header()->resizeSection(5,  25); //External ID
        ui->Devices_treeView_DeviceList->header()->resizeSection(6, 100); //Number of Files
        ui->Devices_treeView_DeviceList->header()->resizeSection(7, 100); //Total File Size
        ui->Devices_treeView_DeviceList->header()->resizeSection(8, 100); //Total space
        ui->Devices_treeView_DeviceList->header()->resizeSection(9, 100); //Free space
        ui->Devices_treeView_DeviceList->header()->resizeSection(10, 25); //group ID

        if (ui->Devices_checkBox_DisplayFullTable->isChecked()) {
            ui->Devices_treeView_DeviceList->header()->showSection(1); //Type
            ui->Devices_treeView_DeviceList->header()->showSection(2); //Active
            ui->Devices_treeView_DeviceList->header()->showSection(3); //ID
            ui->Devices_treeView_DeviceList->header()->showSection(4); //Parent ID
            ui->Devices_treeView_DeviceList->header()->showSection(5); //External ID
            ui->Devices_treeView_DeviceList->header()->showSection(10); //group ID
        } else {
            ui->Devices_treeView_DeviceList->header()->hideSection(1); //Type
            ui->Devices_treeView_DeviceList->header()->hideSection(2); //Active
            ui->Devices_treeView_DeviceList->header()->hideSection(3); //ID
            ui->Devices_treeView_DeviceList->header()->hideSection(4); //Parent ID
            ui->Devices_treeView_DeviceList->header()->hideSection(5); //External ID
            ui->Devices_treeView_DeviceList->header()->hideSection(10); //group ID
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
void MainWindow::saveDeviceTableToFile(QString filePath)
{
    if (databaseMode == "Memory"){
        QFile deviceFile(filePath);

        //Get data
        QSqlQuery query;
        QString querySQL = QLatin1String(R"(
                                    SELECT *
                                    FROM device
                            )");
        query.prepare(querySQL);
        query.exec();

        //Write data
        if (deviceFile.open(QFile::WriteOnly | QFile::Text)) {

            QTextStream textStreamToFile(&deviceFile);

            //Prepare header line
            textStreamToFile << "ID"         << "\t"
                             << "Parent ID"  << "\t"
                             << "Name"       << "\t"
                             << "Type"       << "\t"
                             << "ExternalID" << "\t"
                             << "Path"       << "\t"
                             << "total_file_size" << "\t"
                             << "total_file_count"<< "\t"
                             << "total_space"<< "\t"
                             << "free_space" << "\t"
                             << "active"     << "\t"
                             << "groupID"    << "\t"
                             << '\n';

            //Iterate the records and generate lines
            while (query.next()) {
                const QSqlRecord record = query.record();
                for (int i=0, recCount = record.count() ; i<recCount ; ++i){
                    if (i>0)
                        textStreamToFile << '\t';
                    textStreamToFile << record.value(i).toString();
                    //qDebug()<<record.value(i).toString();
                }
                //Write the result in the file
                textStreamToFile << '\n';
            }
            deviceFile.close();
        }
        else{
            QMessageBox msgBox;
            msgBox.setWindowTitle("Katalog");
            msgBox.setText(tr("Error opening output file."));
            msgBox.setIcon(QMessageBox::Warning);
            msgBox.exec();
        }
    }
}
//--------------------------------------------------------------------------
void MainWindow::updateNumbers(int deviceID, QString storageType) {
    QSqlQuery query;
    QString querySQL;

    if(storageType=="Storage"  or storageType=="Virtual"){
        //device_total_file_count
        querySQL = QLatin1String(R"(
                            UPDATE device
                            SET device_total_file_count =
                                (SELECT SUM(device_total_file_count)
                                FROM device
                                WHERE device_parent_id = :device_id)
                            WHERE device_id = :device_id
                        )");
        query.prepare(querySQL);
        query.bindValue(":device_id", deviceID);
        query.exec();

        //device_total_file_size
        querySQL = QLatin1String(R"(
                            UPDATE device
                            SET device_total_file_size =
                                (SELECT SUM(device_total_file_size)
                                FROM device
                                WHERE device_parent_id = :device_id)
                            WHERE device_id = :device_id
                        )");
        query.prepare(querySQL);
        query.bindValue(":device_id", deviceID);
        query.exec();
    }

    if(storageType !="Storage" and storageType=="Virtual"){
        //device_total_space
        querySQL = QLatin1String(R"(
                            UPDATE device
                            SET device_total_space =
                                (SELECT SUM(device_total_space)
                                FROM device
                                WHERE device_parent_id = :device_id)
                            WHERE device_id = :device_id
                        )");
        query.prepare(querySQL);
        query.bindValue(":device_id",deviceID);
        query.exec();

        //device_free_space
        querySQL = QLatin1String(R"(
                            UPDATE device
                            SET device_free_space =
                                (SELECT SUM(device_free_space)
                                FROM device
                                WHERE device_parent_id = :device_id)
                            WHERE device_id = :device_id
                        )");
        query.prepare(querySQL);
        query.bindValue(":device_id",deviceID);
        query.exec();
    }
    saveDeviceTableToFile(deviceFilePath);
    loadDeviceTableToTreeModel();
}
//--------------------------------------------------------------------------
void MainWindow::synchCatalogAndStorageValues() {
    QSqlQuery query;
    QString querySQL;

    //Synch Catalog numbers
        querySQL = QLatin1String(R"(
                        UPDATE device
                        SET device_total_file_count = catalog.catalog_file_count,
                            device_total_file_size  = catalog.catalog_total_file_size,
                            device_active = catalog.catalog_source_path_is_active
                        FROM catalog
                        WHERE device.device_name = catalog.catalog_name;
                    )");
        query.prepare(querySQL);
        query.exec();

    //Synch Storage numbers
        querySQL = QLatin1String(R"(
                        UPDATE device
                        SET device_total_space = storage.storage_total_space,
                            device_free_space = storage.storage_free_space
                        FROM storage
                        WHERE device.device_external_id = storage.storage_id;
                    )");
        query.prepare(querySQL);
        query.exec();
}
//--------------------------------------------------------------------------
void MainWindow::updateAllNumbers() {

    synchCatalogAndStorageValues();

    QSqlQuery query;
    QString querySQL;
    //Get List of parent items
    querySQL = QLatin1String(R"(
                        WITH RECURSIVE device_tree AS (
                          SELECT
                            device_id,
                            device_parent_id
                          FROM device
                          WHERE device_id = :device_id

                          UNION ALL

                          SELECT
                            parent.device_id,
                            parent.device_parent_id
                          FROM device_tree child
                          JOIN device parent
                            ON parent.device_id = child.device_parent_id
                        )
                        SELECT device_id
                        FROM device_tree
                    )");
    query.prepare(querySQL);
    query.bindValue(":device_id",tempDevice->ID);
    query.exec();

    //Update parents
    while (query.next()) {
        int tempID = query.value(0).toInt();

        Device *tempCurrentDevice = new Device;
        tempCurrentDevice->ID = tempID;
        tempCurrentDevice->loadDevice();

        updateNumbers(tempCurrentDevice->ID, tempCurrentDevice->type);
    }

    saveDeviceTableToFile(deviceFilePath);
    loadDeviceTableToTreeModel();
}
//--------------------------------------------------------------------------
void MainWindow::insertPhysicalStorageGroup() {
    QSqlQuery query;
    QString querySQL;

    querySQL = QLatin1String(R"(
                            SELECT COUNT(*)
                            FROM device
                            WHERE device_id = 1
                        )");
    query.prepare(querySQL);
    query.exec();
    query.next();
    int result = query.value(0).toInt();

    if(result == 0){
        Device *newDeviceItem1 = new Device();
        newDeviceItem1->ID = 1;
        newDeviceItem1->parentID = 0;
        newDeviceItem1->name = tr(" Physical Group");
        newDeviceItem1->type = "Virtual";
        newDeviceItem1->externalID = 0;
        newDeviceItem1->groupID = 0;
        newDeviceItem1->insertDevice();

        Device *newDeviceItem2 = new Device();
        newDeviceItem2->ID = 2;
        newDeviceItem2->parentID = 1;
        newDeviceItem2->name = tr("Default Virtual group");
        newDeviceItem2->type = "Virtual";
        newDeviceItem2->externalID = 0;
        newDeviceItem2->groupID = 0;
        newDeviceItem2->insertDevice();
    }

    saveDeviceTableToFile(deviceFilePath);
    loadDeviceTableToTreeModel();
}
//--------------------------------------------------------------------------
void MainWindow::setDeviceTreeExpandState(bool toggle)
{
    //optionDeviceTreeExpandState values:  collapseAll or 2 =collapse / 0=exp.level0 / 1=exp.level1
    QString iconName = ui->Filters_pushButton_TreeExpandCollapse->icon().name();
    QSettings settings(settingsFilePath, QSettings:: IniFormat);

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
        qDebug() << "Error updating device table:" << query.lastError().text();
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

    qDebug() << "IDs shifted successfully by" << shiftAmount;
}
//--------------------------------------------------------------------------
void MainWindow::loadParentsList()
{   //Load valid list of parents to the panel comboBox. It enables a selection to change the parent of a device

    //Get data
    QSqlQuery query;
    QString querySQL = QLatin1String(R"(
                                SELECT v.device_name, v.device_id
                                FROM device v

                                WHERE device_type NOT IN ("Catalog","Storage")
                                AND device_id !=0
                                AND device_id !=:selected_device_id
                            )");
    // WHERE v.device_group_id = 0

    /*
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
*/

    //querySQL += " ORDER BY device_name ";
    query.prepare(querySQL);
    query.bindValue(":selected_device_id", tempDevice->ID);
    query.exec();

    //Load to comboboxes
    ui->Devices_comboBox_Parent->clear();
    ui->Devices_comboBox_Parent->addItem("Top level",query.value(1).toInt());

    while(query.next())
    {
        ui->Devices_comboBox_Parent->addItem(query.value(0).toString()+" ("+query.value(1).toString()+")",query.value(1).toInt());
    }

    // QList<QStandardItem*> firstColumnItems;

    //Get the list of device names  0 corresponds to the first column
    // Get values from the first two columns
    //QList<QVariant> column1Values;
    //QList<QVariant> column2Values;

    // for (int row = 0; row < deviceTreeModel->rowCount(); ++row) {
    //     for (int column = 0; column < deviceTreeModel->columnCount(); ++column) {
    //         QStandardItem *item = deviceTreeModel->item(row, column);
    //         if (item) {
    //             qDebug() << "Value at row" << row << ", column" << column << ":" << item->text();
    //         }
    //     }

    //     // Recursively call the function for child items
    //     getValuesFromModel(deviceTreeModel->item(row), deviceTreeModel->columnCount());
    // }


    // for (int row = 0; row < deviceTreeModel->rowCount(); ++row) {
    //     QModelIndex index1 = deviceTreeModel->index(row, 0);
    //     QModelIndex index2 = deviceTreeModel->index(row, 3);

    //     QString deviceName = deviceTreeModel->data(index1).toString();
    //     QString deviceID   = deviceTreeModel->data(index2).toString();

    //     ui->Devices_comboBox_Parent->addItem( deviceName +" (" + deviceID + ")", deviceID.toInt());
    // }


}
//--------------------------------------------------------------------------
void MainWindow::addDeviceVirtual()
{   //Create a new virtual device and add it to the selected Device

    Device *newDevice = new Device();
    newDevice->generateDeviceID();
    newDevice->parentID = tempDevice->ID;
    newDevice->name = tr("Virtual") + "_" + QString::number(newDevice->ID);
    newDevice->type = "Virtual";
    newDevice->externalID = 0;
    newDevice->groupID = tempDevice->groupID;
    newDevice->insertDevice();

    //Save data to file
    saveDeviceTableToFile(deviceFilePath);

    //Reload
    loadDeviceTableToTreeModel();
    loadParentsList();
}
//--------------------------------------------------------------------------
void MainWindow::addDeviceStorage()
{//Create a new storage device and add it to the selected Device

    //Create Device and related Storage under Physical group (ID=0)
    Device *newDevice = new Device();
    newDevice->generateDeviceID();
    newDevice->parentID = tempDevice->ID;
    newDevice->name = tr("Storage") + "_" + QString::number(newDevice->ID);
    newDevice->type = "Storage";
    newDevice->storage->generateID();
    newDevice->externalID = newDevice->storage->ID;
    newDevice->groupID = tempDevice->groupID;
    newDevice->insertDevice();
    newDevice->storage->name = newDevice->name; //REMOVE
    newDevice->storage->insertStorage();

    //Save data to file
    saveDeviceTableToFile(deviceFilePath);

    //Reload
    loadDeviceTableToTreeModel();
    loadParentsList();

    //Load table to model
    loadStorageTableToModel();

    //Save data to file and reload
    saveStorageTableToFile();
    loadStorageFileToTable();

    //Refresh
    loadStorageTableToModel();
    updateStorageSelectionStatistics();

    //Enable save button
    //ui->Storage_pushButton_New->setEnabled(true);

    loadParentsList();
}
//--------------------------------------------------------------------------
void MainWindow::editDevice()
{   //Display a panel to edit the device values

    //Refresh parent combobox
    loadParentsList();

    //Load panel and values
    ui->Devices_widget_Edit->setVisible(true);
    ui->Devices_lineEdit_Name->setText(tempDevice->name);
    ui->Devices_label_ItemDeviceTypeValue->setText(tempDevice->type);
    ui->Devices_label_ItemDeviceIDValue->setText(QString::number(tempDevice->ID));

    //Get parent and selected it the combobox
    Device *newDeviceItem = new Device();
    newDeviceItem->ID = tempDevice->parentID;
    newDeviceItem->loadDevice();
    ui->Devices_comboBox_Parent->setCurrentText(newDeviceItem->name+" ("+QString::number(newDeviceItem->ID)+")");
}
//--------------------------------------------------------------------------
void MainWindow::saveDevice()
{//Save the device values from the edit panel

    //Get the ID of the selected parent
    QVariant selectedData = ui->Devices_comboBox_Parent->currentData();
    tempDevice->parentID = selectedData.toInt();
    tempDevice->name = ui->Devices_lineEdit_Name->text();

    Device *parentDevice = new Device();
    parentDevice->ID = tempDevice->parentID;
    parentDevice->loadDevice();

    int newGroupID = parentDevice->groupID;
    if(parentDevice->ID == 0) //If the new parent is root the group_id should be 1 (0 is reserved for the Physical group)
        newGroupID=1;

    //Save name and parent ID
    QSqlQuery query;
    QString querySQL = QLatin1String(R"(
                            UPDATE  device
                            SET     device_name =:device_name,
                                    device_parent_id =:device_parent_id,
                                    device_group_id =:device_group_id
                            WHERE   device_id=:device_id
                                )");
    query.prepare(querySQL);
    query.bindValue(":device_id",        tempDevice->ID);
    query.bindValue(":device_name",      tempDevice->name);
    query.bindValue(":device_parent_id", tempDevice->parentID);
    query.bindValue(":device_group_id",  newGroupID);
    query.exec();

    //Also change the group_id of sub-devices
    Device *loopDevice = new Device();
    if(tempDevice->groupID != newGroupID){
        tempDevice->loadSubDeviceList();
        for(int i=0; i<tempDevice->deviceIDList.count(); i++) {
            loopDevice->ID = tempDevice->deviceIDList[i];
            loopDevice->loadDevice();
            loopDevice->groupID = newGroupID;
            loopDevice->saveDevice();
        }
    }

    //If device = Physical Storage
    if(tempDevice->type == "Storage"){

        QString currentStorageName = tempDevice->name; //selectedStorage->name;
        QString newStorageName     = ui->Devices_lineEdit_Name->text();

        //Update Storage name
        querySQL = QLatin1String(R"(
                                    UPDATE storage
                                    SET storage_name =:storage_name
                                    WHERE storage_id =:storage_id
                                )");

        QSqlQuery updateQuery;
        updateQuery.prepare(querySQL);
        updateQuery.bindValue(":storage_name", tempDevice->name);
        updateQuery.bindValue(":storage_id",   tempDevice->externalID);
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

    //Finalize
    ui->Devices_widget_Edit->hide();

    //Save data to file
    if (databaseMode == "Memory"){
        //Save file
        saveDeviceTableToFile(deviceFilePath);
    }

    //Reload
    loadDeviceTableToTreeModel();

    ui->Devices_pushButton_Edit->setEnabled(false);
}
//--------------------------------------------------------------------------
