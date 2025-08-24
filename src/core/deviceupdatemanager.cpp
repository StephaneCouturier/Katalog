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

    // Check if operation already completed to prevent duplicate reports
    if (!m_operationRunning) {
        qDebug() << "*** SKIPPING continueToNextDevice - operation already completed by onCatalogOperationCompleted ***";
        return;
    }

    qDebug() << "*** COMPLETION PATH 2: continueToNextDevice (POTENTIAL catalog-only data) ***";
    qDebug() << "All devices size:" << m_allDevices.size();
    qDebug() << "Total devices:" << m_totalDevices;
    qDebug() << "Processed devices:" << m_processedDevices;

    // For single device operation, complete
    if (m_allDevices.size() <= 1) {
        qDebug() << "*** Calling completeOperation from continueToNextDevice (single device) ***";
        completeOperation();
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

void DeviceUpdateManager::updateParentStorageAfterCatalogUpdate(Device *device)
{
    if (!device || device->parentID == 0) {
        qDebug() << "No parent device to update";
        m_storageWasUpdated = false;
        return;
    }

    try {
        qDebug() << "Loading parent storage device...";
        Device parentDevice;
        parentDevice.ID = device->parentID;
        parentDevice.loadDevice("defaultConnection");

        if (parentDevice.type == "Storage" && parentDevice.storage) {
            qDebug() << "Updating parent storage space info for:" << parentDevice.name;

            // ✅ Refresh physical storage space information and capture results
            m_storageUpdateResult = parentDevice.storage->updateStorageInfo();

            if (m_storageUpdateResult.wasUpdated) {
                qDebug() << "Storage space updated successfully";
                qDebug() << "New total space:" << m_storageUpdateResult.newTotalSpace;
                qDebug() << "New free space:" << m_storageUpdateResult.newFreeSpace;

                // Update device values with new storage information
                parentDevice.totalSpace = m_storageUpdateResult.newTotalSpace;
                parentDevice.freeSpace = m_storageUpdateResult.newFreeSpace;
                parentDevice.dateTimeUpdated = QDateTime::currentDateTime();

                // Save updated storage device
                parentDevice.saveDevice();

                // Save storage statistics
                parentDevice.saveStatistics(parentDevice.dateTimeUpdated, "update");

                // ✅ Mark that storage was successfully updated
                m_storageWasUpdated = true;

                qDebug() << "Parent storage device updated and saved";
            } else {
                qDebug() << "Storage space not updated - Error:" << m_storageUpdateResult.errorMessage;
                m_storageWasUpdated = false;
            }
        } else {
            qDebug() << "Parent device is not a storage or has no storage object";
            m_storageWasUpdated = false;
        }

    } catch (const std::exception& e) {
        qDebug() << "Error updating parent storage device:" << e.what();
        m_storageWasUpdated = false;
    }
}

void DeviceUpdateManager::updateDeviceHierarchy(Device* rootDevice,
                                                const QString& databaseMode,
                                                const QString& collectionFolder,
                                                const QString& updateType)
{
    qDebug() << "=== DeviceUpdateManager::updateDeviceHierarchy ===";
    qDebug() << "Root device:" << (rootDevice ? rootDevice->name : "NULL");
    qDebug() << "Update type:" << updateType;  // ADD THIS LINE

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
    m_updateType = updateType;  // ADD THIS LINE - Store the operation type
    m_operationStartTime = QDateTime::currentDateTime();

    // Analyze hierarchy to get totals for progress
    analyzeHierarchy(rootDevice);  // CORRECTED - was collectDeviceHierarchy

    setStatus(QString("Starting %1 operation...").arg(updateType));  // UPDATED MESSAGE
    emit operationStarted();
    emit operationRunningChanged();

    // Start recursive update
    updateDeviceRecursive(rootDevice);  // KEPT - don't remove this!
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

    // Catalog devices should never call processChildren
    if (device->type == "Catalog") {
        qDebug() << "*** ERROR: processChildren called for catalog device - this should never happen! ***";
        qDebug() << "*** Catalog completion is handled by onCatalogOperationCompleted ***";
        return;
    }

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

        // Check if operation already completed to prevent duplicate calls
        if (!m_operationRunning) {
            qDebug() << "*** SKIPPING continueToNextDevice from processChildren - operation already completed ***";
            return;
        }

        qDebug() << "*** CALLING continueToNextDevice from processChildren (no children) ***";
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

    // Check if operation already completed to prevent duplicate calls
    if (!m_operationRunning) {
        qDebug() << "*** SKIPPING continueToNextDevice from processChildren - operation already completed ***";
        return;
    }

    qDebug() << "*** CALLING continueToNextDevice from processChildren (all children done) ***";
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
    qDebug() << "=== DeviceUpdateManager::updateCatalogDevice ===";
    qDebug() << "Updating Catalog Device:" << device->name;
    setStatus(QString("Updating catalog device: %1").arg(device->name));

    if (!device->active) {
        qDebug() << "Catalog is inactive, skipping:" << device->name;

        // Create a "skipped" result for reporting
        QList<qint64> skippedResults;
        skippedResults << 0;  // 0 = skipped (not success=1, not error=-1)
        skippedResults << 0;  // No files processed
        skippedResults << 0;  // No delta files
        skippedResults << 0;  // No size
        skippedResults << 0;  // No delta size
        for (int i = 5; i < 14; ++i) skippedResults << 0; // Padding

        m_processedDevices++;
        m_processedCatalogs++;
        updateProgress();

        emit deviceProcessingCompleted(device->name);

        // For inactive catalogs, complete immediately without storage update
        m_operationRunning = false;
        emit operationCompleted(skippedResults);
        emit operationRunningChanged();
        cleanupOperation();
        return;
    }

    if (!m_catalogManager) {
        handleOperationError("CatalogManager not available");
        return;
    }

    // CRITICAL: Set waiting flag and current device
    m_waitingForCatalogCompletion = true;
    m_currentDevice = device;

    // Pass device values for catalog operations (same as Device::updateDevice logic)
    device->catalog->name = device->name;
    device->catalog->sourcePath = device->path;

    // Clean up any existing catalog job
    cleanupCatalogJob();
    m_currentCatalogJob = new CatalogJobStoppable(this);

    // Connect progress updates for UI
    connect(m_currentCatalogJob, &CatalogJobStoppable::catalogProgress,
            this, [this](qint64 filesProcessed, qint64 totalFiles, const QString& currentPath) {
                emit catalogProgress(filesProcessed, totalFiles, currentPath);
            });

    qDebug() << "Starting catalog operation for:" << device->name;

    // CRITICAL: Start the actual catalog operation that will trigger onCatalogOperationCompleted()
    m_catalogManager->startCatalogJobStoppable(
        m_currentCatalogJob,
        device,
        CatalogJobStoppable::UpdateCatalog,
        m_databaseMode,
        m_collectionFolder
        );

    qDebug() << "Catalog operation started, waiting for completion...";
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

QList<qint64> DeviceUpdateManager::buildCatalogUpdateResults(Device* catalogDevice, const Storage::UpdateResult& storageResult)
{
    QList<qint64> results;

    // Catalog update results (first 7 elements)
    results << 1;  // Success
    results << catalogDevice->totalFileCount;
    results << 0;  // Delta files (would need calculation)
    results << catalogDevice->totalFileSize;
    results << 0;  // Delta size (would need calculation)
    results << 0;  // Reserved
    results << 0;  // Reserved

    // Storage update results (next 7 elements)
    results << (storageResult.wasUpdated ? 1 : 0);  // Storage success
    results << storageResult.newTotalSpace;
    results << storageResult.newFreeSpace;
    results << (storageResult.newTotalSpace - storageResult.newFreeSpace);  // Used space
    results << 0;  // Reserved
    results << 0;  // Reserved
    results << 0;  // Reserved

    return results;
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

    // Delegate stop to underlying CatalogManager if active
    if (m_catalogManager && m_catalogManager->catalogOperationRunning()) {
        qDebug() << "Delegating hard stop to CatalogManager";
        m_catalogManager->requestHardStop();
    }

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

    // For catalogs, gentle stop = hard stop (can't pause mid-catalog)
    if (m_catalogManager && m_catalogManager->catalogOperationRunning()) {
        qDebug() << "Delegating gentle stop to CatalogManager (will be hard stop for catalog)";
        m_catalogManager->requestGentleStop();
    }

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

    // Calculate skipped counts for logging only
    int skippedDevices = m_totalDevices - m_processedDevices;
    int skippedCatalogs = m_totalCatalogs - m_processedCatalogs;

    setStatus(QString("Operation cancelled - %1 catalogs skipped").arg(skippedCatalogs));

    // Remove the problematic operationCompleted emission
    // This was causing crashes because reportAllUpdates expects catalog update format,
    // not cancellation format. The UI properly handles cancellation via operationCancelled.

    emit operationCancelled();
    emit operationRunningChanged();

    cleanupOperation();
}

void DeviceUpdateManager::completeOperation()
{
    qDebug() << "=== DeviceUpdateManager::completeOperation ===";
    qDebug() << "*** COMPLETION PATH 3: completeOperation (WRONG - catalog-only data) ***";

    // Prevent duplicate completion
    if (!m_operationRunning) {
        qDebug() << "*** SKIPPING completeOperation - operation already completed by onCatalogOperationCompleted ***";
        return;
    }

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

    // Create results for operations NOT handled by onCatalogOperationCompleted
    QList<qint64> results;
    results << 1;  // Success flag

    if (m_currentDevice && m_currentDevice->type == "Catalog") {
        // CRITICAL NOTE: Single catalog operations should be handled by onCatalogOperationCompleted
        // This path should only be reached for multi-device operations or error cases
        qDebug() << "*** WARNING: completeOperation() called for single catalog - THIS CREATES CATALOG-ONLY REPORTS ***";

        // For single catalog: use actual catalog statistics
        results << m_currentDevice->totalFileCount;  // Total files
        results << 0;  // Delta files (would need to calculate)
        results << m_currentDevice->totalFileSize;   // Total size
        results << 0;  // Delta size (would need to calculate)
        // Add more zeros to pad the list (NO STORAGE DATA)
        for (int i = 5; i < 14; ++i) results << 0;
    } else {
        // For multi-device: use device counts
        results << m_processedCatalogs;
        results << 0;
        results << m_processedDevices;
        results << 0;
        for (int i = 5; i < 14; ++i) results << 0;
    }

    qDebug() << "*** EMITTING operationCompleted from completeOperation with CATALOG-ONLY data ***";
    qDebug() << "Results[0] (success):" << results[0];
    qDebug() << "Results[1] (catalog files):" << results[1];
    qDebug() << "Results[7] (storage updated):" << (results.size() > 7 ? results[7] : -1);

    emit operationCompleted(results);
    emit operationRunningChanged();

    cleanupOperation();
}

bool DeviceUpdateManager::shouldContinue() const
{
    return !m_stopRequested.loadAcquire() && m_operationRunning;
}

void DeviceUpdateManager::setCatalogProgressManager(CatalogProgressManager* catalogProgressManager)
{
    if (catalogProgressManager && m_catalogManager) {
        catalogProgressManager->connectToCatalogManager(m_catalogManager);
        qDebug() << "CatalogProgressManager connected to DeviceUpdateManager's CatalogManager";
    }
}

void DeviceUpdateManager::onCatalogOperationCompleted()
{
    qDebug() << "=== DeviceUpdateManager::onCatalogOperationCompleted ===";
    qDebug() << "*** CRITICAL DEBUG: This method was called! ***";
    qDebug() << "m_currentDevice:" << (m_currentDevice ? m_currentDevice->name : "NULL");
    qDebug() << "m_operationRunning:" << m_operationRunning;
    qDebug() << "m_updateType:" << m_updateType;

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
    m_currentDevice->updateParentsNumbers();

    // Use the fixed storage update method
    Storage::UpdateResult storageResult = updateParentStorage(m_currentDevice);
    QList<qint64> results = buildCatalogUpdateResults(m_currentDevice, storageResult);

    qDebug() << "*** STORAGE UPDATE DEBUG ***";
    qDebug() << "Storage result wasUpdated:" << storageResult.wasUpdated;
    qDebug() << "Storage result errorMessage:" << storageResult.errorMessage;
    qDebug() << "Results[7] (storage updated flag):" << (results.size() > 7 ? results[7] : -1);

    qDebug() << "*** EMITTING operationCompleted from onCatalogOperationCompleted with COMBINED data ***";
    qDebug() << "Results[0] (success):" << results[0];
    qDebug() << "Results[1] (catalog files):" << results[1];
    qDebug() << "Results[7] (storage updated):" << (results.size() > 7 ? results[7] : -1);

    // Mark operation as completed to prevent duplicate completion
    m_operationRunning = false;
    m_waitingForCatalogCompletion = false;

    // Ensure reliable signal delivery
    emit operationCompleted(results);
    emit operationRunningChanged();

    // Use timer for cleanup to ensure signals are processed first
    QTimer::singleShot(10, this, [this]() {
        cleanupOperation();
    });

    qDebug() << "*** CRITICAL DEBUG: About to emit operationCompleted signal ***";
    qDebug() << "Results size:" << results.size();
    qDebug() << "Results[0] (success):" << results[0];
    if (results.size() > 1) qDebug() << "Results[1] (files):" << results[1];
    if (results.size() > 7) qDebug() << "Results[7] (storage):" << results[7];

    emit operationCompleted(results);
    qDebug() << "*** CRITICAL DEBUG: operationCompleted signal emitted! ***";

    emit operationRunningChanged();

    cleanupOperation();

    qDebug() << "=== DeviceUpdateManager::onCatalogOperationCompleted EXIT ===";
}

Storage::UpdateResult DeviceUpdateManager::updateParentStorage(Device* catalogDevice)
{
    qDebug() << "=== DeviceUpdateManager::updateParentStorage (FIXED) ===";

    if (!catalogDevice || catalogDevice->parentID == 0) {
        qDebug() << "No parent device to update";
        return Storage::UpdateResult{}; // Empty result
    }

    try {
        qDebug() << "Loading parent storage device...";
        Device parentDevice;
        parentDevice.ID = catalogDevice->parentID;
        parentDevice.loadDevice("defaultConnection");

        if (parentDevice.type == "Storage" && parentDevice.storage) {
            qDebug() << "Updating parent storage space info for:" << parentDevice.name;

            // Only call updateStorageInfo once and capture the result
            parentDevice.storage->path = parentDevice.path;
            Storage::UpdateResult result = parentDevice.storage->updateStorageInfo();

            qDebug() << "Storage update result:";
            qDebug() << "  wasUpdated:" << result.wasUpdated;
            qDebug() << "  errorCode:" << result.errorCode;
            qDebug() << "  errorMessage:" << result.errorMessage;

            if (result.wasUpdated) {
                // Update device values with new storage information
                parentDevice.freeSpace = result.newFreeSpace;
                parentDevice.totalSpace = result.newTotalSpace;
                parentDevice.dateTimeUpdated = QDateTime::currentDateTime();

                // Save updated storage device
                parentDevice.saveDevice();
                parentDevice.saveStatistics(parentDevice.dateTimeUpdated, "update");

                qDebug() << "Parent storage device updated and saved successfully";
                qDebug() << "New total space:" << result.newTotalSpace;
                qDebug() << "New free space:" << result.newFreeSpace;
            } else {
                qDebug() << "Storage space not updated - Reason:" << result.errorMessage;
            }

            return result; // Return the actual result from updateStorageInfo
        } else {
            qDebug() << "Parent device is not a storage or has no storage object";
            Storage::UpdateResult emptyResult;
            emptyResult.wasUpdated = false;
            emptyResult.errorMessage = "Parent device is not a storage device";
            return emptyResult;
        }

    } catch (const std::exception& e) {
        qDebug() << "Error updating parent storage device:" << e.what();
        Storage::UpdateResult errorResult;
        errorResult.wasUpdated = false;
        errorResult.errorMessage = QString("Exception: %1").arg(e.what());
        return errorResult;
    }
}
