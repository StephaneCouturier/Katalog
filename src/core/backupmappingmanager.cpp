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
 * /////////////////////////////////////////////////////////////////////////////
// Application: Katalog
// File Name:   backupmappingmanager.cpp
// Purpose:     Class for managing association 2 catalogs, one as source of backup, the other as target
// Description:
// Author:      Stephane Couturier
/////////////////////////////////////////////////////////////////////////////
*/

#include "backupmappingmanager.h"
#include <QSqlDatabase>
#include <QSqlError>
#include <QDebug>

BackupMappingManager::BackupMappingManager(const QString& connectionName,
                                           QObject *parent)
    : QObject(parent)
    , m_connectionName(connectionName)
{
}

//------------------------------------------------------------------------------
// Query Building
//------------------------------------------------------------------------------

QString BackupMappingManager::buildBaseQuery()
{
    return QLatin1String(R"(
        SELECT
            dm.mapping_id,
            dm.mapping_name,
            dm.mapping_type,
            dm.mapping_device_source_id,
            d1.device_name AS source_name,
            d1.device_active AS source_active,
            d1.device_path AS source_path,
            d1.device_total_file_size AS source_size,
            d1.device_total_file_count AS source_file_count,
            d1.device_date_updated AS source_date_updated,
            dm.mapping_device_target_id,
            d2.device_name AS target_name,
            d2.device_active AS target_active,
            d2.device_path AS target_path,
            d2.device_total_file_size AS target_size,
            d2.device_total_file_count AS target_file_count,
            d2.device_date_updated AS target_date_updated,
            COALESCE(dm.mapping_strict_copy, 1) AS mapping_strict_copy
        FROM device_mapping dm
        JOIN device d1 ON dm.mapping_device_source_id = d1.device_id
        JOIN device d2 ON dm.mapping_device_target_id = d2.device_id
        WHERE dm.mapping_type = 'Backup'
    )");
}

QString BackupMappingManager::buildFilterClause(const MappingFilter& filter)
{
    if (filter.type == MappingFilter::None) {
        return "";
    }

    QString deviceField = (filter.type == MappingFilter::SourceDevice)
                              ? "dm.mapping_device_source_id"
                              : "dm.mapping_device_target_id";

    return QString(R"(
        AND (%1 = :device_id
            OR %1 IN (
                WITH RECURSIVE hierarchy(device_id, device_parent_id, device_name) AS (
                    SELECT device_id, device_parent_id, device_name
                    FROM device
                    WHERE device_parent_id = :device_parent_id
                    UNION ALL
                    SELECT t.device_id, t.device_parent_id, t.device_name
                    FROM device t
                    JOIN hierarchy h ON t.device_parent_id = h.device_id
                )
                SELECT device_id FROM hierarchy
            )
        )
    )").arg(deviceField);
}

QString BackupMappingManager::buildMappingQuery(const MappingFilter& filter)
{
    QString query = buildBaseQuery();
    query += buildFilterClause(filter);
    query += " ORDER BY dm.mapping_name ASC";
    return query;
}

void BackupMappingManager::bindFilterParameters(QSqlQuery& query,
                                                const MappingFilter& filter)
{
    if (filter.type != MappingFilter::None && filter.deviceId > 0) {
        query.bindValue(":device_id", filter.deviceId);
        query.bindValue(":device_parent_id", filter.deviceId);
    }
}

//------------------------------------------------------------------------------
// Data Retrieval - IDs Only
//------------------------------------------------------------------------------

QList<int> BackupMappingManager::getAllMappingIds()
{
    return getFilteredMappingIds(MappingFilter());
}

QList<int> BackupMappingManager::getFilteredMappingIds(const MappingFilter& filter)
{
    QList<int> ids;

    QString querySQL = buildMappingQuery(filter);

    QSqlQuery query(QSqlDatabase::database(m_connectionName));
    query.prepare(querySQL);
    bindFilterParameters(query, filter);

    if (!query.exec()) {
        QString errorMsg = QString("Failed to load mapping IDs: %1").arg(query.lastError().text());
        qDebug() << "ERROR:" << errorMsg;
        emit error(errorMsg);
        return ids;
    }

    while (query.next()) {
        ids.append(query.value("mapping_id").toInt());
    }

    qDebug() << "Loaded" << ids.size() << "mapping IDs (filter type:"
             << filter.type << ")";

    return ids;
}

//------------------------------------------------------------------------------
// Data Retrieval - Full Info
//------------------------------------------------------------------------------

MappingInfo BackupMappingManager::parseMappingFromQuery(const QSqlQuery& query)
{
    MappingInfo info;

    info.mappingId = query.value("mapping_id").toInt();
    info.mappingName = query.value("mapping_name").toString();
    info.mappingType = query.value("mapping_type").toString();

    info.sourceDeviceId = query.value("mapping_device_source_id").toInt();
    info.sourceName = query.value("source_name").toString();
    info.sourcePath = query.value("source_path").toString();
    info.sourceActive = query.value("source_active").toBool();
    info.sourceSize = query.value("source_size").toLongLong();
    info.sourceFileCount = query.value("source_file_count").toInt();
    info.sourceDateUpdated = query.value("source_date_updated").toString();

    info.targetDeviceId = query.value("mapping_device_target_id").toInt();
    info.targetName = query.value("target_name").toString();
    info.targetPath = query.value("target_path").toString();
    info.targetActive = query.value("target_active").toBool();
    info.targetSize = query.value("target_size").toLongLong();
    info.targetFileCount = query.value("target_file_count").toInt();
    info.targetDateUpdated = query.value("target_date_updated").toString();

    info.strictCopy = query.value("mapping_strict_copy").toInt() != 0;

    return info;
}

QList<MappingInfo> BackupMappingManager::getAllMappings()
{
    return getFilteredMappings(MappingFilter());
}

QList<MappingInfo> BackupMappingManager::getFilteredMappings(const MappingFilter& filter)
{
    QList<MappingInfo> mappings;

    QString querySQL = buildMappingQuery(filter);

    QSqlQuery query(QSqlDatabase::database(m_connectionName));
    query.prepare(querySQL);
    bindFilterParameters(query, filter);

    if (!query.exec()) {
        QString errorMsg = QString("Failed to load mappings: %1").arg(query.lastError().text());
        qDebug() << "ERROR:" << errorMsg;
        emit error(errorMsg);
        return mappings;
    }

    while (query.next()) {
        mappings.append(parseMappingFromQuery(query));
    }

    qDebug() << "Loaded" << mappings.size() << "mappings (filter type:"
             << filter.type << ")";
    emit mappingsLoaded(mappings.size());

    return mappings;
}

MappingInfo BackupMappingManager::getMappingById(int mappingId)
{
    MappingInfo info;

    QSqlQuery query(QSqlDatabase::database(m_connectionName));
    QString querySQL = buildBaseQuery() + " AND dm.mapping_id = :mapping_id";

    query.prepare(querySQL);
    query.bindValue(":mapping_id", mappingId);

    if (!query.exec()) {
        qDebug() << "ERROR: Failed to load mapping" << mappingId
                 << ":" << query.lastError().text();
        return info;
    }

    if (query.next()) {
        info = parseMappingFromQuery(query);
    }

    return info;
}

//------------------------------------------------------------------------------
// Changes
//------------------------------------------------------------------------------

bool BackupMappingManager::deleteMapping(int mappingId)
{
    QSqlQuery query(QSqlDatabase::database(m_connectionName));
    QString querySQL = QLatin1String(R"(
        DELETE FROM device_mapping
        WHERE mapping_id = :mapping_id
    )");
    query.prepare(querySQL);
    query.bindValue(":mapping_id", mappingId);

    if (!query.exec()) {
        QString errorMsg = QString("Failed to delete mapping %1: %2")
                               .arg(mappingId)
                               .arg(query.lastError().text());
        qDebug() << "ERROR:" << errorMsg;
        emit error(errorMsg);
        return false;
    }

    return true;
}

//------------------------------------------------------------------------------
// Statistics
//------------------------------------------------------------------------------

int BackupMappingManager::getMappingCount()
{
    return getFilteredMappingCount(MappingFilter());
}

int BackupMappingManager::getFilteredMappingCount(const MappingFilter& filter)
{
    QString querySQL = "SELECT COUNT(*) FROM device_mapping dm WHERE dm.mapping_type = 'Backup'";
    querySQL += buildFilterClause(filter);

    QSqlQuery query(QSqlDatabase::database(m_connectionName));
    query.prepare(querySQL);
    bindFilterParameters(query, filter);

    if (!query.exec() || !query.next()) {
        qDebug() << "ERROR: Failed to count mappings:" << query.lastError().text();
        return 0;
    }

    return query.value(0).toInt();
}

MappingTotals BackupMappingManager::calculateTotals(const MappingFilter& filter)
{
    MappingTotals totals;

    QString querySQL = QLatin1String(R"(
        SELECT
            COUNT(*) AS total_mappings,
            SUM(d1.device_total_file_size) AS total_source_file_size,
            SUM(d2.device_total_file_size) AS total_target_file_size,
            SUM(d2.device_total_file_size - d1.device_total_file_size) AS total_size_difference,
            ROUND(SUM(d2.device_total_file_size) * 100.0 / NULLIF(SUM(d1.device_total_file_size), 0), 2) AS total_size_difference_percentage,
            SUM(d1.device_total_file_count) AS total_source_file_count,
            SUM(d2.device_total_file_count) AS total_target_file_count,
            SUM(d2.device_total_file_count - d1.device_total_file_count) AS total_file_count_difference,
            ROUND(SUM(d2.device_total_file_count) * 100.0 / NULLIF(SUM(d1.device_total_file_count), 0), 2) AS total_file_count_difference_percentage,
            ROUND(AVG(d2.device_total_file_size) * 100.0 / NULLIF(AVG(d1.device_total_file_size), 0), 2) AS average_size_difference_percentage,
            ROUND(AVG(d2.device_total_file_count) * 100.0 / NULLIF(AVG(d1.device_total_file_count), 0), 2) AS average_file_count_difference_percentage
        FROM device_mapping dm
        JOIN device d1 ON dm.mapping_device_source_id = d1.device_id
        JOIN device d2 ON dm.mapping_device_target_id = d2.device_id
        WHERE dm.mapping_type = 'Backup'
    )");

    querySQL += buildFilterClause(filter);

    QSqlQuery query(QSqlDatabase::database(m_connectionName));
    query.prepare(querySQL);
    bindFilterParameters(query, filter);

    if (!query.exec()) {
        qDebug() << "ERROR: Failed to calculate totals:" << query.lastError().text();
        emit error("Failed to calculate mapping totals");
        return totals;
    }

    if (query.next()) {
        totals.totalMappings = query.value("total_mappings").toInt();
        totals.totalSourceSize = query.value("total_source_file_size").toLongLong();
        totals.totalTargetSize = query.value("total_target_file_size").toLongLong();
        totals.totalSizeDifference = query.value("total_size_difference").toLongLong();
        totals.totalSizeDifferencePercentage = query.value("total_size_difference_percentage").toDouble();
        totals.totalSourceFileCount = query.value("total_source_file_count").toInt();
        totals.totalTargetFileCount = query.value("total_target_file_count").toInt();
        totals.totalFileCountDifference = query.value("total_file_count_difference").toInt();
        totals.totalFileCountDifferencePercentage = query.value("total_file_count_difference_percentage").toDouble();
        totals.averageSizeDifferencePercentage = query.value("average_size_difference_percentage").toDouble();
        totals.averageFileCountDifferencePercentage = query.value("average_file_count_difference_percentage").toDouble();
    }

    qDebug() << "Calculated totals for" << totals.totalMappings << "mappings";
    return totals;
}

