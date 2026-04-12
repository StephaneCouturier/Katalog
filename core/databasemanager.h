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
// File Name:   databasemanager.h
// Purpose:     Database connection lifecycle and migration orchestration
// Description: Shared by K2 (Qt Widgets) and K3 (Qt Quick). UI-agnostic:
//              no QWidget, no QDialog, no QMessageBox. Callers handle UI.
// Author:      Stephane Couturier
/////////////////////////////////////////////////////////////////////////////
*/
#ifndef DATABASEMANAGER_H
#define DATABASEMANAGER_H

#include <QString>
#include <QSqlError>

class Collection;

class DatabaseManager
{
public:
    /**
     * @brief Open the database connection, create all tables, and load
     *        Memory mode data from CSV/idx files if needed.
     */
    static QSqlError connect(const QString &connectionName, Collection *collection);

    /**
     * @brief Close and remove the named database connection.
     */
    static void disconnect(const QString &connectionName);

    /**
     * @brief Full reconnect: disconnect then connect.
     */
    static QSqlError reconnect(const QString &connectionName, Collection *collection);

    /**
     * @brief Run all pending schema migrations in ascending version order.
     *        Returns the first QSqlError encountered; NoError if all succeeded.
     *        Callers are responsible for any UI feedback (busy cursor, dialogs).
     *        Note: the K2-specific search-device data step for 2.6
     *        (migrateExistingSearchDeviceData_2_6) must be called separately
     *        by K2 when schemaVersion was below 2.6 before this call.
     */
    static QSqlError runMigrations(const QString &connectionName, Collection *collection);
};

#endif // DATABASEMANAGER_H
