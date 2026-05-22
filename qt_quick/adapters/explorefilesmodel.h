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
// File Name:   explorefilesmodel.h
// Purpose:     QAbstractTableModel for the Explore page file/folder list
// Description: Columns 0-3 are visible (Name, Size, Date, Directory).
//              Hidden data (entryType, fileType, fullPath, checksum) is
//              accessible via named roles for use in QML delegates.
// Author:      Stephane Couturier
/////////////////////////////////////////////////////////////////////////////
*/
#ifndef EXPLOREFILESMODEL_H
#define EXPLOREFILESMODEL_H

#include <QAbstractTableModel>
#include "../../core/catalog.h"

class ExploreFilesModel : public QAbstractTableModel
{
    Q_OBJECT

public:
    enum Roles {
        NameRole        = Qt::UserRole + 1,
        SizeRole        = Qt::UserRole + 2,
        DateRole        = Qt::UserRole + 3,
        FolderPathRole  = Qt::UserRole + 4,
        FullPathRole    = Qt::UserRole + 5,
        EntryTypeRole   = Qt::UserRole + 6,
        FileTypeRole    = Qt::UserRole + 7,
        ChecksumRole    = Qt::UserRole + 8,
    };

    explicit ExploreFilesModel(QObject *parent = nullptr);

    void populate(const QList<Catalog::ExploreFileEntry> &entries);
    Q_INVOKABLE void removeEntry(int row);
    void clear();

    int     rowCount(const QModelIndex &parent = QModelIndex()) const override;
    int     columnCount(const QModelIndex &parent = QModelIndex()) const override;
    QVariant data(const QModelIndex &index, int role = Qt::DisplayRole) const override;
    QVariant headerData(int section, Qt::Orientation orientation, int role = Qt::DisplayRole) const override;
    QHash<int, QByteArray> roleNames() const override;

private:
    QList<Catalog::ExploreFileEntry> m_entries;
};

#endif // EXPLOREFILESMODEL_H
