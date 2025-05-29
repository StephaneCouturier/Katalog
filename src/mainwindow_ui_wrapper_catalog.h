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
// File Name:   catalog_ui_wrapper.h
// Purpose:     UI wrapper for catalog operations to separate UI from business logic
// Description: Handles all UI interactions for catalog operations
// Author:      Stephane Couturier
/////////////////////////////////////////////////////////////////////////////
*/

#ifndef MAINWINDOW_UI_WRAPPER_CATALOG_H
#define MAINWINDOW_UI_WRAPPER_CATALOG_H

#include "catalog.h"
#include <QMessageBox>
#include <QApplication>
#include <QCoreApplication>

class CatalogUIWrapper
{
public:
    // Result structures for update operations
    struct UpdateResult {
        bool wasUpdated;
        qint64 newFileCount;
        qint64 deltaFileCount;
        qint64 newTotalFileSize;
        qint64 deltaTotalFileSize;
        QString errorMessage;
        bool userCancelled;
    };

    struct ImportValidationResult {
        bool isValid;
        QString errorMessage;
        bool needsUserAction;
    };

    // Static methods for UI-involved operations
    static UpdateResult updateCatalogFilesWithUI(Catalog* catalog,
                                                 const QString& databaseMode,
                                                 const QString& collectionFolder,
                                                 bool reportCannotUpdate = true);

    static ImportValidationResult validateCatalogForImport(Catalog* catalog);

    static bool confirmEmptyDirectoryAction(const QString& catalogName);

    static bool confirmCannotUpdateCatalog(const QString& catalogName,
                                           const QString& sourcePath);

    static void showCatalogUpdateError(const QString& filePath);

    static void showInvalidCatalogError();

    static void showMissingInfoError(const QString& filePath,
                                     const QString& name,
                                     const QString& sourcePath);

private:
    // Helper methods
    static QMessageBox::StandardButton showMessageBox(QMessageBox::Icon icon,
                                                      const QString& title,
                                                      const QString& text,
                                                      QMessageBox::StandardButtons buttons = QMessageBox::Ok);

    static QMessageBox::StandardButton showConfirmationDialog(const QString& title,
                                                              const QString& text,
                                                              QMessageBox::StandardButtons buttons = QMessageBox::Yes | QMessageBox::Cancel);
};

#endif // MAINWINDOW_UI_WRAPPER_CATALOG_H
