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
    return 21; // Same index layout as core Search (SpecExplore EXP-C13)
}

QVariant ExploreFilesModel::data(const QModelIndex &index, int role) const
{
    if (!index.isValid() || index.row() >= m_entries.size())
        return {};

    const Catalog::ExploreFileEntry &e = m_entries[index.row()];

    switch (role) {
    case Qt::DisplayRole:
        switch (index.column()) {
        // Column indices follow core Search::data, so the shared FilesView sort
        // (merged columns 10-12) and the K2-shared sort setting mean the same
        // column in both pages and both versions (SpecExplore EXP-C13, EXP-F18).
        case 0:  return e.name;
        case 1:  return e.size;        // raw qint64 — QML formats it; sort uses the numeric value
        case 2:  return e.dateUpdated;
        case 3:  return e.folderPath;
        case 4:  return QString();     // Catalog Name — not used in Explore
        case 5:  return QString();     // Catalog ID   — not used in Explore
        case 6:  // folders-first sort key, same form as K2's order_value
            return QString((e.entryType == QLatin1String("folder") ? QLatin1Char('1') : QLatin1Char('2'))
                           + (e.entryType == QLatin1String("folder") ? e.folderPath : e.name));
        case 7:  return e.fullPath;
        case 8:  return e.fileType;
        case 9:  return e.mimeType;
        case 10: return e.imageWidth  > 0 ? QVariant(e.imageWidth)  : QVariant();
        case 11: return e.imageHeight > 0 ? QVariant(e.imageHeight) : QVariant();
        case 12: return e.videoDurationSeconds > 0 ? QVariant(e.videoDurationSeconds) : QVariant();
        case 13: return e.videoWidth  > 0 ? QVariant(e.videoWidth)  : QVariant();
        case 14: return e.videoHeight > 0 ? QVariant(e.videoHeight) : QVariant();
        case 15: return e.audioDurationSeconds > 0 ? QVariant(e.audioDurationSeconds) : QVariant();
        case 16: return e.audioArtist;
        case 17: return e.audioAlbum;
        case 18: return e.audioTitle;
        case 19: return e.checksumSha256;
        case 20: return e.checksumExtractionDate;
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
    // Header texts are core Search's, byte for byte (SpecExplore EXP-C13).
    case 0:  return QCoreApplication::translate("MainWindow", "Name");
    case 1:  return QCoreApplication::translate("MainWindow", "Size");
    case 2:  return QCoreApplication::translate("MainWindow", "Date");
    case 3:  return QCoreApplication::translate("MainWindow", "Directory");
    case 8:  return QCoreApplication::translate("MainWindow", "File Type");
    case 9:  return QCoreApplication::translate("MainWindow", "MIME Type");
    case 10: return QCoreApplication::translate("MainWindow", "Width");
    case 11: return QCoreApplication::translate("MainWindow", "Height");
    case 12: return QCoreApplication::translate("MainWindow", "Duration");
    case 16: return QCoreApplication::translate("MainWindow", "Artist");
    case 17: return QCoreApplication::translate("MainWindow", "Album");
    case 18: return QCoreApplication::translate("MainWindow", "Title");
    case 19: return QString(QCoreApplication::translate("MainWindow", "Checksum") + " (SHA256)");
    case 20: return QCoreApplication::translate("MainWindow", "Checksum Date");
    }
    return QString(); // hidden columns — no visible header
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
