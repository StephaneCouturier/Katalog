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
// File Name:   mainwindow_tab_create.cpp
// Purpose:     methods for the screen CREATE
// Description: https://stephanecouturier.github.io/Katalog/docs/Features/Create
// Author:      Stephane Couturier
/////////////////////////////////////////////////////////////////////////////
*/

#include "mainwindow.h"
#include "mainwindow_ui_wrapper_device.h"
#include "src/ui_mainwindow.h"
#include "src/core/catalogprogressmanager.h"

#ifdef Q_OS_UNIX
#include <unistd.h>
#endif

//UI----------------------------------------------------------------------------

    void MainWindow::on_Create_treeView_Explorer_clicked(const QModelIndex &index)
    {//Get the selected folder from the tree to the new Catalog path

        //Get the model/data from the tree
        QFileSystemModel* pathmodel = (QFileSystemModel*)ui->Create_treeView_Explorer->model();
        //get data from the selected file/directory
        QFileInfo fileInfo = pathmodel->fileInfo(index);
        //send the path to the line edit
        ui->Create_lineEdit_NewCatalogPath->setText(fileInfo.filePath());
    }
    //--------------------------------------------------------------------------
    void MainWindow::on_Create_pushButton_PickPath_clicked()
    {//Pick a directory from a dialog window

        //Get current selected path as default path for the dialog window
        QString newSelectedPath = ui->Create_lineEdit_NewCatalogPath->text();

        //Open a dialog for the user to select the directory to be cataloged. Only show directories.
        QString dir = QFileDialog::getExistingDirectory(this, tr("Select the directory to be cataloged in this new catalog"),
                                                        newSelectedPath,
                                                        QFileDialog::ShowDirsOnly
                                                        | QFileDialog::DontResolveSymlinks);

        //Save selected directory, and update input line for the source path
        ui->Create_lineEdit_NewCatalogPath->setText(dir);

        //Select this directory in the treeview.
        loadFileSystem(dir);
    }
    //--------------------------------------------------------------------------
    void MainWindow::on_Create_pushButton_AddDirectoryToExclude_clicked()
    {//Add fodler to the exclusion list
        QString newFolderToExclude = ui->Create_lineEdit_FolderToExclude->text();
        int pathLength = newFolderToExclude.length();
        if (newFolderToExclude !="" and newFolderToExclude !="/" and QVariant(newFolderToExclude.at(pathLength-1)).toString()=="/") {
            newFolderToExclude.remove(pathLength-1,1);
        }

        if(newFolderToExclude!=""){
            //Insert new entry
            QSqlQuery insertQuery(QSqlDatabase::database("defaultConnection"));
            QString insertSQL = QLatin1String(R"(
                                        INSERT INTO parameter (
                                                    parameter_name,
                                                    parameter_type,
                                                    parameter_value2)
                                        VALUES(
                                                    :parameter_name,
                                                    :parameter_type,
                                                    :parameter_value2)
                                )");
            insertQuery.prepare(insertSQL);
            insertQuery.bindValue(":parameter_name", "");
            insertQuery.bindValue(":parameter_type", "exclude_directory");
            insertQuery.bindValue(":parameter_value2", newFolderToExclude);
            insertQuery.exec();

            //Save
            collection->saveParameterTableToFile();

            //Reload to list view
            QSqlQuery queryLoad(QSqlDatabase::database("defaultConnection"));
            QString queryLoadSQL = QLatin1String(R"(
                                        SELECT DISTINCT parameter_value2
                                        FROM parameter
                                        WHERE parameter_type ='exclude_directory'
                                        ORDER BY parameter_value2
                                )");
            if (!queryLoad.exec(queryLoadSQL)) {
                qDebug() << "Failed to execute query";
                return;
            }

            QSqlQueryModel *model = new QSqlQueryModel(this);
            model->setQuery(std::move(queryLoad));

            QSortFilterProxyModel *proxyModel = new QSortFilterProxyModel(this);
            proxyModel->setSourceModel(model);
            proxyModel->setDynamicSortFilter(true);
            ui->Create_treeView_Excluded->setModel(proxyModel);
        }
    }
    //--------------------------------------------------------------------------
    void MainWindow::on_Create_pushButton_AddStorage_clicked()
    {
        //Change tab to show the screen to add a storage
        ui->tabWidget->setCurrentIndex(1); // tab 1 is the Devices tab
        ui->Devices_radioButton_DeviceTree->setChecked(true); // the tree view is required to add a storage
        loadDevicesView(""); // refresh the view
    }
    //--------------------------------------------------------------------------
    void MainWindow::on_Create_pushButton_GenerateFromPath_clicked()
    {//Generate the Catalog name from the path

        QString newCatalogName = ui->Create_lineEdit_NewCatalogPath->text();
        newCatalogName.replace("/","_");
        newCatalogName.replace(":","_");
        ui->Create_lineEdit_NewCatalogName->setText(newCatalogName);
    }
    //--------------------------------------------------------------------------
    void MainWindow::on_Create_pushButton_CreateCatalog_clicked()
    {
        createCatalog();
    }
    //--------------------------------------------------------------------------
    void MainWindow::on_Create_treeView_Excluded_customContextMenuRequested(const QPoint &pos)
    {
        //Get selection data
        QModelIndex index=ui->Create_treeView_Excluded->currentIndex();
        QString selectedDirectory = ui->Create_treeView_Excluded->model()->index(index.row(), 0, index.parent() ).data().toString();

        //Set actions
        QPoint globalPos = ui->Create_treeView_Excluded->mapToGlobal(pos);
        QMenu excludeContextMenu;

        QAction *menuDeviceAction1 = new QAction(QIcon::fromTheme("edit-delete"), tr("Remove this directory"), this);
        excludeContextMenu.addAction(menuDeviceAction1);
        connect(menuDeviceAction1, &QAction::triggered, this, [ selectedDirectory, this]() {
            //Delete
            QSqlQuery query(QSqlDatabase::database("defaultConnection"));
            QString querySQL = QLatin1String(R"(
                                    DELETE FROM parameter
                                    WHERE parameter_type ='exclude_directory'
                                    AND parameter_value2=:parameter_value2
                                )");
            query.prepare(querySQL);
            query.bindValue(":parameter_value2", selectedDirectory);
            query.exec();
            collection->saveParameterTableToFile();

            //Reload
            QSqlQuery queryLoad(QSqlDatabase::database("defaultConnection"));
            QString queryLoadSQL = QLatin1String(R"(
                                        SELECT DISTINCT parameter_value2
                                        FROM parameter
                                        WHERE parameter_type ='exclude_directory'
                                        ORDER BY parameter_value2
                                )");
            if (!queryLoad.exec(queryLoadSQL)) {
                qDebug() << "Failed to execute query";
                return;
            }

            QSqlQueryModel *model = new QSqlQueryModel(this);
            model->setQuery(std::move(queryLoad));

            QSortFilterProxyModel *proxyModel = new QSortFilterProxyModel(this);
            proxyModel->setSourceModel(model);
            proxyModel->setDynamicSortFilter(true);
            ui->Create_treeView_Excluded->setModel(proxyModel);
        });

        excludeContextMenu.exec(globalPos);
    }
    //--------------------------------------------------------------------------
    void MainWindow::on_Create_pushButton_Stop_clicked()
    {
        qDebug() << "=== Create Stop button clicked ===";

        // Stop DeviceUpdateManager (not catalogManager)
        if (deviceUpdateManager && deviceUpdateManager->operationRunning()) {
            qDebug() << "Stopping catalog creation via DeviceUpdateManager";
            deviceUpdateManager->requestHardStop();

            // Immediate UI feedback
            ui->Create_pushButton_Stop->setEnabled(false);
        } else {
            qDebug() << "No active operation - just reset UI";
            restoreCreateCatalogUIState();
        }
    }

//Methods-----------------------------------------------------------------------
    void MainWindow::loadFileSystem(QString newCatalogPath)
    {//Load file system to the Create and the Filter for connected devices treeviews

        //Create a new model, only directories, and set root path
        fileSystemModel = new QFileSystemModel(this);
        fileSystemModel->setFilter(QDir::NoDotAndDotDot | QDir::AllDirs);
        fileSystemModel->setRootPath(newCatalogPath);
        fileSystemModel->sort(0,Qt::AscendingOrder);

        //Load File System to the Create and the Filter treeviews
            //Load File System to the Filter connected device treeview
            ui->Create_treeView_Explorer->setModel(fileSystemModel);
            // Only show the tree, hidding other columns and the header row.
            ui->Create_treeView_Explorer->setColumnWidth(0,250);
            ui->Create_treeView_Explorer->setColumnHidden(1,true);
            ui->Create_treeView_Explorer->setColumnHidden(2,true);
            ui->Create_treeView_Explorer->setColumnHidden(3,true);
            ui->Create_treeView_Explorer->setHeaderHidden(true);
            ui->Create_treeView_Explorer->expandToDepth(10);

            //Load File System to the Filter tab treeview
            ui->Filters_treeView_Directory->setModel(fileSystemModel);
            // Only show the tree, hidding other columns and the header row.
            ui->Filters_treeView_Directory->setColumnWidth(0,250);
            ui->Filters_treeView_Directory->setColumnHidden(1,true);
            ui->Filters_treeView_Directory->setColumnHidden(2,true);
            ui->Filters_treeView_Directory->setColumnHidden(3,true);
            ui->Filters_treeView_Directory->setHeaderHidden(true);
            ui->Filters_treeView_Directory->expandToDepth(1);
    }
    //--------------------------------------------------------------------------
    void MainWindow::setupCatalogManager()
    {
        qDebug() << "Setting up new catalog manager system...";

        // Create the new catalog manager
        catalogManager = new CatalogManager(this);

        // Create catalog progress manager with timer reference
        catalogProgressManager = new CatalogProgressManager(statusBar(), statusBarTimer, this);
        catalogProgressManager->connectToCatalogManager(catalogManager);

        // Connect signals for main operations
        connect(catalogManager, &CatalogManager::catalogOperationCompleted, this, [this]() {
            if (currentUpdateDevice) {
                qDebug() << "Catalog update completed for:" << currentUpdateDevice->name;

                if (!catalogManager->inBatchMode()) {
                    ui->Catalogs_pushButton_Stop->setEnabled(false);
                }

                // Only handle batch updates through old system
                // Single updates are now handled by DeviceUpdateManager
                bool isBatchUpdate = catalogManager->inBatchMode();

                if (isBatchUpdate) {
                    qDebug() << "Batch catalog update - report handled by batchNeedsUIReport signal";
                    // Batch updates handled by existing batch reporting system
                } else {
                    qDebug() << "Single catalog update completed - handled by DeviceUpdateManager (skip old system report)";
                    // REMOVED: reportAllUpdates call for single updates
                    // DeviceUpdateManager now handles all single update reporting with combined catalog+storage data

                    // Only do minimal cleanup if this was an old-style direct operation
                    // (DeviceUpdateManager operations handle their own cleanup)
                    currentUpdateDevice = nullptr;
                }
            }
        });

        // Update error handler
        connect(catalogManager, &CatalogManager::catalogOperationError, this, [this](const QString& error) {
            if (currentUpdateDevice) {
                qDebug() << "Catalog update error:" << error;

                // NEW: Use CatalogManager to detect batch mode instead of old variables
                bool isBatchUpdate = catalogManager->inBatchMode();

                if (!isBatchUpdate) {
                    // Single update - show error to user
                    QMessageBox::warning(this, "Katalog", QString("Catalog update failed: %1").arg(error));

                    // Single update cleanup
                    currentUpdateDevice = nullptr;
                    QApplication::restoreOverrideCursor();
                    ui->Catalogs_pushButton_UpdateActiveDevice->setEnabled(true);
                } else {
                    // Batch update - log error but let CatalogManager handle progression
                    qDebug() << "Batch update error for" << currentUpdateDevice->name << ":" << error;

                    // Clean up current device
                    currentUpdateDevice = nullptr;

                    // NEW: Let CatalogManager handle batch progression automatically
                    // The batch system will detect the error and continue with next catalog
                    // via the existing onCatalogUpdateCompleted mechanism
                }
            }
            // Creation errors are handled in the existing onCatalogOperationCompleted()
        });

        // Update cancellation handler - handle both creation and updates
        connect(catalogManager, &CatalogManager::catalogOperationCancelled, this, [this]() {
            if (currentUpdateDevice) {
                // UPDATE cancellation
                qDebug() << "Catalog update cancelled";

                // NEW: Use CatalogManager to detect batch mode instead of old variables
                bool isBatchUpdate = catalogManager->inBatchMode();

                // Clean up current operation
                currentUpdateDevice = nullptr;

                if (isBatchUpdate) {
                    // NEW: Batch cancellation is handled by CatalogManager automatically
                    // The batchOperationCompleted signal will fire and restore UI
                    qDebug() << "Batch operation was cancelled - CatalogManager will handle cleanup";
                } else {
                    // Single update cleanup
                    QApplication::restoreOverrideCursor();
                    ui->Catalogs_pushButton_UpdateActiveDevice->setEnabled(true);
                }
                ui->Catalogs_pushButton_Stop->setEnabled(false);
            } else {
                // Fallback - just restore basic UI state
                qDebug() << "Unknown operation cancelled - restoring basic UI";
                QApplication::restoreOverrideCursor();
            }
        });

        // Connect batch operation signals
        connect(catalogManager, &CatalogManager::batchOperationCompleted, this, [this]() {
            qDebug() << "Batch operation completed - restoring UI";

            // Reset MainWindow state
            currentUpdateDevice = nullptr;

            // Re-enable the UpdateAllActive button
            ui->Catalogs_pushButton_Stop->setEnabled(false);
            ui->Catalogs_pushButton_UpdateAllActive->setEnabled(true);

            // Refresh the device view
            loadDevicesView("");
        });

        connect(catalogManager, &CatalogManager::batchCatalogStarted, this, [this](Device* device, int currentIndex, int totalCount) {
            qDebug() << "Batch catalog started:" << device->name << "(" << currentIndex << "/" << totalCount << ")";

            // Set current device for MainWindow tracking
            currentUpdateDevice = device;

            // Update progress manager with the current catalog engine if available
            if (catalogProgressManager) {
                CatalogJobStoppable* currentEngine = catalogManager->getCurrentCatalogEngine();
                if (currentEngine) {
                    catalogProgressManager->setCurrentCatalogEngine(currentEngine);
                    qDebug() << "Updated progress manager with current engine";
                }
            }
        });

        connect(catalogManager, &CatalogManager::batchNeedsUIReport, this, [this](Device* device, const QList<qint64>& results, const QString& updateType) {
            qDebug() << "Batch needs UI report for device:" << (device ? device->name : "GLOBAL REPORT");
            qDebug() << "showEachCatalogUpdateSummary:" << showEachCatalogUpdateSummary;
            qDebug() << "Results data:" << results;

            if (device == nullptr) {
                // Final report, always show regardless of user choice
                qDebug() << "Showing final global report";
                Device dummyDevice;
                dummyDevice.name = tr("Update all active catalogs");
                dummyDevice.type = "BatchSummary";
                reportAllUpdates(&dummyDevice, results, updateType);
            } else if (showEachCatalogUpdateSummary) {
                // Individual report, only if user requested
                qDebug() << "Showing individual catalog report for:" << device->name;
                reportAllUpdates(device, results, updateType);
            } else {
                qDebug() << "User chose NO individual reports - skipping individual report for:" << device->name;
            }
        });
        qDebug() << "New catalog manager system setup complete";
    }
    //--------------------------------------------------------------------------
    void MainWindow::createCatalog()
    {
        qDebug() << "=== MainWindow::createCatalog() ENTRY ===";

        // Validation 1: Catalog name
        if (ui->Create_lineEdit_NewCatalogName->text() == "") {
            QMessageBox msgBox;
            msgBox.setWindowTitle("Katalog");
            msgBox.setText(tr("Provide a name for this new catalog.<br/>"));
            msgBox.setIcon(QMessageBox::Warning);
            msgBox.exec();
            return;  // No UI state change needed - validation failed before operation started
        }

        // Validation 2: Catalog path
        if (ui->Create_lineEdit_NewCatalogPath->text() == "") {
            QMessageBox msgBox;
            msgBox.setWindowTitle("Katalog");
            msgBox.setText(tr("Provide a path for this new catalog.<br/>"));
            msgBox.setIcon(QMessageBox::Warning);
            msgBox.exec();
            return;  // No UI state change needed
        }

        // Validation 3: Storage selection
        if (ui->Create_comboBox_StorageSelection->currentText() == "") {
            QMessageBox msgBox;
            msgBox.setWindowTitle("Katalog");
            msgBox.setText(tr("Select a Storage for this new catalog.<br/>(Selection panel on the left and dropdown list)"));
            msgBox.setIcon(QMessageBox::Warning);
            msgBox.exec();
            return;  // No UI state change needed
        }

        // Check if directory exists and is not empty
        QDir sourceDir(ui->Create_lineEdit_NewCatalogPath->text());
        if (!sourceDir.exists()) {
            QMessageBox::warning(this, "Katalog", tr("The source directory does not exist."));
            return;  // No UI state change needed
        }

        if (sourceDir.entryInfoList(QDir::NoDotAndDotDot|QDir::AllEntries).count() == 0) {
            QMessageBox::StandardButton reply = QMessageBox::question(this, "Katalog",
                                                                      tr("The source folder does not contain any file.<br/>This could mean that the source is empty or the device is not mounted to this folder.<br/>Do you want to save it anyway (the catalog would be empty)?"),
                                                                      QMessageBox::Yes | QMessageBox::No);
            if (reply == QMessageBox::No) {
                return;  // No UI state change needed
            }
        }

        //Create a new device and catalog (SAME AS ORIGINAL - Device-centric approach)
        Device *newCatalogDevice = new Device();
        newCatalogDevice->generateDeviceID();
        newCatalogDevice->type = "Catalog";
        newCatalogDevice->name = ui->Create_lineEdit_NewCatalogName->text();

        qDebug() << "Creating new Device - ID:" << newCatalogDevice->ID << "Name:" << newCatalogDevice->name;

        //Check if the catalog name (so the csv file name) already exists
        if (newCatalogDevice->verifyDeviceNameExists()) {
            QMessageBox msgBox;
            msgBox.setWindowTitle("Katalog");
            msgBox.setText(tr("There is already a catalog with this name:<br/><b>")
                           + newCatalogDevice->name
                           + "</b><br/><br/>"+tr("Choose a different name and try again."));
            msgBox.setIcon(QMessageBox::Warning);
            msgBox.exec();
            delete newCatalogDevice;
            return;  // No UI state change needed
        }

        //Continue populating values and add device
        newCatalogDevice->parentID = ui->Create_comboBox_StorageSelection->currentData().toInt();
        newCatalogDevice->catalog->generateID();
        newCatalogDevice->externalID = newCatalogDevice->catalog->ID;
        newCatalogDevice->groupID = 0;
        newCatalogDevice->path = ui->Create_lineEdit_NewCatalogPath->text();
        newCatalogDevice->insertDevice();
        qDebug() << "Device inserted - Parent ID:" << newCatalogDevice->parentID << "Path:" << newCatalogDevice->path;

        //Get inputs and set values of the newCatalog
        newCatalogDevice->catalog->name = newCatalogDevice->name;  // Make sure catalog name matches device name
        newCatalogDevice->catalog->filePath = collection->folder + "/" + newCatalogDevice->name + ".idx";
        newCatalogDevice->catalog->sourcePath = ui->Create_lineEdit_NewCatalogPath->text();
        newCatalogDevice->catalog->includeHidden = ui->Create_checkBox_IncludeHidden->isChecked();
        newCatalogDevice->catalog->storageName = ui->Create_comboBox_StorageSelection->currentText();
        newCatalogDevice->catalog->includeSymblinks = ui->Create_checkBox_IncludeSymblinks->isChecked();
        newCatalogDevice->catalog->isFullDevice = ui->Create_checkBox_isFullDevice->isChecked();
        newCatalogDevice->catalog->includeMetadata = ui->Create_comboBox_MetadataOption->itemData(ui->Create_comboBox_MetadataOption->currentIndex(), Qt::UserRole).toString();
        newCatalogDevice->catalog->appVersion = currentVersion;

        // Set file type from UI radio buttons
        QString fileType = "All";  // Default value
        if (ui->Create_radioButton_FileType_Audio->isChecked()) {
            fileType = "Audio";
        } else if (ui->Create_radioButton_FileType_Image->isChecked()) {
            fileType = "Image";
        } else if (ui->Create_radioButton_FileType_Text->isChecked()) {
            fileType = "Text";
        } else if (ui->Create_radioButton_FileType_Video->isChecked()) {
            fileType = "Video";
        } else if (ui->Create_radioButton_FileType_Any->isChecked()) {
            fileType = "All";
        }
        newCatalogDevice->catalog->fileType = fileType;
        qDebug() << "Catalog configured - File type:" << newCatalogDevice->catalog->fileType;

        //Save new catalog
        newCatalogDevice->catalog->insertCatalog();

        //Add path to parent Storage device if empty
        Device parentStorageDevice;
        parentStorageDevice.ID = newCatalogDevice->parentID;
        parentStorageDevice.loadDevice("defaultConnection");
        if(parentStorageDevice.path == ""){
            parentStorageDevice.path = newCatalogDevice->path;
            parentStorageDevice.saveDevice();
            collection->saveStorageTableToFile();
        }

        //Reload
        loadDevicesView("");
        loadStorageList();

        if (!deviceUpdateManager) {
            qDebug() << "DeviceUpdateManager not available - setting up now";
            setupDeviceUpdateManager();
        }

        // Check if already running
        if (deviceUpdateManager->operationRunning()) {
            QMessageBox::information(this, "Katalog", tr("A device operation is already running."));
            currentUpdateDevice = nullptr;
            return;  // No UI state change needed - operation wasn't started
        }

        // ALL VALIDATIONS PASSED - Now set UI to running state and start operation
        qDebug() << "All validations passed - starting catalog creation";

        // Store device reference
        currentUpdateDevice = newCatalogDevice;
        qDebug() << "*** STORED currentUpdateDevice for creation:" << currentUpdateDevice->name;

        // SURGICAL FIX: Set Create UI to running state AFTER validations pass
        setCreateCatalogUIState(true);  // Only affects Create tab

        // Start the DeviceUpdateManager operation
        deviceUpdateManager->updateDeviceHierarchy(newCatalogDevice,
                                                   collection->databaseMode,
                                                   collection->folder,
                                                   "create");

        qDebug() << "=== MainWindow::createCatalog() EXIT ===";
    }
    //--------------------------------------------------------------------------
    void MainWindow::restoreCreateCatalogUIState()
    {
        // Restore UI to idle state
        QApplication::restoreOverrideCursor();
        ui->Create_pushButton_CreateCatalog->setEnabled(true);
        ui->Create_pushButton_Stop->setEnabled(false);
        qDebug() << "Create catalog UI state restored to idle";
    }
    //--------------------------------------------------------------------------
    void MainWindow::setCreateCatalogUIState(bool isRunning)
    {
        qDebug() << "setCreateCatalogUIState:" << isRunning;

        if (isRunning) {
            // CREATION RUNNING: Disable Create tab buttons, set cursor
            ui->Create_pushButton_CreateCatalog->setEnabled(false);
            ui->Create_pushButton_Stop->setEnabled(true);
            QApplication::setOverrideCursor(Qt::WaitCursor);
        } else {
            // CREATION FINISHED: Restore Create tab buttons, restore cursor
            ui->Create_pushButton_CreateCatalog->setEnabled(true);
            ui->Create_pushButton_Stop->setEnabled(false);
            QApplication::restoreOverrideCursor();
            statusBar()->clearMessage();
        }
    }
    //--------------------------------------------------------------------------
    void MainWindow::cleanupStoppedCatalogCreation()
    {
        qDebug() << "=== cleanupFailedCatalogCreation() START ===";

        // Use the device reference we stored when starting the operation
        if (currentUpdateDevice) {
            qDebug() << "Cleaning up failed catalog creation for device:" << currentUpdateDevice->name;

            // Use the existing backend method to delete the device (no UI confirmation)
            bool success = DeviceUIWrapper::deleteDeviceWithUI(currentUpdateDevice, false);

            if (success) {
                qDebug() << "Device deleted successfully";
            } else {
                qDebug() << "Failed to delete device";
            }


        } else {
            qDebug() << "No device to cleanup";
        }

        // Refresh UI
        loadDevicesView("");
        loadStorageList();

        // Restore UI state
        QApplication::restoreOverrideCursor();
        ui->Create_pushButton_CreateCatalog->setEnabled(true);
        ui->Create_pushButton_Stop->setEnabled(false);

        qDebug() << "=== cleanupFailedCatalogCreation() COMPLETE ===";
    }

    void MainWindow::initiateMetadataFields()
    {
        // Initialize Create combobox
        ui->Create_comboBox_MetadataOption->addItem(tr("None"));
        ui->Create_comboBox_MetadataOption->addItem(tr("MIME Type Only"));
        ui->Create_comboBox_MetadataOption->addItem(tr("Media Basic"));
        ui->Create_comboBox_MetadataOption->addItem(tr("Extended Custom"));
        ui->Create_comboBox_MetadataOption->addItem(tr("Extended Full"));

        ui->Create_comboBox_MetadataOption->setItemData(0, Catalog::METADATA_NONE, Qt::UserRole);
        ui->Create_comboBox_MetadataOption->setItemData(1, Catalog::METADATA_MIME_ONLY, Qt::UserRole);
        ui->Create_comboBox_MetadataOption->setItemData(2, Catalog::METADATA_MEDIA_BASIC, Qt::UserRole);
        ui->Create_comboBox_MetadataOption->setItemData(3, Catalog::METADATA_EXTENDED_CUSTOM, Qt::UserRole);
        ui->Create_comboBox_MetadataOption->setItemData(4, Catalog::METADATA_EXTENDED_FULL, Qt::UserRole);

        // Initialize Catalogs combobox
        ui->Catalogs_comboBox_MetaDataOption->addItem(tr("None"));
        ui->Catalogs_comboBox_MetaDataOption->addItem(tr("MIME Type Only"));
        ui->Catalogs_comboBox_MetaDataOption->addItem(tr("Media Basic"));
        ui->Catalogs_comboBox_MetaDataOption->addItem(tr("Extended Custom"));
        ui->Catalogs_comboBox_MetaDataOption->addItem(tr("Extended Full"));

        ui->Catalogs_comboBox_MetaDataOption->setItemData(0, Catalog::METADATA_NONE, Qt::UserRole);
        ui->Catalogs_comboBox_MetaDataOption->setItemData(1, Catalog::METADATA_MIME_ONLY, Qt::UserRole);
        ui->Catalogs_comboBox_MetaDataOption->setItemData(2, Catalog::METADATA_MEDIA_BASIC, Qt::UserRole);
        ui->Catalogs_comboBox_MetaDataOption->setItemData(3, Catalog::METADATA_EXTENDED_CUSTOM, Qt::UserRole);
        ui->Catalogs_comboBox_MetaDataOption->setItemData(4, Catalog::METADATA_EXTENDED_FULL, Qt::UserRole);

        // Create a model to hide specific items
        QStandardItemModel* createModel = qobject_cast<QStandardItemModel*>(ui->Create_comboBox_MetadataOption->model());
        QStandardItemModel* catalogModel = qobject_cast<QStandardItemModel*>(ui->Catalogs_comboBox_MetaDataOption->model());

        if (createModel) {
            // Hide "MIME Type Only" (index 1) and "Extended Custom" (index 3)
            QStandardItem* mimeItem = createModel->item(1);
            QStandardItem* customItem = createModel->item(3);
            if (mimeItem) mimeItem->setFlags(mimeItem->flags() & ~Qt::ItemIsEnabled);
            if (customItem) customItem->setFlags(customItem->flags() & ~Qt::ItemIsEnabled);
        }

        if (catalogModel) {
            // Hide "MIME Type Only" (index 1) and "Extended Custom" (index 3)
            QStandardItem* mimeItem = catalogModel->item(1);
            QStandardItem* customItem = catalogModel->item(3);
            if (mimeItem) mimeItem->setFlags(mimeItem->flags() & ~Qt::ItemIsEnabled);
            if (customItem) customItem->setFlags(customItem->flags() & ~Qt::ItemIsEnabled);
        }

        // Set default to None (index 0) to match your METADATA_NONE default
        ui->Create_comboBox_MetadataOption->setCurrentIndex(0);
        ui->Catalogs_comboBox_MetaDataOption->setCurrentIndex(0);
    }
    //--------------------------------------------------------------------------
