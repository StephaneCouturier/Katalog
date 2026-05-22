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
// File Name:   filesview.cpp
// Purpose:     QSortFilterProxyModel wrapper for file list models
// Description: Provides intelligent sorting (numeric size, merged metadata)
//              for both SearchSync and ExploreFilesModel.
// Author:      Stephane Couturier
/////////////////////////////////////////////////////////////////////////////
*/
#include "filesview.h"
#include "core/filemetadata.h"

FilesView::FilesView(QObject *parent)
    : QSortFilterProxyModel(parent)
{
}

void FilesView::setCaseSensitive(bool v)
{
    if (m_caseSensitive == v) return;
    m_caseSensitive = v;
    invalidate();
}

void FilesView::sort(int column, int order)
{
    QSortFilterProxyModel::sort(column, Qt::SortOrder(order));
    // QML TableView doesn't reliably respond to layoutChanged from a proxy;
    // a modelReset guarantees the view re-reads all cells in the new sorted order.
    beginResetModel();
    endResetModel();
}

bool FilesView::lessThan(const QModelIndex &left, const QModelIndex &right) const
{
    const int col = left.column();

    // Column 1: file size — numeric sort on raw qint64
    if (col == 1) {
        qint64 l = sourceModel()->data(left).toLongLong();
        qint64 r = sourceModel()->data(right).toLongLong();
        return l < r;
    }

    // Search model merged metadata columns
    if (col == 10) { // Width (image + video merged)
        int l = FileMetadata::mergeMetadataValue(
            sourceModel()->data(sourceModel()->index(left.row(),  10)),
            sourceModel()->data(sourceModel()->index(left.row(),  13)));
        int r = FileMetadata::mergeMetadataValue(
            sourceModel()->data(sourceModel()->index(right.row(), 10)),
            sourceModel()->data(sourceModel()->index(right.row(), 13)));
        return l < r;
    }
    if (col == 11) { // Height (image + video merged)
        int l = FileMetadata::mergeMetadataValue(
            sourceModel()->data(sourceModel()->index(left.row(),  11)),
            sourceModel()->data(sourceModel()->index(left.row(),  14)));
        int r = FileMetadata::mergeMetadataValue(
            sourceModel()->data(sourceModel()->index(right.row(), 11)),
            sourceModel()->data(sourceModel()->index(right.row(), 14)));
        return l < r;
    }
    if (col == 12) { // Duration (video + audio merged)
        int l = FileMetadata::mergeMetadataValue(
            sourceModel()->data(sourceModel()->index(left.row(),  12)),
            sourceModel()->data(sourceModel()->index(left.row(),  15)));
        int r = FileMetadata::mergeMetadataValue(
            sourceModel()->data(sourceModel()->index(right.row(), 12)),
            sourceModel()->data(sourceModel()->index(right.row(), 15)));
        return l < r;
    }

    // All other columns: case-insensitive string sort by default
    QVariant lData = sourceModel()->data(left);
    QVariant rData = sourceModel()->data(right);
    if (lData.typeId() == QMetaType::QString && rData.typeId() == QMetaType::QString) {
        Qt::CaseSensitivity cs = m_caseSensitive ? Qt::CaseSensitive : Qt::CaseInsensitive;
        return QString::compare(lData.toString(), rData.toString(), cs) < 0;
    }

    return QSortFilterProxyModel::lessThan(left, right);
}
