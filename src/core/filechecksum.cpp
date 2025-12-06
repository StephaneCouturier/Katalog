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
// File Name:   filechecksum.cpp
// Purpose:     Implementation of FileChecksum class
// Description: Provides checksum calculation using Qt QCryptographicHash
// Author:      Stephane Couturier
/////////////////////////////////////////////////////////////////////////////
*/

#include "filechecksum.h"
#include "catalog.h"
#include <QFile>
#include <QFileInfo>
#include <QSqlQuery>
#include <QSqlError>
#include <QDateTime>
#include <QDebug>

FileChecksum::FileChecksum(QObject *parent) : QObject(parent)
{
}

//-----------------------------------------------------------------------------------------------------
bool FileChecksum::calculateAndStore(const QString &filePath,
                                     const QString &connectionName,
                                     int catalogId,
                                     QString includeChecksum)
{
    // Guard: If checksum disabled, return immediately
    if (includeChecksum == Catalog::CHECKSUM_NONE) {
        return true;  // Not an error, just skipped
    }

    // Check if file exists
    QFileInfo fileInfo(filePath);
    if (!fileInfo.exists() || !fileInfo.isFile()) {
        qDebug() << "FileChecksum::calculateAndStore - File does not exist:" << filePath;
        return false;
    }

    // Calculate checksum
    QString checksumValue = calculateChecksum(filePath, QCryptographicHash::Sha256);

    if (checksumValue.isEmpty()) {
        qDebug() << "FileChecksum::calculateAndStore - Failed to calculate checksum:" << filePath;
        return false;
    }

    // Store in database
    return updateFileChecksum(connectionName, catalogId,
                              fileInfo.fileName(),
                              fileInfo.absolutePath(),
                              checksumValue,
                              "SHA256");
}

//-----------------------------------------------------------------------------------------------------
QString FileChecksum::calculateChecksum(const QString &filePath,
                                        QCryptographicHash::Algorithm algorithm)
{
    QFile file(filePath);

    if (!file.open(QIODevice::ReadOnly)) {
        qDebug() << "FileChecksum::calculateChecksum - Cannot open file:" << filePath;
        qDebug() << "  Error:" << file.errorString();
        return QString();
    }

    qint64 fileSize = file.size();
    qDebug() << "FileChecksum::calculateChecksum - Processing:" << filePath;
    qDebug() << "  File size:" << QLocale().formattedDataSize(fileSize);

    QCryptographicHash hash(algorithm);

    const qint64 bufferSize = 1024 * 1024 * 1024; // 8 MB buffer
    qint64 totalRead = 0;

    while (!file.atEnd()) {
        QByteArray buffer = file.read(bufferSize);
        if (buffer.isEmpty()) {
            break;
        }
        hash.addData(buffer);
        totalRead += buffer.size();
    }

    file.close();

    QString checksum = hash.result().toHex().toLower();
    qDebug() << "  Checksum:" << checksum;
    qDebug() << "  Bytes processed:" << QLocale().formattedDataSize(totalRead);

    return checksum;
}

//-----------------------------------------------------------------------------------------------------
bool FileChecksum::updateFileChecksum(const QString &connectionName,
                                      int catalogId,
                                      const QString &fileName,
                                      const QString &folderPath,
                                      const QString &checksumValue,
                                      const QString &checksumType)
{
    QSqlDatabase database = QSqlDatabase::database(connectionName);
    if (!database.isOpen()) {
        qDebug() << "FileChecksum::updateFileChecksum - Database not open";
        return false;
    }

    // Build UPDATE query (only for SHA-256 in Increment 1)
    QString queryString = QLatin1String(R"(
        UPDATE file
        SET checksum_sha256 = ?,
            checksum_extraction_date = ?
        WHERE file_catalog_id = ?
          AND file_name = ?
          AND file_folder_path = ?
    )");

    QSqlQuery query(database);
    query.prepare(queryString);

    QString currentDateTime = QDateTime::currentDateTime().toString(Qt::ISODate);

    query.addBindValue(checksumValue);
    query.addBindValue(currentDateTime);
    query.addBindValue(catalogId);
    query.addBindValue(fileName);
    query.addBindValue(folderPath);

    if (!query.exec()) {
        qDebug() << "FileChecksum::updateFileChecksum - Query failed:" << query.lastError().text();
        qDebug() << "Query was:" << queryString;
        return false;
    }

    return true;
}

//-----------------------------------------------------------------------------------------------------
bool FileChecksum::batchUpdateFileChecksum(const QString &connectionName,
                                           int catalogId,
                                           const QStringList &fileNames,
                                           const QStringList &folderPaths,
                                           const QStringList &checksumValues)
{
    if (fileNames.size() != folderPaths.size() || fileNames.size() != checksumValues.size()) {
        qDebug() << "FileChecksum::batchUpdateFileChecksum - Size mismatch in input arrays";
        return false;
    }

    if (fileNames.isEmpty()) {
        return true;  // Nothing to update
    }

    QSqlDatabase database = QSqlDatabase::database(connectionName);
    if (!database.isOpen()) {
        qDebug() << "FileChecksum::batchUpdateFileChecksum - Database not open";
        return false;
    }

    QString currentDateTime = QDateTime::currentDateTime().toString(Qt::ISODate);

    QString queryString = QLatin1String(R"(
        UPDATE file
        SET checksum_sha256 = ?,
            checksum_extraction_date = ?
        WHERE file_catalog_id = ?
          AND file_name = ?
          AND file_folder_path = ?
    )");

    QSqlQuery query(database);
    query.prepare(queryString);

    // Use transaction for batch update
    database.transaction();

    for (int i = 0; i < fileNames.size(); ++i) {
        query.addBindValue(checksumValues[i]);
        query.addBindValue(currentDateTime);
        query.addBindValue(catalogId);
        query.addBindValue(fileNames[i]);
        query.addBindValue(folderPaths[i]);

        if (!query.exec()) {
            qDebug() << "FileChecksum::batchUpdateFileChecksum - Failed for file:"
                     << fileNames[i] << query.lastError().text();
            database.rollback();
            return false;
        }
    }

    database.commit();
    qDebug() << "FileChecksum::batchUpdateFileChecksum - Updated" << fileNames.size() << "files";

    return true;
}

QCryptographicHash::Algorithm FileChecksum::getAlgorithmFromString(const QString &algorithmName)
{
    if (algorithmName == Catalog::CHECKSUM_SHA256 || algorithmName == "SHA256") {
        return QCryptographicHash::Sha256;
    }
    // else if (algorithmName == "SHA512") {
    //     return QCryptographicHash::Sha512;
    // }

    // Default fallback
    qDebug() << "WARNING: Unknown checksum algorithm:" << algorithmName << "- defaulting to SHA256";
    return QCryptographicHash::Sha256;
}
