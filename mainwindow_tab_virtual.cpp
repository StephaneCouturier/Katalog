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

//--------------------------------------------------------------------------
void MainWindow::on_Virtual_pushButton_InsertRootLevel_clicked()
{
    insertVirtualStorageItem(0,tr("Top Item"));
}
//--------------------------------------------------------------------------
void MainWindow::on_Virtual_pushButton_AddSubItem_clicked()
{
    insertVirtualStorageItem(selectedVirtualStorageID,tr("Sub-Item"));
}
//--------------------------------------------------------------------------
void MainWindow::on_Virtual_pushButton_AssignCatalog_clicked()
{
    assignCatalogToVirtualStorage(selectedCatalog->name, selectedVirtualStorageID);
}
//--------------------------------------------------------------------------
void MainWindow::on_Virtual_pushButton_UnassignCatalog_clicked()
{
        unassignCatalogToVirtualStorage(selectedVirtualStorageName, selectedVirtualStorageParentID);
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
    //Save name
    QSqlQuery query;
    QString querySQL = QLatin1String(R"(
                            UPDATE virtual_storage
                            SET    virtual_storage_name=:virtual_storage_name
                            WHERE  virtual_storage_id=:virtual_storage_id
                                )");
    query.prepare(querySQL);
    query.bindValue(":virtual_storage_id",selectedVirtualStorageID);
    query.bindValue(":virtual_storage_name",ui->Virtual_lineEdit_Name->text());
    query.exec();

    ui->Virtual_widget_Edit->hide();

    //Save data to file
    if (databaseMode == "Memory"){
        //Save file
        saveVirtualStorageTableToFile(virtualStorageFilePath);
    }

    //Reload
    loadVirtualStorageTableToTreeModel();

    ui->Virtual_pushButton_Edit->setEnabled(false);
}
//--------------------------------------------------------------------------
void MainWindow::on_Virtual_pushButton_Cancel_clicked()
{
    ui->Virtual_widget_Edit->hide();
}
//--------------------------------------------------------------------------
void MainWindow::on_Virtual_checkBox_DisplayCatalogs_stateChanged(int arg1)
{
    QSettings settings(settingsFilePath, QSettings:: IniFormat);
    settings.setValue("Virtual/DisplayCatalogs", arg1);
    loadVirtualStorageTableToTreeModel();
}
//--------------------------------------------------------------------------
void MainWindow::on_Virtual_treeView_VirutalStorageList_clicked(const QModelIndex &index)
{
    //Get selection data
    selectedVirtualStorageName = ui->Virtual_treeView_VirutalStorageList->model()->index(index.row(), 0, index.parent() ).data().toString();
    selectedVirtualStorageType = ui->Virtual_treeView_VirutalStorageList->model()->index(index.row(), 1, index.parent() ).data().toString();
    selectedVirtualStorageID   = ui->Virtual_treeView_VirutalStorageList->model()->index(index.row(), 3, index.parent() ).data().toInt();
    QModelIndex parentIndex = index.parent();
    selectedVirtualStorageParentID = parentIndex.sibling(parentIndex.row(), 3).data().toInt();
    QString selectedVirtualStorageParentName = parentIndex.sibling(parentIndex.row(), 0).data().toString();

    //Adapt buttons to selection
    if(selectedVirtualStorageType=="VirtualStorage"){
        ui->Virtual_pushButton_Edit->setEnabled(true);
        if(selectedCatalog->name!=""){
            ui->Virtual_pushButton_AssignCatalog->setEnabled(true);
        }
        ui->Virtual_pushButton_UnassignCatalog->setEnabled(false);
        ui->Virtual_pushButton_DeleteItem->setEnabled(true);
        ui->Virtual_label_SelectedAssignedCatalogDisplay->setText("");
        ui->Virtual_label_SelectedVirtualStorage->setText(selectedVirtualStorageName);
        ui->Virtual_label_SelectedVirtualStorageParent->setText("");
    }
    else if(selectedVirtualStorageType=="Catalog"){
        ui->Virtual_pushButton_Edit->setEnabled(false);
        ui->Virtual_pushButton_AssignCatalog->setEnabled(false);
        ui->Virtual_pushButton_UnassignCatalog->setEnabled(true);
        ui->Virtual_pushButton_DeleteItem->setEnabled(false);
        ui->Virtual_label_SelectedAssignedCatalogDisplay->setText(selectedVirtualStorageName);
        ui->Virtual_label_SelectedVirtualStorageParent->setText(selectedVirtualStorageParentName);
        ui->Virtual_label_SelectedVirtualStorage->setText("");
    }
}
//--------------------------------------------------------------------------
void MainWindow::on_Virtual_treeView_VirutalStorageList_customContextMenuRequested(const QPoint &pos)
{
    //Get selection data
    QModelIndex index=ui->Virtual_treeView_VirutalStorageList->currentIndex();
    selectedVirtualStorageName = ui->Virtual_treeView_VirutalStorageList->model()->index(index.row(), 0, index.parent() ).data().toString();
    selectedVirtualStorageType = ui->Virtual_treeView_VirutalStorageList->model()->index(index.row(), 1, index.parent() ).data().toString();
    selectedVirtualStorageID   = ui->Virtual_treeView_VirutalStorageList->model()->index(index.row(), 3, index.parent() ).data().toInt();
    QModelIndex parentIndex = index.parent();
    selectedVirtualStorageParentID = parentIndex.sibling(parentIndex.row(), 3).data().toInt();
    QString selectedVirtualStorageParentName = parentIndex.sibling(parentIndex.row(), 0).data().toString();

    //Set actions for catalogs
    if(selectedVirtualStorageType=="Catalog"){
        QPoint globalPos = ui->Virtual_treeView_VirutalStorageList->mapToGlobal(pos);
        QMenu virtualStorageContextMenu;

        QString virtualStorageName = selectedVirtualStorageName;

        QAction *menuVirtualStorageAction1 = new QAction(QIcon::fromTheme("edit-cut"), tr("Unassign this catalog"), this);
        virtualStorageContextMenu.addAction(menuVirtualStorageAction1);

        connect(menuVirtualStorageAction1, &QAction::triggered, this, [this, virtualStorageName]() {
            unassignCatalogToVirtualStorage(selectedVirtualStorageName, selectedVirtualStorageParentID);
        });

        virtualStorageContextMenu.exec(globalPos);
    }
    else{
        QPoint globalPos = ui->Virtual_treeView_VirutalStorageList->mapToGlobal(pos);
        QMenu virtualStorageContextMenu;

        QString virtualStorageName = selectedVirtualStorageName;

        QAction *menuVirtualStorageAction1 = new QAction(QIcon::fromTheme("document-new"), tr("Add sub-item"), this);
        virtualStorageContextMenu.addAction(menuVirtualStorageAction1);

        QAction *menuVirtualStorageAction2 = new QAction(QIcon::fromTheme("document-edit-sign"), tr("Edit"), this);
        virtualStorageContextMenu.addAction(menuVirtualStorageAction2);

        QAction *menuVirtualStorageAction3 = new QAction(QIcon::fromTheme("edit-delete"), tr("Delete item"), this);
        virtualStorageContextMenu.addAction(menuVirtualStorageAction3);

        connect(menuVirtualStorageAction1, &QAction::triggered, this, [this, virtualStorageName]() {
            insertVirtualStorageItem(selectedVirtualStorageID,"sub-item");
        });

        connect(menuVirtualStorageAction2, &QAction::triggered, this, [this, virtualStorageName]() {
            ui->Virtual_widget_Edit->setVisible(true);
            ui->Virtual_lineEdit_Name->setText(selectedVirtualStorageName);
        });

        connect(menuVirtualStorageAction3, &QAction::triggered, this, [this, virtualStorageName]() {
            deleteVirtualStorageItem();
        });

        virtualStorageContextMenu.exec(globalPos);
    }
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
    if(ui->Filter_comboBox_TreeType->currentText()==tr("Virtual Storage / Catalog")){
        loadVirtualStorageTableToSelectionTreeModel();
    }
}
//--------------------------------------------------------------------------
void MainWindow::assignCatalogToVirtualStorage(QString catalogName,int virtualStorageID)
{
    if( virtualStorageID!=0 and catalogName!=""){
        //Insert catalog
        QSqlQuery query;
        QString querySQL = QLatin1String(R"(
                            INSERT INTO virtual_storage_catalog(
                                        virtual_storage_id,
                                        catalog_name,
                                        directory_path)
                            VALUES(
                                        :virtual_storage_id,
                                        :catalog_name,
                                        :directory_path)
                        )");
        query.prepare(querySQL);
        query.bindValue(":virtual_storage_id", virtualStorageID);
        query.bindValue(":catalog_name", catalogName);
        query.bindValue(":directory_path", "/");
        query.exec();

        //Save data to file
        if (databaseMode == "Memory"){
            //Save file
            saveVirtualStorageCatalogTableToFile(virtualStorageCatalogFilePath);
        }

        //Reload
        loadVirtualStorageTableToTreeModel();
        if(ui->Filter_comboBox_TreeType->currentText()==tr("Virtual Storage / Catalog")){
            loadVirtualStorageTableToSelectionTreeModel();
        }
    }
}
//--------------------------------------------------------------------------
void MainWindow::unassignCatalogToVirtualStorage(QString catalogName,int virtualStorageParentID)
{
    int result = QMessageBox::warning(this,"Katalog",
                                      tr("Do you want to remove this catalog from this virtual storage?"),QMessageBox::Yes|QMessageBox::Cancel);

    if ( result ==QMessageBox::Yes){
        //unassignCatalogToVirtualStorage(selectedVirtualStorageName, selectedVirtualStorageParentID);

        if( virtualStorageParentID!=0 and catalogName!=""){
            //Insert catalog
            QSqlQuery query;
            QString querySQL = QLatin1String(R"(
                                        DELETE FROM virtual_storage_catalog
                                        WHERE virtual_storage_id=:virtual_storage_id
                                        AND catalog_name=:catalog_name
                                    )");
            query.prepare(querySQL);
            query.bindValue(":virtual_storage_id",virtualStorageParentID);
            query.bindValue(":catalog_name",catalogName);
            query.exec();

            //Save data to file
            if (databaseMode == "Memory"){
                //Save file
                saveVirtualStorageCatalogTableToFile(virtualStorageCatalogFilePath);
            }

            //Reload
            loadVirtualStorageTableToTreeModel();
            if(ui->Filter_comboBox_TreeType->currentText()==tr("Virtual Storage / Catalog")){
                loadVirtualStorageTableToSelectionTreeModel();
            }
        }
        ui->Virtual_pushButton_UnassignCatalog->setEnabled(false);
    }
}
//--------------------------------------------------------------------------
void MainWindow::deleteVirtualStorageItem()
{
    bool hasChildren = false;
    bool hasCatalog  = false;
    QSqlQuery queryVerifyChildren;
    QString queryVerifyChildrenSQL = QLatin1String(R"(
                                SELECT COUNT(*)
                                FROM virtual_storage
                                WHERE virtual_storage_parent_id=:virtual_storage_parent_id
                            )");
    queryVerifyChildren.prepare(queryVerifyChildrenSQL);
    queryVerifyChildren.bindValue(":virtual_storage_parent_id",selectedVirtualStorageID);
    queryVerifyChildren.exec();
    queryVerifyChildren.next();

    if(queryVerifyChildren.value(0).toInt()>=1)
        hasChildren = true;

    QSqlQuery queryVerifyCatalog;
    QString queryVerifyCatalogSQL = QLatin1String(R"(
                                SELECT COUNT(*)
                                FROM virtual_storage_catalog
                                WHERE virtual_storage_id=:virtual_storage_id
                            )");
    queryVerifyCatalog.prepare(queryVerifyCatalogSQL);
    queryVerifyCatalog.bindValue(":virtual_storage_id",selectedVirtualStorageID);
    queryVerifyCatalog.exec();
    queryVerifyCatalog.next();

    if(queryVerifyCatalog.value(0).toInt()>=1)
        hasCatalog = true;

    if ( hasChildren == false ){
        if ( hasCatalog == false ){
            int result = QMessageBox::warning(this,"Katalog",
                                              tr("Do you want to delete this virtual storage item?")+"<br/>"+selectedVirtualStorageName,QMessageBox::Yes|QMessageBox::Cancel);

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
                if(ui->Filter_comboBox_TreeType->currentText()==tr("Virtual Storage / Catalog")){
                    loadVirtualStorageTableToSelectionTreeModel();
                }
            }
        }
        else{
            QMessageBox msgBox;
            msgBox.setWindowTitle("Katalog");
            msgBox.setText(tr("The selected item cannot be deleted as long as it has catalogs linked."));
            msgBox.setIcon(QMessageBox::Warning);
            msgBox.exec();
        }
    }
    else{
        QMessageBox msgBox;
        msgBox.setWindowTitle("Katalog");
        msgBox.setText(tr("The selected item cannot be deleted as long as it has sub-items."));
        msgBox.setIcon(QMessageBox::Warning);
        msgBox.exec();
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
                                        catalog_name,
                                        directory_path )
                        VALUES(         :virtual_storage_id,
                                        :catalog_name,
                                        :directory_path )
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
                       << "Catalog Name"          << "\t"
                       << "Directory Path"          << "\t"
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
            query.bindValue(":directory_path",fieldList[2]);
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
    model->setHorizontalHeaderLabels({ tr("Name"), tr("Device Type"), tr("Active"), tr("ID"), tr("Parent ID") });

    // Retrieve data from the database
    QSqlQuery query;
    QString querySQL;
    querySQL = QLatin1String(R"(
                    SELECT  virtual_storage_id,
                            virtual_storage_parent_id,
                            virtual_storage_name
                    FROM  virtual_storage
                    ORDER BY virtual_storage_id ASC
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
        rowItems << new QStandardItem("");
        rowItems << new QStandardItem(QString::number(id));
        rowItems << new QStandardItem(QString::number(parentId));

        QStandardItem* item = rowItems.at(0); // Get the item representing the name
        QStandardItem* parentItem = itemMap.value(parentId);

        if (parentId == 0) {
            model->appendRow(rowItems); // Add top-level items directly to the model
        }
        else{
            if (parentItem) {
                parentItem->appendRow(rowItems); // Append the row to the parent item
            }
            else{
                qDebug() << "Parent item not found for ID:" << id;
                continue; // Skip this row and proceed to the next one
            }
        }

        // Store the item in the map for future reference
        itemMap.insert(id, item);

        if(ui->Virtual_checkBox_DisplayCatalogs->isChecked()==true){
            //Add Catalogs
            queryCatalog.bindValue(":virtual_storage_id",id);
            queryCatalog.exec();
            while(queryCatalog.next()){
                //Get data
                QList<QStandardItem*> rowCatalogItems;
                rowCatalogItems << new QStandardItem(queryCatalog.value(0).toString());
                rowCatalogItems << new QStandardItem(queryCatalog.value(1).toString());
                rowCatalogItems << new QStandardItem(queryCatalog.value(2).toString());
//                rowCatalogItems << new QStandardItem("");

                //Add items
                item->appendRow(rowCatalogItems);
            }
        }
    }

    //Load Model to treeview
    // Connect model to tree/table view
    DeviceTreeView *proxyStorageModel = new DeviceTreeView(this);
    proxyStorageModel->setSourceModel(model);
    ui->Virtual_treeView_VirutalStorageList->setModel(proxyStorageModel);

    ui->Virtual_treeView_VirutalStorageList->QTreeView::sortByColumn(0,Qt::AscendingOrder);
    ui->Virtual_treeView_VirutalStorageList->header()->setSectionResizeMode(QHeaderView::Interactive);
    ui->Virtual_treeView_VirutalStorageList->header()->resizeSection(0, 400); //Name
    ui->Virtual_treeView_VirutalStorageList->header()->resizeSection(1, 125); //Type
    ui->Virtual_treeView_VirutalStorageList->header()->resizeSection(2, 50);  //Active
    ui->Virtual_treeView_VirutalStorageList->header()->resizeSection(3, 100);  //ID
    ui->Virtual_treeView_VirutalStorageList->header()->resizeSection(4, 100);  //Parent ID
    ui->Virtual_treeView_VirutalStorageList->header()->hideSection(1); //Type
    ui->Virtual_treeView_VirutalStorageList->header()->hideSection(2); //Active
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
void MainWindow::saveVirtualStorageCatalogTableToFile(QString filePath)
{
    QFile virtualStorageCatalogFile(filePath);

    //Get data
    QSqlQuery query;
    QString querySQL = QLatin1String(R"(
                             SELECT * FROM virtual_storage_catalog
                        )");
    query.prepare(querySQL);
    query.exec();

    //Write data
    if (virtualStorageCatalogFile.open(QFile::WriteOnly | QFile::Text)) {
        QTextStream textStreamToFile(&virtualStorageCatalogFile);

        //Prepare header line
        textStreamToFile << "ID"    << "\t"
                         << "Name"  << "\t"
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
        virtualStorageCatalogFile.close();
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
