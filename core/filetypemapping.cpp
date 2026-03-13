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
// File Name:   filetypemapping.cpp
// Purpose:     Defines user file types for enhanced searching
// Description:
// Author:      Stephane Couturier
/////////////////////////////////////////////////////////////////////////////
*/
#include "filetypemapping.h"
#include "filemetadata.h"
#include <QMap>
#include <QStringList>
#include <QDebug>

FileTypeMapping::TextCategory FileTypeMapping::selectedTextCategory = FileTypeMapping::TextCategory::All;

// Add this overload for backward compatibility
const QStringList& FileTypeMapping::getSpecificMimeTypeListForText() {
    static QStringList allTextMimeTypes = getSpecificMimeTypeListForText(TextCategory::All);
    return allTextMimeTypes;
}

QStringList FileTypeMapping::getSpecificMimeTypeListForText(TextCategory category) {
    // Unified source of truth for MIME types grouped by category
    static const QMap<TextCategory, QStringList> categoryToMimeTypes = {
        {TextCategory::Documents, {
                                      "application/pdf",
                                      "application/rtf",
                                      "application/msword",
                                      "application/vnd.openxmlformats-officedocument.wordprocessingml.document",
                                      "application/vnd.oasis.opendocument.text"
                                  }},
        {TextCategory::CodeScripts, {
                                        "application/x-shellscript",
                                        "application/javascript",
                                        "application/x-python",
                                        "application/x-php",
                                        "application/json",
                                        "application/xml",
                                        "application/x-httpd-php",
                                        "application/x-perl",
                                        "application/x-ruby",
                                        "application/x-bsh",
                                        "application/x-csh",
                                        "application/x-java-source",
                                        "application/x-lua",
                                        "application/x-tcl",
                                        "application/x-sql",
                                        "application/x-asp",
                                        "application/x-jsp"
                                    }},
        {TextCategory::DataFiles, {
                                      "application/yaml",
                                      "application/x-yaml",
                                      "text/csv",
                                      "application/csv"
                                  }},
        {TextCategory::PlainText, {
                                      "text/plain",
                                      "text/markdown"
                                  }},
        {TextCategory::WebContent, {
                                       "text/html",
                                       "application/xhtml+xml",
                                       "application/rss+xml",
                                       "application/atom+xml"
                                   }},
        {TextCategory::Ebooks, {
                                   "application/epub+zip",
                                   "application/x-mobipocket-ebook"
                               }}
    };

    if (category == TextCategory::All) {
        // Combine all categories into one list
        QStringList allMimeTypes;
        for (auto it = categoryToMimeTypes.begin(); it != categoryToMimeTypes.end(); ++it) {
            allMimeTypes += it.value();
        }
        // Remove duplicates (some MIME types appear in multiple categories)
        allMimeTypes.removeDuplicates();
        return allMimeTypes;
    }

    return categoryToMimeTypes.value(category);
}

bool FileTypeMapping::isUserTypeText(const QString &mimeType)
{
    // User type "Text" definition:
    // * all type = "other" where mimetype starts with "text"
    // * OR mimetype starts with "application" and mimetype in specificListForText

    if (mimeType.startsWith("text")) {
        return true;
    }

    if (mimeType.startsWith("application") && getSpecificMimeTypeListForText().contains(mimeType)) {
        return true;
    }

    return false;
}

bool FileTypeMapping::isUserTypeOther(const QString &mimeType)
{
    // User type "Other" definition:
    // * all type = "other" where mimetype does not start with "text"
    // * AND mimetype starts with "application" and mimetype NOT in specificListForText

    if (mimeType.startsWith("text")) {
        return false;
    }

    if (mimeType.startsWith("application") && !getSpecificMimeTypeListForText().contains(mimeType)) {
        return true;
    }

    return false;
}

QString FileTypeMapping::getSqlFilter(UserFileType user_file_type) {
    switch (user_file_type) {
    case ALL:
        return "";  // No filter
    case IMAGE:
        return "file_type = 'image'";
    case VIDEO:
        return "file_type = 'video'";
    case AUDIO:
        return "file_type = 'audio'";
    case TEXT:{
        // Include files that are classified as text based on MIME type
        QString sql = QString("file_type = 'text' OR (%1)").arg(getSQLforMimeTypesAsText());
        return sql;
    }
    case OTHER:{
        // Exclude files that are classified as text based on MIME type
        // to prevent overlap with TEXT category
        QString textMimeConditions = getSQLforMimeTypesAsText();
        if (!textMimeConditions.isEmpty()) {
            return QString("file_type = 'other' AND NOT (%1)").arg(textMimeConditions);
        } else {
            return "file_type = 'other'";
        }
    }
    case NONE:
        return "(file_type IS NULL OR file_type = '' OR file_type = 'none')";
    default:
        return "";
    }
}

QStringList FileTypeMapping::getExtensionsForFileType(const QString &fileType)
{
    if (fileType.toLower() == "none") {
        return QStringList();
    }

    // Initialize FileMetadata cache if needed
    FileMetadata::initializeExtensionTypeCache();

    QStringList extensions;
    const auto& cache = FileMetadata::getExtensionToTypeCache();

    for (auto it = cache.begin(); it != cache.end(); ++it) {
        if (it.value() == fileType.toLower()) {
            extensions.append(it.key());
        }
    }

    return extensions;
}

QStringList FileTypeMapping::getExtensionsForCataloging(const QString &fileType)
{
    QString lowerType = fileType.toLower();

    if (lowerType == "none") {
        return QStringList(); // Manual filtering needed
    }

    QStringList extensions = getExtensionsForFileType(lowerType);
    QStringList wildcardExtensions;

    for (const QString &ext : extensions) {
        wildcardExtensions << QString("*.%1").arg(ext);
    }

    return wildcardExtensions;
}

QStringList FileTypeMapping::getExtensionsForSearchRegex(const QString &fileType)
{
    QStringList extensions = getExtensionsForFileType(fileType.toLower());
    QStringList regexExtensions;

    for (const QString &ext : extensions) {
        regexExtensions << QString("*.%1$").arg(ext);
    }

    return regexExtensions;
}

QString FileTypeMapping::getSQLforMimeTypesAsText()
{
    // Use the static field - automatically uses current selection
    const QStringList specificList = getSpecificMimeTypeListForText(selectedTextCategory);
    QStringList conditions;

    // Add specific application types from currently selected category
    for (const QString& mimeType : specificList) {
        conditions << QString("mime_type = '%1'").arg(mimeType);
    }
    QString generatedSQL = QString("(%1)").arg(conditions.join(" OR "));

    return generatedSQL;
}

QString FileTypeMapping::getSQLforOtherNonTextMimeTypes()
{
    // User type "Other" definition:
    // * all type = "other" where mimetype does NOT start with "text"
    // * AND mimetype starts with "application" and mimetype NOT in specificListForText
    const QStringList& specificList = getSpecificMimeTypeListForText();
    QStringList conditions;

    // NOT text/* types
    conditions << "mime_type NOT LIKE 'text%'";

    // Application types but NOT in our specific text list
    QStringList notInSpecificList;
    for (const QString& mimeType : specificList) {
        notInSpecificList << QString("mime_type != '%1'").arg(mimeType);
    }

    conditions << QString("mime_type LIKE 'application%'");
    conditions << QString("(%1)").arg(notInSpecificList.join(" AND "));

    return QString("(%1)").arg(conditions.join(" AND "));
}
