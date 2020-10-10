#LICENCE
#    This file is part of Katalog
#
#    Copyright (C) 2020, the Katalog Development team
#
#    Author: Stephane Couturier (Symbioxy)
#
#    Katalog is free software; you can redistribute it and/or modify
#    it under the terms of the GNU General Public License as published by
#    the Free Software Foundation; either version 2 of the License, or
#    (at your option) any later version.
#
#    Katalog is distributed in the hope that it will be useful,
#    but WITHOUT ANY WARRANTY; without even the implied warranty of
#    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
#    GNU General Public License for more details.
#
#    You should have received a copy of the GNU General Public License
#    along with Katalog; if not, write to the Free Software
#    Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA
#
# FILE DESCRIPTION
# /////////////////////////////////////////////////////////////////////////////
#// Application: Katalog
#// File Name:   Katalog.pro
#// Purpose:     Necessary file providing the instructions to build the application in Qt Creator
#// Description:
#// Author:      Stephane Couturier
#// Modified by: Stephane Couturier
#// Created:     2020-07-11
#// Version:     0.1
#/////////////////////////////////////////////////////////////////////////////

#QT Libraries
QT       += core gui charts
QT       += sql widgets widgets
requires(qtConfig(tableview))
requires(qtConfig(treeview))
requires(qtConfig(listview))

#KF5 Libraries
QT       += KCoreAddons
QT       += KI18n
QT       += KXmlGui
QT       += KTextWidgets
QT       += KConfigWidgets
QT       += KWidgetsAddons
QT       += KIOCore
QT       += KCompletion
greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

CONFIG += c++11

# The following define makes your compiler emit warnings if you use
# any Qt feature that has been marked deprecated (the exact warnings
# depend on your compiler). Please consult the documentation of the
# deprecated API in order to know how to port your code away from it.
DEFINES += QT_DEPRECATED_WARNINGS

# You can also make your code fail to compile if it uses deprecated APIs.
# In order to do so, uncomment the following line.
# You can also select to disable deprecated APIs only up to a certain version of Qt.
#DEFINES += QT_DISABLE_DEPRECATED_BEFORE=0x060000    # disables all the APIs deprecated before Qt 6.0.0

SOURCES += \
    catalog.cpp \
    collection.cpp \
    main.cpp \
    mainwindow.cpp \
    mainwindow_setup.cpp \
    mainwindow_tab_collection.cpp \
    mainwindow_tab_create.cpp \
    mainwindow_tab_search.cpp \
    mainwindow_tab_settings.cpp \
    mainwindow_tab_statistics.cpp \
    mainwindow_tab_storage.cpp \
    mainwindow_tab_tags.cpp \
    mainwindow_tab_tests.cpp \
    storage.cpp \
    tag.cpp

HEADERS += \
    catalog.h \
    collection.h \
    database.h \
    initdb.h \
    mainwindow.h \
    storage.h \
    tag.h

FORMS += \
    mainwindow.ui

TRANSLATIONS += \
    Katalog_fr_FR.ts

# Default rules for deployment.
qnx: target.path = /tmp/$${TARGET}/bin
else: unix:!android: target.path = /opt/$${TARGET}/bin
!isEmpty(target.path): INSTALLS += target

DISTFILES += \
    README.md

RESOURCES += \
    Resources.qrc
