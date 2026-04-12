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
// File Name:   mainwindow_ui_wrapper_storage.h
// Purpose:     UI wrapper for Storage class to maintain backward compatibility
// Description: Handles UI interactions and message boxes for storage operations
// Author:      Stephane Couturier
/////////////////////////////////////////////////////////////////////////////
*/
#ifndef MAINWINDOW_UI_WRAPPER_STORAGE_H
#define MAINWINDOW_UI_WRAPPER_STORAGE_H

#include "core/storage.h"
#include <QMessageBox>
#include <QCoreApplication>
#include <QApplication>

class StorageUIWrapper
{
public:
    // Static method to update storage info with UI feedback
    // Returns the old QList<qint64> format for backward compatibility
    static QList<qint64> updateStorageInfoWithUI(Storage* storage, bool reportStorageUpdate = true);

    // Static method to handle empty directory confirmation
    // Returns true if user wants to proceed despite empty directory
    static bool confirmEmptyDirectoryUpdate(Storage* storage);

private:
    // Helper method to show error messages to user
    static void showErrorMessage(const Storage::UpdateResult& result);

    // Helper method to convert UpdateResult to old QList<qint64> format
    static QList<qint64> convertToLegacyFormat(const Storage::UpdateResult& result);
};

#endif // MAINWINDOW_UI_WRAPPER_STORAGE_H
