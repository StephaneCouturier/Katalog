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
// File Name:   storage.cpp
// Purpose:     class to manage storage devices
// Description:
// Author:      Stephane Couturier
/////////////////////////////////////////////////////////////////////////////
*/

#include "storage.h"
#include <QSqlError>
#include <QDebug>

//storage data operation

void Storage::generateID()
{//Generate ID and add it to name
    QSqlQuery queryDeviceNumber(QSqlDatabase::database(m_connectionName));
    QString queryDeviceNumberSQL = QLatin1String(R"(
                                        SELECT MAX(storage_id)
                                        FROM storage
                                    )");
    queryDeviceNumber.prepare(queryDeviceNumberSQL);
    queryDeviceNumber.exec();
    queryDeviceNumber.next();
    int maxID = queryDeviceNumber.value(0).toInt();
    ID = maxID + 1;
    name = name + "_"+QString::number(ID);
}

void Storage::insertStorage()
{
    //Insert new device with default values
    QString querySQL = QLatin1String(R"(
            INSERT INTO storage(
                            storage_id,
                            storage_name,
                            storage_type,
                            storage_path,
                            storage_label,
                            storage_file_system,
                            storage_total_space,
                            storage_free_space,
                            storage_brand,
                            storage_model,
                            storage_serial_number,
                            storage_build_date,
                            storage_comment1,
                            storage_comment2,
                            storage_comment3)
                      VALUES(
                            :new_id,
                            :storage_name,
                            "",
                            "",
                            "",
                            "",
                            0,
                            0,
                            "",
                            "",
                            "",
                            "",
                            "",
                            "",
                            "")
                    )");

    QSqlQuery insertQuery(QSqlDatabase::database(m_connectionName));
    insertQuery.prepare(querySQL);
    insertQuery.bindValue(":new_id", ID);
    insertQuery.bindValue(":storage_name", name);
    if(name=="")
        insertQuery.bindValue(":storage_name","");

    insertQuery.exec();
}

void Storage::deleteStorage()
{
    //Delete from the table
    QSqlQuery queryDeviceNumber(QSqlDatabase::database(m_connectionName));
    QString queryDeviceNumberSQL = QLatin1String(R"(
                                                DELETE FROM storage
                                                WHERE storage_id = :storage_id
                                            )");
    queryDeviceNumber.prepare(queryDeviceNumberSQL);
    queryDeviceNumber.bindValue(":storage_id",ID);
    queryDeviceNumber.exec();
}

void Storage::loadStorage(QString connectionName)
{
    QSqlQuery query(QSqlDatabase::database(connectionName));
    QString querySQL = QLatin1String(R"(
                            SELECT
                                storage_name,
                                storage_type,
                                storage_path,
                                storage_label,
                                storage_file_system,
                                storage_total_space,
                                storage_free_space,
                                storage_brand,
                                storage_model,
                                storage_serial_number,
                                storage_build_date,
                                storage_comment1,
                                storage_comment2,
                                storage_comment3
                            FROM storage
                            WHERE storage_id=:storage_id
                        )");
    query.prepare(querySQL);
    query.bindValue(":storage_id",ID);

    if (query.exec()) {
        if (query.next()) {
            name         = query.value(0).toString();
            type         = query.value(1).toString();
            path         = query.value(2).toString();
            label        = query.value(3).toString();
            fileSystem   = query.value(4).toString();
            totalSpace   = query.value(5).toLongLong();
            freeSpace    = query.value(6).toLongLong();
            brand        = query.value(7).toString();
            model        = query.value(8).toString();
            serialNumber = query.value(9).toString();
            buildDate    = query.value(10).toString();
            comment1     = query.value(11).toString();
            comment2     = query.value(12).toString();
            comment3     = query.value(13).toString();
        } else {
            qDebug() << "No record found for storage_id" << ID;
        }
    } else {
        qDebug() << "Query execution failed:" << query.lastError().text();
    }
}

bool Storage::isDirectoryEmpty(const QString &dirPath)
{
    QDir dir(dirPath);
    return dir.entryInfoList(QDir::NoDotAndDotDot|QDir::AllEntries).count() == 0;
}

bool Storage::getStorageInfo()
{
    QStorageInfo storageInfo;
    storageInfo.setPath(path);

    label        = storageInfo.name();
    fileSystem   = storageInfo.fileSystemType();
    totalSpace   = storageInfo.bytesTotal();
    freeSpace    = storageInfo.bytesAvailable();

    // Check if we got valid values
    return storageInfo.bytesTotal() != -1;
}

//------------------------------------------------------------------------------
qint64 Storage::availableSpace(const QString &path)
{
    const QStorageInfo info(path);
    if (!info.isValid())
        return -1;
    return info.bytesAvailable();
}

Storage::UpdateResult Storage::updateStorageInfo()
{
    UpdateResult result;
    result.wasUpdated = false;
    result.errorCode = Success;

    // Verify if path is available / not empty
    QDir dir(path);

    // Check if path is provided
    if (path.isEmpty()) {
        result.errorCode = ErrorNoPath;
        result.errorMessage = QString("No Path was provided for the Storage: %1. Edit the device to provide one and try again.").arg(name);
        return result;
    }

    // Check if directory is empty
    if (isDirectoryEmpty(path)) {
        result.errorCode = ErrorEmptyDirectory;
        result.errorMessage = QString("Storage: '%1'\n\nThe source folder does not contain any file: '%2'\n\nThis could mean that the device is not mounted to this folder, or the folder is simply empty.").arg(name, path);
        return result;
    }

    // Get current values for comparison later
    qint64 previousStorageFreeSpace  = freeSpace;
    qint64 previousStorageTotalSpace = totalSpace;
    qint64 previousStorageUsedSpace  = previousStorageTotalSpace - previousStorageFreeSpace;
    QDateTime lastUpdate = dateTimeUpdated;

    // Get device information
    if (!getStorageInfo()) {
        result.errorCode = ErrorCannotGetValues;
        result.errorMessage = QString("Katalog could not get values.\n\nCheck that the source folder (%1) is correct, or that the device is mounted to the source folder.").arg(path);
        return result;
    }

    dateTimeUpdated = QDateTime::currentDateTime();

    // Save to Storage table
    QSqlQuery queryTotalSpace(QSqlDatabase::database(m_connectionName));
    QString queryTotalSpaceSQL = QLatin1String(R"(
                                    UPDATE storage
                                    SET storage_total_space = :storage_total_space,
                                        storage_free_space  = :storage_free_space,
                                        storage_label       = :storage_label,
                                        storage_file_system = :storage_file_system
                                    WHERE storage_id = :storage_id
                                    )");
    queryTotalSpace.prepare(queryTotalSpaceSQL);
    queryTotalSpace.bindValue(":storage_total_space", totalSpace);
    queryTotalSpace.bindValue(":storage_free_space", freeSpace);
    queryTotalSpace.bindValue(":storage_label", label);
    queryTotalSpace.bindValue(":storage_file_system", fileSystem);
    queryTotalSpace.bindValue(":storage_id", ID);
    if (!queryTotalSpace.exec()) {
        qDebug() << "ERROR updating storage table:" << queryTotalSpace.lastError().text();
        qDebug() << "Storage ID:" << ID << "TotalSpace:" << totalSpace << "FreeSpace:" << freeSpace;
    }

    // Save to Device table
    queryTotalSpaceSQL = QLatin1String(R"(
                                    UPDATE device
                                    SET device_total_space = :device_total_space,
                                        device_free_space  = :device_free_space
                                    WHERE device_external_id = :device_external_id
                                    AND device_type ='Storage'
                                    )");
    queryTotalSpace.prepare(queryTotalSpaceSQL);
    queryTotalSpace.bindValue(":device_total_space", QString::number(totalSpace));
    queryTotalSpace.bindValue(":device_free_space", QString::number(freeSpace));
    queryTotalSpace.bindValue(":device_external_id", ID);
    queryTotalSpace.exec();

    // Check if the update was actually done (lastUpdate time did not change)
    if (lastUpdate == dateTimeUpdated) {
        result.errorCode = ErrorNotUpdated;
        result.errorMessage = "Storage was not updated - no changes detected.";
        return result;
    }

    // Calculate changes to report
    qint64 newStorageFreeSpace    = freeSpace;
    qint64 deltaStorageFreeSpace  = newStorageFreeSpace - previousStorageFreeSpace;
    qint64 newStorageTotalSpace   = totalSpace;
    qint64 deltaStorageTotalSpace = newStorageTotalSpace - previousStorageTotalSpace;
    qint64 newStorageUsedSpace    = newStorageTotalSpace - newStorageFreeSpace;
    qint64 deltaStorageUsedSpace  = newStorageUsedSpace - previousStorageUsedSpace;

    // Populate result
    result.wasUpdated = true;
    result.errorCode = Success;
    result.newUsedSpace = newStorageUsedSpace;
    result.deltaUsedSpace = deltaStorageUsedSpace;
    result.newFreeSpace = newStorageFreeSpace;
    result.deltaFreeSpace = deltaStorageFreeSpace;
    result.newTotalSpace = newStorageTotalSpace;
    result.deltaTotalSpace = deltaStorageTotalSpace;

    return result;
}
