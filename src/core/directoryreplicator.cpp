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
// File Name:   directoryreplicator.cpp
// Purpose:     Replicate a catalog's directory structure to a target path
// Description: Queries the folder table, computes relative paths, and creates
//              or prunes directories on the target filesystem.
// Author:      Stephane Couturier
/////////////////////////////////////////////////////////////////////////////
*/

#include "directoryreplicator.h"

#include <QSqlQuery>
#include <QSqlDatabase>
#include <QSqlError>
#include <QDir>
#include <QDirIterator>
#include <QSet>
#include <QDebug>
#include <algorithm>

//----------------------------------------------------------------------
DirectoryReplicator::DirectoryReplicator(const QString &connectionName)
    : m_connectionName(connectionName)
{
}

//----------------------------------------------------------------------
ReplicationResult DirectoryReplicator::replicate(
    const QList<int> &catalogIds,
    const QString &sourcePath,
    const QString &targetPath,
    bool dryRun)
{
    ReplicationResult result;

    if (catalogIds.isEmpty()) {
        qWarning() << "DirectoryReplicator::replicate - no catalog IDs provided";
        return result;
    }

    if (sourcePath.isEmpty() || targetPath.isEmpty()) {
        qWarning() << "DirectoryReplicator::replicate - empty source or target path";
        return result;
    }

    // Load folder paths from the database
    QStringList absoluteFolders = loadSourceFolders(catalogIds);
    QStringList relativePaths = toRelativePaths(absoluteFolders, sourcePath);

    qDebug() << "DirectoryReplicator::replicate -"
             << relativePaths.size() << "folders to replicate from"
             << sourcePath << "to" << targetPath
             << (dryRun ? "(dry run)" : "");

    // Sort so parent directories are created before children
    std::sort(relativePaths.begin(), relativePaths.end());

    for (const QString &relativePath : relativePaths) {
        if (relativePath.isEmpty())
            continue;

        QString fullTargetPath = targetPath + "/" + relativePath;
        QDir targetDir(fullTargetPath);

        if (targetDir.exists()) {
            result.alreadyExist.append(relativePath);
        }
        else if (dryRun) {
            result.created.append(relativePath);
        }
        else {
            if (QDir().mkpath(fullTargetPath)) {
                result.created.append(relativePath);
            }
            else {
                result.errors.append(relativePath + " (failed to create)");
                qWarning() << "DirectoryReplicator::replicate - failed to create:" << fullTargetPath;
            }
        }
    }

    qDebug() << "DirectoryReplicator::replicate - Done:"
             << result.createdCount() << "created,"
             << result.skippedCount() << "already existed,"
             << result.errorCount() << "errors";

    return result;
}

//----------------------------------------------------------------------
ReplicationResult DirectoryReplicator::replicateAndPrune(
    const QList<int> &catalogIds,
    const QString &sourcePath,
    const QString &targetPath,
    bool dryRun)
{
    // First, replicate (create missing directories)
    ReplicationResult result = replicate(catalogIds, sourcePath, targetPath, dryRun);

    // Build the set of relative paths that should exist (from the source catalog)
    QStringList absoluteFolders = loadSourceFolders(catalogIds);
    QSet<QString> sourceRelativeSet;
    for (const QString &rel : toRelativePaths(absoluteFolders, sourcePath)) {
        if (!rel.isEmpty())
            sourceRelativeSet.insert(rel);
    }

    // Scan the target filesystem for existing directories
    QStringList targetRelativePaths = scanTargetDirectories(targetPath);

    // Sort deepest-first so we remove children before parents
    std::sort(targetRelativePaths.begin(), targetRelativePaths.end(),
              [](const QString &a, const QString &b) { return a.size() > b.size(); });

    for (const QString &targetRelative : targetRelativePaths) {
        if (sourceRelativeSet.contains(targetRelative))
            continue;

        QString fullPath = targetPath + "/" + targetRelative;
        QDir dir(fullPath);

        // Only remove empty directories
        if (!dir.isEmpty())
            continue;

        if (dryRun) {
            result.removed.append(targetRelative);
        }
        else {
            if (dir.rmdir(fullPath)) {
                result.removed.append(targetRelative);
            }
            else {
                result.errors.append(targetRelative + " (failed to remove orphan)");
                qWarning() << "DirectoryReplicator::replicateAndPrune - failed to remove:" << fullPath;
            }
        }
    }

    qDebug() << "DirectoryReplicator::replicateAndPrune -"
             << result.removedCount() << "orphan directories"
             << (dryRun ? "would be removed" : "removed");

    return result;
}

//----------------------------------------------------------------------
ReplicationResult DirectoryReplicator::replicateFromDrive(
    const QString &sourcePath,
    const QString &targetPath,
    bool dryRun)
{
    ReplicationResult result;

    if (sourcePath.isEmpty() || targetPath.isEmpty()) {
        qWarning() << "DirectoryReplicator::replicateFromDrive - empty source or target path";
        return result;
    }

    const QString sourceNorm = sourcePath.endsWith('/') ? sourcePath.chopped(1) : sourcePath;
    const int sourceLen = sourceNorm.length();

    // Walk the source filesystem — directories only, all names (AllDirs = no name filter applied)
    QDirIterator it(sourceNorm,
                    QDir::Dirs | QDir::AllDirs | QDir::NoDotAndDotDot | QDir::Readable,
                    QDirIterator::Subdirectories);

    QStringList relativePaths;
    while (it.hasNext()) {
        const QString absPath = it.next();
        const QString relative = absPath.mid(sourceLen + 1); // strip "sourceNorm/"
        if (!relative.isEmpty())
            relativePaths.append(relative);
    }

    std::sort(relativePaths.begin(), relativePaths.end());

    qDebug() << "DirectoryReplicator::replicateFromDrive -"
             << relativePaths.size() << "directories found under" << sourcePath
             << (dryRun ? "(dry run)" : "");

    for (const QString &relativePath : relativePaths) {
        const QString fullTargetPath = targetPath + "/" + relativePath;
        QDir targetDir(fullTargetPath);

        if (targetDir.exists()) {
            result.alreadyExist.append(relativePath);
        } else if (dryRun) {
            result.created.append(relativePath);
        } else {
            if (QDir().mkpath(fullTargetPath)) {
                result.created.append(relativePath);
            } else {
                result.errors.append(relativePath + " (failed to create)");
                qWarning() << "DirectoryReplicator::replicateFromDrive - failed to create:" << fullTargetPath;
            }
        }
    }

    qDebug() << "DirectoryReplicator::replicateFromDrive - Done:"
             << result.createdCount() << "created,"
             << result.skippedCount() << "already existed,"
             << result.errorCount() << "errors";

    return result;
}

//----------------------------------------------------------------------
QStringList DirectoryReplicator::loadSourceFolders(const QList<int> &catalogIds)
{
    QStringList folders;

    QSqlQuery query(QSqlDatabase::database(m_connectionName));

    // Build the IN clause with parameter placeholders
    QStringList placeholders;
    for (int i = 0; i < catalogIds.size(); ++i)
        placeholders << QString(":id%1").arg(i);

    QString sql = QString("SELECT DISTINCT folder_path FROM folder "
                          "WHERE folder_catalog_id IN (%1) "
                          "ORDER BY folder_path ASC")
                      .arg(placeholders.join(","));

    query.prepare(sql);
    for (int i = 0; i < catalogIds.size(); ++i)
        query.bindValue(QString(":id%1").arg(i), catalogIds.at(i));

    if (!query.exec()) {
        qWarning() << "DirectoryReplicator::loadSourceFolders - query error:"
                    << query.lastError().text();
        return folders;
    }

    while (query.next()) {
        folders.append(query.value(0).toString());
    }

    qDebug() << "DirectoryReplicator::loadSourceFolders - loaded"
             << folders.size() << "folders for catalog IDs:" << catalogIds;

    return folders;
}

//----------------------------------------------------------------------
QStringList DirectoryReplicator::toRelativePaths(const QStringList &absolutePaths, const QString &sourcePath)
{
    QStringList relativePaths;

    // Ensure sourcePath ends without trailing slash for clean stripping
    QString prefix = sourcePath;
    if (!prefix.endsWith('/'))
        prefix += '/';

    for (const QString &absPath : absolutePaths) {
        if (absPath.startsWith(prefix)) {
            QString relative = absPath.mid(prefix.length());
            if (!relative.isEmpty())
                relativePaths.append(relative);
        }
        else if (absPath == sourcePath) {
            // The root folder itself — skip, it maps to targetPath directly
            continue;
        }
        else {
            qWarning() << "DirectoryReplicator::toRelativePaths - path does not start with source root:"
                        << absPath << "(source:" << sourcePath << ")";
        }
    }

    return relativePaths;
}

//----------------------------------------------------------------------
QStringList DirectoryReplicator::scanTargetDirectories(const QString &targetPath)
{
    QStringList relativePaths;

    QString prefix = targetPath;
    if (!prefix.endsWith('/'))
        prefix += '/';

    QDirIterator it(targetPath, QDir::Dirs | QDir::NoDotAndDotDot, QDirIterator::Subdirectories);
    while (it.hasNext()) {
        QString dirPath = it.next();
        if (dirPath.startsWith(prefix)) {
            relativePaths.append(dirPath.mid(prefix.length()));
        }
    }

    return relativePaths;
}
//----------------------------------------------------------------------
