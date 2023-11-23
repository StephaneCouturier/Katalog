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
// File Name:   virtualstorage.h
// Purpose:     Class/model for the virtual storage device
// Description:
// Author:      Stephane Couturier
/////////////////////////////////////////////////////////////////////////////
*/
#ifndef DEVICE_H
#define DEVICE_H

#include <QVariant>
#include <QSqlQuery>

#include "storage.h"
#include "catalog.h"

class Device
{

public:
    int ID;
    int parentID;
    QString name;
    QString type;
    int externalID;
    QString path;
    qint64 total_file_size;
    qint64 total_file_count;
    qint64 total_space;
    qint64 free_space;
    int groupID;

    Storage *storage = new Storage;
    Catalog *catalog = new Catalog;

    bool hasSubDevice;
    bool hasCatalog;

    void loadDevice();
    void loadDeviceCatalog(); //temp dev

    void getCatalogStorageID();
    void generateDeviceID();
    void insertDevice();
    void verifyHasSubDevice();
    void verifyHasCatalog();
    void deleteDevice();
    void saveDevice();
};

#endif // DEVICE_H
