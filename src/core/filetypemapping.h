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
// File Name:   filetypemapping.h
// Purpose:     Defines user file types for enhanced searching
// Description:
// Author:      Stephane Couturier
/////////////////////////////////////////////////////////////////////////////
*/
#ifndef FILETYPEMAPPING_H
#define FILETYPEMAPPING_H

#include <QString>
#include <QStringList>

    class FileTypeMapping {
public:
    enum UserCategory {
        ALL   = 0,
        AUDIO = 1,
        IMAGE = 2,
        TEXT  = 3,
        VIDEO = 4,
        OTHER = 5,
        NONE  = 6
    };

    enum TextCategory {
        All,
        Documents,
        CodeScripts,
        DataFiles,
        PlainText,
        WebContent,
        Ebooks
    };

    // Static field - current text category selection
    static TextCategory selectedTextCategory;

    // Universal source of truth for application MIME types that are text
    static const QStringList& getSpecificMimeTypeListForText(); // All categories
    static QStringList getSpecificMimeTypeListForText(TextCategory category);

    // Universal text type detection - implements user type "Text" definition
    static bool isUserTypeText(const QString &mimeType);
    // Universal other type detection - implements user type "Other" definition
    static bool isUserTypeOther(const QString &mimeType);

    // Get SQL WHERE clause for user category selection
    static QString getSqlFilter(UserCategory category);

    // Get MIME type suggestions for dropdown based on selected category
    //static QStringList getMimeTypeSuggestions(UserCategory category);

private:
    // Generate SQL for Text and Other user types
    static QString getSQLforMimeTypesAsText();
    static QString getSQLforOtherNonTextMimeTypes();
};

#endif // FILETYPEMAPPING_H
