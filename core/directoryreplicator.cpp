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
//              the corresponding directories on the target filesystem.
// Author:      Stephane Couturier
/////////////////////////////////////////////////////////////////////////////
*/

#include "directoryreplicator.h"

#include <QSqlQuery>
#include <QSqlDatabase>
#include <QSqlError>
#include <QDir>
#include <QDirIterator>
#include <QDebug>
#include <QSet>
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
    bool includeEmptyDirs,
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
    if (!includeEmptyDirs)
        absoluteFolders = removeEmptyFolders(catalogIds, absoluteFolders);
    QStringList relativePaths = toRelativePaths(absoluteFolders, sourcePath);


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


    return result;
}

//----------------------------------------------------------------------
ReplicationResult DirectoryReplicator::replicateFromDrive(
    const QString &sourcePath,
    const QString &targetPath,
    bool includeEmptyDirs,
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
        if (relative.isEmpty())
            continue;

        // Drive mode walks the real filesystem, so emptiness is read straight off
        // disk and hidden entries count as entries (SpecBackup.md BKP-F17). A
        // directory holding only a subdirectory is not empty and is still created.
        if (!includeEmptyDirs
            && QDir(absPath).isEmpty(QDir::AllEntries | QDir::NoDotAndDotDot
                                     | QDir::Hidden | QDir::System)) {
            continue;
        }

        relativePaths.append(relative);
    }

    std::sort(relativePaths.begin(), relativePaths.end());


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
QStringList DirectoryReplicator::removeEmptyFolders(const QList<int> &catalogIds,
                                                    const QStringList &absoluteFolders)
{
    if (absoluteFolders.isEmpty())
        return absoluteFolders;

    // A folder holds a subdirectory when it is the parent of another catalogued
    // folder. Taken from the folder list itself rather than by re-querying per
    // folder: the scan records every intermediate directory, so every parent of a
    // catalogued folder is itself catalogued.
    QSet<QString> foldersWithSubdirectory;
    for (const QString &folder : absoluteFolders) {
        const int lastSeparator = folder.lastIndexOf('/');
        if (lastSeparator > 0)
            foldersWithSubdirectory.insert(folder.left(lastSeparator));
    }

    // A folder holds a file when a file row names it as its direct parent.
    QSet<QString> foldersWithFile;
    QSqlQuery query(QSqlDatabase::database(m_connectionName));

    QStringList placeholders;
    for (int i = 0; i < catalogIds.size(); ++i)
        placeholders << QString(":id%1").arg(i);

    QString sql = QString("SELECT DISTINCT file_folder_path FROM file "
                          "WHERE file_catalog_id IN (%1)")
                      .arg(placeholders.join(","));

    query.prepare(sql);
    for (int i = 0; i < catalogIds.size(); ++i)
        query.bindValue(QString(":id%1").arg(i), catalogIds.at(i));

    if (!query.exec()) {
        // Without the file list every folder would read as empty and the whole tree
        // would be dropped. Replicating too much is recoverable, replicating nothing
        // is not, so fall back on the full list (SpecBackup.md BKP-C1).
        qWarning() << "DirectoryReplicator::removeEmptyFolders - query error:"
                   << query.lastError().text();
        return absoluteFolders;
    }

    while (query.next())
        foldersWithFile.insert(query.value(0).toString());

    QStringList nonEmptyFolders;
    for (const QString &folder : absoluteFolders) {
        if (foldersWithFile.contains(folder) || foldersWithSubdirectory.contains(folder))
            nonEmptyFolders.append(folder);
    }

    return nonEmptyFolders;
}
//----------------------------------------------------------------------
