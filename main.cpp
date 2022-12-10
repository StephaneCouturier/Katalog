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
* /////////////////////////////////////////////////////////////////////////////
// Application: Katalog
// File Name:   main.cpp
// Purpose:     Main file for the application at start
// Description: Generates the app, initialize language, theme, fallback icons, translation, and the main window
// Author:      Stephane Couturier
/////////////////////////////////////////////////////////////////////////////
*/
#include <QApplication>
#include <QCommandLineParser>
#include <QTranslator>
#include <QMessageBox>

#ifdef Q_OS_LINUX
    #include <KAboutData>
    #include <KLocalizedString>
#endif

#include "mainwindow.h"

int main(int argc, char *argv[])
{
    QApplication app(argc, argv);

    //Get language choice and apply translation
        //Get user home path
        QStringList standardsPaths = QStandardPaths::standardLocations(QStandardPaths::HomeLocation);
        QString homePath = standardsPaths[0];

        //Define Setting file path and name
        QString settingsFilePath = homePath + "/.config/katalog_settings.ini";
        QSettings settings(settingsFilePath, QSettings:: IniFormat);
        QString userLanguage = settings.value("Settings/Language").toString();

        //Define a language for the first run (no value in the settings file)
        if ( userLanguage == "" ){

            //Get the language of the user's system
            userLanguage = QLocale::system().name();           

            //If this language is not supported yet, default to Engish US.
            QStringList availableUserLanguages;
            availableUserLanguages  << "de_DE"
                                    << "cz_CZ"
                                    << "fr_FR"
                                    << "en_US";
            if ( availableUserLanguages.contains(userLanguage) == false )
                userLanguage = "en_US";
        }

        QTranslator* translator=new QTranslator(0);
        if (translator->load("Katalog_" + userLanguage, ":translations")) {
            app.installTranslator(translator);
        }

    #ifdef Q_OS_LINUX
        KLocalizedString::setApplicationDomain("Katalog");

        KAboutData aboutData(
             QStringLiteral("Katalog"),         // The program name used internally. (componentName)
             i18n("Katalog"),                   // A displayable program name string. (displayName)
             QStringLiteral("1.15"),             // The program version string. (version)
             // Short description of what the app does. (shortDescription)
             i18n("Katalog is an application to catalog, search, and manage files from any drive, permanent or removable."),
             KAboutLicense::GPL,// The license this code is released under
             i18n("(c) 2021 Stephane JM Couturier"),// Copyright Statement (copyrightStatement = QString())
             // Optional text shown in the About box.
             // Can contain any information desired. (otherText)
             i18n("-"),
             // The program homepage string. (homePageAddress = QString())
             QStringLiteral("https://github.com/StephaneCouturier/Katalog/wiki"),
             // The bug report address
             // (bugsEmailAddress = QLatin1String("submit@bugs.kde.org")
             QStringLiteral("https://github.com/StephaneCouturier/Katalog/issues/new"));
             aboutData.addAuthor(i18n("Stéphane Couturier"), i18n("Creator"), QStringLiteral("katalog@stephanecouturier.com"),
             QStringLiteral("https://github.com/StephaneCouturier/"), QStringLiteral(" "));
        KAboutData::setApplicationData(aboutData);

        QCommandLineParser parser;
        aboutData.setupCommandLine(&parser);
        parser.process(app);
        aboutData.processCommandLine(&parser);
    #else
        QApplication::setStyle("fusion");       
    #endif

    //Set theme (on linux it would use the Desktop one, on windows this will fallbak to the pathset jsut after)
    QIcon::setThemeName( "breeze" );
    //Set icon fallback in case theme is not available
    QIcon::setFallbackSearchPaths(QIcon::fallbackSearchPaths() << ":fallback-icons");

    MainWindow* window = new MainWindow();
    window->show();

    return app.exec();
}
