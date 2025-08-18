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
// File Name:   devicemanager.cpp
// Purpose:     Manager for device hierarchy operations with stoppable support
// Description: Orchestrates device updates across the entire hierarchy,
//              coordinating with CatalogManager for catalog-specific operations
//              Provides Qt property interface for UI integration
// Author:      Stephane Couturier
/////////////////////////////////////////////////////////////////////////////
*/

#include "devicemanager.h"
#include "devicejobstoppable.h"
#include <QDebug>

DeviceManager::DeviceManager(QObject *parent)
    : QObject(parent)
{
    qDebug() << "DeviceManager created";

    // Setup catalog manager integration
    setupCatalogManagerIntegration();

    qDebug() << "DeviceManager initialization complete";
}

DeviceManager::~DeviceManager()
{
    qDebug() << "DeviceManager destructor";
    cleanupDeviceJob();
}

void DeviceManager::startDeviceOperation(DeviceJobStoppable* deviceEngine,
                                         Device* rootDevice,
                                         DeviceJobStoppable::OperationType operationType,
                                         const QString& databaseMode,
                                         const QString& collectionFolder,
                                         CatalogManager* catalogManager)
{
    qDebug() << "=== DeviceManager::startDeviceOperation() ===";
    qDebug() << "Root device:" << (rootDevice ? rootDevice->name : "NULL");
    qDebug() << "Operation type:" << operationType;
    qDebug() << "Database mode:" << databaseMode;

    if (!deviceEngine) {
        emit deviceOperationError("Invalid device engine provided");
        return;
    }

    if (!rootDevice) {
        emit deviceOperationError("Invalid root device provided");
        return;
    }

    if (m_deviceOperationRunning) {
        emit deviceOperationError("Device operation already running");
        return;
    }

    if (!catalogManager) {
        emit deviceOperationError("Catalog manager not available");
        return;
    }

    m_catalogManager = catalogManager;

    // Clean up any previous operation
    cleanupDeviceJob();

    // Set the new device job
    m_currentDeviceJob = deviceEngine;
    deviceEngine->setParent(this);

    // Connect device job signals
    connectDeviceJob(deviceEngine);

    // Reset state
    setDeviceOperationRunning(true);
    setProgress(0);
    setHierarchyProgress(0);
    setCatalogProgress(0);
    setProcessedDevices(0);
    setTotalDevices(0);
    setProcessedCatalogs(0);
    setTotalCatalogs(0);
    setCurrentDeviceName("");
    setStatus("Starting device operation...");
    m_isPaused = false;

    qDebug() << "Starting device operation...";

    // Start the device operation
    deviceEngine->startDeviceOperation(
        rootDevice,
        operationType,
        databaseMode,
        collectionFolder,
        m_catalogManager
        );

    qDebug() << "Device operation started";
}

void DeviceManager::setCatalogManager(CatalogManager* catalogManager)
{
    qDebug() << "DeviceManager::setCatalogManager() - using existing MainWindow CatalogManager";

    m_catalogManager = catalogManager;  // Use MainWindow's CatalogManager instead of creating our own

    // Re-setup integration with the existing catalog manager
    setupCatalogManagerIntegration();
}

void DeviceManager::stopDeviceOperation()
{
    qDebug() << "DeviceManager::stopDeviceOperation()";

    if (m_currentDeviceJob) {
        m_currentDeviceJob->stopDeviceOperation();
    }

    // Also stop catalog manager if running
    if (m_catalogManager && m_catalogManager->catalogOperationRunning()) {
        m_catalogManager->stopCatalogOperation();
    }
}

void DeviceManager::pauseDeviceOperation()
{
    qDebug() << "DeviceManager::pauseDeviceOperation()";

    if (m_currentDeviceJob) {
        m_currentDeviceJob->pauseDeviceOperation();
        m_isPaused = true;
    }

    // Also pause catalog manager if running
    if (m_catalogManager && m_catalogManager->catalogOperationRunning()) {
        m_catalogManager->pauseCatalogOperation();
    }
}

void DeviceManager::resumeDeviceOperation()
{
    qDebug() << "DeviceManager::resumeDeviceOperation()";

    if (m_currentDeviceJob) {
        m_currentDeviceJob->resumeDeviceOperation();
        m_isPaused = false;
    }

    // Also resume catalog manager if running
    if (m_catalogManager && m_catalogManager->catalogOperationRunning()) {
        m_catalogManager->resumeCatalogOperation();
    }
}

void DeviceManager::setCatalogProgressManager(CatalogProgressManager* catalogProgressManager)
{
    qDebug() << "DeviceManager::setCatalogProgressManager()";

    m_catalogProgressManager = catalogProgressManager;

    if (m_catalogProgressManager && m_catalogManager) {
        // Connect the existing CatalogProgressManager to our CatalogManager
        m_catalogProgressManager->setCatalogManager(m_catalogManager);
        qDebug() << "CatalogProgressManager connected to DeviceManager's CatalogManager";
    }
}

void DeviceManager::requestGentleStop()
{
    qDebug() << "DeviceManager::requestGentleStop()";

    if (m_currentDeviceJob) {
        m_currentDeviceJob->requestGentleStop();
    }

    // Update status to inform user
    setStatus("Stopping after current device completes...");
}

void DeviceManager::connectDeviceJob(DeviceJobStoppable* deviceJob)
{
    if (!deviceJob) return;

    qDebug() << "Connecting device job signals";

    // Main operation lifecycle signals
    connect(deviceJob, &DeviceJobStoppable::deviceOperationStarted,
            this, &DeviceManager::onDeviceOperationStarted);
    connect(deviceJob, &DeviceJobStoppable::deviceOperationCompleted,
            this, &DeviceManager::onDeviceOperationCompleted);
    connect(deviceJob, &DeviceJobStoppable::deviceOperationError,
            this, &DeviceManager::onDeviceOperationError);
    connect(deviceJob, &DeviceJobStoppable::deviceOperationCancelled,
            this, &DeviceManager::onDeviceOperationCancelled);

    // Progress signals
    connect(deviceJob, &DeviceJobStoppable::hierarchyProgressChanged,
            this, &DeviceManager::onDeviceHierarchyProgress);
    connect(deviceJob, &DeviceJobStoppable::catalogProgressChanged,
            this, &DeviceManager::onDeviceCatalogProgress);
    connect(deviceJob, &DeviceJobStoppable::currentDeviceChanged,
            this, &DeviceManager::onDeviceCurrentChanged);
    connect(deviceJob, &DeviceJobStoppable::statusUpdate,
            this, &DeviceManager::onDeviceStatusUpdate);

    // Detailed progress signals (forwarded to UI)
    connect(deviceJob, &DeviceJobStoppable::deviceProcessingStarted,
            this, [this](const QString& deviceName, const QString& deviceType) {
                Q_UNUSED(deviceType);
                setCurrentDeviceName(deviceName);
                setStatus(QString("Processing %1...").arg(deviceName));
            });

    connect(deviceJob, &DeviceJobStoppable::deviceProcessingCompleted,
            this, [this](const QString& deviceName, const QList<qint64>& deviceResults) {
                Q_UNUSED(deviceResults);
                qDebug() << "Device completed:" << deviceName;
            });

    qDebug() << "Device job signals connected";
}

void DeviceManager::setupCatalogManagerIntegration()
{
    if (!m_catalogManager || m_catalogManagerIntegrated) return;

    qDebug() << "Setting up catalog manager integration";

    // Connect catalog manager progress signals
    connect(m_catalogManager, &CatalogManager::progressChanged,
            this, &DeviceManager::onCatalogManagerProgressChanged);
    connect(m_catalogManager, &CatalogManager::statusChanged,
            this, &DeviceManager::onCatalogManagerStatusChanged);
    connect(m_catalogManager, &CatalogManager::currentCatalogNameChanged,
            this, [this]() {
                if (m_catalogManager) {
                    setCurrentDeviceName(m_catalogManager->currentCatalogName());
                }
            });

    m_catalogManagerIntegrated = true;
    qDebug() << "Catalog manager integration complete";
}

void DeviceManager::onDeviceOperationStarted()
{
    qDebug() << "=== DeviceManager::onDeviceOperationStarted() ===";

    // Update totals from device job
    if (m_currentDeviceJob) {
        setTotalDevices(m_currentDeviceJob->totalDevicesInHierarchy());
        setTotalCatalogs(m_currentDeviceJob->totalCatalogsInHierarchy());
    }

    setStatus("Device operation started");
    emit deviceOperationStarted();
}

void DeviceManager::onDeviceOperationCompleted(const QList<qint64>& results)
{
    qDebug() << "=== DeviceManager::onDeviceOperationCompleted() ===";
    qDebug() << "Results count:" << results.size();

    setDeviceOperationRunning(false);
    setProgress(100);
    setHierarchyProgress(100);
    setCatalogProgress(100);
    setStatus("Device operation completed successfully");

    // CRITICAL FIX: Get the ROOT device that was originally selected and show proper report
    if (m_currentDeviceJob) {
        Device* rootDevice = m_currentDeviceJob->rootDevice();

        if (rootDevice) {
            qDebug() << "Root device found:" << rootDevice->name << "Type:" << rootDevice->type;

            if (rootDevice->type == "Storage") {
                // Create results list in the format that reportAllUpdates expects for Storage
                QList<qint64> storageResults;
                for (int i = 0; i < 14; ++i) {
                    storageResults << 0;  // Initialize with zeros
                }

                // Set the success flag
                storageResults[0] = 1;  // Success

                // CRITICAL FIX: Use actual storage update results with deltas
                Storage::UpdateResult updateResult = m_currentDeviceJob->getStorageUpdateResult();

                if (updateResult.wasUpdated) {
                    qDebug() << "Using actual storage update results with deltas";

                    // Set storage space values with real deltas (indices 8-13)
                    storageResults[8] = updateResult.newUsedSpace;     // Used space
                    storageResults[9] = updateResult.deltaUsedSpace;   // Delta used space
                    storageResults[10] = updateResult.newFreeSpace;    // Free space
                    storageResults[11] = updateResult.deltaFreeSpace;  // Delta free space
                    storageResults[12] = updateResult.newTotalSpace;   // Total space
                    storageResults[13] = updateResult.deltaTotalSpace; // Delta total space

                    qDebug() << "Storage results - Used:" << storageResults[8] << "(+" << storageResults[9] << ")";
                    qDebug() << "Storage results - Free:" << storageResults[10] << "(+" << storageResults[11] << ")";
                    qDebug() << "Storage results - Total:" << storageResults[12] << "(+" << storageResults[13] << ")";

                } else {
                    qDebug() << "Storage was not updated, using current values with zero deltas";

                    // Fallback to current values with zero deltas
                    if (rootDevice->storage) {
                        storageResults[8] = rootDevice->storage->totalSpace - rootDevice->storage->freeSpace;  // Used space
                        storageResults[9] = 0;   // Delta used space
                        storageResults[10] = rootDevice->storage->freeSpace;  // Free space
                        storageResults[11] = 0;  // Delta free space
                        storageResults[12] = rootDevice->storage->totalSpace; // Total space
                        storageResults[13] = 0;  // Delta total space
                    }
                }

                qDebug() << "Emitting requestReportAllUpdates for Storage device:" << rootDevice->name;
                emit requestReportAllUpdates(rootDevice, storageResults, "update");

            } else if (rootDevice->type == "Catalog") {
                qDebug() << "Root device is Catalog - catalog reports handled by CatalogManager";
                // Catalog reports are handled separately by the existing catalog system

            } else if (rootDevice->type == "Virtual") {
                qDebug() << "Root device is Virtual - creating virtual device report";
                // For virtual devices, create a simple success report
                QList<qint64> virtualResults;
                for (int i = 0; i < 14; ++i) {
                    virtualResults << 0;
                }
                virtualResults[0] = 1;  // Success

                emit requestReportAllUpdates(rootDevice, virtualResults, "update");

            } else {
                qDebug() << "Root device type" << rootDevice->type << "- no specific report implemented";
            }
        } else {
            qDebug() << "No root device found for final report";
        }
    } else {
        qDebug() << "No current device job found";
    }

    emit deviceOperationCompleted(results);

    // Request UI refresh to show updated values
    qDebug() << "Emitting requestUIRefresh signal";
    emit requestUIRefresh();

    // Clean up
    cleanupDeviceJob();
}

void DeviceManager::onDeviceOperationError(const QString& error)
{
    qDebug() << "=== DeviceManager::onDeviceOperationError() ===" << error;

    setDeviceOperationRunning(false);
    setStatus(QString("Device operation failed: %1").arg(error));

    emit deviceOperationError(error);

    // Clean up
    cleanupDeviceJob();
}

void DeviceManager::onDeviceOperationCancelled()
{
    qDebug() << "=== DeviceManager::onDeviceOperationCancelled() ===";

    setDeviceOperationRunning(false);
    setStatus("Device operation cancelled");

    emit deviceOperationCancelled();

    // Clean up
    cleanupDeviceJob();
}

void DeviceManager::onDeviceHierarchyProgress(int processed, int total)
{
    qDebug() << "Device hierarchy progress:" << processed << "/" << total;

    setProcessedDevices(processed);
    setTotalDevices(total);

    // Calculate hierarchy progress percentage
    int hierarchyPercent = (total > 0) ? qMin(100, (processed * 100) / total) : 0;
    setHierarchyProgress(hierarchyPercent);

    // Update overall progress
    updateOverallProgress();

    // Emit detailed progress signal
    emit deviceHierarchyProgress(processed, total);
}

void DeviceManager::onDeviceCatalogProgress(int processed, int total)
{
    qDebug() << "Device catalog progress:" << processed << "/" << total;

    setProcessedCatalogs(processed);
    setTotalCatalogs(total);

    // Calculate catalog progress percentage
    int catalogPercent = (total > 0) ? qMin(100, (processed * 100) / total) : 0;
    setCatalogProgress(catalogPercent);

    // Update overall progress
    updateOverallProgress();

    // Emit detailed progress signal
    emit catalogOperationProgress(processed, total);
}

void DeviceManager::onDeviceCurrentChanged(const QString& deviceName, const QString& operation)
{
    qDebug() << "Current device changed:" << deviceName << "Operation:" << operation;

    setCurrentDeviceName(deviceName);
    setStatus(operation);

    // Emit detailed signal
    emit currentOperationChanged(deviceName, operation);
}

void DeviceManager::onDeviceStatusUpdate(const QString& message)
{
    setStatus(message);
}

void DeviceManager::onCatalogManagerProgressChanged()
{
    if (!m_catalogManager) return;

    // When catalog manager is running, use its progress for the catalog component
    if (m_catalogManager->catalogOperationRunning()) {
        int catalogProgress = m_catalogManager->progress();

        // Update catalog progress (but don't override processed/total counts from device job)
        setCatalogProgress(catalogProgress);
        updateOverallProgress();
    }
}

void DeviceManager::onCatalogManagerStatusChanged()
{
    if (!m_catalogManager) return;

    // Update status from catalog manager when it's active
    if (m_catalogManager->catalogOperationRunning()) {
        QString catalogStatus = m_catalogManager->status();
        if (!catalogStatus.isEmpty()) {
            setStatus(catalogStatus);
        }
    }
}

void DeviceManager::updateOverallProgress()
{
    // Calculate overall progress as a weighted combination of hierarchy and catalog progress
    // Hierarchy progress has more weight since it includes fast operations (Virtual, Storage)
    // Catalog progress represents the heavy file indexing operations

    int overallProgress = 0;

    if (m_totalDevices > 0 && m_totalCatalogs > 0) {
        // If we have both devices and catalogs, weight the progress
        // 30% for hierarchy traversal, 70% for catalog operations (which are the heavy part)
        int hierarchyWeight = 30;
        int catalogWeight = 70;

        overallProgress = (m_hierarchyProgress * hierarchyWeight + m_catalogProgress * catalogWeight) / 100;
    } else if (m_totalDevices > 0) {
        // Only devices (no catalogs), use hierarchy progress
        overallProgress = m_hierarchyProgress;
    } else if (m_totalCatalogs > 0) {
        // Only catalogs, use catalog progress
        overallProgress = m_catalogProgress;
    }

    overallProgress = qBound(0, overallProgress, 100);
    setProgress(overallProgress);
}

void DeviceManager::cleanupDeviceJob()
{
    qDebug() << "DeviceManager::cleanupDeviceJob()";

    if (m_currentDeviceJob) {
        // Disconnect signals to avoid issues during cleanup
        disconnect(m_currentDeviceJob, nullptr, this, nullptr);

        // Delete the job
        m_currentDeviceJob->deleteLater();
        m_currentDeviceJob = nullptr;
    }
}

// Property setters with signal emission

void DeviceManager::setDeviceOperationRunning(bool running)
{
    if (m_deviceOperationRunning != running) {
        m_deviceOperationRunning = running;
        qDebug() << "Device operation running changed to:" << running;
        emit deviceOperationRunningChanged();
    }
}

void DeviceManager::setProgress(int progress)
{
    progress = qBound(0, progress, 100);
    if (m_progress != progress) {
        m_progress = progress;
        qDebug() << "Overall progress changed to:" << progress << "%";
        emit progressChanged();
    }
}

void DeviceManager::setStatus(const QString& status)
{
    if (m_status != status) {
        m_status = status;
        qDebug() << "Status changed to:" << status;
        emit statusChanged();
    }
}

void DeviceManager::setCurrentDeviceName(const QString& name)
{
    if (m_currentDeviceName != name) {
        m_currentDeviceName = name;
        qDebug() << "Current device name changed to:" << name;
        emit currentDeviceNameChanged();
    }
}

void DeviceManager::setHierarchyProgress(int progress)
{
    progress = qBound(0, progress, 100);
    if (m_hierarchyProgress != progress) {
        m_hierarchyProgress = progress;
        qDebug() << "Hierarchy progress changed to:" << progress << "%";
        emit hierarchyProgressChanged();
    }
}

void DeviceManager::setCatalogProgress(int progress)
{
    progress = qBound(0, progress, 100);
    if (m_catalogProgress != progress) {
        m_catalogProgress = progress;
        qDebug() << "Catalog progress changed to:" << progress << "%";
        emit catalogProgressChanged();
    }
}

void DeviceManager::setProcessedDevices(qint64 processed)
{
    if (m_processedDevices != processed) {
        m_processedDevices = processed;
        qDebug() << "Processed devices changed to:" << processed;
        emit processedDevicesChanged();
    }
}

void DeviceManager::setTotalDevices(qint64 total)
{
    if (m_totalDevices != total) {
        m_totalDevices = total;
        qDebug() << "Total devices changed to:" << total;
        emit totalDevicesChanged();
    }
}

void DeviceManager::setProcessedCatalogs(qint64 processed)
{
    if (m_processedCatalogs != processed) {
        m_processedCatalogs = processed;
        qDebug() << "Processed catalogs changed to:" << processed;
        emit processedCatalogsChanged();
    }
}

void DeviceManager::setTotalCatalogs(qint64 total)
{
    if (m_totalCatalogs != total) {
        m_totalCatalogs = total;
        qDebug() << "Total catalogs changed to:" << total;
        emit totalCatalogsChanged();
    }
}
