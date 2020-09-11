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
// File Name:   catalog.h
// Purpose:     Class/model for the catalog (list of files)
// Description:
// Author:      Stephane Couturier
// Modified by: Stephane Couturier
// Created:     2020-07-11
// Version:     0.1
/////////////////////////////////////////////////////////////////////////////
*/


#ifndef CATALOG_H
#define CATALOG_H

#include <QAbstractTableModel>
#include <QTextStream>

class Catalog : public QAbstractTableModel
{
    Q_OBJECT

public:
    Catalog(QObject *parent = nullptr);

    int rowCount(const QModelIndex &parent = QModelIndex()) const override;
    int columnCount(const QModelIndex &parent = QModelIndex()) const override;
    QVariant data(const QModelIndex &index, int role = Qt::DisplayRole) const override;
    QVariant headerData(int section, Qt::Orientation orientation, int role = Qt::DisplayRole) const override;

    void populateFileData(const QList<QString> &fileName,
                      const QList<qint64> &fileSize,
                      const QList<QString> &filePath,
                      const QList<QString> &fileDateTime);

private:
    QList<QString> fileName;
    QList<qint64> fileSize;
    QList<QString> filePath;
    QList<QString> fileDateTime;
};

#endif // CATALOG_H
