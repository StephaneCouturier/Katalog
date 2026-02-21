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
#include <QDateTime>
#include <QFile>
#include <functional>
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
    QString databaseType;
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
    bool loadAllCatalogFiles(std::function<bool(int filesLoaded, int totalFiles, const QString &deviceName)> progressCallback = nullptr);
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

    DeleteCatalogResult deleteCatalogFile(Device *device);

    //Exclude directory management
    bool addExcludeDirectory(const QString &path);
    bool removeExcludeDirectory(const QString &path);
    QStringList getExcludeDirectories();

    //Tag CRUD
    bool createTag(const QString &name, const QString &path, const QString &type, const QDateTime &dateTime);
    bool deleteTag(int tagID);

    //Export operations
    bool exportAllToMemoryMode(const QString &exportFolder);
    bool exportAllCatalogFiles(const QString &outputFolder,
                               std::function<bool(int current, int total, const QString &catalogName)> progressCallback = nullptr);
    bool exportSingleCatalogFoldersFile(int catalogId, const QString &filePath);

    //Data management
    bool insertPhysicalStorageGroup();
    void updateAllDeviceActive();

    enum CollectionFolderStatus {
        VALID_EMPTY,           // Empty folder - can create new collection
        VALID_MEMORY_MODE,     // Contains Memory mode files (CSV)
        VALID_FILE_MODE,       // Contains File mode auxiliary files
        INVALID_MEMORY_FILES,  // Contains Memory mode files but we're in File mode
        INVALID_FILE_FILES,    // Contains File mode files but we're in Memory mode
        INVALID_MIXED_DATA,    // Contains user/system data mixed with collection data
        INVALID_USER_DATA      // Contains user data, not a collection folder
    };

    CollectionFolderStatus validateCollectionFolder(const QString& folderPath, const QString& targetMode) const;
    QString getValidationMessage(CollectionFolderStatus status) const; // For UI messages

private:
    QString m_connectionName = "defaultConnection";

};

#endif // COLLECTION_H
