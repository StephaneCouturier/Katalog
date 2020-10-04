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

#include <KActionCollection>
#include <KMessageBox>
#include <KIO/Job>
#include <KLocalizedString>


//Menu and Icons - Actions KDE setup ---------------------------------------
    void MainWindow::setupActions()
    {
        KStandardAction::quit(qApp, SLOT(quit()), actionCollection());
        //KStandardAction::open(this, SLOT(openFile()), actionCollection());
        //KStandardAction::save(this, SLOT(saveFile()), actionCollection());
        //KStandardAction::saveAs(this, SLOT(saveFileAs()), actionCollection());
        //KStandardAction::openNew(this, SLOT(newFile()), actionCollection());
        setupGUI();
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
        QUrl fileNameFromDialog = QFileDialog::getOpenFileUrl(this, i18n("Open a Katalog collection"));

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
            KMessageBox::error(this, job->errorString());
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
        saveFileAs(QFileDialog::getSaveFileName(this, i18n("Save File As")));
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
    //----------------------------------------------------------------------

//Settings -----------------------------------------------------------------
    void MainWindow::setupFileContextMenu(){
        ui->TrV_FilesFound->setContextMenuPolicy(Qt::CustomContextMenu);
        connect( ui->TrV_FilesFound, SIGNAL(customContextMenuRequested(const QPoint&)),
            this, SLOT(ShowContextMenu(const QPoint&)));

        ui->TrV_FileList->setContextMenuPolicy(Qt::CustomContextMenu);
        connect(ui->TrV_FileList, SIGNAL(customContextMenuRequested(const QPoint&)),
            this, SLOT(ShowContextMenu(const QPoint&)));
    }
    //----------------------------------------------------------------------
    void MainWindow::loadSettings()
    {
        QSettings settings(settingsFile, QSettings::NativeFormat);
        collectionFolder = settings.value("LastCollectionFolder").toString();
        if(collectionFolder == ""){
               collectionFolder = QApplication::applicationDirPath();
        }
        ui->KCB_SearchText->setEditText(settings.value("LastSearchText").toString());
        selectedSearchCatalog   = settings.value("LastSelectedSearchCatalog").toString();
        selectedFileType        = settings.value("LastFileType").toString();
        selectedTextCriteria    = settings.value("LastSearchTextCriteria").toString();
        selectedSearchIn        = settings.value("LastSearchIn").toString();
        selectedMinimumSize     = settings.value("LastMinimumSize").toLongLong();
        selectedMaximumSize     = settings.value("LastMaximumSize").toLongLong();
        selectedSizeUnit        = settings.value("LastSizeUnit").toString();
    }
    //----------------------------------------------------------------------
    void MainWindow::saveSettings()
    {
        QSettings settings(settingsFile, QSettings::NativeFormat);
        QString sText = "N/A";
        settings.setValue("LastCollectionFolder", collectionFolder);
        settings.setValue("LastSearchText", ui->KCB_SearchText->currentText());
        settings.setValue("LastSelectedSearchCatalog", selectedSearchCatalog);
        settings.setValue("LastFileType", selectedFileType);
        settings.setValue("LastSearchTextCriteria", selectedTextCriteria);
        settings.setValue("LastSearchIn", selectedSearchIn);
        settings.setValue("LastMinimumSize", selectedMinimumSize);
        settings.setValue("LastMaximumSize", selectedMaximumSize);
        settings.setValue("LastSizeUnit", selectedSizeUnit);
        //settings.setValue("LastSelectedCatalog", sText);
    }
    //----------------------------------------------------------------------
    void MainWindow::setFileTypes()
    {
        //Filetypes for cataloging
        fileType_Image<< "*.png" << "*.jpg" << "*.gif" << "*.xcf" << "*.tif" << "*.bmp";
        fileType_Audio<< "*.mp3" << "*.wav" << "*.ogg";
        fileType_Video<< "*.wmv" << "*.avi" << "*.mp4" << "*.mkv" << "*.flv"  << "*.webm";
        fileType_Text << "*.txt" << "*.pdf" << "*.odt" << "*.idx" << "*.html" << "*.rtf" << "*.doc" << "*.docx" << "*.epub";

        //filetypes for searching
        fileType_ImageS<< "*.png$" << "*.jpg$" << "*.gif$" << "*.xcf$" << "*.tif$" << "*.bmp$";
        fileType_AudioS<< "*.mp3$" << "*.wav$" << "*.ogg$";
        fileType_VideoS<< "*.wmv$" << "*.avi$" << "*.mp4$" << "*.mkv$" << "*.flv$"  << "*.webm$";
        fileType_TextS << "*.txt$" << "*.pdf$" << "*.odt$" << "*.idx$" << "*.html$" << "*.rtf$" << "*.doc$" << "*.docx$" << "*.epub$";

    }

//Development -------------------------------------------------------
    void MainWindow::hideDevelopmentUIItems()
    {
        //Search
        //ui->L_Regex->hide();
        ui->Search_ChB_Size->hide();

        //Create
        //ui->L_OtherOptions->hide();
        ui->RB_IncludeSubDir->hide();
        ui->RB_IncludeSymblinks->hide();
        ui->RB_IncludeArchives->hide();
        ui->RB_IncludeChecksum->hide();

        //Explore
        ui->L_Filter->hide();
        ui->LE_TextToFilter->hide();

        //Collection
        //ui->PB_RecordCatalogStats->hide();
        ui->L_Directories->hide();
        ui->TV_Directories->hide();
        //ui->PB_C_Rename->hide();

        //Storage
        ui->Storage_PB_SaveAll->hide();

        ui->LE_TextToFilter->hide();
        ui->Storage_PB_New->hide();
        ui->Storage_PB_SearchLocation->hide();
        ui->Storage_PB_OpenFilelight->hide();
        ui->Storage_PB_Update->hide();
        ui->Storage_PB_Delete->hide();
        ui->Storage_L_SpaceUnit->hide();

        //Other tabs
        ui->tabWidget->removeTab(8);
        //ui->tabWidget->removeTab(7);
        //ui->tabWidget->removeTab(6);
        //ui->tabWidget->removeTab(5);

        //QPushButton hello(QPushButton::tr("Hello world!"));

    }
    //----------------------------------------------------------------------
