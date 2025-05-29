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
// File Name:   mainwindow_ui_wrapper_device.h
// Purpose:     UI wrapper for Device class to maintain backward compatibility
// Description: Handles UI interactions and message boxes for device operations
// Author:      Stephane Couturier
/////////////////////////////////////////////////////////////////////////////
*/

#ifndef MAINWINDOW_UI_WRAPPER_DEVICE_H
#define MAINWINDOW_UI_WRAPPER_DEVICE_H

#include "core/device.h"
#include <QList>

// Forward declarations
class QMessageBox;

class DeviceUIWrapper
{
public:
    // Static method to delete device with UI interactions
    static bool deleteDeviceWithUI(Device* device, bool askConfirmation = true);

    // Static method to update device with UI progress indicators
    static QList<qint64> updateDeviceWithUI(Device* device,
                                            QString statiticsRequestSource,
                                            QString databaseMode,
                                            bool reportStorageUpdate,
                                            QString collectionFolder,
                                            bool includeSubDevices);

private:
    // Helper methods
    static bool showConfirmationDialog(const QString& message);
    static void showErrorMessage(const QString& message);
    static void showWaitCursor();
    static void restoreCursor();
};

#endif // MAINWINDOW_UI_WRAPPER_DEVICE_H
