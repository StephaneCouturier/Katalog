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
#include "filemetadata.h"
#include "filetypemapping.h"
#include <algorithm>
#include <QFileInfo>
#include <qapplication.h>

// Constants
const QString Search::SEARCH_IN_FILE_NAMES = "FileNamesOnly";
const QString Search::SEARCH_IN_FILES_AND_FOLDERS = "FilesAndFolders";
const QString Search::SEARCH_IN_FOLDER_PATH = "FolderPathOnly";

const QString Search::TEXT_CRITERIA_ALL_WORDS = "AllWords";
const QString Search::TEXT_CRITERIA_EXACT_PHRASE = "ExactPhrase";
const QString Search::TEXT_CRITERIA_BEGINS_WITH = "BeginsWith";
const QString Search::TEXT_CRITERIA_ANY_WORD = "AnyWord";

const QString Search::SIZE_UNIT_BYTES = "Bytes";
const QString Search::SIZE_UNIT_KIB = "KiB";
const QString Search::SIZE_UNIT_MIB = "MiB";
const QString Search::SIZE_UNIT_GIB = "GiB";
const QString Search::SIZE_UNIT_TIB = "TiB";

Search::Search(QObject *parent) : QAbstractTableModel(parent)
{
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
    searchDuplicatesOnChecksum = false;
    searchDuplicatesChecksumEqual = true;  // default to "="
    duplicatesCompareDevices = false;      // default to Within selected device
    duplicatesDeviceID1 = 0;
    duplicatesDeviceID2 = 0;
    differencesOnChecksum = false;
    differencesChecksumEqual = true;       // default to "="
    searchOnFolderCriteria = false;
    showFoldersOnly = false;
    searchOnTags = false;
    searchInCatalogsChecked = true;
    searchInConnectedChecked = false;
    searchOnFileMetadata = false;
    searchOnMetadataText = false;
    metadataTextSearch = "";
    searchOnFileMetadata = false;
    searchOnMetadataText = false;
    metadataTextSearch = "";
    searchOnMetadataSize = false;
    metadataMinimumHeight = 0;
    metadataMaximumHeight = 1000;
    metadataMinimumWidth = 0;
    metadataMaximumWidth = 1000;
    searchOnMetadataDuration = false;
    metadataDurationMin = QDateTime(QDate(1970, 1, 1), QTime(0, 0, 0));
    metadataDurationMax = QDateTime(QDate(2030, 1, 1), QTime(23, 59, 59));

    // Initialize statistics
    filesFoundNumber = 0;
    filesFoundTotalSize = 0;
    filesFoundAverageSize = 0;
    filesFoundMinSize = 0;
    filesFoundMaxSize = 0;

    // Initialize devices for differences
    diffDevice1 = new Device;
    diffDevice2 = new Device;

    // Initialize catalog tracking
    currentCatalogIndex = 0;
    totalCatalogs = 0;
    currentCatalogName = "";
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
    return 21; // Aligned with Explore model
}

QVariant Search::data(const QModelIndex &index, int role) const
{
    if (role != Qt::DisplayRole) return QVariant();

    switch (index.column()) {
    case 0: return QString(fileNames[index.row()]);
    case 1: return QVariant(fileSizes[index.row()]);
    case 2: return QString(fileDateTimes[index.row()]);
    case 3: return QString(filePaths[index.row()]);        // Directory
    case 4: return QString(fileCatalogs[index.row()]);     // Catalog
    case 5: return int(fileCatalogIDs[index.row()]);       // Type (ID for now)
    case 6: return QVariant(index.row());                  // orderValue (row index)
    case 7: return QString(filePaths[index.row()] + "/" + fileNames[index.row()]); // Path (full path)
    // Metadata - WITH BOUNDS CHECKING to prevent crashes
    case 8: return (index.row() < fileTypes.size()) ? QString(fileTypes[index.row()]) : QString("");
    case 9: return (index.row() < mimeTypes.size()) ? QString(mimeTypes[index.row()]) : QString("");
    case 10: return (index.row() < imageWidths.size() && imageWidths[index.row()] > 0) ? imageWidths[index.row()] : QVariant();
    case 11: return (index.row() < imageHeights.size() && imageHeights[index.row()] > 0) ? imageHeights[index.row()] : QVariant();
    case 12: return (index.row() < videoDurations.size() && videoDurations[index.row()] > 0) ? videoDurations[index.row()] : QVariant();
    case 13: return (index.row() < videoWidths.size() && videoWidths[index.row()] > 0) ? videoWidths[index.row()] : QVariant();
    case 14: return (index.row() < videoHeights.size() && videoHeights[index.row()] > 0) ? videoHeights[index.row()] : QVariant();
    case 15: return (index.row() < audioDurations.size() && audioDurations[index.row()] > 0) ? audioDurations[index.row()] : QVariant();
    case 16: return (index.row() < audioArtists.size()) ? QString(audioArtists[index.row()]) : QString("");
    case 17: return (index.row() < audioAlbums.size()) ? QString(audioAlbums[index.row()]) : QString("");
    case 18: return (index.row() < audioTitles.size()) ? QString(audioTitles[index.row()]) : QString("");
    case 19: return (index.row() < checksumSha256s.size()) ? QString(checksumSha256s[index.row()]) : QString("");
    case 20: return (index.row() < checksumExtractionDates.size()) ? QString(checksumExtractionDates[index.row()]) : QString("");
    }
    return QVariant();
}

QVariant Search::headerData(int section, Qt::Orientation orientation, int role) const
{
    if (role == Qt::DisplayRole && orientation == Qt::Horizontal) {
        switch (section) {
        case 0: return QString(QApplication::translate("MainWindow","Name"));
        case 1: return QString(QApplication::translate("MainWindow","Size"));
        case 2: return QString(QApplication::translate("MainWindow","Date"));
        case 3: return QString(QApplication::translate("MainWindow","Directory"));
        case 4: return QString(QApplication::translate("MainWindow","Catalog Name"));
        case 5: return QString(QApplication::translate("MainWindow","Catalog ID"));
        case 6: return QString(QApplication::translate("MainWindow","orderValue"));
        case 7: return QString(QApplication::translate("MainWindow","Path"));
        // Metadata - aligned with Explore
        case 8: return QString(QApplication::translate("MainWindow","File Type"));
        case 9: return QString(QApplication::translate("MainWindow","MIME Type"));
        case 10: return QString(QApplication::translate("MainWindow","Width"));           //Generic for Image and Video
        case 11: return QString(QApplication::translate("MainWindow","Height"));          //Generic for Image and Video
        case 12: return QString(QApplication::translate("MainWindow","Duration"));        //Generic for Audio and Video
        case 13: return QString(QApplication::translate("MainWindow","Video Width"));
        case 14: return QString(QApplication::translate("MainWindow","Video Height"));
        case 15: return QString(QApplication::translate("MainWindow","Audio Duration"));
        case 16: return QString(QApplication::translate("MainWindow","Artist"));
        case 17: return QString(QApplication::translate("MainWindow","Album"));
        case 18: return QString(QApplication::translate("MainWindow","Title"));
        case 19: return QString(QApplication::translate("MainWindow","Checksum")+" (SHA256)");
        case 20: return QString(QApplication::translate("MainWindow","Checksum Date"));
        }
    }
    return QVariant();
}

QString Search::buildMetadataSearchConditions() const
{
    if (!searchOnFileMetadata || !searchOnMetadataText || metadataTextSearch.isEmpty()) {
        return QString();
    }

    // Simple text search - just "contains" logic
    QString searchPattern = caseSensitive ? metadataTextSearch : metadataTextSearch.toLower();

    // Define basic metadata text fields to search in
    QStringList basicMetadataFields = {
        "audio_artist", "audio_album", "audio_title", "audio_genre",
        "video_codec", "mime_type", "file_type"
    };

    // Build conditions for basic metadata fields
    QStringList basicConditions;
    for (const QString &field : basicMetadataFields) {
        QString condition;
        if (caseSensitive) {
            condition = QString("%1 LIKE '%%2%'").arg(field, searchPattern);
        } else {
            condition = QString("LOWER(%1) LIKE '%%2%'").arg(field, searchPattern);
        }
        basicConditions.append(condition);
    }

    // Add condition for JSON metadata_extended field
    QString jsonCondition;
    if (caseSensitive) {
        jsonCondition = QString("metadata_extended LIKE '%%1%'").arg(searchPattern);
    } else {
        jsonCondition = QString("LOWER(metadata_extended) LIKE '%%1%'").arg(searchPattern);
    }

    // Combine all conditions with OR
    QStringList allConditions = basicConditions;
    allConditions.append(jsonCondition);

    return "(" + allConditions.join(" OR ") + ")";
}

//Search Processing
void Search::prepareSearchPatterns()
{
    // Define how to use the search text
    if (selectedTextCriteria == TEXT_CRITERIA_EXACT_PHRASE)
        regexSearchtext = searchText; // Just search for the exact text entered including spaces, as one text string
    else if (selectedTextCriteria == TEXT_CRITERIA_BEGINS_WITH)
        regexSearchtext = "(^" + searchText + ")";
    else if (selectedTextCriteria == TEXT_CRITERIA_ANY_WORD)
        regexSearchtext = searchText.replace(" ", "|");
    else if (selectedTextCriteria == TEXT_CRITERIA_ALL_WORDS) {
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

    // No regex-based file type filtering when using FileTypeMapping
    // The SQL query already handles file type filtering via FileTypeMapping::getSqlFilter()
    if (searchOnFileCriteria == true && searchOnType == true && selectedFileType != "All") {
        // Skip regex-based file type filtering - rely on SQL filtering only
        regexFileType = "";
        regexPattern = regexSearchtext;
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

QStringList Search::getExtensionsForFileType(const QString &fileType)
{
    if (fileType.toLower() == "none") {
        // "none" type represents extensionless files - return empty list
        return QStringList();
    }

    // Initialize FileMetadata cache if needed
    FileMetadata::initializeExtensionTypeCache();

    QStringList extensions;
    const auto& cache = FileMetadata::getExtensionToTypeCache();

    for (auto it = cache.begin(); it != cache.end(); ++it) {
        if (it.value() == fileType.toLower()) {
            extensions.append(it.key());
        }
    }

    qDebug() << "getExtensionsForFileType(" << fileType << ") found extensions:" << extensions;
    return extensions;
}

void Search::setMultipliers()
{
    // Define a size multiplier depending on the size unit selected
    sizeMultiplierMin = 1;
    if (selectedMinSizeUnit == SIZE_UNIT_KIB)
        sizeMultiplierMin = sizeMultiplierMin * 1024;
    else if (selectedMinSizeUnit == SIZE_UNIT_MIB)
        sizeMultiplierMin = sizeMultiplierMin * 1024 * 1024;
    else if (selectedMinSizeUnit == SIZE_UNIT_GIB)
        sizeMultiplierMin = sizeMultiplierMin * 1024 * 1024 * 1024;
    else if (selectedMinSizeUnit == SIZE_UNIT_TIB)
        sizeMultiplierMin = sizeMultiplierMin * 1024 * 1024 * 1024 * 1024;

    sizeMultiplierMax = 1;
    if (selectedMaxSizeUnit == SIZE_UNIT_KIB)
        sizeMultiplierMax = sizeMultiplierMax * 1024;
    else if (selectedMaxSizeUnit == SIZE_UNIT_MIB)
        sizeMultiplierMax = sizeMultiplierMax * 1024 * 1024;
    else if (selectedMaxSizeUnit == SIZE_UNIT_GIB)
        sizeMultiplierMax = sizeMultiplierMax * 1024 * 1024 * 1024;
    else if (selectedMaxSizeUnit == SIZE_UNIT_TIB)
        sizeMultiplierMax = sizeMultiplierMax * 1024 * 1024 * 1024 * 1024;
}

void Search::processResults()
{
    // Process search results: list of catalogs with results
    deviceFoundModel->clear();

    // Populate the model with unique catalogs from search results
    QMap<int, QString> uniqueCatalogs; // Map of catalog IDs to names

    // Extract unique catalogs from results
    for (int i = 0; i < fileCatalogIDs.size(); ++i) {
        int catalogID = fileCatalogIDs.at(i);
        QString catalogName = fileCatalogs.at(i);
        if (!uniqueCatalogs.contains(catalogID)) {
            uniqueCatalogs.insert(catalogID, catalogName);
        }
    }

    // Add each unique catalog to the model
    for (auto it = uniqueCatalogs.begin(); it != uniqueCatalogs.end(); ++it) {
        int catalogID = it.key();
        QString catalogName = it.value();

        // Query the device table to get the device ID for this catalog
        QSqlQuery query(QSqlDatabase::database(m_connectionName));
        QString querySQL = QLatin1String(R"(
            SELECT device_id
            FROM device
            WHERE device_external_id = :catalog_id
            AND device_type = 'Catalog'
        )");
        query.prepare(querySQL);
        query.bindValue(":catalog_id", catalogID);
        query.exec();

        int deviceID = 0;
        if (query.next()) {
            deviceID = query.value(0).toInt();
        }

        QList<QStandardItem*> rowItems;
        QStandardItem* nameItem = new QStandardItem(catalogName);
        QStandardItem* idItem = new QStandardItem();

        // Set the device ID as integer data
        idItem->setData(deviceID, Qt::DisplayRole);

        rowItems << nameItem << idItem;
        deviceFoundModel->appendRow(rowItems);
    }

    // Process folders only if this option is selected
    if (searchOnFolderCriteria == true && showFoldersOnly == true) {
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
        fileTypes.clear();
        mimeTypes.clear();

        // Clear metadata arrays too
        imageWidths.clear();
        imageHeights.clear();
        videoDurations.clear();
        videoWidths.clear();
        videoHeights.clear();
        audioDurations.clear();
        audioArtists.clear();
        audioAlbums.clear();
        audioTitles.clear();

        for (int i = 0; i < filePaths.size(); ++i) {
            fileNames.append("");
            fileSizes.append(0);
            fileDateTimes.append("");
            fileTypes.append("folder");  // Mark as folder for icon display
            mimeTypes.append("");        // No mime type for folders

            // Append empty metadata values
            imageWidths.append(0);
            imageHeights.append(0);
            videoDurations.append(0);
            videoWidths.append(0);
            videoHeights.append(0);
            audioDurations.append(0);
            audioArtists.append("");
            audioAlbums.append("");
            audioTitles.append("");
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
    // Save Search to db
    QDateTime nowDateTime = QDateTime::currentDateTime();
    searchDateTime = nowDateTime.toString("yyyy-MM-dd hh:mm:ss");
    QString deviceListStr = "";
    if (!selectedDeviceIDList.isEmpty()) {
        QStringList idStrings;
        for (int id : std::as_const(selectedDeviceIDList)) {
            idStrings << QString::number(id);
        }
        deviceListStr = idStrings.join(",");
    }

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
            duplicates_checksum,
            duplicates_checksum_equal,
            duplicates_compare_checked,
            duplicates_device1_ID,
            duplicates_device2_ID,
            differences_checked,
            differences_name,
            differences_size,
            differences_date_modified,
            differences_checksum,
            differences_checksum_equal,
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
            selected_device_ID_list,
            text_exclude,
            case_sensitive,
            metadata_checked,
            metadata_text_checked,
            metadata_text_search,
            metadata_size_checked,
            metadata_size_min_height,
            metadata_size_max_height,
            metadata_size_min_width,
            metadata_size_max_width,
            metadata_duration_checked,
            metadata_duration_min,
            metadata_duration_max
        ) VALUES(
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
            :duplicates_checksum,
            :duplicates_checksum_equal,
            :duplicates_compare_checked,
            :duplicates_device1_ID,
            :duplicates_device2_ID,
            :differences_checked,
            :differences_name,
            :differences_size,
            :differences_date_modified,
            :differences_checksum,
            :differences_checksum_equal,
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
            :selected_device_ID_list,
            :text_exclude,
            :case_sensitive,
            :metadata_checked,
            :metadata_text_checked,
            :metadata_text_search,
            :metadata_size_checked,
            :metadata_size_min_height,
            :metadata_size_max_height,
            :metadata_size_min_width,
            :metadata_size_max_width,
            :metadata_duration_checked,
            :metadata_duration_min,
            :metadata_duration_max
        )
    )");

    query.prepare(querySQL);
    query.bindValue(":date_time", searchDateTime);
    query.bindValue(":text_checked", searchOnFileName);
    query.bindValue(":text_phrase", searchText);
    query.bindValue(":text_criteria", selectedTextCriteria);
    query.bindValue(":text_search_in", selectedSearchIn);
    query.bindValue(":file_criteria_checked", searchOnFileCriteria);
    query.bindValue(":file_type_checked", searchOnType);
    query.bindValue(":file_type", selectedFileType);
    query.bindValue(":file_size_checked", searchOnSize);
    query.bindValue(":file_size_min", selectedMinimumSize);
    query.bindValue(":file_size_min_unit", selectedMinSizeUnit);
    query.bindValue(":file_size_max", selectedMaximumSize);
    query.bindValue(":file_size_max_unit", selectedMaxSizeUnit);
    query.bindValue(":date_modified_checked", searchOnDate);
    query.bindValue(":date_modified_min", selectedDateMin);
    query.bindValue(":date_modified_max", selectedDateMax);
    query.bindValue(":duplicates_checked", searchOnDuplicates);
    query.bindValue(":duplicates_name", searchDuplicatesOnName);
    query.bindValue(":duplicates_size", searchDuplicatesOnSize);
    query.bindValue(":duplicates_date_modified", searchDuplicatesOnDate);
    query.bindValue(":duplicates_compare_checked", duplicatesCompareDevices);
    query.bindValue(":duplicates_device1_ID", duplicatesDeviceID1);
    query.bindValue(":duplicates_device2_ID", duplicatesDeviceID2);
    query.bindValue(":differences_checked", searchOnDifferences);
    query.bindValue(":differences_name", differencesOnName);
    query.bindValue(":differences_size", differencesOnSize);
    query.bindValue(":differences_date_modified", differencesOnDate);
    query.bindValue(":differences_catalogs", QString::number(differencesDeviceID1) + "||" + QString::number(differencesDeviceID2));
    query.bindValue(":folder_criteria_checked", searchOnFolderCriteria);
    query.bindValue(":show_folders", showFoldersOnly);
    query.bindValue(":tag_checked", searchOnTags);
    query.bindValue(":tag", selectedTagName);
    query.bindValue(":search_storage", selectedStorage);
    query.bindValue(":search_catalog", selectedCatalog);
    query.bindValue(":search_catalog_checked", searchInCatalogsChecked);
    query.bindValue(":search_directory_checked", searchInConnectedChecked);
    query.bindValue(":selected_directory", connectedDirectory);
    query.bindValue(":selected_device_ID_list", deviceListStr);
    query.bindValue(":text_exclude", selectedSearchExclude);
    query.bindValue(":case_sensitive", caseSensitive);
    query.bindValue(":metadata_checked", searchOnFileMetadata);
    query.bindValue(":metadata_text_checked", searchOnMetadataText);
    query.bindValue(":metadata_text_search", metadataTextSearch);
    query.bindValue(":metadata_size_checked", searchOnMetadataSize);
    query.bindValue(":metadata_size_min_height", metadataMinimumHeight);
    query.bindValue(":metadata_size_max_height", metadataMaximumHeight);
    query.bindValue(":metadata_size_min_width", metadataMinimumWidth);
    query.bindValue(":metadata_size_max_width", metadataMaximumWidth);
    query.bindValue(":metadata_duration_checked", searchOnMetadataDuration);
    query.bindValue(":metadata_duration_min", metadataDurationMin.toString("HH:mm:ss"));
    query.bindValue(":metadata_duration_max", metadataDurationMax.toString("HH:mm:ss"));
    query.bindValue(":duplicates_checksum", searchDuplicatesOnChecksum);
    query.bindValue(":duplicates_checksum_equal", searchDuplicatesChecksumEqual);
    query.bindValue(":differences_checksum", differencesOnChecksum);
    query.bindValue(":differences_checksum_equal", differencesChecksumEqual);
    query.exec();
    qDebug() << "Search::saveSearchHistoryToTable: lastError" << query.lastError();
}

void Search::loadSearchHistoryCriteria(const QString &connectionName)
{
    // Query
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
            duplicates_checksum,
            duplicates_checksum_equal,
            duplicates_compare_checked,
            duplicates_device1_ID,
            duplicates_device2_ID,
            differences_checked,
            differences_name,
            differences_size,
            differences_date_modified,
            differences_checksum,
            differences_checksum_equal,
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
            selected_device_ID_list,
            metadata_checked,
            metadata_text_checked,
            metadata_text_search,
            metadata_size_checked,
            metadata_size_min_height,
            metadata_size_max_height,
            metadata_size_min_width,
            metadata_size_max_width,
            metadata_duration_checked,
            metadata_duration_min,
            metadata_duration_max
        FROM search
        WHERE date_time =:date_time
    )");
    query.prepare(querySQL);
    query.bindValue(":date_time", searchDateTime);
    query.exec();

    if (query.next()) {
        searchOnFileName = query.value(1).toBool();
        searchText = query.value(2).toString();
        selectedTextCriteria = query.value(3).toString();
        selectedSearchIn = query.value(4).toString();
        caseSensitive = query.value(5).toBool();
        selectedSearchExclude = query.value(6).toString();
        searchOnFileCriteria = query.value(7).toBool();
        searchOnSize = query.value(8).toBool();
        selectedMinimumSize = query.value(9).toLongLong();
        selectedMinSizeUnit = query.value(10).toString();
        selectedMaximumSize = query.value(11).toLongLong();
        selectedMaxSizeUnit = query.value(12).toString();
        searchOnType = query.value(13).toBool();
        selectedFileType = query.value(14).toString();
        searchOnDate = query.value(15).toBool();
        selectedDateMin = query.value(16).toDateTime();
        selectedDateMax = query.value(17).toDateTime();
        searchOnDuplicates = query.value(18).toBool();
        searchDuplicatesOnName = query.value(19).toBool();
        searchDuplicatesOnSize = query.value(20).toBool();
        searchDuplicatesOnDate = query.value(21).toBool();
        searchDuplicatesOnChecksum = query.value(22).toBool();
        searchDuplicatesChecksumEqual = query.value(23).toBool();
        duplicatesCompareDevices = query.value(24).toBool();
        duplicatesDeviceID1 = query.value(25).toInt();
        duplicatesDeviceID2 = query.value(26).toInt();
        searchOnDifferences = query.value(27).toBool();
        differencesOnName = query.value(28).toBool();
        differencesOnSize = query.value(29).toBool();
        differencesOnDate = query.value(30).toBool();
        differencesOnChecksum = query.value(31).toBool();
        differencesChecksumEqual = query.value(32).toBool();
        differencesDevices = query.value(33).toString().split("||");
        if (differencesDevices.length() > 1) {
            differencesDeviceID1 = differencesDevices[0].toInt();
            differencesDeviceID2 = differencesDevices[1].toInt();
        }
        searchOnFolderCriteria = query.value(34).toBool();
        showFoldersOnly = query.value(35).toBool();
        searchOnTags = query.value(36).toBool();
        selectedTagName = query.value(37).toString();
        selectedStorage = query.value(39).toString();
        selectedCatalog = query.value(40).toString();
        searchInCatalogsChecked = query.value(41).toBool();
        searchInConnectedChecked = query.value(42).toBool();
        connectedDirectory = query.value(43).toString();

        selectedDeviceIDList.clear();
        QString deviceListStr = query.value(44).toString(); // Adjust index
        if (!deviceListStr.isEmpty()) {
            const QStringList idStrings = deviceListStr.split(",", Qt::SkipEmptyParts);
            for (const QString& idStr : idStrings) {
                selectedDeviceIDList.append(idStr.toInt());
            }
        }
        searchOnFileMetadata = query.value(45).toBool();
        searchOnMetadataText = query.value(46).toBool();
        metadataTextSearch = query.value(47).toString();
        searchOnMetadataSize = query.value(48).toBool();
        metadataMinimumHeight = query.value(49).toInt();
        metadataMaximumHeight = query.value(50).toInt();
        metadataMinimumWidth = query.value(51).toInt();
        metadataMaximumWidth = query.value(52).toInt();
        searchOnMetadataDuration = query.value(53).toBool();
        metadataDurationMin = QDateTime(QDate(1970, 1, 1), QTime::fromString(query.value(54).toString(), "HH:mm:ss"));
        metadataDurationMax = QDateTime(QDate(2030, 1, 1), QTime::fromString(query.value(55).toString(), "HH:mm:ss"));

        // Calculate multipliers based on loaded units
        setMultipliers();
    }

    // Convert old translated values to internal constants
    selectedSearchIn = mapSearchInToInternal(selectedSearchIn);
    selectedTextCriteria = mapTextCriteriaToInternal(selectedTextCriteria);
    selectedMinSizeUnit = mapSizeUnitToInternal(selectedMinSizeUnit);
    selectedMaxSizeUnit = mapSizeUnitToInternal(selectedMaxSizeUnit);
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
    duplicatesCompareDevices = other->duplicatesCompareDevices;
    duplicatesDeviceID1 = other->duplicatesDeviceID1;
    duplicatesDeviceID2 = other->duplicatesDeviceID2;
    if (other->duplicatesDevice1) {
        if (!duplicatesDevice1) duplicatesDevice1 = new Device;
        duplicatesDevice1->ID = other->duplicatesDevice1->ID;
    }
    if (other->duplicatesDevice2) {
        if (!duplicatesDevice2) duplicatesDevice2 = new Device;
        duplicatesDevice2->ID = other->duplicatesDevice2->ID;
    }
    searchOnDifferences = other->searchOnDifferences;
    differencesOnName = other->differencesOnName;
    differencesOnSize = other->differencesOnSize;
    differencesOnDate = other->differencesOnDate;
    differencesDevices = other->differencesDevices;
    differencesDeviceID1 = other->differencesDeviceID1;
    differencesDeviceID2 = other->differencesDeviceID2;
    searchDuplicatesOnChecksum = other->searchDuplicatesOnChecksum;
    searchDuplicatesChecksumEqual = other->searchDuplicatesChecksumEqual;
    differencesOnChecksum = other->differencesOnChecksum;
    differencesChecksumEqual = other->differencesChecksumEqual;

    searchOnFolderCriteria = other->searchOnFolderCriteria;
    showFoldersOnly = other->showFoldersOnly;
    searchOnTags = other->searchOnTags;
    selectedTagName = other->selectedTagName;

    selectedStorage = other->selectedStorage;
    selectedCatalog = other->selectedCatalog;
    searchInCatalogsChecked = other->searchInCatalogsChecked;
    searchInConnectedChecked = other->searchInConnectedChecked;
    connectedDirectory = other->connectedDirectory;
    selectedDeviceIDList = other->selectedDeviceIDList;

    searchOnFileMetadata =  other->searchOnFileMetadata;
    searchOnMetadataText =  other->searchOnMetadataText;
    metadataTextSearch =  other->metadataTextSearch;

    // Note: no copy of results or statistics
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

    fileTypes.clear();
    mimeTypes.clear();
    imageWidths.clear();
    imageHeights.clear();
    videoDurations.clear();
    videoWidths.clear();
    videoHeights.clear();
    audioDurations.clear();
    audioArtists.clear();
    audioAlbums.clear();
    audioTitles.clear();

    filesFoundNumber = 0;
    filesFoundTotalSize = 0;
    filesFoundAverageSize = 0;
    filesFoundMinSize = 0;
    filesFoundMaxSize = 0;
    filesFoundMinDate = "";
    filesFoundMaxDate = "";
}

void Search::initializeProgressTracking(Device *selectedDevice)
{
    totalFilesProcessed = 0;
    estimatedTotalFiles = 0;

    if (searchInCatalogsChecked) {
        if (selectedDevice->type == "Catalog") {
            estimatedTotalFiles = selectedDevice->totalFileCount;
        } else {
            // Sum up total file counts for all catalogs in the hierarchy
            foreach(const Device::deviceListRow & row, selectedDevice->deviceListTable) {
                if (row.type == "Catalog") {
                    Device *device = new Device;
                    device->ID = row.ID;
                    device->loadDevice(m_connectionName);
                    estimatedTotalFiles += device->totalFileCount;
                    delete device;
                }
            }
        }
    }

    // Initialize progress with 0
    emit searchProgress(0);
}

void Search::updateProgress(int increment)
{
    totalFilesProcessed += increment;
    emit searchProgress(totalFilesProcessed);
}

QString Search::mapSearchInToInternal(const QString& dbValue)
{
    // Try exact match first (for new values)
    if (dbValue == SEARCH_IN_FILE_NAMES ||
        dbValue == SEARCH_IN_FOLDER_PATH ||
        dbValue == SEARCH_IN_FILES_AND_FOLDERS) {
        return dbValue;
    }

    // Map old translated values (add more languages as needed)
    if (dbValue == "File names only"
        || dbValue == "Noms de Fichers uniquement"
        || dbValue == "Nur Dateinamen"
        || dbValue == "Pouze názvy souborů") {
        return SEARCH_IN_FILE_NAMES;
    }
    // Map old translated values - Files and Folder paths
    if (dbValue == "File names or Folder paths"
        || dbValue == "Noms de Fichers ou Chemin des Dossiers"
        || dbValue == "Dateinamen oder Ordnerpfade"
        || dbValue == "Názvy souborů nebo cesty ke složkám") {
        return SEARCH_IN_FILES_AND_FOLDERS;
    }
    if (dbValue == "Folder path only"
        || dbValue == "Chemins des Dossiers uniquement"
        || dbValue == "Nur Ordnerpfad"
        || dbValue == "Pouze cesta ke složce") {
        return SEARCH_IN_FOLDER_PATH;
    }

    // Default fallback
    qDebug() << "Warning: Unknown SearchIn value, using default:" << dbValue;
    return SEARCH_IN_FILES_AND_FOLDERS;
}

int Search::mapSearchInToComboBoxIndex(const QString& internalValue)
{
    if (internalValue == SEARCH_IN_FILE_NAMES) {
        return 0;
    }
    if (internalValue == SEARCH_IN_FILES_AND_FOLDERS) {
        return 1;
    }
    if (internalValue == SEARCH_IN_FOLDER_PATH) {
        return 2;
    }
    return 1;
}

QString Search::mapTextCriteriaToInternal(const QString& dbValue)
{
    // Try exact match first (for new internal values)
    if (dbValue == TEXT_CRITERIA_EXACT_PHRASE ||
        dbValue == TEXT_CRITERIA_BEGINS_WITH ||
        dbValue == TEXT_CRITERIA_ANY_WORD ||
        dbValue == TEXT_CRITERIA_ALL_WORDS) {
        return dbValue;
    }

    // Map old translated values - All Words
    if (dbValue == "All Words"
        || dbValue == "Tous les Mots"
        || dbValue == "Alle Worte"
        || dbValue == "Všechna slova") {
        return TEXT_CRITERIA_ALL_WORDS;
    }

    // Map old translated values - Exact Phrase
    if (dbValue == "Exact Phrase"
        || dbValue == "Phrase Extacte"
        || dbValue == "Exakte Formulierung"
        || dbValue == "Přesná fráze") {
        return TEXT_CRITERIA_EXACT_PHRASE;
    }

    // Map old translated values - Begins With
    if (dbValue == "Begins With"
        || dbValue == "Commence Par"
        || dbValue == "Beginnt mit"
        || dbValue == "Začíná s") {
        return TEXT_CRITERIA_BEGINS_WITH;
    }

    // Map old translated values - Any Word
    if (dbValue == "Any Word"
        || dbValue == "Un des Mots"
        || dbValue == "Jedes Wort"
        || dbValue == "Jakékoli slovo") {
        return TEXT_CRITERIA_ANY_WORD;
    }

    // Default fallback
    qDebug() << "Warning: Unknown text criteria value, using default:" << dbValue;
    return TEXT_CRITERIA_ALL_WORDS;
}

int Search::mapTextCriteriaToComboBoxIndex(const QString& internalValue)
{
    if (internalValue == TEXT_CRITERIA_ALL_WORDS) {
        return 0;
    }
    if (internalValue == TEXT_CRITERIA_EXACT_PHRASE) {
        return 1;
    }
    if (internalValue == TEXT_CRITERIA_BEGINS_WITH) {
        return 2;
    }
    if (internalValue == TEXT_CRITERIA_ANY_WORD) {
        return 3;
    }
    return 1; // Default to "All Words"
}

QString Search::mapSizeUnitToInternal(const QString& dbValue)
{
    // Try exact match first (for new internal values)
    if (dbValue == SIZE_UNIT_BYTES ||
        dbValue == SIZE_UNIT_KIB ||
        dbValue == SIZE_UNIT_MIB ||
        dbValue == SIZE_UNIT_GIB ||
        dbValue == SIZE_UNIT_TIB) {
        return dbValue;
    }

    // Map old translated values - Bytes (if applicable)
    if (dbValue == "Bytes"
        || dbValue == "Octets"
        || dbValue == "Bytes"
        || dbValue == "Bajty") {
        return SIZE_UNIT_BYTES;
    }

    // Map old translated values - KiB
    if (dbValue == "KiB"
        || dbValue == "Kio"
        || dbValue == "KiB"
        || dbValue == "KiB") {
        return SIZE_UNIT_KIB;
    }

    // Map old translated values - MiB
    if (dbValue == "MiB"
        || dbValue == "Mio"
        || dbValue == "MiB"
        || dbValue == "MiB") {
        return SIZE_UNIT_MIB;
    }

    // Map old translated values - GiB
    if (dbValue == "GiB"
        || dbValue == "Gio"
        || dbValue == "GiB"
        || dbValue == "GiB") {
        return SIZE_UNIT_GIB;
    }

    // Map old translated values - TiB
    if (dbValue == "TiB"
        || dbValue == "Tio"
        || dbValue == "TiB"
        || dbValue == "TiB") {
        return SIZE_UNIT_TIB;
    }

    // Default fallback
    qDebug() << "Warning: Unknown size unit value, using default:" << dbValue;
    return SIZE_UNIT_BYTES;
}

int Search::mapSizeUnitToComboBoxIndex(const QString& internalValue)
{
    if (internalValue == SIZE_UNIT_TIB) return 0;
    if (internalValue == SIZE_UNIT_GIB) return 1;
    if (internalValue == SIZE_UNIT_MIB) return 2;
    if (internalValue == SIZE_UNIT_KIB) return 3;
    if (internalValue == SIZE_UNIT_BYTES) return 4;

    return 0; // Default to Bytes
}
