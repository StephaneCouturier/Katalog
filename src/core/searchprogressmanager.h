/*LICENCE
    This file is part of Katalog

    Copyright (C) 2021, the Katalog Development team

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
// File Name:   searchprogressmanager.h
// Purpose:     header for the class that handles search progress reporting
// Description: https://stephanecouturier.github.io/Katalog/docs/Features/Search
// Author:      Stephane Couturier
/////////////////////////////////////////////////////////////////////////////
*///

#ifndef SEARCHPROGRESSMANAGER_H
#define SEARCHPROGRESSMANAGER_H

#include <QObject>
#include <QStatusBar>
#include <QString>
#include <QLocale>
#include <QLabel>

// Forward declarations
class SearchManager;
class Search;

class SearchProgressManager : public QObject
{
    Q_OBJECT

public:
    explicit SearchProgressManager(QStatusBar *statusBar, QTimer *timer, QLabel *statusLabel, QObject *parent = nullptr)
        : QObject(parent), m_statusBar(statusBar), m_statusBarTimer(timer), m_statusBarLabel(statusLabel) {}

    void connectToSearchManager(SearchManager *searchManager);
    void setCurrentSearch(Search *currentSearch);

public slots:
    void updateFromSearchManager();
    void showMessage(const QString &message, int timeout = 0);

signals:
    void statusBarUpdated();  // Keep this for now, can remove later if not needed

private:
    QStatusBar *m_statusBar = nullptr;
    QTimer *m_statusBarTimer = nullptr;
    QLabel *m_statusBarLabel = nullptr;
    SearchManager *m_searchManager = nullptr;
    Search *m_currentSearch = nullptr;
};

#endif // SEARCHPROGRESSMANAGER_H
