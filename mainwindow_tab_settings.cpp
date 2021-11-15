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
// Purpose:     methods for the Fitlers and Settings panel
// Description:
// Author:      Stephane Couturier
// Version:     1.00
/////////////////////////////////////////////////////////////////////////////
*/

#include "mainwindow.h"
#include "ui_mainwindow.h"
#include "filesview.h"//test

#include <QSettings>
#include <QDesktopServices>
#include <QFileDialog>

//ALL/GLOBAL -----------------------------------------------------------------

    void MainWindow::on_tabWidget_currentChanged(int index)
    {
        selectedTab = index;
        QSettings settings(settingsFilePath, QSettings:: IniFormat);
        settings.setValue("Settings/selectedTab", ui->tabWidget->currentIndex());
    }
    //----------------------------------------------------------------------
    void MainWindow::on_Global_tabWidget_currentChanged(int index)
    {
        int selectedTabGlobal = index;
        QSettings settings(settingsFilePath, QSettings:: IniFormat);
        settings.setValue("Settings/selectedTabGlobal", selectedTabGlobal);
    }
    //----------------------------------------------------------------------
    void MainWindow::on_Global_pushButton_ShowHideGlobal_clicked()
    {
        QString visible = ui->Global_pushButton_ShowHideGlobal->text();

        if ( visible == "<<"){ //Hide
                ui->Global_pushButton_ShowHideGlobal->setText(">>");
                ui->Global_tabWidget->setHidden(true);
                ui->Global_label_Global->setHidden(true);

                QSettings settings(settingsFilePath, QSettings:: IniFormat);
                settings.setValue("Settings/ShowHideGlobal", ui->Global_pushButton_ShowHideGlobal->text());
        }
        else{ //Show
                ui->Global_pushButton_ShowHideGlobal->setText("<<");
                ui->Global_tabWidget->setHidden(false);
                ui->Global_label_Global->setHidden(false);

                QSettings settings(settingsFilePath, QSettings:: IniFormat);
                settings.setValue("Settings/ShowHideGlobal", ui->Global_pushButton_ShowHideGlobal->text());
        }

    }

//FILTERS -------------------------------------------------------------
    void MainWindow::on_Filters_pushButton_ResetGlobal_clicked()
    {
            ui->Filters_comboBox_SelectLocation->setCurrentText(tr("All"));
            ui->Filters_comboBox_SelectStorage->setCurrentText(tr("All"));
            ui->Filters_comboBox_SelectCatalog->setCurrentText(tr("All"));
    }
    //----------------------------------------------------------------------
    void MainWindow::on_Filters_comboBox_SelectLocation_currentIndexChanged(const QString &selectedLocation)
    {
        //save selection in settings file;
        QSettings settings(settingsFilePath, QSettings:: IniFormat);
        settings.setValue("LastSearch/SelectedSearchLocation", selectedLocation);

        //Load matching Storage
        refreshStorageSelectionList(selectedLocation);
        selectedSearchLocation = selectedLocation;

        //Load matching Catalog
        loadCatalogsToModel();
        loadStorageTableToModel();
        refreshStorageStatistics();
    }
    //----------------------------------------------------------------------
    void MainWindow::on_Filters_comboBox_SelectStorage_currentIndexChanged(const QString &selectedStorage)
    {
        //save selection in settings file;
        QSettings settings(settingsFilePath, QSettings:: IniFormat);
        settings.setValue("LastSearch/SelectedSearchStorage", selectedStorage);

        //Get selected Location
        QString selectedLocation = ui->Filters_comboBox_SelectLocation->currentText();

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
        QSettings settings(settingsFilePath, QSettings:: IniFormat);
        settings.setValue("LastSearch/SelectedSearchCatalog", selectedCatalog);

        selectedSearchCatalog = selectedCatalog;

    }

//SETTINGS / Collection ----------------------------------------------------

    void MainWindow::on_Collection_pushButton_SelectFolder_clicked()
    {
        //Open a dialog for the user to select the directory of the collection where catalog files are stored.
        QString dir = QFileDialog::getExistingDirectory(this, tr("Select the directory for this collection"),
                                                        collectionFolder,
                                                        QFileDialog::ShowDirsOnly
                                                        | QFileDialog::DontResolveSymlinks);

        //Unless the selection was cancelled, set the new collection folder, and refresh the list of catalogs
        if ( dir !=""){

            collectionFolder = dir;

            //set the new path in Colletion tab
            ui->Collection_lineEdit_CollectionFolder->setText(collectionFolder);

            //redefine the path of the Storage file
            storageFilePath = collectionFolder + "/" + "storage.csv";

            //save Settings for the new collection folder value;
            saveSettings();

            //load the collection for this new folder;
            loadCollection();

        }

        //Reset selected catalog values (to avoid actions on the last selected one)
        selectedCatalogFile="";
        selectedCatalogName="";
        selectedCatalogPath="";
    }
    //----------------------------------------------------------------------
    void MainWindow::on_Collection_lineEdit_CollectionFolder_returnPressed()
    {
        loadCollection();
    }
    //----------------------------------------------------------------------
    void MainWindow::on_Collection_pushButton_OpenFolder_clicked()
    {
        //Open the selected collection folder
        QDesktopServices::openUrl(QUrl::fromLocalFile(collectionFolder));
    }
    //----------------------------------------------------------------------
    void MainWindow::on_Collection_pushButton_Reload_clicked()
    {
        loadCollection();
    }
    //----------------------------------------------------------------------
    void MainWindow::loadCollection()
    {
        //Load Storage list and refresh their statistics
            loadStorageFileToTable();
            loadStorageTableToModel();
            refreshStorageStatistics();

       //load Catalog list, Location list, Storage list, Statistics
            loadCatalogFilesToTable();

            loadCatalogsToModel();

            refreshLocationSelectionList();
            refreshStorageSelectionList(tr("All"));
            refreshCatalogSelectionList(tr("All"), tr("All"));

            //restore Search catalog selection
            //ui->Filters_comboBox_SelectLocation->setCurrentText(selectedSearchLocation);
            //ui->Filters_comboBox_SelectStorage->setCurrentText(selectedSearchStorage);
            //ui->Filters_comboBox_SelectCatalog->setCurrentText(selectedSearchCatalog);

            statisticsFilePath = collectionFolder + "/" + "statistics.csv";
            loadStatisticsData();
            loadStatisticsChart();

            //hide buttons to force user to select a catalog before allowing any action.
            hideCatalogButtons();
    }

//SETTINGS / Settings ------------------------------------------------------

    void MainWindow::on_Settings_checkBox_SaveRecordWhenUpdate_stateChanged()
    {
        QSettings settings(settingsFilePath, QSettings:: IniFormat);
        settings.setValue("Settings/AutoSaveRecordWhenUpdate", ui->Settings_checkBox_SaveRecordWhenUpdate->isChecked());
    }
    //----------------------------------------------------------------------
    void MainWindow::on_Settings_checkBox_KeepOneBackUp_stateChanged()
    {
        QSettings settings(settingsFilePath, QSettings:: IniFormat);
        settings.setValue("Settings/KeepOneBackUp", ui->Settings_checkBox_KeepOneBackUp->isChecked());
    }
    //----------------------------------------------------------------------
    void MainWindow::on_Settings_comboBox_Theme_currentIndexChanged(int index)
    {
        QSettings settings(settingsFilePath, QSettings:: IniFormat);
        settings.setValue("Settings/Theme", index);
    }
    //----------------------------------------------------------------------
    void MainWindow::on_Settings_checkBox_CheckVersion_stateChanged()
    {
        QSettings settings(settingsFilePath, QSettings:: IniFormat);
        settings.setValue("Settings/CheckVersion", ui->Settings_checkBox_CheckVersion->isChecked());
    }
    //----------------------------------------------------------------------

//SETTINGS / About ---------------------------------------------------------

    void MainWindow::on_Settings_pushButton_Wiki_clicked()
    {
        QDesktopServices::openUrl(QUrl("https://github.com/StephaneCouturier/Katalog/wiki"));
    }
    //----------------------------------------------------------------------
    void MainWindow::on_Settings_pushButton_ReleaseNotes_clicked()
    {
        QDesktopServices::openUrl(QUrl("https://github.com/StephaneCouturier/Katalog/releases"));
    }
    //----------------------------------------------------------------------
    void MainWindow::on_Settings_comboBox_Language_currentTextChanged(const QString &selectedLanguage)
    {
        QSettings settings(settingsFilePath, QSettings:: IniFormat);
        settings.setValue("Settings/Language", selectedLanguage);
    }
    //----------------------------------------------------------------------
