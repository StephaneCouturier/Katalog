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
// File Name:   storage_ui_wrapper.cpp
// Purpose:     UI wrapper for Storage class to maintain backward compatibility
// Description: Handles UI interactions and message boxes for storage operations
// Author:      Stephane Couturier
/////////////////////////////////////////////////////////////////////////////
*/

#include "mainwindow_ui_wrapper_storage.h"
#include <QMessageBox>
#include <QCoreApplication>
#include <QApplication>

QList<qint64> StorageUIWrapper::updateStorageInfoWithUI(Storage* storage, bool reportStorageUpdate)
{
    Storage::UpdateResult result = storage->updateStorageInfo();

    // Handle different error cases with UI feedback
    if (!result.wasUpdated) {
        if (reportStorageUpdate) {
            showErrorMessage(result);
        }

        // Return the old format list with zeros for compatibility
        return convertToLegacyFormat(result);
    }

    // Success case - convert to old format
    return convertToLegacyFormat(result);
}

bool StorageUIWrapper::confirmEmptyDirectoryUpdate(Storage* storage)
{
    Storage::UpdateResult result = storage->updateStorageInfo();

    if (result.errorCode == Storage::ErrorEmptyDirectory) {
        QMessageBox msgBox;
        msgBox.setWindowTitle("Katalog");
        msgBox.setText(QCoreApplication::translate("MainWindow",
                                                   "Storage: <b>'%1'</b><br/><br/>"
                                                   "The source folder does not contain any file:<br/><b>'%2'</b><br/><br/>"
                                                   "This could mean that the device is not mounted to this folder,<br/>"
                                                   "or the folder is simply empty.<br/><br/>"
                                                   "Force trying to get values anyhow?").arg(storage->name, storage->path)
                       );
        msgBox.setIcon(QMessageBox::Warning);
        msgBox.setStandardButtons(QMessageBox::Ok | QMessageBox::Cancel);
        msgBox.setDefaultButton(QMessageBox::Cancel);

        return msgBox.exec() == QMessageBox::Ok;
    }

    return true; // No confirmation needed for other cases
}

void StorageUIWrapper::showErrorMessage(const Storage::UpdateResult& result)
{
    QApplication::restoreOverrideCursor();

    QMessageBox msgBox;
    msgBox.setWindowTitle("Katalog");
    msgBox.setIcon(QMessageBox::Warning);

    switch (result.errorCode) {
    case Storage::ErrorNoPath:
        msgBox.setText(QCoreApplication::translate("MainWindow", result.errorMessage.toUtf8()));
        break;

    case Storage::ErrorEmptyDirectory:
        // This case might need special handling - could show confirmation dialog
        msgBox.setText(QCoreApplication::translate("MainWindow", result.errorMessage.toUtf8()));
        break;

    case Storage::ErrorCannotGetValues:
        msgBox.setText(QCoreApplication::translate("MainWindow", result.errorMessage.toUtf8()));
        break;

    case Storage::ErrorNotUpdated:
        // Usually not shown to user, this is an internal state
        return;

    default:
        msgBox.setText("Unknown error occurred during storage update.");
        break;
    }

    msgBox.exec();
}

QList<qint64> StorageUIWrapper::convertToLegacyFormat(const Storage::UpdateResult& result)
{
    QList<qint64> list;

    if (!result.wasUpdated) {
        // Return the old format list with zeros for compatibility
        list.append(0); // not updated
        for (int i = 0; i < 6; ++i) {
            list.append(0);
        }
    } else {
        // Success case - convert to old format
        list.append(1); // updated
        list.append(result.newUsedSpace);
        list.append(result.deltaUsedSpace);
        list.append(result.newFreeSpace);
        list.append(result.deltaFreeSpace);
        list.append(result.newTotalSpace);
        list.append(result.deltaTotalSpace);
    }

    return list;
}
