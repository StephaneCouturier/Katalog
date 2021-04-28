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

void MainWindow::on_tabWidget_currentChanged(int index)
{
    selectedTab = index;
    QSettings settings(settingsFile, QSettings:: IniFormat);
    settings.setValue("Settings/selectedTab", ui->tabWidget->currentIndex());
}
