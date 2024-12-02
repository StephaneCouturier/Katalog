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

void MainWindow::on_BackUp_pushButton_ReloadTargetList_clicked()
{
    loadBackUpDeviceLists("Target");
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


//Methods-----------------------------------------------------------------------

void MainWindow::loadBackUpMapping()
{
    //Load data from table device_mapping
    QSqlQuery query(QSqlDatabase::database("defaultConnection"));

    QString querySQL = QLatin1String(R"(
                            SELECT
                                dm.mapping_id,
                                dm.mapping_name,
                                dm.mapping_type,
                                dm.mapping_device_source_id,
                                d1.device_name,
                                d1.device_active,
                                d1.device_total_file_size,
                                d1.device_total_file_count,
                                d1.device_date_updated,
                                dm.mapping_device_target_id,
                                d2.device_name,
                                d2.device_active,
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

    querySQL += QLatin1String(R"(
                            ORDER BY dm.mapping_id
                        )");

    query.prepare(querySQL);

    if (!query.exec())
    {
        qDebug() << "Error loading device_mapping: " << query.lastError();
        return;
    }

    //Create an sql model for the table
    QSqlQueryModel *queryModel = new QSqlQueryModel();
    queryModel->setQuery(std::move(query));

    queryModel->setHeaderData(0, Qt::Horizontal, tr("ID"));
    queryModel->setHeaderData(1, Qt::Horizontal, tr("Mapping Name"));
    queryModel->setHeaderData(2, Qt::Horizontal, tr("Type"));
    queryModel->setHeaderData(3, Qt::Horizontal, tr("Source ID"));
    queryModel->setHeaderData(4, Qt::Horizontal, tr("Source"));
    queryModel->setHeaderData(5, Qt::Horizontal, tr("Active"));
    queryModel->setHeaderData(6, Qt::Horizontal, tr("File Size"));
    queryModel->setHeaderData(7, Qt::Horizontal, tr("Files"));
    queryModel->setHeaderData(8, Qt::Horizontal, tr("Date Updated"));
    queryModel->setHeaderData(9, Qt::Horizontal, tr("Target ID"));
    queryModel->setHeaderData(10, Qt::Horizontal, tr("Target"));
    queryModel->setHeaderData(11, Qt::Horizontal, tr("Active"));
    queryModel->setHeaderData(12, Qt::Horizontal, tr("File Size"));
    queryModel->setHeaderData(13, Qt::Horizontal, tr("Files"));
    queryModel->setHeaderData(14, Qt::Horizontal, tr("Date Updated"));
    queryModel->setHeaderData(15, Qt::Horizontal, tr("Size Diff."));
    queryModel->setHeaderData(16, Qt::Horizontal, tr("Size Diff.(%)"));
    queryModel->setHeaderData(17, Qt::Horizontal, tr("Files Diff."));
    queryModel->setHeaderData(18, Qt::Horizontal, tr("Files Diff.(%)"));
    queryModel->setHeaderData(19, Qt::Horizontal, tr("Time Diff."));

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
        ui->BackUp_tableView_CurrentMappings->setColumnHidden(9, true);
        ui->BackUp_tableView_CurrentMappings->setColumnHidden(11, true);
        ui->BackUp_tableView_CurrentMappings->setColumnHidden(12, true);
        ui->BackUp_tableView_CurrentMappings->setColumnHidden(13, true);
        ui->BackUp_tableView_CurrentMappings->setColumnHidden(14, true);
    }
    else
    {

        ui->BackUp_tableView_CurrentMappings->setColumnHidden(3, false);
        ui->BackUp_tableView_CurrentMappings->setColumnHidden(5, false);
        ui->BackUp_tableView_CurrentMappings->setColumnHidden(9, false);
        ui->BackUp_tableView_CurrentMappings->setColumnHidden(11, false);
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
    model->setColumnCount(3);
    model->setHorizontalHeaderItem(0, new QStandardItem("parentID"));
    model->setHorizontalHeaderItem(1, new QStandardItem("ID"));
    model->setHorizontalHeaderItem(2, new QStandardItem("Name"));

    //Populate the model from each deviceListTable if type = "Catalog"
    for (int i = 0; i < selectedDevice->deviceListTable.size(); i++)
    {
        if (selectedDevice->deviceListTable.at(i).type == "Catalog")
        {   //Load device
            Device tempDevice;
            tempDevice.ID = selectedDevice->deviceListTable.at(i).ID;
            tempDevice.loadDevice("defaultConnection");

            //Add row
            QList<QStandardItem*> row;
            row.append(new QStandardItem(QString::number(tempDevice.parentID)));
            row.append(new QStandardItem(QString::number(tempDevice.ID)));
            row.append(new QStandardItem(tempDevice.name));
            model->appendRow(row);
        }
    }

    //Load model to the Source view
    if (list =="Source"){
        ui->BackUp_treeView_List1->setModel(model);
        ui->BackUp_treeView_List1->resizeColumnToContents(1);
        ui->BackUp_treeView_List1->setEditTriggers(QAbstractItemView::NoEditTriggers);
    }

    //Load model to the Target view
    if (list =="Target"){
        ui->BackUp_treeView_List2->setModel(model);
        ui->BackUp_treeView_List2->resizeColumnToContents(1);
        ui->BackUp_treeView_List2->setEditTriggers(QAbstractItemView::NoEditTriggers);
    }
}

void MainWindow::saveNewMapping()
{
    //Get the selected devices from BackUp_treeView_List1 and BackUp_treeView_List2
    QModelIndexList selectedIndexes1 = ui->BackUp_treeView_List1->selectionModel()->selectedIndexes();
    QModelIndexList selectedIndexes2 = ui->BackUp_treeView_List2->selectionModel()->selectedIndexes();

    //Get the value of the second column as device id
    QString device1ID = selectedIndexes1.at(1).data().toString();
    QString device2ID = selectedIndexes2.at(1).data().toString();

    //Get mapping name
    QString mappingName = ui->BackUp_lineEdit_Name->text();

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
