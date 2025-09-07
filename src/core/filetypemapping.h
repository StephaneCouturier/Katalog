/*
#ifndef FILETYPEMAPPING_H

#include <QString>
#include <QStringList>
#include <QObject>

class FileTypeMapping {
public:
    enum UserCategory {
        ALL,
        IMAGE,
        VIDEO,
        AUDIO,
        TEXT,
        OTHER,
        NONE
    };

    // Get SQL WHERE clause for user category selection
    static QString getSqlFilter(UserCategory category) {
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

    // Get MIME type suggestions for dropdown based on selected category
    static QStringList getMimeTypeSuggestions(UserCategory category) {
        switch (category) {
        case IMAGE:
            return getImageMimeTypes();
        case VIDEO:
            return getVideoMimeTypes();
        case AUDIO:
            return getAudioMimeTypes();
        case TEXT:
            return getTextMimeTypes();
        case OTHER:
            return getOtherMimeTypes();
        default:
            return QStringList(); // Empty for ALL and NONE
        }
    }

private:
    // Efficient single SQL condition for Text category
    static QString getTextMimeFilter() {
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

    static QStringList getImageMimeTypes();
    static QStringList getVideoMimeTypes();
    static QStringList getAudioMimeTypes();
    static QStringList getTextMimeTypes();
    static QStringList getOtherMimeTypes();
};

#endif // FILETYPEMAPPING_H
*/
#ifndef FILETYPEMAPPING_H
#define FILETYPEMAPPING_H

#include <QString>
#include <QStringList>

    class FileTypeMapping {
public:
    enum UserCategory {
        ALL = 0,
        IMAGE = 1,
        VIDEO = 2,
        AUDIO = 3,
        TEXT = 4,
        OTHER = 5,
        NONE = 6
    };

    // Get SQL WHERE clause for user category selection
    static QString getSqlFilter(UserCategory category);

    // Get MIME type suggestions for dropdown based on selected category
    static QStringList getMimeTypeSuggestions(UserCategory category);

private:
    // Helper methods - all implemented in .cpp file
    static QString getTextMimeFilter();
    static QStringList getImageMimeTypes();
    static QStringList getVideoMimeTypes();
    static QStringList getAudioMimeTypes();
    static QStringList getTextMimeTypes();
    static QStringList getOtherMimeTypes();
};

#endif // FILETYPEMAPPING_H
