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
// File Name:   devicejobstoppable.h
// Purpose:     Stoppable device hierarchy processing engine
// Description: Handles recursive device updates across the entire hierarchy
//              Preserves all business logic from Device::updateDevice()
//              Integrates with CatalogJobStoppable for catalog operations
//              Supports Virtual Groups, Virtual Devices, Storage, and Catalogs
// Author:      Stephane Couturier
/////////////////////////////////////////////////////////////////////////////
*/

#ifndef DEVICEJOBSTOPPABLE_H
#define DEVICEJOBSTOPPABLE_H

#pragma once

#include "device.h"
#include "catalogjobstoppable.h"
#include <QObject>
#include <QQueue>
#include <QList>
#include <QAtomicInt>
#include <QMutex>
#include <QDateTime>
#include <QString>

// Forward declarations
class CatalogManager;

/**
 * @brief The DeviceJobStoppable class
 * Handles hierarchical device operations with full stoppable support
 * Recursively processes device hierarchies while preserving business logic
 * Integrates with CatalogJobStoppable for catalog-specific operations
 */
class DeviceJobStoppable : public QObject
{
    Q_OBJECT

public:
    /**
     * @brief Operation types for device processing
     */
    enum OperationType {
        UpdateDevice = 0,    // Update existing device and hierarchy
        CreateDevice = 1,    // Create new device (future extension)
        DeleteDevice = 2     // Delete device (future extension)
    };

    explicit DeviceJobStoppable(QObject *parent = nullptr);
    ~DeviceJobStoppable();

    /**
     * @brief Start device operation on the specified hierarchy
     * @param rootDevice The root device to start processing from
     * @param operationType Type of operation to perform
     * @param databaseMode Database mode (Memory, File)
     * @param collectionFolder Collection folder path
     * @param catalogManager Catalog manager for catalog operations
     */
    void startDeviceOperation(Device* rootDevice,
                              OperationType operationType,
                              const QString& databaseMode,
                              const QString& collectionFolder,
                              CatalogManager* catalogManager);

    /**
     * @brief Stop the device operation immediately
     */
    void stopDeviceOperation();

    /**
     * @brief Pause the device operation
     */
    void pauseDeviceOperation();

    /**
     * @brief Resume the device operation
     */
    void resumeDeviceOperation();

    // Status queries
    bool isRunning() const { return m_operationRunning; }
    bool isPaused() const { return m_paused.loadAcquire(); }
    bool isStopped() const { return m_stopRequested.loadAcquire(); }

    // Progress queries
    int totalDevicesInHierarchy() const { return m_allDevicesInHierarchy.size(); }
    int processedDevices() const { return m_processedDevices; }
    int totalCatalogsInHierarchy() const { return m_allCatalogsInHierarchy.size(); }
    int processedCatalogs() const { return m_processedCatalogs; }

    // Current operation info
    QString currentDeviceName() const;
    QString currentOperation() const { return m_currentOperation; }
    Device* currentDevice() const { return m_currentDevice; }

    // Results
    QList<qint64> getAccumulatedResults() const { return m_accumulatedResults; }

signals:
    // Main operation lifecycle signals
    void deviceOperationStarted();
    void deviceOperationCompleted(const QList<qint64>& results);
    void deviceOperationError(const QString& error);
    void deviceOperationCancelled();

    // Progress reporting signals
    void hierarchyProgressChanged(int processedDevices, int totalDevices);
    void catalogProgressChanged(int processedCatalogs, int totalCatalogs);
    void currentDeviceChanged(const QString& deviceName, const QString& operation);

    // Detailed status updates
    void statusUpdate(const QString& message);
    void deviceProcessingStarted(const QString& deviceName, const QString& deviceType);
    void deviceProcessingCompleted(const QString& deviceName, const QList<qint64>& deviceResults);

private slots:
    // Integration with CatalogJobStoppable
    void onCatalogOperationCompleted();
    void onCatalogOperationError(const QString& error);
    void onCatalogOperationCancelled();
    void onCatalogProgressUpdate(qint64 filesProcessed, qint64 totalFiles, const QString& currentPath);

private:
    /**
     * @brief Main recursive device processing entry point
     * @param device Device to process
     */
    void processDevice(Device* device);

    /**
     * @brief Process children of the current device
     * @param device Parent device
     */
    void processChildren(Device* device);

    /**
     * @brief Continue processing the next device in queue
     */
    void processNextInQueue();

    // Type-specific processing methods
    /**
     * @brief Process a Virtual Group or Virtual Device
     * Fast operation: just aggregates numbers from children
     * @param device Virtual device to process
     */
    void processVirtualDevice(Device* device);

    /**
     * @brief Process a Storage Device
     * Fast operation: updates disk space info + processes catalog children
     * @param device Storage device to process
     */
    void processStorageDevice(Device* device);

    /**
     * @brief Process a Catalog Device
     * Heavy operation: delegates to CatalogJobStoppable for file indexing
     * @param device Catalog device to process
     */
    void processCatalogDevice(Device* device);

    // Hierarchy analysis and setup
    /**
     * @brief Analyze the device hierarchy and build processing plan
     * @param rootDevice Root device to analyze
     */
    void analyzeDeviceHierarchy(Device* rootDevice);

    /**
     * @brief Recursively build the processing queue
     * @param device Device to add to queue (with its children)
     */
    void buildProcessingQueue(Device* device);

    /**
     * @brief Load and validate device children
     * @param device Parent device
     */
    void loadDeviceChildren(Device* device);

    // Business logic preservation from Device::updateDevice()
    /**
     * @brief Update parent device numbers after children are processed
     * @param device Device whose parents need updating
     */
    void updateParentNumbers(Device* device);

    /**
     * @brief Update related devices (e.g., catalogs sharing same external ID)
     * @param device Device to find related devices for
     */
    void updateRelatedDevices(Device* device);

    /**
     * @brief Save device statistics
     * @param device Device to save statistics for
     */
    void saveDeviceStatistics(Device* device);

    /**
     * @brief Handle completion of a single device processing
     * @param device Device that was processed
     * @param deviceResults Results from device processing
     */
    void processDeviceCompleted(Device* device, const QList<qint64>& deviceResults);

    // Stoppable control
    /**
     * @brief Check if operation should continue
     * @return True if should continue, false if stop requested
     */
    bool shouldContinue() const;

    /**
     * @brief Wait while paused (with stop check)
     */
    void waitIfPaused();

    /**
     * @brief Check if valid to continue operation
     * @return True if object is valid and operation should continue
     */
    bool isValidToContinue() const;

    // Progress and status management
    /**
     * @brief Update overall progress based on processed devices/catalogs
     */
    void updateProgress();

    /**
     * @brief Emit status update with current operation info
     * @param operation Current operation description
     * @param deviceName Current device name
     */
    void emitStatusUpdate(const QString& operation, const QString& deviceName = QString());

    /**
     * @brief Accumulate results from device operations
     * @param deviceResults Results from a single device operation
     */
    void accumulateResults(const QList<qint64>& deviceResults);

    // Operation completion and cleanup
    /**
     * @brief Complete the entire device operation
     */
    void completeOperation();

    /**
     * @brief Handle operation error
     * @param error Error message
     */
    void handleOperationError(const QString& error);

    /**
     * @brief Handle operation cancellation
     */
    void handleOperationCancellation();

    /**
     * @brief Clean up operation state
     */
    void cleanupOperation();

private:
    // Atomic operation control
    QAtomicInt m_stopRequested{0};
    QAtomicInt m_paused{0};
    QAtomicInt m_objectValid{1};
    QMutex m_pauseMutex;

    // Operation state
    bool m_operationRunning = false;
    OperationType m_operationType = UpdateDevice;
    QString m_databaseMode;
    QString m_collectionFolder;
    QString m_currentOperation;

    // Hierarchy management
    Device* m_rootDevice = nullptr;
    Device* m_currentDevice = nullptr;
    QQueue<Device*> m_processingQueue;
    QList<Device*> m_allDevicesInHierarchy;
    QList<Device*> m_allCatalogsInHierarchy;

    // Progress tracking
    int m_processedDevices = 0;
    int m_processedCatalogs = 0;

    // Integration with catalog system
    CatalogManager* m_catalogManager = nullptr;
    CatalogJobStoppable* m_currentCatalogJob = nullptr;
    bool m_waitingForCatalogCompletion = false;

    // Results accumulation (following Device::updateDevice() format)
    QList<qint64> m_accumulatedResults;
    QList<qint64> m_globalCatalogResults;  // For batch catalog updates
    QList<qint64> m_globalStorageResults;  // For batch storage updates

    // Timing and throttling
    QDateTime m_lastProgressEmit;
    static const int PROGRESS_UPDATE_INTERVAL_MS = 100;

    // Error handling
    QString m_lastError;
    bool m_hasErrors = false;
};

#endif // DEVICEJOBSTOPPABLE_H
