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
// File Name:   virtualstorage.cpp
// Purpose:     class to manage virtual storage devices
// Description:
// Author:      Stephane Couturier
/////////////////////////////////////////////////////////////////////////////
*/

#include "device.h"
#include "qsqlerror.h"

void Device::loadDevice(){
    //Retrieve virtual_storage values
    QSqlQuery query;
    QString querySQL = QLatin1String(R"(
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
                                    virtual_storage_group_id
                            FROM  virtual_storage
                            WHERE virtual_storage_id =:virtual_storage_id
                        )");

    query.prepare(querySQL);
    query.bindValue(":virtual_storage_id",ID);

    if (query.exec()) {
        if (query.next()) {
            parentID    = query.value(1).toInt();
            name        = query.value(2).toString();
            type        = query.value(3).toString();
            externalID  = query.value(4).toInt();
            path        = query.value(5).toString();
            total_file_size  = query.value(6).toLongLong();
            total_file_count = query.value(7).toLongLong();
            total_space = query.value(8).toLongLong();
            free_space  = query.value(9).toLongLong();
        } else {
            qDebug() << "No record found for virtual storage_id" << ID;
        }
    } else {
        qDebug() << "Query execution failed:" << query.lastError().text();
    }
}

void Device::loadDeviceCatalog(){
    //Retrieve virtual_storage values
    QSqlQuery query;
    QString querySQL = QLatin1String(R"(
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
                                    virtual_storage_group_id
                            FROM  virtual_storage
                            WHERE virtual_storage_name =:virtual_storage_name
                        )");

    query.prepare(querySQL);
    query.bindValue(":virtual_storage_name",name);

    if (query.exec()) {
        if (query.next()) {
            parentID    = query.value(1).toInt();
            name        = query.value(2).toString();
            type        = query.value(3).toString();
            externalID  = query.value(4).toInt();
            path        = query.value(5).toString();
            total_file_size  = query.value(6).toLongLong();
            total_file_count = query.value(7).toLongLong();
            total_space = query.value(8).toLongLong();
            free_space  = query.value(9).toLongLong();
        } else {
            qDebug() << "No record found for virtual storage_id" << ID;
        }
    } else {
        qDebug() << "Query execution failed:" << query.lastError().text();
    }
}

void Device::getCatalogStorageID(){
    //Retrieve virtual_storage_parent_id for an item in the physical group
    QSqlQuery query;
    QString querySQL = QLatin1String(R"(
                    WITH RECURSIVE find_special AS
                        (SELECT virtual_storage_id, virtual_storage_parent_id, virtual_storage_name
                        FROM virtual_storage WHERE virtual_storage_id = 1

                        UNION ALL

                        SELECT vs.virtual_storage_id, vs.virtual_storage_parent_id, vs.virtual_storage_name
                        FROM virtual_storage AS vs
                        INNER JOIN find_special AS fs ON vs.virtual_storage_parent_id = fs.virtual_storage_id)
                    SELECT virtual_storage_id
                    FROM find_special
                    WHERE virtual_storage_name = :name
                )");
    query.prepare(querySQL);
    query.bindValue(":name", name);

    if (query.exec()) {
        if (query.next()) {
            ID = query.value(0).toInt();
        } else {
            qDebug() << "No record found for virtual_storage_name" << name;
        }
    } else {
        qDebug() << "Query execution failed:" << query.lastError().text();
    }
}

void Device::insertDeviceItem(){

    QSqlQuery query;
    QString querySQL;

    if(ID==0){
        //Generate new ID
        querySQL = QLatin1String(R"(
                        SELECT MAX(virtual_storage_id)
                        FROM virtual_storage
                    )");
        query.prepare(querySQL);
        query.exec();
        query.next();
        ID = query.value(0).toInt() + 1;
    }

    //Insert device
    querySQL = QLatin1String(R"(
                            INSERT INTO virtual_storage(
                                        virtual_storage_id,
                                        virtual_storage_parent_id,
                                        virtual_storage_name,
                                        virtual_storage_type,
                                        virtual_storage_external_id,
                                        virtual_storage_group_id)
                            VALUES(
                                        :virtual_storage_id,
                                        :virtual_storage_parent_id,
                                        :virtual_storage_name,
                                        :virtual_storage_type,
                                        :virtual_storage_external_id,
                                        :virtual_storage_group_id)
                                )");
    query.prepare(querySQL);
    query.bindValue(":virtual_storage_id", ID);
    query.bindValue(":virtual_storage_parent_id",parentID);
    query.bindValue(":virtual_storage_name",name);
    query.bindValue(":virtual_storage_type", type);
    query.bindValue(":virtual_storage_external_id", externalID);
    query.bindValue(":virtual_storage_group_id", groupID);
    query.exec();
}

void Device::verifyHasSubDevice(){

    QSqlQuery queryVerifyChildren;
    QString queryVerifyChildrenSQL = QLatin1String(R"(
                                SELECT COUNT(*)
                                FROM virtual_storage
                                WHERE virtual_storage_parent_id=:virtual_storage_parent_id
                            )");
    queryVerifyChildren.prepare(queryVerifyChildrenSQL);
    queryVerifyChildren.bindValue(":virtual_storage_parent_id", ID);
    queryVerifyChildren.exec();
    queryVerifyChildren.next();

    if(queryVerifyChildren.value(0).toInt()>=1)
        hasSubDevice = true;
}

void Device::verifyHasCatalog(){

    QSqlQuery queryVerifyCatalog;
    QString queryVerifyCatalogSQL = QLatin1String(R"(
                                SELECT COUNT(*)
                                FROM virtual_storage_catalog
                                WHERE virtual_storage_id=:virtual_storage_id
                            )");
    queryVerifyCatalog.prepare(queryVerifyCatalogSQL);
    queryVerifyCatalog.bindValue(":virtual_storage_id", ID);
    queryVerifyCatalog.exec();
    queryVerifyCatalog.next();

    if(queryVerifyCatalog.value(0).toInt()>=1)
        hasCatalog = true;
}

void Device::deleteDevice(){

    QSqlQuery query;
    QString querySQL = QLatin1String(R"(
                                    DELETE FROM virtual_storage
                                    WHERE virtual_storage_id=:virtual_storage_id
                                )");
    query.prepare(querySQL);
    query.bindValue(":virtual_storage_id", ID);
    query.exec();

}
