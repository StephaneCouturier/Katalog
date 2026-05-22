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
// File Name:   explorefilesmodel.cpp
// Purpose:     QAbstractTableModel for the Explore page file/folder list
// Author:      Stephane Couturier
/////////////////////////////////////////////////////////////////////////////
*/
#include "explorefilesmodel.h"
#include <QCoreApplication>

ExploreFilesModel::ExploreFilesModel(QObject *parent)
    : QAbstractTableModel(parent)
{
}

void ExploreFilesModel::populate(const QList<Catalog::ExploreFileEntry> &entries)
{
    beginResetModel();
    m_entries = entries;
    endResetModel();
}

void ExploreFilesModel::removeEntry(int row)
{
    if (row < 0 || row >= m_entries.size()) return;
    beginRemoveRows(QModelIndex(), row, row);
    m_entries.removeAt(row);
    endRemoveRows();
}

void ExploreFilesModel::clear()
{
    beginResetModel();
    m_entries.clear();
    endResetModel();
}

int ExploreFilesModel::rowCount(const QModelIndex &parent) const
{
    Q_UNUSED(parent)
    return m_entries.size();
}

int ExploreFilesModel::columnCount(const QModelIndex &parent) const
{
    Q_UNUSED(parent)
    return 4; // Name, Size, Date, Directory
}

QVariant ExploreFilesModel::data(const QModelIndex &index, int role) const
{
    if (!index.isValid() || index.row() >= m_entries.size())
        return {};

    const Catalog::ExploreFileEntry &e = m_entries[index.row()];

    switch (role) {
    case Qt::DisplayRole:
        switch (index.column()) {
        case 0: return e.name;
        case 1: return e.size;        // raw qint64 — QML formats it; sort uses the numeric value
        case 2: return e.dateUpdated;
        case 3: return e.folderPath;
        }
        return {};

    case NameRole:       return e.name;
    case SizeRole:       return e.size;
    case DateRole:       return e.dateUpdated;
    case FolderPathRole: return e.folderPath;
    case FullPathRole:   return e.fullPath;
    case EntryTypeRole:  return e.entryType;
    case FileTypeRole:   return e.fileType;
    case ChecksumRole:   return e.checksumSha256;
    }
    return {};
}

QVariant ExploreFilesModel::headerData(int section, Qt::Orientation orientation, int role) const
{
    if (orientation != Qt::Horizontal || role != Qt::DisplayRole)
        return {};
    switch (section) {
    case 0: return QCoreApplication::translate("MainWindow", "Name");
    case 1: return QCoreApplication::translate("MainWindow", "Size");
    case 2: return QCoreApplication::translate("MainWindow", "Date");
    case 3: return QCoreApplication::translate("MainWindow", "Directory");
    }
    return {};
}

QHash<int, QByteArray> ExploreFilesModel::roleNames() const
{
    QHash<int, QByteArray> roles = QAbstractTableModel::roleNames();
    roles[NameRole]       = "name";
    roles[SizeRole]       = "size";
    roles[DateRole]       = "dateUpdated";
    roles[FolderPathRole] = "folderPath";
    roles[FullPathRole]   = "fullPath";
    roles[EntryTypeRole]  = "entryType";
    roles[FileTypeRole]   = "fileType";
    roles[ChecksumRole]   = "checksumSha256";
    return roles;
}
