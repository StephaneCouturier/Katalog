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
        //Stop the cataloging process
        if (catalogManager) {
            catalogManager->stopOperation();
            qDebug() << "Catalog creation stopped by user.";
        } else {
            qDebug() << "No catalogManager instance to stop.";
        }

        //Change back mouse cursor
        QApplication::restoreOverrideCursor();
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
    void MainWindow::createCatalog()
    {//Create a new catalog, launch the cataloging and save, and refresh data and UI
        //Change mouse cursor to wait cursor
        QApplication::setOverrideCursor(Qt::WaitCursor);
        ui->Create_pushButton_CreateCatalog->setEnabled(false);
        ui->Create_pushButton_Stop->setEnabled(true);

        // Basic validation (like existing createCatalog method)
        if (ui->Create_lineEdit_NewCatalogName->text().isEmpty()) {
            QMessageBox::warning(this, "Katalog", tr("Provide a name for this new catalog."));
            return;
        }
        if (ui->Create_lineEdit_NewCatalogPath->text().isEmpty()) {
            QMessageBox::warning(this, "Katalog", tr("Provide a path for this new catalog."));
            return;
        }

        // Check if directory exists and is not empty**
        QDir sourceDir(ui->Create_lineEdit_NewCatalogPath->text());
        if (!sourceDir.exists()) {
            QMessageBox::warning(this, "Katalog", tr("Source directory does not exist."));
            return;
        }

        if (sourceDir.entryInfoList(QDir::NoDotAndDotDot|QDir::AllEntries).count() == 0) {
            QMessageBox::StandardButton reply = QMessageBox::question(this, "Katalog",
                                                                      tr("The selected directory is empty. Do you want to create an empty catalog?"),
                                                                      QMessageBox::Yes | QMessageBox::No);
            if (reply == QMessageBox::No) {
                return; // User cancelled - no crash!
            }
        }

        // Create new device and catalog (following existing pattern)
        Device *newDevice = new Device();
        newDevice->generateDeviceID();
        newDevice->type = "Catalog";
        newDevice->name = ui->Create_lineEdit_NewCatalogName->text();

        // Check if name already exists
        if (newDevice->verifyDeviceNameExists()) {
            QMessageBox::warning(this, "Katalog",
                                 tr("There is already a catalog with this name: %1").arg(newDevice->name));
            delete newDevice;
            return;
        }

        // Set up device properties (from existing createCatalog)
        newDevice->catalog->name = newDevice->name;  // *** ADD THIS LINE ***
        newDevice->parentID = ui->Create_comboBox_StorageSelection->currentData().toInt();
        newDevice->catalog->generateID();
        newDevice->externalID = newDevice->catalog->ID;
        newDevice->groupID = 0;
        newDevice->path = ui->Create_lineEdit_NewCatalogPath->text();
        newDevice->insertDevice();

        // Configure catalog properties
        newDevice->catalog->filePath = collection->folder + "/" + newDevice->name + ".idx";
        newDevice->catalog->sourcePath = ui->Create_lineEdit_NewCatalogPath->text();
        newDevice->catalog->includeHidden = ui->Create_checkBox_IncludeHidden->isChecked();
        newDevice->catalog->storageName = ui->Create_comboBox_StorageSelection->currentText();
        newDevice->catalog->includeSymblinks = ui->Create_checkBox_IncludeSymblinks->isChecked();
        newDevice->catalog->isFullDevice = ui->Create_checkBox_isFullDevice->isChecked();
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

        // Use CatalogManager for asynchronous work
        qDebug() << "About to start catalog creation for:" << newDevice->catalog->name;
        catalogManager->startCreateCatalog(newDevice, collection->databaseMode, collection->folder);

        // Reload UI
        loadDevicesView("");
        loadStorageList();

        //Change back mouse cursor
        QApplication::restoreOverrideCursor();
        ui->Create_pushButton_CreateCatalog->setEnabled(true);
        ui->Create_pushButton_Stop->setEnabled(false);
    }
    //----------------------------------------------------------------------
    void MainWindow::updateStatusBarFromCatalogManager()
    {
        qDebug() << "updateStatusBarFromCatalogManager() called";

        if (!catalogManager) {
            qDebug() << "No catalogManager - showing Ready";
            statusBar()->showMessage(tr("Ready"));
            return;
        }

        QString statusMessage;
        qDebug() << "CatalogManager state - Running:" << catalogManager->catalogOperationRunning()
                 << "Progress:" << catalogManager->progress()
                 << "FilesProcessed:" << catalogManager->filesProcessed()
                 << "TotalFiles:" << catalogManager->totalFiles();

        if (catalogManager->catalogOperationRunning()) {
            // Build dynamic status message while catalog operation is running

            if (catalogManager->totalFiles() > 0 && catalogManager->filesProcessed() >= 0) {
                // Show detailed progress with your requested format:
                // "Files processed: 500 | Total files: 50,000 | Progress: 1% | path_of_last_file_added"
                statusMessage = QString("Files processed: %1 | Total files: %2 | Progress: %3%")
                                    .arg(QLocale().toString(catalogManager->filesProcessed()))
                                    .arg(QLocale().toString(catalogManager->totalFiles()))
                                    .arg(catalogManager->progress());

                // Add current path if available
                if (!catalogManager->currentPath().isEmpty()) {
                    statusMessage += QString(" | %1").arg(catalogManager->currentPath());
                }
            } else {
                // Fallback to basic status during initial phases
                statusMessage = catalogManager->status();

                // Add progress percentage if available
                if (catalogManager->progress() > 0) {
                    statusMessage += QString(" (%1%)").arg(catalogManager->progress());
                }
            }
        } else {
            // Catalog operation not running - show final status or ready state
            if (catalogManager->status() == "Ready") {
                statusMessage = tr("Ready");
            } else {
                // Show completion status or error
                statusMessage = catalogManager->status();

                // If we have final file counts, show them
                if (catalogManager->totalFiles() > 0) {
                    statusMessage += QString(" | Files processed: %1")
                    .arg(QLocale().toString(catalogManager->totalFiles()));
                }
            }
        }

        qDebug() << "Setting status bar message:" << statusMessage;

        // Status Bar handling
        statusBar()->show();
        statusBar()->showMessage(statusMessage, 0);  // 0 = no timeout, don't auto-clear
        QCoreApplication::processEvents();  // Force immediate UI update
    }
    //----------------------------------------------------------------------
    void MainWindow::onCatalogOperationCompleted()
    {
        qDebug() << "onCatalogOperationCompleted() called";

        // Get the device that was just processed
        Device* completedDevice = catalogManager->getCurrentDevice();
        if (!completedDevice) {
            qDebug() << "No device found in completed operation";
            QApplication::restoreOverrideCursor();
            return;
        }

        // Update device statistics from the catalog
        completedDevice->totalFileCount = completedDevice->catalog->fileCount;
        completedDevice->totalFileSize = completedDevice->catalog->totalFileSize;
        completedDevice->dateTimeUpdated = QDateTime::currentDateTime();

        // Build list of process results based on the pattern from DeviceUIWrapper::updateDeviceWithUI
        QList<qint64> resultList;

        // Catalog update data (indices 0-6)
        resultList << 1; // [0] Success code
        resultList << completedDevice->catalog->fileCount; // [1] New file count
        resultList << completedDevice->catalog->fileCount; // [2] Delta file count (all files are new for create)
        resultList << completedDevice->catalog->totalFileSize; // [3] New total file size
        resultList << completedDevice->catalog->totalFileSize; // [4] Delta total file size (all size is new for create)
        resultList << 1; // [5] Updated catalogs count (we updated 1 catalog)
        resultList << 0; // [6] Skipped catalogs count (we skipped 0 catalogs)

        // Storage update data (indices 7-13) - using zeros since we don't have storage update
        resultList << 0; // [7] Storage success code (0 = not updated)
        resultList << 0; // [8] Used space
        resultList << 0; // [9] Delta used space
        resultList << 0; // [10] Free space
        resultList << 0; // [11] Delta free space
        resultList << 0; // [12] Total space
        resultList << 0; // [13] Delta total space

        qDebug() << "Calling reportAllUpdates with list:" << resultList;
        qDebug() << "List size:" << resultList.size();

        // Use the reportAllUpdates for consistency
        bool updateResult = reportAllUpdates(completedDevice, resultList, "create");

        if (updateResult == true) {
            // Save device data (since reportAllUpdates doesn't do this)
            completedDevice->saveDevice();

            // Save data to files (exactly like original createCatalog)
            collection->saveDeviceTableToFile();
            completedDevice->catalog->saveCatalogToFile(collection->databaseMode, collection->folder);
            completedDevice->catalog->saveFoldersToFile(collection->databaseMode, collection->folder);

            // Update the new catalog loaded version (exactly like original)
            QDateTime emptyDateTime = *new QDateTime;
            completedDevice->catalog->setDateLoaded(emptyDateTime, "defaultConnection");

            // Save statistics
            collection->saveStatiticsTableToFile();

            // Refresh data and UI (exactly like original createCatalog)
            refreshDifferencesCatalogSelection();
            updateAllDeviceActive();
            loadDevicesView("");

            // Restore selected catalog
            ui->Filters_label_DisplayCatalog->setText(completedDevice->name);
            selectedDevice->ID = completedDevice->ID;
            selectedDevice->loadDevice("defaultConnection");

            // Refresh filter tree
            collection->loadDeviceFileToTable();
            loadDevicesTreeToModel("Filters");
            loadDevicesView("");

            // Change tab to show the result of the catalog creation
            ui->tabWidget->setCurrentIndex(1); // tab 1 is the Collection tab

            // Disable buttons
            ui->Catalogs_pushButton_UpdateCatalog->setEnabled(false);
        } else {
            // Handle failure case (like original createCatalog)
            DeviceUIWrapper::deleteDeviceWithUI(completedDevice, false);
            loadDevicesView("");
        }

        // Change back mouse cursor (like original createCatalog)
        QApplication::restoreOverrideCursor();

        qDebug() << "Catalog creation completed successfully!";
    }
    //----------------------------------------------------------------------
