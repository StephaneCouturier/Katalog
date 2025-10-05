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
// File Name:   mainwindow_tab_settings_exp.cpp
// Purpose:     methods for the Settings panel and collection export features
// Description: https://stephanecouturier.github.io/Katalog/docs/Features/Settings
// Author:      Stephane Couturier
/////////////////////////////////////////////////////////////////////////////
*/

#include "mainwindow.h"

// Export to SQLite File mode
void MainWindow::exportToSQLiteFile()
{
    if (collection->databaseMode == "Memory") {

        QMessageBox msgBox;
        msgBox.setWindowTitle("Katalog");

        //Folder and file name selection

        //Default folder and file name
        QString backupFilePath = collection->folder + "/export.db"; // Path to the output file

        //Open a dialog for the user to select the directory of the collection where catalog files are stored.
        QString selectedBackupFilePath = QFileDialog::getSaveFileName(this, tr("Select the directory and file name for his export."),
                                                                      backupFilePath);

        //Unless the selection was cancelled, set the new collection folder, and refresh all data
        if ( selectedBackupFilePath !=""){
            QFile newBackupFile(selectedBackupFilePath);
            if( newBackupFile.exists())
                newBackupFile.moveToTrash();

            backupFilePath = selectedBackupFilePath;
        }

        //Load all Catalogs indexes into memory

        //Prepare temporary variables
        Device tempCatalogDevice;
        QMutex tempMutex;
        bool tempStopRequested = false;

        // Get the total number of files for all devices
        QSqlQuery fileCountQuery(QSqlDatabase::database(m_connectionName));
        QString fileCountQuerySQL = QLatin1String(R"(
                    SELECT SUM(device_total_file_count)
                    FROM device
                    WHERE device_type ="Catalog"
                    AND device_group_id = 0
                )");
        fileCountQuery.prepare(fileCountQuerySQL);
        fileCountQuery.exec();
        fileCountQuery.next();
        qint64 totalFileCount = fileCountQuery.value(0).toInt();

        // Create the progress dialog
        QProgressDialog progress("Loading devices...", "Cancel", 0, totalFileCount, this);
        progress.setWindowModality(Qt::WindowModal);
        qint64 filesLoaded = 0;

        // List all Catalogs indexes to be loaded into memory
        QSqlQuery query(QSqlDatabase::database(m_connectionName));
        QString querySQL = QLatin1String(R"(
                    SELECT device_id, device_name, device_total_file_count
                    FROM device
                    WHERE device_type ="Catalog"
                )");
        query.prepare(querySQL);
        query.exec();

        while(query.next()){
            int deviceId = query.value(0).toInt();
            QString deviceName = query.value(1).toString();
            qint64 deviceFileCount = query.value(2).toInt();

            progress.setLabelText(QString("Loading all catalogs prior to export<br/> %1 <br/><br/> %2 files loaded out of %3" ).arg(deviceName, QLocale().toString(filesLoaded), QLocale().toString(totalFileCount)) );

            tempCatalogDevice.ID = deviceId;
            tempCatalogDevice.loadDevice(m_connectionName);
            tempCatalogDevice.catalog->loadCatalogFileListToTable(tempMutex, tempStopRequested);

            filesLoaded += deviceFileCount;
            progress.setValue(filesLoaded);

            if (progress.wasCanceled())
                return;
        }

        //Dump all the database in Memory to the sql File
        if (!backupMemoryDatabaseToFile(m_connectionName, backupFilePath)) {
            msgBox.setText(QCoreApplication::translate("MainWindow",
                                                       "Failed to export in-memory database to file.<br/>"
                                                       "<br/> Export file path: <br/><b>%1</b><br/>"
                                                       ).arg( backupFilePath ));
        } else {
            msgBox.setIcon(QMessageBox::Information);
            msgBox.setText(QCoreApplication::translate("MainWindow",
                                                       "Successful export of collection to SQLite database file.<br/>"
                                                       "<br/> Export file path: <br/><b>%1</b><br/>"
                                                       ).arg( backupFilePath ));
        }

        //Inform of end of process
        msgBox.exec();
    }
}

// Export to Memory mode
void MainWindow::exportToMemoryMode()
{
    if (collection->databaseMode == "File") {
        QMessageBox msgBox;
        msgBox.setWindowTitle("Katalog");

        // Folder selection for export
        QString exportFolderPath = collection->folder; // Default to current collection folder

        // Open a dialog for the user to select the export directory
        QString selectedExportFolder = QFileDialog::getExistingDirectory(this,
                                                                         tr("Select the directory for the CSV export"),
                                                                         exportFolderPath);

        // If selection was cancelled, exit
        if (selectedExportFolder.isEmpty()) {
            return;
        }

        exportFolderPath = selectedExportFolder;

        // Create a progress dialog
        QProgressDialog progress("Exporting database to CSV files...", "Cancel", 0, 100, this);
        progress.setWindowModality(Qt::WindowModal);
        progress.setValue(0);

        try {
            // Set the new collection folder temporarily for export
            QString originalFolder = collection->folder;
            collection->folder = exportFolderPath;
            collection->generateCollectionFilesPaths();

            // Save original database mode
            QString originalDatabaseMode = collection->databaseMode;

            // Temporarily set to Memory mode so the function will run
            collection->databaseMode = "Memory";

            // Initialize export folder structure
            progress.setValue(5);
            progress.setLabelText("Creating collection files...");
            collection->generateCollectionFiles();

            // Export parameters
            progress.setValue(10);
            progress.setLabelText("Exporting parameters...");
            collection->saveParameterTableToFile();

            // Export devices
            progress.setValue(15);
            progress.setLabelText("Exporting devices...");
            collection->saveDeviceTableToFile();

            // Export storage
            progress.setValue(20);
            progress.setLabelText("Exporting storage...");
            collection->saveStorageTableToFile();

            // Export tags
            progress.setValue(25);
            progress.setLabelText("Exporting tags...");
            collection->saveTagTableToFile();

            // Export mappings
            progress.setValue(30);
            progress.setLabelText("Exporting device mappings...");
            collection->saveMappingTableToFile();

            // Export search history
            progress.setValue(35);
            progress.setLabelText("Exporting search history...");
            collection->saveSearchHistoryTableToFile();

            // Export statistics
            progress.setValue(40);
            progress.setLabelText("Exporting statistics...");
            collection->saveStatiticsTableToFile();

            // Export catalogs to idx files (this already has its own progress updates)
            exportAllCatalogFiles(progress);

            // Restore original settings
            collection->databaseMode = originalDatabaseMode;
            collection->folder = originalFolder;
            collection->generateCollectionFilesPaths();

            // Ensure progress is complete
            progress.setValue(100);

            // Inform user of successful export
            msgBox.setIcon(QMessageBox::Information);
            msgBox.setText(tr("Successfully exported database to CSV files.<br/>"
                              "<br/>Export directory: <br/><b>%1</b><br/>"
                              "<br/>You can now switch to Memory mode and load this collection.")
                               .arg(exportFolderPath));
            msgBox.exec();
        }
        catch (const std::exception& e) {
            // Restore original folder and mode on error
            collection->databaseMode = "File";
            collection->folder = exportFolderPath;
            collection->generateCollectionFilesPaths();

            msgBox.setIcon(QMessageBox::Warning);
            msgBox.setText(tr("Export failed: %1").arg(e.what()));
            msgBox.exec();
        }
    }
}

bool MainWindow::exportAllCatalogFiles(QProgressDialog &progress)
{
    // Get all catalog devices
    QSqlQuery catalogDevicesQuery(QSqlDatabase::database(m_connectionName));
    QString catalogDevicesQuerySQL = QLatin1String(R"(
        SELECT d.device_id, d.device_name, d.device_path, d.device_total_file_count,
               d.device_total_file_size, d.device_external_id
        FROM device d
        WHERE d.device_type = 'Catalog'
        ORDER BY d.device_id
    )");

    if (!catalogDevicesQuery.exec(catalogDevicesQuerySQL)) {
        qDebug() << "Catalog devices query failed:" << catalogDevicesQuery.lastError().text();
        return false;
    }

    // Count total catalogs
    QSqlQuery countQuery(QSqlDatabase::database(m_connectionName));
    countQuery.prepare("SELECT COUNT(*) FROM device WHERE device_type = 'Catalog'");

    if (!countQuery.exec() || !countQuery.next()) {
        qDebug() << "Count query failed:" << countQuery.lastError().text();
        return false;
    }

    int totalCatalogs = countQuery.value(0).toInt();

    if (totalCatalogs == 0) {
        // No catalogs to export
        return true;
    }

    // Process each catalog device
    int currentCatalog = 0;
    while (catalogDevicesQuery.next()) {
        currentCatalog++;

        // Extract catalog device data
        QString deviceName = catalogDevicesQuery.value(1).toString();
        QString devicePath = catalogDevicesQuery.value(2).toString();
        qint64 fileCount = catalogDevicesQuery.value(3).toLongLong();
        qint64 totalFileSize = catalogDevicesQuery.value(4).toLongLong();
        int catalogId = catalogDevicesQuery.value(5).toInt(); // device_external_id is the catalog_id

        // Update progress
        int progressValue = 20 + (currentCatalog * 60) / totalCatalogs;
        progress.setValue(progressValue);
        progress.setLabelText(QString("Exporting catalog: %1 (%2/%3)")
                                  .arg(deviceName)
                                  .arg(currentCatalog)
                                  .arg(totalCatalogs));

        if (progress.wasCanceled()) {
            return false;
        }

        // Get additional catalog metadata
        QSqlQuery catalogMetaQuery(QSqlDatabase::database(m_connectionName));
        QString catalogMetaQuerySQL = QLatin1String(R"(
            SELECT catalog_include_hidden, catalog_file_type, catalog_storage,
                   catalog_include_symblinks, catalog_is_full_device,
                   catalog_include_metadata, catalog_app_version
            FROM catalog
            WHERE catalog_id = :catalog_id
        )");
        catalogMetaQuery.prepare(catalogMetaQuerySQL);
        catalogMetaQuery.bindValue(":catalog_id", catalogId);

        if (!catalogMetaQuery.exec() || !catalogMetaQuery.next()) {
            qDebug() << "Catalog metadata query failed for catalog" << catalogId << ":" << catalogMetaQuery.lastError().text();
            continue; // Skip this catalog but continue with others
        }

        // Extract catalog metadata
        QString includeHidden = catalogMetaQuery.value(0).toString();
        QString fileType = catalogMetaQuery.value(1).toString();
        QString storageName = catalogMetaQuery.value(2).toString();
        QString includeSymblinks = catalogMetaQuery.value(3).toString();
        QString isFullDevice = catalogMetaQuery.value(4).toString();
        QString includeMetadata = catalogMetaQuery.value(5).toString();
        QString appVersion = catalogMetaQuery.value(6).toString();

        // Create the idx file
        QString idxFilePath = collection->folder + "/" + deviceName + ".idx";
        QFile idxFile(idxFilePath);

        if (!idxFile.open(QIODevice::WriteOnly | QIODevice::Text)) {
            qDebug() << "Cannot open idx file for writing:" << idxFilePath;
            continue; // Skip this catalog but continue with others
        }

        QTextStream idxStream(&idxFile);

        // Write catalog headers - exactly matching the format needed
        idxStream << "<catalogSourcePath>" << devicePath << "\n";
        idxStream << "<catalogFileCount>" << QString::number(fileCount) << "\n";
        idxStream << "<catalogTotalFileSize>" << QString::number(totalFileSize) << "\n";
        idxStream << "<catalogIncludeHidden>" << includeHidden << "\n";
        idxStream << "<catalogFileType>" << fileType << "\n";
        idxStream << "<catalogStorage>" << storageName << "\n";
        idxStream << "<catalogIncludeSymblinks>" << includeSymblinks << "\n";
        idxStream << "<catalogIsFullDevice>" << isFullDevice << "\n";
        idxStream << "<catalogIncludeMetadata>" << includeMetadata << "\n";
        idxStream << "<catalogAppVersion>" << appVersion << "\n";
        idxStream << "<catalogID>" << QString::number(catalogId) << "\n";

        // Get all files for this catalog
        // Get all files for this catalog
        QSqlQuery fileQuery(QSqlDatabase::database(m_connectionName));
        QString fileQuerySQL = QLatin1String(R"(
                                SELECT file_full_path,
                                       file_size,
                                       file_date_updated,
                                       file_extension,
                                       file_type,
                                       mime_type,
                                       mime_verified,
                                       type_mismatch,
                                       image_width,
                                       image_height,
                                       image_orientation,
                                       video_duration_seconds,
                                       video_width,
                                       video_height,
                                       video_codec,
                                       video_framerate,
                                       video_bitrate,
                                       audio_duration_seconds,
                                       audio_artist,
                                       audio_album,
                                       audio_title,
                                       audio_genre,
                                       audio_year,
                                       audio_track_number,
                                       audio_bitrate,
                                       audio_sample_rate,
                                       metadata_extended,
                                       metadata_extraction_date
                                FROM file
                                WHERE file_catalog_id = :catalog_id
                                ORDER BY file_full_path
                            )");
        fileQuery.prepare(fileQuerySQL);
        fileQuery.bindValue(":catalog_id", catalogId);

        if (!fileQuery.exec()) {
            qDebug() << "File query failed for catalog" << catalogId << ":" << fileQuery.lastError().text();
            idxFile.close();
            continue; // Skip this catalog but continue with others
        }

        // Write all file entries
        while (fileQuery.next()) {
            idxStream << fileQuery.value(0).toString() << "\t"   // file_full_path
                      << fileQuery.value(1).toString() << "\t"   // file_size
                      << fileQuery.value(2).toString() << "\t"   // file_date_updated
                      << fileQuery.value(3).toString() << "\t"   // file_extension
                      << fileQuery.value(4).toString() << "\t"   // file_type
                      << fileQuery.value(5).toString() << "\t"   // mime_type
                      << fileQuery.value(6).toString() << "\t"   // mime_verified
                      << fileQuery.value(7).toString() << "\t"   // type_mismatch
                      << fileQuery.value(8).toString() << "\t"   // image_width
                      << fileQuery.value(9).toString() << "\t"   // image_height
                      << fileQuery.value(10).toString() << "\t"  // image_orientation
                      << fileQuery.value(11).toString() << "\t"  // video_duration_seconds
                      << fileQuery.value(12).toString() << "\t"  // video_width
                      << fileQuery.value(13).toString() << "\t"  // video_height
                      << fileQuery.value(14).toString() << "\t"  // video_codec
                      << fileQuery.value(15).toString() << "\t"  // video_framerate
                      << fileQuery.value(16).toString() << "\t"  // video_bitrate
                      << fileQuery.value(17).toString() << "\t"  // audio_duration_seconds
                      << fileQuery.value(18).toString() << "\t"  // audio_artist
                      << fileQuery.value(19).toString() << "\t"  // audio_album
                      << fileQuery.value(20).toString() << "\t"  // audio_title
                      << fileQuery.value(21).toString() << "\t"  // audio_genre
                      << fileQuery.value(22).toString() << "\t"  // audio_year
                      << fileQuery.value(23).toString() << "\t"  // audio_track_number
                      << fileQuery.value(24).toString() << "\t"  // audio_bitrate
                      << fileQuery.value(25).toString() << "\t"  // audio_sample_rate
                      << fileQuery.value(26).toString() << "\t"  // metadata_extended
                      << fileQuery.value(27).toString() << "\n";  // metadata_extraction_date
        }

        idxFile.close();

        // Export folders file (.folders.idx)
        exportSingleCatalogFoldersFile(catalogId, collection->folder + "/" + deviceName + ".folders.idx");
    }

    return true;
}

bool MainWindow::exportSingleCatalogFoldersFile(int catalogId, const QString &filePath)
{
    // Check if there are any folders for this catalog
    QSqlQuery checkQuery(QSqlDatabase::database(m_connectionName));
    checkQuery.prepare("SELECT COUNT(*) FROM folder WHERE folder_catalog_id = :catalog_id");
    checkQuery.bindValue(":catalog_id", catalogId);

    if (!checkQuery.exec() || !checkQuery.next()) {
        qDebug() << "Folder count query failed:" << checkQuery.lastError().text();
        return false;
    }

    int folderCount = checkQuery.value(0).toInt();

    if (folderCount == 0) {
        // No folders to export
        return true;
    }

    // Get folders
    QSqlQuery folderQuery(QSqlDatabase::database(m_connectionName));
    folderQuery.prepare("SELECT folder_catalog_id, folder_path FROM folder WHERE folder_catalog_id = :catalog_id ORDER BY folder_path");
    folderQuery.bindValue(":catalog_id", catalogId);

    if (!folderQuery.exec()) {
        qDebug() << "Folder query failed:" << folderQuery.lastError().text();
        return false;
    }

    // Open folders file
    QFile foldersFile(filePath);
    if (!foldersFile.open(QIODevice::WriteOnly | QIODevice::Text)) {
        qDebug() << "Cannot open folders file for writing:" << filePath;
        return false;
    }

    QTextStream foldersStream(&foldersFile);

    // Write folder entries
    while (folderQuery.next()) {
        foldersStream << folderQuery.value(0).toString() << "\t"
                      << folderQuery.value(1).toString() << "\n";
    }

    foldersFile.close();
    return true;
}

