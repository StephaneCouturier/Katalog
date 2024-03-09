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
// File Name:   storagetreemodel.cpp
// Purpose:
// Description:
// Author:      Stephane Couturier
/////////////////////////////////////////////////////////////////////////////
*/
#include "storagetreemodel.h"
#include "storagetreeitem.h"

#include <QStringList>
#include <QtWidgets>

StorageTreeModel::StorageTreeModel(const QStringList &headers, QObject *parent)
    : QAbstractItemModel(parent)
{
    QVector<QVariant> rootData;
    for (const QString &header : headers)
        rootData << header;

    rootItem = new StorageTreeItem(rootData);
    setupModelData(rootItem);
}

StorageTreeModel::~StorageTreeModel()
{
    delete rootItem;
}

int StorageTreeModel::columnCount(const QModelIndex &parent) const
{
    Q_UNUSED(parent);
    return rootItem->columnCount();
}

QVariant StorageTreeModel::data(const QModelIndex &index, int role) const
{
    if (!index.isValid())
        return QVariant();

    if (role != Qt::DisplayRole && role != Qt::EditRole)
        return QVariant();

    StorageTreeItem *item = getItem(index);

    return item->data(index.column());
}

Qt::ItemFlags StorageTreeModel::flags(const QModelIndex &index) const
{
    if (!index.isValid())
        return Qt::NoItemFlags;

    return Qt::ItemIsEditable | QAbstractItemModel::flags(index);
}

StorageTreeItem *StorageTreeModel::getItem(const QModelIndex &index) const
{
    if (index.isValid()) {
        StorageTreeItem *item = static_cast<StorageTreeItem*>(index.internalPointer());
        if (item)
            return item;
    }
    return rootItem;
}

QVariant StorageTreeModel::headerData(int section, Qt::Orientation orientation,
                               int role) const
{
    if (orientation == Qt::Horizontal && role == Qt::DisplayRole)
        return rootItem->data(section);

    return QVariant();
}

QModelIndex StorageTreeModel::index(int row, int column, const QModelIndex &parent) const
{
    if (parent.isValid() && parent.column() != 0)
        return QModelIndex();

    StorageTreeItem *parentItem = getItem(parent);
    if (!parentItem)
        return QModelIndex();

    StorageTreeItem *childItem = parentItem->child(row);
    if (childItem)
        return createIndex(row, column, childItem);
    return QModelIndex();
}

bool StorageTreeModel::insertColumns(int position, int columns, const QModelIndex &parent)
{
    beginInsertColumns(parent, position, position + columns - 1);
    const bool success = rootItem->insertColumns(position, columns);
    endInsertColumns();

    return success;
}

bool StorageTreeModel::insertRows(int position, int rows, const QModelIndex &parent)
{
    StorageTreeItem *parentItem = getItem(parent);
    if (!parentItem)
        return false;

    beginInsertRows(parent, position, position + rows - 1);
    const bool success = parentItem->insertChildren(position,
                                                    rows,
                                                    rootItem->columnCount());
    endInsertRows();

    return success;
}

QModelIndex StorageTreeModel::parent(const QModelIndex &index) const
{
    if (!index.isValid())
        return QModelIndex();

    StorageTreeItem *childItem = getItem(index);
    StorageTreeItem *parentItem = childItem ? childItem->parent() : nullptr;

    if (parentItem == rootItem || !parentItem)
        return QModelIndex();

    return createIndex(parentItem->childNumber(), 0, parentItem);
}

bool StorageTreeModel::removeColumns(int position, int columns, const QModelIndex &parent)
{
    beginRemoveColumns(parent, position, position + columns - 1);
    const bool success = rootItem->removeColumns(position, columns);
    endRemoveColumns();

    if (rootItem->columnCount() == 0)
        removeRows(0, rowCount());

    return success;
}

bool StorageTreeModel::removeRows(int position, int rows, const QModelIndex &parent)
{
    StorageTreeItem *parentItem = getItem(parent);
    if (!parentItem)
        return false;

    beginRemoveRows(parent, position, position + rows - 1);
    const bool success = parentItem->removeChildren(position, rows);
    endRemoveRows();

    return success;
}

int StorageTreeModel::rowCount(const QModelIndex &parent) const
{
    const StorageTreeItem *parentItem = getItem(parent);

    return parentItem ? parentItem->childCount() : 0;
}

bool StorageTreeModel::setData(const QModelIndex &index, const QVariant &value, int role)
{
    if (role != Qt::EditRole)
        return false;

    StorageTreeItem *item = getItem(index);
    bool result = item->setData(index.column(), value);

    if (result)
        emit dataChanged(index, index, {Qt::DisplayRole, Qt::EditRole});

    return result;
}

bool StorageTreeModel::setHeaderData(int section, Qt::Orientation orientation,
                              const QVariant &value, int role)
{
    if (role != Qt::EditRole || orientation != Qt::Horizontal)
        return false;

    const bool result = rootItem->setData(section, value);

    if (result)
        emit headerDataChanged(orientation, section, section);

    return result;
}

void StorageTreeModel::setupModelData(StorageTreeItem *parent)
{
    QVector<StorageTreeItem*> parents;
    parents << parent; //add rootItem
    int countLocation=0;
    int countStorage=0;
    int countCatalog=0;

    //Add locations (under hidden root item)
    //Get list of Locations
    QSqlQuery queryLocationList;
    QString queryLocationListSQL = QLatin1String(R"(
                            SELECT DISTINCT storage_location,'Location'
                            FROM storage
                            ORDER BY storage_location ASC
    )");
    queryLocationList.prepare(queryLocationListSQL);
    queryLocationList.exec();

    while (queryLocationList.next())
    {
        //Prepare data
        QVector<QVariant> columnData;
        QString lineLocationData = queryLocationList.value(0).toString();
        columnData << queryLocationList.value(0).toString();
        columnData << queryLocationList.value(1).toString();

        // Append a new item to the current parent's list of children.
        StorageTreeItem *parent = parents.last();
        parent->insertChildren(parent->childCount(), 1, rootItem->columnCount());
        for (int column = 0; column < columnData.size(); ++column)
            parent->child(parent->childCount() - 1)->setData(column, columnData[column]);

        //Add Storage devices
        QSqlQuery queryStorageList;
        QString queryStorageListSQL = QLatin1String(R"(
                                SELECT storage_name,'Storage', storage_id
                                FROM storage
                                WHERE storage_location=:storage_location
                                ORDER BY storage_name ASC
        )");
        queryStorageList.prepare(queryStorageListSQL);
        queryStorageList.bindValue(":storage_location",lineLocationData);
        queryStorageList.exec();

        while (queryStorageList.next())
        {
            QVector<QVariant> columnData;
            QString currentStorageName = queryStorageList.value(0).toString();
            columnData << queryStorageList.value(0).toString();
            columnData << queryStorageList.value(1).toString();
            columnData << queryStorageList.value(2).toString();

            parent->child(countLocation)->insertChildren(countStorage, 1, rootItem->columnCount());
            for (int column = 0; column < columnData.size(); ++column)
                parent->child(countLocation)->child(countStorage)->setData(column, columnData[column]);

            //Add Catalogs
            QSqlQuery queryCatalogList;
            QString queryCatalogListSQL = QLatin1String(R"(
                                    SELECT catalog_name,'Catalog', catalog_source_path_is_active
                                    FROM catalog
                                    WHERE catalog_storage=:catalog_storage
                                    ORDER BY catalog_name ASC
            )");
            queryCatalogList.prepare(queryCatalogListSQL);
            queryCatalogList.bindValue(":catalog_storage",currentStorageName);
            queryCatalogList.exec();

            while (queryCatalogList.next())
            {
                QVector<QVariant> columnData;
                columnData << queryCatalogList.value(0).toString();
                columnData << queryCatalogList.value(1).toString();
                columnData << queryCatalogList.value(2).toString();

                parent->child(countLocation)->child(countStorage)->insertChildren(countCatalog, 1, rootItem->columnCount());
                for (int column = 0; column < columnData.size(); ++column)
                    parent->child(countLocation)->child(countStorage)->child(countCatalog)->setData(column, columnData[column]);

                countCatalog=countCatalog+1;
            }

            countStorage=countStorage+1;
            countCatalog=0;
        }

        countLocation=countLocation+1;
        countStorage=0;
    }
}