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
// File Name:   filesview.h
// Purpose:     QSortFilterProxyModel wrapper for file list models
// Description: Provides intelligent sorting (numeric size, merged metadata)
//              for both SearchSync and ExploreFilesModel.
// Author:      Stephane Couturier
/////////////////////////////////////////////////////////////////////////////
*/
#ifndef FILESVIEW_H
#define FILESVIEW_H

#include <QSortFilterProxyModel>

class FilesView : public QSortFilterProxyModel
{
    Q_OBJECT

public:
    explicit FilesView(QObject *parent = nullptr);

    bool caseSensitive() const { return m_caseSensitive; }
    void setCaseSensitive(bool v);

    // Q_INVOKABLE wrapper so QML can call sort(column, Qt.AscendingOrder/Qt.DescendingOrder)
    Q_INVOKABLE void sort(int column, int order = 0);

protected:
    bool lessThan(const QModelIndex &left, const QModelIndex &right) const override;

private:
    bool m_caseSensitive = false;
};

#endif // FILESVIEW_H
