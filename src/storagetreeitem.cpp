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
// File Name:   storagetreeitem.cpp
// Purpose:
// Description:
// Author:      Stephane Couturier
/////////////////////////////////////////////////////////////////////////////
*/
#include "storagetreeitem.h"
#include <QStringList>

StorageTreeItem::StorageTreeItem(const QVector<QVariant> &data, StorageTreeItem *parent)
    : itemData(data),
      parentItem(parent)
{}

StorageTreeItem::~StorageTreeItem()
{
    qDeleteAll(childItems);
}

StorageTreeItem *StorageTreeItem::child(int number)
{
    if (number < 0 || number >= childItems.size())
        return nullptr;
    return childItems.at(number);
}

int StorageTreeItem::childCount() const
{
    return childItems.count();
}

int StorageTreeItem::childNumber() const
{
    if (parentItem)
        return parentItem->childItems.indexOf(const_cast<StorageTreeItem*>(this));
    return 0;
}

int StorageTreeItem::columnCount() const
{
    return itemData.count();
}

QVariant StorageTreeItem::data(int column) const
{
    if (column < 0 || column >= itemData.size())
        return QVariant();
    return itemData.at(column);
}

bool StorageTreeItem::insertChildren(int position, int count, int columns)
{
    if (position < 0 || position > childItems.size())
        return false;

    for (int row = 0; row < count; ++row) {
        QVector<QVariant> data(columns);
        StorageTreeItem *item = new StorageTreeItem(data, this);
        childItems.insert(position, item);
    }

    return true;
}

bool StorageTreeItem::insertColumns(int position, int columns)
{
    if (position < 0 || position > itemData.size())
        return false;

    for (int column = 0; column < columns; ++column)
        itemData.insert(position, QVariant());

    for (StorageTreeItem *child : qAsConst(childItems))
        child->insertColumns(position, columns);

    return true;
}

StorageTreeItem *StorageTreeItem::parent()
{
    return parentItem;
}

bool StorageTreeItem::removeChildren(int position, int count)
{
    if (position < 0 || position + count > childItems.size())
        return false;

    for (int row = 0; row < count; ++row)
        delete childItems.takeAt(position);

    return true;
}

bool StorageTreeItem::removeColumns(int position, int columns)
{
    if (position < 0 || position + columns > itemData.size())
        return false;

    for (int column = 0; column < columns; ++column)
        itemData.remove(position);

    for (StorageTreeItem *child : qAsConst(childItems))
        child->removeColumns(position, columns);

    return true;
}

bool StorageTreeItem::setData(int column, const QVariant &value)
{
    if (column < 0 || column >= itemData.size())
        return false;

    itemData[column] = value;
    return true;
}
