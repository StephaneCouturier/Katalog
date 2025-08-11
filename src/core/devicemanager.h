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
// File Name:   devicemanager.h
// Purpose:     Manager for device hierarchy operations with stoppable support
// Description: Orchestrates device updates across the entire hierarchy,
//              coordinating with CatalogManager for catalog-specific operations
//              Supports Virtual, Storage, and Catalog device types
// Author:      Stephane Couturier
/////////////////////////////////////////////////////////////////////////////
*/

#ifndef DEVICEMANAGER_H
#define DEVICEMANAGER_H

#include "core/catalogprogressmanager.h"
#include "device.h"
#include "devicejobstoppable.h"
#include "catalogmanager.h"

#include <QObject>
#include <QTimer>
#include <QString>
#include <QList>

#pragma once

class CatalogProgressManager;

/**
 * @brief The DeviceManager class
 * Manages device hierarchy operations with full stoppable support
 * Coordinates with CatalogManager for catalog-specific operations
 * Handles Virtual, Storage, and Catalog device types hierarchically
 */
class DeviceManager : public QObject
{
    Q_OBJECT
    Q_PROPERTY(bool deviceOperationRunning READ deviceOperationRunning NOTIFY deviceOperationRunningChanged)
    Q_PROPERTY(int progress READ progress NOTIFY progressChanged)
    Q_PROPERTY(QString status READ status NOTIFY statusChanged)
    Q_PROPERTY(QString currentDeviceName READ currentDeviceName NOTIFY currentDeviceNameChanged)
    Q_PROPERTY(int hierarchyProgress READ hierarchyProgress NOTIFY hierarchyProgressChanged)
    Q_PROPERTY(int catalogProgress READ catalogProgress NOTIFY catalogProgressChanged)
    Q_PROPERTY(qint64 processedDevices READ processedDevices NOTIFY processedDevicesChanged)
    Q_PROPERTY(qint64 totalDevices READ totalDevices NOTIFY totalDevicesChanged)
    Q_PROPERTY(qint64 processedCatalogs READ processedCatalogs NOTIFY processedCatalogsChanged)
    Q_PROPERTY(qint64 totalCatalogs READ totalCatalogs NOTIFY totalCatalogsChanged)

public:
    explicit DeviceManager(QObject *parent = nullptr);
    ~DeviceManager();

    /**
     * @brief Start a device operation on the specified device hierarchy
     * @param deviceEngine The device job engine to use
     * @param rootDevice The root device to start processing from
     * @param operationType Type of operation (Update, Create, etc.)
     * @param databaseMode Database mode (Memory, File)
     * @param collectionFolder Collection folder path
     */
    void startDeviceOperation(DeviceJobStoppable* deviceEngine,
                              Device* rootDevice,
                              DeviceJobStoppable::OperationType operationType,
                              const QString& databaseMode,
                              const QString& collectionFolder,
                              CatalogManager* catalogManager);

    /**
     * @brief Set the catalog manager to use (from MainWindow)
     * @param catalogManager The existing catalog manager instance
     */
    void setCatalogManager(CatalogManager* catalogManager);

    /**
     * @brief Stop the current device operation
     */
    void stopDeviceOperation();

    /**
     * @brief Pause the current device operation
     */
    void pauseDeviceOperation();

    /**
     * @brief Resume the current device operation
     */
    void resumeDeviceOperation();

    // Property getters
    bool deviceOperationRunning() const { return m_deviceOperationRunning; }
    int progress() const { return m_progress; }
    QString status() const { return m_status; }
    QString currentDeviceName() const { return m_currentDeviceName; }
    int hierarchyProgress() const { return m_hierarchyProgress; }
    int catalogProgress() const { return m_catalogProgress; }
    qint64 processedDevices() const { return m_processedDevices; }
    qint64 totalDevices() const { return m_totalDevices; }
    qint64 processedCatalogs() const { return m_processedCatalogs; }
    qint64 totalCatalogs() const { return m_totalCatalogs; }

    /**
     * @brief Get access to the catalog manager for catalog-specific operations
     * @return Pointer to the catalog manager instance
     */
    CatalogManager* catalogManager() const { return m_catalogManager; }

    /**
     * @brief Check if device manager is paused
     * @return True if paused
     */
    bool isPaused() const { return m_isPaused; }

    /**
     * @brief Connect to existing CatalogProgressManager for status bar updates
     * @param catalogProgressManager The progress manager from MainWindow
     */
    void setCatalogProgressManager(CatalogProgressManager* catalogProgressManager);

signals:
    // Main operation signals
    void deviceOperationStarted();
    void deviceOperationCompleted(const QList<qint64>& results);
    void deviceOperationError(const QString& error);
    void deviceOperationCancelled();

    // Property change signals
    void deviceOperationRunningChanged();
    void progressChanged();
    void statusChanged();
    void currentDeviceNameChanged();
    void hierarchyProgressChanged();
    void catalogProgressChanged();
    void processedDevicesChanged();
    void totalDevicesChanged();
    void processedCatalogsChanged();
    void totalCatalogsChanged();

    // Detailed progress signals for UI integration
    void deviceHierarchyProgress(int processedDevices, int totalDevices);
    void catalogOperationProgress(int processedCatalogs, int totalCatalogs);
    void currentOperationChanged(const QString& deviceName, const QString& operation);

    void requestReportAllUpdates(Device* device, const QList<qint64>& results, const QString& updateType);

private slots:
    // Device job integration
    void onDeviceOperationStarted();
    void onDeviceOperationCompleted(const QList<qint64>& results);
    void onDeviceOperationError(const QString& error);
    void onDeviceOperationCancelled();
    void onDeviceHierarchyProgress(int processed, int total);
    void onDeviceCatalogProgress(int processed, int total);
    void onDeviceCurrentChanged(const QString& deviceName, const QString& operation);
    void onDeviceStatusUpdate(const QString& message);

    // Catalog manager integration
    void onCatalogManagerProgressChanged();
    void onCatalogManagerStatusChanged();

private:
    /**
     * @brief Setup connections for device job
     * @param deviceJob The device job to connect to
     */
    void connectDeviceJob(DeviceJobStoppable* deviceJob);

    /**
     * @brief Setup catalog manager integration
     */
    void setupCatalogManagerIntegration();

    /**
     * @brief Clean up current device job
     */
    void cleanupDeviceJob();

    // Property setters
    void setDeviceOperationRunning(bool running);
    void setProgress(int progress);
    void setStatus(const QString& status);
    void setCurrentDeviceName(const QString& name);
    void setHierarchyProgress(int progress);
    void setCatalogProgress(int progress);
    void setProcessedDevices(qint64 processed);
    void setTotalDevices(qint64 total);
    void setProcessedCatalogs(qint64 processed);
    void setTotalCatalogs(qint64 total);

    /**
     * @brief Calculate overall progress from hierarchy and catalog progress
     */
    void updateOverallProgress();

private:
    // Core managers
    CatalogManager* m_catalogManager = nullptr;
    CatalogProgressManager* m_catalogProgressManager = nullptr;
    DeviceJobStoppable* m_currentDeviceJob = nullptr;

    // Operation state
    bool m_deviceOperationRunning = false;
    bool m_isPaused = false;

    // Progress tracking
    int m_progress = 0;                    // Overall progress (0-100)
    int m_hierarchyProgress = 0;           // Device hierarchy progress (0-100)
    int m_catalogProgress = 0;             // Current catalog progress (0-100)

    // Status and current operation
    QString m_status;
    QString m_currentDeviceName;

    // Detailed counters
    qint64 m_processedDevices = 0;
    qint64 m_totalDevices = 0;
    qint64 m_processedCatalogs = 0;
    qint64 m_totalCatalogs = 0;

    // Integration state
    bool m_catalogManagerIntegrated = false;
};

#endif // DEVICEMANAGER_H
