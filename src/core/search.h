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

#include "device.h"
#include "filetypemapping.h"

#include <QAbstractTableModel>
#include <QObject>
#include <QRegularExpression>
#include <QDateTime>
#include <QStringList>
#include <QMutex>
#include <QStandardItemModel>

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

    // Internal constants
    static const QString SEARCH_IN_FILE_NAMES;
    static const QString SEARCH_IN_FILES_AND_FOLDERS;
    static const QString SEARCH_IN_FOLDER_PATH;

    static const QString TEXT_CRITERIA_EXACT_PHRASE;
    static const QString TEXT_CRITERIA_BEGINS_WITH;
    static const QString TEXT_CRITERIA_ANY_WORD;
    static const QString TEXT_CRITERIA_ALL_WORDS;

    static const QString SIZE_UNIT_BYTES;
    static const QString SIZE_UNIT_KIB;
    static const QString SIZE_UNIT_MIB;
    static const QString SIZE_UNIT_GIB;
    static const QString SIZE_UNIT_TIB;

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
    FileTypeMapping::UserFileType selectedUserFileType = FileTypeMapping::ALL;
    bool searchOnDate;
    QDateTime selectedDateMin;
    QDateTime selectedDateMax;

    bool searchOnFileMetadata;
    bool searchOnMetadataText;
    QString metadataTextSearch;
    bool searchOnMetadataSize;
    int metadataMinimumHeight;
    int metadataMaximumHeight;
    int metadataMinimumWidth;
    int metadataMaximumWidth;
    bool searchOnMetadataDuration;
    QDateTime metadataDurationMin;
    QDateTime metadataDurationMax;

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
    QList<int> selectedDeviceIDList;  //stored in db as comma-separated device IDs ("1,2,3")
    QString searchDateTime;

    // Metadata
    QString buildMetadataSearchConditions() const;

    // Results
    QList<QString> fileNames;
    QList<qint64> fileSizes;
    QList<QString> fileDateTimes;
    QList<QString> filePaths;
    QList<QString> fileCatalogs;
    QList<int> fileCatalogIDs;

    QStringList fileTypes;
    QStringList mimeTypes;
    QList<int> imageWidths, imageHeights;
    QList<int> videoDurations, videoWidths, videoHeights;
    QList<int> audioDurations;
    QStringList audioArtists, audioAlbums, audioTitles;
    QStringList fileExtensions;
    QList<bool> mimeVerified;
    QList<bool> typeMismatch;
    QList<int> imageOrientations;
    QStringList videoCodecs;
    QList<double> videoFramerates;
    QList<int> videoBitrates;
    QStringList audioGenres;
    QList<int> audioYears;
    QList<int> audioTrackNumbers;
    QList<int> audioBitrates;
    QList<int> audioSampleRates;
    QStringList metadataExtendeds;
    QStringList metadataExtractionDates;

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
    int progressRefreshRate = 1000;

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
    void processResults();
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

    // Migration helper methods
    QString mapSearchInToInternal(const QString& dbValue);
    QString mapTextCriteriaToInternal(const QString& dbTextCriteriaValue);
    QString mapSizeUnitToInternal(const QString& dbValue);
    int mapSearchInToComboBoxIndex(const QString& internalValue);
    int mapTextCriteriaToComboBoxIndex(const QString& internalValue);
    int mapSizeUnitToComboBoxIndex(const QString& internalValue);

    /**
     * @brief Check if search was requested to stop
     * @return true if stop was requested, false otherwise
     */
    virtual bool wasStopRequested() const { return false; }

    void setFileTypeCategory(FileTypeMapping::UserFileType category) {
        selectedUserFileType = category;
    }

    FileTypeMapping::UserFileType getFileType2Category() const {
        return selectedUserFileType;
    }
private:
    QString m_connectionName = "defaultConnection";
    static QStringList getExtensionsForFileType(const QString &fileType);

signals:
    void searchProgress(int filesProcessed);
};

#endif // SEARCH_H
