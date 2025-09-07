// filetypemapping.cpp
#include "filetypemapping.h"

QString FileTypeMapping::getSqlFilter(UserCategory category) {
    switch (category) {
    case ALL:
        return "";  // No filter
    case IMAGE:
        return "file_type = 'image'";
    case VIDEO:
        return "file_type = 'video'";
    case AUDIO:
        return "file_type = 'audio'";
    case TEXT:
        return QString("file_type = 'other' AND (%1)").arg(getTextMimeFilter());
    case OTHER:
        return QString("file_type = 'other' AND NOT (%1)").arg(getTextMimeFilter());
    case NONE:
        return "(file_type IS NULL OR file_type = '' OR file_type = 'unknown')";
    default:
        return "";
    }
}

QStringList FileTypeMapping::getMimeTypeSuggestions(UserCategory category) {
    switch (category) {
    case AUDIO:
        // For UI purposes - user can filter within audio
        return {"audio/mpeg", "audio/wav", "audio/flac", "audio/ogg", "audio/aac"};
    case IMAGE:
        // For UI purposes - user can filter within images
        return {"image/jpeg", "image/png", "image/gif", "image/webp", "image/tiff"};
    case TEXT:
        // These are ACTUALLY needed because TEXT uses complex MIME filtering
        return getTextMimeTypes();
    case VIDEO:
        // For UI purposes - user can filter within videos
        return {"video/mp4", "video/avi", "video/quicktime", "video/webm", "video/x-matroska"};
    case OTHER:
        // These are ACTUALLY needed because OTHER uses complex MIME filtering
        return getOtherMimeTypes();
    default:
        return QStringList(); // Empty for ALL and NONE
    }
}

QString FileTypeMapping::getTextMimeFilter() {
    return R"(
        LOWER(mime_type) LIKE 'text/%' OR
        mime_type = 'application/pdf' OR
        mime_type = 'application/rtf' OR
        mime_type = 'application/msword' OR
        mime_type LIKE 'application/vnd.openxmlformats-officedocument%' OR
        mime_type LIKE 'application/vnd.ms-%' OR
        mime_type LIKE 'application/vnd.oasis.opendocument%' OR
        mime_type = 'application/epub+zip' OR
        mime_type IN ('text/html', 'text/xml', 'application/xml', 'application/json') OR
        mime_type LIKE 'text/x-%' OR
        mime_type = 'application/x-shellscript'
    )";
}

QStringList FileTypeMapping::getImageMimeTypes() {
    return {
        "image/jpeg", "image/png", "image/gif", "image/bmp", "image/tiff",
        "image/webp", "image/svg+xml", "image/x-icon", "image/heic", "image/avif",
        "image/x-canon-cr2", "image/x-canon-crw", "image/x-nikon-nef",
        "image/x-adobe-dng", "image/x-sony-arw"
    };
}

QStringList FileTypeMapping::getVideoMimeTypes() {
    return {
        "video/mp4", "video/avi", "video/quicktime", "video/x-msvideo",
        "video/x-ms-wmv", "video/x-flv", "video/webm", "video/x-matroska",
        "video/3gpp", "video/mp2t", "video/x-m4v"
    };
}

QStringList FileTypeMapping::getAudioMimeTypes() {
    return {
        "audio/mpeg", "audio/ogg", "audio/wav", "audio/flac", "audio/aac",
        "audio/x-ms-wma", "audio/x-aiff", "audio/mp4", "audio/x-m4a",
        "audio/x-wav"
    };
}

QStringList FileTypeMapping::getTextMimeTypes() {
    return {
        // Plain text
        "text/plain", "text/html", "text/xml", "text/css", "text/javascript",

        // Documents
        "application/pdf", "application/rtf", "application/msword",
        "application/vnd.openxmlformats-officedocument.wordprocessingml.document",
        "application/vnd.oasis.opendocument.text",

        // Ebooks
        "application/epub+zip",

        // Data formats
        "application/json", "application/xml", "text/csv",

        // Development
        "text/x-c", "text/x-c++", "text/x-python", "text/x-java",
        "application/x-shellscript"
    };
}

QStringList FileTypeMapping::getOtherMimeTypes() {
    return {
        // Archives
        "application/zip", "application/x-rar-compressed", "application/x-7z-compressed",
        "application/x-tar", "application/gzip",

        // Executables
        "application/x-executable", "application/x-msdos-program",

        // Data
        "application/x-sqlite3", "application/octet-stream",

        // Fonts
        "font/ttf", "font/otf", "application/font-woff"
    };
}
