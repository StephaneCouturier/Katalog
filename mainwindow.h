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

//QtWidget
#include <QMainWindow>
#include <QApplication>
#include <QAbstractItemView>
#include <QTreeView>
#include <QInputDialog>
#include <QMenu>
#include <QMessageBox>
#include <QFileDialog>
//QtCore
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
//QtGui
#include <QFileSystemModel>
#include <QClipboard>
#include <QStandardItemModel>
#include <QDesktopServices>
#include <QCloseEvent>
#include <QPixmap>
//QtSql
#include <QtSql>
//QtMultimedia
#include <QMediaPlayer>
#include <QMediaMetaData>
//QtNetwork
#include <QNetworkAccessManager>
#include <QNetworkReply>
//QtCharts
#include <QDateTimeAxis>
#include <QtCharts/QBarSeries>
#include <QtCharts/QBarSet>
#include <QtCharts/QLineSeries>
#include <QtCharts/QBarCategoryAxis>
#include <QtCharts/QValueAxis>

//Katalog object classes
#include "catalog.h"
#include "search.h"
#include "storage.h"
#include "device.h"

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

            //Application settings
            QString settingsFilePath;
            void loadSettings();
            void preloadCatalogs();
            void generateCollectionFilesPaths();

            //Parameters
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
            QSqlError initializeDatabase();
            QString databaseMode;
            void    startDatabase();
            QString databaseFilePath;
            void    selectDatabaseFilePath();
            void    selectNewDatabaseFolderPath();
            QString databaseHostName;
            QString databaseName;
            int     databasePort;
            QString databaseUserName;
            QString databasePassword;
            void    clearDatabaseData();

            //Objects
            Device *selectedDevice   = new Device(); //selected    device used for individual device operations
            Device *tempDevice       = new Device(); //temporary   device used for operations on a list of devices
            Device *catalogDevice    = new Device(); //selected catalog/device from Catalog screen
            QStandardItemModel *deviceTreeModel = new QStandardItemModel();

        //Filters panel
            int  deviceTreeExpandState;
            QString selectedConnectedDrivePath;

            void setTreeExpandState(bool toggle);
            void filterFromSelectedDevices();
            void resetSelection();
            void displaySelectedDeviceName();

        //TAB: Search
            Search *newSearch  = new Search(); //temporary search object used to handle a new search and its results
            Search *loadSearch = new Search(); //temporary search object used to load criteria from a previous search.
            Search *lastSearch = new Search(); //temporary search object used to load criteria from the last search.

            void resetToDefaultSearchCriteria();
            void initiateSearchFields();
            void loadSearchCriteria(Search *search);
            void getSearchCriteria();

            QString exportSearchResults();

            void batchProcessSearchResults();
            void insertSearchHistoryToTable();
            void loadSearchHistoryTableToModel();
            void saveSearchHistoryTableToFile();
            void loadSearchHistoryFileToTable();

            QString searchHistoryFilePath;
            int lastSearchSortSection;
            int lastSearchSortOrder;

            void refreshDifferencesCatalogSelection();
            void searchFiles();
            void searchFilesInCatalog(const QString &sourceCatalog);
            void searchFilesInDirectory(const QString &sourceDirectory);

            //Search history
            int lastSearchHistorySortSection;
            int lastSearchHistorySortOrder;

            //Result operations
            QString moveFileToTrash(QString fileFullPath);
            QString deleteFile(QString fileFullPath);

        //TAB: Catalogs
            QString collectionFolder;
            QStringListModel catalogListModel;
            QStringList catalogFileList;
            QStringList catalogSelectedList;

            bool skipCatalogUpdateSummary;
            int lastCatalogsSortSection;
            int lastCatalogsSortOrder;

            void loadCollection();
            void loadCatalogFilesToTable();
            void loadCatalogsTableToModel();
            void updateCatalogsScreenStatistics();
            void recordCollectionStats();
            void recordAllCatalogStats(QDateTime dateTime);
            void convertCatalog(QString catalogSourcePath);
            void backupCatalogFile(QString catalogSourcePath);
            void hideCatalogButtons();
            void updateSingleCatalog(Device *device, bool updateStorage);
            void updateCatalogFileList(Device *device);
            QString requestSource;
            void saveCatalogChanges(Catalog *catalog);
            void importFromVVV();
            void generateCatalogMissingIDs();

            qint64 globalUpdateTotalFiles;
            qint64 globalUpdateDeltaFiles;
            qint64 globalUpdateTotalSize;
            qint64 globalUpdateDeltaSize;

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
            void catalogDirectory(Device *device);
            void loadStorageList();
            void saveCatalogToNewFile(QString newCatalogName);
            void saveFoldersToNewFile(QString newCatalogName);

        //TAB: Storage
            QString storageFilePath;
            QString deviceFilePath;
            QString deviceCatalogFilePath;
            int     selectedStorageIndexRow;
            QStringListModel *storageListModel;
            int lastStorageSortSection;
            int lastStorageSortOrder;

            void createStorageFile();
            void addStorageDevice(QString deviceName);
            void loadStorageFileToTable();
            void loadStorageTableToModel();
            void saveStorageTableToFile();
            void updateStorageInfo(Storage *storage);
            void updateStorageSelectionStatistics();
            void recordAllStorageStats(QDateTime dateTime);

            void displayStoragePicture();
            void loadStorageToPanel();
            void saveStorageFromPanel();

        //TAB: Devices
            void setDeviceTreeExpandState(bool toggle);
            int  optionDeviceTreeExpandState;
            bool optionDisplayCatalogs;
            bool optionDisplayStorage;
            bool optionDisplayPhysicalGroupOnly;
            bool optionDisplayAllExceptPhysicalGroup;
            bool optionDisplayFullTable;

            void loadDeviceFileToTable();
            void insertPhysicalStorageGroup();
            void addDeviceVirtual();
            void addDeviceStorage();
            void editDevice();
            void saveDevice();
            void assignCatalogToDevice(QString catalogName,int deviceID);
            void assignStorageToDevice(int storageID,int deviceID);
            void unassignPhysicalFromDevice(int deviceID, int deviceParentID);
            void deleteDeviceItem();
            void saveDeviceTableToFile(QString filePath);
            void loadDeviceTableToTreeModel();
            void updateNumbers();
            void updateAllNumbers();
            void convertDeviceCatalogFile();
            void importStorageCatalogLinks();
            void shiftIDsInDeviceTable(int shiftAmount);
            void loadParentsList();
            void recordAllDeviceStats(QDateTime dateTime);
            QList<int> verifyStorageWithOutDevice();
            void updateAllDeviceActive();

        //TAB: Statistics
            QString statisticsCatalogFileName;
            QString statisticsCatalogFilePath;
            QString statisticsStorageFileName;
            QString statisticsStorageFilePath;
            QString statisticsDeviceFileName;
            QString statisticsDeviceFilePath;
            QStringList typeOfData;
            QString selectedTypeOfData;
            QStringListModel *listModel;
            QDateTime graphicStartDate;
            void loadStatisticsDataTypes();
            void loadStatisticsCatalogFileToTable();
            void loadStatisticsStorageFileToTable();
            void loadStatisticsDeviceFileToTable();
            void loadStatisticsChart();
            void convertStatistics();
            void saveStatiticsToFile();

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
            void on_Filters_treeView_Devices_customContextMenuRequested(const QPoint &pos);

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
            void on_Settings_pushButton_NewDatabaseFile_clicked();

            void on_Settings_pushButton_SaveHostedParameters_clicked();

            void on_Settings_comboBox_Language_currentTextChanged(const QString &selectedLanguage);
            void on_Settings_comboBox_Theme_currentIndexChanged(int index);
            void on_Settings_checkBox_BiggerIconSize_stateChanged(int arg1);
            void on_Settings_checkBox_SaveRecordWhenUpdate_stateChanged();
            void on_Settings_checkBox_LoadLastCatalog_stateChanged(int arg1);
            void on_Settings_pushButton_OpenSettingsFile_clicked();

            void on_Settings_pushButton_Wiki_clicked();
            void on_Settings_pushButton_ReleaseNotes_clicked();
            void on_Settings_checkBox_CheckVersion_stateChanged();

        //Search
            void on_Search_pushButton_Search_clicked();
            void on_Search_pushButton_CleanSearchText_clicked();
            void on_Search_pushButton_ResetAll_clicked();
            void on_Search_pushButton_ProcessResults_clicked();
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
            void on_Search_checkBox_Type_toggled(bool checked);
            void on_Search_checkBox_Duplicates_toggled(bool checked);
            void on_Search_checkBox_Differences_toggled(bool checked);

            void on_Search_checkBox_ShowFolders_toggled(bool checked);
            void on_Search_checkBox_FolderCriteria_toggled(bool checked);

            void on_Search_treeView_History_activated(const QModelIndex &index);
            void on_Search_pushButton_FileFoundMoreStatistics_clicked();
            void on_SearchTreeViewFilesFoundHeaderSortOrderChanged();
            void on_SearchTreeViewHistoryHeaderSortOrderChanged();
            void on_Search_splitter_Results_splitterMoved();

            void setupFileContextMenus();

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
            void on_Storage_pushButton_CreateCatalog_clicked();
            void on_Storage_treeView_StorageList_clicked(const QModelIndex &index);
            void on_Storage_treeView_StorageList_doubleClicked();
            void on_StorageTreeViewStorageListHeaderSortOrderChanged();
            void on_Storage_pushButton_PanelSave_clicked();
            void on_Storage_pushButton_Edit_clicked();
            void on_Storage_pushButton_PanelCancel_clicked();

        //Devices
            void on_Devices_pushButton_InsertRootLevel_clicked();
            void on_Devices_pushButton_AddVirtual_clicked();
            void on_Devices_pushButton_AddStorage_clicked();
            void on_Devices_pushButton_DeleteItem_clicked();
            void on_Devices_pushButton_Edit_clicked();
            void on_Devices_pushButton_Save_clicked();
            void on_Devices_pushButton_Cancel_clicked();
            void on_Devices_pushButton_AssignCatalog_clicked();
            void on_Devices_pushButton_AssignStorage_clicked();
            void on_Devices_checkBox_DisplayCatalogs_stateChanged(int arg1);
            void on_Devices_checkBox_DisplayStorage_stateChanged(int arg1);
            void on_Devices_checkBox_DisplayPhysicalGroupOnly_stateChanged(int arg1);
            void on_Devices_checkBox_DisplayAllExceptPhysicalGroup_stateChanged(int arg1);
            void on_Devices_checkBox_DisplayFullTable_stateChanged(int arg1);
            void on_Devices_treeView_DeviceList_clicked(const QModelIndex &index);
            void on_Devices_treeView_DeviceList_customContextMenuRequested(const QPoint &pos);
            void on_Devices_pushButton_ImportS_clicked();
            void on_Devices_pushButton_TreeExpandCollapse_clicked();
            void on_Devices_pushButton_EditList_clicked();
            void on_Devices_pushButton_verifStorage_clicked();

        //Statistics
            void on_Statistics_pushButton_EditCatalogStatisticsFile_clicked();
            void on_Statistics_pushButton_EditStorageStatisticsFile_clicked();
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
            void on_TEST_pushButton_TestMedia_clicked();
};

#endif // MAINWINDOW_H
