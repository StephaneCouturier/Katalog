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
// File Name:   collectionimporter.h
// Purpose:     Import / Merge / Update across Collections (#750)
// Description: device/catalog hierarchy, file lists, and extended data
// Author:      Stephane Couturier
/////////////////////////////////////////////////////////////////////////////
*/
#ifndef COLLECTIONIMPORTER_H
#define COLLECTIONIMPORTER_H

#include <QObject>
#include <QAtomicInt>
#include <QSqlError>
#include <QStandardItemModel>
#include "collection.h"

/**
 * @brief Imports devices and catalogs from a source collection into the active target collection.
 *
 * Phase 1 scope: device/catalog hierarchy + file/folder/catalog_filter rows.
 * Supports File (.db) and Memory (CSV folder) source modes.
 * Hosted source is not in scope.
 *
 * Usage:
 *   1. Construct with the target Collection.
 *   2. Call openSource() — populates the source device tree.
 *   3. Call importDevice() / importAllDevices() / updateDeviceFromExternalCollection() as needed.
 *   4. Call close() when done.
 */
class CollectionImporter : public QObject
{
    Q_OBJECT

public:
    explicit CollectionImporter(Collection *targetCollection, QObject *parent = nullptr);
    ~CollectionImporter();

    // --- Source connection ---

    /** Open the source collection (auto-detects File vs Memory from the path). */
    QSqlError openSource(const QString &sourcePath);

    /** Close and remove the source connection. */
    void close();

    /** Whether a source collection is currently open. */
    bool isSourceOpen() const;

    // --- Compatibility check ---

    /** Returns true if source and target share the same dbSchemaVersion. */
    bool checkSchemaCompatibility();

    // --- ID offsets ---

    /** Compute device_id and catalog_id offsets from current target maximums. */
    void buildIdOffsets();

    // --- Operations ---

    /**
     * Import one device from the source (including its full ancestor chain and all
     * Catalog-type data: catalog, file, folder, catalog_filter rows).
     * @param sourceDeviceId  device_id in the source connection.
     * @return true on success.
     */
    bool importDevice(int sourceDeviceId);

    /**
     * Merge: import every top-level device from the source.
     * @return number of devices imported, -1 on error.
     */
    int importAllDevices();

    /**
     * Update: re-import file/folder data for a previously imported device using the
     * stored device_mapping CollectionImport link.
     * @param targetDeviceId  device_id in the target collection.
     * @return true on success.
     */
    bool updateDeviceFromExternalCollection(int targetDeviceId);

    /**
     * Update all devices previously imported from a given source collection path.
     * Opens the source automatically if needed.
     * @param sourcePath  path to the source collection (same as used during import).
     * @return true if at least one device was updated.
     */
    bool updateAllImportsFromSource(const QString &sourcePath);

    /** Request cancellation of an in-progress operation. */
    void requestStop() { m_stopRequested.storeRelease(1); }

    bool wasStopRequested() const { return m_stopRequested.loadAcquire(); }

    // --- Source device model (read-only, for the UI tree view) ---

    /**
     * Build and return a read-only QStandardItemModel populated from the source
     * device table.  The caller takes ownership (or the model lives as long as
     * this importer object).
     */
    QStandardItemModel *buildSourceDeviceModel();

    // --- Accessors ---

    QString sourceConnectionName() const { return m_sourceConnectionName; }
    QString sourcePath() const           { return m_sourcePath; }
    QString sourceMode() const           { return m_sourceMode; }  // "File" or "Memory"
    QString lastError() const            { return m_lastError; }

    /**
     * Aggregate before/after totals for the catalogs touched by the most recent
     * updateAllImportsFromSource() call. Captured from device-level totals the
     * update already reads/overwrites, so there is no extra scan. Used by the UI
     * to show an update report (file-count and size changes).
     */
    struct ImportUpdateStats {
        int    catalogsUpdated = 0;
        qint64 filesBefore     = 0;
        qint64 filesAfter      = 0;
        qint64 sizeBefore      = 0;   // bytes
        qint64 sizeAfter       = 0;   // bytes
    };
    ImportUpdateStats lastUpdateStats() const { return m_lastUpdateStats; }

signals:
    void importProgress(int current, int total, const QString &itemName);
    // Emitted periodically during file-row insertion for a single catalog.
    // catalogIndex/totalCatalogs allow the UI to show "Catalog X of Y | Name".
    // done/total are row counts (qint64 because catalogs can have billions of entries).
    void fileImportProgress(int catalogIndex, int totalCatalogs,
                            const QString &catalogName,
                            qint64 done, qint64 total);

private:
    // Internal helpers
    int  remapAndInsertCatalog(int srcCatalogId);
    int  remapAndInsertDevice(int srcDeviceId, int newParentId);
    void insertFileData(int srcCatalogId, int newCatalogId, const QString &catalogName);
    void insertCatalogFilter(int srcCatalogId, int newCatalogId);
    void recordImportLink(int srcDeviceId, int newDeviceId);
    QString resolveNameConflict(const QString &name, const QStringList &existingNames) const;

    /** Collect the ancestor chain for srcDeviceId (nearest-first), stopping at root. */
    QList<int> getAncestorChain(int srcDeviceId) const;

    /** Ensure ancestor devices exist in the target; return the immediate parent id. */
    int ensureAncestors(int srcDeviceId);

    /** Fetch device_parent_id from the source for a given source device_id. */
    int srcParentId(int srcDeviceId) const;

    /** Fetch device_name from the source for a given source device_id. */
    QString srcDeviceName(int srcDeviceId) const;

    /** Fetch device_type from the source for a given source device_id. */
    QString srcDeviceType(int srcDeviceId) const;

    /** Find all top-level (root) device IDs in the source. */
    QList<int> sourceRootDeviceIds() const;

    /** Recursively import srcDeviceId and all its children into the target. */
    bool importSubTree(int srcDeviceId, int targetParentId, int &imported, int total);

    // Extended data
    void importStorageForCatalog(int srcCatalogId);
    void updateStorageRecord(int srcStorageId, int targetStorageId);
    void importStatisticsForDevice(int srcDeviceId, int newDeviceId);
    void importBackupMappings();
    void importTagsForAllCatalogs();    // tags are folder-level, no catalog dependency
    void importExcludeDirectories();    // parameter rows with type 'exclude_directory'
    QString sourceImageFolderPath() const;
    void copyStorageImage(const QString &picturePath);

    // Members
    Collection         *m_target;
    Collection         *m_sourceCollection = nullptr;  // Used for Memory mode loading
    QString             m_sourceConnectionName;
    QString             m_sourcePath;
    QString             m_sourceMode;   // "File" or "Memory"
    QString             m_lastError;

    ImportUpdateStats   m_lastUpdateStats;   // populated by updateAllImportsFromSource()

    int  m_deviceIdOffset  = 0;
    int  m_catalogIdOffset = 0;
    int  m_storageIdOffset = 0;

    // Per-operation catalog progress counters (reset in importDevice / importAllDevices)
    int  m_catalogImportIndex = 0;   // incremented each time insertFileData is called
    int  m_catalogImportTotal = 0;   // total Catalog-type devices in the source sub-tree

    // Maps srcDeviceId → newDeviceId, populated during an import operation
    QMap<int, int>  m_deviceIdMap;

    // srcCatalogId → newCatalogId, populated during import for Phase 2 tag/storage tracking
    QMap<int, int>  m_catalogIdMap;

    QAtomicInt m_stopRequested{0};
};

#endif // COLLECTIONIMPORTER_H
