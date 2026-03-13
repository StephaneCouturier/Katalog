#include "deviceupdatemanager.h"
#include <QDebug>
#include <QTimer>
#include <QSqlQuery>
#include <QSqlDatabase>
#include <QCoreApplication>

DeviceUpdateManager::DeviceUpdateManager(QObject *parent)
    : QObject(parent)
{
    setupCatalogManager();  // ADD THIS LINE
}

DeviceUpdateManager::~DeviceUpdateManager()
{
    cleanupOperation();
}

void DeviceUpdateManager::setupCatalogManager()
{

    m_catalogManager = new CatalogManager(this);

    connect(m_catalogManager, &CatalogManager::catalogOperationCompleted,
            this, &DeviceUpdateManager::onCatalogOperationCompleted);
    connect(m_catalogManager, &CatalogManager::catalogOperationError,
            this, &DeviceUpdateManager::onCatalogOperationError);
    connect(m_catalogManager, &CatalogManager::catalogOperationCancelled,
            this, &DeviceUpdateManager::onCatalogOperationCancelled);

}

void DeviceUpdateManager::cleanupCatalogJob()
{
    if (m_currentCatalogJob) {
        m_currentCatalogJob->setParent(nullptr);
        m_currentCatalogJob->deleteLater();
        m_currentCatalogJob = nullptr;
    }
}

void DeviceUpdateManager::continueToNextDevice()
{

    // Check if operation already completed to prevent duplicate reports
    if (!m_operationRunning) {
        return;
    }


    // For single device operation, complete
    if (m_allDevices.size() <= 1) {
        completeOperation();
        return;
    }

    // For multi-device operations, check if all devices are processed
    if (m_processedDevices >= m_totalDevices) {
        completeOperation();
        return;
    }

    processNextInQueue();
}

void DeviceUpdateManager::processNextInQueue()
{

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
        m_storageWasUpdated = false;
        return;
    }

    try {
        Device parentDevice;
        parentDevice.ID = device->parentID;
        parentDevice.loadDevice(m_connectionName);

        if (parentDevice.type == "Storage" && parentDevice.storage) {

            // Refresh physical storage space information and capture results
            m_storageUpdateResult = parentDevice.storage->updateStorageInfo();

            if (m_storageUpdateResult.wasUpdated) {

                // Update device values with new storage information
                parentDevice.totalSpace = m_storageUpdateResult.newTotalSpace;
                parentDevice.freeSpace = m_storageUpdateResult.newFreeSpace;
                parentDevice.dateTimeUpdated = QDateTime::currentDateTime();

                // Save updated storage device
                parentDevice.saveDevice();

                // Save storage statistics
                parentDevice.saveStatistics(parentDevice.dateTimeUpdated, "update");

                // Mark that storage was successfully updated
                m_storageWasUpdated = true;

            } else {
                m_storageWasUpdated = false;
            }
        } else {
            m_storageWasUpdated = false;
        }

    } catch (const std::exception& e) {
        qWarning() << "WARNING: Error updating parent storage device:" << e.what();
        m_storageWasUpdated = false;
    }
}

void DeviceUpdateManager::updateDeviceHierarchy(Device* rootDevice,
                                                const QString& databaseMode,
                                                const QString& collectionFolder,
                                                const QString& updateType)
{

    if (!rootDevice) {
        emit operationError("Invalid root device provided");
        return;
    }

    if (m_operationRunning) {
        emit operationError("Operation already running");
        return;
    }

    // Ensure complete clean state before starting
    cleanupOperation();

    // Initialize operation
    m_operationRunning = true;
    m_stopRequested.storeRelease(0);
    m_gentleStopRequested.storeRelease(0);
    m_isPaused = false;
    m_rootDevice = rootDevice;
    m_databaseMode = databaseMode;
    m_collectionFolder = collectionFolder;
    m_updateType = updateType;
    m_operationStartTime = QDateTime::currentDateTime();

    // Initialize storage batch tracking if root is Storage
    if (rootDevice->type == "Storage" && updateType == "update") {
        initializeStorageBatch();
    }

    // Analyze hierarchy to get totals for progress
    analyzeHierarchy(rootDevice);

    //setStatus(QString(tr("Starting %1 operation...")).arg(updateType));
    emit operationStarted();
    emit operationRunningChanged();

    // Start recursive update
    updateDeviceRecursive(rootDevice);
}

Device* DeviceUpdateManager::createDummyDeviceFromList(const QList<Device*>& filteredDevices)
{

    Device* tempDevice = new Device();
    tempDevice->ID = -1;
    tempDevice->name = "Filtered Selection";
    tempDevice->type = "Virtual";
    tempDevice->active = true;
    tempDevice->hasSubDevice = !filteredDevices.isEmpty();

    for (Device* device : filteredDevices) {
        tempDevice->subDevices.append(*device);
        tempDevice->deviceIDList.append(device->ID);
    }

    m_dummyDevice = tempDevice;

    return tempDevice;
}

void DeviceUpdateManager::processNextDevice()
{

    if (!shouldContinue()) {
        handleOperationCancellation();
        return;
    }

    // For Storage batch operations, check if there are more children to process
    if (m_rootDevice && m_rootDevice->type == "Storage") {

        // If there are unprocessed catalog children, continue processing
        // This logic depends on how children are tracked in the recursive system
        // For now, we'll check if all expected catalogs have been processed

        int expectedCatalogs = 0;
        if (m_rootDevice->hasSubDevice) {
            // Count expected active catalog children
            for (const Device& child : m_rootDevice->subDevices) {
                if (child.type == "Catalog" && child.active) {
                    expectedCatalogs++;
                }
            }
        }


        if (m_processedCatalogs < expectedCatalogs) {
            return; // More catalogs to process - wait for next onCatalogOperationCompleted
        } else {
            completeOperation();
            return;
        }
    }

    // For other device types, complete immediately
    completeOperation();
}

QString DeviceUpdateManager::currentDeviceName() const
{
    return m_currentDevice ? m_currentDevice->name : QString();
}

void DeviceUpdateManager::processChildren(Device* device)
{

    // Catalog devices should never call processChildren
    if (device->type == "Catalog") {
        qWarning() << "WARNING: *** ERROR: processChildren called for catalog device - this should never happen! ***";
        return;
    }

    if (!device || !shouldContinue()) {
        if (!shouldContinue()) {
            handleOperationCancellation();
        }
        return;
    }

    // Load children if needed
    if (device->subDevices.isEmpty()) {
        loadDeviceChildren(device);
    }

    // If no children, this device is complete
    if (device->subDevices.isEmpty()) {
        updateParentNumbers(device);
        emit deviceProcessingCompleted(device->name);
        continueToNextDevice();
        return;
    }


    // Storage devices with children use batch processing
    if (device->type == "Storage") {
        if (m_processingContext == VirtualChild) {
            // Don't use batch processing - just process children recursively
            for (const Device& childDevice : device->subDevices) {
                if (!shouldContinue()) {
                    handleOperationCancellation();
                    return;
                }

                Device* childPtr = new Device(childDevice);
                updateDeviceRecursive(childPtr);

                if (m_waitingForCatalogCompletion) {
                    return;
                }
            }

            // All children processed
            updateParentNumbers(device);
            emit deviceProcessingCompleted(device->name);
            continueToNextDevice();
            return;
        } else {
            initializeStorageBatchProcessing(device);
            processNextStorageChild();
            return;
        }
    }

    // For Virtual devices, process children directly without recursive flow control
    if (device->type == "Virtual") {

        // DEBUG: Show initial counter values

        m_virtualDeviceBeingProcessed = device;
        m_remainingVirtualChildren.clear();

        // RESET counters to avoid double counting from hierarchy analysis
        m_processedStorageDevices = 0;
        m_updatedCatalogs = 0;
        m_skippedCatalogs = 0;
        m_totalCatalogFiles = 0;
        m_totalCatalogSize = 0;


        // Add all children to remaining list
        for (const Device& childDevice : device->subDevices) {
            Device* childPtr = new Device(childDevice);
            childPtr->updateActiveState(m_connectionName);
            m_remainingVirtualChildren.append(childPtr);
        }

        // Process first child
        processNextVirtualChild();
        return;
    }

    // All children processed
    updateParentNumbers(device);
    emit deviceProcessingCompleted(device->name);
    continueToNextDevice();
}

void DeviceUpdateManager::updateDeviceRecursive(Device* device)
{

    if (!device || !shouldContinue()) {
        if (!shouldContinue()) {
            handleOperationCancellation();
        }
        return;
    }

    // Check for gentle stop before starting new catalog
    if (m_gentleStopRequested.loadAcquire() && device->type == "Catalog") {
        completeOperation();
        return;
    }

    // Check for hard stop
    if (m_stopRequested.loadAcquire()) {
        handleOperationCancellation();
        return;
    }

    setCurrentDevice(device);
    emit deviceProcessingStarted(device->name, device->type);

    // Update device active state
    device->updateActiveState(m_connectionName);
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
        // Do NOT call processChildren here

    } else {
        updateVirtualDevice(device);
        processChildren(device);
    }
}

void DeviceUpdateManager::updateCatalogDevice(Device* device)
{
    //setStatus(QString(tr("Updating device: %1")).arg(device->name));

    if (!device->active) {

        // Handle inactive catalog - mark as processed and continue
        m_processedDevices++;
        updateProgress();
        emit deviceProcessingCompleted(device->name);

        // For Storage batch operations, this should continue to next child
        if (m_currentStorageDevice) {
            processNextStorageChild();
        } else {
            continueToNextDevice();
        }
        return;
    }

    if (!m_catalogManager) {
        handleOperationError("CatalogManager not available");
        return;
    }

    // CRITICAL: Ensure CatalogManager is in clean state before starting

    if (m_catalogManager->catalogOperationRunning()) {

        // Force stop the running operation
        m_catalogManager->stopCatalogOperation();

        // Give it a moment to clean up
        QTimer::singleShot(100, this, [this, device]() {
            updateCatalogDevice(device);  // Retry after cleanup
        });
        return;
    }

    // Start catalog operation
    m_waitingForCatalogCompletion = true;

    startCatalogOperation(device);
}

void DeviceUpdateManager::startCatalogOperation(Device* device)
{

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

    //Determine correct operation type based on m_updateType
    CatalogJobStoppable::OperationType operationType;
    if (m_updateType == "create") {
        operationType = CatalogJobStoppable::CreateCatalog;
    } else {
        operationType = CatalogJobStoppable::UpdateCatalog;
    }

    // Start the actual catalog operation
    m_catalogManager->startCatalogJobStoppable(
        m_currentCatalogJob,
        device,
        operationType,
        m_databaseMode,
        m_collectionFolder
        );

}

void DeviceUpdateManager::setCatalogProgressManager(CatalogProgressManager* catalogProgressManager)
{
    m_catalogProgressManager = catalogProgressManager;

    if (catalogProgressManager && m_catalogManager) {
        // Connect the existing CatalogProgressManager to our CatalogManager
        m_catalogProgressManager->connectToCatalogManager(m_catalogManager);
    }
}

void DeviceUpdateManager::updateStorageDevice(Device* device)
{

    // Initialize storage update result
    m_storageWasUpdated = false;
    m_storageUpdateResult = Storage::UpdateResult{};

    // Update storage space information (fast operation)
    if (device->storage) {
        device->storage->path = device->path;

        try {
            // Update storage space info using existing Storage class method
            Storage::UpdateResult result = device->storage->updateStorageInfo();

            if (result.wasUpdated) {
                device->freeSpace = result.newFreeSpace;
                device->totalSpace = result.newTotalSpace;

                // Store results for reporting
                m_storageUpdateResult = result;
                m_storageWasUpdated = true;

                // Accumulate Virtual storage updates
                if (m_rootDevice && m_rootDevice->type == "Virtual") {
                    if (!m_virtualStorageWasUpdated) {
                        // First storage device - initialize
                        m_virtualStorageUpdateResult = result;
                        m_virtualStorageWasUpdated = true;
                    } else {
                        // Subsequent storage devices - accumulate
                        m_virtualStorageUpdateResult.newFreeSpace += result.newFreeSpace;
                        m_virtualStorageUpdateResult.newTotalSpace += result.newTotalSpace;
                        m_virtualStorageUpdateResult.deltaFreeSpace += result.deltaFreeSpace;
                        m_virtualStorageUpdateResult.deltaTotalSpace += result.deltaTotalSpace;

                        // Calculate accumulated used space
                        m_virtualStorageUpdateResult.newUsedSpace =
                            m_virtualStorageUpdateResult.newTotalSpace - m_virtualStorageUpdateResult.newFreeSpace;
                        m_virtualStorageUpdateResult.deltaUsedSpace += result.deltaUsedSpace;

                    }

                }

                // Update parent device values
                device->dateTimeUpdated = QDateTime::currentDateTime();
                device->saveDevice();
                device->saveStatistics(device->dateTimeUpdated, "update");

            } else {
                m_storageWasUpdated = false;
            }
        } catch (const std::exception& e) {
            qWarning() << "WARNING: Error updating storage info:" << e.what();
            m_storageWasUpdated = false;
        }
    }

    m_processedDevices++;
    updateProgress();

    emit deviceProcessingCompleted(device->name);
}

void DeviceUpdateManager::updateVirtualDevice(Device* device)
{

    // Virtual devices just aggregate numbers from children
    // The actual aggregation happens after children are processed
    device->saveDevice();

    m_processedDevices++;
    updateProgress();

    emit deviceProcessingCompleted(device->name);

    // Virtual devices continue to process their children in processChildren()
}

void DeviceUpdateManager::stopOperation()
{
    m_stopRequested.storeRelease(1);
    handleOperationCancellation();
}

QList<qint64> DeviceUpdateManager::buildCatalogUpdateResults(Device* catalogDevice, const Storage::UpdateResult& storageResult)
{

    QList<qint64> results;

    // Get the actual catalog deltas from CatalogJobStoppable
    qint64 deltaFileCount = 0;
    qint64 deltaTotalFileSize = 0;

    if (m_currentCatalogJob) {
        QList<qint64> catalogResults = m_currentCatalogJob->getResults();
        if (catalogResults.size() >= 5) {
            deltaFileCount = catalogResults[2];      // Actual delta files
            deltaTotalFileSize = catalogResults[4];  // Actual delta size
        } else {
            qWarning() << "WARNING: Invalid catalog results size:" << catalogResults.size();
        }
    } else {
        qWarning() << "WARNING: No current catalog job - deltas will be 0";
    }

    // Index 0-6: Catalog update results
    results << 1;  // Index 0: Success flag
    results << catalogDevice->totalFileCount;  // Index 1: Total files after update
    results << deltaFileCount;                 // Index 2: FIXED - Actual delta files
    results << catalogDevice->totalFileSize;   // Index 3: Total file size after update
    results << deltaTotalFileSize;             // Index 4: FIXED - Actual delta size
    results << 1;  // Index 5: Updated catalogs count (1 for single catalog)
    results << 0;  // Index 6: Skipped catalogs count (0 for successful single catalog)

    // Index 7-13: Storage update results (keep existing logic)
    if (storageResult.wasUpdated) {
        results << 1;  // Index 7: Storage updated flag (1 = true)
        results << storageResult.newUsedSpace;     // Index 8: Used space
        results << storageResult.deltaUsedSpace;   // Index 9: Delta used space
        results << storageResult.newFreeSpace;     // Index 10: Free space
        results << storageResult.deltaFreeSpace;   // Index 11: Delta free space
        results << storageResult.newTotalSpace;    // Index 12: Total space
        results << storageResult.deltaTotalSpace;  // Index 13: Delta total space
    } else {
        results << 0;  // Index 7: Storage updated flag (0 = false)
        for (int i = 8; i < 14; ++i) results << 0;  // No storage values
    }


    return results;
}

void DeviceUpdateManager::loadDeviceChildren(Device* device)
{
    if (!device) return;


    try {
        // Clear any existing children data first
        device->subDevices.clear();
        device->deviceIDList.clear();

        // Force reload the device which populates deviceIDList
        device->loadDevice(m_connectionName);


        for (int childID : std::as_const(device->deviceIDList)) {
            Device childDevice;
            childDevice.ID = childID;
            childDevice.loadDevice(m_connectionName);
            device->subDevices.append(childDevice);

        }


    } catch (const std::exception& e) {
        qWarning() << "WARNING: Error loading children for device" << device->name << ":" << e.what();
    }
}

void DeviceUpdateManager::onCatalogOperationError(const QString& error)
{

    if (m_currentCatalogJob) {
        m_currentCatalogJob->setParent(nullptr);
        m_currentCatalogJob->deleteLater();
        m_currentCatalogJob = nullptr;
    }

    m_waitingForCatalogCompletion = false;
    handleOperationError(error);
}

void DeviceUpdateManager::onCatalogOperationCancelled()
{

    if (m_currentCatalogJob) {
        m_currentCatalogJob->setParent(nullptr);
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

}

void DeviceUpdateManager::buildDeviceList(Device* device, QList<Device*>& deviceList)
{
    if (!device) return;

    deviceList.append(device);

    // FIXED: Use proper method to load children that converts deviceIDList to subDevices
    if (device->subDevices.isEmpty() && device->hasSubDevice) {
        loadDeviceChildren(device);  // This is the key fix!
    } else if (device->subDevices.isEmpty()) {
        // Force load to check if there are actually children
        loadDeviceChildren(device);
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


    try {
        device->updateParentsNumbers();
    } catch (const std::exception& e) {
        qWarning() << "WARNING: Error updating parent numbers for" << device->name << ":" << e.what();
    }
}

void DeviceUpdateManager::updateRelatedDevices(Device* device)
{
    if (!device || device->type != "Catalog") return;


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

void DeviceUpdateManager::saveDeviceStatistics(Device* device)
{
    if (!device) return;

    try {
        device->saveStatistics(device->dateTimeUpdated, "update");
    } catch (const std::exception& e) {
        qWarning() << "WARNING: Error saving device statistics:" << e.what();
    }
}

void DeviceUpdateManager::handleOperationError(const QString& error)
{

    m_operationRunning = false;
    setStatus(QString("Operation failed: %1").arg(error));

    emit operationError(error);
    emit operationRunningChanged();

    cleanupOperation();
}

void DeviceUpdateManager::cleanupOperation()
{

    // Reset operation state
    m_operationRunning = false;
    m_waitingForCatalogCompletion = false;

    // Clear operation data - COMPLETE RESET
    m_rootDevice = nullptr;
    m_currentDevice = nullptr;
    m_currentStorageDevice = nullptr;        // CRITICAL: Reset storage device pointer
    m_processedDevices = 0;
    m_totalDevices = 0;
    m_processedCatalogs = 0;
    m_totalCatalogs = 0;
    m_currentChildIndex = 0;                 // Reset child processing index

    // Clear catalog counters
    m_updatedCatalogs = 0;
    m_skippedCatalogs = 0;
    m_totalCatalogFiles = 0;
    m_totalCatalogSize = 0;
    m_totalDeltaFiles = 0;
    m_totalDeltaSize = 0;

    // Clear storage update data
    m_storageWasUpdated = false;
    m_storageUpdateResult = Storage::UpdateResult{};

    // Clear Virtual storage tracking
    m_virtualStorageWasUpdated = false;
    m_virtualStorageUpdateResult = Storage::UpdateResult{};
    m_processedStorageDevices = 0;

    // Clear children processing - ONLY clear lists, don't delete objects
    m_childrenToProcess.clear();
    m_allDevices.clear();
    m_catalogDevices.clear();

    // Clear results
    m_accumulatedResults.clear();

    // Reset update type - CRITICAL for proper completion logic
    m_updateType.clear();
    m_databaseMode.clear();
    m_collectionFolder.clear();

    // Reset flags
    m_stopRequested.storeRelease(0);
    m_gentleStopRequested.storeRelease(0);
    m_isPaused = false;

    // Clean up catalog job
    if (m_currentCatalogJob) {
        m_currentCatalogJob->setParent(nullptr);
        m_currentCatalogJob->deleteLater();
        m_currentCatalogJob = nullptr;
    }

    // Clear batch context from previous operation
    if (m_catalogProgressManager) {
        m_catalogProgressManager->clearBatchContext();
    }

    // Disconnect from CatalogManager - but DON'T delete it
    if (m_catalogManager) {
        disconnect(m_catalogManager, nullptr, this, nullptr);
        // Reconnect for next operation
        connect(m_catalogManager, &CatalogManager::catalogOperationCompleted,
                this, &DeviceUpdateManager::onCatalogOperationCompleted);
        connect(m_catalogManager, &CatalogManager::catalogOperationError,
                this, &DeviceUpdateManager::onCatalogOperationError);
        connect(m_catalogManager, &CatalogManager::catalogOperationCancelled,
                this, &DeviceUpdateManager::onCatalogOperationCancelled);
    }

    m_progress = 0;
    emit progressChanged();

    emit operationRunningChanged();

}

void DeviceUpdateManager::cleanupDummyDevice()
{
    if (m_dummyDevice) {
        delete m_dummyDevice;
        m_dummyDevice = nullptr;
    }
}

void DeviceUpdateManager::requestHardStop()
{

    m_stopRequested.storeRelease(1);
    m_gentleStopRequested.storeRelease(1); // Also set gentle stop for consistency

    // Delegate stop to underlying CatalogManager if active
    if (m_catalogManager && m_catalogManager->catalogOperationRunning()) {
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

    m_gentleStopRequested.storeRelease(1);
    setStatus(QCoreApplication::translate("MainWindow", "Stopping after current catalog completes..."));

    // Don't delegate to CatalogManager for storage batch operations
    // because CatalogManager is not in batch mode and will do hard stop

    bool isStorageBatchOperation = (m_currentStorageDevice != nullptr &&
                                    m_rootDevice && m_rootDevice->type == "Storage");

    if (isStorageBatchOperation) {
        // Don't call CatalogManager - let current catalog complete naturally
        // The gentle stop will be handled in onCatalogOperationCompleted()
    } else {
        // For non-storage operations, delegate to CatalogManager as before
        if (m_catalogManager && m_catalogManager->catalogOperationRunning()) {
            m_catalogManager->requestGentleStop();
        }
    }

    // If no catalog is currently running, stop immediately
    if (!m_waitingForCatalogCompletion) {
        QTimer::singleShot(10, this, [this]() {
            completeOperation();
        });
    }
}

void DeviceUpdateManager::handleOperationCancellation()
{

    // ENHANCED: For Storage batch operations, update counts properly
    if (m_currentStorageDevice) {

        // Calculate remaining catalogs that won't be processed
        int remainingChildren = m_childrenToProcess.size() - m_currentChildIndex;
        if (remainingChildren > 0) {
            // We could add these to skipped count if needed for reporting
        }
    }

    m_operationRunning = false;
    m_waitingForCatalogCompletion = false;

    // Calculate skipped counts for logging only
    // int skippedDevices = m_totalDevices - m_processedDevices;
    // int skippedCatalogs = m_totalCatalogs - m_processedCatalogs;

    // setStatus(QString(tr("Operation cancelled - %1 devices skipped, %2 catalogs skipped"))
    //               .arg(skippedDevices).arg(skippedCatalogs));
    setStatus(QString(QCoreApplication::translate("MainWindow", "Operation cancelled")));

    // Emit cancellation (no results - MainWindow handles this differently)
    emit operationCancelled();
    emit operationRunningChanged();

    cleanupOperation();
}

void DeviceUpdateManager::completeOperation()
{

    if (!m_operationRunning) {
        return;
    }


    // Update parent numbers for entire hierarchy
    if (m_rootDevice) {
        try {
            m_rootDevice->updateParentsNumbers();
        } catch (const std::exception& e) {
            qWarning() << "WARNING: Error updating parent numbers:" << e.what();
        }
    }

    m_operationRunning = false;
    m_waitingForCatalogCompletion = false;

    QList<qint64> results;

    if (m_rootDevice && m_rootDevice->type == "Storage") {
        results = buildStorageBatchResults(m_rootDevice);

    } else if (m_rootDevice && m_rootDevice->type == "Virtual") {

        // Save Virtual device statistics after all children processed

        try {
            // Update Virtual device's timestamp and aggregated values
            m_rootDevice->dateTimeUpdated = QDateTime::currentDateTime();

            // The aggregated file counts and sizes should already be updated by updateParentsNumbers() above
            // but we can log them for verification

            // Save the Virtual device to database with updated timestamp
            m_rootDevice->saveDevice();

            // Save Virtual device statistics - this records the update event
            m_rootDevice->saveStatistics(m_rootDevice->dateTimeUpdated, "update");


        } catch (const std::exception& e) {
            qWarning() << "WARNING: Error saving Virtual device statistics:" << e.what();
        }

        // Build results (existing code continues...)

        // Build results array systematically
        results.clear();

        results << 1;                     // Index 0: Success flag
        results << m_totalCatalogFiles;   // Index 1: Total files from all catalogs
        results << 0;                     // Index 2: Delta files
        results << m_totalCatalogSize;    // Index 3: Total size from all catalogs
        results << 0;                     // Index 4: Delta size
        results << m_updatedCatalogs;     // Index 5: Updated catalogs
        results << m_skippedCatalogs;     // Index 6: Skipped catalogs

        // Always add storage section (indices 7-13)
        if (m_virtualStorageWasUpdated && m_processedStorageDevices > 0) {
            results << 1;  // Index 7: Storage updated flag
            results << m_virtualStorageUpdateResult.newUsedSpace;     // Index 8
            results << m_virtualStorageUpdateResult.deltaUsedSpace;   // Index 9
            results << m_virtualStorageUpdateResult.newFreeSpace;     // Index 10
            results << m_virtualStorageUpdateResult.deltaFreeSpace;   // Index 11
            results << m_virtualStorageUpdateResult.newTotalSpace;    // Index 12
            results << m_virtualStorageUpdateResult.deltaTotalSpace;  // Index 13
        } else {
            results << 0;  // Index 7: No storage updated
            results << 0;  // Index 8: Used space
            results << 0;  // Index 9: Delta used space
            results << 0;  // Index 10: Free space
            results << 0;  // Index 11: Delta free space
            results << 0;  // Index 12: Total space
            results << 0;  // Index 13: Delta total space
        }


    } else {

        results << 1;                    // Success flag
        results << m_processedCatalogs;  // Use catalog count
        results << 0;                    // Delta
        results << m_processedDevices;   // Use device count
        results << 0;                    // Delta
        for (int i = 5; i < 14; ++i) results << 0;  // Pad with zeros to index 13
    }


    emit operationCompleted(results);
    emit operationRunningChanged();

    QTimer::singleShot(10, this, [this]() {
        cleanupOperation();
    });
}

bool DeviceUpdateManager::shouldContinue() const
{
    return !m_stopRequested.loadAcquire() && m_operationRunning;
}

void DeviceUpdateManager::onCatalogOperationCompleted()
{
   // qDebug() << "m_pendingVirtualChildren.size():" << m_pendingVirtualChildren.size();

    if (!m_currentDevice) {
        qWarning() << "WARNING: No current device in catalog completion";
        return;
    }


    // Update catalog device and save changes
    m_processedDevices++;
    m_processedCatalogs++;
    updateProgress();

    m_currentDevice->loadDevice(m_connectionName);
    m_currentDevice->saveDevice();
    m_currentDevice->saveStatistics(m_currentDevice->dateTimeUpdated, "update");
    updateRelatedDevices(m_currentDevice);

    emit deviceProcessingCompleted(m_currentDevice->name);

    m_waitingForCatalogCompletion = false;

    // Update parent numbers for this catalog
    m_currentDevice->updateParentsNumbers();

    if (m_virtualDeviceBeingProcessed != nullptr && m_currentDevice) {

        // DEBUG: Show values before incrementing

        // Count successful catalog update
        m_updatedCatalogs++;

        // Accumulate results
        m_totalCatalogFiles += m_currentDevice->totalFileCount;
        m_totalCatalogSize += m_currentDevice->totalFileSize;

        // Emit individual report signal for this catalog
        QList<qint64> catalogResults = buildCatalogUpdateResults(m_currentDevice, m_storageUpdateResult);
        emit catalogCompletedInBatch(m_currentDevice, catalogResults);

        // Continue with next Virtual child
        QTimer::singleShot(50, this, [this]() {
            processNextVirtualChild();
        });
        return;
    } else {
    }

    // if (m_rootDevice && m_rootDevice->type == "Virtual") {
    //     qDebug() << "=== Virtual operation - NOT completing, letting operation continue ===";
    //     return;  // Just return - don't complete the operation
    // }

    // Determine if this is part of a Storage batch operation
    // Creation operations should NEVER be treated as batch operations
    bool isStorageBatchOperation = (m_currentStorageDevice != nullptr && m_updateType == "update" && m_rootDevice && m_rootDevice->type == "Storage");

    if (isStorageBatchOperation) {

        // Accumulate results for this catalog
        accumulateStorageResults(m_currentDevice);

        // Update storage space after catalog completion (like the original Device::updateDevice logic)
        if (m_currentDevice->parentID != 0) {
            Storage::UpdateResult storageResult = updateParentStorage(m_currentDevice);
            if (storageResult.wasUpdated && !m_storageWasUpdated) {
                // Store the first successful storage update for reporting
                m_storageUpdateResult = storageResult;
                m_storageWasUpdated = true;
            }
        }

        // Check for gentle stop request before starting next catalog
        if (m_gentleStopRequested.loadAcquire()) {
            completeStorageBatchOperation();
            return;
        }

        // Defer the next catalog start to allow CatalogManager cleanup
        QTimer::singleShot(50, this, [this]() {
            // Check gentle stop again in the timer callback
            if (m_gentleStopRequested.loadAcquire()) {
                completeStorageBatchOperation();
                return;
            }

            processNextStorageChild();
        });

        return; // Don't emit operationCompleted yet - wait for all catalogs

    } else {

        // Single catalog operation - update parent storage and emit results
        Storage::UpdateResult storageResult = updateParentStorage(m_currentDevice);
        QList<qint64> results = buildCatalogUpdateResults(m_currentDevice, storageResult);


        // Mark operation as completed
        m_operationRunning = false;
        m_waitingForCatalogCompletion = false;

        // Emit completion with combined catalog + storage data
        emit operationCompleted(results);
        emit operationRunningChanged();

        // Cleanup
        QTimer::singleShot(10, this, [this]() {
            cleanupOperation();
        });
    }
}

Storage::UpdateResult DeviceUpdateManager::updateParentStorage(Device* catalogDevice)
{

    if (!catalogDevice || catalogDevice->parentID == 0) {
        return Storage::UpdateResult{}; // Empty result
    }

    try {
        Device parentDevice;
        parentDevice.ID = catalogDevice->parentID;
        parentDevice.loadDevice(m_connectionName);

        if (parentDevice.type == "Storage" && parentDevice.storage) {

            // Only call updateStorageInfo once and capture the result
            parentDevice.storage->path = parentDevice.path;
            Storage::UpdateResult result = parentDevice.storage->updateStorageInfo();


            if (result.wasUpdated) {
                // Update device values with new storage information
                parentDevice.freeSpace = result.newFreeSpace;
                parentDevice.totalSpace = result.newTotalSpace;
                parentDevice.dateTimeUpdated = QDateTime::currentDateTime();

                // Save updated storage device
                parentDevice.saveDevice();
                parentDevice.saveStatistics(parentDevice.dateTimeUpdated, "update");

            } else {
            }

            return result; // Return the actual result from updateStorageInfo
        } else {
            Storage::UpdateResult emptyResult;
            emptyResult.wasUpdated = false;
            emptyResult.errorMessage = "Parent device is not a storage device";
            return emptyResult;
        }

    } catch (const std::exception& e) {
        qWarning() << "WARNING: Error updating parent storage device:" << e.what();
        Storage::UpdateResult errorResult;
        errorResult.wasUpdated = false;
        errorResult.errorMessage = QString("Exception: %1").arg(e.what());
        return errorResult;
    }
}

QList<qint64> DeviceUpdateManager::buildStorageBatchResults(Device* storageDevice)
{

    QList<qint64> results;

    // Index 0-6: Catalog batch results (matching "list" updateType format)
    results << 1;  // Index 0: Success flag
    results << m_totalCatalogFiles;  // Index 1: Total files from all catalogs
    results << m_totalDeltaFiles;    // Index 2: FIXED - Accumulated delta files
    results << m_totalCatalogSize;   // Index 3: Total file size from all catalogs
    results << m_totalDeltaSize;     // Index 4: FIXED - Accumulated delta size
    results << m_updatedCatalogs;    // Index 5: Updated catalogs count
    results << m_skippedCatalogs;    // Index 6: Skipped catalogs count

    // Index 7-13: Storage update results (keep existing logic)
    if (m_storageWasUpdated && m_storageUpdateResult.wasUpdated) {
        results << 1;  // Index 7: Storage updated flag
        results << m_storageUpdateResult.newUsedSpace;     // Index 8: Used space
        results << m_storageUpdateResult.deltaUsedSpace;   // Index 9: Delta used space
        results << m_storageUpdateResult.newFreeSpace;     // Index 10: Free space
        results << m_storageUpdateResult.deltaFreeSpace;   // Index 11: Delta free space
        results << m_storageUpdateResult.newTotalSpace;    // Index 12: Total space
        results << m_storageUpdateResult.deltaTotalSpace;  // Index 13: Delta total space
    } else {
        results << 0;  // Index 7: Storage updated flag (0 = false)
        for (int i = 8; i < 14; ++i) results << 0;  // No storage values
    }


    return results;
}

void DeviceUpdateManager::initializeStorageBatch()
{
    m_updatedCatalogs = 0;
    m_skippedCatalogs = 0;
    m_totalCatalogFiles = 0;
    m_totalCatalogSize = 0;
    m_storageWasUpdated = false;
    m_storageUpdateResult = Storage::UpdateResult{};
    m_totalDeltaFiles = 0;
    m_totalDeltaSize = 0;
}

void DeviceUpdateManager::initializeStorageBatchProcessing(Device* storageDevice)
{

    m_currentStorageDevice = storageDevice;
    m_currentChildIndex = 0;

    // Build list of children to process (create copies for processing)
    m_childrenToProcess.clear();
    for (const Device& childDevice : storageDevice->subDevices) {
        Device* childPtr = new Device(childDevice);

        // Ensure active state is correctly updated
        childPtr->updateActiveState(m_connectionName);

        m_childrenToProcess.append(childPtr);
    }

}

void DeviceUpdateManager::completeStorageBatchOperation()
{
    // Update parent numbers for the entire storage hierarchy
    if (m_currentStorageDevice) {
        try {
            m_currentStorageDevice->updateParentsNumbers();
        } catch (const std::exception& e) {
            qWarning() << "WARNING: Error updating parent numbers:" << e.what();
        }
    }

    // Mark operation as completed
    m_operationRunning = false;
    m_waitingForCatalogCompletion = false;

    // Build final results for the storage batch
    QList<qint64> results = buildStorageBatchResults(m_currentStorageDevice);

    emit operationCompleted(results);
    emit operationRunningChanged();

    // Cleanup
    QTimer::singleShot(10, this, [this]() {
        cleanupOperation();
    });
}

void DeviceUpdateManager::processNextStorageChild()
{
    // Check stop conditions first
    if (!shouldContinue()) {
        handleOperationCancellation();
        return;
    }

    // Check if all children processed
    if (m_currentChildIndex >= m_childrenToProcess.size()) {
        completeStorageBatchOperation();
        return;
    }

    // Get next child to process
    Device* nextChild = m_childrenToProcess[m_currentChildIndex];

    // Move to next child for next iteration
    m_currentChildIndex++;

    // Check for gentle stop before processing any child
    if (m_gentleStopRequested.loadAcquire()) {
        completeStorageBatchOperation();
        return;
    }

    // Skip inactive catalogs but count them
    if (nextChild->type == "Catalog" && !nextChild->active) {

        // Count this as a skipped catalog
        m_skippedCatalogs++;
        m_processedDevices++; // Still count as processed device
        updateProgress();

        emit deviceProcessingCompleted(nextChild->name);

        processNextStorageChild();
        return;
    }

    // Check for gentle stop before starting new catalog (more specific check)
    if (m_gentleStopRequested.loadAcquire() && nextChild->type == "Catalog") {
        completeStorageBatchOperation();
        return;
    }


    // Set batch context for catalogs
    if (nextChild->type == "Catalog") {
        // m_currentChildIndex has already been incremented, so it's the current position
        int currentIndex = m_currentChildIndex;
        int totalCatalogs = 0;

        // Count total catalogs in storage children
        for (Device* child : m_childrenToProcess) {
            if (child->type == "Catalog") totalCatalogs++;
        }

        updateCatalogProgressContext(currentIndex, totalCatalogs);
    }

    // Process this child (will be either active catalog or other device type)
    updateDeviceRecursive(nextChild);

    // If we started an async catalog operation, we'll continue in onCatalogOperationCompleted
    // If it's not a catalog or completes synchronously, continue immediately
    if (!m_waitingForCatalogCompletion) {
        processNextStorageChild(); // Continue to next child
    }
}

void DeviceUpdateManager::accumulateStorageResults(Device* catalogDevice)
{

    if (catalogDevice->active) {
        m_updatedCatalogs++;
        m_totalCatalogFiles += catalogDevice->totalFileCount;
        m_totalCatalogSize += catalogDevice->totalFileSize;

        // Accumulate deltas from catalog job
        if (m_currentCatalogJob) {
            QList<qint64> catalogResults = m_currentCatalogJob->getResults();
            if (catalogResults.size() >= 5) {
                qint64 deltaFiles = catalogResults[2];
                qint64 deltaSize = catalogResults[4];
                m_totalDeltaFiles += deltaFiles;
                m_totalDeltaSize += deltaSize;
            }
        } else {
            qWarning() << "WARNING: No catalog job to get deltas from";
        }

    } else {
        qWarning() << "WARNING: accumulateStorageResults called for inactive catalog - this should not happen";
    }
}

void DeviceUpdateManager::initializeVirtualProcessing(Device* virtualDevice)
{

    m_currentVirtualDevice = virtualDevice;
    m_currentVirtualChildIndex = 0;

    // Build list of children to process (create copies for processing)
    m_virtualChildrenToProcess.clear();
    for (const Device& childDevice : virtualDevice->subDevices) {
        Device* childPtr = new Device(childDevice);
        childPtr->updateActiveState(m_connectionName);
        m_virtualChildrenToProcess.append(childPtr);
    }

}

void DeviceUpdateManager::completeVirtualProcessing()
{

    // Update parent numbers for the entire Virtual hierarchy
    if (m_currentVirtualDevice) {
        try {
            m_currentVirtualDevice->updateParentsNumbers();
        } catch (const std::exception& e) {
            qWarning() << "WARNING: Error updating parent numbers:" << e.what();
        }
    }

    // Mark operation as completed
    m_operationRunning = false;
    m_waitingForCatalogCompletion = false;

    // Build results for Virtual device (use existing logic)
    QList<qint64> results;
    // Build results directly instead of calling buildVirtualResults()
    results << 1;  // Success flag
    results << m_totalCatalogFiles;   // Total files from all catalogs
    results << 0;  // Delta files
    results << m_totalCatalogSize;    // Total size from all catalogs
    results << 0;  // Delta size
    results << m_updatedCatalogs;     // Updated catalogs
    results << m_skippedCatalogs;     // Skipped catalogs

    // Include accumulated storage data if available
    if (m_virtualStorageWasUpdated) {
        results << 1;  // Storage updated flag
        results << m_virtualStorageUpdateResult.newUsedSpace;
        results << m_virtualStorageUpdateResult.deltaUsedSpace;
        results << m_virtualStorageUpdateResult.newFreeSpace;
        results << m_virtualStorageUpdateResult.deltaFreeSpace;
        results << m_virtualStorageUpdateResult.newTotalSpace;
        results << m_virtualStorageUpdateResult.deltaTotalSpace;
    } else {
        results << 0;  // No storage updated
        for (int i = 8; i < 14; ++i) results << 0;
    }

    emit operationCompleted(results);
    emit operationRunningChanged();

    // Cleanup
    QTimer::singleShot(10, this, [this]() {
        cleanupOperation();
    });
}

void DeviceUpdateManager::processNextVirtualChild()
{

    // DEBUG: Show current counter values

    // If no more children, complete Virtual device
    if (m_remainingVirtualChildren.isEmpty()) {

        Device* virtualDevice = m_virtualDeviceBeingProcessed;
        m_virtualDeviceBeingProcessed = nullptr;

        updateParentNumbers(virtualDevice);
        emit deviceProcessingCompleted(virtualDevice->name);
        continueToNextDevice();
        return;
    }

    // Get and process next child
    Device* nextChild = m_remainingVirtualChildren.takeFirst();

    setCurrentDevice(nextChild);
    emit deviceProcessingStarted(nextChild->name, nextChild->type);

    nextChild->updateActiveState(m_connectionName);
    nextChild->dateTimeUpdated = QDateTime::currentDateTime();

    if (nextChild->type == "Storage") {

        updateStorageDevice(nextChild);

        // DEBUG: Show before/after storage counting
        m_processedStorageDevices++;

        // For now, just continue to next child - don't process catalogs yet
        processNextVirtualChild();

    } else if (nextChild->type == "Catalog") {

        if (nextChild->active) {
            // Calculate current catalog index in the batch
            int processedCatalogs = m_updatedCatalogs + m_skippedCatalogs;
            int totalCatalogs = 0;

            // Count total catalogs in this virtual device
            for (const Device* child : m_remainingVirtualChildren) {
                if (child->type == "Catalog") totalCatalogs++;
            }
            totalCatalogs += processedCatalogs; // Add already processed

            // Count current catalog
            totalCatalogs += 1;
            int currentIndex = processedCatalogs + 1;

            updateCatalogProgressContext(currentIndex, totalCatalogs);

            updateCatalogDevice(nextChild);
            if (!m_waitingForCatalogCompletion) {
                processNextVirtualChild();
            } else {
            }
        } else {
            m_skippedCatalogs++;
            processNextVirtualChild();
        }

    } else {
        updateVirtualDevice(nextChild);
        processNextVirtualChild();
    }
}

void DeviceUpdateManager::updateCatalogProgressContext(int currentCatalogIndex, int totalCatalogs)
{
    if (m_catalogProgressManager) {
        m_catalogProgressManager->setBatchContext(currentCatalogIndex, totalCatalogs);
    }
}
