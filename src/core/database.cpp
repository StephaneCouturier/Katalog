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
// File Name:   database.cpp
// Purpose:     Database management
// Description: Handles connection, creation, and updates of the database
// Author:      Stephane Couturier
/////////////////////////////////////////////////////////////////////////////
*/
#include "database.h"
#include "collection.h"
#include <QSettings>
#include <QFile>
#include <QDebug>
#include <QApplication>

//----------------------------------------------------------------------
// Core database functions
//----------------------------------------------------------------------
Database::DatabaseType Database::getDatabaseType(const QString &connectionName)
{
    QSqlDatabase db = QSqlDatabase::database(connectionName);
    QString driver = db.driverName();

    if (driver == "QSQLITE") return DatabaseType::SQLite;
    if (driver == "QMYSQL" || driver == "QMYSQL3") return DatabaseType::MySQL;
    if (driver == "QPSQL") return DatabaseType::PostgreSQL;

    return DatabaseType::SQLite; // default fallback
}

QString Database::getCreateBackupMappingSQL(DatabaseType databaseType)
{
    QString autoIncrementSyntax;

    switch (databaseType) {
        case DatabaseType::SQLite:
            autoIncrementSyntax = "INTEGER PRIMARY KEY AUTOINCREMENT";
            break;
        case DatabaseType::MySQL:
            autoIncrementSyntax = "INT AUTO_INCREMENT PRIMARY KEY";
            break;
        case DatabaseType::PostgreSQL:
            autoIncrementSyntax = "SERIAL PRIMARY KEY";
            break;
    }

    return QString(R"(
                CREATE TABLE IF NOT EXISTS device_mapping(
                    mapping_id                  %1,
                    mapping_name                TEXT,
                    mapping_type                TEXT,
                    mapping_device_source_id    NUMERIC,
                    mapping_device_target_id    NUMERIC,
                    mapping_backup_last_date    TEXT,
                    mapping_backup_last_size    TEXT)
    )").arg(autoIncrementSyntax);
}

QString Database::getCreateTagSQL(DatabaseType databaseType)
{
    QString autoIncrementSyntax;

    switch (databaseType) {
        case DatabaseType::SQLite:
            autoIncrementSyntax = "INTEGER PRIMARY KEY AUTOINCREMENT";
            break;
        case DatabaseType::MySQL:
            autoIncrementSyntax = "INT AUTO_INCREMENT PRIMARY KEY";
            break;
        case DatabaseType::PostgreSQL:
            autoIncrementSyntax = "SERIAL PRIMARY KEY";
            break;
    }

    return QString(R"(
                CREATE TABLE IF NOT EXISTS tag(
                    ID          %1,
                    name        TEXT,
                    path        TEXT,
                    type        TEXT,
                    date_time   TEXT)
    )").arg(autoIncrementSyntax);
}

QString Database::getCreateCatalogSQL(DatabaseType databaseType)
{
    // For MySQL/PostgreSQL, use VARCHAR with specific length for PRIMARY KEY
    // SQLite allows TEXT in PRIMARY KEY
    QString catalogNameType;

    switch (databaseType) {
        case DatabaseType::SQLite:
            catalogNameType = "TEXT";
            break;
        case DatabaseType::MySQL:
            catalogNameType = "VARCHAR(500)";
            break;
        case DatabaseType::PostgreSQL:
            catalogNameType = "VARCHAR(500)";
            break;
    }

    return QString(R"(
                CREATE TABLE IF NOT EXISTS catalog(
                    catalog_id                    NUMERIC,
                    catalog_file_path             TEXT,
                    catalog_name                  %1,
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
                    catalog_app_version           TEXT,
                    PRIMARY KEY(catalog_name))
    )").arg(catalogNameType);
}

QString Database::getCreateFolderSQL(DatabaseType databaseType)
{
    switch (databaseType) {
    case DatabaseType::SQLite:
        return R"(
                CREATE TABLE IF NOT EXISTS folder(
                    folder_catalog_id  NUMERIC,
                    folder_path        TEXT,
                    PRIMARY KEY(folder_catalog_id, folder_path))
            )";

    case DatabaseType::MySQL:
    case DatabaseType::PostgreSQL:
        // Use TEXT without PRIMARY KEY, add UNIQUE constraint instead
        // This avoids VARCHAR length limitations while preventing duplicates
        return R"(
                CREATE TABLE IF NOT EXISTS folder(
                    folder_catalog_id  NUMERIC,
                    folder_path        TEXT,
                    UNIQUE KEY unique_folder (folder_catalog_id, folder_path(700)))
            )";
    }
    return ""; // Should never reach here
}

QString Database::getBeginTransactionSQL(DatabaseType databaseType)
{
    switch (databaseType) {
    case DatabaseType::SQLite:
        return "BEGIN TRANSACTION";
    case DatabaseType::MySQL:
    case DatabaseType::PostgreSQL:
        return "START TRANSACTION";
    }
    return "START TRANSACTION"; // Safe default
}

QString Database::getInsertOrIgnorePrefix(DatabaseType databaseType)
{
    switch (databaseType) {
    case DatabaseType::SQLite:
        return "INSERT OR IGNORE";
    case DatabaseType::MySQL:
        return "INSERT IGNORE";
    case DatabaseType::PostgreSQL:
        return "INSERT"; // PostgreSQL uses ON CONFLICT DO NOTHING at the end
    }
    return "INSERT";
}

bool Database::beginTransaction(const QString &connectionName)
{
    DatabaseType databaseType = getDatabaseType(connectionName);
    QSqlQuery query(QSqlDatabase::database(connectionName));

    if (!query.exec(getBeginTransactionSQL(databaseType))) {
        qDebug() << "Failed to BEGIN transaction:" << query.lastError().text();
        return false;
    }
    return true;
}

bool Database::commitTransaction(const QString &connectionName)
{
    QSqlQuery query(QSqlDatabase::database(connectionName));

    if (!query.exec("COMMIT")) {
        qDebug() << "Failed to COMMIT transaction:" << query.lastError().text();
        return false;
    }
    return true;
}

bool Database::rollbackTransaction(const QString &connectionName)
{
    QSqlQuery query(QSqlDatabase::database(connectionName));

    if (!query.exec("ROLLBACK")) {
        qDebug() << "Failed to ROLLBACK transaction:" << query.lastError().text();
        return false;
    }
    return true;
}

QString Database::getFormattedTimeDifference(DatabaseType databaseType,
                                             const QString &d1DateField,
                                             const QString &d2DateField)
{
    switch (databaseType) {
    case DatabaseType::SQLite:
    {
        // SQLite version using PRINTF and julianday
        // NOT using .arg() because PRINTF has % signs that conflict with QString placeholders
        // Use direct string replacement instead
        QString sql = R"(
                PRINTF('%02d:%02d:%02d %02d:%02d:%02d',
                    CAST(ABS((julianday(FIELD2) - julianday(FIELD1)) * 24 * 60 * 60 / 31536000) AS INTEGER),
                    CAST(ABS(((julianday(FIELD2) - julianday(FIELD1)) * 24 * 60 * 60 % 31536000) / 2592000) AS INTEGER),
                    CAST(ABS(((julianday(FIELD2) - julianday(FIELD1)) * 24 * 60 * 60 % 2592000) / 86400) AS INTEGER),
                    CAST(ABS(((julianday(FIELD2) - julianday(FIELD1)) * 24 * 60 * 60 % 86400) / 3600) AS INTEGER),
                    CAST(ABS(((julianday(FIELD2) - julianday(FIELD1)) * 24 * 60 * 60 % 3600) / 60) AS INTEGER),
                    CAST(ABS((julianday(FIELD2) - julianday(FIELD1)) * 24 * 60 * 60 % 60) AS INTEGER)
                )
            )";
        // Replace placeholders with actual field names (avoiding QString::arg())
        sql.replace("FIELD1", d1DateField);
        sql.replace("FIELD2", d2DateField);
        return sql;
    }

    case DatabaseType::MySQL:
    case DatabaseType::PostgreSQL:
    {
        // MySQL/PostgreSQL version using TIMESTAMPDIFF and MOD() function
        QString sql = QString(R"(
                CONCAT(
                    LPAD(FLOOR(ABS(TIMESTAMPDIFF(SECOND, %1, %2)) / 31536000), 2, '0'), ':',
                    LPAD(FLOOR(MOD(ABS(TIMESTAMPDIFF(SECOND, %1, %2)), 31536000) / 2592000), 2, '0'), ':',
                    LPAD(FLOOR(MOD(ABS(TIMESTAMPDIFF(SECOND, %1, %2)), 2592000) / 86400), 2, '0'), ' ',
                    LPAD(FLOOR(MOD(ABS(TIMESTAMPDIFF(SECOND, %1, %2)), 86400) / 3600), 2, '0'), ':',
                    LPAD(FLOOR(MOD(ABS(TIMESTAMPDIFF(SECOND, %1, %2)), 3600) / 60), 2, '0'), ':',
                    LPAD(MOD(ABS(TIMESTAMPDIFF(SECOND, %1, %2)), 60), 2, '0')
                )
            )").arg(d1DateField, d2DateField);
        return sql;
    }
    }
    return "''"; // Empty string fallback
}

QString Database::getCreateSearchSQL(DatabaseType databaseType)
{
    // For MySQL/PostgreSQL, file size unit fields must be TEXT (not NUMERIC)
    // because they store values like "Bytes", "KB", "MB", "GB", "TB"
    QString sizeUnitType;

    switch (databaseType) {
    case DatabaseType::SQLite:
        sizeUnitType = "NUMERIC";  // SQLite is flexible, accepts both
        break;
    case DatabaseType::MySQL:
    case DatabaseType::PostgreSQL:
        sizeUnitType = "TEXT";     // MySQL/PostgreSQL need TEXT for string values
        break;
    }

    return QString(R"(
                CREATE TABLE IF NOT EXISTS search(
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
                    file_size_min_unit        %1,
                    file_size_max             NUMERIC,
                    file_size_max_unit        %1,
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
    )").arg(sizeUnitType);
}

QSqlError Database::initialize(const QString &connectionName, Collection *collection,
                               const QString &overrideDatabaseMode,
                               const QString &overrideDatabaseFilePath)
{
    // Load database settings from collection settings file
    QSettings settings(collection->settingsFilePath, QSettings::IniFormat);

    // Use override values if provided, otherwise load from settings
    if (!overrideDatabaseMode.isEmpty()) {
        collection->databaseMode = overrideDatabaseMode;
    } else {
        collection->databaseMode = settings.value("Settings/databaseMode").toString();
    }

    if (!overrideDatabaseFilePath.isEmpty()) {
        collection->databaseFilePath = overrideDatabaseFilePath;
    } else {
        collection->databaseFilePath = settings.value("Settings/DatabaseFilePath").toString();
    }

    // Always load other settings from file (not overridden by command line)
    collection->databaseHostName = settings.value("Settings/databaseHostName").toString();
    collection->databaseName = settings.value("Settings/databaseName").toString();
    collection->databasePort = settings.value("Settings/databasePort").toInt();
    collection->databaseUserName = settings.value("Settings/databaseUserName").toString();
    collection->databasePassword = settings.value("Settings/databasePassword").toString();

    // Set defaults if values are not provided
    if (collection->databaseMode.isEmpty())
        collection->databaseMode = "Memory";

    // Prepare database based on selected mode
    QSqlDatabase db;

    if (collection->databaseMode == "Memory") {
        db = QSqlDatabase::addDatabase("QSQLITE", connectionName);
        db.setDatabaseName(":memory:");
    }
    else if (collection->databaseMode == "File") {
        QFile databaseFile(collection->databaseFilePath);
        if (!databaseFile.exists()) {
            return QSqlError("Database file not found", collection->databaseFilePath, QSqlError::ConnectionError);
        }
        else {
            db = QSqlDatabase::addDatabase("QSQLITE", connectionName);
            db.setDatabaseName(collection->databaseFilePath);
        }
    }
    else if (collection->databaseMode == "Hosted") {
        QString driver;
        if (collection->databaseType == "PostgreSQL") {
            driver = "QPSQL";
        } else {
            driver = "QMYSQL";  // Default to MySQL
        }

        db = QSqlDatabase::addDatabase(driver, connectionName);
        db.setHostName(collection->databaseHostName);
        db.setDatabaseName(collection->databaseName);
        db.setPort(collection->databasePort);
        db.setUserName(collection->databaseUserName);
        db.setPassword(collection->databasePassword);
    }

    // Open the database connection
    if (!db.open()) {
        return db.lastError();
    }

    // SQLite pragmas for corruption prevention
    if (getDatabaseType(connectionName) == DatabaseType::SQLite) {
        QSqlQuery pragmaQuery(db);

        // Use WAL mode for better corruption resistance and concurrent access
        if (!pragmaQuery.exec("PRAGMA journal_mode = WAL")) {
            qDebug() << "Failed to set WAL journal mode:" << pragmaQuery.lastError().text();
        }

        // Set synchronous mode for balance between safety and performance
        if (!pragmaQuery.exec("PRAGMA synchronous = NORMAL")) {
            qDebug() << "Failed to set synchronous pragma:" << pragmaQuery.lastError().text();
        }

        // Set page size for better performance
        if (!pragmaQuery.exec("PRAGMA page_size = 4096")) {
            qDebug() << "Failed to set page size:" << pragmaQuery.lastError().text();
        }

        // Set cache size
        if (!pragmaQuery.exec("PRAGMA cache_size = 10000")) {
            qDebug() << "Failed to set cache size:" << pragmaQuery.lastError().text();
        }

        // Enable foreign keys
        if (!pragmaQuery.exec("PRAGMA foreign_keys = ON")) {
            qDebug() << "Failed to enable foreign keys:" << pragmaQuery.lastError().text();
        }

        // Set temp store to memory for better performance
        if (!pragmaQuery.exec("PRAGMA temp_store = MEMORY")) {
            qDebug() << "Failed to set temp store:" << pragmaQuery.lastError().text();
        }

        //qDebug() << "SQLite pragmas set successfully for corruption prevention";
    }

    // Create all necessary tables
    QSqlError tableError = createAllTables(connectionName);
    if (tableError.type() != QSqlError::NoError) {
        return tableError;
    }

    return QSqlError(); // Success
}

QSqlError Database::createAllTables(const QString &connectionName)
{
    QSqlError error;

    // Detect database type
    DatabaseType databaseType = getDatabaseType(connectionName);

    // Create all tables in order
    error = executeSql(connectionName, DatabaseSQL::SQL_CREATE_DEVICE);
    if (error.type() != QSqlError::NoError) return error;

    error = executeSql(connectionName, getCreateCatalogSQL(databaseType));
    if (error.type() != QSqlError::NoError) return error;

    error = executeSql(connectionName, DatabaseSQL::SQL_CREATE_STORAGE);
    if (error.type() != QSqlError::NoError) return error;

    error = executeSql(connectionName, DatabaseSQL::SQL_CREATE_FILE);
    if (error.type() != QSqlError::NoError) return error;

    error = executeSql(connectionName, DatabaseSQL::SQL_CREATE_FILETEMP);
    if (error.type() != QSqlError::NoError) return error;

    error = executeSql(connectionName, getCreateFolderSQL(databaseType));
    if (error.type() != QSqlError::NoError) return error;

    error = executeSql(connectionName, DatabaseSQL::SQL_CREATE_STATISTICS_DEVICE);
    if (error.type() != QSqlError::NoError) return error;

    error = executeSql(connectionName, getCreateSearchSQL(databaseType));
    if (error.type() != QSqlError::NoError) return error;

    error = executeSql(connectionName, getCreateTagSQL(databaseType));
    if (error.type() != QSqlError::NoError) return error;

    error = executeSql(connectionName, DatabaseSQL::SQL_CREATE_PARAMETER);
    if (error.type() != QSqlError::NoError) return error;

    error = executeSql(connectionName, getCreateBackupMappingSQL(databaseType));
    if (error.type() != QSqlError::NoError) return error;

    //Migrate
/*
    error = executeSql(connectionName, DatabaseSQL::SQL_CREATE_STATISTICS_CATALOG);
    if (error.type() != QSqlError::NoError) return error;

    error = executeSql(connectionName, DatabaseSQL::SQL_CREATE_STATISTICS_STORAGE);
    if (error.type() != QSqlError::NoError) return error;

    error = executeSql(connectionName, DatabaseSQL::SQL_CREATE_VIRTUAL_STORAGE);
    if (error.type() != QSqlError::NoError) return error;

    error = executeSql(connectionName, DatabaseSQL::SQL_CREATE_VIRTUAL_STORAGE_CATALOG);
    if (error.type() != QSqlError::NoError) return error;

    error = executeSql(connectionName, DatabaseSQL::SQL_CREATE_DEVICE_CATALOG);
    if (error.type() != QSqlError::NoError) return error;
*/
    return QSqlError(); // Success
}

//----------------------------------------------------------------------
// Utility methods
//----------------------------------------------------------------------

bool Database::tableExists(const QString &connectionName, const QString &tableName)
{
    QSqlQuery query(QSqlDatabase::database(connectionName));
    query.prepare("SELECT name FROM sqlite_master WHERE type='table' AND name=?");
    query.addBindValue(tableName);

    if (query.exec() && query.next()) {
        return true;
    }
    return false;
}

QStringList Database::listTables(const QString &connectionName)
{
    QStringList tables;
    QSqlQuery query(QSqlDatabase::database(connectionName));

    DatabaseType databaseType = getDatabaseType(connectionName);
    QString sql;

    switch(databaseType) {
        case DatabaseType::SQLite:
            sql = "SELECT name FROM sqlite_master WHERE type='table' ORDER BY name";
            break;
        case DatabaseType::MySQL:
            sql = "SHOW TABLES";
            break;
        case DatabaseType::PostgreSQL:
            sql = "SELECT tablename FROM pg_tables WHERE schemaname='public' ORDER BY tablename";
            break;
    }

    if (query.exec(sql)) {
        while (query.next()) {
            tables << query.value(0).toString();
        }
    }

    return tables;
}

QStringList Database::getTableColumns(const QString &connectionName, const QString &tableName)
{
    QStringList columns;
    QSqlQuery query(QSqlDatabase::database(connectionName));

    DatabaseType databaseType = getDatabaseType(connectionName);
    QString sql;

    switch(databaseType) {
        case DatabaseType::SQLite:
            sql = QString("PRAGMA table_info(%1)").arg(tableName);
            break;
        case DatabaseType::MySQL:
            sql = QString("SHOW COLUMNS FROM %1").arg(tableName);
            break;
        case DatabaseType::PostgreSQL:
            sql = QString("SELECT column_name FROM information_schema.columns "
                          "WHERE table_name='%1' ORDER BY ordinal_position").arg(tableName);
            break;
    }

    if (query.exec(sql)) {
        while (query.next()) {
            // SQLite: column name at index 1, others at index 0
            int nameIndex = (databaseType == DatabaseType::SQLite) ? 1 : 0;
            columns << query.value(nameIndex).toString();
        }
    }

    return columns;
}

QSqlError Database::executeSql(const QString &connectionName, const QString &sql)
{
    QSqlQuery query(QSqlDatabase::database(connectionName));

    if (!query.exec(sql)) {
        qDebug() << "Database::executeSql failed:" << query.lastError().text();
        qDebug() << "SQL was:" << sql;
        return query.lastError();
    }

    return QSqlError(); // Success
}

QSqlError Database::dropTableIfExists(const QString &connectionName, const QString &tableName)
{
    if (tableExists(connectionName, tableName)) {
        qDebug() << "Dropping table:" << tableName;

        // First, try to ensure no locks
        QSqlDatabase db = QSqlDatabase::database(connectionName);
        QSqlQuery unlockQuery(db);
        unlockQuery.exec("PRAGMA wal_checkpoint(RESTART)");
        unlockQuery.finish();

        // Try the drop
        QSqlError dropError = executeSql(connectionName, QString("DROP TABLE IF EXISTS %1").arg(tableName));

        if (dropError.type() != QSqlError::NoError) {
            qDebug() << "DROP TABLE failed (table may be locked):" << dropError.text();
            qDebug() << "Migration will continue without dropping" << tableName;
            return QSqlError(); // Return success anyway - not critical
        }

        return dropError;
    } else {
        qDebug() << "Table" << tableName << "doesn't exist, skipping drop";
        return QSqlError(); // Success - nothing to do
    }
}

//----------------------------------------------------------------------
// Updates per Version
//----------------------------------------------------------------------

QSqlError Database::runMigration_2_6(const QString &connectionName)
{
    qDebug() << "=== Database Migration 2.6: Adding selected_device_ID_list to search table ===";

    // Check if column already exists
    QStringList existingColumns = getTableColumns(connectionName, "search");

    if (!existingColumns.contains("selected_device_ID_list")) {
        // Add the new column
        QSqlError addColumnError = executeSql(connectionName,
                                              "ALTER TABLE search ADD COLUMN selected_device_ID_list TEXT");

        if (addColumnError.type() != QSqlError::NoError) {
            qDebug() << "Failed to add selected_device_ID_list column:" << addColumnError.text();
            return addColumnError;
        }

        qDebug() << "Added selected_device_ID_list column to search table";

        // Migrate existing data (simplified version - set default value)
        QSqlQuery updateQuery(QSqlDatabase::database(connectionName));
        updateQuery.exec(R"(
            UPDATE search
            SET selected_device_ID_list = CASE
                WHEN search_catalog_checked = 1 THEN '0'  -- Catalog search
                ELSE '0'  -- Default fallback
            END
            WHERE selected_device_ID_list IS NULL OR selected_device_ID_list = ''
        )");

        qDebug() << "Migrated existing search history device data";
    } else {
        qDebug() << "selected_device_ID_list column already exists, skipping migration";
    }

    qDebug() << "=== Database Migration 2.6 completed ===";
    return QSqlError(); // Success
}

QSqlError Database::runMigration_2_8(const QString &connectionName)
{
    //change cursor
    QApplication::setOverrideCursor(Qt::BusyCursor);


    qDebug() << "=== Database Migration 2.8: PART1 - Drop metadata table, update file/filetemp tables ===";

        // Step 1: Drop the unused metadata table if it exists
        if (auto dropError = dropTableIfExists(connectionName, "metadata");
            dropError.type() != QSqlError::NoError)
        {
            qDebug() << "Warning dropping metadata table:" << dropError.text();
            // Continue - not critical
        }

        // Step 2: Update "file" table structure with DEFAULT NULL for metadata columns
        QStringList existingFileColumns = getTableColumns(connectionName, "file");

        // Define metadata columns and their types
        // Define all columns, their types, and default values in an ordered list
        const QList<QPair<QString, QString>> newFileColumns = {
            {"file_extension", "TEXT"}, {"file_type", "TEXT"}, {"mime_type", "TEXT"},
            {"image_width", "NUMERIC"}, {"image_height", "NUMERIC"}, {"image_orientation", "NUMERIC"},
            {"video_duration_seconds", "NUMERIC"}, {"video_width", "NUMERIC"}, {"video_height", "NUMERIC"}, {"video_codec", "TEXT"},
            {"video_framerate", "NUMERIC"}, {"video_bitrate", "NUMERIC"},
            {"audio_duration_seconds", "NUMERIC"}, {"audio_artist", "TEXT"}, {"audio_album", "TEXT"}, {"audio_title", "TEXT"},
            {"audio_genre", "TEXT"}, {"audio_year", "NUMERIC"}, {"audio_track_number", "NUMERIC"}, {"audio_bitrate", "NUMERIC"}, {"audio_sample_rate", "NUMERIC"},
            {"metadata_extended", "TEXT"}, {"metadata_extraction_date", "TEXT"},
            {"mime_verified", "BOOLEAN"}, {"type_mismatch", "BOOLEAN"}
        };

        // Add missing metadata columns
        for (const auto& [columnName, columnType] : newFileColumns) {
            if (!existingFileColumns.contains(columnName)) {
                qDebug() << "Adding column:" << columnName;
                QString alterSQL = QString("ALTER TABLE file ADD COLUMN %1 %2").arg(columnName, columnType);
                if (auto addColumnError = executeSql(connectionName, alterSQL);
                    addColumnError.type() != QSqlError::NoError)
                {
                    qDebug() << "Error adding column" << columnName << ":" << addColumnError.text();
                    return addColumnError;
                }
                qDebug() << "Added column" << columnName << "to -file- table";
            }
        }

        // Step 3: Update "filetemp" table structure to match file table
        qDebug() << "Updating filetemp table with complete metadata structure...";

        // Get existing filetemp columns
        QStringList existingFiletempColumns = getTableColumns(connectionName, "filetemp");

        // Add missing metadata columns to filetemp (same columns as file table)
        for (const auto& [columnName, columnType] : newFileColumns) {
            if (!existingFiletempColumns.contains(columnName)) {
                qDebug() << "Adding column to filetemp:" << columnName;
                QString alterSQL = QString("ALTER TABLE filetemp ADD COLUMN %1 %2").arg(columnName, columnType);
                if (auto addColumnError = executeSql(connectionName, alterSQL);
                    addColumnError.type() != QSqlError::NoError)
                {
                    qDebug() << "Error adding column" << columnName << "to filetemp:" << addColumnError.text();
                    return addColumnError;
                }
                qDebug() << "Added column" << columnName << "to filetemp table";
            }
        }
        qDebug() << "File and filetemp tables updated with metadata support";


    qDebug() << "=== Database Migration 2.8: PART2 - File type population. DEFERRED ===";
        // qDebug() << "File types will be populated on-demand when each catalog is first used";
        // qDebug() << "This ensures fast startup and processes only catalogs you actually use";


    qDebug() << "=== Database Migration 2.8: PART3 - Update search table ===";
        QStringList existingSearchColumns = getTableColumns(connectionName, "search");
        const QList<QPair<QString, QString>> newSearchColumns = {
            {"metadata_checked", "NUMERIC"}, {"metadata_text_checked", "NUMERIC"}, {"metadata_text_search", "TEXT"},
            {"metadata_size_checked", "NUMERIC"}, {"metadata_size_min_height", "NUMERIC"}, {"metadata_size_max_height", "NUMERIC"},
            {"metadata_size_min_width", "NUMERIC"}, {"metadata_size_max_width", "NUMERIC"}, {"metadata_duration_checked", "NUMERIC"},
            {"metadata_duration_min", "TEXT"}, {"metadata_duration_max", "TEXT"}
        };

        // Add missing metadata columns
        for (const auto& [columnName, columnType] : newSearchColumns) {
            if (!existingSearchColumns.contains(columnName)) {
                qDebug() << "Adding column:" << columnName;
                QString alterSQL = QString("ALTER TABLE search ADD COLUMN %1 %2 DEFAULT NULL").arg(columnName, columnType);
                if (auto addColumnError = executeSql(connectionName, alterSQL);
                    addColumnError.type() != QSqlError::NoError)
                {
                    qDebug() << "Error adding column" << columnName << ":" << addColumnError.text();
                    return addColumnError;
                }
                qDebug() << "Added column" << columnName << "to -search- table";
            }
        }

    qDebug() << "=== Database Migration 2.8: PART4 - Normalize catalog_include_metadata ===";
        QSqlQuery updateCatalogMetadataQuery(QSqlDatabase::database(connectionName));
        updateCatalogMetadataQuery.exec(R"(
            UPDATE catalog
            SET catalog_include_metadata = 'None'
            WHERE catalog_include_metadata IS NULL
               OR catalog_include_metadata = ''
               OR catalog_include_metadata = '0'
               OR catalog_include_metadata = 0
        )");

        int updatedCatalogs = updateCatalogMetadataQuery.numRowsAffected();
        if (updatedCatalogs > 0) {
            qDebug() << "Updated" << updatedCatalogs << "catalog(s) with NULL/empty metadata field to 'None'";
        } else {
            qDebug() << "No catalogs needed metadata field normalization";
        }

        qDebug() << "=== Database Migration 2.8 completed ===";

    //reset cursor
    QApplication::restoreOverrideCursor();

    return QSqlError(); // Success
}

QSqlError Database::runMigration_2_9(const QString &connectionName)
{
    // Change cursor to busy
    QApplication::setOverrideCursor(Qt::BusyCursor);

    qDebug() << "=== Database Migration 2.9: Adding checksum support ===";

    // Step 1: Add checksum columns to file table
    qDebug() << "Step 1: Adding checksum columns to file table";

    const QList<QPair<QString, QString>> newFileColumns = {
        {"checksum_sha256", "TEXT"},
        {"checksum_extraction_date", "TEXT"}
    };

    // Get existing file columns
    QStringList existingFileColumns = getTableColumns(connectionName, "file");

    // Add missing checksum columns to file table
    for (const auto& [columnName, columnType] : newFileColumns) {
        if (!existingFileColumns.contains(columnName)) {
            qDebug() << "Adding column to file:" << columnName;
            QString alterSQL = QString("ALTER TABLE file ADD COLUMN %1 %2").arg(columnName, columnType);
            if (auto addColumnError = executeSql(connectionName, alterSQL);
                addColumnError.type() != QSqlError::NoError)
            {
                qDebug() << "Error adding column" << columnName << ":" << addColumnError.text();
                QApplication::restoreOverrideCursor();
                return addColumnError;
            }
            qDebug() << "Added column" << columnName << "to file table";
        }
    }

    // Step 2: Add checksum columns to filetemp table
    qDebug() << "Step 2: Adding checksum columns to filetemp table";

    // Get existing filetemp columns
    QStringList existingFiletempColumns = getTableColumns(connectionName, "filetemp");

    // Add missing checksum columns to filetemp (same columns as file table)
    for (const auto& [columnName, columnType] : newFileColumns) {
        if (!existingFiletempColumns.contains(columnName)) {
            qDebug() << "Adding column to filetemp:" << columnName;
            QString alterSQL = QString("ALTER TABLE filetemp ADD COLUMN %1 %2").arg(columnName, columnType);
            if (auto addColumnError = executeSql(connectionName, alterSQL);
                addColumnError.type() != QSqlError::NoError)
            {
                qDebug() << "Error adding column" << columnName << "to filetemp:" << addColumnError.text();
                QApplication::restoreOverrideCursor();
                return addColumnError;
            }
            qDebug() << "Added column" << columnName << "to filetemp table";
        }
    }

    qDebug() << "File and filetemp tables updated with checksum support";

    // Step 3: Add checksum column to catalog table
    qDebug() << "Step 3: Adding catalog_include_checksum to catalog table";

    QStringList existingCatalogColumns = getTableColumns(connectionName, "catalog");

    if (!existingCatalogColumns.contains("catalog_include_checksum")) {
        QString alterSQL = "ALTER TABLE catalog ADD COLUMN catalog_include_checksum TEXT";
        if (auto addColumnError = executeSql(connectionName, alterSQL);
            addColumnError.type() != QSqlError::NoError)
        {
            qDebug() << "Error adding catalog_include_checksum column:" << addColumnError.text();
            QApplication::restoreOverrideCursor();
            return addColumnError;
        }
        qDebug() << "Added catalog_include_checksum column to catalog table";
    }

    // Step 4: Normalize catalog_include_checksum (set default to 'None' for existing catalogs)
    qDebug() << "Step 4: Normalizing catalog_include_checksum values";

    QSqlQuery updateCatalogChecksumQuery(QSqlDatabase::database(connectionName));
    updateCatalogChecksumQuery.exec(R"(
        UPDATE catalog
        SET catalog_include_checksum = 'None'
        WHERE catalog_include_checksum IS NULL
           OR catalog_include_checksum = ''
    )");

    int updatedCatalogs = updateCatalogChecksumQuery.numRowsAffected();
    if (updatedCatalogs > 0) {
        qDebug() << "Updated" << updatedCatalogs << "catalog(s) with NULL/empty checksum field to 'None'";
    } else {
        qDebug() << "No catalogs needed checksum field normalization";
    }

    qDebug() << "=== Database Migration 2.9 completed ===";

    // Restore cursor
    QApplication::restoreOverrideCursor();

    return QSqlError(); // Success
}

//----------------------------------------------------------------------
