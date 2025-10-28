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

#include "catalog.h"
#include "filemetadata.h"
#include <QFileInfo>
#include <QSqlQuery>
#include <QSqlDatabase>
#include <QDebug>
#include <QDateTime>
#include <QJsonDocument>
#include <QJsonObject>
#include <qelapsedtimer.h>
#include <qjsonarray.h>

//-----------------------------------------------------------------------------------------------------
FileMetadata::FileMetadata(QObject *parent) : QObject(parent)
{
}
//-----------------------------------------------------------------------------------------------------
// Extensions to File Type cache - created once at first use
QHash<QString, QString> FileMetadata::s_extensionToTypeCache;
bool FileMetadata::s_typeCacheInitialized = false;
QSet<QString> FileMetadata::s_supportedExtensionsCache;
bool FileMetadata::s_cacheInitialized = false;

// ExtractorCollection cache - created once at first use
KFileMetaData::ExtractorCollection* FileMetadata::s_extractorCollection = nullptr;
QMutex FileMetadata::s_extractorMutex;
//-----------------------------------------------------------------------------------------------------
KFileMetaData::ExtractorCollection* FileMetadata::getCachedExtractorCollection()
{
    QMutexLocker lock(&s_extractorMutex);

    if (!s_extractorCollection) {
        s_extractorCollection = new KFileMetaData::ExtractorCollection();
        qDebug() << "ExtractorCollection created and cached for performance";
    }

    return s_extractorCollection;
}
//-----------------------------------------------------------------------------------------------------
void FileMetadata::initializeExtensionTypeCache()
{
    if (s_typeCacheInitialized) {
        qDebug() << "Extension->Type cache already initialized with" << s_extensionToTypeCache.size() << "mappings";
        return;
    }

    QMimeDatabase mimeDb;
    KFileMetaData::ExtractorCollection extractors;

    qDebug() << "Building extension->type cache from MIME database...";

    // Build mapping from MIME types to file types
    const auto allMimeTypes = mimeDb.allMimeTypes();
    for (const auto& mimeType : allMimeTypes) {
        QString mimeTypeName = mimeType.name();
        QString fileType = getFileTypeFromMime(mimeTypeName);

        // Add all extensions for this MIME type
        const auto suffixes = mimeType.suffixes();
        for (const QString& suffix : suffixes) {
            QString lowerSuffix = suffix.toLower();

            // Only override if we have a more specific type (not "other")
            if (!s_extensionToTypeCache.contains(lowerSuffix) ||
                s_extensionToTypeCache[lowerSuffix] == "other") {
                s_extensionToTypeCache[lowerSuffix] = fileType;
            }
        }
    }

    s_typeCacheInitialized = true;
    qDebug() << "Extension->Type cache initialized with" << s_extensionToTypeCache.size() << "mappings";

    // Debug output of some common extensions
    QStringList sampleExts = {"jpg", "mp3", "mp4", "txt", "pdf", "doc", "flac", "m4a"};
    for (const QString& ext : sampleExts) {
        if (s_extensionToTypeCache.contains(ext)) {
            qDebug() << "  " << ext << "->" << s_extensionToTypeCache[ext];
        }
    }
}
//-----------------------------------------------------------------------------------------------------
QString FileMetadata::getMimeTypeFromExtension(const QString &extension)
{
    // Build a comprehensive extension->MIME mapping
    // This avoids file content reading entirely
    static QHash<QString, QString> extMimeMap = {
        // Images
        {"jpg", "image/jpeg"}, {"jpeg", "image/jpeg"},
        {"png", "image/png"},
        {"gif", "image/gif"},
        {"bmp", "image/bmp"},
        {"tiff", "image/tiff"}, {"tif", "image/tiff"},
        {"webp", "image/webp"},
        {"svg", "image/svg+xml"},
        {"ico", "image/x-icon"},
        {"heic", "image/heic"}, {"heif", "image/heif"},
        {"raw", "image/x-raw"},
        {"xcf", "image/x-xcf"},  // GIMP

        // Video
        {"mp4", "video/mp4"},
        {"avi", "video/x-msvideo"},
        {"mkv", "video/x-matroska"},
        {"mov", "video/quicktime"},
        {"wmv", "video/x-ms-wmv"},
        {"flv", "video/x-flv"},
        {"webm", "video/webm"},
        {"m4v", "video/x-m4v"},
        {"mpg", "video/mpeg"}, {"mpeg", "video/mpeg"},
        {"3gp", "video/3gpp"},
        {"ogv", "video/ogg"},
        {"vob", "video/x-ms-vob"},

        // Audio
        {"mp3", "audio/mpeg"},
        {"wav", "audio/wav"},
        {"flac", "audio/flac"},
        {"ogg", "audio/ogg"}, {"oga", "audio/ogg"},
        {"m4a", "audio/mp4"},
        {"aac", "audio/aac"},
        {"wma", "audio/x-ms-wma"},
        {"opus", "audio/opus"},
        {"aiff", "audio/aiff"}, {"aif", "audio/aiff"},
        {"mid", "audio/midi"}, {"midi", "audio/midi"},
        {"amr", "audio/amr"},

        // Documents
        {"pdf", "application/pdf"},
        {"doc", "application/msword"},
        {"docx", "application/vnd.openxmlformats-officedocument.wordprocessingml.document"},
        {"xls", "application/vnd.ms-excel"},
        {"xlsx", "application/vnd.openxmlformats-officedocument.spreadsheetml.sheet"},
        {"ppt", "application/vnd.ms-powerpoint"},
        {"pptx", "application/vnd.openxmlformats-officedocument.presentationml.presentation"},
        {"odt", "application/vnd.oasis.opendocument.text"},
        {"ods", "application/vnd.oasis.opendocument.spreadsheet"},
        {"odp", "application/vnd.oasis.opendocument.presentation"},
        {"rtf", "application/rtf"},
        {"tex", "application/x-tex"},

        // Text
        {"txt", "text/plain"},
        {"html", "text/html"}, {"htm", "text/html"},
        {"xml", "text/xml"},
        {"css", "text/css"},
        {"js", "text/javascript"},
        {"json", "application/json"},
        {"csv", "text/csv"},
        {"md", "text/markdown"},
        {"yaml", "text/yaml"}, {"yml", "text/yaml"},

        // Archives
        {"zip", "application/zip"},
        {"rar", "application/x-rar-compressed"},
        {"7z", "application/x-7z-compressed"},
        {"tar", "application/x-tar"},
        {"gz", "application/gzip"},
        {"bz2", "application/x-bzip2"},
        {"xz", "application/x-xz"},

        // Executables
        {"exe", "application/x-msdownload"},
        {"msi", "application/x-msi"},
        {"deb", "application/x-debian-package"},
        {"rpm", "application/x-rpm"},
        {"dmg", "application/x-apple-diskimage"},
        {"app", "application/x-executable"},

        // Code
        {"cpp", "text/x-c++src"}, {"cc", "text/x-c++src"}, {"cxx", "text/x-c++src"},
        {"h", "text/x-chdr"}, {"hpp", "text/x-c++hdr"},
        {"c", "text/x-csrc"},
        {"py", "text/x-python"},
        {"java", "text/x-java"},
        {"sh", "text/x-shellscript"},
        {"bat", "text/x-msdos-batch"},
        {"pl", "text/x-perl"},
        {"rb", "text/x-ruby"},
        {"php", "text/x-php"},
        {"go", "text/x-go"},
        {"rs", "text/x-rust"},
        {"swift", "text/x-swift"},
        {"kt", "text/x-kotlin"},
        {"scala", "text/x-scala"},
        {"r", "text/x-r"},
        {"m", "text/x-objcsrc"},
        {"mm", "text/x-objc++src"},

        // Other
        {"iso", "application/x-iso9660-image"},
        {"torrent", "application/x-bittorrent"},
        {"apk", "application/vnd.android.package-archive"},
        {"epub", "application/epub+zip"},
        {"mobi", "application/x-mobipocket-ebook"},
        {"idx", "application/octet-stream"}  // Katalog index files
    };

    QString lowerExt = extension.toLower();

    // Return the MIME type if found, otherwise make an educated guess
    if (extMimeMap.contains(lowerExt)) {
        return extMimeMap.value(lowerExt);
    }

    // Fallback: guess based on common patterns
    if (lowerExt.startsWith("doc")) return "application/msword";
    if (lowerExt.startsWith("xls")) return "application/vnd.ms-excel";
    if (lowerExt.startsWith("ppt")) return "application/vnd.ms-powerpoint";

    // Default fallback
    return "application/octet-stream";
}
//-----------------------------------------------------------------------------------------------------
QString FileMetadata::getFileTypeFromExtension(const QString &extension)
{
    // Handle extensionless files or empty extensions
    if (extension.isEmpty()) {
        return "none";  // Default for extensionless files
    }

    // Initialize cache if needed
    if (!s_typeCacheInitialized) {
        initializeExtensionTypeCache();
    }

    QString lowerExt = extension.toLower();

    // Use the cache built from KFileMetaData
    if (s_extensionToTypeCache.contains(lowerExt)) {
        return s_extensionToTypeCache.value(lowerExt);
    }

    // Fallback for unknown extensions
    return "other";
}
//-----------------------------------------------------------------------------------------------------
QString FileMetadata::getFileTypeFromMime(const QString &mimeType)
{
    // FIXED: Return lowercase types consistently
    if (mimeType.startsWith("image/")) {
        return "image";
    } else if (mimeType.startsWith("video/")) {
        return "video";
    } else if (mimeType.startsWith("audio/")) {
        return "audio";
    } else if (mimeType.startsWith("text/") ||
               mimeType == "application/pdf" ||
               mimeType == "application/msword" ||
               mimeType == "application/vnd.openxmlformats-officedocument.wordprocessingml.document" ||
               mimeType == "application/vnd.oasis.opendocument.text" ||
               mimeType == "application/rtf" ||
               mimeType == "application/epub+zip") {
        return "text";
    } else {
        return "other";
    }
}
//-----------------------------------------------------------------------------------------------------
QVariantMap FileMetadata::verifyMimeType(const QString &filePath, const QString &currentFileType)
{
    QVariantMap result;

    try {
        QFileInfo fileInfo(filePath);
        if (!fileInfo.exists() || !fileInfo.isReadable()) {
            result["error"] = "File not accessible";
            return result;
        }

        // Detect actual MIME type (slow operation)
        QMimeDatabase mimeDb;
        QString mimeType = mimeDb.mimeTypeForFile(filePath).name();
        QString mimeBasedType = getFileTypeFromMime(mimeType);

        result["mime_type"] = mimeType;
        result["file_type"] = mimeBasedType;
        result["original_type"] = currentFileType;
        result["has_mismatch"] = (currentFileType.toLower() != mimeBasedType.toLower());
        result["verification_date"] = QDateTime::currentDateTime().toString(Qt::ISODate);

        if (result["has_mismatch"].toBool()) {
            qDebug() << "Type mismatch for" << filePath
                     << "- Extension suggested:" << currentFileType
                     << "- MIME detected:" << mimeBasedType;
        }

    } catch (const std::exception& e) {
        result["error"] = QString("Exception: %1").arg(e.what());
    }

    return result;
}
//-----------------------------------------------------------------------------------------------------
const QHash<QString, QString>& FileMetadata::getExtensionToTypeCache()
{
    // Initialize cache if needed
    if (!s_typeCacheInitialized) {
        initializeExtensionTypeCache();
    }

    return s_extensionToTypeCache;
}
//-----------------------------------------------------------------------------------------------------
bool FileMetadata::batchUpdateFileMetadata(const QString &connectionName,
                                           int catalogId,
                                           const QStringList &fileNames,
                                           const QStringList &folderPaths,
                                           const QList<QVariantMap> &metadataList)
{
    if (fileNames.isEmpty() || fileNames.size() != metadataList.size()) {
        return false;
    }

    QSqlDatabase database = QSqlDatabase::database(connectionName);
    if (!database.isOpen()) {
        qDebug() << "FileMetadata::batchUpdateFileMetadata - Database not open";
        return false;
    }

    // Collect all unique metadata keys across all files
    QSet<QString> allKeys;
    for (const auto& metadata : metadataList) {
        for (auto it = metadata.begin(); it != metadata.end(); ++it) {
            allKeys.insert(it.key());
        }
    }

    if (allKeys.isEmpty()) {
        return true;  // Nothing to update
    }

    // Start transaction for batch update
    QSqlQuery txQuery(database);
    if (!txQuery.exec("BEGIN TRANSACTION")) {
        qDebug() << "Could not start transaction:" << txQuery.lastError().text();
    }

    // Update each metadata field with CASE statement covering all files
    for (const QString& key : allKeys) {
        // NOTE: This used to skip file_type, mime_type, metadata_extraction_date
        // But these need to be updated for existing files getting metadata for the first time
        // Only truly skip if it's empty/not needed

        QString caseClause = QString("CASE");
        QVariantList bindValues;

        for (int i = 0; i < fileNames.size(); ++i) {
            if (metadataList[i].contains(key)) {
                caseClause += " WHEN (file_catalog_id = ? AND file_name = ? AND file_folder_path = ?) THEN ?";
                bindValues << catalogId << fileNames[i] << folderPaths[i] << metadataList[i][key];
            }
        }

        if (bindValues.isEmpty()) {
            continue;  // No values for this key, skip
        }

        caseClause += QString(" ELSE %1 END").arg(key);

        QString updateQuery = QString("UPDATE file SET %1 = %2 WHERE file_catalog_id = ?")
                                  .arg(key, caseClause);
        bindValues << catalogId;

        QSqlQuery query(database);
        query.prepare(updateQuery);

        const QVariantList& constBindValues = bindValues;
        for (const QVariant& value : constBindValues) {
            query.addBindValue(value);
        }

        if (!query.exec()) {
            qDebug() << "Batch update failed for key" << key << ":" << query.lastError().text();
            txQuery.exec("ROLLBACK");
            return false;
        }
    }

    // Commit transaction
    if (!txQuery.exec("COMMIT")) {
        qDebug() << "Could not commit transaction:" << txQuery.lastError().text();
        return false;
    }

    qDebug() << "Batch metadata update: completed for" << fileNames.size() << "files";
    return true;
}
//-----------------------------------------------------------------------------------------------------
void FileMetadata::migrateFileTypesForCatalog(const QString &connectionName,
                                              int catalogId,
                                              std::function<void(int, int, QString)> progressCallback,
                                              std::function<bool()> shouldContinueCallback)
{
    QSqlDatabase database = QSqlDatabase::database(connectionName);
    if (!database.isOpen()) {
        qDebug() << "FileMetadata::migrateFileTypesForCatalog - Database not open";
        return;
    }

    qDebug() << "=== FILE TYPE MIGRATION (extension-based) - NO metadata extraction ===";

    // Step 1: Check if migration is needed
    QSqlQuery checkQuery(database);
    checkQuery.prepare(R"(
        SELECT COUNT(*)
        FROM file
        WHERE file_catalog_id = :catalog_id
        AND (mime_type IS NULL OR mime_type = ''
             OR file_type IS NULL OR file_type = ''
             OR file_extension IS NULL OR file_extension = '')
    )");
    checkQuery.bindValue(":catalog_id", catalogId);

    if (!checkQuery.exec() || !checkQuery.next()) {
        qDebug() << "Failed to check migration status";
        return;
    }

    int totalFiles = checkQuery.value(0).toInt();
    if (totalFiles == 0) {
        qDebug() << "No files need migration";
        return;
    }

    qDebug() << "Starting migration for" << totalFiles << "files (file types only, NO metadata)";
    if (progressCallback) {
        progressCallback(0, totalFiles, "Starting file type conversion...");
    }

    // Step 2: Get all file names that need migration and extract extensions in Qt
    QSqlQuery filesQuery(database);
    filesQuery.prepare(R"(
        SELECT file_name
        FROM file
        WHERE file_catalog_id = :catalog_id
        AND (mime_type IS NULL OR mime_type = ''
             OR file_type IS NULL OR file_type = ''
             OR file_extension IS NULL OR file_extension = '')
    )");
    filesQuery.bindValue(":catalog_id", catalogId);

    if (!filesQuery.exec()) {
        qDebug() << "Failed to query files:" << filesQuery.lastError().text();
        return;
    }

    // Build map of extensions to count
    QHash<QString, int> extensionCounts;
    int extensionlessCount = 0;

    while (filesQuery.next()) {
        QString fileName = filesQuery.value(0).toString();
        QFileInfo fileInfo(fileName);
        QString extension = fileInfo.suffix().toLower();

        if (extension.isEmpty()) {
            extensionlessCount++;
        } else {
            extensionCounts[extension]++;
        }
    }

    // Sort extensions by count (most common first)
    QList<QPair<QString, int>> extensions;
    QHashIterator<QString, int> it(extensionCounts);
    while (it.hasNext()) {
        it.next();
        extensions.append(qMakePair(it.key(), it.value()));
    }
    std::sort(extensions.begin(), extensions.end(),
              [](const QPair<QString, int>& a, const QPair<QString, int>& b) {
                  return a.second > b.second; // Sort by count descending
              });

    qDebug() << "Found" << extensions.size() << "distinct extensions and"
             << extensionlessCount << "extensionless files";

    // Step 3: Begin transaction
    if (!database.transaction()) {
        qDebug() << "Failed to begin transaction";
        return;
    }

    int processedFiles = 0;

    // Step 4: Update files by extension (ONE UPDATE per extension using LIKE pattern)
    for (const auto &extPair : extensions) {
        // Check if we should continue
        if (shouldContinueCallback && !shouldContinueCallback()) {
            database.rollback();
            qDebug() << "Migration stopped by user";
            return;
        }

        QString extension = extPair.first;
        int fileCount = extPair.second;

        // Calculate type and MIME from extension
        QString fileType = getFileTypeFromExtension(extension);
        QString mimeType = getMimeTypeFromExtension(extension);

        // Build LIKE pattern for this extension (case-insensitive)
        QString likePattern = QString("%.%1").arg(extension);

        // Single UPDATE for all files ending with this extension
        QSqlQuery updateQuery(database);
        updateQuery.prepare(R"(
            UPDATE file
            SET file_extension = :extension,
                file_type = :file_type,
                mime_type = :mime_type
            WHERE file_catalog_id = :catalog_id
            AND (mime_type IS NULL OR mime_type = ''
                 OR file_type IS NULL OR file_type = ''
                 OR file_extension IS NULL OR file_extension = '')
            AND LOWER(file_name) LIKE LOWER(:like_pattern)
        )");

        updateQuery.bindValue(":extension", extension);
        updateQuery.bindValue(":file_type", fileType);
        updateQuery.bindValue(":mime_type", mimeType);
        updateQuery.bindValue(":catalog_id", catalogId);
        updateQuery.bindValue(":like_pattern", likePattern);

        if (!updateQuery.exec()) {
            qDebug() << "Failed to update extension" << extension << ":"
                     << updateQuery.lastError().text();
            database.rollback();
            return;
        }

        int rowsAffected = updateQuery.numRowsAffected();
        processedFiles += rowsAffected;

        // Progress callback
        if (progressCallback) {
            QString msg = QString("Converted .%1 files (%2)")
            .arg(extension)
                .arg(rowsAffected);
            progressCallback(processedFiles, totalFiles, msg);
        }
    }

    // Step 5: Handle extensionless files
    if (extensionlessCount > 0) {
        if (shouldContinueCallback && !shouldContinueCallback()) {
            database.rollback();
            qDebug() << "Migration stopped by user";
            return;
        }

        QSqlQuery updateExtensionlessQuery(database);
        updateExtensionlessQuery.prepare(R"(
            UPDATE file
            SET file_extension = '',
                file_type = 'none',
                mime_type = 'application/octet-stream'
            WHERE file_catalog_id = :catalog_id
            AND file_name NOT LIKE '%.%'
            AND (mime_type IS NULL OR mime_type = ''
                 OR file_type IS NULL OR file_type = ''
                 OR file_extension IS NULL OR file_extension = '')
        )");
        updateExtensionlessQuery.bindValue(":catalog_id", catalogId);

        if (!updateExtensionlessQuery.exec()) {
            qDebug() << "Failed to update extensionless files:"
                     << updateExtensionlessQuery.lastError().text();
            database.rollback();
            return;
        }

        int rowsAffected = updateExtensionlessQuery.numRowsAffected();
        processedFiles += rowsAffected;

        if (progressCallback) {
            progressCallback(processedFiles, totalFiles, "Converted extensionless files");
        }
    }

    // Step 6: Commit
    if (!database.commit()) {
        qDebug() << "Failed to commit migration";
        database.rollback();
        return;
    }

    qDebug() << "=== FILE TYPE MIGRATION COMPLETED: " << processedFiles
             << "files (NO metadata was extracted) ===";
    if (progressCallback) {
        progressCallback(processedFiles, totalFiles, "File type conversion completed");
    }
}
//-----------------------------------------------------------------------------------------------------
bool FileMetadata::extractAndStore(const QString &filePath,
                                   const QString &connectionName,
                                   int catalogId,
                                   QString includeMetadata)
{
    static QElapsedTimer timer;
    static int callCount = 0;

    if (callCount == 0) {
        timer.start();
    }
    callCount++;

    QFileInfo fileInfo(filePath);
    if (!fileInfo.exists() || !fileInfo.isReadable()) {
        qDebug() << "  ERROR: File not accessible";
        return false;
    }

    // Extract metadata
    QElapsedTimer extractTimer;
    extractTimer.start();
    QVariantMap metadata = extractMetadata(filePath, includeMetadata);
    int extractMs = extractTimer.elapsed();

    if (metadata.isEmpty()) {
        return true;
    }

    // Store in database
    QElapsedTimer updateTimer;
    updateTimer.start();
    bool result = updateFileMetadata(connectionName, catalogId,
                                     fileInfo.fileName(),
                                     fileInfo.absolutePath(),
                                     metadata);
    int updateMs = updateTimer.elapsed();

    if (callCount % 100 == 0) {
        qDebug() << "Progress:" << callCount << "Extract:" << extractMs << "ms Database:" << updateMs << "ms";
    }

    if (callCount == 5745) {
        qDebug() << "TOTAL TIME:" << timer.elapsed() << "ms for 5745 files";
        qDebug() << "Average per file:" << (timer.elapsed() / 5745) << "ms";
    }

    return result;
}
//-----------------------------------------------------------------------------------------------------
QVariantMap FileMetadata::extractMetadata(const QString &filePath, QString includeMetadata)
{
    static QElapsedTimer timer;
    static int callCount = 0;
    static int totalExtractMs = 0;//, totalUpdateMs = 0;

    if (callCount == 0) timer.start();
    callCount++;

    QVariantMap result;

    // No metadata extraction
    if (includeMetadata == Catalog::METADATA_NONE) {
        qDebug() << "  Skipping: METADATA_NONE";
        return result;
    }

    try {
        QFileInfo fileInfo(filePath);
        if (!fileInfo.exists() || !fileInfo.isReadable()) {
            qDebug() << "  ERROR: File not accessible";
            return result;
        }

        // Get file type and MIME from extension (FAST - no file reading!)
        QString extension = fileInfo.suffix().toLower();
        QString fileType = getFileTypeFromExtension(extension);
        QString guessMimeType = getMimeTypeFromExtension(extension);

        // Always store these basic fields
        result["file_type"] = fileType;
        result["mime_type"] = guessMimeType;
        result["metadata_extraction_date"] = QDateTime::currentDateTime().toString(Qt::ISODate);

        // Check if we should continue with metadata extraction
        bool shouldExtractMetadata = false;

        if (includeMetadata == Catalog::METADATA_MEDIA_BASIC ||
            includeMetadata == Catalog::METADATA_MEDIA_EXTENDED) {
            // Only extract for media files
            shouldExtractMetadata = (fileType == "image" || fileType == "audio" || fileType == "video");
        } else if (includeMetadata == Catalog::METADATA_FULL) {
            // For FULL mode, only extract for known supported types
            // Avoid trying to extract from archives, executables, etc.
            shouldExtractMetadata = (fileType == "image" || fileType == "audio" ||
                                     fileType == "video" || fileType == "text");
        }

        if (!shouldExtractMetadata) {
            return result;
        }
        int extractStartMs = timer.elapsed();
        // Try to extract metadata using KFileMetaData
        KFileMetaData::ExtractorCollection* extractors = getCachedExtractorCollection();
        const auto extractorsList = extractors->fetchExtractors(guessMimeType);

        if (extractorsList.isEmpty()) {
            qDebug() << "  No extractors available";
            result["metadata_extended"] = "NOT SUPPORTED";
            return result;
        }

        // Extract metadata - Add safety try/catch here
        KFileMetaData::PropertyMultiMap properties;

        try {
            KFileMetaData::SimpleExtractionResult extractionResult(filePath, guessMimeType,
                                                                   KFileMetaData::ExtractionResult::ExtractMetaData);

            for (auto extractor : extractorsList) {
                try {
                    extractor->extract(&extractionResult);
                } catch (const std::exception& e) {
                    qDebug() << "  WARNING: Extractor threw exception:" << e.what();
                } catch (...) {
                    qDebug() << "  WARNING: Extractor threw unknown exception";
                }
            }

            properties = extractionResult.properties();

        } catch (const std::exception& e) {
            qDebug() << "  ERROR: Extraction failed:" << e.what();
            result["metadata_extended"] = "FAILED";
            return result;
        } catch (...) {
            qDebug() << "  ERROR: Extraction failed with unknown exception";
            result["metadata_extended"] = "FAILED";
            return result;
        }

        if (properties.isEmpty()) {
            qDebug() << "  No properties extracted";
            result["metadata_extended"] = "EMPTY";
            return result;
        }

        // Process metadata based on file type
        if (fileType == "image") {
            QVariantMap imageData = processImageMetadata(properties);

            if (includeMetadata == Catalog::METADATA_MEDIA_BASIC) {
                // Only essential fields
                if (imageData.contains("image_width"))
                    result["image_width"] = imageData["image_width"];
                if (imageData.contains("image_height"))
                    result["image_height"] = imageData["image_height"];
                if (imageData.contains("image_orientation"))
                    result["image_orientation"] = imageData["image_orientation"];
            } else {
                // All image metadata
                for (auto it = imageData.begin(); it != imageData.end(); ++it) {
                    result[it.key()] = it.value();
                }
            }
        }
        else if (fileType == "video") {
            QVariantMap videoData = processVideoMetadata(properties);
            //qDebug() << "  Video metadata:" << videoData;

            if (includeMetadata == Catalog::METADATA_MEDIA_BASIC) {
                // Only essential fields
                if (videoData.contains("video_duration_seconds"))
                    result["video_duration_seconds"] = videoData["video_duration_seconds"];
                if (videoData.contains("video_width"))
                    result["video_width"] = videoData["video_width"];
                if (videoData.contains("video_height"))
                    result["video_height"] = videoData["video_height"];
            } else {
                // All video metadata
                for (auto it = videoData.begin(); it != videoData.end(); ++it) {
                    result[it.key()] = it.value();
                }
            }
        }
        else if (fileType == "audio") {
            QVariantMap audioData = processAudioMetadata(properties);
            //qDebug() << "  Audio metadata:" << audioData;

            if (includeMetadata == Catalog::METADATA_MEDIA_BASIC) {
                // Only essential fields
                if (audioData.contains("audio_duration_seconds"))
                    result["audio_duration_seconds"] = audioData["audio_duration_seconds"];
                if (audioData.contains("audio_artist"))
                    result["audio_artist"] = audioData["audio_artist"];
                if (audioData.contains("audio_album"))
                    result["audio_album"] = audioData["audio_album"];
                if (audioData.contains("audio_title"))
                    result["audio_title"] = audioData["audio_title"];
            } else {
                // All audio metadata
                for (auto it = audioData.begin(); it != audioData.end(); ++it) {
                    result[it.key()] = it.value();
                }
            }
        }

        // For EXTENDED or FULL levels, add extended JSON
        if (includeMetadata == Catalog::METADATA_MEDIA_EXTENDED ||
            includeMetadata == Catalog::METADATA_FULL) {
            QJsonObject extendedObj;
            for (auto it = properties.begin(); it != properties.end(); ++it) {
                KFileMetaData::PropertyInfo propInfo(it.key());
                QString propName = propInfo.name();
                QVariant propValue = it.value();

                if (!propValue.toString().isEmpty()) {
                    extendedObj[propName] = propValue.toString();
                }
            }

            if (!extendedObj.isEmpty()) {
                QJsonDocument doc(extendedObj);
                result["metadata_extended"] = doc.toJson(QJsonDocument::Compact);
                qDebug() << "  Added extended JSON with" << extendedObj.size() << "properties";
            }
        }

        //qDebug() << "  Final result keys:" << result.keys();
        int extractEndMs = timer.elapsed();
        totalExtractMs += (extractEndMs - extractStartMs);

        if (callCount % 500 == 0 || callCount == 5745) {
            qDebug() << "=== TIMING ===" << callCount << "files extracted,"
                     << "avg extract time:" << (totalExtractMs / callCount) << "ms per file";
        }

        return result;
    } catch (const std::exception& e) {
        qDebug() << "ERROR: Exception in extractMetadata:" << e.what();
        result["metadata_extended"] = "FAILED";
    } catch (...) {
        qDebug() << "ERROR: Unknown exception in extractMetadata";
        result["metadata_extended"] = "FAILED";
    }

    return result;
}
//-----------------------------------------------------------------------------------------------------
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
#if QT_VERSION < QT_VERSION_CHECK(6, 0, 0)
                if (existing.type() == QVariant::List) {
#else
                if (existing.typeId() == QMetaType::QVariantList) {
#endif
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
//-----------------------------------------------------------------------------------------------------
QString FileMetadata::convertMetadataToJson(const QVariantMap &extendedMetadata)
{
    QJsonObject jsonObject;

    // Convert QVariantMap to QJsonObject
    for (auto it = extendedMetadata.begin(); it != extendedMetadata.end(); ++it) {
        QString key = it.key();
        QVariant value = it.value();

        // Convert different value types to appropriate JSON types
#if QT_VERSION < QT_VERSION_CHECK(6, 0, 0)
        if (value.type() == QVariant::List) {
#else
        if (value.typeId() == QMetaType::QVariantList) {
#endif
            QJsonArray jsonArray;
            const QVariantList list = value.toList();  // Make const to avoid detachment
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
//-----------------------------------------------------------------------------------------------------
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

    const QVariantList& constValues = values;
    for (const QVariant &value : constValues) {
        query.addBindValue(value);
    }

    if (!query.exec()) {
        qDebug() << "FileMetadata::updateFileMetadata - Query failed:" << query.lastError().text();
        qDebug() << "Query was:" << queryString;
        return false;
    }

    return true;
}
//-----------------------------------------------------------------------------------------------------
bool FileMetadata::isMetadataSupported(const QString &filePath)
{
    // First check extension - avoid MIME detection if possible
    QFileInfo fileInfo(filePath);
    QString extension = fileInfo.suffix().toLower();

    // Quick check based on extension
    static QSet<QString> supportedExtensions = {
        // Images
        "jpg", "jpeg", "png", "gif", "bmp", "tiff", "tif", "webp", "svg", "heic", "heif", "raw", "xcf",
        // Video
        "mp4", "avi", "mkv", "mov", "wmv", "flv", "webm", "m4v", "mpg", "mpeg", "3gp", "ogv", "vob", "m2ts", "mts",
        // Audio
        "mp3", "wav", "flac", "ogg", "oga", "m4a", "aac", "wma", "opus", "aiff", "aif", "mid", "midi", "amr"
    };

    return supportedExtensions.contains(extension);
}
//-----------------------------------------------------------------------------------------------------
void FileMetadata::initializeExtensionsCache() {
    if (s_cacheInitialized) return;

    // Query all MIME types from KFileMetaData
    KFileMetaData::ExtractorCollection extractors;
    QMimeDatabase mimeDb;

    // Get all MIME types that have extractors
    const auto allMimeTypes = mimeDb.allMimeTypes();
    for (const auto& mimeType : allMimeTypes) {
        auto extractorsList = extractors.fetchExtractors(mimeType.name());
        if (!extractorsList.isEmpty()) {
            // Add all suffixes for this MIME type
            const auto suffixes = mimeType.suffixes();
            for (const QString& suffix : suffixes) {
                s_supportedExtensionsCache.insert(suffix.toLower());
            }
        }
    }

    s_cacheInitialized = true;
    qDebug() << "Metadata extensions cache initialized with"
             << s_supportedExtensionsCache.size() << "extensions";
}
//-----------------------------------------------------------------------------------------------------
bool FileMetadata::isExtensionSupported(const QString &extension)
{
    // Use the cache instead of hardcoded logic
    if (!s_cacheInitialized) {
        initializeExtensionsCache();
    }

    return s_supportedExtensionsCache.contains(extension.toLower());
}
//-----------------------------------------------------------------------------------------------------
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
//-----------------------------------------------------------------------------------------------------
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
#if KFILEMETADATA_VERSION >= QT_VERSION_CHECK(6, 0, 0)
    if (properties.contains(KFileMetaData::Property::VideoCodec)) {
        result["video_codec"] = properties.value(KFileMetaData::Property::VideoCodec).toString();
    }
#endif

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
//-----------------------------------------------------------------------------------------------------
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
//-----------------------------------------------------------------------------------------------------
QString FileMetadata::getExtendedMetadataJson(const QString &filePath)
{
    // Use the FULL metadata level to get all extended metadata
    QVariantMap metadata = extractMetadata(filePath, Catalog::METADATA_FULL);

    if (metadata.contains("metadata_extended")) {
        return metadata["metadata_extended"].toString();
    }

    return QString("{}");  // Return empty JSON object instead of message
}
//-----------------------------------------------------------------------------------------------------
