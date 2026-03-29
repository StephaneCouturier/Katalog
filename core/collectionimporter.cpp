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
// File Name:   collectionimporter.cpp
// Purpose:     Import / Merge / Update across Collections (#750)
// Description: Phase 1 — device/catalog hierarchy and file lists
// Author:      Stephane Couturier
/////////////////////////////////////////////////////////////////////////////
*/
#include "collectionimporter.h"
#include "database.h"
#include "catalog.h"

#include <QFileInfo>
#include <QFile>
#include <QDir>
#include <QSet>
#include <QSqlDatabase>
#include <QSqlQuery>
#include <QSqlError>
#include <QDebug>
#include <QMutex>

static const QString SOURCE_CONN = "importSourceConnection";

//----------------------------------------------------------------------
CollectionImporter::CollectionImporter(Collection *targetCollection, QObject *parent)
    : QObject(parent)
    , m_target(targetCollection)
    , m_sourceConnectionName(SOURCE_CONN)
{
}

CollectionImporter::~CollectionImporter()
{
    close();
}

//----------------------------------------------------------------------
// Source connection
//----------------------------------------------------------------------
QSqlError CollectionImporter::openSource(const QString &sourcePath)
{
    close();  // Close any previously open source

    m_sourcePath = sourcePath;
    QFileInfo info(sourcePath);

    if (info.isFile()) {
        m_sourceMode = "File";
        QSqlDatabase db = QSqlDatabase::addDatabase("QSQLITE", m_sourceConnectionName);
        db.setDatabaseName(sourcePath);
        if (!db.open()) {
            m_lastError = db.lastError().text();
            return db.lastError();
        }
    } else if (info.isDir()) {
        m_sourceMode = "Memory";

        // Create in-memory SQLite for the source
        QSqlDatabase db = QSqlDatabase::addDatabase("QSQLITE", m_sourceConnectionName);
        db.setDatabaseName(":memory:");
        if (!db.open()) {
            m_lastError = db.lastError().text();
            return db.lastError();
        }

        // Create the schema in the source connection
        QSqlError err = Database::createAllTables(m_sourceConnectionName);
        if (err.type() != QSqlError::NoError) {
            m_lastError = err.text();
            return err;
        }

        // Use a temporary Collection to load the CSV files into the source connection
        m_sourceCollection = new Collection(this);
        m_sourceCollection->databaseMode = "Memory";
        m_sourceCollection->folder = sourcePath;
        m_sourceCollection->generateCollectionFilesPaths();
        m_sourceCollection->setConnectionName(m_sourceConnectionName);
        m_sourceCollection->loadParameterFileToTable();
        m_sourceCollection->loadDeviceFileToTable();
        m_sourceCollection->loadCatalogFilesToTable();
        m_sourceCollection->loadCatalogFilterFileToTable();
        m_sourceCollection->loadStorageFileToTable();
        m_sourceCollection->loadStatisticsDeviceFileToTable();
        m_sourceCollection->loadTagFileToTable();
        m_sourceCollection->loadMappingFileToTable();
        m_sourceCollection->loadImageFolderPath();
    } else {
        m_lastError = QString("Path does not exist: %1").arg(sourcePath);
        return QSqlError("path", m_lastError, QSqlError::ConnectionError);
    }

    return QSqlError();  // success
}

void CollectionImporter::close()
{
    if (QSqlDatabase::contains(m_sourceConnectionName)) {
        QSqlDatabase::database(m_sourceConnectionName).close();
        QSqlDatabase::removeDatabase(m_sourceConnectionName);
    }
    delete m_sourceCollection;
    m_sourceCollection = nullptr;
    m_sourcePath.clear();
    m_sourceMode.clear();
    m_deviceIdMap.clear();
    m_catalogIdMap.clear();
    m_storageIdOffset = 0;
}

bool CollectionImporter::isSourceOpen() const
{
    return QSqlDatabase::contains(m_sourceConnectionName)
           && QSqlDatabase::database(m_sourceConnectionName).isOpen();
}

//----------------------------------------------------------------------
// Schema check
//----------------------------------------------------------------------
bool CollectionImporter::checkSchemaCompatibility()
{
    auto readVersion = [](const QString &conn) -> QString {
        QSqlQuery q(QSqlDatabase::database(conn));
        q.prepare(QLatin1String(R"(
            SELECT parameter_value1 FROM parameter
            WHERE parameter_type = 'collection' AND parameter_name = 'version'
        )"));
        q.exec();
        return q.next() ? q.value(0).toString() : QString();
    };

    QString srcVersion = readVersion(m_sourceConnectionName);
    QString tgtVersion = readVersion(m_target->connectionName());

    if (srcVersion.isEmpty() || tgtVersion.isEmpty() || srcVersion != tgtVersion) {
        m_lastError = QString("Schema version mismatch: source=%1, target=%2")
                          .arg(srcVersion, tgtVersion);
        return false;
    }
    return true;
}

//----------------------------------------------------------------------
// ID offsets
//----------------------------------------------------------------------
void CollectionImporter::buildIdOffsets()
{
    QString tgtConn = m_target->connectionName();

    {
        QSqlQuery q(QSqlDatabase::database(tgtConn));
        q.exec("SELECT MAX(device_id) FROM device");
        m_deviceIdOffset = q.next() ? q.value(0).toInt() + 1 : 1;
    }
    {
        QSqlQuery q(QSqlDatabase::database(tgtConn));
        q.exec("SELECT MAX(catalog_id) FROM catalog");
        m_catalogIdOffset = q.next() ? q.value(0).toInt() + 1 : 1;
    }
    {
        QSqlQuery q(QSqlDatabase::database(tgtConn));
        q.exec("SELECT MAX(storage_id) FROM storage");
        m_storageIdOffset = q.next() ? q.value(0).toInt() + 1 : 1;
    }
}

//----------------------------------------------------------------------
// Source device helpers
//----------------------------------------------------------------------
QString CollectionImporter::srcDeviceName(int srcDeviceId) const
{
    QSqlQuery q(QSqlDatabase::database(m_sourceConnectionName));
    q.prepare("SELECT device_name FROM device WHERE device_id = :id");
    q.bindValue(":id", srcDeviceId);
    q.exec();
    return q.next() ? q.value(0).toString() : QString();
}

QString CollectionImporter::srcDeviceType(int srcDeviceId) const
{
    QSqlQuery q(QSqlDatabase::database(m_sourceConnectionName));
    q.prepare("SELECT device_type FROM device WHERE device_id = :id");
    q.bindValue(":id", srcDeviceId);
    q.exec();
    return q.next() ? q.value(0).toString() : QString();
}

int CollectionImporter::srcParentId(int srcDeviceId) const
{
    QSqlQuery q(QSqlDatabase::database(m_sourceConnectionName));
    q.prepare("SELECT device_parent_id FROM device WHERE device_id = :id");
    q.bindValue(":id", srcDeviceId);
    q.exec();
    return q.next() ? q.value(0).toInt() : 0;
}

QList<int> CollectionImporter::sourceRootDeviceIds() const
{
    QList<int> ids;
    QSqlQuery q(QSqlDatabase::database(m_sourceConnectionName));
    q.exec(QLatin1String(R"(
        SELECT device_id FROM device
        WHERE device_parent_id IS NULL OR device_parent_id = 0
        ORDER BY device_order, device_id
    )"));
    while (q.next())
        ids << q.value(0).toInt();
    return ids;
}

//----------------------------------------------------------------------
// Ancestor chain helpers
//----------------------------------------------------------------------
QList<int> CollectionImporter::getAncestorChain(int srcDeviceId) const
{
    // Returns ancestors from root down to the direct parent (exclusive of srcDeviceId).
    QList<int> chain;
    int cur = srcParentId(srcDeviceId);
    while (cur != 0) {
        chain.prepend(cur);
        cur = srcParentId(cur);
    }
    return chain;
}

int CollectionImporter::ensureAncestors(int srcDeviceId)
{
    // Returns the target parent_id that srcDeviceId should use (0 = at root).
    QList<int> ancestors = getAncestorChain(srcDeviceId);
    if (ancestors.isEmpty())
        return 0;  // srcDeviceId is itself a root

    int targetParentId = 0;
    QString tgtConn = m_target->connectionName();

    for (int srcAncestorId : ancestors) {
        // Already mapped from a previous import in this session?
        if (m_deviceIdMap.contains(srcAncestorId)) {
            targetParentId = m_deviceIdMap.value(srcAncestorId);
            continue;
        }

        // ID=1 is the reserved Physical group — always maps to target ID=1
        if (srcAncestorId == 1) {
            m_deviceIdMap[1] = 1;
            targetParentId = 1;
            continue;
        }

        QString ancestorName = srcDeviceName(srcAncestorId);

        // Check if a device with this name already exists in the target at this level
        QSqlQuery findQ(QSqlDatabase::database(tgtConn));
        if (targetParentId == 0) {
            findQ.prepare("SELECT device_id FROM device "
                          "WHERE device_name = :name AND (device_parent_id IS NULL OR device_parent_id = 0)");
            findQ.bindValue(":name", ancestorName);
        } else {
            findQ.prepare("SELECT device_id FROM device "
                          "WHERE device_name = :name AND device_parent_id = :parent");
            findQ.bindValue(":name",   ancestorName);
            findQ.bindValue(":parent", targetParentId);
        }
        findQ.exec();

        if (findQ.next()) {
            // Reuse existing device
            int existingId = findQ.value(0).toInt();
            m_deviceIdMap[srcAncestorId] = existingId;
            targetParentId = existingId;
        } else {
            // Insert a new Virtual ancestor device
            int newId = srcAncestorId + m_deviceIdOffset;

            // Avoid ID collision
            {
                QSqlQuery chk(QSqlDatabase::database(tgtConn));
                chk.prepare("SELECT COUNT(*) FROM device WHERE device_id = :id");
                chk.bindValue(":id", newId);
                chk.exec();
                if (chk.next() && chk.value(0).toInt() > 0) {
                    QSqlQuery maxQ(QSqlDatabase::database(tgtConn));
                    maxQ.exec("SELECT MAX(device_id) FROM device");
                    newId = (maxQ.next() ? maxQ.value(0).toInt() : 0) + 1;
                }
            }

            QSqlQuery ins(QSqlDatabase::database(tgtConn));
            ins.prepare(QLatin1String(R"(
                INSERT INTO device (
                    device_id, device_parent_id, device_name, device_type,
                    device_active, device_group_id, device_order)
                VALUES (:id, :parent, :name, 'Virtual', 1, 0, 0)
            )"));
            ins.bindValue(":id",     newId);
            ins.bindValue(":parent", targetParentId == 0 ? QVariant() : QVariant(targetParentId));
            ins.bindValue(":name",   ancestorName);
            ins.exec();

            m_deviceIdMap[srcAncestorId] = newId;
            targetParentId = newId;
        }
    }

    return targetParentId;
}

//----------------------------------------------------------------------
// Name conflict resolution
//----------------------------------------------------------------------
QString CollectionImporter::resolveNameConflict(const QString &name,
                                                const QStringList &existingNames) const
{
    if (!existingNames.contains(name))
        return name;
    int suffix = 2;
    QString candidate;
    do {
        candidate = QString("%1 (%2)").arg(name).arg(suffix++);
    } while (existingNames.contains(candidate));
    return candidate;
}

//----------------------------------------------------------------------
// Catalog insert
//----------------------------------------------------------------------
int CollectionImporter::remapAndInsertCatalog(int srcCatalogId)
{
    QSqlQuery srcQ(QSqlDatabase::database(m_sourceConnectionName));
    srcQ.prepare(QLatin1String(R"(
        SELECT catalog_id, catalog_file_path, catalog_name, catalog_date_updated,
               catalog_source_path, catalog_file_count, catalog_total_file_size,
               catalog_source_path_is_active, catalog_include_hidden, catalog_file_type,
               catalog_storage, catalog_include_symblinks, catalog_is_full_device,
               catalog_date_loaded, catalog_include_metadata, catalog_include_checksum,
               catalog_app_version
        FROM catalog WHERE catalog_id = :id
    )"));
    srcQ.bindValue(":id", srcCatalogId);
    srcQ.exec();
    if (!srcQ.next()) {
        m_lastError = QString("Source catalog id %1 not found").arg(srcCatalogId);
        return -1;
    }

    int    newCatalogId = srcCatalogId + m_catalogIdOffset;
    QString srcName     = srcQ.value(2).toString();

    // Resolve catalog name conflict against existing target catalog names
    QString tgtConn = m_target->connectionName();
    QSqlQuery namesQ(QSqlDatabase::database(tgtConn));
    namesQ.exec("SELECT catalog_name FROM catalog");
    QStringList existingNames;
    while (namesQ.next())
        existingNames << namesQ.value(0).toString();
    QString newName = resolveNameConflict(srcName, existingNames);

    // Derive the file_path for the target
    QString newFilePath;
    if (m_target->databaseMode == "Memory")
        newFilePath = m_target->folder + "/" + newName + ".idx";
    else
        newFilePath = srcQ.value(1).toString();

    QSqlQuery ins(QSqlDatabase::database(tgtConn));
    ins.prepare(QLatin1String(R"(
        INSERT INTO catalog (
            catalog_id, catalog_file_path, catalog_name, catalog_date_updated,
            catalog_source_path, catalog_file_count, catalog_total_file_size,
            catalog_source_path_is_active, catalog_include_hidden, catalog_file_type,
            catalog_storage, catalog_include_symblinks, catalog_is_full_device,
            catalog_date_loaded, catalog_include_metadata, catalog_include_checksum,
            catalog_app_version)
        VALUES (
            :catalog_id, :catalog_file_path, :catalog_name, :catalog_date_updated,
            :catalog_source_path, :catalog_file_count, :catalog_total_file_size,
            :catalog_source_path_is_active, :catalog_include_hidden, :catalog_file_type,
            :catalog_storage, :catalog_include_symblinks, :catalog_is_full_device,
            :catalog_date_loaded, :catalog_include_metadata, :catalog_include_checksum,
            :catalog_app_version)
    )"));
    ins.bindValue(":catalog_id",                    newCatalogId);
    ins.bindValue(":catalog_file_path",              newFilePath);
    ins.bindValue(":catalog_name",                   newName);
    ins.bindValue(":catalog_date_updated",           srcQ.value(3));
    ins.bindValue(":catalog_source_path",            srcQ.value(4));
    ins.bindValue(":catalog_file_count",             srcQ.value(5));
    ins.bindValue(":catalog_total_file_size",        srcQ.value(6));
    ins.bindValue(":catalog_source_path_is_active",  srcQ.value(7));
    ins.bindValue(":catalog_include_hidden",         srcQ.value(8));
    ins.bindValue(":catalog_file_type",              srcQ.value(9));
    ins.bindValue(":catalog_storage",                srcQ.value(10));
    ins.bindValue(":catalog_include_symblinks",      srcQ.value(11));
    ins.bindValue(":catalog_is_full_device",         srcQ.value(12));
    ins.bindValue(":catalog_date_loaded",            srcQ.value(13));
    ins.bindValue(":catalog_include_metadata",       srcQ.value(14));
    ins.bindValue(":catalog_include_checksum",       srcQ.value(15));
    ins.bindValue(":catalog_app_version",            srcQ.value(16));

    if (!ins.exec()) {
        m_lastError = ins.lastError().text();
        return -1;
    }
    return newCatalogId;
}

//----------------------------------------------------------------------
// Device insert
//----------------------------------------------------------------------
int CollectionImporter::remapAndInsertDevice(int srcDeviceId, int newParentId)
{
    QSqlQuery srcQ(QSqlDatabase::database(m_sourceConnectionName));
    srcQ.prepare(QLatin1String(R"(
        SELECT device_id, device_name, device_type, device_external_id,
               device_path, device_total_file_size, device_total_file_count,
               device_total_space, device_free_space, device_active,
               device_group_id, device_date_updated, device_order
        FROM device WHERE device_id = :id
    )"));
    srcQ.bindValue(":id", srcDeviceId);
    srcQ.exec();
    if (!srcQ.next()) {
        m_lastError = QString("Source device id %1 not found").arg(srcDeviceId);
        return -1;
    }

    QString deviceType  = srcQ.value(2).toString();
    int srcExternalId   = srcQ.value(3).toInt();
    int newDeviceId     = srcDeviceId + m_deviceIdOffset;
    int newExternalId   = (deviceType == "Catalog" && srcExternalId > 0)
                              ? srcExternalId + m_catalogIdOffset
                              : srcExternalId;

    // Avoid ID collision in target
    QString tgtConn = m_target->connectionName();
    {
        QSqlQuery chk(QSqlDatabase::database(tgtConn));
        chk.prepare("SELECT COUNT(*) FROM device WHERE device_id = :id");
        chk.bindValue(":id", newDeviceId);
        chk.exec();
        if (chk.next() && chk.value(0).toInt() > 0) {
            QSqlQuery maxQ(QSqlDatabase::database(tgtConn));
            maxQ.exec("SELECT MAX(device_id) FROM device");
            newDeviceId = (maxQ.next() ? maxQ.value(0).toInt() : 0) + 1;
        }
    }

    QSqlQuery ins(QSqlDatabase::database(tgtConn));
    ins.prepare(QLatin1String(R"(
        INSERT INTO device (
            device_id, device_parent_id, device_name, device_type, device_external_id,
            device_path, device_total_file_size, device_total_file_count,
            device_total_space, device_free_space, device_active,
            device_group_id, device_date_updated, device_order)
        VALUES (
            :device_id, :device_parent_id, :device_name, :device_type, :device_external_id,
            :device_path, :device_total_file_size, :device_total_file_count,
            :device_total_space, :device_free_space, :device_active,
            :device_group_id, :device_date_updated, :device_order)
    )"));
    ins.bindValue(":device_id",              newDeviceId);
    ins.bindValue(":device_parent_id",       newParentId == 0 ? QVariant() : QVariant(newParentId));
    ins.bindValue(":device_name",            srcQ.value(1));
    ins.bindValue(":device_type",            deviceType);
    ins.bindValue(":device_external_id",     newExternalId);
    ins.bindValue(":device_path",            srcQ.value(4));
    ins.bindValue(":device_total_file_size", srcQ.value(5));
    ins.bindValue(":device_total_file_count",srcQ.value(6));
    ins.bindValue(":device_total_space",     srcQ.value(7));
    ins.bindValue(":device_free_space",      srcQ.value(8));
    ins.bindValue(":device_active",          srcQ.value(9));
    ins.bindValue(":device_group_id",        srcQ.value(10));
    ins.bindValue(":device_date_updated",    srcQ.value(11));
    ins.bindValue(":device_order",           srcQ.value(12));

    if (!ins.exec()) {
        m_lastError = ins.lastError().text();
        return -1;
    }

    m_deviceIdMap[srcDeviceId] = newDeviceId;
    return newDeviceId;
}

//----------------------------------------------------------------------
// File + folder data
//----------------------------------------------------------------------
void CollectionImporter::insertFileData(int srcCatalogId, int newCatalogId)
{
    if (m_sourceMode == "Memory") {
        // Load .idx file into source connection so we can query it uniformly
        QSqlQuery pathQ(QSqlDatabase::database(m_sourceConnectionName));
        pathQ.prepare("SELECT catalog_file_path, catalog_name, catalog_app_version "
                      "FROM catalog WHERE catalog_id = :id");
        pathQ.bindValue(":id", srcCatalogId);
        pathQ.exec();
        if (!pathQ.next())
            return;

        Catalog tmpCat;
        tmpCat.ID         = srcCatalogId;
        tmpCat.name       = pathQ.value(1).toString();
        tmpCat.filePath   = pathQ.value(0).toString();
        tmpCat.appVersion = pathQ.value(2).toString();
        tmpCat.dateUpdated = QDateTime::currentDateTime();
        tmpCat.dateLoaded  = QDateTime();
        tmpCat.setConnectionName(m_sourceConnectionName);

        QMutex mutex;
        bool stop = false;
        tmpCat.loadCatalogFileListToTable(mutex, stop);
        tmpCat.loadFoldersToTable();
    }

    QString tgtConn = m_target->connectionName();

    // Copy file rows: source → target with remapped catalog_id
    {
        QSqlQuery srcQ(QSqlDatabase::database(m_sourceConnectionName));
        srcQ.prepare(QLatin1String(R"(
            SELECT file_name, file_folder_path, file_size, file_date_updated,
                   file_catalog, file_full_path, file_extension, file_type,
                   mime_type, mime_verified, type_mismatch,
                   image_width, image_height, image_orientation,
                   video_duration_seconds, video_width, video_height, video_codec,
                   video_framerate, video_bitrate,
                   audio_duration_seconds, audio_artist, audio_album, audio_title,
                   audio_genre, audio_year, audio_track_number, audio_bitrate, audio_sample_rate,
                   metadata_extended, metadata_extraction_date,
                   checksum_sha256, checksum_extraction_date
            FROM file WHERE file_catalog_id = :id
        )"));
        srcQ.bindValue(":id", srcCatalogId);
        srcQ.exec();

        QSqlQuery ins(QSqlDatabase::database(tgtConn));
        ins.prepare(QLatin1String(R"(
            INSERT INTO file (
                file_catalog_id, file_name, file_folder_path, file_size, file_date_updated,
                file_catalog, file_full_path, file_extension, file_type,
                mime_type, mime_verified, type_mismatch,
                image_width, image_height, image_orientation,
                video_duration_seconds, video_width, video_height, video_codec,
                video_framerate, video_bitrate,
                audio_duration_seconds, audio_artist, audio_album, audio_title,
                audio_genre, audio_year, audio_track_number, audio_bitrate, audio_sample_rate,
                metadata_extended, metadata_extraction_date,
                checksum_sha256, checksum_extraction_date)
            VALUES (
                :cid, :name, :folder, :size, :date,
                :catalog, :full_path, :ext, :type,
                :mime, :mime_verified, :type_mismatch,
                :iw, :ih, :io,
                :vd, :vw, :vh, :vc, :vfr, :vbr,
                :ad, :aa, :ab, :at, :ag, :ay, :atn, :abr, :asr,
                :meta_ext, :meta_date, :csum, :csum_date)
        )"));

        while (srcQ.next()) {
            ins.bindValue(":cid",           newCatalogId);
            ins.bindValue(":name",          srcQ.value(0));
            ins.bindValue(":folder",        srcQ.value(1));
            ins.bindValue(":size",          srcQ.value(2));
            ins.bindValue(":date",          srcQ.value(3));
            ins.bindValue(":catalog",       srcQ.value(4));
            ins.bindValue(":full_path",     srcQ.value(5));
            ins.bindValue(":ext",           srcQ.value(6));
            ins.bindValue(":type",          srcQ.value(7));
            ins.bindValue(":mime",          srcQ.value(8));
            ins.bindValue(":mime_verified", srcQ.value(9));
            ins.bindValue(":type_mismatch", srcQ.value(10));
            ins.bindValue(":iw",            srcQ.value(11));
            ins.bindValue(":ih",            srcQ.value(12));
            ins.bindValue(":io",            srcQ.value(13));
            ins.bindValue(":vd",            srcQ.value(14));
            ins.bindValue(":vw",            srcQ.value(15));
            ins.bindValue(":vh",            srcQ.value(16));
            ins.bindValue(":vc",            srcQ.value(17));
            ins.bindValue(":vfr",           srcQ.value(18));
            ins.bindValue(":vbr",           srcQ.value(19));
            ins.bindValue(":ad",            srcQ.value(20));
            ins.bindValue(":aa",            srcQ.value(21));
            ins.bindValue(":ab",            srcQ.value(22));
            ins.bindValue(":at",            srcQ.value(23));
            ins.bindValue(":ag",            srcQ.value(24));
            ins.bindValue(":ay",            srcQ.value(25));
            ins.bindValue(":atn",           srcQ.value(26));
            ins.bindValue(":abr",           srcQ.value(27));
            ins.bindValue(":asr",           srcQ.value(28));
            ins.bindValue(":meta_ext",      srcQ.value(29));
            ins.bindValue(":meta_date",     srcQ.value(30));
            ins.bindValue(":csum",          srcQ.value(31));
            ins.bindValue(":csum_date",     srcQ.value(32));
            ins.exec();
        }
    }

    // Copy folder rows (deduplicated)
    {
        QSqlQuery srcQ(QSqlDatabase::database(m_sourceConnectionName));
        srcQ.prepare("SELECT DISTINCT folder_path FROM folder WHERE folder_catalog_id = :id");
        srcQ.bindValue(":id", srcCatalogId);
        srcQ.exec();

        QSqlQuery ins(QSqlDatabase::database(tgtConn));
        ins.prepare("INSERT INTO folder (folder_catalog_id, folder_path) VALUES (:cid, :path)");
        while (srcQ.next()) {
            ins.bindValue(":cid",  newCatalogId);
            ins.bindValue(":path", srcQ.value(0));
            ins.exec();
        }
    }

    // Persist to disk if target is Memory mode
    if (m_target->databaseMode == "Memory") {
        Catalog saveCat;
        saveCat.ID = newCatalogId;
        saveCat.setConnectionName(tgtConn);

        // Load full catalog metadata so the .idx header is complete
        QSqlQuery catQ(QSqlDatabase::database(tgtConn));
        catQ.prepare(QLatin1String(R"(
            SELECT catalog_name, catalog_source_path, catalog_file_count,
                   catalog_total_file_size, catalog_include_hidden, catalog_file_type,
                   catalog_storage, catalog_include_symblinks, catalog_is_full_device,
                   catalog_include_metadata, catalog_include_checksum, catalog_app_version
            FROM catalog WHERE catalog_id = :id
        )"));
        catQ.bindValue(":id", newCatalogId);
        catQ.exec();
        if (!catQ.next()) {
            qWarning() << "WARNING: insertFileData: catalog" << newCatalogId
                       << "not found in target DB — skipping .idx write";
            return;
        }
        saveCat.name             = catQ.value(0).toString();
        saveCat.sourcePath       = catQ.value(1).toString();
        saveCat.fileCount        = catQ.value(2).toLongLong();
        saveCat.totalFileSize    = catQ.value(3).toLongLong();
        saveCat.includeHidden    = catQ.value(4).toBool();
        saveCat.fileType         = catQ.value(5).toString();
        saveCat.storageName      = catQ.value(6).toString();
        saveCat.includeSymblinks = catQ.value(7).toBool();
        saveCat.isFullDevice     = catQ.value(8).toBool();
        saveCat.includeMetadata  = catQ.value(9).toString();
        saveCat.includeChecksum  = catQ.value(10).toString();
        saveCat.appVersion       = catQ.value(11).toString();

        if (!saveCat.saveCatalogToFile(m_target->databaseMode, m_target->folder))
            qWarning() << "WARNING: insertFileData: failed to write .idx for catalog" << newCatalogId;
        if (!saveCat.saveFoldersToFile(m_target->databaseMode, m_target->folder))
            qWarning() << "WARNING: insertFileData: failed to write .folders.idx for catalog" << newCatalogId;
    }
}

//----------------------------------------------------------------------
// Catalog filter
//----------------------------------------------------------------------
void CollectionImporter::insertCatalogFilter(int srcCatalogId, int newCatalogId)
{
    QSqlQuery srcQ(QSqlDatabase::database(m_sourceConnectionName));
    srcQ.prepare("SELECT filter_path, filter_type FROM catalog_filter WHERE filter_catalog_id = :id");
    srcQ.bindValue(":id", srcCatalogId);
    srcQ.exec();

    QSqlQuery ins(QSqlDatabase::database(m_target->connectionName()));
    ins.prepare("INSERT INTO catalog_filter (filter_catalog_id, filter_path, filter_type) "
                "VALUES (:cid, :path, :type)");
    while (srcQ.next()) {
        ins.bindValue(":cid",  newCatalogId);
        ins.bindValue(":path", srcQ.value(0));
        ins.bindValue(":type", srcQ.value(1));
        ins.exec();
    }
}

//----------------------------------------------------------------------
// Provenance link
//----------------------------------------------------------------------
void CollectionImporter::recordImportLink(int srcDeviceId, int newDeviceId)
{
    QSqlQuery ins(QSqlDatabase::database(m_target->connectionName()));
    ins.prepare(QLatin1String(R"(
        INSERT INTO device_mapping (
            mapping_type, mapping_device_source_id, mapping_device_target_id,
            mapping_source_collection)
        VALUES ('CollectionImport', :src, :tgt, :coll)
    )"));
    ins.bindValue(":src",  srcDeviceId);
    ins.bindValue(":tgt",  newDeviceId);
    ins.bindValue(":coll", m_sourcePath);
    ins.exec();
}

//----------------------------------------------------------------------
// Recursive sub-tree import (private)
//----------------------------------------------------------------------
bool CollectionImporter::importSubTree(int srcDeviceId, int targetParentId,
                                       int &imported, int total)
{
    if (wasStopRequested())
        return false;

    // ID=1 is the reserved Physical group — it always exists in both collections.
    // Never re-insert it; just map source id=1 to target id=1 and recurse.
    int newDeviceId;
    if (srcDeviceId == 1) {
        newDeviceId = 1;
        m_deviceIdMap[1] = 1;
    } else {
        newDeviceId = remapAndInsertDevice(srcDeviceId, targetParentId);
        if (newDeviceId < 0)
            return false;
    }

    QString deviceType = srcDeviceType(srcDeviceId);
    if (deviceType == "Catalog") {
        // Get the source catalog ID (= device_external_id)
        QSqlQuery extQ(QSqlDatabase::database(m_sourceConnectionName));
        extQ.prepare("SELECT device_external_id FROM device WHERE device_id = :id");
        extQ.bindValue(":id", srcDeviceId);
        extQ.exec();
        if (extQ.next()) {
            int srcCatalogId = extQ.value(0).toInt();
            int newCatalogId = remapAndInsertCatalog(srcCatalogId);
            if (newCatalogId > 0) {
                m_catalogIdMap[srcCatalogId] = newCatalogId;
                insertFileData(srcCatalogId, newCatalogId);
                insertCatalogFilter(srcCatalogId, newCatalogId);
                importStorageForCatalog(srcCatalogId);
            }
        }
        recordImportLink(srcDeviceId, newDeviceId);
    }

    importStatisticsForDevice(srcDeviceId, newDeviceId);
    imported++;
    emit importProgress(imported, total, srcDeviceName(srcDeviceId));

    // Recurse for children
    QSqlQuery childQ(QSqlDatabase::database(m_sourceConnectionName));
    childQ.prepare("SELECT device_id FROM device WHERE device_parent_id = :pid "
                   "ORDER BY device_order, device_id");
    childQ.bindValue(":pid", srcDeviceId);
    childQ.exec();

    while (childQ.next()) {
        if (!importSubTree(childQ.value(0).toInt(), newDeviceId, imported, total))
            return false;
    }
    return true;
}

//----------------------------------------------------------------------
// Public operations
//----------------------------------------------------------------------
bool CollectionImporter::importDevice(int sourceDeviceId)
{
    if (!isSourceOpen()) {
        m_lastError = "No source collection is open";
        return false;
    }

    m_stopRequested.storeRelease(0);
    m_deviceIdMap.clear();
    m_catalogIdMap.clear();
    buildIdOffsets();

    QString tgtConn = m_target->connectionName();
    if (!Database::beginTransaction(tgtConn)) {
        m_lastError = "Failed to begin transaction";
        return false;
    }

    int targetParentId = ensureAncestors(sourceDeviceId);

    // Count total devices in the sub-tree for progress reporting
    int total = 0;
    {
        QSqlQuery cntQ(QSqlDatabase::database(m_sourceConnectionName));
        cntQ.exec("SELECT COUNT(*) FROM device");
        total = cntQ.next() ? cntQ.value(0).toInt() : 1;
    }
    int imported = 0;

    bool ok = importSubTree(sourceDeviceId, targetParentId, imported, total);

    if (!ok || wasStopRequested()) {
        Database::rollbackTransaction(tgtConn);
        if (wasStopRequested())
            m_lastError = "Import cancelled";
        return false;
    }

    // Phase 2: cross-device data — run inside same transaction
    importBackupMappings();
    importTagsForAllCatalogs();

    if (!Database::commitTransaction(tgtConn)) {
        m_lastError = "Failed to commit transaction";
        return false;
    }
    return true;
}

int CollectionImporter::importAllDevices()
{
    if (!isSourceOpen()) {
        m_lastError = "No source collection is open";
        return -1;
    }

    m_stopRequested.storeRelease(0);
    QList<int> roots = sourceRootDeviceIds();
    int importedCount = 0;

    m_deviceIdMap.clear();
    m_catalogIdMap.clear();
    buildIdOffsets();

    int total = 0;
    {
        QSqlQuery cntQ(QSqlDatabase::database(m_sourceConnectionName));
        cntQ.exec("SELECT COUNT(*) FROM device");
        total = cntQ.next() ? cntQ.value(0).toInt() : 1;
    }
    int imported = 0;

    const QString tgtConn = m_target->connectionName();
    if (!Database::beginTransaction(tgtConn))
        return -1;

    for (int rootId : roots) {
        if (wasStopRequested()) {
            Database::rollbackTransaction(tgtConn);
            return -1;
        }

        bool ok = importSubTree(rootId, 0, imported, total);

        if (!ok) {
            Database::rollbackTransaction(tgtConn);
            return -1;
        }

        importedCount++;
    }

    // Phase 2: all roots processed — all device/catalog IDs now in maps
    importBackupMappings();
    importTagsForAllCatalogs();

    if (!Database::commitTransaction(tgtConn))
        return -1;

    return importedCount;
}

bool CollectionImporter::updateDevice(int targetDeviceId)
{
    if (!isSourceOpen()) {
        m_lastError = "No source collection is open";
        return false;
    }

    QString tgtConn = m_target->connectionName();

    // Find the CollectionImport mapping for this target device
    QSqlQuery mapQ(QSqlDatabase::database(tgtConn));
    mapQ.prepare(QLatin1String(R"(
        SELECT mapping_device_source_id
        FROM device_mapping
        WHERE mapping_type = 'CollectionImport'
          AND mapping_device_target_id = :id
    )"));
    mapQ.bindValue(":id", targetDeviceId);
    mapQ.exec();
    if (!mapQ.next()) {
        m_lastError = "No CollectionImport link found for target device";
        return false;
    }
    int srcDeviceId = mapQ.value(0).toInt();

    // Get target catalog id
    QSqlQuery devQ(QSqlDatabase::database(tgtConn));
    devQ.prepare("SELECT device_external_id FROM device WHERE device_id = :id");
    devQ.bindValue(":id", targetDeviceId);
    devQ.exec();
    if (!devQ.next()) {
        m_lastError = "Target device not found";
        return false;
    }
    int targetCatalogId = devQ.value(0).toInt();

    // Get source catalog id
    QSqlQuery srcDevQ(QSqlDatabase::database(m_sourceConnectionName));
    srcDevQ.prepare("SELECT device_external_id FROM device WHERE device_id = :id");
    srcDevQ.bindValue(":id", srcDeviceId);
    srcDevQ.exec();
    if (!srcDevQ.next()) {
        m_lastError = "Source device not found for update";
        return false;
    }
    int srcCatalogId = srcDevQ.value(0).toInt();

    if (!Database::beginTransaction(tgtConn)) {
        m_lastError = "Failed to begin transaction for update";
        return false;
    }

    // Delete existing file + folder rows for this catalog in target
    {
        QSqlQuery del(QSqlDatabase::database(tgtConn));
        del.prepare("DELETE FROM file WHERE file_catalog_id = :id");
        del.bindValue(":id", targetCatalogId);
        del.exec();
        del.prepare("DELETE FROM folder WHERE folder_catalog_id = :id");
        del.bindValue(":id", targetCatalogId);
        del.exec();
    }

    // Re-import without ID offset (catalog already exists in target with same IDs)
    int savedDeviceOffset  = m_deviceIdOffset;
    int savedCatalogOffset = m_catalogIdOffset;
    m_deviceIdOffset  = 0;
    m_catalogIdOffset = 0;
    insertFileData(srcCatalogId, targetCatalogId);
    m_deviceIdOffset  = savedDeviceOffset;
    m_catalogIdOffset = savedCatalogOffset;

    // Update catalog metadata
    QSqlQuery srcCatQ(QSqlDatabase::database(m_sourceConnectionName));
    srcCatQ.prepare("SELECT catalog_file_count, catalog_total_file_size, catalog_date_updated "
                    "FROM catalog WHERE catalog_id = :id");
    srcCatQ.bindValue(":id", srcCatalogId);
    srcCatQ.exec();
    if (srcCatQ.next()) {
        QSqlQuery upd(QSqlDatabase::database(tgtConn));
        upd.prepare("UPDATE catalog SET catalog_file_count = :fc, catalog_total_file_size = :fs, "
                    "catalog_date_updated = :du WHERE catalog_id = :id");
        upd.bindValue(":fc", srcCatQ.value(0));
        upd.bindValue(":fs", srcCatQ.value(1));
        upd.bindValue(":du", srcCatQ.value(2));
        upd.bindValue(":id", targetCatalogId);
        upd.exec();
    }

    if (!Database::commitTransaction(tgtConn)) {
        m_lastError = "Failed to commit update transaction";
        return false;
    }
    return true;
}

//----------------------------------------------------------------------
// Storage
//----------------------------------------------------------------------
QString CollectionImporter::sourceImageFolderPath() const
{
    QSqlQuery q(QSqlDatabase::database(m_sourceConnectionName));
    q.prepare("SELECT parameter_value1 FROM parameter "
              "WHERE parameter_name = 'imageFolderPath' AND parameter_type = 'collection'");
    q.exec();
    if (q.next()) {
        QString path = q.value(0).toString();
        if (!path.isEmpty())
            return path;
    }
    if (m_sourceMode == "File")
        return QFileInfo(m_sourcePath).dir().absolutePath() + "/images";
    return m_sourcePath + "/images";
}

void CollectionImporter::copyStorageImage(const QString &picturePath)
{
    if (picturePath.isEmpty())
        return;

    QString tgtFolder = m_target->imageFolderPath.isEmpty()
                      ? m_target->folder + "/images"
                      : m_target->imageFolderPath;
    QDir().mkpath(tgtFolder);

    QString src = sourceImageFolderPath() + "/" + picturePath;
    QString dst = tgtFolder + "/" + picturePath;
    if (QFile::exists(src) && !QFile::exists(dst))
        QFile::copy(src, dst);
}

void CollectionImporter::importStorageForCatalog(int srcCatalogId)
{
    // Identify the storage name referenced by this catalog
    QSqlQuery catQ(QSqlDatabase::database(m_sourceConnectionName));
    catQ.prepare("SELECT catalog_storage FROM catalog WHERE catalog_id = :id");
    catQ.bindValue(":id", srcCatalogId);
    catQ.exec();
    if (!catQ.next()) return;
    const QString storageName = catQ.value(0).toString();
    if (storageName.isEmpty()) return;

    const QString tgtConn = m_target->connectionName();

    // Skip if the same storage name already exists in the target
    {
        QSqlQuery chk(QSqlDatabase::database(tgtConn));
        chk.prepare("SELECT COUNT(*) FROM storage WHERE storage_name = :n");
        chk.bindValue(":n", storageName);
        chk.exec();
        if (chk.next() && chk.value(0).toInt() > 0)
            return;
    }

    // Read the full source storage row
    QSqlQuery srcQ(QSqlDatabase::database(m_sourceConnectionName));
    srcQ.prepare(QLatin1String(R"(
        SELECT storage_id, storage_name, storage_type, storage_location, storage_path,
               storage_label, storage_file_system, storage_total_space, storage_free_space,
               storage_brand, storage_model, storage_serial_number, storage_build_date,
               storage_comment1, storage_comment2, storage_comment3, storage_picture_path
        FROM storage WHERE storage_name = :n
    )"));
    srcQ.bindValue(":n", storageName);
    srcQ.exec();
    if (!srcQ.next()) return;

    int newStorageId = srcQ.value(0).toInt() + m_storageIdOffset;
    {
        QSqlQuery chkId(QSqlDatabase::database(tgtConn));
        chkId.prepare("SELECT COUNT(*) FROM storage WHERE storage_id = :id");
        chkId.bindValue(":id", newStorageId);
        chkId.exec();
        if (chkId.next() && chkId.value(0).toInt() > 0) {
            QSqlQuery maxQ(QSqlDatabase::database(tgtConn));
            maxQ.exec("SELECT MAX(storage_id) FROM storage");
            newStorageId = (maxQ.next() ? maxQ.value(0).toInt() : 0) + 1;
        }
    }

    QSqlQuery ins(QSqlDatabase::database(tgtConn));
    ins.prepare(QLatin1String(R"(
        INSERT INTO storage (
            storage_id, storage_name, storage_type, storage_location, storage_path,
            storage_label, storage_file_system, storage_total_space, storage_free_space,
            storage_brand, storage_model, storage_serial_number, storage_build_date,
            storage_comment1, storage_comment2, storage_comment3, storage_picture_path)
        VALUES (
            :id, :name, :type, :loc, :path,
            :label, :fs, :total, :free,
            :brand, :model, :serial, :build,
            :c1, :c2, :c3, :pic)
    )"));
    ins.bindValue(":id",     newStorageId);
    ins.bindValue(":name",   srcQ.value(1));
    ins.bindValue(":type",   srcQ.value(2));
    ins.bindValue(":loc",    srcQ.value(3));
    ins.bindValue(":path",   srcQ.value(4));
    ins.bindValue(":label",  srcQ.value(5));
    ins.bindValue(":fs",     srcQ.value(6));
    ins.bindValue(":total",  srcQ.value(7));
    ins.bindValue(":free",   srcQ.value(8));
    ins.bindValue(":brand",  srcQ.value(9));
    ins.bindValue(":model",  srcQ.value(10));
    ins.bindValue(":serial", srcQ.value(11));
    ins.bindValue(":build",  srcQ.value(12));
    ins.bindValue(":c1",     srcQ.value(13));
    ins.bindValue(":c2",     srcQ.value(14));
    ins.bindValue(":c3",     srcQ.value(15));
    ins.bindValue(":pic",    srcQ.value(16));
    ins.exec();

    copyStorageImage(srcQ.value(16).toString());
}

//----------------------------------------------------------------------
// Statistics_device
//----------------------------------------------------------------------
void CollectionImporter::importStatisticsForDevice(int srcDeviceId, int newDeviceId)
{
    QSqlQuery srcQ(QSqlDatabase::database(m_sourceConnectionName));
    srcQ.prepare(QLatin1String(R"(
        SELECT date_time, device_name, device_type, device_file_count,
               device_total_file_size, device_free_space, device_total_space, record_type
        FROM statistics_device WHERE device_id = :id
    )"));
    srcQ.bindValue(":id", srcDeviceId);
    srcQ.exec();

    QSqlQuery ins(QSqlDatabase::database(m_target->connectionName()));
    ins.prepare(QLatin1String(R"(
        INSERT INTO statistics_device (
            date_time, device_id, device_name, device_type, device_file_count,
            device_total_file_size, device_free_space, device_total_space, record_type)
        VALUES (:dt, :id, :name, :type, :fc, :fs, :free, :total, :rt)
    )"));

    while (srcQ.next()) {
        ins.bindValue(":dt",    srcQ.value(0));
        ins.bindValue(":id",    newDeviceId);
        ins.bindValue(":name",  srcQ.value(1));
        ins.bindValue(":type",  srcQ.value(2));
        ins.bindValue(":fc",    srcQ.value(3));
        ins.bindValue(":fs",    srcQ.value(4));
        ins.bindValue(":free",  srcQ.value(5));
        ins.bindValue(":total", srcQ.value(6));
        ins.bindValue(":rt",    srcQ.value(7));
        ins.exec();
    }
}

//----------------------------------------------------------------------
// Device_mapping BackUp entries
//----------------------------------------------------------------------
void CollectionImporter::importBackupMappings()
{
    QSqlQuery srcQ(QSqlDatabase::database(m_sourceConnectionName));
    srcQ.exec(QLatin1String(R"(
        SELECT mapping_type, mapping_name, mapping_device_source_id, mapping_device_target_id,
               mapping_backup_last_date, mapping_backup_last_size,
               mapping_strict_copy, mapping_conflict_mode, mapping_source_mode
        FROM device_mapping WHERE mapping_type != 'CollectionImport'
    )"));

    QSqlQuery ins(QSqlDatabase::database(m_target->connectionName()));
    ins.prepare(QLatin1String(R"(
        INSERT INTO device_mapping (
            mapping_type, mapping_name,
            mapping_device_source_id, mapping_device_target_id,
            mapping_backup_last_date, mapping_backup_last_size,
            mapping_strict_copy, mapping_conflict_mode, mapping_source_mode)
        VALUES (
            :type, :name,
            :src, :tgt,
            :date, :size,
            :strict, :conflict, :srcmode)
    )"));

    while (srcQ.next()) {
        int srcId = srcQ.value(2).toInt();
        int tgtId = srcQ.value(3).toInt();
        // Only import if both devices were imported in this session
        if (!m_deviceIdMap.contains(srcId) || !m_deviceIdMap.contains(tgtId))
            continue;

        ins.bindValue(":type",    srcQ.value(0));
        ins.bindValue(":name",    srcQ.value(1));
        ins.bindValue(":src",     m_deviceIdMap.value(srcId));
        ins.bindValue(":tgt",     m_deviceIdMap.value(tgtId));
        ins.bindValue(":date",    srcQ.value(4));
        ins.bindValue(":size",    srcQ.value(5));
        ins.bindValue(":strict",  srcQ.value(6));
        ins.bindValue(":conflict",srcQ.value(7));
        ins.bindValue(":srcmode", srcQ.value(8));
        ins.exec();
    }
}

//----------------------------------------------------------------------
// Tags
//----------------------------------------------------------------------
void CollectionImporter::importTagsForAllCatalogs()
{
    // Tags are folder-level labels (name + directory path), independent of any
    // catalog or device. Copy all source tags, skipping duplicates by (name, path).
    const QString tgtConn = m_target->connectionName();

    QSqlQuery srcTags(QSqlDatabase::database(m_sourceConnectionName));
    srcTags.exec("SELECT name, path, type, date_time FROM tag");

    QSqlQuery ins(QSqlDatabase::database(tgtConn));
    ins.prepare("INSERT INTO tag (name, path, type, date_time) VALUES (:n, :p, :t, :dt)");

    while (srcTags.next()) {
        const QString name = srcTags.value(0).toString();
        const QString path = srcTags.value(1).toString();

        // Skip if (name, path) already exists in target
        QSqlQuery chk(QSqlDatabase::database(tgtConn));
        chk.prepare("SELECT COUNT(*) FROM tag WHERE name = :n AND path = :p");
        chk.bindValue(":n", name);
        chk.bindValue(":p", path);
        chk.exec();
        if (chk.next() && chk.value(0).toInt() > 0)
            continue;

        ins.bindValue(":n",  name);
        ins.bindValue(":p",  path);
        ins.bindValue(":t",  srcTags.value(2));
        ins.bindValue(":dt", srcTags.value(3));
        ins.exec();
    }
}

//----------------------------------------------------------------------
// Source device model (for the UI tree view)
//----------------------------------------------------------------------
QStandardItemModel *CollectionImporter::buildSourceDeviceModel()
{
    auto *model = new QStandardItemModel(this);
    model->setHorizontalHeaderLabels({tr("Name"), tr("Type"), tr("Device ID")});

    if (!isSourceOpen())
        return model;

    QSqlQuery q(QSqlDatabase::database(m_sourceConnectionName));
    q.exec(QLatin1String(R"(
        SELECT device_id, device_parent_id, device_name, device_type
        FROM device ORDER BY device_parent_id, device_order, device_id
    )"));

    struct Row { int id; int parentId; QString name; QString type; };
    QList<Row> rows;
    while (q.next())
        rows.append({q.value(0).toInt(), q.value(1).toInt(),
                     q.value(2).toString(), q.value(3).toString()});

    // Build all items first
    QMap<int, QList<QStandardItem*>> itemMap;
    for (const Row &r : rows) {
        QStandardItem *nameItem = new QStandardItem(r.name);
        nameItem->setData(r.id,       Qt::UserRole);
        nameItem->setData(r.parentId, Qt::UserRole + 1);
        nameItem->setFlags(nameItem->flags() & ~Qt::ItemIsEditable);

        QStandardItem *typeItem = new QStandardItem(r.type);
        typeItem->setFlags(typeItem->flags() & ~Qt::ItemIsEditable);

        QStandardItem *idItem = new QStandardItem(QString::number(r.id));
        idItem->setFlags(idItem->flags() & ~Qt::ItemIsEditable);

        itemMap[r.id] = {nameItem, typeItem, idItem};
    }

    // Synthetic "Collection" root — sentinel ID=0 means "import all"
    QStandardItem *collRoot = new QStandardItem(tr("Collection"));
    collRoot->setData(0, Qt::UserRole);         // sentinel: importAllDevices()
    collRoot->setData(0, Qt::UserRole + 1);
    collRoot->setFlags(collRoot->flags() & ~Qt::ItemIsEditable);
    model->appendRow({collRoot,
                      new QStandardItem(),
                      new QStandardItem("0")});

    // Attach each item to its parent (or to the Collection root)
    for (const Row &r : rows) {
        auto rowItems = itemMap.value(r.id);
        if (r.parentId == 0 || !itemMap.contains(r.parentId)) {
            collRoot->appendRow(rowItems);
        } else {
            itemMap[r.parentId][0]->appendRow(rowItems);
        }
    }

    return model;
}
