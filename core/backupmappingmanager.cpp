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
#include "database.h"
#include <QSqlDatabase>
#include <QSqlError>
#include <QDebug>
#include <QFile>
#include <QTextStream>
#include <QDateTime>

BackupMappingManager::BackupMappingManager(const QString& connectionName,
                                           QObject *parent)
    : QObject(parent)
    , m_connectionName(connectionName)
{
}

//------------------------------------------------------------------------------
// Query Building
//------------------------------------------------------------------------------

QString BackupMappingManager::buildSelectFromJoin()
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
            COALESCE(dm.mapping_strict_copy,    1) AS mapping_strict_copy,
            COALESCE(dm.mapping_conflict_mode, 'RenameOldest') AS mapping_conflict_mode,
            COALESCE(dm.mapping_source_mode, 'Catalog') AS mapping_source_mode,
            dm.mapping_backup_last_date AS backup_last_date,
            COALESCE(dm.mapping_backup_last_size, 0) AS backup_last_size
        FROM device_mapping dm
        JOIN device d1 ON dm.mapping_device_source_id = d1.device_id
        JOIN device d2 ON dm.mapping_device_target_id = d2.device_id
    )");
}

QString BackupMappingManager::buildBaseQuery()
{
    return buildSelectFromJoin() + " WHERE dm.mapping_type != 'CollectionImport'";
}

QString BackupMappingManager::buildFilterClause(const MappingFilter& filter)
{
    QString clause;

    // Type filter — omit when "All" so every mapping_type is included
    if (filter.mappingType != QLatin1String("All") && !filter.mappingType.isEmpty()) {
        clause += " AND dm.mapping_type = :mapping_type";
    }

    // Device filter
    if (filter.type != MappingFilter::None && filter.deviceId > 0) {
        QString deviceField = (filter.type == MappingFilter::SourceDevice)
                                  ? "dm.mapping_device_source_id"
                                  : "dm.mapping_device_target_id";

        clause += QString(R"(
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

    return clause;
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
    if (filter.mappingType != QLatin1String("All") && !filter.mappingType.isEmpty()) {
        query.bindValue(":mapping_type", filter.mappingType);
    }
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
        qWarning() << "WARNING: " << errorMsg;
        emit error(errorMsg);
        return ids;
    }

    while (query.next()) {
        ids.append(query.value("mapping_id").toInt());
    }


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

    info.strictCopy                = query.value("mapping_strict_copy").toInt() != 0;
    info.conflictMode              = conflictModeFromString(query.value("mapping_conflict_mode").toString());
    info.sourceDrive               = query.value("mapping_source_mode").toString() == QLatin1String("Drive");
    info.lastBackupDate            = query.value("backup_last_date").toString();
    info.lastBackupSize            = query.value("backup_last_size").toLongLong();

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
        qWarning() << "WARNING: " << errorMsg;
        emit error(errorMsg);
        return mappings;
    }

    while (query.next()) {
        mappings.append(parseMappingFromQuery(query));
    }

    emit mappingsLoaded(mappings.size());

    return mappings;
}

MappingInfo BackupMappingManager::getMappingById(int mappingId)
{
    MappingInfo info;

    // Lookup by primary key — no mapping_type filter needed
    QSqlQuery query(QSqlDatabase::database(m_connectionName));
    QString querySQL = buildSelectFromJoin() + " WHERE dm.mapping_id = :mapping_id";

    query.prepare(querySQL);
    query.bindValue(":mapping_id", mappingId);

    if (!query.exec()) {
        qWarning() << "WARNING: Failed to load mapping" << mappingId
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

bool BackupMappingManager::invertMapping(int mappingId)
{
    QSqlDatabase db = QSqlDatabase::database(m_connectionName);

    // Step 1: swap source/target device IDs
    {
        QSqlQuery query(db);
        query.prepare(QLatin1String(R"(
            UPDATE device_mapping
            SET mapping_device_source_id = mapping_device_target_id,
                mapping_device_target_id = mapping_device_source_id
            WHERE mapping_id = :mapping_id
        )"));
        query.bindValue(":mapping_id", mappingId);

        if (!query.exec()) {
            QString errorMsg = QString("Failed to invert mapping %1: %2")
                                   .arg(mappingId)
                                   .arg(query.lastError().text());
            qWarning() << "WARNING: " << errorMsg;
            emit error(errorMsg);
            return false;
        }
    }

    // Step 2: if the name contains an arrow ("→" or "->"), reverse the two halves
    {
        QSqlQuery nameQuery(db);
        nameQuery.prepare("SELECT mapping_name FROM device_mapping WHERE mapping_id = :id");
        nameQuery.bindValue(":id", mappingId);
        if (nameQuery.exec() && nameQuery.next()) {
            QString name = nameQuery.value(0).toString();
            // Support both the Unicode arrow and the legacy ASCII arrow
            QString arrow;
            if (name.contains(QStringLiteral(" \u2192 ")))
                arrow = QStringLiteral(" \u2192 ");
            else if (name.contains(QStringLiteral(" -> ")))
                arrow = QStringLiteral(" -> ");

            if (!arrow.isEmpty()) {
                const QStringList parts = name.split(arrow);
                if (parts.size() == 2) {
                    const QString newName = parts.at(1) + arrow + parts.at(0);
                    QSqlQuery updateQuery(db);
                    updateQuery.prepare(
                        "UPDATE device_mapping SET mapping_name = :name WHERE mapping_id = :id");
                    updateQuery.bindValue(":name", newName);
                    updateQuery.bindValue(":id", mappingId);
                    updateQuery.exec();
                }
            }
        }
    }

    return true;
}

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
        qWarning() << "WARNING: " << errorMsg;
        emit error(errorMsg);
        return false;
    }

    return true;
}

bool BackupMappingManager::createMapping(
    const QString &name, const QString &type,
    int sourceId, int targetId,
    bool strictCopy, const QString &conflictMode,
    bool sourceDrive)
{
    QSqlQuery query(QSqlDatabase::database(m_connectionName));
    query.prepare(QLatin1String(R"(
        INSERT INTO device_mapping
        (mapping_name, mapping_type, mapping_device_source_id, mapping_device_target_id,
         mapping_strict_copy, mapping_conflict_mode, mapping_source_mode)
        VALUES (:name, :type, :sourceId, :targetId, :strictCopy, :conflictMode, :sourceMode)
    )"));
    query.bindValue(":name",         name);
    query.bindValue(":type",         type);
    query.bindValue(":sourceId",     sourceId);
    query.bindValue(":targetId",     targetId);
    query.bindValue(":strictCopy",   strictCopy ? 1 : 0);
    query.bindValue(":conflictMode", conflictMode);
    query.bindValue(":sourceMode",   sourceDrive ? QStringLiteral("Drive") : QStringLiteral("Catalog"));
    if (!query.exec()) {
        qWarning() << "BackupMappingManager::createMapping error:" << query.lastError();
        return false;
    }
    return true;
}

bool BackupMappingManager::updateMapping(
    int mappingId,
    const QString &name, const QString &type,
    int sourceId, int targetId,
    bool strictCopy, const QString &conflictMode,
    bool sourceDrive)
{
    QSqlQuery query(QSqlDatabase::database(m_connectionName));
    query.prepare(QLatin1String(R"(
        UPDATE device_mapping SET
            mapping_name             = :name,
            mapping_type             = :type,
            mapping_device_source_id = :sourceId,
            mapping_device_target_id = :targetId,
            mapping_strict_copy      = :strictCopy,
            mapping_conflict_mode    = :conflictMode,
            mapping_source_mode      = :sourceMode
        WHERE mapping_id = :mappingId
    )"));
    query.bindValue(":name",         name);
    query.bindValue(":type",         type);
    query.bindValue(":sourceId",     sourceId);
    query.bindValue(":targetId",     targetId);
    query.bindValue(":strictCopy",   strictCopy ? 1 : 0);
    query.bindValue(":conflictMode", conflictMode);
    query.bindValue(":sourceMode",   sourceDrive ? QStringLiteral("Drive") : QStringLiteral("Catalog"));
    query.bindValue(":mappingId",    mappingId);
    if (!query.exec()) {
        qWarning() << "BackupMappingManager::updateMapping error:" << query.lastError();
        return false;
    }
    return true;
}

bool BackupMappingManager::setLastBackup(int mappingId, const QString &dateTime, qint64 size)
{
    QSqlQuery query(QSqlDatabase::database(m_connectionName));
    query.prepare(QLatin1String(R"(
        UPDATE device_mapping SET
            mapping_backup_last_date = :date,
            mapping_backup_last_size = :size
        WHERE mapping_id = :mappingId
    )"));
    query.bindValue(":date",      dateTime);
    query.bindValue(":size",      size);
    query.bindValue(":mappingId", mappingId);
    if (!query.exec()) {
        qWarning() << "BackupMappingManager::setLastBackup error:" << query.lastError();
        return false;
    }
    return true;
}

QSet<QString> BackupMappingManager::getCatalogFilePaths(int catalogExternalId) const
{
    QSet<QString> paths;
    QSqlQuery q(QSqlDatabase::database(m_connectionName));
    q.prepare(QLatin1String("SELECT file_folder_path || '/' || file_name FROM file WHERE file_catalog_id = :id"));
    q.bindValue(":id", catalogExternalId);
    if (q.exec()) {
        while (q.next())
            paths.insert(q.value(0).toString());
    }
    return paths;
}

int BackupMappingManager::getCatalogFileCount(int catalogExternalId) const
{
    QSqlQuery q(QSqlDatabase::database(m_connectionName));
    q.prepare(QLatin1String("SELECT COUNT(*) FROM file WHERE file_catalog_id = :id"));
    q.bindValue(":id", catalogExternalId);
    if (q.exec() && q.next())
        return q.value(0).toInt();
    return 0;
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
    QString querySQL = "SELECT COUNT(*) FROM device_mapping dm WHERE dm.mapping_type != 'CollectionImport'";
    querySQL += buildFilterClause(filter);

    QSqlQuery query(QSqlDatabase::database(m_connectionName));
    query.prepare(querySQL);
    bindFilterParameters(query, filter);

    if (!query.exec() || !query.next()) {
        qWarning() << "WARNING: Failed to count mappings:" << query.lastError().text();
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
        WHERE dm.mapping_type != 'CollectionImport'
    )");

    querySQL += buildFilterClause(filter);

    QSqlQuery query(QSqlDatabase::database(m_connectionName));
    query.prepare(querySQL);
    bindFilterParameters(query, filter);

    if (!query.exec()) {
        qWarning() << "WARNING: Failed to calculate totals:" << query.lastError().text();
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

    return totals;
}

// ─── Table Display Query ───────────────────────────────────────────────────

QSqlQuery BackupMappingManager::executeTableDisplayQuery(const MappingFilter& filter)
{
    Database::DatabaseType dbType = Database::getDatabaseType(m_connectionName);
    QString timeDiffSQL = Database::getFormattedTimeDifference(
        dbType, "d1.device_date_updated", "d2.device_date_updated");

    QString querySQL = QLatin1String(R"(
        SELECT
            dm.mapping_id,
            dm.mapping_name,
            dm.mapping_type,
            CASE WHEN dm.mapping_type = 'Archive' THEN ''
                 WHEN dm.mapping_strict_copy = 1  THEN 'Strict'
                 ELSE 'Unique' END,
            COALESCE(dm.mapping_source_mode, 'Catalog'),
            CASE dm.mapping_conflict_mode
                 WHEN 'Skip'         THEN 'Skip'
                 WHEN 'RenameOldest' THEN 'Rename oldest'
                 ELSE dm.mapping_conflict_mode END,
            dm.mapping_device_source_id,
            d1.device_name,
            d1.device_active,
            d1.device_path,
            d1.device_total_file_size,
            d1.device_total_file_count,
            d1.device_date_updated,
            dm.mapping_device_target_id,
            d2.device_name,
            d2.device_active,
            d2.device_path,
            d2.device_total_file_size,
            d2.device_total_file_count,
            d2.device_date_updated,
            d2.device_total_file_size - d1.device_total_file_size AS size_difference,
            CASE
                WHEN d1.device_total_file_size > 0
                THEN ROUND(((d2.device_total_file_size - d1.device_total_file_size) * 100.0 / d1.device_total_file_size), 2)
                ELSE NULL
            END AS size_difference_percentage,
            d2.device_total_file_count - d1.device_total_file_count AS file_count_difference,
            CASE
                WHEN d1.device_total_file_count > 0
                THEN ROUND(((d2.device_total_file_count - d1.device_total_file_count) * 100.0 / d1.device_total_file_count), 2)
                ELSE NULL
            END AS file_count_difference_percentage,
    )");

    querySQL += "        (" + timeDiffSQL + ") AS formatted_time_difference\n";

    querySQL += QLatin1String(R"(
        FROM device_mapping dm
        JOIN device d1 ON dm.mapping_device_source_id = d1.device_id
        JOIN device d2 ON dm.mapping_device_target_id = d2.device_id
        WHERE dm.mapping_type != 'CollectionImport'
    )");

    querySQL += buildFilterClause(filter);
    querySQL += " ORDER BY dm.mapping_name ASC";

    QSqlQuery query(QSqlDatabase::database(m_connectionName));
    query.prepare(querySQL);
    bindFilterParameters(query, filter);

    if (!query.exec()) {
        QString errorMsg = QString("Failed to load mapping table: %1").arg(query.lastError().text());
        qWarning() << "WARNING: " << errorMsg;
        emit error(errorMsg);
    }

    return query;
}

// ─── Export ────────────────────────────────────────────────────────────────

QString BackupMappingManager::exportPreviewToCsv(const QList<BackupPreviewRow> &rows,
                                                  const QString &collectionFolder)
{
    const QString timestamp = QDateTime::currentDateTime().toString("yyyy-MM-dd_HH-mm-ss");
    const QString filePath = collectionFolder + "/backup_preview_" + timestamp + ".csv";

    QFile file(filePath);
    if (!file.open(QIODevice::WriteOnly | QIODevice::Text)) {
        qWarning() << "WARNING: Cannot write backup preview CSV:" << filePath;
        return QString();
    }

    QTextStream out(&file);
    out << "Status"      << '\t'
        << "File Name"   << '\t'
        << "Folder Path" << '\t'
        << "Size (bytes)" << '\n';

    for (const BackupPreviewRow &row : rows) {
        out << row.status     << '\t'
            << row.fileName   << '\t'
            << row.folderPath << '\t'
            << row.fileSize   << '\n';
    }

    return filePath;
}

