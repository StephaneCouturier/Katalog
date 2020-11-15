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
// Version:     0.13
/////////////////////////////////////////////////////////////////////////////
*/

#include "mainwindow.h"
#include "ui_mainwindow.h"
#include "database.h"

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
        //Collection folder
        collectionFolder = settings.value("LastCollectionFolder").toString();
        if(collectionFolder == ""){
               collectionFolder = QApplication::applicationDirPath();
        }
        //Last Search values
        ui->Search_kcombobox_SearchText->setEditText(settings.value("LastSearch/SearchText").toString());
        selectedSearchCatalog   = settings.value("LastSearch/SelectedSearchCatalog").toString();
        selectedFileType        = settings.value("LastSearch/FileType").toString();
        selectedTextCriteria    = settings.value("LastSearch/SearchTextCriteria").toString();
        selectedSearchIn        = settings.value("LastSearch/SearchIn").toString();
        selectedMinimumSize     = settings.value("LastSearch/MinimumSize").toLongLong();
        selectedMaximumSize     = settings.value("LastSearch/MaximumSize").toLongLong();
        selectedMinSizeUnit     = settings.value("LastSearch/MinSizeUnit").toString();
        selectedMaxSizeUnit     = settings.value("LastSearch/MaxSizeUnit").toString();

        //Show or Hide ShowHideCatalogResults
        if ( settings.value("Settings/ShowHideCatalogResults") == ">>"){ //Hide
                ui->Search_pushButton_ShowHideCatalogResults->setText(">>");
                ui->Search_listView_CatalogsFound->setHidden(true);
                ui->Search_label_CatalogsWithResults->setHidden(true);
        }

        //General settings
        ui->Settings_checkBox_SaveRecordWhenUpdate->setChecked(settings.value("Settings/AutoSaveRecordWhenUpdate").toBool());
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
        settings.setValue("Settings/AutoSaveRecordWhenUpdate", ui->Settings_checkBox_SaveRecordWhenUpdate->isChecked());
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

        //Storage
        ui->Storage_pushButton_OpenFilelight->hide();
        ui->Storage_label_SpaceUnit->hide();

        //Settings

        //Other tabs
        ui->tabWidget->removeTab(7); //Tags
    }

    //----------------------------------------------------------------------
    void MainWindow::loadCustomTheme1()
    {       
        //colors:
            //blue light	39b2e5
            //blue dark		10a2df
            //green light	81d41a
            //green dark	43bf0c
            //orange light	ff8000
            //orange dark	e36600
            //purple light	a1467e
            //purple dark	8b1871

        /* blue tabwidget bar */
        ui->tabWidget->setStyleSheet(
              "QTabBar               { background:  url(:images/Katalog40.png) no-repeat right;}"
              "QTabBar               { background-color: #0D79A6; qproperty-drawBase:0; outline: none; }"

              "QTabBar::tab          { background-color: #10a2df; color: #EAEAEA; padding: 5px 12px; border-right: 1px solid #39b2e5; }"
              "QTabBar::tab          { background: qlineargradient(x1: 0, y1: 0, x2: 0, y2: 1, stop: 0 #39b2e5, stop: 0.2 #39b2e5, stop: 0.3 #31aee4, stop: 0.5 #1ea6e1, stop: 1 #10a2df); }"

              "QTabBar::tab:selected { background-color: #39b2e5; color: #FFF; font-weight: bold; } "
              "QTabBar::tab:selected { border-top-left-radius: 5px; border-top-right-radius: 5px;} "
              "QTabBar::tab:!selected{ margin-top: 5px; }"
              "QTabWidget::tab-bar   { left: 0px; }"
              );

        //Search tab
        ui->Search_pushButton_Search->setStyleSheet(
              "QPushButton          { color: #43bf0c; padding: 6px; font-weight: bold;} "
              );
        /*
        ui->Search_label_Text->setStyleSheet(
              "QLabel          { border: 1px solid #10a2df; color: #10a2df; padding: 5px 5px; } "
              );
        ui->Search_label_Catalog->setStyleSheet(
              "QLabel          { border: 1px solid #10a2df; color: #10a2df; padding: 5px 5px; } "
              );
        ui->Search_label_File->setStyleSheet(
              "QLabel          { border: 1px solid #10a2df; color: #10a2df; padding: 5px 5px; } "
              );
        */
        ui->Search_line_SeparateResults->setStyleSheet("QFrame { color: #10a2df; } ");

        //Create tab
        ui->Create_pushButton_CreateCatalog->setStyleSheet(
              "QPushButton          { color: #43bf0c; padding: 6px; font-weight: bold;} "
              );
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

    void MainWindow::startDatabase()
    {
        if (!QSqlDatabase::drivers().contains("QSQLITE"))
            QMessageBox::critical(
                        this,
                        "Unable to load database",
                        "This demo needs the SQLITE driver"
                        );

        // Initialize the database:

        QSqlError err = initializeDatabase();
        if (err.type() != QSqlError::NoError) {
            //showError(err);
            return;
        }


        //Statistics_TaV_Test
        // Create the data model:

        storageModel = new QSqlRelationalTableModel(this);
        storageModel->setEditStrategy(QSqlTableModel::OnManualSubmit);

        // Set the model
        //ui->Test_tableView_sql1->setModel(model);
        //ui->Test_tableView_sql1->setSelectionMode(QAbstractItemView::SingleSelection);


       // QSortFilterProxyModel *proxyStorageModel = new QSortFilterProxyModel(this);
        //proxyStorageModel->setSourceModel(model);

        // Connect model to tree/table view
        //ui->test_treeView->setModel(proxyStorageModel);
       /*
        ui->test_treeView->QTreeView::sortByColumn(1,Qt::AscendingOrder);
        ui->test_treeView->QTreeView::sortByColumn(0,Qt::AscendingOrder);
        ui->test_treeView->header()->setSectionResizeMode(QHeaderView::Interactive);
        ui->test_treeView->header()->resizeSection(0,  50); //ID
        ui->test_treeView->header()->resizeSection(1, 150); //Name
        ui->test_treeView->header()->resizeSection(2, 100); //type
        ui->test_treeView->header()->resizeSection(3, 175); //location
        ui->test_treeView->header()->resizeSection(4, 250); //Path
        ui->test_treeView->header()->resizeSection(5,  50); //Path
        ui->test_treeView->header()->resizeSection(6,  75); //FS
        ui->test_treeView->header()->resizeSection(7,  75); //Total
        ui->test_treeView->header()->resizeSection(8,  75); //Free
        //ui->test_treeView->header()->hideSection(1); //Path
    */
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
