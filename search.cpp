#include "search.h"

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
        //searchDateTime              = query.value(0).toDateTime();
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
        searchOnFolderCriteria      = query.value(27).toBool();
        showFoldersOnly             = query.value(28).toBool();

        searchOnTags                = query.value(29).toBool();
        selectedTagName             = query.value(30).toString();

        selectedLocation        = query.value(31).toString();
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