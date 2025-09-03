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
// File Name:   filemetadata.h
// Purpose:     Class for extracting and managing file metadata
// Description: Provides metadata extraction using KFileMetaData for media files
// Author:      Stephane Couturier
/////////////////////////////////////////////////////////////////////////////
*/

#ifndef FILEMETADATA_H
#define FILEMETADATA_H

#include <QObject>
#include <QString>
#include <QVariant>
#include <QSqlError>
#include <QMimeDatabase>
#include <KFileMetaData/ExtractorCollection>
#include <KFileMetaData/SimpleExtractionResult>
#include <KFileMetaData/Properties>
#include <KFileMetaData/PropertyInfo>

/**
 * @brief The FileMetadata class
 * Handles extraction and storage of media file metadata using KFileMetaData
 */
class FileMetadata : public QObject
{
    Q_OBJECT

public:
    explicit FileMetadata(QObject *parent = nullptr);

    // Main extraction and storage method
    static bool extractAndStore(const QString &filePath,
                                const QString &connectionName,
                                int catalogId);

    // Extract metadata without storing (for testing/preview)
    static QVariantMap extractMetadata(const QString &filePath);

    // Update existing file record with metadata
    static bool updateFileMetadata(const QString &connectionName,
                                   int catalogId,
                                   const QString &fileName,
                                   const QString &folderPath,
                                   const QVariantMap &metadata);

    // Check if file type supports metadata extraction
    static bool isMetadataSupported(const QString &filePath);

    // Get supported file extensions
    static QStringList getSupportedExtensions();

private:
    // Helper methods
    static QString getFileType(const QString &mimeType);
    static QVariantMap processImageMetadata(const KFileMetaData::PropertyMultiMap &properties);
    static QVariantMap processVideoMetadata(const KFileMetaData::PropertyMultiMap &properties);
    static QVariantMap processAudioMetadata(const KFileMetaData::PropertyMultiMap &properties);

    // Database helper
    static bool storeMetadataToDatabase(const QString &connectionName,
                                        int catalogId,
                                        const QString &fileName,
                                        const QString &folderPath,
                                        const QString &fileType,
                                        const QString &mimeType,
                                        const QVariantMap &metadata);

signals:
    void metadataExtracted(const QString &filePath);
    void extractionError(const QString &filePath, const QString &error);
};

#endif // FILEMETADATA_H
