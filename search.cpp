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
// File Name:   search.cpp
// Purpose:     class to manage search criteria and results
// Description:
// Author:      Stephane Couturier
/////////////////////////////////////////////////////////////////////////////
*/

#include "search.h"

Search::Search(QObject *parent) : QAbstractTableModel(parent)
{

}

//file list model
int Search::rowCount(const QModelIndex &parent) const
{
    Q_UNUSED(parent);
    return fileNames.length();
}

int Search::columnCount(const QModelIndex &parent) const
{
    Q_UNUSED(parent);
    return 5;
}

QVariant Search::data(const QModelIndex &index, int role) const
{
    if (!index.isValid() || role != Qt::DisplayRole) {
        return QVariant();
    }
    switch (index.column()){
        case 0: return QString(fileNames[index.row()]);
        case 1: return qint64 (fileSizes[index.row()]);
        case 3: return QString(filePaths[index.row()]);
        case 2: return QString(fileDateTimes[index.row()]);
        case 4: return QString(fileCatalogs[index.row()]);
    }
    return QVariant();
}

QVariant Search::headerData(int section, Qt::Orientation orientation, int role) const
{
    if (role == Qt::DisplayRole && orientation == Qt::Horizontal) {
        switch (section){
            case 0: return QString(tr("Name"));
            case 1: return QString(tr("Size"));
            case 3: return QString(tr("Folder"));
            case 2: return QString(tr("Date"));
            case 4: return QString(tr("Catalog"));
        }
    }
    return QVariant();
}

//methods
void Search::loadSearchHistoryCriteria()
{
    //Query
    QSqlQuery query;
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
        differencesCatalogs    = query.value(26).toString().split("||");
        if (differencesCatalogs.length()>1){
            differencesCatalog1 = differencesCatalogs[0];
            differencesCatalog2 = differencesCatalogs[1];
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

void Search::setMultipliers()
{//Define a size multiplier depending on the size unit selected
    sizeMultiplierMin=1;
    if      (selectedMinSizeUnit == QCoreApplication::translate("MainWindow", "KiB"))
            sizeMultiplierMin = sizeMultiplierMin *1024;
    else if (selectedMinSizeUnit == QCoreApplication::translate("MainWindow", "MiB"))
            sizeMultiplierMin = sizeMultiplierMin *1024*1024;
    else if (selectedMinSizeUnit == QCoreApplication::translate("MainWindow", "GiB"))
            sizeMultiplierMin = sizeMultiplierMin *1024*1024*1024;
    else if (selectedMinSizeUnit == QCoreApplication::translate("MainWindow", "TiB"))
            sizeMultiplierMin = sizeMultiplierMin *1024*1024*1024*1024;
    sizeMultiplierMax=1;
    if      (selectedMaxSizeUnit == QCoreApplication::translate("MainWindow", "KiB"))
            sizeMultiplierMax = sizeMultiplierMax *1024;
    else if (selectedMaxSizeUnit == QCoreApplication::translate("MainWindow", "MiB"))
            sizeMultiplierMax = sizeMultiplierMax *1024*1024;
    else if (selectedMaxSizeUnit == QCoreApplication::translate("MainWindow", "GiB"))
            sizeMultiplierMax = sizeMultiplierMax *1024*1024*1024;
    else if (selectedMaxSizeUnit == QCoreApplication::translate("MainWindow", "TiB"))
            sizeMultiplierMax = sizeMultiplierMax *1024*1024*1024*1024;
}
