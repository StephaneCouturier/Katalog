/*
 * DeviceUpdateManager - Unified device hierarchy update system
 * Replaces both CatalogManager and DeviceManager with single clean interface
 * Uses recursive approach like original Device::updateDevice but stoppable
 */

#ifndef DEVICEUPDATEMANAGER_H
#define DEVICEUPDATEMANAGER_H

#include "core/catalogmanager.h"
#include "core/catalogprogressmanager.h"
#include "device.h"
#include "catalogjobstoppable.h"
#include <QObject>
#include <QAtomicInt>
#include <QDateTime>
#include <QTimer>

class DeviceUpdateManager : public QObject
{
    Q_OBJECT
    Q_PROPERTY(bool operationRunning READ operationRunning NOTIFY operationRunningChanged)
    Q_PROPERTY(int progress READ progress NOTIFY progressChanged)
    Q_PROPERTY(QString status READ status NOTIFY statusChanged)
    Q_PROPERTY(QString currentDeviceName READ currentDeviceName NOTIFY currentDeviceNameChanged)

public:
    explicit DeviceUpdateManager(QObject *parent = nullptr);
    ~DeviceUpdateManager();

    // ===== SINGLE UNIFIED INTERFACE =====
    void updateDeviceHierarchy(Device* rootDevice,
                               const QString& databaseMode,
                               const QString& collectionFolder,
                               const QString& updateType = "update");
    QString updateType() const { return m_updateType; }
    // ===== TEMPORARY DEVICE CREATION =====
    Device* createTempVirtualDeviceForActiveCatalogs(const QList<Device*>& activeCatalogs);
    Device* createTempVirtualDeviceForFilter(const QList<Device*>& filteredDevices); 

    // ===== CONTROL INTERFACE =====
    void requestGentleStop();
    void requestHardStop();
    void pauseOperation();
    void resumeOperation();
    void stopOperation();

    // ===== STATE QUERIES =====
    bool operationRunning() const { return m_operationRunning; }
    bool isPaused() const { return m_isPaused; }
    int progress() const { return m_progress; }
    QString status() const { return m_status; }
    QString currentDeviceName() const;

    int processedDevices() const { return m_processedDevices; }
    int totalDevices() const { return m_totalDevices; }
    int processedCatalogs() const { return m_processedCatalogs; }
    int totalCatalogs() const { return m_totalCatalogs; }

    void setCatalogProgressManager(CatalogProgressManager* catalogProgressManager);
    Device* getCurrentDevice() const { return m_currentDevice; }

signals:
    // Main operation lifecycle
    void operationStarted();
    void operationCompleted(const QList<qint64>& results);
    void operationError(const QString& error);
    void operationCancelled();

    // Property change notifications
    void operationRunningChanged();
    void progressChanged();
    void statusChanged();
    void currentDeviceNameChanged();

    // Progress details
    void deviceProcessingStarted(const QString& deviceName, const QString& deviceType);
    void deviceProcessingCompleted(const QString& deviceName);
    void catalogProgress(qint64 filesProcessed, qint64 totalFiles, const QString& currentPath);

private slots:
    void onCatalogOperationCompleted();
    void onCatalogOperationError(const QString& error);
    void onCatalogOperationCancelled();
    void onCatalogProgress(qint64 filesProcessed, qint64 totalFiles, const QString& currentPath);

private:
    QString m_updateType;

    // ===== CORE RECURSIVE LOGIC =====
    void updateDeviceRecursive(Device* device);
    void updateVirtualDevice(Device* device);
    void updateStorageDevice(Device* device);
    void updateCatalogDevice(Device* device);

    // ===== HIERARCHY ANALYSIS =====
    void analyzeHierarchy(Device* rootDevice);
    void buildDeviceList(Device* device, QList<Device*>& deviceList);

    // ===== CHILDREN PROCESSING =====
    void processChildren(Device* device);
    void processNextDevice();

    // ===== STOP CONTROL =====
    bool shouldContinue() const;
    void checkStopRequested();

    // ===== BUSINESS LOGIC =====
    void updateParentNumbers(Device* device);
    void updateRelatedDevices(Device* device);
    void saveDeviceStatistics(Device* device);

    // ===== PROGRESS & STATUS =====
    void updateProgress();
    void setStatus(const QString& status);
    void setCurrentDevice(Device* device);

    // ===== COMPLETION & CLEANUP =====
    void completeOperation();
    void handleOperationError(const QString& error);
    void handleOperationCancellation();
    void cleanupOperation();
    void cleanupTempDevice();

    // ===== CATALOG INTEGRATION =====
    void setupCatalogManager();
    void cleanupCatalogJob();
    void continueToNextDevice();
    void processNextInQueue();

    void updateParentStorageAfterCatalogUpdate(Device* device);
    bool m_storageWasUpdated;
    Storage::UpdateResult m_storageUpdateResult;
    Storage::UpdateResult updateParentStorage(Device* catalogDevice);
    QList<qint64> buildCatalogUpdateResults(Device* catalogDevice, const Storage::UpdateResult& storageResult);

private:
    // ===== OPERATION STATE =====
    bool m_operationRunning = false;
    bool m_isPaused = false;

    // ===== STOP CONTROL =====
    QAtomicInt m_stopRequested{0};
    QAtomicInt m_gentleStopRequested{0};

    // ===== HIERARCHY STATE =====
    Device* m_rootDevice = nullptr;
    Device* m_currentDevice = nullptr;
    Device* m_tempDevice = nullptr;
    QList<Device*> m_allDevices;
    QList<Device*> m_catalogDevices;
    int m_currentDeviceIndex = 0;

    // ===== PROGRESS TRACKING =====
    int m_progress = 0;
    QString m_status;
    int m_processedDevices = 0;
    int m_totalDevices = 0;
    int m_processedCatalogs = 0;
    int m_totalCatalogs = 0;

    // ===== OPERATION PARAMETERS =====
    QString m_databaseMode;
    QString m_collectionFolder;

    // ===== CATALOG OPERATION INTEGRATION =====
    CatalogManager* m_catalogManager = nullptr;
    CatalogJobStoppable* m_currentCatalogJob = nullptr;
    bool m_waitingForCatalogCompletion = false;

    // ===== RESULTS ACCUMULATION =====
    QList<qint64> m_accumulatedResults;

    // ===== TIMING =====
    QDateTime m_operationStartTime;
};

#endif // DEVICEUPDATEMANAGER_H
