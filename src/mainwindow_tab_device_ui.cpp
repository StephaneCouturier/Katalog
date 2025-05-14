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
    settings.setValue("Devices/DisplayFullDeviceTable", arg1);
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
            collection->saveStatiticsTableToFile();
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
            collection->saveStatiticsTableToFile();
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
            collection->saveStatiticsTableToFile();
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
    collection->saveStatiticsTableToFile();
    loadDevicesView();
    editDevice();
}
//--------------------------------------------------------------------------
void MainWindow::on_Devices_pushButton_ApplyToSelection_clicked()
{
    loadDevicesTreeToModel("Filters");
}
//--------------------------------------------------------------------------
