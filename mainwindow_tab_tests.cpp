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
// File Name:   mainwindow_tab_tests.cpp
// Purpose:
// Description:
// Author:      Stephane Couturier
// Modified by: Stephane Couturier
// Created:     2020-07-11
// Version:     0.1
/////////////////////////////////////////////////////////////////////////////
*/

#include "mainwindow.h"
#include "ui_mainwindow.h"
#include "catalog.h"

void MainWindow::LoadCatalogsToModel()
{
    //Load Catalog List To Model

    QList<QString> cNames;
    QList<QString> cSourcePaths;
    QList<QString> cDateUpdates;
    QList<QString> cNums;

    QStringList fileTypes;
    fileTypes << "*.idx";
    //Iterate in the directory to create a list of files and sort it
    //list the file names only
    QDirIterator iterator(collectionFolder, fileTypes, QDir::Files, QDirIterator::Subdirectories);
    while (iterator.hasNext()){
        //catalogList << (iterator.next());

        //LoadCatalogInfo(file);

        // Get infos stored in the file

        QFile catalogFile(iterator.next());
        if(!catalogFile.open(QIODevice::ReadOnly)) {
            KMessageBox::information(this,"No catalog found.");
        }
        //KMessageBox::information(this,"iterator"+iterator.fileName());

        QTextStream textStream(&catalogFile);

        while (true)
        {
            QString line = textStream.readLine();
            if (line.left(13)=="<catalogName>"){
                QString catalogName = line.right(line.size() - line.lastIndexOf(">") - 1);
                cNames.append(catalogName);
            }
            else if (line.left(18)=="<catalogFileCount>"){
                QString catalogFileCount = line.right(line.size() - line.lastIndexOf(">") - 1);
                cNums.append(catalogFileCount);
            }
            else if (line.left(19)=="<catalogSourcePath>"){
                QString catalogSourcePath = line.right(line.size() - line.lastIndexOf(">") - 1);
                cSourcePaths.append(catalogSourcePath);
            }
            else
                break;
        }

        // Get infos about the file itself
        QFileInfo catalogFileInfo(catalogFile);
        //cSourcePaths.append(catalogFileInfo.path());
        //cNames.append(catalogFileInfo.fileName());
        cDateUpdates.append(catalogFileInfo.lastModified().toString(Qt::ISODate));
    }

    // Create model
    Catalog *catalog = new Catalog(this);

    // Populate model with data
    catalog->populateData(cNames, cSourcePaths, cDateUpdates, cNums);

    // Connect model to table view
    ui->TV_Catalogs->setModel(catalog);
    ui->TV_Catalogs->horizontalHeader()->setStretchLastSection(true);

    ui->treeView_2->setModel(catalog);


}
//----------------------------------------------------------------------
void MainWindow::on_TV_Catalogs_clicked(const QModelIndex &index)
{ //Test the click on a line

    if (index.isValid()) {
        int currentRow = index.row();
        int currentCol = index.column();
        //ui->treeView_2->index(currentRow,0);
        //QString cpath = ui->treeView_2->model()->index(index.row(),0)).data().toString();
        //QString cpath = index(index.row(),0)).data().toString();
        //QString cellText = index.data().toString();

    //KMessageBox::information(this,"test:\n"+cellText+"\n"+cpath);
    /*QStringListModel* listModel= qobject_cast<QStringListModel*>(ui->TV_Catalogs->model());
    selectedCatalog = listModel->stringList().at(index.row());
    KMessageBox::information(this,"test:\n"+selectedCatalog);*/
    }
}
//----------------------------------------------------------------------

/*
void MainWindow::on_treeView_2_clicked(const QModelIndex &index)
{
    //Test the click on a line
    QStringListModel* listModel= qobject_cast<QStringListModel*>(ui->treeView_2->model());
    selectedCatalog = listModel->stringList().at(index.row());
    KMessageBox::information(this,"test:\n"+selectedCatalog);
}
*/

void MainWindow::on_treeView_2_activated(const QModelIndex &index)
{
    if (index.isValid()) {
            QString cellText = index.data().toString();

    KMessageBox::information(this,"test:\n"+cellText);
    }
}
