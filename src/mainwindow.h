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
// File Name:   mainwindow.h
// Purpose:     Class for the main window
// Description:
// Author:      Stephane Couturier
/////////////////////////////////////////////////////////////////////////////
*/
#ifndef MAINWINDOW_H
#define MAINWINDOW_H

//Katalog object classes
#include "core/collection.h"
#include "core/devicemanager.h"
#include "core/search.h"
#include "core/device.h"
#include "core/searchmanager.h"
#include "core/searchjobstoppable.h"
#include "core/searchmanager.h"
#include "core/searchprogressmanager.h"
#include "core/searchresultsthrottler.h"
#include "core/catalogmanager.h"
#include "core/catalogjobstoppable.h"
#include "core/catalogprogressmanager.h"

//KDE KF6
#include <KFormat>
#include <KXmlGuiWindow>
#include <KSharedConfig>
#include <KConfigGroup>
#ifdef Q_OS_LINUX
#include <KIconTheme>
#include <KIconLoader>
#endif

//QtWidget
#include <QMainWindow>
#include <QApplication>
#include <QAbstractItemView>
#include <QTreeView>
#include <QInputDialog>
#include <QMenu>
#include <QMessageBox>
#include <QFileDialog>
#include <QButtonGroup>
#include <QProgressDialog>
#include <QStatusBar>
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
#include <QScatterSeries>
#include <QtCharts/QLegendMarker>

#pragma once

QT_BEGIN_NAMESPACE
namespace Ui { class MainWindow; }
QT_END_NAMESPACE

class SearchProcess;

class MainWindow : public KXmlGuiWindow
{
    Q_OBJECT

    public:
        explicit MainWindow(QWidget *parent = nullptr);
        ~MainWindow();

        //Objects
        Collection *collection = nullptr;   // Collection object, used to access the collection of devices, catalogs and storage
        Device *selectedDevice = nullptr;  // Selected device from Selection panel, used for operations on any screen
        Device* currentUpdateDevice = nullptr; // Device selected for update operations
        DeviceManager* deviceManager = nullptr;
        Search *currentSearch = nullptr;
        SearchJobStoppable *loadSearch = new SearchJobStoppable(this); //temporary search object used to load criteria from a previous search.
        SearchJobStoppable *lastSearch = new SearchJobStoppable(this); //temporary search object used to load criteria from the last search.
        SearchManager *searchManager = nullptr;
        SearchProgressManager *searchProgressManager = nullptr;
        SearchResultsThrottler *searchResultsThrottler = nullptr;
        SearchProcess *searchProcess  = nullptr;
        CatalogManager *catalogManager;
        CatalogJobStoppable *catalogJobStoppable = nullptr;
        CatalogProgressManager *catalogProgressManager = nullptr;

        void extracted();
        void launchSearch();
        void sendSearchParametersFromUI(Search *search);

        //FileTypes
        QStringList fileType_Image;
        QStringList fileType_Audio;
        QStringList fileType_Video;
        QStringList fileType_Text;
        QStringList fileType_ImageS;
        QStringList fileType_AudioS;
        QStringList fileType_VideoS;
        QStringList fileType_TextS;

        QSqlError initializeDatabase(const QString &connectionName);
        bool backupMemoryDatabaseToFile(const QString &memoryConnectionName, const QString &filePath);

        //Settings
        bool fileSortCaseSensitive;
        void listShareContents(const QString &serverIP, const QString &shareName, const QString &credString);

        //Command line
        void cmd_updateCatalog(int deviceId, bool displayReport);
        void cmd_listGroup0Catalogs();
        void cmd_updateAllActive(bool displayReport);

    private:
        void debugIconLoadingDetailed();
        void testIconSourceTracking();
        void setupIconThemeWithKF6Test();
        void testWithoutFallbackResources();
#ifdef Q_OS_LINUX
        void setupLinuxIconThemeKF6Test();
#endif
#ifdef Q_OS_WIN
        void setupWindowsIconThemeKF6Test();
#endif


        //Global
            //Application version
            QString currentVersion;
            QString releaseDate;
            bool checkVersionChoice;
            void checkVersion();
            bool firstRun;


            //UI
            Ui::MainWindow *ui;
            void loadCustomThemeLight();
            void loadCustomThemeDark();
            void hideDevelopmentUIItems();
            bool unsavedChanges;
            void closeEvent (QCloseEvent *event);
            QTimer* statusBarTimer;

            // Theme management
            int themeID;
            QString themeColor = "dev"; //blue
            bool isDarkTheme() const;
            bool checkGnomeTheme() const;
            bool checkXfceTheme() const;
            bool checkCinnamonTheme() const;
            void setupIconTheme();
            void refreshAllIcons();
            void debugIconSetup();
#ifdef Q_OS_LINUX
            void setupLinuxIconTheme();
#endif

#ifdef Q_OS_WIN
    void setupWindowsIconTheme();
#endif

            //Application settings
            void loadSettings();
            void preloadCatalogs();

            //Parameters
            int selectedTab;

            //FileTypes
            QStringList fileType_current;
            void setFileTypes();

            //Database
            void selectDatabaseFilePath();
            void selectNewDatabaseFolderPath();
            void applyDatabaseModeToUI();
            void migrateExistingSearchDeviceData_2_6();

            //Objects
            Device *activeDevice   = new Device(); //active device from any screen, used for operations from that screen
            Device *catalogDevice  = new Device(); //selected catalog/device from Catalog screen
            Device *exploreDevice  = new Device(); //tempory catalog/device to be use in Exploore screen
            Device *currentCatalogDevice = nullptr;

            //Collection
            void translateDefaultDevices();

        //Filters panel
            int  filtersTreeExpandState;
            QString selectedConnectedDrivePath;

            void setTreeExpandState(bool toggle);
            void filterFromSelectedDevice();
            void resetSelection();
            void displaySelectedDeviceName();

        //TAB: Search
            enum class SearchButtonState {
                Idle,
                Running,
                Paused,
                Searching
            };
            SearchButtonState searchButtonState = SearchButtonState::Idle;

            void setSearchButtonState(SearchButtonState state);
            void setSearchStateIdle();
            void setSearchStateRunning();
            void setSearchStatePaused();

            // Search management (KJob)
            SearchJobStoppable *searchJobStoppable = nullptr;
            void setupSearchManager();
            void resetSearchButton();
            bool searchButtonClickPending = false;

            bool isSearchRunning = false;
            void resetSearchState();

            void resetToDefaultSearchCriteria();
            void clearSearchResults();
            void initiateSearchFields();
            void loadSearchCriteria(Search *search);
            void getSearchCriteria();

            QString exportSearchResults();

            void batchProcessSearchResults();
            void insertSearchHistoryToTable();
            void loadSearchHistoryTableToModel();

            int lastSearchSortSection;
            int lastSearchSortOrder;

            void refreshDifferencesCatalogSelection();
            void processSearch();
            void searchFilesStoppable();

            //Search history
            int lastSearchHistorySortSection;
            int lastSearchHistorySortOrder;

            //Result operations
            QString moveFileToTrash(QString fileFullPath);
            QString deleteFile(QString fileFullPath);

            void pauseCurrentSearch();
            void resumeCurrentSearch();

            void updateTooltips();

        //TAB: Catalogs
            QStringListModel catalogListModel;
            QStringList catalogFileList;

            void loadCollection();
            void updateCatalogsScreenStatistics();
            void convertCatalog(QString catalogSourcePath);
            void backupFile(QString filePath);
            void saveCatalogChanges();

            void importFromVVV();
            void createMissingParentDirectories();

            // Updates Batch processing
            QList<Device*> batchCatalogs;           // All catalogs to process in batch
            int batchCurrentIndex = 0;              // Current catalog being processed
            bool inBatchMode = false;               // Clear flag: are we in batch mode?
            bool showEachCatalogUpdateSummary = false;
            // Global statistics for batch
            qint64 globalUpdateTotalFiles = 0;
            qint64 globalUpdateDeltaFiles = 0;
            qint64 globalUpdateTotalSize = 0;
            qint64 globalUpdateDeltaSize = 0;
            int updatedCatalogs = 0;
            int skippedCatalogs = 0;
            bool reportAllUpdates(Device *device, QList<qint64> list, QString updateType);

            // QList<Device*> pendingCatalogUpdates;
            // int totalCatalogsToUpdate = 0;
            // int completedCatalogUpdates = 0;
            // QList<Device*> catalogsToUpdate;  // All catalogs to update
            // int currentCatalogIndex = 0;      // Which one we're currently processing

            void startNextCatalogUpdate();

        //TAB: Explore
            QString exploreSelectedFolderFullPath;
            QString exploreSelectedDirectoryName;

            void openCatalogToExplore();
            void loadSelectedDirectoryFilesToExplore();
            void loadCatalogDirectoriesToExplore();

            int lastExploreSortSection;
            int lastExploreSortOrder;
            bool optionDisplayFolders;
            bool optionDisplaySubFolders;

        //TAB: Create
            QFileSystemModel *fileSystemModel;

            QString connectToShare(const QString& serverIP, const QString& directory,
                                   const QString& username, const QString& password);

            void iterateFiles(const QString &shareUrl, const QString &username,
                              const QString &password, const QStringList &fileExtensions);

            void listFilesRecursively(const QString &shareUrl,
                                      const QString &username,
                                      const QString &password,
                                      const QString &currentPath);

            QStringList storageNameList;

            QString fileMetadataString;

            void loadFileSystem(QString newCatalogPath);
            void createCatalog();
            void loadStorageList();
            void onCatalogOperationCompleted();
            void restoreCreateCatalogUIState();
            void removeFileFromResults(QString fullFilePath);
            void cleanupStoppedCatalogCreation();

        //TAB: Storage
            int     selectedStorageIndexRow;
            QStringListModel *storageListModel;
            int lastStorageSortSection;
            int lastStorageSortOrder;
            void updateStorageSelectionStatistics();
            void displayStoragePicture();

        //TAB: Devices
            void setupDeviceManager();

            int lastDevicesSortSection;
            int lastDevicesSortOrder;
            void setDeviceTreeExpandState(bool toggle);
            int  optionDeviceTreeExpandState;
            bool optionDisplayCatalogs;
            bool optionDisplayStorage;
            bool optionDisplayPhysicalGroup;
            bool optionDisplayVirtualGroups;
            bool optionDisplayFullDeviceTable;
            int  deviceTreeExpandState;

            void addDeviceVirtual();
            void addDeviceStorage(int parentID);
            void editDevice();
            void saveDeviceForm();
            void assignCatalogToDevice(Device *catalogDevice, Device *parentDevice);
            void assignStorageToDevice(int storageID,int deviceID);
            void unassignPhysicalFromDevice(int deviceID, int deviceParentID);
            void deleteDeviceItem();

            void loadDevicesView(QString sourceTrigger);
            void loadDevicesTreeToModel(QString targetTreeModel);
            void loadDevicesStorageToModel();
            void loadDevicesCatalogToModel();

            void updateNumbers();
            void updateAllNumbers();
            void shiftIDsInDeviceTable(int shiftAmount);
            void loadParentsList();
            void recordAllDeviceStats(QDateTime dateTime);
            void updateAllDeviceActive();
            void recordDevicesSnapshot();
            int countTreeLevels(const QMap<int, QList<int>>& deviceTree, int parentId);

            //Migration 1.22 to 2.0
            void migrateCollectionFromV1toV2();
            void importVirtualToDevices();
            void importStorageToDevices();
            void importCatalogsToDevices();
            void generateAndAssociateCatalogMissingIDs();
            void importStatistics();
            void loadStatisticsCatalogFileToTable();
            void loadStatisticsStorageFileToTable();
            void loadVirtualStorageFileToTable();
            void loadVirtualStorageCatalogFileToTable();
            void importVirtualAssignmentsToDevices();
            void convertFoldersIdxFiles();
            void importExcludeIntoParameter();
            void convertTags();
            void convertSearchHistory();
            void convertStorage();

        //TAB: Statistics
            QStringList typeOfData;
            QString selectedTypeOfData;
            QStringListModel *listModel;
            QDateTime graphicStartDate;
            void loadStatisticsDataTypes();
            void loadStatisticsChart();

        //TAB: Tags
            void reloadTagsData();
            void loadTagsToTable();
            void loadTagsTableToExistingTagsModel();
            void loadTagsTableToTagsAndFolderListModel();
            void loadFileSystemTags(QString newTagFolderPath);
            QStringListModel *tagListModel;
            QString selectedTagListName;
            QString newTagFolderPath;

        //TAB: BackUp
            bool optionDisplayFullMappingTable;
            void loadBackUpMapping();
            void loadBackUpMappingTotals();
            void loadBackUpMappingTable();
            void loadBackUpDeviceLists(QString list);
            void saveNewMapping();

        //TAB: Settings
            void changeCollectionFolder(QString newDirectory);
            void changeDatabaseFilePath(QString newDatabaseFilePath);

            //Export
            void exportToMemoryMode();
            void exportToSQLiteFile();
            bool exportAllCatalogFiles(QProgressDialog &progress);
            bool exportSingleCatalogFoldersFile(int catalogId, const QString &filePath);

    public slots:
            void displaySearchResults();
            void updateStatusBarFromSearchManager();
            void runDatabaseMigrations();
            void runDatabaseMigration_2_6();

    private slots:
        //Filters
            void on_Filters_pushButton_Filters_Hide_clicked();
            void on_Filters_pushButton_Filters_Show_clicked();
            void on_Filters_pushButton_ResetGlobal_clicked();
            void on_Filters_pushButton_ReloadCollection_clicked();
            void on_Filters_checkBox_SearchInCatalogs_toggled(bool checked);
            void on_Filters_checkBox_SearchInConnectedDrives_toggled(bool checked);

            void on_Filters_pushButton_TreeExpandCollapse_clicked();
            void on_Filters_treeView_Devices_clicked(const QModelIndex &index);
            void on_Filters_treeView_Devices_customContextMenuRequested(const QPoint &pos);

            void on_Filters_treeView_Directory_clicked(const QModelIndex &index);
            void on_Filter_pushButton_PickPath_clicked();

        //Settings
            void on_tabWidget_currentChanged(int index);
            void on_splitter_splitterMoved();

            void on_Settings_comboBox_DatabaseMode_currentTextChanged();
            void on_Settings_pushButton_DatabaseModeApplyAndRestart_clicked();

            void on_Settings_pushButton_SelectFolder_clicked();
            void on_Settings_pushButton_OpenFolder_clicked();
            void on_Settings_lineEdit_CollectionFolder_returnPressed();
            void on_Settings_checkBox_KeepOneBackUp_stateChanged();
            void on_Settings_checkBox_PreloadCatalogs_stateChanged(int arg1);
            void on_Settings_pushButton_ExportToSQLitFile_clicked();
            void on_Settings_pushButton_ApplyFolderpath_clicked();
            void on_Settings_lineEdit_CollectionFolder_textChanged();

            void on_Settings_pushButton_SelectDatabaseFilePath_clicked();
            void on_Settings_pushButton_EditDatabaseFile_clicked();
            void on_Settings_pushButton_NewDatabaseFile_clicked();
            void on_Settings_pushButton_ApplyFilepath_clicked();
            void on_Settings_lineEdit_DatabaseFilePath_textChanged();
            void on_Settings_lineEdit_DatabaseFilePath_returnPressed();
            void on_Settings_pushButton_ExportToMemoryMode_clicked();

            void on_Settings_lineEdit_DataMode_Hosted_HostName_textChanged(const QString &arg1);
            void on_Settings_lineEdit_DataMode_Hosted_DatabaseName_textChanged(const QString &arg1);
            void on_Settings_lineEdit_DataMode_Hosted_Port_textChanged(const QString &arg1);
            void on_Settings_lineEdit_DataMode_Hosted_UserName_textChanged(const QString &arg1);
            void on_Settings_lineEdit_DataMode_Hosted_Password_textChanged(const QString &arg1);

            void on_Settings_comboBox_Language_currentTextChanged(const QString &selectedLanguage);
            void on_Settings_comboBox_Theme_currentIndexChanged(int index);
            void on_Settings_checkBox_BiggerIconSize_stateChanged(int arg1);
            void on_Settings_checkBox_LoadLastCatalog_stateChanged(int arg1);
            void on_Settings_pushButton_OpenSettingsFile_clicked();
            void on_Settings_checkBox_SettingsFileCaseSensitiveSort_stateChanged();

            void on_Settings_pushButton_Documentation_clicked();
            void on_Settings_pushButton_ReleaseNotes_clicked();
            void on_Settings_checkBox_CheckVersion_stateChanged();

        //Search
            // Search manager slots
            void onSearchCompleted();
            void onSearchCancelled();
            void onSearchError(const QString &error);

            void handleSearchCompleted();
            void handleSearchStopped();
            void updateSearchProgress(int filesProcessed);
            void reportSearchStatistics();

            //UI elements
            void on_Search_pushButton_Search_clicked();
            void on_Search_pushButton_Stop_clicked();
            void on_Search_pushButton_CleanSearchText_clicked();
            void on_Search_pushButton_ResetAll_clicked();
            void on_Search_pushButton_ProcessResults_clicked();
            void on_Search_pushButton_PasteFromClipboard_clicked();
            void on_Search_lineEdit_SearchText_returnPressed();
            void on_Search_comboBox_TextCriteria_currentIndexChanged(int index);

            void on_Search_treeView_CatalogsFound_clicked(const QModelIndex &index);
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
            void on_Search_checkBox_DuplicatesName_checkStateChanged(const Qt::CheckState &arg1);
            void on_Search_checkBox_DuplicatesSize_checkStateChanged(const Qt::CheckState &arg1);
            void on_Search_checkBox_DuplicatesDateModified_checkStateChanged(const Qt::CheckState &arg1);
            void on_Search_checkBox_Differences_toggled(bool checked);
            void on_Search_checkBox_DifferencesName_checkStateChanged(const Qt::CheckState &arg1);
            void on_Search_checkBox_DifferencesSize_checkStateChanged(const Qt::CheckState &arg1);
            void on_Search_checkBox_DifferencesDateModified_checkStateChanged(const Qt::CheckState &arg1);

            void on_Search_checkBox_ShowFolders_toggled(bool checked);
            void on_Search_checkBox_FolderCriteria_toggled(bool checked);

            void on_Search_treeView_History_activated(const QModelIndex &index);
            void on_Search_pushButton_FileFoundMoreStatistics_clicked();
            void on_SearchTreeViewFilesFoundHeaderSortOrderChanged();
            void on_SearchTreeViewHistoryHeaderSortOrderChanged();
            void on_Search_splitter_Results_splitterMoved();

            void setupFileContextMenus();

            //Context menu
            void searchContextOpenFile();
            void searchContextOpenFolder();
            void searchContextOpenExplore();
            void searchContextCopyAbsolutePath();
            void searchContextCopyFolderPath();
            void searchContextCopyFileNameWithExtension();
            void searchContextCopyFileNameWithoutExtension();
            void searchContextMoveFileToTrash();
            void searchContextDeleteFile();

        //Create
            void on_Create_pushButton_PickPath_clicked();
            void on_Create_treeView_Explorer_clicked(const QModelIndex &index);
            void on_Create_pushButton_AddDirectoryToExclude_clicked();
            void on_Create_pushButton_AddStorage_clicked();
            void on_Create_pushButton_GenerateFromPath_clicked();
            void on_Create_pushButton_CreateCatalog_clicked();
            void on_Create_pushButton_Stop_clicked();
            void on_Create_treeView_Excluded_customContextMenuRequested(const QPoint &pos);
            void setupCatalogManager();

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
            void exploreContextMoveFileToTrash();
            void exploreContextDeleteFile();

        //Devices
            void on_Devices_radioButton_DeviceTree_clicked();
            void on_Devices_radioButton_StorageList_clicked();
            void on_Devices_radioButton_CatalogList_clicked();
            void on_Devices_pushButton_InsertRootLevel_clicked();
            void on_Devices_pushButton_AddVirtual_clicked();
            void on_Devices_pushButton_AddStorage_clicked();
            void on_Devices_pushButton_Save_clicked();
            void on_Devices_pushButton_Cancel_clicked();
            void on_Devices_checkBox_DisplayCatalogs_stateChanged(int arg1);
            void on_Devices_checkBox_DisplayStorage_stateChanged(int arg1);
            void on_Devices_checkBox_DisplayPhysicalGroup_stateChanged(int arg1);
            void on_Devices_checkBox_DisplayVirtualGroups_stateChanged(int arg1);
            void on_Devices_checkBox_DisplayFullTable_stateChanged(int arg1);
            void on_Devices_treeView_DeviceList_clicked(const QModelIndex &index);
            void on_Devices_treeView_DeviceList_customContextMenuRequested(const QPoint &pos);
            void on_DevicesTreeViewDeviceListHeaderSortOrderChanged();
            void on_Devices_pushButton_TreeExpandCollapse_clicked();
            void on_Devices_pushButton_EditList_clicked();
            void on_Devices_pushButton_SelectPath_clicked();
            void on_Devices_pushButton_Snapshot_clicked();
            void on_Devices_pushButton_ApplyToSelection_clicked();

            //Catalogs
            void on_Catalogs_pushButton_UpdateAllActive_clicked();
            void on_Catalogs_pushButton_UpdateCatalog_clicked();
            void on_Catalogs_pushButton_Import_clicked();

            //Storage
            void on_Storage_pushButton_UpdateStorage_clicked();

            //BackUp
            void on_BackUp_pushButton_SaveMapping_clicked();
            void on_BackUp_pushButton_ReloadDeviceMappings_clicked();
            void on_BackUp_pushButton_ReloadSourceList_clicked();
            void on_BackUp_pushButton_ReloadSourceListWithoutMapping_clicked();
            void on_BackUp_pushButton_ReloadTargetList_clicked();
            void on_BackUp_pushButton_ReloadTargetListWithoutMapping_clicked();            void on_BackUp_pushButton_DeleteSelectedMapping_clicked();
            void on_BackUp_checkBox_DisplayFullTable_checkStateChanged(const Qt::CheckState &arg1);
            void on_BackUp_radioButton_Source_clicked();
            void on_BackUp_radioButton_Target_clicked();

        //Statistics
            void on_Statistics_pushButton_EditDeviceStatisticsFile_clicked();
            void on_Statistics_pushButton_Reload_clicked();
            void on_Statistics_comboBox_SelectSource_currentTextChanged();
            void on_StatisticsComboBoxSelectCatalogCurrentIndexChanged(const QString &selectedCatalog);
            void on_Statistics_comboBox_TypeOfData_currentTextChanged();
            void on_Statistics_lineEdit_GraphicStartDate_returnPressed();
            void on_Statistics_pushButton_ClearDate_clicked();
            void on_Statistics_pushButton_PickDate_clicked();
            void on_Statistics_calendarWidget_clicked(const QDate &date);
            void on_Statistics_checkBox_DisplayEachValue_clicked();

        //Tags
            void on_Tags_pushButton_PickFolder_clicked();
            void on_Tags_pushButton_TagFolder_clicked();
            void on_Tags_pushButton_Reload_clicked();
            void on_Tags_pushButton_OpenTagsFile_clicked();
            void on_Tags_listView_ExistingTags_clicked(const QModelIndex &index);
            void on_Tags_treeview_Explorer_clicked(const QModelIndex &index);
            void on_Tags_treeView_FolderTags_customContextMenuRequested(const QPoint &pos);

        protected:
            void changeEvent(QEvent *event) override;
};

#endif // MAINWINDOW_H
