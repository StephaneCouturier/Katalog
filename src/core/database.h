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
// Purpose:     Database management
// Description: Handles connection, creation, and updates of the database
// Author:      Stephane Couturier
/////////////////////////////////////////////////////////////////////////////
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
// DEVICE  --------------------------------------------------------------

const auto SQL_CREATE_DEVICE = QLatin1String(R"(
                        CREATE TABLE IF NOT EXISTS device(
                            device_id                  NUMERIC PRIMARY KEY,
                            device_parent_id           NUMERIC,
                            device_name                TEXT,
                            device_type                TEXT,
                            device_external_id         NUMERIC,
                            device_path                TEXT,
                            device_total_file_size     NUMERIC default 0,
                            device_total_file_count    NUMERIC default 0,
                            device_total_space         NUMERIC default 0,
                            device_free_space          NUMERIC default 0,
                            device_active              NUMERIC,
                            device_group_id            NUMERIC,
                            device_date_updated        TEXT,
                            device_order               NUMERIC)
            )");

// CATALOG --------------------------------------------------------------

const auto SQL_CREATE_CATALOG = QLatin1String(R"(
                        CREATE TABLE IF NOT EXISTS catalog(
                            catalog_id                    NUMERIC,
                            catalog_file_path             TEXT,
                            catalog_name                  TEXT,
                            catalog_date_updated          TEXT,
                            catalog_source_path           TEXT,
                            catalog_file_count            NUMERIC default 0,
                            catalog_total_file_size       NUMERIC default 0,
                            catalog_source_path_is_active NUMERIC,
                            catalog_include_hidden        TEXT,
                            catalog_file_type             TEXT,
                            catalog_storage               TEXT,
                            catalog_include_symblinks     TEXT,
                            catalog_is_full_device        TEXT,
                            catalog_date_loaded           TEXT,
                            catalog_include_metadata      TEXT,
                            catalog_include_checksum      TEXT,
                            catalog_app_version           TEXT,
                            PRIMARY KEY(catalog_name))
            )");

// STORAGE --------------------------------------------------------------

const auto SQL_CREATE_STORAGE = QLatin1String(R"(
                        CREATE TABLE IF NOT EXISTS storage(
                            storage_id            NUMERIC  primary key default 0,
                            storage_name          TEXT,
                            storage_type          TEXT,
                            storage_location      TEXT,
                            storage_path          TEXT,
                            storage_label         TEXT,
                            storage_file_system   TEXT,
                            storage_total_space   NUMERIC default 0,
                            storage_free_space    NUMERIC default 0,
                            storage_brand         TEXT,
                            storage_model         TEXT,
                            storage_serial_number TEXT,
                            storage_build_date    TEXT,
                            storage_comment1      TEXT,
                            storage_comment2      TEXT,
                            storage_comment3      TEXT)
            )");

// FILE (storing all catalogs files)-------------------------------------

const auto SQL_CREATE_FILE = QLatin1String(R"(
                        CREATE TABLE IF NOT EXISTS file(
                            file_catalog_id   NUMERIC,
                            file_name         TEXT,     -- "home.jpg"
                            file_folder_path  TEXT,     -- "/home/user/photos"
                            file_size         NUMERIC,
                            file_date_updated TEXT,
                            file_catalog      TEXT,
                            file_full_path    TEXT,     -- "/home/user/photos/home.jpg"
                            file_extension    TEXT,     -- "jpg"
                            file_type               TEXT,
                            mime_type               TEXT,
                            mime_verified           NUMERIC,
                            type_mismatch           NUMERIC,
                            image_width             NUMERIC,
                            image_height            NUMERIC,
                            image_orientation       NUMERIC,
                            video_duration_seconds  NUMERIC,
                            video_width             NUMERIC,
                            video_height            NUMERIC,
                            video_codec             TEXT,
                            video_framerate         NUMERIC,
                            video_bitrate           NUMERIC,
                            audio_duration_seconds  NUMERIC,
                            audio_artist            TEXT,
                            audio_album             TEXT,
                            audio_title             TEXT,
                            audio_genre             TEXT,
                            audio_year              NUMERIC,
                            audio_track_number      NUMERIC,
                            audio_bitrate           NUMERIC,
                            audio_sample_rate       NUMERIC,
                            metadata_extended       TEXT,    -- JSON for additional fields
                            metadata_extraction_date TEXT,
                            checksum_sha256          TEXT,
                            checksum_extraction_date TEXT
                )
            )");

// FILETEMP (one-off requests) ------------------------------------------

const auto SQL_CREATE_FILETEMP = QLatin1String(R"(
                        CREATE TABLE IF NOT EXISTS  filetemp(
                            file_catalog_id   NUMERIC,
                            file_name         TEXT,     -- "home.jpg"
                            file_folder_path  TEXT,     -- "/home/user/photos"
                            file_size         NUMERIC,
                            file_date_updated TEXT,
                            file_catalog      TEXT,
                            file_full_path    TEXT,     -- "/home/user/photos/home.jpg"
                            file_extension    TEXT,      -- "jpg"
                            file_type               TEXT,
                            mime_type               TEXT,
                            mime_verified           NUMERIC,
                            type_mismatch           NUMERIC,
                            image_width             NUMERIC,
                            image_height            NUMERIC,
                            image_orientation       NUMERIC,
                            video_duration_seconds  NUMERIC,
                            video_width             NUMERIC,
                            video_height            NUMERIC,
                            video_codec             TEXT,
                            video_framerate         NUMERIC,
                            video_bitrate           NUMERIC,
                            audio_duration_seconds  NUMERIC,
                            audio_artist            TEXT,
                            audio_album             TEXT,
                            audio_title             TEXT,
                            audio_genre             TEXT,
                            audio_year              NUMERIC,
                            audio_track_number      NUMERIC,
                            audio_bitrate           NUMERIC,
                            audio_sample_rate       NUMERIC,
                            metadata_extended       TEXT,    -- JSON for additional fields
                            metadata_extraction_date TEXT,
                            checksum_sha256          TEXT,
                            checksum_extraction_date TEXT
                )
            )");

// FOLDER ---------------------------------------------------------------

const auto SQL_CREATE_FOLDER = QLatin1String(R"(
                        CREATE TABLE IF NOT EXISTS  folder(
                            folder_catalog_id  NUMERIC,
                            folder_path        TEXT,
                            PRIMARY KEY(folder_catalog_id,folder_path))
            )");

// STATISTICS -----------------------------------------------------------

const auto SQL_CREATE_STATISTICS_DEVICE = QLatin1String(R"(
                        CREATE TABLE IF NOT EXISTS  statistics_device(
                            date_time               TEXT,
                            device_id               TEXT,
                            device_name             TEXT,
                            device_type             TEXT,
                            device_file_count       NUMERIC,
                            device_total_file_size  NUMERIC,
                            device_free_space       NUMERIC,
                            device_total_space      NUMERIC,
                            record_type             TEXT)
            )");

// SEARCH ---------------------------------------------------------------

const auto SQL_CREATE_SEARCH = QLatin1String(R"(
                        CREATE TABLE IF NOT EXISTS  search(
                            date_time                 TEXT,
                            text_checked              NUMERIC,
                            text_phrase               TEXT,
                            text_criteria             TEXT,
                            text_search_in            TEXT,
                            file_criteria_checked     NUMERIC,
                            file_type_checked         NUMERIC,
                            file_type                 TEXT,
                            file_size_checked         NUMERIC,
                            file_size_min             NUMERIC,
                            file_size_min_unit        NUMERIC,
                            file_size_max             NUMERIC,
                            file_size_max_unit        NUMERIC,
                            date_modified_checked     NUMERIC,
                            date_modified_min         TEXT,
                            date_modified_max         TEXT,
                            duplicates_checked        NUMERIC,
                            duplicates_name           NUMERIC,
                            duplicates_size           NUMERIC,
                            duplicates_date_modified  NUMERIC,
                            differences_checked       NUMERIC,
                            differences_name          NUMERIC,
                            differences_size          NUMERIC,
                            differences_date_modified NUMERIC,
                            differences_catalogs      TEXT,
                            folder_criteria_checked   NUMERIC,
                            show_folders              NUMERIC,
                            tag_checked               NUMERIC,
                            tag                       TEXT,
                            search_location           TEXT,
                            search_storage            TEXT,
                            search_catalog            TEXT,
                            search_catalog_checked    NUMERIC,
                            search_directory_checked  NUMERIC,
                            selected_directory        TEXT,
                            selected_device_ID_list   TEXT,
                            text_exclude              TEXT,
                            case_sensitive            NUMERIC,
                            metadata_checked          NUMERIC,
                            metadata_text_checked     NUMERIC,
                            metadata_text_search      TEXT,
                            metadata_size_checked     NUMERIC,
                            metadata_size_min_height  NUMERIC,
                            metadata_size_max_height  NUMERIC,
                            metadata_size_min_width   NUMERIC,
                            metadata_size_max_width   NUMERIC,
                            metadata_duration_checked NUMERIC,
                            metadata_duration_min     TEXT,
                            metadata_duration_max     TEXT)
            )");

// TAG ------------------------------------------------------------------

const auto SQL_CREATE_TAG = QLatin1String(R"(
                        CREATE TABLE IF NOT EXISTS  tag(
                            ID          INTEGER PRIMARY KEY AUTOINCREMENT,
                            name		TEXT,
                            path		TEXT,
                            type		TEXT,
                            date_time	TEXT)
            )");

// EXCLUDE --------------------------------------------------------------
const auto SQL_CREATE_PARAMETER = QLatin1String(R"(
                        CREATE TABLE IF NOT EXISTS parameter(
                                parameter_name      TEXT,
                                parameter_type      TEXT,
                                parameter_value1    TEXT,
                                parameter_value2    TEXT)
            )");

// BACKUP MAPPING -------------------------------------------------------

const auto SQL_CREATE_BACKUP_MAPPING = QLatin1String(R"(
                        CREATE TABLE IF NOT EXISTS  device_mapping(
                            mapping_id                  INTEGER PRIMARY KEY AUTOINCREMENT,
                            mapping_name                TEXT,
                            mapping_type                TEXT,
                            mapping_device_source_id    NUMERIC,
                            mapping_device_target_id    NUMERIC,
                            mapping_backup_last_date    TEXT,
                            mapping_backup_last_size    TEXT)
            )");


// MIGRATION 1.22 to 2.0 ------------------------------------------------

// STATISTICS -----------------------------------------------------------

const auto SQL_CREATE_STATISTICS_CATALOG = QLatin1String(R"(
                        CREATE TABLE IF NOT EXISTS  statistics_catalog(
                            date_time               TEXT,
                            catalog_id              NUMERIC,
                            catalog_name            TEXT,
                            catalog_file_count      NUMERIC,
                            catalog_total_file_size NUMERIC,
                            record_type             TEXT)
            )");

const auto SQL_CREATE_STATISTICS_STORAGE = QLatin1String(R"(
                        CREATE TABLE IF NOT EXISTS  statistics_storage(
                            date_time               TEXT,
                            storage_id              TEXT,
                            storage_name            TEXT,
                            storage_free_space      NUMERIC,
                            storage_total_space     NUMERIC,
                            record_type             TEXT)
            )");

// VIRTUALSTORAGE  ------------------------------------------------------

const auto SQL_CREATE_VIRTUAL_STORAGE = QLatin1String(R"(
                       CREATE TABLE IF NOT EXISTS virtual_storage(
                            virtual_storage_id          NUMERIC,
                            virtual_storage_parent_id   NUMERIC,
                            virtual_storage_name        TEXT)
            )");

// VIRTUALSTORAGE CATALOG ------------------------------------------------------

const auto SQL_CREATE_VIRTUAL_STORAGE_CATALOG = QLatin1String(R"(
                       CREATE TABLE IF NOT EXISTS virtual_storage_catalog(
                            virtual_storage_id      NUMERIC,
                            catalog_name            TEXT,
                            directory_path          TEXT)
            )");

// DEVICE CATALOG ------------------------------------------------------

const auto SQL_CREATE_DEVICE_CATALOG = QLatin1String(R"(
                        CREATE TABLE IF NOT EXISTS device_catalog(
                            device_id           NUMERIC,
                            catalog_name        TEXT,
                            directory_path      TEXT)
            )");

}

class Database
{
public:

    enum class DatabaseType {
        SQLite,
        MySQL,
        PostgreSQL
    };
    static DatabaseType getDatabaseType(const QString &connectionName);

    // Get CREATE statements for compatibility with different database systems
    /**
     * @brief Generate CREATE TABLE SQL for device_mapping based on database type
     * @param dbType The type of database (SQLite, MySQL, PostgreSQL)
     * @return SQL string for creating device_mapping table
     */
    static QString getCreateBackupMappingSQL(DatabaseType dbType);

    /**
     * @brief Generate CREATE TABLE SQL for tag based on database type
     * @param dbType The type of database (SQLite, MySQL, PostgreSQL)
     * @return SQL string for creating tag table
     */
    static QString getCreateTagSQL(DatabaseType dbType);
    /**
     * @brief Generate CREATE TABLE SQL for catalog based on database type
     * @param dbType The type of database (SQLite, MySQL, PostgreSQL)
     * @return SQL string for creating catalog table
     */
    static QString getCreateCatalogSQL(DatabaseType dbType);

    /**
     * @brief Generate CREATE TABLE SQL for folder based on database type
     * @param dbType The type of database (SQLite, MySQL, PostgreSQL)
     * @return SQL string for creating folder table
     */
    static QString getCreateFolderSQL(DatabaseType dbType);

    /**
     * @brief Get database-specific SQL for starting a transaction
     * @param dbType The type of database
     * @return SQL string to begin transaction
     */
    static QString getBeginTransactionSQL(DatabaseType dbType);

    /**
     * @brief Get database-specific SQL for INSERT OR IGNORE
     * @param dbType The type of database
     * @return SQL prefix for insert or ignore syntax
     */
    static QString getInsertOrIgnorePrefix(DatabaseType dbType);

    /**
     * @brief Execute BEGIN TRANSACTION with correct syntax for database type
     * @param connectionName Database connection name
     * @return true if successful, false otherwise
     */
    static bool beginTransaction(const QString &connectionName);

    /**
     * @brief Execute COMMIT with error handling
     * @param connectionName Database connection name
     * @return true if successful, false otherwise
     */
    static bool commitTransaction(const QString &connectionName);

    /**
     * @brief Execute ROLLBACK with error handling
     * @param connectionName Database connection name
     * @return true if successful, false otherwise
     */
    static bool rollbackTransaction(const QString &connectionName);

    /**
     * @brief Format time difference for backup mapping display
     * @param dbType Database type
     * @param d1DateField Source device date field name
     * @param d2DateField Target device date field name
     * @return SQL expression for formatted time difference
     */
    static QString getFormattedTimeDifference(DatabaseType dbType,
                                              const QString &d1DateField,
                                              const QString &d2DateField);

    /**
     * @brief search table has file_size_min_unit defined as NUMERIC, but the code is inserting text
     * @param the code is inserting text values like "Bytes", "KB", "MB", etc.
     * @param SQLite allows this (it's flexible with types), but MySQL/MariaDB enforces strict typing.
     * @return Temporary fix, db defintion should be fixed to TEXT
     */
    static QString getCreateSearchSQL(DatabaseType databaseType);

    // Main initialization method - creates connection and sets up database
    // Optional overrides allow command line to bypass settings file values
    static QSqlError initialize(const QString &connectionName, Collection *collection,
                                const QString &overrideDatabaseMode = QString(),
                                const QString &overrideDatabaseFilePath = QString());
    // Create all database tables
    static QSqlError createAllTables(const QString &connectionName);

    // Utility methods
    static bool tableExists(const QString &connectionName, const QString &tableName);
    static QStringList listTables(const QString &connectionName); // Make DB-aware

    // Updates per Version
    static QSqlError runMigration_2_6(const QString &connectionName);
    static QSqlError runMigration_2_8(const QString &connectionName);
    static QSqlError runMigration_2_9(const QString &connectionName);

private:
    // Helper method to execute SQL with error checking
    static QSqlError executeSql(const QString &connectionName, const QString &sql);
    // Updates
    static QSqlError dropTableIfExists(const QString &connectionName, const QString &tableName);
    static QStringList getTableColumns(const QString &connectionName, const QString &tableName);

};

#endif // DATABASE_H
