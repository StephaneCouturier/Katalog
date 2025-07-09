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
// File Name:   database.h
// Purpose:     database queries to create tables
// Description:
// Author:      Stephane Couturier
////////////////////////////////////////////////////////////////////////////////
*/
#ifndef DATABASE_H
#define DATABASE_H

#include <QtSql>
#include <QSqlError>
#include <QSqlQuery>
#include <QSqlDatabase>
#include <QString>

// Forward declaration
class Collection;

namespace DatabaseSQL {
// DEVICE ---------------------------------------------------------------
const auto SQL_CREATE_DEVICE = QLatin1String(R"(
        CREATE TABLE IF NOT EXISTS device(
                        device_id                 INTEGER,
                        device_parent_id          INTEGER,
                        device_name               TEXT,
                        device_type               TEXT,
                        device_external_id        INTEGER,
                        device_path               TEXT,
                        device_total_file_size    INTEGER,
                        device_total_file_count   INTEGER,
                        device_total_space        INTEGER,
                        device_free_space         INTEGER,
                        device_active             INTEGER,
                        device_group_id           INTEGER,
                        device_date_updated       TEXT,
                        device_order              INTEGER)
    )");

// STORAGE --------------------------------------------------------------
const auto SQL_CREATE_STORAGE = QLatin1String(R"(
        CREATE TABLE IF NOT EXISTS storage(
                        storage_id            INTEGER,
                        storage_name          TEXT,
                        storage_type          TEXT,
                        storage_location      TEXT,
                        storage_path          TEXT,
                        storage_label         TEXT,
                        storage_file_system   TEXT,
                        storage_total_space   NUMERIC,
                        storage_free_space    NUMERIC,
                        storage_brand         TEXT,
                        storage_model         TEXT,
                        storage_serial_number TEXT,
                        storage_build_date    TEXT,
                        storage_comment1      TEXT,
                        storage_comment2      TEXT,
                        storage_comment3      TEXT)
    )");

// CATALOG --------------------------------------------------------------
const auto SQL_CREATE_CATALOG = QLatin1String(R"(
        CREATE TABLE IF NOT EXISTS catalog(
                        catalog_id                INTEGER,
                        catalog_name              TEXT,
                        catalog_source_path       TEXT,
                        catalog_include_hidden    INTEGER,
                        catalog_file_type         TEXT,
                        catalog_storage           TEXT,
                        catalog_include_symlinks  INTEGER,
                        catalog_include_metadata  INTEGER,
                        catalog_is_full_device    INTEGER,
                        catalog_app_version       TEXT,
                        catalog_file_count        INTEGER,
                        catalog_total_file_size   INTEGER,
                        catalog_date_loaded       TEXT,
                        catalog_date_updated      TEXT)
    )");

// FILE -----------------------------------------------------------------
const auto SQL_CREATE_FILE = QLatin1String(R"(
        CREATE TABLE IF NOT EXISTS file(
                        file_catalog_id   NUMERIC,
                        file_name         TEXT,
                        file_folder_path  TEXT,
                        file_size         NUMERIC,
                        file_date_updated TEXT,
                        file_catalog      TEXT,
                        file_full_path    TEXT)
    )");

// FILETEMP (one-off requests) ------------------------------------------
const auto SQL_CREATE_FILETEMP = QLatin1String(R"(
        CREATE TABLE IF NOT EXISTS filetemp(
                        file_catalog_id   NUMERIC,
                        file_name         TEXT,
                        file_folder_path  TEXT,
                        file_size         NUMERIC,
                        file_date_updated TEXT,
                        file_catalog      TEXT,
                        file_full_path    TEXT)
    )");

// FOLDER ---------------------------------------------------------------
const auto SQL_CREATE_FOLDER = QLatin1String(R"(
        CREATE TABLE IF NOT EXISTS folder(
                        folder_catalog_id  NUMERIC,
                        folder_path        TEXT,
                        PRIMARY KEY(folder_catalog_id,folder_path))
    )");

// METADATA -------------------------------------------------------------
const auto SQL_CREATE_METADATA = QLatin1String(R"(
        CREATE TABLE IF NOT EXISTS metadata(
                        catalog_name        TEXT,
                        file_name           TEXT,
                        file_path           TEXT,
                        field               TEXT,
                        value               TEXT)
    )");

// STATISTICS -----------------------------------------------------------
const auto SQL_CREATE_STATISTICS_DEVICE = QLatin1String(R"(
        CREATE TABLE IF NOT EXISTS statistics_device(
                        date_time               TEXT,
                        device_id               TEXT,
                        device_name             TEXT,
                        device_type             TEXT,
                        device_file_count       NUMERIC,
                        device_total_file_size  NUMERIC,
                        device_total_space      NUMERIC,
                        device_free_space       NUMERIC,
                        record_type             TEXT)
    )");

// SEARCH ---------------------------------------------------------------
const auto SQL_CREATE_SEARCH = QLatin1String(R"(
        CREATE TABLE IF NOT EXISTS search(
                        date_time                 TEXT,
                        text_search               TEXT,
                        text_criteria             TEXT,
                        text_search_in            TEXT,
                        file_type                 TEXT,
                        file_size_type            TEXT,
                        file_size_value           TEXT,
                        folder_criteria           TEXT,
                        search_catalog            TEXT,
                        search_catalog_checked    NUMERIC,
                        search_directory_checked  NUMERIC,
                        selected_directory        TEXT,
                        selected_device_ID_list   TEXT,
                        text_exclude              TEXT,
                        case_sensitive            NUMERIC)
    )");

// TAG ------------------------------------------------------------------
const auto SQL_CREATE_TAG = QLatin1String(R"(
        CREATE TABLE IF NOT EXISTS tag(
                        ID          INTEGER PRIMARY KEY AUTOINCREMENT,
                        name		TEXT,
                        path		TEXT,
                        type		TEXT,
                        date_time	TEXT)
    )");

// FILE TAG -------------------------------------------------------------
const auto SQL_CREATE_FILE_TAG = QLatin1String(R"(
        CREATE TABLE IF NOT EXISTS file_tag(
                        file_id     INTEGER,
                        tag_id      INTEGER,
                        PRIMARY KEY(file_id, tag_id))
    )");

// PARAMETER ------------------------------------------------------------
const auto SQL_CREATE_PARAMETER = QLatin1String(R"(
        CREATE TABLE IF NOT EXISTS parameter(
                        parameter_name      TEXT,
                        parameter_type      TEXT,
                        parameter_value1    TEXT,
                        parameter_value2    TEXT)
    )");

// SCHEMA ---------------------------------------------------------------
const auto SQL_CREATE_SCHEMA = QLatin1String(R"(
        CREATE TABLE IF NOT EXISTS schema_version(
                        version     TEXT)
    )");

// BACKUP MAPPING -------------------------------------------------------
const auto SQL_CREATE_BACKUP_MAPPING = QLatin1String(R"(
        CREATE TABLE IF NOT EXISTS device_mapping(
                        mapping_id                  INTEGER PRIMARY KEY AUTOINCREMENT,
                        mapping_name                TEXT,
                        mapping_type                TEXT,
                        mapping_device_source_id    NUMERIC,
                        mapping_device_target_id    NUMERIC,
                        mapping_backup_last_date    TEXT,
                        mapping_backup_last_size    TEXT)
    )");
}

class Database
{
public:
    // Main initialization method - creates connection and sets up database
    static QSqlError initialize(const QString &connectionName, Collection *collection);

    // Create all database tables
    static QSqlError createAllTables(const QString &connectionName);

    // Individual table creation methods
    static QSqlError createDeviceTable(const QString &connectionName);
    static QSqlError createStorageTable(const QString &connectionName);
    static QSqlError createCatalogTable(const QString &connectionName);
    static QSqlError createFileTable(const QString &connectionName);
    static QSqlError createFileTempTable(const QString &connectionName);
    static QSqlError createFolderTable(const QString &connectionName);
    static QSqlError createMetadataTable(const QString &connectionName);
    static QSqlError createStatisticsTable(const QString &connectionName);
    static QSqlError createSearchTable(const QString &connectionName);
    static QSqlError createTagTable(const QString &connectionName);
    static QSqlError createFileTagTable(const QString &connectionName);
    static QSqlError createParameterTable(const QString &connectionName);
    static QSqlError createSchemaTable(const QString &connectionName);
    static QSqlError createBackupMappingTable(const QString &connectionName);

    // Utility methods
    static bool tableExists(const QString &connectionName, const QString &tableName);
    static QStringList listTables(const QString &connectionName);

private:
    // Helper method to execute SQL with error checking
    static QSqlError executeSql(const QString &connectionName, const QString &sql);
};

#endif // DATABASE_H
