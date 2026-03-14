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
// File Name:   collection.h
// Purpose:     Class/model for the collection (all contents including devices, catalogs, files)
// Description:
// Author:      Stephane Couturier
/////////////////////////////////////////////////////////////////////////////
*/

#ifndef COLLECTION_H
#define COLLECTION_H

#include "device.h"
#include <QAbstractTableModel>
#include <QFile>
#include <QSqlQuery>
#include <QSqlError>
#include <QSqlRecord>
#include <QRegularExpression>

class Collection : public QObject
{
    Q_OBJECT

public:
    explicit Collection(QObject *parent = nullptr);

    //Main attributes
    QString appVersion; //current version of the app
    QString dbSchemaVersion;
    QString folder;
    QString settingsFilePath;

    QString loadDatabaseSchemaVersion();
    void setDatabaseSchemaVersion();

    //Database management
    QString databaseMode;
    QString databaseHostName;
    QString databaseName;
    int     databasePort;
    QString databaseUserName;
    QString databasePassword;
    QString databaseFilePath;

    //File paths
    QString deviceFilePath;
    QString searchHistoryFilePath;
    QString storageFilePath;
    QString deviceCatalogFilePath;
    QString statisticsCatalogFileName;
    QString statisticsCatalogFilePath;
    QString statisticsStorageFileName;
    QString statisticsStorageFilePath;
    QString statisticsDeviceFileName;
    QString statisticsDeviceFilePath;
    QString parameterFilePath;
    QString tagFilePath;
    QString mappingFilePath;
    void generateCollectionFilesPaths();
    void generateCollectionFiles();

    //File loading
    bool load();
    void clearDatabaseData();
    void loadAllCatalogFiles();
    void loadDeviceFileToTable();
    void loadCatalogFilesToTable();
    void loadStorageFileToTable();
    void loadStatisticsDeviceFileToTable();
    void loadParameterFileToTable();
    void loadSearchHistoryFileToTable();
    void loadTagFileToTable();
    void loadMappingFileToTable();

    //File saving
    void saveDeviceTableToFile();
    void saveStorageTableToFile();
    void saveStatiticsTableToFile();
    void saveParameterTableToFile();
    void saveSearchHistoryTableToFile();
    void saveTagTableToFile();
    void saveMappingTableToFile();

    //File deleting
    enum DeleteCatalogResult {
        DeleteSuccess = 0,
        DeleteFailedToMoveToTrash = 1,
        DeleteInvalidPath = 2
    };
    //QString deleteCatalogFile(Device *device);
    DeleteCatalogResult deleteCatalogFile(Device *device);

    //Data management
    bool insertPhysicalStorageGroup();

};

#endif // COLLECTION_H
