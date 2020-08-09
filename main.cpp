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
// Description:
// Author:      Stephane Couturier
// Modified by: Stephane Couturier
// Created:     2020-07-11
// Version:     0.1
/////////////////////////////////////////////////////////////////////////////
*/

#include <cstdlib>

#include <QApplication>
#include <QCommandLineParser>

#include <KAboutData>
#include <KLocalizedString>

#include "mainwindow.h"

#include <QTableView>

int main(int argc, char *argv[])
{
    QApplication app(argc, argv);

    KLocalizedString::setApplicationDomain("Katalog");

    KAboutData aboutData(
         QStringLiteral("Katalog"),         // The program name used internally. (componentName)
         i18n("Katalog"),                   // A displayable program name string. (displayName)
         QStringLiteral("0.6"),             // The program version string. (version)
         // Short description of what the app does. (shortDescription)
         i18n("Katalog is an application to catalog, search, and manage files from any drive, permanent or removable."),
         KAboutLicense::GPL,// The license this code is released under
         i18n("(c) 2020 Stephane Couturier"),// Copyright Statement (copyrightStatement = QString())
         // Optional text shown in the About box.
         // Can contain any information desired. (otherText)
         i18n(""),
         // The program homepage string. (homePageAddress = QString())
         QStringLiteral("https://sourceforge.net/projects/katalogg"),
         // The bug report email address
         // (bugsEmailAddress = QLatin1String("submit@bugs.kde.org")
         QStringLiteral("https://sourceforge.net/p/katalogg/tickets/"));
         aboutData.addAuthor(i18n("Stephane Couturier"), i18n("Creator"), QStringLiteral("contact@symbioxy.com"),
         QStringLiteral("https://www.stephanecouturier.com/"), QStringLiteral("OSC Username"));
    KAboutData::setApplicationData(aboutData);

    QCommandLineParser parser;
    aboutData.setupCommandLine(&parser);
    parser.process(app);
    aboutData.processCommandLine(&parser);
    MainWindow* window = new MainWindow();
    window->show();

    return app.exec();
}

