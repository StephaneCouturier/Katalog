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
// File Name:   device.h
// Purpose:     Class/model for the devices (virtual, storage, or catalogs)
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
        int order;
        QDateTime dateTimeUpdated;

        //Contents
        Storage *storage = new Storage;
        Catalog *catalog = new Catalog;

        QList<int> deviceIDList;
        struct deviceListRow { int ID; QString type; };
        QVector<deviceListRow> deviceListTable;
        QList<Device> subDevices;

        //States
        bool hasSubDevice;
        bool active;

    //Structures
        // Error codes for device operations
        enum DeleteResult {
            DeleteSuccess = 0,
            DeleteCancelled = 1,
            DeleteHasSubDevices = 2,
            DeleteError = 3
        };

        // Structure for delete operation result
        struct DeleteOperationResult {
            DeleteResult result;
            QString errorMessage;
            QString confirmationMessage; // For UI to display confirmation dialog
            bool needsConfirmation;
        };

        // Structure for update operation progress callbacks
        struct UpdateCallbacks {
            std::function<void()> onStartUpdate;
            std::function<void()> onFinishUpdate;
            std::function<bool(const QString&)> onConfirmation; // Returns true if user confirms
        };

    //Methods
        void loadDevice(QString connectionName);

        void verifyHasSubDevice(QString connectionName);
        bool verifyDeviceNameExists();
        bool verifyParentDeviceExistsInPhysicalGroup();
        bool verifyStorageExternalIDExists();
        bool verifyDeviceHasSourceMapping();
        bool verifyDeviceHasTargetMapping();
        void getIDFromDeviceName();
        void updateActiveState(QString connectionName);

        void updateNumbersFromChildren();
        void updateParentsNumbers();
        void getCatalogStorageID();
        void generateDeviceID();
        void insertDevice();
        void saveDevice();
        void saveStatistics(QDateTime dateTime, QString requestSource);
        void setActiveFromString(const QString& activeStr);

        DeleteOperationResult deleteDevice(bool askConfirmation = true,
                                           const UpdateCallbacks* callbacks = nullptr);

        /**
         * @brief Update storage information only (no catalog processing)
         * Simple storage update for initialization and basic operations
         * @param statisticsRequestSource Source identifier for statistics
         * @return QList<qint64> Results in same format as updateDevice for compatibility
         */
        QList<qint64> updateStorageOnly(const QString& statisticsRequestSource);

private:
        QString m_connectionName = "defaultConnection";
        void loadSubDeviceList(QString connectionName);

};

#endif // DEVICE_H
