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
// Purpose:     Class/model for the catalog (list of files from a device)
// Description:
// Author:      Stephane Couturier
/////////////////////////////////////////////////////////////////////////////
*/

#ifndef CATALOG_H
#define CATALOG_H

#include <QAbstractTableModel>
#include <QLocale>

class Catalog : public QAbstractTableModel
{
    Q_OBJECT

public:
    Catalog(QObject *parent = nullptr);

    QString ID,
            filePath,
            name,
            dateUpdated,
            sourcePath,
            fileCount,
            totalFileSize,
            sourcePathIsActive,
            includeHidden,
            fileType,
            storage,
            includeSymblinks,
            isFullDevice,
            loadedVersion;

    int rowCount(const QModelIndex &parent = QModelIndex()) const override;
    int columnCount(const QModelIndex &parent = QModelIndex()) const override;
    QVariant data(const QModelIndex &index, int role = Qt::DisplayRole) const override;
    QVariant headerData(int section, Qt::Orientation orientation, int role = Qt::DisplayRole) const override;

    void setCatalogName(QString selectedCatalogName);

    void loadCatalogMetaData();

    void populateFileData(  const QList<QString> &fileNames,
                            const QList<qint64>  &fileSizes,
                            const QList<QString> &filePaths,
                            const QList<QString> &fileDateTimes,
                            const QList<QString> &fileCatalogs);

private:
    QList<QString> fileNames;
    QList<qint64>  fileSizes;
    QList<QString> filePaths;
    QList<QString> fileDateTimes;
    QList<QString> fileCatalogs;
};

#endif // CATALOG_H
