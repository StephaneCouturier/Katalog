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
// Purpose:     methods for the search class
// Description: https://stephanecouturier.github.io/Katalog/docs/Features/Search
// Author:      Stephane Couturier
/////////////////////////////////////////////////////////////////////////////
*/
#include "search.h"
#include <QCoreApplication>
#include <algorithm>
#include <QFileInfo>

Search::Search(QObject *parent) : QAbstractTableModel(parent)
{
    // Initialize with default values
    deviceFoundModel->setHorizontalHeaderLabels({ QCoreApplication::translate("MainWindow", "Catalog with results"), QCoreApplication::translate("MainWindow", "ID") });

    // Initialize default search parameters
    searchOnFileName = true;
    caseSensitive = false;
    searchOnFileCriteria = false;
    searchOnSize = false;
    selectedMinimumSize = 0;
    selectedMaximumSize = 1000;
    sizeMultiplierMin = 1;
    sizeMultiplierMax = 1;
    searchOnType = false;
    searchOnDate = false;
    searchOnDuplicates = false;
    searchDuplicatesOnName = true;
    searchDuplicatesOnSize = false;
    searchDuplicatesOnDate = false;
    searchOnDifferences = false;
    differencesOnName = true;
    differencesOnSize = false;
    differencesOnDate = false;
    differencesDeviceID1 = 0;
    differencesDeviceID2 = 0;
    searchOnFolderCriteria = false;
    showFoldersOnly = false;
    searchOnTags = false;
    searchInCatalogsChecked = true;
    searchInConnectedChecked = false;

    // Initialize statistics
    filesFoundNumber = 0;
    filesFoundTotalSize = 0;
    filesFoundAverageSize = 0;
    filesFoundMinSize = 0;
    filesFoundMaxSize = 0;

    // Initialize devices for differences
    diffDevice1 = new Device;
    diffDevice2 = new Device;
}

// QAbstractTableModel implementation
int Search::rowCount(const QModelIndex &parent) const
{
    Q_UNUSED(parent);
    return fileNames.length();
}

int Search::columnCount(const QModelIndex &parent) const
{
    Q_UNUSED(parent);
    return 6;
}

QVariant Search::data(const QModelIndex &index, int role) const
{
    if (!index.isValid() || role != Qt::DisplayRole) {
        return QVariant();
    }
    switch (index.column()) {
    case 0: return QString(fileNames[index.row()]);
    case 1: return qint64(fileSizes[index.row()]);
    case 2: return QString(fileDateTimes[index.row()]);
    case 3: return QString(filePaths[index.row()]);
    case 4: return QString(fileCatalogs[index.row()]);
    case 5: return int(fileCatalogIDs[index.row()]);
    }
    return QVariant();
}

QVariant Search::headerData(int section, Qt::Orientation orientation, int role) const
{
    if (role == Qt::DisplayRole && orientation == Qt::Horizontal) {
        switch (section) {
        case 0: return QString(tr("Name"));
        case 1: return QString(tr("Size"));
        case 2: return QString(tr("Date"));
        case 3: return QString(tr("Folder"));
        case 4: return QString(tr("Catalog Name"));
        case 5: return QString(tr("Catalog ID"));
        }
    }
    return QVariant();
}

void Search::prepareSearchPatterns()
{
    // Define how to use the search text
    if (selectedTextCriteria == QCoreApplication::translate("MainWindow", "Exact Phrase"))
        regexSearchtext = searchText; // Just search for the exact text entered including spaces, as one text string
    else if (selectedTextCriteria == QCoreApplication::translate("MainWindow", "Begins With"))
        regexSearchtext = "(^" + searchText + ")";
    else if (selectedTextCriteria == QCoreApplication::translate("MainWindow", "Any Word"))
        regexSearchtext = searchText.replace(" ", "|");
    else if (selectedTextCriteria == QCoreApplication::translate("MainWindow", "All Words")) {
        QString searchTextToSplit = searchText;
        QString groupRegEx = "";
        QRegularExpression lineSplitExp(" ");
        QStringList lineFieldList = searchTextToSplit.split(lineSplitExp);
        int numberOfSearchWords = lineFieldList.count();
        // Build regex group for one word
        for (int i = 0; i < numberOfSearchWords; i++) {
            groupRegEx = groupRegEx + "(?=.*" + lineFieldList[i] + ")";
        }
        regexSearchtext = groupRegEx;
    }
    else {
        regexSearchtext = "";
    }

    regexPattern = regexSearchtext;

    // Prepare the regexFileType for file types
    if (searchOnFileCriteria == true && searchOnType == true && selectedFileType != "All") {
        // Get the list of file extensions and join it into one string
        if (selectedFileType == "Audio") {
            regexFileType = fileType_AudioS.join("|");
        }
        if (selectedFileType == "Image") {
            regexFileType = fileType_ImageS.join("|");
        }
        if (selectedFileType == "Text") {
            regexFileType = fileType_TextS.join("|");
        }
        if (selectedFileType == "Video") {
            regexFileType = fileType_VideoS.join("|");
        }

        // Replace the *. by .* needed for regex
        regexFileType = regexFileType.replace("*.", ".*");

        // Add the file type expression to the regex
        regexPattern = regexSearchtext + "(" + regexFileType + ")";
    }
    else {
        regexPattern = regexSearchtext;
    }

    // Add the words to exclude to the Regular Expression
    if (selectedSearchExclude != "") {
        // Prepare
        QString searchTextToSplit = selectedSearchExclude;
        QString excludeGroupRegEx = "";
        QRegularExpression lineSplitExp(" ");
        QStringList lineFieldList = searchTextToSplit.split(lineSplitExp);
        int numberOfSearchWords = lineFieldList.count();

        // Build regex group to exclude all words
        // Generate first part = first characters + the first word
        excludeGroupRegEx = "^(?!.*(" + lineFieldList[0];
        // Add more words
        for (int i = 1; i < numberOfSearchWords; i++) {
            excludeGroupRegEx = excludeGroupRegEx + "|" + lineFieldList[i];
        }
        // Last part
        excludeGroupRegEx = excludeGroupRegEx + "))";

        // Add regex group to exclude to the global regexPattern
        regexPattern = excludeGroupRegEx + regexPattern;
    }
}

void Search::setMultipliers()
{
    // Define a size multiplier depending on the size unit selected
    sizeMultiplierMin = 1;
    if (selectedMinSizeUnit == QCoreApplication::translate("MainWindow", "KiB"))
        sizeMultiplierMin = sizeMultiplierMin * 1024;
    else if (selectedMinSizeUnit == QCoreApplication::translate("MainWindow", "MiB"))
        sizeMultiplierMin = sizeMultiplierMin * 1024 * 1024;
    else if (selectedMinSizeUnit == QCoreApplication::translate("MainWindow", "GiB"))
        sizeMultiplierMin = sizeMultiplierMin * 1024 * 1024 * 1024;
    else if (selectedMinSizeUnit == QCoreApplication::translate("MainWindow", "TiB"))
        sizeMultiplierMin = sizeMultiplierMin * 1024 * 1024 * 1024 * 1024;

    sizeMultiplierMax = 1;
    if (selectedMaxSizeUnit == QCoreApplication::translate("MainWindow", "KiB"))
        sizeMultiplierMax = sizeMultiplierMax * 1024;
    else if (selectedMaxSizeUnit == QCoreApplication::translate("MainWindow", "MiB"))
        sizeMultiplierMax = sizeMultiplierMax * 1024 * 1024;
    else if (selectedMaxSizeUnit == QCoreApplication::translate("MainWindow", "GiB"))
        sizeMultiplierMax = sizeMultiplierMax * 1024 * 1024 * 1024;
    else if (selectedMaxSizeUnit == QCoreApplication::translate("MainWindow", "TiB"))
        sizeMultiplierMax = sizeMultiplierMax * 1024 * 1024 * 1024 * 1024;
}

void Search::processResults(bool handleFoldersOnly)
{
    // Process search results: list of catalogs with results
    // Remove duplicates so the catalogs are listed only once, and sort the list
    deviceFoundIDList.removeDuplicates();
    deviceFoundIDList.sort();

    // Keep the catalog file name only
    foreach(QString item, deviceFoundIDList) {
        int index = deviceFoundIDList.indexOf(item);
        QFileInfo fileInfo(item);
        deviceFoundIDList[index] = fileInfo.baseName();
    }

    // Set model headers
    deviceFoundModel->setHorizontalHeaderLabels({ QCoreApplication::translate("MainWindow", "Catalog with results"), QCoreApplication::translate("MainWindow", "ID") });

    // Populate model with folders only if this option is selected
    if (handleFoldersOnly && searchOnFolderCriteria == true && showFoldersOnly == true) {
        QMap<QString, QPair<QString, int>> uniqueFilePaths;

        for (int i = 0; i < filePaths.size(); ++i) {
            QString filePath = filePaths.at(i);
            QString fileCatalog = fileCatalogs.at(i);
            int fileCatalogID = fileCatalogIDs.at(i);

            if (!uniqueFilePaths.contains(filePath)) {
                uniqueFilePaths.insert(filePath, qMakePair(fileCatalog, fileCatalogID));
            }
        }

        filePaths.clear();
        fileCatalogs.clear();
        fileCatalogIDs.clear();

        for (auto it = uniqueFilePaths.begin(); it != uniqueFilePaths.end(); ++it) {
            filePaths.append(it.key());
            fileCatalogs.append(it.value().first);
            fileCatalogIDs.append(it.value().second);
        }

        fileNames.clear();
        fileSizes.clear();
        fileDateTimes.clear();

        for (int i = 0; i < filePaths.size(); ++i) {
            fileNames.append("");
            fileSizes.append(0);
            fileDateTimes.append("");
        }
    }
}

void Search::calculateStatistics()
{
    // Files found Statistics
    // Reset from previous search
    filesFoundNumber = 0;
    filesFoundTotalSize = 0;
    filesFoundAverageSize = 0;
    filesFoundMinSize = 0;
    filesFoundMaxSize = 0;
    filesFoundMinDate = "";
    filesFoundMaxDate = "";

    // Number of files found
    filesFoundNumber = fileNames.count();

    // Total size of files found
    qint64 sizeItem;
    filesFoundTotalSize = 0;
    foreach(sizeItem, fileSizes) {
        filesFoundTotalSize = filesFoundTotalSize + sizeItem;
    }

    // Other statistics, covering the case where no results are returned.
    if (filesFoundNumber != 0) {
        filesFoundAverageSize = filesFoundTotalSize / filesFoundNumber;
        QList<qint64> fileSizeList = fileSizes;
        std::sort(fileSizeList.begin(), fileSizeList.end());
        filesFoundMinSize = fileSizeList.first();
        filesFoundMaxSize = fileSizeList.last();

        QList<QString> fileDateList = fileDateTimes;
        std::sort(fileDateList.begin(), fileDateList.end());
        filesFoundMinDate = fileDateList.first();
        filesFoundMaxDate = fileDateList.last();
    }
}

void Search::processDuplicates(const QString &connectionName)
{
    // This is a placeholder implementation that derived classes should override
    // Note: We could make this pure virtual, but providing an empty implementation
    // allows for easier expansion without breaking existing code
}

void Search::processDifferences(const QString &connectionName)
{
    // This is a placeholder implementation that derived classes should override
}

void Search::saveSearchHistoryToTable(const QString &connectionName)
{
    // This is a placeholder implementation that derived classes should override
}

void Search::copyFrom(const Search* other)
{
    if (!other) return;

    // Copy all search parameters
    searchOnFileName = other->searchOnFileName;
    searchText = other->searchText;
    selectedTextCriteria = other->selectedTextCriteria;
    selectedSearchIn = other->selectedSearchIn;
    caseSensitive = other->caseSensitive;
    selectedSearchExclude = other->selectedSearchExclude;

    searchOnFileCriteria = other->searchOnFileCriteria;
    searchOnSize = other->searchOnSize;
    selectedMinimumSize = other->selectedMinimumSize;
    selectedMaximumSize = other->selectedMaximumSize;
    selectedMinSizeUnit = other->selectedMinSizeUnit;
    selectedMaxSizeUnit = other->selectedMaxSizeUnit;
    sizeMultiplierMin = other->sizeMultiplierMin;
    sizeMultiplierMax = other->sizeMultiplierMax;

    searchOnType = other->searchOnType;
    selectedFileType = other->selectedFileType;

    searchOnDate = other->searchOnDate;
    selectedDateMin = other->selectedDateMin;
    selectedDateMax = other->selectedDateMax;

    searchOnDuplicates = other->searchOnDuplicates;
    searchDuplicatesOnName = other->searchDuplicatesOnName;
    searchDuplicatesOnSize = other->searchDuplicatesOnSize;
    searchDuplicatesOnDate = other->searchDuplicatesOnDate;

    searchOnDifferences = other->searchOnDifferences;
    differencesOnName = other->differencesOnName;
    differencesOnSize = other->differencesOnSize;
    differencesOnDate = other->differencesOnDate;
    differencesDevices = other->differencesDevices;
    differencesDeviceID1 = other->differencesDeviceID1;
    differencesDeviceID2 = other->differencesDeviceID2;

    searchOnFolderCriteria = other->searchOnFolderCriteria;
    showFoldersOnly = other->showFoldersOnly;
    searchOnTags = other->searchOnTags;
    selectedTagName = other->selectedTagName;

    selectedStorage = other->selectedStorage;
    selectedCatalog = other->selectedCatalog;
    searchInCatalogsChecked = other->searchInCatalogsChecked;
    searchInConnectedChecked = other->searchInConnectedChecked;
    connectedDirectory = other->connectedDirectory;

    fileType_AudioS = other->fileType_AudioS;
    fileType_ImageS = other->fileType_ImageS;
    fileType_TextS = other->fileType_TextS;
    fileType_VideoS = other->fileType_VideoS;

    // Note: we don't copy results or statistics
}

void Search::clearResults()
{
    fileNames.clear();
    fileSizes.clear();
    fileDateTimes.clear();
    filePaths.clear();
    fileCatalogs.clear();
    fileCatalogIDs.clear();
    filesFoundList.clear();
    deviceFoundIDList.clear();
    deviceFoundModel->clear();
    deviceFoundModel->setHorizontalHeaderLabels({ QCoreApplication::translate("MainWindow", "Catalog with results"), QCoreApplication::translate("MainWindow", "ID") });

    // Reset statistics
    filesFoundNumber = 0;
    filesFoundTotalSize = 0;
    filesFoundAverageSize = 0;
    filesFoundMinSize = 0;
    filesFoundMaxSize = 0;
    filesFoundMinDate = "";
    filesFoundMaxDate = "";
}

void Search::insertSearchHistoryToTable(const QString &connectionName)
{//Save Search to db
    QDateTime nowDateTime = QDateTime::currentDateTime();
    QString searchDateTime = nowDateTime.toString("yyyy-MM-dd hh:mm:ss");

    QSqlQuery query(QSqlDatabase::database(connectionName));
    QString querySQL = QLatin1String(R"(
                                INSERT INTO search(
                                    date_time,
                                    text_checked,
                                    text_phrase,
                                    text_criteria,
                                    text_search_in,
                                    file_criteria_checked,
                                    file_type_checked,
                                    file_type,
                                    file_size_checked,
                                    file_size_min,
                                    file_size_min_unit,
                                    file_size_max,
                                    file_size_max_unit,
                                    date_modified_checked,
                                    date_modified_min,
                                    date_modified_max,
                                    duplicates_checked,
                                    duplicates_name,
                                    duplicates_size,
                                    duplicates_date_modified,
                                    differences_checked,
                                    differences_name,
                                    differences_size,
                                    differences_date_modified,
                                    differences_catalogs,
                                    folder_criteria_checked,
                                    show_folders,
                                    tag_checked,
                                    tag,
                                    search_location,
                                    search_storage,
                                    search_catalog,
                                    search_catalog_checked,
                                    search_directory_checked,
                                    selected_directory,
                                    text_exclude,
                                    case_sensitive
                                )
                                VALUES(
                                    :date_time,
                                    :text_checked,
                                    :text_phrase,
                                    :text_criteria,
                                    :text_search_in,
                                    :file_criteria_checked,
                                    :file_type_checked,
                                    :file_type,
                                    :file_size_checked,
                                    :file_size_min,
                                    :file_size_min_unit,
                                    :file_size_max,
                                    :file_size_max_unit,
                                    :date_modified_checked,
                                    :date_modified_min,
                                    :date_modified_max,
                                    :duplicates_checked,
                                    :duplicates_name,
                                    :duplicates_size,
                                    :duplicates_date_modified,
                                    :differences_checked,
                                    :differences_name,
                                    :differences_size,
                                    :differences_date_modified,
                                    :differences_catalogs,
                                    :folder_criteria_checked,
                                    :show_folders,
                                    :tag_checked,
                                    :tag,
                                    :search_location,
                                    :search_storage,
                                    :search_catalog,
                                    :search_catalog_checked,
                                    :search_directory_checked,
                                    :selected_directory,
                                    :text_exclude,
                                    :case_sensitive
                                )
                )");

    query.prepare(querySQL);
    query.bindValue(":date_time",                 searchDateTime);
    query.bindValue(":text_checked",              searchOnFileName);
    query.bindValue(":text_phrase",               searchText);
    query.bindValue(":text_criteria",             selectedTextCriteria);
    query.bindValue(":text_search_in",            selectedSearchIn);
    query.bindValue(":file_criteria_checked",     searchOnFileCriteria);
    query.bindValue(":file_type_checked",         searchOnType);
    query.bindValue(":file_type",                 selectedFileType);
    query.bindValue(":file_size_checked",         searchOnSize);
    query.bindValue(":file_size_min",             selectedMinimumSize);
    query.bindValue(":file_size_min_unit",        selectedMinSizeUnit);
    query.bindValue(":file_size_max",             selectedMaximumSize);
    query.bindValue(":file_size_max_unit",        selectedMaxSizeUnit);
    query.bindValue(":date_modified_checked",     searchOnDate);
    query.bindValue(":date_modified_min",         selectedDateMin);
    query.bindValue(":date_modified_max",         selectedDateMax);
    query.bindValue(":duplicates_checked",        searchOnDuplicates);
    query.bindValue(":duplicates_name",           searchDuplicatesOnName);
    query.bindValue(":duplicates_size",           searchDuplicatesOnSize);
    query.bindValue(":duplicates_date_modified",  searchDuplicatesOnDate);
    query.bindValue(":differences_checked",       searchOnDifferences);
    query.bindValue(":differences_name",          differencesOnName);
    query.bindValue(":differences_size",          differencesOnSize);
    query.bindValue(":differences_date_modified", differencesOnDate);
    query.bindValue(":differences_catalogs",      QString::number(differencesDeviceID1) + "||" + QString::number(differencesDeviceID2));
    query.bindValue(":folder_criteria_checked",   searchOnFolderCriteria);
    query.bindValue(":show_folders",              showFoldersOnly);
    query.bindValue(":tag_checked",               searchOnTags);
    query.bindValue(":tag",                       selectedTagName);
    query.bindValue(":search_storage",            selectedStorage);
    query.bindValue(":search_catalog",            selectedCatalog);
    query.bindValue(":search_catalog_checked",    searchInCatalogsChecked);
    query.bindValue(":search_directory_checked",  searchInConnectedChecked);
    query.bindValue(":selected_directory",        connectedDirectory);
    query.bindValue(":text_exclude",              selectedSearchExclude);
    query.bindValue(":case_sensitive",            caseSensitive);
    query.exec();
}

void Search::loadSearchHistoryCriteria(const QString &connectionName)
{
    //Query
    QSqlQuery query(QSqlDatabase::database(connectionName));
    QString querySQL = QLatin1String(R"(
                                SELECT
                                    date_time,
                                    text_checked,
                                    text_phrase,
                                    text_criteria,
                                    text_search_in,
                                    case_sensitive,
                                    text_exclude,
                                    file_criteria_checked,
                                    file_size_checked,
                                    file_size_min,
                                    file_size_min_unit,
                                    file_size_max,
                                    file_size_max_unit,
                                    file_type_checked,
                                    file_type,
                                    date_modified_checked,
                                    date_modified_min,
                                    date_modified_max,
                                    duplicates_checked,
                                    duplicates_name,
                                    duplicates_size,
                                    duplicates_date_modified,
                                    differences_checked,
                                    differences_name,
                                    differences_size,
                                    differences_date_modified,
                                    differences_catalogs,
                                    folder_criteria_checked,
                                    show_folders,
                                    tag_checked,
                                    tag,
                                    search_location,
                                    search_storage,
                                    search_catalog,
                                    search_catalog_checked,
                                    search_directory_checked,
                                    selected_directory
                                FROM search
                                WHERE date_time =:date_time
                               )");
    query.prepare(querySQL);
    query.bindValue(":date_time", searchDateTime);
    query.exec();

    if (query.next()){
        searchOnFileName       = query.value(1).toBool();
        searchText             = query.value(2).toString();
        selectedTextCriteria   = query.value(3).toString();
        selectedSearchIn       = query.value(4).toString();
        caseSensitive          = query.value(5).toBool();
        selectedSearchExclude  = query.value(6).toString();
        searchOnFileCriteria   = query.value(7).toBool();
        searchOnSize           = query.value(8).toBool();
        selectedMinimumSize    = query.value(9).toLongLong();
        selectedMinSizeUnit    = query.value(10).toString();
        selectedMaximumSize    = query.value(11).toLongLong();
        selectedMaxSizeUnit    = query.value(12).toString();
        searchOnType           = query.value(13).toBool();
        selectedFileType       = query.value(14).toString();
        searchOnDate           = query.value(15).toBool();
        selectedDateMin        = query.value(16).toDateTime();
        selectedDateMax        = query.value(17).toDateTime();
        searchOnDuplicates     = query.value(18).toBool();
        searchDuplicatesOnName = query.value(19).toBool();
        searchDuplicatesOnSize = query.value(20).toBool();
        searchDuplicatesOnDate = query.value(21).toBool();
        searchOnDifferences    = query.value(22).toBool();
        differencesOnName      = query.value(23).toBool();
        differencesOnSize      = query.value(24).toBool();
        differencesOnDate      = query.value(25).toBool();
        differencesDevices    = query.value(26).toString().split("||");
        if (differencesDevices.length()>1){
            differencesDeviceID1 = differencesDevices[0].toInt();
            differencesDeviceID2 = differencesDevices[1].toInt();
        }
        searchOnFolderCriteria  = query.value(27).toBool();
        showFoldersOnly         = query.value(28).toBool();
        searchOnTags            = query.value(29).toBool();
        selectedTagName         = query.value(30).toString();
        selectedStorage         = query.value(32).toString();
        selectedCatalog         = query.value(33).toString();
        searchInCatalogsChecked = query.value(34).toBool();
        searchInConnectedChecked= query.value(35).toBool();
        connectedDirectory      = query.value(36).toString();
    }
}
