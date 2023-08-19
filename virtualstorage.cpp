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

#include "virtualstorage.h"

void VirtualStorage::loadVirtualStorage(){
    //Retrieve virtual_storage hierarchy
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
                                    virtual_storage_free_space
                            FROM  virtual_storage
                            WHERE virtual_storage_id =:virtual_storage_id
                        )");
    query.prepare(querySQL);
    query.bindValue(":virtual_storage_id",ID);
    query.exec();
    query.next();

    parentID    = query.value(1).toInt();
    name        = query.value(2).toString();
    type        = query.value(3).toString();
    externalID  = query.value(4).toInt();
    path        = query.value(5).toString();
    total_file_size  = query.value(6).toLongLong();
    total_file_count = query.value(7).toLongLong();
    total_space = query.value(8).toLongLong();
    free_space  = query.value(9).toLongLong();
}
