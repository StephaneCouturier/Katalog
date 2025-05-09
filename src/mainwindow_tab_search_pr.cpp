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
#include "qstatusbar.h"
#include "src/filesview.h"
#include "ui_mainwindow.h"

void MainWindow::launchSearch()
{
    // Toggle between starting and stopping a search
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
        return; // Exit the method early
    }

    // Starting a new search
    // Start animation
    QApplication::setOverrideCursor(Qt::WaitCursor);

    // For memory mode without searchInConnected, use MemorySearch directly
    if (collection->databaseMode == "Memory" && !ui->Filters_checkBox_SearchInConnectedDrives->isChecked()) {
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
        isSearchRunning = true;

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
    // For database mode or searchInConnected, use SearchStoppable for stoppable searches
    else {
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
    }

    // Adapt display of files found for searchInConnected
    if (currentSearch && currentSearch->searchInConnectedChecked == true) {
        ui->Search_treeView_FilesFound->model()->setHeaderData(4, Qt::Horizontal, tr("Source Directory"));
        ui->Search_treeView_FilesFound->header()->resizeSection(4, 400);
        ui->Search_treeView_FilesFound->header()->hideSection(5);
    }
}

void MainWindow::sendSearchParameters(Search *search)
{
    if (!search) return;

    // Clear any existing results
    search->clearResults();

    // Transfer all parameters from UI to the search object
    search->searchOnFileName = ui->Search_checkBox_FileName->isChecked();
    search->searchText = ui->Search_lineEdit_SearchText->text();
    search->selectedTextCriteria = ui->Search_comboBox_TextCriteria->currentText();
    search->selectedSearchIn = ui->Search_comboBox_SearchIn->currentText();
    search->caseSensitive = ui->Search_checkBox_CaseSensitive->isChecked();
    search->selectedSearchExclude = ui->Search_lineEdit_Exclude->text();

    search->searchOnFileCriteria = ui->Search_checkBox_FileCriteria->isChecked();
    search->searchOnSize = ui->Search_checkBox_Size->isChecked();
    search->selectedMinimumSize = ui->Search_spinBox_MinimumSize->value();
    search->selectedMaximumSize = ui->Search_spinBox_MaximumSize->value();
    search->selectedMinSizeUnit = ui->Search_comboBox_MinSizeUnit->currentText();
    search->selectedMaxSizeUnit = ui->Search_comboBox_MaxSizeUnit->currentText();
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
        ui->Search_treeView_CatalogsFound->setModel(currentSearch->deviceFoundModel);
        ui->Search_treeView_CatalogsFound->hideColumn(1);
    } else {
        qWarning() << "displaySearchResults: deviceFoundModel is null";
        // Create an empty model
        QStandardItemModel* emptyModel = new QStandardItemModel(this);
        ui->Search_treeView_CatalogsFound->setModel(emptyModel);
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
    ui->Search_label_NumberResults->setText(QString::number(currentSearch->filesFoundNumber));
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

void MainWindow::updateSearchProgress(int filesProcessed)
{
    // Update lastProcessedFiles for statistics
    lastProcessedFiles = filesProcessed;

    // Build status message
    QString statusMessage;

    // Special values for different states:
    // -1: Search interrupted
    // -2: Catalog loading started
    // -3: Catalog loading finished, processing files
    // -4: Catalog loading progress update (only used by SearchMemory)

    // Special case for interrupted search (-1)
    if (filesProcessed == -1) {
        if (currentSearch) {
            statusMessage = tr("Search interrupted | Files found: %1 | Files processed: %2")
            .arg(currentSearch->fileNames.size())
                .arg(lastProcessedFiles);
        } else {
            statusMessage = tr("Search interrupted. No results available.");
        }

        statusBar()->showMessage(statusMessage, 5000);
        return;
    }

    // Special case for catalog loading started (-2)
    if (filesProcessed == -2) {
        if (currentSearch) {
            statusMessage = tr("Loading Catalog %1 of %2 (%3) | Files found: %4")
            .arg(currentSearch->currentCatalogIndex)
                .arg(currentSearch->totalCatalogs)
                .arg(currentSearch->currentCatalogName)
                .arg(currentSearch->fileNames.size());

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

            statusMessage = tr("Loading Catalog %1 of %2 (%3) | %4 files loaded (%5%) | Files found: %6")
                                .arg(searchMemory->currentCatalogIndex)
                                .arg(searchMemory->totalCatalogs)
                                .arg(searchMemory->currentCatalogName)
                                .arg(searchMemory->currentCatalogFilesLoaded)
                                .arg(QString::number(percentLoaded, 'f', 1))
                                .arg(currentSearch->fileNames.size());

            statusBar()->showMessage(statusMessage);
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
                .arg(currentSearch->fileNames.size());

            statusBar()->showMessage(statusMessage);
        }
        return;
    }

    // Regular progress update
    if (currentSearch) {
        // If we have catalog information, show it
        if (currentSearch->totalCatalogs > 0) {
            statusMessage = tr("Catalog %1 of %2 | Files found: %3 | Files processed: %4")
            .arg(currentSearch->currentCatalogIndex)
                .arg(currentSearch->totalCatalogs)
                .arg(currentSearch->fileNames.size())
                .arg(filesProcessed);
        } else {
            // Otherwise just show files
            statusMessage = tr("Files found: %1 | Files processed: %2")
                                .arg(currentSearch->fileNames.size())
                                .arg(filesProcessed);
        }

        // Calculate percentage if we have an estimate
        if (currentSearch->estimatedTotalFiles > 0) {
            int percentComplete = qMin(100,
                                       static_cast<int>((filesProcessed * 100) / currentSearch->estimatedTotalFiles));
            statusMessage += tr(" (%1%)").arg(percentComplete);
        }
    }

    // Show status message
    statusBar()->showMessage(statusMessage);

    // Process events to keep UI responsive
    QCoreApplication::processEvents();
}

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

    // Don't delete dbSearch, just reset its state if it exists
    if (searchStoppable) {
        searchStoppable->clearResults();
        // Reset connection name to force reinitialization next time
        searchStoppable->connectionName = "";
    }

    // Clear any models or views
    if (ui->Search_treeView_FilesFound->model()) {
        // Create an empty model to replace the existing one
        QStandardItemModel* emptyModel = new QStandardItemModel(this);
        ui->Search_treeView_FilesFound->setModel(emptyModel);
    }

    if (ui->Search_treeView_CatalogsFound->model()) {
        QStandardItemModel* emptyModel = new QStandardItemModel(this);
        ui->Search_treeView_CatalogsFound->setModel(emptyModel);
    }
}
