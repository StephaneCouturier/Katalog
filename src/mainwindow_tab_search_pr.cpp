/*LICENCE
    This file is part of Katalog

    Copyright (C) 2021, the Katalog Development team

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
// File Name:   mainwindow_tab_search_pr.cpp
// Purpose:     methods for the screen SEARCH and the search process itself
// Description: https://stephanecouturier.github.io/Katalog/docs/Features/Search
// Author:      Stephane Couturier
/////////////////////////////////////////////////////////////////////////////
*/

#include "mainwindow.h"
#include "src/filesview.h"
#include "src/ui_mainwindow.h"

//----------------------------------------------------------------------
// Search management
//----------------------------------------------------------------------
void MainWindow::launchSearch()
{// Generic search launch method, using a type of search based on database mode and search type

    // Set animation cursor before starting
    QApplication::setOverrideCursor(Qt::WaitCursor);

    // For memory mode without searchInConnected, use SearchMemory
    if (collection->databaseMode == "Memory" && !ui->Filters_checkBox_SearchInConnectedDrives->isChecked()) {
        qDebug() << "\n Using SearchMemory for memory mode";
        launchSearchMemory();
    }

    // For database mode or searchInConnected, in development mode, use SearchJobStoppable
    else //if (collection->databaseMode != "Memory" && !ui->Filters_checkBox_SearchInConnectedDrives->isChecked())
    {
        qDebug() << "\n Development mode: using SearchJobStoppable";
        launchSearchJobStoppable();
        return;
    }

    // (OBSOLETE) For database mode or searchInConnected, use SearchStoppable
    // else {
    //     qDebug() << "\n Using SearchStoppable for database mode or searchInConnected";
    //     launchSearchStoppable();
    // }

    //Save search history
    currentSearch->saveSearchHistoryToTable("defaultConnection");
}
//----------------------------------------------------------------------
void MainWindow::launchSearchStoppable()
{
    // If a search is running, stop it
    if (isSearchRunning) {
        // If a search is running, stop it
        if (searchStoppable) {
            searchStoppable->stopSearch();
        }

        isSearchRunning = false;

        // Reset "Search" button
        ui->Search_pushButton_Search->setText("Search");
        ui->Search_pushButton_Search->setIcon(QIcon::fromTheme("edit-find"));
        ui->Search_pushButton_Search->setStyleSheet("QPushButton{ background-color: #81d41a; }");

        QApplication::restoreOverrideCursor();
        return;
    }

    // Clear the search view before starting a new search
    resetSearchState();

    // Create database search object if not already created
    if (!searchStoppable) {
        searchStoppable = new SearchStoppable(this);

        // Connect the progress signal
        connect(searchStoppable, &Search::searchProgress, this, &MainWindow::updateSearchProgress);
    }

    // Point the currentSearch to searchStoppable for consistent access
    currentSearch = searchStoppable;

    // Transfer search parameters from UI to the search object
    sendSearchParameters(searchStoppable);

    // Set the search as running
    isSearchRunning = true;

    // Change "Search" button to "Stop" for stoppable searches
    ui->Search_pushButton_Search->setText("Stop");
    ui->Search_pushButton_Search->setIcon(QIcon::fromTheme("process-stop"));
    ui->Search_pushButton_Search->setStyleSheet("QPushButton{ background-color: #ff8000; }");

    // Initialize the database
    if (!searchStoppable->initializeDatabase()) {
        QMessageBox::warning(this, "Katalog", tr("Failed to initialize database connection."));

        // Reset state if initialization fails
        isSearchRunning = false;
        ui->Search_pushButton_Search->setText("Search");
        ui->Search_pushButton_Search->setIcon(QIcon::fromTheme("edit-find"));
        ui->Search_pushButton_Search->setStyleSheet("QPushButton{ background-color: #81d41a; }");

        QApplication::restoreOverrideCursor();
        return;
    }

    // Run the search in the main thread
    searchStoppable->searchFiles(selectedDevice);

    // If we get here, the search is complete
    handleSearchCompleted();

    // Adapt display of files found for searchInConnected
    if (currentSearch && currentSearch->searchInConnectedChecked == true) {
        ui->Search_treeView_FilesFound->model()->setHeaderData(4, Qt::Horizontal, tr("Source Directory"));
        ui->Search_treeView_FilesFound->header()->resizeSection(4, 400);
        ui->Search_treeView_FilesFound->header()->hideSection(5);
    }
}
//----------------------------------------------------------------------
void MainWindow::launchSearchMemory()
{
    // Create memory search object if not already created
    if (!searchMemory) {
        searchMemory = new SearchMemory(this);
    }

    // Always connect the signal, whether searchMemory is new or existing
    // First disconnect any existing connections to avoid duplicates
    disconnect(searchMemory, &Search::searchProgress, this, &MainWindow::updateSearchProgress);
    connect(searchMemory, &Search::searchProgress, this, &MainWindow::updateSearchProgress);

    // Point the currentSearch to searchMemory for consistent access
    currentSearch = searchMemory;

    // Transfer search parameters from UI to the search object
    sendSearchParameters(searchMemory);

    // Set the search as running, but DON'T change button appearance for memory mode
    // since it can't be stopped
    //isSearchRunning = true;

    // Run the search
    searchMemory->searchFiles(selectedDevice);

    // Display the results
    displaySearchResults();

    // Reset the search state when search is complete
    isSearchRunning = false;

    // Restore cursor and enable export buttons
    QApplication::restoreOverrideCursor();
    ui->Search_pushButton_ProcessResults->setEnabled(true);
    ui->Search_comboBox_SelectProcess->setEnabled(true);
}
//----------------------------------------------------------------------
void MainWindow::setupSearchManager()
{
    // Create search manager
    searchManager = new SearchManager(this);

    // Create progress manager with timer reference
    searchProgressManager = new SearchProgressManager(statusBar(), statusBarTimer, this);
    searchProgressManager->connectToSearchManager(searchManager);

    // Remove old timer connections since SearchProgressManager handles it internally now
    // connect(searchManager, &SearchManager::searchCompleted, this, &MainWindow::startStatusBarTimer);
    // connect(searchManager, &SearchManager::searchCancelled, this, &MainWindow::startStatusBarTimer);

    // Keep existing search lifecycle connections:
    connect(searchManager, &SearchManager::searchCompleted,       this, &MainWindow::onSearchCompleted);
    connect(searchManager, &SearchManager::searchCancelled,       this, &MainWindow::onSearchCancelled);
    connect(searchManager, &SearchManager::searchError,           this, &MainWindow::onSearchError);

    connect(searchManager, &SearchManager::searchCompleted, this, [this]() {
        QApplication::restoreOverrideCursor();
        if (currentSearch && currentSearch->fileNames.size() > 0) {
            displaySearchResults();
        }
    });

    connect(searchManager, &SearchManager::searchCancelled, this, [this]() {
        QApplication::restoreOverrideCursor();
        if (currentSearch && currentSearch->fileNames.size() > 0) {
            displaySearchResults();
        }
    });

    connect(searchManager, &SearchManager::searchError, this, [this](const QString &error) {
        QApplication::restoreOverrideCursor();
        QMessageBox::warning(this, tr("Search Error"), error);
    });
}
//----------------------------------------------------------------------
void MainWindow::launchSearchJobStoppable()
{
    qDebug() << "=== launchSearchJobStoppable() called ===";

    if (!searchManager) {
        qDebug() << "ERROR: SearchManager not initialized";
        return;
    }

    // Clear the search view before starting a new search
    clearSearchResults();

    // Show status bar for SearchJobStoppable
    statusBar()->show();

    // Create SearchJobStoppable
    SearchJobStoppable* searchJobStoppable = new SearchJobStoppable(this);
    searchJobStoppable->setDatabaseConnection("defaultConnection");
    currentSearch = searchJobStoppable;

    // Update progress manager with current search for file count display
    if (searchProgressManager) {
        searchProgressManager->setCurrentSearch(currentSearch);
    }

    // Set up basic search parameters
    sendSearchParameters(searchJobStoppable);

    // Update UI for running search (Search button becomes Pause)
    setSearchStateRunning();

    // Start the search
    qDebug() << "Starting SearchJobStoppable via SearchManager";
    searchManager->startSearchJobStoppable(searchJobStoppable, selectedDevice);

    searchResultsThrottler->setCurrentSearch(searchJobStoppable);
    connect(searchJobStoppable, &Search::searchProgress,
            searchResultsThrottler, &SearchResultsThrottler::onSearchProgress);

    qDebug() << "=== launchSearchJobStoppable() complete ===";
}
//----------------------------------------------------------------------
//----------------------------------------------------------------------
// Send search parameters from UI to the search object
void MainWindow::sendSearchParameters(Search *search)
{
    if (!search) return;

    // Clear any existing results
    search->clearResults();

    // Transfer all parameters from UI to the search object
    search->searchOnFileName = ui->Search_checkBox_FileName->isChecked();
    search->searchText = ui->Search_lineEdit_SearchText->text();
    search->selectedTextCriteria = ui->Search_comboBox_TextCriteria->itemData(
                                               ui->Search_comboBox_TextCriteria->currentIndex(), Qt::UserRole).toString();
    search->selectedSearchIn = ui->Search_comboBox_SearchIn->itemData(
                                                                      ui->Search_comboBox_SearchIn->currentIndex(), Qt::UserRole).toString();    search->caseSensitive = ui->Search_checkBox_CaseSensitive->isChecked();
    search->selectedSearchExclude = ui->Search_lineEdit_Exclude->text();

    search->searchOnFileCriteria = ui->Search_checkBox_FileCriteria->isChecked();
    search->searchOnSize = ui->Search_checkBox_Size->isChecked();
    search->selectedMinimumSize = ui->Search_spinBox_MinimumSize->value();
    search->selectedMaximumSize = ui->Search_spinBox_MaximumSize->value();
    search->selectedMinSizeUnit = ui->Search_comboBox_MinSizeUnit->itemData(ui->Search_comboBox_MinSizeUnit->currentIndex(), Qt::UserRole).toString();
    search->selectedMaxSizeUnit = ui->Search_comboBox_MaxSizeUnit->itemData(ui->Search_comboBox_MaxSizeUnit->currentIndex(), Qt::UserRole).toString();
    search->setMultipliers();
    search->searchOnType = ui->Search_checkBox_Type->isChecked();
    search->selectedFileType = ui->Search_comboBox_FileType->itemData(ui->Search_comboBox_FileType->currentIndex(), Qt::UserRole).toString();
    search->searchOnDate = ui->Search_checkBox_Date->isChecked();
    search->selectedDateMin = ui->Search_dateTimeEdit_Min->dateTime();
    search->selectedDateMax = ui->Search_dateTimeEdit_Max->dateTime();
    search->searchOnDuplicates = ui->Search_checkBox_Duplicates->isChecked();
    search->searchDuplicatesOnName = ui->Search_checkBox_DuplicatesName->isChecked();
    search->searchDuplicatesOnSize = ui->Search_checkBox_DuplicatesSize->isChecked();
    search->searchDuplicatesOnDate = ui->Search_checkBox_DuplicatesDateModified->isChecked();
    search->searchOnDifferences = ui->Search_checkBox_Differences->isChecked();
    search->differencesOnName = ui->Search_checkBox_DifferencesName->checkState();
    search->differencesOnSize = ui->Search_checkBox_DifferencesSize->checkState();
    search->differencesOnDate = ui->Search_checkBox_DifferencesDateModified->checkState();
    search->differencesDeviceID1 = ui->Search_comboBox_DifferencesDevice1->currentData().toInt();
    search->differencesDeviceID2 = ui->Search_comboBox_DifferencesDevice2->currentData().toInt();
    search->differencesDevices.clear();
    search->differencesDevices << QString::number(search->differencesDeviceID1) << QString::number(search->differencesDeviceID2);

    search->searchOnFolderCriteria = ui->Search_checkBox_FolderCriteria->isChecked();
    search->showFoldersOnly = ui->Search_checkBox_ShowFolders->isChecked();
    search->searchOnTags = ui->Search_checkBox_Tags->isChecked();
    search->selectedTagName = ui->Search_comboBox_Tags->currentText();

    search->selectedStorage = ui->Filters_label_DisplayStorage->text();
    search->selectedCatalog = ui->Filters_label_DisplayCatalog->text();
    search->searchInCatalogsChecked = ui->Filters_checkBox_SearchInCatalogs->isChecked();
    search->searchInConnectedChecked = ui->Filters_checkBox_SearchInConnectedDrives->isChecked();
    search->connectedDirectory = ui->Filters_lineEdit_SeletedDirectory->text();

    // Transfer file type lists
    search->fileType_AudioS = fileType_AudioS;
    search->fileType_ImageS = fileType_ImageS;
    search->fileType_TextS = fileType_TextS;
    search->fileType_VideoS = fileType_VideoS;

    // Initialize the differences devices
    if (search->diffDevice1 == nullptr) {
        search->diffDevice1 = new Device;
    }
    search->diffDevice1->ID = ui->Search_comboBox_DifferencesDevice1->currentData().toInt();

    if (search->diffDevice2 == nullptr) {
        search->diffDevice2 = new Device;
    }
    search->diffDevice2->ID = ui->Search_comboBox_DifferencesDevice2->currentData().toInt();
}
//----------------------------------------------------------------------
// Display the search results in the UI
void MainWindow::displaySearchResults()
{
    if (!currentSearch) {
        qWarning() << "displaySearchResults: currentSearch is null";
        return;
    }

    // Calculate statistics first
    currentSearch->calculateStatistics();

    // Process results to populate the catalog model
    currentSearch->processResults(currentSearch->showFoldersOnly);

    // List of catalogs in which results were found
    if (currentSearch->deviceFoundModel) {
        currentSearch->deviceFoundModel->setHorizontalHeaderLabels({ QCoreApplication::translate("MainWindow", "Catalog with results") });
        ui->Search_treeView_CatalogsFound->setModel(currentSearch->deviceFoundModel);      
        ui->Search_treeView_CatalogsFound->hideColumn(1);
    } else {
        qWarning() << "displaySearchResults: deviceFoundModel is null";
        // Create an empty model
        QStandardItemModel* emptyQStandardItemModel = new QStandardItemModel(this);
        emptyQStandardItemModel->setHorizontalHeaderLabels({ tr("Catalog with results")});
        ui->Search_treeView_CatalogsFound->setModel(emptyQStandardItemModel);
    }

    // Configure the files view
    FilesView *fileViewModel = new FilesView(this);
    fileViewModel->caseSensitive = fileSortCaseSensitive;
    fileViewModel->setSourceModel(currentSearch);
    ui->Search_treeView_FilesFound->setModel(fileViewModel);

    // Configure the view based on the search type
    if (currentSearch->searchOnFolderCriteria == true && currentSearch->showFoldersOnly == true) {
        // Folder view setup
        ui->Search_treeView_FilesFound->header()->setSectionResizeMode(3, QHeaderView::ResizeToContents); // Path
        ui->Search_treeView_FilesFound->header()->hideSection(0); // Name
        ui->Search_treeView_FilesFound->header()->hideSection(1); // Size
        ui->Search_treeView_FilesFound->header()->hideSection(2); // Date
        ui->Search_label_FoundTitle->setText(tr("Folders found"));
    }
    else {
        // File view setup
        ui->Search_treeView_FilesFound->header()->setSectionResizeMode(QHeaderView::Interactive);
        ui->Search_treeView_FilesFound->header()->setSectionResizeMode(0, QHeaderView::ResizeToContents); // Name
        ui->Search_treeView_FilesFound->header()->resizeSection(1, 110); // Size
        ui->Search_treeView_FilesFound->header()->resizeSection(2, 140); // Date
        ui->Search_treeView_FilesFound->header()->resizeSection(3, 400); // Path
        ui->Search_treeView_FilesFound->header()->resizeSection(4, 140); // Catalog
        ui->Search_treeView_FilesFound->header()->showSection(0);
        ui->Search_treeView_FilesFound->header()->showSection(1);
        ui->Search_treeView_FilesFound->header()->showSection(2);

        // Update label based on search type
        if (currentSearch->searchOnFileCriteria == true && currentSearch->searchOnDuplicates == true) {
            ui->Search_label_FoundTitle->setText(tr("Duplicates found"));
        }
        else if (currentSearch->searchOnFileCriteria == true && currentSearch->searchOnDifferences == true) {
            ui->Search_label_FoundTitle->setText(tr("Differences found"));
        }
        else {
            ui->Search_label_FoundTitle->setText(tr("Files found"));
        }
    }

    // Update results count and size display
    ui->Search_label_NumberResults->setText(QLocale().toString(currentSearch->filesFoundNumber));
    ui->Search_label_SizeResults->setText(QLocale().formattedDataSize(currentSearch->filesFoundTotalSize));

    // Enable/disable the statistics button based on results
    ui->Search_pushButton_FileFoundMoreStatistics->setEnabled(currentSearch->filesFoundNumber > 0);

    // Save the search history and refresh UI
    collection->saveSearchHistoryTableToFile();
    loadSearchHistoryTableToModel();

    // Enable Export
    ui->Search_pushButton_ProcessResults->setEnabled(true);
    ui->Search_comboBox_SelectProcess->setEnabled(true);

    QApplication::restoreOverrideCursor();
}
//----------------------------------------------------------------------
void MainWindow::handleSearchCompleted()
{
    // Reset the search button
    ui->Search_pushButton_Search->setText("Search");
    ui->Search_pushButton_Search->setIcon(QIcon::fromTheme("edit-find"));
    ui->Search_pushButton_Search->setStyleSheet("QPushButton{ background-color: #81d41a; }");

    isSearchRunning = false;

    // Display the results
    displaySearchResults();

    // Enable export
    ui->Search_pushButton_ProcessResults->setEnabled(true);
    ui->Search_comboBox_SelectProcess->setEnabled(true);

    // Stop animation
    QApplication::restoreOverrideCursor();
}
//----------------------------------------------------------------------
void MainWindow::handleSearchStopped()
{
    // Reset the search button
    ui->Search_pushButton_Search->setText("Search");
    ui->Search_pushButton_Search->setIcon(QIcon::fromTheme("edit-find"));
    ui->Search_pushButton_Search->setStyleSheet("QPushButton{ background-color: #81d41a; }");

    isSearchRunning = false;

    // Make sure statistics are calculated for partial results
    if (currentSearch) {
        currentSearch->calculateStatistics();
    }

    // Display the results, even though they're partial
    displaySearchResults();

    // Enable export
    ui->Search_pushButton_ProcessResults->setEnabled(true);
    ui->Search_comboBox_SelectProcess->setEnabled(true);

    // Stop animation
    QApplication::restoreOverrideCursor();

    // No need to update the status bar here, the final progress signal (-1) will do that
}
//----------------------------------------------------------------------
void MainWindow::updateSearchProgress(int filesProcessed)
{    // Special values for different states:
    // -1: Search interrupted
    // -2: Catalog loading started
    // -3: Catalog loading finished, processing files
    // -4: Catalog loading progress update (only used by SearchMemory)

    // Update totalFilesProcessed for statistics
    if (filesProcessed >= 0) {
        currentSearch->totalFilesProcessed = filesProcessed;
    }

    // Build status message
    QString statusMessage;

    // Special case for interrupted search (-1)
    if (filesProcessed == -1) {
        if (currentSearch) {
            statusMessage = tr("Search interrupted | Files found: %1 | Files processed: %2")
                                .arg(QLocale().toString(currentSearch->fileNames.size()))
                                .arg(QLocale().toString(currentSearch->totalFilesProcessed));
        } else {
            statusMessage = tr("Search interrupted. No results available.");
        }

        statusBar()->showMessage(statusMessage, 5000);
        return;
    }

    // Special case for catalog loading started (-2)
    if (filesProcessed == -2) {
        if (currentSearch) {
            statusMessage = tr("Loading Catalog %1 of %2 (%3) | Files found: %4 | Files processed: %5")
                .arg(currentSearch->currentCatalogIndex)
                .arg(currentSearch->totalCatalogs)
                .arg(currentSearch->currentCatalogName)
                .arg(QLocale().toString(currentSearch->fileNames.size())
                .arg(QLocale().toString(currentSearch->totalFilesProcessed)));

            statusBar()->showMessage(statusMessage);
        }
        return;
    }

    // Special case for catalog loading progress update (-4) - only SearchMemory uses this
    if (filesProcessed == -4) {
        SearchMemory* searchMemory = dynamic_cast<SearchMemory*>(currentSearch);
        if (searchMemory) {
            double percentLoaded = 0;
            if (searchMemory->currentCatalogTotalFiles > 0) {
                percentLoaded = (double)searchMemory->currentCatalogFilesLoaded /
                                searchMemory->currentCatalogTotalFiles * 100.0;
            }

            statusMessage = tr("Loading Catalog %1 of %2 (%3) | %4 files loaded (%5%) | Files found: %6 | Files processed: %7")
                                .arg(searchMemory->currentCatalogIndex)
                                .arg(searchMemory->totalCatalogs)
                                .arg(searchMemory->currentCatalogName)
                                .arg(QLocale().toString(searchMemory->currentCatalogFilesLoaded))
                                .arg(QString::number(percentLoaded, 'f', 1))
                                .arg(QLocale().toString(currentSearch->fileNames.size())
                                .arg(QLocale().toString(currentSearch->totalFilesProcessed)));

            statusBar()->show();
            statusBar()->showMessage(statusMessage, 5000);
            statusBarTimer->start(5000);
            QCoreApplication::processEvents();
        }
        return;
    }

    // Special case for catalog loading finished (-3)
    if (filesProcessed == -3) {
        if (currentSearch) {
            statusMessage = tr("Processing Catalog %1 of %2 | Files found: %3 | Processing files...")
            .arg(currentSearch->currentCatalogIndex)
                .arg(currentSearch->totalCatalogs)
                .arg(QLocale().toString(currentSearch->fileNames.size()));

            statusBar()->show();
            statusBar()->showMessage(statusMessage, 5000);
            statusBarTimer->start(5000);
        }
        return;
    }

    // Regular progress update
    if (currentSearch) {
        // If we have catalog information, show it
        if (currentSearch->totalCatalogs > 0) {
            if (!ui->Filters_checkBox_SearchInConnectedDrives->isChecked()){
                statusMessage = tr("Searching in Catalog %1 of %2 | ")
                                    .arg(currentSearch->currentCatalogIndex)
                                    .arg(currentSearch->totalCatalogs);
            }
            statusMessage += tr("Files found: %1 | Files processed: %2")
                                 .arg(QLocale().toString(currentSearch->fileNames.size()))
                                 .arg(QLocale().toString(filesProcessed));
        } else {
            // Otherwise just show files
            statusMessage = tr("Files found: %1 | Files processed: %2")
                                .arg(QLocale().toString(currentSearch->fileNames.size()))
                                .arg(QLocale().toString(filesProcessed));
        }

        // Calculate percentage if we have an estimate
        if (currentSearch->estimatedTotalFiles > 0) {
            int percentComplete = qMin(100,
                                       static_cast<int>((filesProcessed * 100) / currentSearch->estimatedTotalFiles));
            statusMessage += tr(" (%1%)").arg(percentComplete);
        }
    }

    // Show status message with 5 second timeout
    statusBar()->show();
    statusBar()->showMessage(statusMessage, 5000);
    statusBarTimer->start(5000);

    // Process events to keep UI responsive
    QCoreApplication::processEvents();
}

void MainWindow::reportSearchStatistics()
{

    // Check if we have current search results
    if (!currentSearch) {
        QMessageBox::warning(this, "Katalog", tr("No search results available."));
        return;
    }

    // Create header text
    QString headerText;
    // Depending on the search type, we can show either files or folders statistics
    if (currentSearch->showFoldersOnly) {
        // Show folders statistics
        headerText = "<br/><b>" + tr("Folders Found Statistics") + "</b><br/>";
    } else {
        // Show files statistics
        headerText = "<br/><b>" + tr("Files Found Statistics") + "</b><br/>";
    }

    QString filesProcessedText;
    if(!currentSearch->showFoldersOnly){
        headerText += "<br/>"
                      + tr("Catalogs processed: %1 of %2")
                            .arg(currentSearch->currentCatalogIndex)
                            .arg(currentSearch->totalCatalogs)
                      + "<br/>";
    }

    // Check if the search was interrupted - works with both SearchStoppable and SearchJobStoppable
    bool wasInterrupted = (currentSearch &&
                           !isSearchRunning &&
                           currentSearch->wasStopRequested());

    if (wasInterrupted) {
        headerText += "<i>" + tr("Interrupted Search, incomplete results") + "</i><br/>";

        // Add files processed info
        filesProcessedText = tr("<tr><td>Files processed: </td><td><b> %1 </b></td></tr>")
                                 .arg(QLocale().toString(currentSearch->totalFilesProcessed));

        // Show percentage if we have an estimate
        if (currentSearch->estimatedTotalFiles > 0) {
            int percentProcessed = (currentSearch->totalFilesProcessed * 100) / currentSearch->estimatedTotalFiles;
            filesProcessedText += tr("<tr><td>Percentage processed: </td><td><b> %1 %</b></td></tr>")
                                      .arg(percentProcessed);
        }
    } else {
        // For complete searches, show total files processed
        filesProcessedText = tr("<tr><td>Files processed: </td><td><b> %1 </b></td></tr>")
                                 .arg(QLocale().toString(currentSearch->totalFilesProcessed));
    }

    // Add the statistics for files or folders found
    if (currentSearch->showFoldersOnly) {
        // Show folders statistics
        headerText += tr("<table><tr><td>Folders found:  </td><td><b> %1 </b> </td></tr>")
                          .arg(QLocale().toString(currentSearch->filesFoundNumber));
    } else {
        // Show files statistics
        headerText += tr("<table><tr><td>Files found:  </td><td><b> %1 </b> </td></tr>")
                          .arg(QLocale().toString(currentSearch->filesFoundNumber));
    }

    // Add files processed
    headerText += filesProcessedText;

    // Add files statistics
    if(!currentSearch->showFoldersOnly){
        headerText +=
            tr("<tr></tr>"
               "<tr><td>Total size:   </td><td><b> %1 </b>  </td></tr>"
               "<tr><td>Min size:     </td><td><b> %3 </b>  </td></tr>"
               "<tr><td>Max size:     </td><td><b> %4 </b>  </td></tr>"
               "<tr><td>Average size: </td><td><b> %2 </b>  <br/></td></tr>"
               "<tr><td>Min Date:     </td><td><b> %5 </b>  </td></tr>"
               "<tr><td>Max Date:     </td><td><b> %6 </b>  </td></tr>"
               "</table>")
                .arg(QLocale().formattedDataSize(currentSearch->filesFoundTotalSize),
                     QLocale().formattedDataSize(currentSearch->filesFoundAverageSize),
                     QLocale().formattedDataSize(currentSearch->filesFoundMinSize),
                     QLocale().formattedDataSize(currentSearch->filesFoundMaxSize),
                     currentSearch->filesFoundMinDate,
                     currentSearch->filesFoundMaxDate);
    }

    // Show the message box with statistics
    QMessageBox msgBox;
    msgBox.setWindowTitle("Katalog");
    msgBox.setText(headerText);
    msgBox.setIcon(QMessageBox::Information);
    msgBox.exec();
}
//----------------------------------------------------------------------
void MainWindow::resetSearchState()
{
    // Reset any active searches
    isSearchRunning = false;

    // Reset the search button
    ui->Search_pushButton_Search->setText("Search");
    ui->Search_pushButton_Search->setIcon(QIcon::fromTheme("edit-find"));
    ui->Search_pushButton_Search->setStyleSheet("QPushButton{ background-color: #81d41a; }");

    // Reset the current search object
    if (currentSearch) {
        currentSearch->clearResults();
    }

    // Clear any models or views
    if (ui->Search_treeView_FilesFound->model()) {
        // Create an empty model to replace the existing one
        QStandardItemModel* emptyQStandardItemModel = new QStandardItemModel(this);
        emptyQStandardItemModel->setHorizontalHeaderLabels({ tr("Name"), tr("Size"), tr("Date"), tr("Folder"), tr("Catalog Name"), tr("Catalog ID")});
        ui->Search_treeView_FilesFound->setModel(emptyQStandardItemModel);
    }

    if (ui->Search_treeView_CatalogsFound->model()) {
        QStandardItemModel* emptyQStandardItemModel = new QStandardItemModel(this);
        emptyQStandardItemModel->setHorizontalHeaderLabels({ tr("Catalog with results")});
        ui->Search_treeView_CatalogsFound->setModel(emptyQStandardItemModel);
    }
}
//----------------------------------------------------------------------
//----------------------------------------------------------------------
void MainWindow::updateStatusBarFromSearchManager()
{
    if (!searchManager) {
        statusBar()->showMessage(tr("Ready"));
        return;
    }

    QString statusMessage;

    if (searchManager->searchRunning()) {
        // Build dynamic status message while search is running
        statusMessage = searchManager->status();

        // Add progress percentage if available
        if (searchManager->progress() > 0) {
            statusMessage += QString(" (%1%)").arg(searchManager->progress());
        }

        // Add current catalog info if available
        if (!searchManager->currentCatalogName().isEmpty()) {
            statusMessage += QString(" - %1").arg(searchManager->currentCatalogName());
        }

        // Add file count if currentSearch is available
        if (currentSearch && currentSearch->fileNames.size() > 0) {
            statusMessage += QString(" | Files found: %1")
            .arg(QLocale().toString(currentSearch->fileNames.size()));
        }
    } else {
        // Search not running - show final status or ready state
        if (searchManager->status() == "Ready") {
            if (currentSearch && currentSearch->fileNames.size() > 0) {
                statusMessage = tr("Search completed | Files found: %1")
                .arg(QLocale().toString(currentSearch->fileNames.size()));
            } else {
                statusMessage = tr("Ready");
            }
        } else {
            statusMessage = searchManager->status();
        }
    }

    // Update the status bar
    statusBar()->showMessage(statusMessage);
}

//----------------------------------------------------------------------
void MainWindow::pauseCurrentSearch()
{
    if (!currentSearch) {
        qDebug() << "No current search to pause";
        return;
    }

    SearchJobStoppable* searchJobStoppable = qobject_cast<SearchJobStoppable*>(currentSearch);
    if (!searchJobStoppable) {
        qDebug() << "Current search doesn't support pause/resume";
        return;
    }

    if (!searchManager || !searchManager->searchRunning()) {
        qDebug() << "Search is not running - resetting to idle";
        setSearchStateIdle();
        return;
    }

    searchJobStoppable->pauseSearch();
    setSearchStatePaused();
}

void MainWindow::resumeCurrentSearch()
{
    if (!currentSearch) {
        qDebug() << "No current search to resume";
        return;
    }

    SearchJobStoppable* searchJobStoppable = qobject_cast<SearchJobStoppable*>(currentSearch);
    if (!searchJobStoppable) {
        qDebug() << "Current search doesn't support pause/resume";
        return;
    }

    if (!searchManager || !searchManager->searchRunning()) {
        qDebug() << "Search is not running - resetting to idle";
        setSearchStateIdle();
        return;
    }

    searchJobStoppable->resumeSearch();
    setSearchStateRunning();
}

void MainWindow::setSearchStateIdle()
{
    qDebug() << "Setting search state: IDLE";

    // Search button: enabled, shows "Search" (green)
    ui->Search_pushButton_Search->setText(tr("Search"));
    ui->Search_pushButton_Search->setIcon(QIcon::fromTheme("edit-find"));
    ui->Search_pushButton_Search->setStyleSheet("QPushButton{ background-color: #81d41a; }"); // Green
    ui->Search_pushButton_Search->setEnabled(true);

    // Stop button: disabled
    ui->Search_pushButton_Stop->setText(tr("Stop"));
    ui->Search_pushButton_Stop->setIcon(QIcon::fromTheme("process-stop"));
    ui->Search_pushButton_Stop->setStyleSheet("");
    ui->Search_pushButton_Stop->setEnabled(false);
}

void MainWindow::setSearchStateRunning()
{
    qDebug() << "Setting search state: RUNNING";

    // Search button: transforms to "Pause" (orange)
    ui->Search_pushButton_Search->setText(tr("Pause"));
    ui->Search_pushButton_Search->setIcon(QIcon::fromTheme("media-playback-pause"));
    ui->Search_pushButton_Search->setStyleSheet("");
    ui->Search_pushButton_Search->setEnabled(true);

    // Stop button: enabled (red)
    ui->Search_pushButton_Stop->setText(tr("Stop"));
    ui->Search_pushButton_Stop->setIcon(QIcon::fromTheme("process-stop"));
    ui->Search_pushButton_Stop->setEnabled(true);
}

void MainWindow::setSearchStatePaused()
{
    qDebug() << "Setting search state: PAUSED";

    // Search button: transforms to "Resume" (green)
    ui->Search_pushButton_Search->setText(tr("Resume"));
    ui->Search_pushButton_Search->setIcon(QIcon::fromTheme("media-playback-start"));
    ui->Search_pushButton_Search->setEnabled(true);

    // Stop button: still enabled (red)
    ui->Search_pushButton_Stop->setText(tr("Stop"));
    ui->Search_pushButton_Stop->setIcon(QIcon::fromTheme("process-stop"));
    ui->Search_pushButton_Search->setStyleSheet("");
    ui->Search_pushButton_Stop->setEnabled(true);
}

void MainWindow::onSearchCompleted()
{
    qDebug() << "=== onSearchCompleted() called ===";
    qDebug() << "Before reset - Search button state:" << static_cast<int>(m_searchButtonState);
    qDebug() << "Before reset - Stop button enabled:" << ui->Search_pushButton_Stop->isEnabled();

    // Reset search button state when search completes
    setSearchButtonState(SearchButtonState::Idle);  // This will disable Stop button

    QApplication::restoreOverrideCursor();

    if (currentSearch && currentSearch->fileNames.size() > 0) {
        qDebug() << "Displaying search results...";
        displaySearchResults();
        qDebug() << "Search results displayed";
    } else {
        qDebug() << "No search results to display";
    }

    qDebug() << "After reset - Search button state:" << static_cast<int>(m_searchButtonState);
    qDebug() << "After reset - Stop button enabled:" << ui->Search_pushButton_Stop->isEnabled();
    qDebug() << "=== onSearchCompleted() completed ===";
}

void MainWindow::onSearchCancelled()
{
    qDebug() << "=== onSearchCancelled() called ===";
    qDebug() << "Before reset - Search button state:" << static_cast<int>(m_searchButtonState);
    qDebug() << "Before reset - Stop button enabled:" << ui->Search_pushButton_Stop->isEnabled();

    // Reset search button state when search is cancelled
    setSearchButtonState(SearchButtonState::Idle);  // This will disable Stop button

    QApplication::restoreOverrideCursor();

    qDebug() << "After reset - Search button state:" << static_cast<int>(m_searchButtonState);
    qDebug() << "After reset - Stop button enabled:" << ui->Search_pushButton_Stop->isEnabled();
}

void MainWindow::onSearchError(const QString &error)
{
    qDebug() << "Search error:" << error;

    // Reset to idle state
    setSearchStateIdle();

    QApplication::restoreOverrideCursor();
    QMessageBox::warning(this, tr("Search Error"), error);
}

void MainWindow::initializeSearchButtons()
{
    // Set initial state to idle
    setSearchStateIdle();

    // Add tooltips that change based on state
    updateTooltips();
}

void MainWindow::updateTooltips()
{
    QString buttonText = ui->Search_pushButton_Search->text();
    if (buttonText == tr("Search") || buttonText == "Search") {
        ui->Search_pushButton_Search->setToolTip(tr("Start a new search"));
    } else if (buttonText == tr("Pause") || buttonText == "Pause") {
        ui->Search_pushButton_Search->setToolTip(tr("Pause the current search"));
    } else if (buttonText == tr("Resume") || buttonText == "Resume") {
        ui->Search_pushButton_Search->setToolTip(tr("Resume the paused search"));
    }
    ui->Search_pushButton_Stop->setToolTip(tr("Stop the current search"));
}

// 1. Update your setSearchButtonState method to also manage the Stop button:
void MainWindow::setSearchButtonState(SearchButtonState state)
{
    qDebug() << "Setting search button state from" << static_cast<int>(m_searchButtonState)
    << "to" << static_cast<int>(state);

    // Safety checks
    if (!ui) {
        qDebug() << "ERROR: ui is null in setSearchButtonState";
        return;
    }

    if (!ui->Search_pushButton_Search) {
        qDebug() << "ERROR: Search_pushButton_Search is null in setSearchButtonState";
        return;
    }

    if (!ui->Search_pushButton_Stop) {
        qDebug() << "ERROR: Search_pushButton_Stop is null in setSearchButtonState";
        return;
    }

    m_searchButtonState = state;

    switch (state) {
    case SearchButtonState::Idle:
        // Search button
        ui->Search_pushButton_Search->setText("&Search");
        ui->Search_pushButton_Search->setIcon(QIcon::fromTheme("edit-find"));
        ui->Search_pushButton_Search->setStyleSheet("QPushButton{ background-color: #81d41a; }");
        ui->Search_pushButton_Search->setEnabled(true);

        // Stop button - DISABLE when no search running
        ui->Search_pushButton_Stop->setEnabled(false);

        isSearchRunning = false;
        qDebug() << "Button set to IDLE state - Stop button disabled";
        break;

    case SearchButtonState::Running:
        // Search button
        ui->Search_pushButton_Search->setText("&Stop");
        ui->Search_pushButton_Search->setIcon(QIcon::fromTheme("process-stop"));
        ui->Search_pushButton_Search->setStyleSheet("QPushButton{ background-color: #ff8000; }");
        ui->Search_pushButton_Search->setEnabled(true);

        // Stop button - ENABLE when search running
        ui->Search_pushButton_Stop->setEnabled(true);

        isSearchRunning = true;
        qDebug() << "Button set to RUNNING state - Stop button enabled";
        break;

    case SearchButtonState::Paused:
        // Search button
        ui->Search_pushButton_Search->setText("&Resume");
        ui->Search_pushButton_Search->setIcon(QIcon::fromTheme("media-playback-start"));
        ui->Search_pushButton_Search->setStyleSheet("QPushButton{ background-color: #39b2e5; }");
        ui->Search_pushButton_Search->setEnabled(true);

        // Stop button - KEEP ENABLED when paused (so user can still stop)
        ui->Search_pushButton_Stop->setEnabled(true);

        qDebug() << "Button set to PAUSED state - Stop button enabled";
        break;
    }

    qDebug() << "Search button text:" << ui->Search_pushButton_Search->text();
    qDebug() << "Stop button enabled:" << ui->Search_pushButton_Stop->isEnabled();
}
