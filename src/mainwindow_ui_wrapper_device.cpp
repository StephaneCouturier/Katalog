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
// File Name:   mainwindow_ui_wrapper_device.cpp
// Purpose:     UI wrapper for Device class to maintain backward compatibility
// Description: Handles UI interactions and message boxes for device operations
// Author:      Stephane Couturier
/////////////////////////////////////////////////////////////////////////////
*/
#include "mainwindow_ui_wrapper_device.h"
#include <QMessageBox>
#include <QApplication>
#include <QCoreApplication>

bool DeviceUIWrapper::deleteDeviceWithUI(Device* device, bool askConfirmation)
{
    Device::DeleteOperationResult result = device->deleteDevice(askConfirmation);

    // Check for confirmation needed FIRST, before other cases
    if (result.needsConfirmation) {
        if (showConfirmationDialog(QCoreApplication::translate("MainWindow", result.confirmationMessage.toUtf8()))) {
            // User confirmed, try again without asking for confirmation
            return deleteDeviceWithUI(device, false);
        }
        return false; // User cancelled
    }

    // Now handle the actual results
    switch (result.result) {
    case Device::DeleteSuccess:
        return true;

    case Device::DeleteCancelled:
        return false;

    case Device::DeleteHasSubDevices:
        showErrorMessage(QCoreApplication::translate("MainWindow", result.errorMessage.toUtf8()));
        return false;

    case Device::DeleteError:
        showErrorMessage(QCoreApplication::translate("MainWindow", result.errorMessage.toUtf8()));
        return false;
    }

    return false;
}

QList<qint64> DeviceUIWrapper::updateDeviceWithUI(Device* device,
                                                  QString statiticsRequestSource,
                                                  QString databaseMode,
                                                  bool reportStorageUpdate,
                                                  QString collectionFolder,
                                                  bool includeSubDevices)
{
    // Create callbacks for UI interactions
    Device::UpdateCallbacks callbacks;
    callbacks.onStartUpdate = []() { showWaitCursor(); };
    callbacks.onFinishUpdate = []() { restoreCursor(); };

    return device->updateDevice(statiticsRequestSource,
                                databaseMode,
                                reportStorageUpdate,
                                collectionFolder,
                                includeSubDevices,
                                &callbacks);
}

bool DeviceUIWrapper::showConfirmationDialog(const QString& message)
{
    QMessageBox msgBox;
    msgBox.setWindowTitle("Katalog");
    //msgBox.setText(QCoreApplication::translate("MainWindow", message.toUtf8()));
    msgBox.setText(message);
    msgBox.setIcon(QMessageBox::Warning);
    msgBox.setStandardButtons(QMessageBox::Yes | QMessageBox::Cancel);

    return msgBox.exec() == QMessageBox::Yes;
}

void DeviceUIWrapper::showErrorMessage(const QString& message)
{
    QMessageBox msgBox;
    msgBox.setWindowTitle("Katalog");
    msgBox.setText(message);
    msgBox.setIcon(QMessageBox::Warning);
    msgBox.exec();
}

void DeviceUIWrapper::showWaitCursor()
{
    QApplication::setOverrideCursor(Qt::WaitCursor);
}

void DeviceUIWrapper::restoreCursor()
{
    QApplication::restoreOverrideCursor();
}
