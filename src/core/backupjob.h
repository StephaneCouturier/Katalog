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
// File Name:   backupjob.h
// Purpose:     Result structures for backup operations
// Description: Defines BackupReport, the output of BackupJobStoppable,
//              mirroring the categories shown in the Backup Preview.
// Author:      Stephane Couturier
/////////////////////////////////////////////////////////////////////////////
*/
#ifndef BACKUPJOB_H
#define BACKUPJOB_H

#include "catalogdifferenceengine.h"
#include <QStringList>

/**
 * @brief How the backup executor handles files that already exist in the target
 *        but differ from the source (conflicts).
 *
 * Stored as TEXT in device_mapping.mapping_conflict_mode (migration 2.12).
 * String values: "Skip", "RenameOldest" — same pattern as device_type.
 *
 * Only Skip and RenameOldest are implemented (v1).
 * Overwrite and RenameAlways are defined for the backlog and must not be used yet.
 */
enum class ConflictMode {
    Skip          = 0,  ///< Report the conflict, leave target untouched (safe default).
    RenameOldest  = 1,  ///< If source is newer: rename the target file with a datetime
                        ///< stamp (stem_YYYYMMDD-HHmmss.ext), then copy the source.
                        ///< If target is newer or same date: skip (protect newer target).
    // Overwrite   = 2,  ///< (backlog) Source always wins — overwrite target silently, no rename.
    // RenameAlways= 3,  ///< (backlog) Always rename target + copy source, regardless of date.
};

inline QString conflictModeToString(ConflictMode mode)
{
    switch (mode) {
    case ConflictMode::RenameOldest: return QStringLiteral("RenameOldest");
    case ConflictMode::Skip:
    default:                         return QStringLiteral("Skip");
    }
}

inline ConflictMode conflictModeFromString(const QString &s)
{
    if (s == QLatin1String("RenameOldest")) return ConflictMode::RenameOldest;
    return ConflictMode::Skip;
}

/**
 * @brief Report produced after a backup run.
 *
 * Categories mirror the Backup Preview:
 *   copied    — files successfully copied from source to target (new files)
 *   renamed   — target renamed as stem_YYYYMMDD-HHmmss.ext, source copied (RenameOldest mode)
 *   conflicts — files that exist in target but differ and were NOT handled; skipped
 *   errors    — files that could not be copied (I/O error, permission, rename failure, …)
 *
 * "Already in target" (skipped) is not re-listed here; the count can be
 * derived as (total source files) - copied - renamed - conflicts - errors.
 */
struct BackupReport {
    QList<DifferenceFileEntry> copied;
    QList<DifferenceFileEntry> renamed;     // archived old target + replaced with source
    QList<DifferenceFileEntry> conflicts;   // exist in target but differ — skipped
    QStringList                errors;      // "<path>: <reason>"
    qint64                     totalBytesCopied = 0;
    bool                       wasCancelled     = false;

    int copiedCount()    const { return copied.size();    }
    int renamedCount()   const { return renamed.size();   }
    int conflictCount()  const { return conflicts.size(); }
    int errorCount()     const { return errors.size();    }
};

#endif // BACKUPJOB_H
