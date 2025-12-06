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
// File Name:   filechecksum.h
// Purpose:     Class for calculating and managing file checksums
// Description: Provides checksum calculation using Qt QCryptographicHash
// Author:      Stephane Couturier
/////////////////////////////////////////////////////////////////////////////
*/

#ifndef FILECHECKSUM_H
#define FILECHECKSUM_H

#include <QObject>
#include <QString>
#include <QVariant>
#include <QSqlError>
#include <QCryptographicHash>
#include <functional>

/**
 * @brief The FileChecksum class
 * Handles calculation and storage of file checksums using Qt QCryptographicHash
 */
class FileChecksum : public QObject
{
    Q_OBJECT

public:
    explicit FileChecksum(QObject *parent = nullptr);

    // Main calculation and storage method
    static bool calculateAndStore(const QString &filePath,
                                  const QString &connectionName,
                                  int catalogId,
                                  QString includeChecksum);

    // Calculate checksum without storing (for testing/preview)
    static QString calculateChecksum(const QString &filePath,
                                     QCryptographicHash::Algorithm algorithm,
                                     std::function<void(qint64, qint64)> progressCallback = nullptr);

    // Update existing file record with checksum
    static bool updateFileChecksum(const QString &connectionName,
                                   int catalogId,
                                   const QString &fileName,
                                   const QString &folderPath,
                                   const QString &checksumValue,
                                   const QString &checksumType);

    // Batch update checksums (for performance)
    static bool batchUpdateFileChecksum(const QString &connectionName,
                                        int catalogId,
                                        const QStringList &fileNames,
                                        const QStringList &folderPaths,
                                        const QStringList &checksumValues);

    //Utility to get algorithm from string
    static QCryptographicHash::Algorithm getAlgorithmFromString(const QString &algorithmName);

signals:
    void checksumCalculated(const QString &filePath);
    void calculationError(const QString &filePath, const QString &error);
};

#endif // FILECHECKSUM_H
