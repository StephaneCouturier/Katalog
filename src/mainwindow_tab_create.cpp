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
        //newDevice->catalog->setSourcePath(ui->Create_lineEdit_NewCatalogPath->text());

        //Open a dialog for the user to select the directory to be cataloged. Only show directories.
        QString dir = QFileDialog::getExistingDirectory(this, tr("Select the directory to be cataloged in this new catalog"),
                                                        newSelectedPath,
                                                        QFileDialog::ShowDirsOnly
                                                        | QFileDialog::DontResolveSymlinks);
        //Save selected directory, and update input line for the source path
        //newDevice->catalog->setSourcePath(dir);
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
        qDebug() << "Before stop - CatalogManager exists:" << (catalogManager != nullptr);

        if (catalogManager) {
            qDebug() << "Before stop - Catalog operation running:" << catalogManager->catalogOperationRunning();
        }

        // Stop any running catalog operation using the new interface
        if (catalogManager && catalogManager->catalogOperationRunning()) {
            qDebug() << "Stopping active catalog operation via new CatalogManager";
            catalogManager->stopCatalogOperation();  // NEW: Use the new method name

            // IMMEDIATE UI FEEDBACK: Just disable the stop button to show it was clicked
            ui->Create_pushButton_Stop->setEnabled(false);

            // DON'T restore full UI state here - let the cancelled signal handler do it
            // The new system will emit catalogOperationCancelled which restores UI state

        } else {
            qDebug() << "No active catalog operation, but user clicked Stop - just reset buttons";
            restoreCreateCatalogUIState(); // Only restore if no operation was running
        }

        // Clear device reference if stopping
        currentCatalogDevice = nullptr;

        qDebug() << "=== Create Stop button clicked complete ===";
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
        catalogProgressManager->setCatalogManager(catalogManager);

        // Connect signals for main operations
        connect(catalogManager, &CatalogManager::catalogOperationCompleted, this, [this]() {
            if (currentCatalogDevice) {
                // This is a creation operation
                onCatalogOperationCompleted();
            } else if (currentUpdateDevice) {
                // Single update operation - handle UI cleanup directly
                qDebug() << "Single catalog update completed - restoring UI";
                currentUpdateDevice = nullptr;
                QApplication::restoreOverrideCursor();
                ui->Catalogs_pushButton_UpdateCatalog->setEnabled(true);
                loadDevicesView("");
            } else {
                qDebug() << "Catalog operation completed but no device reference found";
                // Fallback - just restore UI
                QApplication::restoreOverrideCursor();
                ui->Catalogs_pushButton_UpdateCatalog->setEnabled(true);
            }
        });

        // CLEANED: Update error handler - use NEW batch system only
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
                    ui->Catalogs_pushButton_UpdateCatalog->setEnabled(true);
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

        // CLEANED: Update cancellation handler - use NEW batch system only
        connect(catalogManager, &CatalogManager::catalogOperationCancelled, this, [this]() {
            if (currentUpdateDevice) {
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
                    ui->Catalogs_pushButton_UpdateCatalog->setEnabled(true);
                }
            }
            // Creation cancellation is handled in existing cleanupStoppedCatalogCreation()
        });

        // Connect batch operation signals (NEW batch system)
        connect(catalogManager, &CatalogManager::batchOperationCompleted, this, [this]() {
            qDebug() << "Batch operation completed - restoring UI";

            // Reset MainWindow state
            currentUpdateDevice = nullptr;

            // Re-enable the UpdateAllActive button
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

            // Use the existing reportAllUpdates method to show the report
            reportAllUpdates(device, results, updateType);
        });

        qDebug() << "New catalog manager system setup complete";
    }
    //--------------------------------------------------------------------------
    void MainWindow::createCatalog()
    {//Create a new catalog, launch the cataloging and save, and refresh data and UI
        qDebug() << "=== MainWindow::createCatalog() ENTRY ===";

        //Change mouse cursor to wait cursor
        QApplication::setOverrideCursor(Qt::WaitCursor);
        ui->Create_pushButton_CreateCatalog->setEnabled(false);
        ui->Create_pushButton_Stop->setEnabled(true);

        //Check if mandatory inputs are provided
        if (ui->Create_lineEdit_NewCatalogName->text() == ""){
            QMessageBox msgBox;
            msgBox.setWindowTitle("Katalog");
            msgBox.setText(tr("Provide a name for this new catalog.<br/>"));
            msgBox.setIcon(QMessageBox::Warning);
            msgBox.exec();
            restoreCreateCatalogUIState();
            return;
        }
        if (ui->Create_lineEdit_NewCatalogPath->text() == ""){
            QMessageBox msgBox;
            msgBox.setWindowTitle("Katalog");
            msgBox.setText(tr("Provide a path for this new catalog.<br/>"));
            msgBox.setIcon(QMessageBox::Warning);
            msgBox.exec();
            restoreCreateCatalogUIState();
            return;
        }
        if (ui->Create_comboBox_StorageSelection->currentText() == ""){
            QMessageBox msgBox;
            msgBox.setWindowTitle("Katalog");
            msgBox.setText(tr("Select a Storage for this new catalog.<br/>(Selection panel on the left and dropdown list)"));
            msgBox.setIcon(QMessageBox::Warning);
            msgBox.exec();
            restoreCreateCatalogUIState();
            return;
        }

        // Check if directory exists and is not empty
        QDir sourceDir(ui->Create_lineEdit_NewCatalogPath->text());
        if (!sourceDir.exists()) {
            QMessageBox::warning(this, "Katalog", tr("Source directory does not exist."));
            restoreCreateCatalogUIState();
            return;
        }

        if (sourceDir.entryInfoList(QDir::NoDotAndDotDot|QDir::AllEntries).count() == 0) {
            QMessageBox::StandardButton reply = QMessageBox::question(this, "Katalog",
                                                                      tr("The selected directory is empty. Do you want to create an empty catalog?"),
                                                                      QMessageBox::Yes | QMessageBox::No);
            if (reply == QMessageBox::No) {
                restoreCreateCatalogUIState();
                return;
            }
        }

        //Create a new device and catalog (SAME AS ORIGINAL - Device-centric approach)

        //Initiate Device entry
        Device *newDevice = new Device();
        newDevice->generateDeviceID();
        newDevice->type = "Catalog";
        newDevice->name = ui->Create_lineEdit_NewCatalogName->text();

        qDebug() << "Creating new Device - ID:" << newDevice->ID << "Name:" << newDevice->name;

        //Check if the catalog name (so the csv file name) already exists
        if (newDevice->verifyDeviceNameExists()){
            QMessageBox msgBox;
            msgBox.setWindowTitle("Katalog");
            msgBox.setText( tr("There is already a catalog with this name:<br/><b>")
                           + newDevice->name
                           + "</b><br/><br/>"+tr("Choose a different name and try again."));
            msgBox.setIcon(QMessageBox::Warning);
            msgBox.exec();
            delete newDevice;
            restoreCreateCatalogUIState();
            return;
        }

        //Continue populating values and add device (SAME AS ORIGINAL)
        newDevice->parentID = ui->Create_comboBox_StorageSelection->currentData().toInt();
        newDevice->catalog->generateID();
        newDevice->externalID = newDevice->catalog->ID;
        newDevice->groupID = 0;
        newDevice->path = ui->Create_lineEdit_NewCatalogPath->text();
        newDevice->insertDevice();

        qDebug() << "Device inserted - Parent ID:" << newDevice->parentID << "Path:" << newDevice->path;

        //Get inputs and set values of the newCatalog (SAME AS ORIGINAL)
        newDevice->catalog->name = newDevice->name;  // Make sure catalog name matches device name
        newDevice->catalog->filePath = collection->folder + "/" + newDevice->name + ".idx";
        newDevice->catalog->sourcePath = ui->Create_lineEdit_NewCatalogPath->text();
        newDevice->catalog->includeHidden = ui->Create_checkBox_IncludeHidden->isChecked();
        newDevice->catalog->storageName = ui->Create_comboBox_StorageSelection->currentText();
        newDevice->catalog->includeSymblinks = ui->Create_checkBox_IncludeSymblinks->isChecked();
        newDevice->catalog->isFullDevice = ui->Create_checkBox_isFullDevice->isChecked();
        newDevice->catalog->includeMetadata = ui->Create_checkBox_IncludeMetadata->isChecked();
        newDevice->catalog->appVersion = currentVersion;

        //Get the file type for the catalog (SAME AS ORIGINAL)
        if      ( ui->Create_radioButton_FileType_Image->isChecked() ){
            newDevice->catalog->fileType = "Image";}
        else if ( ui->Create_radioButton_FileType_Audio->isChecked() ){
            newDevice->catalog->fileType = "Audio";}
        else if ( ui->Create_radioButton_FileType_Video->isChecked() ){
            newDevice->catalog->fileType = "Video";}
        else if ( ui->Create_radioButton_FileType_Text->isChecked() ){
            newDevice->catalog->fileType = "Text";}
        else
            newDevice->catalog->fileType = "All";

        qDebug() << "Catalog configured - File type:" << newDevice->catalog->fileType;

        //Save new catalog (SAME AS ORIGINAL)
        newDevice->catalog->insertCatalog();

        //Add path to parent Storage device if empty (SAME AS ORIGINAL)
        Device parentStorageDevice;
        parentStorageDevice.ID = newDevice->parentID;
        parentStorageDevice.loadDevice("defaultConnection");
        if(parentStorageDevice.path == ""){
            parentStorageDevice.path = newDevice->path;
            parentStorageDevice.saveDevice();
            collection->saveStorageTableToFile();
        }

        //Reload (SAME AS ORIGINAL)
        loadDevicesView("");
        loadStorageList();

        // *** NEW: Use the new catalog system for scanning instead of old updateDeviceWithUI ***
        qDebug() << "About to start catalog creation using new system for:" << newDevice->catalog->name;

        if (!catalogManager) {
            qDebug() << "ERROR: Catalog manager not initialized";
            delete newDevice;
            restoreCreateCatalogUIState();
            return;
        }

        // Store reference to the device for completion handling (Device-centric approach)
        currentCatalogDevice = newDevice;

        // Create a new catalog job stoppable for this operation
        catalogJobStoppable = new CatalogJobStoppable(this);

        // Update progress manager with the new engine
        if (catalogProgressManager) {
            catalogProgressManager->setCurrentCatalogEngine(catalogJobStoppable);
        }

        // Start the catalog operation using the new interface directly (Device-centric)
        catalogManager->startCatalogJobStoppable(
            catalogJobStoppable,
            newDevice,          // Pass the Device object (Device-centric approach)
            CatalogJobStoppable::CreateCatalog,
            collection->databaseMode,
            collection->folder
            );

        qDebug() << "Catalog creation started via new CatalogManager system";

        // NOTE: Don't restore cursor here - let the completion handler do it
        // QApplication::restoreOverrideCursor(); // REMOVED - completion handler will do this

        qDebug() << "=== MainWindow::createCatalog() EXIT ===";
    }
    //--------------------------------------------------------------------------
    void MainWindow::onCatalogOperationCompleted()
    {
        qDebug() << "=== onCatalogOperationCompleted() ENTRY - UI tasks only ===";

        try {
            // Get the device for UI updates
            Device* completedDevice = currentCatalogDevice;

            if (!completedDevice) {
                qDebug() << "ERROR: No device found for UI updates";
                restoreCreateCatalogUIState();
                return;
            }

            // All backend work is already done in CatalogJobStoppable
            // This method only handles UI-specific tasks

            qDebug() << "Backend processing completed, updating UI...";

            // UI Task 1: Save collection data files
            qDebug() << "UI Task 1: Saving collection data files";
            collection->saveDeviceTableToFile();
            collection->saveStatiticsTableToFile();

            // Update parents
            qDebug() << "CRITICAL FIX: Updating parent storage device numbers after creation";
            try {
                // Reload the completed device to ensure we have latest data
                completedDevice->loadDevice("defaultConnection");

                // Update parent numbers (this will update the parent storage device)
                completedDevice->updateParentsNumbers();

                qDebug() << "Parent numbers updated successfully after catalog creation";

                // Save again to persist the updated parent device
                collection->saveDeviceTableToFile();

            } catch (const std::exception& e) {
                qDebug() << "Error updating parent numbers after creation:" << e.what();
            }

            // UI Task 2: Show completion report (UI-specific)
            qDebug() << "UI Task 2: Showing completion report";
            QList<qint64> resultList;
            resultList << 1; // Success
            resultList << completedDevice->totalFileCount;
            resultList << completedDevice->totalFileCount; // Delta (all new)
            resultList << completedDevice->totalFileSize;
            resultList << completedDevice->totalFileSize; // Delta (all new)
            resultList << 1 << 0 << 0 << 0 << 0 << 0 << 0 << 0 << 0; // Padding for reportAllUpdates

            reportAllUpdates(completedDevice, resultList, "create");

            // UI Task 3: Refresh UI displays
            qDebug() << "UI Task 3: Refreshing UI displays";
            refreshDifferencesCatalogSelection();
            updateAllDeviceActive();
            loadDevicesView("");

            // UI Task 4: Update filter tree and selection
            qDebug() << "UI Task 4: Updating selection and filter tree";
            ui->Filters_label_DisplayCatalog->setText(completedDevice->name);
            selectedDevice->ID = completedDevice->ID;
            selectedDevice->loadDevice("defaultConnection");

            collection->loadDeviceFileToTable();
            loadDevicesTreeToModel("Filters");
            loadDevicesView("");

            // UI Task 5: Change to results tab
            qDebug() << "UI Task 5: Changing to Collection tab";
            ui->tabWidget->setCurrentIndex(1); // Collection tab
            ui->Catalogs_pushButton_UpdateCatalog->setEnabled(false);

            qDebug() << "All UI tasks completed successfully";

        } catch (const std::exception& e) {
            qDebug() << "EXCEPTION in UI completion tasks:" << e.what();
        } catch (...) {
            qDebug() << "UNKNOWN EXCEPTION in UI completion tasks";
        }

        // UI Task 6: Restore UI state
        currentCatalogDevice = nullptr;
        restoreCreateCatalogUIState();

        qDebug() << "=== onCatalogOperationCompleted() EXIT ===";
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
    void MainWindow::cleanupStoppedCatalogCreation()
    {
        qDebug() << "=== cleanupFailedCatalogCreation() START ===";

        // Use the device reference we stored when starting the operation
        if (currentCatalogDevice) {
            qDebug() << "Cleaning up failed catalog creation for device:" << currentCatalogDevice->name;

            // Use the existing backend method to delete the device (no UI confirmation)
            bool success = DeviceUIWrapper::deleteDeviceWithUI(currentCatalogDevice, false);

            if (success) {
                qDebug() << "Device deleted successfully";
            } else {
                qDebug() << "Failed to delete device";
            }

            // Clear the reference
            currentCatalogDevice = nullptr;
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
    //--------------------------------------------------------------------------
