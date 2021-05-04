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

        //get path of the file to import
        QString sourceFilePath = QFileDialog::getOpenFileName(this, tr("Select the csv file to be imported"),
                                                        homePath
                                                        );
        //Stop if not path is selected
        if ( sourceFilePath=="" ) return;

    //Define file and prepare stream
        QFile sourceFile(sourceFilePath);

        //QFileInfo sourceFileInfo(sourceFilePath);

        //QMessageBox::information(this,"Katalog","File: \n" + sourceFileInfo.baseName() + "\n"+sourceFileInfo.filePath());

    //Open file and prepare stream
        if(!sourceFile.open(QIODevice::ReadOnly)) {
            QMessageBox::information(this,"Katalog","No catalog found.");
            return;
        }

        QTextStream textStream(&sourceFile);

    //Process and check headers
        QString line = textStream.readLine();

        //QMessageBox::information(this,"Kotation",line);

        //split the line into a fieldlist
        QStringList headerList = line.split('\t');


        //check this is the right source format
        if (line.left(6)!="Volume"){
            QMessageBox::warning(this,"Kotation","A file was found, but could not be loaded.\n");
            return;
        }
        else {
            //int headerFieldNumber = headerList.length();
            //QMessageBox::information(this,"Kotation","ok to import. \n number of fields: \n " + QString::number(headerFieldNumber));
        }

    //prepare database

        //clear database
        QSqlQuery deleteQuery0;
        deleteQuery0.exec("DELETE FROM file");

        //prepare query
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


    //loop through each line and load values to the database
    while (true)
    {

        line = textStream.readLine();
        QMessageBox::information(this,"Kotation","line: stream \n " + line);

        QStringList fieldList = line.split("\t");
        //QMessageBox::information(this,"Kotation","fields: \n " + fieldList[0] +"\n "+ fieldList[1] +"\n "+ fieldList[3] );
//        if (fieldList.count() <2){
//            QMessageBox::information(this,"Kotation","error: \n " + line);
//            return;
//        }
        if (line !=""){
            //Append data to the database
            insertQuery.bindValue(":fileName", fieldList[2].replace("\"",""));
            insertQuery.bindValue(":filePath", fieldList[1].replace("\"",""));
            insertQuery.bindValue(":fileSize", fieldList[3].toLongLong());
            insertQuery.bindValue(":fileDateUpdated", fieldList[5]);
            insertQuery.bindValue(":fileCatalog", fieldList[0]);
            insertQuery.exec();

        }
       //else
          //return;

//        return;
    }




    //Write to target file for Katalog

        // Stream the list to the file
        newCatalogName = "imported_from_VVV";
        QFile fileOut( collectionFolder +"/"+ newCatalogName + ".idx" );

        QTextStream out(&fileOut);

        if(fileOut.open(QIODevice::WriteOnly | QIODevice::Truncate)) {

        //append catalog definition("<catalogName>"+newCatalogName);
        out  << "<catalogSourcePath>undefined" << "\n"
             << "<catalogFileCount>"           << "\n"
             << "<catalogTotalFileSize>"       << "\n"
             << "<catalogIncludeHidden>"       << "\n"
             << "<catalogFileType>"            << "\n"
             << "<catalogStorage>"             << "\n"
             << "<catalogIncludeSymblinks>"    << "\n";

        // write data
        QSqlQuery query;
        QString querySQL = QLatin1String(R"(
                                        SELECT filePath || '/' || fileName,fileSize,fileDateUpdated  FROM file
                                        )");
        query.prepare(querySQL);
        query.exec();

        //    Iterate the result
        while (query.next()) {

            const QSqlRecord record = query.record();
            for (int i=0, recCount = record.count() ; i<recCount ; ++i){
                if (i>0)
                out << '\t';
                out << record.value(i).toString();
            }

             out << '\n';
             QMessageBox::information(this,"Katalog","test record. \n");

        }



            //out << textData;
    //    Close the file
            //storageFile.close();
        }

        QMessageBox::information(this,"Katalog","File converted. See in Catalogs");
        fileOut.close();


    //close file
    sourceFile.close();

    loadCollection();

}
