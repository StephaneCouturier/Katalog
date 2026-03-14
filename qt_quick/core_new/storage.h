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
// File Name:   storage.h
// Purpose:     Class/model for the storage device
// Description:
// Author:      Stephane Couturier
/////////////////////////////////////////////////////////////////////////////
*/

#ifndef STORAGE_H
#define STORAGE_H

#include <QStorageInfo>
#include <QSqlQuery>
#include <QSqlDatabase>
#include <QVariant>
#include <QDateTime>
#include <QDir>

class Storage
{
public:
    // Error codes for updateStorageInfo
    enum ErrorCode {
        Success = 0,
        ErrorNoPath = 1,
        ErrorEmptyDirectory = 2,
        ErrorCannotGetValues = 3,
        ErrorNotUpdated = 4
    };

    // Result structure for updateStorageInfo
    struct UpdateResult {
        bool wasUpdated;
        ErrorCode errorCode;
        qint64 newUsedSpace;
        qint64 deltaUsedSpace;
        qint64 newFreeSpace;
        qint64 deltaFreeSpace;
        qint64 newTotalSpace;
        qint64 deltaTotalSpace;
        QString errorMessage; // For detailed error information
    };

    // Data members
    int ID;
    QString name;
    QString type;
    QString path;
    QString label;
    QString fileSystem;
    qint64  totalSpace;
    qint64  freeSpace;
    QString brand;
    QString model;
    QString serialNumber;
    QString buildDate;
    QString comment1;
    QString comment2;
    QString comment3;
    QDateTime dateTimeUpdated;

    // Constructor
    Storage() : ID(0), totalSpace(0), freeSpace(0) {}

    // Methods
    void generateID();
    void insertStorage();
    void deleteStorage();
    void loadStorage(QString connectionName);

    // Updated method signature - returns structured result instead of mixed list
    UpdateResult updateStorageInfo();

private:
    // Helper methods
    bool isDirectoryEmpty(const QString &dirPath);
    bool getStorageInfo();
};

#endif // STORAGE_H
