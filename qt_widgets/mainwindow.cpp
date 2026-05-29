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
 * /////////////////////////////////////////////////////////////////////////////
// Application: Katalog
// File Name:   mainwindow.cpp
// Purpose:     Class for the main window
// Description: intiate the User Interface, load data into the internal database, recover last user position and display data
// Author:      Stephane Couturier
/////////////////////////////////////////////////////////////////////////////
*/
#include "mainwindow.h"
#include "core/backupmappingmanager.h"
#include "ui_mainwindow.h"
#include "version.h"
#include "core/filemetadata.h"
#include "core/database.h"
#include "core/language.h"
#include "core/catalogjobstoppable.h"

MainWindow::MainWindow(QWidget *parent) : KXmlGuiWindow(parent),
    ui(new Ui::MainWindow)
{
    //Set current version and release date from CMake-generated values
    currentVersion  = KatalogVersion::string();
    //releaseDate     = "2026-04-12";
    releaseDate     = KatalogVersion::buildDate(); //Commented out for release

    // MainWindow is stack-allocated in main.cpp; prevent KMainWindow from
    // calling deleteLater() on close (which would try to delete a stack object).
    setAttribute(Qt::WA_DeleteOnClose, false);

    // Initialize objects first
    collection = new Collection();
    collection->appVersion = currentVersion;
    selectedDevice = new Device();
    searchJobStoppable = nullptr;
    loadSearch = nullptr;

    // Initialize other pointers
    currentSearch = nullptr;
    searchManager = nullptr;
    searchProgressManager = nullptr;
    searchProcess = nullptr;
    isSearchRunning = false;

    // Initialize catalog-related pointers
    catalogProgressManager = nullptr;
    catalogJobStoppable = nullptr;

    // Initialize backup management
    backupMappingManager = nullptr;

    //Default UI settings
        themeID = 1; //default theme is Katalog Colors
        selectedTab = 3; //default value for the first launch = Create screen.

        //Set up the statusbar timer
        statusBarTimer = new QTimer(this);
        statusBarTimer->setSingleShot(true);
        connect(statusBarTimer, &QTimer::timeout, this, [this]() {
            statusBar()->hide();
        });

    //Prepare paths, user setting file, check version
        //Get user home path and application dir path
            QStringList standardsPaths = QStandardPaths::standardLocations(QStandardPaths::HomeLocation);
            QString homePath = standardsPaths[0];
            QString applicationDirPath = QCoreApplication::applicationDirPath();

        //Define Setting file path and name
            //For portable mode, check if there is a settings file located with the executable
            collection->settingsFilePath = applicationDirPath + "/katalog_settings.ini";
            QFile settingsFile(collection->settingsFilePath);
            if(!settingsFile.exists()) {
                //otherwise fall back to default katalog_settings path
                collection->settingsFilePath = homePath + "/.config/katalog_settings.ini";
            }
            QSettings settings(collection->settingsFilePath, QSettings:: IniFormat);

        //Check for new version
            checkVersionChoice = settings.value("Settings/CheckVersion", true).toBool();
            if ( checkVersionChoice == true)
                checkVersion();

        //Prepare file extension cache
            FileMetadata::initializeExtensionTypeCache();

    //Set up and start database (modes: "Memory", "File", or "Hosted")
        Database::initialize(m_connectionName, collection);

    //Set up the interface globally
        //Set up the User Interface
            ui->setupUi(this);

            // Add Quit shortcut
            QShortcut *quitShortcut = new QShortcut(QKeySequence::Quit, this);
            connect(quitShortcut, &QShortcut::activated, qApp, &QApplication::quit);

            // Hide status bar initially
            statusBar()->hide();
            statusBarLabel = new QLabel(this);
            statusBar()->addWidget(statusBarLabel);

            //Splitter widget, invisible
            ui->splitter->setHandleWidth(0);

            // Restore window size and position
            setMinimumSize(720, 480);
            auto config = KSharedConfig::openConfig();
            KConfigGroup group = config->group("MainWindow");

            int savedWidth = group.readEntry("width", 1080);    // Generic key
            int savedHeight = group.readEntry("height", 720);   // Generic key
            int savedX = group.readEntry("x", 100);             // Generic key
            int savedY = group.readEntry("y", 100);             // Generic key

            QTimer::singleShot(100, this, [this, savedWidth, savedHeight, savedX, savedY]() {
                resize(savedWidth, savedHeight);
                move(savedX, savedY);
            });

            //Hide Development UI items
            hideDevelopmentUIItems();

            QButtonGroup buttonGroupDuplicates;
            buttonGroupDuplicates.addButton(ui->Search_radioButton_DuplicatesWithinSelectedDevice);
            buttonGroupDuplicates.addButton(ui->Search_radioButton_DuplicatesCompareTwoDevices);

            QButtonGroup buttonGroupDevices;
            buttonGroupDevices.addButton(ui->Devices_radioButton_DeviceTree);
            buttonGroupDevices.addButton(ui->Devices_radioButton_StorageList);
            buttonGroupDevices.addButton(ui->Devices_radioButton_CatalogList);

            QButtonGroup buttonGroupBackUp;
            buttonGroupBackUp.addButton(ui->BackUp_radioButton_Source);
            buttonGroupBackUp.addButton(ui->BackUp_radioButton_Target);

        //Settings screen
            ui->Settings_lineEdit_DatabaseFilePath->setText(collection->databaseFilePath);
            ui->Settings_comboBox_DatabaseMode->setItemData(0, "Memory", Qt::UserRole);
            ui->Settings_comboBox_DatabaseMode->setItemData(1, "File", Qt::UserRole);
            ui->Settings_comboBox_DatabaseMode->setItemData(2, "Hosted", Qt::UserRole);
            ui->Settings_lineEdit_DataMode_Hosted_HostName->setText(collection->databaseHostName);
            ui->Settings_lineEdit_DataMode_Hosted_DatabaseName->setText(collection->databaseName);
            ui->Settings_lineEdit_DataMode_Hosted_Port->setText(QVariant(collection->databasePort).toString());
            ui->Settings_lineEdit_DataMode_Hosted_UserName->setText(collection->databaseUserName);
            ui->Settings_lineEdit_DataMode_Hosted_Password->setText(collection->databasePassword);
            ui->Settings_label_VersionValue->setText(currentVersion);
            ui->Settings_label_DateValue->setText(releaseDate);

        //Load languages to the Settings combobox, keeping the user's selection
            QString userLanguage = settings.value("Settings/Language").toString();
            const QStringList supportedLanguages = Language::getSupportedLanguages();
            for (const QString& code : supportedLanguages) {
                ui->Settings_comboBox_Language->addItem(
                    Language::getFlagIcon(code),
                    code
                    );
            }
            ui->Settings_comboBox_Language->setCurrentText(userLanguage);

        //Hide some widgets by default
            ui->Statistics_calendarWidget->hide();
            ui->Devices_widget_Edit->hide();
            ui->Devices_widget_ReplaceCatalogsOption->hide();
            ui->Search_widget_DifferencesDevices->hide();
            ui->Create_widget_SambaSettings->hide();
            ui->Search_widget_Duplicates->hide();
            //Search: hide KRename option if not linux
            #ifndef Q_OS_LINUX
                ui->Search_comboBox_SelectProcess->removeItem(2);
            #endif

        //Hide file edtion items
            if( collection->databaseMode != "Memory"){
                ui->Devices_pushButton_EditList->hide();
                ui->Statistics_pushButton_EditDeviceStatisticsFile->hide();
                ui->Statistics_pushButton_Reload->hide();
                ui->Tags_pushButton_OpenTagsFile->hide();
            }

        //Load all other Settings and apply values
            loadSettings();

        //Load custom stylesheet
            //For windows, pick a windows common font.
            #ifdef Q_OS_WIN
            ui->tabWidget->setStyleSheet("font-family: calibri; font-size: 16px;");
            ui->splitter_widget_Filters->setStyleSheet("font-family: calibri; font-size: 16px;");
            #endif

            // Setup icon theme first (platform-specific)
            setupIconTheme();
// #ifdef QT_DEBUG
//             debugIconLoadingDetailed();
//             testIconSourceTracking();
// #endif
            //Load custom Katalog stylesheet instead of default theme
            if (themeID == 1) {
                if (QApplication::palette().color(QPalette::Window).lightness() < 128){
                    loadCustomThemeDark();
                }
                else {
                    loadCustomThemeLight();
                }
            }

    //Load Collection data
            //Create searchMemory initially
            searchJobStoppable = new SearchJobStoppable(this);
            // Both currentSearch and lastSearch point to searchMemory
            currentSearch = searchJobStoppable;
            lastSearch = searchJobStoppable;

        //Load Collection
            loadCollection();
            selectedDevice->loadDevice(m_connectionName);
            filterFromSelectedDevice();

            //Load mapping to backup tab
            loadBackUpMapping();

        //Restore last opened catalog to Explore tab
            if(ui->Settings_checkBox_LoadLastCatalog->isChecked()==true){
                exploreDevice->ID = settings.value("Explore/lastExploreDeviceID").toInt();
                exploreDevice->loadDevice(m_connectionName);
                exploreSelectedFolderFullPath = settings.value("Explore/lastExploreSelectedFolderFullPath").toString();
                exploreSelectedDirectoryName  = settings.value("Explore/lastExploreSelectedDirectoryName").toString();

                openCatalogToExplore();
            }

    //Preload last selected catalogs contents to memory
        if(ui->Settings_checkBox_PreloadCatalogs->isChecked()==true){
            preloadCatalogs();
        }

    //Setup tabs

        //Setup tab: Create
            //Default path to scan
            ui->Create_lineEdit_NewCatalogPath->setText("/");

            //Initiate comboboxes for metadata & checksum fields
            initiateFileTypeFields();
            initiateMetadataFields();
            initiateChecksumFields();
            initiateIncludeHiddenFields();

            //Always Load the file system for the treeview
            loadFileSystem("/");

            //Load the list of Storage devices for Create and Catalog tabs
            loadStorageList();

            //Set up a catalog manager for creation and updates, and device manager for udpate
            // Create catalog progress manager (will be connected to DeviceUpdateManager's CatalogManager)
            catalogProgressManager = new CatalogProgressManager(this);
            connect(catalogProgressManager, &CatalogProgressManager::statusMessageChanged,
                    this, [this](const QString &message, int timeout) {
                        statusBarLabel->setText(message);
                        statusBar()->show();
                        if (timeout > 0) {
                            statusBarTimer->start(timeout);
                        } else {
                            statusBarTimer->stop();
                        }
                    });
            setupDeviceUpdateManager();

        //Setup tab: Tags
            //Set Default path to scan
            ui->Tags_lineEdit_FolderPath->setText("/");

            loadFileSystemTags(newTagFolderPath);
            reloadTagsData();

        //Setup tab: Settings
            //Load path of last collection used
            ui->Settings_lineEdit_CollectionFolder->setText(collection->folder);
            ui->Settings_lineEdit_DatabaseFilePath->setText(collection->databaseFilePath);
            ui->Settings_lineEdit_ImageFolderPath->setText(collection->imageFolderPath);

            //Apply databaseMode in Settings tab
            QMap<QString, QString> databaseModeTranslations = {
                                                               {"Memory", tr("Memory")},
                                                               {"File",   tr("File")},
                                                               {"Hosted", tr("Hosted")},
                                                               };
            QString trMode = databaseModeTranslations.value(collection->databaseMode, collection->databaseMode);
            ui->Settings_comboBox_DatabaseMode->setCurrentText(trMode);
            applyDatabaseModeToUI();

            //Set apply button to disabled
            ui->Settings_pushButton_ApplyFolderpath->setEnabled(false);
            ui->Settings_pushButton_ApplyFilepath->setEnabled(false);

        //Setup tab: Explore
            ui->Explore_checkBox_DisplayFolders->setChecked(optionDisplayFolders);
            ui->Explore_checkBox_DisplaySubFolders->setChecked(optionDisplaySubFolders);

        //Setup tab: Storage
            unsavedChanges = false;

        //Setup tab: Devices

            ui->Devices_checkBox_DisplayCatalogs->setChecked(optionDisplayCatalogs);
            ui->Devices_checkBox_DisplayStorage->setChecked(optionDisplayStorage);
            ui->Devices_checkBox_DisplayPhysicalGroup->setChecked(optionDisplayPhysicalGroup);
            ui->Devices_checkBox_DisplayVirtualGroups->setChecked(optionDisplayVirtualGroups);
            ui->Devices_checkBox_DisplayFullTable->setChecked(optionDisplayFullDeviceTable);
            loadParentsList();

            QString displayContents = settings.value("Devices/DisplayContents").toString();
            if(displayContents=="Tree"){
                ui->Devices_radioButton_DeviceTree->setChecked(true);
                ui->Catalogs_pushButton_UpdateAllActive->setEnabled(false);
            } else if(displayContents=="Storage"){
                ui->Devices_radioButton_StorageList->setChecked(true);
                ui->Catalogs_pushButton_UpdateAllActive->setEnabled(false);
            } else if(displayContents=="Catalogs"){
                ui->Devices_radioButton_CatalogList->setChecked(true);
                ui->Catalogs_pushButton_UpdateAllActive->setEnabled(true);
            }

            // After setting the radio button state:
            if(ui->Devices_radioButton_DeviceTree->isChecked())
                loadDevicesView("Tree");
            else if(ui->Devices_radioButton_StorageList->isChecked())
                loadDevicesView("Storage");
            else if(ui->Devices_radioButton_CatalogList->isChecked())
                loadDevicesView("Catalogs");

        //Setup tab: Search
            //Default values
            initiateSearchFields();
            resetToDefaultSearchCriteria();
            setSearchStateIdle();

            //Load an empty model to display headers
            Catalog *emptyCatalog = new Catalog(this);
            ui->Search_treeView_FilesFound->setModel(emptyCatalog);
            QStandardItemModel *emptyQStandardItemModel = new QStandardItemModel;
            emptyQStandardItemModel->setHorizontalHeaderLabels({ tr("Catalog with results")});
            ui->Search_treeView_CatalogsFound->setModel(emptyQStandardItemModel);

            //Restore last Search values
            QSqlQuery query(QSqlDatabase::database(m_connectionName));
            QString querySQL = QLatin1String(R"(
                                SELECT MAX(date_time)
                                FROM search
                            )");
            query.prepare(querySQL);
            query.exec();
            query.next();

            lastSearch->searchDateTime = query.value(0).toString();
            lastSearch->loadSearchHistoryCriteria(m_connectionName);
            loadSearchCriteria(lastSearch);

            //Restore last Search values
            filterFromSelectedDevice();

            //set up search Manager
            setupSearchManager();

            //Set up search throttler
            searchResultsThrottler = new SearchResultsThrottler(this);
            connect(searchResultsThrottler, &SearchResultsThrottler::updateDisplay,
                    this, &MainWindow::displaySearchResults);

        //Setup tab: BackUp
            ui->BackUp_checkBox_DisplayFullTable->setChecked(optionDisplayFullMappingTable);
            setupBackUpManager();

    //Context menu and other slots and signals
            setupFileContextMenus();

            //Header Order change
            connect(ui->Devices_treeView_DeviceList->header(), &QHeaderView::sortIndicatorChanged,
                    this, &::MainWindow::on_DevicesTreeViewDeviceListHeaderSortOrderChanged);

            connect(ui->Explore_treeView_FileList->header(), &QHeaderView::sortIndicatorChanged,
                    this, &::MainWindow::on_ExploreTreeViewFileListHeaderSortOrderChanged);

            connect(ui->Search_treeView_FilesFound->header(), &QHeaderView::sortIndicatorChanged,
                    this, &::MainWindow::on_SearchTreeViewFilesFoundHeaderSortOrderChanged);

            connect(ui->Search_treeView_History->header(), &QHeaderView::sortIndicatorChanged,
                    this, &::MainWindow::on_SearchTreeViewHistoryHeaderSortOrderChanged);

    //Restore sorting of views
            ui->Devices_treeView_DeviceList->QTreeView::sortByColumn(lastDevicesSortSection,Qt::SortOrder(lastDevicesSortOrder));
            ui->Explore_treeView_FileList->QTreeView::sortByColumn(lastExploreSortSection,Qt::SortOrder(lastExploreSortOrder));
            ui->Search_treeView_FilesFound->QTreeView::sortByColumn(lastSearchSortSection,Qt::SortOrder(lastSearchSortOrder));
            ui->Search_treeView_History->QTreeView::sortByColumn(lastSearchHistorySortSection,Qt::SortOrder(lastSearchHistorySortOrder));
}

MainWindow::~MainWindow()
{
    if (catalogJobStoppable) {
        catalogJobStoppable->stopCatalogOperation();
        catalogJobStoppable->deleteLater();
    }

    // delete collection;  // Comment out
    delete selectedDevice;  // Test this alone
}

void MainWindow::closeEvent (QCloseEvent *event)
{
    if(unsavedChanges == true){

        int result = QMessageBox::warning(this,"Katalog",
                    tr( "Some changes in the Storage device list have not been saved.<br/>"
                        "Save and exit, discard and exit, or cancel exit?"),
                    QMessageBox::Save|QMessageBox::Discard|QMessageBox::Cancel);
        if ( result ==QMessageBox::Cancel){
            event->ignore();
            return;
        }
        else if ( result ==QMessageBox::Save){
            //Save model data to Storage file
            collection->saveStorageTableToFile();

            //Reload Storage file data to table
            collection->loadStorageFileToTable();
        }
        // Discard: fall through to shutdown
    }

    // Stop backup thread if running so it doesn't outlive the window
    if (m_backupThread && m_backupThread->isRunning()) {
        if (m_backupJob)
            m_backupJob->stopBackup();
        m_backupThread->quit();
        m_backupThread->wait(5000);
    }

    //Save window size and position
    auto config = KSharedConfig::openConfig();
    KConfigGroup group = config->group("MainWindow");

    // Save with generic keys - works on any screen
    group.writeEntry("width", size().width());
    group.writeEntry("height", size().height());
    group.writeEntry("x", pos().x());
    group.writeEntry("y", pos().y());
    group.sync();

    // Explicitly close and remove all database connections before exit.
    // Without this, SQLite WAL checkpoints or KDE/Qt cleanup threads can keep
    // the process alive in the Windows Task Manager "Background processes" list.
    {
        const QStringList connections = QSqlDatabase::connectionNames();
        for (const QString &conn : connections) {
            QSqlDatabase::database(conn, false).close();
            QSqlDatabase::removeDatabase(conn);
        }
    }

    event->accept();
    // MainWindow is stack-allocated (WA_DeleteOnClose = false), so KMainWindow's
    // normal quit-on-delete path never fires. Quit the event loop explicitly.
    QApplication::quit();
}
