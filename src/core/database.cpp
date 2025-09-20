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
    QStringList existingColumns = getTableColumns(connectionName, "file");

    // Define metadata columns and their types
    // Define all columns, their types, and default values in a single QMap
    const QMap<QString, QString> newColumns = {
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
    for (auto it = newColumns.constBegin(); it != newColumns.constEnd(); ++it) {
        if (!existingColumns.contains(it.key())) {
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
    qDebug() << "Recreating filetemp table with complete metadata structure...";

    // Drop existing table (don't worry about data - it's temporary)
    if (auto dropFiletempError = dropTableIfExists(connectionName, "filetemp");
        dropFiletempError.type() != QSqlError::NoError)
    {
        qDebug() << "Error dropping filetemp table:" << dropFiletempError.text();
        return dropFiletempError;
    }

    // Create new table with updated structure
    if (auto createError = createFileTempTable(connectionName);
        createError.type() != QSqlError::NoError)
    {
        qDebug() << "Error creating new filetemp table:" << createError.text();
        return createError;
    }
    qDebug() << "File and filetemp tables updated with metadata support";

    qDebug() << "=== Database Migration 2.8: PART2 - Populate file_type fields ===";

    QSqlDatabase db = QSqlDatabase::database(connectionName);
    if (!db.isOpen()) {
        qDebug() << "Database not open for migration 2.9";
        return QSqlError("Database not open", "", QSqlError::ConnectionError);
    }

    // Count files needing migration
    QSqlQuery countQuery(db);
    countQuery.prepare("SELECT COUNT(*) FROM file WHERE file_type IS NULL OR file_type = ''");
    if (!countQuery.exec() || !countQuery.next()) {
        qDebug() << "Failed to count files for migration 2.9:" << countQuery.lastError().text();
        return countQuery.lastError();
    }

    int totalFiles = countQuery.value(0).toInt();
    qDebug() << "Found" << totalFiles << "files needing file_type population";

    if (totalFiles == 0) {
        qDebug() << "No migration needed - all files already have file_type populated";
        return QSqlError(); // Success - nothing to do
    }

    // Begin transaction for better performance
    if (!db.transaction()) {
        qDebug() << "Failed to begin transaction for migration 2.9:" << db.lastError().text();
        return db.lastError();
    }

    // Query files needing migration in batches
    QSqlQuery selectQuery(db);
    selectQuery.prepare(R"(
        SELECT file_name, file_full_path, file_catalog_id
        FROM file
        WHERE file_type IS NULL OR file_type = ''
        ORDER BY file_catalog_id
    )");

    if (!selectQuery.exec()) {
        db.rollback();
        qDebug() << "Failed to select files for migration 2.9:" << selectQuery.lastError().text();
        return selectQuery.lastError();
    }

    // Prepare update query
    QSqlQuery updateQuery(db);
    updateQuery.prepare("UPDATE file SET file_type = ? WHERE file_catalog_id = ? AND file_full_path = ?");

    int processedFiles = 0;
    int batchSize = 1000;

    while (selectQuery.next()) {
        QString fileName = selectQuery.value(0).toString();
        QString fileFullPath = selectQuery.value(1).toString();
        int catalogId = selectQuery.value(2).toInt();

        // Determine file type from extension
        // Use filename if available, otherwise extract from full path
        QString nameForExtraction = fileName.isEmpty() ? fileFullPath : fileName;
        QFileInfo fileInfo(nameForExtraction);
        QString extension = fileInfo.suffix();

        QString fileType;
        if (extension.isEmpty()) {
            fileType = "none"; // Extensionless files
        } else {
            // Use FileMetadata system to determine type
            fileType = FileMetadata::getFileTypeFromExtension(extension);
        }

        // Update the record
        updateQuery.bindValue(0, fileType);
        updateQuery.bindValue(1, catalogId);
        updateQuery.bindValue(2, fileFullPath);

        if (!updateQuery.exec()) {
            qDebug() << "Failed to update file in migration 2.9:" << fileFullPath
                     << updateQuery.lastError().text();
            // Continue with other files instead of failing completely
        }

        processedFiles++;

        // Progress reporting and batch commit
        if (processedFiles % batchSize == 0) {
            int percentComplete = (processedFiles * 100) / totalFiles;
            qDebug() << "Migration 2.8 progress:" << processedFiles << "/" << totalFiles
                     << "(" << percentComplete << "%)";

            // Commit and restart transaction for large datasets
            if (!db.commit() || !db.transaction()) {
                qDebug() << "Transaction restart failed in migration 2.8:" << db.lastError().text();
                return db.lastError();
            }
        }
    }

    // Commit final transaction
    if (!db.commit()) {
        db.rollback();
        qDebug() << "Failed to commit migration 2.9 transaction:" << db.lastError().text();
        return db.lastError();
    }

    qDebug() << "Populated file_type for" << processedFiles << "files";
    qDebug() << "=== Database Migration 2.8 completed ===";
    return QSqlError(); // Success
}


QSqlError Database::dropTableIfExists(const QString &connectionName, const QString &tableName)
{
    if (tableExists(connectionName, tableName)) {
        qDebug() << "Dropping table:" << tableName;
        return executeSql(connectionName, QString("DROP TABLE %1").arg(tableName));
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
