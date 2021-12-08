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
// Purpose:     methods for the mainwindow
// Description: initiate additions to the interface including theme and loading previous settings
// Author:      Stephane Couturier
// Version:     1.00
/////////////////////////////////////////////////////////////////////////////
*/

#include "mainwindow.h"
#include "ui_mainwindow.h"
#include "database.h"

#include <QFileDialog>
#include <QTextStream>
#include <QSaveFile>
#include <QSettings>
#include <QDesktopServices>
#include <QNetworkAccessManager>
#include <QNetworkReply>

#ifdef Q_OS_LINUX
    #include <KActionCollection>
    #include <KIO/Job>
    #include <KMessageBox>
    #include <KLocalizedString>
#endif

//Set up -------------------------------------------------------------------
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
        //Check if a settings file already exists. If not, it is considered first use and one gets generted
        QFile settingsFile(settingsFilePath);
        int themeID = 1; //default value for the theme.
        selectedTab = 3; //default value for the first launch

        if (!settingsFile.exists()){
            //create a file, with default values
                  QSettings settings(settingsFilePath, QSettings:: IniFormat);
                  settings.setValue("General/LastCollectionFolder", QApplication::applicationDirPath());

            //Set Language and theme

            QString userLanguage = QLocale::system().name();
            settings.setValue("Settings/Language", userLanguage);

            QString themeName = tr("Katalog Colors (light)");

            QMessageBox::information(this,"Katalog",tr("<br/><b>Welcome to Katalog!</b><br/><br/>"
                                                       "It seems this is the first run.<br/><br/>"
                                                       "The following Settings have been applied:<br/>"
                                                       " - Language: <b>%1</b><br/> - Theme: <b>%2</b><br/><br/>You can change these in the tab %3.").arg(userLanguage,themeName,tr("Settings")));

            //Language
            ui->Settings_comboBox_Language->setCurrentText(userLanguage);

            //Collection folder choice

            //Go to Create screen
            // add: "Let's go to the Create screen to create your first catalog"

        }

        //Load the settings file

            QSettings settings(settingsFilePath, QSettings:: IniFormat);

            //Collection folder
            collectionFolder = settings.value("LastCollectionFolder").toString();
            if(collectionFolder == ""){
                   collectionFolder = QApplication::applicationDirPath();
            }

            //Restore last Search values
            #ifdef Q_OS_LINUX
                    ui->Search_kcombobox_SearchText->setEditText(settings.value("LastSearch/SearchText").toString());
            #else
                    ui->Search_lineEdit_SearchText->setText(settings.value("LastSearch/SearchText").toString());
            #endif


            selectedSearchLocation  = settings.value("LastSearch/SelectedSearchLocation").toString();
            ui->Filters_comboBox_SelectLocation->setCurrentText(selectedSearchLocation);

            selectedSearchStorage   = settings.value("LastSearch/SelectedSearchStorage").toString();
            ui->Filters_comboBox_SelectStorage->setCurrentText(selectedSearchStorage);

            selectedSearchCatalog   = settings.value("LastSearch/SelectedSearchCatalog").toString();
            ui->Filters_comboBox_SelectCatalog->setCurrentText(selectedSearchCatalog);


            selectedFileType        = settings.value("LastSearch/FileType").toString();
            selectedTextCriteria    = settings.value("LastSearch/SearchTextCriteria").toString();
            selectedSearchIn        = settings.value("LastSearch/SearchIn").toString();
            selectedMinimumSize     = settings.value("LastSearch/MinimumSize").toLongLong();
            selectedMaximumSize     = settings.value("LastSearch/MaximumSize").toLongLong();
            selectedMinSizeUnit     = settings.value("LastSearch/MinSizeUnit").toString();
            selectedMaxSizeUnit     = settings.value("LastSearch/MaxSizeUnit").toString();
            selectedDateMin         = QDateTime::fromString(settings.value("LastSearch/DateMin").toString(),"yyyy/MM/dd hh:mm:ss");
            selectedDateMax         = QDateTime::fromString(settings.value("LastSearch/DateMax").toString(),"yyyy/MM/dd hh:mm:ss");
            searchOnSize            = settings.value("LastSearch/searchOnSize").toBool();
            searchOnDate            = settings.value("LastSearch/searchOnDate").toBool();
            searchOnTags            = settings.value("LastSearch/searchOnTags").toBool();
            searchOnText            = settings.value("LastSearch/searchOnText").toBool();
            showFoldersOnly         = settings.value("LastSearch/showFoldersOnly").toBool();
            selectedTag             = settings.value("LastSearch/SearchTag").toString();
            searchOnDuplicates      = settings.value("LastSearch/DuplicatesOn").toBool();
            hasDuplicatesOnName     = settings.value("LastSearch/hasDuplicatesOnName").toBool();
            hasDuplicatesOnSize     = settings.value("LastSearch/hasDuplicatesOnSize").toBool();
            hasDuplicatesOnDateModified = settings.value("LastSearch/hasDuplicatesOnDateModified").toBool();

            //Show or Hide ShowHideCatalogResults
            if ( settings.value("Settings/ShowHideCatalogResults") == "arrow-right"){ //Hide
                    ui->Search_pushButton_ShowHideCatalogResults->setIcon(QIcon::fromTheme("arrow-right"));
                    ui->Search_listView_CatalogsFound->setHidden(true);
                    ui->Search_label_CatalogsWithResults->setHidden(true);
            }

            //Show or Hide ShowHideCatalogResults
            if ( settings.value("Settings/ShowHideGlobal") == "arrow-right"){ //Hide
                    ui->Global_pushButton_ShowHideGlobal->setIcon(QIcon::fromTheme("arrow-right"));
                    ui->Global_tabWidget->setHidden(true);
                    ui->Global_label_Global->setHidden(true);
            }

            //Show or Hide ShowHideSearchResults
            if ( settings.value("Settings/ShowHideSearchHistory") == "arrow-up"){ //Hide
                    ui->Search_pushButton_ShowHideSearchHistory->setIcon(QIcon::fromTheme("arrow-down"));
                    ui->Search_tableView_History->setHidden(true);
            }

            //General settings
            ui->Settings_checkBox_SaveRecordWhenUpdate->setChecked(settings.value("Settings/AutoSaveRecordWhenUpdate", true).toBool());
            QString themeText = settings.value("Settings/Theme").toString();
            if (themeText==""){
                //fallback on default theme
                themeID=1;
            }
            else
                themeID = settings.value("Settings/Theme").toInt();
            ui->Settings_checkBox_KeepOneBackUp->setChecked(settings.value("Settings/KeepOneBackUp", true).toBool());
            ui->Settings_comboBox_Language->setCurrentText(settings.value("Settings/Language").toString());
            ui->Settings_checkBox_CheckVersion->setChecked(settings.value("Settings/CheckVersion", true).toBool());
            ui->Settings_comboBox_Theme->setCurrentIndex(themeID);

            //Restore last statistics values
            ui->Statistics_comboBox_SelectSource->setCurrentText(settings.value("Statistics/SelectedSource").toString());
            ui->Statistics_comboBox_SelectCatalog->setCurrentText(settings.value("Statistics/SelectedCatalog").toString());
            ui->Statistics_comboBox_TypeOfData->setCurrentText(settings.value("Statistics/TypeOfData").toString());

            //Restore last opened catalog file to Explore
            selectedCatalogFile   = settings.value("Explore/lastSelectedCatalogFile").toString();
            selectedCatalogName   = settings.value("Explore/lastSelectedCatalogName").toString();
            selectedCatalogPath   = settings.value("Explore/lastSelectedCatalogPath").toString();
            selectedDirectoryName = settings.value("Explore/lastSelectedDirectory").toString();

            //last tab selected
            selectedTab = settings.value("Settings/selectedTab").toInt();                      
            int selectedTabGlobal = settings.value("Settings/selectedTabGlobal").toInt();
            ui->Global_tabWidget->setCurrentIndex(selectedTabGlobal);
            ui->tabWidget->setCurrentIndex(selectedTab);

    }
    //----------------------------------------------------------------------
    void MainWindow::saveSettings()
    {
        QSettings settings(settingsFilePath, QSettings:: IniFormat);

        settings.setValue("LastCollectionFolder", collectionFolder);
        #ifdef Q_OS_LINUX
            settings.setValue("LastSearch/SearchText", ui->Search_kcombobox_SearchText->currentText());
        #else        
            settings.setValue("LastSearch/SearchText", ui->Search_lineEdit_SearchText->text());
        #endif
        settings.setValue("LastSearch/SelectedSearchCatalog", selectedSearchCatalog);
        settings.setValue("LastSearch/SelectedSearchStorage", selectedSearchStorage);
        settings.setValue("LastSearch/SelectedSearchLocation", selectedSearchLocation);
        settings.setValue("LastSearch/FileType", selectedFileType);
        settings.setValue("LastSearch/SearchTextCriteria", selectedTextCriteria);
        settings.setValue("LastSearch/SearchIn", selectedSearchIn);
        settings.setValue("LastSearch/MinimumSize", selectedMinimumSize);
        settings.setValue("LastSearch/MaximumSize", selectedMaximumSize);
        settings.setValue("LastSearch/MinSizeUnit", selectedMinSizeUnit);
        settings.setValue("LastSearch/MaxSizeUnit", selectedMaxSizeUnit);

        settings.setValue("LastSearch/showFoldersOnly", ui->Search_checkBox_ShowFolders->isChecked());
        settings.setValue("LastSearch/DuplicatesOn", ui->Search_checkBox_Duplicates->isChecked());
        settings.setValue("LastSearch/hasDuplicatesOnName", ui->Search_checkBox_DuplicateName->isChecked());
        settings.setValue("LastSearch/hasDuplicatesOnSize", ui->Search_checkBox_DuplicateSize->isChecked());
        settings.setValue("LastSearch/hasDuplicatesOnDateModified", ui->Search_checkBox_DuplicateDateModified->isChecked());
        settings.setValue("LastSearch/searchOnSize", ui->Search_checkBox_Size->isChecked());
        settings.setValue("LastSearch/searchOnDate", ui->Search_checkBox_Date->isChecked());
        settings.setValue("LastSearch/DateMin", ui->Search_dateTimeEdit_Min->dateTime().toString("yyyy/MM/dd hh:mm:ss"));
        settings.setValue("LastSearch/DateMax", ui->Search_dateTimeEdit_Max->dateTime().toString("yyyy/MM/dd hh:mm:ss"));
        settings.setValue("LastSearch/searchOnText", ui->Search_checkBox_Text->isChecked());
        settings.setValue("LastSearch/searchOnTags", ui->Search_checkBox_Tags->isChecked());
        settings.setValue("LastSearch/SearchTag", ui->Search_comboBox_Tags->currentText());

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
    //----------------------------------------------------------------------
    void MainWindow::hideDevelopmentUIItems()
    {
        //Search

        //Catalogs

        //Create
            //DEV: the option to include symblinks is not working yet
            ui->Create_checkBox_IncludeSymblinks->hide();

        //Explore

        //Storage
            //DEV: pending a test qnd development of a function to open Filelight at the requested place
            ui->Storage_pushButton_OpenFilelight->hide();

        //Settings

        //Other tabs
            ui->tabWidget->removeTab(8); //ExploreTreeview
            ui->tabWidget->removeTab(7); //ExploreTreeview
    }
    //----------------------------------------------------------------------
    void MainWindow::loadCustomThemeLight()
    {       
        //colors:
            //blue light	39b2e5
            //blue dark		10a2df  0D79A6
            //green light	81d41a
            //green dark	43bf0c
            //orange light	ff8000
            //orange dark	e36600
            //purple light	a1467e
            //purple dark	8b1871

        /* blue tabwidget bar */
            if(developmentMode==true){
                QFile file(":styles/tabwidget_dev.css");
                file.open(QFile::ReadOnly);
                QString styleSheet = QLatin1String(file.readAll());
                ui->tabWidget->setStyleSheet(styleSheet);
            }
            else{
                QFile file(":styles/tabwidget_blue.css");
                file.open(QFile::ReadOnly);
                QString styleSheet = QLatin1String(file.readAll());
                ui->tabWidget->setStyleSheet(styleSheet);
            }
			  
		/* global tabwidget bar */
        ui->Global_tabWidget->setStyleSheet(
            "QComboBox             { background-color: #FFF; padding-left: 6px; }"
            "QLabel                { color: #095676; }"
            "QTabBar::tab          { height: 30px; }"
            "QTabWidget::tab-bar   { left: 0px; }"
            "QTabWidget            { padding: 0px; margin: 0px; }"

            "QPushButton           { text-align: left; padding: 5px 4px; margin: 0px; border: 1px solid #ccc; border-radius: 5px;	padding: 5px;} "
            "QPushButton::hover    { background: #39b2e5; color: #fff; border: 1px solid #39b2e5; 	border-radius: 5px;	padding: 5px;}"
            "QPushButton::pressed  { background: #0D79A6; color: #fff; border: 1px solid #10a2df; 	border-radius: 5px;	padding: 5px;}"

         );
		 
        //Colored buttons
        ui->Search_pushButton_Search->setStyleSheet(
                "QPushButton           { background-color: #81d41a; color: #fff; } "
                "QPushButton::hover    { background-color: #81d41a; color: #fff; border: 1px solid #43bf0c; 	border-radius: 5px;	padding: 5px;}"
                "QPushButton::pressed  { background-color: #43bf0c; color: #fff; border: 1px solid #43bf0c; 	border-radius: 5px;	padding: 5px;}"
              );
        ui->Catalogs_pushButton_UpdateCatalog->setStyleSheet(
                "QPushButton           { background-color: #ff8000; color: #fff; } "
                "QPushButton::hover    { background-color: #ff8000; color: #fff; border: 1px solid #e36600; 	border-radius: 5px;	padding: 5px;}"
                "QPushButton::pressed  { background-color: #e36600; color: #fff; border: 1px solid #e36600; 	border-radius: 5px;	padding: 5px;}"
              );
        ui->Catalogs_pushButton_UpdateAllActive->setStyleSheet(
                "QPushButton           { background-color: #ff8000; color: #fff; } "
                "QPushButton::hover    { background-color: #ff8000; color: #fff; border: 1px solid #e36600; 	border-radius: 5px;	padding: 5px;}"
                "QPushButton::pressed  { background-color: #e36600; color: #fff; border: 1px solid #e36600; 	border-radius: 5px;	padding: 5px;}"
              );
        ui->Storage_pushButton_Update->setStyleSheet(
                "QPushButton           { background-color: #ff8000; color: #fff; } "
                "QPushButton::hover    { background-color: #ff8000; color: #fff; border: 1px solid #e36600; 	border-radius: 5px;	padding: 5px;}"
                "QPushButton::pressed  { background-color: #e36600; color: #fff; border: 1px solid #e36600; 	border-radius: 5px;	padding: 5px;}"
              );
        ui->Create_pushButton_CreateCatalog->setStyleSheet(
                "QPushButton           { background-color: #81d41a; color: #fff; } "
                "QPushButton::hover    { background-color: #81d41a; color: #fff; border: 1px solid #43bf0c; 	border-radius: 5px;	padding: 5px;}"
                "QPushButton::pressed  { background-color: #43bf0c; color: #fff; border: 1px solid #43bf0c; 	border-radius: 5px;	padding: 5px;}"

              );

        //line and other UI items
        ui->Search_line_SeparateResults->setStyleSheet("QFrame { color: #095676; border-top: 1px solid 095676; } ");
        ui->Explore_line_Separate->setStyleSheet("QFrame { color: #095676; border-top: 1px solid 095676;} ");
        ui->Statistics_line_Separate->setStyleSheet("QFrame { color: #095676; border-top: 1px solid 095676;} ");
        ui->Catalogs_line_SeparateSummary_02->setStyleSheet("QFrame { color: #095676; border-top: 1px solid 095676; } ");
        ui->Storage_line_Separate->setStyleSheet("QFrame { color: #095676; border-top: 1px solid 095676; } ");

        //Doted lines on Search screen
        ui->Search_label_LinkImage01->setStyleSheet("QLabel { background: url(:/images/link_blue/link-tree-mid.png) repeat-y left; } ");
        ui->Search_label_LinkImage02->setStyleSheet("QLabel { background: url(:/images/link_blue/link-v.png) repeat-y left; } ");
        ui->Search_label_LinkImage03->setStyleSheet("QLabel { background: url(:/images/link_blue/link-tree-end.png) no-repeat left; } ");
        ui->Search_label_LinkImage04->setStyleSheet("QLabel { background: url(:/images/link_blue/link-h.png) repeat-x left; } ");
        ui->Search_label_LinkImage05->setStyleSheet("QLabel { background: url(:/images/link_blue/link-v.png) repeat-y left; } ");
        ui->Search_label_LinkImage13->setStyleSheet("QLabel { background: url(:/images/link_blue/link-tree-mid.png) repeat-y left; } ");
        ui->Search_label_LinkImage06->setStyleSheet("QLabel { background: url(:/images/link_blue/link-h.png) repeat-x left; } ");
        ui->Search_label_LinkImage07->setStyleSheet("QLabel { background: url(:/images/link_blue/link-tree-end.png) no-repeat left; } ");
        ui->Search_label_LinkImage09->setStyleSheet("QLabel { background: url(:/images/link_blue/link-tree-end.png) repeat-x left; } ");
        ui->Search_label_LinkImage11->setStyleSheet("QLabel { background: url(:/images/link_blue/link-tree-mid.png) repeat-y left; } ");
        ui->Search_label_LinkImage10->setStyleSheet("QLabel { background: url(:/images/link_blue/link-h.png) repeat-x left; } ");
        ui->Search_label_LinkImage12->setStyleSheet("QLabel { background: url(:/images/link_blue/link-h.png) repeat-x left; } ");

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

        storageModel = new QSqlRelationalTableModel(this);
        storageModel->setEditStrategy(QSqlTableModel::OnFieldChange);

    }
    //----------------------------------------------------------------------
    void MainWindow::checkVersion()
     {

         //Get the number of the lastest Version
         QString lastestVersion;
         QString htmlPage;
         QString downloadAddress = "https://github.com/StephaneCouturier/Katalog/releases/latest";
         //NOTES:
         // github will redirect this address to the actual lastest release page.
         // The event will return a message containing this latest release exact address;
         // This address contains the release tag, which is the number to get.

         //Get html message
         QNetworkAccessManager manager;
         QNetworkReply *response = manager.get(QNetworkRequest(QUrl(downloadAddress)));
         QEventLoop event;
         connect(response,SIGNAL(finished()),&event,SLOT(quit()));
         event.exec();
         htmlPage = response->readAll();

         //Parse html text, search, and return the release number
         QString searchString1 = "/Katalog/releases/tag/v";
         QStringList lineValues;
         QTextStream stream(&htmlPage);
         while (!stream.atEnd())
         {
             //Read the next line
             QString line = stream.readLine();

             //Verify it contains the search string
             if (line.contains(searchString1, Qt::CaseSensitive)) {

               //get value
               lineValues = line.split(searchString1);
               lineValues = lineValues[1].split("\">");
               lastestVersion = lineValues[0];
             }

         }

         QString releaseNotesAddress = "https://github.com/StephaneCouturier/Katalog/releases/tag/v" + lastestVersion;


        //inform user if new version is available, and give the choice to download it
        if ( lastestVersion > currentVersion ){

            int result = QMessageBox::information(this,"Katalog",
                      tr("This is version: v%1 <br/><br/>A new version is available: <b>v%2</b> <br/> "
                         "Find the list of new features in the <a href='%3'>Release Notes</a><br/><br/>"
                         "Do you want to download it?")
                         .arg(currentVersion,lastestVersion, releaseNotesAddress),
                         QMessageBox::Yes|QMessageBox::Cancel);

            if ( result ==QMessageBox::Yes){
                QDesktopServices::openUrl(QUrl("https://sourceforge.net/projects/katalogg/files/latest/download"));

            }
        }

     }

    //Menu and Icons - Actions KDE setup ---------------------------------------
	#ifdef Q_OS_LINUX
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
            //ui->plainTextEdit->clear();
            //ui->statusbar->showMessage(fileName);
        }
        //----------------------------------------------------------------------
        void MainWindow::openFile()
        {
            QUrl fileNameFromDialog = QFileDialog::getOpenFileUrl(this, tr("Open a Katalog collection"));

            if (!fileNameFromDialog.isEmpty())
            {
                KIO::Job* job = KIO::storedGet(fileNameFromDialog);
                fileName = fileNameFromDialog.toLocalFile();

                connect(job, SIGNAL(result(KJob*)), this,
                             SLOT(downloadFinished(KJob*)));

                job->exec();
            }

            //ui->statusbar->showMessage(fileName);
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

            #ifdef Q_OS_LINUX
            //KIO::StoredTransferJob* storedJob = (KIO::StoredTransferJob*)job;
            #endif

            //ui->plainTextEdit->setPlainText(QTextStream(storedJob->data(),QIODevice::ReadOnly).readAll());
        }
        //----------------------------------------------------------------------
        void MainWindow::saveFileAs(const QString &outputFileName)
        {
            if (!outputFileName.isNull())
            {
                QSaveFile file(outputFileName);
                file.open(QIODevice::WriteOnly);

                QByteArray outputByteArray;
                //outputByteArray.append(ui->plainTextEdit->toPlainText().toUtf8());
                //outputByteArray.append(ui->LV_FileList->model());
                file.write(outputByteArray);
                file.commit();

                fileName = outputFileName;
            }
        }
        //----------------------------------------------------------------------
        void MainWindow::saveFileAs()
        {
            saveFileAs(QFileDialog::getSaveFileName(this, tr("Save File As")));
            //ui->statusbar->showMessage(fileName);
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
	#endif
    //----------------------------------------------------------------------
