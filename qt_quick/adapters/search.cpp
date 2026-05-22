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
// File Name:   search.cpp
// Purpose:     class to manage search criteria and results
// Description:
// Author:      Stephane Couturier
/////////////////////////////////////////////////////////////////////////////
*/

#include "search.h"  // qt_quick/adapters/search.h → inherits ../../core/searchjobstoppable.h

SearchSync::SearchSync(QObject *parent) : SearchJobStoppable(parent)
{
}

//file list model
int SearchSync::rowCount(const QModelIndex &parent) const
{
    Q_UNUSED(parent);
    return fileNames.length();
}

int SearchSync::columnCount(const QModelIndex &parent) const
{
    Q_UNUSED(parent);
    return 21;
}

QVariant SearchSync::data(const QModelIndex &index, int role) const
{
    if (!index.isValid())
        return QVariant();

    // All roles and all columns delegate to Search::data()
    return Search::data(index, role);
}

QVariant SearchSync::headerData(int section, Qt::Orientation orientation, int role) const
{
    return Search::headerData(section, orientation, role);
}

QHash<int, QByteArray> SearchSync::roleNames() const
{
    // Start with the base QAbstractItemModel defaults (includes "display" for Qt::DisplayRole)
    QHash<int, QByteArray> roles = QAbstractTableModel::roleNames();
    // Merge in the named roles from Search for QML access by role name
    const QHash<int, QByteArray> searchRoles = Search::roleNames();
    for (auto it = searchRoles.cbegin(); it != searchRoles.cend(); ++it) {
        roles[it.key()] = it.value();
    }
    return roles;
}

//----------------------------------------------------------------------
void SearchSync::resetSearchResults()
{
    beginResetModel();
    clearResults();
    endResetModel();
    emit propertiesChanged();
}
//----------------------------------------------------------------------
void SearchSync::searchFiles(Device *selectedDevice)
{
    beginResetModel();
    SearchJobStoppable::searchFiles(selectedDevice);
    endResetModel();
    emit propertiesChanged();
}
//----------------------------------------------------------------------
