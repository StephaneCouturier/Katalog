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
#include "core/device.h"
#include "core/filechecksum.h"
#include "core/statusbarmessagebuilder.h"
#include "mainwindow_ui_wrapper_device.h"

//TAB: DEVICES -------------------------------------------------------------
//--- UI -------------------------------------------------------------------
//--------------------------------------------------------------------------
void MainWindow::on_Devices_radioButton_DeviceTree_clicked()
{
    QSettings settings(collection->settingsFilePath, QSettings:: IniFormat);
    settings.setValue("Devices/DisplayContents", "Tree");

    ui->Catalogs_pushButton_UpdateAllActive->setEnabled(false);

    loadDevicesView("");
}
//--------------------------------------------------------------------------
void MainWindow::on_Devices_radioButton_StorageList_clicked()
{
    QSettings settings(collection->settingsFilePath, QSettings:: IniFormat);
    settings.setValue("Devices/DisplayContents", "Storage");

    ui->Catalogs_pushButton_UpdateAllActive->setEnabled(false);

    loadDevicesView("");
}
//--------------------------------------------------------------------------
void MainWindow::on_Devices_radioButton_CatalogList_clicked()
{
    QSettings settings(collection->settingsFilePath, QSettings:: IniFormat);
    settings.setValue("Devices/DisplayContents", "Catalogs");

    ui->Catalogs_pushButton_UpdateAllActive->setEnabled(true);

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
void MainWindow::on_Devices_pushButton_TreeExpand_clicked()
{
    changeTreeExpandLevel(1,
                          ui->Devices_treeView_DeviceList,
                          deviceTreeExpandState,
                          "Devices/deviceTreeExpandState",
                          ui->Devices_pushButton_TreeCollapse,
                          ui->Devices_pushButton_TreeExpand,
                          false);
}
void MainWindow::on_Devices_pushButton_TreeCollapse_clicked()
{
    changeTreeExpandLevel(-1,
                          ui->Devices_treeView_DeviceList,
                          deviceTreeExpandState,
                          "Devices/deviceTreeExpandState",
                          ui->Devices_pushButton_TreeCollapse,
                          ui->Devices_pushButton_TreeExpand,
                          false);
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
    activeDevice->loadDevice(m_connectionName);

    if((activeDevice->type =="Catalog" && activeDevice->active) || activeDevice->type == "Storage" || activeDevice->type == "Virtual")
        ui->Catalogs_pushButton_UpdateActiveDevice->setEnabled(true);
    else
        ui->Catalogs_pushButton_UpdateActiveDevice->setEnabled(false);

    if((activeDevice->type =="Catalog" && activeDevice->active))
        ui->Catalogs_pushButton_VerifyMIMETypes->setEnabled(true);
    else
        ui->Catalogs_pushButton_VerifyMIMETypes->setEnabled(false);
}
//--------------------------------------------------------------------------
void MainWindow::on_Devices_treeView_DeviceList_customContextMenuRequested(const QPoint &pos)
{
    //Get selection data
    QModelIndex index=ui->Devices_treeView_DeviceList->currentIndex();
    activeDevice->ID   = ui->Devices_treeView_DeviceList->model()->index(index.row(), 3, index.parent() ).data().toInt();
    activeDevice->loadDevice(m_connectionName);

    Device *tempParentDevice = new Device();
    tempParentDevice->ID = activeDevice->parentID;
    tempParentDevice->loadDevice(m_connectionName);

    //Set actions for catalogs
    if(activeDevice->type=="Catalog"){
        QPoint globalPos = ui->Devices_treeView_DeviceList->mapToGlobal(pos);
        QMenu deviceContextMenu;

        QString deviceName = activeDevice->name;

        if(activeDevice->active){
            QAction *menuDeviceAction1 = new QAction(QIcon::fromTheme("media-playlist-repeat"), tr("Update"), this);
            deviceContextMenu.addAction(menuDeviceAction1);
            connect(menuDeviceAction1, &QAction::triggered, this, [this, deviceName]() {
                qDebug() << "=== Filters Context Menu Update (DeviceUpdateManager) ===";

                // Ensure DeviceUpdateManager is set up (like in other tabs)
                if (!deviceUpdateManager) {
                    qDebug() << "DeviceUpdateManager not available - setting up now";
                    setupDeviceUpdateManager();
                }

                // Check if already running
                if (deviceUpdateManager->operationRunning()) {
                    QMessageBox::information(this, "Katalog", tr("A device operation is already running."));
                    return;
                }

                qDebug() << "Filters context menu catalog update for:" << activeDevice->name;

                // Set UI state for catalog operation
                setCatalogUpdateUIState(true);

                // Use DeviceUpdateManager
                deviceUpdateManager->updateDeviceHierarchy(activeDevice,
                                                           collection->databaseMode,
                                                           collection->folder);

                qDebug() << "Filters context menu - DeviceUpdateManager operation started";
            });
        }

        QAction *menuDeviceAction5 = new QAction(QIcon::fromTheme("view-list-tree"), tr("Explore"), this);
        deviceContextMenu.addAction(menuDeviceAction5);
        connect(menuDeviceAction5, &QAction::triggered, this, [this, deviceName]() {
            exploreDevice->ID = activeDevice->ID;
            exploreDevice->loadDevice(m_connectionName);

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

        if (!activeDevice->path.isEmpty() && activeDevice->path != "EXPORT") {
            QAction *menuOpenPath = new QAction(QIcon::fromTheme("document-open-folder"), tr("Open folder"), this);
            deviceContextMenu.addAction(menuOpenPath);
            connect(menuOpenPath, &QAction::triggered, this, [this]() {
                QDesktopServices::openUrl(QUrl::fromLocalFile(activeDevice->path));
            });
        }

        // Add separator before checksum verification
        deviceContextMenu.addSeparator();

        QAction *menuVerifyChecksums = new QAction(QIcon::fromTheme("document-properties"),
                                                   tr("Verify Checksums"), this);
        deviceContextMenu.addAction(menuVerifyChecksums);
        connect(menuVerifyChecksums, &QAction::triggered, this, [this]() {
            verifyCatalogChecksums();
        });

        deviceContextMenu.addSeparator();

        // Filelight
        if(activeDevice->active==true){
            QAction *menuDeviceAction4 = new QAction(QIcon::fromTheme("view-statistics"), tr("Filelight"), this);
            deviceContextMenu.addAction(menuDeviceAction4);
            connect(menuDeviceAction4, &QAction::triggered, this, [this, deviceName]() {
                QProcess::startDetached("filelight", QStringList() << activeDevice->path);
            });
        }

        deviceContextMenu.addSeparator();

        // Show "Unassign" only for assigned catalogs (not exports)
        // Show "Delete" for exports and physical group catalogs
        if(activeDevice->groupID != 0 && activeDevice->path != "EXPORT"){
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
            // Use DeviceUpdateManager for ALL device types
            if (!deviceUpdateManager) {
                setupDeviceUpdateManager();
            }

            if (deviceUpdateManager->operationRunning()) {
                QMessageBox::information(this, "Katalog", tr("A device operation is already running."));
                return;
            }

            // Set UI state for operation
            setCatalogUpdateUIState(true);


            // Use unified DeviceUpdateManager for Storage devices
            deviceUpdateManager->updateDeviceHierarchy(
                activeDevice,
                collection->databaseMode,
                collection->folder,
                "update"
                );
        });

        QAction *menuDeviceAction2 = new QAction(QIcon::fromTheme("document-edit-sign"), tr("Edit"), this);
        deviceContextMenu.addAction(menuDeviceAction2);
        connect(menuDeviceAction2, &QAction::triggered, this, [this, deviceName]() {
            editDevice();
        });

        if (!activeDevice->path.isEmpty()) {
            QAction *menuOpenPath = new QAction(QIcon::fromTheme("document-open-folder"), tr("Open folder"), this);
            deviceContextMenu.addAction(menuOpenPath);
            connect(menuOpenPath, &QAction::triggered, this, [this]() {
                QDesktopServices::openUrl(QUrl::fromLocalFile(activeDevice->path));
            });
        }

        if(activeDevice->active==true){
            QAction *menuDeviceAction5 = new QAction(QIcon::fromTheme("view-statistics"), tr("Filelight"), this);
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
            // Use DeviceUpdateManager for ALL device types
            if (!deviceUpdateManager) {
                setupDeviceUpdateManager();
            }

            if (deviceUpdateManager->operationRunning()) {
                QMessageBox::information(this, "Katalog", tr("A device operation is already running."));
                return;
            }

            // Set UI state for operation
            setCatalogUpdateUIState(true);


            // Use unified DeviceUpdateManager for Storage devices
            deviceUpdateManager->updateDeviceHierarchy(
                activeDevice,
                collection->databaseMode,
                collection->folder
                );
        });

        QAction *menuDeviceAction2 = new QAction(QIcon::fromTheme("document-edit-sign"), tr("Edit"), this);
        deviceContextMenu.addAction(menuDeviceAction2);
        connect(menuDeviceAction2, &QAction::triggered, this, [this, deviceName]() {
            editDevice();
        });

        if (!activeDevice->path.isEmpty()) {
            QAction *menuOpenPath = new QAction(QIcon::fromTheme("document-open-folder"), tr("Open folder"), this);
            deviceContextMenu.addAction(menuOpenPath);
            connect(menuOpenPath, &QAction::triggered, this, [this]() {
                QDesktopServices::openUrl(QUrl::fromLocalFile(activeDevice->path));
            });
        }

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
    deviceUpdateManager->updateDeviceHierarchy(activeDevice,
                                               collection->databaseMode,
                                               collection->folder,
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
void MainWindow::on_Catalogs_pushButton_UpdateActiveDevice_clicked()
{
    qDebug() << "=== Update Active Device (DeviceUpdateManager) ===";

    if (!deviceUpdateManager) {
        //QMessageBox::critical(this, "Katalog", tr("Device update manager not available."));
        qDebug() << "ERROR: DeviceUpdateManager not available";
        return;
    }

    if (deviceUpdateManager->operationRunning()) {
        QMessageBox::information(this, "Katalog", tr("A device operation is already running."));
        return;
    }

    // Set UI state for operation
    setCatalogUpdateUIState(true);

    // Use DeviceUpdateManager for both Catalog and Storage devices
    deviceUpdateManager->updateDeviceHierarchy(activeDevice,
                                               collection->databaseMode,
                                               collection->folder,
                                               "update");

    qDebug() << "Device operation started for:" << activeDevice->name << "Type:" << activeDevice->type;
}
//--------------------------------------------------------------------------
void MainWindow::on_Catalogs_pushButton_UpdateAllActive_clicked()
{
    qDebug() << "=== UPDATE ALL ACTIVE CLICKED ===";

    // Ask user for report choice
    QMessageBox msgBox;
    msgBox.setWindowTitle("Katalog");
    msgBox.setText(tr("Do you want a the summary of updates for each catalog?"));
    msgBox.setIcon(QMessageBox::Question);
    msgBox.setStandardButtons(QMessageBox::Yes|QMessageBox::No | QMessageBox::Cancel);
    int result = msgBox.exec();

    if ( result == QMessageBox::Yes){
        showEachCatalogUpdateSummary = true;
    }
    else if ( result == QMessageBox::No){
        showEachCatalogUpdateSummary = false;
    }
    else if ( result == QMessageBox::Cancel){
        return;
    }

    qDebug() << "User chose to show individual reports:" << showEachCatalogUpdateSummary;

    ui->Catalogs_pushButton_Stop->setEnabled(true);

    // Collect all active catalogs into local list
    QList<Device*> collectedCatalogs;
    int totalCatalogs = 0;
    int inactiveCatalogs = 0;
    int skippedDevices = 0;

    qDebug() << "Collecting catalogs from tree view...";

    for (int row = 0; row < ui->Devices_treeView_DeviceList->model()->rowCount(); ++row) {
        // Get device info
        QModelIndex activeIndex = ui->Devices_treeView_DeviceList->model()->index(row, 2);
        QIcon activeIcon = qvariant_cast<QIcon>(ui->Devices_treeView_DeviceList->model()->data(activeIndex, Qt::DecorationRole));

        Device* loopDevice = new Device();
        loopDevice->ID = ui->Devices_treeView_DeviceList->model()->data(ui->Devices_treeView_DeviceList->model()->index(row, 3)).toInt();
        loopDevice->loadDevice(m_connectionName);

        qDebug() << "Found device:" << loopDevice->name << "Type:" << loopDevice->type;

        if (loopDevice->type == "Catalog") {
            totalCatalogs++;  // Count all catalogs

            // Check if active (has "dialog-ok-apply" icon)
            if (activeIcon.name() == QIcon::fromTheme("dialog-ok-apply").name()) {
                // Active catalog - add to processing list
                collectedCatalogs.append(loopDevice);
                qDebug() << "Added active catalog to batch:" << loopDevice->name;
            } else {
                // Inactive catalog - count but don't process
                inactiveCatalogs++;
                qDebug() << "Counted inactive catalog:" << loopDevice->name;
                delete loopDevice;
            }
        } else {
            qDebug() << "Skipping non-catalog device:" << loopDevice->name;
            delete loopDevice;
            skippedDevices++;
        }
    }

    qDebug() << "Total catalogs found:" << totalCatalogs;
    qDebug() << "Active catalogs to process:" << collectedCatalogs.size();
    qDebug() << "Inactive catalogs (baseline skipped):" << inactiveCatalogs;

    if (collectedCatalogs.isEmpty()) {
        qDebug() << "Catalogs_pushButton_UpdateAllActive: No active catalogs found";
        return;
    }

    // Create dummy Virtual device containing the active catalogs
    Device* dummyVirtualDevice = deviceUpdateManager->createDummyDeviceFromList(collectedCatalogs);
    currentUpdateDevice = dummyVirtualDevice;
    setCatalogUpdateUIState(true);

    // Use DeviceUpdateManager
    deviceUpdateManager->updateDeviceHierarchy(
        dummyVirtualDevice,
        collection->databaseMode,
        collection->folder,
        "update"
        );
}
//--------------------------------------------------------------------------
void MainWindow::on_Catalogs_pushButton_Stop_clicked()
{
    qDebug() << "=== Stop Catalog Operation ===";

    if (!deviceUpdateManager) {
        qDebug() << "DeviceUpdateManager not available";
        return;
    }

    if (!deviceUpdateManager->operationRunning()) {
        qDebug() << "No device operation running";
        ui->Catalogs_pushButton_Stop->setEnabled(false);
        return;
    }

    // Check for Ctrl key modifier for gentle stop
    bool useGentleStop = QApplication::keyboardModifiers() & Qt::ControlModifier;

    if (useGentleStop) {
        qDebug() << "Requesting gentle stop (Ctrl+Click detected)";
        deviceUpdateManager->requestGentleStop();
    } else {
        qDebug() << "Requesting hard stop (immediate)";
        deviceUpdateManager->requestHardStop();
    }
}
//--------------------------------------------------------------------------
void MainWindow::on_Catalogs_pushButton_VerifyMIMETypes_clicked()
{
    // MIME verification
    if (!deviceUpdateManager) {
        setupDeviceUpdateManager();
    }

    if (deviceUpdateManager->operationRunning()) {
        QMessageBox::information(this, "Katalog", tr("A device operation is already running."));
        return;
    }

    // Use CatalogJobStoppable directly for MIME verification
    CatalogJobStoppable* catalogJob = new CatalogJobStoppable(this);
    catalogJob->configureOperation(selectedDevice,
                                   CatalogJobStoppable::VerifyMimeTypes,
                                   collection->databaseMode,
                                   collection->folder);

    // Connect the specific MIME verification completion signal
    connect(catalogJob, &CatalogJobStoppable::mimeVerificationCompleted,
            this, [this, catalogJob](int mismatchCount, const QString& reportPath) {
                catalogJob->deleteLater();
                setCatalogUpdateUIState(false);

                // Create appropriate message based on ACTUAL results from THIS run
                QString title = tr("MIME Verification Complete");
                QString message;
                QMessageBox::StandardButtons buttons = QMessageBox::Ok;

                if (mismatchCount == 0) {
                    message = tr("MIME verification completed successfully.<br/>"
                                 "No mismatches found between file extensions and actual content.");
                } else {
                    message = tr("MIME verification completed.<br/>"
                                 "%1 mismatch(es) found between file extensions and actual content.<br/><br/>"
                                 "Report saved to:<br/>"
                                 "%2")
                    .arg(mismatchCount)
                        .arg(reportPath);
                    buttons = QMessageBox::Ok | QMessageBox::Open;
                }

                QMessageBox msgBox;
                msgBox.setWindowTitle(title);
                msgBox.setText(message);
                msgBox.setIcon(mismatchCount == 0 ? QMessageBox::Information : QMessageBox::Warning);
                msgBox.setStandardButtons(buttons);

                if (mismatchCount > 0) {
                    msgBox.button(QMessageBox::Open)->setText(tr("Open Report"));
                }

                int result = msgBox.exec();

                // Handle opening the report file
                if (result == QMessageBox::Open && !reportPath.isEmpty()) {
                    QDesktopServices::openUrl(QUrl::fromLocalFile(reportPath));
                }
            });

    // Connect error signal
    connect(catalogJob, &CatalogJobStoppable::catalogOperationError,
            this, [this, catalogJob](const QString& error) {
                QMessageBox::warning(this, "Katalog", tr("MIME verification failed:<br/>"
                                                 "%1").arg(error));
                catalogJob->deleteLater();
                setCatalogUpdateUIState(false);
            });

    setCatalogUpdateUIState(true);
    catalogJob->processCatalog();
}
//--------------------------------------------------------------------------
