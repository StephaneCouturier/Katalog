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
#include "qsqlerror.h"

//storage data operation
void Storage::createStorage()
{
    //Generate ID
    QSqlQuery queryDeviceNumber;
    QString queryDeviceNumberSQL = QLatin1String(R"(
                                    SELECT MAX (storage_id)
                                    FROM storage
                                )");
    queryDeviceNumber.prepare(queryDeviceNumberSQL);
    queryDeviceNumber.exec();
    queryDeviceNumber.next();
    int maxID = queryDeviceNumber.value(0).toInt();
    ID = maxID + 1;
    name = name + "_"+QString::number(ID);

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
                            storage_brand_model,
                            storage_serial_number,
                            storage_build_date,
                            storage_content_type,
                            storage_container,
                            storage_comment)
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
                            "")
                    )");

    QSqlQuery insertQuery;
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
    QSqlQuery queryDeviceNumber;
    QString queryDeviceNumberSQL = QLatin1String(R"(
                                                DELETE FROM storage
                                                WHERE storage_id = :storage_id
                                            )");
    queryDeviceNumber.prepare(queryDeviceNumberSQL);
    queryDeviceNumber.bindValue(":storage_id",ID);
    queryDeviceNumber.exec();
}

void Storage::loadStorageMetaData()
{
    QSqlQuery query;
    QString querySQL = QLatin1String(R"(
                            SELECT
                                storage_id,
                                storage_name,
                                storage_type,
                                storage_path,
                                storage_label,
                                storage_file_system,
                                storage_total_space,
                                storage_free_space,
                                storage_brand_model,
                                storage_serial_number,
                                storage_build_date,
                                storage_content_type,
                                storage_container,
                                storage_comment
                            FROM storage
                            WHERE storage_id=:storage_id
                        )");
    query.prepare(querySQL);
    query.bindValue(":storage_id",ID);

    if (query.exec()) {
        if (query.next()) {
            name         = query.value(1).toString();
            type         = query.value(2).toString();
            path         = query.value(4).toString();
            label        = query.value(5).toString();
            fileSystem   = query.value(6).toString();
            totalSpace   = query.value(7).toLongLong();
            freeSpace    = query.value(8).toLongLong();
            brand_model  = query.value(9).toString();
            serialNumber = query.value(10).toString();
            buildDate    = query.value(11).toString();
            contentType  = query.value(12).toString();
            container    = query.value(13).toString();
            comment      = query.value(14).toString();
        } else {
            qDebug() << "No record found for storage_id" << ID;
        }
    } else {
        qDebug() << "Query execution failed:" << query.lastError().text();
    }
}

QList<qint64> Storage::updateStorageInfo()
{
    QList<qint64> list;

    //Get current values for comparison later
    qint64 previousStorageFreeSpace  = freeSpace;
    qint64 previousStorageTotalSpace = totalSpace;
    qint64 previousStorageUsedSpace  = previousStorageTotalSpace - previousStorageFreeSpace;
    QDateTime lastUpdate  = dateUpdated;

    //Get device information
        QStorageInfo storageInfo;
        storageInfo.setPath(path);

        label        = storageInfo.name();
        fileSystem   = storageInfo.fileSystemType();
        totalSpace   = storageInfo.bytesTotal();
        freeSpace    = storageInfo.bytesAvailable();

    //Get confirmation for the update
        qint64 bytesTotal = storageInfo.bytesTotal();
        if (bytesTotal == -1 ){
            // Get the original text
            QString tempText = QString("Katalog could not get values. <br/><br/>"
                               "Check that the source folder ( %1 ) is correct,<br/>"
                               "or that the device is mounted to the source folder.").arg(path);

            // Translate the text using the MainWindow context
            QMessageBox msgBox;
            msgBox.setWindowTitle("Katalog");
            msgBox.setText(QCoreApplication::translate("MainWindow", tempText.toUtf8()));
            msgBox.setIcon(QMessageBox::Warning);
            msgBox.exec();
            return list;
        }

        dateUpdated = QDateTime::currentDateTime();

    //Save to Storage table
        QSqlQuery queryTotalSpace;
        QString queryTotalSpaceSQL = QLatin1String(R"(
                                        UPDATE storage
                                        SET storage_total_space = :storage_total_space,
                                            storage_free_space  = :storage_free_space,
                                            storage_label       = :storage_label,
                                            storage_file_system = :storage_file_system
                                        WHERE storage_id = :storage_id
                                        )");
        queryTotalSpace.prepare(queryTotalSpaceSQL);
        queryTotalSpace.bindValue(":storage_total_space",QString::number(totalSpace));
        queryTotalSpace.bindValue(":storage_free_space",QString::number(freeSpace));
        queryTotalSpace.bindValue(":storage_label",label);
        queryTotalSpace.bindValue(":storage_file_system",fileSystem);
        queryTotalSpace.bindValue(":storage_id", ID);
        queryTotalSpace.exec();

    //Save to VirtualStorage table
        queryTotalSpaceSQL = QLatin1String(R"(
                                        UPDATE device
                                        SET device_total_space = :device_total_space,
                                            device_free_space  = :device_free_space
                                        WHERE device_external_id = :device_external_id
                                        AND device_type ='Storage'
                                        )");
        queryTotalSpace.prepare(queryTotalSpaceSQL);
        queryTotalSpace.bindValue(":device_total_space",QString::number(totalSpace));
        queryTotalSpace.bindValue(":device_free_space",QString::number(freeSpace));
        queryTotalSpace.bindValue(":device_external_id", ID);
        queryTotalSpace.exec();

    //Stop if the update was not done (lastUpdate time did not change)
        if (lastUpdate == dateUpdated)
            return list;

    //Prepare to report changes to the storage
        qint64 newStorageFreeSpace    = freeSpace;
        qint64 deltaStorageFreeSpace  = newStorageFreeSpace - previousStorageFreeSpace;
        qint64 newStorageTotalSpace   = totalSpace;
        qint64 deltaStorageTotalSpace = newStorageTotalSpace - previousStorageTotalSpace;
        qint64 newStorageUsedSpace    = newStorageTotalSpace - newStorageFreeSpace;
        qint64 deltaStorageUsedSpace  = newStorageUsedSpace - previousStorageUsedSpace;

        list.append(newStorageUsedSpace);
        list.append(deltaStorageUsedSpace);
        list.append(newStorageFreeSpace);
        list.append(deltaStorageFreeSpace);
        list.append(newStorageTotalSpace);
        list.append(deltaStorageTotalSpace);

        return list;

}

void Storage::saveStatistics(QDateTime dateTime)
{      
        QSqlQuery querySaveStatistics;
        QString querySaveStatisticsSQL = QLatin1String(R"(
                                        INSERT INTO statistics_storage(
                                                date_time,
                                                storage_name,
                                                storage_free_space,
                                                storage_total_space,
                                                record_type)
                                        VALUES(
                                                :date_time,
                                                :storage_name,
                                                :storage_free_space,
                                                :storage_total_space,
                                                :record_type)
                                        )");
        querySaveStatistics.prepare(querySaveStatisticsSQL);
        querySaveStatistics.bindValue(":date_time", dateTime.toString("yyyy-MM-dd hh:mm:ss"));
        querySaveStatistics.bindValue(":storage_name", name);
        querySaveStatistics.bindValue(":storage_free_space", freeSpace);
        querySaveStatistics.bindValue(":storage_total_space", totalSpace);
        if (dateTime == dateUpdated)
            querySaveStatistics.bindValue(":record_type", "update");
        else
            querySaveStatistics.bindValue(":record_type", "snapshot");

        querySaveStatistics.exec();
}

void Storage::saveStatisticsToFile(QString filePath, QDateTime dateTime)
{
        //Prepare file and data
        QFile fileOut(filePath);
        QString record_type;
        if (dateTime == dateUpdated)
            record_type = "update";
        else
            record_type = "snapshot";

        QString statisticsLine =   dateTime.toString("yyyy-MM-dd hh:mm:ss") + "\t"
                                + name + "\t"
                                + QString::number(freeSpace) + "\t"
                                + QString::number(totalSpace) + "\t"
                                + QString::number(ID) + "\t"
                                + record_type;

        // Write data
        if (fileOut.open(QFile::WriteOnly | QIODevice::Append | QFile::Text)) {
            QTextStream stream(&fileOut);
            stream << statisticsLine << "\n";
        }
        fileOut.close();
}
