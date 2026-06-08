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
#include <QCommandLineParser>
#include <QProcess>

//#ifdef Q_OS_LINUX
//    #include <KAboutData>
//#include <KLocalizedString>
//#endif

#include "mainwindow.h"
#include "core/commandline.h"
#include "core/language.h"

int main(int argc, char *argv[])
{
    QApplication app(argc, argv);
    app.setApplicationName("Katalog");
    app.setApplicationVersion("2.6");
    app.setOrganizationName("Katalog");
    app.setOrganizationDomain("stephanecouturier.github.io/Katalog");

    //Get language choice and apply translation
        //Get user home path
        QStringList standardsPaths = QStandardPaths::standardLocations(QStandardPaths::HomeLocation);
        QString homePath = standardsPaths[0];

        //Define Setting file path and name
        QString settingsFilePath = homePath + "/.config/katalog_settings.ini";
        QSettings settings(settingsFilePath, QSettings:: IniFormat);
        QString userLanguage = settings.value("Settings/Language").toString();

        //Migrate the legacy Czech code "cz_CZ" (used before) to the standard
        //locale name "cs_CZ", so existing Czech users keep their language.
        if ( userLanguage == "cz_CZ" ){
            userLanguage = "cs_CZ";
            settings.setValue("Settings/Language", userLanguage);
        }

        //Define a language for the first run (no value in the settings file)
        if ( userLanguage == "" ){

            //Get the language of the user's system
            //userLanguage = QLocale::system().name();
            userLanguage = Language::getSystemLanguage();

            //If this language is not supported yet, default to English US.
            if (!Language::isLanguageSupported(userLanguage)) {
                userLanguage = "en_US";
            }

            //Save the value
            QSettings settings(settingsFilePath, QSettings:: IniFormat);
            settings.setValue("Settings/Language", userLanguage);
        }
        //Sanitize an already-stored but unsupported value (e.g. a raw system
        //locale such as "en_GB" persisted by an earlier build): fall back to
        //English US so the UI is not left without a usable translation.
        else if (!Language::isLanguageSupported(userLanguage)) {
            userLanguage = "en_US";
            settings.setValue("Settings/Language", userLanguage);
        }

        QTranslator* translator=new QTranslator(0);
        if (translator->load("Katalog_" + userLanguage, ":translations")) {
            app.installTranslator(translator);
        }

    #ifdef Q_OS_LINUX
        //KLocalizedString::setApplicationDomain("Katalog");
    #else
        QApplication::setStyle("fusion");
    #endif

    //Command line parser
        QStringList args = QCoreApplication::arguments();

        // Handle restart action early (application lifecycle management)
        for (int i = 1; i < args.size(); ++i) {
            QString arg = args[i];
            if (arg == "restart") {

                // Prepare arguments for restarting the application
                QStringList newArgs;
                bool skipNext = false;
                for (const QString &cmdArg : QApplication::arguments()) {
                    if (skipNext) {
                        skipNext = false;
                        continue;
                    }
                    if (cmdArg == "--catalogID" || cmdArg == "--report" || cmdArg == "--myoption") {
                        // Skip the next argument as it is the value for the current option
                        skipNext = true;
                    } else if (cmdArg.startsWith("--")) {
                        // Include other options
                        newArgs << cmdArg;
                    } else if (cmdArg.startsWith("-")) {
                        // Include short options
                        newArgs << cmdArg;
                    } else {
                        // Skip positional arguments (actions like "restart")
                        continue;
                    }
                }

                // Start the new process
                QProcess::startDetached(QApplication::applicationFilePath(), newArgs);

                // Exit the current application
                return 0;
            }
        }

        // Check for other command line arguments (excluding restart)
        bool hasCommandLineArgs = false;
        for (int i = 1; i < args.size(); ++i) {
            QString arg = args[i];
            if (arg == "list_catalogs" || arg == "update_catalog" || arg == "update_all_active" ||
                arg == "search" || arg == "--search" || arg == "-s" ||
                arg == "--csv" || arg == "--report" ||
                arg == "--help" || arg == "-h" ||
                arg == "--version" || arg == "-v") {
                hasCommandLineArgs = true;
                break;
            }
        }

        if (hasCommandLineArgs) {
            // Command line mode - create command line handler
            CommandLineHandler cmdHandler;

            int result = cmdHandler.processCommandLine(app);
            if (result >= 0) {
                return result; // Exit with command line result
            }
            // If result is -1, fall through to GUI mode
        }

        // If no specific action is requested, create and show the main window
        MainWindow window;
        window.show();

        return app.exec();
}
