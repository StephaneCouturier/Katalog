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
// Purpose:     methods for the Settings panel, collection export, and collection import features
// Description: https://stephanecouturier.github.io/Katalog/docs/Features/Settings
// Author:      Stephane Couturier
/////////////////////////////////////////////////////////////////////////////
*/

#include "mainwindow.h"
#include "ui_mainwindow.h"
#include "core/collectionimporter.h"
#include "core/statusbarmessagebuilder.h"
#include "devicetreeview.h"

#include <QCoreApplication>
#include <QElapsedTimer>
#include <QFileDialog>

// Export to SQLite File mode
// Supported source modes: Memory, Hosted
void MainWindow::exportToSQLiteFile()
{
    const bool isMemory = (collection->databaseMode == "Memory");
    const bool isHosted = (collection->databaseMode == "Hosted");

    if (!isMemory && !isHosted)
        return;

    QMessageBox msgBox;
    msgBox.setWindowTitle("Katalog");

    // Default output path
    QString backupFilePath = collection->folder + "/export.db";

    QString selectedBackupFilePath = QFileDialog::getSaveFileName(
        this,
        tr("Select"),
        backupFilePath);

    if (selectedBackupFilePath.isEmpty())
        return;

    backupFilePath = selectedBackupFilePath;

    QFile existingFile(backupFilePath);
    if (existingFile.exists())
        existingFile.moveToTrash();

    bool ok = false;

    if (isMemory) {
        // Memory → SQLite: load all catalog indexes into memory first, then backup
        QProgressDialog progress(tr("Loading"), tr("Cancel"), 0, 100, this);
        progress.setWindowModality(Qt::WindowModal);

        bool cancelled = !collection->loadAllCatalogFiles(
            [&progress](int filesLoaded, int totalFiles, const QString &deviceName) -> bool {
                int percent = totalFiles > 0 ? (filesLoaded * 100) / totalFiles : 0;
                progress.setValue(percent);
                progress.setLabelText(tr("Loading") + " " + deviceName + " (" + QLocale().toString(filesLoaded) + "/" + QLocale().toString(totalFiles) + ")");
                return !progress.wasCanceled();
            });

        if (cancelled)
            return;

        ok = backupMemoryDatabaseToFile(m_connectionName, backupFilePath);

    } else {
        // Hosted → SQLite: copy all tables to a fresh SQLite file
        QProgressDialog progress(tr("Export"), tr("Cancel"), 0, 100, this);
        progress.setWindowModality(Qt::WindowModal);
        progress.setValue(0);

        ok = collection->exportToSQLiteFile(backupFilePath,
            [&progress](int current, int total, const QString &tableName) -> bool {
                int percent = total > 0 ? (current * 100) / total : 0;
                progress.setValue(percent);
                progress.setLabelText(tr("Export") + " " + tableName + " (" + QString::number(current) + "/" + QString::number(total) + ")");
                return !progress.wasCanceled();
            });
    }

    if (!ok) {
        msgBox.setIcon(QMessageBox::Warning);
        msgBox.setText(tr("Export") + "<br/><br/>" + backupFilePath);
    } else {
        msgBox.setIcon(QMessageBox::Information);
        msgBox.setText(tr("Export") + "<br/><br/>" + backupFilePath);
    }

    msgBox.exec();
}

// Export to Memory mode (CSV folder)
// Supported source modes: File, Hosted
void MainWindow::exportToMemoryMode()
{
    const bool isFile   = (collection->databaseMode == "File");
    const bool isHosted = (collection->databaseMode == "Hosted");

    if (!isFile && !isHosted)
        return;

    QMessageBox msgBox;
    msgBox.setWindowTitle("Katalog");

    QString exportFolderPath = collection->folder;

    QString selectedExportFolder = QFileDialog::getExistingDirectory(
        this,
        tr("Select a folder"),
        exportFolderPath);

    if (selectedExportFolder.isEmpty())
        return;

    exportFolderPath = selectedExportFolder;

    QProgressDialog progress(tr("Export"), tr("Cancel"), 0, 100, this);
    progress.setWindowModality(Qt::WindowModal);
    progress.setValue(0);

    try {
        // Export all CSV table files
        progress.setValue(10);
        progress.setLabelText(tr("Export"));
        collection->exportAllToMemoryMode(exportFolderPath);

        progress.setValue(40);

        // Export per-catalog .idx files
        collection->exportAllCatalogFiles(exportFolderPath,
            [&progress](int current, int total, const QString &catalogName) -> bool {
                int progressValue = 40 + (current * 55) / total;
                progress.setValue(progressValue);
                progress.setLabelText(tr("Export") + " " + catalogName + " (" + QString::number(current) + "/" + QString::number(total) + ")");
                return !progress.wasCanceled();
            });

        progress.setValue(100);

        msgBox.setIcon(QMessageBox::Information);
        msgBox.setText(tr("Export") + "<br/><br/>" + exportFolderPath);
        msgBox.exec();
    }
    catch (const std::exception& e) {
        msgBox.setIcon(QMessageBox::Warning);
        msgBox.setText(tr("Export") + ": " + tr("Error") + "\n" + QString::fromStdString(e.what()));
        msgBox.exec();
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

//----------------------------------------------------------------------
// Collection Import / Update
//----------------------------------------------------------------------

void MainWindow::refreshImportUpdateSourceList()
{
    QStringList paths = collection->getImportSourcePaths();
    ui->Import_comboBox_updateSource->clear();
    ui->Import_comboBox_updateSource->addItems(paths);
    ui->Import_pushButton_updateSelected->setEnabled(!paths.isEmpty());
}

//----------------------------------------------------------------------
// In Memory mode, the import writes to the in-memory SQLite but loadCollection()
// wipes and reloads from CSV files.  Persist the affected tables to disk first.
static void persistImportToFiles(Collection *collection)
{
    if (collection->databaseMode != "Memory")
        return;
    collection->saveDeviceTableToFile();
    collection->saveMappingTableToFile();
    collection->saveCatalogFilterTableToFile();
    collection->saveStorageTableToFile();
    collection->saveTagTableToFile();
    collection->saveStatiticsTableToFile();
}

//----------------------------------------------------------------------
void MainWindow::openImportSource(const QString &path)
{
    ui->Import_lineEdit_sourcePath->setText(path);

    if (!m_importer)
        m_importer = new CollectionImporter(collection, this);

    QSqlError err = m_importer->openSource(path);
    if (err.type() != QSqlError::NoError) {
        updateStatusBarMessage(
            StatusBarMessageBuilder()
                .setOperation(tr("Collection Import"))
                .setStatus(tr("Error"))
                .setCurrentItem(err.text())
                .build());
        return;
    }

    if (!m_importer->checkSchemaCompatibility()) {
        updateStatusBarMessage(
            StatusBarMessageBuilder()
                .setOperation(tr("Collection Import"))
                .setStatus(tr("Error"))
                .setCurrentItem(tr("Schema version mismatch. Import cancelled. (%1)").arg(m_importer->lastError()))
                .build());
        m_importer->close();
        return;
    }

    QStandardItemModel *model = m_importer->buildSourceDeviceModel();
    DeviceTreeView *proxy = new DeviceTreeView(this);
    proxy->setSourceModel(model);
    proxy->setKatalogTheme(themeID > 0);
    ui->Import_comboBox_sourceDevice->setTreeModel(proxy);
    ui->Import_comboBox_sourceDevice->setIdColumn(3);  // col 3 = DEVICE_ID in DeviceTreeColumns layout
    ui->Import_comboBox_sourceDevice->expandAll();
}

//----------------------------------------------------------------------
void MainWindow::on_Import_pushButton_select_clicked()
{
    QString startDir = ui->Import_lineEdit_sourcePath->text().trimmed();
    if (startDir.isEmpty())
        startDir = collection->folder;

    QString path;

    if (ui->Import_comboBox_mode->currentIndex() == 0) { // File mode
        path = QFileDialog::getOpenFileName(
            this,
            tr("Select"),
            startDir);
    } else {
        path = QFileDialog::getExistingDirectory(
            this,
            tr("Select"),
            startDir);
    }

    if (path.isEmpty())
        return;

    openImportSource(path);
}

//----------------------------------------------------------------------
void MainWindow::on_Import_lineEdit_sourcePath_returnPressed()
{
    QString path = ui->Import_lineEdit_sourcePath->text().trimmed();
    if (!path.isEmpty())
        openImportSource(path);
}

//----------------------------------------------------------------------
void MainWindow::on_Import_pushButton_importSelected_clicked()
{
    if (!m_importer || !m_importer->isSourceOpen()) {
        updateStatusBarMessage(
            StatusBarMessageBuilder()
                .setOperation(tr("Collection Import"))
                .setStatus(tr("Error"))
                .setCurrentItem(tr("No source collection is open."))
                .build());
        return;
    }

    int srcDeviceId = ui->Import_comboBox_sourceDevice->selectedDeviceId();

    updateStatusBarMessage(
        StatusBarMessageBuilder()
            .setOperation(tr("Collection Import"))
            .setStatus(tr("In Progress"))
            .build());
    QApplication::processEvents();

    // Track elapsed time so we can compute ETA for large file tables.
    QElapsedTimer importTimer;
    importTimer.start();

    QMetaObject::Connection progressConn = connect(
        m_importer, &CollectionImporter::fileImportProgress,
        this, [this, &importTimer](int catalogIndex, int totalCatalogs,
                                   const QString &catalogName,
                                   qint64 done, qint64 total) {
            StatusBarMessageBuilder builder;
            builder.setOperation(tr("Collection Import"))
                   .setStatus(tr("In Progress"))
                   .setDeviceContext(catalogIndex, totalCatalogs, catalogName)
                   .setProcess(tr("Files"), done, total);

            // ETA — only after ≥ 500 ms to avoid wild numbers at the very start.
            const qint64 elapsedMs = importTimer.elapsed();
            if (elapsedMs >= 500 && total > 0 && done > 0) {
                const qint64 etaSec = (elapsedMs * (total - done)) / (done * 1000);
                QString etaStr;
                if (etaSec < 60)
                    etaStr = tr("%1s").arg(etaSec);
                else if (etaSec < 3600)
                    etaStr = tr("%1m %2s").arg(etaSec / 60).arg(etaSec % 60);
                else
                    etaStr = tr("%1h %2m").arg(etaSec / 3600).arg((etaSec % 3600) / 60);
                builder.setTimeToCompletion(tr("%1").arg(etaStr));
            }

            updateStatusBarMessage(builder.build());
            // Allow repaints without processing user-input events (avoids re-entrancy).
            QCoreApplication::processEvents(QEventLoop::ExcludeUserInputEvents);
        },
        Qt::DirectConnection);

    bool ok;
    if (srcDeviceId == 0) {
        // Synthetic "Collection" root — import all root-level devices
        ok = (m_importer->importAllDevices() >= 0);
    } else {
        ok = m_importer->importDevice(srcDeviceId);
    }

    disconnect(progressConn);

    if (ok) {
        updateStatusBarMessage(
            StatusBarMessageBuilder()
                .setOperation(tr("Collection Import"))
                .setStatus(tr("Completed"))
                .build());
        persistImportToFiles(collection);
        loadCollection();
        refreshImportUpdateSourceList();
    } else {
        updateStatusBarMessage(
            StatusBarMessageBuilder()
                .setOperation(tr("Collection Import"))
                .setStatus(tr("Error"))
                .setCurrentItem(m_importer->lastError())
                .build());
    }
}

//----------------------------------------------------------------------
void MainWindow::on_Import_pushButton_updateSelected_clicked()
{
    QString sourcePath = ui->Import_comboBox_updateSource->currentText().trimmed();

    if (!m_importer)
        m_importer = new CollectionImporter(collection, this);

    updateStatusBarMessage(
        StatusBarMessageBuilder()
            .setOperation(tr("Collection Update"))
            .setStatus(tr("In Progress"))
            .build());
    QApplication::processEvents();

    bool ok = m_importer->updateAllImportsFromSource(sourcePath);

    if (ok) {
        updateStatusBarMessage(
            StatusBarMessageBuilder()
                .setOperation(tr("Collection Update"))
                .setStatus(tr("Completed"))
                .build());
        persistImportToFiles(collection);
        loadCollection();
    } else {
        updateStatusBarMessage(
            StatusBarMessageBuilder()
                .setOperation(tr("Collection Update"))
                .setStatus(tr("Error"))
                .setCurrentItem(m_importer->lastError())
                .build());
    }
}
