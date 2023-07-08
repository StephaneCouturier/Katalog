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
// File Name:   mainwindow_tab_virtual.cpp
// Purpose:     methods for the screen Virtual
// Description: https://github.com/StephaneCouturier/Katalog/wiki/Virtual
// Author:      Stephane Couturier
/////////////////////////////////////////////////////////////////////////////
*/

#include "mainwindow.h"
#include "ui_mainwindow.h"
#include "devicetreeview.h"

void MainWindow::on_Virtual_pushButton_Load_clicked()
{
    loadVirtualStorageTableToTreeModel();
}
//--------------------------------------------------------------------------
void MainWindow::on_Virtual_pushButton_InsertRootLevel_clicked()
{
    insertVirtualStorageItem(0,"root Item");
}
//--------------------------------------------------------------------------
void MainWindow::on_Virtual_pushButton_InsertChild_clicked()
{
    insertVirtualStorageItem(selectedVirtualStorageID,"child Item");
}
//--------------------------------------------------------------------------
void MainWindow::on_Virtual_pushButton_DeleteItem_clicked()
{
    deleteVirtualStorageItem();
}
//--------------------------------------------------------------------------
void MainWindow::on_Virtual_pushButton_Edit_clicked()
{
    ui->Virtual_widget_Edit->setVisible(true);
    ui->Virtual_lineEdit_Name->setText(selectedVirtualStorageName);
}
//--------------------------------------------------------------------------
void MainWindow::on_Virtual_pushButton_Save_clicked()
{

}
//--------------------------------------------------------------------------
void MainWindow::on_Virtual_pushButton_Cancel_clicked()
{

}
void MainWindow::on_Virtual_treeView_VirutalStorageList_clicked(const QModelIndex &index)
{
    selectedVirtualStorageID   = ui->Virtual_treeView_VirutalStorageList->model()->index(index.row(), 2, index.parent() ).data().toInt();
    selectedVirtualStorageName = ui->Virtual_treeView_VirutalStorageList->model()->index(index.row(), 0, index.parent() ).data().toString();
}
//--------------------------------------------------------------------------
void MainWindow::insertVirtualStorageItem(int parentID, QString name)
{
    //Generate new ID
    QSqlQuery query;
    QString querySQL = QLatin1String(R"(
                            SELECT MAX(virtual_storage_id)
                            FROM virtual_storage
                        )");
    query.prepare(querySQL);
    query.exec();
    query.next();
    int newID=query.value(0).toInt()+1;

    //Insert device
    querySQL = QLatin1String(R"(
                            INSERT INTO virtual_storage(
                                        virtual_storage_id,
                                        virtual_storage_parent_id,
                                        virtual_storage_name)
                            VALUES(
                                        :virtual_storage_id,
                                        :virtual_storage_parent_id,
                                        :virtual_storage_name)
                                )");
    query.prepare(querySQL);
    query.bindValue(":virtual_storage_id",newID);
    query.bindValue(":virtual_storage_parent_id",parentID);
    query.bindValue(":virtual_storage_name",name);
    query.exec();

    //Save data to file
    if (databaseMode == "Memory"){
        //Save file
        saveVirtualStorageTableToFile(virtualStorageFilePath);
    }

    //Reload
    loadVirtualStorageTableToTreeModel();
    //loadVirtualStorageTableToSelectionTreeModel()
}
//--------------------------------------------------------------------------
void MainWindow::deleteVirtualStorageItem()
{
    int result = QMessageBox::warning(this,"Katalog",
                                      tr("Do you want to delete this virtual storage item?")+"\n"+selectedCatalog->filePath,QMessageBox::Yes|QMessageBox::Cancel);

    if ( result ==QMessageBox::Yes){
        //Delete selected ID
        QSqlQuery query;
        QString querySQL = QLatin1String(R"(
                            DELETE FROM virtual_storage
                            WHERE virtual_storage_id=:virtual_storage_id
                        )");
        query.prepare(querySQL);
        query.bindValue(":virtual_storage_id",selectedVirtualStorageID);
        query.exec();
        //Save data to file
        if (databaseMode == "Memory"){
            //Save file
            saveVirtualStorageTableToFile(virtualStorageFilePath);
        }

        //Reload
        loadVirtualStorageTableToTreeModel();
        //loadVirtualStorageTableToSelectionTreeModel()
    }
}
//--------------------------------------------------------------------------
void MainWindow::loadVirtualStorageFileToTable()
{
    QSqlQuery query;
    QString querySQL;
    querySQL = QLatin1String(R"(
                        DELETE FROM virtual_storage
                    )");
    query.prepare(querySQL);
    query.exec();

    querySQL = QLatin1String(R"(
                        INSERT INTO virtual_storage (
                                        virtual_storage_id,
                                        virtual_storage_parent_id,
                                        virtual_storage_name )
                        VALUES(         :virtual_storage_id,
                                        :virtual_storage_parent_id,
                                        :virtual_storage_name )
                    )");
    query.prepare(querySQL);

    //Define storage file and prepare stream
    QFile virtualStorageFile(virtualStorageFilePath);
    QTextStream textStream(&virtualStorageFile);

    //Open file or return information
    if(!virtualStorageFile.open(QIODevice::ReadOnly)) {
        // Create it, if it does not exist
        QFile newVirtualStorageFile(virtualStorageFilePath);
        newVirtualStorageFile.open(QFile::WriteOnly | QFile::Text);
        QTextStream stream(&newVirtualStorageFile);
        stream << "ID"            << "\t"
               << "Parent ID"     << "\t"
               << "Name"          << "\t"
               << '\n';
        newVirtualStorageFile.close();
    }

    //Test file validity (application breaks between v0.13 and v0.14)
    QString line = textStream.readLine();

    //Load virtualStorage device lines to table
    while (true)
    {
        QString line = textStream.readLine();
        if (line.isNull())
            break;
        else
            if (line.left(2)!="ID"){//skip the first line with headers

                //Split the string with tabulation into a list
                QStringList fieldList = line.split('\t');

                QString querySQL = QLatin1String(R"(
                                INSERT INTO virtual_storage(
                                        virtual_storage_id,
                                        virtual_storage_parent_id,
                                        virtual_storage_name )
                                VALUES(
                                        :virtual_storage_id,
                                        :virtual_storage_parent_id,
                                        :virtual_storage_name )
                                )");

                QSqlQuery insertQuery;
                insertQuery.prepare(querySQL);
                insertQuery.bindValue(":virtual_storage_id",fieldList[0].toInt());
                insertQuery.bindValue(":virtual_storage_parent_id",fieldList[1]);
                insertQuery.bindValue(":virtual_storage_name",fieldList[2]);
                insertQuery.exec();
            }
    }
    virtualStorageFile.close();
}
//--------------------------------------------------------------------------
void MainWindow::loadVirtualStorageCatalogFileToTable()
{
    QSqlQuery query;
    QString querySQL;
    querySQL = QLatin1String(R"(
                        DELETE FROM virtual_storage_catalog
                    )");
    query.prepare(querySQL);
    query.exec();

    querySQL = QLatin1String(R"(
                        INSERT INTO virtual_storage_catalog (
                                        virtual_storage_id,
                                        catalog_name )
                        VALUES(         :virtual_storage_id,
                                        :catalog_name )
                    )");
    query.prepare(querySQL);

    //Define storage file and prepare stream
    QFile virtualStorageCatalogFile(virtualStorageCatalogFilePath);
    QTextStream textStream(&virtualStorageCatalogFile);

    //Open file or return information
    if(!virtualStorageCatalogFile.open(QIODevice::ReadOnly)) {
        // Create it, if it does not exist
        QFile newVirtualStorageCatalogFile(virtualStorageCatalogFilePath);
        if(!newVirtualStorageCatalogFile.open(QIODevice::ReadOnly)) {
            if (newVirtualStorageCatalogFile.open(QFile::WriteOnly | QFile::Text)) {
                QTextStream stream(&newVirtualStorageCatalogFile);
                stream << "ID"            << "\t"
                       << "Name"          << "\t"
                       << '\n';
                newVirtualStorageCatalogFile.close();
            }
        }
    }

    //Test file validity (application breaks between v0.13 and v0.14)
    QString line = textStream.readLine();

    //Load virtualStorage device lines to table
    while (true)
    {
        QString line = textStream.readLine();
        if (line.isNull())
            break;
        else if (line.left(2)!="ID"){//skip the first line with headers

            //Split the string with tabulation into a list
            QStringList fieldList = line.split('\t');
            query.bindValue(":virtual_storage_id",fieldList[0].toInt());
            query.bindValue(":catalog_name",fieldList[1]);
            query.exec();
        }
    }
    virtualStorageCatalogFile.close();
}
//--------------------------------------------------------------------------
void MainWindow::loadVirtualStorageTableToTreeModel()
{
    //Prepare query for catalogs
    QSqlQuery queryCatalog;
    QString queryCatalogSQL;
    queryCatalogSQL = QLatin1String(R"(
                        SELECT vsc.catalog_name,'Catalog', c.catalog_source_path_is_active
                        FROM  virtual_storage_catalog vsc
                        INNER JOIN catalog c ON vsc.catalog_name = c.catalog_name
                        WHERE vsc.virtual_storage_id =:virtual_storage_id
                        ORDER BY vsc.catalog_name
                    )");
    queryCatalog.prepare(queryCatalogSQL);

    // Create the tree model
    QStandardItemModel *model = new QStandardItemModel();
    model->setHorizontalHeaderLabels({ tr("Name"), tr("Device Type"), tr("ID"), tr("Parent ID") });

    // Retrieve data from the database
    QSqlQuery query;
    QString querySQL;
    querySQL = QLatin1String(R"(
                    SELECT  virtual_storage_id,
                            virtual_storage_parent_id,
                            virtual_storage_name
                    FROM  virtual_storage
                )");
    query.prepare(querySQL);
    query.exec();

    //Populate Model
    // Create a map to store items by ID for easy access
    QMap<int, QStandardItem*> itemMap;

    while (query.next()) {
        int id = query.value(0).toInt();
        int parentId = query.value(1).toInt();
        QString name = query.value(2).toString();

        // Create the item for this row
        QList<QStandardItem*> rowItems;
        rowItems << new QStandardItem(name);
        rowItems << new QStandardItem("VirtualStorage");
        rowItems << new QStandardItem(QString::number(id));
        rowItems << new QStandardItem(QString::number(parentId));

        QStandardItem* item = rowItems.at(0); // Get the item representing the name
        QStandardItem* parentItem = itemMap.value(parentId);

        if (parentId == 0) {
            model->appendRow(rowItems); // Add top-level items directly to the model
        } else {
            if (parentItem) {
                parentItem->appendRow(rowItems); // Append the row to the parent item
            } else {
                qDebug() << "Parent item not found for ID:" << id;
                continue; // Skip this row and proceed to the next one
            }
        }

        // Store the item in the map for future reference
        itemMap.insert(id, item);
    }

    //Load Model to treeview
    // Connect model to tree/table view
    DeviceTreeView *proxyStorageModel = new DeviceTreeView(this);
    proxyStorageModel->setSourceModel(model);
    //    proxyStorageModel->setHeaderData(0, Qt::Horizontal, tr("ID"));
    //    proxyStorageModel->setHeaderData(1, Qt::Horizontal, tr("Name"));
    //    proxyStorageModel->setHeaderData(2, Qt::Horizontal, tr("Type"));
    //    proxyStorageModel->setHeaderData(3, Qt::Horizontal, tr("Location"));
    ui->Virtual_treeView_VirutalStorageList->setModel(proxyStorageModel);

    ui->Virtual_treeView_VirutalStorageList->QTreeView::sortByColumn(0,Qt::AscendingOrder);
    ui->Virtual_treeView_VirutalStorageList->header()->setSectionResizeMode(QHeaderView::Interactive);
    ui->Virtual_treeView_VirutalStorageList->header()->resizeSection(0, 300); //Name
    ui->Virtual_treeView_VirutalStorageList->header()->resizeSection(1, 125); //Type
    ui->Virtual_treeView_VirutalStorageList->header()->resizeSection(2, 50); //ID
    ui->Virtual_treeView_VirutalStorageList->header()->resizeSection(3, 50); //Parent ID
    ui->Virtual_treeView_VirutalStorageList->header()->hideSection(1); //Type
    ui->Virtual_treeView_VirutalStorageList->expandAll();
}
//--------------------------------------------------------------------------
void MainWindow::saveVirtualStorageTableToFile(QString filePath)
{
    QFile virtualStorageFile(filePath);

    //Get data
    QSqlQuery query;
    QString querySQL = QLatin1String(R"(
                             SELECT * FROM virtual_storage
                                        )");
    query.prepare(querySQL);
    query.exec();

    //Write data
    if (virtualStorageFile.open(QFile::WriteOnly | QFile::Text)) {

        QTextStream textStreamToFile(&virtualStorageFile);

        //Prepare header line
        textStreamToFile << "ID"         << "\t"
                         << "Parent ID"  << "\t"
                         << "Name"       << "\t"
                         << '\n';

        //Iterate the records and generate lines
        while (query.next()) {
            const QSqlRecord record = query.record();
            for (int i=0, recCount = record.count() ; i<recCount ; ++i){
                if (i>0)
                    textStreamToFile << '\t';
                textStreamToFile << record.value(i).toString();
            }
            //Write the result in the file
            textStreamToFile << '\n';
        }
        virtualStorageFile.close();
    }
    else{
        QMessageBox msgBox;
        msgBox.setWindowTitle("Katalog");
        msgBox.setText(tr("Error opening output file."));
        msgBox.setIcon(QMessageBox::Warning);
        msgBox.exec();
    }
}
//--------------------------------------------------------------------------
