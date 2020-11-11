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
// Purpose:     methods for the scren Collection AND the screen Explore
// Description:
// Author:      Stephane Couturier
// Modified by: Stephane Couturier
// Created:     2020-10-10
// Version:     0.1
/////////////////////////////////////////////////////////////////////////////
*/

#include "mainwindow.h"
#include "ui_mainwindow.h"

#include "collection.h"
#include "catalog.h"

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
    QString selectedFileName   = ui->Explore_treeView_FileList->model()->index(index.row(), 1, QModelIndex()).data().toString();
    QString selectedFileFolder = ui->Explore_treeView_FileList->model()->index(index.row(), 4, QModelIndex()).data().toString();
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
//Load a catalog to view the files
void MainWindow::loadCatalogFilesToExplore()
{
    // Start animation while opening
    QApplication::setOverrideCursor(Qt::WaitCursor);

    //Set up temporary lists
    QList<QString> cfileNames;
    QList<qint64>  cfileSizes;
    QList<QString> cfilePaths;
    QList<QString> cfileDateTimes;

    // Get infos stored in the file
    QFile catalogFile(selectedCatalogFile);
    if(!catalogFile.open(QIODevice::ReadOnly)) {
        QMessageBox::information(this,"Katalog","No catalog found.");
        return;
    }

    QTextStream textStream(&catalogFile);

    while (true)
    {
        QString line = textStream.readLine();
        if (line.isNull())
            break;
        else
            if (line.left(1)!="<"){
                //Split the string with \t into a list
                QRegExp tagExp("\t");
                QStringList fieldList = line.split(tagExp);

                int fieldListCount = fieldList.count();

                // Get the filePath from the list:
                QString filePath        = fieldList[0];

                // Get the fileSize from the list if available
                qint64 fileSize;
                if (fieldListCount == 3){
                        fileSize = fieldList[1].toLongLong();}
                else fileSize = 0;

                // Get the fileDateTime from the list if available
                QString fileDateTime;
                if (fieldListCount == 3){
                        fileDateTime = fieldList[2];}
                else fileDateTime = "";

                //Get file informations
                QFileInfo file(filePath);

                //Append data to the lists
                cfileNames.append(file.fileName());
                cfileSizes.append(fileSize);
                cfilePaths.append(file.path());
                cfileDateTimes.append(fileDateTime);
            }
        }

    // Create model
    Catalog *catalog = new Catalog(this);

    // Populate model with data
    catalog->populateFileData(cfileNames, cfileSizes, cfilePaths, cfileDateTimes);

    QSortFilterProxyModel *proxyModel = new QSortFilterProxyModel(this);
    proxyModel->setSourceModel(catalog);

    // Connect model to tree/table view
    ui->Explore_treeView_FileList->setModel(proxyModel);
    ui->Explore_treeView_FileList->QTreeView::sortByColumn(0,Qt::AscendingOrder);
    ui->Explore_treeView_FileList->header()->setSectionResizeMode(QHeaderView::Interactive);
    ui->Explore_treeView_FileList->header()->resizeSection(0, 600); //Name
    ui->Explore_treeView_FileList->header()->resizeSection(1, 110); //Size
    ui->Explore_treeView_FileList->header()->resizeSection(2, 140); //Date
    ui->Explore_treeView_FileList->header()->resizeSection(3, 400); //Path

    int catalogFilesNumber = catalog->rowCount();
    ui->Explore_label_FilesNumberDisplay->setNum(catalogFilesNumber);

    //DEV   Stop animation
    QApplication::restoreOverrideCursor();
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
