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

#include "mainwindow.h"
#include "devicemappingview.h"
#include "ui_mainwindow.h"

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
    QString mappingID = selectedIndexes.at(0).data().toString();

    //Delete the mapping in the table device_mapping
    QSqlQuery query(QSqlDatabase::database("defaultConnection"));
    QString querySQL = QLatin1String(R"(
                            DELETE FROM device_mapping
                            WHERE mapping_id = :mapping_id
                        )");
    query.prepare(querySQL);
    query.bindValue(":mapping_id", mappingID);

    if (!query.exec())
    {
        qDebug() << "Error deleting device_mapping: " << query.lastError();
        return;
    }

    //Reload the mapping table
    loadBackUpMapping();
    collection->saveMappingTableToFile();
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

//Methods-----------------------------------------------------------------------
void MainWindow::loadBackUpMapping()
{
    loadBackUpMappingTotals();
    loadBackUpMappingTable();
}

void MainWindow::loadBackUpMappingTotals()
{

    ui->BackUp_label_CurrentMappings_DeviceValue->setText(selectedDevice->name);

    //Load data from table device_mapping
    QSqlQuery query(QSqlDatabase::database("defaultConnection"));
    QString querySQL;
    querySQL = QLatin1String(R"(
        SELECT
            COUNT(*) AS total_mappings,

            SUM(d1.device_total_file_size) AS total_source_file_size,
            SUM(d2.device_total_file_size) AS total_target_file_size,
            SUM(d2.device_total_file_size - d1.device_total_file_size) AS total_size_difference,
            ROUND(SUM(d2.device_total_file_size) * 100.0 / NULLIF(SUM(d1.device_total_file_size), 0), 2) AS total_size_difference_percentage,

            SUM(d1.device_total_file_count) AS total_source_file_count,
            SUM(d2.device_total_file_count) AS total_target_file_count,
            SUM(d2.device_total_file_count - d1.device_total_file_count) AS total_file_count_difference,
            ROUND(SUM(d2.device_total_file_count) * 100.0 / NULLIF(SUM(d1.device_total_file_count), 0), 2) AS total_file_count_difference_percentage,

            ROUND(AVG(d2.device_total_file_size) * 100.0 / NULLIF(AVG(d1.device_total_file_size), 0), 2) AS avg_size_difference_percentage,
            ROUND(AVG(d2.device_total_file_count) * 100.0 / NULLIF(AVG(d1.device_total_file_count), 0), 2) AS avg_file_count_difference_percentage
        FROM
            device_mapping dm,
            device d1,
            device d2
        WHERE
            dm.mapping_device_source_id = d1.device_id
            AND dm.mapping_device_target_id = d2.device_id
                        )");

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

    //querySQL +=" ORDER BY dm.mapping_name ASC ";
    query.prepare(querySQL);
    query.bindValue(":device_id",        selectedDevice->ID);
    query.bindValue(":device_parent_id", selectedDevice->ID);

    if (!query.exec())
    {
        qDebug() << "Error loading device_mapping: " << query.lastError();
        return;
    }

    query.next();
    ui->BackUp_label_TotalMappings_Value->setText(query.value(0).toString());

    qint64 mapped = query.value(2).toLongLong();
    qint64 difference = selectedDevice->totalFileSize - mapped;
    float coverage = static_cast<float>(selectedDevice->totalFileSize - difference) / static_cast<float>(selectedDevice->totalFileSize) * 100;

    ui->BackUp_label_TotalMappings_DeviceCoverageLabelSizeValue->setText(QLocale().formattedDataSize(selectedDevice->totalFileSize));
    ui->BackUp_label_TotalMappings_DeviceCoverageLabelSizeMapped->setText(QLocale().formattedDataSize(mapped));
    ui->BackUp_label_TotalMappings_DeviceCoverageLabelSizePercentValue->setText(QLocale().toString(coverage, 'f', 2) + " %");
    ui->BackUp_label_TotalMappings_DeviceCoverageLabelSizeDiffValue->setText(QLocale().formattedDataSize((selectedDevice->totalFileSize - query.value(2).toLongLong())) + "  ");
    ui->BackUp_label_TotalMappings_DeviceCoverageLabelSizePercentUnlinkedValue->setText(QLocale().toString(100-coverage, 'f', 2) + " %");

    ui->BackUp_label_TotalMappings_SizeSourceValue->setText(QLocale().formattedDataSize((query.value(1).toLongLong())) + "  ");
    ui->BackUp_label_TotalMappings_SizeTargetValue->setText(QLocale().formattedDataSize((query.value(2).toLongLong())) + "  ");
    ui->BackUp_label_TotalMappings_SizeDiffValue->setText(QLocale().formattedDataSize(query.value(3).toLongLong()) + "  ");
    ui->BackUp_label_TotalMappings_SizePercentValue->setText(QLocale().toString(query.value(4).toDouble(), 'f', 2) + " %");

    ui->BackUp_label_TotalMappings_FilesSourceTotal->setText(query.value(5).toString());
    ui->BackUp_label_TotalMappings_FilesTargetTotal->setText(query.value(6).toString());
    ui->BackUp_label_TotalMappings_FilesDiffValue->setText(QLocale().toString(query.value(7).toLongLong()) + "  ");
    ui->BackUp_label_TotalMappings_FilesPercentValue->setText(QLocale().toString(query.value(8).toDouble(), 'f', 2) + " %");

}

void MainWindow::loadBackUpMappingTable()
{
    //Load data from table device_mapping
    QSqlQuery query(QSqlDatabase::database("defaultConnection"));
    QString querySQL;
    querySQL = QLatin1String(R"(
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

                                (
                                    PRINTF('%02d:%02d:%02d %02d:%02d:%02d',
                                        CAST(ABS((julianday(d2.device_date_updated) - julianday(d1.device_date_updated)) * 24 * 60 * 60 / 31536000) AS INTEGER),
                                        CAST(ABS(((julianday(d2.device_date_updated) - julianday(d1.device_date_updated)) * 24 * 60 * 60 % 31536000) / 2592000) AS INTEGER),
                                        CAST(ABS(((julianday(d2.device_date_updated) - julianday(d1.device_date_updated)) * 24 * 60 * 60 % 2592000) / 86400) AS INTEGER),
                                        CAST(ABS(((julianday(d2.device_date_updated) - julianday(d1.device_date_updated)) * 24 * 60 * 60 % 86400) / 3600) AS INTEGER),
                                        CAST(ABS(((julianday(d2.device_date_updated) - julianday(d1.device_date_updated)) * 24 * 60 * 60 % 3600) / 60) AS INTEGER),
                                        CAST(ABS((julianday(d2.device_date_updated) - julianday(d1.device_date_updated)) * 24 * 60 * 60 % 60) AS INTEGER)
                                    )
                                ) AS formatted_time_difference

                            FROM device_mapping dm,
                                device d1,
                                device d2
                            WHERE dm.mapping_device_source_id = d1.device_id
                            AND   dm.mapping_device_target_id = d2.device_id
                        )");

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
    //proxyModel->caseSensitive = fileSortCaseSensitive;
    proxyModel->setSourceModel(queryModel);

    //Load model to the view
    ui->BackUp_tableView_CurrentMappings->setModel(proxyModel);
    ui->BackUp_tableView_CurrentMappings->resizeColumnsToContents();
    ui->BackUp_tableView_CurrentMappings->setEditTriggers(QAbstractItemView::NoEditTriggers);

    //If the setting is checked, display all columns
    QSettings settings(collection->settingsFilePath, QSettings:: IniFormat);
    if (optionDisplayFullMappingTable == false)// ui->BackUp_checkBox_DisplayFullTable->isChecked() == false )
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
            tempDevice.loadDevice("defaultConnection");

            Device tempParentDevice;
            tempParentDevice.ID = tempDevice.parentID;
            tempParentDevice.loadDevice("defaultConnection");

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
    QSqlQuery query(QSqlDatabase::database("defaultConnection"));
    QString querySQL = QLatin1String(R"(
                            INSERT INTO device_mapping
                            (   mapping_name,
                                mapping_type,
                                mapping_device_source_id,
                                mapping_device_target_id
                            )
                            VALUES
                            (   :mapping_name,
                                :mapping_type,
                                :mapping_device_source_id,
                                :mapping_device_target_id
                            )
                        )");
    query.prepare(querySQL);
    query.bindValue(":mapping_name", mappingName);
    query.bindValue(":mapping_type", "Backup");
    query.bindValue(":mapping_device_source_id", device1ID);
    query.bindValue(":mapping_device_target_id", device2ID);

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
