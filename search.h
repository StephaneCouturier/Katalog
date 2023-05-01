#ifndef SEARCH_H
#define SEARCH_H

#include "qdatetime.h"
#include <QAbstractTableModel>
#include <QCoreApplication>
#include <QSqlQuery>

class Search
{
public:

    //Search Criteria
    QString searchDateTime;

    bool searchOnFileName;
    QString searchText;
    QString selectedTextCriteria;
    QString selectedSearchIn;
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
    QStringList differencesCatalogs;
    QString differencesCatalog1;
    QString differencesCatalog2;

    bool searchOnFolderCriteria;
    bool showFoldersOnly;
    bool searchOnTags;
    QString selectedTagName;

    QString selectedLocation;
    QString selectedStorage;
    QString selectedCatalog;
    bool    searchInCatalogsChecked;
    bool    searchInConnectedChecked;
    QString connectedDirectory;

    //Methods
    void loadSearchHistoryCriteria();
    void setMultipliers();
};

#endif // SEARCH_H
