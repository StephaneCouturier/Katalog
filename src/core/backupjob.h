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
 * @brief Report produced after a backup run.
 *
 * Categories mirror the Backup Preview:
 *   copied    — files successfully copied from source to target
 *   conflicts — files that exist in target but differ; not overwritten (v1)
 *   errors    — files that could not be copied (I/O error, permission, …)
 *
 * "Already in target" (skipped) is not re-listed here; the count can be
 * derived as (total source files) - copied - conflicts - errors.
 */
struct BackupReport {
    QList<DifferenceFileEntry> copied;
    QList<DifferenceFileEntry> conflicts;   // exist in target but differ — skipped
    QStringList                errors;      // "<path>: <reason>"
    qint64                     totalBytesCopied = 0;
    bool                       wasCancelled     = false;

    int copiedCount()    const { return copied.size();    }
    int conflictCount()  const { return conflicts.size(); }
    int errorCount()     const { return errors.size();    }
};

#endif // BACKUPJOB_H
