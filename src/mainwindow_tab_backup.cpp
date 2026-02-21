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
// File Name:   mainwindow_tab_backup.cpp
// Purpose:     methods for the screen CREATE
// Description: https://stephanecouturier.github.io/Katalog/docs/Features/Create
// Author:      Stephane Couturier
/////////////////////////////////////////////////////////////////////////////
*/

#include "core/backupprofilegenerator.h"
#include "core/backupjobstoppable.h"
#include <QTimer>
#include "core/catalogdifferenceengine.h"
#include "core/directoryreplicator.h"
#include "core/statusbarmessagebuilder.h"
#include "mainwindow.h"
#include "devicemappingview.h"
#include "ui_mainwindow.h"
#include "core/database.h"

//UI----------------------------------------------------------------------------

void MainWindow::on_BackUp_pushButton_SaveMapping_clicked()
{
    saveNewMapping();
    collection->saveMappingTableToFile();
}
void MainWindow::on_BackUp_pushButton_ReloadSourceList_clicked()
{
    loadBackUpDeviceLists("Source");
}

void MainWindow::on_BackUp_pushButton_ReloadSourceListWithoutMapping_clicked()
{
    loadBackUpDeviceLists("Source_without_mapping");
}

void MainWindow::on_BackUp_pushButton_ReloadTargetList_clicked()
{
    loadBackUpDeviceLists("Target");
}

void MainWindow::on_BackUp_pushButton_ReloadTargetListWithoutMapping_clicked()
{
    loadBackUpDeviceLists("Target_without_mapping");
}

void MainWindow::on_BackUp_pushButton_ReloadDeviceMappings_clicked()
{
    loadBackUpMapping();
}

void MainWindow::on_BackUp_pushButton_DeleteSelectedMapping_clicked()
{
    //Get the selected mapping_id
    QModelIndexList selectedIndexes = ui->BackUp_tableView_CurrentMappings->selectionModel()->selectedIndexes();
    int mappingID = selectedIndexes.at(0).data().toInt();

    //Delete the mapping via core manager
    if (!backupMappingManager) {
        backupMappingManager = new BackupMappingManager(m_connectionName, this);
    }

    if (!backupMappingManager->deleteMapping(mappingID)) {
        return;
    }

    //Reload the mapping table
    loadBackUpMapping();
    collection->saveMappingTableToFile();
}

void MainWindow::on_BackUp_pushButton_ReplicateDirectories_clicked()
{
    //Get the selected mapping_id
    QModelIndexList selectedIndexes = ui->BackUp_tableView_CurrentMappings->selectionModel()->selectedIndexes();
    if (selectedIndexes.isEmpty()) {
        QMessageBox::warning(this, "Katalog", tr("Select a mapping first."));
        return;
    }
    int mappingID = selectedIndexes.at(0).data().toInt();

    //Get mapping info (source and target device IDs and paths)
    if (!backupMappingManager) {
        backupMappingManager = new BackupMappingManager(m_connectionName, this);
    }
    MappingInfo mapping = backupMappingManager->getMappingById(mappingID);

    //Load source and target devices to get catalog IDs (externalID)
    Device sourceDevice;
    sourceDevice.ID = mapping.sourceDeviceId;
    sourceDevice.loadDevice(m_connectionName);

    Device targetDevice;
    targetDevice.ID = mapping.targetDeviceId;
    targetDevice.loadDevice(m_connectionName);

    //Validate both devices are catalogs with valid paths
    if (sourceDevice.type != "Catalog" || targetDevice.type != "Catalog") {
        QMessageBox::warning(this, "Katalog",
                             tr("Both source and target must be Catalog devices."));
        return;
    }

    if (!QDir(sourceDevice.path).exists()) {
        QMessageBox::warning(this, "Katalog",
                             tr("Source path is not accessible: %1").arg(sourceDevice.path));
        return;
    }

    if (!QDir(targetDevice.path).exists()) {
        QMessageBox::warning(this, "Katalog",
                             tr("Target path is not accessible: %1").arg(targetDevice.path));
        return;
    }

    //In Memory mode, load the source catalog's folders into the in-memory database first
    if (collection->databaseMode == "Memory") {
        sourceDevice.catalog->loadFoldersToTable();
    }

    //Replicate the directories from source catalog to target catalog
    DirectoryReplicator replicator(m_connectionName);
    ReplicationResult result = replicator.replicate(
        {sourceDevice.externalID},
        sourceDevice.path,
        targetDevice.path
    );

    //Display result
    QMessageBox::information(this, "Katalog",
                             tr("Directory replication completed.\n\n"
                                "Directories created: %1\n"
                                "Directories already existing: %2\n"
                                "Errors: %3")
                                 .arg(result.createdCount())
                                 .arg(result.skippedCount())
                                 .arg(result.errorCount()));
}

void MainWindow::on_BackUp_pushButton_BackUpPreview_clicked()
{
    loadBackupPreview();
}

void MainWindow::on_BackUp_pushButton_RunBackup_clicked()
{
    runBackup();
}

void MainWindow::on_BackUp_pushButton_CancelBackup_clicked()
{
    if (m_backupJob)
        m_backupJob->stopBackup();
}

// ─── Shared comparison helper ──────────────────────────────────────────────

MainWindow::BackupCompareResult MainWindow::compareForBackup(
    const Device &sourceDevice, const Device &targetDevice, bool strictCopy)
{
    BackupCompareResult out;

    if (strictCopy) {
        // PATH-AWARE (strict mirror): delegate to CatalogDifferenceEngine::compareStrict()
        CatalogDifferenceEngine engine(m_connectionName);
        StrictDifferenceResult r = engine.compareStrict(
            sourceDevice.externalID,
            targetDevice.externalID,
            sourceDevice.path,
            targetDevice.path
        );
        out.filesToCopy   = r.filesToCopy;
        out.fileConflicts = r.conflicts;
        out.skippedCount  = r.skippedCount;

    } else {
        // DEDUP mode (original): use CatalogDifferenceEngine — a file is skipped
        // if an identical name+size exists ANYWHERE in the target, regardless of path.
        CatalogDifferenceEngine engine(m_connectionName);
        QList<int> sourceIds = CatalogDifferenceEngine::resolveCatalogDeviceIds(
                                   const_cast<Device*>(&sourceDevice), m_connectionName);
        QList<int> targetIds = CatalogDifferenceEngine::resolveCatalogDeviceIds(
                                   const_cast<Device*>(&targetDevice), m_connectionName);

        DifferenceResult result = engine.compare(
            sourceIds, targetIds,
            CatalogDifferenceEngine::Name | CatalogDifferenceEngine::Size,
            false,
            "file"
        );

        // Build a set of all target file names to classify conflicts
        QSet<QString> targetFileNames;
        {
            QSqlQuery q(QSqlDatabase::database(m_connectionName));
            q.prepare(QString("SELECT DISTINCT file_name FROM file WHERE file_catalog_id = %1")
                          .arg(targetDevice.externalID));
            if (q.exec()) {
                while (q.next())
                    targetFileNames.insert(q.value(0).toString());
            }
        }

        for (const DifferenceFileEntry &entry : result.onlyInSource) {
            if (targetFileNames.contains(entry.fileName))
                out.fileConflicts.append(entry);
            else
                out.filesToCopy.append(entry);
        }

        // Skipped = files in source that are in both (dedup engine removed them)
        {
            QSqlQuery q(QSqlDatabase::database(m_connectionName));
            q.prepare(QString("SELECT COUNT(*) FROM file WHERE file_catalog_id = %1")
                          .arg(sourceDevice.externalID));
            if (q.exec() && q.next())
                out.skippedCount = q.value(0).toInt()
                                   - out.filesToCopy.size()
                                   - out.fileConflicts.size();
        }
    }

    return out;
}

// ─── Backup executor ───────────────────────────────────────────────────────

void MainWindow::runBackup()
{
    //Get the selected mapping_id
    QModelIndexList selectedIndexes = ui->BackUp_tableView_CurrentMappings->selectionModel()->selectedIndexes();
    if (selectedIndexes.isEmpty()) {
        QMessageBox::warning(this, "Katalog", tr("Select a mapping first."));
        return;
    }
    int mappingID = selectedIndexes.at(0).data().toInt();

    if (!backupMappingManager)
        backupMappingManager = new BackupMappingManager(m_connectionName, this);
    MappingInfo mapping = backupMappingManager->getMappingById(mappingID);

    Device sourceDevice;
    sourceDevice.ID = mapping.sourceDeviceId;
    sourceDevice.loadDevice(m_connectionName);

    Device targetDevice;
    targetDevice.ID = mapping.targetDeviceId;
    targetDevice.loadDevice(m_connectionName);

    if (sourceDevice.type != "Catalog" || targetDevice.type != "Catalog") {
        QMessageBox::warning(this, "Katalog",
                             tr("Both source and target must be Catalog devices."));
        return;
    }

    if (!QDir(targetDevice.path).exists()) {
        QMessageBox::warning(this, "Katalog",
                             tr("Target path is not accessible: %1").arg(targetDevice.path));
        return;
    }

    // Optionally update both catalogs from filesystem before comparing
    if (ui->BackUp_checkBox_UpdateBeforeBackup->isChecked()) {
        if (deviceUpdateManager->operationRunning()) {
            QMessageBox::warning(this, "Katalog",
                tr("A catalog update is already in progress. Please wait and try again."));
            return;
        }
        m_pendingBackupMappingId    = mappingID;
        m_pendingBackupSourceDevice = sourceDevice;
        m_pendingBackupTargetDevice = targetDevice;
        m_backupUpdatePhase         = BackupUpdatePhase::UpdatingSource;

        ui->BackUp_pushButton_RunBackup->setEnabled(false);
        ui->BackUp_label_ExecutionStatus->setVisible(true);
        ui->BackUp_label_ExecutionStatus->setText(
            StatusBarMessageBuilder()
                .setOperation(tr("Backup"))
                .setStatus(tr("Updating source catalog…"))
                .build());

        setupDeviceUpdateManagerForBackup();
        deviceUpdateManager->updateDeviceHierarchy(
            &m_pendingBackupSourceDevice, collection->databaseMode, collection->folder, "update");
        return;
    }

    executeBackup(sourceDevice, targetDevice, mapping);
}

void MainWindow::setupDeviceUpdateManagerForBackup()
{
    // Temporarily replace normal device-tab connections with backup-specific ones
    disconnect(deviceUpdateManager, nullptr, this, nullptr);
    connect(deviceUpdateManager, &DeviceUpdateManager::operationCompleted,
            this, &MainWindow::continueBackupAfterCatalogUpdate);
    connect(deviceUpdateManager, &DeviceUpdateManager::operationError,
            this, [this](const QString &error) {
                m_backupUpdatePhase      = BackupUpdatePhase::None;
                m_pendingBackupMappingId = -1;
                setupDeviceUpdateManager();  // restore normal device-tab connections
                ui->BackUp_pushButton_RunBackup->setEnabled(true);
                ui->BackUp_label_ExecutionStatus->setVisible(false);
                QMessageBox::warning(this, "Katalog",
                    tr("Catalog update failed: %1").arg(error));
            });
    connect(deviceUpdateManager, &DeviceUpdateManager::operationCancelled,
            this, [this]() {
                m_backupUpdatePhase      = BackupUpdatePhase::None;
                m_pendingBackupMappingId = -1;
                setupDeviceUpdateManager();
                ui->BackUp_pushButton_RunBackup->setEnabled(true);
                ui->BackUp_label_ExecutionStatus->setVisible(false);
            });
}

void MainWindow::continueBackupAfterCatalogUpdate()
{
    if (m_backupUpdatePhase == BackupUpdatePhase::UpdatingSource) {
        // Source done — now update target.
        // NOTE: DeviceUpdateManager defers cleanupOperation() 10 ms after emitting
        // operationCompleted. Starting the target update synchronously here would
        // have that cleanup fire mid-operation and corrupt DeviceUpdateManager state.
        // Delay past the cleanup window (>10 ms) so the manager is fully reset first.
        m_backupUpdatePhase = BackupUpdatePhase::UpdatingTarget;
        ui->BackUp_label_ExecutionStatus->setText(
            StatusBarMessageBuilder()
                .setOperation(tr("Backup"))
                .setStatus(tr("Updating target catalog…"))
                .build());
        QTimer::singleShot(50, this, [this]() {
            deviceUpdateManager->updateDeviceHierarchy(
                &m_pendingBackupTargetDevice, collection->databaseMode, collection->folder, "update");
        });
        return;
    }

    // Both catalogs updated — restore normal device connections then execute
    m_backupUpdatePhase = BackupUpdatePhase::None;
    setupDeviceUpdateManager();  // restore normal device-tab connections

    if (!backupMappingManager)
        backupMappingManager = new BackupMappingManager(m_connectionName, this);
    MappingInfo mapping = backupMappingManager->getMappingById(m_pendingBackupMappingId);
    m_pendingBackupMappingId = -1;

    // Reload device data so the updated catalog IDs/stats are current
    m_pendingBackupSourceDevice.loadDevice(m_connectionName);
    m_pendingBackupTargetDevice.loadDevice(m_connectionName);

    executeBackup(m_pendingBackupSourceDevice, m_pendingBackupTargetDevice, mapping);
}

void MainWindow::executeBackup(Device sourceDevice, Device targetDevice, MappingInfo mapping)
{
    // Save target device so onBackupFinished() can update the target catalog afterward
    m_pendingBackupTargetDevice = targetDevice;

    //Memory mode: load file data into 'file' table first
    if (collection->databaseMode == "Memory") {
        QMutex mutex;
        bool stopRequested = false;
        sourceDevice.catalog->loadCatalogFileListToTable(mutex, stopRequested);
        targetDevice.catalog->loadCatalogFileListToTable(mutex, stopRequested);
    }

    //Run comparison (respects strictCopy setting)
    BackupCompareResult cmp = compareForBackup(sourceDevice, targetDevice, mapping.strictCopy);

    if (cmp.filesToCopy.isEmpty() && cmp.fileConflicts.isEmpty()) {
        ui->BackUp_pushButton_RunBackup->setEnabled(true);
        ui->BackUp_label_ExecutionStatus->setVisible(false);
        QMessageBox::information(this, "Katalog",
                                 tr("Source and target are already in sync. Nothing to copy."));
        return;
    }

    //Setup and launch the backup worker thread
    m_backupJob = new BackupJobStoppable();
    m_backupJob->setFiles(cmp.filesToCopy);
    m_backupJob->setSourcePath(sourceDevice.path);
    m_backupJob->setTargetPath(targetDevice.path);

    m_backupThread = new QThread(this);
    m_backupJob->moveToThread(m_backupThread);

    connect(m_backupThread, &QThread::started,
            m_backupJob,    &BackupJobStoppable::runBackup);
    connect(m_backupJob,    &BackupJobStoppable::backupProgress,
            this,           &MainWindow::onBackupProgress);
    connect(m_backupJob,    &BackupJobStoppable::backupFinished,
            this,           &MainWindow::onBackupFinished);
    connect(m_backupJob,    &BackupJobStoppable::backupFinished,
            m_backupThread, &QThread::quit);
    connect(m_backupThread, &QThread::finished,
            m_backupJob,    &QObject::deleteLater);
    connect(m_backupThread, &QThread::finished,
            m_backupThread, &QObject::deleteLater);

    //Show execution panel, disable Run button
    // Use a 0–1000 scale so the bar works with large byte counts (qint64 → int safe)
    ui->BackUp_progressBar->setMinimum(0);
    ui->BackUp_progressBar->setMaximum(1000);
    ui->BackUp_progressBar->setValue(0);
    ui->BackUp_label_ExecutionStatus->setText(tr("Starting backup…"));
    m_backupTimer.start();
    ui->BackUp_label_ExecutionStatus->setVisible(true);
    ui->BackUp_progressBar->setVisible(true);
    ui->BackUp_pushButton_CancelBackup->setVisible(true);
    ui->BackUp_pushButton_RunBackup->setEnabled(false);
    ui->BackUp_pushButton_CancelBackup->setEnabled(true);

    m_backupThread->start();
}

void MainWindow::onBackupProgress(int filesDone, int totalFiles,
                                   qint64 bytesCopied, qint64 totalBytes,
                                   const QString &currentFile)
{
    // Progress bar: 0–1000 byte-based scale (more accurate than file count for mixed sizes)
    if (totalBytes > 0)
        ui->BackUp_progressBar->setValue(static_cast<int>(bytesCopied * 1000 / totalBytes));

    // Speed and ETA — only after ≥ 500 ms to avoid wild numbers at the very start
    const qint64 elapsedMs = m_backupTimer.elapsed();
    double speedBps = -1.0;
    QString etaStr;

    if (elapsedMs >= 500 && bytesCopied > 0) {
        speedBps = static_cast<double>(bytesCopied) / (elapsedMs / 1000.0);

        if (speedBps > 0 && totalBytes > bytesCopied) {
            const qint64 etaSec = static_cast<qint64>((totalBytes - bytesCopied) / speedBps);
            if (etaSec < 60)
                etaStr = tr("%1s").arg(etaSec);
            else if (etaSec < 3600)
                etaStr = tr("%1m %2s").arg(etaSec / 60).arg(etaSec % 60);
            else
                etaStr = tr("%1h %2m").arg(etaSec / 3600).arg((etaSec % 3600) / 60);
        }
    }

    const QString msg = StatusBarMessageBuilder()
        .setOperation(tr("backup"))
        .setProcess(tr("Copying"), filesDone + 1, totalFiles)
        .setSizeProgress(bytesCopied, totalBytes)
        .setSpeed(speedBps)
        .setTimeToCompletion(etaStr)
        .setCurrentItem(currentFile)
        .build();

    ui->BackUp_label_ExecutionStatus->setText(msg);
}

void MainWindow::onBackupFinished(const BackupReport &report)
{
    m_backupJob    = nullptr;
    m_backupThread = nullptr;

    ui->BackUp_pushButton_RunBackup->setEnabled(true);
    ui->BackUp_pushButton_CancelBackup->setEnabled(false);
    ui->BackUp_progressBar->setValue(ui->BackUp_progressBar->maximum());

    // Format total elapsed time
    const qint64 elapsedSec = m_backupTimer.elapsed() / 1000;
    QString elapsedStr;
    if (elapsedSec < 60)
        elapsedStr = tr("%1s").arg(elapsedSec);
    else if (elapsedSec < 3600)
        elapsedStr = tr("%1m %2s").arg(elapsedSec / 60).arg(elapsedSec % 60);
    else
        elapsedStr = tr("%1h %2m").arg(elapsedSec / 3600).arg((elapsedSec % 3600) / 60);

    const QString msg = StatusBarMessageBuilder()
        .setOperation(tr("backup"))
        .setStatus(report.wasCancelled ? tr("Cancelled") : tr("Complete"))
        .setSizeProgress(report.totalBytesCopied, report.totalBytesCopied)
        .setTimeToCompletion(elapsedStr)
        .build();
    ui->BackUp_label_ExecutionStatus->setText(msg);

    showBackupReport(report);

    // Update the target catalog to reflect the newly copied files
    if (!report.wasCancelled
            && ui->BackUp_checkBox_UpdateBeforeBackup->isChecked()
            && !deviceUpdateManager->operationRunning()) {

        ui->BackUp_label_ExecutionStatus->setText(
            StatusBarMessageBuilder()
                .setOperation(tr("Backup"))
                .setStatus(tr("Updating target catalog…"))
                .build());

        disconnect(deviceUpdateManager, nullptr, this, nullptr);
        connect(deviceUpdateManager, &DeviceUpdateManager::operationCompleted,
                this, [this, msg]() {
                    setupDeviceUpdateManager();
                    ui->BackUp_label_ExecutionStatus->setText(msg);
                });
        connect(deviceUpdateManager, &DeviceUpdateManager::operationError,
                this, [this](const QString&) { setupDeviceUpdateManager(); });
        connect(deviceUpdateManager, &DeviceUpdateManager::operationCancelled,
                this, [this]() { setupDeviceUpdateManager(); });

        deviceUpdateManager->updateDeviceHierarchy(
            &m_pendingBackupTargetDevice, collection->databaseMode, collection->folder, "update");
    }
}

void MainWindow::showBackupReport(const BackupReport &report)
{
    //Replace the preview table with the backup report
    QStandardItemModel *model = new QStandardItemModel(this);
    model->setHorizontalHeaderItem(0, new QStandardItem(tr("Status")));
    model->setHorizontalHeaderItem(1, new QStandardItem(tr("File Name")));
    model->setHorizontalHeaderItem(2, new QStandardItem(tr("Path")));
    model->setHorizontalHeaderItem(3, new QStandardItem(tr("Size")));

    for (const DifferenceFileEntry &e : report.copied) {
        QList<QStandardItem*> row;
        row << new QStandardItem(tr("copied"))
            << new QStandardItem(e.fileName)
            << new QStandardItem(e.folderPath)
            << new QStandardItem(QLocale().formattedDataSize(e.fileSize));
        model->appendRow(row);
    }
    for (const DifferenceFileEntry &e : report.conflicts) {
        QList<QStandardItem*> row;
        row << new QStandardItem(tr("conflict — skipped"))
            << new QStandardItem(e.fileName)
            << new QStandardItem(e.folderPath)
            << new QStandardItem(QLocale().formattedDataSize(e.fileSize));
        model->appendRow(row);
    }
    for (const QString &err : report.errors) {
        QList<QStandardItem*> row;
        row << new QStandardItem(tr("error"))
            << new QStandardItem(err)
            << new QStandardItem(QString())
            << new QStandardItem(QString());
        model->appendRow(row);
    }

    ui->BackUp_tableView_PreviewFiles->setModel(model);
    ui->BackUp_tableView_PreviewFiles->setEditTriggers(QAbstractItemView::NoEditTriggers);
    ui->BackUp_tableView_PreviewFiles->setSelectionBehavior(QAbstractItemView::SelectRows);
    ui->BackUp_tableView_PreviewFiles->horizontalHeader()->setSectionResizeMode(QHeaderView::ResizeToContents);

    ui->BackUp_label_PreviewSummary->setText(
        tr("Report — Copied: <b>%1 file(s) (%2)</b>  |  Conflicts (skipped): <b>%3 file(s)</b>  |  Errors: <b>%4</b>")
            .arg(report.copiedCount())
            .arg(QLocale().formattedDataSize(report.totalBytesCopied))
            .arg(report.conflictCount())
            .arg(report.errorCount())
    );
    ui->BackUp_label_PreviewSummary->setVisible(true);
    ui->BackUp_tableView_PreviewFiles->setVisible(true);
}

void MainWindow::loadBackupPreview()
{
    //Get the selected mapping_id
    QModelIndexList selectedIndexes = ui->BackUp_tableView_CurrentMappings->selectionModel()->selectedIndexes();
    if (selectedIndexes.isEmpty()) {
        QMessageBox::warning(this, "Katalog", tr("Select a mapping first."));
        return;
    }
    int mappingID = selectedIndexes.at(0).data().toInt();

    //Get mapping info
    if (!backupMappingManager)
        backupMappingManager = new BackupMappingManager(m_connectionName, this);
    MappingInfo mapping = backupMappingManager->getMappingById(mappingID);

    //Load source and target devices
    Device sourceDevice;
    sourceDevice.ID = mapping.sourceDeviceId;
    sourceDevice.loadDevice(m_connectionName);

    Device targetDevice;
    targetDevice.ID = mapping.targetDeviceId;
    targetDevice.loadDevice(m_connectionName);

    if (sourceDevice.type != "Catalog" || targetDevice.type != "Catalog") {
        QMessageBox::warning(this, "Katalog",
                             tr("Both source and target must be Catalog devices."));
        return;
    }

    //Memory mode: load file data into 'file' table first
    if (collection->databaseMode == "Memory") {
        QMutex mutex;
        bool stopRequested = false;
        sourceDevice.catalog->loadCatalogFileListToTable(mutex, stopRequested);
        targetDevice.catalog->loadCatalogFileListToTable(mutex, stopRequested);
    }

    //Run comparison (respects strictCopy setting of this mapping)
    BackupCompareResult cmp = compareForBackup(sourceDevice, targetDevice, mapping.strictCopy);

    //Build preview model (Status: "to copy" / "conflict")
    QStandardItemModel *model = new QStandardItemModel(this);
    model->setHorizontalHeaderItem(0, new QStandardItem(tr("Status")));
    model->setHorizontalHeaderItem(1, new QStandardItem(tr("File Name")));
    model->setHorizontalHeaderItem(2, new QStandardItem(tr("Path")));
    model->setHorizontalHeaderItem(3, new QStandardItem(tr("Size")));

    qint64 copySize = 0;
    for (const DifferenceFileEntry &entry : cmp.filesToCopy) {
        QList<QStandardItem*> row;
        row << new QStandardItem(tr("to copy"))
            << new QStandardItem(entry.fileName)
            << new QStandardItem(entry.folderPath)
            << new QStandardItem(QLocale().formattedDataSize(entry.fileSize));
        model->appendRow(row);
        copySize += entry.fileSize;
    }

    qint64 conflictSize = 0;
    for (const DifferenceFileEntry &entry : cmp.fileConflicts) {
        QList<QStandardItem*> row;
        row << new QStandardItem(tr("conflict"))
            << new QStandardItem(entry.fileName)
            << new QStandardItem(entry.folderPath)
            << new QStandardItem(QLocale().formattedDataSize(entry.fileSize));
        model->appendRow(row);
        conflictSize += entry.fileSize;
    }

    ui->BackUp_tableView_PreviewFiles->setModel(model);
    ui->BackUp_tableView_PreviewFiles->setEditTriggers(QAbstractItemView::NoEditTriggers);
    ui->BackUp_tableView_PreviewFiles->setSelectionBehavior(QAbstractItemView::SelectRows);
    ui->BackUp_tableView_PreviewFiles->horizontalHeader()->setSectionResizeMode(QHeaderView::ResizeToContents);

    //Summary line — includes mode indicator
    const QString modeLabel = mapping.strictCopy ? tr("strict copy") : tr("dedup");
    ui->BackUp_label_PreviewSummary->setText(
        tr("Preview [%1] — To copy: <b>%2 file(s) (%3)</b>  |  Conflicts (will be skipped): <b>%4 file(s) (%5)</b>  |  Already in target: <b>%6 file(s)</b>")
            .arg(modeLabel)
            .arg(cmp.filesToCopy.size())
            .arg(QLocale().formattedDataSize(copySize))
            .arg(cmp.fileConflicts.size())
            .arg(QLocale().formattedDataSize(conflictSize))
            .arg(cmp.skippedCount)
    );

    //Show preview section
    ui->BackUp_label_PreviewSummary->setVisible(true);
    ui->BackUp_tableView_PreviewFiles->setVisible(true);
}

void MainWindow::on_BackUp_checkBox_DisplayFullTable_checkStateChanged(const Qt::CheckState &arg1)
{
    QSettings settings(collection->settingsFilePath, QSettings:: IniFormat);
    settings.setValue("BackUp/DisplayFullMappingTable", arg1);
    optionDisplayFullMappingTable = arg1;
    loadBackUpMapping();
}

void MainWindow::on_BackUp_radioButton_Source_clicked()
{
    QSettings settings(collection->settingsFilePath, QSettings:: IniFormat);
    settings.setValue("BackUp/FilterMappingTable", "Source");
}

void MainWindow::on_BackUp_radioButton_Target_clicked()
{
    QSettings settings(collection->settingsFilePath, QSettings:: IniFormat);
    settings.setValue("BackUp/FilterMappingTable", "Target");
}

void MainWindow::on_BackUp_pushButton_GenerateLuckyBackupProfile_clicked()
{
    // Initialization
    if (!backupMappingManager) {
        backupMappingManager = new BackupMappingManager(m_connectionName, this);
    }

    // Check if we have any mappings first
    if (backupMappingManager->getMappingCount() == 0) {
        QMessageBox::information(
            this,
            "Katalog",
            tr("No backup links found.")
            );
        return;
    }

    // Determine which mappings to use
    QList<int> mappingIds;
    bool useSelectedLinks = ui->BackUp_checkBox_OnlySelectedLinks->isChecked();

    if (useSelectedLinks) {
        // Use only filtered mappings based on current radio button selection
        MappingFilter filter;

        if (ui->BackUp_radioButton_Source->isChecked()) {
            filter = MappingFilter(MappingFilter::SourceDevice, selectedDevice->ID);
            qDebug() << "Filtering by Source device:" << selectedDevice->ID;
        } else if (ui->BackUp_radioButton_Target->isChecked()) {
            filter = MappingFilter(MappingFilter::TargetDevice, selectedDevice->ID);
            qDebug() << "Filtering by Target device:" << selectedDevice->ID;
        } else {
            // No radio button selected - use all
            qDebug() << "No filter radio button checked, using all mappings";
        }

        mappingIds = backupMappingManager->getFilteredMappingIds(filter);

        if (mappingIds.isEmpty()) {
            QMessageBox::information(
                this,
                "Katalog",
                tr("No backup links found.")
                );
            return;
        }

        qDebug() << "Generating profile from" << mappingIds.size() << "filtered mapping(s)";
    } else {
        // Empty list = ALL mappings
        qDebug() << "Generating profile from ALL mappings (checkbox not checked)";
    }

    // Create profile generator
    BackupProfileGenerator* generator = new BackupProfileGenerator(m_connectionName, this);

    connect(generator, &BackupProfileGenerator::profileGenerationCompleted,
            this, [this](const QString& profilePath, int taskCount) {

                ui->BackUp_pushButton_GenerateLuckyBackupProfile->setEnabled(true);

                QMessageBox msgBox(this);
                msgBox.setWindowTitle("Katalog");
                msgBox.setIcon(QMessageBox::Information);
                msgBox.setText(tr("Backup profile created."));
                msgBox.setStandardButtons(QMessageBox::Ok);
                msgBox.exec();

                qDebug() << "LuckyBackup profile generated successfully:";
                qDebug() << "  Path:" << profilePath;
                qDebug() << "  Tasks:" << taskCount;
            });

    connect(generator, &BackupProfileGenerator::profileGenerationFailed,
            this, [this](const QString& errorMessage) {

                ui->BackUp_pushButton_GenerateLuckyBackupProfile->setEnabled(true);

                QMessageBox msgBox(this);
                msgBox.setWindowTitle("Katalog");
                msgBox.setIcon(QMessageBox::Critical);
                msgBox.setText("Failed to generate Backup profile");
                msgBox.setInformativeText(errorMessage);
                msgBox.setStandardButtons(QMessageBox::Ok);
                msgBox.exec();

                qDebug() << "ERROR: Failed to generate LuckyBackup profile:";
                qDebug() << "Common causes:\n"
                            "• Directory ~/.luckyBackup/profiles/ is not writable\n"
                            "• No backup mappings exist in the database\n"
                            "• Source or destination paths are empty\n\n"
                            "Please check the error message above and try again.";
                qDebug() << "  Error:" << errorMessage;
            });

    BackupProfileResult result = generator->generateProfile(mappingIds);

    generator->deleteLater();
}

void MainWindow::on_BackUp_checkBox_OnlySelectedLinks_checkStateChanged(const Qt::CheckState &arg1)
{
    QSettings settings(collection->settingsFilePath, QSettings::IniFormat);
    settings.setValue("BackUp/OnlySelectedLinks", arg1 == Qt::Checked);
}

//Methods-----------------------------------------------------------------------
void MainWindow::loadBackUpMapping()
{
    loadBackUpMappingTotals();
    loadBackUpMappingTable();
}

void MainWindow::loadBackUpMappingTotals()
{
    // Create manager if not exists
    if (!backupMappingManager) {
        backupMappingManager = new BackupMappingManager(m_connectionName, this);
    }

    ui->BackUp_label_CurrentMappings_DeviceValue->setText(selectedDevice->name);

    // Determine current filter
    MappingFilter filter;
    if (!optionDisplayFullMappingTable) {
        if (ui->BackUp_radioButton_Source->isChecked()) {
            filter = MappingFilter(MappingFilter::SourceDevice, selectedDevice->ID);
        } else if (ui->BackUp_radioButton_Target->isChecked()) {
            filter = MappingFilter(MappingFilter::TargetDevice, selectedDevice->ID);
        }
    }

    // Calculate totals through manager
    MappingTotals totals = backupMappingManager->calculateTotals(filter);

    // Update UI labels with EXISTING elements only
    ui->BackUp_label_TotalMappings_Value->setText(QString::number(totals.totalMappings));

    // Device coverage (using existing labels)
    qint64 mapped = totals.totalSourceSize;
    qint64 difference = selectedDevice->totalFileSize - mapped;
    float coverage = static_cast<float>(selectedDevice->totalFileSize - difference) / static_cast<float>(selectedDevice->totalFileSize) * 100;

    ui->BackUp_label_TotalMappings_DeviceCoverageLabelSizeValue->setText(
        QLocale().formattedDataSize(selectedDevice->totalFileSize)
        );
    ui->BackUp_label_TotalMappings_DeviceCoverageLabelSizeMappedValue->setText(
        QLocale().formattedDataSize(mapped)
        );
    ui->BackUp_label_TotalMappings_DeviceCoverageLabelSizePercentValue->setText(
        QLocale().toString(coverage, 'f', 2) + " %"
        );
    ui->BackUp_label_TotalMappings_DeviceCoverageLabelSizeDiffValue->setText(
        QLocale().formattedDataSize(selectedDevice->totalFileSize - mapped) + "  "
        );
    ui->BackUp_label_TotalMappings_DeviceCoverageLabelSizePercentUnlinkedValue->setText(
        QLocale().toString(100 - coverage, 'f', 2) + " %"
        );

    // Size comparison
    ui->BackUp_label_TotalMappings_SizeSourceValue->setText(
        QLocale().formattedDataSize(totals.totalSourceSize) + "  "
        );
    ui->BackUp_label_TotalMappings_SizeTargetValue->setText(
        QLocale().formattedDataSize(totals.totalTargetSize) + "  "
        );
    ui->BackUp_label_TotalMappings_SizeDiffValue->setText(
        QLocale().formattedDataSize(totals.totalSizeDifference) + "  "
        );
    ui->BackUp_label_TotalMappings_SizePercentValue->setText(
        QLocale().toString(totals.totalSizeDifferencePercentage, 'f', 2) + " %"
        );

    // File count comparison
    ui->BackUp_label_TotalMappings_FilesSourceTotal->setText(
        QString::number(totals.totalSourceFileCount)
        );
    ui->BackUp_label_TotalMappings_FilesTargetTotal->setText(
        QString::number(totals.totalTargetFileCount)
        );
    ui->BackUp_label_TotalMappings_FilesDiffValue->setText(
        QLocale().toString(totals.totalFileCountDifference) + "  "
        );
    ui->BackUp_label_TotalMappings_FilesPercentValue->setText(
        QLocale().toString(totals.totalFileCountDifferencePercentage, 'f', 2) + " %"
        );
}

void MainWindow::loadBackUpMappingTable()
{
    // Get database type for time difference formatting
    Database::DatabaseType databaseType = Database::getDatabaseType(m_connectionName);
    QString timeDiffSQL = Database::getFormattedTimeDifference(databaseType,
                                                               "d1.device_date_updated",
                                                               "d2.device_date_updated");

    // DEBUG: Print the generated time diff SQL
    // qDebug() << "=== DEBUG loadBackUpMappingTable ===";
    // qDebug() << "Database type:" << (int)databaseType;
    // qDebug() << "Time diff SQL:" << timeDiffSQL;

    //Load data from table device_mapping
    QSqlQuery query(QSqlDatabase::database(m_connectionName));

    // Build the complete SELECT statement
    QString querySQL = QLatin1String(R"(
                            SELECT
                                dm.mapping_id,
                                dm.mapping_name,
                                dm.mapping_type,
                                dm.mapping_device_source_id,
                                d1.device_name,
                                d1.device_active,
                                d1.device_path,
                                d1.device_total_file_size,
                                d1.device_total_file_count,
                                d1.device_date_updated,
                                dm.mapping_device_target_id,
                                d2.device_name,
                                d2.device_active,
                                d2.device_path,
                                d2.device_total_file_size,
                                d2.device_total_file_count,
                                d2.device_date_updated,

                                d2.device_total_file_size - d1.device_total_file_size AS size_difference,
                                CASE
                                    WHEN d1.device_total_file_size > 0
                                    THEN ROUND(((d2.device_total_file_size - d1.device_total_file_size) * 100.0 / d1.device_total_file_size), 2)
                                    ELSE NULL
                                END AS size_difference_percentage,

                                d2.device_total_file_count - d1.device_total_file_count AS file_count_difference,
                                CASE
                                    WHEN d1.device_total_file_count > 0
                                    THEN ROUND(((d2.device_total_file_count - d1.device_total_file_count) * 100.0 / d1.device_total_file_count), 2)
                                    ELSE NULL
                                END AS file_count_difference_percentage,

)");

    // Append the database-specific time difference calculation
    querySQL += "                                (" + timeDiffSQL + ") AS formatted_time_difference\n";

    // Add FROM and WHERE clauses
    querySQL += QLatin1String(R"(
                            FROM device_mapping dm,
                                device d1,
                                device d2
                            WHERE dm.mapping_device_source_id = d1.device_id
                            AND   dm.mapping_device_target_id = d2.device_id
                        )");

    // Add device filtering based on radio button selection
    if(ui->BackUp_radioButton_Target->isChecked()==true){
        if (      selectedDevice->type == "Storage" ){
            querySQL += " AND d2.device_parent_id =:device_parent_id ";
        }
        else if ( selectedDevice->type == "Catalog" ){
            querySQL += " AND d2.device_id =:device_id ";
        }
        else if ( selectedDevice->type == "Virtual" ){
            QString prepareSQL = QLatin1String(R"(
                                            AND d2.device_id IN (
                                            WITH RECURSIVE hierarchy AS (
                                                 SELECT device_id, device_parent_id, device_name
                                                 FROM device
                                                 WHERE device_id = :device_id
                                                 UNION ALL
                                                 SELECT t.device_id, t.device_parent_id, t.device_name
                                                 FROM device t
                                                 JOIN hierarchy h ON t.device_parent_id = h.device_id
                                            )
                                            SELECT device_id
                                            FROM hierarchy)
                                        )");
            querySQL += prepareSQL;
        }
    }
    else{
        if (      selectedDevice->type == "Storage" ){
            querySQL += " AND d1.device_parent_id =:device_parent_id ";
        }
        else if ( selectedDevice->type == "Catalog" ){
            querySQL += " AND d1.device_id =:device_id ";
        }
        else if ( selectedDevice->type == "Virtual" ){
            QString prepareSQL = QLatin1String(R"(
                                            AND d1.device_id IN (
                                            WITH RECURSIVE hierarchy AS (
                                                 SELECT device_id, device_parent_id, device_name
                                                 FROM device
                                                 WHERE device_id = :device_id
                                                 UNION ALL
                                                 SELECT t.device_id, t.device_parent_id, t.device_name
                                                 FROM device t
                                                 JOIN hierarchy h ON t.device_parent_id = h.device_id
                                            )
                                            SELECT device_id
                                            FROM hierarchy)
                                        )");
            querySQL += prepareSQL;
        }
    }

    querySQL +=" ORDER BY dm.mapping_name ASC ";

    query.prepare(querySQL);
    query.bindValue(":device_id",        selectedDevice->ID);
    query.bindValue(":device_parent_id", selectedDevice->ID);

    if (!query.exec())
    {
        qDebug() << "Error loading device_mapping: " << query.lastError();
        return;
    }

    //Create an sql model for the table
    QSqlQueryModel *queryModel = new QSqlQueryModel(this);
    queryModel->setQuery(std::move(query));

    queryModel->setHeaderData(0, Qt::Horizontal, tr("ID"));
    queryModel->setHeaderData(1, Qt::Horizontal, tr("Mapping Name"));
    queryModel->setHeaderData(2, Qt::Horizontal, tr("Type"));
    queryModel->setHeaderData(3, Qt::Horizontal, tr("Source ID"));
    queryModel->setHeaderData(4, Qt::Horizontal, tr("Source"));
    queryModel->setHeaderData(5, Qt::Horizontal, tr("Active"));
    queryModel->setHeaderData(6, Qt::Horizontal, tr("Path"));
    queryModel->setHeaderData(7, Qt::Horizontal, tr("File Size"));
    queryModel->setHeaderData(8, Qt::Horizontal, tr("Files"));
    queryModel->setHeaderData(9, Qt::Horizontal, tr("Date Updated"));
    queryModel->setHeaderData(10, Qt::Horizontal, tr("Target ID"));
    queryModel->setHeaderData(11, Qt::Horizontal, tr("Target"));
    queryModel->setHeaderData(12, Qt::Horizontal, tr("Active"));
    queryModel->setHeaderData(13, Qt::Horizontal, tr("Path"));
    queryModel->setHeaderData(14, Qt::Horizontal, tr("File Size"));
    queryModel->setHeaderData(15, Qt::Horizontal, tr("Files"));
    queryModel->setHeaderData(16, Qt::Horizontal, tr("Date Updated"));
    queryModel->setHeaderData(17, Qt::Horizontal, tr("Size Diff."));
    queryModel->setHeaderData(18, Qt::Horizontal, tr("Size Diff.(%)"));
    queryModel->setHeaderData(19, Qt::Horizontal, tr("Files Diff."));
    queryModel->setHeaderData(20, Qt::Horizontal, tr("Files Diff.(%)"));
    queryModel->setHeaderData(21, Qt::Horizontal, tr("Date Diff."));

    DeviceMappingView *proxyModel = new DeviceMappingView(this);
    proxyModel->setSourceModel(queryModel);

    //Load model to the view
    ui->BackUp_tableView_CurrentMappings->setModel(proxyModel);
    ui->BackUp_tableView_CurrentMappings->resizeColumnsToContents();
    ui->BackUp_tableView_CurrentMappings->setEditTriggers(QAbstractItemView::NoEditTriggers);
    ui->BackUp_tableView_CurrentMappings->setSelectionBehavior(QAbstractItemView::SelectRows);
    ui->BackUp_tableView_CurrentMappings->setSelectionMode(QAbstractItemView::ExtendedSelection);

    //If the setting is checked, display all columns
    QSettings settings(collection->settingsFilePath, QSettings:: IniFormat);
    if (optionDisplayFullMappingTable == false)
    {
        ui->BackUp_tableView_CurrentMappings->setColumnHidden(3, true);
        ui->BackUp_tableView_CurrentMappings->setColumnHidden(5, true);
        ui->BackUp_tableView_CurrentMappings->setColumnHidden(6, true);
        ui->BackUp_tableView_CurrentMappings->setColumnHidden(9, true);
        ui->BackUp_tableView_CurrentMappings->setColumnHidden(10, true);
        ui->BackUp_tableView_CurrentMappings->setColumnHidden(12, true);
        ui->BackUp_tableView_CurrentMappings->setColumnHidden(13, true);
        ui->BackUp_tableView_CurrentMappings->setColumnHidden(14, true);
    }
    else
    {
        ui->BackUp_tableView_CurrentMappings->setColumnHidden(3, false);
        ui->BackUp_tableView_CurrentMappings->setColumnHidden(5, false);
        ui->BackUp_tableView_CurrentMappings->setColumnHidden(6, false);
        ui->BackUp_tableView_CurrentMappings->setColumnHidden(9, false);
        ui->BackUp_tableView_CurrentMappings->setColumnHidden(10, false);
        ui->BackUp_tableView_CurrentMappings->setColumnHidden(12, false);
        ui->BackUp_tableView_CurrentMappings->setColumnHidden(13, false);
        ui->BackUp_tableView_CurrentMappings->setColumnHidden(14, false);
    }
    ui->BackUp_tableView_CurrentMappings->setColumnHidden(0, true);
    ui->BackUp_tableView_CurrentMappings->setColumnHidden(2, true);

    ui->BackUp_tableView_CurrentMappings->horizontalHeader()->setSectionResizeMode(QHeaderView::ResizeToContents);
}

void MainWindow::loadBackUpDeviceLists(QString list)
{
    //Create a model for the table
    QStandardItemModel *model = new QStandardItemModel();
    model->setColumnCount(4);
    model->setHorizontalHeaderItem(0, new QStandardItem(tr("Parent Device")));
    model->setHorizontalHeaderItem(1, new QStandardItem(tr("Device ID")));
    model->setHorizontalHeaderItem(2, new QStandardItem(tr("Device Name")));
    model->setHorizontalHeaderItem(3, new QStandardItem(tr("Size")));

    //Populate the model from each deviceListTable if type = "Catalog"
    for (int i = 0; i < selectedDevice->deviceListTable.size(); i++)
    {
        if (selectedDevice->deviceListTable.at(i).type == "Catalog")
        {   //Load device an its parent
            Device tempDevice;
            tempDevice.ID = selectedDevice->deviceListTable.at(i).ID;
            tempDevice.loadDevice(m_connectionName);

            Device tempParentDevice;
            tempParentDevice.ID = tempDevice.parentID;
            tempParentDevice.loadDevice(m_connectionName);

            //Add row if valid for the type of list
            if (list == "Source_without_mapping") {
                // Only add devices that do NOT have a mapping
                if (!tempDevice.verifyDeviceHasSourceMapping()) {
                    QList<QStandardItem*> row;
                    row.append(new QStandardItem(tempParentDevice.name));
                    row.append(new QStandardItem(QString::number(tempDevice.ID)));
                    row.append(new QStandardItem(tempDevice.name));
                    row.append(new QStandardItem(QLocale().formattedDataSize((tempDevice.totalFileSize)) + "  "));
                    model->appendRow(row);
                }
            }
            else if (list == "Target_without_mapping") {
                // Only add devices that do NOT have a mapping
                if (!tempDevice.verifyDeviceHasTargetMapping()) {
                    QList<QStandardItem*> row;
                    row.append(new QStandardItem(tempParentDevice.name));
                    row.append(new QStandardItem(QString::number(tempDevice.ID)));
                    row.append(new QStandardItem(tempDevice.name));
                    row.append(new QStandardItem(QLocale().formattedDataSize((tempDevice.totalFileSize)) + "  "));
                    model->appendRow(row);
                }
            }
            else {
                // For other list types, add all devices
                QList<QStandardItem*> row;
                row.append(new QStandardItem(tempParentDevice.name));
                row.append(new QStandardItem(QString::number(tempDevice.ID)));
                row.append(new QStandardItem(tempDevice.name));
                row.append(new QStandardItem(QLocale().formattedDataSize((tempDevice.totalFileSize)) + "  "));
                model->appendRow(row);
            }
        }
    }

    //Load model to the Target view
    if (list.contains("Target")){
        ui->BackUp_treeView_List2->setModel(model);
        ui->BackUp_treeView_List2->resizeColumnToContents(1);
        ui->BackUp_treeView_List2->setEditTriggers(QAbstractItemView::NoEditTriggers);
        ui->BackUp_treeView_List2->header()->setSectionResizeMode(QHeaderView::ResizeToContents);
    }
    else{
        ui->BackUp_treeView_List1->setModel(model);
        ui->BackUp_treeView_List1->resizeColumnToContents(1);
        ui->BackUp_treeView_List1->setEditTriggers(QAbstractItemView::NoEditTriggers);
        ui->BackUp_treeView_List1->header()->setSectionResizeMode(QHeaderView::ResizeToContents);
    }
}

void MainWindow::saveNewMapping()
{
    //Get data and validate it
        //Check if models are valid and have data
        QAbstractItemModel* model1 = ui->BackUp_treeView_List1->model();
        QAbstractItemModel* model2 = ui->BackUp_treeView_List2->model();

        if (!model1 || !model2) {
            QMessageBox::warning(this, "Katalog", tr("Populate the lists first (One or both device lists are empty)."));
            return;
        }

        if (model1->rowCount() == 0 || model2->rowCount() == 0) {
            QMessageBox::warning(this, "Katalog", tr("Populate the lists first (One or both device lists are empty)."));
            return;
        }

        //Get selection models
        QItemSelectionModel* selectionModel1 = ui->BackUp_treeView_List1->selectionModel();
        QItemSelectionModel* selectionModel2 = ui->BackUp_treeView_List2->selectionModel();

        //Check if selection models exist
        if (!selectionModel1 || !selectionModel2) {
            QMessageBox::warning(this, "Katalog", tr("Invalid selection model"));
            return;
        }

        //Get selected rows
        QModelIndexList selectedRows1 = selectionModel1->selectedRows();
        QModelIndexList selectedRows2 = selectionModel2->selectedRows();

        //Validate selections
        if (selectedRows1.isEmpty() || selectedRows2.isEmpty()) {
            QMessageBox::warning(this, "Katalog",
                                 tr("Select a device from both lists."));
            return;
        }

        //Safely get device IDs using first selected row
        QModelIndex deviceIndex1 = selectedRows1.first().siblingAtColumn(1);
        QModelIndex deviceIndex2 = selectedRows2.first().siblingAtColumn(1);

        //Additional null check
        if (!deviceIndex1.isValid() || !deviceIndex2.isValid()) {
            QMessageBox::warning(this, "Katalog", tr("Invalid device selection."));
            return;
        }

        QString device1ID = deviceIndex1.data().toString();
        QString device2ID = deviceIndex2.data().toString();


        //Validate device IDs
        if (device1ID.isEmpty() || device2ID.isEmpty()) {
            QMessageBox::warning(this, "Katalog", tr("Empty device ID."));
            return;
        }

        //Validate mapping name
        QString mappingName = ui->BackUp_lineEdit_Name->text().trimmed();
        if (mappingName.isEmpty()) {
            QMessageBox::warning(this, "Katalog", tr("Provide a mapping name."));
            return;
        }

        //Prevent mapping a device to itself
        if (device1ID == device2ID) {
            QMessageBox::warning(this, "Katalog", tr("Select a different source or target (a device shall not be mapped to itself)."));
            return;
        }

    //Insert mapping in the table device_mapping
    QSqlQuery query(QSqlDatabase::database(m_connectionName));
    QString querySQL = QLatin1String(R"(
                            INSERT INTO device_mapping
                            (   mapping_name,
                                mapping_type,
                                mapping_device_source_id,
                                mapping_device_target_id,
                                mapping_strict_copy
                            )
                            VALUES
                            (   :mapping_name,
                                :mapping_type,
                                :mapping_device_source_id,
                                :mapping_device_target_id,
                                :mapping_strict_copy
                            )
                        )");
    query.prepare(querySQL);
    query.bindValue(":mapping_name", mappingName);
    query.bindValue(":mapping_type", "Backup");
    query.bindValue(":mapping_device_source_id", device1ID);
    query.bindValue(":mapping_device_target_id", device2ID);
    query.bindValue(":mapping_strict_copy", ui->BackUp_checkBox_StrictCopy->isChecked() ? 1 : 0);

    if (!query.exec())
    {
        qDebug() << "Error inserting device_mapping: " << query.lastError();
        return;
    }

    //Reload the mapping table
    loadBackUpMapping();

    //Clear the mapping name
    ui->BackUp_lineEdit_Name->clear();

    //Clear the selection
    ui->BackUp_treeView_List1->clearSelection();
    ui->BackUp_treeView_List2->clearSelection();

}

void MainWindow::setupBackUpManager()
{
    backupMappingManager = new BackupMappingManager(m_connectionName, this);

    //Hide the preview section until the user clicks "Preview Backup"
    ui->BackUp_label_PreviewSummary->setVisible(false);
    ui->BackUp_tableView_PreviewFiles->setVisible(false);

    //Hide the execution panel until a backup is running
    ui->BackUp_label_ExecutionStatus->setVisible(false);
    ui->BackUp_progressBar->setVisible(false);
    ui->BackUp_pushButton_CancelBackup->setVisible(false);
}
