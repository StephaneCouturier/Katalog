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
#include "core/filemetadata.h"


#include <QDialog>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QTextBrowser>
#include <QLabel>
#include <QPushButton>
#include <QMessageBox>
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonArray>
#include <QJsonValue>
#include <QJsonParseError>
#include <QRegularExpression>
#include <QClipboard>
#include <QApplication>
#include <QIcon>
#include "core/filemetadata.h"


//----------------------------------------------------------------------
//--- Search management ------------------------------------------------
//----------------------------------------------------------------------
void MainWindow::launchSearch()
{// Generic search launch method, using a type of search based on database mode and search type

    // Set animation cursor before starting
    QApplication::setOverrideCursor(Qt::WaitCursor);

    qDebug() << "=== launchSearch() called ===";

    if (!searchManager) {
        qDebug() << "ERROR: SearchManager not initialized";
        return;
    }

    // Clear the search view before starting a new search
    clearSearchResults();

    // Show status bar for Search
    statusBar()->show();

    // Create SearchJobStoppable
    SearchJobStoppable* searchJobStoppable = new SearchJobStoppable(this);
    searchJobStoppable->setDatabaseConnection("defaultConnection");

    // Enable memory mode when in development mode and memory mode**
    if (collection->databaseMode == "Memory") {
        qDebug() << "Memory mode: Enabling memory mode for SearchJobStoppable";
        searchJobStoppable->setMemoryModeEnabled(true);
    }

    connect(searchJobStoppable, &Search::searchProgress, this, &MainWindow::updateSearchProgress);

    currentSearch = searchJobStoppable;

    // Update progress manager with current search for file count display
    if (searchProgressManager) {
        searchProgressManager->setCurrentSearch(currentSearch);
    }

    // Set up basic search parameters
    sendSearchParametersFromUI(searchJobStoppable);

    setSearchButtonState(SearchButtonState::Running);

    // Update UI for running search (Search button becomes Pause)
    setSearchStateRunning();

    // Start the search
    qDebug() << "Starting Search via SearchManager";
    searchManager->startSearchJobStoppable(searchJobStoppable, selectedDevice);

    searchResultsThrottler->setCurrentSearch(searchJobStoppable);
    connect(searchJobStoppable, &Search::searchProgress,
            searchResultsThrottler, &SearchResultsThrottler::onSearchProgress);

    qDebug() << "=== launchSearch() complete ===";
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
void MainWindow::sendSearchParametersFromUI(Search *search)
{// Send search parameters from UI to the search object
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
    int fileTypeIndex = ui->Search_comboBox_FileType->currentIndex();
    FileTypeMapping::UserCategory selectedCategory =
        static_cast<FileTypeMapping::UserCategory>(
            ui->Search_comboBox_FileType->itemData(fileTypeIndex).toInt()
            );
    search->selectedFileType2Category = selectedCategory;
    // Map FileType to legacy selectedFileType for search history compatibility
    switch (selectedCategory) {
        case FileTypeMapping::ALL:
            search->selectedFileType = "All";
            break;
        case FileTypeMapping::AUDIO:
            search->selectedFileType = "Audio";
            break;
        case FileTypeMapping::IMAGE:
            search->selectedFileType = "Image";
            break;
        case FileTypeMapping::TEXT:
            search->selectedFileType = "Text";
            break;
        case FileTypeMapping::VIDEO:
            search->selectedFileType = "Video";
            break;
        case FileTypeMapping::OTHER:
            search->selectedFileType = "Other";
            break;
        case FileTypeMapping::NONE:
            search->selectedFileType = "None";
            break;
        default:
            search->selectedFileType = "All";
            break;
    }

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

    // Populate the selectedDeviceList
    search->selectedDeviceIDList.clear();

    if (search->searchInCatalogsChecked && selectedDevice) {
        if (selectedDevice->type == "Catalog") {
            // Single catalog selected
            search->selectedDeviceIDList.append(selectedDevice->ID);
        }
        else if (selectedDevice->type == "Storage") {
            // Storage selected - add the storage device ID
            search->selectedDeviceIDList.append(selectedDevice->ID);
        }
        else if (selectedDevice->type == "All" || selectedDevice->ID == 0) {
            // "All" selected - use 0 as convention
            search->selectedDeviceIDList.append(0);
        }
        else {
            // Other device types (Virtual, etc.) - add the device ID
            search->selectedDeviceIDList.append(selectedDevice->ID);
        }
    }
    else if (search->searchInConnectedChecked) {
        // Directory search - use 0 or could be empty
        search->selectedDeviceIDList.append(0);
    }
    else {
        // No specific search type - fallback
        search->selectedDeviceIDList.append(0);
    }
}
//----------------------------------------------------------------------
//--- States & transitions ---------------------------------------------
//----------------------------------------------------------------------
void MainWindow::handleSearchCompleted()
{
    // Reset the search button
    ui->Search_pushButton_Search->setText(tr("Search"));
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
    ui->Search_pushButton_Search->setText(tr("Search"));
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
void MainWindow::resetSearchState()
{
    // Reset any active searches
    isSearchRunning = false;

    // Reset the search button
    ui->Search_pushButton_Search->setText(tr("Search"));
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
void MainWindow::pauseCurrentSearch()
{
    qDebug() << "=== pauseCurrentSearch() called ===";

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

    qDebug() << "Calling pauseSearch() on SearchJobStoppable";
    searchJobStoppable->pauseSearch();

    qDebug() << "Calling setSearchStatePaused()";
    setSearchStatePaused();

    qDebug() << "State after pause should be:" << static_cast<int>(SearchButtonState::Paused);
    qDebug() << "Actual state is:" << static_cast<int>(searchButtonState);
}
//----------------------------------------------------------------------
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
//----------------------------------------------------------------------
void MainWindow::onSearchCompleted()
{
    qDebug() << "=== onSearchCompleted() called ===";
    qDebug() << "Before reset - Search button state:" << static_cast<int>(searchButtonState);
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

    qDebug() << "After reset - Search button state:" << static_cast<int>(searchButtonState);
    qDebug() << "After reset - Stop button enabled:" << ui->Search_pushButton_Stop->isEnabled();
    qDebug() << "=== onSearchCompleted() completed ===";
}
//----------------------------------------------------------------------
void MainWindow::onSearchCancelled()
{
    qDebug() << "=== onSearchCancelled() called ===";
    qDebug() << "Before reset - Search button state:" << static_cast<int>(searchButtonState);
    qDebug() << "Before reset - Stop button enabled:" << ui->Search_pushButton_Stop->isEnabled();

    // Reset search button state when search is cancelled
    setSearchButtonState(SearchButtonState::Idle);  // This will disable Stop button

    QApplication::restoreOverrideCursor();

    qDebug() << "After reset - Search button state:" << static_cast<int>(searchButtonState);
    qDebug() << "After reset - Stop button enabled:" << ui->Search_pushButton_Stop->isEnabled();
}
//----------------------------------------------------------------------
void MainWindow::onSearchError(const QString &error)
{
    qDebug() << "Search error:" << error;

    // Reset to idle state
    setSearchStateIdle();

    QApplication::restoreOverrideCursor();
    QMessageBox::warning(this, tr("Search Error"), error);
}
//----------------------------------------------------------------------
//--- Button handling --------------------------------------------------
//----------------------------------------------------------------------
void MainWindow::setSearchStateIdle()
{
    qDebug() << "Setting search state: IDLE";
    setSearchButtonState(SearchButtonState::Idle);
    updateTooltips();
}
//----------------------------------------------------------------------
void MainWindow::setSearchStateRunning()
{
    qDebug() << "Setting search state: RUNNING";
    setSearchButtonState(SearchButtonState::Running);
    updateTooltips();
}
//----------------------------------------------------------------------
void MainWindow::setSearchStatePaused()
{
    qDebug() << "Setting search state: PAUSED";
    setSearchButtonState(SearchButtonState::Paused);
    updateTooltips();
}
//----------------------------------------------------------------------
void MainWindow::setSearchButtonState(SearchButtonState state)
{
    qDebug() << "Setting search button state from" << static_cast<int>(searchButtonState)
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

    searchButtonState = state;

    switch (state) {
    case SearchButtonState::Idle:
        // Search button
        ui->Search_pushButton_Search->setText(tr("Search"));
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
        ui->Search_pushButton_Search->setText(tr("Pause"));
        ui->Search_pushButton_Search->setIcon(QIcon::fromTheme("process-stop"));
        ui->Search_pushButton_Search->setIcon(QIcon::fromTheme("media-playback-pause"));
        ui->Search_pushButton_Search->setStyleSheet("");
        ui->Search_pushButton_Search->setEnabled(true);

        // Stop button - ENABLE when search running
        ui->Search_pushButton_Stop->setEnabled(true);

        isSearchRunning = true;
        qDebug() << "Button set to RUNNING state - Stop button enabled";
        break;

    case SearchButtonState::Paused:
        // Search button
        ui->Search_pushButton_Search->setText(tr("Resume"));
        ui->Search_pushButton_Search->setIcon(QIcon::fromTheme("media-playback-start"));
        //ui->Search_pushButton_Search->setStyleSheet("QPushButton{ background-color: #39b2e5; }");
        ui->Search_pushButton_Search->setEnabled(true);

        // Stop button - KEEP ENABLED when paused (so user can still stop)
        ui->Search_pushButton_Stop->setEnabled(true);

        qDebug() << "Button set to PAUSED state - Stop button enabled";
        break;

    case SearchButtonState::Searching:  // New state for memory mode
        ui->Search_pushButton_Search->setText(tr("Searching..."));
        ui->Search_pushButton_Search->setIcon(QIcon::fromTheme("edit-find"));
        ui->Search_pushButton_Search->setStyleSheet("QPushButton{ background-color: #81d41a; }");
        ui->Search_pushButton_Search->setEnabled(false);  // Deactivated during search
        ui->Search_pushButton_Stop->setEnabled(false);    // No stop for memory mode
        isSearchRunning = true;
        break;

    }

    qDebug() << "Search button text:" << ui->Search_pushButton_Search->text();
    qDebug() << "Stop button enabled:" << ui->Search_pushButton_Stop->isEnabled();
}
//----------------------------------------------------------------------
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

// Replace the displayExtendedMetadataJson method implementation with this updated version:

void MainWindow::displayExtendedMetadataJson(int catalogId, const QString &folderPath, const QString &fileName)
{
    QString jsonMetadata;
    QString filePath = folderPath + "/" + fileName;

    // Try to get stored JSON from database if we have a catalog
    if (catalogId > 0) {
        QSqlQuery query(QSqlDatabase::database("defaultConnection"));
        QString querySQL = QLatin1String(R"(
            SELECT metadata_extended
            FROM file
            WHERE file_name = :file_name
            AND file_folder_path = :folder_path
            AND file_catalog_id = :catalog_id
            AND metadata_extended IS NOT NULL
            AND metadata_extended != ''
        )");

        query.prepare(querySQL);
        query.bindValue(":file_name", fileName);
        query.bindValue(":folder_path", folderPath);
        query.bindValue(":catalog_id", catalogId);

        if (query.exec() && query.next()) {
            jsonMetadata = query.value(0).toString();
        }
    }

    // If no stored JSON found, try to extract it fresh from the file
    if (jsonMetadata.isEmpty()) {
        QFileInfo fileInfo(filePath);
        if (fileInfo.exists() && fileInfo.isReadable()) {
            jsonMetadata = FileMetadata::getExtendedMetadataJson(filePath);

            if (jsonMetadata == "No extended metadata available for this file") {
                QMessageBox::information(this, "Extended Metadata",
                                         QString("No extended metadata available for:\n%1\n\nThis could mean:\n"
                                                 "• The file type doesn't support metadata extraction\n"
                                                 "• The file has no embedded metadata").arg(fileName));
                return;
            }
        } else {
            QMessageBox::information(this, "Extended Metadata",
                                     QString("Cannot access file:\n%1\n\nThe file path may not be accessible.").arg(filePath));
            return;
        }
    }

    // Convert JSON to HTML table
    // Convert JSON to HTML table
    QString htmlTable = convertJsonToHtmlTable(jsonMetadata);

    // Create custom dialog
    QDialog *metadataDialog = new QDialog(this);
    metadataDialog->setWindowTitle(QString("Extended Metadata - %1").arg(fileName));
    metadataDialog->resize(600, 500);
    metadataDialog->setModal(true);

    // Create main layout (pass dialog as parent immediately)
    QVBoxLayout *mainLayout = new QVBoxLayout(metadataDialog);

    // Add file info label
    QLabel *fileInfoLabel = new QLabel(QString("<b>%1:</b> %2").arg(tr("File"), fileName));
    fileInfoLabel->setStyleSheet("padding: 8px; background-color: #f8f9fa; border-radius: 4px;");
    mainLayout->addWidget(fileInfoLabel);

    // Create text browser for HTML display
    QTextBrowser *textBrowser = new QTextBrowser();
    textBrowser->setHtml(htmlTable);
    textBrowser->setOpenExternalLinks(false);
    textBrowser->setStyleSheet("QTextBrowser { border: 1px solid #ddd; border-radius: 4px; }");
    mainLayout->addWidget(textBrowser);

    // Create horizontal layout for buttons
    QWidget *buttonWidget = new QWidget();
    QHBoxLayout *buttonLayout = new QHBoxLayout(buttonWidget);
    buttonLayout->addStretch(); // Push buttons to the right

    // Copy button
    QPushButton *copyButton = new QPushButton(tr("Copy JSON"));
    copyButton->setIcon(QIcon::fromTheme("edit-copy"));
    connect(copyButton, &QPushButton::clicked, [copyButton, jsonMetadata]() {
        QApplication::clipboard()->setText(jsonMetadata);
        copyButton->setIcon(QIcon::fromTheme("dialog-ok-apply"));
        copyButton->setText(tr("Copied"));
        QTimer::singleShot(2000, [copyButton]() {
            copyButton->setIcon(QIcon::fromTheme("edit-copy"));
            copyButton->setText(tr("Copy JSON"));
        });
    });

    // Close button
    QPushButton *closeButton = new QPushButton(tr("Close"));
    closeButton->setIcon(QIcon::fromTheme("dialog-close"));
    closeButton->setDefault(true);
    connect(closeButton, &QPushButton::clicked, metadataDialog, &QDialog::accept);

    // Add buttons to button layout
    buttonLayout->addWidget(copyButton);
    buttonLayout->addWidget(closeButton);

    // Add button widget to main layout
    mainLayout->addWidget(buttonWidget);

    // Show dialog
    metadataDialog->exec();
    metadataDialog->deleteLater();
}

QString MainWindow::formatJsonForDisplay(const QString &jsonString)
{
    // Parse and reformat JSON with proper indentation
    QJsonParseError error;
    QJsonDocument doc = QJsonDocument::fromJson(jsonString.toUtf8(), &error);

    if (error.error != QJsonParseError::NoError) {
        return QString("JSON Parse Error: %1\n\nRaw content:\n%2").arg(error.errorString(), jsonString);
    }

    // Return formatted JSON with indentation
    return doc.toJson(QJsonDocument::Indented);
}

// Replace the convertJsonToHtmlTable method implementation with this fixed version:

QString MainWindow::convertJsonToHtmlTable(const QString &jsonString)
{
    // Parse JSON
    QJsonParseError error;
    QJsonDocument doc = QJsonDocument::fromJson(jsonString.toUtf8(), &error);

    if (error.error != QJsonParseError::NoError) {
        return QString("<p style='color: red;'>JSON Parse Error: %1</p><pre>%2</pre>").arg(error.errorString(), jsonString);
    }

    QJsonObject jsonObj = doc.object();
    if (jsonObj.isEmpty()) {
        return "<p>No metadata available.</p>";
    }

    // Start building HTML table with Gwenview-like styling
    QString html = QString(R"(
        <style>
        table {
            border-collapse: collapse;
            width: 100%;
            font-family: -apple-system, BlinkMacSystemFont, 'Segoe UI', Roboto, sans-serif;
            font-size: 13px;
        }
        th {
            background-color: #f5f5f5;
            color: #333;
            padding: 8px 12px;
            text-align: left;
            border-bottom: 2px solid #ddd;
            font-weight: 600;
        }
        td {
            padding: 6px 12px;
            border-bottom: 1px solid #eee;
            vertical-align: top;
        }
        tr:nth-child(even) {
            background-color: #fafafa;
        }
        tr:hover {
            background-color: #f0f8ff;
        }
        .field-name {
            font-weight: 500;
            color: #2c3e50;
            min-width: 180px;
            max-width: 250px;
        }
        .field-value {
            word-break: break-word;
            color: #34495e;
        }
        .array-value {
            font-style: italic;
            color: #7f8c8d;
        }
        </style>
        <table>
        <thead>
            <tr>
                <th class="field-name">%1</th>
                <th class="field-value">%2</th>
            </tr>
        </thead>
        <tbody>
    )").arg(tr("File Property"), tr("Value"));

    // Sort keys for consistent display
    QStringList keys = jsonObj.keys();
    //keys.sort();
    // Custom sorting function to group related fields
    std::sort(keys.begin(), keys.end(), [](const QString &a, const QString &b) {
        // Define priority groups
        QStringList priority = {
            "Title", "Artist", "Album", "Genre", "Year", "Track",  // Audio metadata first
            "Duration", "Bitrate", "Sample Rate",                  // Audio technical
            "Width", "Height", "Orientation",                      // Image/Video dimensions together
            "Codec", "Framerate"                                   // Video technical
        };

        // Check if both keys are in priority list
        int indexA = -1, indexB = -1;

        for (int i = 0; i < priority.size(); ++i) {
            QString cleanKeyA = a.split('_').last();  // Get last part (e.g., "width" from "image_width")
            QString cleanKeyB = b.split('_').last();

            if (cleanKeyA.compare(priority[i], Qt::CaseInsensitive) == 0) {
                indexA = i;
            }
            if (cleanKeyB.compare(priority[i], Qt::CaseInsensitive) == 0) {
                indexB = i;
            }
        }

        // If both have priority, use priority order
        if (indexA >= 0 && indexB >= 0) {
            return indexA < indexB;
        }

        // If only one has priority, it comes first
        if (indexA >= 0) return true;
        if (indexB >= 0) return false;

        // Otherwise, use alphabetical
        return a < b;
    });

    // Add each key-value pair as table row
    for (const QString &key : keys) {
        QJsonValue value = jsonObj[key];
        QString displayKey = key;
        QString displayValue;

        // Format the key name (make it more readable)
        displayKey = displayKey.replace('_', ' ');

        // Simple title case conversion
        QStringList words = displayKey.split(' ');
        for (QString &word : words) {
            if (!word.isEmpty()) {
                word[0] = word[0].toUpper();
            }
        }
        displayKey = words.join(' ');

        // Handle different value types
        if (value.isArray()) {
            QJsonArray array = value.toArray();
            QStringList arrayItems;
            for (const QJsonValue &item : array) {
                arrayItems << item.toString();
            }
            displayValue = QString("<span class='array-value'>[%1]</span>").arg(arrayItems.join(", "));
        } else if (value.isObject()) {
            displayValue = "<span class='array-value'>[Complex Object]</span>";
        } else if (value.isString()) {
            displayValue = value.toString();
        } else if (value.isDouble() || value.toVariant().canConvert<int>()) {
            // Check if this is a duration field
            if (key.contains("duration", Qt::CaseInsensitive) ) {
                int duration = value.toVariant().toInt();
                if (duration > 0) {
                    // Determine if it's video or audio duration for formatting
                    if (key.contains("video", Qt::CaseInsensitive)) {
                        // Video Duration - show as H:MM:SS
                        int hours = duration / 3600;
                        int minutes = (duration % 3600) / 60;
                        int seconds = duration % 60;
                        displayValue = QString("%1:%2:%3")
                                           .arg(hours, 2, 10, QChar('0'))
                                           .arg(minutes, 2, 10, QChar('0'))
                                           .arg(seconds, 2, 10, QChar('0'));
                    } else {
                        // Audio Duration - show as MM:SS
                        int minutes = duration / 60;
                        int seconds = duration % 60;
                        displayValue = QString("%1:%2")
                                           .arg(minutes, 2, 10, QChar('0'))
                                           .arg(seconds, 2, 10, QChar('0'));
                    }
                } else {
                    displayValue = QString::number(duration);
                }
            } else {
                // Regular numeric value
                displayValue = QString::number(value.toVariant().toDouble());
            }
        } else if (value.isBool()) {
            displayValue = value.toBool() ? "Yes" : "No";
        } else {
            displayValue = value.toVariant().toString();
        }

        // Escape HTML characters in values
        displayValue = displayValue.toHtmlEscaped();

        // Add table row
        html += QString(R"(
            <tr>
                <td class="field-name">%1</td>
                <td class="field-value">%2</td>
            </tr>
        )").arg(displayKey, displayValue);
    }

    html += "</tbody></table>";
    return html;
}

//----------------------------------------------------------------------
//--- Display results --------------------------------------------------
//----------------------------------------------------------------------
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
        ui->Search_treeView_FilesFound->header()->resizeSection(3, 300); // Path
        ui->Search_treeView_FilesFound->header()->resizeSection(4, 120); // Catalog
        ui->Search_treeView_FilesFound->header()->resizeSection(5, 60);  // Type

        // Metadata columns - updated for merged columns
        ui->Search_treeView_FilesFound->header()->resizeSection(8, 80);   // File Type
        ui->Search_treeView_FilesFound->header()->resizeSection(9, 100);  // MIME Type
        ui->Search_treeView_FilesFound->header()->resizeSection(10, 80);  // Width (merged)
        ui->Search_treeView_FilesFound->header()->resizeSection(11, 80);  // Height (merged)
        ui->Search_treeView_FilesFound->header()->resizeSection(12, 90);  // Duration (merged)

        // Hide redundant columns
        ui->Search_treeView_FilesFound->hideColumn(6); // order (for explore only)
        ui->Search_treeView_FilesFound->hideColumn(7); // path  (for explore only)

        ui->Search_treeView_FilesFound->hideColumn(13); // video_width (merged)
        ui->Search_treeView_FilesFound->hideColumn(14); // video_height (merged)
        ui->Search_treeView_FilesFound->hideColumn(15); // audio_duration (merged)

        // Audio metadata columns (shifted indices due to hidden columns)
        ui->Search_treeView_FilesFound->header()->resizeSection(16, 120); // Artist
        ui->Search_treeView_FilesFound->header()->resizeSection(17, 120); // Album
        ui->Search_treeView_FilesFound->header()->resizeSection(18, 150); // Title

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
//--- Reporting --------------------------------------------------------
//----------------------------------------------------------------------
void MainWindow::updateSearchProgress(int filesProcessed)
{    // Special values for different states:
    // -1: Search interrupted
    // -2: Catalog loading started
    // -3: Catalog loading finished, processing files
    // -4: Catalog loading progress update

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

    // Special case for catalog loading progress update (-4) - used by both SearchMemory and SearchJobStoppable
    if (filesProcessed == -4) {
        SearchJobStoppable* searchJobStoppable = dynamic_cast<SearchJobStoppable*>(currentSearch);

        double percentLoaded = 0;
        if (searchJobStoppable->currentCatalogTotalFiles > 0) {
            percentLoaded = (double)searchJobStoppable->currentCatalogFilesLoaded /
                            searchJobStoppable->currentCatalogTotalFiles * 100.0;
        }

        statusMessage = tr("Loading Catalog %1 of %2 (%3) | %4 files loaded (%5%) | Files found: %6 | Files processed: %7")
                            .arg(searchJobStoppable->currentCatalogIndex)
                            .arg(searchJobStoppable->totalCatalogs)
                            .arg(searchJobStoppable->currentCatalogName)
                            .arg(QLocale().toString(searchJobStoppable->currentCatalogFilesLoaded))
                            .arg(QString::number(percentLoaded, 'f', 1))
                            .arg(QLocale().toString(currentSearch->fileNames.size()))
                            .arg(QLocale().toString(currentSearch->totalFilesProcessed));

        // Append PAUSED status if search is paused
        if (searchJobStoppable->isPaused()) {
            statusMessage += tr(" | CATALOG LOADING PAUSED");
        }



        statusBar()->show();
        statusBar()->showMessage(statusMessage);
        QCoreApplication::processEvents();

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
//----------------------------------------------------------------------
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

    // Check if the search was interrupted - works with SearchJobStoppable
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
void MainWindow::removeFileFromResults(QString fullFilePath)
{
    // Remove the file from the list of results and refresh the UI
    // Find the index of the file in the results
    qDebug() << "Removing file from results:" << fullFilePath << QFileInfo(fullFilePath).absoluteFilePath();

    int index = currentSearch->filePaths.indexOf(QFileInfo(fullFilePath).absoluteFilePath());

    if (index != -1) {
        // Remove the file from all lists
        currentSearch->filePaths.removeAt(index);
        currentSearch->fileNames.removeAt(index);
        currentSearch->fileSizes.removeAt(index);
        currentSearch->fileDateTimes.removeAt(index);
        currentSearch->fileCatalogs.removeAt(index);

        // Update the number of files found and their total size
        currentSearch->filesFoundNumber = currentSearch->fileNames.size();
        currentSearch->filesFoundTotalSize = 0;

        for (const qint64 &size : currentSearch->fileSizes) {
            currentSearch->filesFoundTotalSize += size;
        }
    }
}
