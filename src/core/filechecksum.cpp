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
                                        QCryptographicHash::Algorithm algorithm,
                                        std::function<void(qint64, qint64)> progressCallback)
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

    const qint64 bufferSize = 8 * 1024 * 1024; // 8 MB buffer
    const qint64 progressInterval = 25 * 1024 * 1024; // Report every 25 MB
    qint64 totalRead = 0;
    qint64 lastProgressReport = 0;

    while (!file.atEnd()) {
        QByteArray buffer = file.read(bufferSize);
        if (buffer.isEmpty()) {
            break;
        }
        hash.addData(buffer);
        totalRead += buffer.size();

        // Call progress callback if provided
        if (progressCallback && (totalRead - lastProgressReport) >= progressInterval) {
            qDebug() << "    Progress callback: Read"
                     << QLocale().formattedDataSize(totalRead)
                     << "of" << QLocale().formattedDataSize(fileSize);

            // Call the callback - it will throw or return false if we should stop
            try {
                progressCallback(totalRead, fileSize);
            } catch (...) {
                // Callback indicated we should stop
                qDebug() << "    Progress callback indicated stop - aborting checksum";
                file.close();
                return QString(); // Return empty to indicate interrupted
            }

            lastProgressReport = totalRead;
        }
    }

    file.close();

    // Finalize the hash
    QString checksum = hash.result().toHex().toLower();

    // Final progress callback AFTER hash is complete
    if (progressCallback && totalRead > 0) {
        qDebug() << "    Final progress callback: Checksum complete";
        try {
            progressCallback(totalRead, fileSize);
        } catch (...) {
            // Ok if stop requested now, work is done
        }
    }

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

    QString currentDateTime = QDateTime::currentDateTime().toString("yyyy/MM/dd hh:mm:ss");

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

    QString currentDateTime = QDateTime::currentDateTime().toString("yyyy/MM/dd hh:mm:ss");;

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
//-----------------------------------------------------------------------------------------------------
FileChecksum::VerificationResult FileChecksum::verifyChecksum(
    const QString &filePath,
    const QString &expectedChecksum,
    QCryptographicHash::Algorithm algorithm,
    std::function<void(qint64, qint64)> progressCallback)
{
    VerificationResult result;
    result.expectedChecksum = expectedChecksum;

    // Check file exists
    if (!QFileInfo::exists(filePath)) {
        result.success = false;
        result.match = false;
        result.errorMessage = "File not found";
        return result;
    }

    // Calculate actual checksum
    QString actualChecksum = calculateChecksum(filePath, algorithm, progressCallback);

    if (actualChecksum.isEmpty()) {
        result.success = false;
        result.match = false;
        result.errorMessage = "Failed to calculate checksum";
        return result;
    }

    // Compare
    result.success = true;
    result.actualChecksum = actualChecksum;
    result.match = (actualChecksum.toLower() == expectedChecksum.toLower());

    return result;
}
//-----------------------------------------------------------------------------------------------------
QString FileChecksum::getFileChecksum(const QString &connectionName, int catalogId,
                                      const QString &fileName, const QString &folderPath)
{
    QSqlQuery query(QSqlDatabase::database(connectionName));
    query.prepare("SELECT checksum_sha256 FROM file "
                  "WHERE file_catalog_id = :catalog_id "
                  "AND file_name = :file_name "
                  "AND file_folder_path = :folder_path");
    query.bindValue(":catalog_id", catalogId);
    query.bindValue(":file_name", fileName);
    query.bindValue(":folder_path", folderPath);

    if (query.exec() && query.next()) {
        return query.value(0).toString();
    }

    return QString(); // Empty = no checksum
}
//-----------------------------------------------------------------------------------------------------
int FileChecksum::countFilesWithChecksum(const QString &connectionName, int catalogId)
{
    QSqlQuery query(QSqlDatabase::database(connectionName));
    query.prepare("SELECT COUNT(*) FROM file "
                  "WHERE file_catalog_id = :file_catalog_id "
                  "AND checksum_sha256 IS NOT NULL "
                  "AND checksum_sha256 != ''");
    query.bindValue(":file_catalog_id", catalogId);

    if (query.exec() && query.next()) {
        return query.value(0).toInt();
        qDebug() << "FileChecksum::countFilesWithChecksum - Count for catalog"
                 << catalogId << "is" << query.value(0).toInt();
    }
    else {
        qDebug() << "FileChecksum::countFilesWithChecksum - Query failed:" << query.lastError().text();
    }

    return 0;
}
//-----------------------------------------------------------------------------------------------------
FileChecksum::CatalogVerificationResult FileChecksum::verifyCatalogChecksums(
    const QString &connectionName,
    int catalogId,
    const QString &catalogSourcePath,
    std::function<bool()> shouldContinue,
    std::function<void(int, int, const QString&)> progressCallback)
{
    CatalogVerificationResult result;
    result.totalFiles = 0;
    result.verified = 0;
    result.mismatches = 0;
    result.missing = 0;

    // Query files with checksums
    QSqlQuery query(QSqlDatabase::database(connectionName));
    query.prepare("SELECT file_name, file_folder_path, checksum_sha256 "
                  "FROM file "
                  "WHERE file_catalog_id = :catalog_id "
                  "AND checksum_sha256 IS NOT NULL "
                  "AND checksum_sha256 != ''");
    query.bindValue(":catalog_id", catalogId);

    if (!query.exec()) {
        qDebug() << "Failed to query files for verification:" << query.lastError().text();
        return result;
    }

    // Count total files
    QSqlQuery countQuery(QSqlDatabase::database(connectionName));
    countQuery.prepare("SELECT COUNT(*) FROM file "
                       "WHERE file_catalog_id = :catalog_id "
                       "AND checksum_sha256 IS NOT NULL "
                       "AND checksum_sha256 != ''");
    countQuery.bindValue(":catalog_id", catalogId);
    if (countQuery.exec() && countQuery.next()) {
        result.totalFiles = countQuery.value(0).toInt();
    }

    int processed = 0;

    while (query.next()) {
        // Check for stop
        if (shouldContinue && !shouldContinue()) {
            qDebug() << "Checksum verification stopped by user";
            break;
        }

        QString fileName = query.value(0).toString();
        QString folderPath = query.value(1).toString();
        QString expectedChecksum = query.value(2).toString();

        QString filePath = folderPath + "/" + fileName;

        processed++;

        // Progress callback
        if (progressCallback) {
            progressCallback(processed, result.totalFiles, fileName);
        }

        // Check if file exists
        if (!QFileInfo::exists(filePath)) {
            result.missing++;
            result.missingFiles << filePath;
            continue;
        }

        // Calculate actual checksum (no progress callback for individual files)
        QString actualChecksum = calculateChecksum(filePath, QCryptographicHash::Sha256, nullptr);

        if (actualChecksum.isEmpty()) {
            qDebug() << "Failed to calculate checksum for:" << filePath;
            continue;
        }

        // Compare
        if (actualChecksum.toLower() == expectedChecksum.toLower()) {
            result.verified++;
        } else {
            result.mismatches++;
            result.mismatchedFiles << filePath;
        }
    }

    return result;
}
//-----------------------------------------------------------------------------------------------------
