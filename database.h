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
////////////////////////////////////////////////////////////////////////////////
*/
#ifndef DATABASE_H
#define DATABASE_H

#include <QtSql>

//CREATE TABLES -----------------------------------------------------------------

        // CATALOG --------------------------------------------------------------

            const auto SQL_CREATE_CATALOG = QLatin1String(R"(
                       CREATE TABLE IF NOT EXISTS catalog(
                            catalogFilePath             TEXT,
                            catalogName                 TEXT,
                            catalogDateUpdated          TEXT,
                            catalogSourcePath           TEXT,
                            catalogFileCount            REAL,
                            catalogTotalFileSize        REAL,
                            catalogSourcePathIsActive   REAL,
                            catalogIncludeHidden        TEXT,
                            catalogFileType             TEXT,
                            catalogStorage              TEXT,
                            catalogIncludeSymblinks     TEXT,
                            catalogIsFullDevice         TEXT,
                            catalogLoadedVersion        TEXT,
                            catalogIncludeMetadata      TEXT,
                            PRIMARY KEY("catalogName"))
            )");

        // STORAGE --------------------------------------------------------------

            const auto SQL_CREATE_STORAGE = QLatin1String(R"(
                       CREATE TABLE IF NOT EXISTS storage(
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

        // FILESALL (storing all catalogs files)---------------------------------

            const auto SQL_CREATE_FILESALL = QLatin1String(R"(
                       CREATE TABLE IF NOT EXISTS filesall(
                            id_file             INTEGER,
                            fileName            TEXT,
                            filePath            TEXT,
                            fileSize            REAL,
                            fileDateUpdated     TEXT,
                            fileCatalog         TEXT,
                            fileFullPath        TEXT,
                            PRIMARY KEY("id_file" AUTOINCREMENT))
                )");

        // FILE (one-off requests) ----------------------------------------------

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

        // FOLDER ---------------------------------------------------------------

            const auto SQL_CREATE_FOLDER = QLatin1String(R"(
                   CREATE TABLE IF NOT EXISTS  folder(
                            folderHash          TEXT,
                            folderCatalogName   TEXT,
                            folderPath          TEXT,
                            PRIMARY KEY(folderHash,folderCatalogName))
            )");

        // METADATA -------------------------------------------------------------

            const auto SQL_CREATE_METADATA = QLatin1String(R"(
                   CREATE TABLE IF NOT EXISTS  metadata(
                            catalogName         TEXT,
                            fileName            TEXT,
                            filePath            TEXT,
                            field               TEXT,
                            value               TEXT)
            )");

        // STATISTICS -----------------------------------------------------------

            const auto SQL_CREATE_STATISTICS = QLatin1String(R"(
                   CREATE TABLE IF NOT EXISTS  statistics(
                            dateTime             TEXT,
                            catalogName          TEXT,
                            catalogFileCount     REAL,
                            catalogTotalFileSize REAL,
                            recordType TEXT)
            )");

        // SEARCH ---------------------------------------------------------------

            const auto SQL_CREATE_SEARCH = QLatin1String(R"(
                   CREATE TABLE IF NOT EXISTS  search(
                            dateTime                TEXT,
                            TextChecked             INTEGER,
                            TextPhrase              TEXT,
                            TextCriteria            TEXT,
                            TextSearchIn            TEXT,
                            FileType                TEXT,
                            FileSizeChecked         INTEGER,
                            FileSizeMin             INTEGER,
                            FileSizeMinUnit         INTEGER,
                            FileSizeMax             INTEGER,
                            FileSizeMaxUnit         INTEGER,
                            DateModifiedChecked     INTEGER,
                            DateModifiedMin         TEXT,
                            DateModifiedMax         TEXT,
                            DuplicatesChecked       INTEGER,
                            DuplicatesName          INTEGER,
                            DuplicatesSize          INTEGER,
                            DuplicatesDateModified  INTEGER,
                            DifferencesChecked      INTEGER,
                            DifferencesName         INTEGER,
                            DifferencesSize         INTEGER,
                            DifferencesDateModified INTEGER,
                            DifferencesCatalogs     TEXT,
                            ShowFolders             INTEGER,
                            TagChecked              INTEGER,
                            Tag                     TEXT,
                            searchLocation          TEXT,
                            searchStorage           TEXT,
                            searchCatalog           TEXT,
                            SearchCatalogChecked    INTEGER,
                            SearchDirectoryChecked  INTEGER,
                            SeletedDirectory        TEXT,
                            TextExclude             TEXT,
                            CaseSensitive           INTEGER)
            )");

        // TAG ------------------------------------------------------------------

            const auto SQL_CREATE_TAG = QLatin1String(R"(
                       CREATE TABLE IF NOT EXISTS  tag(
                            Name		TEXT,
                            Path		TEXT,
                            Type		TEXT,
                            dateTime	TEXT)
            )");

//-------------------------------------------------------------------------------
// Database initialization ------------------------------------------------------

    QSqlError initializeDatabase(QString databaseMode, QString databaseFilePath)
    {
        QSqlDatabase db = QSqlDatabase::addDatabase("QSQLITE");

        //databaseMode = "File";
        if (databaseMode=="Memory"){
            db.setDatabaseName(":memory:");
        }
        else if (databaseMode=="File"){

            /*TEMPDEV*/databaseFilePath = "/home/stephane/Development/katalog.db";
            db.setDatabaseName(databaseFilePath);
        }

        if (!db.open())
            return db.lastError();

        QStringList tables = db.tables();

        QSqlQuery q;
        if (!q.exec(SQL_CREATE_CATALOG))
            return q.lastError();

        if (!q.exec(SQL_CREATE_STORAGE))
            return q.lastError();

        if (!q.exec(SQL_CREATE_FILESALL))
            return q.lastError();

        if (!q.exec(SQL_CREATE_FILE))
            return q.lastError();

        if (!q.exec(SQL_CREATE_FOLDER))
            return q.lastError();

        if (!q.exec(SQL_CREATE_METADATA))
            return q.lastError();

        if (!q.exec(SQL_CREATE_STATISTICS))
            return q.lastError();

        if (!q.exec(SQL_CREATE_SEARCH))
            return q.lastError();

        if (!q.exec(SQL_CREATE_TAG))
            return q.lastError();

        return QSqlError();
    }


#endif // DATABASE_H
