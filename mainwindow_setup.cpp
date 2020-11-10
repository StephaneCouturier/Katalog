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
// File Name:   mainwindow_setup.cpp
// Purpose:
// Description:
// Author:      Stephane Couturier
// Modified by: Stephane Couturier
// Created:     2020-07-11
// Version:     0.8
/////////////////////////////////////////////////////////////////////////////
*/

#include "mainwindow.h"
#include "ui_mainwindow.h"

#include <QFileDialog>
#include <QTextStream>
#include <QSaveFile>
#include <QSettings>

//#include <KActionCollection>
//#include <KIO/Job>
//#include <KMessageBox>
//#include <KLocalizedString>

//Settings -----------------------------------------------------------------
    void MainWindow::setupFileContextMenu(){
        ui->Search_treeView_FilesFound->setContextMenuPolicy(Qt::CustomContextMenu);
        connect( ui->Search_treeView_FilesFound, SIGNAL(customContextMenuRequested(const QPoint&)),
            this, SLOT(ShowContextMenu(const QPoint&)));

        ui->Explore_treeView_FileList->setContextMenuPolicy(Qt::CustomContextMenu);
        connect(ui->Explore_treeView_FileList, SIGNAL(customContextMenuRequested(const QPoint&)),
            this, SLOT(ShowContextMenu(const QPoint&)));
    }
    //----------------------------------------------------------------------
    void MainWindow::loadSettings()
    {
        QSettings settings(settingsFile, QSettings:: IniFormat);
        collectionFolder = settings.value("LastCollectionFolder").toString();
        if(collectionFolder == ""){
               collectionFolder = QApplication::applicationDirPath();
        }
        ui->Search_kcombobox_SearchText->setEditText(settings.value("LastSearch/SearchText").toString());
        selectedSearchCatalog   = settings.value("LastSearch/SelectedSearchCatalog").toString();
        selectedFileType        = settings.value("LastSearch/FileType").toString();
        selectedTextCriteria    = settings.value("LastSearch/SearchTextCriteria").toString();
        selectedSearchIn        = settings.value("LastSearch/SearchIn").toString();
        selectedMinimumSize     = settings.value("LastSearch/MinimumSize").toLongLong();
        selectedMaximumSize     = settings.value("LastSearch/MaximumSize").toLongLong();
        selectedMinSizeUnit     = settings.value("LastSearch/MinSizeUnit").toString();
        selectedMaxSizeUnit     = settings.value("LastSearch/MaxSizeUnit").toString();
        ui->Settings_ChBx_SaveRecordWhenUpdate->setChecked(settings.value("Settings/AutoSaveRecordWhenUpdate").toBool());
        ui->Settings_comboBox_Theme->setCurrentText(settings.value("Settings/UseDefaultDesktopTheme").toString());
        ui->Settings_checkBox_KeepOneBackUp->setChecked(settings.value("Settings/KeepOneBackUp").toBool());

    }
    //----------------------------------------------------------------------
    void MainWindow::saveSettings()
    {
        QSettings settings(settingsFile, QSettings:: IniFormat);
        //QString sText = "N/A";
        settings.setValue("LastCollectionFolder", collectionFolder);
        settings.setValue("LastSearch/SearchText", ui->Search_kcombobox_SearchText->currentText());
        settings.setValue("LastSearch/SelectedSearchCatalog", selectedSearchCatalog);
        settings.setValue("LastSearch/FileType", selectedFileType);
        settings.setValue("LastSearch/SearchTextCriteria", selectedTextCriteria);
        settings.setValue("LastSearch/SearchIn", selectedSearchIn);
        settings.setValue("LastSearch/MinimumSize", selectedMinimumSize);
        settings.setValue("LastSearch/MaximumSize", selectedMaximumSize);
        settings.setValue("LastSearch/MinSizeUnit", selectedMinSizeUnit);
        settings.setValue("LastSearch/MaxSizeUnit", selectedMaxSizeUnit);
        settings.setValue("Settings/AutoSaveRecordWhenUpdate", ui->Settings_ChBx_SaveRecordWhenUpdate->isChecked());
        settings.setValue("Settings/UseDefaultDesktopTheme", ui->Settings_comboBox_Theme->currentText());
        settings.setValue("Settings/KeepOneBackUp", ui->Settings_checkBox_KeepOneBackUp->isChecked());
        //settings.setValue("LastSelectedCatalog", sText);
    }
    //----------------------------------------------------------------------
    void MainWindow::setFileTypes()
    {
        //Filetypes for cataloging
        fileType_Image<< "*.png" << "*.jpg" << "*.gif" << "*.xcf" << "*.tif" << "*.bmp";
        fileType_Audio<< "*.mp3" << "*.wav" << "*.ogg" << "*.aif";
        fileType_Video<< "*.wmv" << "*.avi" << "*.mp4" << "*.mkv" << "*.flv"  << "*.webm";
        fileType_Text << "*.txt" << "*.pdf" << "*.odt" << "*.idx" << "*.html" << "*.rtf" << "*.doc" << "*.docx" << "*.epub";

        //filetypes for searching
        fileType_ImageS<< "*.png$" << "*.jpg$" << "*.gif$" << "*.xcf$" << "*.tif$" << "*.bmp$";
        fileType_AudioS<< "*.mp3$" << "*.wav$" << "*.ogg$" << "*.aif$";
        fileType_VideoS<< "*.wmv$" << "*.avi$" << "*.mp4$" << "*.mkv$" << "*.flv$"  << "*.webm$";
        fileType_TextS << "*.txt$" << "*.pdf$" << "*.odt$" << "*.idx$" << "*.html$" << "*.rtf$" << "*.doc$" << "*.docx$" << "*.epub$";

    }

//Development -------------------------------------------------------
    void MainWindow::hideDevelopmentUIItems()
    {
        //Search
        ui->Search_label_Regex->hide();
        //ui->HL_Location_and_Storage->hide();

        //Create
        ui->Create_checkBox_IncludeSubDir->hide();
        ui->Create_checkBox_IncludeSymblinks->hide();
        ui->Create_checkBox_IncludeArchives->hide();
        ui->Create_checkBox_IncludeChecksum->hide();

        //Explore
        ui->Explore_label_Directories->hide();
        ui->Explore_treeview_Directories->hide();

        //Collection
        ui->Collection_pushButton_Convert->hide();
        //ui->Explore_L_Directories->hide();
        //ui->Explore_treeview_Directories->hide();
        //ui->PB_C_Rename->hide();

        //Storage
        ui->Storage_PB_SaveAll->hide();
        ui->Storage_PB_New->hide();
        //ui->Storage_PB_SearchLocation->hide();
        ui->Storage_PB_OpenFilelight->hide();
        ui->Storage_PB_Update->hide();
        ui->Storage_PB_Delete->hide();
        ui->Storage_L_SpaceUnit->hide();

        //Settings
        //ui->Settings_label_Theme->hide();
        //ui->Settings_comboBox_Theme->hide();

        //Other tabs
        ui->tabWidget->removeTab(10);
        ui->tabWidget->removeTab(9);
        ui->tabWidget->removeTab(8);
        ui->tabWidget->removeTab(7);

        //Test translation QPushButton hello(QPushButton::tr("Hello world!"));

    }
    //----------------------------------------------------------------------
    void MainWindow::loadTypeOfData()
    {
        typeOfData << "Number of files" << "Total file size";
        listModel = new QStringListModel(this);
        listModel->setStringList(typeOfData);
        ui->Stats_comboBox_TypeOfData->setModel(listModel);
        ui->Stats_comboBox_TypeOfData->setCurrentText(typeOfData[1]);
    }

    //----------------------------------------------------------------------
    void MainWindow::loadCustomTheme1()
    {       
        //Global
        ui->tabWidget->setStyleSheet(
              "QTabBar              { background-color: transparent; qproperty-drawBase:0;}"
              "QTabBar::tab         { background-color: #10a2df; color: #EEE; padding: 5px 20px; }"
              "QTabBar::tab:selected{ background-color: #eff0f1; color: #10a2df; font-weight: bold; text-decoration: none;} "
              );

        //Search Tab
        ui->Search_pushButton_Search->setStyleSheet(
              "QPushButton          { color: #43bf0c; padding: 6px; font-weight: bold;} "
              );
        ui->Create_pushButton_CreateCatalog->setStyleSheet(
              "QPushButton          { color: #43bf0c; padding: 6px; font-weight: bold;} "
              );
        ui->Search_line_SeparateResults->setStyleSheet("QFrame { color: #10a2df; } ");
        ui->Collection_line_SeparateCatalogs->setStyleSheet("QFrame { color: #10a2df; } ");

        ui->Search_label_LinkImage1->setStyleSheet("QLabel { background: url(:/images/link_blue/link-tree-mid.png) repeat-y left; } ");
        ui->Search_label_LinkImage2->setStyleSheet("QLabel { background: url(:/images/link_blue/link-v.png) repeat-y left; } ");
        ui->Search_label_LinkImage3->setStyleSheet("QLabel { background: url(:/images/link_blue/link-tree-end.png) no-repeat left; } ");
        ui->Search_label_LinkImage4->setStyleSheet("QLabel { background: url(:/images/link_blue/link-h.png) repeat-x left; } ");
        ui->Search_label_LinkImage5->setStyleSheet("QLabel { background: url(:/images/link_blue/link-tree-mid.png) repeat-y left; } ");
        ui->Search_label_LinkImage6->setStyleSheet("QLabel { background: url(:/images/link_blue/link-tree-end.png) no-repeat left; } ");
        ui->Search_label_LinkImage7->setStyleSheet("QLabel { background: url(:/images/link_blue/link-h.png) repeat-x left; } ");
        ui->Search_label_LinkImage8->setStyleSheet("QLabel { background: url(:/images/link_blue/link-h.png) repeat-x left; } ");

    }

    //----------------------------------------------------------------------
    void MainWindow::loadCustomTheme2()
    {
        //Mainwindow
        //ui->MainWindow.setStyleSheet(QString("QTabBar::tab:selected { background: lightgray; } "));
        //ui->centralwidget->setStyleSheet(QString("QTabBar::tab:selected { background: lightgray; } "));

        //TabWidget
        //ui->tabWidget->setStyleSheet(QString("QTabBar::tab:selected { background: white; color: #43bf0c} "));
        //ui->tabWidget->setStyleSheet(QString("QTabBar::tab:selected { background: white; border: 5px solid; boder-color: #43bf0c} "));

        ui->tabWidget->setStyleSheet(
              "QTabBar::tab         { padding: 6px 14px; background: #CCC; } "
              "QTabBar::tab:selected{ background: #10a2df; color: white; font-weight: bold; text-decoration: none;} "

              ); //"QWidget              { background: #EEE; } "
            //"QTabWidget::pane     { border: 0; } "

        ui->Search_pushButton_Search->setStyleSheet(
              "QPushButton          { background: #43bf0c; color: white; padding: 6px; font-weight: bold;} ");

        ui->Search_treeView_FilesFound->setStyleSheet(
              "QTreeView            { background: white; color: white; padding: 6px; } ");

                //SB_MinimumSize
        //font-weight: bold;

    }

/*
    //Menu and Icons - Actions KDE setup ---------------------------------------
        void MainWindow::setupActions()
        {

            KStandardAction::quit(qApp, SLOT(quit()), actionCollection());
            //KStandardAction::open(this, SLOT(openFile()), actionCollection());
            //KStandardAction::save(this, SLOT(saveFile()), actionCollection());
            //KStandardAction::saveAs(this, SLOT(saveFileAs()), actionCollection());
            //KStandardAction::openNew(this, SLOT(newFile()), actionCollection());
            setupGUI();
            *
        }
        //----------------------------------------------------------------------
        void MainWindow::newFile()
        {
            fileName.clear();
            ui->plainTextEdit->clear();
            ui->statusbar->showMessage(fileName);
        }
        //----------------------------------------------------------------------
        void MainWindow::openFile()
        {
            QUrl fileNameFromDialog = QFileDialog::getOpenFileUrl(this, ("Open a Katalog collection"));

            if (!fileNameFromDialog.isEmpty())
            {
                KIO::Job* job = KIO::storedGet(fileNameFromDialog);
                fileName = fileNameFromDialog.toLocalFile();

                connect(job, SIGNAL(result(KJob*)), this,
                             SLOT(downloadFinished(KJob*)));

                job->exec();
            }

            ui->statusbar->showMessage(fileName);
        }
        //----------------------------------------------------------------------
        void MainWindow::downloadFinished(KJob* job)
        {
            if (job->error())
            {
                QMessageBox::warning(this, "Katalog",job->errorString());
                fileName.clear();
                return;
            }

            KIO::StoredTransferJob* storedJob = (KIO::StoredTransferJob*)job;
            ui->plainTextEdit->setPlainText(QTextStream(storedJob->data(),
                                            QIODevice::ReadOnly).readAll());
        }
        //----------------------------------------------------------------------
        void MainWindow::saveFileAs(const QString &outputFileName)
        {
            if (!outputFileName.isNull())
            {
                QSaveFile file(outputFileName);
                file.open(QIODevice::WriteOnly);

                QByteArray outputByteArray;
                outputByteArray.append(ui->plainTextEdit->toPlainText().toUtf8());
                //outputByteArray.append(ui->LV_FileList->model());
                file.write(outputByteArray);
                file.commit();

                fileName = outputFileName;
            }
        }
        //----------------------------------------------------------------------
        void MainWindow::saveFileAs()
        {
            saveFileAs(QFileDialog::getSaveFileName(this, ("Save File As")));
            ui->statusbar->showMessage(fileName);
        }
        //----------------------------------------------------------------------
        void MainWindow::saveFile()
        {
            if (!fileName.isEmpty())
            {
                saveFileAs(fileName);
            }
            else
            {
                saveFileAs();
            }
        }
*/
    //----------------------------------------------------------------------
