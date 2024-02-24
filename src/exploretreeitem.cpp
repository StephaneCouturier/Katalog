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
// File Name:   exploretreeitem.cpp
// Purpose:
// Description:
// Author:      Stephane Couturier
/////////////////////////////////////////////////////////////////////////////
*/
#include "exploretreeitem.h"

#include <QStringList>

ExploreTreeItem::ExploreTreeItem(const QList<QVariant> &data, ExploreTreeItem *parent, unsigned int id)
{
    m_parentItem = parent;
    m_itemData = data;
    _id = id;
}

ExploreTreeItem::~ExploreTreeItem()
{
    qDeleteAll(m_childItems);
}

void ExploreTreeItem::appendChild(ExploreTreeItem *item)
{
    m_childItems.append(item);
}

ExploreTreeItem *ExploreTreeItem::child(int row)
{
    return m_childItems.value(row);
}

int ExploreTreeItem::childCount() const
{
    return m_childItems.count();
}

int ExploreTreeItem::columnCount() const
{
    return m_itemData.count();
}

QVariant ExploreTreeItem::data(int column) const
{
    return m_itemData.value(column);
}

ExploreTreeItem *ExploreTreeItem::parentItem()
{
    return m_parentItem;
}

int ExploreTreeItem::row() const
{
    if (m_parentItem)
        return m_parentItem->m_childItems.indexOf(const_cast<ExploreTreeItem*>(this));

    return 0;
}
