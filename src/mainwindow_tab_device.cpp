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
void MainWindow::on_Devices_radioButton_DeviceTree_clicked()
{
    QSettings settings(collection->settingsFilePath, QSettings:: IniFormat);
    settings.setValue("Devices/DisplayContents", "Tree");

    loadDevicesView();
}
//--------------------------------------------------------------------------
void MainWindow::on_Devices_radioButton_StorageList_clicked()
{
    QSettings settings(collection->settingsFilePath, QSettings:: IniFormat);
    settings.setValue("Devices/DisplayContents", "Storage");

    loadDevicesView();
}
//--------------------------------------------------------------------------
void MainWindow::on_Devices_radioButton_CatalogList_clicked()
{
    QSettings settings(collection->settingsFilePath, QSettings:: IniFormat);
    settings.setValue("Devices/DisplayContents", "Catalogs");

    loadDevicesView();
}
//--------------------------------------------------------------------------
void MainWindow::on_Devices_pushButton_InsertRootLevel_clicked()
{
    Device *newDevice = new Device();
    newDevice->generateDeviceID();
    newDevice->type = "Virtual";
    newDevice->name = tr("Virtual Group") + "_" + QString::number(newDevice->ID);
    newDevice->parentID = 0;
    newDevice->externalID = 0;
    newDevice->groupID = 1; //only DeviceID 1 can be a top item in group 0 (Pyhsical group)
    newDevice->insertDevice();

    //Save data to file
    collection->saveDeviceTableToFile();

    //Reload
    loadDevicesTreeToModel("Filters");
    loadDevicesView();
    loadParentsList();
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
    //Define parent ID
    int parentID;
    if(selectedDevice->type =="Virtual")
        parentID = selectedDevice->ID;
    else
        parentID = 1;

    //add Storage device
    addDeviceStorage(parentID);
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
    if(arg1==0){
        ui->Devices_checkBox_DisplayCatalogs->hide();
        ui->Devices_widget_ReplaceCatalogsOption->show();
    }
    else{
        ui->Devices_widget_ReplaceCatalogsOption->hide();
        ui->Devices_checkBox_DisplayCatalogs->show();
    }

    loadDevicesView();
}
//--------------------------------------------------------------------------
void MainWindow::on_Devices_checkBox_DisplayCatalogs_stateChanged(int arg1)
{
    QSettings settings(collection->settingsFilePath, QSettings:: IniFormat);
    settings.setValue("Devices/DisplayCatalogs", arg1);
    loadDevicesView();
}
//--------------------------------------------------------------------------
void MainWindow::on_Devices_checkBox_DisplayPhysicalGroup_stateChanged(int arg1)
{
    QSettings settings(collection->settingsFilePath, QSettings:: IniFormat);
    settings.setValue("Devices/DisplayPhysicalGroup", arg1);
    if(arg1==0)
        ui->Devices_checkBox_DisplayVirtualGroups->setChecked(true);
    loadDevicesView();
}
//--------------------------------------------------------------------------
void MainWindow::on_Devices_checkBox_DisplayVirtualGroups_stateChanged(int arg1)
{
    QSettings settings(collection->settingsFilePath, QSettings:: IniFormat);
    settings.setValue("Devices/DisplayVirtualGroups", arg1);
    if(arg1==0)
        ui->Devices_checkBox_DisplayPhysicalGroup->setChecked(true);
    loadDevicesView();
}
//--------------------------------------------------------------------------
void MainWindow::on_Devices_checkBox_DisplayFullTable_stateChanged(int arg1)
{
    QSettings settings(collection->settingsFilePath, QSettings:: IniFormat);
    settings.setValue("Devices/DisplayFullTable", arg1);
    loadDevicesView();
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
            if ( ui->Settings_checkBox_KeepOneBackUp->isChecked() == true){
                backupFile(activeDevice->catalog->filePath);
            }
            //Update and report
            activeDevice->catalog->appVersion = currentVersion;
            reportAllUpdates(activeDevice,
                             activeDevice->updateDevice("update",
                                                        collection->databaseMode,
                                                        true,
                                                        collection->collectionFolder,
                                                        true),
                             "update");
            //Refresh
            collection->saveDeviceTableToFile();
            collection->saveStatiticsToFile();
            loadDevicesView();
        });

        QAction *menuDeviceAction5 = new QAction(QIcon::fromTheme("document-new"), tr("Explore"), this);
        deviceContextMenu.addAction(menuDeviceAction5);
        connect(menuDeviceAction5, &QAction::triggered, this, [this, deviceName]() {
            exploreDevice->ID = activeDevice->ID;
            exploreDevice->loadDevice();

            exploreSelectedFolderFullPath = exploreDevice->path;
            exploreSelectedDirectoryName  = exploreDevice->path;

            openCatalogToExplore();

            //Go to explore tab
            ui->tabWidget->setCurrentIndex(2);
        });

        QAction *menuDeviceAction2 = new QAction(QIcon::fromTheme("document-edit-sign"), tr("Edit"), this);
        deviceContextMenu.addAction(menuDeviceAction2);
        connect(menuDeviceAction2, &QAction::triggered, this, [this, deviceName]() {
            editDevice();
        });

        if(activeDevice->active==true){
            QAction *menuDeviceAction4 = new QAction(QIcon::fromTheme("view-statistics"), tr("Filelight"), this);
            deviceContextMenu.addAction(menuDeviceAction4);
            connect(menuDeviceAction4, &QAction::triggered, this, [this, deviceName]() {
                QProcess::startDetached("filelight", QStringList() << activeDevice->path);
            });
        }

        deviceContextMenu.addSeparator();

        if(activeDevice->groupID !=0){
            QAction *menuDeviceAction3 = new QAction(QIcon::fromTheme("edit-cut"), tr("Unassign this catalog"), this);
            deviceContextMenu.addAction(menuDeviceAction3);
            connect(menuDeviceAction3, &QAction::triggered, this, [this, deviceName]() {
                unassignPhysicalFromDevice(activeDevice->ID, activeDevice->parentID);
            });
        }
        else{
            QAction *menuDeviceAction4 = new QAction(QIcon::fromTheme("edit-delete"), tr("Delete this catalog"), this);
            deviceContextMenu.addAction(menuDeviceAction4);
            connect(menuDeviceAction4, &QAction::triggered, this, [this, deviceName]() {
                deleteDeviceItem();

            });
        }

        deviceContextMenu.exec(globalPos);
    }
    else if(activeDevice->type=="Storage"){
        QPoint globalPos = ui->Devices_treeView_DeviceList->mapToGlobal(pos);
        QMenu deviceContextMenu;

        QString deviceName = activeDevice->name;

        QAction *menuDeviceAction1 = new QAction(QIcon::fromTheme("media-playlist-repeat"), tr("Update"), this);
        deviceContextMenu.addAction(menuDeviceAction1);
        connect(menuDeviceAction1, &QAction::triggered, this, [this, deviceName]() {
            reportAllUpdates(activeDevice,
                             activeDevice->updateDevice("update",
                                                        collection->databaseMode,
                                                        true,
                                                        collection->collectionFolder,
                                                        true),
                             "list");
            collection->saveDeviceTableToFile();
            collection->saveStatiticsToFile();
            loadDevicesView();
        });

        QAction *menuDeviceAction2 = new QAction(QIcon::fromTheme("document-edit-sign"), tr("Edit"), this);
        deviceContextMenu.addAction(menuDeviceAction2);
        connect(menuDeviceAction2, &QAction::triggered, this, [this, deviceName]() {
            editDevice();
        });

        if(activeDevice->active==true){
            QAction *menuDeviceAction5 = new QAction(QIcon::fromTheme("gparted"), tr("Filelight"), this);
            deviceContextMenu.addAction(menuDeviceAction5);
            connect(menuDeviceAction5, &QAction::triggered, this, [this, deviceName]() {
                QProcess::startDetached("filelight", QStringList() << activeDevice->path);
            });
        }
        deviceContextMenu.addSeparator();

        if(activeDevice->groupID !=0){
            QAction *menuDeviceAction3 = new QAction(QIcon::fromTheme("edit-cut"), tr("Unassign this storage"), this);
            deviceContextMenu.addAction(menuDeviceAction3);

            connect(menuDeviceAction3, &QAction::triggered, this, [this, deviceName]() {
                unassignPhysicalFromDevice(activeDevice->ID, activeDevice->parentID);
            });
        }

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

        QAction *menuDeviceAction2 = new QAction(QIcon::fromTheme("document-edit-sign"), tr("Edit"), this);
        deviceContextMenu.addAction(menuDeviceAction2);
        connect(menuDeviceAction2, &QAction::triggered, this, [this, deviceName]() {
            editDevice();
        });

        QAction *menuDeviceAction3 = new QAction(QIcon::fromTheme("media-playlist-repeat"), tr("Update"), this);
        deviceContextMenu.addAction(menuDeviceAction3);
        connect(menuDeviceAction3, &QAction::triggered, this, [this, deviceName]() {
            reportAllUpdates(activeDevice,
                             activeDevice->updateDevice("update",
                                                        collection->databaseMode,
                                                        true,
                                                        collection->collectionFolder,
                                                        true),
                             "update");
            collection->saveDeviceTableToFile();
            collection->saveStatiticsToFile();
            loadDevicesView();
        });

        deviceContextMenu.addSeparator();

        QAction *menuDeviceAction1 = new QAction(QIcon::fromTheme("document-new"), tr("Add Virtual device"), this);
        deviceContextMenu.addAction(menuDeviceAction1);
        connect(menuDeviceAction1, &QAction::triggered, this, [this, deviceName]() {
            addDeviceVirtual();
        });

        if(activeDevice->groupID ==0){
            QAction *menuDeviceAction6 = new QAction(QIcon::fromTheme("document-new"), tr("Add Storage device"), this);
            deviceContextMenu.addAction(menuDeviceAction6);
            connect(menuDeviceAction6, &QAction::triggered, this, [this, deviceName]() {
                addDeviceStorage(activeDevice->ID);
            });
        }

        QAction *menuDeviceAction5 = new QAction(QIcon::fromTheme("document-new"), tr("Assign selected catalog"), this);
        deviceContextMenu.addAction(menuDeviceAction5);
        connect(menuDeviceAction5, &QAction::triggered, this, [this, deviceName]() {
            assignCatalogToDevice(selectedDevice, activeDevice);
        });

        if (selectedDevice->type != "Catalog") {
            menuDeviceAction5->setEnabled(false);
        }

        deviceContextMenu.addSeparator();

        QAction *menuDeviceAction4 = new QAction(QIcon::fromTheme("edit-delete"), tr("Delete"), this);
        deviceContextMenu.addAction(menuDeviceAction4);
        connect(menuDeviceAction4, &QAction::triggered, this, [this, deviceName]() {
            if(activeDevice->ID !=1)
                deleteDeviceItem();
            else{
                QMessageBox msgBox;
                msgBox.setWindowTitle("Katalog");
                msgBox.setText(QCoreApplication::translate("MainWindow",
                                                           "This Group is necessary to host Storage and Catalogs.<br/>"
                                                           "It cannot be deleted."
                                                           ) );
                msgBox.setIcon(QMessageBox::Information);
                msgBox.exec();
            }
        });

        deviceContextMenu.exec(globalPos);
    }
}
//--------------------------------------------------------------------------
void MainWindow::on_DevicesTreeViewDeviceListHeaderSortOrderChanged(){

    QSettings settings(collection->settingsFilePath, QSettings:: IniFormat);

    QHeaderView *devicesTreeHeader = ui->Devices_treeView_DeviceList->header();

    lastDevicesSortSection = devicesTreeHeader->sortIndicatorSection();
    lastDevicesSortOrder   = devicesTreeHeader->sortIndicatorOrder();

    settings.setValue("Devices/lastDevicesSortSection", lastDevicesSortSection);
    settings.setValue("Devices/lastDevicesSortOrder",   lastDevicesSortOrder);
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
void MainWindow::on_Storage_pushButton_UpdateStorage_clicked()
{
    reportAllUpdates(activeDevice,
                     activeDevice->updateDevice("update",
                                                collection->databaseMode,
                                                true,
                                                collection->collectionFolder,
                                                false),
                     "update");
    collection->saveDeviceTableToFile();
    collection->saveStatiticsToFile();
    loadDevicesView();
    editDevice();
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
    loadDevicesView();

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
    loadDevicesView();

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

        activeDevice->name = catalog_name;
        activeDevice->catalog->loadCatalog();

       // assignCatalogToDevice(catalog_name,device_id);

        collection->saveDeviceTableToFile();
        loadDevicesView();
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

        //Verif if catalog is not already assigned.
        QSqlQuery queryCurrentCatalogsExternalID;
        QString queryCurrentCatalogsExternalIDSQL = QLatin1String(R"(
                            SELECT COUNT(*)
                            FROM device
                            WHERE device_parent_id =:device_parent_id
                            AND device_external_id =:device_external_id
                        )");
        //AND device_external_id :=device_external_id
        queryCurrentCatalogsExternalID.prepare(queryCurrentCatalogsExternalIDSQL);
        queryCurrentCatalogsExternalID.bindValue(":device_parent_id", parentDevice->ID);
        queryCurrentCatalogsExternalID.bindValue(":device_external_id", catalogDevice->externalID);
        queryCurrentCatalogsExternalID.exec();
        queryCurrentCatalogsExternalID.next();
        bool catalogAlreadyAssigned = queryCurrentCatalogsExternalID.value(0).toInt() > 0;

        if(catalogAlreadyAssigned==true){
            QMessageBox msgBox;
            msgBox.setWindowTitle("Katalog");
            msgBox.setText(QCoreApplication::translate("MainWindow",
                                                       "The catalog is already assigned to this Virtual device."
                                                       ));
            msgBox.setIcon(QMessageBox::Information);
            msgBox.exec();
            return;
        }
        else{
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
            query.bindValue(":device_name", catalogDevice->name);
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
            loadDevicesView();
        }
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
        loadDevicesView();
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
            loadDevicesView();
        }
    }
}
//--------------------------------------------------------------------------
void MainWindow::deleteDeviceItem()
{
    activeDevice->deleteDevice(true);

    Device parentDevice;
    parentDevice.ID = activeDevice->parentID;
    parentDevice.loadDevice();
    parentDevice.updateNumbersFromChildren();
    parentDevice.updateParentsNumbers();

    //Save data to files
    collection->saveDeviceTableToFile();
    collection->saveStorageTableToFile();
    if(activeDevice->type =="Catalog")
        collection->deleteCatalogFile(activeDevice);

    //Reload data to models
    updateStorageSelectionStatistics();
    loadDevicesTreeToModel("Filters");
    loadDevicesView();
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
void MainWindow::updateNumbers() {

    activeDevice->updateNumbersFromChildren();

    collection->saveDeviceTableToFile();
    loadDevicesView();
}
//--------------------------------------------------------------------------
void MainWindow::updateAllNumbers()
{
    activeDevice->updateParentsNumbers();

    collection->saveDeviceTableToFile();
    loadDevicesView();
}
//--------------------------------------------------------------------------
void MainWindow::setDeviceTreeExpandState(bool toggle)
{
    //DEV:
    if (developmentMode==false)
        deviceTreeExpandState = 0;
    else{
        //Count the number of tree level from root
        QMap<int, QList<int>> deviceTree;
        QSqlQuery query("SELECT device_id, device_parent_id FROM device");
        while (query.next()) {
            int deviceId = query.value(0).toInt();
            int parentId = query.value(1).toInt();
            deviceTree[parentId].append(deviceId);
        }
        int treeLevels = countTreeLevels(deviceTree, 0);

        //optionDeviceTreeExpandState values:  collapseAll / 0 / 2 to x levels / x+1 last level
        QString iconName = ui->Devices_pushButton_TreeExpandCollapse->icon().name();
        QSettings settings(collection->settingsFilePath, QSettings:: IniFormat);

        if (toggle==true){

            if ( iconName=="collapse-all"/*deviceTreeExpandState == 2*/ ){
                //collapsed > expand first level
                ui->Devices_pushButton_TreeExpandCollapse->setIcon(QIcon::fromTheme("expand-all"));
                deviceTreeExpandState =-1;
                ui->Devices_treeView_DeviceList->collapseAll();
                settings.setValue("Devices/deviceTreeExpandState", deviceTreeExpandState);
            }
            else if ( iconName=="expand-all" /*deviceTreeExpandState == 0 */){
                //expanded first level > expand to second level
                if(deviceTreeExpandState == treeLevels-3){
                    ui->Devices_pushButton_TreeExpandCollapse->setIcon(QIcon::fromTheme("collapse-all"));
                    deviceTreeExpandState +=1;
                    ui->Devices_treeView_DeviceList->expandToDepth(deviceTreeExpandState);
                    settings.setValue("Devices/deviceTreeExpandState", deviceTreeExpandState);
                }
                else{
                    deviceTreeExpandState +=1;
                    ui->Devices_treeView_DeviceList->expandToDepth(deviceTreeExpandState);
                    settings.setValue("Devices/deviceTreeExpandState", deviceTreeExpandState);
                }

            }
            else if ( iconName=="expand-all" /* deviceTreeExpandState == 1 */){
                //expanded second level > collapse
                ui->Devices_pushButton_TreeExpandCollapse->setIcon(QIcon::fromTheme("collapse-all"));
                ui->Devices_treeView_DeviceList->expandToDepth(treeLevels);
                settings.setValue("Devices/deviceTreeExpandState", deviceTreeExpandState);
            }
        }
        else
        {
            if ( deviceTreeExpandState == 0 ){
                ui->Devices_pushButton_TreeExpandCollapse->setIcon(QIcon::fromTheme("expand-all"));
                ui->Devices_treeView_DeviceList->collapseAll();
                ui->Devices_treeView_DeviceList->expandToDepth(deviceTreeExpandState);
            }
            // else if ( deviceTreeExpandState == 1 ){
            //     ui->Devices_pushButton_TreeExpandCollapse->setIcon(QIcon::fromTheme("collapse-all"));
            //     ui->Devices_treeView_DeviceList->collapseAll();
            //     ui->Devices_treeView_DeviceList->expandToDepth(deviceTreeExpandState);
            // }
            // else{
            //     ui->Devices_pushButton_TreeExpandCollapse->setIcon(QIcon::fromTheme("expand-all"));
            //     ui->Devices_treeView_DeviceList->collapseAll();
            // }
            else if ( deviceTreeExpandState != treeLevels-3 ){
                ui->Devices_pushButton_TreeExpandCollapse->setIcon(QIcon::fromTheme("collapse-all"));
                ui->Devices_treeView_DeviceList->collapseAll();
                ui->Devices_treeView_DeviceList->expandToDepth(deviceTreeExpandState);
            }
            else{
                ui->Devices_pushButton_TreeExpandCollapse->setIcon(QIcon::fromTheme("expand-all"));
                ui->Devices_treeView_DeviceList->collapseAll();
            }
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
{//Load valid list of parents to the panel comboBox. It enables a selection to change the parent of a device.

    //Get data
    //A device can only be moved within its group (0= Physical, 1= Virtual)
    QSqlQuery query;
    QString querySQL = QLatin1String(R"(
                                SELECT device_name, device_id
                                FROM device
                                WHERE device_id !=0
                                AND device_id !=:selected_device_id
                                AND device_group_id =:device_group_id
                            )");

    if(activeDevice->type == "Catalog"){
        querySQL += QLatin1String(R"(AND device_type NOT IN ("Catalog"))");
    }
    else //if(activeDevice->type == "Virtual")
    {
        querySQL += QLatin1String(R"(AND device_type NOT IN ("Catalog","Storage"))");
    }

    query.prepare(querySQL);
    query.bindValue(":selected_device_id", activeDevice->ID);
    query.bindValue(":device_group_id", activeDevice->groupID);
    query.exec();

    //Load to comboboxes
    ui->Devices_comboBox_Parent->clear();
    ui->Devices_comboBox_Parent->addItem("Top level", query.value(1).toInt());

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
    loadDevicesTreeToModel("Filters");
    loadDevicesView();
    loadParentsList();

    //Make it the activeDevice and edit
    activeDevice->ID = newDevice->ID;
    activeDevice->loadDevice();
    editDevice();
}
//--------------------------------------------------------------------------
void MainWindow::addDeviceStorage(int parentID)
{//Create a new storage device, and add it to the selected Device

    //Create Device and related Storage under Physical group (ID=0)
    Device *newDevice = new Device();
    newDevice->generateDeviceID();
    newDevice->parentID = parentID;
    newDevice->name = tr("Storage") + "_" + QString::number(newDevice->ID);
    newDevice->type = "Storage";
    newDevice->storage->generateID();
    newDevice->externalID = newDevice->storage->ID;
    newDevice->groupID = 0;
    newDevice->insertDevice();
    newDevice->storage->name = newDevice->name; //REMOVE
    newDevice->storage->insertStorage();

    //Save data to file
    collection->saveDeviceTableToFile();

    //Reload
    loadDevicesTreeToModel("Filters");
    loadDevicesView();
    loadParentsList();

    //Load table to model
    //loadStorageTableToModel();

    //Save data to file and reload
    collection->saveStorageTableToFile();
    collection->loadStorageFileToTable();

    //Refresh
    //loadStorageTableToModel();
    updateStorageSelectionStatistics();

    //Make it the activeDevice and edit
    activeDevice->ID = newDevice->ID;
    activeDevice->loadDevice();
    loadDevicesTreeToModel("Filters");
    loadDevicesView();
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

    if(activeDevice->type =="Catalog"){
        ui->Devices_widget_EditCatalogFields->show();
        ui->Devices_widget_EditStorageFields->hide();

        //ui->Catalogs_label_NameDisplay->setText(activeDevice->name);
        //ui->Catalogs_label_Path->setText(activeDevice->catalog->sourcePath);
        ui->Catalogs_comboBox_FileType->setCurrentText(activeDevice->catalog->fileType);
        //ui->Catalogs_label_StorageDisplay->setText(activeDevice->catalog->storageName);
        ui->Catalogs_checkBox_IncludeHidden->setChecked(activeDevice->catalog->includeHidden);
        ui->Catalogs_checkBox_IncludeMetadata->setChecked(activeDevice->catalog->includeMetadata);
        //DEV: ui->Catalogs_checkBox_isFullDevice->setChecked(selectedCatalogIsFullDevice);
    }
    else if(activeDevice->type =="Storage"){
        ui->Devices_widget_EditStorageFields->show();
        ui->Devices_widget_EditCatalogFields->hide();

        ui->Storage_lineEdit_Panel_ID->setText(QString::number(activeDevice->storage->ID));
        ui->Storage_label_NameDisplay->setText(activeDevice->storage->name);
        ui->Storage_lineEdit_Panel_Type->setText(activeDevice->storage->type);
        ui->Storage_label_Panel_Path->setText(activeDevice->storage->path);
        ui->Storage_lineEdit_Panel_Label->setText(activeDevice->storage->label);
        ui->Storage_lineEdit_Panel_FileSystem->setText(activeDevice->storage->fileSystem);

        ui->Storage_lineEdit_Panel_Total->setText(QString::number(activeDevice->totalSpace));
        ui->Storage_lineEdit_Panel_Free->setText(QString::number(activeDevice->freeSpace));
        ui->Storage_label_Panel_TotalSpace->setText(QLocale().formattedDataSize(activeDevice->totalSpace));
        ui->Storage_label_Panel_FreeSpace->setText(QLocale().formattedDataSize(activeDevice->freeSpace));

        ui->Storage_lineEdit_Panel_BrandModel->setText(activeDevice->storage->brand_model);
        ui->Storage_lineEdit_Panel_SerialNumber->setText(activeDevice->storage->serialNumber);
        ui->Storage_lineEdit_Panel_BuildDate->setText(activeDevice->storage->buildDate);
        ui->Storage_lineEdit_Panel_ContentType->setText(activeDevice->storage->contentType);
        ui->Storage_lineEdit_Panel_Container->setText(activeDevice->storage->container);
        ui->Storage_lineEdit_Panel_Comment->setText(activeDevice->storage->comment);

        displayStoragePicture();
    }
    else{
        ui->Devices_widget_EditStorageFields->hide();
        ui->Devices_widget_EditCatalogFields->hide();
    }

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

    //Keep previous values
    QString previousPath = activeDevice->path;
    QString previousName = activeDevice->name;
    Device previousParentDevice;
    previousParentDevice.ID = activeDevice->parentID;
    previousParentDevice.loadDevice();

    //Get new name and parentID
    activeDevice->parentID = ui->Devices_comboBox_Parent->currentData().toInt();
    activeDevice->name = ui->Devices_lineEdit_Name->text();
    if (previousName != activeDevice->name and activeDevice->verifyDeviceNameExists() and activeDevice->type=="Catalog"){
        QMessageBox msgBox;
        msgBox.setWindowTitle("Katalog");
        msgBox.setText( tr("There is already a Catalog with this name:<br/><b>").arg(activeDevice->type)
                       + activeDevice->name
                       + "</b><br/><br/>"+tr("Choose a different name and try again."));
        msgBox.setIcon(QMessageBox::Critical);
        msgBox.exec();
        return;
    }

    //Get new path: remove the / at the end if any, except for / alone (root directory in Linux)
    activeDevice->path = ui->Devices_lineEdit_Path->text();
    int pathLength = activeDevice->path.length();
    if (activeDevice->path !="" and activeDevice->path !="/" and QVariant(activeDevice->path.at(pathLength-1)).toString()=="/") {
         activeDevice->path.remove(pathLength-1,1);
    }
    activeDevice->catalog->sourcePath = activeDevice->path;

    Device newParentDevice;
    newParentDevice.ID = activeDevice->parentID;
    newParentDevice.loadDevice();

    if (activeDevice->type == "Catalog" and activeDevice->groupID == 0 and newParentDevice.type !="Storage"){
        QMessageBox msgBox;
        msgBox.setWindowTitle("Katalog");
        msgBox.setText( tr("A Catalog in the Physical group can only be set under a Storage or this group. Select a Storage in this group.<br/><br/>"
                          "To use this catalog under a device in a virtual group, use the Assign command.<b>")
                       );
        msgBox.setIcon(QMessageBox::Warning);
        msgBox.exec();
        return;
    }

    //Update groupIDs
        //From the new parent device
        int newGroupID = newParentDevice.groupID;
        if(newParentDevice.ID == 0) //If the new parent is root, the group_id should be 1 (0 is reserved for the Physical group)
            newGroupID=1;

        //Also change the group_id of sub-devices
        Device loopDevice;
        if(activeDevice->groupID != newGroupID){
            for(int i=0; i<activeDevice->deviceIDList.count(); i++) {
                loopDevice.ID = activeDevice->deviceIDList[i];
                loopDevice.loadDevice();
                loopDevice.groupID = newGroupID;
                loopDevice.saveDevice();
            }
        }

    //Save device
    activeDevice->groupID = newGroupID;
    activeDevice->totalSpace = ui->Storage_lineEdit_Panel_Total->text().toLongLong();
    activeDevice->freeSpace  = ui->Storage_lineEdit_Panel_Free->text().toLongLong();
    activeDevice->saveDevice();
    collection->saveDeviceTableToFile();

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

        //loadCatalogsTableToModel();
        updateCatalogsScreenStatistics();

        //Save data to file
        if (collection->databaseMode=="Memory"){
            activeDevice->catalog->storageName = newParentDevice.name;
            activeDevice->catalog->saveCatalog();
            activeDevice->catalog->updateCatalogFileHeaders(collection->databaseMode);
        }

        activeDevice->catalog->renameCatalogFile(activeDevice->name);

        //Update the list of files if the changes impact the contents (i.e. path, file type, hidden)
        if (activeDevice->path != previousPath)
        {
            int updatechoice = QMessageBox::warning(this, "Katalog",
                                                    tr("Update the catalog contents based on the new path?\n")
                                                    , QMessageBox::Yes
                                                        | QMessageBox::No);
            if ( updatechoice == QMessageBox::Yes){
                reportAllUpdates(activeDevice,
                                 activeDevice->updateDevice("update",
                                                            collection->databaseMode,
                                                            true,
                                                            collection->collectionFolder,
                                                            true),
                                 "update");
            }
        }

        saveCatalogChanges();

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

        //loadStorageTableToModel();
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
                    loopCatalog.name = listCatalogQuery.value(0).toString();
                    loopCatalog.catalog->loadCatalog();
                    loopCatalog.catalog->storageName = newStorageName;
                    loopCatalog.catalog->updateCatalogFileHeaders(collection->databaseMode);
                }

                //Refresh
                if(collection->databaseMode=="Memory")
                    collection->loadCatalogFilesToTable();
            }
        }

        //Save changes to selected Storage device from the edition panel

        //Update storage
        QSqlQuery queryStorage;
        QString queryStorageSQL = QLatin1String(R"(
                                    UPDATE storage
                                    SET storage_id = :new_storage_id,
                                        storage_type =:storage_type,
                                        storage_location =:storage_location,
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

        queryStorage.prepare(queryStorageSQL);
        queryStorage.bindValue(":new_storage_id",        ui->Storage_lineEdit_Panel_ID->text());
        queryStorage.bindValue(":storage_type",          ui->Storage_lineEdit_Panel_Type->text());
        queryStorage.bindValue(":storage_label",         ui->Storage_lineEdit_Panel_Label->text());
        queryStorage.bindValue(":storage_file_system",   ui->Storage_lineEdit_Panel_FileSystem->text());
        queryStorage.bindValue(":storage_brand_model",   ui->Storage_lineEdit_Panel_BrandModel->text());
        queryStorage.bindValue(":storage_serial_number", ui->Storage_lineEdit_Panel_SerialNumber->text());
        queryStorage.bindValue(":storage_build_date",    ui->Storage_lineEdit_Panel_BuildDate->text());
        queryStorage.bindValue(":storage_content_type",  ui->Storage_lineEdit_Panel_ContentType->text());
        queryStorage.bindValue(":storage_container",     ui->Storage_lineEdit_Panel_Container->text());
        queryStorage.bindValue(":storage_comment",       ui->Storage_lineEdit_Panel_Comment->text());
        queryStorage.bindValue(":storage_id",            activeDevice->storage->ID);
        queryStorage.exec();

        //loadStorageTableToModel();
        updateStorageSelectionStatistics();

        //Save data to file
        collection->saveStorageTableToFile();
    }

    //Update previous and new parent device values
    previousParentDevice.updateNumbersFromChildren();
    previousParentDevice.updateParentsNumbers();
    newParentDevice.updateNumbersFromChildren();
    newParentDevice.updateParentsNumbers();

    //Refresh Filters tree if device name changed
    if(previousName != activeDevice->name)
        loadDevicesTreeToModel("Filters");

    //Finalize
    ui->Devices_widget_Edit->hide();

    //Save data to file
    collection->saveDeviceTableToFile();

    //Reload
    loadDevicesView();
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

    //For Storage and Catalog devices
        //Get the list of devices
        QSqlQuery query;
        QString querySQL = QLatin1String(R"(
                                            SELECT device_id
                                            FROM   device
                                            WHERE  device_type = 'Storage' OR device_type = 'Catalog'
                                    )");
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
//--- View -----------------------------------------------------------------
void MainWindow::loadDevicesView(){
    if(ui->Devices_radioButton_StorageList->isChecked()==true){
        loadDevicesStorageToModel();
        ui->Devices_widget_TreeOptions->hide();
        ui->Devices_widget_CatalogStats->hide();
        ui->Devices_widget_StorageStats->show();
        //ui->Devices_treeView_DeviceList->QTreeView::sortByColumn(lastDevicesSortSection,Qt::SortOrder(lastDevicesSortOrder));
    }
    else if(ui->Devices_radioButton_CatalogList->isChecked()==true){
        loadDevicesCatalogToModel();
        ui->Devices_widget_TreeOptions->hide();
        ui->Devices_widget_CatalogStats->show();
        ui->Devices_widget_StorageStats->hide();
        //ui->Devices_treeView_DeviceList->QTreeView::sortByColumn(lastDevicesSortSection,Qt::SortOrder(lastDevicesSortOrder));
    }
    else{
        loadDevicesTreeToModel("Devices");
        ui->Devices_widget_TreeOptions->show();
        ui->Devices_widget_CatalogStats->hide();
        ui->Devices_widget_StorageStats->hide();
        //ui->Devices_treeView_DeviceList->QTreeView::sortByColumn(lastDevicesSortSection,Qt::SortOrder(lastDevicesSortOrder));
    }
}
//--------------------------------------------------------------------------
void MainWindow::loadDevicesTreeToModel(QString targetTreeModel)
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
                            device_group_id,
                            device_date_updated
                    FROM  device
                )");

    if (ui->Devices_checkBox_DisplayPhysicalGroup->isChecked() == true and
        ui->Devices_checkBox_DisplayVirtualGroups->isChecked() == false) {

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
                        device_group_id,
                        device_date_updated
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
                        child.device_group_id,
                        child.device_date_updated
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
                        device_group_id,
                        device_date_updated
                    FROM device_tree
                )");
    }
    else if (ui->Devices_checkBox_DisplayPhysicalGroup->isChecked() == false and
             ui->Devices_checkBox_DisplayVirtualGroups->isChecked() == true) {

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
                        device_group_id,
                        device_date_updated
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
                        child.device_group_id,
                        child.device_date_updated
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
                        device_group_id,
                        device_date_updated
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
    QStandardItemModel *devicesTreeModel = new QStandardItemModel();

    devicesTreeModel->setHorizontalHeaderLabels({
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
                                                tr("Date updated"),
                                                tr("Path"),
                                                tr("Group ID"),
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
        QString dateTimeUpdated = query.value(12).toString();

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
        rowItems << new QStandardItem(dateTimeUpdated);
        rowItems << new QStandardItem(path);
        rowItems << new QStandardItem(QString::number(groupID));

        //Get the item representing the name, and map the parent ID
        QStandardItem* item = rowItems.at(0);
        QStandardItem* parentItem = itemMap.value(parentId);

        //Add top-level items directly to the model
        if (parentId == 0) {
            devicesTreeModel->appendRow(rowItems);
        }
        //else append the row to the parent item
        else{
            if (parentItem) {
                parentItem->appendRow(rowItems);
            }
            else if(id!=0){
                // Skip this row and proceed to the next one
                qDebug() << "loadDevicesTreeToModel - Parent item not found for ID:" << id;
                continue;
            }
        }

        // Store the item in the map for future reference
        itemMap.insert(id, item);
    }

    if(targetTreeModel=="Devices" or targetTreeModel=="All"){
        //Load Model to treeview (Devices tab)
        DeviceTreeView *deviceTreeViewForDeviceTab = new DeviceTreeView(this);
        deviceTreeViewForDeviceTab->setSourceModel(devicesTreeModel);
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
        ui->Devices_treeView_DeviceList->header()->resizeSection(11, 150); //date updated
        ui->Devices_treeView_DeviceList->header()->setSectionResizeMode(12, QHeaderView::ResizeToContents); //Path
        ui->Devices_treeView_DeviceList->header()->resizeSection(13,  25); //Group ID

        ui->Devices_treeView_DeviceList->header()->showSection( 8); //Used space
        ui->Devices_treeView_DeviceList->header()->showSection( 9); //Free space
        ui->Devices_treeView_DeviceList->header()->showSection(10); //Total space

        if (ui->Devices_checkBox_DisplayFullTable->isChecked()) {
            ui->Devices_treeView_DeviceList->header()->showSection(1); //Type
            ui->Devices_treeView_DeviceList->header()->showSection(2); //Active
            ui->Devices_treeView_DeviceList->header()->showSection(3); //ID
            ui->Devices_treeView_DeviceList->header()->showSection(4); //Parent ID
            ui->Devices_treeView_DeviceList->header()->showSection(5); //External ID
            ui->Devices_treeView_DeviceList->header()->showSection(13); //Group ID
        } else {
            ui->Devices_treeView_DeviceList->header()->hideSection(1); //Type
            ui->Devices_treeView_DeviceList->header()->hideSection(2); //Active
            ui->Devices_treeView_DeviceList->header()->hideSection(3); //ID
            ui->Devices_treeView_DeviceList->header()->hideSection(4); //Parent ID
            ui->Devices_treeView_DeviceList->header()->hideSection(5); //External ID
            ui->Devices_treeView_DeviceList->header()->hideSection(13); //Group ID
        }

        ui->Devices_treeView_DeviceList->expandAll();
    }
    if(targetTreeModel=="Filters" or targetTreeModel=="All"){
        QStandardItemModel *filtersTreeModel = new QStandardItemModel();
        filtersTreeModel = devicesTreeModel;
        //Load Model to treeview (Filters/Device tree)
        DeviceTreeView *deviceTreeViewForSelectionPanel = new DeviceTreeView(this);
        deviceTreeViewForSelectionPanel->setSourceModel(filtersTreeModel);
        ui->Filters_treeView_Devices->setModel(deviceTreeViewForSelectionPanel);
        ui->Filters_treeView_Devices->sortByColumn(0,Qt::AscendingOrder);
        ui->Filters_treeView_Devices->header()->setSectionResizeMode(QHeaderView::ResizeToContents);
        ui->Filters_treeView_Devices->hideColumn(1);
        ui->Filters_treeView_Devices->hideColumn(2);
        ui->Filters_treeView_Devices->hideColumn(3);
        ui->Filters_treeView_Devices->setColumnWidth(2,0);
        ui->Filters_treeView_Devices->collapseAll();
        ui->Filters_treeView_Devices->header()->hide();

        //Hide all columns but the first
        for (int var = 1; var < deviceTreeViewForSelectionPanel->columnCount(); ++var) {
            ui->Filters_treeView_Devices->header()->hideSection(var);
        }

        //Restore Expand or Collapse Device Tree
        setTreeExpandState(false);
        setDeviceTreeExpandState(false);
        ui->Filters_treeView_Devices->setModel(deviceTreeViewForSelectionPanel);
        ui->Filters_treeView_Devices->expandAll();
    }
}
//--------------------------------------------------------------------------
void MainWindow::loadDevicesStorageToModel(){
    //Refresh active state
    updateAllDeviceActive();

    //Retrieve device hierarchy
    QSqlQuery loadStorageQuery;
    QString loadStorageQuerySQL;

    loadStorageQuerySQL = QLatin1String(R"(
                    SELECT  device_id,
                            0,
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
                            device_date_updated,
                            storage_type          ,
                            storage_label         ,
                            storage_file_system   ,
                            storage_brand_model   ,
                            storage_serial_number ,
                            storage_build_date    ,
                            storage_content_type  ,
                            storage_container     ,
                            storage_comment
                    FROM  device d
                    JOIN  storage s ON d.device_external_id = s.storage_id
                    WHERE device_type = 'Storage'
                )");

    if ( selectedDevice->ID == 0 ){
        //No filter
    }
    else if ( selectedDevice->type == "Catalog" ){
        loadStorageQuerySQL += " AND device_id =:device_parent_id";
    }
    else{
        QString prepareSQL = QLatin1String(R"(
                                    AND d.device_id IN (
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

    loadStorageQuerySQL +=" ORDER BY device_type DESC, device_parent_id ASC, device_id ASC ";
    loadStorageQuery.prepare(loadStorageQuerySQL);
    loadStorageQuery.bindValue(":device_id", selectedDevice->ID);
    loadStorageQuery.bindValue(":device_parent_id", selectedDevice->parentID);
    loadStorageQuery.exec();

    //Prepare the tree model: headers
    QStandardItemModel *storageTreeModel = new QStandardItemModel();
    storageTreeModel->setHorizontalHeaderLabels({
                                                 tr("Name"),
                                                 tr("Device Type"),
                                                 tr("Active"),
                                                 tr("ID"),
                                                 tr("Parent ID"),
                                                 tr("Storage ID"),
                                                 tr("Number of files"),
                                                 tr("Total Size"),
                                                 tr("Used space"),
                                                 tr("Free space"),
                                                 tr("Total space"),
                                                 tr("Date updated"),
                                                 tr("Path"),
                                                 tr("Group ID"),
                                                 tr("Type"),
                                                 tr("Label"),
                                                 tr("FileSystem"),
                                                 tr("Brand/Model"),
                                                 tr("Serial Number"),
                                                 tr("Build Date"),
                                                 tr("Content Type"),
                                                 tr("Container"),
                                                 tr("Comment"),
                                                 "" });

    //Create a map to store items by ID for easy access
    QMap<int, QStandardItem*> itemMap;

    //Populate model
    while (loadStorageQuery.next()) {

        //Get data for the item
        int id = loadStorageQuery.value(0).toInt();
        int parentId = loadStorageQuery.value(1).toInt();
        QString name = loadStorageQuery.value(2).toString();
        QString type = loadStorageQuery.value(3).toString();
        int externalId = loadStorageQuery.value(4).toInt();
        QString path = loadStorageQuery.value(5).toString();
        qint64 size = loadStorageQuery.value(6).toLongLong();
        qint64 number = loadStorageQuery.value(7).toLongLong();
        qint64 total_space = loadStorageQuery.value(8).toLongLong();
        qint64 free_space = loadStorageQuery.value(9).toLongLong();
        qint64 used_space = total_space - free_space;
        bool isActive = loadStorageQuery.value(10).toBool();
        int groupID = loadStorageQuery.value(11).toBool();
        QString dateTimeUpdated = loadStorageQuery.value(12).toString();

        QString storage_type = loadStorageQuery.value(13).toString();
        QString storage_label = loadStorageQuery.value(14).toString();
        QString storage_file_system = loadStorageQuery.value(15).toString();
        QString storage_brand_model = loadStorageQuery.value(16).toString();
        QString storage_serial_number = loadStorageQuery.value(17).toString();
        QString storage_build_date = loadStorageQuery.value(18).toString();
        QString storage_content_type = loadStorageQuery.value(19).toString();
        QString storage_container = loadStorageQuery.value(20).toString();
        QString storage_comment = loadStorageQuery.value(21).toString();

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
        rowItems << new QStandardItem(dateTimeUpdated);
        rowItems << new QStandardItem(path);
        rowItems << new QStandardItem(QString::number(groupID));
        rowItems << new QStandardItem(storage_type);
        rowItems << new QStandardItem(storage_label);
        rowItems << new QStandardItem(storage_file_system);
        rowItems << new QStandardItem(storage_brand_model);
        rowItems << new QStandardItem(storage_serial_number);
        rowItems << new QStandardItem(storage_build_date);
        rowItems << new QStandardItem(storage_content_type);
        rowItems << new QStandardItem(storage_container);
        rowItems << new QStandardItem(storage_comment);

        //Get the item representing the name, and map the parent ID
        QStandardItem* item = rowItems.at(0);
        QStandardItem* parentItem = itemMap.value(parentId);

        //Add top-level items directly to the model
        if (parentId == 0) {
            storageTreeModel->appendRow(rowItems);
        }
        //else append the row to the parent item
        else{
            if (parentItem) {
                parentItem->appendRow(rowItems);
            }
            else if(id!=0){
                // Skip this row and proceed to the next one
                qDebug() << "loadDevicesTreeToModel - Parent item not found for ID:" << id;
                continue;
            }
        }

        // Store the item in the map for future reference
        itemMap.insert(id, item);
    }

    //Load Model to treeview (Virtual tab)
    DeviceTreeView *deviceTreeViewForDeviceTab = new DeviceTreeView(this);
    deviceTreeViewForDeviceTab->setSourceModel(storageTreeModel);
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
    ui->Devices_treeView_DeviceList->header()->resizeSection(11, 150); //date updated
    ui->Devices_treeView_DeviceList->header()->setSectionResizeMode(12, QHeaderView::ResizeToContents); //Path
    ui->Devices_treeView_DeviceList->header()->resizeSection(13,  25); //Group ID

    ui->Devices_treeView_DeviceList->header()->hideSection( 1); //Type
    ui->Devices_treeView_DeviceList->header()->hideSection( 3); //ID
    ui->Devices_treeView_DeviceList->header()->hideSection( 4); //Parent ID
    ui->Devices_treeView_DeviceList->header()->showSection( 8); //Used space
    ui->Devices_treeView_DeviceList->header()->showSection( 9); //Free space
    ui->Devices_treeView_DeviceList->header()->showSection(10); //Total space

    if (ui->Devices_checkBox_DisplayFullTable->isChecked()) {
        ui->Devices_treeView_DeviceList->header()->showSection(2); //Active
        ui->Devices_treeView_DeviceList->header()->showSection(5); //External ID
        ui->Devices_treeView_DeviceList->header()->showSection(13); //Group ID
    } else {
        ui->Devices_treeView_DeviceList->header()->hideSection(2); //Active
        ui->Devices_treeView_DeviceList->header()->hideSection(5); //External ID
        ui->Devices_treeView_DeviceList->header()->hideSection(13); //Group ID
    }

    ui->Devices_treeView_DeviceList->expandAll();
}
//--------------------------------------------------------------------------
void MainWindow::loadDevicesCatalogToModel(){

    //Refresh active state
    updateAllDeviceActive();

    //Retrieve device hierarchy
    QSqlQuery loadCatalogQuery;
    QString loadCatalogQuerySQL;

    loadCatalogQuerySQL = QLatin1String(R"(
                    SELECT  device_id,
                            0,
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
                            device_date_updated,
                            c.catalog_file_path            ,
                            c.catalog_file_type            ,
                            c.catalog_include_hidden       ,
                            c.catalog_include_metadata     ,
                            (SELECT e.device_name FROM device e WHERE e.device_id = d.device_parent_id),
                            c.catalog_is_full_device       ,
                            c.catalog_date_loaded          ,
                            c.catalog_app_version
                    FROM  device d
                    JOIN catalog c ON d.device_external_id = c.catalog_id
                    WHERE device_type = 'Catalog'
                    AND device_group_id = 0
                )");

    if (      selectedDevice->type == "Storage" ){
        loadCatalogQuerySQL += " AND device_parent_id =:device_parent_id ";
    }
    else if ( selectedDevice->type == "Catalog" ){
        loadCatalogQuerySQL += " AND device_id =:device_id ";
    }
    else if ( selectedDevice->type == "Virtual" ){
        QString prepareSQL = QLatin1String(R"(
                                        AND d.device_id IN (
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
        loadCatalogQuerySQL += prepareSQL;
    }

    loadCatalogQuerySQL +=" ORDER BY device_type DESC, device_parent_id ASC, device_id ASC ";
    loadCatalogQuery.prepare(loadCatalogQuerySQL);
    loadCatalogQuery.bindValue(":device_id",        selectedDevice->ID);
    loadCatalogQuery.bindValue(":device_parent_id", selectedDevice->ID);
    loadCatalogQuery.exec();

    //Prepare the tree model: headers
    QStandardItemModel *catalogTreeModel = new QStandardItemModel();
    catalogTreeModel->setHorizontalHeaderLabels({
                                                 tr("Name"),
                                                 tr("Device Type"),
                                                 tr("Active"),
                                                 tr("ID"),
                                                 tr("Parent ID"),
                                                 tr("Catalog ID"),
                                                 tr("Number of files"),
                                                 tr("Total Size"),
                                                 tr("Used space"),
                                                 tr("Free space"),
                                                 tr("Total space"),
                                                 tr("Date updated"),
                                                 tr("Path"),
                                                 tr("Group ID"),
                                                 tr("File Type"),
                                                 tr("include hidden"),
                                                 tr("include metadata"),
                                                 tr("Parent storage"),
                                                 tr("Fulldevice"),
                                                 tr("Date Loaded"),
                                                 tr("App Version"),
                                                 tr("File Path"),
                                                 "" });

    //Create a map to store items by ID for easy access
    QMap<int, QStandardItem*> itemMap;

    //Populate model
    while (loadCatalogQuery.next()) {

        //Get data forthe item
        int id = loadCatalogQuery.value(0).toInt();
        int parentId = loadCatalogQuery.value(1).toInt();
        QString name = loadCatalogQuery.value(2).toString();
        QString type = loadCatalogQuery.value(3).toString();
        int externalId = loadCatalogQuery.value(4).toInt();
        QString path = loadCatalogQuery.value(5).toString();
        qint64 size = loadCatalogQuery.value(6).toLongLong();
        qint64 number = loadCatalogQuery.value(7).toLongLong();
        qint64 total_space = loadCatalogQuery.value(8).toLongLong();
        qint64 free_space = loadCatalogQuery.value(9).toLongLong();
        qint64 used_space = total_space - free_space;
        bool isActive = loadCatalogQuery.value(10).toBool();
        int groupID = loadCatalogQuery.value(11).toBool();
        QString dateTimeUpdated = loadCatalogQuery.value(12).toString();
        QString catalog_file_path = loadCatalogQuery.value(13).toString();
        QString catalog_file_type = loadCatalogQuery.value(14).toString();
        QString catalog_include_hidden = loadCatalogQuery.value(15).toString();
        QString catalog_include_metadata = loadCatalogQuery.value(16).toString();
        QString parent_storage = loadCatalogQuery.value(17).toString();
        QString catalog_is_full_device = loadCatalogQuery.value(18).toString();
        QString catalog_date_loaded = loadCatalogQuery.value(19).toString();
        QString catalog_app_version = loadCatalogQuery.value(20).toString();

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
        rowItems << new QStandardItem(dateTimeUpdated);
        rowItems << new QStandardItem(path);
        rowItems << new QStandardItem(QString::number(groupID));
        rowItems << new QStandardItem(catalog_file_type);
        rowItems << new QStandardItem(catalog_include_hidden);
        rowItems << new QStandardItem(catalog_include_metadata);
        rowItems << new QStandardItem(parent_storage);
        rowItems << new QStandardItem(catalog_is_full_device);
        rowItems << new QStandardItem(catalog_date_loaded);
        rowItems << new QStandardItem(catalog_app_version);
        rowItems << new QStandardItem(catalog_file_path);

        //Get the item representing the name, and map the parent ID
        QStandardItem* item = rowItems.at(0);
        QStandardItem* parentItem = itemMap.value(parentId);

        //Add top-level items directly to the model
        if (parentId == 0) {
            catalogTreeModel->appendRow(rowItems);
        }
        //else append the row to the parent item
        else{
            if (parentItem) {
                parentItem->appendRow(rowItems);
            }
            else if(id!=0){
                // Skip this row and proceed to the next one
                qDebug() << "loadDevicesTreeToModel - Parent item not found for ID:" << id;
                continue;
            }
        }

        // Store the item in the map for future reference
        itemMap.insert(id, item);
    }

    //Load Model to treeview (Virtual tab)
    DeviceTreeView *deviceTreeViewForDeviceTab = new DeviceTreeView(this);
    deviceTreeViewForDeviceTab->setSourceModel(catalogTreeModel);
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
    ui->Devices_treeView_DeviceList->header()->resizeSection(11, 150); //date updated
    ui->Devices_treeView_DeviceList->header()->setSectionResizeMode(12, QHeaderView::ResizeToContents); //Path
    ui->Devices_treeView_DeviceList->header()->resizeSection(13,  25); //Group ID

    ui->Devices_treeView_DeviceList->header()->hideSection( 1); //Type
    ui->Devices_treeView_DeviceList->header()->hideSection( 3); //ID
    ui->Devices_treeView_DeviceList->header()->hideSection( 4); //Parent ID
    ui->Devices_treeView_DeviceList->header()->hideSection( 8); //Used space
    ui->Devices_treeView_DeviceList->header()->hideSection( 9); //Free space
    ui->Devices_treeView_DeviceList->header()->hideSection(10); //Total space

    if (ui->Devices_checkBox_DisplayFullTable->isChecked()) {
        ui->Devices_treeView_DeviceList->header()->showSection(2); //Active
        ui->Devices_treeView_DeviceList->header()->showSection(5); //External ID
        ui->Devices_treeView_DeviceList->header()->showSection(13); //Group ID
    } else {
        ui->Devices_treeView_DeviceList->header()->hideSection(2); //Active
        ui->Devices_treeView_DeviceList->header()->hideSection(5); //External ID
        ui->Devices_treeView_DeviceList->header()->hideSection(13); //Group ID
    }

    if (collection->databaseMode !="Memory") {
        ui->Devices_treeView_DeviceList->header()->hideSection(22); //Active
    }

    ui->Devices_treeView_DeviceList->expandAll();
}
//--------------------------------------------------------------------------

//--------------------------------------------------------------------------
//--- Storage --------------------------------------------------------------
void MainWindow::loadStorageList()
{//Load Storage selection to comboBoxes

    //Get data
    QSqlQuery query;
    QString querySQL = QLatin1String(R"(
                                SELECT device_id, device_name
                                FROM   device
                                WHERE  device_type = 'Storage'

                            )");//AND    device_group_id = 0

    if ( selectedDevice->type == "Storage" ){
        querySQL += QLatin1String(R"( AND device_name ='%1' )").arg(selectedDevice->name);
        ui->Create_comboBox_StorageSelection->setCurrentText(selectedDevice->name); //replace by ID
    }
    else if ( selectedDevice->type == "Catalog" ){
        querySQL += " AND device_id =:device_parent_id";
    }
    else if ( selectedDevice->type == "Virtual" ){
        QString prepareSQL = QLatin1String(R"(
                                AND device_id IN (
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

    querySQL += " ORDER BY device_name ";
    query.prepare(querySQL);
    query.bindValue(":device_id", selectedDevice->ID);
    query.bindValue(":device_parent_id", selectedDevice->parentID);
    query.exec();

    //Clear comboboxes and load selected Storage device list
    ui->Create_comboBox_StorageSelection->clear();
    while(query.next())
    {
        ui->Create_comboBox_StorageSelection->addItem(query.value(1).toString(),query.value(0).toInt());
    }
}
//--------------------------------------------------------------------------
void MainWindow::displayStoragePicture()
{//Load and display the picture of the storage device
    QString picturePath = collection->collectionFolder + "/images/" + QString::number(activeDevice->storage->ID) + ".jpg";
    QPixmap pic(picturePath);
    QFile file(picturePath);
    if(file.exists()){
        ui->Storage_label_Picture_2->setScaledContents(true);
        ui->Storage_label_Picture_2->setPixmap(pic.scaled(350, 300, Qt::KeepAspectRatio));
    }
    else{
        QPixmap empty("");
        ui->Storage_label_Picture_2->setPixmap(empty);
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
                                            COUNT (device_id),
                                            SUM(device_free_space),
                                            SUM(device_total_space)
                                        FROM device
                                        WHERE device_type = 'Storage'
                            )");
    if (    selectedDevice->type  == tr("All") )
    {
        //No filtering
    }
    else if ( selectedDevice->type == "Storage" ){
        querySQL += " AND device_id =:device_id ";
    }
    else if ( selectedDevice->type == "Catalog" ){
        querySQL += " AND device_id =:device_parent_id";
    }
    else if ( selectedDevice->type == "Virtual" ){
        QString prepareSQL = QLatin1String(R"(
                                    AND  device_id IN (
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
    query.bindValue(":device_id", selectedDevice->ID);
    query.bindValue(":device_parent_id", selectedDevice->parentID);
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

//--------------------------------------------------------------------------
//--- Catalogs -------------------------------------------------------------
//--------------------------------------------------------------------------
void MainWindow::on_Catalogs_pushButton_Import_clicked()
{
    importFromVVV();
}
//--------------------------------------------------------------------------
void MainWindow::saveCatalogChanges()
{
    Device previousCatalog;
    previousCatalog.ID = activeDevice->ID;
    previousCatalog.loadDevice();

    //Get new values
    //Other values
    activeDevice->catalog->fileType         = ui->Catalogs_comboBox_FileType->itemData(ui->Catalogs_comboBox_FileType->currentIndex(),Qt::UserRole).toString();
    activeDevice->catalog->includeHidden    = ui->Catalogs_checkBox_IncludeHidden->isChecked();
    activeDevice->catalog->includeMetadata  = ui->Catalogs_checkBox_IncludeMetadata->isChecked();
    activeDevice->catalog->isFullDevice     = ui->Catalogs_checkBox_isFullDevice->checkState();
    //DEV:QString newIncludeSymblinks  = ui->Catalogs_checkBox_IncludeSymblinks->currentText();

    //Confirm save changes
    QString message = tr("Save changes to the definition of the catalog?<br/>");
    message = message + "<table> <tr><td width=155><i>" + tr("field") + "</i></td><td width=125><i>" + tr("previous value") + "</i></td><td width=200><i>" + tr("new value") + "</i></td>";

    if(activeDevice->catalog->fileType       !=previousCatalog.catalog->fileType)
        message = message + "<tr><td>" + tr("File Type")    + "</td><td>" + previousCatalog.catalog->fileType     + "</td><td><b>" + activeDevice->catalog->fileType      + "</b></td></tr>";
    if(activeDevice->catalog->includeHidden  != previousCatalog.catalog->includeHidden)
        message = message + "<tr><td>" + tr("Include Hidden")   + "</td><td>" + QVariant(previousCatalog.catalog->includeHidden).toString()   + "</td><td><b>" + QVariant(activeDevice->catalog->includeHidden).toString()   + "</b></td></tr>";
    if(activeDevice->catalog->includeMetadata  != previousCatalog.catalog->includeMetadata)
        message = message + "<tr><td>" + tr("Include Metadata") + "</td><td>" + QVariant(previousCatalog.catalog->includeMetadata).toString() + "</td><td><b>" + QVariant(activeDevice->catalog->includeMetadata).toString() + "</b></td></tr>";
    if(activeDevice->catalog->isFullDevice  != previousCatalog.catalog->isFullDevice)
        message = message + "<tr><td>" + tr("Is Full Device") + "</td><td>" + QVariant(previousCatalog.catalog->isFullDevice).toString() + "</td><td><b>" + QVariant(activeDevice->catalog->isFullDevice).toString() + "</b></td></tr>";

    message = message + "</table>";

    if(    (activeDevice->catalog->sourcePath       !=previousCatalog.catalog->sourcePath)
        or (activeDevice->catalog->fileType         !=previousCatalog.catalog->fileType)
        or (activeDevice->catalog->includeHidden    !=previousCatalog.catalog->includeHidden)
        or (activeDevice->catalog->includeMetadata  !=previousCatalog.catalog->includeMetadata))
    {
        message = message + + "<br/><br/>" + tr("(The catalog must be updated to reflect these changes)");
    }

    int result = QMessageBox::warning(this, "Katalog", message, QMessageBox::Yes | QMessageBox::Cancel);
    if ( result == QMessageBox::Cancel){
        return;
    }

    //Write all changes to database (except change of name)
    activeDevice->catalog->saveCatalog();

    //Write changes to catalog file (update headers only)
    activeDevice->catalog->updateCatalogFileHeaders(collection->databaseMode);

    //Refresh display
    //loadCatalogsTableToModel();

    //Update the list of files if the changes impact the contents (i.e. path, file type, hidden)
    if (       activeDevice->catalog->sourcePath      != previousCatalog.catalog->sourcePath
        or activeDevice->catalog->includeHidden   != previousCatalog.catalog->includeHidden
        or activeDevice->catalog->includeMetadata != previousCatalog.catalog->includeMetadata
        or activeDevice->catalog->fileType        != previousCatalog.catalog->fileType)
    {
        int updatechoice = QMessageBox::warning(this, "Katalog",
                                                tr("Update the catalog content with the new criteria?\n")
                                                , QMessageBox::Yes
                                                    | QMessageBox::No);
        if ( updatechoice == QMessageBox::Yes){
            activeDevice->catalog->loadCatalog();
            reportAllUpdates(activeDevice,
                             activeDevice->updateDevice("update",
                                                        collection->databaseMode,
                                                        true,
                                                        collection->collectionFolder,
                                                        true),
                             "update");
        }
    }

    //Refresh
    if(collection->databaseMode=="Memory")
        collection->loadCatalogFilesToTable();

    //loadCatalogsTableToModel();

    //Hide edition section
    //ui->Catalogs_widget_EditCatalog->hide();

}
//--------------------------------------------------------------------------
void MainWindow::updateCatalogsScreenStatistics()
{
    QSqlQuery querySumCatalogValues;

    //Prepare the query
    QString querySumCatalogValuesSQL  = QLatin1String(R"(
                                        SELECT  COUNT(device_id),
                                                SUM(device_total_file_size),
                                                SUM(device_total_file_count)
                                        FROM device d
                                        WHERE device_type = 'Catalog'
                                    )");

    if (      selectedDevice->type == "Storage" ){
        querySumCatalogValuesSQL += " AND device_parent_id =:device_parent_id ";
    }
    else if ( selectedDevice->type == "Catalog" ){
        querySumCatalogValuesSQL += " AND device_id =:device_id ";
    }
    else if ( selectedDevice->type == "Virtual" ){
        QString prepareSQL = QLatin1String(R"(
                                        AND d.device_id IN (
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
        querySumCatalogValuesSQL += prepareSQL;
    }

    //Execute and use results
    querySumCatalogValues.prepare(querySumCatalogValuesSQL);
    querySumCatalogValues.bindValue(":device_id", selectedDevice->ID);
    querySumCatalogValues.bindValue(":device_parent_id", selectedDevice->ID);
    querySumCatalogValues.exec();
    querySumCatalogValues.next();

    ui->Catalogs_label_Catalogs->setText(QString::number(querySumCatalogValues.value(0).toInt()));
    ui->Catalogs_label_TotalSize->setText(QLocale().formattedDataSize(querySumCatalogValues.value(1).toLongLong()));
    ui->Catalogs_label_TotalNumber->setText(QLocale().toString(querySumCatalogValues.value(2).toInt()));
}
//--------------------------------------------------------------------------
void MainWindow::backupFile(QString filePath)
{//Copy the file to the same location, adding .bak for the new file name.
    QString targetFilePath = filePath + ".bak";

    //Verify if a bak up file already exist and remove it.
    if (QFile::exists(targetFilePath))
    {
        QFile::remove(targetFilePath);
    }

    //Copy
    QFile::copy(filePath, targetFilePath);
}
//--------------------------------------------------------------------------
void MainWindow::importFromVVV()
{
    //Select file

    //Get path of the file to import
    QString sourceFilePath = QFileDialog::getOpenFileName(this, tr("Select the csv file to be imported"), collection->collectionFolder);

    //Stop if no path is selected
    if ( sourceFilePath=="" ) return;

    //Define file
    QFile sourceFile(sourceFilePath);

    //Prepare a dateTime to add to device or catalog anmes and avoid duplicates
    QString dateTimeForCatalogName = "_" + QDateTime::currentDateTime().toString("yy-MM-ss hh-mm-ss");


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
                                                    file_name,
                                                    file_folder_path,
                                                    file_size,
                                                    file_date_updated,
                                                    file_catalog )
                                    VALUES(
                                                    :file_name,
                                                    :file_folder_path,
                                                    :file_size,
                                                    :file_date_updated,
                                                    :file_catalog )
                                                )");
    insertQuery.prepare(insertSQL);

    //Prepare insert query for folder
    QSqlQuery insertFolderQuery;
    QString insertFolderSQL = QLatin1String(R"(
                                        INSERT OR IGNORE INTO folder(
                                            folder_catalog_name,
                                            folder_path
                                         )
                                        VALUES(
                                            :folder_catalog_name,
                                            :folder_path)
                                        )");
    insertFolderQuery.prepare(insertFolderSQL);


    //prepare file and stream

    if(!sourceFile.open(QIODevice::ReadOnly)) {
        QMessageBox::information(this,"Katalog",tr("No catalog found."));
        return;
    }

    QTextStream textStream(&sourceFile);
    QString     line;

    //Process and check Headers line
    line = textStream.readLine();

    //Check this is the right source format
    if (line.left(6)!="Volume"){
        QApplication::restoreOverrideCursor();
        QMessageBox::warning(this,"Kotation",tr("A file was found, but could not be loaded") +".\n");
        return;
    }

    //load all files to the database


    while (true)
    {
        //Read the new line
        line = textStream.readLine();

        if (line !=""){
            QStringList fieldList = line.split("\t");
            if ( fieldList.count()==7 ){

                //Append file data to the database  ( removing " characters)
                insertQuery.bindValue(":file_name", fieldList[2].remove("\""));
                insertQuery.bindValue(":file_folder_path", fieldList[1].remove("\""));
                insertQuery.bindValue(":file_size", fieldList[3].toLongLong());
                insertQuery.bindValue(":file_date_updated", fieldList[5]);
                insertQuery.bindValue(":file_catalog", fieldList[0].remove("\"").replace("/","_") + dateTimeForCatalogName);
                insertQuery.exec();

                //Append folder data to the database
                insertFolderQuery.bindValue(":folder_catalog_name", fieldList[0].remove("\"").replace("/","_") + dateTimeForCatalogName);
                insertFolderQuery.bindValue(":folder_path",         fieldList[1].remove("\""));
                insertFolderQuery.exec();
            }
        }
        else
            break;
    }

    //complete table for missing folders
    createMissingParentDirectories();

    //close source file
    sourceFile.close();

    //Stream the list of files and folders out to the target catalog file(s)
    //Define a root folder, compensating for the fact that VVV export does not contain one

    QString virtualCatalogFolder = "/import";

    //Get a list of the source catalogs
    QString listCatalogSQL = QLatin1String(R"(
                                    SELECT DISTINCT file_catalog
                                    FROM file
                                                )");
    QSqlQuery listCatalogQuery;
    listCatalogQuery.prepare(listCatalogSQL);
    listCatalogQuery.exec();

    //Create a virtual device to host the new catalogs
    Device importVirtualDevice;
    importVirtualDevice.generateDeviceID();
    importVirtualDevice.type ="Virtual";
    importVirtualDevice.name = "imports from VVV " + dateTimeForCatalogName;
    importVirtualDevice.parentID = 0;
    importVirtualDevice.groupID = 1;
    importVirtualDevice.path = virtualCatalogFolder;
    importVirtualDevice.dateTimeUpdated = QDateTime::currentDateTime();
    importVirtualDevice.insertDevice();
    collection->saveDeviceTableToFile();

    //Iterate each catalog to generate related files
    while (listCatalogQuery.next()){

        //Create Device
        Device importedDevice;
        importedDevice.generateDeviceID();
        importedDevice.name = listCatalogQuery.value(0).toString();
        importedDevice.type ="Catalog";
        importedDevice.parentID = importVirtualDevice.ID;
        importedDevice.groupID = 1;
        importedDevice.path = virtualCatalogFolder;
        importedDevice.catalog->generateID();
        importedDevice.externalID = importedDevice.catalog->ID;
        importedDevice.dateTimeUpdated = QDateTime::currentDateTime();
        importedDevice.insertDevice();

        //Get info for the new catalog
        importedDevice.catalog->name = importedDevice.name;
        importedDevice.catalog->filePath = collection->collectionFolder + "/" + importedDevice.name + ".idx";
        importedDevice.catalog->sourcePath = virtualCatalogFolder;
        importedDevice.catalog->includeHidden = 1;
        importedDevice.catalog->includeSymblinks = 0;
        importedDevice.catalog->isFullDevice = 0;
        importedDevice.catalog->includeMetadata = 0;
        importedDevice.catalog->appVersion = currentVersion;
        importedDevice.catalog->insertCatalog();

        //Update total number and size of the files
        QString listCatalogSQL = QLatin1String(R"(
                                                    SELECT COUNT(*), SUM(file_size)
                                                    FROM file
                                                    WHERE file_catalog =:file_catalog
                                            )");
        QSqlQuery listCatalogQuery;
        listCatalogQuery.prepare(listCatalogSQL);
        listCatalogQuery.bindValue(":file_catalog", importedDevice.name);
        listCatalogQuery.exec();
        listCatalogQuery.next();

        importedDevice.totalFileCount = listCatalogQuery.value(0).toLongLong();
        importedDevice.totalFileSize  = listCatalogQuery.value(1).toLongLong();
        importedDevice.saveDevice();
        collection->saveDeviceTableToFile();

        //Save device
        collection->saveDeviceTableToFile();


        //Export the catalog file

        //Prepare the catalog file path
        QFile fileOut(importedDevice.catalog->filePath);

        //Prepare the stream and file headers
        QTextStream out(&fileOut);
        if(fileOut.open(QIODevice::WriteOnly | QIODevice::Truncate)) {

            out  << "<catalogSourcePath>" + virtualCatalogFolder << "\n"
                << "<catalogFileCount>" + QString::number(importedDevice.totalFileCount)    << "\n"
                << "<catalogTotalFileSize>" + QString::number(importedDevice.totalFileSize) << "\n"
                << "<catalogIncludeHidden>"       << "\n"
                << "<catalogFileType>"            << "\n"
                << "<catalogStorage>"             << "\n"
                << "<catalogIncludeSymblinks>"    << "\n"
                << "<catalogIsFullDevice>"        << "\n"
                << "<catalogIncludeMetadata>"     << "\n"
                << "<catalogAppVersion>" + currentVersion << "\n"
                << "<catalogID>" + QString::number(importedDevice.externalID) << "\n";
        }

        //Get the list of file to add
        QString listFilesSQL = QLatin1String(R"(
                                                SELECT
                                                    file_folder_path,
                                                    file_name,
                                                    file_size,
                                                    file_date_updated
                                                FROM file
                                                WHERE file_catalog =:file_catalog
                                            )");
        QSqlQuery listFilesQuery;
        listFilesQuery.prepare(listFilesSQL);
        listFilesQuery.bindValue(":file_catalog", importedDevice.name);
        listFilesQuery.exec();

        //Write the results in the file
        while (listFilesQuery.next()) {
            out << virtualCatalogFolder + listFilesQuery.value(0).toString() + "/" + listFilesQuery.value(1).toString();
            out << '\t';
            out << listFilesQuery.value(2).toString();
            out << '\t';
            out << listFilesQuery.value(3).toString();
            out << '\n';
        }

        fileOut.close();

        //Refresh catalogs
        collection->loadCatalogFilesToTable();


        //Export the folder file

        //Prepare the fodlers file path
        QFile fileFolderOut(collection->collectionFolder + "/" + importedDevice.name + ".folders.idx");

        //Prepare the stream and file headers
        QTextStream folderOut(&fileFolderOut);
        if(fileFolderOut.open(QIODevice::WriteOnly | QIODevice::Truncate)) {

            //Get the list of file to add
            QString listFoldersSQL = QLatin1String(R"(
                                            SELECT
                                                folder_catalog_name,
                                                folder_path
                                            FROM folder
                                            WHERE folder_catalog_name =:folder_catalog_name
                                        )");
            QSqlQuery listFoldersQuery;
            listFoldersQuery.prepare(listFoldersSQL);
            listFoldersQuery.bindValue(":folder_catalog_name", importedDevice.name);
            listFoldersQuery.exec();

            //Write the results in the file
            while (listFoldersQuery.next()) {
                folderOut << listFoldersQuery.value(0).toString();
                folderOut << '\t';
                folderOut << virtualCatalogFolder + listFoldersQuery.value(1).toString();
                folderOut << '\n';
            }
        }

        fileFolderOut.close();
    }

    //update virtual device
    importVirtualDevice.updateNumbersFromChildren();
    collection->saveDeviceTableToFile();

    //Stop animation
    QApplication::restoreOverrideCursor();

    loadCollection();
}
//--------------------------------------------------------------------------
bool MainWindow::reportAllUpdates(Device *device, QList<qint64> list, QString updateType)
{//Provide a report for any combinaison of updates (updateType = create, single, or list) and devices
    QMessageBox msgBox;
    QString message;
    bool reportAvailable;

    //Catalog updates
    if (device->type=="Catalog" and updateType=="update"){

        if(list[0]==1){//Catalog updated
            qDebug()<<"reportAllUpdates / catalogs / update / catalog updated";

            message = QString(tr("<br/>Catalog updated:&nbsp;<b>%1</b><br/>")).arg(device->name);
            message += QString(tr("path:&nbsp;<b>%1</b><br/>")).arg(device->path);
            message += QString("<table>"
                               "<tr><td>Number of files: </td><td align='center'><b> %1 </b></td><td>&nbsp; &nbsp; (added: </td><td align='right'><b> %2 </b>)&nbsp; &nbsp; </td></tr>"
                               "<tr><td>Total file size: </td><td align='right'> <b> %3 </b></td><td>&nbsp; &nbsp; (added: </td><td align='right'><b> %4 </b>)&nbsp; &nbsp; </td></tr>"
                               ).arg(QString::number(list[1]),
                                QString::number(list[2]),
                                QLocale().formattedDataSize(list[3]),
                                QLocale().formattedDataSize(list[4]));
            reportAvailable = true;
        }

        if(list[7]==1){//Parent storage updated
            qDebug()<<"reportAllUpdates / catalogs / update / storage updated";
            Device parentDevice;
            parentDevice.ID = device->parentID;
            parentDevice.loadDevice();

            message += (tr("<br/>"
                           "<tr><td colspan=4>Storage updated:&nbsp; <b>%1</b></td></tr>"
                           "<tr><td colspan=4>path:&nbsp; <b> %8 </b> <br/></td></tr>"
                           "<tr><td> Used Space: </td><td align='right'><b> %2 </b></td><td>&nbsp; &nbsp; (added: </td><td align='right'><b> %3 </b>)</td></tr>"
                           "<tr><td> Free Space: </td><td align='right'><b> %4 </b></td><td>&nbsp; &nbsp; (added: </td><td align='right'><b> %5 </b>)</td></tr>"
                           "<tr><td>Total Space: </td><td align='right'><b> %6 </b></td><td>&nbsp; &nbsp; (added: </td><td align='right'><b> %7 </b>)</td></tr>"
                           "</table>"
                           ).arg(parentDevice.name,
                                 QLocale().formattedDataSize(list[8]),
                                 QLocale().formattedDataSize(list[9]),
                                 QLocale().formattedDataSize(list[10]),
                                 QLocale().formattedDataSize(list[11]),
                                 QLocale().formattedDataSize(list[12]),
                                 QLocale().formattedDataSize(list[13]),
                                 parentDevice.path
                                 ));
            reportAvailable = true;
        }

        if(list[0]==1 or list[7]==1){
            msgBox.setWindowTitle("Katalog");
            msgBox.setText(message);
            msgBox.setIcon(QMessageBox::Information);
            msgBox.exec();
        }
    }
    if (device->type=="Catalog" and updateType=="create"){
        if(list[0]==1){//Catalog updated
            message = QString(tr("<br/>Catalog created:&nbsp;<b>%1</b><br/>")).arg(device->name);
            message += QString(tr("path:&nbsp;<b>%1</b><br/>")).arg(device->path);

            message += QString("<table>"
                               "<tr><td>Number of files: </td><td align='center'><b> %1 </b></td><td></tr>"
                               "<tr><td>Total file size: </td><td align='right'> <b> %2 </b></td><td></tr>"
                               ).arg(QString::number(list[1]),
                                QLocale().formattedDataSize(list[3])
                                );
            reportAvailable = true;
        }

        if(list[7]==1){//Parent storage updated
            Device parentDevice;
            parentDevice.ID = device->parentID;
            parentDevice.loadDevice();

            message += (tr("<br/>"
                           "<tr><td colspan=4>Storage updated:&nbsp; <b>%1</b></td></tr>"
                           "<tr><td colspan=4>path:&nbsp; <b> %8 </b> <br/></td></tr>"
                           "<tr><td> Used Space: </td><td align='right'><b> %2 </b></td><td>&nbsp; &nbsp; (added: </td><td align='right'><b> %3 </b>)</td></tr>"
                           "<tr><td> Free Space: </td><td align='right'><b> %4 </b></td><td>&nbsp; &nbsp; (added: </td><td align='right'><b> %5 </b>)</td></tr>"
                           "<tr><td>Total Space: </td><td align='right'><b> %6 </b></td><td>&nbsp; &nbsp; (added: </td><td align='right'><b> %7 </b>)</td></tr>"
                           "</table>"
                           ).arg(parentDevice.name,
                                 QLocale().formattedDataSize(list[8]),
                                 QLocale().formattedDataSize(list[9]),
                                 QLocale().formattedDataSize(list[10]),
                                 QLocale().formattedDataSize(list[11]),
                                 QLocale().formattedDataSize(list[12]),
                                 QLocale().formattedDataSize(list[13]),
                                 parentDevice.path
                                 ));
            reportAvailable = true;
        }

        if(list[0]==1 or list[7]==1){
            msgBox.setWindowTitle("Katalog");
            msgBox.setText(message);
            msgBox.setIcon(QMessageBox::Information);
            msgBox.exec();
        }
    }

    //Storage updates
    if (device->type=="Storage" and updateType=="update"){
        message.clear();
        message += (tr("<table>"
                       "<tr><td colspan=4>Storage updated:&nbsp; <b>%1</b></td></tr>"
                       "<tr><td colspan=4>path:&nbsp; <b> %8 </b> <br/></td></tr>"
                       "<tr><td> Used Space: </td><td align='right'><b> %2 </b></td><td>&nbsp; &nbsp; (added: </td><td align='right'><b> %3 </b>)</td></tr>"
                       "<tr><td> Free Space: </td><td align='right'><b> %4 </b></td><td>&nbsp; &nbsp; (added: </td><td align='right'><b> %5 </b>)</td></tr>"
                       "<tr><td>Total Space: </td><td align='right'><b> %6 </b></td><td>&nbsp; &nbsp; (added: </td><td align='right'><b> %7 </b>)</td></tr>"
                       "</table>"
                       ).arg(activeDevice->name,
                             QLocale().formattedDataSize(list[8]),
                             QLocale().formattedDataSize(list[9]),
                             QLocale().formattedDataSize(list[10]),
                             QLocale().formattedDataSize(list[11]),
                             QLocale().formattedDataSize(list[12]),
                             QLocale().formattedDataSize(list[13]),
                             activeDevice->path
                             ));
        msgBox.setWindowTitle("Katalog");
        msgBox.setText(message);
        msgBox.setIcon(QMessageBox::Information);
        msgBox.exec();
        reportAvailable = true;
    }
    if (updateType=="list"){
        if(list[0]==1){//Catalog updated
            message = QString(tr("<table>"
                                 "<br/>Selected active catalogs from <b>%1</b> are updated.&nbsp;<br/>")).arg(device->name);
            message += QString(
                           "<tr><td>Number of files: </td><td align='center'><b> %1 </b></td><td>&nbsp; &nbsp; (added: </td><td align='right'><b> %2 </b>)&nbsp; &nbsp; </td></tr>"
                           "<tr><td>Total file size: </td><td align='right'> <b> %3 </b></td><td>&nbsp; &nbsp; (added: </td><td align='right'><b> %4 </b>)&nbsp; &nbsp; </td></tr>"
                           ).arg(QString::number(list[1]),
                                QString::number(list[2]),
                                QLocale().formattedDataSize(list[3]),
                                QLocale().formattedDataSize(list[4]));

            message += "</table>" + QString(tr("<br/><br/> %1 updated Catalogs (active), %2 skipped Catalogs (inactive)")).arg(QString::number(list[5]),QString::number(list[6]));
            reportAvailable = true;
        }

        if(list[7]==1){//Storage updated
            message += (tr("<tr><td colspan=4></td></tr>"
                           "<tr><td colspan=4></td></tr>"
                           "<tr><td colspan=4>Storage updated:&nbsp; <b>%1</b></td></tr>"
                           "<tr><td colspan=4>path:&nbsp; <b> %8 </b> <br/></td></tr>"
                           "<tr><td> Used Space: </td><td align='right'><b> %2 </b></td><td>&nbsp; &nbsp; (added: </td><td align='right'><b> %3 </b>)</td></tr>"
                           "<tr><td> Free Space: </td><td align='right'><b> %4 </b></td><td>&nbsp; &nbsp; (added: </td><td align='right'><b> %5 </b>)</td></tr>"
                           "<tr><td>Total Space: </td><td align='right'><b> %6 </b></td><td>&nbsp; &nbsp; (added: </td><td align='right'><b> %7 </b>)</td></tr>"
                           "</table>"
                           ).arg(device->name,
                                 QLocale().formattedDataSize(list[8]),
                                 QLocale().formattedDataSize(list[9]),
                                 QLocale().formattedDataSize(list[10]),
                                 QLocale().formattedDataSize(list[11]),
                                 QLocale().formattedDataSize(list[12]),
                                 QLocale().formattedDataSize(list[13]),
                                 device->path
                                 ));
            reportAvailable = true;
        }

        if(list[0]==1 or list[7]==1){
            msgBox.setWindowTitle("Katalog");
            msgBox.setText(message);
            msgBox.setIcon(QMessageBox::Information);
            msgBox.exec();
        }
    }

    //Virtual updates
    if (device->type =="Virtual"){
        reportAvailable = false;
    }

    return reportAvailable;
}
//--------------------------------------------------------------------------
void MainWindow::createMissingParentDirectories() {
    QSqlQuery query;

    // Select distinct folder paths
    query.exec("SELECT DISTINCT folder_catalog_name, folder_path FROM folder");

    // Iterate through the result set
    while (query.next()) {
        QString folderCatalogName = query.value(0).toString();
        QString folderPath = query.value(1).toString();

        // Split the folder path into components
        QStringList folders = folderPath.split('/', Qt::SkipEmptyParts);
        QString currentPath;

        // Iterate through the components and insert missing parent directories
        for (const QString& folder : folders) {
            currentPath += '/' + folder;

            // Check if the current path exists in the table
            QSqlQuery checkQuery;
            checkQuery.prepare("SELECT 1 FROM folder WHERE folder_catalog_name = :catalog AND folder_path = :path");
            checkQuery.bindValue(":catalog", folderCatalogName);
            checkQuery.bindValue(":path", currentPath);

            if (!checkQuery.exec()) {
                qDebug() << "Error checking path:" << checkQuery.lastError().text();
            }

            // If the current path doesn't exist, insert it
            if (!checkQuery.next()) {
                QSqlQuery insertQuery;
                insertQuery.prepare("INSERT INTO folder (folder_catalog_name, folder_path) VALUES (:catalog, :path)");
                insertQuery.bindValue(":catalog", folderCatalogName);
                insertQuery.bindValue(":path", currentPath);

                if (!insertQuery.exec()) {
                    qDebug() << "Error inserting path:" << insertQuery.lastError().text();
                }
            }
        }
    }
}
//--------------------------------------------------------------------------
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
        QDesktopServices::openUrl(QUrl::fromLocalFile(selectedDevice->catalog->filePath));
}
//--------------------------------------------------------------------------
void MainWindow::on_Catalogs_pushButton_UpdateAllActive_clicked()
{
    globalUpdateTotalFiles = 0;
    globalUpdateDeltaFiles = 0;
    globalUpdateTotalSize  = 0;
    globalUpdateDeltaSize  = 0;

    //User to choose showing or skipping summary for each catalog update
    bool showEachCatalogUpdateSummary = false;

    QMessageBox msgBox;
    msgBox.setWindowTitle("Katalog");
    msgBox.setText(tr("Update all active catalogs")+"<br/><br/>"+tr("Do you want a the summary of updates for each catalog?"));
    msgBox.setIcon(QMessageBox::Question);
    msgBox.setStandardButtons(QMessageBox::Yes|QMessageBox::No | QMessageBox::Cancel);
    int result = msgBox.exec();

    if ( result == QMessageBox::Yes){
        showEachCatalogUpdateSummary = true;
    }
    else if ( result == QMessageBox::Cancel){
        return;
    }

    int updatedCatalogs = 0;
    int skippedCatalogs = 0;

    // Loop through each row of the displayed model
    Device loopDevice;
    for (int row = 0; row < ui->Devices_treeView_DeviceList->model()->rowCount(); ++row) {
        // Get the index for the "Active" field in the current row
        QModelIndex activeIndex = ui->Devices_treeView_DeviceList->model()->index(row, 2);

        // Retrieve the data for the "Active" field (assuming it contains an icon)
        QIcon activeIcon = qvariant_cast<QIcon>(ui->Devices_treeView_DeviceList->model()->data(activeIndex, Qt::DecorationRole));

        // Check if the icon is set to "dialog-ok-apply"
        if (activeIcon.name() == QIcon::fromTheme("dialog-ok-apply").name()) {
            updatedCatalogs +=1;
            loopDevice.ID = ui->Devices_treeView_DeviceList->model()->data(ui->Devices_treeView_DeviceList->model()->index(row, 3)).toInt();
            loopDevice.loadDevice();
            loopDevice.catalog->appVersion = currentVersion;

            QList<qint64> list = loopDevice.updateDevice("update",
                                                         collection->databaseMode,
                                                         false,
                                                         collection->collectionFolder,
                                                         true);
            if ( showEachCatalogUpdateSummary == true ){
                reportAllUpdates(&loopDevice, list, "update");
            }

            if(list.count()>0){
                globalUpdateTotalFiles += list[1];
                globalUpdateDeltaFiles += list[2];
                globalUpdateTotalSize  += list[3];
                globalUpdateDeltaSize  += list[4];
            }
        }
        else
            skippedCatalogs +=1;
    }

    QList<qint64> globalList;
    globalList <<1;
    globalList <<globalUpdateTotalFiles;
    globalList <<globalUpdateDeltaFiles;
    globalList <<globalUpdateTotalSize;
    globalList <<globalUpdateDeltaSize;
    globalList <<updatedCatalogs;
    globalList <<skippedCatalogs;
    globalList <<0;
    globalList <<0;
    globalList <<0;
    globalList <<0;
    globalList <<0;
    globalList <<0;
    globalList <<0;

    reportAllUpdates(selectedDevice, globalList, "list");

    collection->saveDeviceTableToFile();
    collection->saveStatiticsToFile();

    loadDevicesView();
}
//--------------------------------------------------------------------------
void MainWindow::on_Catalogs_pushButton_UpdateCatalog_clicked()
{
    activeDevice->catalog->appVersion = currentVersion;
    reportAllUpdates(activeDevice,
                     activeDevice->updateDevice("update",
                                                collection->databaseMode,
                                                false,
                                                collection->collectionFolder,
                                                true),
                     "update");
    collection->saveDeviceTableToFile();
    collection->saveStatiticsToFile();

    loadDevicesView();
    loadStatisticsChart();
}
//--------------------------------------------------------------------------

//--------------------------------------------------------------------------
//--- Migration 1.22 to 2.0
//--------------------------------------------------------------------------
//-------------- UI --------------------------------------------------------
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

//-------------- Methods ---------------------------------------------------
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
    loadDevicesView();

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
    loadDevicesView();

}

void MainWindow::importStatistics()
{
    qDebug()<<"importStatistics() empty";
}

int MainWindow::countTreeLevels(const QMap<int, QList<int>>& deviceTree, int parentId) {
    if (!deviceTree.contains(parentId)) {
        return 0;
    }
    int maxLevel = 0;
    for (int childId : deviceTree[parentId]) {
        int level = countTreeLevels(deviceTree, childId);
        if (level > maxLevel) {
            maxLevel = level;
        }
    }
    return maxLevel + 1;
}

//--------------------------------------------------------------------------
//--- DEV: metadata --------------------------------------------------------
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
