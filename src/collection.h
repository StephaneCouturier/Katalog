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
#include <QMessageBox>
#include <QCoreApplication>
#include <QRegularExpression>

class Collection
{

public:

    //Main attributes
    QString appVersion;
    QString version;
    QString folder;
    QString settingsFilePath;

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
    QString tagsFilePath;
    void generateCollectionFilesPaths();
    void generateCollectionFiles();

    //File loading
    void loadAllCatalogFiles();
    void loadDeviceFileToTable();
    void loadCatalogFilesToTable();
    void loadStorageFileToTable();
    void loadStatisticsDeviceFileToTable();
    void loadParameters();

    //File saving
    void saveDeviceTableToFile();
    void saveStorageTableToFile();
    void saveStatiticsToFile();
    void createStorageFile();
    void saveParametersToFile();

    //File deleting
    void deleteCatalogFile(Device *device);

    //Data management
    void insertPhysicalStorageGroup();

};

#endif // COLLECTION_H
