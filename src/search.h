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
// File Name:   search.h
// Purpose:     Class/model for the search (criteria for running the search of files and folders and results)
// Description:
// Author:      Stephane Couturier
/////////////////////////////////////////////////////////////////////////////
*/

#ifndef SEARCH_H
#define SEARCH_H

#include "src/device.h"
#include "src/filesview.h"
#include <QDateTime>
#include <QStandardItemModel>
#include <QAbstractTableModel>
#include <QCoreApplication>
#include <QSqlQuery>
#include <QStringListModel>

class Search : public QAbstractTableModel
{
    Q_OBJECT

public:
    Search(QObject *parent = nullptr);

    int rowCount(const QModelIndex &parent = QModelIndex()) const override;
    int columnCount(const QModelIndex &parent = QModelIndex()) const override;
    QVariant data(const QModelIndex &index, int role = Qt::DisplayRole) const override;
    QVariant headerData(int section, Qt::Orientation orientation, int role = Qt::DisplayRole) const override;

    //Search Inputs
    QString searchDateTime;
    QString regexPattern;
    QString regexSearchtext;
    QString regexFileType;

    //Search Criteria               //field name in db
    bool searchOnFileName;          //text_checked
    QString searchText;             //text_phrase
    QString selectedTextCriteria;   //text_criteria
    QString selectedSearchIn;       //text_search_in
    bool caseSensitive;
    QString selectedSearchExclude;

    bool searchOnFileCriteria;
    bool searchOnSize;
    qint64 selectedMinimumSize;
    qint64 selectedMaximumSize;
    QString selectedMinSizeUnit;
    QString selectedMaxSizeUnit;
    qint64  sizeMultiplierMin;
    qint64  sizeMultiplierMax;
    bool searchOnType;
    QString selectedFileType;
    bool searchOnDate;
    QDateTime selectedDateMin;
    QDateTime selectedDateMax;

    bool searchOnDuplicates;
    bool searchDuplicatesOnName;
    bool searchDuplicatesOnSize;
    bool searchDuplicatesOnDate;

    bool searchOnDifferences;
    bool differencesOnName;
    bool differencesOnSize;
    bool differencesOnDate;
    QStringList differencesDevices;
    int differencesDeviceID1;
    int differencesDeviceID2;

    bool searchOnFolderCriteria;
    bool showFoldersOnly;
    bool searchOnTags;
    QString selectedTagName;

    QString selectedStorage;
    QString selectedCatalog;
    bool    searchInCatalogsChecked;
    bool    searchInConnectedChecked;
    QString connectedDirectory;

    QStringList fileType_AudioS;
    QStringList fileType_ImageS;
    QStringList fileType_TextS;
    QStringList fileType_VideoS;

    //Search execution
    Device *diffDevice1 = new Device;
    Device *diffDevice2 = new Device;
    void searchFiles(Device *selectedDevice);
    void searchFilesInCatalog(Device *device);
    void searchFilesInDirectory(const QString &sourceDirectory);

    //Results
    QList<QString> fileNames;
    QList<qint64>  fileSizes;
    QList<QString> fileDateTimes;
    QList<QString> filePaths;
    QList<QString> fileCatalogs;
    QList<int>     fileCatalogIDs;

    qint64 filesFoundNumber;
    qint64 filesFoundTotalSize;
    qint64 filesFoundAverageSize;
    qint64 filesFoundMinSize;
    qint64 filesFoundMaxSize;
    QString filesFoundMinDate;
    QString filesFoundMaxDate;

    QStringList filesFoundList;
    QStringList deviceFoundIDList;
    QStandardItemModel *deviceFoundModel = new QStandardItemModel;
    QStringList searchTextList;

    //Methods
    void loadSearchHistoryCriteria();
    void setMultipliers();
    void insertSearchHistoryToTable(QString connectionName);
};

#endif // SEARCH_H
