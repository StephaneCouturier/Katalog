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
// File Name:   databasemanager.cpp
// Purpose:     Database connection lifecycle and migration orchestration
// Description: Shared by K2 (Qt Widgets) and K3 (Qt Quick). UI-agnostic:
//              no QWidget, no QDialog, no QMessageBox. Callers handle UI.
// Author:      Stephane Couturier
/////////////////////////////////////////////////////////////////////////////
*/

#include "core/databasemanager.h"
#include "core/database.h"
#include "core/collection.h"
#include <QSqlDatabase>
#include <QVersionNumber>

//----------------------------------------------------------------------
QSqlError DatabaseManager::connect(const QString &connectionName, Collection *collection)
{
    // Database::initialize() opens the connection, sets pragmas, and creates all tables.
    QSqlError err = Database::initialize(connectionName, collection);
    if (err.type() != QSqlError::NoError)
        return err;

    // In Memory mode, catalog data must be loaded from CSV/idx files.
    if (collection->databaseMode == "Memory") {
        collection->generateCollectionFilesPaths();
        collection->load();
    }

    return QSqlError();
}
//----------------------------------------------------------------------
void DatabaseManager::disconnect(const QString &connectionName)
{
    QSqlDatabase db = QSqlDatabase::database(connectionName, false);
    if (db.isOpen())
        db.close();
    QSqlDatabase::removeDatabase(connectionName);
}
//----------------------------------------------------------------------
QSqlError DatabaseManager::reconnect(const QString &connectionName, Collection *collection)
{
    disconnect(connectionName);
    return connect(connectionName, collection);
}
//----------------------------------------------------------------------
QSqlError DatabaseManager::runMigrations(const QString &connectionName, Collection *collection)
{
    const QVersionNumber schemaVersion =
        QVersionNumber::fromString(collection->loadDatabaseSchemaVersion());

    if (schemaVersion < QVersionNumber::fromString("2.6")) {
        collection->dbSchemaVersion = "2.6";
        QSqlError err = Database::runMigration_2_6(connectionName);
        if (err.type() != QSqlError::NoError) return err;
        collection->setDatabaseSchemaVersion();
        // NOTE: K2 must call migrateExistingSearchDeviceData_2_6() separately
        //       when schemaVersion was below 2.6 before this call.
    }

    if (schemaVersion < QVersionNumber::fromString("2.8")) {
        collection->dbSchemaVersion = "2.8";
        QSqlError err = Database::runMigration_2_8(connectionName);
        if (err.type() != QSqlError::NoError) return err;
        collection->setDatabaseSchemaVersion();
    }

    if (schemaVersion < QVersionNumber::fromString("2.9")) {
        collection->dbSchemaVersion = "2.9";
        QSqlError err = Database::runMigration_2_9(connectionName);
        if (err.type() != QSqlError::NoError) return err;
        collection->setDatabaseSchemaVersion();
    }

    if (schemaVersion < QVersionNumber::fromString("2.10")) {
        collection->dbSchemaVersion = "2.10";
        QSqlError err = Database::runMigration_2_10(connectionName);
        if (err.type() != QSqlError::NoError) return err;
        collection->setDatabaseSchemaVersion();
    }

    if (schemaVersion < QVersionNumber::fromString("2.11")) {
        collection->dbSchemaVersion = "2.11";
        QSqlError err = Database::runMigration_2_11(connectionName);
        if (err.type() != QSqlError::NoError) return err;
        collection->setDatabaseSchemaVersion();
    }

    if (schemaVersion < QVersionNumber::fromString("2.12")) {
        collection->dbSchemaVersion = "2.12";
        QSqlError err = Database::runMigration_2_12(connectionName);
        if (err.type() != QSqlError::NoError) return err;
        collection->setDatabaseSchemaVersion();
    }

    if (schemaVersion < QVersionNumber::fromString("2.13")) {
        collection->dbSchemaVersion = "2.13";
        QSqlError err = Database::runMigration_2_13(connectionName);
        if (err.type() != QSqlError::NoError) return err;
        collection->setDatabaseSchemaVersion();
    }

    // Unconditional column guard: mapping_source_collection was added to runMigration_2_11
    // after some databases had already been migrated to schema 2.11, so it may be absent
    // even when schemaVersion == "2.11".
    {
        QSqlError err = Database::ensureMappingSourceCollectionColumn(connectionName);
        if (err.type() != QSqlError::NoError) return err;
    }

    return QSqlError();
}
