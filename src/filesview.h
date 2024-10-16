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
// File Name:   devicetreeview.h
// Purpose:     Class/model to display a list of files
// Description:
// Author:      Stephane Couturier
/////////////////////////////////////////////////////////////////////////////
*/
#ifndef FILESVIEW_H
#define FILESVIEW_H

#include <QSortFilterProxyModel>

class FilesView  : public QSortFilterProxyModel
{
    Q_OBJECT

public:
    FilesView(QObject *parent = nullptr);
    bool caseSensitive = false;

protected:
    bool lessThan(const QModelIndex &left, const QModelIndex &right) const override;
    QVariant data(const QModelIndex &proxyIndex, int role = Qt::DisplayRole) const override;

private:
    QString percentBrush;
    QVariant headerData(int section, Qt::Orientation orientation, int role) const override;
};
#endif // FILESVIEW_H
