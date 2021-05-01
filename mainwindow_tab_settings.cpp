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
//#include <KMessageBox>
//#include <KLocalizedString>

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

    //Load matching Catalog
    //refreshCatalogSelectionList();

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

    //Load matching Catalog
    //refreshCatalogSelectionList();
    selectedSearchStorage = selectedStorage;

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

