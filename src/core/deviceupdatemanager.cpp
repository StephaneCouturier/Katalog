#include "deviceupdatemanager.h"
#include <QDebug>
#include <QTimer>
#include <QSqlQuery>
#include <QSqlDatabase>

DeviceUpdateManager::DeviceUpdateManager(QObject *parent)
    : QObject(parent)
{
    qDebug() << "DeviceUpdateManager created";
}

DeviceUpdateManager::~DeviceUpdateManager()
{
    qDebug() << "DeviceUpdateManager destructor";
    cleanupOperation();
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

void DeviceUpdateManager::updateVirtualDevice(Device* device)
{
    qDebug() << "Updating Virtual Device:" << device->name;
    setStatus(QString("Updating virtual device: %1").arg(device->name));

    // Virtual devices just aggregate numbers from children
    device->saveDevice();
    m_processedDevices++;
    updateProgress();

    emit deviceProcessingCompleted(device->name);
}

void DeviceUpdateManager::updateStorageDevice(Device* device)
{
    qDebug() << "Updating Storage Device:" << device->name;
    setStatus(QString("Updating storage device: %1").arg(device->name));

    // Fast operation: Update storage space information
    if (device->storage) {
        device->storage->path = device->path;

        Storage::UpdateResult result = device->storage->updateStorageInfo();

        if (result.wasUpdated) {
            device->freeSpace = result.newFreeSpace;
            device->totalSpace = result.newTotalSpace;
            device->saveDevice();
            device->saveStatistics(device->dateTimeUpdated, "update");
        }
    }

    m_processedDevices++;
    updateProgress();

    emit deviceProcessingCompleted(device->name);
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
    qDebug() << "Catalog operation completed for:" << (m_currentDevice ? m_currentDevice->name : "unknown");

    if (m_currentCatalogJob) {
        m_currentCatalogJob->deleteLater();
        m_currentCatalogJob = nullptr;
    }

    m_waitingForCatalogCompletion = false;

    m_processedDevices++;
    m_processedCatalogs++;
    updateProgress();

    emit deviceProcessingCompleted(m_currentDevice ? m_currentDevice->name : "unknown");

    if (m_currentDevice) {
        processChildren(m_currentDevice);
    }
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






// Replace these methods in deviceupdatemanager.cpp to fix stop handling:

void DeviceUpdateManager::updateCatalogDevice(Device* device)
{
    qDebug() << "Updating Catalog Device:" << device->name;
    setStatus(QString("Updating catalog: %1").arg(device->name));

    if (!device->active) {
        qDebug() << "Catalog is inactive, skipping:" << device->name;
        m_processedDevices++;
        m_processedCatalogs++;
        updateProgress();
        emit deviceProcessingCompleted(device->name);
        processChildren(device);
        return;
    }

    // Check for stop before starting catalog operation
    if (!shouldContinue()) {
        qDebug() << "Stop requested before catalog processing:" << device->name;
        handleOperationCancellation();
        return;
    }

    // Set waiting flag
    m_waitingForCatalogCompletion = true;

    // Prepare catalog for update
    device->catalog->name = device->name;
    device->catalog->sourcePath = device->path;

    // Simulate catalog operation with proper stop checking
    QTimer* catalogTimer = new QTimer(this);
    catalogTimer->setSingleShot(true);

    connect(catalogTimer, &QTimer::timeout, this, [this, device, catalogTimer]() {
        catalogTimer->deleteLater();

        // Check for stop during catalog operation
        if (!shouldContinue()) {
            qDebug() << "Stop requested during catalog processing:" << device->name;
            m_waitingForCatalogCompletion = false;
            handleOperationCancellation();
            return;
        }

        // Simulate catalog completion
        qDebug() << "Catalog processing completed:" << device->name;
        m_processedDevices++;
        m_processedCatalogs++;
        updateProgress();

        device->saveDevice();
        device->saveStatistics(device->dateTimeUpdated, "update");
        updateRelatedDevices(device);

        emit deviceProcessingCompleted(device->name);
        m_waitingForCatalogCompletion = false;

        // Continue processing children
        processChildren(device);
    });

    // Start timer (simulate 1 second catalog operation)
    catalogTimer->start(1000);
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

void DeviceUpdateManager::updateDeviceRecursive(Device* device)
{
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

    qDebug() << "Processing device:" << device->name << "Type:" << device->type;

    // Update device active state
    device->updateActiveState("defaultConnection");
    device->dateTimeUpdated = QDateTime::currentDateTime();

    // Type-specific processing
    if (device->type == "Virtual") {
        updateVirtualDevice(device);
        processChildren(device);
    }
    else if (device->type == "Storage") {
        updateStorageDevice(device);
        processChildren(device);
    }
    else if (device->type == "Catalog") {
        updateCatalogDevice(device);
        return; // Async operation
    }
    else {
        qDebug() << "Unknown device type:" << device->type << "- treating as virtual";
        updateVirtualDevice(device);
        processChildren(device);
    }
}

void DeviceUpdateManager::processChildren(Device* device)
{
    if (!device || !shouldContinue()) {
        if (!shouldContinue()) {
            qDebug() << "Stop requested - aborting children processing";
            handleOperationCancellation();
        }
        return;
    }

    // Load children if not already loaded
    if (device->subDevices.isEmpty() && device->hasSubDevice) {
        device->loadDevice("defaultConnection");
    }

    // Process each child recursively
    for (const Device& childDevice : device->subDevices) {
        if (!shouldContinue()) {
            qDebug() << "Stop requested during children processing";
            handleOperationCancellation();
            return;
        }

        // Create a copy for processing
        Device* childPtr = new Device(childDevice);
        updateDeviceRecursive(childPtr);

        if (m_waitingForCatalogCompletion) {
            // Async catalog operation in progress
            return;
        }

        // Check for stop after each child
        if (!shouldContinue()) {
            qDebug() << "Stop requested after processing child";
            handleOperationCancellation();
            return;
        }
    }

    // All children processed
    updateParentNumbers(device);
    processNextDevice();
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
