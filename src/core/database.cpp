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
// Description: Handles connection, creation, and modification
// Author:      Stephane Couturier
/////////////////////////////////////////////////////////////////////////////
*/
#include "database.h"
#include "collection.h"
#include "filemetadata.h"
#include <QSettings>
#include <QFile>
#include <QDebug>
#include <QApplication>

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
        db = QSqlDatabase::addDatabase("QPSQL", connectionName);
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

    // Open the database connection
    if (!db.open()) {
        return db.lastError();
    }

    if (db.driverName() == "QSQLITE") {
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

        qDebug() << "SQLite pragmas set successfully for corruption prevention";
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

    // Create all tables in order
    // error = createSchemaTable(connectionName);
    // if (error.type() != QSqlError::NoError) return error;

    error = createDeviceTable(connectionName);
    if (error.type() != QSqlError::NoError) return error;

    error = createCatalogTable(connectionName);
    if (error.type() != QSqlError::NoError) return error;

    error = createStorageTable(connectionName);
    if (error.type() != QSqlError::NoError) return error;

    error = createFileTable(connectionName);
    if (error.type() != QSqlError::NoError) return error;

    error = createFileTempTable(connectionName);
    if (error.type() != QSqlError::NoError) return error;

    error = createFolderTable(connectionName);
    if (error.type() != QSqlError::NoError) return error;

    error = createStatisticsDeviceTable(connectionName);
    if (error.type() != QSqlError::NoError) return error;

    error = createSearchTable(connectionName);
    if (error.type() != QSqlError::NoError) return error;

    error = createTagTable(connectionName);
    if (error.type() != QSqlError::NoError) return error;

    error = createParameterTable(connectionName);
    if (error.type() != QSqlError::NoError) return error;

    error = createBackupMappingTable(connectionName);
    if (error.type() != QSqlError::NoError) return error;

    //Migrate
    error = createStatisticsCatalogTable(connectionName);
    if (error.type() != QSqlError::NoError) return error;

    error = createStatisticsStorageTable(connectionName);
    if (error.type() != QSqlError::NoError) return error;

    error = createVirtualStorageTable(connectionName);
    if (error.type() != QSqlError::NoError) return error;

    error = createVirtualStorageCatalogTable(connectionName);
    if (error.type() != QSqlError::NoError) return error;

    error = createDeviceCatalogTable(connectionName);
    if (error.type() != QSqlError::NoError) return error;
    return QSqlError(); // Success
}

QSqlError Database::createDeviceTable(const QString &connectionName)
{
    return executeSql(connectionName, DatabaseSQL::SQL_CREATE_DEVICE);
}

QSqlError Database::createStorageTable(const QString &connectionName)
{
    return executeSql(connectionName, DatabaseSQL::SQL_CREATE_STORAGE);
}

QSqlError Database::createCatalogTable(const QString &connectionName)
{
    return executeSql(connectionName, DatabaseSQL::SQL_CREATE_CATALOG);
}

QSqlError Database::createFileTable(const QString &connectionName)
{
    return executeSql(connectionName, DatabaseSQL::SQL_CREATE_FILE);
}

QSqlError Database::createFileTempTable(const QString &connectionName)
{
    return executeSql(connectionName, DatabaseSQL::SQL_CREATE_FILETEMP);
}

QSqlError Database::createFolderTable(const QString &connectionName)
{
    return executeSql(connectionName, DatabaseSQL::SQL_CREATE_FOLDER);
}

QSqlError Database::createStatisticsDeviceTable(const QString &connectionName)
{
    return executeSql(connectionName, DatabaseSQL::SQL_CREATE_STATISTICS_DEVICE);
}

QSqlError Database::createSearchTable(const QString &connectionName)
{
    return executeSql(connectionName, DatabaseSQL::SQL_CREATE_SEARCH);
}

QSqlError Database::createTagTable(const QString &connectionName)
{
    return executeSql(connectionName, DatabaseSQL::SQL_CREATE_TAG);
}

QSqlError Database::createParameterTable(const QString &connectionName)
{
    return executeSql(connectionName, DatabaseSQL::SQL_CREATE_PARAMETER);
}

QSqlError Database::createBackupMappingTable(const QString &connectionName)
{
    return executeSql(connectionName, DatabaseSQL::SQL_CREATE_BACKUP_MAPPING);
}

//Migrate
QSqlError Database::createStatisticsCatalogTable(const QString &connectionName)
{
    return executeSql(connectionName, DatabaseSQL::SQL_CREATE_STATISTICS_CATALOG);
}

QSqlError Database::createStatisticsStorageTable(const QString &connectionName)
{
    return executeSql(connectionName, DatabaseSQL::SQL_CREATE_STATISTICS_STORAGE);
}

QSqlError Database::createVirtualStorageTable(const QString &connectionName)
{
    return executeSql(connectionName, DatabaseSQL::SQL_CREATE_VIRTUAL_STORAGE);
}

QSqlError Database::createVirtualStorageCatalogTable(const QString &connectionName)
{
    return executeSql(connectionName, DatabaseSQL::SQL_CREATE_VIRTUAL_STORAGE_CATALOG);
}

QSqlError Database::createDeviceCatalogTable(const QString &connectionName)
{
    return executeSql(connectionName, DatabaseSQL::SQL_CREATE_DEVICE_CATALOG);
}


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

    if (query.exec("SELECT name FROM sqlite_master WHERE type='table' ORDER BY name")) {
        while (query.next()) {
            tables << query.value(0).toString();
        }
    }

    return tables;
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

//----------------------------------------------------------------------
// Updates
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

//----------------------------------------------------------------------
QSqlError Database::runMigration_2_8(const QString &connectionName)
{
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
    // Define all columns, their types, and default values in a single QMap
    const QMap<QString, QString> newFileColumns = {
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
    for (auto it = newFileColumns.constBegin(); it != newFileColumns.constEnd(); ++it) {
        if (!existingFileColumns.contains(it.key())) {
            qDebug() << "Adding column:" << it.key();
            QString alterSQL = QString("ALTER TABLE file ADD COLUMN %1 %2 DEFAULT NULL").arg(it.key(), it.value());
            if (auto addColumnError = executeSql(connectionName, alterSQL);
                addColumnError.type() != QSqlError::NoError)
            {
                qDebug() << "Error adding column" << it.key() << ":" << addColumnError.text();
                return addColumnError;
            }
            qDebug() << "Added column" << it.key() << "to -file- table";
        }
    }

    // Step 3: Update "filetemp" table structure to match file table
    qDebug() << "Updating filetemp table with complete metadata structure...";

    // Get existing filetemp columns
    QStringList existingFiletempColumns = getTableColumns(connectionName, "filetemp");

    // Add missing metadata columns to filetemp (same columns as file table)
    for (auto it = newFileColumns.constBegin(); it != newFileColumns.constEnd(); ++it) {
        if (!existingFiletempColumns.contains(it.key())) {
            qDebug() << "Adding column to filetemp:" << it.key();
            QString alterSQL = QString("ALTER TABLE filetemp ADD COLUMN %1 %2 DEFAULT NULL").arg(it.key(), it.value());
            if (auto addColumnError = executeSql(connectionName, alterSQL);
                addColumnError.type() != QSqlError::NoError)
            {
                qDebug() << "Error adding column" << it.key() << "to filetemp:" << addColumnError.text();
                return addColumnError;
            }
            qDebug() << "Added column" << it.key() << "to filetemp table";
        }
    }
    qDebug() << "File and filetemp tables updated with metadata support";

    qDebug() << "=== Database Migration 2.8: PART2 - File type population. DEFERRED ===";
    // qDebug() << "File types will be populated on-demand when each catalog is first used";
    // qDebug() << "This ensures fast startup and processes only catalogs you actually use";

    qDebug() << "=== Database Migration 2.8: PART3 - Update search table ===";
    QStringList existingSearchColumns = getTableColumns(connectionName, "search");
    const QMap<QString, QString> newSearchColumns = {
        {"metadata_checked", "NUMERIC"}, {"metadata_text_checked", "NUMERIC"}, {"metadata_text_search", "TEXT"},
        {"metadata_size_checked", "NUMERIC"}, {"metadata_size_min_height", "NUMERIC"}, {"metadata_size_max_height", "NUMERIC"},
        {"metadata_size_min_width", "NUMERIC"}, {"metadata_size_max_width", "NUMERIC"}, {"metadata_duration_checked", "NUMERIC"},
        {"metadata_duration_min", "TEXT"}, {"metadata_duration_max", "TEXT"}
    };

    // Add missing metadata columns
    for (auto it = newSearchColumns.constBegin(); it != newSearchColumns.constEnd(); ++it) {
        if (!existingSearchColumns.contains(it.key())) {
            qDebug() << "Adding column:" << it.key();
            QString alterSQL = QString("ALTER TABLE search ADD COLUMN %1 %2 DEFAULT NULL").arg(it.key(), it.value());
            if (auto addColumnError = executeSql(connectionName, alterSQL);
                addColumnError.type() != QSqlError::NoError)
            {
                qDebug() << "Error adding column" << it.key() << ":" << addColumnError.text();
                return addColumnError;
            }
            qDebug() << "Added column" << it.key() << "to -file- table";
        }
    }

    qDebug() << "=== Database Migration 2.8: PART4 - Normalize catalog_include_metadata ===";
    QSqlQuery updateCatalogMetadataQuery(QSqlDatabase::database(connectionName));
    updateCatalogMetadataQuery.exec(R"(
        UPDATE catalog
        SET catalog_include_metadata = 'None'
        WHERE catalog_include_metadata IS NULL
           OR catalog_include_metadata = ''
    )");

    int updatedCatalogs = updateCatalogMetadataQuery.numRowsAffected();
    if (updatedCatalogs > 0) {
        qDebug() << "Updated" << updatedCatalogs << "catalog(s) with NULL/empty metadata field to 'None'";
    } else {
        qDebug() << "No catalogs needed metadata field normalization";
    }

    qDebug() << "=== Database Migration 2.8 completed ===";
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

QStringList Database::getTableColumns(const QString &connectionName, const QString &tableName)
{
    QStringList columns;
    QSqlQuery query(QSqlDatabase::database(connectionName));

    if (query.exec(QString("PRAGMA table_info(%1)").arg(tableName))) {
        while (query.next()) {
            columns << query.value(1).toString(); // column name is at index 1
        }
    }

    return columns;
}
