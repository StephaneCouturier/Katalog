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
// File Name:   catalog_ui_wrapper.cpp
// Purpose:     UI wrapper for catalog operations
// Description: Implementation of UI-specific catalog operations
// Author:      Stephane Couturier
/////////////////////////////////////////////////////////////////////////////
*/

#include "mainwindow_ui_wrapper_catalog.h"
#include "mainwindow_ui_wrapper_device.h"

CatalogUIWrapper::ImportValidationResult CatalogUIWrapper::validateCatalogForImport(Catalog* catalog)
{
    ImportValidationResult result;
    result.isValid = true;
    result.needsUserAction = false;

    // Check for missing critical information (legacy catalogs)
    if (catalog->filePath == "not recorded" ||
        catalog->name == "not recorded" ||
        catalog->sourcePath == "not recorded") {

        result.isValid = false;
        result.needsUserAction = true;
        result.errorMessage = QCoreApplication::translate("MainWindow",
                                                          "It seems this catalog was not correctly imported or has an old format.<br/>"
                                                          "Edit it and make sure it has the following first 2 lines:<br/><br/>"
                                                          "<catalogSourcePath>/folderpath<br/>"
                                                          "<catalogFileCount>10000<br/><br/>"
                                                          "Copy/paste these lines at the beginning of the file and modify the values after the >:<br/>"
                                                          "- the catalogSourcePath is the folder to catalog the files from.<br/>"
                                                          "- the catalogFileCount number does not matter as much, it can be updated.<br/>");
        return result;
    }

    // Check for empty required fields
    if (catalog->filePath.isEmpty() || catalog->sourcePath.isEmpty()) {
        result.isValid = false;
        result.needsUserAction = true;
        result.errorMessage = QCoreApplication::translate("MainWindow",
                                                          "Select a catalog first (some info is missing).<br/> "
                                                          "currentCatalogFilePath: %1 <br/>"
                                                          "currentCatalogName: %2 <br/> "
                                                          "currentCatalogSourcePath: %3").arg(
                                      catalog->filePath, catalog->name, catalog->sourcePath);
        return result;
    }

    return result;
}

bool CatalogUIWrapper::confirmEmptyDirectoryAction(const QString& catalogName)
{
    QString message = QCoreApplication::translate("MainWindow",
                                                  "The source folder does not contain any file.<br/>This could mean that the source is empty or the device is not mounted to this folder.<br/>Do you want to save it anyway (the catalog would be empty)?");

    QMessageBox::StandardButton result = showConfirmationDialog(
        "Katalog",
        message,
        QMessageBox::Yes | QMessageBox::Cancel);

    return result == QMessageBox::Yes;
}

bool CatalogUIWrapper::confirmCannotUpdateCatalog(const QString& catalogName, const QString& sourcePath)
{
    QString message = QCoreApplication::translate("MainWindow",
                                                  "The catalog <b>%1</b> cannot be updated.<br/>"
                                                  "<br/> The source folder was not found.<br/><b>%2</b><br/>"
                                                  "<br/><br/> Possible reasons:<br/>"
                                                  "    - the device is not connected and mounted,<br/>"
                                                  "    - the source folder was moved or renamed.,<br/>"
                                                  "    - the source folder entered is incorrect.").arg(catalogName, sourcePath);

    showMessageBox(QMessageBox::Warning, "Katalog", message);
    return false; // This is just informational, not a confirmation
}

void CatalogUIWrapper::showCatalogUpdateError(const QString& filePath)
{
    QString message = QCoreApplication::translate("MainWindow", "Could not open file.");
    showMessageBox(QMessageBox::Information, "Katalog", message);
}

void CatalogUIWrapper::showInvalidCatalogError()
{
    // This will show the detailed message from validateCatalogForImport
    // The actual message is handled by the validation result
}

void CatalogUIWrapper::showMissingInfoError(const QString& filePath, const QString& name, const QString& sourcePath)
{
    QString message = QCoreApplication::translate("MainWindow",
                                                  "Select a catalog first (some info is missing).<br/> "
                                                  "currentCatalogFilePath: %1 <br/>"
                                                  "currentCatalogName: %2 <br/> "
                                                  "currentCatalogSourcePath: %3").arg(filePath, name, sourcePath);

    showMessageBox(QMessageBox::Information, "Katalog", message);
}

QMessageBox::StandardButton CatalogUIWrapper::showMessageBox(QMessageBox::Icon icon,
                                                             const QString& title,
                                                             const QString& text,
                                                             QMessageBox::StandardButtons buttons)
{
    QApplication::restoreOverrideCursor();
    QMessageBox msgBox;
    msgBox.setWindowTitle(title);
    msgBox.setText(text);
    msgBox.setIcon(icon);
    msgBox.setStandardButtons(buttons);
    int result = msgBox.exec();
    return static_cast<QMessageBox::StandardButton>(result);
}

QMessageBox::StandardButton CatalogUIWrapper::showConfirmationDialog(const QString& title,
                                                                     const QString& text,
                                                                     QMessageBox::StandardButtons buttons)
{
    return showMessageBox(QMessageBox::Question, title, text, buttons);
}

bool CatalogUIWrapper::saveCatalogToFileWithUI(Catalog* catalog,
                                               const QString& databaseMode,
                                               const QString& collectionFolder)
{
    bool success = catalog->saveCatalogToFile(databaseMode, collectionFolder);

    if (!success && databaseMode == "Memory") {
        showFileWriteError();
        return false;
    }

    return success;
}

bool CatalogUIWrapper::saveFoldersToFileWithUI(Catalog* catalog,
                                               const QString& databaseMode,
                                               const QString& collectionFolder)
{
    bool success = catalog->saveFoldersToFile(databaseMode, collectionFolder);

    if (!success && databaseMode == "Memory") {
        showFileWriteError();
        return false;
    }

    return success;
}

void CatalogUIWrapper::showFileWriteError()
{
    QMessageBox msgBox;
    msgBox.setWindowTitle("Katalog");
    msgBox.setText(QCoreApplication::translate("MainWindow", "Error opening output file."));
    msgBox.setIcon(QMessageBox::Warning);
    msgBox.exec();
}

bool CatalogUIWrapper::updateCatalogFileHeadersWithUI(Catalog* catalog, const QString& databaseMode)
{
    bool success = catalog->updateCatalogFileHeaders(databaseMode);

    if (!success && databaseMode == "Memory") {
        QMessageBox msgBox;
        msgBox.setWindowTitle("Katalog");
        msgBox.setText(QCoreApplication::translate("MainWindow", "Could not open file."));
        msgBox.setIcon(QMessageBox::Information);
        msgBox.exec();
        return false;
    }

    return success;
}

bool CatalogUIWrapper::deleteCatalogFileWithUI(Collection* collection, Device* device)
{
    Collection::DeleteCatalogResult result = collection->deleteCatalogFile(device);

    switch (result) {
    case Collection::DeleteSuccess:
        return true;

    case Collection::DeleteFailedToMoveToTrash:
        DeviceUIWrapper::showErrorMessage(QString("Failed to move catalog file to trash: %1")
                                              .arg(device->catalog->filePath));
        return false;

    case Collection::DeleteInvalidPath:
        DeviceUIWrapper::showErrorMessage(QCoreApplication::translate("MainWindow",
                                                                      "Select a catalog with a valid path."));
        return false;
    }

    return false;
}
