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
        ui->Create_lineEdit_NewCatalogPath->setText(newSelectedPath);

        //Select this directory in the treeview.
        loadFileSystem(newSelectedPath);
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
            qDebug()<<"DEBUG: query: "<<insertQuery.lastError();

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

            QSqlQueryModel *model = new QSqlQueryModel;
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
        loadDevicesView(); // refresh the view
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

            QSqlQueryModel *model = new QSqlQueryModel;
            model->setQuery(std::move(queryLoad));

            QSortFilterProxyModel *proxyModel = new QSortFilterProxyModel(this);
            proxyModel->setSourceModel(model);
            proxyModel->setDynamicSortFilter(true);
            ui->Create_treeView_Excluded->setModel(proxyModel);
        });

        excludeContextMenu.exec(globalPos);
    }
    //--------------------------------------------------------------------------

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

        //If samba, check the connection and define the catalog path
        if (ui->Create_comboBox_SourceType->currentIndex()==1){
            qDebug() << "Creating catalog from Samba share.";
            QString sambaServerIP = ui->Create_lineEdit_SambaServerIP->text();
            QString sambaDirectory = ui->Create_lineEdit_SambaDirectory->text();
            QString sambaUser = ui->Create_lineEdit_SambaUser->text();
            QString sambaPassword = ui->Create_lineEdit_SambaPassword->text();

            if (sambaServerIP.isEmpty() || sambaDirectory.isEmpty()) {
                QMessageBox msgBox;
                msgBox.setWindowTitle("Katalog");
                msgBox.setText(tr("Provide the Samba server IP and directory to access."));
                msgBox.setIcon(QMessageBox::Warning);
                msgBox.exec();
                return;
            }

            // Check the connection
            QString sharePath = connectToShare(sambaServerIP, sambaDirectory, sambaUser, sambaPassword);
            qDebug() << "----------";
            qDebug() << "DEBUG: createCatalog / Share path:" << sharePath;
            if (sharePath == "false") {
                qDebug() << "DEBUG: createCatalog / Failed to connect to share.";
                return;
            }
            // Save the mounted path
            ui->Create_lineEdit_NewCatalogPath->setText(sharePath);
        }

        //Check if mandatory inputs are provided
        if (ui->Create_lineEdit_NewCatalogName->text() == ""){
            QMessageBox msgBox;
            msgBox.setWindowTitle("Katalog");
            msgBox.setText(tr("Provide a name for this new catalog.<br/>"));
            msgBox.setIcon(QMessageBox::Warning);
            msgBox.exec();
            return;
        }
        if (ui->Create_lineEdit_NewCatalogPath->text() == ""){
            QMessageBox msgBox;
            msgBox.setWindowTitle("Katalog");
            msgBox.setText(tr("Provide a path for this new catalog.<br/>"));
            msgBox.setIcon(QMessageBox::Warning);
            msgBox.exec();
            return;
        }
        if (ui->Create_comboBox_StorageSelection->currentText() == ""){
            QMessageBox msgBox;
            msgBox.setWindowTitle("Katalog");
            msgBox.setText(tr("Select a Storage for this new catalog.<br/>(Selection panel on the left and dropdown list)"));
            msgBox.setIcon(QMessageBox::Warning);
            msgBox.exec();
            return;
        }

        //Create a new device and catalog

            //Initiate Device entry
            Device *newDevice = new Device();
            newDevice->generateDeviceID();
            newDevice->type = "Catalog";
            newDevice->name = ui->Create_lineEdit_NewCatalogName->text();

            //Check if the catalog name (so the csv file name) already exists
            if (newDevice->verifyDeviceNameExists()){
                QMessageBox msgBox;
                msgBox.setWindowTitle("Katalog");
                msgBox.setText( tr("There is already a catalog with this name:<br/><b>")
                               + newDevice->name
                               + "</b><br/><br/>"+tr("Choose a different name and try again."));
                msgBox.setIcon(QMessageBox::Warning);
                msgBox.exec();
                return;
            }

            //Continue populating values and add device
            newDevice->parentID = ui->Create_comboBox_StorageSelection->currentData().toInt();
            newDevice->catalog->generateID();
            newDevice->externalID = newDevice->catalog->ID;
            newDevice->groupID = 0;
            newDevice->path = ui->Create_lineEdit_NewCatalogPath->text();
            newDevice->insertDevice();

            //Get inputs and set values of the newCatalog
            newDevice->catalog->filePath = collection->folder + "/" + newDevice->name + ".idx";
            newDevice->catalog->sourcePath = ui->Create_lineEdit_NewCatalogPath->text();
            newDevice->catalog->includeHidden = ui->Create_checkBox_IncludeHidden->isChecked();
            newDevice->catalog->storageName = ui->Create_comboBox_StorageSelection->currentText();
            newDevice->catalog->includeSymblinks = ui->Create_checkBox_IncludeSymblinks->isChecked();
            newDevice->catalog->isFullDevice = ui->Create_checkBox_isFullDevice->isChecked();
            newDevice->catalog->includeMetadata = ui->Create_checkBox_IncludeMetadata->isChecked();
            newDevice->catalog->appVersion = currentVersion;

            //Get the file type for the catalog
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

            //Save new catalog
            newDevice->catalog->insertCatalog();

            //Add path to parent Storage device if empty
            Device parentStorageDevice;
            parentStorageDevice.ID = newDevice->parentID;
            parentStorageDevice.loadDevice("defaultConnection");
            if(parentStorageDevice.path == ""){
                parentStorageDevice.path = newDevice->path;
                parentStorageDevice.saveDevice();
                collection->saveStorageTableToFile();
            }

            //Reload
            loadDevicesView();
            loadStorageList();

        //Launch the scan and cataloging of files, including statistics
            bool updateResult = reportAllUpdates(newDevice,
                                                 newDevice->updateDevice("create",
                                                                         collection->databaseMode,
                                                                         false,
                                                                         collection->folder,
                                                                         true),
                                                 "create");

            if (updateResult==true){
                newDevice->saveDevice();

                //Save data to files
                collection->saveDeviceTableToFile();
                newDevice->catalog->saveCatalogToFile(collection->databaseMode, collection->folder);
                newDevice->catalog->saveFoldersToFile(collection->databaseMode, collection->folder);

                //Update the new catalog loadedversion to indicate that files are already in memory
                QDateTime emptyDateTime = *new QDateTime; //Using an empty date as the function will manage creating one if needed
                newDevice->catalog->setDateLoaded(emptyDateTime, "defaultConnection");

                //Save statistics
                collection->saveStatiticsToFile();

                //Refresh data and UI
                    //Refresh the catalog list for the combobox of the Search screen
                    refreshDifferencesCatalogSelection();

                    //Refresh Catalogs list
                    updateAllDeviceActive();
                    loadDevicesView();

                    //Restore selected catalog
                    ui->Filters_label_DisplayCatalog->setText(newDevice->name);
                    selectedDevice->ID = newDevice->ID;
                    selectedDevice->loadDevice("defaultConnection");

                    //Refresh filter tree
                    collection->loadDeviceFileToTable();
                    loadDevicesTreeToModel("Filters");
                    loadDevicesView();

                    //Change tab to show the result of the catalog creation
                    ui->tabWidget->setCurrentIndex(1); // tab 1 is the Collection tab

                    //Disable buttons
                    ui->Catalogs_pushButton_UpdateCatalog->setEnabled(false);
            }
            else{
                newDevice->deleteDevice(false);
                loadDevicesView();
            }
    }
    //--------------------------------------------------------------------------
    QString MainWindow::connectToShare(const QString &serverIP, const QString &directory, const QString &username, const QString &password)
    {
        qDebug() << " " << "Connecting to share:" << serverIP << directory << username << password;
        QString sharePath = "false";

        // Format credentials properly
        QString credString;
        if (!username.isEmpty()) {
            if (!username.contains("\\")) {
                credString = QString("WORKGROUP\\%1%2").arg(username,
                                                            !password.isEmpty() ? "%" + password : "");
            } else {
                credString = QString("%1%2").arg(username,
                                                 !password.isEmpty() ? "%" + password : "");
            }
        }

        // Verify the share exists using smbclient
        QProcess smbList;
        QStringList listArgs;

        if (username.isEmpty()) {
            listArgs << "-N"; // Anonymous/guest access
        } else {
            listArgs << "-U" << credString;
        }

        listArgs << "-L" << serverIP;

        qDebug() << "Verifying share access with args:" <<
            listArgs.join(" ").replace(QRegularExpression("\\%.+"), "%***");

        smbList.start("smbclient", listArgs);
        smbList.waitForFinished();
        QString listOutput = smbList.readAllStandardOutput();
        QString listError = smbList.readAllStandardError();

        if (listError.contains("NT_STATUS_LOGON_FAILURE")) {
            QMessageBox msgBox;
            msgBox.setWindowTitle("Katalog");
            msgBox.setText(tr("Authentication failed for share access.<br/><br/>"
                              "Things to check:<br/>"
                              "- Username format (try with DOMAIN\\username)<br/>"
                              "- Password correctness<br/>"
                              "- Share permissions<br/><br/>"
                              "Debug info:<br/>") +
                           listError.replace("\n", "<br/>"));
            msgBox.setIcon(QMessageBox::Warning);
            msgBox.exec();
            return "false";
        }


        // Format the share path based on OS
        auto currentOS = QOperatingSystemVersion::current();


        if (currentOS.type() == QOperatingSystemVersion::Windows) {
            sharePath = QString("//%1/%2").arg(serverIP, directory);
            if (!QDir(sharePath).exists()) {
                sharePath = QString("\\\\%1\\%2").arg(serverIP, directory);
                qDebug() << "DEBUG: createCatalog / OS / Share path:" << currentOS.type() << sharePath;
            }
        }
        else if (currentOS.type() == QOperatingSystemVersion::MacOS) {
            QString volumePath = QString("/Volumes/%1").arg(directory);
            if (QDir(volumePath).exists()) {
                sharePath = volumePath;
            } else {
                sharePath = QString("//%1/%2").arg(serverIP, directory);
            }
            qDebug() << "DEBUG: createCatalog / OS / Share path:" << currentOS.type() << sharePath;
        }
        else { // Linux
        #ifdef Q_OS_UNIX
            qDebug() << "DEBUG: createCatalog / Format the share path for Linux:" << sharePath;

            QString mntPath = QString("/run/user/%1/gvfs/smb-share:server=%2,share=%3")
                                  .arg(QString::number(getuid()), serverIP, directory);

            if (QDir(mntPath).exists()) {
                sharePath = mntPath;
            } else {
                // Check if it's mounted elsewhere
                QProcess mountProcess;
                mountProcess.start("mount");
                mountProcess.waitForFinished();
                QString mountOutput = mountProcess.readAllStandardOutput();

                // Look for the share in mount output
                QRegularExpression re(QString("on\\s+(/[^\\s]+).*%1.*%2").arg(serverIP, directory));
                QRegularExpressionMatch match = re.match(mountOutput);
                if (match.hasMatch()) {
                    sharePath = match.captured(1);
                } else {
                    sharePath = QString("smb://%1/%2").arg(serverIP, directory);
                }
            }







            // Now try to access the specific share
            QProcess smbClient;
            QStringList args;
            QString shareUrl = QString("//%1/%2").arg(serverIP, directory);

            if (username.isEmpty()) {
                args << "-N";
            } else {
                args << "-U" << credString;
            }

            args << shareUrl << "-c" << "ls";

            qDebug() << "Running share access with args:" <<
                args.join(" ").replace(QRegularExpression("\\%.+"), "%***");

            smbClient.start("smbclient", args);
            smbClient.waitForFinished();
            QString output = smbClient.readAllStandardOutput();
            QString error = smbClient.readAllStandardError();
            qDebug() << "Directory listing output:" << output;
            qDebug() << "Directory listing error:" << error;


            QList<SmbEntry> results = listSmbDirectory(serverIP, directory, sharePath, credString);

            qDebug() << "Found" << results.size() << "entries";




            if (!error.isEmpty() && !error.contains("NT_STATUS_OK")) {
                QString errorMsg = error;
                if (error.contains("Not enough '\\' characters")) {
                    errorMsg += tr("<br/><br/>Note: There might be an issue with the username format. "
                                   "Try these formats:<br/>"
                                   "- username<br/>"
                                   "- DOMAIN\\username<br/>"
                                   "- username@DOMAIN");
                }

                QMessageBox msgBox;
                msgBox.setWindowTitle("Katalog");
                msgBox.setText(tr("Error accessing share:<br/>") + shareUrl +
                               tr("<br/><br/>Error message:<br/>") + errorMsg.replace("\n", "<br/>"));
                msgBox.setIcon(QMessageBox::Warning);
                msgBox.exec();
                //return;
            }

            // Parse and display the files if successful
            if (!output.isEmpty()) {
                QStringList files = output.split("\n", Qt::SkipEmptyParts);
                QString fileList;

                for (const QString& file : files) {
                    if (!file.trimmed().isEmpty()) {
                        fileList += file.trimmed() + "<br/>";
                    }
                }

                QMessageBox msgBox;
                msgBox.setWindowTitle("Katalog");
                msgBox.setText(tr("Share is accessible:<br/>") + shareUrl +
                               tr("<br/><br/>Sample files found:<br/>") + fileList);
                msgBox.setIcon(QMessageBox::Information);
                msgBox.exec();
            } else {
                QMessageBox msgBox;
                msgBox.setWindowTitle("Katalog");
                msgBox.setText(tr("Share is accessible but appears to be empty:<br/>") + shareUrl);
                msgBox.setIcon(QMessageBox::Information);
                msgBox.exec();
            }


        #else
                sharePath = QString("smb://%1/%2").arg(serverIP, directory);
        #endif
        }

        qDebug() << "DEBUG: createCatalog / OS / Share path:" << currentOS.type() << sharePath;

        // Verify we can access the specific share
        // QDir dir(sharePath);
        // if (!dir.exists()) {
        //     QMessageBox msgBox;
        //     msgBox.setWindowTitle("Katalog");
        //     msgBox.setText(tr("Unable to access share. Please mount it first in your file manager."));
        //     msgBox.setIcon(QMessageBox::Warning);
        //     msgBox.exec();
        //     return false;
        // }

        // If we got here, the share is accessible
        qDebug() << "DEBUG: createCatalog / Share mounted at:" << sharePath;
        return sharePath;
    }


    void MainWindow::iterateFiles(const QString &shareUrl,
                                  const QString &username,
                                  const QString &password,
                                  const QStringList &fileExtensions)
    {
        QString credString;
        if (!username.isEmpty()) {
            if (!username.contains("\\")) {
                credString = QString("WORKGROUP\\%1%2").arg(username,
                                                            !password.isEmpty() ? "%" + password : "");
            } else {
                credString = QString("%1%2").arg(username,
                                                 !password.isEmpty() ? "%" + password : "");
            }
        }

        QProcess smbClient;
        QStringList args;

        if (username.isEmpty()) {
            args << "-N";
        } else {
            args << "-U" << credString;
        }

        args << shareUrl << "-c" << "ls";

        qDebug() << "Running share access with args:" <<
            args.join(" ").replace(QRegularExpression("\\%.+"), "%***");

        smbClient.start("smbclient", args);
        smbClient.waitForFinished();
        QString output = smbClient.readAllStandardOutput();
        QString error = smbClient.readAllStandardError();

        qDebug() << "Directory listing output:" << output;
        qDebug() << "Directory listing error:" << error;

        if (!error.isEmpty() && !error.contains("NT_STATUS_OK")) {
            qDebug() << "Error accessing share:" << error;
            return;
        }

        QStringList files = output.split("\n", Qt::SkipEmptyParts);
        for (const QString& file : files) {
            if (!file.trimmed().isEmpty()) {
                qDebug() << "File:" << file.trimmed();
                // Process the file as needed
            }
        }
    }

    void MainWindow::listFilesRecursively(const QString &shareUrl, const QString &username, const QString &password, const QString &currentPath)
    {
        QString credString;
        if (!username.isEmpty()) {
            if (!username.contains("\\")) {
                credString = QString("WORKGROUP\\%1%2").arg(username,
                                                            !password.isEmpty() ? "%" + password : "");
            } else {
                credString = QString("%1%2").arg(username,
                                                 !password.isEmpty() ? "%" + password : "");
            }
        }

        QProcess smbClient;
        QStringList args;

        if (username.isEmpty()) {
            args << "-N";
        } else {
            args << "-U" << credString;
        }

        args << shareUrl << "-c" << QString("cd %1; ls").arg(currentPath);

        qDebug() << "Running share access with args:" <<
            args.join(" ").replace(QRegularExpression("\\%.+"), "%***");

        smbClient.start("smbclient", args);
        smbClient.waitForFinished();
        QString output = smbClient.readAllStandardOutput();
        QString error = smbClient.readAllStandardError();

        qDebug() << "Directory listing output:" << output;
        qDebug() << "Directory listing error:" << error;

        if (!error.isEmpty() && !error.contains("NT_STATUS_OK")) {
            qDebug() << "Error accessing share:" << error;
            return;
        }

        QStringList lines = output.split("\n", Qt::SkipEmptyParts);
        for (const QString& line : lines) {
            if (line.trimmed().isEmpty()) continue;

            QStringList parts = line.split(QRegularExpression("\\s+"), Qt::SkipEmptyParts);
            if (parts.size() < 2) continue;

            QString name = parts.last();
            QString type = parts.first();

            if (type == "D") {
                // It's a directory, recurse into it
                QString newPath = currentPath.isEmpty() ? name : currentPath + "/" + name;
                qDebug() << "Entering directory:" << newPath;
                listFilesRecursively(shareUrl, username, password, newPath);
            } else {
                // It's a file, process it
                QString filePath = currentPath.isEmpty() ? name : currentPath + "/" + name;
                qDebug() << "File:" << filePath;
                // Add your file processing logic here
            }
        }
    }
//DEV --------------------------------------------------------------------------

    //--- Metadata
    void MainWindow::setMediaFile(QString filePath)
    {
        QFile mediaFile(filePath);
        if(mediaFile.exists()==true){
            m_player = new QMediaPlayer(this);
//            connect(m_player, SIGNAL(mediaStatusChanged(QMediaPlayer::MediaStatus)), this, SLOT(onMediaStatusChanged(QMediaPlayer::MediaStatus)));
//            m_player->setSource(QUrl::fromLocalFile(filePath));

//             QMediaPlayer *player = new QMediaPlayer(this);
//            QMediaPlayer player;
 //            player->setMedia(QUrl::fromLocalFile(filePath));

            // Wait for the media to be loaded
//            if (player->mediaStatus() != QMediaPlayer::LoadedMedia) {
//                QObject::connect(&player, &QMediaPlayer::mediaStatusChanged, &app, [&player, &app](QMediaPlayer::MediaStatus status) {
//                    if (status == QMediaPlayer::LoadedMedia) {
//                        app.quit();
//                    }
//                });

//                return a.exec();
//            }

            // Retrieve the video's metadata
//            QVariant resolution = player->metaData(QMediaMetaData::Resolution);

//            if (resolution.isValid()) {
//                QSize videoResolution = resolution.toSize();
//                qDebug() << "Video resolution:" << videoResolution.width() << "x" << videoResolution.height();
//            } else {
//                qDebug() << "Failed to retrieve video resolution.";
//            }
        }
    }
    //--------------------------------------------------------------------------
    void MainWindow::onMediaStatusChanged(QMediaPlayer::MediaStatus status)
    {
        if (status == QMediaPlayer::LoadedMedia)
            getMetaData(m_player);
    }   
    //--------------------------------------------------------------------------
    void MainWindow::getMetaData(QMediaPlayer *player)
    {
        QMediaMetaData metaData = player->metaData();

        QVariant Resolution = metaData.value(QMediaMetaData::Resolution);
        QMessageBox msgBox;
        msgBox.setWindowTitle("Katalog");
        msgBox.setText(tr("Resolution")+": <br/>" + Resolution.toString());
        msgBox.setIcon(QMessageBox::Information);
        msgBox.exec();
        //QVideoFrame frame = player->currentFrame();

     }
    //--------------------------------------------------------------------------

    /*
    void MainWindow::getMetaData(QMediaPlayer *player)
    {
        QMessageBox::information(this,"Katalog","getMetaData");

        QMediaMetaData metaData = player->metaData();

        QVariant Resolution = metaData.value(QMediaMetaData::Resolution);
        QMessageBox::information(this,"Katalog","Resolution:<br/>" + Resolution.toString());

        QVariant Duration = player->metaData()[QMediaMetaData::Duration];
        QMessageBox::information(this,"Katalog","Duration:<br/>" + Duration.toString());

        QVariant MediaType = player->metaData()[QMediaMetaData::MediaType];
        QMessageBox::information(this,"Katalog","MediaType:<br/>" + MediaType.toString());
     }


     QString filePath = "/home/stephane/Vidéos/COPY/test8.mkv";

    */


    //--- Samba server -----------------------------------------------------------------------

     void MainWindow::on_Create_comboBox_SourceType_currentTextChanged(const QString &arg1)
     {
         if (arg1 == "Samba share") {
             ui->Create_widget_SambaSettings->show();
             ui->Create_widget_DriveSettings->hide();
         } else {
             ui->Create_widget_SambaSettings->hide();
             ui->Create_widget_DriveSettings->show();
         }
     }

     void MainWindow::on_Create_radioButton_SambaDirectory_clicked()
     {
         ui->Create_widget_SambaSettings->show();
         ui->Create_widget_DriveSettings->hide();
     }
    //--------------------------------------------------------------------------
     void MainWindow::on_Create_radioButton_MountedDrive_clicked()
     {
         ui->Create_widget_SambaSettings->hide();
         ui->Create_widget_DriveSettings->show();
     }
    //--------------------------------------------------------------------------
     void MainWindow::on_Create_pushButton_VerifyConnection_clicked()
     {
         QString sambaServerIP = ui->Create_lineEdit_SambaServerIP->text();
         QString sambaDirectory = ui->Create_lineEdit_SambaDirectory->text();
         QString sambaUser = ui->Create_lineEdit_SambaUser->text();
         QString sambaPassword = ui->Create_lineEdit_SambaPassword->text();

         // Save settings as before
         QSettings settings(collection->settingsFilePath, QSettings::IniFormat);
         settings.setValue("Settings/sambaServerIP", sambaServerIP);
         settings.setValue("Settings/sambaDirectory", sambaDirectory);
         settings.setValue("Settings/sambaUser", sambaUser);
         settings.setValue("Settings/sambaPassword", sambaPassword);

         // Format credentials properly
         QString credString;
         if (!sambaUser.isEmpty()) {
             if (!sambaUser.contains("\\")) {
                 credString = QString("WORKGROUP\\%1%2").arg(sambaUser,
                                                             !sambaPassword.isEmpty() ? "%" + sambaPassword : "");
             } else {
                 credString = QString("%1%2").arg(sambaUser,
                                                  !sambaPassword.isEmpty() ? "%" + sambaPassword : "");
             }
         }
         qDebug() << "Testing connection with credentials:" << (sambaUser.isEmpty() ? "anonymous" : sambaUser);

         // First, verify the share exists using smbclient
         QProcess smbList;
         QStringList listArgs;

         if (sambaUser.isEmpty()) {
             listArgs << "-N";  // Anonymous/guest access
         } else {
             listArgs << "-U" << credString;
         }

         listArgs << "-L" << sambaServerIP;

         qDebug() << "Running smbclient with args:" <<
             listArgs.join(" ").replace(QRegularExpression("\\%.+"), "%***");

         smbList.start("smbclient", listArgs);
         smbList.waitForFinished();
         QString listOutput = smbList.readAllStandardOutput();
         QString listError = smbList.readAllStandardError();

         qDebug() << "Share list output:" << listOutput;
         qDebug() << "Share list error:" << listError;

         if (listError.contains("NT_STATUS_LOGON_FAILURE")) {
             QMessageBox msgBox;
             msgBox.setWindowTitle("Katalog");
             msgBox.setText(tr("Authentication failed for share access.<br/><br/>"
                               "Things to check:<br/>"
                               "- Username format (try with DOMAIN\\username)<br/>"
                               "- Password correctness<br/>"
                               "- Share permissions<br/><br/>"
                               "Debug info:<br/>") +
                            listError.replace("\n", "<br/>"));
             msgBox.setIcon(QMessageBox::Warning);
             msgBox.exec();
             return;
         }






         // Now try to access the specific share
         QProcess smbClient;
         QStringList args;
         QString shareUrl = QString("//%1/%2").arg(sambaServerIP, sambaDirectory);

         if (sambaUser.isEmpty()) {
             args << "-N";
         } else {
             args << "-U" << credString;
         }

         args << shareUrl << "-c" << "ls";

         qDebug() << "Running share access with args:" <<
             args.join(" ").replace(QRegularExpression("\\%.+"), "%***");

         smbClient.start("smbclient", args);
         smbClient.waitForFinished();
         QString output = smbClient.readAllStandardOutput();
         QString error = smbClient.readAllStandardError();

         qDebug() << "Directory listing output:" << output;
         qDebug() << "Directory listing error:" << error;

         if (!error.isEmpty() && !error.contains("NT_STATUS_OK")) {
             QString errorMsg = error;
             if (error.contains("Not enough '\\' characters")) {
                 errorMsg += tr("<br/><br/>Note: There might be an issue with the username format. "
                                "Try these formats:<br/>"
                                "- username<br/>"
                                "- DOMAIN\\username<br/>"
                                "- username@DOMAIN");
             }

             QMessageBox msgBox;
             msgBox.setWindowTitle("Katalog");
             msgBox.setText(tr("Error accessing share:<br/>") + shareUrl +
                            tr("<br/><br/>Error message:<br/>") + errorMsg.replace("\n", "<br/>"));
             msgBox.setIcon(QMessageBox::Warning);
             msgBox.exec();
             return;
         }

         // Parse and display the files if successful
         if (!output.isEmpty()) {
             QStringList files = output.split("\n", Qt::SkipEmptyParts);
             QString fileList;

             for (const QString& file : files) {
                 if (!file.trimmed().isEmpty()) {
                     fileList += file.trimmed() + "<br/>";
                 }
             }

             QMessageBox msgBox;
             msgBox.setWindowTitle("Katalog");
             msgBox.setText(tr("Share is accessible:<br/>") + shareUrl +
                            tr("<br/><br/>Sample files found:<br/>") + fileList);
             msgBox.setIcon(QMessageBox::Information);
             msgBox.exec();
         } else {
             QMessageBox msgBox;
             msgBox.setWindowTitle("Katalog");
             msgBox.setText(tr("Share is accessible but appears to be empty:<br/>") + shareUrl);
             msgBox.setIcon(QMessageBox::Information);
             msgBox.exec();
         }




         //TEST iterateFiles
         qDebug() << "Testing iterateFiles";
         QStringList fileExtensions = {"*.txt", "*.pdf"}; // Add your extensions
         iterateFiles(shareUrl, sambaUser, sambaPassword, fileExtensions);
         qDebug() << "Tested iterateFiles";

         qDebug() << "Testing listFilesRecursively";
         //QString shareUrl = QString("//%1/%2").arg(serverIP, directory);
         listFilesRecursively(shareUrl, sambaUser, sambaPassword, "");
         qDebug() << "Testing listFilesRecursively complete";

     }
    //--------------------------------------------------------------------------

     class SmbEntry {
     public:
         QString name;
         bool isDirectory;
         qint64 size;
         QDateTime modified;
         QString fullPath;
     };
    //--------------------------------------------------------------------------
     QList<SmbEntry> listSmbDirectory(const QString &serverIP, const QString &shareName,
                                      const QString &path, const QString &credString) {
         QList<SmbEntry> entries;
         QProcess smbClient;
         QStringList args;
         QString shareUrl = QString("//%1/%2").arg(serverIP, shareName);

         // Set up authentication
         if (credString.isEmpty()) {
             args << "-N";
         } else {
             args << "-U" << credString;
         }

         args << shareUrl;

         // Build recursive listing command
         QString cmd = QString("recurse ON; ls %1*").arg(path.isEmpty() ? "" : path + "/");
         args << "-c" << cmd;

         qDebug() << "Running recursive list with args:" <<
             args.join(" ").replace(QRegularExpression("\\%.+"), "%***");

         smbClient.start("smbclient", args);
         smbClient.waitForFinished();
         QString output = smbClient.readAllStandardOutput();
         QString error = smbClient.readAllStandardError();

         if (!error.isEmpty() && !error.contains("NT_STATUS_OK")) {
             qDebug() << "Error listing directory:" << error;
             return entries;
         }

         // Parse the output
         QStringList lines = output.split("\n", Qt::SkipEmptyParts);
         QRegularExpression filePattern(
             "^\\s*([A-Z\\s]+)\\s+(\\d+)\\s+([A-Z][a-z]{2}\\s+[A-Z][a-z]{2}\\s+\\d+\\s+\\d{2}:\\d{2}:\\d{2}\\s+\\d{4})\\s+(.+)$"
             );

         QString currentDir;
         for (const QString &line : lines) {
             // Check for directory marker
             if (line.startsWith("  .")) {
                 continue;
             }

             if (line.startsWith("  D") && line.contains("0 ")) {
                 // This is likely a directory marker line
                 QRegularExpressionMatch match = filePattern.match(line);
                 if (match.hasMatch()) {
                     QString name = match.captured(4);
                     if (name != "." && name != "..") {
                         SmbEntry entry;
                         entry.name = name;
                         entry.isDirectory = true;
                         entry.size = 0;
                         entry.fullPath = path.isEmpty() ? name : path + "/" + name;

                         // Parse the date/time
                         QString dateStr = match.captured(3);
                         QDateTime dateTime = QDateTime::fromString(dateStr, "ddd MMM dd HH:mm:ss yyyy");
                         entry.modified = dateTime;

                         entries.append(entry);

                         // Recursively list this directory
                         QString newPath = path.isEmpty() ? name : path + "/" + name;
                         entries.append(listSmbDirectory(serverIP, shareName, newPath, credString));
                     }
                 }
             } else {
                 // Parse file entry
                 QRegularExpressionMatch match = filePattern.match(line);
                 if (match.hasMatch()) {
                     QString attributes = match.captured(1).trimmed();
                     if (!attributes.startsWith("D")) {  // Skip directory entries as we handle them above
                         SmbEntry entry;
                         entry.name = match.captured(4);
                         entry.isDirectory = false;
                         entry.size = match.captured(2).toLongLong();
                         entry.fullPath = path.isEmpty() ? entry.name : path + "/" + entry.name;

                         // Parse the date/time
                         QString dateStr = match.captured(3);
                         QDateTime dateTime = QDateTime::fromString(dateStr, "ddd MMM dd HH:mm:ss yyyy");
                         entry.modified = dateTime;

                         entries.append(entry);
                     }
                 }
             }
         }

         return entries;
     }

    // Usage example:
     void MainWindow::listShareContents(const QString &serverIP, const QString &shareName,
                                        const QString &credString) {
         QList<SmbEntry> allEntries = listSmbDirectory(serverIP, shareName, "", credString);

         // Filter files based on extensions if needed
         QStringList fileExtensions = {"*.txt", "*.pdf"}; // Add your extensions
         QList<SmbEntry> filteredEntries;

         for (const SmbEntry &entry : allEntries) {
             if (!entry.isDirectory) {
                 for (const QString &ext : fileExtensions) {
                     // Create a new string for the pattern
                     QString pattern = ext;
                     pattern.replace("*", ".*");
                     QRegularExpression re(pattern);
                     if (entry.name.contains(re)) {
                         filteredEntries.append(entry);
                         break;
                     }
                 }
             }
         }

         // Use the entries
         for (const SmbEntry &entry : filteredEntries) {
             qDebug() << "Found:" << entry.fullPath
                      << "Size:" << entry.size
                      << "Modified:" << entry.modified;
             // Process the file as needed
         }
     }
    //--------------------------------------------------------------------------
