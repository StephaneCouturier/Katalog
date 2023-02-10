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
// Purpose:
// Description:
// Author:      Stephane Couturier
/////////////////////////////////////////////////////////////////////////////
*/
#include "exploretreemodel.h"

#include <QStringList>
#include <QtSql>


ExploreTreeModel::ExploreTreeModel(QObject *parent)
    : QAbstractItemModel(parent)
{
    QList<QVariant> rootData;
    rootData << tr("Folder") << tr("No of items") << tr("Full path");
    rootItem = new ExploreTreeItem(rootData);
    setupModelData(rootItem);
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
//    if (!index.isValid())
//        return 0;

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

void ExploreTreeModel::setCatalog(QString newCatalogName, QString newCatalogSourcePath)
{
    catalogName = newCatalogName;
    catalogSourcePath = newCatalogSourcePath;
    setupModelData(rootItem);
}

void ExploreTreeModel::setupModelData(ExploreTreeItem *parent)
{
    QList<ExploreTreeItem*> parents;
    parents << parent;

        QSqlQuery query;
        QString querySQL = QLatin1String(R"(
                                SELECT DISTINCT (REPLACE(folderPath, :selectedCatalogPath, '')) AS filePath,
                                       folderPath AS fullPath
                                FROM  folder
                                WHERE folderCatalogName=:folderCatalogName
                                ORDER BY folderPath ASC
                            )");
        query.prepare(querySQL);
        query.bindValue(":folderCatalogName", catalogName);
        query.bindValue(":selectedCatalogPath",catalogSourcePath);
        query.exec();
query.next();

        int idPath = query.record().indexOf("filePath");
        int idIdx = query.record().indexOf("fullPath");

        QList<QVariant> columnData;

        while (query.next())
        {
           QString name         = query.value(idPath).toString();

                   int id_file          = query.value(idIdx).toInt();
                   QString folderPath;

                   QStringList nodeString = name.split("/", Qt::SkipEmptyParts);
                   QString temppath = "";

                   int lastidx = 0;
                   for(int node = 0; node < nodeString.count(); ++node)
                   {
                        temppath += nodeString.at(node);
                        //if(node != nodeString.count() - 1)
                        temppath += "/";

                        unsigned int hash = qHash(temppath);
                        columnData.clear();

                        columnData << nodeString.at(node);

                        int idx = findNode(hash, parents);

                        if(idx != -1)
                        {
                            lastidx = idx;
                        }
                        else
                        {
                           QString sQuery =  "";
                           if(node == nodeString.count() - 1)
                           {
                               sQuery += "SELECT count(*) FROM filesall WHERE id_file=";
                               sQuery += QString::number(id_file);
                               sQuery += ";";
                           }
                           else
                           {
                               sQuery += "SELECT count(*) FROM fileAll WHERE filePath like '";
                               sQuery += temppath;
                               sQuery += "%';";
                           }

                           int nChild = 0;
                           QSqlQuery query2(sQuery);

                           if(query2.next())
                                nChild = query2.value(0).toInt();

                           columnData << nChild;

                           folderPath = catalogSourcePath + "/" + temppath;
                           folderPath.truncate(folderPath.length()-1);
                           columnData << folderPath;

                           if(lastidx != -1)
                           {
                               parents.at(lastidx)->appendChild(new ExploreTreeItem(columnData, parents.at(lastidx), hash));
                               parents <<  parents.at(lastidx)->child( parents.at(lastidx)->childCount()-1);
                               lastidx = -1;
                           }
                           else
                           {
                               parents.last()->appendChild(new ExploreTreeItem(columnData, parents.last(), hash));
                               parents <<  parents.last()->child( parents.last()->childCount()-1);
                           }
                       }
                   }
        }
}
