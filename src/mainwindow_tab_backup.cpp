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
#include "devicetreeview.h"
#include "ui_mainwindow.h"
#include <QMap>

//UI----------------------------------------------------------------------------

void MainWindow::on_BackUp_pushButton_CreateLinkShowHide_clicked()
{
    QString iconName = ui->BackUp_pushButton_CreateLinkShowHide->icon().name();

    if (iconName == "go-down") { //Hide
        ui->BackUp_pushButton_CreateLinkShowHide->setIcon(QIcon::fromTheme("go-up"));
        ui->BackUp_widget_CreateLink->setHidden(true);
        QSettings settings(collection->settingsFilePath, QSettings::IniFormat);
        settings.setValue("BackUp/ShowHideCreateLink", "go-up");
    } else { //Show
        ui->BackUp_pushButton_CreateLinkShowHide->setIcon(QIcon::fromTheme("go-down"));
        ui->BackUp_widget_CreateLink->setHidden(false);
        QSettings settings(collection->settingsFilePath, QSettings::IniFormat);
        settings.setValue("BackUp/ShowHideCreateLink", "go-down");
    }
}

void MainWindow::on_BackUp_pushButton_GenerateMappingName_clicked()
{
    const int sourceId = ui->BackUp_comboBox_Source->selectedDeviceId();
    const int targetId = ui->BackUp_comboBox_Target->selectedDeviceId();
    if (sourceId <= 0 || targetId <= 0)
        return;

    // Look up device names from the flat models (col 1 = device ID, col 2 = device name)
    auto nameForId = [](QAbstractItemModel *m, int id) -> QString {
        if (!m) return {};
        for (int r = 0; r < m->rowCount(); ++r)
            if (m->index(r, 1).data().toInt() == id)
                return m->index(r, 2).data().toString();
        return {};
    };

    const QString sourceName = nameForId(ui->BackUp_treeView_ListSources->model(), sourceId);
    const QString targetName = nameForId(ui->BackUp_treeView_ListTargets->model(), targetId);
    if (!sourceName.isEmpty() && !targetName.isEmpty())
        ui->BackUp_lineEdit_Name->setText(sourceName + " -> " + targetName);
}

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

void MainWindow::on_BackUp_tableView_CurrentMappings_customContextMenuRequested(const QPoint &pos)
{
    // Select the right-clicked row so subsequent action handlers find the correct mapping
    QModelIndex clickedIndex = ui->BackUp_tableView_CurrentMappings->indexAt(pos);
    if (!clickedIndex.isValid())
        return;
    ui->BackUp_tableView_CurrentMappings->setCurrentIndex(clickedIndex);

    QModelIndexList selectedIndexes = ui->BackUp_tableView_CurrentMappings->selectionModel()->selectedIndexes();
    if (selectedIndexes.isEmpty())
        return;

    QPoint globalPos = ui->BackUp_tableView_CurrentMappings->mapToGlobal(pos);
    QMenu mappingContextMenu;

    // ── Primary actions ───────────────────────────────────────────────────────
    QAction *runAction = new QAction(QIcon::fromTheme("media-playback-start"), tr("Run Backup"), this);
    mappingContextMenu.addAction(runAction);
    connect(runAction, &QAction::triggered, this, [this]() { runBackup(); });

    QAction *previewAction = new QAction(QIcon::fromTheme("go-next"), tr("Preview Backup"), this);
    mappingContextMenu.addAction(previewAction);
    connect(previewAction, &QAction::triggered, this, [this]() {
        on_BackUp_pushButton_BackUpPreview_clicked();
    });

    QAction *replicateAction = new QAction(QIcon::fromTheme("edit-copy"), tr("Replicate Directories"), this);
    mappingContextMenu.addAction(replicateAction);
    connect(replicateAction, &QAction::triggered, this, [this]() {
        on_BackUp_pushButton_ReplicateDirectories_clicked();
    });

    // ── Mapping management ────────────────────────────────────────────────────
    mappingContextMenu.addSeparator();

    QAction *invertAction = new QAction(QIcon::fromTheme("object-flip-horizontal"), tr("Invert (swap source and target)"), this);
    mappingContextMenu.addAction(invertAction);
    connect(invertAction, &QAction::triggered, this, [this, selectedIndexes]() {
        int mappingID = selectedIndexes.at(0).data().toInt();
        if (!backupMappingManager)
            backupMappingManager = new BackupMappingManager(m_connectionName, this);
        if (!backupMappingManager->invertMapping(mappingID))
            return;
        loadBackUpMapping();
        collection->saveMappingTableToFile();
    });

    QAction *renameAction = new QAction(QIcon::fromTheme("edit-rename"), tr("Rename"), this);
    renameAction->setEnabled(false);
    mappingContextMenu.addAction(renameAction);

    QAction *rsyncAction = new QAction(QIcon::fromTheme("document-export"), tr("Export to rsync"), this);
    rsyncAction->setEnabled(false);
    mappingContextMenu.addAction(rsyncAction);

    // ── Destructive ───────────────────────────────────────────────────────────
    mappingContextMenu.addSeparator();

    QAction *deleteAction = new QAction(QIcon::fromTheme("edit-delete"), tr("Delete"), this);
    mappingContextMenu.addAction(deleteAction);
    connect(deleteAction, &QAction::triggered, this, [this, selectedIndexes]() {
        int mappingID = selectedIndexes.at(0).data().toInt();
        if (!backupMappingManager)
            backupMappingManager = new BackupMappingManager(m_connectionName, this);
        if (!backupMappingManager->deleteMapping(mappingID))
            return;
        loadBackUpMapping();
        collection->saveMappingTableToFile();
    });

    mappingContextMenu.exec(globalPos);
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
    if (!backupMappingManager)
        backupMappingManager = new BackupMappingManager(m_connectionName, this);
    MappingInfo mapping = backupMappingManager->getMappingById(mappingID);

    //Load source and target devices to get catalog IDs (externalID)
    Device sourceDevice;
    sourceDevice.ID = mapping.sourceDeviceId;
    sourceDevice.loadDevice(m_connectionName);

    Device targetDevice;
    targetDevice.ID = mapping.targetDeviceId;
    targetDevice.loadDevice(m_connectionName);

    //Validate both devices are catalogs
    if (sourceDevice.type != "Catalog" || targetDevice.type != "Catalog") {
        QMessageBox::warning(this, "Katalog",
                             tr("Both source and target must be Catalog devices."));
        return;
    }

    //Refresh active state from filesystem, then check availability
    sourceDevice.updateActiveState(m_connectionName);
    targetDevice.updateActiveState(m_connectionName);

    auto showUnavailable = [this](const QString &role, const QString &name) {
        ui->BackUp_label_ProgressSummary->setVisible(true);
        ui->BackUp_label_ProgressSummary->setText(
            StatusBarMessageBuilder()
                .setOperation(tr("Replicate"))
                .setStatus(tr("%1 not available").arg(role))
                .setCatalogName(name)
                .build());
    };

    if (!sourceDevice.active) { showUnavailable(tr("Source"), sourceDevice.name); return; }
    if (!targetDevice.active) { showUnavailable(tr("Target"), targetDevice.name); return; }

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
        m_pendingBackupOperation    = PendingBackupOperation::ReplicateDirectories;

        ui->BackUp_pushButton_ReplicateDirectories->setEnabled(false);
        ui->BackUp_label_ProgressSummary->setVisible(false);

        setupDeviceUpdateManagerForBackup();
        deviceUpdateManager->updateDeviceHierarchy(
            &m_pendingBackupSourceDevice, collection->databaseMode, collection->folder, "update");
        return;
    }

    executeReplicate(sourceDevice, targetDevice);
}

void MainWindow::executeReplicate(const Device &sourceDevice, const Device &targetDevice)
{
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

    //Update status bar with completion statistics
    ui->BackUp_label_ProgressSummary->setVisible(true);
    ui->BackUp_label_ProgressSummary->setText(
        StatusBarMessageBuilder()
            .setOperation(tr("Replicate"))
            .setStatus(tr("Completed"))
            .addResult(tr("Created"),          result.createdCount())
            .addResult(tr("Already existing"), result.skippedCount())
            .addResult(tr("Errors"),           result.errorCount())
            .build()
    );

}

void MainWindow::on_BackUp_pushButton_BackUpPreview_clicked()
{
    if (ui->BackUp_checkBox_UpdateBeforeBackup->isChecked()) {
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

        if (deviceUpdateManager->operationRunning()) {
            QMessageBox::warning(this, "Katalog",
                tr("A catalog update is already in progress. Please wait and try again."));
            return;
        }

        m_pendingBackupMappingId    = mappingID;
        m_pendingBackupSourceDevice = sourceDevice;
        m_pendingBackupTargetDevice = targetDevice;
        m_backupUpdatePhase         = BackupUpdatePhase::UpdatingSource;
        m_pendingBackupOperation    = PendingBackupOperation::Preview;

        ui->BackUp_pushButton_BackUpPreview->setEnabled(false);
        ui->BackUp_label_ProgressSummary->setVisible(false);

        setupDeviceUpdateManagerForBackup();
        deviceUpdateManager->updateDeviceHierarchy(
            &m_pendingBackupSourceDevice, collection->databaseMode, collection->folder, "update");
        return;
    }

    loadBackupPreview();
}

void MainWindow::on_BackUp_pushButton_ExportPreview_clicked()
{
    QAbstractItemModel *model = ui->BackUp_tableView_PreviewFiles->model();
    if (!model || model->rowCount() == 0)
        return;

    QList<BackupPreviewRow> rows;
    rows.reserve(model->rowCount());
    for (int r = 0; r < model->rowCount(); ++r) {
        BackupPreviewRow row;
        row.status     = model->index(r, 0).data().toString();
        row.fileName   = model->index(r, 1).data().toString();
        row.folderPath = model->index(r, 2).data().toString();
        row.fileSize   = model->index(r, 3).data(Qt::UserRole).toLongLong();
        rows.append(row);
    }

    const QString filePath = BackupMappingManager::exportPreviewToCsv(rows, collection->folder);

    QMessageBox msgBox;
    msgBox.setWindowTitle("Katalog");
    msgBox.setTextFormat(Qt::RichText);
    QString exportFileName = "file://" + filePath;
    msgBox.setText(tr("Results exported to the collection folder:")
                       +"<br/><a href='"+exportFileName+"'>"+exportFileName+"</a>");
    msgBox.setIcon(QMessageBox::Information);
    msgBox.exec();
}

void MainWindow::on_BackUp_pushButton_RunBackup_clicked()
{
    switch (backupButtonState) {
        case BackupButtonState::Idle:    runBackup();            break;
        case BackupButtonState::Running: pauseCurrentBackup();   break;
        case BackupButtonState::Paused:  resumeCurrentBackup();  break;
    }
}

void MainWindow::pauseCurrentBackup()
{
    if (!m_backupJob) return;
    m_backupJob->pauseBackup();
    setBackupButtonState(BackupButtonState::Paused);
    ui->BackUp_label_ProgressSummary->setText(
        StatusBarMessageBuilder().setOperation(tr("backup")).setStatus(tr("Paused")).build());
}

void MainWindow::resumeCurrentBackup()
{
    if (!m_backupJob) return;
    m_backupJob->resumeBackup();
    setBackupButtonState(BackupButtonState::Running);
}

void MainWindow::setBackupButtonState(BackupButtonState state)
{
    backupButtonState = state;
    switch (state) {
        case BackupButtonState::Idle:
            ui->BackUp_pushButton_RunBackup->setText(tr("Run Backup"));
            ui->BackUp_pushButton_RunBackup->setIcon(QIcon::fromTheme("media-playback-start"));
            ui->BackUp_pushButton_RunBackup->setEnabled(true);
            ui->BackUp_pushButton_CancelBackup->setVisible(false);
            break;
        case BackupButtonState::Running:
            ui->BackUp_pushButton_RunBackup->setText(tr("Pause"));
            ui->BackUp_pushButton_RunBackup->setIcon(QIcon::fromTheme("media-playback-pause"));
            ui->BackUp_pushButton_RunBackup->setEnabled(true);
            ui->BackUp_pushButton_CancelBackup->setVisible(true);
            break;
        case BackupButtonState::Paused:
            ui->BackUp_pushButton_RunBackup->setText(tr("Resume"));
            ui->BackUp_pushButton_RunBackup->setIcon(QIcon::fromTheme("media-playback-start"));
            ui->BackUp_pushButton_RunBackup->setEnabled(true);
            ui->BackUp_pushButton_CancelBackup->setVisible(true);
            break;
    }
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

    //Refresh active state from filesystem, then check availability
    sourceDevice.updateActiveState(m_connectionName);
    targetDevice.updateActiveState(m_connectionName);

    auto showUnavailable = [this](const QString &role, const QString &name) {
        ui->BackUp_label_ProgressSummary->setVisible(true);
        ui->BackUp_label_ProgressSummary->setText(
            StatusBarMessageBuilder()
                .setOperation(tr("Backup"))
                .setStatus(tr("%1 not available").arg(role))
                .setCatalogName(name)
                .build());
    };

    if (!sourceDevice.active) { showUnavailable(tr("Source"), sourceDevice.name); return; }
    if (!targetDevice.active) { showUnavailable(tr("Target"), targetDevice.name); return; }

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
        m_pendingBackupOperation    = PendingBackupOperation::RunBackup;

        ui->BackUp_pushButton_RunBackup->setEnabled(false);
        ui->BackUp_label_ProgressSummary->setVisible(false);

        setupDeviceUpdateManagerForBackup();
        deviceUpdateManager->updateDeviceHierarchy(
            &m_pendingBackupSourceDevice, collection->databaseMode, collection->folder, "update");
        return;
    }

    executeBackup(sourceDevice, targetDevice, mapping);

    ui->BackUp_pushButton_ExportPreview->setEnabled(true);
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
                m_pendingBackupOperation = PendingBackupOperation::None;
                m_pendingBackupMappingId = -1;
                setupDeviceUpdateManager();  // restore normal device-tab connections
                ui->BackUp_pushButton_RunBackup->setEnabled(true);
                ui->BackUp_pushButton_BackUpPreview->setEnabled(true);
                ui->BackUp_pushButton_ReplicateDirectories->setEnabled(true);
                ui->BackUp_label_ProgressSummary->setVisible(false);
                QMessageBox::warning(this, "Katalog",
                    tr("Catalog update failed: %1").arg(error));
            });
    connect(deviceUpdateManager, &DeviceUpdateManager::operationCancelled,
            this, [this]() {
                m_backupUpdatePhase      = BackupUpdatePhase::None;
                m_pendingBackupOperation = PendingBackupOperation::None;
                m_pendingBackupMappingId = -1;
                setupDeviceUpdateManager();
                ui->BackUp_pushButton_RunBackup->setEnabled(true);
                ui->BackUp_pushButton_BackUpPreview->setEnabled(true);
                ui->BackUp_pushButton_ReplicateDirectories->setEnabled(true);
                ui->BackUp_label_ProgressSummary->setVisible(false);
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

    const PendingBackupOperation op = m_pendingBackupOperation;
    m_pendingBackupOperation = PendingBackupOperation::None;

    switch (op) {
        case PendingBackupOperation::RunBackup:
            // executeBackup() transitions the button to Running (Pause) via setBackupButtonState
            executeBackup(m_pendingBackupSourceDevice, m_pendingBackupTargetDevice, mapping);
            break;
        case PendingBackupOperation::Preview:
            ui->BackUp_pushButton_BackUpPreview->setEnabled(true);
            loadBackupPreview();
            break;
        case PendingBackupOperation::ReplicateDirectories:
            ui->BackUp_pushButton_ReplicateDirectories->setEnabled(true);
            executeReplicate(m_pendingBackupSourceDevice, m_pendingBackupTargetDevice);
            break;
        default:
            break;
    }
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
        ui->BackUp_label_ProgressSummary->setVisible(false);
        return;
    }

    //Setup and launch the backup worker thread
    m_backupJob = new BackupJobStoppable();
    m_backupJob->setFiles(cmp.filesToCopy);
    m_backupJob->setConflictMode(mapping.conflictMode);
    if (mapping.conflictMode == ConflictMode::RenameOldest)
        m_backupJob->setConflictFiles(cmp.fileConflicts);
    m_backupJob->setSourcePath(sourceDevice.path);
    m_backupJob->setTargetPath(targetDevice.path);
    const bool isArchive = (mapping.mappingType == QLatin1String("Archive"));
    m_currentBackupIsArchive = isArchive;
    m_backupJob->setArchiveMode(isArchive);

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
    ui->BackUp_label_ProgressSummary->setText(
        StatusBarMessageBuilder()
            .setOperation(m_currentBackupIsArchive ? tr("Archive") : tr("Backup"))
            .setStatus(tr("Starting…"))
            .build());
    m_backupTimer.start();
    ui->BackUp_label_ProgressSummary->setVisible(true);
    ui->BackUp_progressBar->setVisible(true);
    setBackupButtonState(BackupButtonState::Running);

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
        .setProcess(m_currentBackupIsArchive ? tr("Moving") : tr("Copying"), filesDone + 1, totalFiles)
        .setSizeProgress(bytesCopied, totalBytes)
        .setSpeed(speedBps)
        .setTimeToCompletion(etaStr)
        .setCurrentItem(currentFile)
        .build();

    ui->BackUp_label_ProgressSummary->setText(msg);
}

void MainWindow::onBackupFinished(const BackupReport &report)
{
    m_backupJob    = nullptr;
    m_backupThread = nullptr;

    setBackupButtonState(BackupButtonState::Idle);
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
        .setOperation(m_currentBackupIsArchive ? tr("archive") : tr("backup"))
        .setStatus(report.wasCancelled ? tr("Cancelled") : tr("Complete"))
        .setSizeProgress(report.totalBytesCopied, report.totalBytesCopied)
        .setTimeToCompletion(elapsedStr)
        .build();
    ui->BackUp_label_ProgressSummary->setText(msg);

    showBackupReport(report);

    // Update the target catalog to reflect the newly copied files
    if (!report.wasCancelled
            && ui->BackUp_checkBox_UpdateBeforeBackup->isChecked()
            && !deviceUpdateManager->operationRunning()) {

        disconnect(deviceUpdateManager, nullptr, this, nullptr);
        connect(deviceUpdateManager, &DeviceUpdateManager::operationCompleted,
                this, [this, msg]() {
                    setupDeviceUpdateManager();
                    ui->BackUp_label_ProgressSummary->setText(msg);
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
    for (const DifferenceFileEntry &e : report.moved) {
        QList<QStandardItem*> row;
        row << new QStandardItem(tr("moved"))
            << new QStandardItem(e.fileName)
            << new QStandardItem(e.folderPath)
            << new QStandardItem(QLocale().formattedDataSize(e.fileSize));
        model->appendRow(row);
    }
    for (const DifferenceFileEntry &e : report.renamed) {
        QList<QStandardItem*> row;
        row << new QStandardItem(tr("archived + replaced"))
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

    ui->BackUp_label_ProgressSummary->setText(
        StatusBarMessageBuilder()
            .setOperation(tr("Report"))
            .setStatus(report.wasCancelled ? tr("Cancelled") : tr("Complete"))
            .addResult(tr("Copied"),              report.copiedCount(),   report.totalBytesCopied)
            .addResult(tr("Archived+replaced"),   report.renamedCount())
            .addResult(tr("Conflicts (skipped)"), report.conflictCount())
            .addResult(tr("Errors"),              report.errorCount())
            .build()
    );
    ui->BackUp_label_ProgressSummary->setVisible(true);
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

    //Refresh active state from filesystem (preview uses catalog data from DB, so offline is allowed)
    sourceDevice.updateActiveState(m_connectionName);
    targetDevice.updateActiveState(m_connectionName);

    //Memory mode: load file data into 'file' table first
    if (collection->databaseMode == "Memory") {
        QMutex mutex;
        bool stopRequested = false;
        sourceDevice.catalog->loadCatalogFileListToTable(mutex, stopRequested);
        targetDevice.catalog->loadCatalogFileListToTable(mutex, stopRequested);
    }

    //Run comparison (respects strictCopy setting of this mapping)
    BackupCompareResult cmp = compareForBackup(sourceDevice, targetDevice, mapping.strictCopy);
    const bool isArchive = (mapping.mappingType == QLatin1String("Archive"));

    //Build preview model (Status: "to copy" / "to move" / "conflict")
    QStandardItemModel *model = new QStandardItemModel(this);
    model->setHorizontalHeaderItem(0, new QStandardItem(tr("Status")));
    model->setHorizontalHeaderItem(1, new QStandardItem(tr("File Name")));
    model->setHorizontalHeaderItem(2, new QStandardItem(tr("Path")));
    model->setHorizontalHeaderItem(3, new QStandardItem(tr("Size")));

    qint64 copySize = 0;
    for (const DifferenceFileEntry &entry : cmp.filesToCopy) {
        auto *sizeItem = new QStandardItem(QLocale().formattedDataSize(entry.fileSize));
        sizeItem->setData(entry.fileSize, Qt::UserRole);
        QList<QStandardItem*> row;
        row << new QStandardItem(isArchive ? tr("to move") : tr("to copy"))
            << new QStandardItem(entry.fileName)
            << new QStandardItem(entry.folderPath)
            << sizeItem;
        model->appendRow(row);
        copySize += entry.fileSize;
    }

    qint64 conflictSize = 0;
    for (const DifferenceFileEntry &entry : cmp.fileConflicts) {
        auto *sizeItem = new QStandardItem(QLocale().formattedDataSize(entry.fileSize));
        sizeItem->setData(entry.fileSize, Qt::UserRole);
        QList<QStandardItem*> row;
        row << new QStandardItem(tr("conflict"))
            << new QStandardItem(entry.fileName)
            << new QStandardItem(entry.folderPath)
            << sizeItem;
        model->appendRow(row);
        conflictSize += entry.fileSize;
    }

    ui->BackUp_tableView_PreviewFiles->setModel(model);
    ui->BackUp_tableView_PreviewFiles->setEditTriggers(QAbstractItemView::NoEditTriggers);
    ui->BackUp_tableView_PreviewFiles->setSelectionBehavior(QAbstractItemView::SelectRows);
    ui->BackUp_tableView_PreviewFiles->horizontalHeader()->setSectionResizeMode(QHeaderView::ResizeToContents);

    //Summary line — includes mode indicator and offline notice if either device is unavailable
    const QString modeLabel = mapping.strictCopy ? tr("strict copy") : tr("dedup");
    QString offlineNote;
    if (!sourceDevice.active && !targetDevice.active)
        offlineNote = tr(" — source & target offline");
    else if (!sourceDevice.active)
        offlineNote = tr(" — source offline");
    else if (!targetDevice.active)
        offlineNote = tr(" — target offline");

    ui->BackUp_label_ProgressSummary->setText(
        StatusBarMessageBuilder()
            .setOperation(tr("Preview"))
            .setStatus(modeLabel + offlineNote)
            .addResult(isArchive ? tr("To move") : tr("To copy"), cmp.filesToCopy.size(), copySize)
            .addResult(tr("Conflicts (skipped)"), cmp.fileConflicts.size(), conflictSize)
            .addResult(tr("Already in target"),   cmp.skippedCount)
            .build()
    );

    //Show preview section and enable export
    ui->BackUp_label_ProgressSummary->setVisible(true);
    ui->BackUp_tableView_PreviewFiles->setVisible(true);
    ui->BackUp_pushButton_ExportPreview->setEnabled(true);
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

void MainWindow::on_BackUp_comboBox_MappingType_currentIndexChanged(int /*index*/)
{
    loadBackUpMapping();
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
        // Use only filtered mappings based on current radio button + type selection
        const QString mappingType = ui->BackUp_comboBox_MappingType->currentText();
        MappingFilter filter;
        filter.mappingType = mappingType;

        if (ui->BackUp_radioButton_Source->isChecked()) {
            filter = MappingFilter(MappingFilter::SourceDevice, selectedDevice->ID, mappingType);
            qDebug() << "Filtering by Source device:" << selectedDevice->ID;
        } else if (ui->BackUp_radioButton_Target->isChecked()) {
            filter = MappingFilter(MappingFilter::TargetDevice, selectedDevice->ID, mappingType);
            qDebug() << "Filtering by Target device:" << selectedDevice->ID;
        } else {
            // No radio button selected - use all of selected type
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
    const QString mappingType = ui->BackUp_comboBox_MappingType->currentText();
    MappingFilter filter;
    filter.mappingType = mappingType;
    if (!optionDisplayFullMappingTable) {
        if (ui->BackUp_radioButton_Source->isChecked()) {
            filter = MappingFilter(MappingFilter::SourceDevice, selectedDevice->ID, mappingType);
        } else if (ui->BackUp_radioButton_Target->isChecked()) {
            filter = MappingFilter(MappingFilter::TargetDevice, selectedDevice->ID, mappingType);
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
    if (!backupMappingManager)
        backupMappingManager = new BackupMappingManager(m_connectionName, this);

    // Build filter — same logic as loadBackUpMappingTotals()
    const QString mappingType = ui->BackUp_comboBox_MappingType->currentText();
    MappingFilter filter;
    filter.mappingType = mappingType;
    if (!optionDisplayFullMappingTable) {
        if (ui->BackUp_radioButton_Source->isChecked()) {
            filter = MappingFilter(MappingFilter::SourceDevice, selectedDevice->ID, mappingType);
        } else if (ui->BackUp_radioButton_Target->isChecked()) {
            filter = MappingFilter(MappingFilter::TargetDevice, selectedDevice->ID, mappingType);
        }
    }

    // All SQL stays in core — get the executed query from the manager
    QSqlQuery query = backupMappingManager->executeTableDisplayQuery(filter);

    QSqlQueryModel *queryModel = new QSqlQueryModel(this);
    queryModel->setQuery(std::move(query));

    queryModel->setHeaderData( 0, Qt::Horizontal, tr("ID"));
    queryModel->setHeaderData( 1, Qt::Horizontal, tr("Mapping Name"));
    queryModel->setHeaderData( 2, Qt::Horizontal, tr("Type"));
    queryModel->setHeaderData( 3, Qt::Horizontal, tr("Copy mode"));
    queryModel->setHeaderData( 4, Qt::Horizontal, tr("On conflict"));
    queryModel->setHeaderData( 5, Qt::Horizontal, tr("Source ID"));
    queryModel->setHeaderData( 6, Qt::Horizontal, tr("Source"));
    queryModel->setHeaderData( 7, Qt::Horizontal, tr("Active"));
    queryModel->setHeaderData( 8, Qt::Horizontal, tr("Path"));
    queryModel->setHeaderData( 9, Qt::Horizontal, tr("File Size"));
    queryModel->setHeaderData(10, Qt::Horizontal, tr("Files"));
    queryModel->setHeaderData(11, Qt::Horizontal, tr("Date Updated"));
    queryModel->setHeaderData(12, Qt::Horizontal, tr("Target ID"));
    queryModel->setHeaderData(13, Qt::Horizontal, tr("Target"));
    queryModel->setHeaderData(14, Qt::Horizontal, tr("Active"));
    queryModel->setHeaderData(15, Qt::Horizontal, tr("Path"));
    queryModel->setHeaderData(16, Qt::Horizontal, tr("File Size"));
    queryModel->setHeaderData(17, Qt::Horizontal, tr("Files"));
    queryModel->setHeaderData(18, Qt::Horizontal, tr("Date Updated"));
    queryModel->setHeaderData(19, Qt::Horizontal, tr("Size Diff."));
    queryModel->setHeaderData(20, Qt::Horizontal, tr("Size Diff.(%)"));
    queryModel->setHeaderData(21, Qt::Horizontal, tr("Files Diff."));
    queryModel->setHeaderData(22, Qt::Horizontal, tr("Files Diff.(%)"));
    queryModel->setHeaderData(23, Qt::Horizontal, tr("Date Diff."));

    DeviceMappingView *proxyModel = new DeviceMappingView(this);
    proxyModel->setSourceModel(queryModel);

    ui->BackUp_tableView_CurrentMappings->setModel(proxyModel);
    ui->BackUp_tableView_CurrentMappings->resizeColumnsToContents();
    ui->BackUp_tableView_CurrentMappings->setEditTriggers(QAbstractItemView::NoEditTriggers);
    ui->BackUp_tableView_CurrentMappings->setSelectionBehavior(QAbstractItemView::SelectRows);
    ui->BackUp_tableView_CurrentMappings->setSelectionMode(QAbstractItemView::ExtendedSelection);

    // Columns toggled by the "full table" option
    //   col  2: mapping_type       col  3: copy mode      col  4: conflict mode
    //   col  5: source ID          col  7: source active   col  8: source path
    //   col 11: source date        col 12: target ID       col 14: target active
    //   col 15: target path        col 16: target file size
    const bool full = optionDisplayFullMappingTable;
    ui->BackUp_tableView_CurrentMappings->setColumnHidden( 2, !full);
    ui->BackUp_tableView_CurrentMappings->setColumnHidden( 3, !full);
    ui->BackUp_tableView_CurrentMappings->setColumnHidden( 4, !full);
    ui->BackUp_tableView_CurrentMappings->setColumnHidden( 5, !full);
    ui->BackUp_tableView_CurrentMappings->setColumnHidden( 7, !full);
    ui->BackUp_tableView_CurrentMappings->setColumnHidden( 8, !full);
    ui->BackUp_tableView_CurrentMappings->setColumnHidden(11, !full);
    ui->BackUp_tableView_CurrentMappings->setColumnHidden(12, !full);
    ui->BackUp_tableView_CurrentMappings->setColumnHidden(14, !full);
    ui->BackUp_tableView_CurrentMappings->setColumnHidden(15, !full);
    ui->BackUp_tableView_CurrentMappings->setColumnHidden(16, !full);
    ui->BackUp_tableView_CurrentMappings->setColumnHidden( 0, true); // always hidden: ID

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

    // Extra data for the TreeComboBox hierarchical model (DeviceTreeView needs type + active)
    QMap<QString, QString> parentTypeMap;   // parentName → device type ("Virtual", "Storage", …)
    QMap<int, bool>        deviceActiveMap; // deviceId  → active state

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

            parentTypeMap.insert(tempParentDevice.name, tempParentDevice.type);
            deviceActiveMap.insert(tempDevice.ID, tempDevice.active);

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
    TreeComboBox *combo;
    QTreeView    *tree;
    if (list.contains("Target")){
        ui->BackUp_treeView_ListTargets->setModel(model);
        ui->BackUp_treeView_ListTargets->resizeColumnToContents(1);
        ui->BackUp_treeView_ListTargets->setEditTriggers(QAbstractItemView::NoEditTriggers);
        ui->BackUp_treeView_ListTargets->header()->setSectionResizeMode(QHeaderView::ResizeToContents);
        combo = ui->BackUp_comboBox_Target;
        tree  = ui->BackUp_treeView_ListTargets;
    }
    else{
        ui->BackUp_treeView_ListSources->setModel(model);
        ui->BackUp_treeView_ListSources->resizeColumnToContents(1);
        ui->BackUp_treeView_ListSources->setEditTriggers(QAbstractItemView::NoEditTriggers);
        ui->BackUp_treeView_ListSources->header()->setSectionResizeMode(QHeaderView::ResizeToContents);
        combo = ui->BackUp_comboBox_Source;
        tree  = ui->BackUp_treeView_ListSources;
    }

    // Build hierarchical model for the TreeComboBox.
    // Column layout matches DeviceTreeView expectations:
    //   col 0 = name, col 1 = type, col 2 = active, col 3 = device ID (idColumn default)
    // Parent nodes (Virtual/Storage) are visible but not selectable.
    // Catalog leaf nodes are selectable.
    QStandardItemModel *comboModel = new QStandardItemModel(this);
    QMap<QString, QStandardItem*> parentItems;

    for (int row = 0; row < model->rowCount(); ++row) {
        const QString parentName = model->item(row, 0)->text(); // col 0 = parent device name
        const int     deviceId   = model->item(row, 1)->text().toInt(); // col 1 = device ID
        const QString deviceName = model->item(row, 2)->text(); // col 2 = catalog name

        if (!parentItems.contains(parentName)) {
            auto *groupItem = new QStandardItem(parentName);
            groupItem->setFlags(Qt::ItemIsEnabled); // not selectable
            const QString parentType = parentTypeMap.value(parentName, "Virtual");
            QList<QStandardItem*> groupRow = {
                groupItem,
                new QStandardItem(parentType), // col 1: type → drives icon selection
                new QStandardItem(),            // col 2: active (n/a for groups)
                new QStandardItem()             // col 3: id (n/a for groups)
            };
            comboModel->appendRow(groupRow);
            parentItems.insert(parentName, groupItem);
        }

        const bool isActive = deviceActiveMap.value(deviceId, false);
        QList<QStandardItem*> childRow = {
            new QStandardItem(deviceName),
            new QStandardItem("Catalog"),                         // col 1: type
            new QStandardItem(isActive ? "1" : "0"),              // col 2: active → icon variant
            new QStandardItem(QString::number(deviceId))          // col 3: id
        };
        parentItems[parentName]->appendRow(childRow);
    }

    DeviceTreeView *comboProxy = new DeviceTreeView(this);
    comboProxy->setSourceModel(comboModel);
    comboProxy->setKatalogTheme(themeID > 0);
    combo->setTreeModel(comboProxy);
    combo->expandToDepth(1);

    // Sync: tree view selection → TreeComboBox (one-way; new selectionModel after setModel)
    connect(tree->selectionModel(), &QItemSelectionModel::selectionChanged,
            this, [combo](const QItemSelection &selected, const QItemSelection &) {
        if (selected.isEmpty()) return;
        const int id = selected.indexes().first().siblingAtColumn(1).data().toInt();
        if (id > 0) combo->setSelectedDeviceId(id);
    });
}

void MainWindow::saveNewMapping()
{
    //Read selections from the combo boxes (primary selection source)
    const int sourceId = ui->BackUp_comboBox_Source->selectedDeviceId();
    const int targetId = ui->BackUp_comboBox_Target->selectedDeviceId();

    if (sourceId <= 0) {
        QMessageBox::warning(this, "Katalog", tr("Select a source catalog first."));
        return;
    }
    if (targetId <= 0) {
        QMessageBox::warning(this, "Katalog", tr("Select a target catalog first."));
        return;
    }

    const QString mappingName = ui->BackUp_lineEdit_Name->text().trimmed();
    if (mappingName.isEmpty()) {
        QMessageBox::warning(this, "Katalog", tr("Provide a mapping name."));
        return;
    }

    if (sourceId == targetId) {
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
                                mapping_strict_copy,
                                mapping_conflict_mode
                            )
                            VALUES
                            (   :mapping_name,
                                :mapping_type,
                                :mapping_device_source_id,
                                :mapping_device_target_id,
                                :mapping_strict_copy,
                                :mapping_conflict_mode
                            )
                        )");
    query.prepare(querySQL);
    query.bindValue(":mapping_name", mappingName);
    query.bindValue(":mapping_type", ui->BackUp_comboBox_CreateMappingType->currentText());
    query.bindValue(":mapping_device_source_id", sourceId);
    query.bindValue(":mapping_device_target_id", targetId);
    query.bindValue(":mapping_strict_copy", ui->BackUp_checkBox_StrictCopy->isChecked() ? 1 : 0);
    query.bindValue(":mapping_conflict_mode",
                    conflictModeToString(static_cast<ConflictMode>(
                        ui->BackUp_comboBox_ConflictMode->currentIndex())));

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
    ui->BackUp_treeView_ListSources->clearSelection();
    ui->BackUp_treeView_ListTargets->clearSelection();

}

void MainWindow::setupBackUpManager()
{
    backupMappingManager = new BackupMappingManager(m_connectionName, this);

    //Hide progress/preview area until the user triggers an action
    ui->BackUp_label_ProgressSummary->setVisible(false);
    ui->BackUp_tableView_PreviewFiles->setVisible(false);
    ui->BackUp_progressBar->setVisible(false);
    ui->BackUp_pushButton_CancelBackup->setVisible(false);
    ui->BackUp_pushButton_ExportPreview->setEnabled(false);
}
