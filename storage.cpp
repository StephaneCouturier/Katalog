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

//set storage device definition
void Storage::setID(int selectedID)
{
    ID = selectedID;
}

//storage data operation
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

        qint64 bytesTotal = storageInfo.bytesTotal();

    //Get confirmation for the update
        if (bytesTotal == -1 ){
            //QMessageBox::warning(this,tr("Katalog"),tr("Katalog could not get values. <br/> Check the source folder, or that the device is mounted to the source folder."));
            return;
        }

    //SQL updates
        QSqlQuery queryTotalSpace;

        QString queryTotalSpaceSQL = QLatin1String(R"(
                                        UPDATE storage
                                        SET storage_total_space = :storage_total_space,
                                            storage_free_space  = :storage_free_space,
                                            storage_label       = :storage_label,
                                            storage_file_system = :storage_file_system
                                        WHERE storageID = :storageID
                                        )");
        queryTotalSpace.prepare(queryTotalSpaceSQL);
        queryTotalSpace.bindValue(":storage_total_space",QString::number(storageInfo.bytesTotal()));
        queryTotalSpace.bindValue(":storage_free_space",QString::number(storageInfo.bytesAvailable()));
        queryTotalSpace.bindValue(":storage_label",storageInfo.name());
        queryTotalSpace.bindValue(":storage_file_system",storageInfo.fileSystemType());
        queryTotalSpace.exec();

}
