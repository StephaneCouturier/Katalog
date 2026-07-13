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
// File Name:   backuppreviewmodel.cpp
// Purpose:     QAbstractTableModel for the Backup Preview file list
// Author:      Stephane Couturier
/////////////////////////////////////////////////////////////////////////////
*/
#include "backuppreviewmodel.h"
#include <QLocale>

BackupPreviewModel::BackupPreviewModel(QObject *parent)
    : QAbstractTableModel(parent)
{
}

void BackupPreviewModel::populate(const QList<BackupPreviewRow> &rows, int conflictCount)
{
    beginResetModel();
    m_rows          = rows;
    m_conflictCount = qBound(0, conflictCount, m_rows.size());
    endResetModel();
}

void BackupPreviewModel::clear()
{
    beginResetModel();
    m_rows.clear();
    m_conflictCount = 0;
    endResetModel();
}

int BackupPreviewModel::rowCount(const QModelIndex &parent) const
{
    Q_UNUSED(parent)
    return m_rows.size();
}

int BackupPreviewModel::columnCount(const QModelIndex &parent) const
{
    Q_UNUSED(parent)
    return 4; // Status, File Name, Path, Size
}

QVariant BackupPreviewModel::data(const QModelIndex &index, int role) const
{
    if (!index.isValid() || index.row() >= m_rows.size())
        return {};

    const BackupPreviewRow &r          = m_rows[index.row()];
    const bool              isConflict = index.row() >= (m_rows.size() - m_conflictCount);

    switch (role) {
    case FileNameRole:    return r.fileName;
    case FolderPathRole:  return r.folderPath;
    case FileSizeStrRole: return QLocale().formattedDataSize(r.fileSize);
    case IsConflictRole:  return isConflict;

    case Qt::DisplayRole:
        switch (index.column()) {
        case 0: return r.status; // status text is localised at build time in previewBackup()
        case 1: return r.fileName;
        case 2: return r.folderPath;
        case 3: return QLocale().formattedDataSize(r.fileSize);
        }
        return {};
    }
    return {};
}

QHash<int, QByteArray> BackupPreviewModel::roleNames() const
{
    QHash<int, QByteArray> roles = QAbstractTableModel::roleNames();
    roles[FileNameRole]    = "fileName";
    roles[FolderPathRole]  = "folderPath";
    roles[FileSizeStrRole] = "fileSizeStr";
    roles[IsConflictRole]  = "isConflict";
    return roles;
}
