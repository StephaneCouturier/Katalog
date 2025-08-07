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
// File Name:   catalogmanager.h
// Purpose:     Class/model to manage catalog operations using KJob framework
// Description: Manages catalog creation/update with progress reporting and cancellation
// Author:      Stephane Couturier
/////////////////////////////////////////////////////////////////////////////
*/
#ifndef CATALOGMANAGER_H
#define CATALOGMANAGER_H

#pragma once

#include <QObject>
#include <QTimer>
#include "catalogjob.h"

class Catalog;

/**
 * @brief The CatalogManager class
 * Manages catalog creation/update operations using KJob framework
 * Provides a clean interface for the UI to interact with catalog jobs
 * Similar to SearchManager but for catalog operations
 */
class CatalogManager : public QObject
{
    Q_OBJECT

    Q_PROPERTY(bool catalogOperationRunning READ catalogOperationRunning NOTIFY catalogOperationRunningChanged)
    Q_PROPERTY(int progress READ progress NOTIFY progressChanged)
    Q_PROPERTY(QString status READ status NOTIFY statusChanged)
    Q_PROPERTY(QString currentPath READ currentPath NOTIFY currentPathChanged)
    Q_PROPERTY(qint64 filesProcessed READ filesProcessed NOTIFY filesProcessedChanged)
    Q_PROPERTY(qint64 totalFiles READ totalFiles NOTIFY totalFilesChanged)

public:
    explicit CatalogManager(QObject *parent = nullptr);
    ~CatalogManager();

    // Properties
    bool catalogOperationRunning() const { return m_catalogOperationRunning; }
    int progress() const { return m_progress; }
    QString status() const { return m_status; }
    QString currentPath() const { return m_currentPath; }
    qint64 filesProcessed() const { return m_filesProcessed; }
    qint64 totalFiles() const { return m_totalFiles; }

    // Get the current catalog being processed
    Catalog* getCurrentCatalog() const;
    Device* getCurrentDevice() const;

public slots:
    // Start catalog operations
    //void startCreateCatalog(Catalog *catalog, const QString &databaseMode, const QString &collectionFolder);
    //void startUpdateCatalog(Catalog *catalog, const QString &databaseMode, const QString &collectionFolder);
    void startCreateCatalog(Device *device, const QString &databaseMode, const QString &collectionFolder);
    void startUpdateCatalog(Device *device, const QString &databaseMode, const QString &collectionFolder);

    // Control operations (placeholder for future stoppable implementation)
    void stopOperation();
    void pauseOperation();
    void resumeOperation();

private slots:
    void onJobResult(KJob *job);
    void onJobPercent();
    void onJobInfoMessage(KJob *job, const QString &message);
    void onFilesProcessedUpdate(qint64 processed, qint64 total, const QString &currentPath);

private:
    void setCatalogOperationRunning(bool running);
    void setProgress(int progress);
    void setStatus(const QString &status);
    void setCurrentPath(const QString &path);
    void setFilesProcessed(qint64 processed);
    void setTotalFiles(qint64 total);
    void cleanupJob();

    CatalogJob *m_currentJob = nullptr;
    bool m_catalogOperationRunning = false;
    int m_progress = 0;
    QString m_status = "Ready";
    QString m_currentPath;
    qint64 m_filesProcessed = 0;
    qint64 m_totalFiles = 0;
    bool m_isPaused = false;

signals:
    void catalogOperationRunningChanged();
    void progressChanged();
    void statusChanged();
    void currentPathChanged();
    void filesProcessedChanged();
    void totalFilesChanged();
    
    void catalogOperationCompleted();
    void catalogOperationCancelled();
    void catalogOperationError(const QString &error);

    // Special progress update for status bar
    void progressUpdate(qint64 filesProcessed, qint64 totalFiles, int progressPercent, const QString &currentPath);
};

#endif // CATALOGMANAGER_H
