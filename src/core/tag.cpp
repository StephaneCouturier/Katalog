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
// File Name:   tag.cpp
// Purpose:     class to create a model used to display tags and their attributes
// Description:
// Author:      Stephane Couturier
/////////////////////////////////////////////////////////////////////////////
*/

#include "tag.h"

Tag::Tag(QObject *parent) : QAbstractTableModel(parent)
{

}

int Tag::rowCount(const QModelIndex &parent) const
{
    Q_UNUSED(parent);
    return folderPath.length();
}

int Tag::columnCount(const QModelIndex &parent) const
{
    Q_UNUSED(parent);
    return 3;
}

QVariant Tag::data(const QModelIndex &index, int role) const
{
    if (!index.isValid()) return QVariant();

    if (role == Qt::DisplayRole) {
        switch (index.column()){
        case 0: return int(ID[index.row()]);
        case 1: return QString(folderPath[index.row()]);
        case 2: return QString(tagName[index.row()]);
        }
        return QVariant();
    }

    // Named roles for QML
    const int row = index.row();
    switch (role) {
    case IdRole:     return int(ID[row]);
    case FolderRole: return QString(folderPath[row]);
    case NameRole:   return QString(tagName[row]);
    }
    return QVariant();
}

QHash<int, QByteArray> Tag::roleNames() const
{
    QHash<int, QByteArray> roles;
    roles[IdRole]     = "tagId";
    roles[FolderRole] = "folder";
    roles[NameRole]   = "name";
    return roles;
}

QVariant Tag::headerData(int section, Qt::Orientation orientation, int role) const
{
    if (role == Qt::DisplayRole && orientation == Qt::Horizontal) {
        switch (section){
        case 0: return QString(tr("ID"));
        case 1: return QString(tr("Folder"));
        case 2: return QString(tr("Tag"));
        }
    }
    return QVariant();
}

// Create a method to populate the model with data
void Tag::populateTagData(
    const QList<int> &tTagID,
    const QList<QString> &tFolderPath,
    const QList<QString> &tTagName)
{
    ID.clear();
    ID = tTagID;
    folderPath.clear();
    folderPath = tFolderPath;
    tagName.clear();
    tagName = tTagName;

    return;
}
//----------------------------------------------------------------------
void Tag::loadFromDatabase(const QString &connectionName, const QString &filterName)
{
    QList<int>     tTagIDs;
    QList<QString> tFolderPaths;
    QList<QString> tTagNames;

    QSqlQuery query(QSqlDatabase::database(connectionName));
    QString querySQL = QLatin1String("SELECT ID, path, name FROM tag");
    if (!filterName.isEmpty())
        querySQL += QLatin1String(" WHERE name = :name");

    query.prepare(querySQL);
    if (!filterName.isEmpty())
        query.bindValue(":name", filterName);

    if (query.exec()) {
        while (query.next()) {
            tTagIDs      << query.value(0).toInt();
            tFolderPaths << query.value(1).toString();
            tTagNames    << query.value(2).toString();
        }
    }

    populateTagData(tTagIDs, tFolderPaths, tTagNames);
}
//----------------------------------------------------------------------
QList<QString> Tag::tagNames() const
{
    return tagName;
}
