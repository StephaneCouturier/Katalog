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
// File Name:   catalogdifferenceengine.h
// Purpose:     Reusable engine for comparing two sets of catalogs
// Description: Finds files present in one set but not the other (set difference),
//              and files matching on some fields but differing on others (conflicts).
//              Used by Search > Differences and Backup features.
// Author:      Stephane Couturier
/////////////////////////////////////////////////////////////////////////////
*/
#ifndef CATALOGDIFFERENCEENGINE_H
#define CATALOGDIFFERENCEENGINE_H

#include <QString>
#include <QList>
#include <QFlags>

class Device;
class QSqlQuery;

struct DifferenceFileEntry {
    QString fileName;
    QString folderPath;
    qint64 fileSize = 0;
    QString dateUpdated;          // source file date
    QString targetDateUpdated;    // target file date (conflicts only; empty otherwise)
    QString catalog;
    int catalogId = 0;
    QString checksum;
};

struct DifferenceResult {
    QList<DifferenceFileEntry> onlyInSource;      // Files in source but not in target
    QList<DifferenceFileEntry> onlyInTarget;      // Files in target but not in source
    QList<DifferenceFileEntry> differentContent;  // Same match fields, different checksum
};

// Result of a path-aware strict comparison (used by the strictCopy backup mode)
struct StrictDifferenceResult {
    QList<DifferenceFileEntry> filesToCopy;  // Source files absent from the corresponding target path
    QList<DifferenceFileEntry> conflicts;    // Source files at matching target path but different size
    int skippedCount = 0;                    // Files already in sync
};

class CatalogDifferenceEngine
{
public:
    enum CompareField {
        Name     = 0x1,
        Size     = 0x2,
        Date     = 0x4,
        Checksum = 0x8
    };
    Q_DECLARE_FLAGS(CompareFields, CompareField)

    explicit CatalogDifferenceEngine(const QString &connectionName);

    /**
     * @brief Compare two sets of catalog devices and find differences.
     *
     * In standard mode (checksumNotEqual=false):
     *   Uses matchFields for the join condition. Returns files unique to each side.
     *   If Checksum is in matchFields, files must also have matching checksums to be "same".
     *
     * In checksum-not-equal mode (checksumNotEqual=true):
     *   Uses matchFields (without Checksum) for the join condition.
     *   Returns files matching on join fields but with different checksums (conflicts).
     *
     * @param sourceDeviceIds  Device IDs for the source side (catalog-type device tree IDs)
     * @param targetDeviceIds  Device IDs for the target side
     * @param matchFields      Fields that define "same file" for the join condition
     * @param checksumNotEqual If true, find files with matching fields but different checksums
     * @return DifferenceResult with categorized file entries
     */
    DifferenceResult compare(
        const QList<int> &sourceDeviceIds,
        const QList<int> &targetDeviceIds,
        CompareFields matchFields,
        bool checksumNotEqual = false,
        const QString &tableName = QLatin1String("filetemp")
    );

    /**
     * @brief Path-aware comparison for strictCopy backup mode.
     *
     * A file is "to copy" if no file with the same name exists at the corresponding
     * relative path in the target catalog. A file is a "conflict" if a file with the
     * same name exists at that path but with a different size.
     *
     * @param sourceCatalogId  externalID of the source catalog
     * @param targetCatalogId  externalID of the target catalog
     * @param sourceRoot       Root path of the source device (used to compute relative paths)
     * @param targetRoot       Root path of the target device
     * @return StrictDifferenceResult with filesToCopy, conflicts and skippedCount
     */
    StrictDifferenceResult compareStrict(
        int sourceCatalogId,
        int targetCatalogId,
        const QString &sourceRoot,
        const QString &targetRoot
    );

    /**
     * @brief Path-aware strict comparison using a live filesystem walk as source.
     *
     * Used in Drive source mode: every file physically present under sourceRoot
     * is compared against the target catalog. Excluded folders are not skipped —
     * the full tree is walked. The source device must be connected and mounted.
     *
     * @param sourceRoot       Filesystem path to walk recursively
     * @param targetCatalogId  externalID of the target catalog
     * @param targetRoot       Root path of the target device (to compute relative paths)
     * @return StrictDifferenceResult with filesToCopy, conflicts and skippedCount
     */
    StrictDifferenceResult compareStrictFromDrive(
        const QString &sourceRoot,
        int targetCatalogId,
        const QString &targetRoot
    );

    /**
     * @brief Resolve a device to its catalog device IDs.
     *
     * If the device is a Catalog, returns its own ID.
     * If the device is a Virtual/Storage, returns the IDs of its child Catalog devices.
     *
     * @param device           The device to resolve
     * @param connectionName   Database connection name
     * @return List of catalog-type device IDs
     */
    static QList<int> resolveCatalogDeviceIds(Device *device, const QString &connectionName);

private:
    QString m_connectionName;

    static QString buildJoinCondition(const QString &alias1, const QString &alias2, CompareFields fields);
    static QString deviceIdListToString(const QList<int> &ids);
    static DifferenceFileEntry entryFromQuery(const QSqlQuery &query);
};

Q_DECLARE_OPERATORS_FOR_FLAGS(CatalogDifferenceEngine::CompareFields)

#endif // CATALOGDIFFERENCEENGINE_H
