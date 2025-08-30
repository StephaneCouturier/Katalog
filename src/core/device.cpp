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
#include <QSqlError>
#include <QApplication>

void Device::loadDevice(QString connectionName){
    QElapsedTimer totalTimer;
    totalTimer.start();
    QElapsedTimer stepTimer;
    stepTimer.start();
    bool useTimerForDebug = false;

    QSqlDatabase db = QSqlDatabase::database(connectionName);
    if (!db.isOpen()) {
        qDebug() << "DEBUG: Database is not open.";
        return;
    }
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

    if(useTimerForDebug) qDebug() << "      TIMER3: prepare load device query:" << stepTimer.elapsed() << "ms"; stepTimer.restart();

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

    if(useTimerForDebug) {qDebug() << "      TIMER3: load device query.exec:" << stepTimer.elapsed() << "ms"; stepTimer.restart();}

    // Get active state
    updateActiveState(connectionName);

    //Load storage values
    if(type == "Storage"){
        storage->ID = externalID;
        storage->loadStorage(connectionName);
        storage->path = path;
        storage->totalSpace = totalSpace;
        storage->freeSpace  = freeSpace;
    }
    if(useTimerForDebug){
        qDebug() << "      TIMER3: Load storage values:" << stepTimer.elapsed() << "ms";
        stepTimer.restart();
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
    if(useTimerForDebug){
        qDebug() << "      TIMER3: Load catalog values:" << stepTimer.elapsed() << "ms";
        stepTimer.restart();
    }

    //Load sub-device list
    loadSubDeviceList(connectionName);

    if(useTimerForDebug){
        qDebug() << "      TIMER3: Load sub-device list:" << stepTimer.elapsed() << "ms";
        stepTimer.restart();
    }

    //Update states
    verifyHasSubDevice(connectionName);
    if(useTimerForDebug){
        qDebug() << "      TIMER3: verifyHasSubDevice:" << stepTimer.elapsed() << "ms";
        stepTimer.restart();
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

    subDevices.clear();
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

bool Device::verifyDeviceHasSourceMapping()
{
    QSqlQuery query(QSqlDatabase::database("defaultConnection"));
    query.prepare("SELECT COUNT(*) FROM device_mapping WHERE mapping_device_source_id = :deviceId");
    query.bindValue(":deviceId", ID);

    if (!query.exec()) {
        return false;
    }

    if (query.next()) {
        int count = query.value(0).toInt();
        return count > 0;
    }

    return false;
}

bool Device::verifyDeviceHasTargetMapping()
{
    QSqlQuery query(QSqlDatabase::database("defaultConnection"));
    query.prepare("SELECT COUNT(*) FROM device_mapping WHERE mapping_device_target_id = :deviceId");
    query.bindValue(":deviceId", ID);

    if (!query.exec()) {
        return false;
    }

    if (query.next()) {
        int count = query.value(0).toInt();
        return count > 0;
    }

    return false;
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

void Device::updateActiveState(QString connectionName)
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

Device::DeleteOperationResult Device::deleteDevice(bool askConfirmation, const UpdateCallbacks* callbacks)
{
    DeleteOperationResult result;
    result.needsConfirmation = askConfirmation;
    result.result = DeleteSuccess;

    verifyHasSubDevice("defaultConnection");

    if (hasSubDevice) {
        result.result = DeleteHasSubDevices;
        result.errorMessage = "The selected device cannot be deleted as long as it has sub-devices.";
        result.needsConfirmation = false;
        return result;
    }

    if (askConfirmation) {
        // result.confirmationMessage = QString(
        //                                    "Do you want to <span style='color: red';>delete</span> this %1 device?"
        //                                     "<table><tr><td>ID:   </td><td><b> %2 </td></tr><tr><td>Name: </td><td><b> %3 </td></tr>"
        //                                     "<tr><td></td></tr></table>"
        //                                     )
        //                                  .arg(type, QString::number(ID), name);


        result.confirmationMessage = QString(
                                         QApplication::translate("MainWindow", "Do you want to <span style='color: red';>delete</span> this %1 device?").arg(type)
                                         +"<table><tr><td>" +QApplication::translate("MainWindow", "ID")+":   </td><td><b>" +QString::number(ID)+" </td></tr><tr><td>"
                                         +QApplication::translate("MainWindow", "Name") +": </td><td><b>"+ name +"</td></tr><tr><td></td></tr></table>"
                                         );




        if (callbacks && callbacks->onConfirmation) {
            if (!callbacks->onConfirmation(result.confirmationMessage)) {
                qDebug() << "Device::deleteDevice - User cancelled";
                result.result = DeleteCancelled;
                return result;
            }
        } else if (askConfirmation) {
            result.needsConfirmation = true;
            result.result = DeleteCancelled; // or create a new enum value like DeleteNeedsConfirmation
            return result;
        }
    }

    // Perform the actual deletion
    QSqlQuery query(QSqlDatabase::database("defaultConnection"));
    QString querySQL = QLatin1String(R"(
                        DELETE FROM device
                        WHERE device_id=:device_id
                    )");
    query.prepare(querySQL);
    query.bindValue(":device_id", ID);

    if (!query.exec()) {
        qDebug() << "Device::deleteDevice - Database error:" << query.lastError().text();
        result.result = DeleteError;
        result.errorMessage = "Database error: " + query.lastError().text();
        return result;
    }

    // Delete related data
    if (type == "Storage") {
        storage->ID = externalID;
        storage->deleteStorage();
    }

    if (type == "Catalog") {
        catalog->deleteCatalog();
    }

    result.result = DeleteSuccess;
    return result;
}

void Device::setActiveFromString(const QString& activeStr) {
    active = (activeStr.toLower() == "true" ||
              activeStr == "1" ||
              activeStr.toLower() == "yes");
}

QList<qint64> Device::updateStorageOnly(const QString& statisticsRequestSource)
{
    // Simple storage-only update
    // Used for early initialization in Collection::insertPhysicalStorageGroup()

    qDebug() << "updateStorageOnly for device:" << name;

    QList<qint64> deviceUpdatesList;

    // Only valid for Storage devices
    if (type != "Storage") {
        qDebug() << "ERROR: updateStorageOnly called on non-Storage device:" << type;
        // Return empty results to indicate error
        return QList<qint64>();
    }

    // Update device state (same as original logic)
    updateActiveState("defaultConnection");
    dateTimeUpdated = QDateTime::currentDateTime();

    // === STORAGE UPDATE ONLY (extracted from Device::updateDevice) ===

    // Update storage path from device for updateStorageInfo()
    storage->path = path;

    // Add zeros for catalog results (no catalog processing)
    deviceUpdatesList << 0;  // Catalog updated flag
    deviceUpdatesList << 0;  // Total file count
    deviceUpdatesList << 0;  // Delta file count
    deviceUpdatesList << 0;  // Total size
    deviceUpdatesList << 0;  // Delta size
    deviceUpdatesList << 0;  // Updated catalogs count
    deviceUpdatesList << 0;  // Skipped catalogs count

    // Update storage itself (same as original logic)
    Storage::UpdateResult storageResult = storage->updateStorageInfo();

    // Convert to old format for compatibility with existing code
    QList<qint64> storageUpdates;
    if (storageResult.wasUpdated) {
        storageUpdates << 1 << storageResult.newUsedSpace << storageResult.deltaUsedSpace
                       << storageResult.newFreeSpace << storageResult.deltaFreeSpace
                       << storageResult.newTotalSpace << storageResult.deltaTotalSpace;

        // Update device properties (same as original logic)
        freeSpace = storageResult.newFreeSpace;
        totalSpace = storageResult.newTotalSpace;

        // Save device and statistics (same as original logic)
        saveDevice();
        saveStatistics(dateTimeUpdated, statisticsRequestSource);

    } else {
        storageUpdates << 0 << 0 << 0 << 0 << 0 << 0 << 0;
    }

    // Append storage results to device results list (same format as original)
    deviceUpdatesList += storageUpdates[0];  // Storage updated flag
    deviceUpdatesList += storageUpdates[1];  // Used space
    deviceUpdatesList += storageUpdates[2];  // Delta used space
    deviceUpdatesList += storageUpdates[3];  // Free space
    deviceUpdatesList += storageUpdates[4];  // Delta free space
    deviceUpdatesList += storageUpdates[5];  // Total space
    deviceUpdatesList += storageUpdates[6];  // Delta total space

    qDebug() << "updateStorageOnly completed. Storage updated:" << storageResult.wasUpdated;

    return deviceUpdatesList;
}
