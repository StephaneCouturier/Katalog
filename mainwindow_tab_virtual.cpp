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
#include "virtualstorage.h"

//TAB: VIRTUAL -------------------------------------------------------------

//--- UI -------------------------------------------------------------------
//--------------------------------------------------------------------------
void MainWindow::on_Virtual_pushButton_InsertRootLevel_clicked()
{
    insertVirtualStorageItem(0, 0, tr("Top Item"), "VirtualStorage", 0);
}
//--------------------------------------------------------------------------
void MainWindow::on_Virtual_pushButton_AddSubItem_clicked()
{
    insertVirtualStorageItem(0, selectedVirtualStorageID, tr("Sub-Item"), "VirtualStorage", 0);
}
//--------------------------------------------------------------------------
void MainWindow::on_Virtual_pushButton_AssignCatalog_clicked()
{
    assignCatalogToVirtualStorage(selectedCatalog->name, selectedVirtualStorageID);
}
//--------------------------------------------------------------------------
void MainWindow::on_Virtual_pushButton_AssignStorage_clicked()
{
    assignStorageToVirtualStorage(selectedStorage->ID, selectedVirtualStorageID);
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
void MainWindow::on_Virtual_pushButton_TreeExpandCollapse_clicked()
{
    setVirtualStorageTreeExpandState(true);
}
//--------------------------------------------------------------------------
void MainWindow::on_Virtual_checkBox_DisplayCatalogs_stateChanged(int arg1)
{
    QSettings settings(settingsFilePath, QSettings:: IniFormat);
    settings.setValue("Virtual/DisplayCatalogs", arg1);
    loadVirtualStorageTableToTreeModel();
}
//--------------------------------------------------------------------------
void MainWindow::on_Virtual_checkBox_DisplayPhysicalGroupOnly_stateChanged(int arg1)
{
    QSettings settings(settingsFilePath, QSettings:: IniFormat);
    settings.setValue("Virtual/DisplayPhysicalGroupOnly", arg1);
    if(arg1>0)
        ui->Virtual_checkBox_DisplayAllExceptPhysicalGroup->setChecked(false);
    loadVirtualStorageTableToTreeModel();
}
//--------------------------------------------------------------------------
void MainWindow::on_Virtual_checkBox_DisplayAllExceptPhysicalGroup_stateChanged(int arg1)
{
    QSettings settings(settingsFilePath, QSettings:: IniFormat);
    settings.setValue("Virtual/DisplayAllExceptPhysicalGroup", arg1);
    if(arg1>0)
        ui->Virtual_checkBox_DisplayPhysicalGroupOnly->setChecked(false);
    loadVirtualStorageTableToTreeModel();
}
//--------------------------------------------------------------------------
void MainWindow::on_Virtual_checkBox_DisplayFullTable_stateChanged(int arg1)
{
    QSettings settings(settingsFilePath, QSettings:: IniFormat);
    settings.setValue("Virtual/DisplayFullTable", arg1);
    loadVirtualStorageTableToTreeModel();
}
//--------------------------------------------------------------------------
void MainWindow::on_Virtual_treeView_VirtualStorageList_clicked(const QModelIndex &index)
{
    //Get selection data
    selectedVirtualStorageName = ui->Virtual_treeView_VirtualStorageList->model()->index(index.row(), 0, index.parent() ).data().toString();
    selectedVirtualStorageType = ui->Virtual_treeView_VirtualStorageList->model()->index(index.row(), 1, index.parent() ).data().toString();
    selectedVirtualStorageID   = ui->Virtual_treeView_VirtualStorageList->model()->index(index.row(), 3, index.parent() ).data().toInt();
    QModelIndex parentIndex = index.parent();
    selectedVirtualStorageParentID = parentIndex.sibling(parentIndex.row(), 3).data().toInt();
    QString selectedVirtualStorageParentName = parentIndex.sibling(parentIndex.row(), 0).data().toString();

    //Adapt buttons to selection
    if(selectedVirtualStorageType=="VirtualStorage"){
        ui->Virtual_pushButton_Edit->setEnabled(true);
        if(selectedCatalog->name!=""){
            ui->Virtual_pushButton_AssignCatalog->setEnabled(true);
        }
        ui->Virtual_pushButton_DeleteItem->setEnabled(true);
        ui->Virtual_label_SelectedVirtualStorage->setText(selectedVirtualStorageName);
    }
    else if(selectedVirtualStorageType=="Catalog"){
        ui->Virtual_pushButton_Edit->setEnabled(false);
        ui->Virtual_pushButton_AssignCatalog->setEnabled(false);
        ui->Virtual_pushButton_DeleteItem->setEnabled(false);
        ui->Virtual_label_SelectedVirtualStorage->setText("");
    }
}
//--------------------------------------------------------------------------
void MainWindow::on_Virtual_treeView_VirtualStorageList_customContextMenuRequested(const QPoint &pos)
{
    //Get selection data
    QModelIndex index=ui->Virtual_treeView_VirtualStorageList->currentIndex();
    selectedVirtualStorageName = ui->Virtual_treeView_VirtualStorageList->model()->index(index.row(), 0, index.parent() ).data().toString();
    selectedVirtualStorageType = ui->Virtual_treeView_VirtualStorageList->model()->index(index.row(), 1, index.parent() ).data().toString();
    selectedVirtualStorageID   = ui->Virtual_treeView_VirtualStorageList->model()->index(index.row(), 3, index.parent() ).data().toInt();
    QModelIndex parentIndex = index.parent();
    selectedVirtualStorageParentID = parentIndex.sibling(parentIndex.row(), 3).data().toInt();
    QString selectedVirtualStorageParentName = parentIndex.sibling(parentIndex.row(), 0).data().toString();

    //Set actions for catalogs
    if(selectedVirtualStorageType=="Catalog"){
        QPoint globalPos = ui->Virtual_treeView_VirtualStorageList->mapToGlobal(pos);
        QMenu virtualStorageContextMenu;

        QString virtualStorageName = selectedVirtualStorageName;

        QAction *menuVirtualStorageAction1 = new QAction(QIcon::fromTheme("media-playlist-repeat"), tr("Update"), this);
        virtualStorageContextMenu.addAction(menuVirtualStorageAction1);

        connect(menuVirtualStorageAction1, &QAction::triggered, this, [this, virtualStorageName]() {
            updateAllNumbers();
        });

        virtualStorageContextMenu.addSeparator();

        QAction *menuVirtualStorageAction3 = new QAction(QIcon::fromTheme("edit-cut"), tr("Unassign this catalog"), this);
        virtualStorageContextMenu.addAction(menuVirtualStorageAction3);

        connect(menuVirtualStorageAction3, &QAction::triggered, this, [this, virtualStorageName]() {
            unassignPhysicalFromVirtualStorage(selectedVirtualStorageID, selectedVirtualStorageParentID);
        });

        virtualStorageContextMenu.exec(globalPos);
    }
    else if(selectedVirtualStorageType=="Storage"){
        QPoint globalPos = ui->Virtual_treeView_VirtualStorageList->mapToGlobal(pos);
        QMenu virtualStorageContextMenu;

        QString virtualStorageName = selectedVirtualStorageName;

        QAction *menuVirtualStorageAction1 = new QAction(QIcon::fromTheme("media-playlist-repeat"), tr("Update"), this);
        virtualStorageContextMenu.addAction(menuVirtualStorageAction1);

        connect(menuVirtualStorageAction1, &QAction::triggered, this, [this, virtualStorageName]() {
            updateNumbers(selectedVirtualStorageID, selectedVirtualStorageType);
        });

        virtualStorageContextMenu.addSeparator();

        QAction *menuVirtualStorageAction3 = new QAction(QIcon::fromTheme("edit-cut"), tr("Unassign this storage"), this);
        virtualStorageContextMenu.addAction(menuVirtualStorageAction3);

        connect(menuVirtualStorageAction3, &QAction::triggered, this, [this, virtualStorageName]() {
            unassignPhysicalFromVirtualStorage(selectedVirtualStorageID, selectedVirtualStorageParentID);
        });

        virtualStorageContextMenu.exec(globalPos);
    }
    else{
        QPoint globalPos = ui->Virtual_treeView_VirtualStorageList->mapToGlobal(pos);
        QMenu virtualStorageContextMenu;

        QString virtualStorageName = selectedVirtualStorageName;

        QAction *menuVirtualStorageAction1 = new QAction(QIcon::fromTheme("document-new"), tr("Add sub-item"), this);
        virtualStorageContextMenu.addAction(menuVirtualStorageAction1);

        connect(menuVirtualStorageAction1, &QAction::triggered, this, [this, virtualStorageName]() {
            insertVirtualStorageItem(0, selectedVirtualStorageID, "sub-item", "VirtualStorage" , 0);
        });


        QAction *menuVirtualStorageAction2 = new QAction(QIcon::fromTheme("document-edit-sign"), tr("Edit"), this);
        virtualStorageContextMenu.addAction(menuVirtualStorageAction2);

        connect(menuVirtualStorageAction2, &QAction::triggered, this, [this, virtualStorageName]() {
            ui->Virtual_widget_Edit->setVisible(true);
            ui->Virtual_lineEdit_Name->setText(selectedVirtualStorageName);
        });

        QAction *menuVirtualStorageAction3 = new QAction(QIcon::fromTheme("media-playlist-repeat"), tr("Update"), this);
        virtualStorageContextMenu.addAction(menuVirtualStorageAction3);

        connect(menuVirtualStorageAction3, &QAction::triggered, this, [this, virtualStorageName]() {
            updateNumbers(selectedVirtualStorageID, selectedVirtualStorageType);
        });

        virtualStorageContextMenu.addSeparator();

        QAction *menuVirtualStorageAction4 = new QAction(QIcon::fromTheme("edit-delete"), tr("Delete"), this);
        virtualStorageContextMenu.addAction(menuVirtualStorageAction4);

        connect(menuVirtualStorageAction4, &QAction::triggered, this, [this, virtualStorageName]() {
            deleteVirtualStorageItem();
        });

        virtualStorageContextMenu.exec(globalPos);
    }
}
//--------------------------------------------------------------------------

//--- UI / Methods temp v1.x -----------------------------------------------
void MainWindow::on_Virtual_pushButton_ImportS_clicked()
{
    convertVirtualStorageCatalogFile();
    importStorageCatalogLinks();
}
//--------------------------------------------------------------------------
void MainWindow::convertVirtualStorageCatalogFile() {

    QSqlQuery query;
    QString querySQL;

    //Update VirtualStorage type
    //save table to update columns and reload
    saveVirtualStorageTableToFile(virtualStorageFilePath);
    loadVirtualStorageFileToTable();

    //update type
    querySQL = QLatin1String(R"(
                            UPDATE virtual_storage
                            SET virtual_storage_type="VirtualStorage"
                            WHERE virtual_storage_type=""
                        )");
    query.prepare(querySQL);
    query.exec();

    saveVirtualStorageTableToFile(virtualStorageFilePath);
    loadVirtualStorageFileToTable();
    loadVirtualStorageTableToTreeModel();

    //Insert Catalog assignments from virtual_storage_catalog
    querySQL = QLatin1String(R"(
                        INSERT INTO virtual_storage(
                                        virtual_storage_id,
                                        virtual_storage_parent_id,
                                        virtual_storage_name,
                                        virtual_storage_type,
                                        virtual_storage_external_id )
                        VALUES(         :virtual_storage_id,
                                        :virtual_storage_parent_id,
                                        :virtual_storage_name,
                                        :virtual_storage_type,
                                        :virtual_storage_external_id )
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
    //Load virtualStorage device lines to table
    while (true)
    {
        QString line = textStream.readLine();
        if (line.isNull())
            break;
        else if (line.left(2)!="ID"){//skip the first line with headers

            //Generate new ID
            QSqlQuery queryID;
            QString queryIDSQL = QLatin1String(R"(
                            SELECT MAX(virtual_storage_id)
                            FROM virtual_storage
                        )");
            queryID.prepare(queryIDSQL);
            queryID.exec();
            queryID.next();
            int newID=queryID.value(0).toInt()+1;

            //Split the string with tabulation into a list
            QStringList fieldList = line.split('\t');

            query.bindValue(":virtual_storage_id",newID);
            query.bindValue(":virtual_storage_parent_id",fieldList[0]);
            query.bindValue(":virtual_storage_name",fieldList[1]);
            query.bindValue(":virtual_storage_type","Catalog");
            query.bindValue(":virtual_storage_external_id",fieldList[1]);
            query.exec();
        }
    }
    virtualStorageCatalogFile.close();
    //Save file
    saveVirtualStorageTableToFile(virtualStorageFilePath);
    loadVirtualStorageTableToTreeModel();

}
//--------------------------------------------------------------------------
void MainWindow::importStorageCatalogLinks() {

    QSqlQuery query;
    QString querySQL;
    querySQL = QLatin1String(R"(
                            SELECT catalog_storage,
                                    catalog_name,
                                    storage.storage_id,
                                    virtual_storage.virtual_storage_id
                            FROM catalog
                            JOIN storage ON storage.storage_name = catalog.catalog_storage
                            JOIN virtual_storage ON storage.storage_id = virtual_storage.virtual_storage_external_id
                         )");
    query.prepare(querySQL);
    query.exec();

    while (query.next()) {
        QString storage_name = query.value(0).toString();
        QString catalog_name = query.value(1).toString();
        QString storage_id   = query.value(2).toString();
        int vstorage_id   = query.value(3).toInt();

        selectedCatalog->setName(catalog_name);
        selectedCatalog->loadCatalogMetaData();

        assignCatalogToVirtualStorage(catalog_name,vstorage_id);

        saveVirtualStorageTableToFile(virtualStorageFilePath);
        loadVirtualStorageTableToTreeModel();
    }
}
//--------------------------------------------------------------------------

//--- Methods --------------------------------------------------------------
//--------------------------------------------------------------------------
void MainWindow::insertVirtualStorageItem(int newID, int parentID, QString name, QString type, QString externalID)
{
    QSqlQuery query;
    QString querySQL;

    if(newID==0){
        //Generate new ID
        querySQL = QLatin1String(R"(
                        SELECT MAX(virtual_storage_id)
                        FROM virtual_storage
                    )");
        query.prepare(querySQL);
        query.exec();
        query.next();
        newID=query.value(0).toInt()+1;
    }

    //Insert device
    querySQL = QLatin1String(R"(
                            INSERT INTO virtual_storage(
                                        virtual_storage_id,
                                        virtual_storage_parent_id,
                                        virtual_storage_name,
                                        virtual_storage_type,
                                        virtual_storage_external_id)
                            VALUES(
                                        :virtual_storage_id,
                                        :virtual_storage_parent_id,
                                        :virtual_storage_name,
                                        :virtual_storage_type,
                                        :virtual_storage_external_id)
                                )");
    query.prepare(querySQL);
    query.bindValue(":virtual_storage_id",newID);
    query.bindValue(":virtual_storage_parent_id",parentID);
    query.bindValue(":virtual_storage_name",name);
    query.bindValue(":virtual_storage_type", type);
    query.bindValue(":virtual_storage_external_id", externalID);
    query.exec();

    //Save data to file
    saveVirtualStorageTableToFile(virtualStorageFilePath);

    //Reload
    loadVirtualStorageTableToTreeModel();
    if(ui->Filter_comboBox_TreeType->currentText()==tr("Virtual Storage / Catalog")){
        loadVirtualStorageTableToTreeModel();
    }
}
//--------------------------------------------------------------------------
void MainWindow::assignCatalogToVirtualStorage(QString catalogName,int virtualStorageID)
{
    if( virtualStorageID!=0 and catalogName!=""){

        //Generate new ID
        QSqlQuery queryID;
        QString queryIDSQL = QLatin1String(R"(
                            SELECT MAX(virtual_storage_id)
                            FROM virtual_storage
                        )");
        queryID.prepare(queryIDSQL);
        queryID.exec();
        queryID.next();
        int newID = queryID.value(0).toInt()+1;

        //Insert catalog
        QSqlQuery query;
        QString querySQL = QLatin1String(R"(
                            INSERT INTO virtual_storage(
                                        virtual_storage_id,
                                        virtual_storage_parent_id,
                                        virtual_storage_name,
                                        virtual_storage_type,
                                        virtual_storage_external_id,
                                        virtual_storage_path,
                                        virtual_storage_total_file_size,
                                        virtual_storage_total_file_count,
                                        virtual_storage_total_space,
                                        virtual_storage_free_space)
                            VALUES(
                                        :virtual_storage_id,
                                        :virtual_storage_parent_id,
                                        :virtual_storage_name,
                                        :virtual_storage_type,
                                        :virtual_storage_external_id,
                                        :virtual_storage_path,
                                        :virtual_storage_total_file_size,
                                        :virtual_storage_total_file_count,
                                        :virtual_storage_total_space,
                                        :virtual_storage_free_space)
                        )");
        query.prepare(querySQL);
        query.bindValue(":virtual_storage_id", newID);
        query.bindValue(":virtual_storage_parent_id", virtualStorageID);
        query.bindValue(":virtual_storage_name", selectedCatalog->name);
        query.bindValue(":virtual_storage_type", "Catalog");
        query.bindValue(":virtual_storage_external_id", selectedCatalog->name);
        query.bindValue(":virtual_storage_path", selectedCatalog->sourcePath);
        query.bindValue(":virtual_storage_total_file_size", selectedCatalog->totalFileSize);
        query.bindValue(":virtual_storage_total_file_count", selectedCatalog->fileCount);
        query.bindValue(":virtual_storage_total_space", 0);
        query.bindValue(":virtual_storage_free_space", 0);
        query.exec();

        //Save data to file
        if (databaseMode == "Memory"){
            //Save file
            saveVirtualStorageTableToFile(virtualStorageFilePath);
        }

        //Reload
        loadVirtualStorageTableToTreeModel();
        if(ui->Filter_comboBox_TreeType->currentText()==tr("Virtual Storage / Catalog")){
            loadVirtualStorageTableToTreeModel();
        }
    }
}
//--------------------------------------------------------------------------
void MainWindow::assignStorageToVirtualStorage(int storageID,int virtualStorageID)
{
    if( virtualStorageID!=0 and storageID!=0){

        //Generate new ID
        QSqlQuery queryID;
        QString queryIDSQL = QLatin1String(R"(
                            SELECT MAX(virtual_storage_id)
                            FROM virtual_storage
                        )");
        queryID.prepare(queryIDSQL);
        queryID.exec();
        queryID.next();
        int newID = queryID.value(0).toInt()+1;

        //Insert storage
        QSqlQuery query;
        QString querySQL = QLatin1String(R"(
                            INSERT INTO virtual_storage(
                                        virtual_storage_id,
                                        virtual_storage_parent_id,
                                        virtual_storage_name,
                                        virtual_storage_type,
                                        virtual_storage_external_id,
                                        virtual_storage_path,
                                        virtual_storage_total_file_size,
                                        virtual_storage_total_file_count,
                                        virtual_storage_total_space,
                                        virtual_storage_free_space)
                            VALUES(
                                        :virtual_storage_id,
                                        :virtual_storage_parent_id,
                                        :virtual_storage_name,
                                        :virtual_storage_type,
                                        :virtual_storage_external_id,
                                        :virtual_storage_path,
                                        :virtual_storage_total_file_size,
                                        :virtual_storage_total_file_count,
                                        :virtual_storage_total_space,
                                        :virtual_storage_free_space)
                        )");
        query.prepare(querySQL);
        query.bindValue(":virtual_storage_id", newID);
        query.bindValue(":virtual_storage_parent_id", virtualStorageID);
        query.bindValue(":virtual_storage_name", selectedStorage->name);
        query.bindValue(":virtual_storage_type", "Storage");
        query.bindValue(":virtual_storage_external_id", selectedStorage->ID);
        query.bindValue(":virtual_storage_path", selectedStorage->path);
        query.bindValue(":virtual_storage_total_file_size", 0);
        query.bindValue(":virtual_storage_total_file_count", 0);
        query.bindValue(":virtual_storage_total_space", selectedStorage->totalSpace);
        query.bindValue(":virtual_storage_free_space", selectedStorage->freeSpace);
        query.exec();

        //Save data to file
        if (databaseMode == "Memory"){
            //Save file
            saveVirtualStorageTableToFile(virtualStorageFilePath);
        }

        //Reload
        loadVirtualStorageTableToTreeModel();
        if(ui->Filter_comboBox_TreeType->currentText()==tr("Virtual Storage / Catalog")){
            //loadVirtualStorageTableToSelectionTreeModel();
        }
    }
}
//--------------------------------------------------------------------------
void MainWindow::unassignPhysicalFromVirtualStorage(int virtualStorageID, int virtualStorageParentID)
{
    int result = QMessageBox::warning(this,"Katalog",
                                      tr("Do you want to remove this storage or catalog from this virtual storage?"),QMessageBox::Yes|QMessageBox::Cancel);

    if ( result ==QMessageBox::Yes){
        //unassignCatalogToVirtualStorage(selectedVirtualStorageName, selectedVirtualStorageParentID);

        if( virtualStorageID!=0 and virtualStorageParentID!=0){
            //Insert catalog
            QSqlQuery query;
            QString querySQL = QLatin1String(R"(
                                        DELETE FROM virtual_storage
                                        WHERE virtual_storage_id=:virtual_storage_id
                                        AND   virtual_storage_parent_id=:virtual_storage_parent_id
                                    )");
            query.prepare(querySQL);
            query.bindValue(":virtual_storage_id",virtualStorageID);
            query.bindValue(":virtual_storage_parent_id",virtualStorageParentID);
            query.exec();

            //Save data to file
            if (databaseMode == "Memory"){
                //Save file
                saveVirtualStorageTableToFile(virtualStorageCatalogFilePath);
            }

            //Reload
            loadVirtualStorageTableToTreeModel();
            if(ui->Filter_comboBox_TreeType->currentText()==tr("Virtual Storage / Catalog")){
                loadVirtualStorageTableToTreeModel();
            }
        }
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
                    loadVirtualStorageTableToTreeModel();
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
    if(databaseMode=="Memory"){
        //Clear table
        QSqlQuery query;
        QString querySQL;
        querySQL = QLatin1String(R"(
                        DELETE FROM virtual_storage
                    )");
        query.prepare(querySQL);
        query.exec();

        //Define storage file and prepare stream
        QFile virtualStorageFile(virtualStorageFilePath);
        QTextStream textStream(&virtualStorageFile);

        //Open file or create it
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
                    QSqlQuery insertQuery;
                    querySQL = QLatin1String(R"(
                        INSERT INTO virtual_storage (
                                        virtual_storage_id,
                                        virtual_storage_parent_id,
                                        virtual_storage_name,
                                        virtual_storage_type,
                                        virtual_storage_external_id,
                                        virtual_storage_path,
                                        virtual_storage_total_file_size,
                                        virtual_storage_total_file_count,
                                        virtual_storage_total_space,
                                        virtual_storage_free_space )
                        VALUES(
                                        :virtual_storage_id,
                                        :virtual_storage_parent_id,
                                        :virtual_storage_name,
                                        :virtual_storage_type,
                                        :virtual_storage_external_id,
                                        :virtual_storage_path,
                                        :virtual_storage_total_file_size,
                                        :virtual_storage_total_file_count,
                                        :virtual_storage_total_space,
                                        :virtual_storage_free_space )
                    )");
                    insertQuery.prepare(querySQL);
                    insertQuery.bindValue(":virtual_storage_id",fieldList[0].toInt());
                    insertQuery.bindValue(":virtual_storage_parent_id",fieldList[1]);
                    insertQuery.bindValue(":virtual_storage_name",fieldList[2]);
                    if(fieldList.size()>3){//prevent issues with files created in v1.22
                        insertQuery.bindValue(":virtual_storage_type",fieldList[3]);
                        insertQuery.bindValue(":virtual_storage_external_id",fieldList[4]);
                        insertQuery.bindValue(":virtual_storage_path",fieldList[5]);
                        insertQuery.bindValue(":virtual_storage_total_file_size",fieldList[6]);
                        insertQuery.bindValue(":virtual_storage_total_file_count",fieldList[7]);
                        insertQuery.bindValue(":virtual_storage_total_space",fieldList[8]);
                        insertQuery.bindValue(":virtual_storage_free_space",fieldList[9]);
                    }
                    insertQuery.exec();
                }
        }
        virtualStorageFile.close();

        insertPhysicalStorageGroup();
    }
}
//--------------------------------------------------------------------------
void MainWindow::loadVirtualStorageTableToTreeModel()
{
    //Synch data to virtual_storage
    synchCatalogAndStorageValues();

    //Retrieve virtual_storage hierarchy
    QSqlQuery query;
    QString querySQL;

    querySQL = QLatin1String(R"(
                    SELECT  virtual_storage_id,
                            virtual_storage_parent_id,
                            virtual_storage_name,
                            virtual_storage_type,
                            virtual_storage_external_id,
                            virtual_storage_path,
                            virtual_storage_total_file_size,
                            virtual_storage_total_file_count,
                            virtual_storage_total_space,
                            virtual_storage_free_space,
                            virtual_storage_active
                    FROM  virtual_storage
                )");

    if (ui->Virtual_checkBox_DisplayPhysicalGroupOnly->isChecked() == true) {
        querySQL = QLatin1String(R"(

                    WITH RECURSIVE virtual_storage_tree AS (
                      SELECT
                        virtual_storage_id,
                        virtual_storage_parent_id,
                        virtual_storage_name,
                        virtual_storage_type,
                        virtual_storage_external_id,
                        virtual_storage_path,
                        virtual_storage_total_file_size,
                        virtual_storage_total_file_count,
                        virtual_storage_total_space,
                        virtual_storage_free_space,
                        virtual_storage_active
                      FROM virtual_storage
                      WHERE virtual_storage_id = 1

                      UNION ALL

                      SELECT
                        child.virtual_storage_id,
                        child.virtual_storage_parent_id,
                        child.virtual_storage_name,
                        child.virtual_storage_type,
                        child.virtual_storage_external_id,
                        child.virtual_storage_path,
                        child.virtual_storage_total_file_size,
                        child.virtual_storage_total_file_count,
                        child.virtual_storage_total_space,
                        child.virtual_storage_free_space,
                        child.virtual_storage_active
                      FROM virtual_storage_tree parent
                      JOIN virtual_storage child ON child.virtual_storage_parent_id = parent.virtual_storage_id
                    )
                    SELECT
                        virtual_storage_id,
                        virtual_storage_parent_id,
                        virtual_storage_name,
                        virtual_storage_type,
                        virtual_storage_external_id,
                        virtual_storage_path,
                        virtual_storage_total_file_size,
                        virtual_storage_total_file_count,
                        virtual_storage_total_space,
                        virtual_storage_free_space,
                        virtual_storage_active
                    FROM virtual_storage_tree
                )");
    }
    else if (ui->Virtual_checkBox_DisplayAllExceptPhysicalGroup->isChecked() == true) {
        querySQL = QLatin1String(R"(
                    WITH RECURSIVE virtual_storage_tree AS (
                      SELECT
                        virtual_storage_id,
                        virtual_storage_parent_id,
                        virtual_storage_name,
                        virtual_storage_type,
                        virtual_storage_external_id,
                        virtual_storage_path,
                        virtual_storage_total_file_size,
                        virtual_storage_total_file_count,
                        virtual_storage_total_space,
                        virtual_storage_free_space,
                        virtual_storage_active
                      FROM virtual_storage
                      WHERE virtual_storage_id <> 1

                      UNION ALL

                      SELECT
                        child.virtual_storage_id,
                        child.virtual_storage_parent_id,
                        child.virtual_storage_name,
                        child.virtual_storage_type,
                        child.virtual_storage_external_id,
                        child.virtual_storage_path,
                        child.virtual_storage_total_file_size,
                        child.virtual_storage_total_file_count,
                        child.virtual_storage_total_space,
                        child.virtual_storage_free_space,
                        child.virtual_storage_active
                      FROM virtual_storage_tree parent
                      JOIN virtual_storage child ON child.virtual_storage_parent_id = parent.virtual_storage_id
                      WHERE parent.virtual_storage_id <> 1
                    )
                    SELECT DISTINCT -- Add DISTINCT to remove duplicates
                        virtual_storage_id,
                        virtual_storage_parent_id,
                        virtual_storage_name,
                        virtual_storage_type,
                        virtual_storage_external_id,
                        virtual_storage_path,
                        virtual_storage_total_file_size,
                        virtual_storage_total_file_count,
                        virtual_storage_total_space,
                        virtual_storage_free_space,
                        virtual_storage_active
                    FROM virtual_storage_tree
                )");
    }

    if (ui->Virtual_checkBox_DisplayCatalogs->isChecked() == false) {
        querySQL += QLatin1String(R"(
                    WHERE virtual_storage_type !='Catalog'
                )");
    }

    querySQL +=" ORDER BY virtual_storage_id ASC ";
    query.prepare(querySQL);
    query.exec();

    //Prepare the tree model: headers
    QStandardItemModel *model = new QStandardItemModel();
    model->setHorizontalHeaderLabels({tr("Name"),
                                      tr("Device Type"),
                                      tr("Active"),
                                      tr("ID"),
                                      tr("Parent ID"),
                                      tr("External ID"),
                                      tr("Number of files"),
                                      tr("Total Size"),
                                      tr("Total space"),
                                      tr("Free space"),
                                      "" });

    //Create a map to store items by ID for easy access
    QMap<int, QStandardItem*> itemMap;

    //Populate model
    while (query.next()) {

        //Get data forthe item
        int id = query.value(0).toInt();
        int parentId = query.value(1).toInt();
        QString name = query.value(2).toString();
        QString type = query.value(3).toString();
        int externalId = query.value(4).toInt();
        qint64 size = query.value(6).toLongLong();
        qint64 number = query.value(7).toLongLong();
        qint64 total_space = query.value(8).toLongLong();
        qint64 free_space = query.value(9).toLongLong();
        bool isActive = query.value(10).toBool();

        //Create the item for this row
        QList<QStandardItem*> rowItems;
        rowItems << new QStandardItem(name);
        rowItems << new QStandardItem(type);
        rowItems << new QStandardItem(QString::number(isActive));
        rowItems << new QStandardItem(QString::number(id));
        rowItems << new QStandardItem(QString::number(parentId));
        rowItems << new QStandardItem(QString::number(externalId));
        rowItems << new QStandardItem(QString::number(number));
        rowItems << new QStandardItem(QString::number(size));
        rowItems << new QStandardItem(QString::number(total_space));
        rowItems << new QStandardItem(QString::number(free_space));

        //Get the item representing the name, and map the parent ID
        QStandardItem* item = rowItems.at(0);
        QStandardItem* parentItem = itemMap.value(parentId);

        //Add top-level items directly to the model
        if (parentId == 0) {
            model->appendRow(rowItems);
        }
        //else append the row to the parent item
        else{
            if (parentItem) {
                parentItem->appendRow(rowItems);
            }
            else{
                // Skip this row and proceed to the next one
                qDebug() << "Parent item not found for ID:" << id;
                continue;
            }
        }

        // Store the item in the map for future reference
        itemMap.insert(id, item);
    }

    //Load Model to treeview (Virtual tab)
        DeviceTreeView *deviceTreeViewForVirtualTab = new DeviceTreeView(this);
        deviceTreeViewForVirtualTab->setSourceModel(model);
        ui->Virtual_treeView_VirtualStorageList->setModel(deviceTreeViewForVirtualTab);

        //Customize tree display
        ui->Virtual_treeView_VirtualStorageList->QTreeView::sortByColumn(0,Qt::AscendingOrder);
        ui->Virtual_treeView_VirtualStorageList->header()->setSectionResizeMode(QHeaderView::Interactive);
        ui->Virtual_treeView_VirtualStorageList->header()->resizeSection(0, 350); //Name
        ui->Virtual_treeView_VirtualStorageList->header()->resizeSection(1, 100); //Type
        ui->Virtual_treeView_VirtualStorageList->header()->resizeSection(2,  25); //Active
        ui->Virtual_treeView_VirtualStorageList->header()->resizeSection(3,  25); //ID
        ui->Virtual_treeView_VirtualStorageList->header()->resizeSection(4,  25); //Parent ID
        ui->Virtual_treeView_VirtualStorageList->header()->resizeSection(5,  25); //External ID
        ui->Virtual_treeView_VirtualStorageList->header()->resizeSection(6, 100); //Number of Files
        ui->Virtual_treeView_VirtualStorageList->header()->resizeSection(7, 100); //Total File Size

        if (ui->Virtual_checkBox_DisplayFullTable->isChecked()) {
            ui->Virtual_treeView_VirtualStorageList->header()->showSection(1); //Type
            ui->Virtual_treeView_VirtualStorageList->header()->showSection(2); //Active
            ui->Virtual_treeView_VirtualStorageList->header()->showSection(3); //ID
            ui->Virtual_treeView_VirtualStorageList->header()->showSection(4); //Parent ID
            ui->Virtual_treeView_VirtualStorageList->header()->showSection(5); //External ID
        } else {
            ui->Virtual_treeView_VirtualStorageList->header()->hideSection(1); //Type
            ui->Virtual_treeView_VirtualStorageList->header()->hideSection(2); //Active
            ui->Virtual_treeView_VirtualStorageList->header()->hideSection(3); //ID
            ui->Virtual_treeView_VirtualStorageList->header()->hideSection(4); //Parent ID
            ui->Virtual_treeView_VirtualStorageList->header()->hideSection(5); //External ID
        }

        ui->Virtual_treeView_VirtualStorageList->expandAll();

    //Load Model to treeview (Filters/Device tree)
        if(selectedTreeType==tr("Virtual Storage / Catalog")){
            DeviceTreeView *deviceTreeViewForSelectionPanel = new DeviceTreeView(this);

            ui->Filters_treeView_Devices->setModel(deviceTreeViewForSelectionPanel);
            deviceTreeViewForSelectionPanel->setSourceModel(model);
            ui->Filters_treeView_Devices->header()->setSectionResizeMode(QHeaderView::ResizeToContents);
            ui->Filters_treeView_Devices->sortByColumn(0,Qt::AscendingOrder);
            ui->Filters_treeView_Devices->hideColumn(1);
            ui->Filters_treeView_Devices->hideColumn(2);
            ui->Filters_treeView_Devices->hideColumn(3);
            ui->Filters_treeView_Devices->setColumnWidth(2,0);
            ui->Filters_treeView_Devices->collapseAll();
            ui->Filters_treeView_Devices->header()->hide();

            for (int var = 1; var < deviceTreeViewForSelectionPanel->columnCount(); ++var) {
                ui->Filters_treeView_Devices->header()->hideSection(var);
            }

            //Restore Expand or Collapse Device Tree
            setTreeExpandState(false);
            deviceTreeViewForSelectionPanel->boldColumnList.clear();
            ui->Filters_treeView_Devices->setModel(deviceTreeViewForSelectionPanel);
            ui->Filters_treeView_Devices->expandAll();
        }
}
//--------------------------------------------------------------------------
void MainWindow::saveVirtualStorageTableToFile(QString filePath)
{
    if (databaseMode == "Memory"){
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
}
//--------------------------------------------------------------------------
void MainWindow::updateNumbers(int virtualStorageID, QString storageType) {
    QSqlQuery query;
    QString querySQL;

    if(storageType=="Storage"  or storageType=="VirtualStorage"){
        //virtual_storage_total_file_count
        querySQL = QLatin1String(R"(
                            UPDATE virtual_storage
                            SET virtual_storage_total_file_count =
                                (SELECT SUM(virtual_storage_total_file_count)
                                FROM virtual_storage
                                WHERE virtual_storage_parent_id = :virtual_storage_id)
                            WHERE virtual_storage_id = :virtual_storage_id
                        )");
        query.prepare(querySQL);
        query.bindValue(":virtual_storage_id",virtualStorageID);
        query.exec();

        //virtual_storage_total_file_size
        querySQL = QLatin1String(R"(
                            UPDATE virtual_storage
                            SET virtual_storage_total_file_size =
                                (SELECT SUM(virtual_storage_total_file_size)
                                FROM virtual_storage
                                WHERE virtual_storage_parent_id = :virtual_storage_id)
                            WHERE virtual_storage_id = :virtual_storage_id
                        )");
        query.prepare(querySQL);
        query.bindValue(":virtual_storage_id",virtualStorageID);
        query.exec();
    }

    if(storageType !="Storage" and storageType=="VirtualStorage"){
        //virtual_storage_total_space
        querySQL = QLatin1String(R"(
                            UPDATE virtual_storage
                            SET virtual_storage_total_space =
                                (SELECT SUM(virtual_storage_total_space)
                                FROM virtual_storage
                                WHERE virtual_storage_parent_id = :virtual_storage_id)
                            WHERE virtual_storage_id = :virtual_storage_id
                        )");
        query.prepare(querySQL);
        query.bindValue(":virtual_storage_id",virtualStorageID);
        query.exec();

        //virtual_storage_free_space
        querySQL = QLatin1String(R"(
                            UPDATE virtual_storage
                            SET virtual_storage_free_space =
                                (SELECT SUM(virtual_storage_free_space)
                                FROM virtual_storage
                                WHERE virtual_storage_parent_id = :virtual_storage_id)
                            WHERE virtual_storage_id = :virtual_storage_id
                        )");
        query.prepare(querySQL);
        query.bindValue(":virtual_storage_id",virtualStorageID);
        query.exec();
    }
    saveVirtualStorageTableToFile(virtualStorageFilePath);
    loadVirtualStorageTableToTreeModel();
}
//--------------------------------------------------------------------------
void MainWindow::synchCatalogAndStorageValues() {
    QSqlQuery query;
    QString querySQL;

    //Synch Catalog numbers
//    if(selectedVirtualStorageType=="Catalog"){
        querySQL = QLatin1String(R"(
                        UPDATE virtual_storage
                        SET virtual_storage_total_file_count = catalog.catalog_file_count,
                            virtual_storage_total_file_size  = catalog.catalog_total_file_size,
                            virtual_storage_active = catalog.catalog_source_path_is_active
                        FROM catalog
                        WHERE virtual_storage.virtual_storage_external_id = catalog.catalog_name;
                    )");
        query.prepare(querySQL);
        query.exec();
//    }
//    //Synch Storage numbers
//    else if(selectedVirtualStorageType=="Storage"){
        querySQL = QLatin1String(R"(
                        UPDATE virtual_storage
                        SET virtual_storage_name = storage.storage_name,
                            virtual_storage_total_space = storage.storage_total_space,
                            virtual_storage_free_space = storage.storage_free_space
                        FROM storage
                        WHERE virtual_storage.virtual_storage_external_id = storage.storage_id;
                    )");
        query.prepare(querySQL);
        query.exec();
//    }
}
//--------------------------------------------------------------------------
void MainWindow::updateAllNumbers() {

    synchCatalogAndStorageValues();

    QSqlQuery query;
    QString querySQL;
    //Get List of parent items
    querySQL = QLatin1String(R"(
                        WITH RECURSIVE virtual_storage_tree AS (
                          SELECT
                            virtual_storage_id,
                            virtual_storage_parent_id
                          FROM virtual_storage
                          WHERE virtual_storage_id = :virtual_storage_id

                          UNION ALL

                          SELECT
                            parent.virtual_storage_id,
                            parent.virtual_storage_parent_id
                          FROM virtual_storage_tree child
                          JOIN virtual_storage parent
                            ON parent.virtual_storage_id = child.virtual_storage_parent_id
                        )
                        SELECT virtual_storage_id
                        FROM virtual_storage_tree
                    )");
    query.prepare(querySQL);
    query.bindValue(":virtual_storage_id",selectedVirtualStorageID);
    query.exec();

    //Update parents
    while (query.next()) {
        int tempID = query.value(0).toInt();

        VirtualStorage *tempCurrentVirtualStorage = new VirtualStorage;
        tempCurrentVirtualStorage->ID = tempID;
        tempCurrentVirtualStorage->loadVirtualStorage();

        updateNumbers(tempCurrentVirtualStorage->ID, tempCurrentVirtualStorage->type);
    }

    saveVirtualStorageTableToFile(virtualStorageFilePath);
    loadVirtualStorageTableToTreeModel();
}
//--------------------------------------------------------------------------
void MainWindow::insertPhysicalStorageGroup() {
    QSqlQuery query;
    QString querySQL;

    querySQL = QLatin1String(R"(
                            SELECT COUNT(*)
                            FROM virtual_storage
                            WHERE virtual_storage_id = 1
                        )");
    query.prepare(querySQL);
    query.exec();
    query.next();
    int result = query.value(0).toInt();

    if(result == 0){
        insertVirtualStorageItem(1, 0, tr(" Physical Group"), "VirtualStorage", 0);
        insertVirtualStorageItem(2, 1, tr("Default location"), "VirtualStorage", 0);
    }

    saveVirtualStorageTableToFile(virtualStorageFilePath);
    loadVirtualStorageTableToTreeModel();
}
//--------------------------------------------------------------------------
void MainWindow::setVirtualStorageTreeExpandState(bool toggle)
{
    //optionDeviceTreeExpandState values:  collapseAll or 2 =collapse / 0=exp.level0 / 1=exp.level1
    QString iconName = ui->Filters_pushButton_TreeExpandCollapse->icon().name();
    QSettings settings(settingsFilePath, QSettings:: IniFormat);

    if (toggle==true){

        if ( optionDeviceTreeExpandState == 2 ){
            //collapsed > expand first level
            ui->Virtual_pushButton_TreeExpandCollapse->setIcon(QIcon::fromTheme("expand-all"));
            optionDeviceTreeExpandState = 0;
            ui->Virtual_treeView_VirtualStorageList->expandToDepth(0);
            settings.setValue("Virtual/optionDeviceTreeExpandState", optionDeviceTreeExpandState);
        }
        else if ( optionDeviceTreeExpandState == 0 ){
            //expanded first level > expand to second level
            ui->Virtual_pushButton_TreeExpandCollapse->setIcon(QIcon::fromTheme("collapse-all"));
            optionDeviceTreeExpandState = 1;
            ui->Virtual_treeView_VirtualStorageList->expandToDepth(1);
            settings.setValue("Virtual/optionDeviceTreeExpandState", optionDeviceTreeExpandState);
        }
        else if ( optionDeviceTreeExpandState == 1 ){
            //expanded second level > collapse
            ui->Virtual_pushButton_TreeExpandCollapse->setIcon(QIcon::fromTheme("expand-all"));
            optionDeviceTreeExpandState = 2;
            ui->Virtual_treeView_VirtualStorageList->collapseAll();
            settings.setValue("Virtual/optionDeviceTreeExpandState", optionDeviceTreeExpandState);
        }
    }
    else
    {
        if ( optionDeviceTreeExpandState == 0 ){
            ui->Filters_pushButton_TreeExpandCollapse->setIcon(QIcon::fromTheme("expand-all"));
            ui->Filters_treeView_Devices->collapseAll();
            ui->Filters_treeView_Devices->expandToDepth(optionDeviceTreeExpandState);
        }
        else if ( optionDeviceTreeExpandState == 1 ){
            ui->Filters_pushButton_TreeExpandCollapse->setIcon(QIcon::fromTheme("collapse-all"));
            ui->Filters_treeView_Devices->collapseAll();
            ui->Filters_treeView_Devices->expandToDepth(optionDeviceTreeExpandState);
        }
        else{
            ui->Filters_pushButton_TreeExpandCollapse->setIcon(QIcon::fromTheme("expand-all"));
            ui->Filters_treeView_Devices->collapseAll();
        }
    }
}
//--------------------------------------------------------------------------
void MainWindow::shiftIDsInVirtualStorageTable(int shiftAmount)
{
    QSqlQuery query;

    // First, update the rows with parentID = 0 to keep them unchanged
    QString sql = "UPDATE virtual_storage SET virtual_storage_id = virtual_storage_id + :shiftAmount "
                  "WHERE virtual_storage_parent_id = 0";
    query.prepare(sql);
    query.bindValue(":shiftAmount", shiftAmount);
    if (!query.exec()) {
        qDebug() << "Error updating virtual_storage table:" << query.lastError().text();
        return;
    }

    // Next, update the rows with parentID != 0 to shift their IDs
    sql = "UPDATE virtual_storage SET virtual_storage_id = virtual_storage_id + :shiftAmount, "
          "virtual_storage_parent_id = virtual_storage_parent_id + :shiftAmount "
          "WHERE virtual_storage_parent_id != 0";
    query.prepare(sql);
    query.bindValue(":shiftAmount", shiftAmount);
    if (!query.exec()) {
        qDebug() << "Error updating virtual_storage table:" << query.lastError().text();
        return;
    }

    qDebug() << "IDs shifted successfully by" << shiftAmount;
}
//--------------------------------------------------------------------------
