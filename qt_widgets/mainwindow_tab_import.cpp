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
    ui->Import_label_status->setText(tr("Opening source..."));
    QApplication::processEvents();

    if (!m_importer)
        m_importer = new CollectionImporter(collection, this);

    QSqlError err = m_importer->openSource(path);
    if (err.type() != QSqlError::NoError) {
        ui->Import_label_status->setText(tr("Error opening source: %1").arg(err.text()));
        return;
    }

    if (!m_importer->checkSchemaCompatibility()) {
        ui->Import_label_status->setText(
            tr("Schema version mismatch — import aborted. (%1)").arg(m_importer->lastError()));
        m_importer->close();
        return;
    }

    QStandardItemModel *model = m_importer->buildSourceDeviceModel();
    ui->Import_comboBox_sourceDevice->setTreeModel(model);
    ui->Import_comboBox_sourceDevice->setIdColumn(2);
    ui->Import_comboBox_sourceDevice->expandAll();

    ui->Import_label_status->setText(
        tr("Source opened (%1 mode). Select a device and click Import.")
            .arg(m_importer->sourceMode()));
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
        ui->Import_label_status->setText(tr("No source collection is open."));
        return;
    }

    int srcDeviceId = ui->Import_comboBox_sourceDevice->selectedDeviceId();

    ui->Import_label_status->setText(tr("Importing..."));
    QApplication::processEvents();

    bool ok;
    if (srcDeviceId == 0) {
        // Synthetic "Collection" root — import all root-level devices
        ok = (m_importer->importAllDevices() >= 0);
    } else {
        ok = m_importer->importDevice(srcDeviceId);
    }

    if (ok) {
        ui->Import_label_status->setText(tr("Import completed successfully."));
        persistImportToFiles(collection);
        loadCollection();
    } else {
        ui->Import_label_status->setText(
            tr("Import failed: %1").arg(m_importer->lastError()));
    }
}
