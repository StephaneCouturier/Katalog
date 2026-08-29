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

    enum HostnameValidationType {
        Localhost,       // localhost, 127.x.x.x, ::1
        PrivateNetwork,  // 192.168.x.x, 10.x.x.x, 172.16-31.x.x
        PublicOrInvalid  // Public IPs, domains, or malformed input
    };

    static DatabaseType getDatabaseType(const QString &connectionName);

    // CREATE statements to manage compatibility with different database systems
    static QString getSQLCreateTableDevice(DatabaseType dbType);
    static QString getSQLCreateTableCatalog(DatabaseType dbType);
    static QString getSQLCreateTableStorage(DatabaseType dbType);
    static QString getSQLCreateTableFile(DatabaseType dbType);
    static QString getSQLCreateTableFileTemp(DatabaseType dbType);
    static QString getSQLCreateTableFolder(DatabaseType dbType);
    static QString getSQLCreateTableStatisticsDevice(DatabaseType dbType);
    static QString getSQLCreateTableSearch(DatabaseType databaseType);
    static QString getSQLCreateTableTag(DatabaseType dbType);
    static QString getSQLCreateTableParameter(DatabaseType dbType);
    static QString getSQLCreateTableBackupMapping(DatabaseType dbType);
    static QString getSQLCreateTableCatalogFilter(DatabaseType dbType);

    // Helper methods
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

    /**
     * @brief Validates a hostname for security - ensures database connections are local or on private networks
     * @param hostname The hostname to validate
     * @return HostnameValidationType indicating the security level
     */
    static HostnameValidationType validateHostname(const QString &hostname);

    static QString getFormattedTimeDifference(DatabaseType dbType,
                                              const QString &d1DateField,
                                              const QString &d2DateField);

    // Main initialization method - creates connection and sets up database
    // Optional overrides allow command line to bypass settings file values
    static QSqlError initialize(const QString &connectionName, Collection *collection,
                                const QString &overrideDatabaseMode = QString(),
                                const QString &overrideDatabaseFilePath = QString());
    // Create all database tables
    static QSqlError createAllTables(const QString &connectionName);
    static QString getSQLCreateIndexes(DatabaseType dbType);

    // Utility methods
    static bool tableExists(const QString &connectionName, const QString &tableName);
    static QStringList listTables(const QString &connectionName); // Make DB-aware

    // Updates per Version
    static QSqlError runMigration_2_6(const QString &connectionName);
    static QSqlError runMigration_2_8(const QString &connectionName);
    static QSqlError runMigration_2_9(const QString &connectionName);
    static QSqlError runMigration_2_10(const QString &connectionName);
    static QSqlError runMigration_2_11(const QString &connectionName);
    static QSqlError runMigration_2_12(const QString &connectionName);
    static QSqlError runMigration_2_13(const QString &connectionName);
    static QSqlError ensureMappingSourceCollectionColumn(const QString &connectionName);
    static QSqlError ensureDeviceCommentColumn(const QString &connectionName);

private:
    // Helper method to execute SQL with error checking
    static QSqlError executeSql(const QString &connectionName, const QString &sql);
    // Updates
    static QSqlError dropTableIfExists(const QString &connectionName, const QString &tableName);
    static QStringList getTableColumns(const QString &connectionName, const QString &tableName);

};

#endif // DATABASE_H
