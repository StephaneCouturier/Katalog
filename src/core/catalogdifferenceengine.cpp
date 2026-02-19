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

#include <QSqlQuery>
#include <QSqlError>
#include <QSqlDatabase>
#include <QDebug>

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
    const QString &tableName)
{
    DifferenceResult result;

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
        }

        qDebug() << "CatalogDifferenceEngine::compare (checksum≠) - Found"
                 << result.differentContent.size() << "conflicts";
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
        }

        qDebug() << "CatalogDifferenceEngine::compare - Found"
                 << result.onlyInSource.size() << "only-in-source,"
                 << result.onlyInTarget.size() << "only-in-target";
    }

    qDebug() << "  Source device IDs:" << sourceIds;
    qDebug() << "  Target device IDs:" << targetIds;

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
