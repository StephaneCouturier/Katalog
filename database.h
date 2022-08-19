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
// File Name:   database.h
// Purpose:     database queries
// Description:
// Author:      Stephane Couturier
/////////////////////////////////////////////////////////////////////////////
*/
#ifndef DATABASE_H
#define DATABASE_H

#include <QtSql>
#include <QLocale>
#include <QMessageBox>

// Catalog -----------------------------------------------------------------

    //Create table query
            const auto SQL_CREATE_CATALOG = QLatin1String(R"(
                           CREATE TABLE IF NOT EXISTS  catalog(
                                    catalogID  int AUTO_INCREMENT primary key ,
                                    catalogFilePath         TEXT ,
                                    catalogName             TEXT  ,
                                    catalogDateUpdated      TEXT  ,
                                    catalogSourcePath       TEXT  ,
                                    catalogFileCount        REAL  ,
                                    catalogTotalFileSize    REAL ,
                                    catalogSourcePathIsActive  REAL ,
                                    catalogIncludeHidden    TEXT  ,
                                    catalogFileType         TEXT  ,
                                    catalogStorage          TEXT  ,
                                    catalogIncludeSymblinks TEXT,
                                    catalogIsFullDevice     TEXT,
                                    catalogLoadedVersion    TEXT)
            )");

    //Insert row  query
            const auto SQL_INSERT_CATALOG = QLatin1String(R"(
                            INSERT INTO catalog(
                                    catalogFilePath,
                                    catalogName,
                                    catalogDateUpdated,
                                    catalogSourcePath,
                                    catalogFileCount,
                                    catalogTotalFileSize,
                                    catalogSourcePathIsActive,
                                    catalogIncludeHidden,
                                    catalogFileType,
                                    catalogStorage,
                                    catalogIncludeSymblinks,
                                    catalogIsFullDevice,
                                    catalogLoadedVersion)
                                VALUES(?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?)
            )");

    //Insert row binding
            inline QVariant addCatalog( QSqlQuery &q,
                                        QString catalogFilePath,
                                        QString catalogName,
                                        QString catalogDateUpdated,
                                        QString catalogSourcePath,
                                        qint64 catalogFileCount,
                                        qint64 catalogTotalFileSize,
                                        int catalogSourcePathIsActive,
                                        QString catalogIncludeHidden,
                                        QString catalogFileType,
                                        QString catalogStorage,
                                        QString catalogIncludeSymblinks,
                                        QString catalogIsFullDevice,
                                        QString catalogLoadedVersion
                            )
                        {

                            q.addBindValue(catalogFilePath);
                            q.addBindValue(catalogName);
                            q.addBindValue(catalogDateUpdated);
                            q.addBindValue(catalogSourcePath);
                            q.addBindValue(catalogFileCount);
                            q.addBindValue(catalogTotalFileSize);
                            q.addBindValue(catalogSourcePathIsActive);
                            q.addBindValue(catalogIncludeHidden);
                            q.addBindValue(catalogFileType);
                            q.addBindValue(catalogStorage);
                            q.addBindValue(catalogIncludeSymblinks);
                            q.addBindValue(catalogIsFullDevice);
                            q.addBindValue(catalogLoadedVersion);
                            q.exec();
                            return 0;
                        }

// Storage --------------------------------------------------------------

    //Create table query
            const auto SQL_CREATE_STORAGE = QLatin1String(R"(
                           CREATE TABLE IF NOT EXISTS  storage(
                                storageID  int  primary key default 0,
                                storageName         TEXT,
                                storageType         TEXT,
                                storageLocation     TEXT,
                                storagePath         TEXT,
                                storageLabel        TEXT,
                                storageFileSystem   TEXT,
                                storageTotalSpace   REAL default 0,
                                storageFreeSpace    REAL default 0,
                                storageBrandModel   TEXT,
                                storageSerialNumber TEXT,
                                storageBuildDate    TEXT,
                                storageContentType  TEXT,
                                storageContainer    TEXT,
                                storageComment      TEXT)
            )");

// FILESALL (storing all catalogs files)----------------------------------------------------

        //Create table query
                const auto SQL_CREATE_FILESALL = QLatin1String(R"(
                               create  table  if not exists  filesall(
                                        id_file             INTEGER,
                                        fileName            TEXT,
                                        filePath            TEXT,
                                        fileSize            REAL,
                                        fileDateUpdated     TEXT,
                                        fileCatalog         TEXT,
                                        fileFullPath        TEXT,
                                        PRIMARY KEY("id_file" AUTOINCREMENT))
                )");

// FILE (one-off requests) ----------------------------------------------------

        //Create table query
                const auto SQL_CREATE_FILE = QLatin1String(R"(
                               CREATE TABLE IF NOT EXISTS  file(
                                        id_file             INTEGER,
                                        fileName            TEXT,
                                        filePath            TEXT,
                                        fileSize            REAL,
                                        fileDateUpdated     TEXT,
                                        fileCatalog         TEXT,
                                        fileFullPath        TEXT,
                                        PRIMARY KEY("id_file" AUTOINCREMENT))
                )");


// FOLDER ---------------------------------------------------------------------

                //Create table query
                        const auto SQL_CREATE_FOLDER = QLatin1String(R"(
                                       CREATE TABLE IF NOT EXISTS  folder(
                                                folderHash          TEXT,
                                                folderCatalogName   TEXT,
                                                folderPath          TEXT)
                        )");


// STATISTICS ------------------------------------------------

        //Create table query
                const auto SQL_CREATE_STATISTICS = QLatin1String(R"(
                               CREATE TABLE IF NOT EXISTS  statistics(
                                        dateTime             TEXT,
                                        catalogName          TEXT,
                                        catalogFileCount     REAL,
                                        catalogTotalFileSize REAL,
                                        recordType TEXT)
                )");


// SEARCH ----------------------------------------------------

        //Create table query
                const auto SQL_CREATE_SEARCH = QLatin1String(R"(
                               CREATE TABLE IF NOT EXISTS  search(
                                        dateTime            TEXT,
                                        TextChecked         INTEGER,
                                        TextPhrase          TEXT,
                                        TextCriteria        TEXT,
                                        TextSearchIn        TEXT,
                                        FileType            TEXT,
                                        FileSizeChecked     INTEGER,
                                        FileSizeMin         INTEGER,
                                        FileSizeMinUnit     INTEGER,
                                        FileSizeMax         INTEGER,
                                        FileSizeMaxUnit     INTEGER,
                                        DateModifiedChecked	INTEGER,
                                        DateModifiedMin     TEXT,
                                        DateModifiedMax     TEXT,
                                        DuplicatesChecked	INTEGER,
                                        DuplicatesName      INTEGER,
                                        DuplicatesSize      INTEGER,
                                        DuplicatesDateModified   INTEGER,
                                        DifferencesChecked	INTEGER,
                                        DifferencesName     INTEGER,
                                        DifferencesSize     INTEGER,
                                        DifferencesDateModified  INTEGER,
                                        DifferencesCatalogs TEXT,
                                        ShowFolders         INTEGER,
                                        TagChecked          INTEGER,
                                        Tag                 TEXT,
                                        searchLocation      TEXT,
                                        searchStorage       TEXT,
                                        searchCatalog       TEXT,
                                        SearchCatalogChecked    INTEGER,
                                        SearchDirectoryChecked  INTEGER,
                                        SeletedDirectory    TEXT,
                                        TextExclude         TEXT,
                                        CaseSensitive       INTEGER)
                )");

// TAG ----------------------------------------------------

        //Create table query
                const auto SQL_CREATE_TAG = QLatin1String(R"(
                               CREATE TABLE IF NOT EXISTS  tag(
                                        Name		TEXT,
                                        Path		TEXT,
                                        Type		TEXT,
                                        dateTime	TEXT)
                )");

// STATISTICS ----------------------------------------------------

        //Create table query
                const auto SQL_CREATE_CALENDAR = QLatin1String(R"(
                                CREATE TABLE IF NOT EXISTS calendar (
                                  d date UNIQUE NOT NULL,
                                  dayofweek INT NOT NULL,
                                  weekday TEXT NOT NULL,
                                  quarter INT NOT NULL,
                                  year INT NOT NULL,
                                  month INT NOT NULL,
                                  day INT NOT NULL)
                )");

// Database initialization ------------------------------------------------

QSqlError initializeDatabase()
{
    QSqlDatabase db = QSqlDatabase::addDatabase("QSQLITE");
    db.setDatabaseName(":memory:");
    //db.setDatabaseName("/home/stephane/Development/katalog.db");

    if (!db.open())
        return db.lastError();

    QStringList tables = db.tables();

    QSqlQuery q;
    if (!q.exec(SQL_CREATE_STORAGE))
        return q.lastError();

    if (!q.exec(SQL_CREATE_FILE))
        return q.lastError();

    if (!q.exec(SQL_CREATE_FILESALL))
        return q.lastError();

    if (!q.exec(SQL_CREATE_FOLDER))
        return q.lastError();

    if (!q.exec(SQL_CREATE_STATISTICS))
        return q.lastError();

    if (!q.exec(SQL_CREATE_SEARCH))
        return q.lastError();

    if (!q.exec(SQL_CREATE_TAG))
        return q.lastError();

    if (!q.exec(SQL_CREATE_CALENDAR))
        return q.lastError();

    return QSqlError();
}

#endif // DATABASE_H
