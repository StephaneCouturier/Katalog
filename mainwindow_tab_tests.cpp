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
// File Name:   mainwindow_tab_tests.cpp
// Purpose:
// Description:
// Author:      Stephane Couturier
// Modified by: Stephane Couturier
// Created:     2020-07-11
// Version:     0.1
/////////////////////////////////////////////////////////////////////////////
*/

#include "mainwindow.h"
#include "ui_mainwindow.h"
#include "catalog.h"
#include "database.h"
#include <QSortFilterProxyModel>
#include <QDesktopServices>
//----------------------------------------------------------------------
void MainWindow::startSQLDB()
{
    if (!QSqlDatabase::drivers().contains("QSQLITE"))
        QMessageBox::critical(
                    this,
                    "Unable to load database",
                    "This demo needs the SQLITE driver"
                    );

    // Initialize the database:
    /*
     * QSqlError err = InitializeDatabase();
    if (err.type() != QSqlError::NoError) {
        //showError(err);
        return;
    }


    //Statistics_TaV_Test
    // Create the data model:
    model2 = new QSqlRelationalTableModel(this);
    model2->setEditStrategy(QSqlTableModel::OnManualSubmit);
    model2->setTable("storage");

    // Set the localized header captions:
    model2->setHeaderData(0, Qt::Horizontal, tr("ID"));
    model2->setHeaderData(1, Qt::Horizontal, tr("Name"));
    model2->setHeaderData(2, Qt::Horizontal, tr("Type"));
    model2->setHeaderData(3, Qt::Horizontal, tr("Location"));
    model2->setHeaderData(4, Qt::Horizontal, tr("Path"));
    model2->setHeaderData(5, Qt::Horizontal, tr("Label"));
    model2->setHeaderData(6, Qt::Horizontal, tr("FileSystem"));
    model2->setHeaderData(7, Qt::Horizontal, tr("Total"));
    model2->setHeaderData(8, Qt::Horizontal, tr("Free"));

    // Populate the model:
    if (!model2->select()) {
        //showError(model->lastError());
        return;
    }

    // Set the model
    ui->test_tableView->setModel(model2);
    ui->test_tableView->setSelectionMode(QAbstractItemView::SingleSelection);

    ui->test_listView->setModel(model2);

    ui->test_treeView->setModel(model2);
    QSortFilterProxyModel *proxyStorageModel = new QSortFilterProxyModel(this);
    proxyStorageModel->setSourceModel(model2);

    // Connect model to tree/table view
    ui->test_treeView->setModel(proxyStorageModel);
    ui->test_treeView->QTreeView::sortByColumn(1,Qt::AscendingOrder);
    ui->test_treeView->QTreeView::sortByColumn(0,Qt::AscendingOrder);
    ui->test_treeView->header()->setSectionResizeMode(QHeaderView::Interactive);
    ui->test_treeView->header()->resizeSection(0,  50); //ID
    ui->test_treeView->header()->resizeSection(1, 150); //Name
    ui->test_treeView->header()->resizeSection(2, 100); //type
    ui->test_treeView->header()->resizeSection(3, 175); //location
    ui->test_treeView->header()->resizeSection(4, 250); //Path
    ui->test_treeView->header()->resizeSection(5,  50); //Path
    ui->test_treeView->header()->resizeSection(6,  75); //FS
    ui->test_treeView->header()->resizeSection(7,  75); //Total
    ui->test_treeView->header()->resizeSection(8,  75); //Free
    //ui->test_treeView->header()->hideSection(1); //Path
*/
}
//----------------------------------------------------------------------
void MainWindow::on_test_pb_insert_clicked()
{
    KMessageBox::information(this,"clicked.");


/*
    QSqlQuery q;
    if (!q.exec(INSERT_STORAGE_SQL))
        return;// q.lastError();
    QVariant storageId5 = addStorage(q,
                                    3,
                                    "Maxtor_3Tb",
                                    "tbd3",
                                    "DK-External Drives 3'5",
                                    "/run/media/stephane/Maxtor_3Tb",
                                    "tbd",
                                    "tbd",
                                    3000,
                                    30);

    KMessageBox::information(this,"insert done.");


    model2 = new QSqlRelationalTableModel(this);
    model2->setEditStrategy(QSqlTableModel::OnManualSubmit);
    model2->setTable("storage");

*/

}
//----------------------------------------------------------------------

//----------------------------------------------------------------------

/*
    // Create the data model:
    model = new QSqlRelationalTableModel(this);
    model->setEditStrategy(QSqlTableModel::OnManualSubmit);
    model->setTable("books");

    // Remember the indexes of the columns:
    authorIdx = model->fieldIndex("author");
    genreIdx = model->fieldIndex("genre");

    // Set the relations to the other database tables:
    model->setRelation(authorIdx, QSqlRelation("authors", "id", "name"));
    model->setRelation(genreIdx, QSqlRelation("genres", "id", "name"));

    // Set the localized header captions:
    model->setHeaderData(authorIdx, Qt::Horizontal, tr("Author Name"));
    model->setHeaderData(genreIdx, Qt::Horizontal, tr("Genre"));
    model->setHeaderData(model->fieldIndex("title"),
                         Qt::Horizontal, tr("Title"));
    model->setHeaderData(model->fieldIndex("year"), Qt::Horizontal, tr("Year"));
    model->setHeaderData(model->fieldIndex("rating"),
                         Qt::Horizontal, tr("Rating"));

    // Populate the model:
    if (!model->select()) {
        //showError(model->lastError());
        return;
    }

    // Set the model and hide the ID column:
    ui->tableView_books->setModel(model);
    //ui->tableView_books->setItemDelegate(new BookDelegate(ui.bookTable));
    ui->tableView_books->setColumnHidden(model->fieldIndex("id"), true);
    ui->tableView_books->setSelectionMode(QAbstractItemView::SingleSelection);
*/


// Remember the indexes of the columns:
//authorIdx = model->fieldIndex("author");
// = model->fieldIndex("genre");

// Set the relations to the other database tables:
//model->setRelation(authorIdx, QSqlRelation("authors", "id", "name"));
//model->setRelation(genreIdx, QSqlRelation("genres", "id", "name"));
