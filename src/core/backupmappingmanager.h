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
// File Name:   backupmappingmanager.h
// Purpose:     Class for managing association 2 catalogs, one as source of backup, the other as target
// Description:
// Author:      Stephane Couturier
/////////////////////////////////////////////////////////////////////////////
*/
#ifndef BACKUPMAPPINGMANAGER_H
#define BACKUPMAPPINGMANAGER_H

#include <QObject>
#include <QString>
#include <QList>
#include <QSqlQuery>

/**
 * @brief Represents a backup mapping with source and target information
 */
struct MappingInfo {
    // Order optimized to minimize padding
    qint64 sourceSize;
    qint64 targetSize;
    QString mappingName;
    QString mappingType;
    QString sourceName;
    QString sourcePath;
    QString sourceDateUpdated;
    QString targetName;
    QString targetPath;
    QString targetDateUpdated;
    int mappingId;
    int sourceDeviceId;
    int sourceFileCount;
    int targetDeviceId;
    int targetFileCount;
    bool sourceActive;
    bool targetActive;

    MappingInfo()
        : sourceSize(0)
        , targetSize(0)
        , mappingId(-1)
        , sourceDeviceId(-1)
        , sourceFileCount(0)
        , targetDeviceId(-1)
        , targetFileCount(0)
        , sourceActive(false)
        , targetActive(false)
    {}
};

/**
 * @brief Aggregated statistics for a set of mappings
 */
struct MappingTotals {
    int totalMappings;
    qint64 totalSourceSize;
    qint64 totalTargetSize;
    qint64 totalSizeDifference;
    double totalSizeDifferencePercentage;
    int totalSourceFileCount;
    int totalTargetFileCount;
    int totalFileCountDifference;
    double totalFileCountDifferencePercentage;
    double averageSizeDifferencePercentage;
    double averageFileCountDifferencePercentage;

    MappingTotals()
        : totalMappings(0)
        , totalSourceSize(0)
        , totalTargetSize(0)
        , totalSizeDifference(0)
        , totalSizeDifferencePercentage(0.0)
        , totalSourceFileCount(0)
        , totalTargetFileCount(0)
        , totalFileCountDifference(0)
        , totalFileCountDifferencePercentage(0.0)
        , averageSizeDifferencePercentage(0.0)
        , averageFileCountDifferencePercentage(0.0)
    {}
};

/**
 * @brief Filter criteria for backup mappings
 */
struct MappingFilter {
    enum FilterType {
        None,           // No filter (all mappings)
        SourceDevice,   // Filter by source device and its children
        TargetDevice    // Filter by target device and its children
    };

    FilterType type;
    int deviceId;

    MappingFilter()
        : type(None)
        , deviceId(-1)
    {}

    MappingFilter(FilterType t, int devId)
        : type(t)
        , deviceId(devId)
    {}
};

/**
 * @brief Backend manager for backup mappings
 */
class BackupMappingManager : public QObject
{
    Q_OBJECT

public:
    explicit BackupMappingManager(const QString& connectionName,
                                  QObject *parent = nullptr);

    // Query methods
    QList<int> getAllMappingIds();
    QList<int> getFilteredMappingIds(const MappingFilter& filter);
    QList<MappingInfo> getAllMappings();
    QList<MappingInfo> getFilteredMappings(const MappingFilter& filter);
    MappingInfo getMappingById(int mappingId);

    // Changes
    bool deleteMapping(int mappingId);

    // Statistics
    int getMappingCount();
    int getFilteredMappingCount(const MappingFilter& filter);
    MappingTotals calculateTotals(const MappingFilter& filter);

    // Query building (for UI compatibility)
    QString buildMappingQuery(const MappingFilter& filter);

signals:
    void mappingsLoaded(int count);
    void error(const QString& message);

private:
    QString buildBaseQuery();
    QString buildFilterClause(const MappingFilter& filter);
    void bindFilterParameters(QSqlQuery& query, const MappingFilter& filter);
    MappingInfo parseMappingFromQuery(const QSqlQuery& query);

    QString m_connectionName;
};

#endif // BACKUPMAPPINGMANAGER_H
