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
// Purpose:
// Description:
// Author:      Stephane Couturier
/////////////////////////////////////////////////////////////////////////////
*/
#include "directorytreemodel.h"
#include "directorytreeitem.h"

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
    QVector<DirectoryTreeItem*> parents;
    parents << parent; //add rootItem
    int lastAdded =0;
//    int countLocation=1;
//    int countStorage=1;
//    int countCatalog=0;

    //prepare query to load file info
    QSqlQuery getDirectoriesQuery;

        //shorten the paths as they all start with the catalog path
        QString getDirectoriesSQL = QLatin1String(R"(
                                        SELECT DISTINCT (REPLACE(file_path, :selectedCatalogPath||'/', ''))
                                        FROM filesall
                                        WHERE   file_catalog =:file_catalog
                                        ORDER BY file_path ASC
                                    )");
        getDirectoriesQuery.prepare(getDirectoriesSQL);
        getDirectoriesQuery.bindValue(":file_catalog",modelCatalogName);
        getDirectoriesQuery.bindValue(":selectedCatalogPath",modelCatalogPath);
        getDirectoriesQuery.exec();


        QString lastAddedPath;

        //Add folders
        while (getDirectoriesQuery.next())
        {
            //Prepare data
            QVector<QVariant> columnData;
            QString directoryPath = getDirectoriesQuery.value(0).toString();
            columnData << getDirectoriesQuery.value(0).toString();
            columnData << getDirectoriesQuery.value(1).toString();

            //TEST/ add all
                // Append a new item to the current parent's list of children.
//                DirectoryTreeItem *parent = parents.last();
//                parent->insertChildren(parent->childCount(), 1, rootItem->columnCount());
//                for (int column = 0; column < columnData.size(); ++column)
//                    parent->child(parent->childCount() - 1)->setData(column, columnData[column]);

            if (directoryPath.contains("/")==false){
                // Append a new item to the current parent's list of children.
                DirectoryTreeItem *parent = parents.last();
                parent->insertChildren(parent->childCount(), 1, rootItem->columnCount());
                for (int column = 0; column < columnData.size(); ++column)
                    parent->child(parent->childCount() - 1)->setData(column, columnData[column]);

                lastAddedPath = directoryPath;

//                lastAdded=parents.last()->childCount();
                lastAdded=parents.count();//.last()->childCount();
                //lastAdded=parents.last()->childNumber();//.last()->childCount();
            }
            else{
                if ( directoryPath.remove(lastAddedPath+"/").contains("/")==false )
                {
                    // Append a new item to the current parent's list of children.
                    DirectoryTreeItem *parent = parents.last();
                    parent->insertChildren(lastAdded, 1, rootItem->columnCount());
//                    parent->insertChildren(parent->childCount(), 1, rootItem->columnCount());
                    for (int column = 0; column < columnData.size(); ++column)
                        parent->child(parent->childCount() - 1)->setData(column, columnData[column]);

                    //lastAddedPath = directoryPath;
                }
            }

        }
}

void DirectoryTreeModel::setModelCatlog(QString newModelCatalogName, QString newModelCatalogPath)
{
    modelCatalogName = newModelCatalogName;
    modelCatalogPath = newModelCatalogPath;
    setupModelData(rootItem);
}
