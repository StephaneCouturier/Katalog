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
// File Name:   backuppreviewmodel.h
// Purpose:     QAbstractTableModel for the Backup Preview file list
// Description: Columns 0-3 (Status, File Name, Path, Size). The status label
//              and column headers are rendered by the QML delegate; the model
//              exposes only the row data via named roles, plus an isConflict
//              flag so the delegate can style conflict rows.
// Author:      Stephane Couturier
/////////////////////////////////////////////////////////////////////////////
*/
#ifndef BACKUPPREVIEWMODEL_H
#define BACKUPPREVIEWMODEL_H

#include <QAbstractTableModel>
#include "../../core/backupmappingmanager.h"   // BackupPreviewRow

class BackupPreviewModel : public QAbstractTableModel
{
    Q_OBJECT

public:
    enum Roles {
        FileNameRole    = Qt::UserRole + 1,
        FolderPathRole  = Qt::UserRole + 2,
        FileSizeStrRole = Qt::UserRole + 3,
        IsConflictRole  = Qt::UserRole + 4,
    };

    explicit BackupPreviewModel(QObject *parent = nullptr);

    // rows: the flat preview rows (copy/move rows first, then conflicts).
    // conflictCount: number of trailing rows that are conflicts.
    void populate(const QList<BackupPreviewRow> &rows, int conflictCount);
    void clear();

    int     rowCount(const QModelIndex &parent = QModelIndex()) const override;
    int     columnCount(const QModelIndex &parent = QModelIndex()) const override;
    QVariant data(const QModelIndex &index, int role = Qt::DisplayRole) const override;
    QHash<int, QByteArray> roleNames() const override;

private:
    QList<BackupPreviewRow> m_rows;
    int                     m_conflictCount = 0; // trailing rows that are conflicts
};

#endif // BACKUPPREVIEWMODEL_H
