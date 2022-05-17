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
// Purpose:     Class/model to display a tree of directories
// Description:
// Author:      Stephane Couturier
/////////////////////////////////////////////////////////////////////////////
*/
#ifndef EXPLORETREEMODEL_H
#define EXPLORETREEMODEL_H

#include "exploretreeitem.h"
#include <QAbstractItemModel>
#include <QModelIndex>
#include <QVariant>

class ExploreTreeModel : public QAbstractItemModel
{
    Q_OBJECT
public:
    explicit ExploreTreeModel(QObject* parent=0);
    ~ExploreTreeModel();

    QVariant data(const QModelIndex &index, int role) const override;
    Qt::ItemFlags flags(const QModelIndex &index) const override;
    QVariant headerData(int section, Qt::Orientation orientation,
                            int role = Qt::DisplayRole) const override;
    QModelIndex index(int row, int column,
                          const QModelIndex &parent = QModelIndex()) const override;
    QModelIndex parent(const QModelIndex &index) const override;
    int rowCount(const QModelIndex &parent = QModelIndex()) const override;
    int columnCount(const QModelIndex &parent = QModelIndex()) const override;

    QString catalogName;
    QString catalogSourcePath;

    void setCatalog(QString newCatalogName, QString newCatalogSourcePath);

private:
    void setupModelData(ExploreTreeItem *parent);
    int findNode(unsigned int& hash, const QList<ExploreTreeItem*>& tList);

    ExploreTreeItem *rootItem;

};
#endif // EXPLORETREEMODEL_H
