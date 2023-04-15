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
#include <QSqlQuery>
#include <QVariant>
#include <QMessageBox>
#include <QCoreApplication>

//set storage device definition
void Storage::setID(int selectedID)
{
    ID = selectedID;
}
void Storage::setName(QString selectedName)
{
    name = selectedName;
}
void Storage::setFreeSpace(qint64 selectedFreeSpace)
{
    freeSpace = selectedFreeSpace;
}
void Storage::setTotalSpace(qint64 selectedTotalSpace)
{
    totalSpace = selectedTotalSpace;
}
void Storage::setDateUpdated(QDateTime dateTime)
{
    dateUpdated = dateTime;
}
void Storage::setLocation(QString selectedLocation){
    location = selectedLocation;
}

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
    int newID = maxID + 1;

    //Insert new device with default values
    QString querySQL = QLatin1String(R"(
            INSERT INTO storage(
                            storage_id,
                            storage_name,
                            storage_type,
                            storage_location,
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
                            :new_location,
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
    insertQuery.bindValue(":new_id", newID);
    insertQuery.bindValue(":storage_name", name + "_"+QString::number(newID));
    if(name=="")
        insertQuery.bindValue(":storage_name","");

    insertQuery.bindValue(":new_location", location);
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
                                storage_location,
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
    query.exec();
    query.next();

    name         = query.value(1).toString();
    type         = query.value(2).toString();
    location     = query.value(3).toString();
    path         = query.value(4).toString();
    label        = query.value(5).toString();
    fileSystem   = query.value(6).toString();
    totalSpace   = query.value(7).toLongLong();
    freeSpace    = query.value(8).toLongLong();
    brand        = query.value(9).toString();
    model        = query.value(10).toString();
    serialNumber = query.value(11).toString();
    buildDate    = query.value(12).toString();
    contentType  = query.value(13).toString();
    container    = query.value(14).toString();
    comment      = query.value(15).toString();
}

void Storage::updateStorageInfo()
{
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
            return;
        }

        dateUpdated = QDateTime::currentDateTime();

    //Save
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
