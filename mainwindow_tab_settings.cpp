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
// File Name:   mainwindow_tab_settings.cpp
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
#include "filesview.h"//test

#include <QSettings>
#include <QDesktopServices>
#include <QFileDialog>

//Tab: ALL -----------------------------------------------------------------------------

void MainWindow::on_tabWidget_currentChanged(int index)
{
    selectedTab = index;
    QSettings settings(settingsFile, QSettings:: IniFormat);
    settings.setValue("Settings/selectedTab", ui->tabWidget->currentIndex());
}

void MainWindow::on_Global_tabWidget_currentChanged(int index)
{
    int selectedTabGlobal = index;
    QSettings settings(settingsFile, QSettings:: IniFormat);
    settings.setValue("Settings/selectedTabGlobal", selectedTabGlobal);
}

void MainWindow::on_Global_pushButton_ShowHideGlobal_clicked()
{
    QString visible = ui->Global_pushButton_ShowHideGlobal->text();

    if ( visible == "<<"){ //Hide
            ui->Global_pushButton_ShowHideGlobal->setText(">>");
            ui->Global_tabWidget->setHidden(true);
            ui->Global_label_Global->setHidden(true);

            QSettings settings(settingsFile, QSettings:: IniFormat);
            settings.setValue("Settings/ShowHideGlobal", ui->Global_pushButton_ShowHideGlobal->text());
    }
    else{ //Show
            ui->Global_pushButton_ShowHideGlobal->setText("<<");
            ui->Global_tabWidget->setHidden(false);
            ui->Global_label_Global->setHidden(false);

            QSettings settings(settingsFile, QSettings:: IniFormat);
            settings.setValue("Settings/ShowHideGlobal", ui->Global_pushButton_ShowHideGlobal->text());
    }

}

//Tab: FILTERS -----------------------------------------------------------------------------
void MainWindow::on_Filters_pushButton_ResetGlobal_clicked()
{
        ui->Filters_comboBox_SelectLocation->setCurrentText("All");
        ui->Filters_comboBox_SelectStorage->setCurrentText("All");
        ui->Filters_comboBox_SelectCatalog->setCurrentText("All");
}
//----------------------------------------------------------------------
void MainWindow::on_Filters_comboBox_SelectLocation_currentIndexChanged(const QString &selectedLocation)
{
    //save selection in settings file;
    QSettings settings(settingsFile, QSettings:: IniFormat);
    settings.setValue("LastSearch/SelectedSearchLocation", selectedLocation);

    //Load matching Storage
    refreshStorageSelectionList(selectedLocation);

    selectedSearchLocation = selectedLocation;

    //Load matching Catalog
    loadCatalogsToModel();
    loadStorageTableToModel();

}
//----------------------------------------------------------------------
void MainWindow::on_Filters_comboBox_SelectStorage_currentIndexChanged(const QString &selectedStorage)
{
    //save selection in settings file;
    QSettings settings(settingsFile, QSettings:: IniFormat);
    settings.setValue("LastSearch/SelectedSearchStorage", selectedStorage);

    //Get selected Location and Storage
    QString selectedLocation = ui->Filters_comboBox_SelectLocation->currentText();
    //QString selectedStorage  = ui->Filters_comboBox_SelectStorage->currentText();

    //Load matching Storage
    refreshCatalogSelectionList(selectedLocation, selectedStorage);

    selectedSearchStorage = selectedStorage;

    //Load matching Catalog
    loadCatalogsToModel();
}
//----------------------------------------------------------------------
void MainWindow::on_Filters_comboBox_SelectCatalog_currentIndexChanged(const QString &selectedCatalog)
{
    //save selection in settings file;
    QSettings settings(settingsFile, QSettings:: IniFormat);
    settings.setValue("LastSearch/SelectedSearchCatalog", selectedCatalog);

}

//Tab: SETTINGS -----------------------------------------------------------------------------

void MainWindow::on_Settings_checkBox_SaveRecordWhenUpdate_stateChanged()
{
    QSettings settings(settingsFile, QSettings:: IniFormat);
    settings.setValue("Settings/AutoSaveRecordWhenUpdate", ui->Settings_checkBox_SaveRecordWhenUpdate->isChecked());
}

void MainWindow::on_Settings_checkBox_KeepOneBackUp_stateChanged()
{
    QSettings settings(settingsFile, QSettings:: IniFormat);
    settings.setValue("Settings/KeepOneBackUp", ui->Settings_checkBox_KeepOneBackUp->isChecked());
}

void MainWindow::on_Settings_comboBox_Theme_currentTextChanged()
{
    QSettings settings(settingsFile, QSettings:: IniFormat);
    settings.setValue("Settings/UseDefaultDesktopTheme", ui->Settings_comboBox_Theme->currentText());
}

void MainWindow::on_Settings_pushButton_Wiki_clicked()
{
    QDesktopServices::openUrl(QUrl("https://github.com/StephaneCouturier/Katalog/wiki"));
}

void MainWindow::on_Settings_pushButton_ReleaseNotes_clicked()
{
    QDesktopServices::openUrl(QUrl("https://github.com/StephaneCouturier/Katalog/releases/tag/v0.16"));
}


//Tab: IMPORT -----------------------------------------------------------------------------

void MainWindow::on_Settings_pushButton_ImportVVV_clicked()
{
    importFromVVV();
}

void MainWindow::importFromVVV()
{
    //Select file

        //Get user home path
        QStringList standardsPaths = QStandardPaths::standardLocations(QStandardPaths::HomeLocation);
        QString homePath = standardsPaths[0];

        //Get path of the file to import
        QString sourceFilePath = QFileDialog::getOpenFileName(this, tr("Select the csv file to be imported"),homePath);

        //Stop if no path is selected
        if ( sourceFilePath=="" ) return;

        //Define file
        QFile sourceFile(sourceFilePath);

    //Open the source file and load all data into the database

        // Start animation while cataloging
        QApplication::setOverrideCursor(Qt::WaitCursor);

        //clear database
            QSqlQuery deleteQuery;
            deleteQuery.exec("DELETE FROM file");

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

        //prepare file and stream

            if(!sourceFile.open(QIODevice::ReadOnly)) {
                QMessageBox::information(this,"Katalog","No catalog found.");
                return;
            }

            QTextStream textStream(&sourceFile);
            QString     line;

        //Process and check Headers line
            line = textStream.readLine();

            //Split the line into a fieldlist
            QStringList headerList = line.split('\t');

            //Check this is the right source format
            if (line.left(6)!="Volume"){
                QMessageBox::warning(this,"Kotation","A file was found, but could not be loaded.\n");
                return;
            }
            //else {
                //int headerFieldNumber = headerList.length();
                //QMessageBox::information(this,"Kotation","ok to import. \n number of fields: \n " + QString::number(headerFieldNumber));
            //}

        //load all files to the database

                while (true)
                {
                    //Read the newt line
                    line = textStream.readLine();

                    if (line !=""){
                        QStringList fieldList = line.split("\t");
                        if ( fieldList.count()==7 ){

                            //Append data to the database
                            insertQuery.bindValue(":fileName", fieldList[2].replace("\"",""));
                            insertQuery.bindValue(":filePath", fieldList[1].replace("\"",""));
                            insertQuery.bindValue(":fileSize", fieldList[3].toLongLong());
                            insertQuery.bindValue(":fileDateUpdated", fieldList[5]);
                            insertQuery.bindValue(":fileCatalog", fieldList[0].replace("\"",""));
                            insertQuery.exec();
                        }
                    }
                   else
                        break;


            }

        //close source file
        sourceFile.close();


////TEST LOAD TO EXPLORE----------

        // Load all files and create model
        QString selectSQL = QLatin1String(R"(
                            SELECT  fileName AS Name,
                                    fileSize AS Size,
                                    fileDateUpdated AS Date,
                                    fileCatalog AS Catalog,
                                    filePath AS Path
                            FROM file
                                        )");
        QSqlQuery loadCatalogQuery;
        loadCatalogQuery.prepare(selectSQL);
        loadCatalogQuery.exec();

        QSqlQueryModel *loadCatalogQueryModel = new QSqlQueryModel;
        loadCatalogQueryModel->setQuery(loadCatalogQuery);

        FilesView *proxyModel2 = new FilesView(this);
        proxyModel2->setSourceModel(loadCatalogQueryModel);

        // Connect model to tree/table view
        ui->Explore_treeView_FileList->setModel(proxyModel2);
        ui->Explore_treeView_FileList->QTreeView::sortByColumn(0,Qt::AscendingOrder);
        ui->Explore_treeView_FileList->header()->setSectionResizeMode(QHeaderView::Interactive);
        ui->Explore_treeView_FileList->header()->resizeSection(0, 600); //Name
        ui->Explore_treeView_FileList->header()->resizeSection(1, 110); //Size
        ui->Explore_treeView_FileList->header()->resizeSection(2, 140); //Date
        ui->Explore_treeView_FileList->header()->resizeSection(3, 400); //Path

////END - TEST ----------


    //Stream the list of files out to the target calog file(s)

        //Get a list of the source catalogs
            QString listCatalogSQL = QLatin1String(R"(
                                SELECT DISTINCT fileCatalog
                                FROM file
                                            )");
            QSqlQuery listCatalogQuery;
            listCatalogQuery.prepare(listCatalogSQL);
            listCatalogQuery.exec();

        //Iterate in the list to generate a catalog file for each catalog
        while (listCatalogQuery.next()){

            //Get catalog name
                QString formerCatalogName = listCatalogQuery.value(0).toString();

            //Generate a name for the file itself, without specical characters
                newCatalogName = formerCatalogName;
                newCatalogName.replace("/","_");
                newCatalogName.replace("\\","_");

            //Prepare the catalog file path
                QFile fileOut( collectionFolder +"/"+ newCatalogName + ".idx" );

            //Get statistics of the files for the list
                QString listCatalogSQL = QLatin1String(R"(
                                    SELECT COUNT(*), SUM(fileSize)
                                    FROM file
                                    WHERE fileCatalog =:fileCatalog
                                                )");
                QSqlQuery listCatalogQuery;
                listCatalogQuery.prepare(listCatalogSQL);
                listCatalogQuery.bindValue(":fileCatalog",formerCatalogName);
                listCatalogQuery.exec();
                listCatalogQuery.next();

                qint64 totalFiles = listCatalogQuery.value(0).toLongLong();
                qint64 totalSize  = listCatalogQuery.value(1).toLongLong();

            //Prepare the stream and file headers
                QTextStream out(&fileOut);
                if(fileOut.open(QIODevice::WriteOnly | QIODevice::Truncate)) {

                    //append catalog definition("<catalogName>"+newCatalogName);
                    out  << "<catalogSourcePath>imported"  << "\n"
                         << "<catalogFileCount>"+QString::number(totalFiles)           << "\n"
                         << "<catalogTotalFileSize>"+QString::number(totalSize)       << "\n"
                         << "<catalogIncludeHidden>"       << "\n"
                         << "<catalogFileType>"            << "\n"
                         << "<catalogStorage>"             << "\n"
                         << "<catalogIncludeSymblinks>"    << "\n";
                }



            //Get the list of file to add
            QString listFilesSQL = QLatin1String(R"(
                                SELECT *
                                FROM file
                                WHERE fileCatalog =:fileCatalog
                                            )");
            QSqlQuery listFilesQuery;
            listFilesQuery.prepare(listFilesSQL);
            listFilesQuery.bindValue(":fileCatalog",formerCatalogName);
            listFilesQuery.exec();

            //Write the results in the file
            while (listFilesQuery.next()) {

//                const QSqlRecord record = listFilesQuery.record();
//                for (int i=0, recCount = record.count() ; i<recCount ; ++i){
//                    if (i>0)
//                    out << '\t';
//                    out << record.value(i).toString();
//                }

                    out << listFilesQuery.value(1).toString() + "/" + listFilesQuery.value(0).toString();
                    out << '\t';
                    out << listFilesQuery.value(2).toString();
                    out << '\t';
                    out << listFilesQuery.value(3).toString();
                    out << '\n';

            }



        }

/*


                //Reopen the source file, now to get the files list itself
                if(!sourceFile.open(QIODevice::ReadOnly)) {
                    QMessageBox::information(this,"Katalog","No catalog found.");
                    return;
                }
                //Prepare the stream
                QTextStream textStream2(&sourceFile);


                if(fileOut.open(QIODevice::WriteOnly | QIODevice::Append)) {

                    //Read the first line to skip headers
                        line = textStream2.readLine();

                    while (true)
                    {
                        //Read the newt line
                        line = textStream2.readLine();

                        if (line !=""){
                            QStringList fieldList = line.split("\t");
                            if ( fieldList.count()==7 and fieldList[0]==formerCatalogName ){
                                out << fieldList[1].replace("\"","")+"/"+fieldList[2].replace("\"","") << '\t';
                                out << fieldList[3] << '\t';
                                out << fieldList[5] ;
                                out << '\n';
                            }
                        }
                       else
                            break;

                    }
                }
            //close files
            sourceFile.close();

            fileOut.close();

    ui->tabWidget->setCurrentIndex(1);
*/
    //QMessageBox::information(this,"Katalog","File converted.");

        //Stop animation
        QApplication::restoreOverrideCursor();

    loadCollection();

}
