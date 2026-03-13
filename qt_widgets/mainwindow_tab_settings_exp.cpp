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

        // Load all catalog indexes into memory, with progress
        QProgressDialog progress("Loading devices...", "Cancel", 0, 100, this);
        progress.setWindowModality(Qt::WindowModal);

        bool cancelled = !collection->loadAllCatalogFiles(
            [&progress](int filesLoaded, int totalFiles, const QString &deviceName) -> bool {
                int percent = totalFiles > 0 ? (filesLoaded * 100) / totalFiles : 0;
                progress.setValue(percent);
                progress.setLabelText(
                    QCoreApplication::translate("MainWindow",
                        "Loading all catalogs prior to export<br/> %1 <br/><br/> %2 files loaded out of %3")
                        .arg(deviceName, QLocale().toString(filesLoaded), QLocale().toString(totalFiles)));
                return !progress.wasCanceled();
            });

        if (cancelled)
            return;

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
        QString exportFolderPath = collection->folder;

        QString selectedExportFolder = QFileDialog::getExistingDirectory(this,
                                                                         tr("Select the directory for the CSV export"),
                                                                         exportFolderPath);

        if (selectedExportFolder.isEmpty()) {
            return;
        }

        exportFolderPath = selectedExportFolder;

        // Create a progress dialog
        QProgressDialog progress("Exporting database to CSV files...", "Cancel", 0, 100, this);
        progress.setWindowModality(Qt::WindowModal);
        progress.setValue(0);

        try {
            // Export all tables via core
            progress.setValue(10);
            progress.setLabelText("Exporting tables...");
            collection->exportAllToMemoryMode(exportFolderPath);

            progress.setValue(40);

            // Export catalogs to idx files via core
            collection->exportAllCatalogFiles(exportFolderPath,
                [&progress](int current, int total, const QString &catalogName) -> bool {
                    int progressValue = 40 + (current * 55) / total;
                    progress.setValue(progressValue);
                    progress.setLabelText(QString("Exporting catalog: %1 (%2/%3)")
                                              .arg(catalogName)
                                              .arg(current)
                                              .arg(total));
                    return !progress.wasCanceled();
                });

            progress.setValue(100);

            msgBox.setIcon(QMessageBox::Information);
            msgBox.setText(tr("Successfully exported database to CSV files.<br/>"
                              "<br/>Export directory: <br/><b>%1</b><br/>"
                              "<br/>You can now switch to Memory mode and load this collection.")
                               .arg(exportFolderPath));
            msgBox.exec();
        }
        catch (const std::exception& e) {
            msgBox.setIcon(QMessageBox::Warning);
            msgBox.setText(tr("Export failed: %1").arg(e.what()));
            msgBox.exec();
        }
    }
}

bool MainWindow::exportAllCatalogFiles(QProgressDialog &progress)
{
    return collection->exportAllCatalogFiles(collection->folder,
        [&progress](int current, int total, const QString &catalogName) -> bool {
            int progressValue = 20 + (current * 60) / total;
            progress.setValue(progressValue);
            progress.setLabelText(QString("Exporting catalog: %1 (%2/%3)")
                                      .arg(catalogName)
                                      .arg(current)
                                      .arg(total));
            return !progress.wasCanceled();
        });
}

bool MainWindow::exportSingleCatalogFoldersFile(int catalogId, const QString &filePath)
{
    return collection->exportSingleCatalogFoldersFile(catalogId, filePath);
}

