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
// File Name:   devicetreeview.h
// Purpose:     Class/model to display a tree of storage devices and catalogs
// Description:
// Author:      Stephane Couturier
/////////////////////////////////////////////////////////////////////////////
*/
#ifndef DEVICETREEVIEW_H
#define DEVICETREEVIEW_H

#include <QSortFilterProxyModel>

class DeviceTreeView  : public QSortFilterProxyModel
{
    Q_OBJECT

public:
    DeviceTreeView(QObject *parent = nullptr);

    QString m_selectedDeviceName;
    QString m_selectedDeviceType;
    void setSelectedDeviceInfo(QString selectedName,QString selectedType);

private:
    QVariant data( const QModelIndex &index, int role ) const;
    QVariant headerData(int section, Qt::Orientation orientation, int role) const;

};

#endif // DEVICETREEVIEW_H
