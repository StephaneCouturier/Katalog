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
/////////////////////////////////////////////////////////////////////////////
*/

#include "mainwindow.h"
#include "ui_mainwindow.h"
#include "tag.h"

//UI------------------------------------------------------------------------

    void MainWindow::on_Tags_pushButton_PickFolder_clicked()
    {
        //Get current selected path as default path for the dialog window
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
        reloadTagsData();

    }
    //----------------------------------------------------------------------
    void MainWindow::on_Tags_pushButton_Reload_clicked()
    {
        reloadTagsData();
    }
    //----------------------------------------------------------------------
    void MainWindow::on_Tags_pushButton_OpenTagsFile_clicked()
    {
        tagsFilePath = collectionFolder + "/" + "tags.csv";
        QDesktopServices::openUrl(QUrl::fromLocalFile(tagsFilePath));
    }
    //----------------------------------------------------------------------
    void MainWindow::on_Tags_listView_ExistingTags_clicked(const QModelIndex &index)
    {
        selectedTagListName = ui->Tags_listView_ExistingTags->model()->index(index.row(), 0, QModelIndex()).data().toString();
        ui->Tags_lineEdit_TagName->setText(selectedTagListName);
        //loadTagsTableToModel();
        loadTagsTableToTagsAndFolderListModel();
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

//Methods-------------------------------------------------------------------

    void MainWindow::reloadTagsData()
    {
        selectedTagListName="";
        if(databaseMode=="Memory")
                loadTagsToTable();
        loadTagsTableToExistingTagsModel();
        loadTagsTableToTagsAndFolderListModel();
    }
    //----------------------------------------------------------------------
    void MainWindow::loadFileSystemTags(QString newTagFolderPath)
    {
        //Load file system for the treeview

         // Creates a new model
            fileSystemModel = new QFileSystemModel(this);

         // Set filter to show only directories
            fileSystemModel->setFilter(QDir::NoDotAndDotDot | QDir::AllDirs);

         // QFileSystemModel requires root path
            QString rootPath ="/";
            fileSystemModel->setRootPath(rootPath);
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
    void MainWindow::loadTagsToTable()
    {
        //Clear table
        QSqlQuery queryDelete;
        QString queryDeleteSQL = QLatin1String(R"(
                                    DELETE FROM tag
                                )");
        queryDelete.prepare(queryDeleteSQL);
        queryDelete.exec();

        //Prepare for using the csv file storing tag data
        tagsFilePath = collectionFolder + "/" + "tags.csv";

        QFile tagFile(tagsFilePath);
        if(!tagFile.open(QIODevice::ReadOnly)) {
                return;
        }

        // Loop through each line of the csv file, and load data in table
        QTextStream textStream(&tagFile);
        QStringList fieldList;
        QSqlQuery queryInsert;
        while (true)
        {
            QString line = textStream.readLine();
            if (line.isNull())
                    break;
            else{
                //Split the line with tabulation into a list
                fieldList = line.split('\t');

                //Check data
                if (fieldList.count()!=2)
                        return;

                //Append data to the table
                QString queryInsertSQL = QLatin1String(R"(
                                            INSERT INTO tag(
                                                            name,
                                                            path,
                                                            type,
                                                            date_time
                                            )
                                            VALUES(
                                                            :name,
                                                            :path,
                                                            :type,
                                                            :date_time
                                            )
                )");
                queryInsert.prepare(queryInsertSQL);
                queryInsert.bindValue(":name",fieldList[1]);
                queryInsert.bindValue(":path",fieldList[0]);
                queryInsert.bindValue(":type","not_stored_in_file_yet");    //fieldList[2]
                queryInsert.bindValue(":date_time","not_stored_in_file_yet");//fieldList[3]
                queryInsert.exec();
             }
        }
    }
    //----------------------------------------------------------------------
    void MainWindow::loadTagsTableToExistingTagsModel()
    {
        //Set up temporary lists
        QList<QString> tFolderPaths;
        QList<QString> tTagNames;

		//Get full list of tags
		QSqlQuery query;
		QString querySQL = QLatin1String(R"(
                                    SELECT path, name
                                    FROM tag
                                    )");
        query.prepare(querySQL);
		query.exec();

		//Populate lists
		while(query.next()){
                        tFolderPaths << query.value(0).toString();
                        tTagNames    << query.value(1).toString();
		}

        // Create model
        Tag *tagModel = new Tag(this);

        // Populate model with data
        tagModel->populateTagData(tFolderPaths, tTagNames);

        QSortFilterProxyModel *proxyStorageModel = new QSortFilterProxyModel(this);
        proxyStorageModel->setSourceModel(tagModel);

        QList<QString> tTagUniqueNames;
        tTagUniqueNames = tTagNames;
        tTagUniqueNames.removeDuplicates();

        //Load list of catalogs in which files where found
        tagListModel = new QStringListModel(this);
        tagListModel->setStringList(tTagUniqueNames);
        ui->Tags_listView_ExistingTags->setModel(tagListModel);
        ui->Search_comboBox_Tags->setModel(tagListModel);
    }
    //----------------------------------------------------------------------

    void MainWindow::loadTagsTableToTagsAndFolderListModel()
    {
        //Set up temporary lists
        QList<QString> tFolderPaths;
        QList<QString> tTagNames;

        //Get full list of tags
        QSqlQuery query;
        QString querySQL = QLatin1String(R"(
                                    SELECT path, name
                                    FROM tag
                                    )");
        if(selectedTagListName!=""){
            querySQL = querySQL + " WHERE name=:name";
        }
        query.prepare(querySQL);

        query.bindValue(":name",selectedTagListName);

        query.exec();

        //Populate lists
        while(query.next()){
                        tFolderPaths << query.value(0).toString();
                        tTagNames    << query.value(1).toString();
        }

        // Create model
        Tag *tagModel = new Tag(this);

        // Populate model with data
        tagModel->populateTagData(tFolderPaths, tTagNames);

        QSortFilterProxyModel *proxyStorageModel = new QSortFilterProxyModel(this);
        proxyStorageModel->setSourceModel(tagModel);

        // Connect model to tree/table view
        ui->Tags_treeView_FolderTags->setModel(proxyStorageModel);
        ui->Tags_treeView_FolderTags->QTreeView::sortByColumn(1,Qt::AscendingOrder);
        ui->Tags_treeView_FolderTags->QTreeView::sortByColumn(0,Qt::AscendingOrder);
        ui->Tags_treeView_FolderTags->header()->setSectionResizeMode(QHeaderView::Interactive);
        ui->Tags_treeView_FolderTags->header()->resizeSection(0, 350); //Folder
    }
    //----------------------------------------------------------------------
