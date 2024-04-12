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
// File Name:   device.h
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
    //Attributes  
        //Saved
        int ID = 0;
        int parentID = 0;
        QString name;
        QString type;
        int externalID = 0;
        QString path;
        qint64 totalFileSize = 0;
        qint64 totalFileCount = 0;
        qint64 totalSpace = 0;
        qint64 freeSpace = 0;
        int groupID = 0;
        QDateTime dateTimeUpdated;

        //Contents
        Storage *storage = new Storage;
        Catalog *catalog = new Catalog;

        QList<int> deviceIDList;
        struct deviceListRow { int ID; QString type; };
        QVector<deviceListRow> deviceListTable;

        //States
        bool hasSubDevice;
        bool active;

    //Methods
        void loadDevice();

        void verifyHasSubDevice();
        bool verifyDeviceNameExists();
        bool verifyParentDeviceExistsInPhysicalGroup();
        bool verifyStorageExternalIDExists();
        void updateActive();

        QList<qint64> updateDevice(QString statiticsRequestSource,
                                   QString databaseMode,
                                   bool reportStorageUpdate,
                                   QString collectionFolder,
                                   bool includeSubDevices);
        void updateNumbersFromChildren();
        void updateParentsNumbers();

        void getCatalogStorageID();
        void generateDeviceID();
        void insertDevice();
        void deleteDevice(bool askConfirmation);
        void saveDevice();
        void saveStatistics(QDateTime dateTime, QString requestSource);

private:
        void loadSubDeviceList();

};

#endif // DEVICE_H
