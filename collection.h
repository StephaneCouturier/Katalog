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
// File Name:   collection.h
// Purpose:     Class/model for the collection (list of catalogs)
// Description:
// Author:      Stephane Couturier
// Modified by: Stephane Couturier
// Created:     2020-07-11
// Version:     0.1
/////////////////////////////////////////////////////////////////////////////
*/

#ifndef COLLECTION_H
#define COLLECTION_H

#include <QAbstractTableModel>
#include <QTextStream>

class Collection : public QAbstractTableModel
{
    Q_OBJECT

public:
    Collection(QObject *parent = nullptr);

    int rowCount(const QModelIndex &parent = QModelIndex()) const override;
    int columnCount(const QModelIndex &parent = QModelIndex()) const override;
    QVariant data(const QModelIndex &index, int role = Qt::DisplayRole) const override;
    QVariant headerData(int section, Qt::Orientation orientation, int role = Qt::DisplayRole) const override;

    void populateData(const QList<QString>  &catalogName,
                      const QList<QString>  &catalogDateUpdated,
                      const QList<qint64>   &catalogFileCount,
                      const QList<QString>  &catalogSourcePath,
                      const QList<bool>     &catalogSourcePathIsActive,
                      //const QList<qint64> &catalogTotalFileSize,
                      const QList<QString>  &catalogFilePath);

private:
    QList<QString>  catalogName;
    QList<QString>  catalogDateUpdated;
    QList<qint64>   catalogFileCount;
    QList<QString>  catalogSourcePath;
    QList<bool>     catalogSourcePathIsActive;
    //QList<qint64> catalogTotalFileSize;
    QList<QString>  catalogFilePath;

};

#endif // COLLECTION_H
