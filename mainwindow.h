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
// File Name:   mainwindow.h
// Purpose:     Class for the main window
// Description:
// Author:      Stephane Couturier
/////////////////////////////////////////////////////////////////////////////
*/
#ifndef MAINWINDOW_H
#define MAINWINDOW_H

//#include QtWidget
#include <QMainWindow>
#include <QApplication>
#include <QAbstractItemView>
#include <QTreeView>
#include <QInputDialog>
#include <QMenu>
#include <QMessageBox>
#include <QFileDialog>
//#include QtCore
#include <QFile>
#include <QFileInfo>
#include <QStringList>
#include <QStringListModel>
#include <QTranslator>
#include <QDateTime>
#include <QAbstractTableModel>
#include <QSortFilterProxyModel>
#include <QStorageInfo>
#include <QStandardPaths>
#include <QTextStream>
#include <QSaveFile>
#include <QSettings>
//#include QtGui
#include <QFileSystemModel>
#include <QClipboard>
#include <QStandardItemModel>
#include <QDesktopServices>
#include <QCloseEvent>
//#include QtSql
#include <QtSql>
//#include QtMultimedia
#include <QMediaPlayer>
#include <QMediaMetaData>
//#include QtNetwork
#include <QNetworkAccessManager>
#include <QNetworkReply>
//#include QtCharts
#include <QDateTimeAxis>
#include <QtCharts/QBarSeries>
#include <QtCharts/QBarSet>
#include <QtCharts/QLineSeries>
#include <QtCharts/QBarCategoryAxis>
#include <QtCharts/QValueAxis>

#include "catalog.h"
#include "storage.h"
#include "devicetreeview.h"

QT_BEGIN_NAMESPACE
namespace Ui { class MainWindow; }
QT_END_NAMESPACE

class MainWindow : public QMainWindow
{
    Q_OBJECT

    public:
        explicit MainWindow(QWidget *parent = nullptr);
        ~MainWindow();

    private:
        //Global
            //Application version
            QString currentVersion;
            QString releaseDate;
            bool checkVersionChoice;
            void checkVersion();
            bool firstRun;
            bool developmentMode;

            //UI
            Ui::MainWindow *ui;
            void loadCustomThemeLight();
            void loadCustomThemeDark();
            void hideDevelopmentUIItems();
            bool unsavedChanges;
            void closeEvent (QCloseEvent *event);

            //KDE menus/icons
            void setupActions();

            //Application settings
            QString settingsFilePath;
            void loadSettings();
            void saveSettings();
            void preloadCatalogs();
            void generateCollectionFilesPaths();

            //Parameters
            QString fileName;
            int selectedTab;

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

            //Database
            QSqlRelationalTableModel *storageModel;
            QSqlError initializeDatabase();
            QString databaseMode;
            void    startDatabase();
            QString databaseFilePath;
            void    selectDatabaseFilePath();
            QString databaseHostName;
            QString databaseName;
            int     databasePort;
            QString databaseUserName;
            QString databasePassword;

            //Objects
            Catalog *newCatalog      = new Catalog(); //temporary catalog used to create a new catalog entry
            Catalog *selectedCatalog = new Catalog(); //selected catalog used for individual catalog operation from various screens (explore, update, edit, delete)
            Catalog *tempCatalog     = new Catalog(); //temporary catalog used for operations in Search screen or temporary operations (list of catalogs)
            DeviceTreeView *deviceTreeProxyModel = new DeviceTreeView(); //tree of devices for selection and filtering
            Storage *selectedStorage = new Storage(); //selected storage used for individual storage operation in Storage screen (update)
            Storage *tempStorage = new Storage(); //temporary storage used for individual storage operation in Storage screen (update)

        //Filters
            bool searchInFileCatalogsChecked;
            bool searchInConnectedDriveChecked;           
            int  deviceTreeExpandState;

            QString selectedDeviceName;
            QString selectedDeviceType;
            int     selectedDeviceID;

            QString selectedFilterStorageLocation;
            QString selectedFilterStorageName;
            QString selectedFilterCatalogName;
            QString selectedConnectedDrivePath;

            void setTreeExpandState(bool toggle);
            void filterFromSelectedDevices();
            void resetSelection();

        //TAB: Search
            QString searchText;
            QString regexPattern;
            QString selectedFileType;
            QString selectedTextCriteria;
            QString selectedSearchIn;
            QString selectedSearchExclude;
            bool    searchOnSize;
            qint64  selectedMinimumSize;
            qint64  selectedMaximumSize;
            qint64  sizeMultiplierMin;
            qint64  sizeMultiplierMax;
            QString selectedMinSizeUnit;
            QString selectedMaxSizeUnit;
            QString selectedTagName;
            bool searchOnDuplicates;
            bool hasDuplicatesOnName;
            bool hasDuplicatesOnSize;
            bool hasDuplicatesOnDateModified;
            bool searchDuplicatesOnDate;
            bool searchDuplicatesOnTags;
            bool searchDuplicatesOnText;
            bool searchOnDifferences;
            bool hasDifferencesOnName;
            bool hasDifferencesOnSize;
            bool hasDifferencesOnDateModified;
            QString selectedDifferencesCatalog1;
            QString selectedDifferencesCatalog2;
            bool searchOnDate;
            bool searchOnTags;
            bool searchOnFileName;
            bool searchOnFileCriteria;
            bool searchOnFolderCriteria;
            QDateTime selectedDateMin;
            QDateTime selectedDateMax;
            bool showFoldersOnly;
            QString selectedTag;
            bool caseSensitive;

            QString sourceCatalog;
            QStringList catalogSelectedList;
            void initiateSearchValues();
            void refreshLocationSelectionList();
            void refreshStorageSelectionList(QString selectedLocation);
            void refreshCatalogSelectionList(QString selectedLocation, QString selectedStorage);
            QString exportSearchResults();
            QString getCatalogStorageName(QString catalogFilePath);
            void insertSearchHistoryToTable();
            void loadSearchHistoryTableToModel();
            void saveSearchHistoryTableToFile();
            void loadSearchHistoryFileToTable();
            QString searchHistoryFilePath;
            int lastSearchSortSection;
            int lastSearchSortOrder;
            void refreshDifferencesCatalogSelection();

            //search
            QList<QString>  sFileNames;
            QList<qint64>   sFileSizes;
            QList<QString>  sFilePaths;
            QList<QString>  sFileDateTimes;
            QList<QString>  sFileCatalogs;
            QString regexSearchtext;
            QString regexFileType;
            void searchFiles();
            void searchFilesInCatalog(const QString &sourceCatalog);
            void searchFilesInDirectory(const QString &sourceDirectory);
            qint64 filesFoundNumber;
            qint64 filesFoundTotalSize;
            qint64 filesFoundAverageSize;
            qint64 filesFoundMinSize;
            qint64 filesFoundMaxSize;
            QString filesFoundMinDate;
            QString filesFoundMaxDate;

            //outputs
            QStringList filesFoundList;
            QStringList catalogFoundList;
            QStringListModel *catalogFoundListModel;
            QStringList searchTextList;

            //search history
            int lastSearchHistorySortSection;
            int lastSearchHistorySortOrder;

        //TAB: Catalogs
            QString collectionFolder;
            QStringListModel catalogListModel;
            QStringList catalogFileList;

            bool skipCatalogUpdateSummary;
            int lastCatalogsSortSection;
            int lastCatalogsSortOrder;

            void loadCollection();
            void loadCatalogFilesToTable();
            void loadCatalogsTableToModel();
            int verifyCatalogPath(QString catalogSourcePath);
            void recordSelectedCatalogStats(QString selectedCatalogName,
                                            int selectedCatalogFileCount,
                                            qint64 selectedCatalogTotalFileSize);
            void recordCollectionStats();
            void recordAllCatalogStats();
            void convertCatalog(QString catalogSourcePath);
            void backupCatalog(QString catalogSourcePath);
            void hideCatalogButtons();
            void updateSingleCatalog(Catalog *catalog);
            void updateCatalogFileList(Catalog *catalog);
            QString requestSource;
            void saveCatalogChanges(Catalog *catalog);
            void importFromVVV();

        //TAB: Explore
            QString selectedDirectoryName;
            QString selectedDirectoryFullPath;

            void openCatalogToExplore();
            void loadSelectedDirectoryFilesToExplore();
            void loadCatalogDirectoriesToExplore();

            int lastExploreSortSection;
            int lastExploreSortOrder;
            bool optionDisplayFolders;
            bool optionDisplaySubFolders;

        //TAB: Create
            QFileSystemModel *fileSystemModel;
            QStringListModel *fileListModel;

            QStringList storageNameList;
            QString excludeFilePath;

            QString fileMetadataString;
            //QMediaMetaData mediaMetadata;

            QMediaPlayer *m_player;

            void loadFileSystem(QString newCatalogPath);
            void createCatalog();
            void catalogDirectory(Catalog *catalog);
            void loadStorageList();
            void saveCatalogToNewFile(QString newCatalogName);
            void saveFoldersToNewFile(QString newCatalogName);

        //TAB: Storage
            QString storageFilePath;
            int     selectedStorageIndexRow;
            QStringListModel *storageListModel;
            QStringList locationCatalogList;
            int lastStorageSortSection;
            int lastStorageSortOrder;

            void createStorageList();
            void addStorageDevice(QString deviceName);
            void loadStorageFileToTable();
            void loadStorageTableToModel();
            void loadStorageTableToSelectionTreeModel();
            void saveStorageModelToFile();
            void saveStorageData();
            void updateStorageInfo(Storage *storage);
            void updateStorageSelectionStatistics();

        //TAB: Statistics
            QString statisticsFileName;
            QString statisticsFilePath;
            QStringList typeOfData;
            QString selectedTypeOfData;
            QStringListModel *listModel;
            QString graphicStartDate;
            void loadStatisticsDataTypes();
            void loadStatisticsData();
            void loadStatisticsChart();

        //TAB: Tags
            void reloadTagsData();
            void loadTagsToTable();
            void loadTagsTableToExistingTagsModel();
            void loadTagsTableToTagsAndFolderListModel();
            void loadFileSystemTags(QString newTagFolderPath);
            QStringListModel *tagListModel;
            QString selectedTagListName;
            QString tagsFilePath;
            QString newTagFolderPath;

   private slots:

        //Filters
            void on_Filters_pushButton_Filters_Hide_clicked();
            void on_Filters_pushButton_Filters_Show_clicked();
            void on_Filters_pushButton_ResetGlobal_clicked();
            void on_Filters_pushButton_ReloadCollection_clicked();
            void on_Filters_checkBox_SearchInCatalogs_toggled(bool checked);
            void on_Filters_checkBox_SearchInConnectedDrives_toggled(bool checked);

            void on_Filter_pushButton_Search_clicked();
            void on_Filter_pushButton_Explore_clicked();
            void on_Filter_pushButton_Update_clicked();
            void on_Filters_pushButton_TreeExpandCollapse_clicked();
            void on_Filters_treeView_Devices_clicked(const QModelIndex &index);

            void on_Filters_treeView_Directory_clicked(const QModelIndex &index);
            void on_Filter_pushButton_PickPath_clicked();

        //Settings
            void on_tabWidget_currentChanged(int index);
            void on_splitter_splitterMoved();

            void on_Settings_comboBox_DatabaseMode_currentTextChanged();

            void on_Settings_pushButton_SelectFolder_clicked();
            void on_Settings_pushButton_OpenFolder_clicked();
            void on_Settings_lineEdit_CollectionFolder_returnPressed();
            void on_Settings_checkBox_KeepOneBackUp_stateChanged();
            void on_Settings_checkBox_PreloadCatalogs_stateChanged(int arg1);

            void on_Settings_pushButton_SelectDatabaseFilePath_clicked();
            void on_Settings_pushButton_EditDatabaseFile_clicked();

            void on_Settings_pushButton_SaveHostedParameters_clicked();

            void on_Settings_comboBox_Language_currentTextChanged(const QString &selectedLanguage);
            void on_Settings_comboBox_Theme_currentIndexChanged(int index);
            void on_Settings_checkBox_SaveRecordWhenUpdate_stateChanged();
            void on_Settings_pushButton_OpenSettingsFile_clicked();

            void on_Settings_pushButton_Wiki_clicked();
            void on_Settings_pushButton_ReleaseNotes_clicked();
            void on_Settings_checkBox_CheckVersion_stateChanged();

        //Search
            void on_Search_pushButton_Search_clicked();
            void on_Search_pushButton_CleanSearchText_clicked();
            void on_Search_pushButton_ResetAll_clicked();
            void on_Search_pushButton_ExportResults_clicked();
            void on_Search_pushButton_PasteFromClipboard_clicked();
            void on_Search_lineEdit_SearchText_returnPressed();

            void on_Search_listView_CatalogsFound_clicked(const QModelIndex &index);
            void on_Search_treeView_FilesFound_clicked(const QModelIndex &index);
            void on_Search_treeView_FilesFound_customContextMenuRequested(const QPoint &pos);
            void on_Search_pushButton_ShowHideSearchCriteria_clicked();
            void on_Search_pushButton_ShowHideCatalogResults_clicked();
            void on_Search_pushButton_ShowHideSearchHistory_clicked();

            void on_Search_checkBox_FileName_toggled(bool checked);

            void on_Search_checkBox_FileCriteria_toggled(bool checked);
            void on_Search_checkBox_Date_toggled(bool checked);
            void on_Search_checkBox_Size_toggled(bool checked);
            void on_Search_checkBox_Tags_toggled(bool checked);
            void on_Search_checkBox_Duplicates_toggled(bool checked);
            void on_Search_checkBox_Differences_toggled(bool checked);

            void on_Search_checkBox_ShowFolders_toggled(bool checked);
            void on_Search_checkBox_FolderCriteria_toggled(bool checked);

            void on_Search_treeView_History_activated(const QModelIndex &index);
            void on_Search_pushButton_FileFoundMoreStatistics_clicked();
            void on_SearchTreeViewFilesFoundHeaderSortOrderChanged();
            void on_SearchTreeViewHistoryHeaderSortOrderChanged();
            void on_Search_splitter_Results_splitterMoved();

            void setupFileContextMenu();
            void getLocationCatalogList(QString location);

            //context menu
            void searchContextOpenFile();
            void searchContextOpenFolder();
            void searchContextOpenExplore();
            void searchContextCopyAbsolutePath();
            void searchContextCopyFolderPath();
            void searchContextCopyFileNameWithExtension();
            void searchContextCopyFileNameWithoutExtension();
            void searchContextMoveFileToFolder();
            void searchContextMoveFileToTrash();
            void searchContextDeleteFile();

        //Catalogs
            void on_Catalogs_pushButton_Search_clicked();
            void on_Catalogs_pushButton_ExploreCatalog_clicked();
            void on_Catalogs_pushButton_Cancel_clicked();
            void on_Catalogs_pushButton_UpdateCatalog_clicked();
            void on_Catalogs_pushButton_UpdateAllActive_clicked();
            void on_Catalogs_pushButton_EditCatalogFile_clicked();
            void on_Catalogs_pushButton_ViewCatalogStats_clicked();
            void on_Catalogs_pushButton_Import_clicked();
            void on_Catalogs_pushButton_DeleteCatalog_clicked();

            void on_Catalogs_treeView_CatalogList_clicked(const QModelIndex &index);
            void on_Catalogs_treeView_CatalogList_doubleClicked();
            void on_CatalogsTreeViewCatalogListHeaderSortOrderChanged();

            void on_Catalogs_pushButton_Save_clicked();
            void on_Catalogs_pushButton_Open_clicked();
            void on_Catalogs_pushButton_SelectPath_clicked();
            void on_Catalogs_pushButton_Snapshot_clicked();

        //Create
            void setMediaFile(QString filePath);
            void onMediaStatusChanged(QMediaPlayer::MediaStatus status);
            void getMetaData(QMediaPlayer *player);

            void on_Create_pushButton_PickPath_clicked();
            void on_Create_treeView_Explorer_clicked(const QModelIndex &index);
            void on_Create_pushButton_EditExcludeList_clicked();
            void on_Create_pushButton_AddStorage_clicked();
            void on_Create_pushButton_GenerateFromPath_clicked();
            void on_Create_pushButton_CreateCatalog_clicked();

        //Explore
            void on_Explore_splitter_splitterMoved();
            void on_Explore_treeview_Directories_clicked(const QModelIndex &index);
                        void on_Explore_treeView_FileList_clicked(const QModelIndex &index);
            void on_ExplorePushButtonLoadClicked();
            void on_Explore_checkBox_DisplayFolders_toggled(bool checked);
            void on_Explore_checkBox_DisplaySubFolders_toggled(bool checked);
            void on_Explore_pushButton_OrderFoldersFirst_clicked();
            void on_ExploreTreeViewFileListHeaderSortOrderChanged();

            //context menu directories
            void on_Explore_treeview_Directories_customContextMenuRequested(const QPoint &pos);
            void exploreContextTagFolder();

            //context menu files
            void on_Explore_treeView_FileList_customContextMenuRequested(const QPoint &pos);
            void exploreContextOpenFile();
            void exploreContextOpenFolder();
            void exploreContextCopyAbsolutePath();
            void exploreContextCopyFolderPath();
            void exploreContextCopyFileNameWithExtension();
            void exploreContextCopyFileNameWithoutExtension();
            void exploreContextMoveFileToFolder();
            void exploreContextMoveFileToTrash();
            void exploreContextDeleteFile();

        //Storage
            void on_Storage_pushButton_CreateList_clicked();
            void on_Storage_pushButton_Reload_clicked();
            void on_Storage_pushButton_EditAll_clicked();
            void on_Storage_pushButton_SaveAll_clicked();
            void on_Storage_pushButton_New_clicked();
            void on_Storage_pushButton_OpenFilelight_clicked();
            void on_Storage_pushButton_Update_clicked();
            void on_Storage_pushButton_Delete_clicked();
            void on_Storage_pushButton_SearchStorage_clicked();
            void on_Storage_pushButton_SearchLocation_clicked();
            void on_Storage_pushButton_CreateCatalog_clicked();
            void on_Storage_treeView_StorageList_clicked(const QModelIndex &index);
            void on_Storage_treeView_StorageList_doubleClicked(const QModelIndex &index);
            void on_StorageTreeViewStorageListHeaderSortOrderChanged();

        //Statistics
            void on_Statistics_pushButton_EditStatisticsFile_clicked();
            void on_Statistics_pushButton_Reload_clicked();
            void on_Statistics_comboBox_SelectSource_currentTextChanged();
            void on_StatisticsComboBoxSelectCatalogCurrentIndexChanged(const QString &selectedCatalog);
            void on_Statistics_comboBox_TypeOfData_currentTextChanged();
            void on_Statistics_lineEdit_GraphicStartDate_returnPressed();
            void on_Statistics_pushButton_ClearDate_clicked();
            void on_Statistics_pushButton_PickDate_clicked();
            void on_Statistics_calendarWidget_clicked(const QDate &date);

        //Tags
            void on_Tags_pushButton_PickFolder_clicked();
            void on_Tags_pushButton_TagFolder_clicked();
            void on_Tags_pushButton_Reload_clicked();
            void on_Tags_pushButton_OpenTagsFile_clicked();
            void on_Tags_listView_ExistingTags_clicked(const QModelIndex &index);
            void on_Tags_treeview_Explorer_clicked(const QModelIndex &index);

        //DEV
            void on_Storage_pushButton_TestMedia_clicked();

};

#endif // MAINWINDOW_H
