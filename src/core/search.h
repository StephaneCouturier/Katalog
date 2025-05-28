/*LICENCE
    This file is part of Katalog

    Copyright (C) 2021, the Katalog Development team

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
// File Name:   search.cpp
// Purpose:     header for the search class
// Description: https://stephanecouturier.github.io/Katalog/docs/Features/Search
// Author:      Stephane Couturier
/////////////////////////////////////////////////////////////////////////////
*/

#ifndef SEARCH_H
#define SEARCH_H

#include <QAbstractTableModel>
#include <QObject>
#include <QRegularExpression>
#include <QDateTime>
#include <QStringList>
#include <QMutex>
#include <QStandardItemModel>
#include "device.h"

class Search : public QAbstractTableModel
{
    Q_OBJECT

public:
    explicit Search(QObject *parent = nullptr);
    virtual ~Search() = default;

    // QAbstractTableModel implementation
    int rowCount(const QModelIndex &parent = QModelIndex()) const override;
    int columnCount(const QModelIndex &parent = QModelIndex()) const override;
    QVariant data(const QModelIndex &index, int role = Qt::DisplayRole) const override;
    QVariant headerData(int section, Qt::Orientation orientation, int role = Qt::DisplayRole) const override;

    // Search criteria and configuration
    QString regexPattern;
    QString regexSearchtext;
    QString regexFileType;
    bool searchOnFileName;
    QString searchText;
    QString selectedTextCriteria;
    QString selectedSearchIn;
    bool caseSensitive;
    QString selectedSearchExclude;
    bool searchOnFileCriteria;
    bool searchOnSize;
    qint64 selectedMinimumSize = 0;
    qint64 selectedMaximumSize = 0;
    QString selectedMinSizeUnit;
    QString selectedMaxSizeUnit;
    qint64 sizeMultiplierMin;
    qint64 sizeMultiplierMax;
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
    bool searchInCatalogsChecked;
    bool searchInConnectedChecked;
    QString connectedDirectory;
    QString searchDateTime;

    // File type lists
    QStringList fileType_AudioS;
    QStringList fileType_ImageS;
    QStringList fileType_TextS;
    QStringList fileType_VideoS;

    // Results
    QList<QString> fileNames;
    QList<qint64> fileSizes;
    QList<QString> fileDateTimes;
    QList<QString> filePaths;
    QList<QString> fileCatalogs;
    QList<int> fileCatalogIDs;
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
    qint64 totalFilesProcessed = 0;
    qint64 estimatedTotalFiles = 0; //for percentage calculation
    void initializeProgressTracking(Device *selectedDevice);
    void updateProgress(int increment);

    // Device objects for differences
    Device *diffDevice1 = nullptr;
    Device *diffDevice2 = nullptr;

    //For search progress tracking
    int currentCatalogIndex;
    int totalCatalogs;
    QString currentCatalogName;

    // Core search functionality
    void prepareSearchPatterns();
    void setMultipliers();
    void processResults(bool handleFoldersOnly = true);
    void calculateStatistics();
    virtual void processDuplicates(const QString &connectionName);
    virtual void processDifferences(const QString &connectionName);
    void saveSearchHistoryToTable(const QString &connectionName);
    void loadSearchHistoryCriteria(const QString &connectionName);

    // Abstract methods to be implemented by derived classes
    virtual void searchFilesInCatalog(Device *device, QMutex &mutex, bool &stopRequested) = 0;
    virtual void searchFilesInDirectory(const QString &sourceDirectory, QMutex &mutex, bool &stopRequested) = 0;

    // Import/Export of search parameters
    virtual void copyFrom(const Search* other);
    virtual void clearResults();

signals:
    void searchProgress(int filesProcessed);
};

#endif // SEARCH_H
