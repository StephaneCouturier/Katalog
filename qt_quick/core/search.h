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

#ifndef SEARCH_SYNC_H
#define SEARCH_SYNC_H

#include <QDateTime>
#include <QStandardItemModel>
#include <QAbstractTableModel>
#include <QCoreApplication>
#include <QSqlQuery>
#include <QStringListModel>
#include <QSqlQueryModel>
#include "core/device.h"
#include "filesview.h"

class SearchSync : public QAbstractTableModel
{
    Q_OBJECT

    Q_PROPERTY(QVariantMap properties READ properties WRITE setProperties NOTIFY propertiesChanged)

public:
    SearchSync(QObject *parent = nullptr);

    //Q_PROPERTY sources
    QVariantMap properties() const {
        QVariantMap map;
        //Search file
            //Search text
            map["searchOnFileName"] = searchOnFileName;
            map["searchText"] = searchText;
            map["selectedSearchWith"] = selectedSearchWith;
            map["selectedSearchIn"] = selectedSearchIn;
            map["caseSensitive"] = caseSensitive;
            map["selectedFileType"] = selectedFileType;
            map["selectedSearchExclude"] = selectedSearchExclude;
            //File attributes
            //Size
            map["searchOnSize"] = searchOnSize;
            map["selectedMinimumSize"] = selectedMinimumSize;
            map["selectedMaximumSize"] = selectedMaximumSize;
            map["selectedMinSizeUnit"] = selectedMinSizeUnit;
            map["selectedMaxSizeUnit"] = selectedMaxSizeUnit;
            map["searchOnType"] = searchOnType;
            //Date
            map["searchOnDate"] = searchOnDate;
            map["selectedDateMin"] = selectedDateMin;
            map["selectedDateMax"] = selectedDateMax;
            //Duplicates
            map["searchOnDuplicates"] = searchOnDuplicates;
            map["searchDuplicatesOnName"] = searchDuplicatesOnName;
            map["searchDuplicatesOnSize"] = searchDuplicatesOnSize;
            map["searchDuplicatesOnDate"] = searchDuplicatesOnDate;
            //Differences
            map["searchOnDifferences"] = searchOnDifferences;
            map["differencesOnName"] = differencesOnName;
            map["differencesOnSize"] = differencesOnSize;
            map["differencesOnDate"] = differencesOnDate;
            map["differencesDevices"] = differencesDevices;
            map["differencesDevice1"] = differencesDevice1;
            map["differencesDevice2"] = differencesDevice2;
        //Search results
            map["filesFoundNumber"] = filesFoundNumber;
            map["filesFoundTotalSize"] = filesFoundTotalSize;
            map["filesFoundAverageSize"] = filesFoundAverageSize;
            map["filesFoundMinSize"] = filesFoundMinSize;
            map["filesFoundMaxSize"] = filesFoundMaxSize;
            map["filesFoundMinDate"] = filesFoundMinDate;
            map["filesFoundMaxDate"] = filesFoundMaxDate;
        //Global search options
            map["searchInCatalogsChecked"] = searchInCatalogsChecked;

        return map;
    }

    void setProperties(const QVariantMap &map) {
        //Search file name
            if (map.contains("searchOnFileName"))       { searchOnFileName          = map["searchOnFileName"].toBool(); }
            if (map.contains("searchText"))             { searchText                = map["searchText"].toString(); }
            if (map.contains("selectedSearchWith"))     { selectedSearchWith        = map["selectedSearchWith"].toString(); }
            if (map.contains("selectedSearchIn"))       { selectedSearchIn          = map["selectedSearchIn"].toString(); }
            if (map.contains("caseSensitive"))          { caseSensitive             = map["caseSensitive"].toBool(); }
            if (map.contains("selectedFileType"))       { selectedFileType          = map["selectedFileType"].toString(); }
            if (map.contains("selectedSearchExclude"))  { selectedSearchExclude     = map["selectedSearchExclude"].toString(); }

        //File attributes
            if (map.contains("searchOnFileCriteria"))   { searchOnFileCriteria      = map["searchOnFileCriteria"].toBool(); }

            //Size
            if (map.contains("searchOnSize"))           { searchOnSize              = map["searchOnSize"].toBool(); }
            if (map.contains("selectedMinimumSize"))    { selectedMinimumSize       = map["selectedMinimumSize"].toInt(); }
            if (map.contains("selectedMaximumSize"))    { selectedMaximumSize       = map["selectedMaximumSize"].toInt(); }
            if (map.contains("selectedMinSizeUnit"))    { selectedMinSizeUnit       = map["selectedMinSizeUnit"].toString(); }
            if (map.contains("selectedMaxSizeUnit"))    { selectedMaxSizeUnit       = map["selectedMaxSizeUnit"].toString(); }
            //Type
            if (map.contains("searchOnType"))           { searchOnType              = map["searchOnType"].toBool(); }
            if (map.contains("selectedFileType"))       { selectedFileType          = map["selectedFileType"].toString(); }

            //Date
            if (map.contains("searchOnDate"))           { searchOnDate              = map["searchOnDate"].toBool(); }
            if (map.contains("selectedDateMin"))        { selectedDateMin           = map["selectedDateMin"].toDateTime(); }
            if (map.contains("selectedDateMax"))        { selectedDateMax           = map["selectedDateMax"].toDateTime(); }

        //Folder attributes
            if (map.contains("searchOnFolderCriteria")) { searchOnFolderCriteria    = map["searchOnFolderCriteria"].toBool(); }
            if (map.contains("showFoldersOnly"))        { showFoldersOnly           = map["showFoldersOnly"].toBool(); }
            if (map.contains("searchOnTags"))           { searchOnTags              = map["searchOnTags"].toBool(); }
            if (map.contains("selectedTagName"))        { selectedTagName           = map["selectedTagName"].toString(); }

        //Duplicates
            if (map.contains("searchOnDuplicates"))     { searchOnDuplicates        = map["searchOnDuplicates"].toBool(); }
            if (map.contains("searchDuplicatesOnName")) { searchDuplicatesOnName    = map["searchDuplicatesOnName"].toBool(); }
            if (map.contains("searchDuplicatesOnSize")) { searchDuplicatesOnSize    = map["searchDuplicatesOnSize"].toBool(); }
            if (map.contains("searchDuplicatesOnDate")) { searchDuplicatesOnDate    = map["searchDuplicatesOnDate"].toBool(); }

        //Differences
            if (map.contains("searchOnDifferences"))    { searchOnDifferences       = map["searchOnDifferences"].toBool(); }
            if (map.contains("differencesOnName"))      { differencesOnName         = map["differencesOnName"].toBool(); }
            if (map.contains("differencesOnSize"))      { differencesOnSize         = map["differencesOnSize"].toBool(); }
            if (map.contains("differencesOnDate"))      { differencesOnDate         = map["differencesOnDate"].toBool(); }
            if (map.contains("differencesDevices"))     { differencesDevices        = map["differencesDevices"].toStringList(); }
            if (map.contains("differencesDevice1"))     { differencesDevice1        = map["differencesDevice1"].toInt(); }
            if (map.contains("differencesDevice2"))     { differencesDevice2        = map["differencesDevice2"].toInt(); }

        //Search results
            if (map.contains("filesFoundNumber"))       { filesFoundNumber          = map["filesFoundNumber"].toInt(); }
            if (map.contains("filesFoundTotalSize"))    { filesFoundTotalSize       = map["filesFoundTotalSize"].toLongLong(); }
            if (map.contains("filesFoundAverageSize"))  { filesFoundAverageSize     = map["filesFoundAverageSize"].toLongLong(); }
            if (map.contains("filesFoundMinSize"))      { filesFoundMinSize         = map["filesFoundMinSize"].toLongLong(); }
            if (map.contains("filesFoundMaxSize"))      { filesFoundMaxSize         = map["filesFoundMaxSize"].toLongLong(); }
            if (map.contains("filesFoundMinDate"))      { filesFoundMinDate         = map["filesFoundMinDate"].toString(); }
            if (map.contains("filesFoundMaxDate"))      { filesFoundMaxDate         = map["filesFoundMaxDate"].toString(); }

        //Global search options
            if (map.contains("searchInCatalogsChecked")){ searchInCatalogsChecked   = map["searchInCatalogsChecked"].toBool(); }

        emit propertiesChanged();
    }

    //Results data model
    int rowCount(const QModelIndex &parent = QModelIndex()) const override;
    int columnCount(const QModelIndex &parent = QModelIndex()) const override;
    QVariant data(const QModelIndex &index, int role = Qt::DisplayRole) const override;
    QVariant headerData(int section, Qt::Orientation orientation, int role = Qt::DisplayRole) const override;


    //Attributes to move to Q_PROPERTY
    //Search Inputs
    QString searchDateTime;
    QString regexPattern;
    QString regexSearchtext;
    QString regexFileType;

    //FileTypes
    QStringList fileType_Image;
    QStringList fileType_Audio;
    QStringList fileType_Video;
    QStringList fileType_Text;
    QStringList fileType_ImageS;
    QStringList fileType_AudioS;
    QStringList fileType_VideoS;
    QStringList fileType_TextS;
    QStringList fileType_current;
    void setFileTypes();

    QString selectedStorage;
    QString selectedCatalog;
    //bool searchInCatalogsChecked;
    bool searchInConnectedChecked;
    QString connectedDirectory;

    //Results
    QList<QString> fileNames;
    QList<qint64>  fileSizes;
    QList<QString> filePaths;
    QList<QString> fileDateTimes;
    QList<QString> fileCatalogs;

    QStringList filesFoundList;
    QStringList deviceFoundIDList;
    QStandardItemModel *deviceFoundModel = new QStandardItemModel;
    QStringList searchTextList;

signals:
    void propertiesChanged();
    void filesFoundNumberChanged();

public slots:
    QString testFunction();

    void setMultipliers();

    void resetSearchResults();
    void searchFiles(Device *selectedDevice);
    void searchFilesInCatalog(Device *device);
    void searchFilesInDirectory(const QString &sourceDirectory);

private:
    //File name search
    bool searchOnFileName;
    QString searchText;
    QString selectedSearchWith;
    QString selectedSearchIn;
    bool caseSensitive;
    QString selectedSearchExclude;

    //File attributes
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

    //Folder attributes
    bool searchOnFolderCriteria;
    bool showFoldersOnly;
    bool searchOnTags;
    QString selectedTagName;

    //Duplicates
    bool searchOnDuplicates;
    bool searchDuplicatesOnName;
    bool searchDuplicatesOnSize;
    bool searchDuplicatesOnDate;

    //Differences
    bool searchOnDifferences;
    bool differencesOnName;
    bool differencesOnSize;
    bool differencesOnDate;
    QStringList differencesDevices;
    int differencesDevice1;
    int differencesDevice2;

    //Search results
    qint64 filesFoundNumber;
    qint64 filesFoundTotalSize;
    qint64 filesFoundAverageSize;
    qint64 filesFoundMinSize;
    qint64 filesFoundMaxSize;
    QString filesFoundMinDate;
    QString filesFoundMaxDate;

    //Global search options
    bool searchInCatalogsChecked;
};

#endif // SEARCH_SYNC_H
