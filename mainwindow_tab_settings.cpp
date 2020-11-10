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

//#include <KMessageBox>
//#include <KLocalizedString>

//Tab: SETTINGS -----------------------------------------------------------------------------

void MainWindow::on_Settings_ChBx_SaveRecordWhenUpdate_stateChanged()
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



//Tab: SETTINGS_DEV -----------------------------------------------------------------------------

    void MainWindow::FileTypesEditor()
    {
        //Create model
        listModel = new QStringListModel(this);

        // Make data
        if (fileType_current.empty()==true)
            fileType_current = fileType_Video;
        //List << "PNG" << "JPG" << "BMP";

        // Populate our model
        listModel->setStringList(fileType_current);

        // Glue model and view together
        ui->listView_3->setModel(listModel);
        ui->comboBox->setModel(listModel);

        // Add additional feature so that
        // we can manually modify the data in ListView
        // It may be triggered by hitting any key or double-click etc.
        ui->listView_3->
                setEditTriggers(QAbstractItemView::AnyKeyPressed |
                                QAbstractItemView::DoubleClicked);
    }
    //----------------------------------------------------------------------
    void MainWindow::on_pushButton_8_clicked()
    {
    // Add button clicked
        // Adding at the end

        // Get the position
        int row = listModel->rowCount();

        // Enable add one or more rows
        listModel->insertRows(row,1);

        // Get the row for Edit mode
        QModelIndex index = listModel->index(row);

        // Enable item selection and put it edit mode
        ui->listView_3->setCurrentIndex(index);
        ui->listView_3->edit(index);

        fileType_current = listModel->stringList();
    }
    //----------------------------------------------------------------------
    void MainWindow::on_pushButton_7_clicked()
    {
    // Insert button clicked

        // Get the position
        int row = ui->listView_3->currentIndex().row();

        // Enable add one or more rows
        listModel->insertRows(row,1);

        // Get row for Edit mode
        QModelIndex index = listModel->index(row);

        // Enable item selection and put it edit mode
        ui->listView_3->setCurrentIndex(index);
        ui->listView_3->edit(index);

        fileType_current = listModel->stringList();
    }
    //----------------------------------------------------------------------
    void MainWindow::on_pushButton_9_clicked()
    {
    // Delete button clicked
        // For delete operation,
        // we're dealing with a Model not a View
        // Get the position
        listModel->removeRows(ui->listView_3->currentIndex().row(),1);

        fileType_current = listModel->stringList();
    }

    //----------------------------------------------------------------------

