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
        int ID;
        int parentID;
        QString name;
        QString type;
        int externalID;
        QString path;
        qint64 totalFileSize;
        qint64 totalFileCount;
        qint64 totalSpace;
        qint64 freeSpace;
        int groupID;
        QDateTime dateTimeUpdated;

        //Contents
        Storage *storage = new Storage;
        Catalog *catalog = new Catalog;
        QList<int> deviceIDList;


        //Define a structure to represent a row with multiple columns
        struct deviceListRow {
            int ID;
            QString type;
        };

        // Define a QVector to store the rows
        QVector<deviceListRow> deviceListTable;
        //States
        bool hasSubDevice;
        bool hasCatalog;
        bool active;

    //Methods
        void loadDevice();
        void loadDeviceCatalog(); //temp dev
        void loadSubDeviceList();
        void verifyHasSubDevice();
        void verifyHasCatalog();
        void updateActive();

        QList<qint64> updateDevice(QString requestSource, QString databaseMode);
        void updateNumbersFromChildren();
        void updateParentsNumbers();

        void getCatalogStorageID();
        void generateDeviceID();
        void insertDevice();
        void deleteDevice();
        void saveDevice();

        void saveStatistics(QDateTime dateTime);
        void saveStatisticsToFile(QString filePath, QDateTime dateTime);
};

#endif // DEVICE_H
