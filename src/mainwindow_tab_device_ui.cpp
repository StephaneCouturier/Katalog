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
// File Name:   mainwindow_tab_device_ui.cpp
// Purpose:     https://stephanecouturier.github.io/Katalog/docs/Features/Devices
// Description:
// Author:      Stephane Couturier
/////////////////////////////////////////////////////////////////////////////
*/

#include "mainwindow.h"
#include "ui_mainwindow.h"
#include "devicetreeview.h"
#include "core/device.h"
#include "mainwindow_ui_wrapper_device.h"

//TAB: DEVICES -------------------------------------------------------------
//--- UI -------------------------------------------------------------------
//--------------------------------------------------------------------------
void MainWindow::on_Devices_radioButton_DeviceTree_clicked()
{
    QSettings settings(collection->settingsFilePath, QSettings:: IniFormat);
    settings.setValue("Devices/DisplayContents", "Tree");

    loadDevicesView("");
}
//--------------------------------------------------------------------------
void MainWindow::on_Devices_radioButton_StorageList_clicked()
{
    QSettings settings(collection->settingsFilePath, QSettings:: IniFormat);
    settings.setValue("Devices/DisplayContents", "Storage");

    loadDevicesView("");
}
//--------------------------------------------------------------------------
void MainWindow::on_Devices_radioButton_CatalogList_clicked()
{
    QSettings settings(collection->settingsFilePath, QSettings:: IniFormat);
    settings.setValue("Devices/DisplayContents", "Catalogs");

    loadDevicesView("");
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
    loadDevicesView("");
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

    loadDevicesView("");
}
//--------------------------------------------------------------------------
void MainWindow::on_Devices_checkBox_DisplayCatalogs_stateChanged(int arg1)
{
    QSettings settings(collection->settingsFilePath, QSettings:: IniFormat);
    settings.setValue("Devices/DisplayCatalogs", arg1);
    loadDevicesView("");
}
//--------------------------------------------------------------------------
void MainWindow::on_Devices_checkBox_DisplayPhysicalGroup_stateChanged(int arg1)
{
    QSettings settings(collection->settingsFilePath, QSettings:: IniFormat);
    settings.setValue("Devices/DisplayPhysicalGroup", arg1);
    if(arg1==0)
        ui->Devices_checkBox_DisplayVirtualGroups->setChecked(true);
    loadDevicesView("");
}
//--------------------------------------------------------------------------
void MainWindow::on_Devices_checkBox_DisplayVirtualGroups_stateChanged(int arg1)
{
    QSettings settings(collection->settingsFilePath, QSettings:: IniFormat);
    settings.setValue("Devices/DisplayVirtualGroups", arg1);
    if(arg1==0)
        ui->Devices_checkBox_DisplayPhysicalGroup->setChecked(true);
    loadDevicesView("");
}
//--------------------------------------------------------------------------
void MainWindow::on_Devices_checkBox_DisplayFullTable_stateChanged(int arg1)
{
    QSettings settings(collection->settingsFilePath, QSettings:: IniFormat);
    settings.setValue("Devices/DisplayFullDeviceTable", arg1);
    loadDevicesView("");
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
            // Use new catalog system for context menu update
            if (!catalogManager || catalogManager->catalogOperationRunning()) {
                qDebug() << "Catalog manager not available for context menu update";
                return;
            }

            qDebug() << "Device list context menu catalog update for:" << activeDevice->name;

            // Clear batch mode - this is a single update
            inBatchMode = false;
            currentUpdateDevice = activeDevice;
            activeDevice->catalog->appVersion = currentVersion;

            CatalogJobStoppable* catalogJobStoppable = new CatalogJobStoppable(this);

            if (catalogProgressManager) {
                catalogProgressManager->setCurrentCatalogEngine(catalogJobStoppable);
            }

            catalogManager->startCatalogJobStoppable(
                catalogJobStoppable,
                activeDevice,
                CatalogJobStoppable::UpdateCatalog,
                collection->databaseMode,
                collection->folder
                );
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

        // Storage update action
        QAction *menuDeviceAction1 = new QAction(QIcon::fromTheme("media-playlist-repeat"), tr("Update"), this);
        deviceContextMenu.addAction(menuDeviceAction1);
        connect(menuDeviceAction1, &QAction::triggered, this, [this, deviceName]() {
            qDebug() << "Storage update requested for:" << activeDevice->name;

            // Ensure DeviceManager is set up with proper UI integration
            if (!deviceManager) {
                setupDeviceManager();
            }

            // Ensure CatalogProgressManager is connected (double-check)
            if (catalogProgressManager && deviceManager) {
                deviceManager->setCatalogProgressManager(catalogProgressManager);
            }

            // Start device operation using DeviceManager
            DeviceJobStoppable* deviceEngine = new DeviceJobStoppable(this);

            deviceManager->startDeviceOperation(
                deviceEngine,
                activeDevice,
                DeviceJobStoppable::UpdateDevice,
                collection->databaseMode,
                collection->folder,
                catalogManager);

            qDebug() << "Device operation started with proper UI integration";
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
            // Use new catalog system for context menu update
            if (!catalogManager || catalogManager->catalogOperationRunning()) {
                qDebug() << "Catalog manager not available for context menu update";
                return;
            }

            qDebug() << "Device list context menu catalog update for:" << activeDevice->name;

            // Clear batch mode - this is a single update
            inBatchMode = false;
            currentUpdateDevice = activeDevice;
            activeDevice->catalog->appVersion = currentVersion;

            CatalogJobStoppable* catalogJobStoppable = new CatalogJobStoppable(this);

            if (catalogProgressManager) {
                catalogProgressManager->setCurrentCatalogEngine(catalogJobStoppable);
            }

            catalogManager->startCatalogJobStoppable(
                catalogJobStoppable,
                activeDevice,
                CatalogJobStoppable::UpdateCatalog,
                collection->databaseMode,
                collection->folder
                );
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
    list += DeviceUIWrapper::updateDeviceWithUI(activeDevice,
                                                "update",
                                                collection->databaseMode,
                                                true,
                                                collection->folder,
                                                false);
    //Report the change
    reportAllUpdates(activeDevice,
                     list,
                     "update");
    collection->saveDeviceTableToFile();
    collection->saveStatiticsTableToFile();
    loadDevicesView("");
    editDevice();
}
//--------------------------------------------------------------------------
void MainWindow::on_Devices_pushButton_ApplyToSelection_clicked()
{
    loadDevicesTreeToModel("Filters");
}
//--------------------------------------------------------------------------
void MainWindow::on_Catalogs_pushButton_UpdateCatalog_clicked()
{
    qDebug() << "=== SINGLE UPDATE CLICKED ===";

    if (!activeDevice || activeDevice->type != "Catalog") {
        qDebug() << "ERROR: No valid catalog device selected";
        return;
    }

    if (!catalogManager || catalogManager->catalogOperationRunning()) {
        qDebug() << "ERROR: Catalog manager not available or already running";
        return;
    }

    qDebug() << "Starting SINGLE catalog update for:" << activeDevice->name;

    // ENSURE we're NOT in batch mode
    inBatchMode = false;
    currentUpdateDevice = nullptr;
    qDebug() << "Cleared batch mode flag";

    // UI Protection
    QApplication::setOverrideCursor(Qt::WaitCursor);
    ui->Catalogs_pushButton_UpdateCatalog->setEnabled(false);
    qDebug() << "Set wait cursor and disabled button";

    // Set current device
    currentUpdateDevice = activeDevice;
    activeDevice->catalog->appVersion = currentVersion;
    qDebug() << "Set current device:" << activeDevice->name;

    // Start update
    CatalogJobStoppable* catalogJobStoppable = new CatalogJobStoppable(this);

    if (catalogProgressManager) {
        catalogProgressManager->setCurrentCatalogEngine(catalogJobStoppable);
        qDebug() << "Updated progress manager";
    }

    qDebug() << "Starting catalog update operation";
    catalogManager->startCatalogJobStoppable(
        catalogJobStoppable,
        activeDevice,
        CatalogJobStoppable::UpdateCatalog,
        collection->databaseMode,
        collection->folder
        );

    qDebug() << "Single catalog update started";
}
//--------------------------------------------------------------------------
void MainWindow::on_Catalogs_pushButton_UpdateAllActive_clicked()
{
    qDebug() << "=== UPDATE ALL ACTIVE CLICKED ===";

    if (!catalogManager) {
        qDebug() << "ERROR: Catalog manager not initialized";
        return;
    }

    if (catalogManager->catalogOperationRunning()) {
        qDebug() << "ERROR: Catalog operation already running";
        return;
    }

    // REUSE ORIGINAL TEXT: Ask user for report choice
    QMessageBox msgBox;
    msgBox.setWindowTitle("Katalog");
    msgBox.setText(tr("Do you want to see a report for each updated catalog?"));
    msgBox.setIcon(QMessageBox::Question);
    msgBox.setStandardButtons(QMessageBox::Yes|QMessageBox::No | QMessageBox::Cancel);
    int result = msgBox.exec();

    if ( result == QMessageBox::Yes){
        showEachCatalogUpdateSummary = true;  // Use member variable, not local variable
    }
    else if ( result == QMessageBox::Cancel){
        qDebug() << "User cancelled batch update";
        return;
    }

    qDebug() << "User chose to show individual reports:" << showEachCatalogUpdateSummary;

    // CLEAR ALL STATE - Start fresh
    qDeleteAll(batchCatalogs);
    batchCatalogs.clear();
    batchCurrentIndex = 0;
    inBatchMode = false;  // Will be set to true if we find catalogs
    currentUpdateDevice = nullptr;

    // Initialize global statistics
    globalUpdateTotalFiles = 0;
    globalUpdateDeltaFiles = 0;
    globalUpdateTotalSize = 0;
    globalUpdateDeltaSize = 0;
    updatedCatalogs = 0;
    skippedCatalogs = 0;

    qDebug() << "Collecting active catalogs from tree view...";

    // COLLECT ALL ACTIVE CATALOGS (same logic as original)
    for (int row = 0; row < ui->Devices_treeView_DeviceList->model()->rowCount(); ++row) {
        // Get the index for the "Active" field in the current row
        QModelIndex activeIndex = ui->Devices_treeView_DeviceList->model()->index(row, 2);

        // Retrieve the data for the "Active" field (assuming it contains an icon)
        QIcon activeIcon = qvariant_cast<QIcon>(ui->Devices_treeView_DeviceList->model()->data(activeIndex, Qt::DecorationRole));

        // Check if the icon is set to "dialog-ok-apply"
        if (activeIcon.name() == QIcon::fromTheme("dialog-ok-apply").name()) {
            Device* loopDevice = new Device();
            loopDevice->ID = ui->Devices_treeView_DeviceList->model()->data(ui->Devices_treeView_DeviceList->model()->index(row, 3)).toInt();
            loopDevice->loadDevice("defaultConnection");

            qDebug() << "Found active device:" << loopDevice->name << "Type:" << loopDevice->type;

            // Only process catalogs
            if (loopDevice->type == "Catalog") {
                loopDevice->catalog->appVersion = currentVersion;
                batchCatalogs.append(loopDevice);
                qDebug() << "Added catalog to batch:" << loopDevice->name;
            } else {
                qDebug() << "Skipping non-catalog device:" << loopDevice->name;
                delete loopDevice;
                skippedCatalogs += 1;  // Count non-catalogs as skipped
            }
        }
    }

    qDebug() << "Collected" << batchCatalogs.size() << "catalogs for batch update";
    qDebug() << "Skipped" << skippedCatalogs << "non-catalog devices";

    if (batchCatalogs.isEmpty()) {
        qDebug() << "No active catalogs found";
        QMessageBox::information(this, "Katalog", tr("No active catalogs found to update."));
        return;
    }

    // SET BATCH MODE and start processing
    inBatchMode = true;
    batchCurrentIndex = 0;

    qDebug() << "Starting batch mode with" << batchCatalogs.size() << "catalogs";
    qDebug() << "Disabling UpdateAllActive button during batch";

    // Disable the button during batch operation
    ui->Catalogs_pushButton_UpdateAllActive->setEnabled(false);

    // Start processing the first catalog
    startCurrentBatchCatalog();
}
//--------------------------------------------------------------------------
