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
// File Name:   mainwindow_tab_device_pr.cpp
// Purpose:     https://stephanecouturier.github.io/Katalog/docs/Features/Devices
// Description:
// Author:      Stephane Couturier
/////////////////////////////////////////////////////////////////////////////
*/

#include "core/statusbarmessagebuilder.h"
#include "mainwindow.h"
#include "ui_mainwindow.h"
#include "devicetreeview.h"
#include "core/database.h"
#include "core/device.h"
#include "core/filechecksum.h"
#include "mainwindow_ui_wrapper_device.h"
#include "mainwindow_ui_wrapper_catalog.h"

#include <QSet>
#include <QMap>
#include <QFile>
#include <QMutex>

//--- Methods --------------------------------------------------------------
//--------------------------------------------------------------------------
QStandardItem* addNumericItem(qint64 value) {
    QStandardItem *item = new QStandardItem();
    item->setData(value, Qt::DisplayRole);
    return item;
}

void MainWindow::assignCatalogToDevice(Device *catalogDevice, Device *parentDevice)
{
    // Check if already assigned
    if (Device::isCatalogAssigned(catalogDevice->externalID, parentDevice->ID, m_connectionName)) {
        QMessageBox msgBox;
        msgBox.setWindowTitle("Katalog");
        msgBox.setText(QCoreApplication::translate("MainWindow",
                                                   "The catalog is already assigned to this Virtual device."
                                                   ));
        msgBox.setIcon(QMessageBox::Information);
        msgBox.exec();
        return;
    }

    // Assign via core
    if (Device::assignCatalogToDevice(catalogDevice, parentDevice, m_connectionName)) {
        //Save data to file
        if (collection->databaseMode == "Memory") {
            collection->saveDeviceTableToFile();
        }

        //Reload
        loadDevicesView("");
    }
}
//--------------------------------------------------------------------------
void MainWindow::assignStorageToDevice(int storageID,int deviceID)
{
    // Assign via core
    if (Device::assignStorageToDevice(selectedDevice->storage, deviceID, m_connectionName)) {
        //Save data to file
        if (collection->databaseMode == "Memory") {
            collection->saveDeviceTableToFile();
        }

        //Reload
        loadDevicesView("");
    }
}
//--------------------------------------------------------------------------
void MainWindow::unassignPhysicalFromDevice(int deviceID, int deviceParentID)
{
    int result = QMessageBox::warning(this,"Katalog",
                                      tr("Do you want to unassign this catalog from this virtual device?"),QMessageBox::Yes|QMessageBox::Cancel);

    if ( result ==QMessageBox::Yes){
        // Unassign via core
        if (Device::unassignFromDevice(deviceID, deviceParentID, m_connectionName)) {
            //Save data to file
            if (collection->databaseMode == "Memory") {
                collection->saveDeviceTableToFile();
            }

            //Reload
            loadDevicesView("");
        }
    }
}
//--------------------------------------------------------------------------
void MainWindow::deleteDeviceItem()
{
    bool success = DeviceUIWrapper::deleteDeviceWithUI(activeDevice, true);
    if (!success) {
        return; // User cancelled or error occurred
    }

    Device parentDevice;
    parentDevice.ID = activeDevice->parentID;
    parentDevice.loadDevice(m_connectionName);
    parentDevice.updateNumbersFromChildren();
    parentDevice.updateParentsNumbers();

    //Save data to files
    collection->saveDeviceTableToFile();
    collection->saveStorageTableToFile();

    //Delete the corresponding catalog file in memory mode
    if(activeDevice->type =="Catalog"){
        bool success = CatalogUIWrapper::deleteCatalogFileWithUI(collection, activeDevice);
        if (!success) {
            QMessageBox msgBox;
            msgBox.setWindowTitle("Katalog");
            msgBox.setText(tr("Deletion failed"));
            msgBox.setIcon(QMessageBox::Warning);
            msgBox.exec();
        }
    }

    //Reload data to models
    updateStorageSelectionStatistics();
    loadDevicesTreeToModel("Filters");
    loadDevicesView("");
    filterFromSelectedDevice();
}
//--------------------------------------------------------------------------
void MainWindow::splitCatalogBySubDirectory()
{
    // activeDevice has already been loaded in the context menu handler

    if (collection->databaseMode == "Memory")
        activeDevice->catalog->loadFoldersToTable();

    QStringList subDirs = activeDevice->catalog->listImmediateSubdirectories();
    if (subDirs.isEmpty()) {
        QMessageBox::information(this, "Katalog",
            tr("This catalog has no immediate sub-directories to split by."));
        return;
    }

    int confirmed = QMessageBox::warning(this, "Katalog",
        tr("Split catalog \"%1\"?\n\n"
           "This will create %2 new catalogs "
           "(one per sub-directory plus one for root files) "
           "and remove the original.\n"
           "This operation cannot be undone.")
            .arg(activeDevice->catalog->name)
            .arg(subDirs.count() + 1),
        QMessageBox::Yes | QMessageBox::Cancel);
    if (confirmed != QMessageBox::Yes)
        return;

    // Memory mode: load file data before splitting
    if (collection->databaseMode == "Memory") {
        QMutex mutex;
        bool stop = false;
        activeDevice->catalog->loadCatalogFileListToTable(mutex, stop);
    }

    QList<Catalog*> newCatalogs = activeDevice->catalog->executeSplitBySubDirectory(
        collection->databaseMode, collection->folder);

    if (newCatalogs.isEmpty()) {
        QMessageBox::warning(this, "Katalog", tr("Split failed: no catalogs were created."));
        return;
    }

    // Load parent device to inherit groupID for new device rows
    Device parentDevice;
    parentDevice.ID = activeDevice->parentID;
    parentDevice.loadDevice(m_connectionName);

    // Create a device row for each new catalog
    for (Catalog *c : std::as_const(newCatalogs)) {
        Device newDev;
        newDev.ID             = Device::generateNextDeviceID(m_connectionName);
        newDev.parentID       = parentDevice.ID;
        newDev.name           = c->name;
        newDev.type           = "Catalog";
        newDev.externalID     = c->ID;
        newDev.path           = c->sourcePath;
        newDev.totalFileSize  = c->totalFileSize;
        newDev.totalFileCount = c->fileCount;
        newDev.groupID        = parentDevice.groupID;
        newDev.active         = activeDevice->active;
        newDev.order          = 0;
        newDev.insertDevice();
    }

    // Save original .idx paths before deletion (Memory mode)
    QString origFilePath        = activeDevice->catalog->filePath;
    QString origFoldersFilePath = origFilePath;
    origFoldersFilePath.replace(origFoldersFilePath.lastIndexOf(".idx"), 4, ".folders.idx");

    // Delete the original device and catalog (files/folders already redistributed)
    activeDevice->deleteDevice(false);

    if (collection->databaseMode == "Memory") {
        collection->saveDeviceTableToFile();
        QFile::remove(origFilePath);
        QFile::remove(origFoldersFilePath);
    }

    qDeleteAll(newCatalogs);
    newCatalogs.clear();

    updateStorageSelectionStatistics();
    loadDevicesTreeToModel("Filters");
    loadDevicesView("");
    filterFromSelectedDevice();
}
//--------------------------------------------------------------------------
void MainWindow::recordDevicesSnapshot()
{
    //Get the current total values
    QSqlQuery queryLastCatalog(QSqlDatabase::database(m_connectionName));
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

    QSqlQuery queryLastStorage(QSqlDatabase::database(m_connectionName));
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
    QSqlQuery queryNew(QSqlDatabase::database(m_connectionName));
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

    QSqlQuery queryNewStorage(QSqlDatabase::database(m_connectionName));
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
    loadDevicesView("");
}
//--------------------------------------------------------------------------
void MainWindow::updateAllNumbers()
{
    activeDevice->updateParentsNumbers();

    collection->saveDeviceTableToFile();
    loadDevicesView("");
}
//--------------------------------------------------------------------------
void MainWindow::setDeviceTreeExpandState(bool toggle)
{
    deviceTreeExpandState = 0;
}
//--------------------------------------------------------------------------
void MainWindow::shiftIDsInDeviceTable(int shiftAmount)
{
    QSqlQuery query(QSqlDatabase::database(m_connectionName));

    // First, update the rows with parentID = 0 to keep them unchanged
    QString sql = "UPDATE device SET device_id = device_id + :shiftAmount "
                  "WHERE device_parent_id = 0";
    query.prepare(sql);
    query.bindValue(":shiftAmount", shiftAmount);
    if (!query.exec()) {
        qWarning() << "WARNING: shiftIDsInDeviceTable - Error updating device table:" << query.lastError().text();
        return;
    }

    // Next, update the rows with parentID != 0 to shift their IDs
    sql = "UPDATE device SET device_id = device_id + :shiftAmount, "
          "device_parent_id = device_parent_id + :shiftAmount "
          "WHERE device_parent_id != 0";
    query.prepare(sql);
    query.bindValue(":shiftAmount", shiftAmount);
    if (!query.exec()) {
        qWarning() << "WARNING: Error updating device table:" << query.lastError().text();
        return;
    }

}
//--------------------------------------------------------------------------
void MainWindow::loadParentsList()
{//Load valid list of parents to the TreeComboBox. It enables a selection to change the parent of a device.

    //A device can only be moved within its group (0= Physical, 1= Virtual)
    QSqlQuery query(QSqlDatabase::database(m_connectionName));
    QString querySQL = QLatin1String(R"(
        SELECT device_id, device_parent_id, device_name, device_type, device_active
        FROM device
        WHERE device_id != 0
        AND device_id != :selected_device_id
        AND device_group_id = :device_group_id
    )");

    if (activeDevice->type == "Catalog") {
        querySQL += QLatin1String(R"( AND device_type NOT IN ("Catalog"))");
    } else {
        querySQL += QLatin1String(R"( AND device_type NOT IN ("Catalog","Storage"))");
    }

    query.prepare(querySQL);
    query.bindValue(":selected_device_id", activeDevice->ID);
    query.bindValue(":device_group_id", activeDevice->groupID);
    query.exec();

    // Build hierarchical tree model — same column layout as buildFilteredDeviceTreeModel():
    // col 0=Name, col 1=Type, col 2=Active, col 3=ID (read by selectedDeviceId()), col 4=ParentID
    QStandardItemModel *treeModel = new QStandardItemModel(this);
    QMap<int, QStandardItem*> itemMap;

    while (query.next()) {
        int id       = query.value(0).toInt();
        int parentId = query.value(1).toInt();
        QList<QStandardItem*> row = {
            new QStandardItem(query.value(2).toString()),
            new QStandardItem(query.value(3).toString()),
            new QStandardItem(query.value(4).toString()),
            new QStandardItem(QString::number(id)),
            new QStandardItem(QString::number(parentId))
        };
        QStandardItem *parentItem = itemMap.value(parentId, nullptr);
        if (!parentItem)
            treeModel->appendRow(row);
        else
            parentItem->appendRow(row);
        itemMap.insert(id, row[0]);
    }

    DeviceTreeView *proxy = new DeviceTreeView(this);
    proxy->setSourceModel(treeModel);
    proxy->setKatalogTheme(themeID > 0);

    ui->Devices_comboBox_Parent->setTreeModel(proxy);
    ui->Devices_comboBox_Parent->expandToDepth(2);
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
    loadDevicesView("");
    filterFromSelectedDevice();
    loadParentsList();

    //Make it the activeDevice and edit
    activeDevice->ID = newDevice->ID;
    activeDevice->loadDevice(m_connectionName);
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
    newDevice->storage->name = newDevice->name;
    newDevice->storage->insertStorage();

    //Save data to file
    collection->saveDeviceTableToFile();

    //Reload
    loadDevicesTreeToModel("Filters");
    loadDevicesView("");
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
    activeDevice->loadDevice(m_connectionName);
    loadDevicesTreeToModel("Filters");
    loadDevicesView("");
    editDevice();
}
//--------------------------------------------------------------------------
void MainWindow::editDevice()
{   //Display a panel to edit the device values

    //Refresh parent combobox
    loadParentsList();

    //Load panel and values
    ui->Storage_label_Picture_2->setVisible(true);

    ui->Devices_widget_Edit->setVisible(true);
    ui->Devices_lineEdit_Name->setText(activeDevice->name);
    ui->Devices_label_ItemDeviceTypeValue->setText(activeDevice->type);
    ui->Devices_label_ItemDeviceIDValue->setText(QString::number(activeDevice->ID));

    if(activeDevice->type =="Catalog"){
        ui->Devices_widget_EditCatalogFields->show();
        ui->Devices_widget_EditStorageFields->hide();
        // Find and set the file type using ItemData (multilingual-safe)
        int fileTypeIndex = ui->Catalogs_comboBox_FileType->findData(
            activeDevice->catalog->fileType, Qt::UserRole);
        if (fileTypeIndex != -1) {
            ui->Catalogs_comboBox_FileType->setCurrentIndex(fileTypeIndex);
        } else {
            // Default to "All" if not found
            ui->Catalogs_comboBox_FileType->setCurrentIndex(0);
        }
        // Find and set the includeHidden value using ItemData (multilingual-safe)
        int includeHiddenIndex = ui->Catalogs_comboBox_IncludeHidden->findData(
            activeDevice->catalog->includeHidden, Qt::UserRole);
        if (includeHiddenIndex != -1) {
            ui->Catalogs_comboBox_IncludeHidden->setCurrentIndex(includeHiddenIndex);
        } else {
            // Default to "None" (index 0) if not found
            ui->Catalogs_comboBox_IncludeHidden->setCurrentIndex(0);
        }
        for (int i = 0; i < ui->Catalogs_comboBox_MetaDataOption->count(); ++i) {
            if (ui->Catalogs_comboBox_MetaDataOption->itemData(i, Qt::UserRole).toString() == activeDevice->catalog->includeMetadata) {
                ui->Catalogs_comboBox_MetaDataOption->setCurrentIndex(i);
                break;
            }
        }
        for (int i = 0; i < ui->Catalogs_comboBox_ChecksumOption->count(); ++i) {
            if (ui->Catalogs_comboBox_ChecksumOption->itemData(i, Qt::UserRole).toString() == activeDevice->catalog->includeChecksum) {
                ui->Catalogs_comboBox_ChecksumOption->setCurrentIndex(i);
                break;
            }
        }
        //DEV: ui->Catalogs_checkBox_isFullDevice->setChecked(selectedCatalogIsFullDevice);

        // Per-catalog exclude folders
        {
            const QStringList folders = activeDevice->catalog->getExcludeFolders();
            QStringListModel *model = new QStringListModel(folders, this);
            ui->Devices_listView_ExcludeFolders->setModel(model);
        }
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

        loadStoragePictureComboBox();
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

    //Pre-select the current parent in the tree
    ui->Devices_comboBox_Parent->setSelectedDeviceId(activeDevice->parentID);
}
//--------------------------------------------------------------------------
void MainWindow::saveDeviceForm()
{//Save the device values from the edit panel

    //Keep previous values
    activeDevice->loadDevice(m_connectionName);
    int previousExternalID = activeDevice->externalID;
    QString previousName = activeDevice->name;
    QString previousPath = activeDevice->path;
    Device previousParentDevice;
    previousParentDevice.ID = activeDevice->parentID;
    previousParentDevice.loadDevice(m_connectionName);

    //Get new values: name, parentID, externalID
    int selectedParentId = ui->Devices_comboBox_Parent->selectedDeviceId();
    activeDevice->parentID = (selectedParentId > 0) ? selectedParentId : 0;
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
    newParentDevice.loadDevice(m_connectionName);

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
            loopDevice.loadDevice(m_connectionName);
            loopDevice.groupID = newGroupID;
            loopDevice.saveDevice();
        }
    }

    //Save device to database
    activeDevice->groupID = newGroupID;
    activeDevice->totalSpace = ui->Storage_lineEdit_Panel_Total->text().toLongLong();
    activeDevice->freeSpace  = ui->Storage_lineEdit_Panel_Free->text().toLongLong();
    activeDevice->saveDevice();
    //Update device if path was changed (for non-Catalog types; Catalogs are handled in saveCatalogChanges)
    if (activeDevice->type == "Storage" && !previousPath.isEmpty() && activeDevice->path != previousPath){
        // Ask user how to handle the path change in child catalog indexes
        QMessageBox msgBox;
        msgBox.setWindowTitle("Katalog");
        msgBox.setText(tr("The storage path changed.")
                       + "<br/><br/><table>"
                       + "<tr><td>" + tr("Old path:") + "</td><td><b>" + previousPath          + "</b></td></tr>"
                       + "<tr><td>" + tr("New path:") + "</td><td><b>" + activeDevice->path    + "</b></td></tr>"
                       + "</table><br/>"
                       + tr("How should the catalog indexes be updated?"));
        msgBox.setIcon(QMessageBox::Question);
        QPushButton *btnReplace  = msgBox.addButton(tr("Replace path root"), QMessageBox::AcceptRole);
        QPushButton *btnRescan   = msgBox.addButton(tr("Full re-scan"),        QMessageBox::AcceptRole);
        /*QPushButton *btnSkip =*/ msgBox.addButton(tr("Skip"),                QMessageBox::RejectRole);
        msgBox.exec();

        if (msgBox.clickedButton() == btnReplace) {
            setCatalogUpdateUIState(true);
            deviceUpdateManager->replaceStorageRoot(activeDevice,
                                                    previousPath,
                                                    activeDevice->path,
                                                    collection->databaseMode,
                                                    collection->folder);
        } else if (msgBox.clickedButton() == btnRescan) {
            setCatalogUpdateUIState(true);
            deviceUpdateManager->updateDeviceHierarchy(activeDevice,
                                                       collection->databaseMode,
                                                       collection->folder,
                                                       "update");
        }
        // else Skip — do nothing further
    } else if (activeDevice->type != "Catalog" && activeDevice->path != previousPath){
        // Other non-Catalog device types (Virtual, Group…)
        setCatalogUpdateUIState(true);
        deviceUpdateManager->updateDeviceHierarchy(activeDevice,
                                                   collection->databaseMode,
                                                   collection->folder,
                                                   "update");
    }
    collection->saveDeviceTableToFile();

    //If device is a catalog, save catalog changes
    if(activeDevice->type == "Catalog"){
        activeDevice->catalog->storageName = newParentDevice.name;
        saveCatalogChanges(previousPath);
        updateCatalogsScreenStatistics();
        loadDevicesTreeToModel("Filters");
    }

    //If device is Storage, rename in storage table and update device values
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

        QSqlQuery updateQuery(QSqlDatabase::database(m_connectionName));
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

            QSqlQuery updateNameQuery(QSqlDatabase::database(m_connectionName));
            updateNameQuery.prepare(updateNameQuerySQL);
            updateNameQuery.bindValue(":new_storage_name", newStorageName);
            updateNameQuery.bindValue(":storage_id", selectedDevice->storage->ID);
            updateNameQuery.exec();

            if (collection->databaseMode=="Memory"){
                collection->saveStatiticsTableToFile();
            }

            //Update catalogs (database mode)
            QString updateCatalogQuerySQL = QLatin1String(R"(
                                    UPDATE catalog
                                    SET catalog_storage = :new_storage_name
                                    WHERE catalog_storage =:current_storage_name
                                )");

            QSqlQuery updateCatalogQuery(QSqlDatabase::database(m_connectionName));
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

                QSqlQuery listCatalogQuery(QSqlDatabase::database(m_connectionName));
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
        QSqlQuery queryStorage(QSqlDatabase::database(m_connectionName));
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
                                        storage_comment3 =:storage_comment3,
                                        storage_picture_path =:storage_picture_path
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
        queryStorage.bindValue(":storage_picture_path",  ui->Storage_comboBox_PicturePath->currentText());
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
    loadDevicesView("");
    loadStorageList();
}
//--------------------------------------------------------------------------
void MainWindow::recordAllDeviceStats(QDateTime dateTime)
{// Save the values (free space and total space) of all storage devices, completing a snapshop of the collection.

    //Get the list of storage devices
    QSqlQuery query(QSqlDatabase::database(m_connectionName));
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
        loopDevice.loadDevice(m_connectionName);
        loopDevice.saveStatistics(dateTime,"snapshot");
    }
    collection->saveStatiticsTableToFile();

    //Refresh
    collection->loadStatisticsDeviceFileToTable();
    loadStatisticsChart();
}

//--------------------------------------------------------------------------
//--- View -----------------------------------------------------------------
void MainWindow::loadDevicesView(QString sourceTrigger){
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
        if(sourceTrigger !="Filters"){
            loadDevicesTreeToModel("Devices");
        }
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
    collection->updateAllDeviceActive();

    //Retrieve device hierarchy
    QSqlQuery query(QSqlDatabase::database(m_connectionName));
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
                  AND device_parent_id = 0

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
    QStandardItemModel *devicesTreeModel = new QStandardItemModel(this);

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
        rowItems << addNumericItem(id);                             //3
        rowItems << addNumericItem(parentId);                       //4
        rowItems << addNumericItem(externalId);                     //5
        rowItems << addNumericItem(number);                         //6
        rowItems << addNumericItem(size);                           //7
        rowItems << addNumericItem(used_space);                     //8
        rowItems << addNumericItem(free_space);                     //9
        rowItems << addNumericItem(total_space);                    //10
        rowItems << new QStandardItem(dateTimeUpdated);             //11
        rowItems << new QStandardItem(path);                        //12
        rowItems << addNumericItem(groupID);                        //13

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
        deviceTreeViewForDeviceTab->setKatalogTheme(themeID > 0);
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
        QStandardItemModel *filtersTreeModel = new QStandardItemModel(this);
        filtersTreeModel = devicesTreeModel;
        //Load Model to treeview (Filters/Device tree)
        DeviceTreeView *deviceTreeViewForSelectionPanel = new DeviceTreeView(this);
        deviceTreeViewForSelectionPanel->setKatalogTheme(themeID > 0);
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
        //ui->Filters_treeView_Devices->setModel(deviceTreeViewForSelectionPanel);
        if (filtersTreeExpandState == -1) {
            ui->Filters_treeView_Devices->collapseAll();
        } else if (filtersTreeExpandState >= 0) {
            ui->Filters_treeView_Devices->expandToDepth(filtersTreeExpandState);
        }

        //ui->Filters_treeView_Devices->expandAll();
    }

    // At the end, refresh both tree states to update max levels
    changeTreeExpandLevel(0, ui->Filters_treeView_Devices, filtersTreeExpandState,
                          "Selection/filtersTreeExpandState",
                          ui->Filters_pushButton_TreeCollapse, ui->Filters_pushButton_TreeExpand,
                          true); // force refresh

    changeTreeExpandLevel(0, ui->Devices_treeView_DeviceList, deviceTreeExpandState,
                          "Devices/deviceTreeExpandState",
                          ui->Devices_pushButton_TreeCollapse, ui->Devices_pushButton_TreeExpand,
                          true); // force refresh
}
//--------------------------------------------------------------------------
void MainWindow::loadDevicesStorageToModel(){
    //Refresh active state
    collection->updateAllDeviceActive();

    //Retrieve device hierarchy
    QSqlQuery loadStorageQuery(QSqlDatabase::database(m_connectionName));
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
    QStandardItemModel *storageTreeModel = new QStandardItemModel(this);
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
        rowItems << addNumericItem(id);                             //3
        rowItems << addNumericItem(parentId);                       //4
        rowItems << addNumericItem(externalId);                     //5
        rowItems << addNumericItem(number);                         //6
        rowItems << addNumericItem(size);                           //7
        rowItems << addNumericItem(used_space);                     //8
        rowItems << addNumericItem(free_space);                     //9
        rowItems << addNumericItem(total_space);                    //10
        rowItems << new QStandardItem(dateTimeUpdated);             //11
        rowItems << new QStandardItem(path);                        //12
        rowItems << addNumericItem(groupID);                        //13
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
                continue;
            }
        }

        // Store the item in the map for future reference
        itemMap.insert(id, item);
    }

    //Load Model to treeview (Virtual tab)
    DeviceTreeView *deviceTreeViewForDeviceTab = new DeviceTreeView(this);
    deviceTreeViewForDeviceTab->setSourceModel(storageTreeModel);
    deviceTreeViewForDeviceTab->setKatalogTheme(themeID > 0);
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
    //Permanent fields
    ui->Devices_treeView_DeviceList->header()->hideSection( 1); //Type
    ui->Devices_treeView_DeviceList->header()->hideSection( 3); //ID
    ui->Devices_treeView_DeviceList->header()->hideSection( 4); //Parent ID
    ui->Devices_treeView_DeviceList->header()->showSection( 5); //External ID
    ui->Devices_treeView_DeviceList->header()->showSection( 8); //Used space
    ui->Devices_treeView_DeviceList->header()->showSection( 9); //Free space
    ui->Devices_treeView_DeviceList->header()->showSection(10); //Total space
    ui->Devices_treeView_DeviceList->header()->hideSection(13); //Group ID

    //Optional fields
    if (ui->Devices_checkBox_DisplayFullTable->isChecked()) {
        ui->Devices_treeView_DeviceList->header()->showSection( 2); //Active
        ui->Devices_treeView_DeviceList->header()->showSection(14); //storage_type
        ui->Devices_treeView_DeviceList->header()->showSection(17); //storage_brand
        ui->Devices_treeView_DeviceList->header()->showSection(18); //storage_model
        ui->Devices_treeView_DeviceList->header()->showSection(19); //storage_serial_number
        ui->Devices_treeView_DeviceList->header()->showSection(20); //storage_build_date
        ui->Devices_treeView_DeviceList->header()->showSection(21); //Comment 1
        ui->Devices_treeView_DeviceList->header()->showSection(22); //Comment 2
        ui->Devices_treeView_DeviceList->header()->showSection(23); //Comment 3
    } else {
        ui->Devices_treeView_DeviceList->header()->hideSection(2); //Active
        ui->Devices_treeView_DeviceList->header()->hideSection(14); //storage_type
        ui->Devices_treeView_DeviceList->header()->hideSection(17); //storage_brand
        ui->Devices_treeView_DeviceList->header()->hideSection(18); //storage_model
        ui->Devices_treeView_DeviceList->header()->hideSection(19); //storage_serial_number
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
    collection->updateAllDeviceActive();

    //Retrieve device hierarchy
    QSqlQuery loadCatalogQuery(QSqlDatabase::database(m_connectionName));
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
                            c.catalog_include_checksum     ,
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
    QStandardItemModel *catalogTreeModel = new QStandardItemModel(this);
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
                                                 tr("Hidden"),           //25
                                                 tr("Metadata"),         //26
                                                 tr("Checksum"),         //27
                                                 tr("Parent storage"),   //28
                                                 tr("Fulldevice"),       //29
                                                 tr("Date Loaded"),      //30
                                                 tr("App Version"),      //31
                                                 tr("File Path"),        //32
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
        QString catalog_include_checksum = loadCatalogQuery.value(17).toString();
        QString parent_storage = loadCatalogQuery.value(18).toString();
        QString catalog_is_full_device = loadCatalogQuery.value(19).toString();
        QString catalog_date_loaded = loadCatalogQuery.value(20).toString();
        QString catalog_app_version = loadCatalogQuery.value(21).toString();

        //Create the item for this row
        //Device fields
        QList<QStandardItem*> rowItems;
        rowItems << new QStandardItem(name);                        //0
        rowItems << new QStandardItem(type);                        //1
        rowItems << new QStandardItem(QString::number(isActive));   //2
        rowItems << addNumericItem(id);                             //3
        rowItems << addNumericItem(parentId);                       //4
        rowItems << addNumericItem(externalId);                     //5
        rowItems << addNumericItem(number);                         //6
        rowItems << addNumericItem(size);                           //7
        rowItems << addNumericItem(used_space);                     //8
        rowItems << addNumericItem(free_space);                     //9
        rowItems << addNumericItem(total_space);                    //10
        rowItems << new QStandardItem(dateTimeUpdated);             //11
        rowItems << new QStandardItem(path);                        //12
        rowItems << addNumericItem(groupID);                        //13

        //Storage fields: add empty rows
        for (int var = 0; var < 10; ++var) {
            rowItems << new QStandardItem("");                      //14 to //23
        }

        //Catalog fields
        rowItems << new QStandardItem(catalog_file_type);           //24
        rowItems << new QStandardItem(catalog_include_hidden);      //25
        rowItems << new QStandardItem(catalog_include_metadata);    //26
        rowItems << new QStandardItem(catalog_include_checksum);    //27
        rowItems << new QStandardItem(parent_storage);              //28
        rowItems << new QStandardItem(catalog_is_full_device);      //29
        rowItems << new QStandardItem(catalog_date_loaded);         //30
        rowItems << new QStandardItem(catalog_app_version);         //31
        rowItems << new QStandardItem(catalog_file_path);           //32

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
                continue;
            }
        }

        // Store the item in the map for future reference
        itemMap.insert(id, item);
    }

    //Load Model to treeview (Virtual tab)
    DeviceTreeView *deviceTreeViewForDeviceTab = new DeviceTreeView(this);
    deviceTreeViewForDeviceTab->setSourceModel(catalogTreeModel);
    deviceTreeViewForDeviceTab->setKatalogTheme(themeID > 0);
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
    ui->Devices_treeView_DeviceList->header()->resizeSection(25,  75); //Include Hidden
    ui->Devices_treeView_DeviceList->header()->resizeSection(26, 100); //Include metadata
    ui->Devices_treeView_DeviceList->header()->resizeSection(27, 100); //Include checksum
    ui->Devices_treeView_DeviceList->header()->setSectionResizeMode(28, QHeaderView::ResizeToContents); //Parent storage
    ui->Devices_treeView_DeviceList->header()->resizeSection(29,  50); //Is full device
    ui->Devices_treeView_DeviceList->header()->resizeSection(30, 150); //Date Loaded
    ui->Devices_treeView_DeviceList->header()->resizeSection(31,  50); //App Version
    ui->Devices_treeView_DeviceList->header()->setSectionResizeMode(32, QHeaderView::ResizeToContents); //File path

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

    //Hide development fields
    ui->Devices_treeView_DeviceList->header()->hideSection(29); //catalog_is_full_device

    // Apply DisplayFullTable choice
    if (ui->Devices_checkBox_DisplayFullTable->isChecked()) {
        ui->Devices_treeView_DeviceList->header()->showSection(2); //Active
        ui->Devices_treeView_DeviceList->header()->showSection(5); //External ID
        ui->Devices_treeView_DeviceList->header()->showSection(30); //Date loaded
        ui->Devices_treeView_DeviceList->header()->showSection(31); //app version
        ui->Devices_treeView_DeviceList->header()->showSection(32); //File path

    } else {
        ui->Devices_treeView_DeviceList->header()->hideSection(2); //Active
        ui->Devices_treeView_DeviceList->header()->hideSection(5); //External ID
        ui->Devices_treeView_DeviceList->header()->hideSection(30); //Date loaded
        ui->Devices_treeView_DeviceList->header()->hideSection(31); //app version
        ui->Devices_treeView_DeviceList->header()->hideSection(32); //File path
    }

    if (collection->databaseMode !="Memory") { //Fields that are only relevant in Memory mode
        ui->Devices_treeView_DeviceList->header()->hideSection(30); //Date loaded
        ui->Devices_treeView_DeviceList->header()->hideSection(32); //File path
    }

    ui->Devices_treeView_DeviceList->expandAll();
}
//--------------------------------------------------------------------------

//--------------------------------------------------------------------------
//--- Device ---------------------------------------------------------------
void MainWindow::setupDeviceUpdateManager()
{

    if (!deviceUpdateManager) {
        deviceUpdateManager = new DeviceUpdateManager(this);
    }

    // DISCONNECT any existing connections first
    disconnect(deviceUpdateManager, nullptr, this, nullptr);

    // Connect operation lifecycle signals
    connect(deviceUpdateManager, &DeviceUpdateManager::operationStarted,
            this, &MainWindow::onDeviceUpdateStarted);
    connect(deviceUpdateManager, &DeviceUpdateManager::operationCompleted,
            this, &MainWindow::onDeviceUpdateCompleted);
    connect(deviceUpdateManager, &DeviceUpdateManager::operationError,
            this, &MainWindow::onDeviceUpdateError);
    connect(deviceUpdateManager, &DeviceUpdateManager::operationCancelled,
            this, &MainWindow::onDeviceUpdateCancelled);

    // Connect CatalogProgressManager to DeviceUpdateManager's CatalogManager
    if (catalogProgressManager && deviceUpdateManager) {
        deviceUpdateManager->setCatalogProgressManager(catalogProgressManager);
    }

    // Connect individual catalog batch reports
    connect(deviceUpdateManager, &DeviceUpdateManager::catalogCompletedInBatch,
            this, [this](Device* catalogDevice, const QList<qint64>& results) {

        // Check if user wants individual reports (from UpdateAllActive dialog)
        if (showEachCatalogUpdateSummary) {
            reportAllUpdates(catalogDevice, results, "update");
        } else {
        }
    });

    // CatalogProgressManager to DeviceUpdateManager's internal CatalogManager
    if (catalogProgressManager) {
        deviceUpdateManager->setCatalogProgressManager(catalogProgressManager);
    }

}
//--------------------------------------------------------------------------
void MainWindow::onDeviceUpdateCompleted(const QList<qint64>& results)
{

    // Determine report device and correct updateType for reportAllUpdates
    Device* reportDevice = deviceUpdateManager->m_rootDevice;
    bool isCatalogCreation = (deviceUpdateManager->m_updateType == "create");
    bool isReplaceRoot     = (deviceUpdateManager->m_updateType == "replaceRoot");

    // Save collection data
    collection->saveDeviceTableToFile();
    collection->saveStatiticsTableToFile();

    // STEP 1: Call reportAllUpdates with correct parameters
    if (reportDevice) {
        reportAllUpdates(reportDevice, results, deviceUpdateManager->m_updateType);
    } else {
    }

    // replaceRoot: minimal UI refresh then done
    if (isReplaceRoot) {
        setCatalogUpdateUIState(false);
        loadDevicesView("");
        loadDevicesTreeToModel("Filters");
        if (selectedDevice) {
            selectedDevice->loadDevice(m_connectionName);
            updateCatalogsScreenStatistics();
        }
        currentUpdateDevice = nullptr;
        return;
    }

    // STEP 2: UI restoration (existing logic continues...)
    if (isCatalogCreation) {
        if (reportDevice) {
            // Complete UI refresh (like original working code)
            refreshDuplicatesDeviceSelection();
            refreshDifferencesDeviceSelection();
            collection->updateAllDeviceActive();
            loadDevicesView("");
            ui->Filters_label_DisplayCatalog->setText(reportDevice->name);
            selectedDevice->ID = reportDevice->ID;
            selectedDevice->loadDevice(m_connectionName);
            collection->loadDeviceFileToTable();
            loadDevicesTreeToModel("Filters");  // Key fix for Filters treeview
            loadDevicesView("");
            ui->tabWidget->setCurrentIndex(1); // Collection tab
            ui->Catalogs_pushButton_UpdateActiveDevice->setEnabled(false);
        }

        loadStorageList();

        // Report indexing duration
        {
            const QDateTime endTime = QDateTime::currentDateTime();
            const qint64 elapsedMs = m_catalogCreateTimer.elapsed();
            const int totalSec = static_cast<int>(elapsedMs / 1000);
            const QString duration = QString("%1:%2:%3")
                .arg(totalSec / 3600,         2, 10, QLatin1Char('0'))
                .arg((totalSec % 3600) / 60,  2, 10, QLatin1Char('0'))
                .arg(totalSec % 60,           2, 10, QLatin1Char('0'));
            const QString timingMsg = tr("Indexing - Start: %1 | End: %2 | Duration: %3")
                .arg(m_catalogCreateStartTime.toString("hh:mm:ss"))
                .arg(endTime.toString("hh:mm:ss"))
                .arg(duration);
            statusBarLabel->setText(timingMsg);
        }

        // SURGICAL FIX: Clean Create tab restoration (no mixed contexts)
        setCreateCatalogUIState(false);  // Use dedicated Create method
    } else {
        setCatalogUpdateUIState(false);

        // Full UI refresh for updates to show updated dates and values
        loadDevicesView("");                    // Refresh main device tree
        loadDevicesTreeToModel("Filters");     // Refresh filters tree view

        // Reload device statistics for updates
        if (selectedDevice) {
            selectedDevice->loadDevice(m_connectionName);
            updateCatalogsScreenStatistics();
        }
    }

    // Clear references Last
    if (isCatalogCreation) {
        currentUpdateDevice = nullptr;
    } else {
        // Clear for regular updates
        currentUpdateDevice = nullptr;
    }

}
//--------------------------------------------------------------------------
QList<Device*> MainWindow::collectActiveCatalogs()
{
    QList<Device*> activeCatalogs;

    QSqlQuery query(QSqlDatabase::database(m_connectionName));
    QString querySQL = QLatin1String(R"(
        SELECT device_id
        FROM device
        WHERE device_type = 'Catalog' AND device_active = 1
    )");

    query.exec(querySQL);
    while (query.next()) {
        Device* catalog = new Device();
        catalog->ID = query.value(0).toInt();
        catalog->loadDevice(m_connectionName);
        activeCatalogs.append(catalog);
    }

    return activeCatalogs;
}
//--------------------------------------------------------------------------
void MainWindow::setCatalogUpdateUIState(bool isRunning)
{

    if (isRunning) {
        // Disable update buttons during operation
        ui->Catalogs_pushButton_UpdateActiveDevice->setEnabled(false);
        ui->Catalogs_pushButton_UpdateAllActive->setEnabled(false);

        // Enable stop button
        ui->Catalogs_pushButton_Stop->setEnabled(true);

        // Set cursor
        QApplication::setOverrideCursor(Qt::WaitCursor);

    } else {
        // Re-enable update buttons
        bool deviceActivated = (activeDevice && (activeDevice->type == "Catalog" || activeDevice->type == "Storage"));
        ui->Catalogs_pushButton_UpdateActiveDevice->setEnabled(deviceActivated);
        QSettings settings(collection->settingsFilePath, QSettings::IniFormat);
        QString displayContents = settings.value("Devices/DisplayContents", "Tree").toString();
        bool isCatalogListView = (displayContents == "Catalogs");
        ui->Catalogs_pushButton_UpdateAllActive->setEnabled(isCatalogListView);

        // Disable stop button
        ui->Catalogs_pushButton_Stop->setEnabled(false);

        // Restore cursor
        QApplication::restoreOverrideCursor();

        // Clear status
        //statusBarLabel->clear();
    }
}
//--------------------------------------------------------------------------
void MainWindow::onDeviceUpdateStarted()
{

    // UI is already set by button handler, just log
}
//--------------------------------------------------------------------------
void MainWindow::onDeviceUpdateError(const QString& error)
{

    if (deviceUpdateManager->m_updateType == "create") {

        // SURGICAL FIX: Call the existing cleanup method for creation errors too
        cleanupStoppedCatalogCreation();

    } else {
        setCatalogUpdateUIState(false);  // Use Catalog context restoration
    }

    // Show error message
    //QMessageBox::warning(this, "Katalog", tr("Catalog operation failed:\n%1").arg(error));
    statusBarLabel->setText(tr("Operation cancelled"));
}
//--------------------------------------------------------------------------
void MainWindow::onDeviceUpdateCancelled()
{

    // PREVENT DOUBLE CALL
    static bool alreadyHandling = false;
    if (alreadyHandling) {
        return;
    }
    alreadyHandling = true;

    if (deviceUpdateManager->m_updateType == "create") {

        // Call the existing cleanup method for creation cancellation
        cleanupStoppedCatalogCreation();

        //statusBarLabel->setText(tr("Operation cancelled"));
    } else {
        setCatalogUpdateUIState(false);  // Use Catalog context restoration
        //statusBarLabel->setText(tr("Operation cancelled"));
    }


    // Reset flag at end
    alreadyHandling = false;
}
//--------------------------------------------------------------------------
void MainWindow::onDeviceUpdateProgress()
{
    if (!deviceUpdateManager) return;

    // Update progress display
    QString status = deviceUpdateManager->status();
    QString currentDevice = deviceUpdateManager->currentDeviceName();

    // Update status bar with current progress
    StatusBarMessageBuilder builder;
    builder.setOperation(tr("Update"))
        .setStatus(tr("In Progress"));
    if (!currentDevice.isEmpty()) {
        builder.setDeviceContext(1, 1, currentDevice);
    }
    // Add process info from deviceUpdateManager if available
    statusBarLabel->setText(builder.build());

    // Log progress for debugging
}
//--------------------------------------------------------------------------

//--------------------------------------------------------------------------
//--- Storage --------------------------------------------------------------
void MainWindow::loadStorageList()
{//Load Storage selection to comboBoxes

    //Get data
    QSqlQuery query(QSqlDatabase::database(m_connectionName));
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
void MainWindow::loadStoragePictureComboBox()
{// Populate the picture combobox with image files found in the images folder
    const QString currentPicture = activeDevice->storage->picturePath;

    // Block signals while rebuilding to avoid triggering currentIndexChanged
    ui->Storage_comboBox_PicturePath->blockSignals(true);
    ui->Storage_comboBox_PicturePath->clear();
    ui->Storage_comboBox_PicturePath->addItem("");  // empty = no picture

    QDir dir(collection->imageFolderPath);
    if (dir.exists()) {
        const QStringList filters = {"*.png","*.jpg","*.jpeg","*.bmp","*.gif","*.webp"};
        const QStringList files = dir.entryList(filters, QDir::Files, QDir::Name);
        for (const QString &f : files)
            ui->Storage_comboBox_PicturePath->addItem(f);
    }

    // Restore current selection
    int idx = ui->Storage_comboBox_PicturePath->findText(currentPicture);
    ui->Storage_comboBox_PicturePath->setCurrentIndex(idx >= 0 ? idx : 0);
    ui->Storage_comboBox_PicturePath->blockSignals(false);

    displayStoragePicture();
}
//--------------------------------------------------------------------------
void MainWindow::displayStoragePicture()
{// Load and display the picture of the storage device
    QString pictureFileName = ui->Storage_comboBox_PicturePath->currentText();
    QString picturePath;
    if (pictureFileName.isEmpty())
        picturePath = collection->imageFolderPath + "/" + QString::number(activeDevice->storage->ID) + ".jpg";
    else
        picturePath = collection->imageFolderPath + "/" + pictureFileName;

    QFile file(picturePath);
    if (file.exists()) {
        QPixmap pic(picturePath);
        ui->Storage_label_Picture_2->setPixmap(pic.scaled(350, 300, Qt::KeepAspectRatio, Qt::SmoothTransformation));
    } else {
        ui->Storage_label_Picture_2->setPixmap(QPixmap());
    }
}
//--------------------------------------------------------------------------
void MainWindow::on_Storage_comboBox_PicturePath_currentIndexChanged(int /*index*/)
{
    displayStoragePicture();
}
//--------------------------------------------------------------------------
void MainWindow::on_Storage_pushButton_ReloadPictures_clicked()
{
    loadStoragePictureComboBox();
}
//--------------------------------------------------------------------------
void MainWindow::updateStorageSelectionStatistics()
{
    //Get storage statistics
    QSqlQuery query(QSqlDatabase::database(m_connectionName));

    //Prepare the main part of the query
    QString querySQL = QLatin1String(R"(
                                        SELECT
                                            COUNT(device_id),
                                            SUM(device_free_space),
                                            SUM(device_total_space)
                                        FROM device
                                        WHERE device_type = 'Storage'
                            )");
    if (    selectedDevice->type  == "All" )
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
        ui->Storage_label_PercentFreeValue->setText(QString::number(round(freepercent))+"%");}
    else ui->Storage_label_PercentFreeValue->setText("");
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
void MainWindow::saveCatalogChanges(const QString &previousPath)
{
    Device previousCatalog;
    previousCatalog.ID = activeDevice->ID;
    previousCatalog.loadDevice(m_connectionName);

    //Get new values
    //Other values
    activeDevice->catalog->fileType         = ui->Catalogs_comboBox_FileType->itemData(
                ui->Catalogs_comboBox_FileType->currentIndex(),Qt::UserRole).toString();
    activeDevice->catalog->includeHidden    = ui->Catalogs_comboBox_IncludeHidden->itemData(
                ui->Catalogs_comboBox_IncludeHidden->currentIndex(), Qt::UserRole).toBool();
    activeDevice->catalog->includeMetadata  = ui->Catalogs_comboBox_MetaDataOption->itemData(
                ui->Catalogs_comboBox_MetaDataOption->currentIndex(), Qt::UserRole).toString();
    activeDevice->catalog->includeChecksum  = ui->Catalogs_comboBox_ChecksumOption->itemData(
                ui->Catalogs_comboBox_ChecksumOption->currentIndex(), Qt::UserRole).toString();
    activeDevice->catalog->isFullDevice     = ui->Catalogs_checkBox_isFullDevice->checkState();
    //DEV:QString newIncludeSymblinks  = ui->Catalogs_checkBox_IncludeSymblinks->currentText();

    //Confirm save changes to catalog's files selection
    bool changesToFileSelectionMade = false;
    bool rescanNeeded = false;
    QString message = tr("Save changes to the definition of the catalog?<br/>");
    message = message + "<table> <tr><td width=155><i>" + tr("field") + "</i></td><td width=125><i>" + tr("previous value") + "</i></td><td width=200><i>" + tr("new value") + "</i></td>";

    if(activeDevice->catalog->fileType       !=previousCatalog.catalog->fileType){
        message = message + "<tr><td>" + tr("File Type")    + "</td><td>" + previousCatalog.catalog->fileType     + "</td><td><b>" + activeDevice->catalog->fileType      + "</b></td></tr>";
        changesToFileSelectionMade = true;
        rescanNeeded = true;
    }
    if(activeDevice->catalog->includeHidden  != previousCatalog.catalog->includeHidden){
        QString previousValue = previousCatalog.catalog->includeHidden ? tr("All") : tr("None");
        QString newValue = activeDevice->catalog->includeHidden ? tr("All") : tr("None");
        message = message + "<tr><td>" + tr("Include Hidden") + "</td><td>" + previousValue + "</td><td><b>" + newValue + "</b></td></tr>";
        changesToFileSelectionMade = true;
        rescanNeeded = true;
    }
    if(activeDevice->catalog->includeMetadata != previousCatalog.catalog->includeMetadata){
        message = message + "<tr><td>" + tr("Include Metadata") + "</td><td>" + QVariant(previousCatalog.catalog->includeMetadata).toString() + "</td><td><b>" + QVariant(activeDevice->catalog->includeMetadata).toString() + "</b></td></tr>";
        changesToFileSelectionMade = true;
        rescanNeeded = true;
    }
    if(activeDevice->catalog->includeChecksum != previousCatalog.catalog->includeChecksum){
        message = message + "<tr><td>" + tr("Include Checksum") + "</td><td>" + previousCatalog.catalog->includeChecksum + "</td><td><b>" + activeDevice->catalog->includeChecksum + "</b></td></tr>";
        changesToFileSelectionMade = true;
        // Rescan needed except when changing from a hash to None (we keep existing hashes)
        if(!(previousCatalog.catalog->includeChecksum != Catalog::CHECKSUM_NONE
             && activeDevice->catalog->includeChecksum == Catalog::CHECKSUM_NONE)){
            rescanNeeded = true;
        }
    }
    if(activeDevice->catalog->isFullDevice  != previousCatalog.catalog->isFullDevice){
        message = message + "<tr><td>" + tr("Is Full Device") + "</td><td>" + QVariant(previousCatalog.catalog->isFullDevice).toString() + "</td><td><b>" + QVariant(activeDevice->catalog->isFullDevice).toString() + "</b></td></tr>";
        changesToFileSelectionMade = true;
        rescanNeeded = true;
    }
    if(activeDevice->path != previousPath){
        message = message + "<tr><td>" + tr("Source Path") + "</td><td>" + previousPath + "</td><td><b>" + activeDevice->path + "</b></td></tr>";
        changesToFileSelectionMade = true;
        rescanNeeded = true;
    }
    message = message + "</table>";

    if(changesToFileSelectionMade){
        message = message + + "<br/><br/>" + tr("(The catalog must be updated to reflect these changes)");
        int result = QMessageBox::warning(this, "Katalog", message, QMessageBox::Yes | QMessageBox::Cancel);
        if ( result == QMessageBox::Cancel){
            // Restore path if it was changed (already saved by saveDeviceForm before this method)
            if (activeDevice->path != previousPath) {
                activeDevice->path = previousPath;
                activeDevice->catalog->sourcePath = previousPath;
                activeDevice->saveDevice();
            }
            return;
        }
    }

    // Write all changes to database
    activeDevice->catalog->saveCatalog();

    // (for Memory mode) Update catalog file headers and Rename catalog file if device name changed
    activeDevice->catalog->updateCatalogFileHeaders(collection->databaseMode);
    activeDevice->catalog->renameCatalogFile(activeDevice->name);

    // Handle metadata field transitions based on includeMetadata change
    if (activeDevice->catalog->includeMetadata != previousCatalog.catalog->includeMetadata) {
        activeDevice->catalog->handleMetadataTransition(
            previousCatalog.catalog->includeMetadata,
            activeDevice->catalog->includeMetadata
            );
    }

    // Update the list of files if the changes impact the contents (i.e. path, file type, hidden, checksum)
    if (rescanNeeded) {
        if (activeDevice->path != previousPath) {
            // Path changed: offer quick prefix update or full re-scan
            QMessageBox msgBox;
            msgBox.setWindowTitle("Katalog");
            msgBox.setText(tr("The catalog source path changed.")
                           + "<br/><br/><table>"
                           + "<tr><td>" + tr("Old path:") + "</td><td><b>" + previousPath          + "</b></td></tr>"
                           + "<tr><td>" + tr("New path:") + "</td><td><b>" + activeDevice->path    + "</b></td></tr>"
                           + "</table><br/>"
                           + tr("How should the catalog indexes be updated?"));
            msgBox.setIcon(QMessageBox::Question);
            QPushButton *btnReplace = msgBox.addButton(tr("Replace path root"), QMessageBox::AcceptRole);
            QPushButton *btnRescan  = msgBox.addButton(tr("Full re-index"),        QMessageBox::AcceptRole);
            /*QPushButton *btnSkip =*/ msgBox.addButton(tr("Skip"),               QMessageBox::RejectRole);
            msgBox.exec();

            if (msgBox.clickedButton() == btnReplace) {
                setCatalogUpdateUIState(true);
                deviceUpdateManager->replaceStorageRoot(activeDevice,
                                                        previousPath,
                                                        activeDevice->path,
                                                        collection->databaseMode,
                                                        collection->folder);
            } else if (msgBox.clickedButton() == btnRescan) {
                activeDevice->catalog->loadCatalog();
                setCatalogUpdateUIState(true);
                deviceUpdateManager->updateDeviceHierarchy(activeDevice,
                                                           collection->databaseMode,
                                                           collection->folder,
                                                           "update");
            }
            // else Skip — do nothing further
        } else {
            // Other criteria changed (file type, hidden, checksum…): original Yes/No
            int updatechoice = QMessageBox::warning(this, "Katalog",
                                                    tr("Update the catalog content with the new criteria?\n"),
                                                    QMessageBox::Yes | QMessageBox::No);
            if (updatechoice == QMessageBox::Yes) {
                activeDevice->catalog->loadCatalog();
                setCatalogUpdateUIState(true);
                deviceUpdateManager->updateDeviceHierarchy(activeDevice,
                                                           collection->databaseMode,
                                                           collection->folder,
                                                           "update");
            }
        }
    }

    //Refresh
    if(collection->databaseMode=="Memory")
        collection->loadCatalogFilesToTable();
}
//--------------------------------------------------------------------------
void MainWindow::updateCatalogsScreenStatistics()
{
    QSqlQuery querySumCatalogValues(QSqlDatabase::database(m_connectionName));

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
    ui->Catalogs_label_TotalSizeValue->setText(QLocale().formattedDataSize(querySumCatalogValues.value(1).toLongLong()));
    ui->Catalogs_label_TotalNumberValue->setText(QLocale().toString(querySumCatalogValues.value(2).toInt()));
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

    //Prepare a dateTime to add to device or catalog names and avoid duplicates
    QString dateTimeForCatalogName = "_" + QDateTime::currentDateTime().toString("yy-MM-dd hh-mm-ss");

    // Start animation while cataloging
    QApplication::setOverrideCursor(Qt::WaitCursor);

    //Open file for first pass
    if(!sourceFile.open(QIODevice::ReadOnly)) {
        QApplication::restoreOverrideCursor();
        QMessageBox::information(this,"Katalog",tr("No catalog found."));
        return;
    }

    QTextStream textStream(&sourceFile);
    QString line;

    //Process and check Headers line
    line = textStream.readLine();

    //Check this is the right source format
    if (line.left(6)!="Volume"){
        sourceFile.close();
        QApplication::restoreOverrideCursor();
        QMessageBox::warning(this,"Katalog",tr("A file was found, but could not be loaded") +".\n");
        return;
    }

    // FIRST PASS: Collect unique catalog names
        QSet<QString> uniqueCatalogNames;
        while (!textStream.atEnd()) {
            line = textStream.readLine();
            if (!line.isEmpty()) {
                QStringList fieldList = line.split("\t");
                if (fieldList.count() == 7) {
                    QString catalogName = fieldList[0];
                    catalogName.remove("\"");
                    catalogName.replace("/", "_");
                    catalogName += dateTimeForCatalogName;
                    uniqueCatalogNames.insert(catalogName);
                }
            }
        }
        sourceFile.close();

        // CREATE DEVICES AND CATALOGS, BUILD NAME->ID MAP
        QString virtualCatalogFolder = "/import";

        //Create a virtual device to host the new catalogs
        Device importVirtualDevice;
        importVirtualDevice.generateDeviceID();
        importVirtualDevice.type = "Virtual";
        importVirtualDevice.name = "imports from VVV " + dateTimeForCatalogName;
        importVirtualDevice.parentID = 0;
        importVirtualDevice.groupID = 1;
        importVirtualDevice.path = virtualCatalogFolder;
        importVirtualDevice.dateTimeUpdated = QDateTime::currentDateTime();
        importVirtualDevice.insertDevice();
        collection->saveDeviceTableToFile();

        // Map catalog name -> catalog ID and Device pointer
        QMap<QString, qint64> catalogNameToId;
        QMap<QString, Device*> catalogNameToDevice;

        for (const QString& catalogName : uniqueCatalogNames) {
            //Create Device
            Device* importedDevice = new Device();
            importedDevice->generateDeviceID();
            importedDevice->name = catalogName;
            importedDevice->type = "Catalog";
            importedDevice->parentID = importVirtualDevice.ID;
            importedDevice->groupID = 1;
            importedDevice->path = virtualCatalogFolder;
            importedDevice->catalog->generateID();
            importedDevice->externalID = importedDevice->catalog->ID;
            importedDevice->dateTimeUpdated = QDateTime::currentDateTime();
            importedDevice->insertDevice();

            //Get info for the new catalog
            importedDevice->catalog->name = importedDevice->name;
            importedDevice->catalog->filePath = collection->folder + "/" + importedDevice->name + ".idx";
            importedDevice->catalog->sourcePath = virtualCatalogFolder;
            importedDevice->catalog->includeHidden = 1;
            importedDevice->catalog->includeSymblinks = 0;
            importedDevice->catalog->isFullDevice = 0;
            importedDevice->catalog->includeMetadata = Catalog::METADATA_NONE;
            importedDevice->catalog->appVersion = currentVersion;
            importedDevice->catalog->insertCatalog();

            // Store mapping
            catalogNameToId[catalogName] = importedDevice->catalog->ID;
            catalogNameToDevice[catalogName] = importedDevice;
        }

    // SECOND PASS: Import files and folders with correct IDs

        //Clear database tables
        QSqlQuery deleteQuery(QSqlDatabase::database(m_connectionName));
        deleteQuery.exec("DELETE FROM file");
        deleteQuery.exec("DELETE FROM folder");

        //Prepare query to load file info
        QSqlQuery insertQuery(QSqlDatabase::database(m_connectionName));
        QString insertSQL = QLatin1String(R"(
                                        INSERT INTO file (
                                                        file_catalog_id,
                                                        file_name,
                                                        file_folder_path,
                                                        file_size,
                                                        file_date_updated,
                                                        file_catalog )
                                        VALUES(
                                                        :file_catalog_id,
                                                        :file_name,
                                                        :file_folder_path,
                                                        :file_size,
                                                        :file_date_updated,
                                                        :file_catalog )
                                                    )");
        insertQuery.prepare(insertSQL);

        //Prepare insert query for folder
        QSqlQuery insertFolderQuery(QSqlDatabase::database(m_connectionName));
        Database::DatabaseType dbType = Database::getDatabaseType(m_connectionName);
        QString insertFolderSQL;
        if (dbType == Database::DatabaseType::PostgreSQL) {
            // PostgreSQL requires ON CONFLICT clause
            insertFolderSQL = QString(R"(
                                            %1 INTO folder(
                                                folder_catalog_id,
                                                folder_path
                                             )
                                            VALUES(
                                                :folder_catalog_id,
                                                :folder_path)
                                            ON CONFLICT (folder_catalog_id, folder_path) DO NOTHING
                                            )").arg(Database::getInsertOrIgnorePrefix(dbType));
        } else {
            insertFolderSQL = QString(R"(
                                            %1 INTO folder(
                                                folder_catalog_id,
                                                folder_path
                                             )
                                            VALUES(
                                                :folder_catalog_id,
                                                :folder_path)
                                            )").arg(Database::getInsertOrIgnorePrefix(dbType));
        }
        insertFolderQuery.prepare(insertFolderSQL);

        //Re-open file for second pass
        if(!sourceFile.open(QIODevice::ReadOnly)) {
            QApplication::restoreOverrideCursor();
            QMessageBox::information(this,"Katalog",tr("No catalog found."));
            return;
        }

        textStream.setDevice(&sourceFile);
        textStream.readLine(); // Skip header

        while (!textStream.atEnd()) {
            line = textStream.readLine();
            if (!line.isEmpty()) {
                QStringList fieldList = line.split("\t");
                if (fieldList.count() == 7) {
                    QString catalogName = fieldList[0];
                    catalogName.remove("\"");
                    catalogName.replace("/", "_");
                    catalogName += dateTimeForCatalogName;

                    qint64 catalogId = catalogNameToId.value(catalogName, 0);
                    QString folderPath = virtualCatalogFolder + QString(fieldList[1]).remove("\"");

                    //Append file data to the database
                    insertQuery.bindValue(":file_catalog_id", catalogId);
                    insertQuery.bindValue(":file_name", QString(fieldList[2]).remove("\""));
                    insertQuery.bindValue(":file_folder_path", folderPath);
                    insertQuery.bindValue(":file_size", fieldList[3].toLongLong());
                    insertQuery.bindValue(":file_date_updated", fieldList[5]);
                    insertQuery.bindValue(":file_catalog", catalogName);
                    insertQuery.exec();

                    //Append folder data to the database
                    insertFolderQuery.bindValue(":folder_catalog_id", catalogId);
                    insertFolderQuery.bindValue(":folder_path", folderPath);
                    insertFolderQuery.exec();
                }
            }
        }

        sourceFile.close();

        //Insert root folder for each catalog
        for (auto it = catalogNameToId.begin(); it != catalogNameToId.end(); ++it) {
            insertFolderQuery.bindValue(":folder_catalog_id", it.value());
            insertFolderQuery.bindValue(":folder_path", virtualCatalogFolder);
            insertFolderQuery.exec();
        }

        //Complete table for missing folders
        createMissingParentDirectories();

    // EXPORT CATALOG FILES AND UPDATE STATISTICS

        //Iterate each catalog to generate related files
        for (auto it = catalogNameToDevice.begin(); it != catalogNameToDevice.end(); ++it) {
            Device* importedDevice = it.value();

            //Update total number and size of the files
            QString statsSQL = QLatin1String(R"(
                                                        SELECT COUNT(*), SUM(file_size)
                                                        FROM file
                                                        WHERE file_catalog_id =:file_catalog_id
                                                )");
            QSqlQuery statsQuery(QSqlDatabase::database(m_connectionName));
            statsQuery.prepare(statsSQL);
            statsQuery.bindValue(":file_catalog_id", importedDevice->externalID);
            statsQuery.exec();
            statsQuery.next();

            importedDevice->totalFileCount = statsQuery.value(0).toLongLong();
            importedDevice->totalFileSize  = statsQuery.value(1).toLongLong();
            importedDevice->saveDevice();
            collection->saveDeviceTableToFile();

            //Export the catalog file

            //Prepare the catalog file path
            QFile fileOut(importedDevice->catalog->filePath);

            //Prepare the stream and file headers
            QTextStream out(&fileOut);
            if(fileOut.open(QIODevice::WriteOnly | QIODevice::Truncate)) {

                out  << "<catalogSourcePath>" + virtualCatalogFolder << "\n"
                    << "<catalogFileCount>" + QString::number(importedDevice->totalFileCount)    << "\n"
                    << "<catalogTotalFileSize>" + QString::number(importedDevice->totalFileSize) << "\n"
                    << "<catalogIncludeHidden>"       << "\n"
                    << "<catalogFileType>"            << "\n"
                    << "<catalogStorage>"             << "\n"
                    << "<catalogIncludeSymblinks>"    << "\n"
                    << "<catalogIsFullDevice>"        << "\n"
                    << "<catalogIncludeMetadata>"     << "\n"
                    << "<catalogAppVersion>" + currentVersion << "\n"
                    << "<catalogID>" + QString::number(importedDevice->externalID) << "\n";
            }

            //Get the list of file to add
            QString listFilesSQL = QLatin1String(R"(
                                                    SELECT
                                                        file_folder_path,
                                                        file_name,
                                                        file_size,
                                                        file_date_updated
                                                    FROM file
                                                    WHERE file_catalog_id =:file_catalog_id
                                                )");
            QSqlQuery listFilesQuery(QSqlDatabase::database(m_connectionName));
            listFilesQuery.prepare(listFilesSQL);
            listFilesQuery.bindValue(":file_catalog_id", importedDevice->externalID);
            listFilesQuery.exec();

            //Write the results in the file
            while (listFilesQuery.next()) {
                // file_folder_path already includes the /import prefix
                out << listFilesQuery.value(0).toString() + "/" + listFilesQuery.value(1).toString();
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

            //Prepare the folders file path
            QFile fileFolderOut(collection->folder + "/" + importedDevice->name + ".folders.idx");

            //Prepare the stream and file headers
            QTextStream folderOut(&fileFolderOut);
            if(fileFolderOut.open(QIODevice::WriteOnly | QIODevice::Truncate)) {

                //Get the list of file to add
                QString listFoldersSQL = QLatin1String(R"(
                                                SELECT
                                                    folder_catalog_id,
                                                    folder_path
                                                FROM folder
                                                WHERE folder_catalog_id =:folder_catalog_id
                                            )");
                QSqlQuery listFoldersQuery(QSqlDatabase::database(m_connectionName));
                listFoldersQuery.prepare(listFoldersSQL);
                listFoldersQuery.bindValue(":folder_catalog_id", importedDevice->externalID);
                listFoldersQuery.exec();

                //Write the results in the file
                while (listFoldersQuery.next()) {
                    // folder_path already includes the /import prefix
                    folderOut << listFoldersQuery.value(0).toString();
                    folderOut << '\t';
                    folderOut << listFoldersQuery.value(1).toString();
                    folderOut << '\n';
                }
            }

            fileFolderOut.close();
        }

    //Cleanup allocated Device pointers
    qDeleteAll(catalogNameToDevice);

    //update virtual device
    importVirtualDevice.updateNumbersFromChildren();
    collection->saveDeviceTableToFile();

    //Stop animation
    QApplication::restoreOverrideCursor();

    loadCollection();
}
//--------------------------------------------------------------------------
void MainWindow::createMissingParentDirectories() {
    QSqlQuery query(QSqlDatabase::database(m_connectionName));

    // Select distinct folder paths
    query.exec("SELECT DISTINCT folder_catalog_id, folder_path FROM folder");

    // Iterate through the result set
    while (query.next()) {
        int folderCatalogID = query.value(0).toInt();
        QString folderPath = query.value(1).toString();

        // Split the folder path into components
        QStringList folders = folderPath.split('/', Qt::SkipEmptyParts);
        QString currentPath;

        // Iterate through the components and insert missing parent directories
        for (const QString& folder : folders) {
            currentPath += '/' + folder;

            // Check if the current path exists in the table
            QSqlQuery checkQuery(QSqlDatabase::database(m_connectionName));
            checkQuery.prepare("SELECT 1 FROM folder WHERE folder_catalog_id = :catalog_id AND folder_path = :path");
            checkQuery.bindValue(":folder_catalog_id", folderCatalogID);
            checkQuery.bindValue(":path", currentPath);

            if (!checkQuery.exec()) {
                qWarning() << "WARNING: Error checking path:" << checkQuery.lastError().text();
            }

            // If the current path doesn't exist, insert it
            if (!checkQuery.next()) {
                QSqlQuery insertQuery(QSqlDatabase::database(m_connectionName));
                insertQuery.prepare("INSERT INTO folder (folder_catalog_id, folder_path) VALUES (:folder_catalog_id, :path)");
                insertQuery.bindValue(":folder_catalog_id", folderCatalogID);
                insertQuery.bindValue(":path", currentPath);

                if (!insertQuery.exec()) {
                    qWarning() << "WARNING: Error inserting path:" << insertQuery.lastError().text();
                }
            }
        }
    }
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
void MainWindow::verifyCatalogChecksums()
{
    if (!activeDevice || !activeDevice->catalog) {
        return;
    }

    Catalog *catalog = activeDevice->catalog;

    // In Memory mode, load catalog first if needed
    if (collection->databaseMode == "Memory") {
        if (catalog->dateLoaded < catalog->dateUpdated || catalog->dateLoaded.isNull()) {

            bool stopRequested = false;
            QMutex mutex;

            // Connect to catalog loading progress using statusbar
            QMetaObject::Connection progressConnection = connect(catalog, &Catalog::loadProgress,
                this, [this, catalog](int current, int total) {

                StatusBarMessageBuilder builder;
                builder.setOperation(tr("Verify Checksums"))
                    .setStatus(tr("Loading"))
                    .setDeviceContext(1, 1, catalog->name)
                    .setProcess(tr("files"), current, total);

                statusBar()->show();
                statusBarLabel->setText(builder.build());
                QCoreApplication::processEvents();
            });

            catalog->loadCatalogFileListToTable(mutex, stopRequested);

            disconnect(progressConnection);

            // Clear status bar
            statusBarLabel->clear();
        }
    }

    // Count files to verify
    int totalFiles = FileChecksum::countFilesWithChecksum(m_connectionName, catalog->ID);

    if (totalFiles == 0) {
        QMessageBox::information(this, tr("Katalog"),
                                tr("No checksums to verify."));
        return;
    }

    // Confirm
    int choice = QMessageBox::question(this, tr("Katalog"),
                                      tr("Verify checksums for %1 files?").arg(totalFiles),
                                      QMessageBox::Yes | QMessageBox::No);

    if (choice != QMessageBox::Yes) {
        return;
    }

    // Run verification with statusbar progress
    bool userCancelled = false;

    auto shouldContinue = [&]() {
        return !userCancelled;
    };

    auto progressCallback = [&](int current, int total, const QString &fileName) {
        StatusBarMessageBuilder builder;
        builder.setOperation(tr("Verify Checksums"))
            .setStatus(tr("Verifying"))
            .setDeviceContext(1, 1, catalog->name)
            .setProcess(fileName, current, total);

        statusBar()->show();  // ← ADD THIS
        statusBarLabel->setText(builder.build());
        QCoreApplication::processEvents();
    };

    // Run verification
    FileChecksum::CatalogVerificationResult result =
        FileChecksum::verifyCatalogChecksums(m_connectionName,
                                            catalog->ID,
                                            catalog->sourcePath,
                                            shouldContinue,
                                            progressCallback);

    // Clear status bar
    statusBarLabel->clear();

    // Show results
    QString resultMessage;
    QMessageBox::Icon icon;

    if (result.mismatches == 0 && result.missing == 0) {
        // All good
        icon = QMessageBox::Information;
        resultMessage = tr("Verified:") + " " + QString::number(result.verified) + "\n\n"
                      + tr("All checksums match.");
    } else {
        // Problems found
        icon = QMessageBox::Warning;
        resultMessage = tr("Verified:") + " " + QString::number(result.verified) + "\n"
                      + tr("Mismatches:") + " " + QString::number(result.mismatches) + "\n"
                      + tr("Missing:") + " " + QString::number(result.missing) + "\n";

        if (result.mismatches > 0) {
            resultMessage += "\n" + tr("Mismatched files:") + "\n";
            for (const QString &file : result.mismatchedFiles) {
                resultMessage += "  " + file + "\n";
            }
        }

        if (result.missing > 0) {
            resultMessage += "\n" + tr("Missing files:") + "\n";
            for (const QString &file : result.missingFiles) {
                resultMessage += "  " + file + "\n";
            }
        }
    }

    QMessageBox msgBox(this);
    msgBox.setWindowTitle("Katalog");
    msgBox.setIcon(icon);
    msgBox.setText(resultMessage);
    msgBox.exec();
}
//--------------------------------------------------------------------------

//--------------------------------------------------------------------------
//--- Reporting ------------------------------------------------------------
//--------------------------------------------------------------------------
bool MainWindow::reportAllUpdates(Device *device, QList<qint64> list, QString updateType)
{//Provide a report for any combinaison of updates (updateType = create, single, or list) and devices
    QApplication::restoreOverrideCursor();

    //Storage root path replace
    if (updateType == "replaceRoot") {
        if (list.size() >= 4 && list[0] == 1) {
            statusBarLabel->setText(
                tr("Storage path updated:")
                + " " + QString::number(list[1]) + " " + tr("catalog(s),")
                + " " + QString::number(list[2]) + " " + tr("file(s),")
                + " " + QString::number(list[3]) + " " + tr("folder(s)"));
        }
        return true;
    }

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
            parentDevice.loadDevice(m_connectionName);

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
            parentDevice.loadDevice(m_connectionName);

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
        //Storage
        message += "<table>";
        message += "<tr><td>"+tr("Storage updated: ")+ "</td><td align='center'><b>" + device->name + "</b></td></tr>";
        message += "<tr><td>"+tr("Path: ")           + "</td><td align='right'> <b>" + device->path + "</b></td></tr>";
        message += "</table>";

        //Catalogs
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
        if(list[0]==1){
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

            //Catalog updated
            message = QString(tr("<table>"
                                 "<br/>Selected active catalogs from <b>%1</b> are updated.&nbsp;<br/>")).arg(device->name);
            message += QString(
                           "<tr><td>Number of files: </td><td align='center'><b> %1 </b></td><td>&nbsp; &nbsp; (added: </td><td align='right'><b> %2 </b>)&nbsp; &nbsp; </td></tr>"
                           "<tr><td>Total file size: </td><td align='right'> <b> %3 </b></td><td>&nbsp; &nbsp; (added: </td><td align='right'><b> %4 </b>)&nbsp; &nbsp; </td></tr>"
                           ).arg(QString::number(list[1]),
                                QString::number(list[2]),
                                QLocale().formattedDataSize(list[3]),
                                QLocale().formattedDataSize(list[4]));

            message += "</table><br/>" + QString(tr("Catalogs updated:<b> %1 </b>(%2 skipped)")).arg(QString::number(list[5]),QString::number(list[6]))+"<br/>";
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
    if (device->type=="Virtual" and updateType=="update"){
        message = "";

        for (int i = 0; i < list.size(); ++i) {
        }

        // Report virtual device updated
        if(list.size() > 0 && list[0]==1){
            message += "<table>";
            message += "<tr><td>" + tr("Virtual device updated: ") + "</td><td align='center'><b>" + device->name + "</b></td></tr>";
            message += "</table><br/>";

            // ALWAYS show catalog statistics (list[1-6]) - moved OUT of the storage check
            message += "<table>";
            message += QString(
                "<tr><td>Number of files: </td><td align='center'><b> %1 </b></td><td>&nbsp; &nbsp; (added: </td><td align='right'><b> %2 </b>)&nbsp; &nbsp; </td></tr>"
                "<tr><td>Total file size: </td><td align='right'> <b> %3 </b></td><td>&nbsp; &nbsp; (added: </td><td align='right'><b> %4 </b>)&nbsp; &nbsp; </td></tr>"
                ).arg(QString::number(list[1]),
                      QString::number(list[2]),
                      QLocale().formattedDataSize(list[3]),
                      QLocale().formattedDataSize(list[4]));
            message += "</table><br/>";

            message += QString(tr("Catalogs updated:<b> %1 </b>(%2 skipped)")).arg(QString::number(list[5]),QString::number(list[6]))+"<br/>";

            reportAvailable = true;
        }

        // Report child storage updates if any occurred (OPTIONAL - only if storage was updated)
        if(list[7]==1){
            message += "<br/>";
            message += "<table>";
            message += "<tr><td>" + tr("Storage") + "</td></tr>";
            message += "<tr><td>" + tr("Used Space: ") + "</td><td align='right'><b>" + QLocale().formattedDataSize(list[8])  + "</b></td><td>&nbsp; &nbsp; " + tr("(added: ") + "&nbsp; &nbsp; </td><td align='right'><b>" + QLocale().formattedDataSize(list[9])  + "</b>)</td></tr>";
            message += "<tr><td>" + tr("Free Space: ") + "</td><td align='right'><b>" + QLocale().formattedDataSize(list[10]) + "</b></td><td>&nbsp; &nbsp; " + tr("(added: ") + "&nbsp; &nbsp; </td><td align='right'><b>" + QLocale().formattedDataSize(list[11]) + "</b>)</td></tr>";
            message += "<tr><td>" + tr("Total Space: ") + "</td><td align='right'><b>" + QLocale().formattedDataSize(list[12]) + "</b></td><td>&nbsp; &nbsp; " + tr("(added: ") + "&nbsp; &nbsp; </td><td align='right'><b>" + QLocale().formattedDataSize(list[13]) + "</b>)</td></tr>";
            message += "</table>";
        }

        // Show the report
        if(list[0]==1){
            msgBox.setWindowTitle("Katalog");
            msgBox.setText(message);
            msgBox.setIcon(QMessageBox::Information);
            msgBox.exec();
        }
    }

    return reportAvailable;
}
//--------------------------------------------------------------------------

//--------------------------------------------------------------------------
//--- Command lines
//--------------------------------------------------------------------------
void MainWindow::cmd_updateCatalog(int deviceId, bool displayReport)
{

    //Set selected device to the one specified by catalogId
    selectedDevice->ID = deviceId;
    selectedDevice->loadDevice(m_connectionName);

    if(selectedDevice->type != "Catalog"){
        return;
    }


    // Perform the update operation
    //Update and report if active
    if(selectedDevice->active==true){
        deviceUpdateManager->updateDeviceHierarchy(selectedDevice,
                                                  collection->databaseMode,
                                                  collection->folder,
                                                  "update");

        //Save data
        collection->saveDeviceTableToFile();
        collection->saveStatiticsTableToFile();

        //Report device info after update
    }
    else{
    }
}
//--------------------------------------------------------------------------
void MainWindow::cmd_listGroup0Catalogs()
{
    //Query the database for all devices of type catalog in the device group 0
    QSqlQuery query(QSqlDatabase::database(m_connectionName));
    QString querySQL = QLatin1String(R"(
                                    SELECT device_id, device_name, device_active
                                    FROM device
                                    WHERE device_type = 'Catalog'
                                    AND device_group_id = 0
                                    ORDER BY device_id
                                )");

    //Prepare and execute the query
    query.prepare(querySQL);
    query.exec();

    while (query.next()) {
    }
}
//--------------------------------------------------------------------------
void MainWindow::cmd_updateAllActive(bool displayReport)
{
    //Select all active catalog devices from database
    QSqlQuery query(QSqlDatabase::database(m_connectionName));
    QString querySQL = QLatin1String(R"(
                           SELECT device_id
                           FROM device
                           WHERE device_type = 'Catalog'
                           AND device_active = 1
                           ORDER BY device_id
                       )");
    query.prepare(querySQL);
    query.exec();

    //Update each catalog
    while (query.next()) {
        int deviceID = query.value(0).toInt();
        cmd_updateCatalog(deviceID, displayReport);
    }

}
//--------------------------------------------------------------------------

QStandardItemModel* MainWindow::buildFilteredDeviceTreeModel(QObject *parent)
{
    // Build a device tree model filtered by selectedDevice->deviceIDList
    // This reuses the same SQL logic as loadDevicesTreeToModel() but returns the model

    QSqlQuery query(QSqlDatabase::database(m_connectionName));
    QString querySQL;

    // Use recursive CTE to get device hierarchy
    querySQL = QLatin1String(R"(
        WITH RECURSIVE device_tree AS (
            -- Base case: start with root devices (Physical Group and Virtual Groups)
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

            -- Recursive case: children
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
            device_active
        FROM device_tree
        WHERE 1=1
    )");

    // Apply filtering based on selectedDevice->deviceIDList if not empty
    if (!selectedDevice->deviceIDList.isEmpty() && selectedDevice->ID != 0) {
        // Build IN clause for filtering
        QStringList idStrings;
        for (int id : selectedDevice->deviceIDList) {
            idStrings << QString::number(id);
        }

        // Include the devices and their ancestors to maintain tree structure
        querySQL += QString(" AND (device_id IN (%1) OR device_id IN ("
                            "    WITH RECURSIVE ancestors AS ("
                            "        SELECT device_parent_id FROM device WHERE device_id IN (%1)"
                            "        UNION"
                            "        SELECT d.device_parent_id FROM device d"
                            "        JOIN ancestors a ON d.device_id = a.device_parent_id"
                            "    )"
                            "    SELECT device_parent_id FROM ancestors WHERE device_parent_id > 0"
                            "))").arg(idStrings.join(","));
    }

    querySQL += " ORDER BY level ASC, device_type DESC, device_parent_id ASC, device_id ASC";

    query.prepare(querySQL);
    query.exec();

    // Create the model with minimal columns: Name, Type, Active, ID, ParentID
    QStandardItemModel *model = new QStandardItemModel(parent);
    model->setHorizontalHeaderLabels({
        tr("Name"),         // 0
        tr("Device Type"),  // 1
        tr("Active"),       // 2
        tr("ID"),           // 3
        tr("Parent ID")     // 4
    });

    // Map to store items by ID for building hierarchy
    QMap<int, QStandardItem*> itemMap;

    while (query.next()) {
        int id = query.value(0).toInt();
        int parentId = query.value(1).toInt();
        QString name = query.value(2).toString();
        QString type = query.value(3).toString();
        //int externalId = query.value(4).toInt();
        bool isActive = query.value(5).toBool();

        // Create row items
        QList<QStandardItem*> rowItems;
        rowItems << new QStandardItem(name);                        // 0 - Name
        rowItems << new QStandardItem(type);                        // 1 - Type
        rowItems << new QStandardItem(QString::number(isActive));   // 2 - Active
        rowItems << new QStandardItem(QString::number(id));         // 3 - ID
        rowItems << new QStandardItem(QString::number(parentId));   // 4 - Parent ID

        QStandardItem* item = rowItems.at(0);
        QStandardItem* parentItem = itemMap.value(parentId);

        // Add to model
        if (parentId == 0) {
            model->appendRow(rowItems);
        } else if (parentItem) {
            parentItem->appendRow(rowItems);
        } else {
            // Parent not found, add at root level
            model->appendRow(rowItems);
        }

        itemMap.insert(id, item);
    }

    return model;
}
