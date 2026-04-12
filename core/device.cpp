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
#include <QCoreApplication>
#include <qelapsedtimer.h>

void Device::loadDevice(QString connectionName){
    QElapsedTimer totalTimer;
    totalTimer.start();
    QElapsedTimer stepTimer;
    stepTimer.start();
    bool useTimerForDebug = false;

    QSqlDatabase db = QSqlDatabase::database(connectionName);
    if (!db.isOpen()) {
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

    if(useTimerForDebug){
    }

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
            qWarning() << "WARNING: DEBUG: loadDevice query failed, no record found for device_id" << ID;
        }
    } else {
        qWarning() << "WARNING: DEBUG: loadDevice query execution failed:" << query.lastError().text();
        return;
    }


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
        stepTimer.restart();
    }

    //Load catalog values
    if(type == "Catalog"){
        catalog->ID = externalID;
        catalog->loadCatalog();
        if (catalog->includeMetadata == "false") {
            catalog->includeMetadata = Catalog::METADATA_NONE;
        }
        catalog->name = name;
        catalog->sourcePath = path;
        catalog->fileCount = totalFileCount;
        catalog->totalFileSize = totalFileSize;
    }
    if(useTimerForDebug){
        stepTimer.restart();
    }

    //Load sub-device list
    loadSubDeviceList(connectionName);

    if(useTimerForDebug){
        stepTimer.restart();
    }

    //Update states
    verifyHasSubDevice(connectionName);
    if(useTimerForDebug){
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
    QSqlQuery queryGetCatalogStorageID(QSqlDatabase::database(m_connectionName));
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
            qWarning() << "WARNING: getCatalogStorageID failed, no record found for device_name" << name;
        }
    } else {
        qWarning() << "WARNING: DEBUG: getCatalogStorageID query execution failed:" << queryGetCatalogStorageID.lastError().text();
    }
}

void Device::generateDeviceID()
{//Generate new ID
    if(ID==0){
        QSqlQuery queryGenerateDeviceID(QSqlDatabase::database(m_connectionName));
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

    QSqlQuery queryInsertDevice(QSqlDatabase::database(m_connectionName));
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
    QSqlQuery query(QSqlDatabase::database(m_connectionName));
    QString querySQL = QLatin1String(R"(
                                    SELECT COUNT(*)
                                    FROM   device
                                    WHERE  device_name = :device_name
                                )");

    query.prepare(querySQL);
    query.bindValue(":device_name", name);

    if (!query.exec() and ID !=0) {
        qWarning() << "WARNING: DEBUG: Error executing verifyDeviceNameExists:" << query.lastError().text();
        return false;
    }

    query.next();
    return query.value(0).toInt() > 0;
}

bool Device::verifyParentDeviceExistsInPhysicalGroup()
{
    QSqlQuery query(QSqlDatabase::database(m_connectionName));
    QString querySQL = QLatin1String(R"(
                                    SELECT COUNT(*)
                                    FROM   device
                                    WHERE  device_id = :device_id
                                    AND group_id = 0
                                )");

    query.prepare(querySQL);
    query.bindValue(":device_id", parentID);

    if (!query.exec()) {
        qWarning() << "WARNING: DEBUG: Error executing verifyDeviceNameExists:" << query.lastError().text();
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
    QSqlQuery queryExternalID(QSqlDatabase::database(m_connectionName));
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
    QSqlQuery query(QSqlDatabase::database(m_connectionName));
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
    QSqlQuery query(QSqlDatabase::database(m_connectionName));
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
    QSqlQuery queryIDFromDeviceName(QSqlDatabase::database(m_connectionName));
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
    QSqlQuery query(QSqlDatabase::database(m_connectionName));
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
    QSqlQuery query(QSqlDatabase::database(m_connectionName));
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
    QSqlQuery query(QSqlDatabase::database(m_connectionName));
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
        tempCurrentDevice.loadDevice(m_connectionName);
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
    QSqlQuery querySaveStatistics(QSqlDatabase::database(m_connectionName));
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

    verifyHasSubDevice(m_connectionName);

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
                                         QCoreApplication::translate("MainWindow", "Do you want to <span style='color: red';>delete</span> this %1 device?").arg(type)
                                         +"<table><tr><td>" +QCoreApplication::translate("MainWindow", "ID")+":   </td><td><b>" +QString::number(ID)+" </td></tr><tr><td>"
                                         +QCoreApplication::translate("MainWindow", "Name") +": </td><td><b>"+ name +"</td></tr><tr><td></td></tr></table>"
                                         );




        if (callbacks && callbacks->onConfirmation) {
            if (!callbacks->onConfirmation(result.confirmationMessage)) {
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
    QSqlQuery query(QSqlDatabase::database(m_connectionName));
    QString querySQL = QLatin1String(R"(
                        DELETE FROM device
                        WHERE device_id=:device_id
                    )");
    query.prepare(querySQL);
    query.bindValue(":device_id", ID);

    if (!query.exec()) {
        qWarning() << "WARNING: Device::deleteDevice - Database error:" << query.lastError().text();
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


    QList<qint64> deviceUpdatesList;

    // Only valid for Storage devices
    if (type != "Storage") {
        qWarning() << "WARNING: updateStorageOnly called on non-Storage device:" << type;
        // Return empty results to indicate error
        return QList<qint64>();
    }

    // Update device state (same as original logic)
    updateActiveState(m_connectionName);
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


    return deviceUpdatesList;
}

//----------------------------------------------------------------------
// Static assign/unassign operations
//----------------------------------------------------------------------

bool Device::isCatalogAssigned(int catalogExternalId, int parentDeviceId, const QString &connectionName)
{
    QSqlQuery query(QSqlDatabase::database(connectionName));
    QString querySQL = QLatin1String(R"(
        SELECT COUNT(*)
        FROM device
        WHERE device_parent_id =:device_parent_id
        AND device_external_id =:device_external_id
    )");
    query.prepare(querySQL);
    query.bindValue(":device_parent_id", parentDeviceId);
    query.bindValue(":device_external_id", catalogExternalId);
    query.exec();
    query.next();
    return query.value(0).toInt() > 0;
}

int Device::generateNextDeviceID(const QString &connectionName)
{
    QSqlQuery query(QSqlDatabase::database(connectionName));
    QString querySQL = QLatin1String(R"(
        SELECT MAX(device_id)
        FROM device
    )");
    query.prepare(querySQL);
    query.exec();
    query.next();
    return query.value(0).toInt() + 1;
}

bool Device::assignCatalogToDevice(Device *catalogDevice, Device *parentDevice, const QString &connectionName)
{
    if (parentDevice->ID == 0 || catalogDevice->ID == 0)
        return false;

    // Check if already assigned
    if (isCatalogAssigned(catalogDevice->externalID, parentDevice->ID, connectionName))
        return false;

    int newID = generateNextDeviceID(connectionName);

    QSqlQuery query(QSqlDatabase::database(connectionName));
    QString querySQL = QLatin1String(R"(
        INSERT INTO device(
            device_id,
            device_parent_id,
            device_name,
            device_type,
            device_external_id,
            device_path,
            device_total_file_size,
            device_total_file_count,
            device_total_space,
            device_free_space,
            device_active,
            device_group_id,
            device_date_updated)
        VALUES(
            :device_id,
            :device_parent_id,
            :device_name,
            :device_type,
            :device_external_id,
            :device_path,
            :device_total_file_size,
            :device_total_file_count,
            :device_total_space,
            :device_free_space,
            :device_active,
            :device_group_id,
            :device_date_updated)
    )");
    query.prepare(querySQL);
    query.bindValue(":device_id", newID);
    query.bindValue(":device_parent_id", parentDevice->ID);
    query.bindValue(":device_name", catalogDevice->name);
    query.bindValue(":device_type", "Catalog");
    query.bindValue(":device_external_id", catalogDevice->catalog->ID);
    query.bindValue(":device_path", catalogDevice->catalog->sourcePath);
    query.bindValue(":device_total_file_size", catalogDevice->catalog->totalFileSize);
    query.bindValue(":device_total_file_count", catalogDevice->catalog->fileCount);
    query.bindValue(":device_total_space", 0);
    query.bindValue(":device_free_space", 0);
    query.bindValue(":device_active", catalogDevice->active);
    query.bindValue(":device_group_id", parentDevice->groupID);
    query.bindValue(":device_date_updated", catalogDevice->dateTimeUpdated);

    if (!query.exec()) {
        qWarning() << "WARNING: Failed to assign catalog to device:" << query.lastError().text();
        return false;
    }

    return true;
}

bool Device::assignStorageToDevice(Storage *storage, int parentDeviceId, const QString &connectionName)
{
    if (parentDeviceId == 0 || storage->ID == 0)
        return false;

    int newID = generateNextDeviceID(connectionName);

    QSqlQuery query(QSqlDatabase::database(connectionName));
    QString querySQL = QLatin1String(R"(
        INSERT INTO device(
            device_id,
            device_parent_id,
            device_name,
            device_type,
            device_external_id,
            device_path,
            device_total_file_size,
            device_total_file_count,
            device_total_space,
            device_free_space)
        VALUES(
            :device_id,
            :device_parent_id,
            :device_name,
            :device_type,
            :device_external_id,
            :device_path,
            :device_total_file_size,
            :device_total_file_count,
            :device_total_space,
            :device_free_space)
    )");
    query.prepare(querySQL);
    query.bindValue(":device_id", newID);
    query.bindValue(":device_parent_id", parentDeviceId);
    query.bindValue(":device_name", storage->name);
    query.bindValue(":device_type", "Storage");
    query.bindValue(":device_external_id", storage->ID);
    query.bindValue(":device_path", storage->path);
    query.bindValue(":device_total_file_size", 0);
    query.bindValue(":device_total_file_count", 0);
    query.bindValue(":device_total_space", storage->totalSpace);
    query.bindValue(":device_free_space", storage->freeSpace);

    if (!query.exec()) {
        qWarning() << "WARNING: Failed to assign storage to device:" << query.lastError().text();
        return false;
    }

    return true;
}

bool Device::unassignFromDevice(int deviceID, int deviceParentID, const QString &connectionName)
{
    if (deviceID == 0 || deviceParentID == 0)
        return false;

    QSqlQuery query(QSqlDatabase::database(connectionName));
    QString querySQL = QLatin1String(R"(
        DELETE FROM device
        WHERE device_id=:device_id
        AND   device_parent_id=:device_parent_id
    )");
    query.prepare(querySQL);
    query.bindValue(":device_id", deviceID);
    query.bindValue(":device_parent_id", deviceParentID);

    if (!query.exec()) {
        qWarning() << "WARNING: Failed to unassign from device:" << query.lastError().text();
        return false;
    }

    return true;
}
//----------------------------------------------------------------------
int Device::getMaxHierarchyDepth(const QString &connectionName)
{
    QSqlQuery query(QSqlDatabase::database(connectionName));
    query.prepare(QLatin1String(R"(
        WITH RECURSIVE device_tree AS (
          SELECT device_id, device_parent_id, 0 AS level
          FROM device
          WHERE device_parent_id = 0
          UNION ALL
          SELECT child.device_id, child.device_parent_id, parent.level + 1 AS level
          FROM device_tree parent
          JOIN device child ON child.device_parent_id = parent.device_id
        )
        SELECT MAX(level) AS total_levels FROM device_tree
    )"));

    if (query.exec() && query.next()) {
        int depth = query.value(0).toInt();
        return (depth == 0) ? 4 : depth; // fallback if tree is flat
    }
    return 4; // safe default
}
//----------------------------------------------------------------------
QString Device::getDevicePath(int deviceId, const QString &connectionName)
{
    // Walk up via device_parent_id, building the ancestor chain iteratively.
    QSqlDatabase db = QSqlDatabase::database(connectionName);
    QStringList parts;
    int id = deviceId;
    while (id > 0) {
        QSqlQuery q(db);
        q.prepare("SELECT device_name, device_parent_id FROM device WHERE device_id = :id");
        q.bindValue(":id", id);
        if (!q.exec() || !q.next())
            break;
        parts.prepend(q.value(0).toString());
        id = q.value(1).toInt();
    }
    return parts.join(" / ");
}
