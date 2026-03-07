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
// File Name:   directoryreplicator.h
// Purpose:     Replicate a catalog's directory structure to a target path
// Description: Reads folder paths from the folder table for a set of catalog IDs,
//              computes relative paths, and creates the corresponding directories
//              under a target root. Shared by Backup, Archive, and future features.
// Author:      Stephane Couturier
/////////////////////////////////////////////////////////////////////////////
*/
#ifndef DIRECTORYREPLICATOR_H
#define DIRECTORYREPLICATOR_H

#include <QString>
#include <QStringList>
#include <QList>

struct ReplicationResult {
    QStringList created;        // Directories successfully created on target
    QStringList alreadyExist;   // Directories that already existed on target
    QStringList removed;        // Orphan directories removed from target (when pruning)
    QStringList errors;         // Directories that failed (with error description)

    int createdCount() const   { return created.size(); }
    int skippedCount() const   { return alreadyExist.size(); }
    int removedCount() const   { return removed.size(); }
    int errorCount() const     { return errors.size(); }
    int totalProcessed() const { return created.size() + alreadyExist.size() + errors.size(); }
};

class DirectoryReplicator
{
public:
    explicit DirectoryReplicator(const QString &connectionName);

    /**
     * @brief Replicate directory structure from source catalogs to a target path.
     *
     * Reads all folder paths from the folder table for the given catalog IDs,
     * strips the sourcePath prefix to obtain relative paths, then creates
     * each directory under targetPath.
     *
     * @param catalogIds        Catalog IDs (catalog.ID / folder.folder_catalog_id)
     * @param sourcePath        Source catalog root path (to compute relative paths)
     * @param targetPath        Target root path where directories will be created
     * @param dryRun            If true, compute what would happen without creating anything
     * @return ReplicationResult with created, skipped, removed, and error lists
     */
    ReplicationResult replicate(
        const QList<int> &catalogIds,
        const QString &sourcePath,
        const QString &targetPath,
        bool dryRun = false
    );

    /**
     * @brief Replicate and prune: create missing directories, remove orphans.
     *
     * Same as replicate(), but also scans the target for directories that
     * do not exist in the source catalog and removes them (empty dirs only).
     *
     * @param catalogIds        Catalog IDs
     * @param sourcePath        Source catalog root path
     * @param targetPath        Target root path
     * @param dryRun            If true, compute without modifying filesystem
     * @return ReplicationResult including removed orphan directories
     */
    ReplicationResult replicateAndPrune(
        const QList<int> &catalogIds,
        const QString &sourcePath,
        const QString &targetPath,
        bool dryRun = false
    );

    /**
     * @brief Replicate directory structure by walking the source filesystem directly.
     *
     * Used in Drive source mode: scans the source path on disk instead of reading
     * from the folder table. All directories found under sourcePath are created
     * under targetPath at the same relative location. The source device must be
     * connected and mounted.
     *
     * @param sourcePath        Source root path to walk
     * @param targetPath        Target root path where directories will be created
     * @param dryRun            If true, compute without creating anything
     * @return ReplicationResult with created, skipped, and error lists
     */
    ReplicationResult replicateFromDrive(
        const QString &sourcePath,
        const QString &targetPath,
        bool dryRun = false
    );

private:
    QString m_connectionName;

    /**
     * @brief Load all folder paths from the folder table for given catalog IDs.
     * @return List of absolute folder paths from the database.
     */
    QStringList loadSourceFolders(const QList<int> &catalogIds);

    /**
     * @brief Convert absolute folder paths to relative paths by stripping the source root.
     * @return List of relative paths (e.g. "Photos/2024" from "/mnt/drive/Photos/2024").
     */
    static QStringList toRelativePaths(const QStringList &absolutePaths, const QString &sourcePath);

    /**
     * @brief Scan the target directory recursively and return all subdirectory relative paths.
     */
    static QStringList scanTargetDirectories(const QString &targetPath);
};

#endif // DIRECTORYREPLICATOR_H
