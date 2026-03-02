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
// File Name:   backupjob.cpp
// Purpose:     Pre-flight space check for backup/archive operations
// Description: Implements evaluateBackupSpace(), which computes whether the
//              target filesystem has sufficient space before a run begins.
// Author:      Stephane Couturier
/////////////////////////////////////////////////////////////////////////////
*/

#include "backupjob.h"

BackupSpaceCheck evaluateBackupSpace(
    qint64 availableBytes,
    const QList<DifferenceFileEntry> &filesToCopy,
    const QList<DifferenceFileEntry> &fileConflicts,
    ConflictMode conflictMode,
    qint64 lowThreshold)
{
    BackupSpaceCheck result;
    result.available = availableBytes;

    for (const DifferenceFileEntry &e : filesToCopy)
        result.required += e.fileSize;

    // RenameOldest: the old target file is renamed and kept, so the new copy
    // and the renamed copy both consume space simultaneously.
    if (conflictMode == ConflictMode::RenameOldest)
        for (const DifferenceFileEntry &e : fileConflicts)
            result.required += e.fileSize;

    if (availableBytes < 0) {
        result.status = BackupSpaceStatus::Unknown;
        return result;
    }

    if (availableBytes < result.required)
        result.status = BackupSpaceStatus::Insufficient;
    else if (availableBytes - result.required < lowThreshold)
        result.status = BackupSpaceStatus::Low;
    else
        result.status = BackupSpaceStatus::OK;

    return result;
}
