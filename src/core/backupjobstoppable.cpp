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
// File Name:   backupjobstoppable.cpp
// Purpose:     Worker class for executing a backup operation
// Description: Copies files from source to target with progress reporting
//              and cancellation support.
// Author:      Stephane Couturier
/////////////////////////////////////////////////////////////////////////////
*/

#include "backupjobstoppable.h"

#include <QDir>
#include <QFile>
#include <QFileInfo>
#include <QDebug>

//----------------------------------------------------------------------
BackupJobStoppable::BackupJobStoppable(QObject *parent)
    : QObject(parent)
{
}

//----------------------------------------------------------------------
void BackupJobStoppable::setFiles(const QList<DifferenceFileEntry> &files)
{
    m_files = files;
}

//----------------------------------------------------------------------
void BackupJobStoppable::setSourcePath(const QString &path)
{
    // Normalise: remove trailing slash so mid() arithmetic is consistent
    m_sourcePath = path.endsWith('/') ? path.chopped(1) : path;
}

//----------------------------------------------------------------------
void BackupJobStoppable::setTargetPath(const QString &path)
{
    m_targetPath = path.endsWith('/') ? path.chopped(1) : path;
}

//----------------------------------------------------------------------
void BackupJobStoppable::stopBackup()
{
    m_stopRequested.storeRelease(1);
}

//----------------------------------------------------------------------
void BackupJobStoppable::runBackup()
{
    BackupReport report;

    const int totalFiles = m_files.size();

    // Pre-compute total bytes for the progress signal
    qint64 totalBytes = 0;
    for (const DifferenceFileEntry &e : m_files)
        totalBytes += e.fileSize;

    int  filesDone  = 0;
    qint64 bytesCopied = 0;

    for (const DifferenceFileEntry &entry : m_files) {
        if (!shouldContinue()) {
            report.wasCancelled = true;
            break;
        }

        emit backupProgress(filesDone, totalFiles, bytesCopied, totalBytes, entry.fileName);

        // Compute source and target paths
        // entry.folderPath is the absolute folder in the SOURCE catalog
        //   e.g.  /media/USB/Photos/2024
        // m_sourcePath is the catalog root
        //   e.g.  /media/USB
        // relative folder: /Photos/2024
        const QString relFolder  = entry.folderPath.mid(m_sourcePath.length());
        const QString targetFolder = m_targetPath + relFolder;
        const QString sourceFile   = QDir(entry.folderPath).filePath(entry.fileName);
        const QString targetFile   = QDir(targetFolder).filePath(entry.fileName);

        // Conflict: target file already exists (different content — we never overwrite in v1)
        if (QFileInfo::exists(targetFile)) {
            report.conflicts.append(entry);
            qDebug() << "BackupJobStoppable: conflict (target exists):" << targetFile;
            ++filesDone;
            continue;
        }

        // Create target directory tree if needed
        if (!QDir().mkpath(targetFolder)) {
            const QString msg = sourceFile + ": failed to create target directory " + targetFolder;
            report.errors.append(msg);
            qWarning() << "BackupJobStoppable:" << msg;
            ++filesDone;
            continue;
        }

        // Copy the file
        if (QFile::copy(sourceFile, targetFile)) {
            report.copied.append(entry);
            report.totalBytesCopied += entry.fileSize;
            bytesCopied += entry.fileSize;
            qDebug() << "BackupJobStoppable: copied" << sourceFile << "->" << targetFile;
        } else {
            const QString msg = sourceFile + ": copy failed";
            report.errors.append(msg);
            qWarning() << "BackupJobStoppable:" << msg;
        }

        ++filesDone;
    }

    // Final progress tick (100 %)
    emit backupProgress(filesDone, totalFiles, bytesCopied, totalBytes, QString());
    emit backupFinished(report);
}
