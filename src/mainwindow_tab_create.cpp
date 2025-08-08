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
#include "ui_mainwindow.h"
#include "mainwindow_ui_wrapper_device.h"

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

        // Create the catalog progress manager
        catalogProgressManager = new CatalogProgressManager(this);
        catalogProgressManager->setCatalogManager(catalogManager);
        catalogProgressManager->setStatusBar(statusBar());

        // Connect catalog manager signals (same as before but using new system)
        connect(catalogManager, &CatalogManager::specialProgressUpdate,
                this, &MainWindow::updateStatusBarFromCatalogManager);
        connect(catalogManager, &CatalogManager::progressChanged,
                this, &MainWindow::updateStatusBarFromCatalogManager);
        connect(catalogManager, &CatalogManager::statusChanged,
                this, &MainWindow::updateStatusBarFromCatalogManager);
        connect(catalogManager, &CatalogManager::filesProcessedChanged,
                this, &MainWindow::updateStatusBarFromCatalogManager);
        connect(catalogManager, &CatalogManager::totalFilesChanged,
                this, &MainWindow::updateStatusBarFromCatalogManager);
        connect(catalogManager, &CatalogManager::currentPathChanged,
                this, &MainWindow::updateStatusBarFromCatalogManager);

        connect(catalogManager, &CatalogManager::catalogOperationCompleted,
                this, &MainWindow::onCatalogOperationCompleted);

        connect(catalogManager, &CatalogManager::catalogOperationRunningChanged,
                this, [this]() {
                    if (catalogManager->catalogOperationRunning()) {
                        // Stop any existing status bar timer during catalog operations
                        if (statusBarTimer) {
                            statusBarTimer->stop();
                        }

                        // Update progress manager with current catalog engine
                        if (catalogProgressManager && catalogManager->getCurrentCatalogEngine()) {
                            catalogProgressManager->setCurrentCatalogEngine(catalogManager->getCurrentCatalogEngine());
                        }
                    }
                });

        connect(catalogManager, &CatalogManager::catalogOperationError,
                this, [this](const QString &error) {
                    restoreCreateCatalogUIState();
                    QMessageBox::warning(this, "Katalog", tr("Catalog creation failed: %1").arg(error));
                });

        connect(catalogManager, &CatalogManager::catalogOperationCancelled,
                this, [this]() {
                    restoreCreateCatalogUIState();
                    QMessageBox::information(this, "Katalog", tr("Catalog operation was cancelled."));
                });

        qDebug() << "New catalog manager system setup complete";
    }
    //--------------------------------------------------------------------------
    void MainWindow::createCatalog()
    {//Create a new catalog, launch the cataloging and save, and refresh data and UI
        //Change mouse cursor to wait cursor
        QApplication::setOverrideCursor(Qt::WaitCursor);
        ui->Create_pushButton_CreateCatalog->setEnabled(false);
        ui->Create_pushButton_Stop->setEnabled(true);

        // Basic validation (like existing createCatalog method)
        if (ui->Create_lineEdit_NewCatalogName->text().isEmpty()) {
            QMessageBox::warning(this, "Katalog", tr("Provide a name for this new catalog."));
            restoreCreateCatalogUIState();
            return;
        }
        if (ui->Create_lineEdit_NewCatalogPath->text().isEmpty()) {
            QMessageBox::warning(this, "Katalog", tr("Provide a path for this new catalog."));
            restoreCreateCatalogUIState();
            return;
        }

        // Check if directory exists and is not empty**
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
                return; // User cancelled - no crash!
            }
        }

        // Create new device and catalog (same as before)
        Device *newDevice = new Device();
        newDevice->type = "Catalog";
        newDevice->groupID = 1;
        newDevice->parentID = ui->Create_comboBox_StorageSelection->currentData().toInt();
        newDevice->catalog = new Catalog(this);
        newDevice->catalog->name = ui->Create_lineEdit_NewCatalogName->text();
        newDevice->catalog->sourcePath = ui->Create_lineEdit_NewCatalogPath->text();
        newDevice->catalog->includeHidden = ui->Create_checkBox_IncludeHidden->isChecked();
        newDevice->catalog->includeSymblinks = ui->Create_checkBox_IncludeSymblinks->isChecked();
        newDevice->catalog->includeMetadata = ui->Create_checkBox_IncludeMetadata->isChecked();
        newDevice->catalog->appVersion = currentVersion;

        // Set file type
        if (ui->Create_radioButton_FileType_Image->isChecked()) {
            newDevice->catalog->fileType = "Image";
        } else if (ui->Create_radioButton_FileType_Audio->isChecked()) {
            newDevice->catalog->fileType = "Audio";
        } else if (ui->Create_radioButton_FileType_Video->isChecked()) {
            newDevice->catalog->fileType = "Video";
        } else if (ui->Create_radioButton_FileType_Text->isChecked()) {
            newDevice->catalog->fileType = "Text";
        } else {
            newDevice->catalog->fileType = "All";
        }

        // Save catalog to database
        newDevice->catalog->insertCatalog();

        // NEW: Use the new catalog system directly instead of startCreateCatalog()
        qDebug() << "About to start catalog creation using new system for:" << newDevice->catalog->name;

        if (!catalogManager) {
            qDebug() << "ERROR: Catalog manager not initialized";
            restoreCreateCatalogUIState();
            return;
        }

        // Create a new catalog job stoppable for this operation
        catalogJobStoppable = new CatalogJobStoppable(this);

        // Update progress manager with the new engine
        if (catalogProgressManager) {
            catalogProgressManager->setCurrentCatalogEngine(catalogJobStoppable);
        }

        // Start the catalog operation using the new interface directly
        catalogManager->startCatalogJobStoppable(
            catalogJobStoppable,
            newDevice,
            CatalogJobStoppable::CreateCatalog,
            collection->databaseMode,
            collection->folder
            );

        // Reload UI
        loadDevicesView("");
        loadStorageList();

        //Change back mouse cursor
        QApplication::restoreOverrideCursor();
    }
    //--------------------------------------------------------------------------
    void MainWindow::updateStatusBarFromCatalogManager()
    {
        // SIMPLIFIED: The CatalogProgressManager now handles all status bar updates automatically
        // This method can be removed or simplified to just defer to the progress manager

        if (catalogProgressManager) {
            // Let the progress manager handle the status bar updates
            catalogProgressManager->updateFromCatalogManager();
        } else {
            // Fallback for basic status if progress manager is not available
            if (!catalogManager) {
                statusBar()->showMessage(tr("Ready"));
                return;
            }

            QString statusMessage = catalogManager->catalogOperationRunning()
                                        ? catalogManager->status()
                                        : tr("Ready");
            statusBar()->showMessage(statusMessage);
        }
    }
    //--------------------------------------------------------------------------
    void MainWindow::onCatalogOperationCompleted()
    {
        qDebug() << "onCatalogOperationCompleted() called";

        try {
            // Get the device that was just processed - but validate it immediately
            Device* completedDevice = catalogManager->getCurrentDevice();
            if (!completedDevice) {
                qDebug() << "ERROR: No device found in completed operation";
                QApplication::restoreOverrideCursor();
                return;
            }

            // *** SAFETY: Copy device data immediately to avoid pointer issues ***
            // Create a safe copy of the device data before any cleanup happens
            QString deviceName = completedDevice->name;
            QString devicePath = completedDevice->path;
            QString deviceType = completedDevice->type;
            qint64 fileCount = completedDevice->catalog ? completedDevice->catalog->fileCount : 0;
            qint64 totalFileSize = completedDevice->catalog ? completedDevice->catalog->totalFileSize : 0;
            int deviceID = completedDevice->ID;

            qDebug() << "Device data copied safely - Name:" << deviceName << "Files:" << fileCount;

            // Update device statistics from the catalog (if catalog exists)
            if (completedDevice->catalog) {
                completedDevice->totalFileCount = fileCount;
                completedDevice->totalFileSize = totalFileSize;
                completedDevice->dateTimeUpdated = QDateTime::currentDateTime();
            }

            // Build list of process results
            QList<qint64> resultList;
            resultList << 1; // [0] Success code
            resultList << fileCount; // [1] New file count
            resultList << fileCount; // [2] Delta file count (all files are new for create)
            resultList << totalFileSize; // [3] New total file size
            resultList << totalFileSize; // [4] Delta total file size (all size is new for create)
            resultList << 1; // [5] Updated catalogs count
            resultList << 0; // [6] Skipped catalogs count
            resultList << 0; // [7] Storage success code (0 = not updated)
            resultList << 0; // [8] Used space
            resultList << 0; // [9] Delta used space
            resultList << 0; // [10] Free space
            resultList << 0; // [11] Delta free space
            resultList << 0; // [12] Total space
            resultList << 0; // [13] Delta total space

            qDebug() << "Calling reportAllUpdates with list:" << resultList;
            qDebug() << "List size:" << resultList.size();

            // *** SAFETY: Call reportAllUpdates with proper error handling ***
            bool updateResult = false;
            try {
                updateResult = reportAllUpdates(completedDevice, resultList, "create");
                qDebug() << "reportAllUpdates completed successfully, result:" << updateResult;
            } catch (const std::exception& e) {
                qDebug() << "EXCEPTION in reportAllUpdates:" << e.what();
                updateResult = false; // Treat as failure but continue
            } catch (...) {
                qDebug() << "UNKNOWN EXCEPTION in reportAllUpdates";
                updateResult = false; // Treat as failure but continue
            }

            // *** SAFETY: Only continue with post-processing if device is still valid ***
            if (updateResult && completedDevice && completedDevice->catalog) {
                qDebug() << "Processing post-completion tasks...";

                try {
                    qDebug() << "Step 1: About to call completedDevice->saveDevice()";
                    completedDevice->saveDevice();
                    qDebug() << "Step 1: completedDevice->saveDevice() completed";

                    qDebug() << "Step 2: About to call collection->saveDeviceTableToFile()";
                    collection->saveDeviceTableToFile();
                    qDebug() << "Step 2: collection->saveDeviceTableToFile() completed";

                    // *** SKIP FILE SAVES - Already done in createCatalogWithProgress() ***
                    qDebug() << "Step 3: Skipping catalog file saves (already completed in createCatalogWithProgress)";
                    // NOTE: We don't call saveCatalogToFile() or saveFoldersToFile() here because:
                    // 1. They were already handled properly in createCatalogWithProgress()
                    // 2. Calling them again causes crashes due to fileListModel state issues
                    // 3. The files are already written successfully to disk

                    qDebug() << "Step 4: About to update catalog loaded version";
                    QDateTime emptyDateTime = QDateTime::currentDateTime();
                    completedDevice->catalog->setDateLoaded(emptyDateTime, "defaultConnection");
                    qDebug() << "Step 4: Catalog loaded version updated";

                    qDebug() << "Step 5: About to call collection->saveStatiticsTableToFile()";
                    collection->saveStatiticsTableToFile();
                    qDebug() << "Step 5: saveStatiticsTableToFile completed";

                    qDebug() << "Step 6: About to refresh UI - refreshDifferencesCatalogSelection()";
                    refreshDifferencesCatalogSelection();
                    qDebug() << "Step 6: refreshDifferencesCatalogSelection completed";

                    qDebug() << "Step 7: About to call updateAllDeviceActive()";
                    updateAllDeviceActive();
                    qDebug() << "Step 7: updateAllDeviceActive completed";

                    qDebug() << "Step 8: About to call loadDevicesView(\"\")";
                    loadDevicesView("");
                    qDebug() << "Step 8: loadDevicesView completed";

                    qDebug() << "Step 9: About to restore selected catalog";
                    ui->Filters_label_DisplayCatalog->setText(deviceName);
                    selectedDevice->ID = deviceID;
                    selectedDevice->loadDevice("defaultConnection");
                    qDebug() << "Step 9: Selected catalog restored";

                    qDebug() << "Step 10: About to refresh filter tree";
                    collection->loadDeviceFileToTable();
                    qDebug() << "Step 10a: loadDeviceFileToTable completed";

                    loadDevicesTreeToModel("Filters");
                    qDebug() << "Step 10b: loadDevicesTreeToModel completed";

                    loadDevicesView("");
                    qDebug() << "Step 10c: second loadDevicesView completed";

                    qDebug() << "Step 11: About to change tab and disable buttons";
                    ui->tabWidget->setCurrentIndex(1); // tab 1 is the Collection tab
                    ui->Catalogs_pushButton_UpdateCatalog->setEnabled(false);
                    qDebug() << "Step 11: Tab changed and buttons disabled";

                    qDebug() << "Post-completion tasks completed successfully";

                } catch (const std::exception& e) {
                    qDebug() << "EXCEPTION in post-completion tasks:" << e.what();
                    // Continue anyway - the catalog was created successfully
                } catch (...) {
                    qDebug() << "UNKNOWN EXCEPTION in post-completion tasks";
                    // Continue anyway - the catalog was created successfully
                }

            } else {
                qDebug() << "Skipping post-completion tasks due to failure or invalid device";

                // Handle failure case - but only if device is still valid
                if (completedDevice) {
                    try {
                        DeviceUIWrapper::deleteDeviceWithUI(completedDevice, false);
                        loadDevicesView("");
                    } catch (...) {
                        qDebug() << "Exception during cleanup of failed catalog";
                    }
                }
            }

            // Always restore cursor, regardless of success/failure
            //QApplication::restoreOverrideCursor();
            qDebug() << "Catalog creation completion handler finished successfully";

        } catch (const std::exception& e) {
            qDebug() << "EXCEPTION in onCatalogOperationCompleted():" << e.what();
            //QApplication::restoreOverrideCursor();
        } catch (...) {
            qDebug() << "UNKNOWN EXCEPTION in onCatalogOperationCompleted()";
            //QApplication::restoreOverrideCursor();
        }

        restoreCreateCatalogUIState();
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


    // void MainWindow::updateCatalog(Device *device)  // If this method exists
    // {
    //     qDebug() << "MainWindow::updateCatalog() - Using new catalog system";

    //     if (!catalogManager) {
    //         qDebug() << "ERROR: Catalog manager not initialized";
    //         return;
    //     }

    //     // Create a new catalog job stoppable for this operation
    //     catalogJobStoppable = new CatalogJobStoppable(this);

    //     // Update progress manager with the new engine
    //     if (catalogProgressManager) {
    //         catalogProgressManager->setCurrentCatalogEngine(catalogJobStoppable);
    //     }

    //     // Start the catalog update using the new interface directly
    //     catalogManager->startCatalogJobStoppable(
    //         catalogJobStoppable,
    //         device,
    //         CatalogJobStoppable::UpdateCatalog,
    //         collection->databaseMode,
    //         collection->folder
    //         );
    // }



    // void MainWindow::startCreateCatalog(Device *device, const QString &databaseMode, const QString &collectionFolder)
    // {
    //     qDebug() << "MainWindow::startCreateCatalog() - Using new catalog system";

    //     if (!catalogManager) {
    //         qDebug() << "ERROR: Catalog manager not initialized";
    //         return;
    //     }

    //     // Create a new catalog job stoppable for this operation
    //     catalogJobStoppable = new CatalogJobStoppable(this);

    //     // Update progress manager with the new engine
    //     if (catalogProgressManager) {
    //         catalogProgressManager->setCurrentCatalogEngine(catalogJobStoppable);
    //     }

    //     // Start the catalog operation using the new interface
    //     catalogManager->startCatalogJobStoppable(
    //         catalogJobStoppable,
    //         device,
    //         CatalogJobStoppable::CreateCatalog,
    //         databaseMode,
    //         collectionFolder
    //         );
    // }

    // void MainWindow::startUpdateCatalog(Device *device, const QString &databaseMode, const QString &collectionFolder)
    // {
    //     qDebug() << "MainWindow::startUpdateCatalog() - Using new catalog system";

    //     if (!catalogManager) {
    //         qDebug() << "ERROR: Catalog manager not initialized";
    //         return;
    //     }

    //     // Create a new catalog job stoppable for this operation
    //     catalogJobStoppable = new CatalogJobStoppable(this);

    //     // Update progress manager with the new engine
    //     if (catalogProgressManager) {
    //         catalogProgressManager->setCurrentCatalogEngine(catalogJobStoppable);
    //     }

    //     // Start the catalog operation using the new interface
    //     catalogManager->startCatalogJobStoppable(
    //         catalogJobStoppable,
    //         device,
    //         CatalogJobStoppable::UpdateCatalog,
    //         databaseMode,
    //         collectionFolder
    //         );
    // }

    // void MainWindow::stopCatalogOperation()
    // {
    //     qDebug() << "MainWindow::stopCatalogOperation() - Using new catalog system";

    //     if (catalogManager) {
    //         catalogManager->stopCatalogOperation();
    //     }
    // }
