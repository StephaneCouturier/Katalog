/*LICENCE
    This file is part of Katalog

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
/////////////////////////////////////////////////////////////////////////////
// Application: Katalog
// File Name:   language.cpp
// Purpose:     Language management implementation
// Description: Centralizes language data and provides query interface
// Author:      Stephane Couturier
/////////////////////////////////////////////////////////////////////////////
*/

#include "language.h"
#include <QLocale>

// Static language data array - updated syntax for Language class
const QList<Language::LanguageData> Language::supportedLanguages = {
    {"bg_BG", "Bulgarian", ":/images/flags/bg.png"},
    {"cz_CZ", "Czech", ":/images/flags/cz.png"},
    {"da_DK", "Danish", ":/images/flags/dk.png"},
    {"de_DE", "German", ":/images/flags/de.png"},
    {"en_US", "English", ":/images/flags/us.png"},
    {"es_ES", "Spanish", ":/images/flags/es.png"},
    {"fi_FI", "Finnish", ":/images/flags/fi.png"},
    {"fr_FR", "French", ":/images/flags/fr.png"},
    {"hi_IN", "Hindi", ":/images/flags/in.png"},
    {"hu_HU", "Hungarian", ":/images/flags/hu.png"},
    {"it_IT", "Italian", ":/images/flags/it.png"},
    {"ja_JP", "Japanese", ":/images/flags/jp.png"},
    {"nb_NO", "Norwegian", ":/images/flags/no.png"},
    {"nl_NL", "Dutch", ":/images/flags/nl.png"},
    {"pl_PL", "Polish", ":/images/flags/pl.png"},
    {"pt_PT", "Portuguese", ":/images/flags/pt.png"},
    {"ro_RO", "Romanian", ":/images/flags/ro.png"},
    {"si_SI", "Slovenian", ":/images/flags/si.png"},
    {"sk_SK", "Slovak", ":/images/flags/sk.png"},
    {"sr_RS", "Serbian", ":/images/flags/rs.png"},
    {"sv_SE", "Swedish", ":/images/flags/se.png"},
    {"zh_CN", "Chinese", ":/images/flags/cn.png"}
};

QStringList Language::getSupportedLanguages()
{
    QStringList codes;
    for (const auto& lang : supportedLanguages) {
        codes << lang.code;
    }
    return codes;
}

QIcon Language::getFlagIcon(const QString& languageCode)
{
    for (const auto& lang : supportedLanguages) {
        if (lang.code == languageCode) {
            return QIcon(lang.flagPath);
        }
    }
    return QIcon(":/images/flags/us.png"); // Default to US flag
}

QString Language::getDisplayName(const QString& languageCode)
{
    for (const auto& lang : supportedLanguages) {
        if (lang.code == languageCode) {
            return lang.displayName;
        }
    }
    return "English"; // Default
}

QString Language::getSystemLanguage()
{
    QString systemLocale = QLocale::system().name();
    if (isLanguageSupported(systemLocale)) {
        return systemLocale;
    }
    return "en_US"; // Fallback
}

bool Language::isLanguageSupported(const QString& languageCode)
{
    return getSupportedLanguages().contains(languageCode);
}
