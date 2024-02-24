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
#/////////////////////////////////////////////////////////////////////////////

#QT Libraries
    QT  += core widgets gui sql charts network multimedia

    requires(qtConfig(tableview))
    requires(qtConfig(treeview))
    requires(qtConfig(listview))

#KF5 Libraries
#linux: {
#    QT += KCoreAddons
#    QT += KI18n
#    QT += KXmlGui
#    QT += KConfigWidgets
#    QT += KWidgetsAddons
#    QT += KIOCore
#    QT += KCompletion
#}

#CONFIG += c++11

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
    src/catalog.cpp \
    src/catalogsview.cpp \
    src/collection.cpp \
    src/device.cpp \
    src/devicetreeview.cpp \
    src/directorytreeitem.cpp \
    src/directorytreemodel.cpp \
    src/exploretreeitem.cpp \
    src/exploretreemodel.cpp \
    src/exploretreeview.cpp \
    src/filesview.cpp \
    src/main.cpp \
    src/mainwindow.cpp \
    src/mainwindow_setup.cpp \
    src/mainwindow_tab_catalogs.cpp \
    src/mainwindow_tab_create.cpp \
    src/mainwindow_tab_device.cpp \
    src/mainwindow_tab_explore.cpp \
    src/mainwindow_tab_filters.cpp \
    src/mainwindow_tab_search.cpp \
    src/mainwindow_tab_settings.cpp \
    src/mainwindow_tab_statistics.cpp \
    src/mainwindow_tab_storage.cpp \
    src/mainwindow_tab_tags.cpp \
    src/search.cpp \
    src/storage.cpp \
    src/storagetreeitem.cpp \
    src/storagetreemodel.cpp \
    src/storageview.cpp \
    src/tag.cpp

HEADERS += \
    src/catalog.h \
    src/catalogsview.h \
    src/collection.h \
    src/database.h \
    src/device.h \
    src/devicetreeview.h \
    src/directorytreeitem.h \
    src/directorytreemodel.h \
    src/exploretreeitem.h \
    src/exploretreemodel.h \
    src/exploretreeview.h \
    src/filesview.h \
    src/mainwindow.h \
    src/search.h \
    src/storage.h \
    src/storagetreeitem.h \
    src/storagetreemodel.h \
    src/storageview.h \
    src/tag.h \
    src/tag.h

FORMS += \
    src/mainwindow.ui

TRANSLATIONS += \
    src/translations/Katalog_de_DE.ts \
    src/translations/Katalog_en_US.ts \
    src/translations/Katalog_fr_FR.ts \
    src/translations/Katalog_cz_CZ.ts

# Default rules for deployment.
#qnx: target.path = /tmp/$${TARGET}/bin
#else: unix:!android: target.path = /opt/$${TARGET}/bin
#!isEmpty(target.path): INSTALLS += target

#TARGET = Katalog.bin

RESOURCES += \
    src/icons.qrc \
    src/images.qrc \
    src/Resources.qrc \
    src/styles.qrc \
    src/translations.qrc

DISTFILES += \
    docs/github_pages/README.md \
    docs/github_pages/background.jpg \
    docs/github_pages/index.html \
    docs/github_pages/logo.jpg \
    docs/github_pages/script.js \
    docs/github_pages/style.css \
    src/images/Appname_Logo.png \
    src/images/Banner.png \
    src/images/Katalog_logo_64.ico \
    src/images/drive_blue.png \
    src/images/drive_gray.png \
    src/images/drive_green.png \
    src/images/drive_orange.png \
    src/images/flags/cz.png \
    src/images/flags/de.png \
    src/images/flags/fr.png \
    src/images/flags/us.png \
    src/images/link-h.png \
    src/images/link-tree-end.png \
    src/images/link-tree-mid.png \
    src/images/link-v.png \
    src/images/link_blue/link-h.png \
    src/images/link_blue/link-tree-end.png \
    src/images/link_blue/link-tree-mid.png \
    src/images/link_blue/link-v.png \
    src/images/screenshots/Filters_catalogs_storage.png \
    src/images/screenshots/Filters_catalogs_virtual.png \
    src/images/screenshots/Filters_connecteddrives.png \
    src/images/screenshots/Virtual_screen.png \
    src/LICENSE.md \
    src/README.md \
    src/fallback-icons/address-book-new.png \
    src/fallback-icons/application-x-zerosize.png \
    src/fallback-icons/audio-x-mpeg.png \
    src/fallback-icons/collapse-all.png \
    src/fallback-icons/configure.png \
    src/fallback-icons/dialog-ok-apply.png \
    src/fallback-icons/document-edit-sign.png \
    src/fallback-icons/document-edit.png \
    src/fallback-icons/document-export.png \
    src/fallback-icons/document-new.png \
    src/fallback-icons/document-open.png \
    src/fallback-icons/document-preview-archive.png \
    src/fallback-icons/document-save.png \
    src/fallback-icons/drive-harddisk-root.png \
    src/fallback-icons/drive-harddisk.png \
    src/fallback-icons/drive-multidisk.png \
    src/fallback-icons/edit-clear-history.png \
    src/fallback-icons/edit-copy.png \
    src/fallback-icons/edit-delete.png \
    src/fallback-icons/edit-download.png \
    src/fallback-icons/edit-find.png \
    src/fallback-icons/edit-paste-in-place.png \
    src/fallback-icons/edit-paste.png \
    src/fallback-icons/edit-select.png \
    src/fallback-icons/expand-all.png \
    src/fallback-icons/folder-new.png \
    src/fallback-icons/folder.png \
    src/fallback-icons/format-convert-to-path.png \
    src/fallback-icons/go-down.png \
    src/fallback-icons/go-next.png \
    src/fallback-icons/go-previous.png \
    src/fallback-icons/go-up.png \
    src/fallback-icons/gparted.png \
    src/fallback-icons/image-jpeg.png \
    src/fallback-icons/internet-web-browser.png \
    src/fallback-icons/journal-new.png \
    src/fallback-icons/kontact-import-wizard.png \
    src/fallback-icons/media-optical-blu-ray.png \
    src/fallback-icons/media-optical.png \
    src/fallback-icons/media-playlist-repeat.png \
    src/fallback-icons/office-chart-line.png \
    src/fallback-icons/tag.png \
    src/fallback-icons/tools-wizard.png \
    src/fallback-icons/user-trash.png \
    src/fallback-icons/video-mp4.png \
    src/fallback-icons/view-filter.png \
    src/fallback-icons/view-list-text.png \
    src/fallback-icons/view-list-tree.png \
    src/fallback-icons/view-media-playlist.png \
    src/fallback-icons/view-refresh.png \
    src/fallback-icons/view-statistics.png \
    src/fallback-icons/xml-node-duplicate.png \
    src/styles/tabwidget_blue.css \
    src/styles/tabwidget_dev.css \
    src/translations/Katalog_cz_CZ.qm \
    src/translations/Katalog_de_DE.qm \
    src/translations/Katalog_en_US.qm \
    src/translations/Katalog_fr_FR.qm

#For executable icon under Windows
RC_ICONS = src/images/Katalog_logo_64.ico
ICON = src/images/Katalog_logo_64.ico
