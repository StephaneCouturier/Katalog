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
// File Name:   catalogdifferenceengine.cpp
// Purpose:     Reusable engine for comparing two sets of catalogs
// Description: Implements catalog comparison using SQL JOIN queries on the
//              filetemp table. Extracted from searchjobstoppable.cpp.
// Author:      Stephane Couturier
/////////////////////////////////////////////////////////////////////////////
*/

#include "catalogdifferenceengine.h"
#include "device.h"
#include "backupmappingmanager.h"

#include <QSqlQuery>
#include <QSqlError>
#include <QSqlDatabase>
#include <QDebug>
#include <QDirIterator>
#include <QFileInfo>
#include <QCoreApplication>
#include <QSet>

namespace {
// Yield to the UI event loop periodically so a long comparison keeps the UI
// responsive on the main thread (BKP-C8), and report whether the user asked to
// stop. Called once per processed row; pumps events every ~512 rows.
inline bool yieldAndCheckStop(int &counter, bool *stopRequested)
{
    if ((++counter & 0x1FF) == 0)
        QCoreApplication::processEvents();
    return stopRequested && *stopRequested;
}
}

//----------------------------------------------------------------------
CatalogDifferenceEngine::CatalogDifferenceEngine(const QString &connectionName)
    : m_connectionName(connectionName)
{
}

//----------------------------------------------------------------------
DifferenceResult CatalogDifferenceEngine::compare(
    const QList<int> &sourceDeviceIds,
    const QList<int> &targetDeviceIds,
    CompareFields matchFields,
    bool checksumNotEqual,
    const QString &tableName,
    bool *stopRequested)
{
    DifferenceResult result;
    int yieldN = 0;

    if (sourceDeviceIds.isEmpty() || targetDeviceIds.isEmpty()) {
        qWarning() << "CatalogDifferenceEngine::compare - empty device ID list";
        return result;
    }

    QString sourceIds = deviceIdListToString(sourceDeviceIds);
    QString targetIds = deviceIdListToString(targetDeviceIds);

    // Subquery template to resolve device_id → catalog external IDs
    QString catalogIdSubquery = R"(
        SELECT device_external_id FROM device
        WHERE device_id IN(%1) AND device_type = 'Catalog'
    )";

    QString sourceCatalogIds = catalogIdSubquery.arg(sourceIds);
    QString targetCatalogIds = catalogIdSubquery.arg(targetIds);

    QSqlQuery query(QSqlDatabase::database(m_connectionName));

    if (checksumNotEqual) {
        // Checksum ≠ mode: find files matching on other fields but with different checksums
        CompareFields joinFields = matchFields & ~Checksum;
        QString joinCondition = buildJoinCondition("t1", "t2", joinFields);

        if (joinCondition.isEmpty()) {
            qWarning() << "CatalogDifferenceEngine::compare - no join fields for checksum≠ mode";
            return result;
        }

        // Source side: files in source with different checksum in target
        QString sqlSource = QString(R"(
            SELECT  t1.file_name,
                    t1.file_size,
                    t1.file_date_updated,
                    t1.file_folder_path,
                    t1.file_catalog,
                    t1.file_catalog_id,
                    t1.checksum_sha256
            FROM %4 t1
            INNER JOIN %4 t2 ON %1
            WHERE t1.checksum_sha256 IS NOT NULL
              AND t1.checksum_sha256 != ''
              AND t2.checksum_sha256 IS NOT NULL
              AND t2.checksum_sha256 != ''
              AND t1.checksum_sha256 != t2.checksum_sha256
              AND t1.file_catalog_id IN (%2)
              AND t2.file_catalog_id IN (%3)
        )").arg(joinCondition, sourceCatalogIds, targetCatalogIds, tableName);

        query.prepare(sqlSource);

        if (!query.exec()) {
            qWarning() << "CatalogDifferenceEngine::compare - checksum≠ source query error:"
                        << query.lastError().text();
            return result;
        }

        while (query.next()) {
            result.differentContent.append(entryFromQuery(query));
            if (yieldAndCheckStop(yieldN, stopRequested)) return result;
        }

        // Target side: files in target with different checksum in source
        QString sqlTarget = QString(R"(
            SELECT  t1.file_name,
                    t1.file_size,
                    t1.file_date_updated,
                    t1.file_folder_path,
                    t1.file_catalog,
                    t1.file_catalog_id,
                    t1.checksum_sha256
            FROM %4 t1
            INNER JOIN %4 t2 ON %1
            WHERE t1.checksum_sha256 IS NOT NULL
              AND t1.checksum_sha256 != ''
              AND t2.checksum_sha256 IS NOT NULL
              AND t2.checksum_sha256 != ''
              AND t1.checksum_sha256 != t2.checksum_sha256
              AND t1.file_catalog_id IN (%3)
              AND t2.file_catalog_id IN (%2)
        )").arg(joinCondition, sourceCatalogIds, targetCatalogIds, tableName);

        query.prepare(sqlTarget);

        if (!query.exec()) {
            qWarning() << "CatalogDifferenceEngine::compare - checksum≠ target query error:"
                        << query.lastError().text();
            return result;
        }

        while (query.next()) {
            result.differentContent.append(entryFromQuery(query));
            if (yieldAndCheckStop(yieldN, stopRequested)) return result;
        }

    }
    else {
        // Standard mode: find files unique to each side
        QString joinCondition = buildJoinCondition("t1", "t2", matchFields);

        if (joinCondition.isEmpty()) {
            qWarning() << "CatalogDifferenceEngine::compare - no match fields specified";
            return result;
        }

        // Files only in source (missing from target)
        QString sqlSource = QString(R"(
            SELECT  t1.file_name,
                    t1.file_size,
                    t1.file_date_updated,
                    t1.file_folder_path,
                    t1.file_catalog,
                    t1.file_catalog_id,
                    t1.checksum_sha256
            FROM %4 t1
            LEFT JOIN %4 t2
                ON %1
                AND t2.file_catalog_id IN (%3)
            WHERE t1.file_catalog_id IN (%2)
            AND t2.file_name IS NULL
        )").arg(joinCondition, sourceCatalogIds, targetCatalogIds, tableName);

        query.prepare(sqlSource);

        if (!query.exec()) {
            qWarning() << "CatalogDifferenceEngine::compare - source query error:"
                        << query.lastError().text();
            return result;
        }

        while (query.next()) {
            result.onlyInSource.append(entryFromQuery(query));
            if (yieldAndCheckStop(yieldN, stopRequested)) return result;
        }

        // Files only in target (missing from source)
        QString sqlTarget = QString(R"(
            SELECT  t1.file_name,
                    t1.file_size,
                    t1.file_date_updated,
                    t1.file_folder_path,
                    t1.file_catalog,
                    t1.file_catalog_id,
                    t1.checksum_sha256
            FROM %4 t1
            LEFT JOIN %4 t2
                ON %1
                AND t2.file_catalog_id IN (%2)
            WHERE t1.file_catalog_id IN (%3)
            AND t2.file_name IS NULL
        )").arg(joinCondition, sourceCatalogIds, targetCatalogIds, tableName);

        query.prepare(sqlTarget);

        if (!query.exec()) {
            qWarning() << "CatalogDifferenceEngine::compare - target query error:"
                        << query.lastError().text();
            return result;
        }

        while (query.next()) {
            result.onlyInTarget.append(entryFromQuery(query));
            if (yieldAndCheckStop(yieldN, stopRequested)) return result;
        }

    }


    return result;
}

//----------------------------------------------------------------------
QList<int> CatalogDifferenceEngine::resolveCatalogDeviceIds(Device *device, const QString &connectionName)
{
    QList<int> catalogIds;

    if (!device)
        return catalogIds;

    device->loadDevice(connectionName);

    if (device->type == "Catalog") {
        catalogIds.append(device->ID);
    }
    else {
        for (const auto &row : device->deviceListTable) {
            if (row.type == "Catalog") {
                catalogIds.append(row.ID);
            }
        }
    }

    return catalogIds;
}

//----------------------------------------------------------------------
QString CatalogDifferenceEngine::buildJoinCondition(const QString &alias1, const QString &alias2, CompareFields fields)
{
    QStringList conditions;

    if (fields & Name)
        conditions << QString("%1.file_name = %2.file_name").arg(alias1, alias2);
    if (fields & Size)
        conditions << QString("%1.file_size = %2.file_size").arg(alias1, alias2);
    if (fields & Date)
        conditions << QString("%1.file_date_updated = %2.file_date_updated").arg(alias1, alias2);
    if (fields & Checksum)
        conditions << QString("%1.checksum_sha256 = %2.checksum_sha256").arg(alias1, alias2);

    return conditions.join(" AND ");
}

//----------------------------------------------------------------------
QString CatalogDifferenceEngine::deviceIdListToString(const QList<int> &ids)
{
    QStringList parts;
    for (int id : ids) {
        parts << QString::number(id);
    }
    return parts.join(",");
}

//----------------------------------------------------------------------
DifferenceFileEntry CatalogDifferenceEngine::entryFromQuery(const QSqlQuery &query)
{
    DifferenceFileEntry entry;
    entry.fileName    = query.value(0).toString();
    entry.fileSize    = query.value(1).toLongLong();
    entry.dateUpdated = query.value(2).toString();
    entry.folderPath  = query.value(3).toString();
    entry.catalog     = query.value(4).toString();
    entry.catalogId   = query.value(5).toInt();
    entry.checksum    = query.value(6).toString();
    return entry;
}
//----------------------------------------------------------------------
StrictDifferenceResult CatalogDifferenceEngine::compareStrict(
    int sourceCatalogId,
    int targetCatalogId,
    const QString &sourceRoot,
    const QString &targetRoot,
    bool *stopRequested)
{
    StrictDifferenceResult result;
    int yieldN = 0;

    // Normalize roots: remove trailing slash
    const QString sourceRootNorm = sourceRoot.endsWith('/') ? sourceRoot.chopped(1) : sourceRoot;
    const QString targetRootNorm = targetRoot.endsWith('/') ? targetRoot.chopped(1) : targetRoot;
    const int sourceRootLen = sourceRootNorm.length();

    QSqlQuery q(QSqlDatabase::database(m_connectionName));

    // Files to copy: no file at the corresponding relative path in target
    q.prepare(R"(
        SELECT file_name, file_folder_path, file_size, file_date_updated
        FROM file f1
        WHERE f1.file_catalog_id = :sourceId
        AND NOT EXISTS (
            SELECT 1 FROM file f2
            WHERE f2.file_catalog_id = :targetId
            AND f2.file_name = f1.file_name
            AND f2.file_folder_path = :targetRoot || SUBSTR(f1.file_folder_path, :rootLen + 1)
        )
    )");
    q.bindValue(":sourceId",   sourceCatalogId);
    q.bindValue(":targetId",   targetCatalogId);
    q.bindValue(":targetRoot", targetRootNorm);
    q.bindValue(":rootLen",    sourceRootLen);

    if (q.exec()) {
        while (q.next()) {
            DifferenceFileEntry e;
            e.fileName    = q.value(0).toString();
            e.folderPath  = q.value(1).toString();
            e.fileSize    = q.value(2).toLongLong();
            e.dateUpdated = q.value(3).toString();
            result.filesToCopy.append(e);
            if (yieldAndCheckStop(yieldN, stopRequested)) return result;
        }
    } else {
        qWarning() << "CatalogDifferenceEngine::compareStrict - filesToCopy error:"
                   << q.lastError().text();
    }

    // Conflicts: file exists at the corresponding path but with a different size.
    // JOIN (instead of EXISTS) so we can return the target file's date alongside
    // the source fields — needed for direction check in RenameOldest conflict mode.
    q.prepare(R"(
        SELECT f1.file_name,
               f1.file_folder_path,
               f1.file_size,
               f1.file_date_updated,
               f2.file_date_updated AS target_date_updated
        FROM file f1
        JOIN file f2
            ON  f2.file_catalog_id = :targetId
            AND f2.file_name       = f1.file_name
            AND f2.file_folder_path = :targetRoot || SUBSTR(f1.file_folder_path, :rootLen + 1)
            AND f2.file_size       != f1.file_size
        WHERE f1.file_catalog_id = :sourceId
    )");
    q.bindValue(":sourceId",   sourceCatalogId);
    q.bindValue(":targetId",   targetCatalogId);
    q.bindValue(":targetRoot", targetRootNorm);
    q.bindValue(":rootLen",    sourceRootLen);

    if (q.exec()) {
        while (q.next()) {
            DifferenceFileEntry e;
            e.fileName          = q.value(0).toString();
            e.folderPath        = q.value(1).toString();
            e.fileSize          = q.value(2).toLongLong();
            e.dateUpdated       = q.value(3).toString();
            e.targetDateUpdated = q.value(4).toString();
            result.conflicts.append(e);
            if (yieldAndCheckStop(yieldN, stopRequested)) return result;
        }
    } else {
        qWarning() << "CatalogDifferenceEngine::compareStrict - conflicts error:"
                   << q.lastError().text();
    }

    // Skipped = total source files minus to-copy and conflicts
    q.prepare("SELECT COUNT(*) FROM file WHERE file_catalog_id = :sourceId");
    q.bindValue(":sourceId", sourceCatalogId);
    if (q.exec() && q.next())
        result.skippedCount = q.value(0).toInt()
                              - result.filesToCopy.size()
                              - result.conflicts.size();

    return result;
}
//----------------------------------------------------------------------
StrictDifferenceResult CatalogDifferenceEngine::compareStrictFromDrive(
    const QString &sourceRoot,
    int targetCatalogId,
    const QString &targetRoot,
    bool *stopRequested)
{
    StrictDifferenceResult result;
    int yieldN = 0;

    const QString sourceRootNorm = sourceRoot.endsWith('/') ? sourceRoot.chopped(1) : sourceRoot;
    const QString targetRootNorm = targetRoot.endsWith('/') ? targetRoot.chopped(1) : targetRoot;

    // Build a lookup map from the target catalog:
    // key = relative folder path (starts and ends with '/') + file_name
    // e.g. "/docs/" + "report.pdf" → "/docs/report.pdf"
    struct TargetEntry { qint64 size; QString dateUpdated; };
    QHash<QString, TargetEntry> targetMap;
    {
        QSqlQuery q(QSqlDatabase::database(m_connectionName));
        q.prepare("SELECT file_name, file_folder_path, file_size, file_date_updated "
                  "FROM file WHERE file_catalog_id = :targetId");
        q.bindValue(":targetId", targetCatalogId);
        if (q.exec()) {
            const int targetRootLen = targetRootNorm.length();
            while (q.next()) {
                const QString name     = q.value(0).toString();
                const QString fullPath = q.value(1).toString();
                // Relative folder = everything after targetRoot, including trailing slash
                const QString relFolder = fullPath.mid(targetRootLen);
                targetMap.insert(relFolder + name, {q.value(2).toLongLong(), q.value(3).toString()});
                if (yieldAndCheckStop(yieldN, stopRequested)) return result;
            }
        } else {
            qWarning() << "compareStrictFromDrive: failed to load target catalog:"
                       << q.lastError().text();
            return result;
        }
    }

    // Walk the source filesystem
    int totalSourceFiles = 0;
    QDirIterator it(sourceRootNorm,
                    QDir::Files | QDir::Hidden | QDir::System | QDir::NoDotAndDotDot,
                    QDirIterator::Subdirectories);
    const int sourceRootLen = sourceRootNorm.length();

    while (it.hasNext()) {
        it.next();
        const QFileInfo fi(it.fileInfo());
        totalSourceFiles++;

        // Relative folder: strip sourceRoot prefix, keep trailing slash
        // e.g. "/source/docs" → "/docs/"
        const QString absDir    = fi.absolutePath();
        const QString relFolder = absDir.mid(sourceRootLen) + "/";
        const QString key       = relFolder + fi.fileName();

        DifferenceFileEntry e;
        e.fileName   = fi.fileName();
        e.folderPath = absDir + "/";
        e.fileSize   = fi.size();
        e.dateUpdated = fi.lastModified().toString(Qt::ISODate);

        auto targetIt = targetMap.constFind(key);
        if (targetIt == targetMap.constEnd()) {
            result.filesToCopy.append(e);
        } else if (targetIt->size != fi.size()) {
            e.targetDateUpdated = targetIt->dateUpdated;
            result.conflicts.append(e);
        }
        // else: same name, same relative path, same size → already in sync (skipped)
        if (yieldAndCheckStop(yieldN, stopRequested)) return result;
    }

    result.skippedCount = totalSourceFiles
                          - result.filesToCopy.size()
                          - result.conflicts.size();

    return result;
}
//----------------------------------------------------------------------
BackupCompareResult CatalogDifferenceEngine::compareForBackup(
    const Device &sourceDevice, const Device &targetDevice,
    bool strictCopy, bool sourceDrive, bool *stopRequested)
{
    BackupCompareResult out;

    if (sourceDrive) {
        // DRIVE mode: walk the source filesystem directly (independent of the source
        // catalog index, so it works even when "Update catalogs" is off).
        const StrictDifferenceResult r = compareStrictFromDrive(
            sourceDevice.path, targetDevice.externalID, targetDevice.path, stopRequested);
        out.filesToCopy   = r.filesToCopy;
        out.fileConflicts = r.conflicts;
        out.skippedCount  = r.skippedCount;
    } else if (strictCopy) {
        const StrictDifferenceResult r = compareStrict(
            sourceDevice.externalID, targetDevice.externalID,
            sourceDevice.path, targetDevice.path, stopRequested);
        out.filesToCopy   = r.filesToCopy;
        out.fileConflicts = r.conflicts;
        out.skippedCount  = r.skippedCount;
    } else {
        const QList<int> sourceIds = resolveCatalogDeviceIds(
            const_cast<Device*>(&sourceDevice), m_connectionName);
        const QList<int> targetIds = resolveCatalogDeviceIds(
            const_cast<Device*>(&targetDevice), m_connectionName);
        const DifferenceResult diff = compare(
            sourceIds, targetIds, Name | Size, false, QStringLiteral("file"), stopRequested);

        BackupMappingManager manager(m_connectionName);
        const QSet<QString> targetPaths = manager.getCatalogFilePaths(targetDevice.externalID);
        const int           srcRootLen  = sourceDevice.path.length();
        for (const DifferenceFileEntry &e : diff.onlyInSource) {
            const QString relFolder    = e.folderPath.mid(srcRootLen);
            const QString expectedPath = targetDevice.path + relFolder + QLatin1Char('/') + e.fileName;
            if (targetPaths.contains(expectedPath))
                out.fileConflicts.append(e);
            else
                out.filesToCopy.append(e);
        }
        out.skippedCount = manager.getCatalogFileCount(sourceDevice.externalID)
                           - out.filesToCopy.size()
                           - out.fileConflicts.size();
    }
    return out;
}
//----------------------------------------------------------------------
