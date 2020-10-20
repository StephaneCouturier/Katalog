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
// Modified by: Stephane Couturier
// Created:     2020-07-11
// Version:     0.8
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
#include <KXmlGuiWindow>
#include <KMessageBox>
#include <KComboBox>

class KJob;
class QTableView;
//class KTextEdit;

QT_BEGIN_NAMESPACE
namespace Ui { class MainWindow; }
QT_END_NAMESPACE

class MainWindow : public KXmlGuiWindow
{
    Q_OBJECT

    public:
        explicit MainWindow(QWidget *parent = nullptr);
        ~MainWindow();

    private:
        //Global
            //KDE
            Ui::MainWindow *ui;

            QSqlRelationalTableModel *model;
            QSqlRelationalTableModel *model2;
            int authorIdx, genreIdx;
            void startSQLDB();

            void hideDevelopmentUIItems();
            //KDE menus/icons
            void setupActions();

            //parameters
            QString fileName;

            //Application settings
            QString settingsFile;
            void loadSettings();
            void saveSettings();

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

        //TAB: Collection
            QStringListModel catalogListModel;
            QStringList catalogFileList;
            QString selectedCatalogFile;
            QString selectedCatalogName;
            QString selectedCatalogPath;
            qint64 selectedCatalogFileCount;
            QString selectedCatalogTotalFileSize;
            bool selectedCatalogIncludeHidden;
            QString selectedCatalogFileType;
            QString selectedCatalogStorage;
            bool selectedCatalogIncludeSymblinks;
            void LoadCatalog(QString fileName);
            void SaveCatalog(QString newCatalogName);
            void loadCatalogsToModel();
            void LoadFilesToModel();
            bool verifyCatalogPath(QString catalogSourcePath);
            void recordSelectedCatalogStats();
            void convertCatalog(QString catalogSourcePath);

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

            QList<QString>  sFileNames;
            QList<qint64>   sFileSizes;
            QList<QString>  sFilePaths;
            QList<QString>  sFileDateTimes;

            //search
            QString regexSearchtext;
            QString regexFileType;
            void SearchFiles();
            void SearchFilesInCatalog(const QString &sourceCatalog);

            //outputs
            QStringList filesFoundList;
            QStringList catalogFoundList;
            QStringListModel *catalogFoundListModel;
            QStringList searchTextList;
            void refreshCatalogSelectionList();

        //TAB: Create Catalog Tab
            QFileSystemModel *fileSystemModel;
            QStringListModel *fileListModel;
            QString newCatalogPath;
            QString newCatalogName;
            QString newCatalogStorage;
            void LoadFileSystem(QString newCatalogPath);
            void CatalogDirectory(QString newCatalogPath,
                                  bool includeHidden,
                                  QString fileType,
                                  QStringList fileTypes,
                                  QString newCatalogStorage,
                                  bool includeSymblinks);
            void loadStorageList();
            QStringList storageNameList;

        //TAB: Storage
            QString storageFilePath;
            void loadStorageModel();
            QString selectedStorageName;
            QString selectedStorageID;
            QString selectedStorageType;
            QString selectedStorageLocation;
            QString selectedStoragePath;
            int     selectedStorageIndexRow;
            QStringListModel *storageListModel;
            QStringList locationCatalogList;

        //TAB: Statistics
            QStringList typeOfData;
            QString selectedTypeOfData;
            void loadTypeOfData();

        //TAB: Tags
            void loadFileSystemTags(QString newTagFolderPath);
            QStringListModel *tagListModel;

        //TESTS
            QString collectionFolder;
            void FileTypesEditor();
            QStringListModel *listModel;

    private slots:
        //Menu KDE
            void newFile();
            void openFile();
            void saveFile();
            void saveFileAs();
            void saveFileAs(const QString &outputFileName);
            void downloadFinished(KJob* job);

        //Search
            void on_PB_S_ResetAll_clicked();
            void on_KCB_SearchText_returnPressed();
            void on_PB_Search_clicked();
            void setupFileContextMenu();
            void on_PB_ExportResults_clicked();
            QString getCatalogStorageName(QString catalogFilePath);
            void getLocationCatalogList(QString location);

            //context menu
            void contextOpenFile();
            void contextOpenFolder();
            void contextCopyAbsolutePath();
            void contextCopyFolderPath();
            void contextCopyFileNameWithExtension();
            void contextCopyFileNameWithoutExtension();
            void on_TrV_FilesFound_customContextMenuRequested(const QPoint &pos);
            void on_TrV_FilesFound_clicked(const QModelIndex &index);

        //Create
            void on_Create_PB_AddStorage_clicked();

            void on_PB_PickPath_clicked();
            void on_PB_GenerateFromPath_clicked();
            void on_PB_CreateCatalog_clicked();
            void on_TV_Explorer_activated(const QModelIndex &index);

        //Collection
            void on_Collection_PB_ViewCatalogStats_clicked();
            void on_Collection_PB_Reload_clicked();
            void on_Collection_PB_Search_clicked();
            void on_PB_C_OpenFolder_clicked();
            void on_PB_ViewCatalog_clicked();
            void on_PB_UpdateCatalog_clicked();
            void on_PB_C_Rename_clicked(); //not active yet
            void on_PB_EditCatalogFile_clicked();
            void on_Collection_pushButton_Convert_clicked();
            void on_PB_ExportCatalog_clicked();
            void on_PB_DeleteCatalog_clicked();
            void on_TrV_CatalogList_activated(const QModelIndex &index);
            void on_TrV_FileList_clicked(const QModelIndex &index);
            void on_TrV_CatalogList_doubleClicked(const QModelIndex &index);
            void on_TrV_FileList_customContextMenuRequested(const QPoint &pos);
            void context2CopyAbsolutePath();

        //Explore
            void exploreLoadDirectories();

        //Storage
            void getStorageInfo(const QString &storagePath);
            void on_Storage_TrV_activated(const QModelIndex &index);

            void on_Storage_PB_CreateList_clicked();
            void on_Storage_PB_Reload_clicked();
            void on_Storage_PB_EditAll_clicked();
            void on_Storage_PB_SaveAll_clicked();

            void on_Storage_PB_New_clicked();
            void on_Storage_PB_SearchStorage_clicked();
            void on_Storage_PB_SearchLocation_clicked();
            void on_Storage_PB_CreateCatalog_clicked();
            void on_Storage_PB_OpenFilelight_clicked();
            void on_Storage_PB_Update_clicked();
            void on_Storage_PB_Delete_clicked();

        //Tags
            void on_Tags_PB_Reload_clicked();

            void on_TV_Explorer_2_activated(const QModelIndex &index);
            void on_PB_T_PickFolder_clicked();
            void loadFolderTagModel();

        //Settings
            void on_Settings_ChBx_SaveRecordWhenUpdate_stateChanged();
            //DEV
            void on_PB_SelectCollectionFolder_clicked();
            void on_pushButton_8_clicked();
            void on_pushButton_7_clicked();
            void on_pushButton_9_clicked();

        //Stats
            void on_Stats_PB_OpenStatsFile_clicked();
            void on_Stats_PB_Reload_clicked();
            void on_Stats_CB_SelectCatalog_currentIndexChanged();
            void on_Stats_comboBox_TypeOfData_currentIndexChanged();
            void statsLoadChart();
            void statsLoadChart2();//DEV

        //Tests
            void on_TR_CatalogFoundList_clicked(const QModelIndex &index);
            void on_PB_GetTextFromClipboard_clicked();
            void on_PB_RecordCatalogStats_clicked();
            void on_PB_TagFolder_clicked();
            void on_LI_ExistingTags_activated(const QModelIndex &index);
            void on_test_pb_insert_clicked();

};

#endif // MAINWINDOW_H
