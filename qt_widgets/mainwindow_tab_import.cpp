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
// File Name:   mainwindow_tab_import.cpp
// Purpose:     Import / Merge / Update across Collections (#750)
// Description: K2 UI for CollectionImporter (Phase 1)
// Author:      Stephane Couturier
/////////////////////////////////////////////////////////////////////////////
*/

#include "mainwindow.h"
#include "ui_mainwindow.h"
#include "core/collectionimporter.h"
#include "core/statusbarmessagebuilder.h"

#include <QFileDialog>

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
                .setOperation(tr("COLLECTION IMPORT"))
                .setStatus(tr("Error"))
                .setCurrentItem(err.text())
                .build());
        return;
    }

    if (!m_importer->checkSchemaCompatibility()) {
        updateStatusBarMessage(
            StatusBarMessageBuilder()
                .setOperation(tr("COLLECTION IMPORT"))
                .setStatus(tr("Error"))
                .setCurrentItem(tr("Schema version mismatch. Import cancelled. (%1)").arg(m_importer->lastError()))
                .build());
        m_importer->close();
        return;
    }

    QStandardItemModel *model = m_importer->buildSourceDeviceModel();
    ui->Import_comboBox_sourceDevice->setTreeModel(model);
    ui->Import_comboBox_sourceDevice->setIdColumn(2);
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
            tr("Select source collection file"),
            startDir,
            tr("Katalog database (*.db);;All files (*)"));
    } else {
        path = QFileDialog::getExistingDirectory(
            this,
            tr("Select source collection folder (Memory mode)"),
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
                .setOperation(tr("COLLECTION IMPORT"))
                .setStatus(tr("Error"))
                .setCurrentItem(tr("No source collection is open."))
                .build());
        return;
    }

    int srcDeviceId = ui->Import_comboBox_sourceDevice->selectedDeviceId();

    updateStatusBarMessage(
        StatusBarMessageBuilder()
            .setOperation(tr("COLLECTION IMPORT"))
            .setStatus(tr("Importing"))
            .build());
    QApplication::processEvents();

    bool ok;
    if (srcDeviceId == 0) {
        // Synthetic "Collection" root — import all root-level devices
        ok = (m_importer->importAllDevices() >= 0);
    } else {
        ok = m_importer->importDevice(srcDeviceId);
    }

    if (ok) {
        updateStatusBarMessage(
            StatusBarMessageBuilder()
                .setOperation(tr("COLLECTION IMPORT"))
                .setStatus(tr("Completed"))
                .build());
        persistImportToFiles(collection);
        loadCollection();
    } else {
        updateStatusBarMessage(
            StatusBarMessageBuilder()
                .setOperation(tr("COLLECTION IMPORT"))
                .setStatus(tr("Error"))
                .setCurrentItem(m_importer->lastError())
                .build());
    }
}

//----------------------------------------------------------------------
void MainWindow::on_Import_pushButton_updateSelected_clicked()
{
    if (!selectedDevice || selectedDevice->ID == 0) {
        updateStatusBarMessage(
            StatusBarMessageBuilder()
                .setOperation(tr("COLLECTION UPDATE"))
                .setStatus(tr("Error"))
                .setCurrentItem(tr("No device selected."))
                .build());
        return;
    }

    if (!m_importer)
        m_importer = new CollectionImporter(collection, this);

    updateStatusBarMessage(
        StatusBarMessageBuilder()
            .setOperation(tr("COLLECTION UPDATE"))
            .setStatus(tr("Updating..."))
            .build());
    QApplication::processEvents();

    bool ok = m_importer->updateDeviceFromExternalCollection(selectedDevice->ID);

    if (ok) {
        updateStatusBarMessage(
            StatusBarMessageBuilder()
                .setOperation(tr("COLLECTION UPDATE"))
                .setStatus(tr("Completed"))
                .build());
        persistImportToFiles(collection);
        loadCollection();
    } else {
        updateStatusBarMessage(
            StatusBarMessageBuilder()
                .setOperation(tr("COLLECTION UPDATE"))
                .setStatus(tr("Error"))
                .setCurrentItem(m_importer->lastError())
                .build());
    }
}
