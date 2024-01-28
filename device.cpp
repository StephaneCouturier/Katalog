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
* /////////////////////////////////////////////////////////////////////////////
// Application: Katalog
// File Name:   device.cpp
// Purpose:     class to manage devices
// Description:
// Author:      Stephane Couturier
/////////////////////////////////////////////////////////////////////////////
*/

#include "device.h"
#include "qapplication.h"
#include "qsqlerror.h"

void Device::loadDevice(){
    //Load device values
    QSqlQuery query;
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
                                    device_group_id
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
            totalSpace = query.value(8).toLongLong();
            freeSpace  = query.value(9).toLongLong();
            groupID     = query.value(10).toInt();
        } else if (ID !=0){
            qDebug() << "loadDevice failed, no record found for device_id" << ID;
        }
    } else {
        qDebug() << "Query execution failed:" << query.lastError().text();
        return;
    }

    //Load storage values
    if(type == "Storage"){
        storage->ID = externalID;
        storage->loadStorage();
        storage->path = path;
    }

    //Load catalog values
    if(type == "Catalog"){
        catalog->ID = externalID;
        catalog->name = name; //temp
        catalog->loadCatalog();
        catalog->sourcePath = path;
    }

    //Load sub-device list
    loadSubDeviceList();

    //Update states
    verifyHasSubDevice();
    updateActive();
}

void Device::loadSubDeviceList()
{
    //Get catalog data based on filters
    //Generate SQL query from filters
    QSqlQuery query;
    QString querySQL;

    //Prepare Query
    if(type !="All"){
        querySQL  = QLatin1String(R"(
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
        querySQL  = QLatin1String(R"(
                                    SELECT
                                        device_id, device_type
                                    FROM device
                            )");
    }

    //Execute query
    query.prepare(querySQL);
    query.bindValue(":device_id",        ID);
    query.bindValue(":device_parent_id", ID);
    query.exec();

    deviceIDList.clear();
    deviceListTable.clear();
    while (query.next()) {
        deviceIDList<<query.value(0).toInt();
        deviceListTable.append({query.value(0).toInt(),query.value(1).toString()});
    }
}

void Device::getCatalogStorageID(){
    //Retrieve device_parent_id for an item in the physical group
    QSqlQuery query;
    QString querySQL = QLatin1String(R"(
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
    query.prepare(querySQL);
    query.bindValue(":name", name);

    if (query.exec()) {
        if (query.next()) {
            ID = query.value(0).toInt();
        } else {
            qDebug() << "getCatalogStorageID failed, no record found for device_name" << name;
        }
    } else {
        qDebug() << "getCatalogStorageID query execution failed:" << query.lastError().text();
    }
}

void Device::generateDeviceID()
{//Generate new ID
    if(ID==0){
        QSqlQuery query;
        QString querySQL;
        querySQL = QLatin1String(R"(
                            SELECT MAX(device_id)
                            FROM device
                        )");
        query.prepare(querySQL);
        query.exec();
        query.next();
        ID = query.value(0).toInt() + 1;
    }
}

void Device::insertDevice()
{//Insert device in table

    QSqlQuery query;
    QString querySQL;
    querySQL = QLatin1String(R"(
                            INSERT INTO device(
                                        device_id,
                                        device_parent_id,
                                        device_name,
                                        device_type,
                                        device_path,
                                        device_external_id,
                                        device_group_id)
                            VALUES(
                                        :device_id,
                                        :device_parent_id,
                                        :device_name,
                                        :device_type,
                                        :device_path,
                                        :device_external_id,
                                        :device_group_id)
                                )");
    query.prepare(querySQL);
    query.bindValue(":device_id", ID);
    query.bindValue(":device_parent_id",parentID);
    query.bindValue(":device_name",name);
    query.bindValue(":device_type", type);
    query.bindValue(":device_path", path);
    query.bindValue(":device_external_id", externalID);
    query.bindValue(":device_group_id", groupID);
    query.exec();
}

void Device::verifyHasSubDevice(){

    QSqlQuery queryVerifyChildren;
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

void Device::deleteDevice(){

    verifyHasSubDevice();

    if ( hasSubDevice == false ){

            QMessageBox msgBox;
            msgBox.setWindowTitle("Katalog");
            QString impactMessage;
            if(type=="Storage"){
                impactMessage = QCoreApplication::translate("MainWindow","This will remove the device and the storage details."); //DEV: and all statistics history
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
            int result = msgBox.exec();

            if ( result ==QMessageBox::Yes){

                //Delete selected ID
                QSqlQuery query;
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
    }
    else{
        QMessageBox msgBox;
        msgBox.setWindowTitle("Katalog");
        msgBox.setText(QCoreApplication::translate("MainWindow", "The selected device cannot be deleted as long as it has sub-devices."));
        msgBox.setIcon(QMessageBox::Warning);
        msgBox.exec();
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

void Device::saveDevice()
{//Update database with device values
    QSqlQuery query;
    QString querySQL = QLatin1String(R"(
                            UPDATE device
                            SET    device_name =:device_name,
                                   device_parent_id =:device_parent_id,
                                   device_external_id =:device_external_id,
                                   device_path =:device_path,
                                   device_group_id =:device_group_id,
                                   device_total_file_size =:device_total_file_size,
                                   device_total_file_count =:device_total_file_count
                            WHERE  device_id=:device_id
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
    query.exec();
}

QList<qint64> Device::updateDevice(QString statiticsRequestSource,
                                   QString databaseMode,
                                   bool reportStorageUpdate,
                                   QString collectionFolder)
{//Update device and related children storage or catalog information where relevant
    QApplication::setOverrideCursor(Qt::WaitCursor);
    //Prepare
    QList<qint64> deviceUpdatesList;
    dateTimeUpdated = QDateTime::currentDateTime();
    updateActive();

    //Update device and children depending on type
    if (type=="Catalog"){
        //Update this device/catalog (files) and its storage (space)
        deviceUpdatesList  = catalog->updateCatalogFiles(databaseMode, collectionFolder);
        totalFileSize  = deviceUpdatesList[1];
        totalFileCount = deviceUpdatesList[3];

        //catalog->updateCatalogFile(databaseMode);
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
        Device parentDevice;
        parentDevice.ID = parentID;
        parentDevice.loadDevice();
        deviceUpdatesList << parentDevice.storage->updateStorageInfo(reportStorageUpdate);
        parentDevice.saveStatistics(dateTimeUpdated, statiticsRequestSource);
    }

    else if (type=="Storage"){
        //Update device/storage and all its catalogs
        //Get list of catalogs
        loadSubDeviceList();

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
                updatedDevice.loadDevice();

                QList<qint64> catalogUpdatesList = updatedDevice.catalog->updateCatalogFiles(databaseMode, collectionFolder);

                if(catalogUpdatesList.count()>0){
                    //Update catalog with new values
                    updatedDevice.totalFileCount = catalogUpdatesList[1];
                    updatedDevice.totalFileSize  = catalogUpdatesList[3];
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

        //Update storage itself
        QList<qint64> storageUpdates = storage->updateStorageInfo(true);
        totalSpace = storageUpdates[3];
        freeSpace = storageUpdates[5];
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
    QSqlQuery query;
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
    QSqlQuery query;
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

        Device *tempCurrentDevice = new Device;
        tempCurrentDevice->ID = tempID;
        tempCurrentDevice->loadDevice();
        tempCurrentDevice->updateNumbersFromChildren();
    }
}

void Device::updateActive()
{
    // Verify that the path is active (= the related drive is mounted)
    if(path !=""){
        QDir dir(path);
        active = dir.exists();
    }
    else {
        active = false;
    }

    QSqlQuery query;
    QString querySQL = QLatin1String(R"(
                        UPDATE device
                        SET    device_active =:device_active
                        WHERE  device_id =:device_id
                    )");
    query.prepare(querySQL);
    query.bindValue(":device_active", active);
    query.bindValue(":device_id", ID);
    query.exec();
}

void Device::saveStatistics(QDateTime dateTime, QString requestSource)
{
    QSqlQuery querySaveStatistics;
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
