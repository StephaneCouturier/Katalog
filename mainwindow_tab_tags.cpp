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
// File Name:   mainwindow_tab_tags.cpp
// Purpose:     methods for the screen Tags
// Description:
// Author:      Stephane Couturier
// Version:     1.00
/////////////////////////////////////////////////////////////////////////////
*/

#include "mainwindow.h"
#include "ui_mainwindow.h"
#include "tag.h"

#include <QFileDialog>
#include <QStandardItemModel>

//UI
void MainWindow::on_Tags_pushButton_PickFolder_clicked()
{
    //Get current selected path as default path for the dialog window
    QString newTagFolderPath;
    newTagFolderPath = ui->Tags_lineEdit_FolderPath->text();

    //Open a dialog for the user to select the directory to be cataloged. Only show directories.
    QString dir = QFileDialog::getExistingDirectory(this, tr("Select the directory to be cataloged in this new catalog"),
                                                    newTagFolderPath,
                                                    QFileDialog::ShowDirsOnly
                                                    | QFileDialog::DontResolveSymlinks);
    //Send the selected directory to LE_NewCatalogPath (input line for the New Catalog Path)
    ui->Tags_lineEdit_FolderPath->setText(dir);

    //Select this directory in the treeview.
    loadFileSystemTags(newTagFolderPath);
}
//----------------------------------------------------------------------
void MainWindow::on_Tags_pushButton_TagFolder_clicked()
{
    QString selectedTagFolder = ui->Tags_lineEdit_FolderPath->text();
    QString selectedTagName   = ui->Tags_lineEdit_TagName->text();

    if(selectedTagName == ""){
        QMessageBox::information(this,"Katalog","Please enter or select a tag for this folder.");
        return;
    }

    //file
    QString tagsFilePath = collectionFolder + "/" + "tags.csv";

    QString tagLine = selectedTagFolder + '\t' + selectedTagName;

    //Append data to the lists
    QFile tagsFile(tagsFilePath);
    if(tagsFile.open(QFile::Append | QFile::Text)) {
        QTextStream stream(&tagsFile);
        stream << tagLine << '\n';
        tagsFile.close();
    }
    //else
        //KMessageBox::information(this,"No tags file found.");

    //Refresh the lists
    loadFolderTagModel();

}
//----------------------------------------------------------------------
void MainWindow::on_Tags_pushButton_Reload_clicked()
{
    loadFolderTagModel();
}
//----------------------------------------------------------------------
void MainWindow::on_Tags_listView_ExistingTags_clicked(const QModelIndex &index)
{
    QString selectedTag = ui->Tags_listView_ExistingTags->model()->index(index.row(), 0, QModelIndex()).data().toString();
    ui->Tags_lineEdit_TagName->setText(selectedTag);
}
//----------------------------------------------------------------------
void MainWindow::on_Tags_treeview_Explorer_clicked(const QModelIndex &index)
{
    //Sends the selected folder in the tree for the New Catalog Path)
            //Get the model/data from the tree
            QFileSystemModel* pathmodel = (QFileSystemModel*)ui->Tags_treeview_Explorer->model();
            //get data from the selected file/directory
            QFileInfo fileInfo = pathmodel->fileInfo(index);
            //send the path to the line edit
            ui->Tags_lineEdit_FolderPath->setText(fileInfo.filePath());
}
//----------------------------------------------------------------------

//Load file system for the treeview
void MainWindow::loadFileSystemTags(QString newTagFolderPath)
{
        //QString newTagFolderPath="/";
     // Creates a new model
        fileSystemModel = new QFileSystemModel(this);

     // Set filter to show only directories
        fileSystemModel->setFilter(QDir::NoDotAndDotDot | QDir::AllDirs);

     // QFileSystemModel requires root path
        //QString rootPath ="/";
        //fileSystemModel->setRootPath(rootPath);
        fileSystemModel->setRootPath(newTagFolderPath);
     // Enable/Disable modifying file system
        //qfilesystemmodel->setReadOnly(true);

    // Attach the model to the view
        ui->Tags_treeview_Explorer->setModel(fileSystemModel);

    // Only show the tree, hidding other columns and the header row.
        ui->Tags_treeview_Explorer->setColumnWidth(0,250);
        ui->Tags_treeview_Explorer->setColumnHidden(1,true);
        ui->Tags_treeview_Explorer->setColumnHidden(2,true);
        ui->Tags_treeview_Explorer->setColumnHidden(3,true);
        ui->Tags_treeview_Explorer->setHeaderHidden(true);
        ui->Tags_treeview_Explorer->expandToDepth(1);
}
//----------------------------------------------------------------------

void MainWindow::loadFolderTagModel()
{
    //Set up temporary lists
    QList<QString> tFolderPaths;
    QList<QString> tTagNames;

    //Get data

        // Get infos stored in the file
        QString tagFilePath = collectionFolder + "/" + "tags.csv";

        QFile tagFile(tagFilePath);
        if(!tagFile.open(QIODevice::ReadOnly)) {
            //KMessageBox::information(this,"No tag file found.");
            //ui->TrV_Storage->nativeParentWidget()model()->>removeRows();
            return;
        }

        QTextStream textStream(&tagFile);

        //qint64 storageGrandTotal = 0;
        //qint64 storageGrandFree  = 0;

        while (true)
        {
            QString line = textStream.readLine();
            if (line.isNull())
                break;
            else
                if (line.left(6)!="Folder"){

                    //Split the string with tabulation into a list
                    QStringList fieldList = line.split('\t');

                    //QIcon icon;
                    //icon.fromTheme("drive-harddisk");

                    //Append data to the lists
                    tFolderPaths.append(fieldList[0]);
                    tTagNames.append(fieldList[1]);

                    //storageGrandTotal = storageGrandTotal + fieldList[7].toLongLong();
                    //storageGrandFree  = storageGrandFree  + fieldList[8].toLongLong();
                }
            }

        //Calculate totals
        //int storageCount = sNames.count();
        //qint64 storageGrandUsed = storageGrandTotal - storageGrandFree;

    // Create model
    Tag *tagModel = new Tag(this);

    // Populate model with data
    tagModel->populateTagData(tFolderPaths,
                                      tTagNames
                                      );

    QSortFilterProxyModel *proxyStorageModel = new QSortFilterProxyModel(this);
    proxyStorageModel->setSourceModel(tagModel);

    // Connect model to tree/table view
    ui->Tags_treeView_FolderTags->setModel(proxyStorageModel);
    ui->Tags_treeView_FolderTags->QTreeView::sortByColumn(1,Qt::AscendingOrder);
    ui->Tags_treeView_FolderTags->QTreeView::sortByColumn(0,Qt::AscendingOrder);
    ui->Tags_treeView_FolderTags->header()->setSectionResizeMode(QHeaderView::Interactive);
    ui->Tags_treeView_FolderTags->header()->resizeSection(0, 350); //Folder
    //ui->TrV_FolderTags->header()->resizeSection(1, 200); //Tag
    //ui->TrV_FolderTags->header()->hideSection(0); //Path

    //ui->L_StorageCountValue->setText(QString::number(storageCount));
    //ui->L_StorageSpaceTotalValue->setText(QString::number(storageGrandTotal));
    //ui->L_StorageSpaceUsedValue->setText(QString::number(storageGrandUsed));
    //ui->L_StorageSpaceFreeValue->setText(QString::number(storageGrandFree));

    QList<QString> tTagUniqueNames;
    tTagUniqueNames = tTagNames;
    tTagUniqueNames.removeDuplicates();

    //ui->LI_ExistingTags->setModel()

    //QStringListModel* model;
    //QStringList stringList;// = tTagNames;
    //stringList.append("test");

    //model->setStringList(stringList);
    //ui->LI_ExistingTags->setModel(model);

    //catalogFoundList.removeDuplicates();

    //Keep the catalog file name only
//    foreach(QString item, catalogFoundList){
//            int index = catalogFoundList.indexOf(item);
//            //QDir dir(item);
//            QFileInfo fileInfo(item);

//            //catalogFoundList[index] = dir.dirName();
//            catalogFoundList[index] = fileInfo.baseName();

//    }

    //Load list of catalogs in which files where found
    tagListModel = new QStringListModel(this);
    tagListModel->setStringList(tTagUniqueNames);
    ui->Tags_listView_ExistingTags->setModel(tagListModel);

}
//----------------------------------------------------------------------

