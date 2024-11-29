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
    collection->saveMappingTableToFile();
}

void MainWindow::loadBackUpMapping()
{
    qDebug() << "loadBackUpMapping";

    //Load data from table device_mapping
    QSqlQuery query(QSqlDatabase::database("defaultConnection"));

    //select data from device_mapping and device tables
    // query.prepare("SELECT bm.id, "
    //                   "bm.name, "
    //                   "d1.name as device_source, "
    //                   "d2.name as device_target, "
    //                   "bm.backup_last_date, "
    //                   "bm.backup_last_size "
    //               "FROM device_mapping bm, "
    //                   "device d1, "
    //                   "device d2 "
    //               "WHERE bm.device_source_id = d1.id AND bm.device_target_id = d2.id");

    //select data from device_mapping table joining with device table
    // query.prepare("SELECT bm.id, "
    //                   "bm.name, "
    //                   "d1.name as device_source, "
    //                   "d2.name as device_target, "
    //                   "bm.backup_last_date, "
    //                   "bm.backup_last_size "
    //               "FROM device_mapping bm "
    //               "JOIN device d1 ON bm.device_source_id = d1.id "
    //               "JOIN device d2 ON bm.device_target_id = d2.id");

    query.prepare("SELECT * FROM device_mapping");

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
    model->setHorizontalHeaderItem(4, new QStandardItem("device_target_id"));
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
        qDebug() << "row: " << row;
    }

    //Load model to the view
    ui->BackUp_tableView_CurrentMappings->setModel(model);

}
