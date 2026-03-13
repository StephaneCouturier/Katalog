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
// File Name:   devicejobstoppable.cpp
// Purpose:     Stoppable device hierarchy processing engine implementation
// Description: Handles recursive device updates across the entire hierarchy
//              Preserves all business logic from Device::updateDevice()
//              Integrates with CatalogJobStoppable for catalog operations
// Author:      Stephane Couturier
/////////////////////////////////////////////////////////////////////////////
*/

#include "devicejobstoppable.h"
#include "catalogmanager.h"
#include <QDebug>
#include <QThread>
#include <QMutexLocker>
#include <QSqlQuery>
#include <QSqlDatabase>

DeviceJobStoppable::DeviceJobStoppable(QObject *parent)
    : QObject(parent)
{
}

DeviceJobStoppable::~DeviceJobStoppable()
{
    m_objectValid.storeRelease(0);
    cleanupOperation();
}

void DeviceJobStoppable::startDeviceOperation(Device* rootDevice,
                                              OperationType operationType,
                                              const QString& databaseMode,
                                              const QString& collectionFolder,
                                              CatalogManager* catalogManager)
{

    if (!rootDevice) {
        handleOperationError("Invalid root device provided");
        return;
    }

    if (!catalogManager) {
        handleOperationError("Catalog manager not available");
        return;
    }

    if (m_operationRunning) {
        handleOperationError("Device operation already running");
        return;
    }

    // Initialize operation state
    m_operationRunning = true;
    m_rootDevice = rootDevice;
    m_operationType = operationType;
    m_databaseMode = databaseMode;
    m_collectionFolder = collectionFolder;
    m_catalogManager = catalogManager;

    // Reset control flags
    m_stopRequested.storeRelease(0);
    m_paused.storeRelease(0);
    m_gentleStopRequested.storeRelease(0);
    m_objectValid.storeRelease(1);

    // Reset progress tracking
    m_processedDevices = 0;
    m_processedCatalogs = 0;
    m_currentDevice = nullptr;
    m_waitingForCatalogCompletion = false;
    m_hasErrors = false;

    // Clear previous state
    m_processingQueue.clear();
    m_allDevicesInHierarchy.clear();
    m_allCatalogsInHierarchy.clear();
    m_accumulatedResults.clear();
    m_globalCatalogResults.clear();
    m_globalStorageResults.clear();


    // Analyze hierarchy and build processing plan
    analyzeDeviceHierarchy(m_rootDevice);


    emit deviceOperationStarted();

    // Start processing
    if (!m_processingQueue.isEmpty()) {
        processNextInQueue();
    } else {
        completeOperation();
    }
}

void DeviceJobStoppable::stopDeviceOperation()
{
    m_stopRequested.storeRelease(1);

    // Stop current catalog operation if running
    if (m_currentCatalogJob) {
        m_currentCatalogJob->stopCatalogOperation();
    }
}

void DeviceJobStoppable::pauseDeviceOperation()
{
    m_paused.storeRelease(1);

    // Pause current catalog operation if running
    if (m_currentCatalogJob) {
        m_currentCatalogJob->pauseCatalogOperation();
    }
}

void DeviceJobStoppable::resumeDeviceOperation()
{
    m_paused.storeRelease(0);

    // Resume current catalog operation if running
    if (m_currentCatalogJob) {
        m_currentCatalogJob->resumeCatalogOperation();
    }
}

QString DeviceJobStoppable::currentDeviceName() const
{
    return m_currentDevice ? m_currentDevice->name : QString();
}

void DeviceJobStoppable::analyzeDeviceHierarchy(Device* rootDevice)
{

    // Recursively build the processing queue and analyze hierarchy
    buildProcessingQueue(rootDevice);

}

void DeviceJobStoppable::buildProcessingQueue(Device* device)
{
    if (!device) return;

    // Add this device to our tracking lists
    m_allDevicesInHierarchy.append(device);
    m_processingQueue.enqueue(device);

    // Track catalogs separately for progress reporting
    if (device->type == "Catalog") {
        m_allCatalogsInHierarchy.append(device);
    }


    // Load children using public method
    loadDeviceChildren(device);

    // Recursively process children
    // FIX: subDevices is QList<Device>, not QList<Device*>
    for (const Device& childDevice : device->subDevices) {
        // Create a copy of the child device for processing
        Device* childDevicePtr = new Device(childDevice);
        buildProcessingQueue(childDevicePtr);
    }
}

void DeviceJobStoppable::loadDeviceChildren(Device* device)
{
    if (!device) return;

    try {
        // Load device which populates deviceIDList
        device->loadDevice(m_connectionName);

        // Create actual Device objects from deviceIDList
        device->subDevices.clear();
        for (int childID : device->deviceIDList) {
            Device childDevice;
            childDevice.ID = childID;
            childDevice.loadDevice(m_connectionName);
            device->subDevices.append(childDevice);
        }

    } catch (const std::exception& e) {
        qWarning() << "WARNING: Error loading children for device" << device->name << ":" << e.what();
    }
}

void DeviceJobStoppable::processNextInQueue()
{

    if (!shouldContinue()) {
        qWarning() << "WARNING: Stop requested, aborting queue processing";
        handleOperationCancellation();
        return;
    }

    // Check for gentle stop before starting new device (but not for single catalog creation)
    if (m_gentleStopRequested.loadAcquire() && m_operationType == UpdateDevice) {
        completeOperation();
        return;
    }

    waitIfPaused();

    if (m_processingQueue.isEmpty()) {
        completeOperation();
        return;
    }

    Device* nextDevice = m_processingQueue.dequeue();
    m_currentDevice = nextDevice;


    emit deviceProcessingStarted(nextDevice->name, nextDevice->type);
    updateProgress();

    // Process the device based on its type
    processDevice(nextDevice);
}

void DeviceJobStoppable::processDevice(Device* device)
{
    if (!device || !shouldContinue()) return;


    // Update device active state (same as original logic)
    device->updateActiveState(m_connectionName);
    device->dateTimeUpdated = QDateTime::currentDateTime();

    // Type-specific processing
    if (device->type == "Virtual") {
        processVirtualDevice(device);
    } else if (device->type == "Storage") {
        processStorageDevice(device);
    } else if (device->type == "Catalog") {
        processCatalogDevice(device);
        return; // Async operation - continues in onCatalogOperationCompleted()
    } else {
        processVirtualDevice(device);
    }

    // For non-catalog devices, immediately continue to next
    processDeviceCompleted(device, QList<qint64>());
}

void DeviceJobStoppable::processVirtualDevice(Device* device)
{

    emitStatusUpdate("Updating virtual device", device->name);

    // Virtual devices just aggregate numbers from their children
    // The actual aggregation happens after all children are processed
    // via updateParentNumbers() calls

    // Save device state
    device->saveDevice();

}

void DeviceJobStoppable::processStorageDevice(Device* device)
{

    emitStatusUpdate("Updating storage device", device->name);

    // Fast operation: Update storage space information
    if (device->storage) {

        // Update storage path from device path (same as Device::updateDevice logic)
        device->storage->path = device->path;

        // Update both storage table and device table automatically
        Storage::UpdateResult result = device->storage->updateStorageInfo();

        if (result.wasUpdated) {

            // Update device values with new storage information (for consistency)
            device->totalSpace = result.newTotalSpace;
            device->freeSpace = result.newFreeSpace;

            // Store the deltas for reporting
            m_storageUpdateResult = result;

        } else {
            // Store empty result for reporting
            m_storageUpdateResult = Storage::UpdateResult();
        }
    }

    // NOTE: No need to call saveDevice() or insertStorage()
    // because updateStorageInfo() already saved to database

    // Save statistics (same as Device::updateDevice logic)
    saveDeviceStatistics(device);

}

void DeviceJobStoppable::processCatalogDevice(Device* device)
{

    if (!device->catalog) {
        handleOperationError(QString("Invalid catalog for device: %1").arg(device->name));
        return;
    }

    // Check if catalog is active. If not skip it and continue
    if (!device->active) {

        // Create a "skipped" result for reporting
        QList<qint64> skippedResults;
        skippedResults << 0;  // 0 = skipped (not success=1, not error=-1)
        skippedResults << 0;  // No files processed
        skippedResults << 0;  // No delta files
        skippedResults << 0;  // No size
        skippedResults << 0;  // No delta size
        for (int i = 5; i < 14; ++i) skippedResults << 0; // Padding

        // Mark as processed and continue to next device
        processDeviceCompleted(device, skippedResults);
        return;
    }

    emitStatusUpdate("Updating catalog", device->name);

    // Set waiting flag
    m_waitingForCatalogCompletion = true;

    // Pass device values for catalog operations (same as Device::updateDevice logic)
    device->catalog->name = device->name;
    device->catalog->sourcePath = device->path;

    // Create catalog job for this specific catalog
    m_currentCatalogJob = new CatalogJobStoppable(this);

    // FIX: Connect to correct catalog operation signals
    connect(m_currentCatalogJob, &CatalogJobStoppable::catalogOperationFinished,
            this, &DeviceJobStoppable::onCatalogOperationCompleted);
    connect(m_currentCatalogJob, &CatalogJobStoppable::catalogOperationError,
            this, &DeviceJobStoppable::onCatalogOperationError);
    connect(m_currentCatalogJob, &CatalogJobStoppable::catalogProgress,
            this, &DeviceJobStoppable::onCatalogProgressUpdate);


    // Start catalog operation through existing CatalogManager
    m_catalogManager->startCatalogJobStoppable(
        m_currentCatalogJob,
        device,
        CatalogJobStoppable::UpdateCatalog,
        m_databaseMode,
        m_collectionFolder
        );

}

void DeviceJobStoppable::onCatalogOperationCompleted()
{

    if (!m_currentCatalogJob || !m_currentDevice) {
        qWarning() << "WARNING: Catalog operation completed but no current job/device";
        return;
    }

    // Get catalog results
    QList<qint64> catalogResults;
    catalogResults << 1;  // Success flag
    catalogResults << m_currentCatalogJob->filesProcessed;
    catalogResults << m_currentCatalogJob->countedTotalFiles;
    catalogResults << 0;  // Delta files (placeholder)
    catalogResults << 0;  // Delta size (placeholder)


    // Update device with catalog results (same as Device::updateDevice logic)
    if (catalogResults.count() > 0 && catalogResults[0] == 1) {
        m_currentDevice->totalFileCount = m_currentCatalogJob->filesProcessed;
        m_currentDevice->totalFileSize = m_currentCatalogJob->countedTotalFiles;

        // Save device and statistics
        m_currentDevice->saveDevice();
        saveDeviceStatistics(m_currentDevice);

        // Update related devices (same as Device::updateDevice logic)
        updateRelatedDevices(m_currentDevice);
    }

    // Clean up catalog job
    m_currentCatalogJob->setParent(nullptr);
    m_currentCatalogJob->deleteLater();
    m_currentCatalogJob = nullptr;
    m_waitingForCatalogCompletion = false;

    // Use a timer to ensure CatalogManager cleanup is complete
    // before continuing to the next device
    QTimer::singleShot(100, this, [this, catalogResults]() {
        if (!shouldContinue()) {
            handleOperationCancellation();
            return;
        }

        // Verify CatalogManager is ready for next operation
        if (m_catalogManager && m_catalogManager->catalogOperationRunning()) {
            // Try again after a longer delay
            QTimer::singleShot(500, this, [this, catalogResults]() {
                processDeviceCompleted(m_currentDevice, catalogResults);
            });
        } else {
            processDeviceCompleted(m_currentDevice, catalogResults);
        }
    });
}

void DeviceJobStoppable::onCatalogOperationError(const QString& error)
{

    // Clean up catalog job
    if (m_currentCatalogJob) {
        m_currentCatalogJob->setParent(nullptr);
        m_currentCatalogJob->deleteLater();
        m_currentCatalogJob = nullptr;
    }
    m_waitingForCatalogCompletion = false;

    // Handle the error
    handleOperationError(QString("Catalog operation failed: %1").arg(error));
}

void DeviceJobStoppable::onCatalogOperationCancelled()
{

    // Clean up catalog job
    if (m_currentCatalogJob) {
        m_currentCatalogJob->setParent(nullptr);
        m_currentCatalogJob->deleteLater();
        m_currentCatalogJob = nullptr;
    }
    m_waitingForCatalogCompletion = false;

    // Handle cancellation
    handleOperationCancellation();
}

void DeviceJobStoppable::onCatalogProgressUpdate(qint64 filesProcessed, qint64 totalFiles, const QString& currentPath)
{
    Q_UNUSED(currentPath);

    // Update catalog progress
    if (totalFiles > 0) {
        int catalogPercent = qMin(100, static_cast<int>((filesProcessed * 100) / totalFiles));
        emit catalogProgressChanged(m_processedCatalogs + catalogPercent, totalCatalogsInHierarchy() * 100);
    }

    updateProgress();
}

void DeviceJobStoppable::processDeviceCompleted(Device* device, const QList<qint64>& deviceResults)
{

    // Accumulate results
    accumulateResults(deviceResults);

    // Update progress
    m_processedDevices++;
    if (device->type == "Catalog") {
        m_processedCatalogs++;
    }

    emit deviceProcessingCompleted(device->name, deviceResults);
    updateProgress();

    // Continue to next device
    processNextInQueue();
}

void DeviceJobStoppable::updateParentNumbers(Device* device)
{
    if (!device) return;

    // This preserves the critical business logic from Device::updateDevice()
    // that was missing in the new async system


    try {
        // Update parent device numbers (same as Device::updateDevice logic)
        device->updateParentsNumbers();

    } catch (const std::exception& e) {
        qWarning() << "WARNING: Error updating parent numbers for" << device->name << ":" << e.what();
    }
}

void DeviceJobStoppable::updateRelatedDevices(Device* device)
{
    if (!device || device->type != "Catalog") return;


    // Update related devices (same logic as Device::updateDevice)
    // This handles catalogs that share the same external ID
    try {
        QSqlQuery queryRelatedDevice(QSqlDatabase::database(m_connectionName));
        QString queryRelatedDeviceSQL = R"(
            SELECT device_id
            FROM device
            WHERE device_external_id = :device_external_id
            AND device_type = 'Catalog'
            AND device_id != :device_id
        )";

        queryRelatedDevice.prepare(queryRelatedDeviceSQL);
        queryRelatedDevice.bindValue(":device_external_id", device->externalID);
        queryRelatedDevice.bindValue(":device_id", device->ID);
        queryRelatedDevice.exec();

        while (queryRelatedDevice.next()) {
            Device relatedDevice;
            relatedDevice.ID = queryRelatedDevice.value(0).toInt();
            relatedDevice.loadDevice(m_connectionName);
            relatedDevice.totalFileCount = device->totalFileCount;
            relatedDevice.totalFileSize = device->totalFileSize;
            relatedDevice.saveDevice();
            relatedDevice.updateParentsNumbers();

        }
    } catch (const std::exception& e) {
        qWarning() << "WARNING: Error updating related devices:" << e.what();
    }
}

void DeviceJobStoppable::saveDeviceStatistics(Device* device)
{
    if (!device) return;

    try {
        // Save statistics (same as Device::updateDevice logic)
        device->saveStatistics(device->dateTimeUpdated, "update");
    } catch (const std::exception& e) {
        qWarning() << "WARNING: Error saving statistics for" << device->name << ":" << e.what();
    }
}

bool DeviceJobStoppable::shouldContinue() const
{
    return m_objectValid.loadAcquire() && !m_stopRequested.loadAcquire();
}

void DeviceJobStoppable::waitIfPaused()
{
    while (m_paused.loadAcquire() && shouldContinue()) {
        QMutexLocker locker(&m_pauseMutex);
        QThread::msleep(100);
    }
}

bool DeviceJobStoppable::isValidToContinue() const
{
    return shouldContinue() && m_operationRunning;
}

void DeviceJobStoppable::updateProgress()
{
    // Emit hierarchy progress
    emit hierarchyProgressChanged(m_processedDevices, totalDevicesInHierarchy());

    // Emit catalog progress
    emit catalogProgressChanged(m_processedCatalogs, totalCatalogsInHierarchy());

    // Throttle progress updates
    QDateTime now = QDateTime::currentDateTime();
    if (m_lastProgressEmit.isValid() &&
        m_lastProgressEmit.msecsTo(now) < PROGRESS_UPDATE_INTERVAL_MS) {
        return;
    }
    m_lastProgressEmit = now;
}

void DeviceJobStoppable::emitStatusUpdate(const QString& operation, const QString& deviceName)
{
    m_currentOperation = operation;
    QString message = deviceName.isEmpty() ? operation : QString("%1: %2").arg(operation, deviceName);
    emit statusUpdate(message);
    emit currentDeviceChanged(deviceName, operation);
}

void DeviceJobStoppable::accumulateResults(const QList<qint64>& deviceResults)
{
    // Accumulate results in the same format as Device::updateDevice()
    if (deviceResults.isEmpty()) return;

    // Initialize accumulated results if empty
    if (m_accumulatedResults.isEmpty()) {
        #if (QT_VERSION < QT_VERSION_CHECK(6, 0, 0))
        QVector<qint64> tempVector(deviceResults.size(), 0);
        m_accumulatedResults = tempVector.toList();
        #else
        m_accumulatedResults = QList<qint64>(deviceResults.size(), 0);
        #endif
    }

    // Add device results to accumulated totals
    for (int i = 0; i < qMin(deviceResults.size(), m_accumulatedResults.size()); ++i) {
        m_accumulatedResults[i] += deviceResults[i];
    }
}

void DeviceJobStoppable::completeOperation()
{

    // Update parent numbers after all devices in hierarchy are processed
    if (m_rootDevice) {
        try {
            // First, reload the root device to get latest data from database
            m_rootDevice->loadDevice(m_connectionName);

            // Update numbers from children (aggregates from child devices)
            m_rootDevice->updateNumbersFromChildren();

            // Save the updated device
            m_rootDevice->saveDevice();

            // Update parent numbers starting from the root device
            // This will recursively update all parents in the hierarchy
            m_rootDevice->updateParentsNumbers();


        } catch (const std::exception& e) {
            qWarning() << "WARNING: Error updating parent numbers for hierarchy:" << e.what();
        }
    }

    m_operationRunning = false;

    // Emit completion with accumulated results
    emit deviceOperationCompleted(m_accumulatedResults);

    cleanupOperation();
}

void DeviceJobStoppable::handleOperationError(const QString& error)
{

    m_lastError = error;
    m_hasErrors = true;
    m_operationRunning = false;

    emit deviceOperationError(error);

    cleanupOperation();
}

void DeviceJobStoppable::handleOperationCancellation()
{

    m_operationRunning = false;

    emit deviceOperationCancelled();

    cleanupOperation();
}

void DeviceJobStoppable::cleanupOperation()
{

    // Clean up catalog job if still active
    if (m_currentCatalogJob) {
        m_currentCatalogJob->setParent(nullptr);
        m_currentCatalogJob->deleteLater();
        m_currentCatalogJob = nullptr;
    }

    // Clear state
    m_waitingForCatalogCompletion = false;
    m_currentDevice = nullptr;
    m_rootDevice = nullptr;
    m_catalogManager = nullptr;

    // Clear collections
    m_processingQueue.clear();
    m_allDevicesInHierarchy.clear();
    m_allCatalogsInHierarchy.clear();
}

void DeviceJobStoppable::requestGentleStop()
{
    m_gentleStopRequested.storeRelease(1);
}

