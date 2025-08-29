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

            // Refresh physical storage space information and capture results
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

                // Mark that storage was successfully updated
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
    qDebug() << "Root device type:" << (rootDevice ? rootDevice->type : "NULL");
    qDebug() << "Update type:" << updateType;

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
        qDebug() << "=== STORAGE ROOT DEVICE - Initializing batch tracking ===";
        initializeStorageBatch();
    }

    // Analyze hierarchy to get totals for progress
    analyzeHierarchy(rootDevice);

    setStatus(QString("Starting %1 operation...").arg(updateType));
    emit operationStarted();
    emit operationRunningChanged();

    // Start recursive update
    updateDeviceRecursive(rootDevice);
}



Device* DeviceUpdateManager::createDummyDeviceFromList(const QList<Device*>& filteredDevices)
{
    qDebug() << "Creating temporary virtual device for" << filteredDevices.size() << "filtered devices";

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

    qDebug() << "Temporary filtered device created with" << tempDevice->subDevices.size() << "children";
    m_dummyDevice = tempDevice;

    return tempDevice;
}

void DeviceUpdateManager::processNextDevice()
{
    qDebug() << "=== DeviceUpdateManager::processNextDevice ===";

    if (!shouldContinue()) {
        handleOperationCancellation();
        return;
    }

    // For Storage batch operations, check if there are more children to process
    if (m_rootDevice && m_rootDevice->type == "Storage") {
        qDebug() << "Storage batch operation - checking for remaining children";

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

        qDebug() << "Expected catalogs:" << expectedCatalogs << "Processed:" << m_processedCatalogs;

        if (m_processedCatalogs < expectedCatalogs) {
            qDebug() << "Still processing catalogs - waiting for next completion";
            return; // More catalogs to process - wait for next onCatalogOperationCompleted
        } else {
            qDebug() << "All catalogs processed for storage - completing batch operation";
            completeOperation();
            return;
        }
    }

    // For other device types, complete immediately
    qDebug() << "Non-storage operation - completing";
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
        qDebug() << "No children found for:" << device->name;
        updateParentNumbers(device);
        emit deviceProcessingCompleted(device->name);
        continueToNextDevice();
        return;
    }

    qDebug() << "Found" << device->subDevices.size() << "children for:" << device->name;

    // Storage devices with children use batch processing
    if (device->type == "Storage") {
        if (m_processingContext == VirtualChild) {
            qDebug() << "=== STORAGE DEVICE (Virtual Child) - Using simple recursive processing ===";
            // Don't use batch processing - just process children recursively
            for (const Device& childDevice : device->subDevices) {
                if (!shouldContinue()) {
                    handleOperationCancellation();
                    return;
                }

                Device* childPtr = new Device(childDevice);
                updateDeviceRecursive(childPtr);

                if (m_waitingForCatalogCompletion) {
                    qDebug() << "Async catalog operation in progress for Storage child";
                    return;
                }
            }

            // All children processed
            updateParentNumbers(device);
            emit deviceProcessingCompleted(device->name);
            continueToNextDevice();
            return;
        } else {
            qDebug() << "=== STORAGE DEVICE (Root) - Using batch processing ===";
            initializeStorageBatchProcessing(device);
            processNextStorageChild();
            return;
        }
    }

    // For Virtual devices, process children directly without recursive flow control
    if (device->type == "Virtual") {
        qDebug() << "=== VIRTUAL DEVICE - Starting continuation-based processing ===";

        // DEBUG: Show initial counter values
        qDebug() << "INITIAL COUNTER VALUES:";
        qDebug() << "  m_processedStorageDevices:" << m_processedStorageDevices;
        qDebug() << "  m_updatedCatalogs:" << m_updatedCatalogs;
        qDebug() << "  m_skippedCatalogs:" << m_skippedCatalogs;
        qDebug() << "  m_totalCatalogFiles:" << m_totalCatalogFiles;
        qDebug() << "  m_totalCatalogSize:" << m_totalCatalogSize;

        m_virtualDeviceBeingProcessed = device;
        m_remainingVirtualChildren.clear();

        // RESET counters to avoid double counting from hierarchy analysis
        m_processedStorageDevices = 0;
        m_updatedCatalogs = 0;
        m_skippedCatalogs = 0;
        m_totalCatalogFiles = 0;
        m_totalCatalogSize = 0;

        qDebug() << "RESET COUNTERS TO ZERO";

        // Add all children to remaining list
        for (const Device& childDevice : device->subDevices) {
            Device* childPtr = new Device(childDevice);
            childPtr->updateActiveState("defaultConnection");
            m_remainingVirtualChildren.append(childPtr);
            qDebug() << "Added to remaining list:" << childPtr->name << "Type:" << childPtr->type << "Active:" << childPtr->active;
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
        // Do NOT call processChildren here

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
    qDebug() << "Update type:" << m_updateType;
    setStatus(QString("Updating catalog device: %1").arg(device->name));

    if (!device->active) {
        qDebug() << "Catalog is inactive, skipping:" << device->name;

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
    qDebug() << "CatalogManager state check:";
    qDebug() << "  catalogOperationRunning():" << m_catalogManager->catalogOperationRunning();

    if (m_catalogManager->catalogOperationRunning()) {
        qDebug() << "*** ERROR: CatalogManager reports operation still running!";
        qDebug() << "*** FORCING CatalogManager stop to clean state ***";

        // Force stop the running operation
        m_catalogManager->stopCatalogOperation();

        // Give it a moment to clean up
        QTimer::singleShot(100, this, [this, device]() {
            qDebug() << "*** RETRYING catalog operation after forced cleanup ***";
            updateCatalogDevice(device);  // Retry after cleanup
        });
        return;
    }

    // Start catalog operation
    qDebug() << "Starting catalog operation for:" << device->name << "Type:" << m_updateType;
    m_waitingForCatalogCompletion = true;

    startCatalogOperation(device);
}

void DeviceUpdateManager::startCatalogOperation(Device* device)
{
    qDebug() << "=== DeviceUpdateManager::startCatalogOperation ===";
    qDebug() << "Starting catalog operation for:" << device->name;

    // CRITICAL: Set waiting flag and current device
    m_waitingForCatalogCompletion = true;
    m_currentDevice = device;

    // Pass device values for catalog operations (same as Device::updateDevice logic)
    device->catalog->name = device->name;
    device->catalog->sourcePath = device->path;

    // Clean up any existing catalog job
    cleanupCatalogJob();
    m_currentCatalogJob = new CatalogJobStoppable(this);

    // Forward catalog progress properly
    connect(m_currentCatalogJob, &CatalogJobStoppable::catalogProgress,
            this, [this](qint64 filesProcessed, qint64 totalFiles, const QString& currentPath) {
                emit catalogProgress(filesProcessed, totalFiles, currentPath);
            });

    qDebug() << "Starting catalog operation for:" << device->name;

    // Start the actual catalog operation
    m_catalogManager->startCatalogJobStoppable(
        m_currentCatalogJob,
        device,
        CatalogJobStoppable::UpdateCatalog,
        m_databaseMode,
        m_collectionFolder
        );

    qDebug() << "Catalog operation started, waiting for completion...";
}

void DeviceUpdateManager::setCatalogProgressManager(CatalogProgressManager* catalogProgressManager)
{
    if (catalogProgressManager && m_catalogManager) {
        // Connect CatalogProgressManager to our internal CatalogManager
        // This ensures the working progress chain is preserved:
        // CatalogJobStoppable → CatalogManager → CatalogProgressManager → StatusBar
        catalogProgressManager->connectToCatalogManager(m_catalogManager);
        qDebug() << "CatalogProgressManager connected to DeviceUpdateManager's CatalogManager";

        // IMPORTANT: The catalog progress signals from DeviceUpdateManager should be
        // transparent - they exist for consistency but CatalogProgressManager handles the UI
    }
}

void DeviceUpdateManager::updateStorageDevice(Device* device)
{
    qDebug() << "=== DeviceUpdateManager::updateStorageDevice ===";
    qDebug() << "Updating Storage Device:" << device->name;
    setStatus(QString("Updating storage device: %1").arg(device->name));

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
                qDebug() << "Storage space updated - Free:" << device->freeSpace
                         << "Total:" << device->totalSpace;

                // Store results for reporting
                m_storageUpdateResult = result;
                m_storageWasUpdated = true;

                // Accumulate Virtual storage updates
                if (m_rootDevice && m_rootDevice->type == "Virtual") {
                    if (!m_virtualStorageWasUpdated) {
                        // First storage device - initialize
                        m_virtualStorageUpdateResult = result;
                        m_virtualStorageWasUpdated = true;
                        qDebug() << "Virtual storage tracking - First storage device initialized";
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

                        qDebug() << "Virtual storage tracking - Accumulated storage device results";
                    }

                    m_processedStorageDevices++;
                    qDebug() << "Virtual storage summary - Processed devices:" << m_processedStorageDevices;
                    qDebug() << "  Total free space:" << m_virtualStorageUpdateResult.newFreeSpace;
                    qDebug() << "  Total total space:" << m_virtualStorageUpdateResult.newTotalSpace;
                    qDebug() << "  Delta free space:" << m_virtualStorageUpdateResult.deltaFreeSpace;
                }

                // Update parent device values
                device->dateTimeUpdated = QDateTime::currentDateTime();
                device->saveDevice();
                device->saveStatistics(device->dateTimeUpdated, "update");

                qDebug() << "Storage device updated and saved successfully";
            } else {
                qDebug() << "Storage space not updated - Error:" << result.errorMessage;
                m_storageWasUpdated = false;
            }
        } catch (const std::exception& e) {
            qDebug() << "Error updating storage info:" << e.what();
            m_storageWasUpdated = false;
        }
    }

    m_processedDevices++;
    updateProgress();

    emit deviceProcessingCompleted(device->name);
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
    qDebug() << "=== DeviceUpdateManager::buildCatalogUpdateResults ===";

    QList<qint64> results;

    // Get the actual catalog deltas from CatalogJobStoppable
    qint64 deltaFileCount = 0;
    qint64 deltaTotalFileSize = 0;

    if (m_currentCatalogJob) {
        QList<qint64> catalogResults = m_currentCatalogJob->getResults();
        if (catalogResults.size() >= 5) {
            deltaFileCount = catalogResults[2];      // Actual delta files
            deltaTotalFileSize = catalogResults[4];  // Actual delta size
            qDebug() << "Using actual catalog deltas - Files:" << deltaFileCount << "Size:" << deltaTotalFileSize;
        } else {
            qDebug() << "WARNING: Invalid catalog results size:" << catalogResults.size();
        }
    } else {
        qDebug() << "WARNING: No current catalog job - deltas will be 0";
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

    qDebug() << "Built results for catalog:" << catalogDevice->name;
    qDebug() << "  Files:" << results[1] << "Delta files:" << results[2];
    qDebug() << "  Size:" << results[3] << "Delta size:" << results[4];
    qDebug() << "  Storage updated:" << results[7];

    return results;
}

void DeviceUpdateManager::loadDeviceChildren(Device* device)
{
    if (!device) return;

    qDebug() << "=== DeviceUpdateManager::loadDeviceChildren ===";
    qDebug() << "Loading children for device:" << device->name;

    try {
        // Clear any existing children data first
        device->subDevices.clear();
        device->deviceIDList.clear();

        // Force reload the device which populates deviceIDList
        device->loadDevice("defaultConnection");

        qDebug() << "Device has" << device->deviceIDList.size() << "child IDs";
        qDebug() << "hasSubDevice flag:" << device->hasSubDevice;

        for (int childID : std::as_const(device->deviceIDList)) {
            Device childDevice;
            childDevice.ID = childID;
            childDevice.loadDevice("defaultConnection");
            device->subDevices.append(childDevice);

            qDebug() << "  Loaded child:" << childDevice.name << "Type:" << childDevice.type << "Active:" << childDevice.active;
        }

        qDebug() << "Loaded" << device->subDevices.size() << "children for device:" << device->name;

    } catch (const std::exception& e) {
        qDebug() << "Error loading children for device" << device->name << ":" << e.what();
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

    // FIXED: Use proper method to load children that converts deviceIDList to subDevices
    if (device->subDevices.isEmpty() && device->hasSubDevice) {
        qDebug() << "Device" << device->name << "has children but subDevices is empty - loading children";
        loadDeviceChildren(device);  // This is the key fix!
    } else if (device->subDevices.isEmpty()) {
        // Force load to check if there are actually children
        qDebug() << "Device" << device->name << "checking for children";
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
        m_currentCatalogJob->deleteLater();
        m_currentCatalogJob = nullptr;
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

    setStatus("Ready");
    m_progress = 0;
    emit progressChanged();

    emit operationRunningChanged();

    qDebug() << "DeviceUpdateManager cleanup completed - ALL state reset";
}

void DeviceUpdateManager::cleanupDummyDevice()
{
    if (m_dummyDevice) {
        qDebug() << "Cleaning up temporary device:" << m_dummyDevice->name;
        delete m_dummyDevice;
        m_dummyDevice = nullptr;
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

    // Don't delegate to CatalogManager for storage batch operations
    // because CatalogManager is not in batch mode and will do hard stop

    bool isStorageBatchOperation = (m_currentStorageDevice != nullptr &&
                                    m_rootDevice && m_rootDevice->type == "Storage");

    if (isStorageBatchOperation) {
        qDebug() << "Storage batch operation - handling gentle stop locally";
        qDebug() << "Current catalog will complete, then operation will stop";
        // Don't call CatalogManager - let current catalog complete naturally
        // The gentle stop will be handled in onCatalogOperationCompleted()
    } else {
        // For non-storage operations, delegate to CatalogManager as before
        if (m_catalogManager && m_catalogManager->catalogOperationRunning()) {
            qDebug() << "Non-storage operation - delegating gentle stop to CatalogManager";
            m_catalogManager->requestGentleStop();
        }
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

    // ENHANCED: For Storage batch operations, update counts properly
    if (m_currentStorageDevice) {
        qDebug() << "Cancelling Storage batch operation:";
        qDebug() << "  Updated catalogs so far:" << m_updatedCatalogs;
        qDebug() << "  Skipped catalogs so far:" << m_skippedCatalogs;

        // Calculate remaining catalogs that won't be processed
        int remainingChildren = m_childrenToProcess.size() - m_currentChildIndex;
        if (remainingChildren > 0) {
            qDebug() << "  Remaining children that will be skipped:" << remainingChildren;
            // We could add these to skipped count if needed for reporting
        }
    }

    m_operationRunning = false;
    m_waitingForCatalogCompletion = false;

    // Calculate skipped counts for logging only
    int skippedDevices = m_totalDevices - m_processedDevices;
    int skippedCatalogs = m_totalCatalogs - m_processedCatalogs;

    setStatus(QString("Operation cancelled - %1 devices skipped, %2 catalogs skipped")
                  .arg(skippedDevices).arg(skippedCatalogs));

    // Emit cancellation (no results - MainWindow handles this differently)
    emit operationCancelled();
    emit operationRunningChanged();

    cleanupOperation();
}

void DeviceUpdateManager::completeOperation()
{
    qDebug() << "=== DeviceUpdateManager::completeOperation ===";

    // Prevent duplicate completion
    if (!m_operationRunning) {
        qDebug() << "*** SKIPPING completeOperation - operation already completed ***";
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

    // Determine result format based on root device type and operation
    QList<qint64> results;

    if (m_rootDevice && m_rootDevice->type == "Storage") {
        qDebug() << "*** STORAGE BATCH COMPLETION - Building storage batch results ***";
        results = buildStorageBatchResults(m_rootDevice);

    } else if (m_rootDevice && m_rootDevice->type == "Virtual") {
        qDebug() << "*** VIRTUAL DEVICE COMPLETION - Building virtual device results ***";
        qDebug() << "Virtual stats - Updated catalogs:" << m_updatedCatalogs << "Skipped:" << m_skippedCatalogs;
        qDebug() << "Virtual stats - Total files:" << m_totalCatalogFiles << "Total size:" << m_totalCatalogSize;
        qDebug() << "Virtual stats - Storage updated:" << m_virtualStorageWasUpdated << "Storage devices:" << m_processedStorageDevices;

        // FIXED: Build results array systematically
        results.clear();  // Ensure clean start

        results << 1;                     // Index 0: Success flag
        results << m_totalCatalogFiles;   // Index 1: Total files from all catalogs
        results << 0;                     // Index 2: Delta files
        results << m_totalCatalogSize;    // Index 3: Total size from all catalogs
        results << 0;                     // Index 4: Delta size
        results << m_updatedCatalogs;     // Index 5: Updated catalogs
        results << m_skippedCatalogs;     // Index 6: Skipped catalogs

        // Always add storage section (indices 7-13)
        if (m_virtualStorageWasUpdated && m_processedStorageDevices > 0) {
            qDebug() << "Adding storage data to results";
            results << 1;  // Index 7: Storage updated flag
            results << m_virtualStorageUpdateResult.newUsedSpace;     // Index 8
            results << m_virtualStorageUpdateResult.deltaUsedSpace;   // Index 9
            results << m_virtualStorageUpdateResult.newFreeSpace;     // Index 10
            results << m_virtualStorageUpdateResult.deltaFreeSpace;   // Index 11
            results << m_virtualStorageUpdateResult.newTotalSpace;    // Index 12
            results << m_virtualStorageUpdateResult.deltaTotalSpace;  // Index 13
        } else {
            qDebug() << "Adding empty storage data to results";
            results << 0;  // Index 7: No storage updated
            results << 0;  // Index 8: Used space
            results << 0;  // Index 9: Delta used space
            results << 0;  // Index 10: Free space
            results << 0;  // Index 11: Delta free space
            results << 0;  // Index 12: Total space
            results << 0;  // Index 13: Delta total space
        }

        qDebug() << "Built Virtual results array with" << results.size() << "elements";
        for (int i = 0; i < results.size(); ++i) {
            qDebug() << "  results[" << i << "] = " << results[i];
        }

    } else {
        qDebug() << "*** FALLBACK COMPLETION - Using generic format ***";

        results << 1;                    // Success flag
        results << m_processedCatalogs;  // Use catalog count
        results << 0;                    // Delta
        results << m_processedDevices;   // Use device count
        results << 0;                    // Delta
        for (int i = 5; i < 14; ++i) results << 0;  // Pad with zeros to index 13
    }

    qDebug() << "*** EMITTING operationCompleted ***";
    qDebug() << "Final results array size:" << results.size();
    qDebug() << "Results[0] (success):" << (results.size() > 0 ? results[0] : -999);
    qDebug() << "Results[5] (updated catalogs):" << (results.size() > 5 ? results[5] : -999);
    qDebug() << "Results[6] (skipped catalogs):" << (results.size() > 6 ? results[6] : -999);
    qDebug() << "Results[7] (storage updated):" << (results.size() > 7 ? results[7] : -999);

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
    qDebug() << "=== DIAGNOSTIC: DeviceUpdateManager::onCatalogOperationCompleted() ENTRY ===";
    qDebug() << "m_currentDevice:" << (m_currentDevice ? m_currentDevice->name : "NULL");
    qDebug() << "m_rootDevice:" << (m_rootDevice ? m_rootDevice->name : "NULL");
    qDebug() << "m_rootDevice type:" << (m_rootDevice ? m_rootDevice->type : "NULL");
    qDebug() << "m_updateType:" << m_updateType;
    qDebug() << "m_activeVirtualDevice:" << (m_currentVirtualDevice ? m_currentVirtualDevice->name : "NULL");
   // qDebug() << "m_pendingVirtualChildren.size():" << m_pendingVirtualChildren.size();

    if (!m_currentDevice) {
        qDebug() << "ERROR: No current device in catalog completion";
        return;
    }

    qDebug() << "Catalog processing completed for:" << m_currentDevice->name;

    // Update catalog device and save changes
    m_processedDevices++;
    m_processedCatalogs++;
    updateProgress();

    m_currentDevice->loadDevice("defaultConnection");
    m_currentDevice->saveDevice();
    m_currentDevice->saveStatistics(m_currentDevice->dateTimeUpdated, "update");
    updateRelatedDevices(m_currentDevice);

    emit deviceProcessingCompleted(m_currentDevice->name);

    m_waitingForCatalogCompletion = false;

    // Update parent numbers for this catalog
    m_currentDevice->updateParentsNumbers();

    // Accumulate results
    m_totalCatalogFiles += m_currentDevice->totalFileCount;
    m_totalCatalogSize += m_currentDevice->totalFileSize;

    if (m_virtualDeviceBeingProcessed != nullptr && m_currentDevice) {
        qDebug() << "=== VIRTUAL MODE - Counting catalog completion ===";

        // DEBUG: Show values before incrementing
        qDebug() << "BEFORE increment - m_updatedCatalogs:" << m_updatedCatalogs;
        qDebug() << "BEFORE accumulate - m_totalCatalogFiles:" << m_totalCatalogFiles << "m_totalCatalogSize:" << m_totalCatalogSize;
        qDebug() << "Catalog files:" << m_currentDevice->totalFileCount << "size:" << m_currentDevice->totalFileSize;

        // Count successful catalog update
        m_updatedCatalogs++;

        // Accumulate results
        m_totalCatalogFiles += m_currentDevice->totalFileCount;
        m_totalCatalogSize += m_currentDevice->totalFileSize;

        // DEBUG: Show values after incrementing
        qDebug() << "AFTER increment - m_updatedCatalogs:" << m_updatedCatalogs;
        qDebug() << "AFTER accumulate - m_totalCatalogFiles:" << m_totalCatalogFiles << "m_totalCatalogSize:" << m_totalCatalogSize;

        // Continue with next Virtual child
        QTimer::singleShot(50, this, [this]() {
            processNextVirtualChild();
        });
        return;
    } else {
        qDebug() << "=== NOT IN VIRTUAL MODE - Using standard completion ===";
    }

    // if (m_rootDevice && m_rootDevice->type == "Virtual") {
    //     qDebug() << "=== Virtual operation - NOT completing, letting operation continue ===";
    //     return;  // Just return - don't complete the operation
    // }

    // Determine if this is part of a Storage batch operation
    // Creation operations should NEVER be treated as batch operations
    bool isStorageBatchOperation = (m_currentStorageDevice != nullptr && m_updateType == "update" && m_rootDevice && m_rootDevice->type == "Storage");

    if (isStorageBatchOperation) {
        qDebug() << "=== STORAGE BATCH OPERATION - Catalog completed ===";

        // Accumulate results for this catalog
        accumulateStorageResults(m_currentDevice);

        // Update storage space after catalog completion (like the original Device::updateDevice logic)
        if (m_currentDevice->parentID != 0) {
            Storage::UpdateResult storageResult = updateParentStorage(m_currentDevice);
            if (storageResult.wasUpdated && !m_storageWasUpdated) {
                // Store the first successful storage update for reporting
                m_storageUpdateResult = storageResult;
                m_storageWasUpdated = true;
                qDebug() << "Storage space updated during batch operation";
            }
        }

        // Check for gentle stop request before starting next catalog
        if (m_gentleStopRequested.loadAcquire()) {
            qDebug() << "Gentle stop requested - completing operation instead of processing next child";
            completeStorageBatchOperation();
            return;
        }

        // Defer the next catalog start to allow CatalogManager cleanup
        qDebug() << "Deferring next child processing to allow CatalogManager cleanup...";
        QTimer::singleShot(50, this, [this]() {
            // Check gentle stop again in the timer callback
            if (m_gentleStopRequested.loadAcquire()) {
                qDebug() << "Gentle stop requested during timer - completing operation";
                completeStorageBatchOperation();
                return;
            }

            qDebug() << "Timer triggered - continuing to next child in storage batch...";
            processNextStorageChild();
        });

        return; // Don't emit operationCompleted yet - wait for all catalogs

    } else {
        qDebug() << "=== SINGLE CATALOG OPERATION - Emitting results immediately ===";

        // Single catalog operation - update parent storage and emit results
        Storage::UpdateResult storageResult = updateParentStorage(m_currentDevice);
        QList<qint64> results = buildCatalogUpdateResults(m_currentDevice, storageResult);

        qDebug() << "*** STORAGE UPDATE DEBUG ***";
        qDebug() << "Storage result wasUpdated:" << storageResult.wasUpdated;
        qDebug() << "Results[7] (storage updated flag):" << (results.size() > 7 ? results[7] : -1);

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

QList<qint64> DeviceUpdateManager::buildStorageBatchResults(Device* storageDevice)
{
    qDebug() << "=== DeviceUpdateManager::buildStorageBatchResults ===";
    qDebug() << "Building batch results for storage:" << storageDevice->name;
    qDebug() << "FINAL COUNTS - Updated:" << m_updatedCatalogs << "Skipped:" << m_skippedCatalogs;

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

    qDebug() << "Built storage batch results:";
    qDebug() << "  Updated catalogs:" << results[5] << "Skipped:" << results[6];
    qDebug() << "  Total files:" << results[1] << "Delta files:" << results[2];
    qDebug() << "  Total size:" << results[3] << "Delta size:" << results[4];
    qDebug() << "  Storage updated:" << results[7];

    return results;
}

void DeviceUpdateManager::initializeStorageBatch()
{
    qDebug() << "=== DeviceUpdateManager::initializeStorageBatch ===";
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
    qDebug() << "=== DeviceUpdateManager::initializeStorageBatchProcessing ===";
    qDebug() << "Storage device:" << storageDevice->name;

    m_currentStorageDevice = storageDevice;
    m_currentChildIndex = 0;

    // Build list of children to process (create copies for processing)
    m_childrenToProcess.clear();
    for (const Device& childDevice : storageDevice->subDevices) {
        Device* childPtr = new Device(childDevice);

        // Ensure active state is correctly updated
        childPtr->updateActiveState("defaultConnection");

        m_childrenToProcess.append(childPtr);
        qDebug() << "  Child to process:" << childPtr->name << "Type:" << childPtr->type << "Active:" << childPtr->active;
    }

    qDebug() << "Initialized batch processing for" << m_childrenToProcess.size() << "children";
}

void DeviceUpdateManager::completeStorageBatchOperation()
{
    qDebug() << "=== DeviceUpdateManager::completeStorageBatchOperation ===";
    qDebug() << "Final batch summary:";
    qDebug() << "  Updated catalogs:" << m_updatedCatalogs;
    qDebug() << "  Skipped catalogs:" << m_skippedCatalogs;
    qDebug() << "  Total catalog files:" << m_totalCatalogFiles;
    qDebug() << "  Total catalog size:" << m_totalCatalogSize;
    qDebug() << "  Storage was updated:" << m_storageWasUpdated;

    // Update parent numbers for the entire storage hierarchy
    if (m_currentStorageDevice) {
        try {
            m_currentStorageDevice->updateParentsNumbers();
            qDebug() << "Parent numbers updated for storage hierarchy";
        } catch (const std::exception& e) {
            qDebug() << "Error updating parent numbers:" << e.what();
        }
    }

    // Mark operation as completed
    m_operationRunning = false;
    m_waitingForCatalogCompletion = false;

    // Build final results for the storage batch
    QList<qint64> results = buildStorageBatchResults(m_currentStorageDevice);

    qDebug() << "*** EMITTING operationCompleted for Storage batch ***";
    qDebug() << "Results[0] (success):" << results[0];
    qDebug() << "Results[1] (total files):" << results[1];
    qDebug() << "Results[5] (updated catalogs):" << results[5];
    qDebug() << "Results[6] (skipped catalogs):" << results[6];
    qDebug() << "Results[7] (storage updated):" << results[7];

    // For Storage batch operations, report storage update and catalog summary
    m_updateType = "list";

    emit operationCompleted(results);
    emit operationRunningChanged();

    // Cleanup
    QTimer::singleShot(10, this, [this]() {
        cleanupOperation();
    });
}

void DeviceUpdateManager::processNextStorageChild()
{
    qDebug() << "=== DeviceUpdateManager::processNextStorageChild ===";
    qDebug() << "Current child index:" << m_currentChildIndex << "/ Total children:" << m_childrenToProcess.size();

    // Check stop conditions first
    if (!shouldContinue()) {
        qDebug() << "Stop requested during batch processing";
        handleOperationCancellation();
        return;
    }

    // Check if all children processed
    if (m_currentChildIndex >= m_childrenToProcess.size()) {
        qDebug() << "All children processed for storage - completing batch operation";
        completeStorageBatchOperation();
        return;
    }

    // Get next child to process
    Device* nextChild = m_childrenToProcess[m_currentChildIndex];
    qDebug() << "Processing next child:" << nextChild->name << "Type:" << nextChild->type << "Active:" << nextChild->active;

    // Move to next child for next iteration
    m_currentChildIndex++;

    // Check for gentle stop before processing any child
    if (m_gentleStopRequested.loadAcquire()) {
        qDebug() << "Gentle stop requested - completing batch operation";
        completeStorageBatchOperation();
        return;
    }

    // Skip inactive catalogs but count them
    if (nextChild->type == "Catalog" && !nextChild->active) {
        qDebug() << "Catalog is inactive, skipping:" << nextChild->name;
        qDebug() << "BEFORE: m_updatedCatalogs=" << m_updatedCatalogs << "m_skippedCatalogs=" << m_skippedCatalogs;

        // Count this as a skipped catalog
        m_skippedCatalogs++;
        m_processedDevices++; // Still count as processed device
        updateProgress();

        qDebug() << "AFTER: m_updatedCatalogs=" << m_updatedCatalogs << "m_skippedCatalogs=" << m_skippedCatalogs;
        emit deviceProcessingCompleted(nextChild->name);

        qDebug() << "Skipped inactive catalog, continuing to next child";
        processNextStorageChild();
        return;
    }

    // Check for gentle stop before starting new catalog (more specific check)
    if (m_gentleStopRequested.loadAcquire() && nextChild->type == "Catalog") {
        qDebug() << "Gentle stop requested - stopping before catalog:" << nextChild->name;
        completeStorageBatchOperation();
        return;
    }

    qDebug() << "Processing active child:" << nextChild->name;

    // Process this child (will be either active catalog or other device type)
    updateDeviceRecursive(nextChild);

    // If we started an async catalog operation, we'll continue in onCatalogOperationCompleted
    // If it's not a catalog or completes synchronously, continue immediately
    if (!m_waitingForCatalogCompletion) {
        qDebug() << "Child processed synchronously, continuing to next";
        processNextStorageChild(); // Continue to next child
    }
}

void DeviceUpdateManager::accumulateStorageResults(Device* catalogDevice)
{
    qDebug() << "=== DeviceUpdateManager::accumulateStorageResults ===";
    qDebug() << "Accumulating results for catalog:" << catalogDevice->name;

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
                qDebug() << "Added deltas - Files:" << deltaFiles << "Size:" << deltaSize;
                qDebug() << "Total deltas now - Files:" << m_totalDeltaFiles << "Size:" << m_totalDeltaSize;
            }
        } else {
            qDebug() << "WARNING: No catalog job to get deltas from";
        }

        qDebug() << "Catalog updated - Files:" << catalogDevice->totalFileCount << "Size:" << catalogDevice->totalFileSize;
        qDebug() << "Running totals - Updated:" << m_updatedCatalogs << "Skipped:" << m_skippedCatalogs;
    } else {
        qDebug() << "ERROR: accumulateStorageResults called for inactive catalog - this should not happen";
    }
}

void DeviceUpdateManager::initializeVirtualProcessing(Device* virtualDevice)
{
    qDebug() << "=== DeviceUpdateManager::initializeVirtualProcessing ===";
    qDebug() << "Virtual device:" << virtualDevice->name;

    m_currentVirtualDevice = virtualDevice;
    m_currentVirtualChildIndex = 0;

    // Build list of children to process (create copies for processing)
    m_virtualChildrenToProcess.clear();
    for (const Device& childDevice : virtualDevice->subDevices) {
        Device* childPtr = new Device(childDevice);
        childPtr->updateActiveState("defaultConnection");
        m_virtualChildrenToProcess.append(childPtr);
        qDebug() << "  Virtual child to process:" << childPtr->name << "Type:" << childPtr->type << "Active:" << childPtr->active;
    }

    qDebug() << "Initialized Virtual processing for" << m_virtualChildrenToProcess.size() << "children";
}

void DeviceUpdateManager::completeVirtualProcessing()
{
    qDebug() << "=== DeviceUpdateManager::completeVirtualProcessing ===";
    qDebug() << "Final Virtual summary:";
    qDebug() << "  Updated catalogs:" << m_updatedCatalogs;
    qDebug() << "  Skipped catalogs:" << m_skippedCatalogs;
    qDebug() << "  Processed storage devices:" << m_processedStorageDevices;

    // Update parent numbers for the entire Virtual hierarchy
    if (m_currentVirtualDevice) {
        try {
            m_currentVirtualDevice->updateParentsNumbers();
            qDebug() << "Parent numbers updated for Virtual hierarchy";
        } catch (const std::exception& e) {
            qDebug() << "Error updating parent numbers:" << e.what();
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

    qDebug() << "*** EMITTING operationCompleted for Virtual device ***";
    emit operationCompleted(results);
    emit operationRunningChanged();

    // Cleanup
    QTimer::singleShot(10, this, [this]() {
        cleanupOperation();
    });
}

void DeviceUpdateManager::processNextVirtualChild()
{
    qDebug() << "=== DeviceUpdateManager::processNextVirtualChild ===";
    qDebug() << "Remaining children:" << m_remainingVirtualChildren.size();

    // DEBUG: Show current counter values
    qDebug() << "CURRENT COUNTERS:";
    qDebug() << "  Storage devices:" << m_processedStorageDevices;
    qDebug() << "  Updated catalogs:" << m_updatedCatalogs;
    qDebug() << "  Skipped catalogs:" << m_skippedCatalogs;

    // If no more children, complete Virtual device
    if (m_remainingVirtualChildren.isEmpty()) {
        qDebug() << "=== ALL VIRTUAL CHILDREN PROCESSED ===";
        qDebug() << "FINAL COUNTS:";
        qDebug() << "  Storage devices processed:" << m_processedStorageDevices;
        qDebug() << "  Catalogs updated:" << m_updatedCatalogs;
        qDebug() << "  Catalogs skipped:" << m_skippedCatalogs;
        qDebug() << "  Total files:" << m_totalCatalogFiles;
        qDebug() << "  Total size:" << m_totalCatalogSize;

        Device* virtualDevice = m_virtualDeviceBeingProcessed;
        m_virtualDeviceBeingProcessed = nullptr;

        updateParentNumbers(virtualDevice);
        emit deviceProcessingCompleted(virtualDevice->name);
        continueToNextDevice();
        return;
    }

    // Get and process next child
    Device* nextChild = m_remainingVirtualChildren.takeFirst();
    qDebug() << "Processing next Virtual child:" << nextChild->name << "Type:" << nextChild->type << "Active:" << nextChild->active;

    setCurrentDevice(nextChild);
    emit deviceProcessingStarted(nextChild->name, nextChild->type);

    nextChild->updateActiveState("defaultConnection");
    nextChild->dateTimeUpdated = QDateTime::currentDateTime();

    if (nextChild->type == "Storage") {
        qDebug() << "=== PROCESSING STORAGE DEVICE:" << nextChild->name << "===";

        updateStorageDevice(nextChild);

        // DEBUG: Show before/after storage counting
        qDebug() << "BEFORE storage count increment:" << m_processedStorageDevices;
        m_processedStorageDevices++;
        qDebug() << "AFTER storage count increment:" << m_processedStorageDevices;

        // For now, just continue to next child - don't process catalogs yet
        processNextVirtualChild();

    } else if (nextChild->type == "Catalog") {
        qDebug() << "=== PROCESSING CATALOG DEVICE:" << nextChild->name << "Active:" << nextChild->active << "===";

        if (nextChild->active) {
            updateCatalogDevice(nextChild);
            if (!m_waitingForCatalogCompletion) {
                qDebug() << "Catalog completed synchronously - continuing";
                processNextVirtualChild();
            } else {
                qDebug() << "Catalog started async - waiting for completion";
            }
        } else {
            qDebug() << "BEFORE skip count increment:" << m_skippedCatalogs;
            m_skippedCatalogs++;
            qDebug() << "AFTER skip count increment:" << m_skippedCatalogs;
            processNextVirtualChild();
        }

    } else {
        qDebug() << "=== PROCESSING OTHER DEVICE TYPE:" << nextChild->type << "===";
        updateVirtualDevice(nextChild);
        processNextVirtualChild();
    }
}
