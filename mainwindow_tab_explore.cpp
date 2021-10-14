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
// File Name:   mainwindow_tab_explore.cpp
// Purpose:     methods for the screen Explore
// Description:
// Author:      Stephane Couturier
// Version:     1.00
/////////////////////////////////////////////////////////////////////////////
*/

#include "mainwindow.h"
#include "ui_mainwindow.h"

//#include "collection.h"
#include "catalog.h"
#include "filesview.h"

#include <QTextStream>
#include <QDesktopServices>
#include <QFileDialog>
#include <QSortFilterProxyModel>

//#include <KMessageBox>
//#include <KLocalizedString>

//----------------------------------------------------------------------

void MainWindow::on_Explore_treeView_FileList_clicked(const QModelIndex &index)
{
    //Get file from selected row
    QString selectedFileName   = ui->Explore_treeView_FileList->model()->index(index.row(), 0, QModelIndex()).data().toString();
    QString selectedFileFolder = ui->Explore_treeView_FileList->model()->index(index.row(), 3, QModelIndex()).data().toString();
    QString selectedFile = selectedFileFolder+"/"+selectedFileName;

    //Open the file (fromLocalFile needed for spaces in file name)
    QDesktopServices::openUrl(QUrl::fromLocalFile(selectedFile));
}
//----------------------------------------------------------------------

//Context menu
void MainWindow::on_Explore_treeView_FileList_customContextMenuRequested(const QPoint &pos)
{
    // for most widgets
    QPoint globalPos = ui->Explore_treeView_FileList->mapToGlobal(pos);

    QMenu fileContextMenu;

//            QAction *menuAction1 = new QAction(QIcon::fromTheme("document-open-data"),(tr("Open file")), this);
//            connect(menuAction1, &QAction::triggered, this, &MainWindow::contextOpenFile);
//            fileContextMenu.addAction(menuAction1);

//            QAction *menuAction2 = new QAction(QIcon::fromTheme("document-open-data"),(tr("Open folder")), this);
//            connect(menuAction2, &QAction::triggered, this, &MainWindow::contextOpenFolder);
//            fileContextMenu.addAction(menuAction2);

//            fileContextMenu.addSeparator();

    QAction *menuAction3 = new QAction(QIcon::fromTheme("edit-copy"),(tr("Copy absolute path")), this);
    connect( menuAction3,&QAction::triggered, this, &MainWindow::context2CopyAbsolutePath);
    fileContextMenu.addAction(menuAction3);

//            QAction *menuAction4 = new QAction(QIcon::fromTheme("edit-copy"),(tr("Copy file name with extension")), this);
//            connect( menuAction4,&QAction::triggered, this, &MainWindow::contextCopyFileNameWithExtension);
//            fileContextMenu.addAction(menuAction4);

//            QAction *menuAction5 = new QAction(QIcon::fromTheme("edit-copy"),(tr("Copy file name without extension")), this);
//            connect( menuAction5,&QAction::triggered, this, &MainWindow::contextCopyFileNameWithoutExtension);
//            fileContextMenu.addAction(menuAction5);

    //fileContextMenu.addSeparator();

    // TEST:  copy file to..., cut file, move file to..., trash, delete, the full Dolphin menu!
    //QAction *menuAction30 = new QAction(QIcon::fromTheme("edit-copy"),(tr("TEST")), this);
    //fileContextMenu.addAction(menuAction30);

    QAction* selectedItem = fileContextMenu.exec(globalPos);
    if (selectedItem)
    {
        //something
    }
    else
    {
        //KMessageBox::information(this,"test:\n did nothing.");
    }
}
//----------------------------------------------------------------------

void MainWindow::on_Explore_treeview_Directories_clicked(const QModelIndex &index)
{
    selectedDirectoryName = ui->Explore_treeview_Directories->model()->index(index.row(), 0, QModelIndex()).data().toString();

    loadSelectedDirectoryFilesToExplore();
}
//----------------------------------------------------------------------
void MainWindow::openCatalogToExplore()
{
    //Reset selectedDirectoryName to load and display all catalog files
    selectedDirectoryName.clear();

    //Load the files of the Selected Catalog
    loadCatalogFilesToExplore();
    loadCatalogDirectoriesToExplore();
    loadSelectedDirectoryFilesToExplore();

    //Go to the Explorer tab
    ui->Explore_label_CatalogNameDisplay->setText(selectedCatalogName);
    ui->Explore_label_CatalogPathDisplay->setText(selectedCatalogPath);
    ui->tabWidget->setCurrentIndex(2); // tab 0 is the Explorer tab
}


//Load a catalog to view the files
void MainWindow::loadCatalogFilesToExplore()
{
    // Start animation while opening
    QApplication::setOverrideCursor(Qt::WaitCursor);

    // Get infos stored in the file
    QFile catalogFile(selectedCatalogFile);
    if(!catalogFile.open(QIODevice::ReadOnly)) {
        QMessageBox::information(this,"Katalog",tr("No catalog found."));
        return;
    }
    QFileInfo catalogFileInfo(selectedCatalogFile);

    QTextStream textStream(&catalogFile);

    //clear database
    QSqlQuery deleteQuery0;
    deleteQuery0.exec("DELETE FROM file");

    //prepare query to load file info
    QSqlQuery insertQuery;
    QString insertSQL = QLatin1String(R"(
                        INSERT INTO file (
                                        fileName,
                                        filePath,
                                        fileSize,
                                        fileDateUpdated,
                                        fileCatalog )
                        VALUES(
                                        :fileName,
                                        :filePath,
                                        :fileSize,
                                        :fileDateUpdated,
                                        :fileCatalog )
                                    )");
    insertQuery.prepare(insertSQL);

    //set temporary values
        QString     line;
        QStringList fieldList;
        int         fieldListCount;
        QString     filePath;
        qint64      fileSize;
        QString     fileDateTime;
        //QRegExp tagExp; tagExp.setPattern("\t");

    //load each file
    while (true)
    {
        line = textStream.readLine();
        if (line.isNull())
            break;
        else
            if (line.left(1)!="<"){

                //Split the string with \t (tabulation) into a list
                QRegExp tagExp("\t"); //setpattern
                fieldList.clear();
                fieldList = line.split(tagExp);

                fieldListCount = fieldList.count();

                // Get the filePath from the list:
                filePath        = fieldList[0];

                // Get the fileSize from the list if available
                if (fieldListCount == 3){
                        fileSize = fieldList[1].toLongLong();}
                else fileSize = 0;

                // Get the fileDateTime from the list if available
                if (fieldListCount == 3){
                        fileDateTime = fieldList[2];}
                else fileDateTime = "";

                //Get file informations
                QFileInfo file(filePath);

                //Append data to the database
                insertQuery.bindValue(":fileName", file.fileName());
                insertQuery.bindValue(":filePath", file.path());
                insertQuery.bindValue(":fileSize", fileSize);
                insertQuery.bindValue(":fileDateUpdated", fileDateTime);
                insertQuery.bindValue(":fileCatalog", catalogFileInfo.baseName());
                insertQuery.exec();
            }
    }

    catalogFile.close();

    //Stop animation
    QApplication::restoreOverrideCursor();
}

void MainWindow::loadSelectedDirectoryFilesToExplore()
{
    // Load all files and create model
    QString selectSQL = QLatin1String(R"(
                        SELECT  fileName AS Name,
                                fileSize AS Size,
                                fileDateUpdated AS Date,
                                filePath AS Path,
                                fileCatalog AS Catalog
                        FROM file
                                    )");

    if (selectedDirectoryName!=""){
        selectSQL = selectSQL + " WHERE filePath =:filePath";
    }

    QSqlQuery loadCatalogQuery;
    loadCatalogQuery.prepare(selectSQL);
    loadCatalogQuery.bindValue(":filePath",selectedDirectoryName);
    loadCatalogQuery.exec();

    QSqlQueryModel *loadCatalogQueryModel = new QSqlQueryModel;
    loadCatalogQueryModel->setQuery(loadCatalogQuery);

    FilesView *proxyModel2 = new FilesView(this);
    proxyModel2->setSourceModel(loadCatalogQueryModel);

    proxyModel2->setHeaderData(0, Qt::Horizontal, tr("Name"));
    proxyModel2->setHeaderData(1, Qt::Horizontal, tr("Size"));
    proxyModel2->setHeaderData(2, Qt::Horizontal, tr("Date"));
    proxyModel2->setHeaderData(3, Qt::Horizontal, tr("Path"));
    proxyModel2->setHeaderData(4, Qt::Horizontal, tr("Catalog"));

    // Connect model to tree/table view
    ui->Explore_treeView_FileList->setModel(proxyModel2);
    ui->Explore_treeView_FileList->QTreeView::sortByColumn(0,Qt::AscendingOrder);
    ui->Explore_treeView_FileList->header()->setSectionResizeMode(QHeaderView::Interactive);
    ui->Explore_treeView_FileList->header()->resizeSection(0, 600); //Name
    ui->Explore_treeView_FileList->header()->resizeSection(1, 110); //Size
    ui->Explore_treeView_FileList->header()->resizeSection(2, 140); //Date
    ui->Explore_treeView_FileList->header()->resizeSection(3, 400); //Path

    QString countSQL = QLatin1String(R"(
                        SELECT  count (*)
                        FROM file
                                    )");

    if (selectedDirectoryName!=""){
        countSQL = countSQL + " WHERE filePath =:filePath";
    }

    QSqlQuery countQuery;
    countQuery.prepare(countSQL);
    countQuery.bindValue(":filePath",selectedDirectoryName);
    countQuery.exec();
    countQuery.next();

    ui->Explore_label_FilesNumberDisplay->setNum(countQuery.value(0).toInt());

}

//Load a catalog's directory to view the files
void MainWindow::loadCatalogDirectoriesToExplore()
{
    //prepare query to load file info
    QSqlQuery getDirectoriesQuery;
    QString getDirectoriesSQL = QLatin1String(R"(
                                SELECT DISTINCT filePath
                                FROM file
                                ORDER BY filePath ASC
                                    )");
    getDirectoriesQuery.prepare(getDirectoriesSQL);
    getDirectoriesQuery.exec();

    QSqlQueryModel *getDirectoriesQueryModel = new QSqlQueryModel;
    getDirectoriesQueryModel->setQuery(getDirectoriesQuery);

    QSortFilterProxyModel *getDirectoriesProxyModel = new QSortFilterProxyModel;
    getDirectoriesProxyModel->setSourceModel(getDirectoriesQueryModel);

    getDirectoriesProxyModel->setHeaderData(0, Qt::Horizontal, tr("Directory"));

    // Connect model to tree/table view
    ui->Explore_treeview_Directories->setModel(getDirectoriesProxyModel);
    ui->Explore_treeview_Directories->QTreeView::sortByColumn(0,Qt::AscendingOrder);
    ui->Explore_treeview_Directories->header()->setSectionResizeMode(QHeaderView::Interactive);
    ui->Explore_treeview_Directories->header()->resizeSection(0, 600); //Directory

    QString countSQL = QLatin1String(R"(
                        SELECT count (DISTINCT (filePath))
                        FROM file
                                    )");
    QSqlQuery countQuery;
    countQuery.prepare(countSQL);
    countQuery.exec();
    countQuery.next();

   ui->Explore_label_DirectoryNumberDisplay->setNum(countQuery.value(0).toInt());

}

//----------------------------------------------------------------------
void MainWindow::context2CopyAbsolutePath()
{
    QModelIndex index=ui->Explore_treeView_FileList->currentIndex();
    QString selectedFileName   = ui->Explore_treeView_FileList->model()->index(index.row(), 0, QModelIndex()).data().toString();
    QString selectedFileFolder = ui->Explore_treeView_FileList->model()->index(index.row(), 3, QModelIndex()).data().toString();
    QString selectedFileAbsolutePath = selectedFileFolder+"/"+selectedFileName;
    QClipboard *clipboard = QGuiApplication::clipboard();
    QString originalText = clipboard->text();
    clipboard->setText(selectedFileAbsolutePath);
}
//----------------------------------------------------------------------
