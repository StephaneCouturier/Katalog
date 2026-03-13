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
// File Name:   exploretreemodel.cpp
// Purpose:     Class/model to build a tree of directories
// Description:
// Author:      Stephane Couturier
/////////////////////////////////////////////////////////////////////////////
*/
#include "exploretreemodel.h"
#include "core/foldertreeloader.h"
#include <functional>

ExploreTreeModel::ExploreTreeModel(QObject *parent)
    : QAbstractItemModel(parent)
{
    QList<QVariant> rootData;
    rootData << tr("Folder") << tr("No of items") << tr("Full path");
    rootItem = new ExploreTreeItem(rootData);
}

ExploreTreeModel::~ExploreTreeModel()
{
    delete rootItem;
}

int ExploreTreeModel::columnCount(const QModelIndex &parent) const
{
    if (parent.isValid())
        return static_cast<ExploreTreeItem*>(parent.internalPointer())->columnCount();
    else
        return rootItem->columnCount();
}

QVariant ExploreTreeModel::data(const QModelIndex &index, int role) const
{
    if (!index.isValid())
        return QVariant();

    if (role != Qt::DisplayRole)
        return QVariant();

    ExploreTreeItem *item = static_cast<ExploreTreeItem*>(index.internalPointer());

    return item->data(index.column());
}

Qt::ItemFlags ExploreTreeModel::flags(const QModelIndex &index) const
{
    if (!index.isValid())
        return Qt::NoItemFlags;

    return QAbstractItemModel::flags(index);
}

QVariant ExploreTreeModel::headerData(int section, Qt::Orientation orientation,
                               int role) const
{
    if (orientation == Qt::Horizontal && role == Qt::DisplayRole)
        return rootItem->data(section);

    return QVariant();
}

QModelIndex ExploreTreeModel::index(int row, int column, const QModelIndex &parent)
            const
{
    if (!hasIndex(row, column, parent))
        return QModelIndex();

    ExploreTreeItem *parentItem;

    if (!parent.isValid())
        parentItem = rootItem;
    else
        parentItem = static_cast<ExploreTreeItem*>(parent.internalPointer());

    ExploreTreeItem *childItem = parentItem->child(row);
    if (childItem)
        return createIndex(row, column, childItem);
    else
        return QModelIndex();
}

QModelIndex ExploreTreeModel::parent(const QModelIndex &index) const
{
    if (!index.isValid())
        return QModelIndex();

    ExploreTreeItem *childItem = static_cast<ExploreTreeItem*>(index.internalPointer());
    ExploreTreeItem *parentItem = childItem->parentItem();

    if (parentItem == rootItem)
        return QModelIndex();

    return createIndex(parentItem->row(), 0, parentItem);
}

int ExploreTreeModel::rowCount(const QModelIndex &parent) const
{
    ExploreTreeItem *parentItem;
    if (parent.column() > 0)
        return 0;

    if (!parent.isValid())
        parentItem = rootItem;
    else
        parentItem = static_cast<ExploreTreeItem*>(parent.internalPointer());

    return parentItem->childCount();
}

int ExploreTreeModel::findNode(unsigned int& hash, const QList<ExploreTreeItem*>& tList)
{
    for(int idx = 0; idx < tList.size(); ++idx)
    {
        unsigned int z = tList.at(idx)->getIndex();
        if(z == hash)
            return idx;
    }

    return -1;
}

void ExploreTreeModel::setCatalog(int newCatalogID, QString newCatalogSourcePath)
{
    catalogID = newCatalogID;

    catalogSourcePath = newCatalogSourcePath;

    // Handle special case for EXPORT catalogs or paths without separators
    int pos = newCatalogSourcePath.lastIndexOf(QChar('/'));
    if (pos >= 0) {
        catalogSourcePathRoot = newCatalogSourcePath.left(pos);
    } else {
        // For paths like "EXPORT" with no separator, use empty string
        // This will make REPLACE work correctly in setupModelData
        catalogSourcePathRoot = "";
    }
}

void ExploreTreeModel::setupModelData(ExploreTreeItem *parent)
{
    // Load data from core (UI-agnostic)
    QList<FolderNode*> rootNodes = FolderTreeLoader::loadExploreTree(
        m_connectionName, catalogID, catalogSourcePathRoot);

    // Recursively convert FolderNode hierarchy to ExploreTreeItem hierarchy
    std::function<void(const QList<FolderNode*>&, ExploreTreeItem*)> buildTree;
    buildTree = [&buildTree](const QList<FolderNode*>& nodes, ExploreTreeItem* parentItem) {
        for (FolderNode *node : nodes) {
            QList<QVariant> columnData;
            columnData << node->name;
            columnData << node->fileCount;
            columnData << node->fullPath;

            ExploreTreeItem *treeItem = new ExploreTreeItem(columnData, parentItem);
            parentItem->appendChild(treeItem);

            if (!node->children.isEmpty()) {
                buildTree(node->children, treeItem);
            }
        }
    };

    buildTree(rootNodes, parent);

    qDeleteAll(rootNodes);
}
