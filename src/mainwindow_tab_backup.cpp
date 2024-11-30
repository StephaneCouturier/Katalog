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
#include "ui_mainwindow.h"

void MainWindow::on_BackUp_pushButton_SaveMapping_clicked()
{
    saveNewMapping();
    collection->saveMappingTableToFile();
}
void MainWindow::on_BackUp_pushButton_ReloadLists_clicked()
{
    loadBackUpDeviceLists();
}

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
                        dm.mapping_device_target_id,
                        d2.device_name,
                        dm.mapping_backup_last_date,
                        dm.mapping_backup_last_size
                    FROM device_mapping dm,
                        device d1,
                        device d2
                    WHERE dm.mapping_device_source_id = d1.device_id
                    AND   dm.mapping_device_target_id = d2.device_id
                                )");

    query.prepare(querySQL);

    if (!query.exec())
    {
        qDebug() << "Error loading device_mapping: " << query.lastError();
        return;
    }

    //Create a model for the table
    QStandardItemModel *model = new QStandardItemModel();
    model->setColumnCount(2);
    model->setHorizontalHeaderItem(0, new QStandardItem("id"));
    model->setHorizontalHeaderItem(1, new QStandardItem("name"));
    model->setHorizontalHeaderItem(2, new QStandardItem("type"));
    model->setHorizontalHeaderItem(3, new QStandardItem("device_source_id"));
    model->setHorizontalHeaderItem(3, new QStandardItem("device_source_name"));
    model->setHorizontalHeaderItem(4, new QStandardItem("device_target_id"));
    model->setHorizontalHeaderItem(4, new QStandardItem("device_target_name"));
    model->setHorizontalHeaderItem(5, new QStandardItem("backup_last_date"));
    model->setHorizontalHeaderItem(6, new QStandardItem("backup_last_size"));

    //Populate model from the query
    while (query.next())
    {
        QList<QStandardItem*> row;
        row.append(new QStandardItem(query.value(0).toString()));
        row.append(new QStandardItem(query.value(1).toString()));
        row.append(new QStandardItem(query.value(2).toString()));
        row.append(new QStandardItem(query.value(3).toString()));
        row.append(new QStandardItem(query.value(4).toString()));
        row.append(new QStandardItem(query.value(5).toString()));
        row.append(new QStandardItem(query.value(6).toString()));
        model->appendRow(row);
    }

    //Load model to the view
    ui->BackUp_tableView_CurrentMappings->setModel(model);
    ui->BackUp_tableView_CurrentMappings->resizeColumnsToContents();
    ui->BackUp_tableView_CurrentMappings->setEditTriggers(QAbstractItemView::NoEditTriggers);
}
void MainWindow::loadBackUpDeviceLists()
{
    //Create a model for the table
    QStandardItemModel *model = new QStandardItemModel();
    model->setColumnCount(3);
    model->setHorizontalHeaderItem(0, new QStandardItem("parentname"));
    model->setHorizontalHeaderItem(1, new QStandardItem("id"));
    model->setHorizontalHeaderItem(2, new QStandardItem("name"));

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

    //Load model to the view
    ui->BackUp_treeView_List1->setModel(model);
    ui->BackUp_treeView_List1->resizeColumnToContents(1);
    ui->BackUp_treeView_List1->setEditTriggers(QAbstractItemView::NoEditTriggers);
    //Load model to the view
    ui->BackUp_treeView_List2->setModel(model);
    ui->BackUp_treeView_List2->resizeColumnToContents(1);
    ui->BackUp_treeView_List2->setEditTriggers(QAbstractItemView::NoEditTriggers);
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
