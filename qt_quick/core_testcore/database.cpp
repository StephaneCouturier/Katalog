// src/core/database.cpp
#include "database.h"
#include "collection.h"
#include <QSettings>
#include <QFile>
#include <QDebug>

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

    // ADD THIS: Set SQLite pragmas for consistent behavior
    if (db.driverName() == "QSQLITE") {
        QSqlQuery pragmaQuery(db);

        // Set synchronous mode (important for consistency)
        if (!pragmaQuery.exec("PRAGMA synchronous = NORMAL")) {
            qDebug() << "Failed to set synchronous pragma:" << pragmaQuery.lastError().text();
        }

        // Set journal mode (important for memory databases)
        if (!pragmaQuery.exec("PRAGMA journal_mode = MEMORY")) {
            qDebug() << "Failed to set journal mode:" << pragmaQuery.lastError().text();
        }

        // Ensure foreign keys are enabled
        if (!pragmaQuery.exec("PRAGMA foreign_keys = ON")) {
            qDebug() << "Failed to enable foreign keys:" << pragmaQuery.lastError().text();
        }

        // Set temp store to memory for better performance
        if (!pragmaQuery.exec("PRAGMA temp_store = MEMORY")) {
            qDebug() << "Failed to set temp store:" << pragmaQuery.lastError().text();
        }

        qDebug() << "SQLite pragmas set successfully";
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

    error = createMetadataTable(connectionName);
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

QSqlError Database::createMetadataTable(const QString &connectionName)
{
    return executeSql(connectionName, DatabaseSQL::SQL_CREATE_METADATA);
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
