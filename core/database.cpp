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
#include <QCoreApplication>
#include <qhostaddress.h>

//----------------------------------------------------------------------
// Core database definition
//----------------------------------------------------------------------
QString Database::getSQLCreateTableDevice(DatabaseType dbType)
{
    // MySQL/PostgreSQL need BIGINT for large file sizes (NUMERIC defaults to ~9GB max)
    QString largeNumeric = (dbType == DatabaseType::SQLite) ? "NUMERIC" : "BIGINT";

    return QString(R"(
                CREATE TABLE IF NOT EXISTS device(
                    device_id                  %1 PRIMARY KEY,
                    device_parent_id           %1,
                    device_name                TEXT,
                    device_type                TEXT,
                    device_external_id         %1,
                    device_path                TEXT,
                    device_total_file_size     %1 default 0,
                    device_total_file_count    %1 default 0,
                    device_total_space         %1 default 0,
                    device_free_space          %1 default 0,
                    device_active              %1,
                    device_group_id            %1,
                    device_date_updated        TEXT,
                    device_order               %1)
            )").arg(largeNumeric);
}

QString Database::getSQLCreateTableCatalog(DatabaseType databaseType)
{
    QString catalogIdType;
    QString catalogNameType;
    QString largeNumeric;
    QString primaryKey;
    QString uniqueCatalogName;

    switch (databaseType) {
    case DatabaseType::SQLite:
        catalogIdType     = "NUMERIC";
        catalogNameType   = "TEXT";
        largeNumeric      = "NUMERIC";
        primaryKey        = "PRIMARY KEY(catalog_id)";
        uniqueCatalogName = "UNIQUE(catalog_name)";
        break;
    case DatabaseType::MySQL:
        catalogIdType     = "BIGINT NOT NULL";
        catalogNameType   = "VARCHAR(500)";
        largeNumeric      = "BIGINT";
        primaryKey        = "PRIMARY KEY(catalog_id)";
        uniqueCatalogName = "UNIQUE KEY (catalog_name(500))";
        break;
    case DatabaseType::PostgreSQL:
        catalogIdType     = "BIGINT NOT NULL";
        catalogNameType   = "VARCHAR(500)";
        largeNumeric      = "BIGINT";
        primaryKey        = "PRIMARY KEY(catalog_id)";
        uniqueCatalogName = "UNIQUE(catalog_name)";
        break;
    }

    return QString(R"(
                CREATE TABLE IF NOT EXISTS catalog(
                    catalog_id                    %3,
                    catalog_file_path             TEXT,
                    catalog_name                  %1,
                    catalog_date_updated          TEXT,
                    catalog_source_path           TEXT,
                    catalog_file_count            %2 default 0,
                    catalog_total_file_size       %2 default 0,
                    catalog_source_path_is_active %2,
                    catalog_include_hidden        TEXT,
                    catalog_file_type             TEXT,
                    catalog_storage               TEXT,
                    catalog_include_symblinks     TEXT,
                    catalog_is_full_device        TEXT,
                    catalog_date_loaded           TEXT,
                    catalog_include_metadata      TEXT,
                    catalog_include_checksum      TEXT,
                    catalog_app_version           TEXT,
                    %4,
                    %5)
    )").arg(catalogNameType, largeNumeric, catalogIdType, primaryKey, uniqueCatalogName);
}

QString Database::getSQLCreateTableStorage(DatabaseType dbType)
{
    // MySQL/PostgreSQL need BIGINT for large file sizes (NUMERIC defaults to ~9GB max)
    QString largeNumeric = (dbType == DatabaseType::SQLite) ? "NUMERIC" : "BIGINT";

    return QString(R"(
                CREATE TABLE IF NOT EXISTS storage(
                    storage_id            %1  primary key default 0,
                    storage_name          TEXT,
                    storage_type          TEXT,
                    storage_location      TEXT,
                    storage_path          TEXT,
                    storage_label         TEXT,
                    storage_file_system   TEXT,
                    storage_total_space   %1 default 0,
                    storage_free_space    %1 default 0,
                    storage_brand         TEXT,
                    storage_model         TEXT,
                    storage_serial_number TEXT,
                    storage_build_date    TEXT,
                    storage_comment1      TEXT,
                    storage_comment2      TEXT,
                    storage_comment3      TEXT,
                    storage_picture_path  TEXT)
            )").arg(largeNumeric);
}

QString Database::getSQLCreateTableFile(DatabaseType dbType)
{
    // MySQL/PostgreSQL need BIGINT for file_size (single files can exceed 9GB)
    QString largeNumeric = (dbType == DatabaseType::SQLite) ? "NUMERIC" : "BIGINT";

    return QString(R"(
                CREATE TABLE IF NOT EXISTS file(
                    file_catalog_id         %1,
                    file_name               TEXT,     -- "home.jpg"
                    file_folder_path        TEXT,     -- "/home/user/photos"
                    file_size               %1,
                    file_date_updated       TEXT,
                    file_catalog            TEXT,
                    file_full_path          TEXT,     -- "/home/user/photos/home.jpg"
                    file_extension          TEXT,     -- "jpg"
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
                    metadata_extended       TEXT,     -- JSON for additional fields
                    metadata_extraction_date TEXT,
                    checksum_sha256          TEXT,
                    checksum_extraction_date TEXT)
            )").arg(largeNumeric);
}

QString Database::getSQLCreateTableFileTemp(DatabaseType dbType)
{
    // MySQL/PostgreSQL need BIGINT for file_size (single files can exceed 9GB)
    QString largeNumeric = (dbType == DatabaseType::SQLite) ? "NUMERIC" : "BIGINT";

    return QString(R"(
                CREATE TABLE IF NOT EXISTS filetemp(
                    file_catalog_id         %1,
                    file_name               TEXT,     -- "home.jpg"
                    file_folder_path        TEXT,     -- "/home/user/photos"
                    file_size               %1,
                    file_date_updated       TEXT,
                    file_catalog            TEXT,
                    file_full_path          TEXT,     -- "/home/user/photos/home.jpg"
                    file_extension          TEXT,     -- "jpg"
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
                    metadata_extended       TEXT,     -- JSON for additional fields
                    metadata_extraction_date TEXT,
                    checksum_sha256          TEXT,
                    checksum_extraction_date TEXT)
            )").arg(largeNumeric);
}

QString Database::getSQLCreateTableFolder(DatabaseType databaseType)
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

QString Database::getSQLCreateTableStatisticsDevice(DatabaseType dbType)
{
    // MySQL/PostgreSQL need BIGINT for large file sizes
    QString largeNumeric = (dbType == DatabaseType::SQLite) ? "NUMERIC" : "BIGINT";

    return QString(R"(
                CREATE TABLE IF NOT EXISTS statistics_device(
                    date_time               TEXT,
                    device_id               %1,
                    device_name             TEXT,
                    device_type             TEXT,
                    device_file_count       %1,
                    device_total_file_size  %1,
                    device_free_space       %1,
                    device_total_space      %1,
                    record_type             TEXT)
            )").arg(largeNumeric);
}

QString Database::getSQLCreateTableSearch(DatabaseType databaseType)
{
    QString sizeUnitType;
    QString largeNumeric;

    switch (databaseType) {
    case DatabaseType::SQLite:
        sizeUnitType = "NUMERIC";
        largeNumeric = "NUMERIC";
        break;
    case DatabaseType::MySQL:
    case DatabaseType::PostgreSQL:
        sizeUnitType = "TEXT";      // For string values like "Bytes", "KB", etc.
        largeNumeric = "BIGINT";    // For file sizes
        break;
    }

    return QString(R"(
                CREATE TABLE IF NOT EXISTS search(
                    date_time                   TEXT,
                    text_checked                NUMERIC,
                    text_phrase                 TEXT,
                    text_criteria               TEXT,
                    text_search_in              TEXT,
                    file_criteria_checked       NUMERIC,
                    file_type_checked           NUMERIC,
                    file_type                   TEXT,
                    file_size_checked           NUMERIC,
                    file_size_min               %2,
                    file_size_min_unit          %1,
                    file_size_max               %2,
                    file_size_max_unit          %1,
                    date_modified_checked       NUMERIC,
                    date_modified_min           TEXT,
                    date_modified_max           TEXT,
                    duplicates_checked          NUMERIC,
                    duplicates_name             NUMERIC,
                    duplicates_size             NUMERIC,
                    duplicates_date_modified    NUMERIC,
                    duplicates_checksum         NUMERIC,
                    duplicates_checksum_equal   NUMERIC,
                    duplicates_compare_checked  NUMERIC,
                    duplicates_device1_ID       NUMERIC,
                    duplicates_device2_ID       NUMERIC,
                    differences_checked         NUMERIC,
                    differences_name            NUMERIC,
                    differences_size            NUMERIC,
                    differences_date_modified   NUMERIC,
                    differences_checksum        NUMERIC,
                    differences_checksum_equal  NUMERIC,
                    differences_catalogs        TEXT,
                    folder_criteria_checked     NUMERIC,
                    show_folders                NUMERIC,
                    tag_checked                 NUMERIC,
                    tag                         TEXT,
                    search_location             TEXT,
                    search_storage              TEXT,
                    search_catalog              TEXT,
                    search_catalog_checked      NUMERIC,
                    search_directory_checked    NUMERIC,
                    selected_directory          TEXT,
                    selected_device_ID_list     TEXT,
                    text_exclude                TEXT,
                    case_sensitive              NUMERIC,
                    metadata_checked            NUMERIC,
                    metadata_text_checked       NUMERIC,
                    metadata_text_search        TEXT,
                    metadata_size_checked       NUMERIC,
                    metadata_size_min_height    NUMERIC,
                    metadata_size_max_height    NUMERIC,
                    metadata_size_min_width     NUMERIC,
                    metadata_size_max_width     NUMERIC,
                    metadata_duration_checked   NUMERIC,
                    metadata_duration_min       TEXT,
                    metadata_duration_max       TEXT)
    )").arg(sizeUnitType, largeNumeric);
}

QString Database::getSQLCreateTableTag(DatabaseType databaseType)
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

QString Database::getSQLCreateTableParameter(DatabaseType dbType)
{
    return R"(
            CREATE TABLE IF NOT EXISTS parameter(
                    parameter_name      TEXT,
                    parameter_type      TEXT,
                    parameter_value1    TEXT,
                    parameter_value2    TEXT)
            )";
}

QString Database::getSQLCreateTableBackupMapping(DatabaseType databaseType)
{
    QString autoIncrementSyntax;
    QString largeNumeric;

    switch (databaseType) {
    case DatabaseType::SQLite:
        autoIncrementSyntax = "INTEGER PRIMARY KEY AUTOINCREMENT";
        largeNumeric = "NUMERIC";
        break;
    case DatabaseType::MySQL:
        autoIncrementSyntax = "INT AUTO_INCREMENT PRIMARY KEY";
        largeNumeric = "BIGINT";
        break;
    case DatabaseType::PostgreSQL:
        autoIncrementSyntax = "SERIAL PRIMARY KEY";
        largeNumeric = "BIGINT";
        break;
    }

    return QString(R"(
                CREATE TABLE IF NOT EXISTS device_mapping(
                    mapping_id                  %1,
                    mapping_name                TEXT,
                    mapping_type                TEXT,
                    mapping_device_source_id    %2,
                    mapping_device_target_id    %2,
                    mapping_backup_last_date    TEXT,
                    mapping_backup_last_size    %2,
                    mapping_strict_copy                   INTEGER DEFAULT 1,
                    mapping_conflict_mode                 TEXT DEFAULT 'RenameOldest',
                    mapping_source_mode                   TEXT DEFAULT 'Catalog',
                    mapping_source_collection             TEXT)
            )").arg(autoIncrementSyntax, largeNumeric);
}

QString Database::getSQLCreateTableCatalogFilter(DatabaseType databaseType)
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
        CREATE TABLE IF NOT EXISTS catalog_filter (
            filter_id         %1,
            filter_catalog_id INTEGER NOT NULL,
            filter_type       TEXT    NOT NULL DEFAULT 'exclude_folder',
            filter_value      TEXT    NOT NULL,
            UNIQUE(filter_catalog_id, filter_type, filter_value)
        )
    )").arg(autoIncrementSyntax);
}

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
//----------------------------------------------------------------------
Database::HostnameValidationType Database::validateHostname(const QString &hostname)
{
    QString trimmedHost = hostname.trimmed().toLower();

    // Empty hostname is invalid
    if (trimmedHost.isEmpty()) {
        return PublicOrInvalid;
    }

    // Remove any protocol prefix (http://, https://, etc.) - these shouldn't be in hostname
    if (trimmedHost.contains("://")) {
        return PublicOrInvalid;
    }

    // Check for localhost variations
    if (trimmedHost == "localhost" || trimmedHost == "::1") {
        return Localhost;
    }

    // Check for IPv4 loopback range (127.0.0.0/8)
    QHostAddress addr(trimmedHost);
    if (!addr.isNull() && addr.protocol() == QAbstractSocket::IPv4Protocol) {
        quint32 ipv4 = addr.toIPv4Address();

        // 127.0.0.0/8 - loopback
        if ((ipv4 & 0xFF000000) == 0x7F000000) {
            return Localhost;
        }

        // 10.0.0.0/8 - private
        if ((ipv4 & 0xFF000000) == 0x0A000000) {
            return PrivateNetwork;
        }

        // 172.16.0.0/12 - private
        if ((ipv4 & 0xFFF00000) == 0xAC100000) {
            return PrivateNetwork;
        }

        // 192.168.0.0/16 - private
        if ((ipv4 & 0xFFFF0000) == 0xC0A80000) {
            return PrivateNetwork;
        }

        // Any other IP is considered public/invalid
        return PublicOrInvalid;
    }

    // Check for IPv6 addresses
    if (!addr.isNull() && addr.protocol() == QAbstractSocket::IPv6Protocol) {
        // ::1 already handled above
        // For simplicity, treat other IPv6 as invalid for now
        // (could be enhanced to check for link-local fe80::/10, ULA fc00::/7, etc.)
        return PublicOrInvalid;
    }

    // If it's not a recognized IP format, it might be a domain name
    // Domain names that could resolve to public IPs are not allowed
    return PublicOrInvalid;
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

    // Set database host defaults for Hosted mode
    if (collection->databaseHostName.isEmpty()) {
        collection->databaseHostName = "localhost";
    }
    if (collection->databasePort == 0) {
        collection->databasePort = 3306;  // Default MySQL/MariaDB port
    }

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
            qWarning() << "WARNING: Failed to set WAL journal mode:" << pragmaQuery.lastError().text();
        }

        // Set synchronous mode for balance between safety and performance
        if (!pragmaQuery.exec("PRAGMA synchronous = NORMAL")) {
            qWarning() << "WARNING: Failed to set synchronous pragma:" << pragmaQuery.lastError().text();
        }

        // Set page size for better performance
        if (!pragmaQuery.exec("PRAGMA page_size = 4096")) {
            qWarning() << "WARNING: Failed to set page size:" << pragmaQuery.lastError().text();
        }

        // Set cache size
        if (!pragmaQuery.exec("PRAGMA cache_size = 10000")) {
            qWarning() << "WARNING: Failed to set cache size:" << pragmaQuery.lastError().text();
        }

        // Enable foreign keys
        if (!pragmaQuery.exec("PRAGMA foreign_keys = ON")) {
            qWarning() << "WARNING: Failed to enable foreign keys:" << pragmaQuery.lastError().text();
        }

        // Set temp store to memory for better performance
        if (!pragmaQuery.exec("PRAGMA temp_store = MEMORY")) {
            qWarning() << "WARNING: Failed to set temp store:" << pragmaQuery.lastError().text();
        }

        //qDebug() << "SQLite pragmas set successfully for corruption prevention";
    }

    // Create all necessary tables
    QSqlError tableError = createAllTables(connectionName);
    if (tableError.type() != QSqlError::NoError) {
        return tableError;
    }

    // After creating tables, create indexes
    DatabaseType dbType = getDatabaseType(connectionName);
    QString indexSQL = getSQLCreateIndexes(dbType);

    // For SQLite, can execute all at once
    // For MySQL, need to execute one by one and ignore "already exists" errors
    QStringList indexStatements = indexSQL.split(';', Qt::SkipEmptyParts);
    for (const QString& stmt : indexStatements) {
        QString trimmed = stmt.trimmed();
        if (!trimmed.isEmpty()) {
            QSqlQuery query(QSqlDatabase::database(connectionName));
            if (!query.exec(trimmed)) {
                // Ignore "index already exists" errors
                QString error = query.lastError().text().toLower();
                if (!error.contains("already exists") && !error.contains("duplicate")) {
                    qWarning() << "WARNING: Index creation warning:" << query.lastError().text();
                }
            }
        }
    }

    return QSqlError(); // Success
}

QSqlError Database::createAllTables(const QString &connectionName)
{
    QSqlError error;

    // Detect database type
    DatabaseType databaseType = getDatabaseType(connectionName);

    // Create all tables in order
    error = executeSql(connectionName, getSQLCreateTableDevice(databaseType));
    if (error.type() != QSqlError::NoError) return error;

    error = executeSql(connectionName, getSQLCreateTableCatalog(databaseType));
    if (error.type() != QSqlError::NoError) return error;

    error = executeSql(connectionName, getSQLCreateTableStorage(databaseType));
    if (error.type() != QSqlError::NoError) return error;

    error = executeSql(connectionName, getSQLCreateTableFile(databaseType));
    if (error.type() != QSqlError::NoError) return error;

    error = executeSql(connectionName, getSQLCreateTableFileTemp(databaseType));
    if (error.type() != QSqlError::NoError) return error;

    error = executeSql(connectionName, getSQLCreateTableFolder(databaseType));
    if (error.type() != QSqlError::NoError) return error;

    error = executeSql(connectionName, getSQLCreateTableStatisticsDevice(databaseType));
    if (error.type() != QSqlError::NoError) return error;

    error = executeSql(connectionName, getSQLCreateTableSearch(databaseType));
    if (error.type() != QSqlError::NoError) return error;

    error = executeSql(connectionName, getSQLCreateTableTag(databaseType));
    if (error.type() != QSqlError::NoError) return error;

    error = executeSql(connectionName, getSQLCreateTableParameter(databaseType));
    if (error.type() != QSqlError::NoError) return error;

    error = executeSql(connectionName, getSQLCreateTableBackupMapping(databaseType));
    if (error.type() != QSqlError::NoError) return error;

    error = executeSql(connectionName, getSQLCreateTableCatalogFilter(databaseType));
    if (error.type() != QSqlError::NoError) return error;

    return QSqlError(); // Success
}

QString Database::getSQLCreateIndexes(DatabaseType dbType)
{
    // MySQL requires prefix length for TEXT columns in indexes
    // SQLite doesn't support prefix lengths but handles TEXT indexes fine

    switch (dbType) {
    case DatabaseType::SQLite:
        return R"(
            CREATE INDEX IF NOT EXISTS idx_file_catalog_path ON file(file_catalog_id, file_full_path);
            CREATE INDEX IF NOT EXISTS idx_filetemp_catalog_path ON filetemp(file_catalog_id, file_full_path);
            CREATE INDEX IF NOT EXISTS idx_file_catalog_folder ON file(file_catalog_id, file_folder_path);
        )";

    case DatabaseType::MySQL:
        // MySQL needs prefix length for TEXT columns (max 767 bytes for InnoDB)
        return R"(
            CREATE INDEX idx_file_catalog_path ON file(file_catalog_id, file_full_path(500));
            CREATE INDEX idx_filetemp_catalog_path ON filetemp(file_catalog_id, file_full_path(500));
            CREATE INDEX idx_file_catalog_folder ON file(file_catalog_id, file_folder_path(500));
        )";

    case DatabaseType::PostgreSQL:
        // PostgreSQL uses functional indexes with LEFT() for prefix indexing
        return R"(
            CREATE INDEX IF NOT EXISTS idx_file_catalog_path ON file(file_catalog_id, LEFT(file_full_path, 500));
            CREATE INDEX IF NOT EXISTS idx_filetemp_catalog_path ON filetemp(file_catalog_id, LEFT(file_full_path, 500));
            CREATE INDEX IF NOT EXISTS idx_file_catalog_folder ON file(file_catalog_id, LEFT(file_folder_path, 500));
        )";
    }
    return "";
}

//----------------------------------------------------------------------
// Utility methods
//----------------------------------------------------------------------
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
        qWarning() << "WARNING: Failed to BEGIN transaction:" << query.lastError().text();
        return false;
    }
    return true;
}

bool Database::commitTransaction(const QString &connectionName)
{
    QSqlQuery query(QSqlDatabase::database(connectionName));

    if (!query.exec("COMMIT")) {
        qWarning() << "WARNING: Failed to COMMIT transaction:" << query.lastError().text();
        return false;
    }
    return true;
}

bool Database::rollbackTransaction(const QString &connectionName)
{
    QSqlQuery query(QSqlDatabase::database(connectionName));

    if (!query.exec("ROLLBACK")) {
        qWarning() << "WARNING: Failed to ROLLBACK transaction:" << query.lastError().text();
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
    {
        // MySQL version using TIMESTAMPDIFF and MOD() function
        QString sql = QString(R"(
                CONCAT(
                    LPAD(FLOOR(ABS(TIMESTAMPDIFF(SECOND, %1, %2)) / 31536000), 2, '0'), ':',
                    LPAD(FLOOR(MOD(ABS(TIMESTAMPDIFF(SECOND, %1, %2)), 31536000) / 2592000), 2, '0'), ':',
                    LPAD(FLOOR(MOD(ABS(TIMESTAMPDIFF(SECOND, %1, %2)), 2592000) / 86400), 2, '0'), ' ',
                    LPAD(FLOOR(MOD(ABS(TIMESTAMPDIFF(SECOND, %1, %2)), 86400) / 3600), 2, '0'), ':',
                    LPAD(FLOOR(MOD(ABS(TIMESTAMPDIFF(SECOND, %1, %2)), 3600) / 60), 2, '0'), ':',
                    LPAD(CAST(MOD(ABS(TIMESTAMPDIFF(SECOND, %1, %2)), 60) AS CHAR), 2, '0')
                )
            )").arg(d1DateField, d2DateField);
        return sql;
    }

    case DatabaseType::PostgreSQL:
    {
        // PostgreSQL version using EXTRACT(EPOCH FROM ...) instead of TIMESTAMPDIFF
        QString sql = QString(R"(
                CONCAT(
                    LPAD(FLOOR(ABS(EXTRACT(EPOCH FROM (%2 - %1))) / 31536000)::TEXT, 2, '0'), ':',
                    LPAD(FLOOR(MOD(ABS(EXTRACT(EPOCH FROM (%2 - %1)))::BIGINT, 31536000) / 2592000)::TEXT, 2, '0'), ':',
                    LPAD(FLOOR(MOD(ABS(EXTRACT(EPOCH FROM (%2 - %1)))::BIGINT, 2592000) / 86400)::TEXT, 2, '0'), ' ',
                    LPAD(FLOOR(MOD(ABS(EXTRACT(EPOCH FROM (%2 - %1)))::BIGINT, 86400) / 3600)::TEXT, 2, '0'), ':',
                    LPAD(FLOOR(MOD(ABS(EXTRACT(EPOCH FROM (%2 - %1)))::BIGINT, 3600) / 60)::TEXT, 2, '0'), ':',
                    LPAD(MOD(ABS(EXTRACT(EPOCH FROM (%2 - %1)))::BIGINT, 60)::TEXT, 2, '0')
                )
            )").arg(d1DateField, d2DateField);
        return sql;
    }
    }
    return "''"; // Empty string fallback
}


bool Database::tableExists(const QString &connectionName, const QString &tableName)
{
    QSqlQuery query(QSqlDatabase::database(connectionName));

    DatabaseType databaseType = getDatabaseType(connectionName);
    QString sql;

    switch(databaseType) {
        case DatabaseType::SQLite:
            sql = "SELECT name FROM sqlite_master WHERE type='table' AND name=?";
            break;
        case DatabaseType::MySQL:
            sql = "SELECT TABLE_NAME FROM information_schema.TABLES WHERE TABLE_SCHEMA=DATABASE() AND TABLE_NAME=?";
            break;
        case DatabaseType::PostgreSQL:
            sql = "SELECT tablename FROM pg_tables WHERE schemaname='public' AND tablename=?";
            break;
    }

    query.prepare(sql);
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
        qWarning() << "WARNING: Database::executeSql failed:" << query.lastError().text();
        return query.lastError();
    }

    return QSqlError(); // Success
}

QSqlError Database::dropTableIfExists(const QString &connectionName, const QString &tableName)
{
    if (tableExists(connectionName, tableName)) {

        // First, try to ensure no locks (SQLite-specific)
        QSqlDatabase db = QSqlDatabase::database(connectionName);
        if (getDatabaseType(connectionName) == DatabaseType::SQLite) {
            QSqlQuery unlockQuery(db);
            unlockQuery.exec("PRAGMA wal_checkpoint(RESTART)");
            unlockQuery.finish();
        }

        // Try the drop
        QSqlError dropError = executeSql(connectionName, QString("DROP TABLE IF EXISTS %1").arg(tableName));

        if (dropError.type() != QSqlError::NoError) {
            qWarning() << "WARNING: DROP TABLE failed (table may be locked):" << dropError.text();
            return QSqlError(); // Return success anyway - not critical
        }

        return dropError;
    } else {
        return QSqlError(); // Success - nothing to do
    }
}

//----------------------------------------------------------------------
// Updates per Version
//----------------------------------------------------------------------
QSqlError Database::runMigration_2_11(const QString &connectionName)
{
    // --- Part 1: Add storage_picture_path column ---
    QStringList existingColumns = getTableColumns(connectionName, "storage");
    if (!existingColumns.contains("storage_picture_path")) {
        QSqlError err = executeSql(connectionName,
            "ALTER TABLE storage ADD COLUMN storage_picture_path TEXT");
        if (err.type() != QSqlError::NoError) {
            qWarning() << "WARNING: Failed to add storage_picture_path column:" << err.text();
            return err;
        }
    }

    // --- Part 2: Fix catalog PRIMARY KEY (catalog_name → catalog_id) ---
    // Idempotency check: only migrate if catalog_name is still the PK
    DatabaseType dbType = getDatabaseType(connectionName);
    bool needsCatalogPkMigration = false;
    {
        QSqlQuery pkCheckQuery(QSqlDatabase::database(connectionName));
        if (dbType == DatabaseType::SQLite) {
            // PRAGMA table_info columns: cid(0), name(1), type(2), notnull(3), dflt_value(4), pk(5)
            // pk=1 marks the primary key column
            if (pkCheckQuery.exec("PRAGMA table_info(catalog)")) {
                while (pkCheckQuery.next()) {
                    if (pkCheckQuery.value(5).toInt() == 1) {
                        needsCatalogPkMigration = (pkCheckQuery.value(1).toString() == "catalog_name");
                        break;
                    }
                }
            }
        } else if (dbType == DatabaseType::MySQL) {
            if (pkCheckQuery.exec(
                    "SELECT COLUMN_NAME FROM information_schema.KEY_COLUMN_USAGE "
                    "WHERE TABLE_NAME = 'catalog' AND CONSTRAINT_NAME = 'PRIMARY' "
                    "AND TABLE_SCHEMA = DATABASE()")) {
                if (pkCheckQuery.next())
                    needsCatalogPkMigration = (pkCheckQuery.value(0).toString() == "catalog_name");
            }
        }
    }

    if (needsCatalogPkMigration) {
        QSqlError err;
        if (dbType == DatabaseType::SQLite) {
            // SQLite cannot alter a PRIMARY KEY in-place — rebuild the table
            if (!beginTransaction(connectionName))
                return QSqlError("transaction", "Failed to begin transaction for catalog PK migration",
                                 QSqlError::UnknownError);

            err = executeSql(connectionName, QLatin1String(R"(
                CREATE TABLE catalog_new (
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
                    PRIMARY KEY(catalog_id),
                    UNIQUE(catalog_name)
                )
            )"));
            if (err.type() != QSqlError::NoError) { rollbackTransaction(connectionName); return err; }

            err = executeSql(connectionName, "INSERT INTO catalog_new SELECT * FROM catalog");
            if (err.type() != QSqlError::NoError) { rollbackTransaction(connectionName); return err; }

            err = executeSql(connectionName, "DROP TABLE catalog");
            if (err.type() != QSqlError::NoError) { rollbackTransaction(connectionName); return err; }

            err = executeSql(connectionName, "ALTER TABLE catalog_new RENAME TO catalog");
            if (err.type() != QSqlError::NoError) { rollbackTransaction(connectionName); return err; }

            if (!commitTransaction(connectionName)) {
                rollbackTransaction(connectionName);
                return QSqlError("transaction", "Failed to commit catalog PK migration",
                                 QSqlError::UnknownError);
            }
        } else if (dbType == DatabaseType::MySQL) {
            err = executeSql(connectionName, QLatin1String(R"(
                ALTER TABLE catalog
                    MODIFY catalog_id BIGINT NOT NULL,
                    DROP PRIMARY KEY,
                    ADD PRIMARY KEY (catalog_id),
                    ADD UNIQUE KEY (catalog_name(500))
            )"));
            if (err.type() != QSqlError::NoError) {
                qWarning() << "WARNING: Failed to migrate catalog PRIMARY KEY (MySQL):" << err.text();
                return err;
            }
        }
    }

    // --- Part 3: Add mapping_source_collection column to device_mapping ---
    QStringList mappingColumns = getTableColumns(connectionName, "device_mapping");
    if (!mappingColumns.contains("mapping_source_collection")) {
        QSqlError err = executeSql(connectionName,
            "ALTER TABLE device_mapping ADD COLUMN mapping_source_collection TEXT");
        if (err.type() != QSqlError::NoError) {
            qWarning() << "WARNING: Failed to add mapping_source_collection column:" << err.text();
            return err;
        }
    }

    return QSqlError();
}
//----------------------------------------------------------------------

QSqlError Database::runMigration_2_6(const QString &connectionName)
{

    // Check if column already exists
    QStringList existingColumns = getTableColumns(connectionName, "search");

    if (!existingColumns.contains("selected_device_ID_list")) {
        // Add the new column
        QSqlError addColumnError = executeSql(connectionName,
                                              "ALTER TABLE search ADD COLUMN selected_device_ID_list TEXT");

        if (addColumnError.type() != QSqlError::NoError) {
            qWarning() << "WARNING: Failed to add selected_device_ID_list column:" << addColumnError.text();
            return addColumnError;
        }


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

    } else {
    }

    return QSqlError(); // Success
}

QSqlError Database::runMigration_2_8(const QString &connectionName)
{

        // Step 1: Drop the unused metadata table if it exists
        if (auto dropError = dropTableIfExists(connectionName, "metadata");
            dropError.type() != QSqlError::NoError)
        {
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
                QString alterSQL = QString("ALTER TABLE file ADD COLUMN %1 %2").arg(columnName, columnType);
                if (auto addColumnError = executeSql(connectionName, alterSQL);
                    addColumnError.type() != QSqlError::NoError)
                {
                    return addColumnError;
                }
            }
        }

        // Step 3: Update "filetemp" table structure to match file table

        // Get existing filetemp columns
        QStringList existingFiletempColumns = getTableColumns(connectionName, "filetemp");

        // Add missing metadata columns to filetemp (same columns as file table)
        for (const auto& [columnName, columnType] : newFileColumns) {
            if (!existingFiletempColumns.contains(columnName)) {
                QString alterSQL = QString("ALTER TABLE filetemp ADD COLUMN %1 %2").arg(columnName, columnType);
                if (auto addColumnError = executeSql(connectionName, alterSQL);
                    addColumnError.type() != QSqlError::NoError)
                {
                    return addColumnError;
                }
            }
        }


        // qDebug() << "File types will be populated on-demand when each catalog is first used";
        // qDebug() << "This ensures fast startup and processes only catalogs you actually use";


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
                QString alterSQL = QString("ALTER TABLE search ADD COLUMN %1 %2 DEFAULT NULL").arg(columnName, columnType);
                if (auto addColumnError = executeSql(connectionName, alterSQL);
                    addColumnError.type() != QSqlError::NoError)
                {
                    return addColumnError;
                }
            }
        }

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
        } else {
        }


    return QSqlError(); // Success
}

QSqlError Database::runMigration_2_9(const QString &connectionName)
{


    // Step 1: Add checksum columns to file table

    const QList<QPair<QString, QString>> newFileColumns = {
        {"checksum_sha256", "TEXT"},
        {"checksum_extraction_date", "TEXT"}
    };

    // Get existing file columns
    QStringList existingFileColumns = getTableColumns(connectionName, "file");

    // Add missing checksum columns to file table
    for (const auto& [columnName, columnType] : newFileColumns) {
        if (!existingFileColumns.contains(columnName)) {
            QString alterSQL = QString("ALTER TABLE file ADD COLUMN %1 %2").arg(columnName, columnType);
            if (auto addColumnError = executeSql(connectionName, alterSQL);
                addColumnError.type() != QSqlError::NoError)
            {
                return addColumnError;
            }
        }
    }


    // Step 2: Add checksum columns to filetemp table

    // Get existing filetemp columns
    QStringList existingFiletempColumns = getTableColumns(connectionName, "filetemp");

    // Add missing checksum columns to filetemp (same columns as file table)
    for (const auto& [columnName, columnType] : newFileColumns) {
        if (!existingFiletempColumns.contains(columnName)) {
            QString alterSQL = QString("ALTER TABLE filetemp ADD COLUMN %1 %2").arg(columnName, columnType);
            if (auto addColumnError = executeSql(connectionName, alterSQL);
                addColumnError.type() != QSqlError::NoError)
            {
                return addColumnError;
            }
        }
    }



    // Step 3: Add checksum column to catalog table

    QStringList existingCatalogColumns = getTableColumns(connectionName, "catalog");

    if (!existingCatalogColumns.contains("catalog_include_checksum")) {
        QString alterSQL = "ALTER TABLE catalog ADD COLUMN catalog_include_checksum TEXT";
        if (auto addColumnError = executeSql(connectionName, alterSQL);
            addColumnError.type() != QSqlError::NoError)
        {
            return addColumnError;
        }
    }


    // Step 4: Normalize catalog_include_checksum (set default to 'None' for existing catalogs)

    QSqlQuery updateCatalogChecksumQuery(QSqlDatabase::database(connectionName));
    updateCatalogChecksumQuery.exec(R"(
        UPDATE catalog
        SET catalog_include_checksum = 'None'
        WHERE catalog_include_checksum IS NULL
           OR catalog_include_checksum = ''
    )");

    int updatedCatalogs = updateCatalogChecksumQuery.numRowsAffected();
    if (updatedCatalogs > 0) {
    } else {
    }


    // Step 5: Add checksum & new duplicate search columns to search table

    QStringList existingSearchColumns = getTableColumns(connectionName, "search");

    const QList<QPair<QString, QString>> newSearchColumns = {
        {"duplicates_checksum", "NUMERIC"},
        {"duplicates_checksum_equal", "NUMERIC"},
        {"differences_checksum", "NUMERIC"},
        {"differences_checksum_equal", "NUMERIC"},
        {"duplicates_compare_checked", "NUMERIC"},
        {"duplicates_device1_ID", "NUMERIC"},
        {"duplicates_device2_ID", "NUMERIC"}
    };

    for (const auto& [columnName, columnType] : newSearchColumns) {
        if (!existingSearchColumns.contains(columnName)) {
            QString alterSQL = QString("ALTER TABLE search ADD COLUMN %1 %2 DEFAULT NULL").arg(columnName, columnType);
            if (auto addColumnError = executeSql(connectionName, alterSQL);
                addColumnError.type() != QSqlError::NoError)
            {
                return addColumnError;
            }
        }
    }



    return QSqlError(); // Success
}

//----------------------------------------------------------------------
QSqlError Database::runMigration_2_10(const QString &connectionName)
{

    QStringList existingColumns = getTableColumns(connectionName, "device_mapping");

    if (!existingColumns.contains("mapping_strict_copy")) {
        QSqlError err = executeSql(connectionName,
            "ALTER TABLE device_mapping ADD COLUMN mapping_strict_copy INTEGER DEFAULT 1");
        if (err.type() != QSqlError::NoError) {
            qWarning() << "WARNING: Failed to add mapping_strict_copy column:" << err.text();
            return err;
        }
    } else {
    }

    if (!existingColumns.contains("mapping_conflict_mode")) {
        QSqlError err = executeSql(connectionName,
            "ALTER TABLE device_mapping ADD COLUMN mapping_conflict_mode TEXT DEFAULT 'RenameOldest'");
        if (err.type() != QSqlError::NoError) {
            qWarning() << "WARNING: Failed to add mapping_conflict_mode column:" << err.text();
            return err;
        }
    } else {
    }

    if (!existingColumns.contains("mapping_source_mode")) {
        QSqlError err = executeSql(connectionName,
            "ALTER TABLE device_mapping ADD COLUMN mapping_source_mode TEXT DEFAULT 'Catalog'");
        if (err.type() != QSqlError::NoError) {
            qWarning() << "WARNING: Failed to add mapping_source_mode column:" << err.text();
            return err;
        }
    } else {
    }

    QSqlError filterErr = executeSql(connectionName,
        getSQLCreateTableCatalogFilter(getDatabaseType(connectionName)));
    if (filterErr.type() != QSqlError::NoError) {
        qWarning() << "WARNING: Failed to create catalog_filter table:" << filterErr.text();
        return filterErr;
    }

    return QSqlError();
}

//----------------------------------------------------------------------
