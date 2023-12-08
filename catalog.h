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
#include <QDateTime>
#include <QSqlQuery>
#include <QFileInfo>
#include <QRegularExpression>
#include <QMessageBox>
#include <QCoreApplication>
#include <QDirIterator>
#include <QStringListModel>

class Catalog : public QAbstractTableModel
{
    Q_OBJECT

public:
    Catalog(QObject *parent = nullptr);

    //Attributes
    //Saved
        int ID;
        QString name;
        QString filePath;
        QDateTime dateUpdated;
        QString sourcePath;
        qint64  fileCount = 0;
        qint64  totalFileSize = 0;
        bool    includeHidden;
        QString fileType;
        QString storageName;
        bool    includeSymblinks;
        bool    isFullDevice;
        QDateTime dateLoaded;
        bool    includeMetadata;
        QString appVersion;

    QStringListModel *fileListModel;

    int rowCount(const QModelIndex &parent = QModelIndex()) const override;
    int columnCount(const QModelIndex &parent = QModelIndex()) const override;
    QVariant data(const QModelIndex &index, int role = Qt::DisplayRole) const override;
    QVariant headerData(int section, Qt::Orientation orientation, int role = Qt::DisplayRole) const override;

    void setSourcePath(QString selectedSourcePath);
    void updateFileCount();//from database
    void updateTotalFileSize();//from database
    void setStorageName(QString selectedStorageName);
    void setDateLoaded(QDateTime dateTime);
    void setDateUpdated(QDateTime dateTime);

    void generateID();
    void insertCatalog();
    void deleteCatalog();
    void saveCatalog();
    QList<qint64> updateCatalogFiles(QString databaseMode);
    void catalogDirectory(QString databaseMode);

    void loadCatalog();//from database
    void renameCatalog(QString newCatalogName);
    void renameCatalogFile(QString newCatalogName);
    void updateStorageNameToFile();
    void loadCatalogFileListToTable();
    void loadFoldersToTable();
    void saveStatistics(QDateTime dateTime);
    void saveStatisticsToFile(QString filePath, QDateTime dateTime);
    bool catalogNameExists();

    void populateFileData( const QList<QString> &fileNames,
                           const QList<qint64>  &fileSizes,
                           const QList<QString> &filePaths,
                           const QList<QString> &fileDateTimes,
                           const QList<QString> &fileCatalogs);

private:
    QStringList fileExtensions;

    QList<QString> fileNames;
    QList<qint64>  fileSizes;
    QList<QString> filePaths;
    QList<QString> fileDateTimes;
    QList<QString> fileCatalogs;

    void getFileTypes();

};

#endif // CATALOG_H
