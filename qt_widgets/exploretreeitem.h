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
// File Name:   exploretreeitem.h
// Purpose:     Class/model to build a tree of directories
// Description:
// Author:      Stephane Couturier
/////////////////////////////////////////////////////////////////////////////
*/
#ifndef EXPLORETREEITEM_H
#define EXPLORETREEITEM_H

#include <QList>
#include <QVariant>

class ExploreTreeItem
{
public:
    explicit ExploreTreeItem(const QList<QVariant> &data, ExploreTreeItem *parentItem = 0, unsigned int id = 0);
    ~ExploreTreeItem();

    void appendChild(ExploreTreeItem *child);

    ExploreTreeItem *child(int row);
    int childCount() const;
    int columnCount() const;
    QVariant data(int column) const;
    int row() const;

    unsigned int getIndex(){return _id;};

    ExploreTreeItem *parentItem();

private:

    QList<ExploreTreeItem*> m_childItems;
    QList<QVariant> m_itemData;
    ExploreTreeItem *m_parentItem;
    unsigned int _id;
};

#endif // EXPLORETREEITEM_H
