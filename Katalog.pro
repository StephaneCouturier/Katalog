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
    src/mainwindow_tab_create.cpp \
    src/mainwindow_tab_device.cpp \
    src/mainwindow_tab_explore.cpp \
    src/mainwindow_tab_filters.cpp \
    src/mainwindow_tab_search.cpp \
    src/mainwindow_tab_settings.cpp \
    src/mainwindow_tab_statistics.cpp \
    src/mainwindow_tab_tags.cpp \
    src/search.cpp \
    src/storage.cpp \
    src/tag.cpp

HEADERS += \
    src/catalog.h \
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
    src/tag.h \
    src/tag.h

FORMS += \
    src/mainwindow.ui

TRANSLATIONS += \
    src/translations/Katalog_de_DE.ts \
    src/translations/Katalog_en_US.ts \
    src/translations/Katalog_fr_FR.ts \
    src/translations/Katalog_cz_CZ.ts


RESOURCES += \
    src/icons.qrc \
    src/images.qrc \
    src/Resources.qrc \
    src/styles.qrc \
    src/translations.qrc

DISTFILES += \
    README.md \
    docs/404.html \
    docs/Gemfile \
    docs/_config.yml \
    docs/assets/css/styles.07bd052f.css \
    docs/assets/images/docsVersionDropdown-35e13cbe46c9923327f30a76a90bff3b.png \
    docs/assets/images/docusaurus-plushie-banner-a60f7593abca1e3eef26a9afa244e4fb.jpeg \
    docs/assets/images/localeDropdown-f0d995e751e7656a1b0dbbc1134e49c2.png \
    docs/assets/js/01a85c17.c39a71aa.js \
    docs/assets/js/0e384e19.4d608f23.js \
    docs/assets/js/14eb3368.f42f1f05.js \
    docs/assets/js/1538.c41fc585.js \
    docs/assets/js/15ca0c76.d7671995.js \
    docs/assets/js/17896441.2a03a2bf.js \
    docs/assets/js/18c41134.184af6b0.js \
    docs/assets/js/1df93b7f.4d2cbab8.js \
    docs/assets/js/1e4232ab.c07700c0.js \
    docs/assets/js/1f391b9e.ccd9807d.js \
    docs/assets/js/2237.eeaafd0c.js \
    docs/assets/js/3242.273b0293.js \
    docs/assets/js/36994c47.364b5155.js \
    docs/assets/js/393a0b35.64b2fa22.js \
    docs/assets/js/393be207.e78e0cf7.js \
    docs/assets/js/42fb4d58.487fc62c.js \
    docs/assets/js/464ce5e7.7064c938.js \
    docs/assets/js/533a09ca.df8cd23f.js \
    docs/assets/js/5566110a.4c7c2cf0.js \
    docs/assets/js/59362658.17971da3.js \
    docs/assets/js/5a5c66b3.49c559b4.js \
    docs/assets/js/5c868d36.d18e263c.js \
    docs/assets/js/5e95c892.24f64a67.js \
    docs/assets/js/6875c492.3376aca5.js \
    docs/assets/js/6bc7eaab.6a8164e3.js \
    docs/assets/js/709de83f.2f935902.js \
    docs/assets/js/73664a40.fc26c721.js \
    docs/assets/js/7661071f.afa073f0.js \
    docs/assets/js/7ba4394c.94d24dc4.js \
    docs/assets/js/814f3328.2f3a5a97.js \
    docs/assets/js/822bd8ab.232ee0de.js \
    docs/assets/js/8717b14a.75625c2c.js \
    docs/assets/js/925b3f96.443ef430.js \
    docs/assets/js/9dcdaee3.8a70eed4.js \
    docs/assets/js/9e4087bc.1245fdd5.js \
    docs/assets/js/a6aa9e1f.0699f0bc.js \
    docs/assets/js/a7456010.05dfefb2.js \
    docs/assets/js/a7bd4aaa.8877968a.js \
    docs/assets/js/a94703ab.b99c471b.js \
    docs/assets/js/aba21aa0.c8001e84.js \
    docs/assets/js/acecf23e.bc284cef.js \
    docs/assets/js/ccc49370.be59fe37.js \
    docs/assets/js/d9f32620.1649c650.js \
    docs/assets/js/dff1c289.af052cbd.js \
    docs/assets/js/e273c56f.8174ca32.js \
    docs/assets/js/e44a2883.e5f33e9e.js \
    docs/assets/js/f4f34a3a.5ffc00e5.js \
    docs/assets/js/f55d3e7a.4bb90ea6.js \
    docs/assets/js/main.a7e0d78f.js \
    docs/assets/js/main.a7e0d78f.js.LICENSE.txt \
    docs/assets/js/runtime~main.307c189a.js \
    docs/blog/archive/index.html \
    docs/blog/atom.xml \
    docs/blog/first-blog-post/index.html \
    docs/blog/index.html \
    docs/blog/long-blog-post/index.html \
    docs/blog/mdx-blog-post/index.html \
    docs/blog/rss.xml \
    docs/blog/tags/docusaurus/index.html \
    docs/blog/tags/facebook/index.html \
    docs/blog/tags/hello/index.html \
    docs/blog/tags/hola/index.html \
    docs/blog/tags/index.html \
    docs/blog/welcome/index.html \
    docs/cz/index.md \
    docs/docs/category/tutorial---basics/index.html \
    docs/docs/category/tutorial---extras/index.html \
    docs/docs/intro/index.html \
    docs/docs/tutorial-basics/congratulations/index.html \
    docs/docs/tutorial-basics/create-a-blog-post/index.html \
    docs/docs/tutorial-basics/create-a-document/index.html \
    docs/docs/tutorial-basics/create-a-page/index.html \
    docs/docs/tutorial-basics/deploy-your-site/index.html \
    docs/docs/tutorial-basics/markdown-features/index.html \
    docs/docs/tutorial-extras/manage-docs-versions/index.html \
    docs/docs/tutorial-extras/translate-your-site/index.html \
    docs/en/index.md \
    docs/fr/index.html \
    docs/img/Banner_1.20.png \
    docs/img/Katalog_logo_1.20.png \
    docs/img/docusaurus-social-card.jpg \
    docs/img/docusaurus.png \
    docs/img/favicon.ico \
    docs/img/logo.svg \
    docs/img/undraw_docusaurus_mountain.svg \
    docs/img/undraw_docusaurus_react.svg \
    docs/img/undraw_docusaurus_tree.svg \
    docs/index.html \
    docs/markdown-page/index.html \
    docs/sitemap.xml \
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
    src/fallback-icons-dark/address-book-new.png \
    src/fallback-icons-dark/application-x-zerosize.png \
    src/fallback-icons-dark/audio-x-mpeg.png \
    src/fallback-icons-dark/collapse-all.png \
    src/fallback-icons-dark/configure.png \
    src/fallback-icons-dark/dialog-ok-apply.png \
    src/fallback-icons-dark/document-edit-sign.png \
    src/fallback-icons-dark/document-edit.png \
    src/fallback-icons-dark/document-export.png \
    src/fallback-icons-dark/document-new.png \
    src/fallback-icons-dark/document-open.png \
    src/fallback-icons-dark/document-preview-archive.png \
    src/fallback-icons-dark/document-save.png \
    src/fallback-icons-dark/drive-harddisk-root.png \
    src/fallback-icons-dark/drive-harddisk.png \
    src/fallback-icons-dark/drive-multidisk.png \
    src/fallback-icons-dark/edit-clear-history.png \
    src/fallback-icons-dark/edit-copy.png \
    src/fallback-icons-dark/edit-delete.png \
    src/fallback-icons-dark/edit-download.png \
    src/fallback-icons-dark/edit-find.png \
    src/fallback-icons-dark/edit-paste-in-place.png \
    src/fallback-icons-dark/edit-paste.png \
    src/fallback-icons-dark/edit-select.png \
    src/fallback-icons-dark/expand-all.png \
    src/fallback-icons-dark/folder-new.png \
    src/fallback-icons-dark/folder.png \
    src/fallback-icons-dark/format-convert-to-path.png \
    src/fallback-icons-dark/go-down.png \
    src/fallback-icons-dark/go-next.png \
    src/fallback-icons-dark/go-previous.png \
    src/fallback-icons-dark/go-up.png \
    src/fallback-icons-dark/gparted.png \
    src/fallback-icons-dark/image-jpeg.png \
    src/fallback-icons-dark/internet-web-browser.png \
    src/fallback-icons-dark/journal-new.png \
    src/fallback-icons-dark/kontact-import-wizard.png \
    src/fallback-icons-dark/media-optical-blu-ray.png \
    src/fallback-icons-dark/media-optical.png \
    src/fallback-icons-dark/media-playlist-repeat.png \
    src/fallback-icons-dark/office-chart-line.png \
    src/fallback-icons-dark/tag.png \
    src/fallback-icons-dark/tools-wizard.png \
    src/fallback-icons-dark/user-trash.png \
    src/fallback-icons-dark/video-mp4.png \
    src/fallback-icons-dark/view-filter.png \
    src/fallback-icons-dark/view-list-text.png \
    src/fallback-icons-dark/view-list-tree.png \
    src/fallback-icons-dark/view-media-playlist.png \
    src/fallback-icons-dark/view-refresh.png \
    src/fallback-icons-dark/view-statistics.png \
    src/fallback-icons-dark/xml-node-duplicate.png \
    src/styles/tabwidget_blue.css \
    src/styles/tabwidget_dev.css \
    src/translations/Katalog_cz_CZ.qm \
    src/translations/Katalog_de_DE.qm \
    src/translations/Katalog_en_US.qm \
    src/translations/Katalog_fr_FR.qm

#For executable icon under Windows
RC_ICONS = src/images/Katalog_logo_64.ico
ICON = src/images/Katalog_logo_64.ico
