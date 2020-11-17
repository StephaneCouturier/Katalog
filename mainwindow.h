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

#include <QMainWindow>
#include <QApplication>
#include <QAction>
#include <QFile>
#include <QFileInfo>
#include <QFileSystemModel>
#include <QStringList>
#include <QStringListModel>
#include <QAbstractItemView>
#include <QTreeView>
#include <QInputDialog>
#include <QTranslator>
#include <QDirModel>
#include <QDateTime>
#include <QAbstractTableModel>
#include <QMenu>
#include <QClipboard>
#include <QSortFilterProxyModel>
#include <QStorageInfo>
#include <QDebug>
#include <QtSql>
#include <QStandardPaths>
#include <QMessageBox>

#include <KComboBox>
#include <KXmlGuiWindow>
//#include <KMessageBox>
//class KJob;

QT_BEGIN_NAMESPACE
namespace Ui { class MainWindow; }
QT_END_NAMESPACE

class MainWindow : public KXmlGuiWindow //WIN QMainWindow
{
    Q_OBJECT

    public:
        explicit MainWindow(QWidget *parent = nullptr);
        ~MainWindow();

    private:
        //Global
            //UI
            Ui::MainWindow *ui;
            void loadCustomTheme1();
            void hideDevelopmentUIItems();

            //KDE menus/icons
            //void setupActions();

            //Application settings
            QString settingsFile;
            void loadSettings();
            void saveSettings();

            //Parameters
            QString fileName;

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
            QString fileTypeSelected;
            void setFileTypes();

            //database
            QSqlRelationalTableModel *storageModel;
            void startDatabase();

            //void refreshStorageStatistics();


        //TAB: Search
            //inputs
            QString searchText;
            QString regexPattern;
            QString selectedSearchCatalog;
            QString selectedFileType;
            QString selectedTextCriteria;
            QString selectedSearchIn;
            qint64  selectedMinimumSize;
            qint64  selectedMaximumSize;
            qint64  sizeMultiplierMin;
            qint64  sizeMultiplierMax;
            QString selectedMinSizeUnit;
            QString selectedMaxSizeUnit;
            QString selectedTags;
            QString sourceCatalog;
            QStringListModel catalogSelectionList;
            void initiateSearchValues();
            QString getCatalogStorageName(QString catalogFilePath);

            QList<QString>  sFileNames;
            QList<qint64>   sFileSizes;
            QList<QString>  sFilePaths;
            QList<QString>  sFileDateTimes;

            //search
            QString regexSearchtext;
            QString regexFileType;
            void searchFiles();
            void searchFilesInCatalog(const QString &sourceCatalog);

            //outputs
            QStringList filesFoundList;
            QStringList catalogFoundList;
            QStringListModel *catalogFoundListModel;
            QStringList searchTextList;
            void refreshCatalogSelectionList();

        //TAB: Collection
            QString collectionFolder;
            QStringListModel catalogListModel;
            QStringList catalogFileList;
            QString selectedCatalogFile;
            QString selectedCatalogName;
            QString selectedCatalogDateTime;
            QString selectedCatalogPath;
            qint64  selectedCatalogFileCount;
            qint64  selectedCatalogTotalFileSize;
            bool    selectedCatalogIncludeHidden;
            QString selectedCatalogFileType;
            QString selectedCatalogStorage;
            bool    selectedCatalogIncludeSymblinks;

            void loadCollection();
            void loadCatalogsToModel();
            bool verifyCatalogPath(QString catalogSourcePath);
            void recordSelectedCatalogStats(QString selectedCatalogName,
                                            int selectedCatalogFileCount,
                                            qint64 selectedCatalogTotalFileSize);
            void convertCatalog(QString catalogSourcePath);
            void backupCatalog(QString catalogSourcePath);
            void hideCatalogButtons();

        //TAB: Explore
            void loadCatalogFilesToExplore();

        //TAB: Create Catalog Tab
            QFileSystemModel *fileSystemModel;
            QStringListModel *fileListModel;
            QString newCatalogPath;
            QString newCatalogName;
            QString newCatalogStorage;
            QStringList storageNameList;

            void loadFileSystem(QString newCatalogPath);
            void catalogDirectory(QString newCatalogPath,
                                  bool includeHidden,
                                  QString fileType,
                                  QStringList fileTypes,
                                  QString newCatalogStorage,
                                  bool includeSymblinks);
            void loadStorageList();
            void saveCatalogToNewFile(QString newCatalogName);

        //TAB: Storage
            QString storageFilePath;
            QString selectedStorageName;
            int selectedStorageID;
            QString selectedStorageType;
            QString selectedStorageLocation;
            QString selectedStoragePath;
            int     selectedStorageIndexRow;
            QStringListModel *storageListModel;
            QStringList locationCatalogList;

            void loadStorageFileToTable();
            void loadStorageTableToModel();
            void saveStorageModelToFile();
            void saveStorageData();
            void updateStorageInfo();
            void refreshStorageStatistics();

        //TAB: Statistics
            QStringList typeOfData;
            QString selectedTypeOfData;
            QStringListModel *listModel;
            void loadStatisticsDataTypes();
            void loadStatisticsChart();

        //TAB: Tags
            void loadFileSystemTags(QString newTagFolderPath);
            QStringListModel *tagListModel;

    private slots:
        //Menu KDE
            /*
            void newFile();
            void openFile();
            void saveFile();
            void saveFileAs();
            void saveFileAs(const QString &outputFileName);
            void downloadFinished(KJob* job);
            */

        //Search
            void on_Search_pushButton_Search_clicked();
            void on_Search_pushButton_ResetAll_clicked();
            void on_Search_pushButton_ExportResults_clicked();
            void on_Search_pushButton_PasteFromClipboard_clicked();
            void on_Search_kcombobox_SearchText_returnPressed();

            void on_Search_listView_CatalogsFound_clicked(const QModelIndex &index);
            void on_Search_treeView_FilesFound_clicked(const QModelIndex &index);
            void on_Search_treeView_FilesFound_customContextMenuRequested(const QPoint &pos);
            void on_Search_pushButton_ShowHideCatalogResults_clicked();

            void setupFileContextMenu();
            void getLocationCatalogList(QString location);

            //context menu
            void contextOpenFile();
            void contextOpenFolder();
            void contextCopyAbsolutePath();
            void contextCopyFolderPath();
            void contextCopyFileNameWithExtension();
            void contextCopyFileNameWithoutExtension();

        //Collection
            void on_Collection_pushButton_SelectFolder_clicked();
            void on_Collection_pushButton_Reload_clicked();
            void on_Collection_pushButton_OpenFolder_clicked();
            void on_Collection_lineEdit_CollectionFolder_returnPressed();

            void on_Collection_pushButton_Search_clicked();
            void on_Collection_pushButton_ViewCatalog_clicked();
            void on_Collection_pushButton_Rename_clicked();
            void on_Collection_pushButton_UpdateCatalog_clicked();
            void on_Collection_pushButton_EditCatalogFile_clicked();
            void on_Collection_pushButton_RecordCatalogStats_clicked();
            void on_Collection_pushButton_ViewCatalogStats_clicked();
            void on_Collection_pushButton_Convert_clicked();
            void on_Collection_pushButton_DeleteCatalog_clicked();

            void on_Collection_treeView_CatalogList_clicked(const QModelIndex &index);
            void on_Collection_treeView_CatalogList_doubleClicked(const QModelIndex &index);

            void on_Explore_treeView_FileList_clicked(const QModelIndex &index);
            void on_Explore_treeView_FileList_customContextMenuRequested(const QPoint &pos);
            void context2CopyAbsolutePath();

        //Create
            void on_Create_pushButton_PickPath_clicked();
            void on_Create_treeView_Explorer_clicked(const QModelIndex &index);
            void on_Create_pushButton_AddStorage_clicked();
            void on_Create_pushButton_GenerateFromPath_clicked();
            void on_Create_pushButton_CreateCatalog_clicked();

        //Explore

        //Storage
            void on_Storage_pushButton_CreateList_clicked();
            void on_Storage_pushButton_Reload_clicked();
            void on_Storage_pushButton_EditAll_clicked();
            void on_Storage_pushButton_SaveAll_clicked(); //DEV

            void on_Storage_pushButton_New_clicked(); //DEV
            void on_Storage_pushButton_SearchStorage_clicked();
            void on_Storage_pushButton_SearchLocation_clicked();
            void on_Storage_pushButton_CreateCatalog_clicked();
            void on_Storage_pushButton_OpenFilelight_clicked(); //DEV
            void on_Storage_pushButton_Update_clicked(); //DEV
            void on_Storage_pushButton_Delete_clicked(); //DEV

            void on_Storage_treeView_StorageList_clicked(const QModelIndex &index);


        //Statistics
            void on_Statistics_pushButton_EditStatisticsFile_clicked();
            void on_Statistics_pushButton_Reload_clicked();
            void on_Statistics_comboBox_SelectCatalog_currentIndexChanged();
            void on_Statistics_comboBox_TypeOfData_currentIndexChanged();

        //Tags
            void on_Tags_pushButton_PickFolder_clicked();
            void on_Tags_pushButton_TagFolder_clicked();
            void on_Tags_pushButton_Reload_clicked();
            void on_Tags_listView_ExistingTags_clicked(const QModelIndex &index);
            void on_Tags_treeview_Explorer_clicked(const QModelIndex &index);
            void loadFolderTagModel();

        //Settings
            void on_Settings_checkBox_SaveRecordWhenUpdate_stateChanged();
            void on_Settings_checkBox_KeepOneBackUp_stateChanged();
            void on_Settings_comboBox_Theme_currentTextChanged();

};

#endif // MAINWINDOW_H
