/*LICENCE
 T his file is par*t of Katalog

 Copyright (C) 2025, the Katalog Development team

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
 / ///////////////////////////////////////////////////////////////////////////
 // Application: Katalog
 // File Name:   language.h
 // Purpose:     Language management for internationalization
 // Description: Provides centralized access to supported languages with codes,
 //              display names, and flag icons for UI and backend use
 // Author:      Stephane Couturier
 /////////////////////////////////////////////////////////////////////////////
*/

 #ifndef LANGUAGE_H
 #define LANGUAGE_H

 #include <QString>
 #include <QStringList>
 #include <QIcon>

 class Language
 {
 public:
     struct LanguageData {
         QString code;        // ex: "fr_FR"
         QString displayName; // ex: "French"
         QString flagPath;    // ex: ":/images/flags/fr.png"
     };

     // Static methods for accessing language data
     static QStringList getSupportedLanguages();
     static QIcon getFlagIcon(const QString& languageCode);
     static QString getFlagPath(const QString& languageCode);
     static QString getDisplayName(const QString& languageCode);
     static QString getSystemLanguage();
     static bool isLanguageSupported(const QString& languageCode);

 private:
     static const QList<LanguageData> supportedLanguages;
 };

 #endif // LANGUAGE_H
