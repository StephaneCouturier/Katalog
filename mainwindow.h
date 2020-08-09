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
// Version:     0.1
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
            void HideDevelopmentUIItems();
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
            QStringList fileType_current;
            void setFileTypes();
            QString fileTypeSelected;

        //TAB: Search
            QString searchText;
            QString regexPattern;
            QString selectedSearchCatalog;
            QString selectedFileType;
            QString selectedTextCriteria;
            QString selectedSearchIn;
            QString sourceCatalog;
            QStringListModel catalogSelectionList;
            QStringList filesFoundList;
            QStringList catalogFoundList;
            QStringListModel *catalogFoundListModel;
            void initiateSearchValues();
            //void refreshCatalogSelectionList();
            void SearchFilesInCatalog(const QString &sourceCatalog);
            void SearchFiles();
            QString regexSearchtext;
            QString regexFileType;

            bool selectedPathOnly;
            bool selectedNameOnly;
            QStringList searchTextList;

        //TAB: Create Catalog Tab
            QFileSystemModel *fileSystemModel;
            QStringListModel *fileListModel;
            QString newCatalogPath;
            QString newCatalogName;
            void LoadFileSystem(QString newCatalogPath);
            void CatalogDirectory(QString newCatalogPath);

        //TAB: Explore Catalogs
            QStringListModel catalogListModel;
            QStringList catalogList;
            QString selectedCatalogFile;
            QString selectedCatalogName;
            QString selectedCatalogPath;
            void LoadCatalogList();
            void LoadCatalog(QString fileName);
            void SaveCatalog(QString newCatalogName);

        //TESTS
            void LoadCatalogsToModel();
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
            void on_LV_FilesFoundList_clicked(const QModelIndex &index);
            void setupFileContextMenu();
            //context menu
            void on_LV_FilesFoundList_customContextMenuRequested(const QPoint &pos);
            void on_LV_FileList_customContextMenuRequested(const QPoint &pos);
            void contextOpenFile();
            void contextOpenFolder();
            void contextCopyAbsolutePath();
            void contextCopyFileNameWithExtension();
            void contextCopyFileNameWithoutExtension();
            void on_PB_ExportResults_clicked();

        //Create
            void on_PB_PickPath_clicked();
            void on_PB_GenerateFromPath_clicked();
            void on_PB_CreateCatalog_clicked();
            void on_TV_Explorer_activated(const QModelIndex &index);

        //Collection
            void on_PB_UpdateCatalog_clicked();
            void on_PB_ViewCatalog_clicked();
            void on_PB_ExportCatalog_clicked();
            void on_PB_DeleteCatalog_clicked();
            void on_TrV_CatalogList_activated(const QModelIndex &index);
            void on_LV_FileList_clicked(const QModelIndex &index);

        //Settings
            void on_PB_SelectCollectionFolder_clicked();
            void on_pushButton_8_clicked();
            void on_pushButton_7_clicked();
            void on_pushButton_9_clicked();
            //void on_listWidget_clicked(const QModelIndex &index);
        //Tests


            void on_PB_EditCatalogFile_clicked();
};


#endif // MAINWINDOW_H
