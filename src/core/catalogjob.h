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
// File Name:   catalogjob.h
// Purpose:     KJob wrapper for catalog operations (follows SearchJob pattern exactly)
// Description: Simple KJob implementation that wraps CatalogJobStoppable
// Author:      Stephane Couturier
/////////////////////////////////////////////////////////////////////////////
*/
#ifndef CATALOGJOB_H
#define CATALOGJOB_H

#pragma once

#include <KJob>
#include <QTimer>
#include <QMutex>
#include "catalogjobstoppable.h"
#include "device.h"

/**
 * @brief The CatalogJob class
 * KJob implementation for catalog operations
 * Follows the exact same pattern as SearchJob for consistency and reliability
 */
class CatalogJob : public KJob
{
    Q_OBJECT

public:
    explicit CatalogJob(QObject *parent = nullptr);

    // Configure the catalog job
    void setCatalogJobStoppable(CatalogJobStoppable *catalogEngine);
    void setTargetDevice(Device *device);
    void setOperationType(CatalogJobStoppable::OperationType operationType);
    void setDatabaseMode(const QString &databaseMode);
    void setCollectionFolder(const QString &collectionFolder);

    // Public method to start the job
    void startJob();

    // Get the catalog engine
    CatalogJobStoppable* getCatalogEngine() const { return m_catalogEngine; }

    // Get the target device
    Device* getTargetDevice() const { return m_targetDevice; }

    // Get operation type
    CatalogJobStoppable::OperationType getOperationType() const { return m_operationType; }

private:
    CatalogJobStoppable *m_catalogEngine = nullptr;
    Device *m_targetDevice = nullptr;
    CatalogJobStoppable::OperationType m_operationType = CatalogJobStoppable::CreateCatalog;
    QString m_databaseMode;
    QString m_collectionFolder;
    QTimer *m_executeTimer = nullptr;
    QMutex m_mutex;
    bool m_suspended = false;
    qint64 m_lastFilesProcessed = 0;

protected:
    void start() override;
    bool doKill() override;
    bool doSuspend() override;
    bool doResume() override;

private slots:
    void onCatalogProgress(qint64 filesProcessed, qint64 totalFiles, const QString &currentPath);
    void executeCatalogOperation();

signals:
    void catalogStarted();
    void catalogFinished();
    void catalogProgress(qint64 filesProcessed, qint64 totalFiles, const QString &currentPath);
};

#endif // CATALOGJOB_H
