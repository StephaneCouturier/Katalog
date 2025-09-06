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
// File Name:   filemetadata.cpp
// Purpose:     Class for extracting and managing file metadata
// Description: Provides metadata extraction using KFileMetaData for media files
// Author:      Stephane Couturier
/////////////////////////////////////////////////////////////////////////////
*/

#include "filemetadata.h"
#include <QFileInfo>
#include <QSqlQuery>
#include <QSqlDatabase>
#include <QDebug>
#include <QDateTime>
#include <QJsonDocument>
#include <QJsonObject>
#include <qjsonarray.h>
#include "catalog.h"

FileMetadata::FileMetadata(QObject *parent) : QObject(parent)
{
}

// Manager to extract and store metadata in database
bool FileMetadata::extractAndStore(const QString &filePath,
                                   const QString &connectionName,
                                   int catalogId,
                                   QString includeMetadata)
{
    QFileInfo fileInfo(filePath);
    if (!fileInfo.exists() || !fileInfo.isReadable()) {
        qDebug() << "FileMetadata::extractAndStore - File not accessible:" << filePath;
        return false;
    }

    // Extract metadata using the main extraction method
    QVariantMap metadata = extractMetadata(filePath, includeMetadata);

    // Process is complete if no metadata was extracted (METADATA_NONE case)
    if (metadata.isEmpty()) {
        return true;
    }

    // Store in database
    return updateFileMetadata(connectionName, catalogId,
                              fileInfo.fileName(),
                              fileInfo.absolutePath(),
                              metadata);
}

//Extract metadata
QVariantMap FileMetadata::extractMetadata(const QString &filePath, QString includeMetadata)
{
    QVariantMap result;

    // Handle METADATA_NONE case - return empty
    if (includeMetadata == Catalog::METADATA_NONE) {
        return result;
    }

    try {
        QFileInfo fileInfo(filePath);
        if (!fileInfo.exists() || !fileInfo.isReadable()) {
            return result;
        }

        // Get MIME type once
        QMimeDatabase mimeDb;
        QString mimeType = mimeDb.mimeTypeForFile(filePath).name();
        QString fileType = getFileType(mimeType);

        // Add basic info for all levels except NONE
        result["file_type"] = fileType;
        result["mime_type"] = mimeType;
        result["metadata_extraction_date"] = QDateTime::currentDateTime().toString(Qt::ISODate);

        // For METADATA_MIME_ONLY, just return basic info
        if (includeMetadata == Catalog::METADATA_MIME_ONLY) {
            return result;
        }

        // For MEDIA_BASIC, EXTENDED_CUSTOM, and EXTENDED_FULL: extract structured metadata
        if (includeMetadata == Catalog::METADATA_MEDIA_BASIC ||
            includeMetadata == Catalog::METADATA_EXTENDED_CUSTOM ||
            includeMetadata == Catalog::METADATA_EXTENDED_FULL) {

            // Try to extract metadata using KFileMetaData
            KFileMetaData::ExtractorCollection extractors;
            auto extractorsList = extractors.fetchExtractors(mimeType);

            if (!extractorsList.isEmpty()) {
                // Extract metadata with metadata-only level for performance
                KFileMetaData::SimpleExtractionResult extractionResult(filePath, mimeType,
                                                                       KFileMetaData::ExtractionResult::ExtractMetaData);

                for (auto extractor : extractorsList) {
                    extractor->extract(&extractionResult);
                }

                auto properties = extractionResult.properties();
                if (!properties.isEmpty()) {
                    // Process metadata based on file type
                    if (fileType == "image") {
                        QVariantMap imageData = processImageMetadata(properties);
                        for (auto it = imageData.begin(); it != imageData.end(); ++it) {
                            result[it.key()] = it.value();
                        }
                    }
                    else if (fileType == "video") {
                        QVariantMap videoData = processVideoMetadata(properties);
                        for (auto it = videoData.begin(); it != videoData.end(); ++it) {
                            result[it.key()] = it.value();
                        }
                    }
                    else if (fileType == "audio") {
                        QVariantMap audioData = processAudioMetadata(properties);
                        for (auto it = audioData.begin(); it != audioData.end(); ++it) {
                            result[it.key()] = it.value();
                        }
                    }

                    // For EXTENDED_FULL level, also extract ALL metadata as JSON
                    if (includeMetadata == Catalog::METADATA_EXTENDED_FULL) {
                        QVariantMap extendedMetadata = extractExtendedMetadata(properties);
                        if (!extendedMetadata.isEmpty()) {
                            QString jsonMetadata = convertMetadataToJson(extendedMetadata);
                            result["metadata_extended"] = jsonMetadata;
                        }
                    }
                }
            }
        }

    } catch (const std::exception& e) {
        qDebug() << "FileMetadata::extractMetadata - Exception:" << e.what();
    }

    return result;
}

QVariantMap FileMetadata::extractExtendedMetadata(const KFileMetaData::PropertyMultiMap &properties)
{
    QVariantMap extendedData;

    // Extract ALL properties available from KFileMetaData
    for (auto it = properties.begin(); it != properties.end(); ++it) {
        KFileMetaData::Property::Property property = it.key();
        QVariant value = it.value();

        // Get property name using PropertyInfo
        KFileMetaData::PropertyInfo info(property);
        QString propertyName = info.name();

        // Store the property with its name as key
        if (!value.isNull() && !value.toString().isEmpty()) {
            // Handle multiple values for the same property
            if (extendedData.contains(propertyName)) {
                // Convert to list if multiple values exist
                QVariant existing = extendedData[propertyName];
                if (existing.type() == QVariant::List) {
                    QVariantList list = existing.toList();
                    list.append(value);
                    extendedData[propertyName] = list;
                } else {
                    QVariantList list;
                    list.append(existing);
                    list.append(value);
                    extendedData[propertyName] = list;
                }
            } else {
                extendedData[propertyName] = value;
            }
        }
    }

    return extendedData;
}

QString FileMetadata::convertMetadataToJson(const QVariantMap &extendedMetadata)
{
    QJsonObject jsonObject;

    // Convert QVariantMap to QJsonObject
    for (auto it = extendedMetadata.begin(); it != extendedMetadata.end(); ++it) {
        QString key = it.key();
        QVariant value = it.value();

        // Convert different value types to appropriate JSON types
        if (value.type() == QVariant::List) {
            QJsonArray jsonArray;
            QVariantList list = value.toList();
            for (const QVariant &item : list) {
                jsonArray.append(QJsonValue::fromVariant(item));
            }
            jsonObject[key] = jsonArray;
        } else {
            jsonObject[key] = QJsonValue::fromVariant(value);
        }
    }

    // Convert to JSON string
    QJsonDocument doc(jsonObject);
    return doc.toJson(QJsonDocument::Compact);
}

//Update existing file record with metadata
bool FileMetadata::updateFileMetadata(const QString &connectionName,
                                      int catalogId,
                                      const QString &fileName,
                                      const QString &folderPath,
                                      const QVariantMap &metadata)
{
    QSqlDatabase database = QSqlDatabase::database(connectionName);
    if (!database.isOpen()) {
        qDebug() << "FileMetadata::updateFileMetadata - Database not open";
        return false;
    }

    // Build UPDATE query dynamically based on available metadata
    QStringList setParts;
    QVariantList values;

    // Basic metadata
    if (metadata.contains("file_type")) {
        setParts << "file_type = ?";
        values << metadata["file_type"];
    }
    if (metadata.contains("mime_type")) {
        setParts << "mime_type = ?";
        values << metadata["mime_type"];
    }

    // Image metadata
    if (metadata.contains("image_width")) {
        setParts << "image_width = ?";
        values << metadata["image_width"];
    }
    if (metadata.contains("image_height")) {
        setParts << "image_height = ?";
        values << metadata["image_height"];
    }
    if (metadata.contains("image_orientation")) {
        setParts << "image_orientation = ?";
        values << metadata["image_orientation"];
    }

    // Video metadata
    if (metadata.contains("video_duration_seconds")) {
        setParts << "video_duration_seconds = ?";
        values << metadata["video_duration_seconds"];
    }
    if (metadata.contains("video_width")) {
        setParts << "video_width = ?";
        values << metadata["video_width"];
    }
    if (metadata.contains("video_height")) {
        setParts << "video_height = ?";
        values << metadata["video_height"];
    }
    if (metadata.contains("video_codec")) {
        setParts << "video_codec = ?";
        values << metadata["video_codec"];
    }
    if (metadata.contains("video_framerate")) {
        setParts << "video_framerate = ?";
        values << metadata["video_framerate"];
    }
    if (metadata.contains("video_bitrate")) {
        setParts << "video_bitrate = ?";
        values << metadata["video_bitrate"];
    }

    // Audio metadata
    if (metadata.contains("audio_duration_seconds")) {
        setParts << "audio_duration_seconds = ?";
        values << metadata["audio_duration_seconds"];
    }
    if (metadata.contains("audio_artist")) {
        setParts << "audio_artist = ?";
        values << metadata["audio_artist"];
    }
    if (metadata.contains("audio_album")) {
        setParts << "audio_album = ?";
        values << metadata["audio_album"];
    }
    if (metadata.contains("audio_title")) {
        setParts << "audio_title = ?";
        values << metadata["audio_title"];
    }
    if (metadata.contains("audio_genre")) {
        setParts << "audio_genre = ?";
        values << metadata["audio_genre"];
    }
    if (metadata.contains("audio_year")) {
        setParts << "audio_year = ?";
        values << metadata["audio_year"];
    }
    if (metadata.contains("audio_track_number")) {
        setParts << "audio_track_number = ?";
        values << metadata["audio_track_number"];
    }
    if (metadata.contains("audio_bitrate")) {
        setParts << "audio_bitrate = ?";
        values << metadata["audio_bitrate"];
    }
    if (metadata.contains("audio_sample_rate")) {
        setParts << "audio_sample_rate = ?";
        values << metadata["audio_sample_rate"];
    }

    // Extended metadata (JSON)
    if (metadata.contains("metadata_extended")) {
        setParts << "metadata_extended = ?";
        values << metadata["metadata_extended"];
    }

    // Extraction date
    if (metadata.contains("metadata_extraction_date")) {
        setParts << "metadata_extraction_date = ?";
        values << metadata["metadata_extraction_date"];
    }

    if (setParts.isEmpty()) {
        // No metadata to update
        return true;
    }

    // Build and execute query
    QString queryString = QString("UPDATE file SET %1 WHERE file_catalog_id = ? AND file_name = ? AND file_folder_path = ?")
                              .arg(setParts.join(", "));

    values << catalogId << fileName << folderPath;

    QSqlQuery query(database);
    query.prepare(queryString);

    for (const QVariant &value : values) {
        query.addBindValue(value);
    }

    if (!query.exec()) {
        qDebug() << "FileMetadata::updateFileMetadata - Query failed:" << query.lastError().text();
        qDebug() << "Query was:" << queryString;
        return false;
    }

    return true;
}

//Check if file type supports metadata extraction
bool FileMetadata::isMetadataSupported(const QString &filePath)
{
    QMimeDatabase mimeDb;
    QString mimeType = mimeDb.mimeTypeForFile(filePath).name();

    KFileMetaData::ExtractorCollection extractors;
    auto extractorsList = extractors.fetchExtractors(mimeType);

    return !extractorsList.isEmpty();
}

//Get supported file extensions
QStringList FileMetadata::getSupportedExtensions()
{
    // This is a simplified list - in practice you'd query KFileMetaData for all supported types
    return {"jpg", "jpeg", "png", "gif", "bmp", "tiff", "webp",  // Images
            "mp4", "mkv", "avi", "mov", "wmv", "flv", "webm",     // Videos
            "mp3", "ogg", "flac", "wav", "aac", "m4a", "wma"};   // Audio
}

void FileMetadata::testExtendedMetadata(const QString &filePath)
{
    // Test all metadata levels
    QStringList levels = {
        Catalog::METADATA_NONE,
        Catalog::METADATA_MIME_ONLY,
        Catalog::METADATA_MEDIA_BASIC,
        Catalog::METADATA_EXTENDED_CUSTOM,
        Catalog::METADATA_EXTENDED_FULL
    };

    for (const QString &level : levels) {
        qDebug() << "\n--- Testing metadata level:" << level << "---";

        QVariantMap metadata = extractMetadata(filePath, level);
    }
}

// Helper methods

QString FileMetadata::getFileType(const QString &mimeType)
{
    if (mimeType.startsWith("image/")) {
        return "image";
    } else if (mimeType.startsWith("video/")) {
        return "video";
    } else if (mimeType.startsWith("audio/")) {
        return "audio";
    }
    return "other";
}

QVariantMap FileMetadata::processImageMetadata(const KFileMetaData::PropertyMultiMap &properties)
{
    QVariantMap result;

    // Image dimensions
    if (properties.contains(KFileMetaData::Property::Width)) {
        result["image_width"] = properties.value(KFileMetaData::Property::Width).toInt();
    }
    if (properties.contains(KFileMetaData::Property::Height)) {
        result["image_height"] = properties.value(KFileMetaData::Property::Height).toInt();
    }
    if (properties.contains(KFileMetaData::Property::ImageOrientation)) {
        result["image_orientation"] = properties.value(KFileMetaData::Property::ImageOrientation).toInt();
    }

    return result;
}

QVariantMap FileMetadata::processVideoMetadata(const KFileMetaData::PropertyMultiMap &properties)
{
    QVariantMap result;

    // Video duration
    if (properties.contains(KFileMetaData::Property::Duration)) {
        result["video_duration_seconds"] = properties.value(KFileMetaData::Property::Duration).toInt();
    }

    // Video dimensions
    if (properties.contains(KFileMetaData::Property::Width)) {
        result["video_width"] = properties.value(KFileMetaData::Property::Width).toInt();
    }
    if (properties.contains(KFileMetaData::Property::Height)) {
        result["video_height"] = properties.value(KFileMetaData::Property::Height).toInt();
    }

    // Video codec
    if (properties.contains(KFileMetaData::Property::VideoCodec)) {
        result["video_codec"] = properties.value(KFileMetaData::Property::VideoCodec).toString();
    }

    // Frame rate
    if (properties.contains(KFileMetaData::Property::FrameRate)) {
        result["video_framerate"] = properties.value(KFileMetaData::Property::FrameRate).toDouble();
    }

    // Bitrate
    if (properties.contains(KFileMetaData::Property::BitRate)) {
        result["video_bitrate"] = properties.value(KFileMetaData::Property::BitRate).toInt();
    }

    return result;
}

QVariantMap FileMetadata::processAudioMetadata(const KFileMetaData::PropertyMultiMap &properties)
{
    QVariantMap result;

    // Audio duration
    if (properties.contains(KFileMetaData::Property::Duration)) {
        result["audio_duration_seconds"] = properties.value(KFileMetaData::Property::Duration).toInt();
    }

    // Audio artist
    if (properties.contains(KFileMetaData::Property::Artist)) {
        result["audio_artist"] = properties.value(KFileMetaData::Property::Artist).toString();
    }

    // Audio album
    if (properties.contains(KFileMetaData::Property::Album)) {
        result["audio_album"] = properties.value(KFileMetaData::Property::Album).toString();
    }

    // Audio title
    if (properties.contains(KFileMetaData::Property::Title)) {
        result["audio_title"] = properties.value(KFileMetaData::Property::Title).toString();
    }

    // Audio genre
    if (properties.contains(KFileMetaData::Property::Genre)) {
        result["audio_genre"] = properties.value(KFileMetaData::Property::Genre).toString();
    }

    // Release year
    if (properties.contains(KFileMetaData::Property::ReleaseYear)) {
        result["audio_year"] = properties.value(KFileMetaData::Property::ReleaseYear).toInt();
    }

    // Track number
    if (properties.contains(KFileMetaData::Property::TrackNumber)) {
        result["audio_track_number"] = properties.value(KFileMetaData::Property::TrackNumber).toInt();
    }

    // Bitrate
    if (properties.contains(KFileMetaData::Property::BitRate)) {
        result["audio_bitrate"] = properties.value(KFileMetaData::Property::BitRate).toInt();
    }

    // Sample rate
    if (properties.contains(KFileMetaData::Property::SampleRate)) {
        result["audio_sample_rate"] = properties.value(KFileMetaData::Property::SampleRate).toInt();
    }

    return result;
}

QString FileMetadata::getExtendedMetadataJson(const QString &filePath)
{
    QVariantMap metadata = extractMetadata(filePath, Catalog::METADATA_EXTENDED_FULL);

    if (metadata.contains("metadata_extended")) {
        return metadata["metadata_extended"].toString();
    }

    return QString("No extended metadata available for this file");
}
