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
// File Name:   backupjobstoppable.h
// Purpose:     Worker class for executing a backup operation
// Description: Copies files from source to target with progress reporting
//              and cancellation support. Runs in a QThread.
//              Follows the CatalogJobStoppable pattern.
// Author:      Stephane Couturier
/////////////////////////////////////////////////////////////////////////////
*/
#ifndef BACKUPJOBSTOPPABLE_H
#define BACKUPJOBSTOPPABLE_H

#include "backupjob.h"

#include <QObject>
#include <QAtomicInt>
#include <QMutex>
#include <QString>

/**
 * @brief Executes a backup by copying a list of files from source to target.
 *
 * Usage:
 *   auto *job = new BackupJobStoppable();
 *   job->setFiles(filesToCopy);
 *   job->setConflictMode(mapping.conflictMode);
 *   if (mapping.conflictMode == ConflictMode::KeepBoth)
 *       job->setConflictFiles(fileConflicts);
 *   job->setSourcePath(sourceDevice.path);
 *   job->setTargetPath(targetDevice.path);
 *
 *   QThread *thread = new QThread();
 *   job->moveToThread(thread);
 *   connect(thread, &QThread::started,  job,    &BackupJobStoppable::runBackup);
 *   connect(job,    &BackupJobStoppable::backupFinished, thread, &QThread::quit);
 *   connect(thread, &QThread::finished, job,    &QObject::deleteLater);
 *   connect(thread, &QThread::finished, thread, &QObject::deleteLater);
 *   thread->start();
 *
 * To cancel:  job->stopBackup();
 */
class BackupJobStoppable : public QObject
{
    Q_OBJECT

public:
    explicit BackupJobStoppable(QObject *parent = nullptr);

    /** New files to copy (no counterpart in target). */
    void setFiles(const QList<DifferenceFileEntry> &files);

    /**
     * @brief Conflict files (exist in target but differ).
     * Only processed when conflictMode is KeepBoth.
     * In KeepBoth mode: if source is newer, the target is archived and the
     * source is copied; otherwise the file is reported as a skipped conflict.
     */
    void setConflictFiles(const QList<DifferenceFileEntry> &conflicts);

    /** How to handle files that already exist in the target but differ. */
    void setConflictMode(ConflictMode mode);

    /** Absolute path of the source catalog root (e.g. "/media/USB_Drive"). */
    void setSourcePath(const QString &path);

    /** Absolute path of the target catalog root (e.g. "/media/Backup"). */
    void setTargetPath(const QString &path);

    /** Request graceful cancellation — checked between files. */
    void stopBackup();

    /** Suspend execution after the current file finishes. */
    void pauseBackup();

    /** Resume a paused backup. */
    void resumeBackup();

    bool wasStopRequested() const { return m_stopRequested.loadAcquire() != 0; }
    bool isPaused()         const { return m_paused.loadAcquire() != 0; }

public slots:
    /** Main blocking method — call from a worker thread. */
    void runBackup();

signals:
    /**
     * @brief Emitted after each file is processed (copied, conflicted, or errored).
     * @param filesDone  Number of files processed so far (for progress bar)
     * @param totalFiles Total number of files to process
     * @param bytesCopied Bytes successfully copied so far
     * @param totalBytes  Total bytes in the to-copy list
     * @param currentFile File name currently being processed (empty when done)
     */
    void backupProgress(int filesDone, int totalFiles,
                        qint64 bytesCopied, qint64 totalBytes,
                        const QString &currentFile);

    /** Emitted once when the backup finishes (cancelled or complete). */
    void backupFinished(const BackupReport &report);

private:
    QList<DifferenceFileEntry> m_files;
    QList<DifferenceFileEntry> m_conflictFiles;
    QString        m_sourcePath;
    QString        m_targetPath;
    ConflictMode   m_conflictMode = ConflictMode::Skip;
    QAtomicInt     m_stopRequested{0};
    QAtomicInt     m_paused{0};
    mutable QMutex m_pauseMutex;

    bool shouldContinue() const { return m_stopRequested.loadAcquire() == 0; }

    /** Blocks the calling thread while paused (and not stopped). */
    void waitIfPaused();

    /** Build the archived filename: stem_YYYYMMDD-HHmmss.ext */
    static QString buildArchivedFileName(const QString &filePath);
};

#endif // BACKUPJOBSTOPPABLE_H
