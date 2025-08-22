#include "deviceupdatemanager.h"
#include <QDebug>
#include <QTimer>
#include <QSqlQuery>
#include <QSqlDatabase>

DeviceUpdateManager::DeviceUpdateManager(QObject *parent)
    : QObject(parent)
{
    qDebug() << "DeviceUpdateManager created";
    setupCatalogManager();  // ADD THIS LINE
}

DeviceUpdateManager::~DeviceUpdateManager()
{
    qDebug() << "DeviceUpdateManager destructor";
    cleanupOperation();
}


void DeviceUpdateManager::setupCatalogManager()
{
    qDebug() << "DeviceUpdateManager::setupCatalogManager()";

    m_catalogManager = new CatalogManager(this);

    connect(m_catalogManager, &CatalogManager::catalogOperationCompleted,
            this, &DeviceUpdateManager::onCatalogOperationCompleted);
    connect(m_catalogManager, &CatalogManager::catalogOperationError,
            this, &DeviceUpdateManager::onCatalogOperationError);
    connect(m_catalogManager, &CatalogManager::catalogOperationCancelled,
            this, &DeviceUpdateManager::onCatalogOperationCancelled);

    qDebug() << "CatalogManager setup complete";
}

void DeviceUpdateManager::cleanupCatalogJob()
{
    if (m_currentCatalogJob) {
        qDebug() << "Cleaning up catalog job";
        m_currentCatalogJob->deleteLater();
        m_currentCatalogJob = nullptr;
    }
}

void DeviceUpdateManager::continueToNextDevice()
{
    qDebug() << "=== DeviceUpdateManager::continueToNextDevice ===";

    // For single device operation, complete
    if (m_allDevices.size() <= 1) {
        completeOperation();  // FIXED: Remove results parameter
        return;
    }

    // For multi-device operations, check if all devices are processed
    if (m_processedDevices >= m_totalDevices) {
        qDebug() << "All devices processed, completing operation";
        completeOperation();
        return;
    }

    processNextInQueue();
}

void DeviceUpdateManager::processNextInQueue()
{
    qDebug() << "=== DeviceUpdateManager::processNextInQueue ===";

    if (m_processedDevices >= m_totalDevices) {
        completeOperation();
        return;
    }

    // Simplified - complete for now
    completeOperation();
}

void DeviceUpdateManager::updateDeviceHierarchy(Device* rootDevice,
                                                const QString& databaseMode,
                                                const QString& collectionFolder)
{
    qDebug() << "=== DeviceUpdateManager::updateDeviceHierarchy ===";
    qDebug() << "Root device:" << (rootDevice ? rootDevice->name : "NULL");

    if (!rootDevice) {
        emit operationError("Invalid root device provided");
        return;
    }

    if (m_operationRunning) {
        emit operationError("Operation already running");
        return;
    }

    // Initialize operation
    m_operationRunning = true;
    m_stopRequested.storeRelease(0);
    m_gentleStopRequested.storeRelease(0);
    m_isPaused = false;
    m_rootDevice = rootDevice;
    m_databaseMode = databaseMode;
    m_collectionFolder = collectionFolder;
    m_operationStartTime = QDateTime::currentDateTime();

    // Analyze hierarchy to get totals for progress
    analyzeHierarchy(rootDevice);

    setStatus("Starting device hierarchy update...");
    emit operationStarted();
    emit operationRunningChanged();

    // Start recursive update
    updateDeviceRecursive(rootDevice);
}

Device* DeviceUpdateManager::createTempVirtualDeviceForActiveCatalogs(const QList<Device*>& activeCatalogs)
{
    qDebug() << "Creating temporary virtual device for" << activeCatalogs.size() << "active catalogs";

    Device* tempDevice = new Device();
    tempDevice->ID = -1;
    tempDevice->name = "Active Catalogs (Update Operation)";
    tempDevice->type = "Virtual";
    tempDevice->active = true;
    tempDevice->hasSubDevice = !activeCatalogs.isEmpty();

    for (Device* catalog : activeCatalogs) {
        tempDevice->subDevices.append(*catalog);
        tempDevice->deviceIDList.append(catalog->ID);
    }

    qDebug() << "Temporary virtual device created with" << tempDevice->subDevices.size() << "children";
    m_tempDevice = tempDevice;

    return tempDevice;
}

Device* DeviceUpdateManager::createTempVirtualDeviceForFilter(const QList<Device*>& filteredDevices)
{
    qDebug() << "Creating temporary virtual device for" << filteredDevices.size() << "filtered devices";

    Device* tempDevice = new Device();
    tempDevice->ID = -2;
    tempDevice->name = "Filtered Selection (Update Operation)";
    tempDevice->type = "Virtual";
    tempDevice->active = true;
    tempDevice->hasSubDevice = !filteredDevices.isEmpty();

    for (Device* device : filteredDevices) {
        tempDevice->subDevices.append(*device);
        tempDevice->deviceIDList.append(device->ID);
    }

    qDebug() << "Temporary filtered device created with" << tempDevice->subDevices.size() << "children";
    m_tempDevice = tempDevice;

    return tempDevice;
}


void DeviceUpdateManager::processNextDevice()
{
    if (!shouldContinue()) {
        handleOperationCancellation();
        return;
    }

    // Check if we're done with all processing
    completeOperation();
}

QString DeviceUpdateManager::currentDeviceName() const
{
    return m_currentDevice ? m_currentDevice->name : QString();
}

void DeviceUpdateManager::processChildren(Device* device)
{
    qDebug() << "=== DeviceUpdateManager::processChildren ===";
    qDebug() << "Processing children for:" << device->name << "Type:" << device->type;

    if (!device || !shouldContinue()) {
        if (!shouldContinue()) {
            qDebug() << "Stop requested - aborting children processing";
            handleOperationCancellation();
        }
        return;
    }

    // Load children if not already loaded
    if (device->subDevices.isEmpty() && device->hasSubDevice) {
        qDebug() << "Loading children for device:" << device->name;
        device->loadDevice("defaultConnection");
    }

    // If no children, this device is complete
    if (device->subDevices.isEmpty()) {
        qDebug() << "No children found for:" << device->name;
        updateParentNumbers(device);
        emit deviceProcessingCompleted(device->name);
        continueToNextDevice();
        return;
    }

    qDebug() << "Found" << device->subDevices.size() << "children for:" << device->name;

    // Process each child recursively
    for (const Device& childDevice : device->subDevices) {
        if (!shouldContinue()) {
            qDebug() << "Stop requested during children processing";
            handleOperationCancellation();
            return;
        }

        // Create a copy for processing
        Device* childPtr = new Device(childDevice);
        qDebug() << "Processing child:" << childPtr->name << "Type:" << childPtr->type;

        updateDeviceRecursive(childPtr);

        // If we started an async catalog operation, wait for it to complete
        if (m_waitingForCatalogCompletion) {
            qDebug() << "Async catalog operation in progress, waiting...";
            return;
        }

        // Check for stop after each child
        if (!shouldContinue()) {
            qDebug() << "Stop requested after processing child";
            handleOperationCancellation();
            return;
        }
    }

    // All children processed for this device
    qDebug() << "All children processed for:" << device->name;
    updateParentNumbers(device);
    emit deviceProcessingCompleted(device->name);
    continueToNextDevice();
}

void DeviceUpdateManager::updateDeviceRecursive(Device* device)
{
    qDebug() << "=== DeviceUpdateManager::updateDeviceRecursive ===";
    qDebug() << "Processing device:" << device->name << "Type:" << device->type;

    if (!device || !shouldContinue()) {
        if (!shouldContinue()) {
            qDebug() << "Stop requested - aborting recursive processing";
            handleOperationCancellation();
        }
        return;
    }

    // Check for gentle stop before starting new catalog
    if (m_gentleStopRequested.loadAcquire() && device->type == "Catalog") {
        qDebug() << "Gentle stop requested - stopping before catalog:" << device->name;
        completeOperation();
        return;
    }

    // Check for hard stop
    if (m_stopRequested.loadAcquire()) {
        qDebug() << "Hard stop requested - aborting processing for:" << device->name;
        handleOperationCancellation();
        return;
    }

    setCurrentDevice(device);
    emit deviceProcessingStarted(device->name, device->type);

    // Update device active state
    device->updateActiveState("defaultConnection");
    device->dateTimeUpdated = QDateTime::currentDateTime();

    // Type-specific processing with proper flow control
    if (device->type == "Virtual") {
        updateVirtualDevice(device);
        processChildren(device);  // Virtual devices process children

    } else if (device->type == "Storage") {
        updateStorageDevice(device);
        processChildren(device);  // Storage devices process catalog children

    } else if (device->type == "Catalog") {
        updateCatalogDevice(device);
        // NOTE: Catalog completion is handled in onCatalogOperationCompleted()
        // Do NOT call processChildren here - catalogs are leaf nodes

    } else {
        qDebug() << "Unknown device type:" << device->type << "- treating as virtual";
        updateVirtualDevice(device);
        processChildren(device);
    }
}

void DeviceUpdateManager::updateCatalogDevice(Device* device)
{
    // Remove all the timer simulation code and replace with:

    if (!device->active) {
        m_processedDevices++;
        m_processedCatalogs++;
        updateProgress();
        emit deviceProcessingCompleted(device->name);
        continueToNextDevice();
        return;
    }

    if (!m_catalogManager) {
        handleOperationError("CatalogManager not available");
        return;
    }

    m_waitingForCatalogCompletion = true;
    m_currentDevice = device;

    device->catalog->name = device->name;
    device->catalog->sourcePath = device->path;

    cleanupCatalogJob();
    m_currentCatalogJob = new CatalogJobStoppable(this);

    connect(m_currentCatalogJob, &CatalogJobStoppable::catalogProgress,
            this, [this](qint64 filesProcessed, qint64 totalFiles, const QString& currentPath) {
                emit catalogProgress(filesProcessed, totalFiles, currentPath);
            });

    m_catalogManager->startCatalogJobStoppable(
        m_currentCatalogJob,
        device,
        CatalogJobStoppable::UpdateCatalog,
        m_databaseMode,
        m_collectionFolder
        );
}

void DeviceUpdateManager::updateStorageDevice(Device* device)
{
    qDebug() << "=== DeviceUpdateManager::updateStorageDevice ===";
    qDebug() << "Updating Storage Device:" << device->name;
    setStatus(QString("Updating storage device: %1").arg(device->name));

    // Update storage space information (fast operation)
    if (device->storage) {
        device->storage->path = device->path;

        try {
            // Update storage space info using existing Storage class method
            Storage::UpdateResult result = device->storage->updateStorageInfo();

            if (result.wasUpdated) {
                device->freeSpace = result.newFreeSpace;
                device->totalSpace = result.newTotalSpace;
                qDebug() << "Storage space updated - Free:" << device->freeSpace
                         << "Total:" << device->totalSpace;
            }
        } catch (const std::exception& e) {
            qDebug() << "Error updating storage info:" << e.what();
        }
    }

    // Save storage device changes
    device->saveDevice();
    device->saveStatistics(device->dateTimeUpdated, "update");

    m_processedDevices++;
    updateProgress();

    emit deviceProcessingCompleted(device->name);

    // Storage devices continue to process their catalog children in processChildren()
}

void DeviceUpdateManager::updateVirtualDevice(Device* device)
{
    qDebug() << "=== DeviceUpdateManager::updateVirtualDevice ===";
    qDebug() << "Updating Virtual Device:" << device->name;
    setStatus(QString("Updating virtual device: %1").arg(device->name));

    // Virtual devices just aggregate numbers from children
    // The actual aggregation happens after children are processed
    device->saveDevice();

    m_processedDevices++;
    updateProgress();

    emit deviceProcessingCompleted(device->name);

    // Virtual devices continue to process their children in processChildren()
}



void DeviceUpdateManager::pauseOperation()
{
    m_isPaused = true;
    setStatus("Operation paused");
}

void DeviceUpdateManager::resumeOperation()
{
    m_isPaused = false;
    setStatus("Operation resumed");
}

void DeviceUpdateManager::stopOperation()
{
    m_stopRequested.storeRelease(1);
    handleOperationCancellation();
}

void DeviceUpdateManager::onCatalogOperationCompleted()
{
    qDebug() << "=== DeviceUpdateManager::onCatalogOperationCompleted ===";

    if (!m_currentDevice) {
        qDebug() << "ERROR: No current device in catalog completion";
        return;
    }

    qDebug() << "Catalog processing completed for:" << m_currentDevice->name;

    m_processedDevices++;
    m_processedCatalogs++;
    updateProgress();

    m_currentDevice->loadDevice("defaultConnection");
    m_currentDevice->saveDevice();
    m_currentDevice->saveStatistics(m_currentDevice->dateTimeUpdated, "update");
    updateRelatedDevices(m_currentDevice);

    emit deviceProcessingCompleted(m_currentDevice->name);

    m_waitingForCatalogCompletion = false;

    continueToNextDevice();  // FIXED: Use new method
}

void DeviceUpdateManager::onCatalogOperationError(const QString& error)
{
    qDebug() << "Catalog operation error:" << error;

    if (m_currentCatalogJob) {
        m_currentCatalogJob->deleteLater();
        m_currentCatalogJob = nullptr;
    }

    m_waitingForCatalogCompletion = false;
    handleOperationError(error);
}

void DeviceUpdateManager::onCatalogOperationCancelled()
{
    qDebug() << "Catalog operation cancelled";

    if (m_currentCatalogJob) {
        m_currentCatalogJob->deleteLater();
        m_currentCatalogJob = nullptr;
    }

    m_waitingForCatalogCompletion = false;
    handleOperationCancellation();
}

void DeviceUpdateManager::onCatalogProgress(qint64 filesProcessed, qint64 totalFiles, const QString& currentPath)
{
    emit catalogProgress(filesProcessed, totalFiles, currentPath);
}

void DeviceUpdateManager::checkStopRequested()
{
    // Placeholder for future use
}

void DeviceUpdateManager::setCurrentDevice(Device* device)
{
    if (m_currentDevice != device) {
        m_currentDevice = device;
        emit currentDeviceNameChanged();
    }
}

void DeviceUpdateManager::updateProgress()
{
    int progress = 0;
    if (m_totalDevices > 0) {
        progress = (m_processedDevices * 100) / m_totalDevices;
    }

    if (m_progress != progress) {
        m_progress = progress;
        emit progressChanged();
    }
}

void DeviceUpdateManager::setStatus(const QString& status)
{
    if (m_status != status) {
        m_status = status;
        emit statusChanged();
    }
}

void DeviceUpdateManager::analyzeHierarchy(Device* rootDevice)
{
    m_allDevices.clear();
    m_catalogDevices.clear();
    m_processedDevices = 0;
    m_processedCatalogs = 0;

    buildDeviceList(rootDevice, m_allDevices);

    for (Device* device : m_allDevices) {
        if (device->type == "Catalog") {
            m_catalogDevices.append(device);
        }
    }

    m_totalDevices = m_allDevices.size();
    m_totalCatalogs = m_catalogDevices.size();

    qDebug() << "Hierarchy analyzed - Total devices:" << m_totalDevices << "Total catalogs:" << m_totalCatalogs;
}

void DeviceUpdateManager::buildDeviceList(Device* device, QList<Device*>& deviceList)
{
    if (!device) return;

    deviceList.append(device);

    // Load children if needed
    if (device->subDevices.isEmpty() && device->hasSubDevice) {
        device->loadDevice("defaultConnection");
    }

    // Recursively add children
    for (const Device& childDevice : device->subDevices) {
        Device* childPtr = new Device(childDevice);
        buildDeviceList(childPtr, deviceList);
    }
}

void DeviceUpdateManager::updateParentNumbers(Device* device)
{
    if (!device) return;

    qDebug() << "Updating parent numbers for device:" << device->name;

    try {
        device->updateParentsNumbers();
        qDebug() << "Parent numbers updated successfully for:" << device->name;
    } catch (const std::exception& e) {
        qDebug() << "Error updating parent numbers for" << device->name << ":" << e.what();
    }
}

void DeviceUpdateManager::updateRelatedDevices(Device* device)
{
    if (!device || device->type != "Catalog") return;

    qDebug() << "Updating related devices for catalog:" << device->name;

    try {
        QSqlQuery queryRelatedDevice(QSqlDatabase::database("defaultConnection"));
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
            relatedDevice.loadDevice("defaultConnection");
            relatedDevice.totalFileCount = device->totalFileCount;
            relatedDevice.totalFileSize = device->totalFileSize;
            relatedDevice.saveDevice();
            relatedDevice.updateParentsNumbers();
        }
    } catch (const std::exception& e) {
        qDebug() << "Error updating related devices:" << e.what();
    }
}

void DeviceUpdateManager::saveDeviceStatistics(Device* device)
{
    if (!device) return;

    try {
        device->saveStatistics(device->dateTimeUpdated, "update");
    } catch (const std::exception& e) {
        qDebug() << "Error saving device statistics:" << e.what();
    }
}

void DeviceUpdateManager::handleOperationError(const QString& error)
{
    qDebug() << "DeviceUpdateManager::handleOperationError:" << error;

    m_operationRunning = false;
    setStatus(QString("Operation failed: %1").arg(error));

    emit operationError(error);
    emit operationRunningChanged();

    cleanupOperation();
}

void DeviceUpdateManager::cleanupOperation()
{
    qDebug() << "DeviceUpdateManager::cleanupOperation()";

    cleanupTempDevice();

    m_rootDevice = nullptr;
    m_currentDevice = nullptr;
    m_currentDeviceIndex = 0;
    m_allDevices.clear();
    m_catalogDevices.clear();
    m_accumulatedResults.clear();

    if (m_currentCatalogJob) {
        m_currentCatalogJob->deleteLater();
        m_currentCatalogJob = nullptr;
    }

    m_waitingForCatalogCompletion = false;
}

void DeviceUpdateManager::cleanupTempDevice()
{
    if (m_tempDevice) {
        qDebug() << "Cleaning up temporary device:" << m_tempDevice->name;
        delete m_tempDevice;
        m_tempDevice = nullptr;
    }
}

void DeviceUpdateManager::requestHardStop()
{
    qDebug() << "DeviceUpdateManager::requestHardStop() - IMMEDIATE STOP";

    m_stopRequested.storeRelease(1);
    m_gentleStopRequested.storeRelease(1); // Also set gentle stop for consistency

    setStatus("Hard stopping immediately...");

    // Immediately cancel any waiting operations
    if (m_waitingForCatalogCompletion) {
        m_waitingForCatalogCompletion = false;
    }

    // Force immediate cleanup
    QTimer::singleShot(10, this, [this]() {
        handleOperationCancellation();
    });
}

void DeviceUpdateManager::requestGentleStop()
{
    qDebug() << "DeviceUpdateManager::requestGentleStop() - STOP AFTER CURRENT CATALOG";

    m_gentleStopRequested.storeRelease(1);
    setStatus("Gentle stopping after current catalog completes...");

    // If no catalog is currently running, stop immediately
    if (!m_waitingForCatalogCompletion) {
        qDebug() << "No catalog running - stopping immediately";
        QTimer::singleShot(10, this, [this]() {
            completeOperation();
        });
    }
}

void DeviceUpdateManager::handleOperationCancellation()
{
    qDebug() << "=== DeviceUpdateManager::handleOperationCancellation ===";
    qDebug() << "Processed devices:" << m_processedDevices << "/" << m_totalDevices;
    qDebug() << "Processed catalogs:" << m_processedCatalogs << "/" << m_totalCatalogs;

    m_operationRunning = false;
    m_waitingForCatalogCompletion = false;

    // Calculate skipped counts for reporting
    int skippedDevices = m_totalDevices - m_processedDevices;
    int skippedCatalogs = m_totalCatalogs - m_processedCatalogs;

    setStatus(QString("Operation cancelled - %1 catalogs skipped").arg(skippedCatalogs));

    // Create results with skip information
    QList<qint64> results;
    results << 0;  // Operation cancelled
    results << m_processedCatalogs;     // Processed catalogs
    results << skippedCatalogs;         // Skipped catalogs due to stop
    results << m_processedDevices;      // Processed devices
    results << skippedDevices;          // Skipped devices due to stop

    emit operationCancelled();
    emit operationCompleted(results);  // Send results for reporting
    emit operationRunningChanged();

    cleanupOperation();
}

void DeviceUpdateManager::completeOperation()
{
    qDebug() << "=== DeviceUpdateManager::completeOperation ===";
    qDebug() << "Total devices processed:" << m_processedDevices << "/" << m_totalDevices;
    qDebug() << "Total catalogs processed:" << m_processedCatalogs << "/" << m_totalCatalogs;

    // Update parent numbers for entire hierarchy
    if (m_rootDevice) {
        try {
            m_rootDevice->updateParentsNumbers();
            qDebug() << "Parent numbers updated for hierarchy";
        } catch (const std::exception& e) {
            qDebug() << "Error updating parent numbers:" << e.what();
        }
    }

    m_operationRunning = false;
    m_waitingForCatalogCompletion = false;

    // Calculate final results
    QList<qint64> results;
    results << 1;  // Success flag
    results << m_processedCatalogs;     // Total processed catalogs
    results << 0;  // No skipped catalogs (completed successfully)
    results << m_processedDevices;      // Total processed devices
    results << 0;  // No skipped devices (completed successfully)

    setStatus(QString("Operation completed - %1 catalogs processed").arg(m_processedCatalogs));

    emit operationCompleted(results);
    emit operationRunningChanged();

    cleanupOperation();
}

bool DeviceUpdateManager::shouldContinue() const
{
    return !m_stopRequested.loadAcquire() && m_operationRunning;
}


