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
// File Name:   directorytreemodel.cpp
// Purpose:     Class/model to display a tree of directories
// Description:
// Author:      Stephane Couturier
/////////////////////////////////////////////////////////////////////////////
*/
#include "directorytreemodel.h"
#include "directorytreeitem.h"
#include "core/database.h"
#include "core/foldertreeloader.h"

DirectoryTreeModel::DirectoryTreeModel(const QStringList &headers, QObject *parent)
    : QAbstractItemModel(parent)
{
    QVector<QVariant> rootData;
    for (const QString &header : headers)
        rootData << header;

    rootItem = new DirectoryTreeItem(rootData);
    setupModelData(rootItem);
}

DirectoryTreeModel::~DirectoryTreeModel()
{
    delete rootItem;
}

int DirectoryTreeModel::columnCount(const QModelIndex &parent) const
{
    Q_UNUSED(parent);
    return rootItem->columnCount();
}

QVariant DirectoryTreeModel::data(const QModelIndex &index, int role) const
{
    if (!index.isValid())
        return QVariant();

    if (role != Qt::DisplayRole && role != Qt::EditRole)
        return QVariant();

    DirectoryTreeItem *item = getItem(index);

    return item->data(index.column());
}

Qt::ItemFlags DirectoryTreeModel::flags(const QModelIndex &index) const
{
    if (!index.isValid())
        return Qt::NoItemFlags;

    return Qt::ItemIsEditable | QAbstractItemModel::flags(index);
}

DirectoryTreeItem *DirectoryTreeModel::getItem(const QModelIndex &index) const
{
    if (index.isValid()) {
        DirectoryTreeItem *item = static_cast<DirectoryTreeItem*>(index.internalPointer());
        if (item)
            return item;
    }
    return rootItem;
}

QVariant DirectoryTreeModel::headerData(int section, Qt::Orientation orientation,
                               int role) const
{
    if (orientation == Qt::Horizontal && role == Qt::DisplayRole)
        return rootItem->data(section);

    return QVariant();
}

QModelIndex DirectoryTreeModel::index(int row, int column, const QModelIndex &parent) const
{
    if (parent.isValid() && parent.column() != 0)
        return QModelIndex();

    DirectoryTreeItem *parentItem = getItem(parent);
    if (!parentItem)
        return QModelIndex();

    DirectoryTreeItem *childItem = parentItem->child(row);
    if (childItem)
        return createIndex(row, column, childItem);
    return QModelIndex();
}

bool DirectoryTreeModel::insertColumns(int position, int columns, const QModelIndex &parent)
{
    beginInsertColumns(parent, position, position + columns - 1);
    const bool success = rootItem->insertColumns(position, columns);
    endInsertColumns();

    return success;
}

bool DirectoryTreeModel::insertRows(int position, int rows, const QModelIndex &parent)
{
    DirectoryTreeItem *parentItem = getItem(parent);
    if (!parentItem)
        return false;

    beginInsertRows(parent, position, position + rows - 1);
    const bool success = parentItem->insertChildren(position,
                                                    rows,
                                                    rootItem->columnCount());
    endInsertRows();

    return success;
}

QModelIndex DirectoryTreeModel::parent(const QModelIndex &index) const
{
    if (!index.isValid())
        return QModelIndex();

    DirectoryTreeItem *childItem = getItem(index);
    DirectoryTreeItem *parentItem = childItem ? childItem->parent() : nullptr;

    if (parentItem == rootItem || !parentItem)
        return QModelIndex();

    return createIndex(parentItem->childNumber(), 0, parentItem);
}

bool DirectoryTreeModel::removeColumns(int position, int columns, const QModelIndex &parent)
{
    beginRemoveColumns(parent, position, position + columns - 1);
    const bool success = rootItem->removeColumns(position, columns);
    endRemoveColumns();

    if (rootItem->columnCount() == 0)
        removeRows(0, rowCount());

    return success;
}

bool DirectoryTreeModel::removeRows(int position, int rows, const QModelIndex &parent)
{
    DirectoryTreeItem *parentItem = getItem(parent);
    if (!parentItem)
        return false;

    beginRemoveRows(parent, position, position + rows - 1);
    const bool success = parentItem->removeChildren(position, rows);
    endRemoveRows();

    return success;
}

int DirectoryTreeModel::rowCount(const QModelIndex &parent) const
{
    const DirectoryTreeItem *parentItem = getItem(parent);

    return parentItem ? parentItem->childCount() : 0;
}

bool DirectoryTreeModel::setData(const QModelIndex &index, const QVariant &value, int role)
{
    if (role != Qt::EditRole)
        return false;

    DirectoryTreeItem *item = getItem(index);
    bool result = item->setData(index.column(), value);

    if (result)
        emit dataChanged(index, index, {Qt::DisplayRole, Qt::EditRole});

    return result;
}

bool DirectoryTreeModel::setHeaderData(int section, Qt::Orientation orientation,
                              const QVariant &value, int role)
{
    if (role != Qt::EditRole || orientation != Qt::Horizontal)
        return false;

    const bool result = rootItem->setData(section, value);

    if (result)
        emit headerDataChanged(orientation, section, section);

    return result;
}

void DirectoryTreeModel::setupModelData(DirectoryTreeItem *parent)
{
    // Load data from core (UI-agnostic)
    QList<FolderNode*> nodes = FolderTreeLoader::loadDirectoryTree(
        m_connectionName, modelCatalogId, modelCatalogPath);

    // Convert FolderNode list to DirectoryTreeItem hierarchy
    QVector<DirectoryTreeItem*> parents;
    parents << parent;
    int lastAdded = 0;
    QString lastAddedPath;

    for (FolderNode *node : nodes) {
        QVector<QVariant> columnData;
        QString directoryPath = node->name;
        columnData << node->name;
        columnData << node->fullPath;

        if (!directoryPath.contains("/")) {
            DirectoryTreeItem *parentItem = parents.last();
            parentItem->insertChildren(parentItem->childCount(), 1, rootItem->columnCount());
            for (int column = 0; column < columnData.size(); ++column)
                parentItem->child(parentItem->childCount() - 1)->setData(column, columnData[column]);

            lastAddedPath = directoryPath;
            lastAdded = parents.count();
        }
        else {
            if (!directoryPath.remove(lastAddedPath + "/").contains("/")) {
                DirectoryTreeItem *parentItem = parents.last();
                parentItem->insertChildren(lastAdded, 1, rootItem->columnCount());
                for (int column = 0; column < columnData.size(); ++column)
                    parentItem->child(parentItem->childCount() - 1)->setData(column, columnData[column]);
            }
        }
    }

    qDeleteAll(nodes);
}

void DirectoryTreeModel::setModelCatlog(int newModelCatalogId, QString newModelCatalogPath)
{
    modelCatalogId = newModelCatalogId;
    modelCatalogPath = newModelCatalogPath;
    setupModelData(rootItem);
}
