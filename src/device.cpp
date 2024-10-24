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
// File Name:   device.cpp
// Purpose:     class to manage devices
// Description:
// Author:      Stephane Couturier
/////////////////////////////////////////////////////////////////////////////
*/

#include "device.h"
#include <QApplication>
#include <QSqlError>

void Device::loadDevice(QString connectionName){
    QSqlDatabase db = QSqlDatabase::database(connectionName);
    QSqlQuery query(db);
    QString querySQL = QLatin1String(R"(
                            SELECT  device_id,
                                    device_parent_id,
                                    device_name,
                                    device_type,
                                    device_external_id,
                                    device_path,
                                    device_total_file_size,
                                    device_total_file_count,
                                    device_total_space,
                                    device_free_space,
                                    device_group_id,
                                    device_order
                            FROM  device
                            WHERE device_id =:device_id
                        )");

    query.prepare(querySQL);
    query.bindValue(":device_id", ID);

    if (query.exec()) {
        if (query.next()) {
            parentID    = query.value(1).toInt();
            name        = query.value(2).toString();
            type        = query.value(3).toString();
            externalID  = query.value(4).toInt();
            path        = query.value(5).toString();
            totalFileSize  = query.value(6).toLongLong();
            totalFileCount = query.value(7).toLongLong();
            totalSpace  = query.value(8).toLongLong();
            freeSpace   = query.value(9).toLongLong();
            groupID     = query.value(10).toInt();
            order       = query.value(11).toInt();
        } else if (ID !=0){
            qDebug() << "DEBUG: loadDevice query failed, no record found for device_id" << ID;
        }
    } else {
        qDebug() << "DEBUG: loadDevice query execution failed:" << query.lastError().text();
        return;
    }

    //Load storage values
    if(type == "Storage"){
        storage->ID = externalID;
        storage->loadStorage(connectionName);
        storage->path = path;
        storage->totalSpace = totalSpace;
        storage->freeSpace  = freeSpace;
    }

    //Load catalog values
    if(type == "Catalog"){
        catalog->ID = externalID;
        catalog->loadCatalog(connectionName);
        catalog->name = name;
        catalog->sourcePath = path;
        catalog->fileCount = totalFileCount;
        catalog->totalFileSize = totalFileSize;
    }

    //Load sub-device list
    loadSubDeviceList(connectionName);
    loadSubDeviceTree(connectionName);

    //Update states
    verifyHasSubDevice(connectionName);
    updateActive(connectionName);
}

void Device::loadSubDeviceTree(QString connectionName) {
    subDevices.clear();

    QSqlDatabase db = QSqlDatabase::database(connectionName);
    if (!db.isOpen()) {
        qDebug() << "DEBUG: Database is not open.";
        return;
    }

    QSqlQuery query(db);
    QString querySQL = QLatin1String(R"(
                                    SELECT
                                        device_id, device_type
                                    FROM device
                                    WHERE device_id IN (
                                        WITH RECURSIVE hierarchy AS (
                                            SELECT device_id
                                            FROM device
                                            WHERE device_id = :device_id
                                            UNION ALL
                                            SELECT t.device_id
                                            FROM device t
                                            JOIN hierarchy h ON t.device_parent_id = h.device_id
                                        )
                                        SELECT device_id
                                        FROM hierarchy )
                                    AND device_id != :device_id
                        )");
    query.prepare(querySQL);
    query.bindValue(":device_id",        ID);

    if (query.exec()) {
        while (query.next()) {
            Device subDevice;
            subDevice.ID = query.value(0).toInt();
            subDevice.type = query.value(1).toString();
            subDevice.loadDevice(connectionName);
            subDevices.append(subDevice);
        }
    } else {
        qDebug() << "DEBUG: Failed to execute device/loadSubDeviceTree:" << query.lastError().text();
    }
}

void Device::loadSubDeviceList(QString connectionName)
{
    //Get catalog data based on filters
    //Generate SQL query from filters
    QSqlQuery queryLoadSubDeviceList(QSqlDatabase::database(connectionName));
    QString queryLoadSubDeviceListSQL;

    //Prepare Query
    if(type !="All"){
        queryLoadSubDeviceListSQL  = QLatin1String(R"(
                                    SELECT
                                        device_id, device_type
                                    FROM device
                                    WHERE device_id IN (
                                    WITH RECURSIVE hierarchy AS (
                                        SELECT device_id
                                        FROM device
                                        WHERE device_id = :device_id
                                        UNION ALL
                                        SELECT t.device_id
                                        FROM device t
                                        JOIN hierarchy h ON t.device_parent_id = h.device_id
                                    )
                                    SELECT device_id
                                    FROM hierarchy )
                                    AND device_id != :device_id
                            )");
    }
    else{
        queryLoadSubDeviceListSQL  = QLatin1String(R"(
                                    SELECT
                                        device_id, device_type
                                    FROM device
                            )");
    }

    //Execute query
    queryLoadSubDeviceList.prepare(queryLoadSubDeviceListSQL);
    queryLoadSubDeviceList.bindValue(":device_id",        ID);
    queryLoadSubDeviceList.bindValue(":device_parent_id", ID);
    queryLoadSubDeviceList.exec();

    deviceIDList.clear();
    deviceListTable.clear();
    while (queryLoadSubDeviceList.next()) {
        deviceIDList<<queryLoadSubDeviceList.value(0).toInt();
        deviceListTable.append({queryLoadSubDeviceList.value(0).toInt(),queryLoadSubDeviceList.value(1).toString()});
    }
}

void Device::getCatalogStorageID(){
    //Retrieve device_parent_id for an item in the physical group
    QSqlQuery queryGetCatalogStorageID(QSqlDatabase::database("defaultConnection"));
    QString queryGetCatalogStorageIDSQL = QLatin1String(R"(
                    WITH RECURSIVE find_special AS
                        (SELECT device_id, device_parent_id, device_name
                        FROM device WHERE device_id = 1

                        UNION ALL

                        SELECT vs.device_id, vs.device_parent_id, vs.device_name
                        FROM device AS vs
                        INNER JOIN find_special AS fs ON vs.device_parent_id = fs.device_id)
                    SELECT device_id
                    FROM find_special
                    WHERE device_name = :name
                )");
    queryGetCatalogStorageID.prepare(queryGetCatalogStorageIDSQL);
    queryGetCatalogStorageID.bindValue(":name", name);

    if (queryGetCatalogStorageID.exec()) {
        if (queryGetCatalogStorageID.next()) {
            ID = queryGetCatalogStorageID.value(0).toInt();
        } else {
            qDebug() << "getCatalogStorageID failed, no record found for device_name" << name;
        }
    } else {
        qDebug() << "DEBUG: getCatalogStorageID query execution failed:" << queryGetCatalogStorageID.lastError().text();
    }
}

void Device::generateDeviceID()
{//Generate new ID
    if(ID==0){
        QSqlQuery queryGenerateDeviceID(QSqlDatabase::database("defaultConnection"));
        QString queryGenerateDeviceIDSQL;
        queryGenerateDeviceIDSQL = QLatin1String(R"(
                            SELECT MAX(device_id)
                            FROM device
                        )");
        queryGenerateDeviceID.prepare(queryGenerateDeviceIDSQL);
        queryGenerateDeviceID.exec();
        queryGenerateDeviceID.next();
        ID = queryGenerateDeviceID.value(0).toInt() + 1;
    }
}

void Device::insertDevice()
{//Insert device in table

    QSqlQuery queryInsertDevice(QSqlDatabase::database("defaultConnection"));
    QString queryInsertDeviceSQL;
    queryInsertDeviceSQL = QLatin1String(R"(
                            INSERT INTO device(
                                        device_id,
                                        device_parent_id,
                                        device_name,
                                        device_type,
                                        device_path,
                                        device_external_id,
                                        device_total_file_size,
                                        device_total_file_count,
                                        device_total_space,
                                        device_free_space,
                                        device_group_id,
                                        device_order)
                            VALUES(
                                        :device_id,
                                        :device_parent_id,
                                        :device_name,
                                        :device_type,
                                        :device_path,
                                        :device_external_id,
                                        :device_total_file_size,
                                        :device_total_file_count,
                                        :device_total_space,
                                        :device_free_space,
                                        :device_group_id,
                                        :device_order)
                                )");
    queryInsertDevice.prepare(queryInsertDeviceSQL);
    queryInsertDevice.bindValue(":device_id", ID);
    queryInsertDevice.bindValue(":device_parent_id", parentID);
    queryInsertDevice.bindValue(":device_name",name);
    queryInsertDevice.bindValue(":device_type", type);
    queryInsertDevice.bindValue(":device_path", path);
    queryInsertDevice.bindValue(":device_external_id", externalID);
    queryInsertDevice.bindValue(":device_total_file_size", totalFileSize);
    queryInsertDevice.bindValue(":device_total_file_count", totalFileCount);
    queryInsertDevice.bindValue(":device_total_space", totalSpace);
    queryInsertDevice.bindValue(":device_free_space", freeSpace);
    queryInsertDevice.bindValue(":device_group_id", groupID);
    queryInsertDevice.bindValue(":device_order", order);
    queryInsertDevice.exec();
}

bool Device::verifyDeviceNameExists()
{
    QSqlQuery query(QSqlDatabase::database("defaultConnection"));
    QString querySQL = QLatin1String(R"(
                                    SELECT COUNT(*)
                                    FROM   device
                                    WHERE  device_name = :device_name
                                )");

    query.prepare(querySQL);
    query.bindValue(":device_name", name);

    if (!query.exec() and ID !=0) {
        qDebug() << "DEBUG: Error executing verifyDeviceNameExists:" << query.lastError().text();
        return false;
    }

    query.next();
    return query.value(0).toInt() > 0;
}

bool Device::verifyParentDeviceExistsInPhysicalGroup()
{
    QSqlQuery query(QSqlDatabase::database("defaultConnection"));
    QString querySQL = QLatin1String(R"(
                                    SELECT COUNT(*)
                                    FROM   device
                                    WHERE  device_id = :device_id
                                    AND group_id = 0
                                )");

    query.prepare(querySQL);
    query.bindValue(":device_id", parentID);

    if (!query.exec()) {
        qDebug() << "DEBUG: Error executing verifyDeviceNameExists:" << query.lastError().text();
        return false;
    }

    query.next();
    return query.value(0).toInt() > 0;
}

void Device::verifyHasSubDevice(QString connectionName)
{
    QSqlQuery queryVerifyChildren(QSqlDatabase::database(connectionName));
    QString queryVerifyChildrenSQL = QLatin1String(R"(
                                SELECT COUNT(*)
                                FROM device
                                WHERE device_parent_id=:device_parent_id
                            )");
    queryVerifyChildren.prepare(queryVerifyChildrenSQL);
    queryVerifyChildren.bindValue(":device_parent_id", ID);
    queryVerifyChildren.exec();
    queryVerifyChildren.next();

    if(queryVerifyChildren.value(0).toInt()>0){
        hasSubDevice = true;
    }
    else
        hasSubDevice = false;
}

bool Device::verifyStorageExternalIDExists()
{
    QSqlQuery queryExternalID(QSqlDatabase::database("defaultConnection"));
    QString queryExternalIDSQL = QLatin1String(R"(
                                SELECT COUNT(device_external_id)
                                FROM device
                                WHERE device_external_id=:device_external_id
                                AND device_type ='Storage'
                            )");
    queryExternalID.prepare(queryExternalIDSQL);
    queryExternalID.bindValue(":device_external_id", externalID);
    queryExternalID.exec();
    queryExternalID.next();
    return queryExternalID.value(0).toInt() > 0;
}

void Device::getIDFromDeviceName()
{
    QSqlQuery queryIDFromDeviceName(QSqlDatabase::database("defaultConnection"));
    QString queryIDFromDeviceNameSQL = QLatin1String(R"(
                                SELECT device_id
                                FROM device
                                WHERE device_name=:device_name
                            )");
    queryIDFromDeviceName.prepare(queryIDFromDeviceNameSQL);
    queryIDFromDeviceName.bindValue(":device_name", name);
    queryIDFromDeviceName.exec();
    queryIDFromDeviceName.next();

    ID = queryIDFromDeviceName.value(0).toInt();
}

void Device::deleteDevice(bool askConfirmation)
{
    verifyHasSubDevice("defaultConnection");

    if ( hasSubDevice == false ){

            QMessageBox msgBox;
            msgBox.setWindowTitle("Katalog");
            QString impactMessage;
            int result;

            if (askConfirmation==true){

                if(type=="Storage"){
                    impactMessage = QCoreApplication::translate("MainWindow","This will remove the device and the storage details.");
                }
                msgBox.setText(QCoreApplication::translate("MainWindow", "Do you want to <span style='color: red';>delete</span> this %1 device?"
                                                                         "<table>"
                                                                         "<tr><td>ID:   </td><td><b> %2 </td></tr>"
                                                                         "<tr><td>Name: </td><td><b> %3 </td></tr>"
                                                                         "<tr><td></td></tr>"
                                                                         "<tr><td></td><td>%4</td></tr>"
                                                                         "</table>").arg(type, QString::number(ID), name,impactMessage));
                msgBox.setIcon(QMessageBox::Warning);
                msgBox.setStandardButtons(QMessageBox::Yes|QMessageBox::Cancel);
                result = msgBox.exec();
            }
            else
                result = QMessageBox::Yes;

            if ( result == QMessageBox::Yes){

                //Delete selected ID
                QSqlQuery query(QSqlDatabase::database("defaultConnection"));
                QString querySQL = QLatin1String(R"(
                                    DELETE FROM device
                                    WHERE device_id=:device_id
                                )");
                query.prepare(querySQL);
                query.bindValue(":device_id", ID);
                query.exec();

                if(type == "Storage"){
                    storage->ID = externalID;
                    storage->deleteStorage();
                }
            }

        //Delete storage values
        if(type == "Storage"){
            storage->deleteStorage();
        }

        //Delete catalog values
        if(type == "Catalog"){
            catalog->deleteCatalog();
        }
    }
    else{
        QMessageBox msgBox;
        msgBox.setWindowTitle("Katalog");
        msgBox.setText(QCoreApplication::translate("MainWindow", "The selected device cannot be deleted as long as it has sub-devices."));
        msgBox.setIcon(QMessageBox::Warning);
        msgBox.exec();
    }
}

void Device::saveDevice()
{//Update database with device values
    QSqlQuery query(QSqlDatabase::database("defaultConnection"));
    QString querySQL = QLatin1String(R"(
                            UPDATE  device
                            SET     device_name =:device_name,
                                    device_parent_id =:device_parent_id,
                                    device_external_id =:device_external_id,
                                    device_path =:device_path,
                                    device_group_id =:device_group_id,
                                    device_total_file_size =:device_total_file_size,
                                    device_total_file_count =:device_total_file_count,
                                    device_total_space =:device_total_space,
                                    device_free_space =:device_free_space,
                                    device_group_id =:device_group_id,
                                    device_date_updated =:device_date_updated,
                                    device_order=:device_order
                            WHERE   device_id=:device_id
                        )");
    query.prepare(querySQL);
    query.bindValue(":device_id", ID);
    query.bindValue(":device_name", name);
    query.bindValue(":device_parent_id", parentID);
    query.bindValue(":device_external_id", externalID);
    query.bindValue(":device_path", path);
    query.bindValue(":device_group_id", groupID);
    query.bindValue(":device_total_file_size", totalFileSize);
    query.bindValue(":device_total_file_count", totalFileCount);
    query.bindValue(":device_total_space", totalSpace);
    query.bindValue(":device_free_space", freeSpace);
    query.bindValue(":device_group_id", groupID);
    query.bindValue(":device_date_updated", dateTimeUpdated.toString("yyyy-MM-dd hh:mm:ss"));
    query.bindValue(":device_order", order);
    query.exec();
}

QList<qint64> Device::updateDevice(QString statiticsRequestSource,
                                   QString databaseMode,
                                   bool reportStorageUpdate,
                                   QString collectionFolder,
                                   bool includeSubDevices)
{//Update device and related children storage or catalog information where relevant
    QApplication::setOverrideCursor(Qt::WaitCursor);
    //Prepare
    QList<qint64> deviceUpdatesList;
    Device parentDevice;
    parentDevice.ID = parentID;
    parentDevice.loadDevice("defaultConnection");
    updateActive("defaultConnection");
    dateTimeUpdated = QDateTime::currentDateTime();

    //Update device and children depending on type
    if (type=="Catalog"){
        //Pass device values for messages
        catalog->name = name;
        catalog->sourcePath = path;

        //Update this device/catalog (files) and its storage (space)
        deviceUpdatesList  = catalog->updateCatalogFiles(databaseMode, collectionFolder, true);
        totalFileSize  = deviceUpdatesList[1];
        totalFileCount = deviceUpdatesList[3];

        if( deviceUpdatesList.count() > 0 and deviceUpdatesList[0]==1){
            //Update catalog with new values
            totalFileCount = deviceUpdatesList[1];
            totalFileSize  = deviceUpdatesList[3];
            saveStatistics(dateTimeUpdated, statiticsRequestSource);
            deviceUpdatesList<<1;
            deviceUpdatesList<<0;
        }
        else{
            deviceUpdatesList<<0;
            deviceUpdatesList<<0;
        }

        //Update the parent Storage and add the update values to the list
        QList<qint64> storageUpdatesList = parentDevice.updateDevice("update",
                                                                     databaseMode,
                                                                     reportStorageUpdate,
                                                                     collectionFolder,
                                                                     false);
        deviceUpdatesList.append(storageUpdatesList);

        //Update related devices (other catalog devices using the same catalog ID)
        QSqlQuery queryRelatedDevice(QSqlDatabase::database("defaultConnection"));
        QString queryRelatedDeviceSQL = QLatin1String(R"(
                                    SELECT device_id
                                    FROM device
                                    WHERE device_external_id =:device_external_id
                                    AND device_type = 'Catalog'
                                    AND device_id !=:device_id
                                )");
        queryRelatedDevice.prepare(queryRelatedDeviceSQL);
        queryRelatedDevice.bindValue(":device_external_id", externalID);
        queryRelatedDevice.bindValue(":device_id",ID);
        queryRelatedDevice.exec();
        while(queryRelatedDevice.next()){
            Device relatedDevice;
            relatedDevice.ID = queryRelatedDevice.value(0).toInt();
            relatedDevice.loadDevice("defaultConnection");
            relatedDevice.totalFileCount = totalFileCount;
            relatedDevice.totalFileSize  = totalFileSize;
            relatedDevice.saveDevice();
            relatedDevice.updateParentsNumbers();
            parentDevice.saveStatistics(dateTimeUpdated, statiticsRequestSource);
        }
    }

    else if (type=="Storage"){
        //Update device/storage and all its catalogs
        //Update catalogs
        if(includeSubDevices==true){
            //Get list of catalogs
            loadSubDeviceList("defaultConnection");

            //Process the list
            qint64 globalUpdateFileCount = 0;
            qint64 globalUpdateDeltaFileCount = 0;
            qint64 globalUpdateTotalSize = 0;
            qint64 globalUpdateDeltaTotalSize = 0;
            int updatedCatalogs = 0;
            int skippedCatalogs = 0;

            //deviceListTable
            if(deviceIDList.count()>0){
                for(int deviceID = 0; deviceID<deviceIDList.count(); deviceID++) {
                    Device updatedDevice;
                    updatedDevice.ID = deviceIDList[deviceID];
                    updatedDevice.loadDevice("defaultConnection");

                    QList<qint64> catalogUpdatesList = updatedDevice.catalog->updateCatalogFiles(databaseMode, collectionFolder, false);

                    if(catalogUpdatesList[0]==1){
                        //Update catalog with new values
                        updatedDevice.totalFileCount = catalogUpdatesList[1];
                        updatedDevice.totalFileSize  = catalogUpdatesList[3];
                        updatedDevice.dateTimeUpdated = dateTimeUpdated;
                        updatedDevice.saveDevice();
                        updatedDevice.saveStatistics(dateTimeUpdated, statiticsRequestSource);

                        globalUpdateFileCount       += catalogUpdatesList[1];
                        globalUpdateDeltaFileCount  += catalogUpdatesList[2];
                        globalUpdateTotalSize       += catalogUpdatesList[3];
                        globalUpdateDeltaTotalSize  += catalogUpdatesList[4];
                        updatedCatalogs +=1;
                    }
                    else
                        skippedCatalogs +=1;
                }

                deviceUpdatesList << 1;
                deviceUpdatesList << globalUpdateFileCount;
                deviceUpdatesList << globalUpdateDeltaFileCount;
                deviceUpdatesList << globalUpdateTotalSize;
                deviceUpdatesList << globalUpdateDeltaTotalSize;
                deviceUpdatesList << updatedCatalogs;
                deviceUpdatesList << skippedCatalogs;
            }
            else {
                //Add 0 for catalog
                deviceUpdatesList<<0;
                deviceUpdatesList<<0;
                deviceUpdatesList<<0;
                deviceUpdatesList<<0;
                deviceUpdatesList<<0;
                deviceUpdatesList<<0;
                deviceUpdatesList<<0;
            }
        }

        //Update storage itself
        QList<qint64> storageUpdates = storage->updateStorageInfo(reportStorageUpdate);
        freeSpace  = storageUpdates[3];
        totalSpace = storageUpdates[5];
        saveStatistics(dateTimeUpdated, statiticsRequestSource);

        deviceUpdatesList += storageUpdates[0];
        deviceUpdatesList += storageUpdates[1];
        deviceUpdatesList += storageUpdates[2];
        deviceUpdatesList += storageUpdates[3];
        deviceUpdatesList += storageUpdates[4];
        deviceUpdatesList += storageUpdates[5];
        deviceUpdatesList += storageUpdates[6];
    }

    else if (type=="Virtual"){
        //Update all sub virtual devices, storage, catalogs, and this device
        // if(includeSubDevices==true){
            //DEV: update all children devices
        // }
        qDebug()<<"Updating a list of devices from a virtual one is not avaialable yet.";

        saveStatistics(dateTimeUpdated, statiticsRequestSource);

        //DEV: also save statistics of all parents
    }

    //Save changes
    saveDevice();

    //Update parent devices
    updateParentsNumbers();

    if( deviceUpdatesList.count() == 0)
        deviceUpdatesList<<0;

    QApplication::restoreOverrideCursor();

    return deviceUpdatesList;
}

void Device::updateNumbersFromChildren()
{
    QSqlQuery query(QSqlDatabase::database("defaultConnection"));
    QString querySQL;

    //Update file values
    if(type=="Storage"  or type=="Virtual"){
        //device_total_file_count
        querySQL = QLatin1String(R"(
                            UPDATE device
                            SET device_total_file_count =
                                (SELECT SUM(device_total_file_count)
                                FROM device
                                WHERE device_parent_id = :device_id)
                            WHERE device_id = :device_id
                        )");
        query.prepare(querySQL);
        query.bindValue(":device_id", ID);
        query.exec();

        //device_total_file_size
        querySQL = QLatin1String(R"(
                            UPDATE device
                            SET device_total_file_size =
                                (SELECT SUM(device_total_file_size)
                                FROM device
                                WHERE device_parent_id = :device_id)
                            WHERE device_id = :device_id
                        )");
        query.prepare(querySQL);
        query.bindValue(":device_id", ID);
        query.exec();
    }

    //Update space values
    if(type=="Virtual"){
        //device_total_space
        querySQL = QLatin1String(R"(
                            UPDATE device
                            SET device_total_space =
                                (SELECT SUM(device_total_space)
                                FROM device
                                WHERE device_parent_id = :device_id)
                            WHERE device_id = :device_id
                        )");
        query.prepare(querySQL);
        query.bindValue(":device_id", ID);
        query.exec();

        //device_free_space
        querySQL = QLatin1String(R"(
                            UPDATE device
                            SET device_free_space =
                                (SELECT SUM(device_free_space)
                                FROM device
                                WHERE device_parent_id = :device_id)
                            WHERE device_id = :device_id
                        )");
        query.prepare(querySQL);
        query.bindValue(":device_id", ID);
        query.exec();
    }
}

void Device::updateParentsNumbers()
{//recursively update parent numbers, from bottom to top
    QSqlQuery query(QSqlDatabase::database("defaultConnection"));
    QString querySQL;

    //Get List of parent items
    querySQL = QLatin1String(R"(
                        WITH RECURSIVE device_tree AS (
                          SELECT
                            device_id,
                            device_parent_id
                          FROM device
                          WHERE device_id = :device_id

                          UNION ALL

                          SELECT
                            parent.device_id,
                            parent.device_parent_id
                          FROM device_tree child
                          JOIN device parent
                            ON parent.device_id = child.device_parent_id
                        )
                        SELECT device_id
                        FROM device_tree
                    )");
    query.prepare(querySQL);
    query.bindValue(":device_id", ID);
    query.exec();

    //Update parents
    while (query.next()) {
        int tempID = query.value(0).toInt();

        Device tempCurrentDevice;
        tempCurrentDevice.ID = tempID;
        tempCurrentDevice.loadDevice("defaultConnection");
        tempCurrentDevice.updateNumbersFromChildren();
    }
}

void Device::updateActive(QString connectionName)
{//Update the Active value: verify that the path is active = the related drive is mounted
    if(path !=""){
        QDir dir(path);
        active = dir.exists();
    }
    else {
        active = false;
    }

    QSqlQuery queryUpdateActive(QSqlDatabase::database(connectionName));
    QString queryUpdateActiveSQL = QLatin1String(R"(
                        UPDATE device
                        SET    device_active =:device_active
                        WHERE  device_id =:device_id
                    )");
    queryUpdateActive.prepare(queryUpdateActiveSQL);
    queryUpdateActive.bindValue(":device_active", active);
    queryUpdateActive.bindValue(":device_id", ID);
    queryUpdateActive.exec();
}

void Device::saveStatistics(QDateTime dateTime, QString requestSource)
{
    QSqlQuery querySaveStatistics(QSqlDatabase::database("defaultConnection"));
    QString querySaveStatisticsSQL = QLatin1String(R"(
                                        INSERT INTO statistics_device(
                                                date_time,
                                                device_id,
                                                device_name,
                                                device_type,
                                                device_file_count,
                                                device_total_file_size,
                                                device_free_space,
                                                device_total_space,
                                                record_type)
                                        VALUES(
                                                :date_time,
                                                :device_id,
                                                :device_name,
                                                :device_type,
                                                :device_file_count,
                                                :device_total_file_size,
                                                :device_free_space,
                                                :device_total_space,
                                                :record_type)
                                    )");
    querySaveStatistics.prepare(querySaveStatisticsSQL);
    querySaveStatistics.bindValue(":date_time", dateTime.toString("yyyy-MM-dd hh:mm:ss"));
    querySaveStatistics.bindValue(":device_id", ID);
    querySaveStatistics.bindValue(":device_name", name);
    querySaveStatistics.bindValue(":device_type", type);
    querySaveStatistics.bindValue(":device_file_count", totalFileCount);
    querySaveStatistics.bindValue(":device_total_file_size", totalFileSize);
    querySaveStatistics.bindValue(":device_free_space", freeSpace);
    querySaveStatistics.bindValue(":device_total_space", totalSpace);
    querySaveStatistics.bindValue(":record_type", requestSource);

    querySaveStatistics.exec();
}
