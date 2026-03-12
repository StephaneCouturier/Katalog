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

#include <QDateTime>
#include <QDir>
#include <QFile>
#include <QFileInfo>
#include <QThread>
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
void BackupJobStoppable::setConflictFiles(const QList<DifferenceFileEntry> &conflicts)
{
    m_conflictFiles = conflicts;
}

//----------------------------------------------------------------------
void BackupJobStoppable::setConflictMode(ConflictMode mode)
{
    m_conflictMode = mode;
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
void BackupJobStoppable::setArchiveMode(bool archive)
{
    m_archiveMode = archive;
}

//----------------------------------------------------------------------
QString BackupJobStoppable::buildArchivedFileName(const QString &filePath)
{
    const QFileInfo fi(filePath);
    const QString   stamp = QDateTime::currentDateTime().toString("yyyyMMdd-HHmmss");
    const QString   ext   = fi.suffix();
    const QString   stem  = fi.completeBaseName();
    const QString   name  = stem + QLatin1Char('_') + stamp
                            + (ext.isEmpty() ? QString() : QLatin1Char('.') + ext);
    return fi.dir().filePath(name);
}

//----------------------------------------------------------------------
bool BackupJobStoppable::copyFileChunked(const QString &source, const QString &target,
                                         qint64 fileSize,
                                         int filesDone, int totalFiles,
                                         qint64 &bytesCopied, qint64 totalBytes,
                                         const QString &fileName)
{
    static constexpr qint64 kChunkSize = 1 * 1024 * 1024; // 1 MB

    QFile src(source);
    if (!src.open(QIODevice::ReadOnly)) {
        qWarning() << "BackupJobStoppable::copyFileChunked - cannot open source:" << source;
        return false;
    }

    QFile dst(target);
    if (!dst.open(QIODevice::WriteOnly | QIODevice::Truncate)) {
        qWarning() << "BackupJobStoppable::copyFileChunked - cannot open target:" << target;
        return false;
    }

    QByteArray buffer(kChunkSize, Qt::Uninitialized);
    qint64 bytesWrittenForFile = 0;

    while (!src.atEnd()) {
        waitIfPaused();
        if (!shouldContinue()) {
            dst.close();
            QFile::remove(target);
            return false;
        }

        const qint64 bytesRead = src.read(buffer.data(), kChunkSize);
        if (bytesRead < 0) {
            qWarning() << "BackupJobStoppable::copyFileChunked - read error:" << source;
            dst.close();
            QFile::remove(target);
            return false;
        }

        const qint64 bytesWritten = dst.write(buffer.constData(), bytesRead);
        if (bytesWritten != bytesRead) {
            qWarning() << "BackupJobStoppable::copyFileChunked - write error:" << target;
            dst.close();
            QFile::remove(target);
            return false;
        }

        bytesWrittenForFile += bytesWritten;
        bytesCopied         += bytesWritten;
        emit backupProgress(filesDone, totalFiles, bytesCopied, totalBytes, fileName);
    }

    // Guard: if catalog size is stale this may differ, but the copy itself succeeded
    if (fileSize > 0 && bytesWrittenForFile != fileSize) {
        qWarning() << "BackupJobStoppable::copyFileChunked - size mismatch for" << source
                   << "expected" << fileSize << "got" << bytesWrittenForFile;
    }

    return true;
}

//----------------------------------------------------------------------
void BackupJobStoppable::stopBackup()
{
    m_stopRequested.storeRelease(1);
}

//----------------------------------------------------------------------
void BackupJobStoppable::pauseBackup()
{
    m_paused.storeRelease(1);
}

//----------------------------------------------------------------------
void BackupJobStoppable::resumeBackup()
{
    m_paused.storeRelease(0);
}

//----------------------------------------------------------------------
void BackupJobStoppable::waitIfPaused()
{
    while (m_paused.loadAcquire() && shouldContinue()) {
        QMutexLocker locker(&m_pauseMutex);
        QThread::msleep(100);
    }
}

//----------------------------------------------------------------------
void BackupJobStoppable::runBackup()
{
    BackupReport report;

    // In RenameOldest mode the conflict files are also processed, so include them
    // in the total count and byte estimate for accurate progress reporting.
    const int conflictsToProcess =
        (m_conflictMode == ConflictMode::RenameOldest) ? m_conflictFiles.size() : 0;
    const int totalFiles = m_files.size() + conflictsToProcess;

    // Pre-compute total bytes for the progress signal
    qint64 totalBytes = 0;
    for (const DifferenceFileEntry &e : m_files)
        totalBytes += e.fileSize;
    if (m_conflictMode == ConflictMode::RenameOldest) {
        for (const DifferenceFileEntry &e : m_conflictFiles)
            totalBytes += e.fileSize;
    }

    int    filesDone   = 0;
    qint64 bytesCopied = 0;

    // ── Phase 1: copy new files (no counterpart in target) ───────────────────
    for (const DifferenceFileEntry &entry : m_files) {
        if (!shouldContinue()) {
            report.wasCancelled = true;
            break;
        }
        waitIfPaused();
        if (!shouldContinue()) {
            report.wasCancelled = true;
            break;
        }

        emit backupProgress(filesDone, totalFiles, bytesCopied, totalBytes, entry.fileName);

        // Compute source and target paths.
        // entry.folderPath is the absolute folder in the SOURCE catalog
        //   e.g.  /media/USB/Photos/2024
        // m_sourcePath is the catalog root
        //   e.g.  /media/USB
        // relative folder: /Photos/2024
        const QString relFolder    = entry.folderPath.mid(m_sourcePath.length());
        const QString targetFolder = m_targetPath + relFolder;
        const QString sourceFile   = QDir(entry.folderPath).filePath(entry.fileName);
        const QString targetFile   = QDir(targetFolder).filePath(entry.fileName);

        // Unexpected conflict: target file already exists despite catalog check
        if (QFileInfo::exists(targetFile)) {
            report.conflicts.append(entry);
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

        // Move or copy the file
        if (m_archiveMode) {
            // Try atomic rename first (same filesystem: instant, no data copied)
            if (QFile::rename(sourceFile, targetFile)) {
                report.totalBytesCopied += entry.fileSize;
                bytesCopied += entry.fileSize;
                report.moved.append(entry);
            } else if (copyFileChunked(sourceFile, targetFile, entry.fileSize,
                                       filesDone, totalFiles, bytesCopied, totalBytes,
                                       entry.fileName)) {
                // Cross-filesystem fallback: copy then delete source
                report.totalBytesCopied += entry.fileSize;
                // bytesCopied already updated chunk-by-chunk inside copyFileChunked
                if (!QFile::remove(sourceFile))
                    report.errors.append(sourceFile + ": copied to target but source delete failed");
                report.moved.append(entry);
            } else {
                if (!shouldContinue()) {
                    report.wasCancelled = true;
                    break;
                }
                const QString msg = sourceFile + ": move failed";
                report.errors.append(msg);
                qWarning() << "BackupJobStoppable:" << msg;
            }
        } else {
            if (copyFileChunked(sourceFile, targetFile, entry.fileSize,
                                filesDone, totalFiles, bytesCopied, totalBytes,
                                entry.fileName)) {
                report.totalBytesCopied += entry.fileSize;
                // bytesCopied already updated chunk-by-chunk inside copyFileChunked
                report.copied.append(entry);
            } else {
                if (!shouldContinue()) {
                    report.wasCancelled = true;
                    break;
                }
                const QString msg = sourceFile + ": copy failed";
                report.errors.append(msg);
                qWarning() << "BackupJobStoppable:" << msg;
            }
        }

        ++filesDone;
    }

    // ── Phase 2: handle conflict files (RenameOldest mode only) ─────────────
    if (!report.wasCancelled && m_conflictMode == ConflictMode::RenameOldest) {
        for (const DifferenceFileEntry &entry : m_conflictFiles) {
            if (!shouldContinue()) {
                report.wasCancelled = true;
                break;
            }
            waitIfPaused();
            if (!shouldContinue()) {
                report.wasCancelled = true;
                break;
            }

            emit backupProgress(filesDone, totalFiles, bytesCopied, totalBytes, entry.fileName);

            const QString relFolder    = entry.folderPath.mid(m_sourcePath.length());
            const QString targetFolder = m_targetPath + relFolder;
            const QString sourceFile   = QDir(entry.folderPath).filePath(entry.fileName);
            const QString targetFile   = QDir(targetFolder).filePath(entry.fileName);

            // Use filesystem dates for the direction check (more reliable than catalog dates).
            const QFileInfo srcInfo(sourceFile);
            const QFileInfo tgtInfo(targetFile);

            if (!srcInfo.exists()) {
                report.errors.append(sourceFile + ": source file not found");
                ++filesDone;
                continue;
            }

            if (!tgtInfo.exists() || srcInfo.lastModified() <= tgtInfo.lastModified()) {
                // Target is newer or same age — do not overwrite; report as conflict.
                report.conflicts.append(entry);
                ++filesDone;
                continue;
            }

            // Source is newer: archive the old target file, then copy the new one.
            const QString archivedFile = buildArchivedFileName(targetFile);
            if (!QFile::rename(targetFile, archivedFile)) {
                const QString msg = targetFile + ": failed to rename for archiving";
                report.errors.append(msg);
                qWarning() << "BackupJobStoppable:" << msg;
                ++filesDone;
                continue;
            }

            bool transferOk = false;
            if (m_archiveMode) {
                // Try atomic rename first (same filesystem: instant, no data copied)
                if (QFile::rename(sourceFile, targetFile)) {
                    transferOk = true;
                    bytesCopied += entry.fileSize; // rename is instant; account bytes here
                } else if (copyFileChunked(sourceFile, targetFile, entry.fileSize,
                                           filesDone, totalFiles, bytesCopied, totalBytes,
                                           entry.fileName)) {
                    // Cross-filesystem fallback: copy then delete source
                    // bytesCopied already updated chunk-by-chunk inside copyFileChunked
                    transferOk = true;
                    if (!QFile::remove(sourceFile))
                        report.errors.append(sourceFile + ": moved (archived+replaced) but source delete failed");
                }
            } else {
                if (copyFileChunked(sourceFile, targetFile, entry.fileSize,
                                    filesDone, totalFiles, bytesCopied, totalBytes,
                                    entry.fileName)) {
                    // bytesCopied already updated chunk-by-chunk inside copyFileChunked
                    transferOk = true;
                }
            }

            if (transferOk) {
                report.renamed.append(entry);
                report.totalBytesCopied += entry.fileSize;
                // bytesCopied already updated above (by rename or copyFileChunked)
            } else {
                // Restore the archived file to avoid data loss.
                QFile::rename(archivedFile, targetFile);
                if (!shouldContinue()) {
                    report.wasCancelled = true;
                    break;
                }
                const QString msg = sourceFile + ": move/copy failed after archiving (restored original)";
                report.errors.append(msg);
                qWarning() << "BackupJobStoppable:" << msg;
            }

            ++filesDone;
        }
    }

    // Final progress tick (100 %)
    emit backupProgress(filesDone, totalFiles, bytesCopied, totalBytes, QString());
    emit backupFinished(report);
}
