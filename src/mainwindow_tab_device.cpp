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
// File Name:   mainwindow_tab_device.cpp
// Purpose:     https://stephanecouturier.github.io/Katalog/docs/Features/Devices
// Description:
// Author:      Stephane Couturier
/////////////////////////////////////////////////////////////////////////////
*/

#include "mainwindow.h"
#include "ui_mainwindow.h"
#include "devicetreeview.h"
#include "device.h"

//TAB: DEVICES -------------------------------------------------------------
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
void MainWindow::on_Devices_treeView_DeviceList_clicked(const QModelIndex &index)
{
    //Get selection data
    activeDevice->ID = ui->Devices_treeView_DeviceList->model()->index(index.row(), 3, index.parent() ).data().toInt();
    activeDevice->loadDevice("defaultConnection");

    if(activeDevice->type =="Catalog")
        ui->Catalogs_pushButton_UpdateCatalog->setEnabled(true);
    else
        ui->Catalogs_pushButton_UpdateCatalog->setEnabled(false);

}
//--------------------------------------------------------------------------
void MainWindow::on_Devices_treeView_DeviceList_customContextMenuRequested(const QPoint &pos)
{
    //Get selection data
    QModelIndex index=ui->Devices_treeView_DeviceList->currentIndex();
    activeDevice->ID   = ui->Devices_treeView_DeviceList->model()->index(index.row(), 3, index.parent() ).data().toInt();
    activeDevice->loadDevice("defaultConnection");

    Device *tempParentDevice = new Device();
    tempParentDevice->ID = activeDevice->parentID;
    tempParentDevice->loadDevice("defaultConnection");

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
                                                        collection->folder,
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
            exploreDevice->loadDevice("defaultConnection");

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
                                                        collection->folder,
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

        QAction *menuDeviceAction3 = new QAction(QIcon::fromTheme("media-playlist-repeat"), tr("Update"), this);
        deviceContextMenu.addAction(menuDeviceAction3);
        connect(menuDeviceAction3, &QAction::triggered, this, [this, deviceName]() {
            reportAllUpdates(activeDevice,
                             activeDevice->updateDevice("update",
                                                        collection->databaseMode,
                                                        true,
                                                        collection->folder,
                                                        true),
                             "update");
            collection->saveDeviceTableToFile();
            collection->saveStatiticsToFile();
            loadDevicesView();
        });

        QAction *menuDeviceAction2 = new QAction(QIcon::fromTheme("document-edit-sign"), tr("Edit"), this);
        deviceContextMenu.addAction(menuDeviceAction2);
        connect(menuDeviceAction2, &QAction::triggered, this, [this, deviceName]() {
            editDevice();
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
        if (activeDevice->groupID == 0) {
            deviceContextMenu.removeAction(menuDeviceAction5);
        }
        if (activeDevice->groupID != 0 and selectedDevice->type != "Catalog") {
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
    //Prepare a list with 0 for catalog update, as no catalog is updated
    QList<qint64> list;
    list <<0<<0<<0<<0<<0<<0<<0;

    //Update storage and add to the list
    list += activeDevice->updateDevice("update",
                                                collection->databaseMode,
                                                true,
                                                collection->folder,
                                                false);
    //Report the change
    reportAllUpdates(activeDevice,
                     list,
                     "update");
    collection->saveDeviceTableToFile();
    collection->saveStatiticsToFile();
    loadDevicesView();
    editDevice();
}
//--------------------------------------------------------------------------
void MainWindow::on_Devices_pushButton_ApplyToSelection_clicked()
{
    loadDevicesTreeToModel("Filters");
}
//--------------------------------------------------------------------------

//--- Methods --------------------------------------------------------------
//--------------------------------------------------------------------------
void MainWindow::assignCatalogToDevice(Device *catalogDevice, Device *parentDevice)
{
    if( parentDevice->ID!=0 and catalogDevice->ID !=0){

        //Verif if catalog is not already assigned.
        QSqlQuery queryCurrentCatalogsExternalID(QSqlDatabase::database("defaultConnection"));
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
            QSqlQuery queryID(QSqlDatabase::database("defaultConnection"));
            QString queryIDSQL = QLatin1String(R"(
                                SELECT MAX(device_id)
                                FROM device
                            )");
            queryID.prepare(queryIDSQL);
            queryID.exec();
            queryID.next();
            int newID = queryID.value(0).toInt()+1;

            //Insert catalog
            QSqlQuery query(QSqlDatabase::database("defaultConnection"));
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
        QSqlQuery queryID(QSqlDatabase::database("defaultConnection"));
        QString queryIDSQL = QLatin1String(R"(
                            SELECT MAX(device_id)
                            FROM device
                        )");
        queryID.prepare(queryIDSQL);
        queryID.exec();
        queryID.next();
        int newID = queryID.value(0).toInt()+1;

        //Insert storage
        QSqlQuery query(QSqlDatabase::database("defaultConnection"));
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
                                      tr("Do you want to unassign this catalog from this virtual device?"),QMessageBox::Yes|QMessageBox::Cancel);

    if ( result ==QMessageBox::Yes){

        if( deviceID!=0 and deviceParentID!=0){
            //Insert catalog
            QSqlQuery query(QSqlDatabase::database("defaultConnection"));
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
    parentDevice.loadDevice("defaultConnection");
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
    filterFromSelectedDevice();
}
//--------------------------------------------------------------------------
void MainWindow::recordDevicesSnapshot()
{
    //Get the current total values
    QSqlQuery queryLastCatalog(QSqlDatabase::database("defaultConnection"));
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

    QSqlQuery queryLastStorage(QSqlDatabase::database("defaultConnection"));
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
    QSqlQuery queryNew(QSqlDatabase::database("defaultConnection"));
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

    QSqlQuery queryNewStorage(QSqlDatabase::database("defaultConnection"));
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
        QSqlQuery query("SELECT device_id, device_parent_id FROM device", QSqlDatabase::database("defaultConnection"));
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
    QSqlQuery query(QSqlDatabase::database("defaultConnection"));

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
    QSqlQuery query(QSqlDatabase::database("defaultConnection"));
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
    query.next();

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
    filterFromSelectedDevice();
    loadParentsList();

    //Make it the activeDevice and edit
    activeDevice->ID = newDevice->ID;
    activeDevice->loadDevice("defaultConnection");
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
    filterFromSelectedDevice();
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
    activeDevice->loadDevice("defaultConnection");
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
    if(collection->databaseMode=="Memory")
        ui->Storage_label_Picture_2->setVisible(true);
    else
        ui->Storage_label_Picture_2->setVisible(false);

    ui->Devices_widget_Edit->setVisible(true);
    ui->Devices_lineEdit_Name->setText(activeDevice->name);
    ui->Devices_label_ItemDeviceTypeValue->setText(activeDevice->type);
    ui->Devices_label_ItemDeviceIDValue->setText(QString::number(activeDevice->ID));

    if(activeDevice->type =="Catalog"){
        ui->Devices_widget_EditCatalogFields->show();
        ui->Devices_widget_EditStorageFields->hide();
        ui->Catalogs_comboBox_FileType->setCurrentText(activeDevice->catalog->fileType);
        ui->Catalogs_checkBox_IncludeHidden->setChecked(activeDevice->catalog->includeHidden);
        ui->Catalogs_checkBox_IncludeMetadata->setChecked(activeDevice->catalog->includeMetadata);
        //DEV: ui->Catalogs_checkBox_isFullDevice->setChecked(selectedCatalogIsFullDevice);
    }
    else if(activeDevice->type =="Storage"){
        ui->Devices_widget_EditStorageFields->show();
        ui->Devices_widget_EditCatalogFields->hide();

        ui->Storage_lineEdit_Panel_ID->setText(QString::number(activeDevice->storage->ID));
        ui->Storage_lineEdit_Panel_Type->setText(activeDevice->storage->type);
        ui->Storage_lineEdit_Panel_Label->setText(activeDevice->storage->label);
        ui->Storage_lineEdit_Panel_FileSystem->setText(activeDevice->storage->fileSystem);

        ui->Storage_lineEdit_Panel_Total->setText(QString::number(activeDevice->totalSpace));
        ui->Storage_lineEdit_Panel_Free->setText(QString::number(activeDevice->freeSpace));
        ui->Storage_label_Panel_TotalSpace->setText(QLocale().formattedDataSize(activeDevice->totalSpace));
        ui->Storage_label_Panel_FreeSpace->setText(QLocale().formattedDataSize(activeDevice->freeSpace));

        ui->Storage_lineEdit_Panel_Brand->setText(activeDevice->storage->brand);
        ui->Storage_lineEdit_Panel_Model->setText(activeDevice->storage->model);
        ui->Storage_lineEdit_Panel_SerialNumber->setText(activeDevice->storage->serialNumber);
        ui->Storage_lineEdit_Panel_BuildDate->setText(activeDevice->storage->buildDate);
        ui->Storage_lineEdit_Panel_Comment1->setText(activeDevice->storage->comment1);
        ui->Storage_lineEdit_Panel_Comment2->setText(activeDevice->storage->comment2);
        ui->Storage_lineEdit_Panel_Comment3->setText(activeDevice->storage->comment3);

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
    newDeviceItem->loadDevice("defaultConnection");
    ui->Devices_comboBox_Parent->setCurrentText(newDeviceItem->name+" ("+QString::number(newDeviceItem->ID)+")");
}
//--------------------------------------------------------------------------
void MainWindow::saveDeviceForm()
{//Save the device values from the edit panel

    //Keep previous values
    activeDevice->loadDevice("defaultConnection");
    int previousExternalID = activeDevice->externalID;
    QString previousName = activeDevice->name;
    Device previousParentDevice;
    previousParentDevice.ID = activeDevice->parentID;
    previousParentDevice.loadDevice("defaultConnection");

    //Get new values: name, parentID, externalID
    activeDevice->parentID = ui->Devices_comboBox_Parent->currentData().toInt();
    activeDevice->name = ui->Devices_lineEdit_Name->text();

    if (activeDevice->type == "Storage")
        activeDevice->externalID = ui->Storage_lineEdit_Panel_ID->text().toInt();

    if (previousName != activeDevice->name
        and activeDevice->verifyDeviceNameExists()==true
        and activeDevice->type=="Catalog"){
        //Duplicate catalog names are not allowed
        QMessageBox msgBox;
        msgBox.setWindowTitle("Katalog");
        msgBox.setText( tr("There is already a Catalog with this name:<br/><b>").arg(activeDevice->type)
                       + activeDevice->name
                       + "</b><br/><br/>"+tr("Choose a different name and try again."));
        msgBox.setIcon(QMessageBox::Warning);
        msgBox.exec();
        return;
    }
    if (previousExternalID != activeDevice->externalID
        and activeDevice->verifyStorageExternalIDExists()==true
        and activeDevice->type=="Storage"){
        //Duplicate storage IDs (device external ID) are not allowed
        QMessageBox msgBox;
        msgBox.setWindowTitle("Katalog");
        msgBox.setText( tr("There is already a Storage with this ID.<b>")
                       + "<br/><br/>"+tr("Choose a different ID and try again."));
        msgBox.setIcon(QMessageBox::Warning);
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
    newParentDevice.loadDevice("defaultConnection");

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
                loopDevice.loadDevice("defaultConnection");
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

    //If device is a catalog, save catalog changes
    if(activeDevice->type == "Catalog"){
        activeDevice->catalog->storageName = newParentDevice.name;
        saveCatalogChanges();
        updateCatalogsScreenStatistics();
        loadDevicesTreeToModel("Filters");
    }

    //If device is Storage, rename in storage table
    if(activeDevice->type == "Storage"){

        QString currentStorageName = activeDevice->name;
        QString newStorageName     = ui->Devices_lineEdit_Name->text();

        //Update Storage name
        QString queryUpdateStorageSQL = QLatin1String(R"(
                                    UPDATE storage
                                    SET storage_name =:storage_name,
                                        storage_id   =:new_storage_id
                                    WHERE storage_id =:storage_id
                                )");

        QSqlQuery updateQuery(QSqlDatabase::database("defaultConnection"));
        updateQuery.prepare(queryUpdateStorageSQL);
        updateQuery.bindValue(":storage_name", activeDevice->name);
        updateQuery.bindValue(":new_storage_id", previousExternalID);
        updateQuery.bindValue(":storage_id", activeDevice->externalID);
        updateQuery.exec();

        //loadStorageTableToModel();
        updateStorageSelectionStatistics();

        //Save data to file
        collection->saveStorageTableToFile();

        //Update name in statistics and catalogs
        if (currentStorageName != newStorageName){
            //Update statistics
            QString updateNameQuerySQL = QLatin1String(R"(
                                    UPDATE statistics_storage
                                    SET storage_name = :new_storage_name
                                    WHERE storage_id =:storage_id
                                )");

            QSqlQuery updateNameQuery(QSqlDatabase::database("defaultConnection"));
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

            QSqlQuery updateCatalogQuery(QSqlDatabase::database("defaultConnection"));
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

                QSqlQuery listCatalogQuery(QSqlDatabase::database("defaultConnection"));
                listCatalogQuery.prepare(listCatalogQuerySQL);
                listCatalogQuery.bindValue(":new_storage_name", newStorageName);
                listCatalogQuery.exec();

                //Edit and save each one
                Device loopCatalog;
                while (listCatalogQuery.next()){
                    loopCatalog.catalog = new Catalog;
                    loopCatalog.name = listCatalogQuery.value(0).toString();
                    loopCatalog.catalog->loadCatalog("defaultConnection");
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
        QSqlQuery queryStorage(QSqlDatabase::database("defaultConnection"));
        QString queryStorageSQL = QLatin1String(R"(
                                    UPDATE storage
                                    SET storage_id =:new_storage_id,
                                        storage_type =:storage_type,
                                        storage_location =:storage_location,
                                        storage_label =:storage_label,
                                        storage_file_system =:storage_file_system,
                                        storage_total_space =:storage_total_space,
                                        storage_free_space =:storage_free_space,
                                        storage_brand =:storage_brand,
                                        storage_model =:storage_model,
                                        storage_serial_number =:storage_serial_number,
                                        storage_build_date =:storage_build_date,
                                        storage_comment1 =:storage_comment1,
                                        storage_comment2 =:storage_comment2,
                                        storage_comment3 =:storage_comment3
                                    WHERE storage_id =:storage_id
                                )");

        queryStorage.prepare(queryStorageSQL);
        queryStorage.bindValue(":new_storage_id",        ui->Storage_lineEdit_Panel_ID->text());
        queryStorage.bindValue(":storage_type",          ui->Storage_lineEdit_Panel_Type->text());
        queryStorage.bindValue(":storage_label",         ui->Storage_lineEdit_Panel_Label->text());
        queryStorage.bindValue(":storage_file_system",   ui->Storage_lineEdit_Panel_FileSystem->text());
        queryStorage.bindValue(":storage_brand",         ui->Storage_lineEdit_Panel_Brand->text());
        queryStorage.bindValue(":storage_model",         ui->Storage_lineEdit_Panel_Model->text());
        queryStorage.bindValue(":storage_serial_number", ui->Storage_lineEdit_Panel_SerialNumber->text());
        queryStorage.bindValue(":storage_build_date",    ui->Storage_lineEdit_Panel_BuildDate->text());
        queryStorage.bindValue(":storage_comment1",      ui->Storage_lineEdit_Panel_Comment1->text());
        queryStorage.bindValue(":storage_comment2",      ui->Storage_lineEdit_Panel_Comment2->text());
        queryStorage.bindValue(":storage_comment3",      ui->Storage_lineEdit_Panel_Comment3->text());
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
    loadStorageList();
}
//--------------------------------------------------------------------------
void MainWindow::recordAllDeviceStats(QDateTime dateTime)
{// Save the values (free space and total space) of all storage devices, completing a snapshop of the collection.

    //Get the list of storage devices
    QSqlQuery query(QSqlDatabase::database("defaultConnection"));
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
        loopDevice.loadDevice("defaultConnection");
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
        QSqlQuery query(QSqlDatabase::database("defaultConnection"));
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
            loopDevice.loadDevice("defaultConnection");
            loopDevice.updateActive("defaultConnection");
        }
}

//--------------------------------------------------------------------------
//--- View -----------------------------------------------------------------
void MainWindow::loadDevicesView(){
    int tempLastDevicesSortSection = lastDevicesSortSection;
    int tempLastDevicesSortOrder   = lastDevicesSortOrder;

    if(ui->Devices_radioButton_StorageList->isChecked()==true){
        loadDevicesStorageToModel();
        ui->Devices_widget_TreeOptions->hide();
        ui->Devices_widget_CatalogStats->hide();
        ui->Devices_widget_StorageStats->show();
    }
    else if(ui->Devices_radioButton_CatalogList->isChecked()==true){
        loadDevicesCatalogToModel();
        ui->Devices_widget_TreeOptions->hide();
        ui->Devices_widget_CatalogStats->show();
        ui->Devices_widget_StorageStats->hide();
    }
    else{
        loadDevicesTreeToModel("Devices");
        ui->Devices_widget_TreeOptions->show();
        ui->Devices_widget_CatalogStats->hide();
        ui->Devices_widget_StorageStats->hide();
    }

    lastDevicesSortSection = tempLastDevicesSortSection;
    lastDevicesSortOrder   = tempLastDevicesSortOrder;
    ui->Devices_treeView_DeviceList->QTreeView::sortByColumn(lastDevicesSortSection,Qt::SortOrder(lastDevicesSortOrder));
}
//--------------------------------------------------------------------------
void MainWindow::loadDevicesTreeToModel(QString targetTreeModel)
{
    //Refresh active state
    updateAllDeviceActive();

    //Retrieve device hierarchy
    QSqlQuery query(QSqlDatabase::database("defaultConnection"));
    QString querySQL;


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
                    device_date_updated,
                    0 AS level
                  FROM device
                  WHERE device_parent_id = 0

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
                    child.device_date_updated,
                    parent.level + 1 AS level
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
                    device_date_updated,
                    level
                FROM device_tree

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
                        device_date_updated,
                        0 AS level
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
                        child.device_date_updated,
                        parent.level + 1 AS level
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
                        device_date_updated,
                        level
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
                        device_date_updated,
                        0 AS level
                      FROM device
                      WHERE device_group_id <> 0

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
                        child.device_date_updated,
                        parent.level + 1 AS level
                      FROM device_tree parent
                      JOIN device child ON child.device_parent_id = parent.device_id
                      WHERE parent.device_id <> 1
                    )
                    SELECT DISTINCT
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
                        device_date_updated,
                        level
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

    querySQL +=" ORDER BY level ASC, device_type DESC, device_parent_id ASC, device_id ASC; ";
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
        rowItems << new QStandardItem(name);                        //0
        rowItems << new QStandardItem(type);                        //1
        rowItems << new QStandardItem(QString::number(isActive));   //2
        rowItems << new QStandardItem(QString::number(id));         //3
        rowItems << new QStandardItem(QString::number(parentId));   //4
        rowItems << new QStandardItem(QString::number(externalId)); //5
        rowItems << new QStandardItem(QString::number(number));     //6
        rowItems << new QStandardItem(QString::number(size));       //7
        rowItems << new QStandardItem(QString::number(used_space)); //8
        rowItems << new QStandardItem(QString::number(free_space)); //9
        rowItems << new QStandardItem(QString::number(total_space));//10
        rowItems << new QStandardItem(dateTimeUpdated);             //11
        rowItems << new QStandardItem(path);                        //12
        rowItems << new QStandardItem(QString::number(groupID));    //13

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
        ui->Devices_treeView_DeviceList->header()->resizeSection( 2,  30); //Active
        ui->Devices_treeView_DeviceList->header()->resizeSection( 3,  50); //ID
        ui->Devices_treeView_DeviceList->header()->resizeSection( 4,  50); //Parent ID
        ui->Devices_treeView_DeviceList->header()->resizeSection( 5,  50); //External ID
        ui->Devices_treeView_DeviceList->header()->resizeSection( 6, 100); //Number of Files
        ui->Devices_treeView_DeviceList->header()->resizeSection( 7, 100); //Total File Size
        ui->Devices_treeView_DeviceList->header()->resizeSection( 8, 100); //Used space
        ui->Devices_treeView_DeviceList->header()->resizeSection( 9, 100); //Free space
        ui->Devices_treeView_DeviceList->header()->resizeSection(10, 100); //Total space
        ui->Devices_treeView_DeviceList->header()->setSectionResizeMode(11, QHeaderView::ResizeToContents); //Date updated
        ui->Devices_treeView_DeviceList->header()->setSectionResizeMode(12, QHeaderView::ResizeToContents); //Path
        ui->Devices_treeView_DeviceList->header()->resizeSection(13,  30); //Group ID

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
        ui->Filters_treeView_Devices->setModel(deviceTreeViewForSelectionPanel);
        ui->Filters_treeView_Devices->expandAll();
    }
}
//--------------------------------------------------------------------------
void MainWindow::loadDevicesStorageToModel(){
    //Refresh active state
    updateAllDeviceActive();

    //Retrieve device hierarchy
    QSqlQuery loadStorageQuery(QSqlDatabase::database("defaultConnection"));
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
                            storage_type,
                            storage_label,
                            storage_file_system,
                            storage_brand,
                            storage_model,
                            storage_serial_number,
                            storage_build_date,
                            storage_comment1,
                            storage_comment2,
                            storage_comment3
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
                                                 tr("Brand"),
                                                 tr("Model"),
                                                 tr("Serial Number"),
                                                 tr("Build Date"),
                                                 tr("Comment 1"),
                                                 tr("Comment 2"),
                                                 tr("Comment 3"),
                                                 "" });

    //Create a map to store items by ID for easy access
    QMap<int, QStandardItem*> itemMap;

    //Populate model
    while (loadStorageQuery.next()) {

        //Get data for the item
        int id                          = loadStorageQuery.value(0).toInt();
        int parentId                    = loadStorageQuery.value(1).toInt();
        QString name                    = loadStorageQuery.value(2).toString();
        QString type                    = loadStorageQuery.value(3).toString();
        int externalId                  = loadStorageQuery.value(4).toInt();
        QString path                    = loadStorageQuery.value(5).toString();
        qint64 size                     = loadStorageQuery.value(6).toLongLong();
        qint64 number                   = loadStorageQuery.value(7).toLongLong();
        qint64 total_space              = loadStorageQuery.value(8).toLongLong();
        qint64 free_space               = loadStorageQuery.value(9).toLongLong();
        qint64 used_space               = total_space - free_space;
        bool isActive                   = loadStorageQuery.value(10).toBool();
        int groupID                     = loadStorageQuery.value(11).toBool();
        QString dateTimeUpdated         = loadStorageQuery.value(12).toString();

        QString storage_type            = loadStorageQuery.value(13).toString();
        QString storage_label           = loadStorageQuery.value(14).toString();
        QString storage_file_system     = loadStorageQuery.value(15).toString();
        QString storage_brand           = loadStorageQuery.value(16).toString();
        QString storage_model           = loadStorageQuery.value(17).toString();
        QString storage_serial_number   = loadStorageQuery.value(18).toString();
        QString storage_build_date      = loadStorageQuery.value(19).toString();
        QString storage_comment1        = loadStorageQuery.value(20).toString();
        QString storage_comment2        = loadStorageQuery.value(21).toString();
        QString storage_comment3        = loadStorageQuery.value(22).toString();

        //Create the item for this row
        QList<QStandardItem*> rowItems;
        rowItems << new QStandardItem(name);                        //0
        rowItems << new QStandardItem(type);                        //1
        rowItems << new QStandardItem(QString::number(isActive));   //2
        rowItems << new QStandardItem(QString::number(id));         //3
        rowItems << new QStandardItem(QString::number(parentId));   //4
        rowItems << new QStandardItem(QString::number(externalId)); //5
        rowItems << new QStandardItem(QString::number(number));     //6
        rowItems << new QStandardItem(QString::number(size));       //7
        rowItems << new QStandardItem(QString::number(used_space)); //8
        rowItems << new QStandardItem(QString::number(free_space)); //9
        rowItems << new QStandardItem(QString::number(total_space));//10
        rowItems << new QStandardItem(dateTimeUpdated);             //11
        rowItems << new QStandardItem(path);                        //12
        rowItems << new QStandardItem(QString::number(groupID));    //13

        rowItems << new QStandardItem(storage_type);                //14
        rowItems << new QStandardItem(storage_label);               //15
        rowItems << new QStandardItem(storage_file_system);         //16
        rowItems << new QStandardItem(storage_brand);               //17
        rowItems << new QStandardItem(storage_model);               //18
        rowItems << new QStandardItem(storage_serial_number);       //19
        rowItems << new QStandardItem(storage_build_date);          //20
        rowItems << new QStandardItem(storage_comment1);            //21
        rowItems << new QStandardItem(storage_comment2);            //22
        rowItems << new QStandardItem(storage_comment3);            //23

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
    ui->Devices_treeView_DeviceList->header()->resizeSection( 2,  30); //Active
    ui->Devices_treeView_DeviceList->header()->resizeSection( 3,  50); //ID
    ui->Devices_treeView_DeviceList->header()->resizeSection( 4,  50); //Parent ID
    ui->Devices_treeView_DeviceList->header()->resizeSection( 5,  50); //External ID
    ui->Devices_treeView_DeviceList->header()->resizeSection( 6, 100); //Number of Files
    ui->Devices_treeView_DeviceList->header()->resizeSection( 7, 100); //Total File Size
    ui->Devices_treeView_DeviceList->header()->resizeSection( 8, 100); //Used space
    ui->Devices_treeView_DeviceList->header()->resizeSection( 9, 100); //Free space
    ui->Devices_treeView_DeviceList->header()->resizeSection(10, 100); //Total space
    ui->Devices_treeView_DeviceList->header()->setSectionResizeMode(11, QHeaderView::ResizeToContents); //Date updated
    ui->Devices_treeView_DeviceList->header()->setSectionResizeMode(12, QHeaderView::ResizeToContents); //Path
    ui->Devices_treeView_DeviceList->header()->resizeSection(13,  30); //Group ID

    ui->Devices_treeView_DeviceList->header()->setSectionResizeMode(14, QHeaderView::ResizeToContents); //Type
    ui->Devices_treeView_DeviceList->header()->setSectionResizeMode(15, QHeaderView::ResizeToContents); //Label
    ui->Devices_treeView_DeviceList->header()->setSectionResizeMode(16, QHeaderView::ResizeToContents); //File System
    ui->Devices_treeView_DeviceList->header()->resizeSection(17, 150); //Brand
    ui->Devices_treeView_DeviceList->header()->resizeSection(18, 150); //Model
    ui->Devices_treeView_DeviceList->header()->resizeSection(19, 150); //SerialNumber
    ui->Devices_treeView_DeviceList->header()->resizeSection(20, 100); //Build Date
    ui->Devices_treeView_DeviceList->header()->resizeSection(21, 150); //Comment1
    ui->Devices_treeView_DeviceList->header()->resizeSection(22, 150); //Comment2
    ui->Devices_treeView_DeviceList->header()->resizeSection(23, 150); //Comment3

    //Show or Hide
    ui->Devices_treeView_DeviceList->header()->hideSection( 1); //Type
    ui->Devices_treeView_DeviceList->header()->hideSection( 3); //ID
    ui->Devices_treeView_DeviceList->header()->hideSection( 4); //Parent ID
    ui->Devices_treeView_DeviceList->header()->showSection( 5); //External ID
    ui->Devices_treeView_DeviceList->header()->showSection( 8); //Used space
    ui->Devices_treeView_DeviceList->header()->showSection( 9); //Free space
    ui->Devices_treeView_DeviceList->header()->showSection(10); //Total space
    ui->Devices_treeView_DeviceList->header()->hideSection(13); //Group ID

    if (ui->Devices_checkBox_DisplayFullTable->isChecked()) {
        ui->Devices_treeView_DeviceList->header()->showSection( 2); //Active
        ui->Devices_treeView_DeviceList->header()->showSection(17); //storage_brand
        ui->Devices_treeView_DeviceList->header()->showSection(18); //storage_model
        ui->Devices_treeView_DeviceList->header()->showSection(19); //storage_serial_number
        ui->Devices_treeView_DeviceList->header()->showSection(20); //storage_build_date
        ui->Devices_treeView_DeviceList->header()->showSection(21); //Comment 1
        ui->Devices_treeView_DeviceList->header()->showSection(22); //Comment 2
        ui->Devices_treeView_DeviceList->header()->showSection(23); //Comment 3
    } else {
        ui->Devices_treeView_DeviceList->header()->hideSection(2); //Active
        ui->Devices_treeView_DeviceList->header()->showSection(17); //storage_brand
        ui->Devices_treeView_DeviceList->header()->showSection(18); //storage_model
        ui->Devices_treeView_DeviceList->header()->showSection(19); //storage_serial_number
        ui->Devices_treeView_DeviceList->header()->hideSection(20); //storage_build_date
        ui->Devices_treeView_DeviceList->header()->hideSection(21); //Comment 1
        ui->Devices_treeView_DeviceList->header()->hideSection(22); //Comment 2
        ui->Devices_treeView_DeviceList->header()->hideSection(23); //Comment 3
    }

    ui->Devices_treeView_DeviceList->expandAll();
}
//--------------------------------------------------------------------------
void MainWindow::loadDevicesCatalogToModel(){

    //Refresh active state
    updateAllDeviceActive();

    //Retrieve device hierarchy
    QSqlQuery loadCatalogQuery(QSqlDatabase::database("defaultConnection"));
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
                                                tr("Name"),             //0
                                                tr("Device Type"),      //1
                                                tr("Active"),           //2
                                                tr("ID"),               //3
                                                tr("Parent ID"),        //4
                                                tr("Catalog ID"),       //5
                                                tr("Number of files"),  //6
                                                tr("Total Size"),       //7
                                                tr("Used space"),       //8
                                                tr("Free space"),       //9
                                                tr("Total space"),      //10
                                                tr("Date updated"),     //11
                                                tr("Path"),             //12
                                                tr("Group ID"),         //13
                                                "14",                   //14
                                                "15",                   //15
                                                "16",                   //16
                                                "17",                   //17
                                                "18",                   //18
                                                "19",                   //19
                                                "20",                   //20
                                                "21",                   //21
                                                "22",                   //22
                                                "23",                   //23
                                                tr("File Type"),        //24
                                                tr("include hidden"),   //25
                                                tr("include metadata"), //26
                                                tr("Parent storage"),   //27
                                                tr("Fulldevice"),       //28
                                                tr("Date Loaded"),      //29
                                                tr("App Version"),      //30
                                                tr("File Path"),        //31
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
        //Device fields
            QList<QStandardItem*> rowItems;
            rowItems << new QStandardItem(name);                        //0
            rowItems << new QStandardItem(type);                        //1
            rowItems << new QStandardItem(QString::number(isActive));   //2

            QStandardItem *idItem = new QStandardItem();
            idItem->setData(id, Qt::DisplayRole);
            rowItems << idItem;                                         //3

            QStandardItem *parentIdItem = new QStandardItem();
            parentIdItem->setData(parentId, Qt::DisplayRole);
            rowItems << parentIdItem;                                   //4

            QStandardItem *externalIdItem = new QStandardItem();
            externalIdItem->setData(externalId, Qt::DisplayRole);
            rowItems << externalIdItem;                                 //5

            QStandardItem *numberItem = new QStandardItem();
            numberItem->setData(number, Qt::DisplayRole);
            rowItems << numberItem;                                     //6

            QStandardItem *sizeItem = new QStandardItem();
            sizeItem->setData(size, Qt::DisplayRole);
            rowItems << sizeItem;                                       //7

            QStandardItem *usedSpaceItem = new QStandardItem();
            usedSpaceItem->setData(used_space, Qt::DisplayRole);
            rowItems << usedSpaceItem;                                  //8

            QStandardItem *freeSpaceItem = new QStandardItem();
            freeSpaceItem->setData(free_space, Qt::DisplayRole);
            rowItems << freeSpaceItem;                                  //9

            QStandardItem *totalSpaceItem = new QStandardItem();
            totalSpaceItem->setData(total_space, Qt::DisplayRole);
            rowItems << totalSpaceItem;                                 //10

            rowItems << new QStandardItem(dateTimeUpdated);             //11
            rowItems << new QStandardItem(path);                        //12
            rowItems << new QStandardItem(groupID);                     //13

            //Storage fields: add empty rows
            for (int var = 0; var < 10; ++var) {
                rowItems << new QStandardItem("");                      //14 to //23
            }

            //Catalog fields
            rowItems << new QStandardItem(catalog_file_type);           //24
            rowItems << new QStandardItem(catalog_include_hidden);      //25
            rowItems << new QStandardItem(catalog_include_metadata);    //26
            rowItems << new QStandardItem(parent_storage);              //27
            rowItems << new QStandardItem(catalog_is_full_device);      //28
            rowItems << new QStandardItem(catalog_date_loaded);         //29
            rowItems << new QStandardItem(catalog_app_version);         //30
            rowItems << new QStandardItem(catalog_file_path);           //31

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
        //Device
        ui->Devices_treeView_DeviceList->QTreeView::sortByColumn(0,Qt::AscendingOrder);
        ui->Devices_treeView_DeviceList->header()->setSectionResizeMode(QHeaderView::Interactive);
        ui->Devices_treeView_DeviceList->header()->setSectionResizeMode(0, QHeaderView::ResizeToContents); //Name
        ui->Devices_treeView_DeviceList->header()->resizeSection( 1, 100); //Type
        ui->Devices_treeView_DeviceList->header()->resizeSection( 2,  30); //Active
        ui->Devices_treeView_DeviceList->header()->resizeSection( 3,  50); //ID
        ui->Devices_treeView_DeviceList->header()->resizeSection( 4,  50); //Parent ID
        ui->Devices_treeView_DeviceList->header()->resizeSection( 5,  50); //External ID
        ui->Devices_treeView_DeviceList->header()->resizeSection( 6, 100); //Number of Files
        ui->Devices_treeView_DeviceList->header()->resizeSection( 7, 100); //Total File Size
        ui->Devices_treeView_DeviceList->header()->resizeSection( 8, 100); //Used space
        ui->Devices_treeView_DeviceList->header()->resizeSection( 9, 100); //Free space
        ui->Devices_treeView_DeviceList->header()->resizeSection(10, 100); //Total space
        ui->Devices_treeView_DeviceList->header()->setSectionResizeMode(11, QHeaderView::ResizeToContents); //Date updated
        ui->Devices_treeView_DeviceList->header()->setSectionResizeMode(12, QHeaderView::ResizeToContents); //Path
        ui->Devices_treeView_DeviceList->header()->resizeSection(13,  30); //Group ID

        //Storage (no sizing, all fields will be hidden

        //Catalog
        ui->Devices_treeView_DeviceList->header()->setSectionResizeMode(24, QHeaderView::ResizeToContents); //File type
        ui->Devices_treeView_DeviceList->header()->resizeSection(25,  50);  //Include Hidden
        ui->Devices_treeView_DeviceList->header()->resizeSection(26,  50); //Include metadata
        ui->Devices_treeView_DeviceList->header()->setSectionResizeMode(27, QHeaderView::ResizeToContents); //Parent storage
        ui->Devices_treeView_DeviceList->header()->resizeSection(28,  50); //Is full device
        ui->Devices_treeView_DeviceList->header()->resizeSection(29, 150); //Date Loaded
        ui->Devices_treeView_DeviceList->header()->resizeSection(30,  50); //App Version
        ui->Devices_treeView_DeviceList->header()->setSectionResizeMode(31, QHeaderView::ResizeToContents); //File path

    //Show and Hide
    for (int var = 14; var < 24; ++var) {
        ui->Devices_treeView_DeviceList->header()->hideSection(var); //Storage empty fields 14 to 23
    }

    ui->Devices_treeView_DeviceList->header()->hideSection( 1); //Type
    ui->Devices_treeView_DeviceList->header()->hideSection( 3); //ID
    ui->Devices_treeView_DeviceList->header()->hideSection( 4); //Parent ID
    ui->Devices_treeView_DeviceList->header()->hideSection( 8); //Used space
    ui->Devices_treeView_DeviceList->header()->hideSection( 9); //Free space
    ui->Devices_treeView_DeviceList->header()->hideSection(10); //Total space
    ui->Devices_treeView_DeviceList->header()->hideSection(13); //Group ID

    if(developmentMode==true){
        ui->Devices_treeView_DeviceList->header()->showSection(26); //catalog_include_metadata
        ui->Devices_treeView_DeviceList->header()->showSection(28); //catalog_is_full_device
    }
    else{
        ui->Devices_treeView_DeviceList->header()->hideSection(26); //catalog_include_metadata
        ui->Devices_treeView_DeviceList->header()->hideSection(28); //catalog_is_full_device
    }

    if (ui->Devices_checkBox_DisplayFullTable->isChecked()) {
        ui->Devices_treeView_DeviceList->header()->showSection(2); //Active
        ui->Devices_treeView_DeviceList->header()->showSection(5); //External ID
        ui->Devices_treeView_DeviceList->header()->showSection(29); //Date loaded
        ui->Devices_treeView_DeviceList->header()->showSection(30); //app version
        ui->Devices_treeView_DeviceList->header()->showSection(31); //File path

    } else {
        ui->Devices_treeView_DeviceList->header()->hideSection(2); //Active
        ui->Devices_treeView_DeviceList->header()->hideSection(5); //External ID
        ui->Devices_treeView_DeviceList->header()->hideSection(29); //Date loaded
        ui->Devices_treeView_DeviceList->header()->hideSection(30); //app version
        ui->Devices_treeView_DeviceList->header()->hideSection(31); //File path
    }

    if (collection->databaseMode !="Memory") { //Fields that are only relevant in Memory mode
        ui->Devices_treeView_DeviceList->header()->hideSection(29); //Date loaded
        ui->Devices_treeView_DeviceList->header()->hideSection(31); //File path
    }

    ui->Devices_treeView_DeviceList->expandAll();
}
//--------------------------------------------------------------------------

//--------------------------------------------------------------------------
//--- Storage --------------------------------------------------------------
void MainWindow::loadStorageList()
{//Load Storage selection to comboBoxes

    //Get data
    QSqlQuery query(QSqlDatabase::database("defaultConnection"));
    QString querySQL = QLatin1String(R"(
                                SELECT device_id, device_name
                                FROM   device
                                WHERE  device_type = 'Storage'

                            )");//AND    device_group_id = 0

    if ( selectedDevice->type == "Storage" ){
        querySQL += QLatin1String(R"( AND device_name ='%1' )").arg(selectedDevice->name);
        ui->Create_comboBox_StorageSelection->setCurrentText(selectedDevice->name);
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
    QString picturePath = collection->folder + "/images/" + QString::number(activeDevice->storage->ID) + ".jpg";
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
    QSqlQuery query(QSqlDatabase::database("defaultConnection"));

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
    previousCatalog.loadDevice("defaultConnection");

    //Get new values
    //Other values
    activeDevice->catalog->fileType         = ui->Catalogs_comboBox_FileType->itemData(ui->Catalogs_comboBox_FileType->currentIndex(),Qt::UserRole).toString();
    activeDevice->catalog->includeHidden    = ui->Catalogs_checkBox_IncludeHidden->isChecked();
    activeDevice->catalog->includeMetadata  = ui->Catalogs_checkBox_IncludeMetadata->isChecked();
    activeDevice->catalog->isFullDevice     = ui->Catalogs_checkBox_isFullDevice->checkState();
    //DEV:QString newIncludeSymblinks  = ui->Catalogs_checkBox_IncludeSymblinks->currentText();

    //Confirm save changes
    bool changesMade = false;
    QString message = tr("Save changes to the definition of the catalog?<br/>");
    message = message + "<table> <tr><td width=155><i>" + tr("field") + "</i></td><td width=125><i>" + tr("previous value") + "</i></td><td width=200><i>" + tr("new value") + "</i></td>";

    if(activeDevice->catalog->fileType       !=previousCatalog.catalog->fileType){
        message = message + "<tr><td>" + tr("File Type")    + "</td><td>" + previousCatalog.catalog->fileType     + "</td><td><b>" + activeDevice->catalog->fileType      + "</b></td></tr>";
        changesMade = true;
    }
    if(activeDevice->catalog->includeHidden  != previousCatalog.catalog->includeHidden){
        message = message + "<tr><td>" + tr("Include Hidden")   + "</td><td>" + QVariant(previousCatalog.catalog->includeHidden).toString()   + "</td><td><b>" + QVariant(activeDevice->catalog->includeHidden).toString()   + "</b></td></tr>";
        changesMade = true;
    }
    if(activeDevice->catalog->includeMetadata  != previousCatalog.catalog->includeMetadata){
        message = message + "<tr><td>" + tr("Include Metadata") + "</td><td>" + QVariant(previousCatalog.catalog->includeMetadata).toString() + "</td><td><b>" + QVariant(activeDevice->catalog->includeMetadata).toString() + "</b></td></tr>";
        changesMade = true;
    }
    if(activeDevice->catalog->isFullDevice  != previousCatalog.catalog->isFullDevice){
        message = message + "<tr><td>" + tr("Is Full Device") + "</td><td>" + QVariant(previousCatalog.catalog->isFullDevice).toString() + "</td><td><b>" + QVariant(activeDevice->catalog->isFullDevice).toString() + "</b></td></tr>";
        changesMade = true;
    }
    message = message + "</table>";

    if(    (activeDevice->catalog->sourcePath       !=previousCatalog.catalog->sourcePath)
        or (activeDevice->catalog->fileType         !=previousCatalog.catalog->fileType)
        or (activeDevice->catalog->includeHidden    !=previousCatalog.catalog->includeHidden)
        or (activeDevice->catalog->includeMetadata  !=previousCatalog.catalog->includeMetadata))
    {
        message = message + + "<br/><br/>" + tr("(The catalog must be updated to reflect these changes)");
    }

    if(  changesMade ){
        int result = QMessageBox::warning(this, "Katalog", message, QMessageBox::Yes | QMessageBox::Cancel);
        if ( result == QMessageBox::Cancel){
            return;
        }
    }

    activeDevice->catalog->saveCatalog();
    activeDevice->catalog->updateCatalogFileHeaders(collection->databaseMode);
    activeDevice->catalog->renameCatalogFile(activeDevice->name);

    //Write all changes to database (except change of name)
    activeDevice->catalog->saveCatalog();

    //Write changes to catalog file (update headers only)
    activeDevice->catalog->updateCatalogFileHeaders(collection->databaseMode);

    //Update the list of files if the changes impact the contents (i.e. path, file type, hidden)
    if (   activeDevice->catalog->sourcePath      != previousCatalog.catalog->sourcePath
        or activeDevice->catalog->includeHidden   != previousCatalog.catalog->includeHidden
        or activeDevice->catalog->includeMetadata != previousCatalog.catalog->includeMetadata
        or activeDevice->catalog->fileType        != previousCatalog.catalog->fileType)
    {
        int updatechoice = QMessageBox::warning(this, "Katalog",
                                                tr("Update the catalog content with the new criteria?\n")
                                                , QMessageBox::Yes
                                                    | QMessageBox::No);
        if ( updatechoice == QMessageBox::Yes){
            activeDevice->catalog->loadCatalog("defaultConnection");
            reportAllUpdates(activeDevice,
                             activeDevice->updateDevice("update",
                                                        collection->databaseMode,
                                                        true,
                                                        collection->folder,
                                                        true),
                             "update");
        }
    }

    //Refresh
    if(collection->databaseMode=="Memory")
        collection->loadCatalogFilesToTable();
}
//--------------------------------------------------------------------------
void MainWindow::updateCatalogsScreenStatistics()
{
    QSqlQuery querySumCatalogValues(QSqlDatabase::database("defaultConnection"));

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
    QString sourceFilePath = QFileDialog::getOpenFileName(this, tr("Select the csv file to be imported"), collection->folder);

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
    QSqlQuery deleteQuery(QSqlDatabase::database("defaultConnection"));
    deleteQuery.exec("DELETE FROM file");

    //prepare query to load file info
    QSqlQuery insertQuery(QSqlDatabase::database("defaultConnection"));
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
    QSqlQuery insertFolderQuery(QSqlDatabase::database("defaultConnection"));
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
    QSqlQuery listCatalogQuery(QSqlDatabase::database("defaultConnection"));
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
        importedDevice.catalog->filePath = collection->folder + "/" + importedDevice.name + ".idx";
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
        QSqlQuery listCatalogQuery(QSqlDatabase::database("defaultConnection"));
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
        QSqlQuery listFilesQuery(QSqlDatabase::database("defaultConnection"));
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
        QFile fileFolderOut(collection->folder + "/" + importedDevice.name + ".folders.idx");

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
            QSqlQuery listFoldersQuery(QSqlDatabase::database("defaultConnection"));
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
            message += "<table>";
            message += "<tr><td>" + tr("Catalog updated: ") + "</td><td align='center'><b>" + device->name + "</b></td></tr>";
            message += "<tr><td>" + tr("Path: ")            + "</td><td align='right'> <b>" + device->path + "</b></td></tr>";
            message += "</table>";
            message += "<br/><table>";
            message += "<tr><td>" + tr("Number of files: ") + "</td><td align='center'><b>" + QString::number(list[1]) + "</b></td><td>&nbsp; &nbsp; "             + tr("(added: ") + "&nbsp; &nbsp; </td><td align='right'><b>" + QString::number(list[2])             + "</b>)</td></tr>";
            message += "<tr><td>" + tr("Total file size: ") + "</td><td align='right'> <b>" + QLocale().formattedDataSize(list[3]) + "</b></td><td>&nbsp; &nbsp; " + tr("(added: ") + "&nbsp; &nbsp; </td><td align='right'><b>" + QLocale().formattedDataSize(list[4]) + "</b>)</td></tr>";
            message += "</table>";
            reportAvailable = true;
        }

        if(list[7]==1){//Parent storage updated
            Device parentDevice;
            parentDevice.ID = device->parentID;
            parentDevice.loadDevice("defaultConnection");

            message += "<br/>";
            message += "<table>";
            message += "<tr><td>"+tr("Storage updated: ")+ "</td><td align='center'><b>" + parentDevice.name + "</b></td></tr>";
            message += "<tr><td>"+tr("Path: ")           + "</td><td align='right'> <b>" + parentDevice.path + "</b></td></tr>";
            message += "</table>";
            message += "<br/>";
            message += "<table>";
            message += "<tr><td>" +  tr("Used Space: ") + "</td><td align='right'><b>" + QLocale().formattedDataSize(list[8])  + "</b></td><td>&nbsp; &nbsp; " + tr("(added: ") + "&nbsp; &nbsp; </td><td align='right'><b>" + QLocale().formattedDataSize(list[9])  + "</b>)</td></tr>";
            message += "<tr><td>" +  tr("Free Space: ") + "</td><td align='right'><b>" + QLocale().formattedDataSize(list[10]) + "</b></td><td>&nbsp; &nbsp; " + tr("(added: ") + "&nbsp; &nbsp; </td><td align='right'><b>" + QLocale().formattedDataSize(list[11]) + "</b>)</td></tr>";
            message += "<tr><td>" + tr("Total Space: ") + "</td><td align='right'><b>" + QLocale().formattedDataSize(list[12]) + "</b></td><td>&nbsp; &nbsp; " + tr("(added: ") + "&nbsp; &nbsp; </td><td align='right'><b>" + QLocale().formattedDataSize(list[13]) + "</b>)</td></tr>";
            message += "</table>";
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
        if(list[0]==1){//Catalog created
            message += "<table>";
            message += "<tr><td>" + tr("Catalog created: ") + "</td><td align='center'><b>" + device->name + "</b></td></tr>";
            message += "<tr><td>" + tr("Path: ")            + "</td><td align='right'> <b>" + device->path + "</b></td></tr>";
            message += "</table>";
            message += "<br/><table>";
            message += "<tr><td>" + tr("Number of files: ") + "</td><td align='center'><b>" + QString::number(list[1])             + "</b></td></tr>";
            message += "<tr><td>" + tr("Total file size: ") + "</td><td align='right'> <b>" + QLocale().formattedDataSize(list[3]) + "</b></td></tr>";
            message += "</table>";
            reportAvailable = true;
        }

        if(list[7]==1){//Parent storage updated
            Device parentDevice;
            parentDevice.ID = device->parentID;
            parentDevice.loadDevice("defaultConnection");

            message += "<br/>";
            message += "<table>";
            message += "<tr><td>"+tr("Storage updated: ")+ "</td><td align='center'><b>" + parentDevice.name + "</b></td></tr>";
            message += "<tr><td>"+tr("Path: ")           + "</td><td align='right'> <b>" + parentDevice.path + "</b></td></tr>";
            message += "</table>";
            message += "<br/>";
            message += "<table>";
            message += "<tr><td>" +  tr("Used Space: ") + "</td><td align='right'><b>" + QLocale().formattedDataSize(list[8])  + "</b></td><td>&nbsp; &nbsp; " + tr("(added: ") + "&nbsp; &nbsp; </td><td align='right'><b>" + QLocale().formattedDataSize(list[9])  + "</b>)</td></tr>";
            message += "<tr><td>" +  tr("Free Space: ") + "</td><td align='right'><b>" + QLocale().formattedDataSize(list[10]) + "</b></td><td>&nbsp; &nbsp; " + tr("(added: ") + "&nbsp; &nbsp; </td><td align='right'><b>" + QLocale().formattedDataSize(list[11]) + "</b>)</td></tr>";
            message += "<tr><td>" + tr("Total Space: ") + "</td><td align='right'><b>" + QLocale().formattedDataSize(list[12]) + "</b></td><td>&nbsp; &nbsp; " + tr("(added: ") + "&nbsp; &nbsp; </td><td align='right'><b>" + QLocale().formattedDataSize(list[13]) + "</b>)</td></tr>";
            message += "</table>";
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
        message += "<br/>";
        message += "<table>";
        message += "<tr><td>"+tr("Storage updated: ")+ "</td><td align='center'><b>" + device->name + "</b></td></tr>";
        message += "<tr><td>"+tr("Path: ")           + "</td><td align='right'> <b>" + device->path + "</b></td></tr>";
        message += "</table>";
        message += "<br/>";
        message += "<table>";
        message += "<tr><td>" +  tr("Used Space: ") + "</td><td align='right'><b>" + QLocale().formattedDataSize(list[8])  + "</b></td><td>&nbsp; &nbsp; " + tr("(added: ") + "&nbsp; &nbsp; </td><td align='right'><b>" + QLocale().formattedDataSize(list[9])  + "</b>)</td></tr>";
        message += "<tr><td>" +  tr("Free Space: ") + "</td><td align='right'><b>" + QLocale().formattedDataSize(list[10]) + "</b></td><td>&nbsp; &nbsp; " + tr("(added: ") + "&nbsp; &nbsp; </td><td align='right'><b>" + QLocale().formattedDataSize(list[11]) + "</b>)</td></tr>";
        message += "<tr><td>" + tr("Total Space: ") + "</td><td align='right'><b>" + QLocale().formattedDataSize(list[12]) + "</b></td><td>&nbsp; &nbsp; " + tr("(added: ") + "&nbsp; &nbsp; </td><td align='right'><b>" + QLocale().formattedDataSize(list[13]) + "</b>)</td></tr>";
        message += "</table>";

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
            message += "<br/>";
            message += "<table>";
            message += "<tr><td>"+tr("Storage updated: ")+ "</td><td align='center'><b>" + device->name + "</b></td></tr>";
            message += "<tr><td>"+tr("Path: ")           + "</td><td align='right'> <b>" + device->path + "</b></td></tr>";
            message += "</table>";
            message += "<br/>";
            message += "<table>";
            message += "<tr><td>" +  tr("Used Space: ") + "</td><td align='right'><b>" + QLocale().formattedDataSize(list[8])  + "</b></td><td>&nbsp; &nbsp; " + tr("(added: ") + "&nbsp; &nbsp; </td><td align='right'><b>" + QLocale().formattedDataSize(list[9])  + "</b>)</td></tr>";
            message += "<tr><td>" +  tr("Free Space: ") + "</td><td align='right'><b>" + QLocale().formattedDataSize(list[10]) + "</b></td><td>&nbsp; &nbsp; " + tr("(added: ") + "&nbsp; &nbsp; </td><td align='right'><b>" + QLocale().formattedDataSize(list[11]) + "</b>)</td></tr>";
            message += "<tr><td>" + tr("Total Space: ") + "</td><td align='right'><b>" + QLocale().formattedDataSize(list[12]) + "</b></td><td>&nbsp; &nbsp; " + tr("(added: ") + "&nbsp; &nbsp; </td><td align='right'><b>" + QLocale().formattedDataSize(list[13]) + "</b>)</td></tr>";
            message += "</table>";
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
    QSqlQuery query(QSqlDatabase::database("defaultConnection"));

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
            QSqlQuery checkQuery(QSqlDatabase::database("defaultConnection"));
            checkQuery.prepare("SELECT 1 FROM folder WHERE folder_catalog_name = :catalog AND folder_path = :path");
            checkQuery.bindValue(":catalog", folderCatalogName);
            checkQuery.bindValue(":path", currentPath);

            if (!checkQuery.exec()) {
                qDebug() << "Error checking path:" << checkQuery.lastError().text();
            }

            // If the current path doesn't exist, insert it
            if (!checkQuery.next()) {
                QSqlQuery insertQuery(QSqlDatabase::database("defaultConnection"));
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
            loopDevice.loadDevice("defaultConnection");
            loopDevice.catalog->appVersion = currentVersion;

            QList<qint64> list = loopDevice.updateDevice("update",
                                                         collection->databaseMode,
                                                         false,
                                                         collection->folder,
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
                                                collection->folder,
                                                true),
                     "update");
    collection->saveDeviceTableToFile();
    collection->saveStatiticsToFile();

    loadDevicesView();
    loadStatisticsChart();
}
//--------------------------------------------------------------------------
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
//--- Migration 1.22 to 2.0
//--------------------------------------------------------------------------
//------ Global method -----------------------------------------------
void MainWindow::migrateCollection()
{
    // Start animation while opening
    QApplication::setOverrideCursor(Qt::WaitCursor);

    //Convert Storage
        convertStorage();

    //Devices
        //Delete default virtual and storage
        QSqlQuery query(QSqlDatabase::database("defaultConnection"));
        QString querySQL = QLatin1String(R"(
                                        DELETE FROM device
                                        WHERE device_id ='2' OR device_id = '3'
                                    )");
        query.prepare(querySQL);
        query.exec();

        QString query2SQL = "DELETE FROM storage WHERE storage_id = (SELECT MAX(storage_id) FROM storage);";
        query.prepare(query2SQL);
        query.exec();

        QString query3SQL = "DELETE FROM parameter WHERE parameter_name = 'version'";
        query.prepare(query3SQL);
        query.exec();

        //Import Virtual devices in Physical group from locations
        importVirtualToDevices();

        //Create from storage
        importStorageToDevices();

        //Create from catalog
        importCatalogsToDevices();

        //Create Catalog IDs
        generateAndAssociateCatalogMissingIDs();

    //Statistics
        importStatistics();

    //Assignments of catalogs to virutal devices
        importVirtualAssignmentsToDevices();

    //Convert .folders.idx Files
        convertFoldersIdxFiles();

    //Convert exclude.csv file into parameter.csv
        importExcludeIntoParameter();

    //Convert Tags
        convertTags();

    //Convert Search History
        convertSearchHistory();



    //Close procedure
        //Add the current version
        QSqlQuery insertQuery(QSqlDatabase::database("defaultConnection"));
        QString insertSQL = QLatin1String(R"(
                                        INSERT INTO parameter (
                                                    parameter_name,
                                                    parameter_type,
                                                    parameter_value1)
                                        VALUES(
                                                    :parameter_name,
                                                    :parameter_type,
                                                    :parameter_value1)
                                )");
        insertQuery.prepare(insertSQL);
        insertQuery.bindValue(":parameter_name", "version");
        insertQuery.bindValue(":parameter_type", "collection");
        insertQuery.bindValue(":parameter_value1", "2.0");
        insertQuery.exec();
        collection->saveParameterTableToFile();

        QMessageBox msgBox;
        msgBox.setWindowTitle("Katalog");
        msgBox.setText(tr("Upgraded collection to v2.0."));
        msgBox.setIcon(QMessageBox::Information);
        msgBox.exec();

    loadCollection();
    loadDevicesView();
    loadDevicesTreeToModel("Filters");
    QApplication::restoreOverrideCursor();
    ui->tabWidget->setCurrentIndex(1);
}
//------ Method per object --------------------------------------------------------------
void MainWindow::importVirtualToDevices()
{
    //Create Virtual device in Physical group from locations
    QSqlQuery query(QSqlDatabase::database("defaultConnection"));
    QString querySQL = QLatin1String(R"(
                                    SELECT DISTINCT storage_location
                                    FROM storage
                                    ORDER BY storage_location ASC
                                )");
    query.prepare(querySQL);
    query.exec();

    while(query.next()){
        Device newDevice;
        newDevice.generateDeviceID();
        newDevice.parentID = 1;
        newDevice.name = query.value(0).toString();
        newDevice.type = "Virtual";
        newDevice.insertDevice();
    }
    collection->saveDeviceTableToFile();
}

void MainWindow::importStorageToDevices()
{
    //Create from storage
    QSqlQuery query(QSqlDatabase::database("defaultConnection"));
    QString querySQL = QLatin1String(R"(
                                    SELECT  storage_id,
                                            storage_name,
                                            storage_path,
                                            storage_location,
                                            storage_total_space,
                                            storage_free_space
                                    FROM storage
                                    ORDER BY storage_id ASC
                                )");
    query.prepare(querySQL);
    query.exec();

    while(query.next()){
        Device newDevice;
        newDevice.generateDeviceID();
        newDevice.externalID = query.value(0).toInt();
        newDevice.name = query.value(1).toString();
        newDevice.path = query.value(2).toString();
        newDevice.totalSpace = query.value(4).toLongLong();
        newDevice.freeSpace = query.value(5).toLongLong();
        newDevice.type = "Storage";
        newDevice.groupID = 0;

        //Find parent id
            QSqlQuery queryParent(QSqlDatabase::database("defaultConnection"));
            QString queryParentSQL = QLatin1String(R"(
                                        SELECT device_id
                                        FROM device
                                        WHERE device_name =:device_name
                                        AND device_type = 'Virtual'
                                    )");
            queryParent.prepare(queryParentSQL);
            queryParent.bindValue(":device_name", query.value(3).toString());
            queryParent.exec();
            queryParent.next();
            newDevice.parentID = queryParent.value(0).toInt();

        newDevice.insertDevice();
        newDevice.updateParentsNumbers();
    }
    collection->saveDeviceTableToFile();
}

void MainWindow::importCatalogsToDevices()
{
    //Create from catalogs
    QSqlQuery query(QSqlDatabase::database("defaultConnection"));
    QString querySQL = QLatin1String(R"(
                                    SELECT  catalog_id,
                                            catalog_name,
                                            catalog_storage,
                                            catalog_source_path,
                                            catalog_file_count,
                                            catalog_total_file_size
                                    FROM catalog
                                )");
    query.prepare(querySQL);
    query.exec();

    while(query.next()){
        Device newDevice;
        newDevice.generateDeviceID();
        newDevice.name = query.value(1).toString();
        newDevice.path = query.value(3).toString();
        newDevice.totalFileCount = query.value(4).toLongLong();
        newDevice.totalFileSize = query.value(5).toLongLong();
        newDevice.type = "Catalog";
        newDevice.groupID = 0;

        //Find parent id
        QSqlQuery queryParent(QSqlDatabase::database("defaultConnection"));
        QString queryParentSQL = QLatin1String(R"(
                                        SELECT device_id
                                        FROM device
                                        WHERE device_name =:device_name
                                        AND device_type = 'Storage'
                                    )");
        queryParent.prepare(queryParentSQL);
        queryParent.bindValue(":device_name", query.value(2).toString());
        queryParent.exec();
        queryParent.next();
        newDevice.parentID = queryParent.value(0).toInt();

        newDevice.insertDevice();
        newDevice.updateParentsNumbers();
    }

    collection->saveDeviceTableToFile();
}

void MainWindow::generateAndAssociateCatalogMissingIDs()
{
    //Get catalogs with missing ID
    QSqlQuery query(QSqlDatabase::database("defaultConnection"));
    QString querySQL = QLatin1String(R"(
                                    SELECT device_id
                                    FROM device
                                    WHERE device_type = 'Catalog'
                                )");
    query.prepare(querySQL);
    query.exec();

    //Loop and generate an ID
    while(query.next()){
        Device device;
        device.ID = query.value(0).toInt();
        device.loadDevice("defaultConnection");

        if (device.catalog->ID == 0){
            device.catalog->generateID();

            //Update database with catalog values
            QSqlQuery query(QSqlDatabase::database("defaultConnection"));
            QString querySQL = QLatin1String(R"(
                                    UPDATE catalog
                                    SET    catalog_id =:catalog_id
                                    WHERE  catalog_name=:catalog_name
                                )");
            query.prepare(querySQL);
            query.bindValue(":catalog_id", device.catalog->ID);
            query.bindValue(":catalog_name", device.name);
            query.exec();
        }

        device.externalID = device.catalog->ID;
        device.saveDevice();
        device.loadDevice("defaultConnection");
    }

    //Update all catalog devices external id
    querySQL = QLatin1String(R"(
                                    UPDATE device
                                    SET device_external_id = (SELECT catalog_id FROM catalog WHERE device_name = catalog_name)
                                    WHERE device_type = 'Catalog'
                                )");
    query.prepare(querySQL);
    query.exec();

    collection->saveDeviceTableToFile();


    //Update all catalog files with their new ID
    QSqlQuery queryUpdateFiles(QSqlDatabase::database("defaultConnection"));
    QString queryUpdateFilesSQL = QLatin1String(R"(
                                    SELECT device_id, device_name
                                    FROM device
                                    WHERE device_type ='Catalog'
                                )");
    queryUpdateFiles.prepare(queryUpdateFilesSQL);
    queryUpdateFiles.exec();

    while(queryUpdateFiles.next()){
        Device tempDevice;
        tempDevice.ID = queryUpdateFiles.value(0).toInt();
        tempDevice.loadDevice("defaultConnection");
        tempDevice.catalog->updateCatalogFileHeaders(collection->databaseMode);
    }
}

void MainWindow::importVirtualAssignmentsToDevices()
{   
    //Load data
    loadVirtualStorageFileToTable();
    loadVirtualStorageCatalogFileToTable();

    //Virtual storage
        QSqlQuery query(QSqlDatabase::database("defaultConnection"));
        QString querySQL;

        //Create a temporary table
        querySQL = QLatin1String(R"(
                        CREATE TEMPORARY TABLE IF NOT EXISTS temp_device (
                            device_id NUMERIC,
                            device_parent_id NUMERIC,
                            device_name TEXT,
                            device_type TEXT,
                            device_external_id NUMERIC,
                            device_path TEXT,
                            device_total_file_size NUMERIC DEFAULT 0,
                            device_total_file_count NUMERIC DEFAULT 0,
                            device_total_space NUMERIC DEFAULT 0,
                            device_free_space NUMERIC DEFAULT 0,
                            device_active NUMERIC,
                            device_group_id NUMERIC,
                            device_date_updated TEXT,
                            device_order NUMERIC
                        )
                    )");
        query.prepare(querySQL);
        query.exec();

        //Insert data from virtual_storage into the temporary table
        querySQL = QLatin1String(R"(
                        INSERT INTO temp_device (
                            device_id,
                            device_parent_id,
                            device_name,
                            device_type,
                            device_group_id
                        )
                        SELECT
                            virtual_storage_id,
                            virtual_storage_parent_id,
                            virtual_storage_name,
                            'Virtual',
                            1
                        FROM
                            virtual_storage
                    )");
        query.prepare(querySQL);
        query.exec();

        //Get max ID to set a shift amount for new IDs to avoid duplicates
        int shiftAmount = 1;
        query.exec("SELECT MAX(device_id) FROM device");
        query.next();
        shiftAmount = query.value(0).toInt();

        //Shift ID in temp table

            // First, update the rows with parentID = 0 to keep them unchanged
            QString sql = "UPDATE temp_device SET device_id = device_id + :shiftAmount "
                          "WHERE device_parent_id = 0";
            query.prepare(sql);
            query.bindValue(":shiftAmount", shiftAmount);
            if (!query.exec()) {
                qDebug() << "shiftIDsInDeviceTable - Error updating device table:" << query.lastError().text();
                return;
            }

            // Next, update the rows with parentID != 0 to shift their IDs
            sql = "UPDATE temp_device SET device_id = device_id + :shiftAmount, "
                  "device_parent_id = device_parent_id + :shiftAmount "
                  "WHERE device_parent_id != 0";
            query.prepare(sql);
            query.bindValue(":shiftAmount", shiftAmount);
            if (!query.exec()) {
                qDebug() << "Error updating device table:" << query.lastError().text();
                return;
            }

        //Insert new devices
        querySQL = QLatin1String(R"(
                        INSERT OR IGNORE INTO device (
                            device_id,
                            device_parent_id,
                            device_name,
                            device_type,
                            device_group_id
                        )
                        SELECT
                            device_id,
                            device_parent_id,
                            device_name,
                            device_type,
                            device_group_id
                        FROM
                            temp_device
                    )");
        query.prepare(querySQL);
        query.exec();


    //Catalog Assignements
        //Create new devices for catalogs assigned to virtual devices
        querySQL = QLatin1String(R"(
                                    SELECT *
                                    FROM virtual_storage_catalog
                                )");
        query.prepare(querySQL);
        query.exec();
        qDebug()<<query.lastError();

        while(query.next()){
            Device newCatalog;
            newCatalog.generateDeviceID();
            newCatalog.parentID = query.value(0).toInt() + shiftAmount;
            newCatalog.name = query.value(1).toString();
            newCatalog.type = "Catalog";
            newCatalog.groupID = 1;
            newCatalog.insertDevice();
            newCatalog.updateParentsNumbers();
        }

        //Update values from catalog with same name
        querySQL = QLatin1String(R"(
                UPDATE device AS d1
                SET
                    device_external_id = d2.device_external_id,
                    device_total_file_size = d2.device_total_file_size,
                    device_total_file_count = d2.device_total_file_count,
                    device_path = d2.device_path
                FROM
                    device AS d2
                WHERE
                    d1.device_id > :shiftAmount
                    AND d2.device_id <= :shiftAmount
                    AND d1.device_name = d2.device_name
                    AND d2.device_external_id IS NOT NULL
               )");

        query.prepare(querySQL);
        query.bindValue(":shiftAmount", shiftAmount);
        query.exec();

        //Update parents numbers
        querySQL = QLatin1String(R"(
                                    SELECT device_id
                                    FROM device
                                    WHERE device_type = 'Catalog'
                                    AND device_group_id = 1
                                )");
        query.prepare(querySQL);
        query.exec();
        qDebug()<<query.lastError();

        while(query.next()){
            Device tempCatalog;
            tempCatalog.ID = query.value(0).toInt();
            tempCatalog.loadDevice("defaultConnection");
            tempCatalog.updateParentsNumbers();
        }

    //Save results
    collection->saveDeviceTableToFile();
}

void MainWindow::importStatistics()
{
    QSqlQuery query(QSqlDatabase::database("defaultConnection"));
    QString querySQL;

    //Load data from statistics_storage file
    loadStatisticsStorageFileToTable();

    //Insert data from statistics_storage
    querySQL = QLatin1String(R"(
                    INSERT INTO statistics_device (
                        date_time,
                        device_id,
                        device_name,
                        device_type,
                        device_file_count,
                        device_total_file_size,
                        device_free_space,
                        device_total_space,
                        record_type
                    )
                    SELECT
                        statistics_storage.date_time,
                        device.device_id,
                        device.device_name,
                        'Storage',
                        NULL,
                        NULL,
                        statistics_storage.storage_free_space,
                        statistics_storage.storage_total_space,
                        statistics_storage.record_type
                    FROM
                        statistics_storage
                    JOIN
                        storage ON statistics_storage.storage_id = storage.storage_id
                    JOIN
                        device ON storage.storage_id = device.device_external_id
                    WHERE
                        device.device_type = 'Storage';
                )");
    query.prepare(querySQL);
    query.exec();

    //Load data from statistics_catalog file
    loadStatisticsCatalogFileToTable();

    //Insert data from statistics_catalog
    querySQL = QLatin1String(R"(
                    INSERT INTO statistics_device (
                        date_time,
                        device_id,
                        device_name,
                        device_type,
                        device_file_count,
                        device_total_file_size,
                        device_free_space,
                        device_total_space,
                        record_type
                    )
                    SELECT
                        statistics_catalog.date_time,
                        device.device_id,
                        device.device_name,
                        'Catalog',
                        statistics_catalog.catalog_file_count,
                        statistics_catalog.catalog_total_file_size,
                        NULL,
                        NULL,
                        statistics_catalog.record_type
                    FROM
                        statistics_catalog
                    JOIN
                        catalog ON statistics_catalog.catalog_name = catalog.catalog_name
                    JOIN
                        device ON catalog.catalog_id = device.device_external_id
                    WHERE
                        device.device_type = 'Catalog';
                )");
    query.prepare(querySQL);
    query.exec();

    //Update Storage file stats from Catalogs when snapshots
    querySQL = QLatin1String(R"(
                    UPDATE statistics_device AS sd_storage
                    SET
                        device_file_count = (
                            SELECT SUM(sd_catalog.device_file_count)
                            FROM statistics_device AS sd_catalog
                            JOIN device AS d_catalog ON sd_catalog.device_id = d_catalog.device_id
                            WHERE
                                sd_catalog.date_time = sd_storage.date_time AND
                                sd_catalog.record_type = 'snapshot' AND
                                d_catalog.device_type = 'Catalog' AND
                                d_catalog.device_parent_id = sd_storage.device_id
                        ),
                        device_total_file_size = (
                            SELECT SUM(sd_catalog.device_total_file_size)
                            FROM statistics_device AS sd_catalog
                            JOIN device AS d_catalog ON sd_catalog.device_id = d_catalog.device_id
                            WHERE
                                sd_catalog.date_time = sd_storage.date_time AND
                                sd_catalog.record_type = 'snapshot' AND
                                d_catalog.device_type = 'Catalog' AND
                                d_catalog.device_parent_id = sd_storage.device_id
                        )
                    WHERE
                        sd_storage.record_type = 'snapshot' AND
                        sd_storage.device_type = 'Storage';
                )");
    query.prepare(querySQL);
    query.exec();

    collection->saveStatiticsToFile();
}

void MainWindow::loadStatisticsCatalogFileToTable()
{// Load the contents of the statistics file into the database

    QString statisticsCatalogFilePath;
    statisticsCatalogFilePath = collection->folder + "/statistics_catalog.csv";

    //clear database table
    QSqlQuery deleteQuery(QSqlDatabase::database("defaultConnection"));
    deleteQuery.exec("DELETE FROM statistics_catalog");

    // Get infos stored in the file
    QFile statisticsCatalogFile(statisticsCatalogFilePath);
    if(!statisticsCatalogFile.open(QIODevice::ReadOnly)) {
        return;
    }

    QTextStream textStream(&statisticsCatalogFile);

    //prepare query to load file info
    QSqlQuery insertQuery(QSqlDatabase::database("defaultConnection"));
    QString insertSQL = QLatin1String(R"(
                                INSERT INTO statistics_catalog (
                                                date_time,
                                                catalog_name,
                                                catalog_file_count,
                                                catalog_total_file_size,
                                                record_type )
                                VALUES(
                                                :date_time,
                                                :catalog_name,
                                                :catalog_file_count,
                                                :catalog_total_file_size,
                                                :record_type )
                                            )");
    insertQuery.prepare(insertSQL);

    //set temporary values
    QString     line;
    QStringList fieldList;

    QString     dateTime;
    QString     catalogName;
    qint64      catalogFileCount;
    qint64      catalogTotalFileSize;
    QString     recordType;
    QRegularExpression tagExp("\t");

    //Skip titles line
    //line = textStream.readLine();

    //load file to database
    while (!textStream.atEnd())
    {
        line = textStream.readLine();
        if (line.isNull())
            break;
        else
        {
            //Split the string with \t (tabulation) into a list
            fieldList.clear();
            fieldList = line.split(tagExp);
            dateTime                = fieldList[0];
            catalogName             = fieldList[1];
            catalogFileCount        = fieldList[2].toLongLong();
            catalogTotalFileSize    = fieldList[3].toLongLong();
            recordType              = fieldList[4];

            //Append data to the database
            insertQuery.bindValue(":date_time", dateTime);
            insertQuery.bindValue(":catalog_name", catalogName);
            insertQuery.bindValue(":catalog_file_count", QString::number(catalogFileCount));
            insertQuery.bindValue(":catalog_total_file_size", QString::number(catalogTotalFileSize));
            insertQuery.bindValue(":record_type", recordType);
            insertQuery.exec();
        }
    }
}

void MainWindow::loadStatisticsStorageFileToTable()
{// Load the contents of the storage statistics file into the database

    QString statisticsStorageFilePath;
    statisticsStorageFilePath = collection->folder + "/statistics_storage.csv";

    //clear database table
    QSqlQuery deleteQuery(QSqlDatabase::database("defaultConnection"));
    deleteQuery.exec("DELETE FROM statistics_storage");

    // Get infos stored in the file
    QFile statisticsStorageFile(statisticsStorageFilePath);
    if(!statisticsStorageFile.open(QIODevice::ReadOnly)) {
        return;
    }

    QTextStream textStream(&statisticsStorageFile);

    //prepare query to load file info
    QSqlQuery insertQuery(QSqlDatabase::database("defaultConnection"));
    QString insertSQL = QLatin1String(R"(
                                INSERT INTO statistics_storage (
                                                date_time,
                                                storage_id,
                                                storage_name,
                                                storage_free_space,
                                                storage_total_space,
                                                record_type )
                                VALUES(
                                                :date_time,
                                                :storage_id,
                                                :storage_name,
                                                :storage_free_space,
                                                :storage_total_space,
                                                :record_type )
                                            )");
    insertQuery.prepare(insertSQL);

    //set temporary values
    QString     line;
    QStringList fieldList;
    QString     dateTime;
    int         storageID;
    QString     storageName;
    qint64      storageFreeSpace;
    qint64      storageTotalSpace;
    QString     recordType;
    QRegularExpression tagExp("\t");

    //Skip titles line
    //line = textStream.readLine();

    //load file to database
    while (!textStream.atEnd())
    {
        line = textStream.readLine();
        if (line.isNull())
            break;
        else
        {
            //Split the string with \t (tabulation) into a list
            fieldList.clear();
            fieldList = line.split(tagExp);
            dateTime            = fieldList[0];
            storageName         = fieldList[1];
            storageFreeSpace    = fieldList[2].toLongLong();
            storageTotalSpace   = fieldList[3].toLongLong();
            storageID           = fieldList[4].toInt();
            recordType          = fieldList[5];

            //Append data to the database
            insertQuery.bindValue(":date_time", dateTime);
            insertQuery.bindValue(":storage_id", storageID);
            insertQuery.bindValue(":storage_name", storageName);
            insertQuery.bindValue(":storage_free_space", QString::number(storageFreeSpace));
            insertQuery.bindValue(":storage_total_space", QString::number(storageTotalSpace));
            insertQuery.bindValue(":record_type", recordType);
            insertQuery.exec();
        }
    }
}

void MainWindow::loadVirtualStorageFileToTable()
{
    QString virtualStorageFilePath;
    virtualStorageFilePath = collection->folder + "/virtual_storage.csv";

    QSqlQuery query(QSqlDatabase::database("defaultConnection"));
    QString querySQL;
    querySQL = QLatin1String(R"(
                        DELETE FROM virtual_storage
                    )");
    query.prepare(querySQL);
    query.exec();

    querySQL = QLatin1String(R"(
                        INSERT INTO virtual_storage (
                                        virtual_storage_id,
                                        virtual_storage_parent_id,
                                        virtual_storage_name )
                        VALUES(         :virtual_storage_id,
                                        :virtual_storage_parent_id,
                                        :virtual_storage_name )
                    )");
    query.prepare(querySQL);

    //Define storage file and prepare stream
    QFile virtualStorageFile(virtualStorageFilePath);
    QTextStream textStream(&virtualStorageFile);

    //Open file or return information
    if(!virtualStorageFile.open(QIODevice::ReadOnly)) {
        // Create it, if it does not exist
        QFile newVirtualStorageFile(virtualStorageFilePath);
        newVirtualStorageFile.open(QFile::WriteOnly | QFile::Text);
        QTextStream stream(&newVirtualStorageFile);
        stream << "ID"            << "\t"
               << "Parent ID"     << "\t"
               << "Name"          << "\t"
               << '\n';
        newVirtualStorageFile.close();
    }

    //Test file validity (application breaks between v0.13 and v0.14)
    QString line = textStream.readLine();

    //Load virtualStorage device lines to table
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
                                INSERT INTO virtual_storage(
                                        virtual_storage_id,
                                        virtual_storage_parent_id,
                                        virtual_storage_name )
                                VALUES(
                                        :virtual_storage_id,
                                        :virtual_storage_parent_id,
                                        :virtual_storage_name )
                                )");

                QSqlQuery insertQuery(QSqlDatabase::database("defaultConnection"));
                insertQuery.prepare(querySQL);
                insertQuery.bindValue(":virtual_storage_id",fieldList[0].toInt());
                insertQuery.bindValue(":virtual_storage_parent_id",fieldList[1]);
                insertQuery.bindValue(":virtual_storage_name",fieldList[2]);
                insertQuery.exec();
            }
    }
    virtualStorageFile.close();
}

void MainWindow::loadVirtualStorageCatalogFileToTable()
{
    QString virtualStorageCatalogFilePath;
    virtualStorageCatalogFilePath = collection->folder + "/virtual_storage_catalog.csv";

    QSqlQuery query(QSqlDatabase::database("defaultConnection"));
    QString querySQL;
    querySQL = QLatin1String(R"(
                        DELETE FROM virtual_storage_catalog
                    )");
    query.prepare(querySQL);
    query.exec();

    querySQL = QLatin1String(R"(
                        INSERT INTO virtual_storage_catalog (
                                        virtual_storage_id,
                                        catalog_name,
                                        directory_path )
                        VALUES(         :virtual_storage_id,
                                        :catalog_name,
                                        :directory_path )
                    )");
    query.prepare(querySQL);

    //Define storage file and prepare stream
    QFile virtualStorageCatalogFile(virtualStorageCatalogFilePath);
    QTextStream textStream(&virtualStorageCatalogFile);

    //Open file or return information
    if(!virtualStorageCatalogFile.open(QIODevice::ReadOnly)) {
        // Create it, if it does not exist
        QFile newVirtualStorageCatalogFile(virtualStorageCatalogFilePath);
        if(!newVirtualStorageCatalogFile.open(QIODevice::ReadOnly)) {
            if (newVirtualStorageCatalogFile.open(QFile::WriteOnly | QFile::Text)) {
                QTextStream stream(&newVirtualStorageCatalogFile);
                stream << "ID"            << "\t"
                       << "Catalog Name"          << "\t"
                       << "Directory Path"          << "\t"
                       << '\n';
                newVirtualStorageCatalogFile.close();
            }
        }
    }

    //Test file validity (application breaks between v0.13 and v0.14)
    QString line = textStream.readLine();

    //Load virtualStorage device lines to table
    while (true)
    {
        QString line = textStream.readLine();
        if (line.isNull())
            break;
        else if (line.left(2)!="ID"){//skip the first line with headers

            //Split the string with tabulation into a list
            QStringList fieldList = line.split('\t');
            query.bindValue(":virtual_storage_id",fieldList[0].toInt());
            query.bindValue(":catalog_name",fieldList[1]);
            query.bindValue(":directory_path",fieldList[2]);
            query.exec();
        }
    }
    virtualStorageCatalogFile.close();
}

void MainWindow::convertFoldersIdxFiles()
{//Convert catalog folder.idx files
    QSqlQuery query(QSqlDatabase::database("defaultConnection"));
    QString querySQL = QLatin1String(R"(
                            SELECT device_id, device_external_id
                            FROM device
                            WHERE device_id IN (
                                SELECT MIN(device_id)
                                FROM device
                                WHERE device_type = 'Catalog'
                                GROUP BY device_external_id
                            )
                            ORDER BY device_external_id;
                        )");
    query.prepare(querySQL);
    query.exec();

    while(query.next()){
        Device tempDevice;
        tempDevice.ID = query.value(0).toInt();
        tempDevice.loadDevice("defaultConnection");

        QFileInfo fileInfo(tempDevice.catalog->filePath);
        QString fileNameWithOutExtension = fileInfo.baseName();
        QString fileFolder = fileInfo.absolutePath();
        QString folderIdxFilePath = fileFolder + "/" + fileNameWithOutExtension + ".folders.idx";
        QFile folderIdxFile(folderIdxFilePath);

        if (folderIdxFile.exists()){

            QFile file(folderIdxFilePath);

            if (file.open(QIODevice::ReadOnly | QIODevice::Text)) {
                QTextStream in(&file);
                QStringList lines = in.readAll().split('\n');
                file.close();

                if (file.open(QIODevice::WriteOnly | QIODevice::Text | QIODevice::Truncate)) {
                    QTextStream out(&file);

                    foreach (const QString &line, lines) {
                        if (!line.isEmpty()) {
                            QStringList lineParts = line.split('\t');
                            if (lineParts.size() >= 2) {
                                QString secondValue = lineParts[1];
                                out << tempDevice.externalID << "\t" << secondValue << "\n";
                            }
                        }
                    }

                    file.close();
                }
            }
        }
    }
}

void MainWindow::importExcludeIntoParameter()
{//Import exclude.csv entries into parameter.csv

    QString excludeFilePath;
    excludeFilePath = collection->folder + "/exclude.csv";

    QSqlQuery query(QSqlDatabase::database("defaultConnection"));
    QString querySQL;
    querySQL = QLatin1String(R"(
                        INSERT INTO parameter (
                                        parameter_name,
                                        parameter_type,
                                        parameter_value1,
                                        parameter_value2 )
                        VALUES(         :parameter_name,
                                        :parameter_type,
                                        :parameter_value1,
                                        :parameter_value2 )
                    )");
    query.prepare(querySQL);

    //Define exclude file and prepare stream
    QFile excludeFile(excludeFilePath);
    QTextStream textStream(&excludeFile);

    //Open file or return information
    if(excludeFile.open(QIODevice::ReadOnly)) {

        //Test file validity (application breaks between v0.13 and v0.14)
        QString line = textStream.readLine();

        //Load virtualStorage device lines to table
        while (true)
        {
            QString line = textStream.readLine();
            if (line.isNull())
                break;
            else{
                QSqlQuery insertQuery(QSqlDatabase::database("defaultConnection"));
                insertQuery.prepare(querySQL);
                insertQuery.bindValue(":parameter_name", "");
                insertQuery.bindValue(":parameter_type", "exclude_directory");
                insertQuery.bindValue(":parameter_value1", "All catalogs");
                insertQuery.bindValue(":parameter_value2", line);
                insertQuery.exec();
                qDebug()<<"DEBUG: importExcludeIntoParameter query: "<<insertQuery.lastError();
            }
        }
        excludeFile.close();
    }
    else
        qDebug()<<"failed to open exclude file: "<<excludeFilePath;

    //Update collection version and save
    collection->version = currentVersion;
    collection->updateCollectionVersion();
    collection->saveParameterTableToFile();
}

void MainWindow::convertTags()
{//Convert Tags
    QFile tagFile(collection->tagFilePath);

    //Open file or return information
    if(!tagFile.open(QIODevice::ReadOnly)) {
        return;
    }

    QTextStream textStream(&tagFile);
    while (true)
    {
        QString line = textStream.readLine();
        if (line.isNull())
            break;
        else
            {
                //Split the string with tabulation into a list
                QStringList fieldList = line.split('\t');
                //qDebug()<<"fieldList"<<fieldList;
                QSqlQuery insertQuery(QSqlDatabase::database("defaultConnection"));
                QString insertQuerySQL = QLatin1String(R"(
                                        INSERT INTO tag(
                                            ID,
                                            name,
                                            path,
                                            type,
                                            date_time)
                                        VALUES(
                                            NULL,
                                            :name,
                                            :path,
                                            :type,
                                            :date_time)
                                        )");
                insertQuery.prepare(insertQuerySQL);
                insertQuery.bindValue(":name",      fieldList[1]);
                insertQuery.bindValue(":path",      fieldList[0]);
                insertQuery.bindValue(":type",      "");
                insertQuery.bindValue(":date_time", "");
                insertQuery.exec();
            }
    }
    tagFile.close();

    collection->saveTagTableToFile();
}

void MainWindow::convertSearchHistory()
{//Convert SearchHistory
    QFile searchHistoryFile(collection->searchHistoryFilePath);

    //Open file or return information
    if(!searchHistoryFile.open(QIODevice::ReadOnly)) {
        return;
    }

    QTextStream textStream(&searchHistoryFile);
    while (true)
    {
        QString line = textStream.readLine();
        if (line.isNull())
            break;
        else
        {
            //Split the string with tabulation into a list
            QStringList fieldList = line.split('\t');
            //qDebug()<<"fieldList"<<fieldList;
            QSqlQuery insertQuery(QSqlDatabase::database("defaultConnection"));
            QString insertQuerySQL = QLatin1String(R"(
                                                INSERT INTO search(
                                                    date_time,
                                                    text_checked,
                                                    text_phrase,
                                                    text_criteria,
                                                    text_search_in,
                                                    file_type,
                                                    file_size_checked,
                                                    file_size_min,
                                                    file_size_min_unit,
                                                    file_size_max,
                                                    file_size_max_unit,
                                                    date_modified_checked,
                                                    date_modified_min,
                                                    date_modified_max,
                                                    duplicates_checked,
                                                    duplicates_name,
                                                    duplicates_size,
                                                    duplicates_date_modified,
                                                    differences_checked,
                                                    differences_name,
                                                    differences_size,
                                                    differences_date_modified,
                                                    differences_catalogs,
                                                    show_folders,
                                                    tag_checked,
                                                    tag,
                                                    search_location,
                                                    search_storage,
                                                    search_catalog,
                                                    search_catalog_checked,
                                                    search_directory_checked,
                                                    selected_directory,
                                                    text_exclude,
                                                    case_sensitive,
                                                    file_type_checked,
                                                    file_criteria_checked,
                                                    folder_criteria_checked
                                                    )
                                                VALUES(
                                                    :date_time,
                                                    :text_checked,
                                                    :text_phrase,
                                                    :text_criteria,
                                                    :text_search_in,
                                                    :file_type,
                                                    :file_size_checked,
                                                    :file_size_min,
                                                    :file_size_min_unit,
                                                    :file_size_max,
                                                    :file_size_max_unit,
                                                    :date_modified_checked,
                                                    :date_modified_min,
                                                    :date_modified_max,
                                                    :duplicates_checked,
                                                    :duplicates_name,
                                                    :duplicates_size,
                                                    :duplicates_date_modified,
                                                    :differences_checked,
                                                    :differences_name,
                                                    :differences_size,
                                                    :differences_date_modified,
                                                    :differences_catalogs,
                                                    :show_folders,
                                                    :tag_checked,
                                                    :tag,
                                                    :search_location,
                                                    :search_storage,
                                                    :search_catalog,
                                                    :search_catalog_checked,
                                                    :search_directory_checked,
                                                    :selected_directory,
                                                    :text_exclude,
                                                    :case_sensitive,
                                                    :file_type_checked,
                                                    :file_criteria_checked,
                                                    :folder_criteria_checked
                                                    )
                                                )");

            insertQuery.prepare(insertQuerySQL);
            insertQuery.bindValue(":date_time",                 fieldList[0]);
            insertQuery.bindValue(":text_checked",              fieldList[1]);
            insertQuery.bindValue(":text_phrase",               fieldList[2]);
            insertQuery.bindValue(":text_criteria",             fieldList[3]);
            insertQuery.bindValue(":text_search_in",            fieldList[4]);
            insertQuery.bindValue(":file_type",                 fieldList[5]);
            insertQuery.bindValue(":file_size_checked",         fieldList[6]);
            insertQuery.bindValue(":file_size_min",             fieldList[7]);
            insertQuery.bindValue(":file_size_min_unit",        fieldList[8]);
            insertQuery.bindValue(":file_size_max",             fieldList[9]);
            insertQuery.bindValue(":file_size_max_unit",        fieldList[10]);
            insertQuery.bindValue(":date_modified_checked",     fieldList[11]);
            insertQuery.bindValue(":date_modified_min",         fieldList[12]);
            insertQuery.bindValue(":date_modified_max",         fieldList[13]);
            insertQuery.bindValue(":duplicates_checked",        fieldList[14]);
            insertQuery.bindValue(":duplicates_name",           fieldList[15]);
            insertQuery.bindValue(":duplicates_size",           fieldList[16]);
            insertQuery.bindValue(":duplicates_date_modified",  fieldList[17]);
            insertQuery.bindValue(":show_folders",              fieldList[18]);
            insertQuery.bindValue(":tag_checked",               fieldList[19]);
            insertQuery.bindValue(":tag",                       fieldList[20]);
            insertQuery.bindValue(":search_location",           fieldList[21]);
            insertQuery.bindValue(":search_storage",            fieldList[22]);
            insertQuery.bindValue(":search_catalog",            fieldList[23]);
            insertQuery.bindValue(":search_catalog_checked",    fieldList[24]);
            insertQuery.bindValue(":search_directory_checked",  fieldList[25]);
            insertQuery.bindValue(":selected_directory",        fieldList[26]);
            insertQuery.bindValue(":text_exclude",              fieldList[27]);
            insertQuery.bindValue(":case_sensitive",            fieldList[28]);
            insertQuery.bindValue(":differences_checked",       fieldList[29]);
            insertQuery.bindValue(":differences_name",          fieldList[30]);
            insertQuery.bindValue(":differences_size",          fieldList[31]);
            insertQuery.bindValue(":differences_date_modified", fieldList[32]);
            insertQuery.bindValue(":differences_catalogs",      fieldList[33]);
            insertQuery.bindValue(":file_type_checked",         fieldList[34]);
            insertQuery.bindValue(":file_criteria_checked",     fieldList[35]);
            insertQuery.bindValue(":folder_criteria_checked",   fieldList[36]);
            insertQuery.exec();

        }
    }
    searchHistoryFile.close();

    collection->saveSearchHistoryTableToFile();
}

void MainWindow::convertStorage()
{//Convert Tags
    QSqlQuery query(QSqlDatabase::database("defaultConnection"));
    QString querySQL = QLatin1String(R"(
                                        DELETE FROM storage
                                    )");
    query.prepare(querySQL);
    query.exec();

    QFile storageFile(collection->storageFilePath);

    //Open file or return information
    if(!storageFile.open(QIODevice::ReadOnly)) {
        return;
    }

    QTextStream textStream(&storageFile);
    while (true)
    {
        QString line = textStream.readLine();
        if (line.isNull())
            break;
        else
        {
            //Split the string with tabulation into a list
            QStringList fieldList = line.split('\t');
            //qDebug()<<"fieldList"<<fieldList;
            QSqlQuery insertQuery(QSqlDatabase::database("defaultConnection"));
            QString insertQuerySQL = QLatin1String(R"(
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
                                                storage_brand,
                                                storage_model,
                                                storage_serial_number,
                                                storage_build_date,
                                                storage_comment1,
                                                storage_comment2,
                                                storage_comment3)
                                          VALUES(
                                                :storage_id,
                                                :storage_name,
                                                :storage_type,
                                                :storage_location,
                                                :storage_path,
                                                :storage_label,
                                                :storage_file_system,
                                                :storage_total_space,
                                                :storage_free_space,
                                                :storage_brand,
                                                :storage_model,
                                                :storage_serial_number,
                                                :storage_build_date,
                                                :storage_comment1,
                                                :storage_comment2,
                                                :storage_comment3)
                                    )");
            insertQuery.prepare(insertQuerySQL);
            insertQuery.bindValue(":storage_id",            fieldList[0]);
            insertQuery.bindValue(":storage_name",          fieldList[1]);
            insertQuery.bindValue(":storage_type",          fieldList[2]);
            insertQuery.bindValue(":storage_location",      fieldList[3]);
            insertQuery.bindValue(":storage_path",          fieldList[4]);
            insertQuery.bindValue(":storage_label",         fieldList[5]);
            insertQuery.bindValue(":storage_file_system",   fieldList[6]);
            insertQuery.bindValue(":storage_total_space",   fieldList[7]);
            insertQuery.bindValue(":storage_free_space",    fieldList[8]);
            insertQuery.bindValue(":storage_brand",         fieldList[9]);
            insertQuery.bindValue(":storage_model",         fieldList[9]);
            insertQuery.bindValue(":storage_serial_number", fieldList[10]);
            insertQuery.bindValue(":storage_build_date",    fieldList[11]);
            insertQuery.bindValue(":storage_comment1",      fieldList[12]);
            insertQuery.bindValue(":storage_comment2",      fieldList[13]);
            insertQuery.bindValue(":storage_comment3",      fieldList[14]);
            insertQuery.exec();
        }
    }
    storageFile.close();

    collection->saveStorageTableToFile();
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
