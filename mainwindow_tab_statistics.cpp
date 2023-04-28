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
// File Name:   mainwindow_tab_statistics.cpp
// Purpose:     methods for the screen STATISTICS
// Description: https://github.com/StephaneCouturier/Katalog/wiki/Statistics
// Author:      Stephane Couturier
/////////////////////////////////////////////////////////////////////////////
*/

#include "mainwindow.h"
#include "ui_mainwindow.h"

//UI------------------------------------------------------------------------
    void MainWindow::on_Statistics_comboBox_SelectSource_currentTextChanged()
    {
        QString selectedSource = ui->Statistics_comboBox_SelectSource->itemData(ui->Statistics_comboBox_SelectSource->currentIndex(),Qt::UserRole).toString();

        //Save selection in settings file;
        QSettings settings(settingsFilePath, QSettings:: IniFormat);
        settings.setValue("Statistics/SelectedSource", selectedSource);

        //Display selection combo boxes depending on data source
        if (selectedSource ==tr("collection snapshots")){
            ui->Statistics_label_DataType->show();
            ui->Statistics_comboBox_TypeOfData->show();
        }
        else if(selectedSource ==tr("catalog updates")){
            ui->Statistics_label_DataType->show();
            ui->Statistics_comboBox_TypeOfData->show();
        }
        else if(selectedSource ==tr("storage updates")){
            ui->Statistics_label_DataType->hide();
            ui->Statistics_comboBox_TypeOfData->hide();
        }

        //Load the graph
        loadStatisticsChart();
    }

    //----------------------------------------------------------------------
    void MainWindow::on_StatisticsComboBoxSelectCatalogCurrentIndexChanged(const QString &selectedCatalog)
    {       
        //save selection in settings file;
        QSettings settings(settingsFilePath, QSettings:: IniFormat);
        settings.setValue("Statistics/SelectedCatalog", selectedCatalog);

        selectedFilterCatalogName = selectedCatalog;

        //load the graph
        loadStatisticsChart();
    }
    //----------------------------------------------------------------------
    void MainWindow::on_Statistics_comboBox_TypeOfData_currentTextChanged()
    {
        QString typeOfData = ui->Statistics_comboBox_TypeOfData->itemData(ui->Statistics_comboBox_TypeOfData->currentIndex(),Qt::UserRole).toString();

        //save selection in settings file;
        QSettings settings(settingsFilePath, QSettings:: IniFormat);
        settings.setValue("Statistics/TypeOfData", typeOfData);

        //load the graph
        loadStatisticsChart();
    }

    //----------------------------------------------------------------------
    void MainWindow::on_Statistics_pushButton_EditCatalogStatisticsFile_clicked()
    {
        QDesktopServices::openUrl(QUrl::fromLocalFile(statisticsCatalogFilePath));
    }
    //----------------------------------------------------------------------
    void MainWindow::on_Statistics_pushButton_EditStorageStatisticsFile_clicked()
    {
        QDesktopServices::openUrl(QUrl::fromLocalFile(statisticsStorageFilePath));
    }
    //----------------------------------------------------------------------
    void MainWindow::on_Statistics_pushButton_Reload_clicked()
    {
        if(databaseMode=="Memory"){
            loadStatisticsCatalogFileToTable();
            loadStatisticsStorageFileToTable();
        }
        loadStatisticsChart();
    }
    //----------------------------------------------------------------------
    void MainWindow::on_Statistics_lineEdit_GraphicStartDate_returnPressed()
    {
        graphicStartDate = QDateTime::fromString(ui->Statistics_lineEdit_GraphicStartDate->text(),"yyyy-mm-dd");

        QSettings settings(settingsFilePath, QSettings:: IniFormat);
        settings.setValue("Statistics/graphStartDate", graphicStartDate.date().toString("yyyy-MM-dd"));

        //load the graph
        loadStatisticsChart();
    }
    //----------------------------------------------------------------------
    void MainWindow::on_Statistics_pushButton_ClearDate_clicked()
    {
        graphicStartDate = *new QDateTime;
        ui->Statistics_lineEdit_GraphicStartDate->setText("");

        QSettings settings(settingsFilePath, QSettings:: IniFormat);
        settings.setValue("Statistics/graphStartDate", "");

        //load the graph
        loadStatisticsChart();

        ui->Statistics_calendarWidget->hide();
    }
    //----------------------------------------------------------------------
    void MainWindow::on_Statistics_calendarWidget_clicked(const QDate &date)
    {
        graphicStartDate.setDate(date);
        graphicStartDate.setTime(QTime::fromString("00:00:00"));
        ui->Statistics_lineEdit_GraphicStartDate->setText(graphicStartDate.date().toString("yyyy-MM-dd"));

        QSettings settings(settingsFilePath, QSettings:: IniFormat);
        settings.setValue("Statistics/graphStartDate", graphicStartDate.date().toString("yyyy-MM-dd"));

        //load the graph
        loadStatisticsChart();

        ui->Statistics_calendarWidget->hide();
    }
    //----------------------------------------------------------------------
    void MainWindow::on_Statistics_pushButton_PickDate_clicked()
    {
        ui->Statistics_calendarWidget->setHidden(false);
    }
    //----------------------------------------------------------------------

//Methods-------------------------------------------------------------------
    void MainWindow::loadStatisticsDataTypes()
    {//Populate the comboxboxes

        //Populate the comboxbox for selected source

            //Get last value
            QSettings settings(settingsFilePath, QSettings:: IniFormat);
            QString lastSelectedSourceValue = settings.value("Statistics/SelectedSource").toString();

            //Generate list of values
            ui->Statistics_comboBox_SelectSource->setItemData(0, "catalog updates", Qt::UserRole);
            ui->Statistics_comboBox_SelectSource->setItemData(1, "storage updates", Qt::UserRole);
            ui->Statistics_comboBox_SelectSource->setItemData(2, "collection snapshots", Qt::UserRole);

            //Restore last selection value or default
            if (lastSelectedSourceValue !=""){
                ui->Statistics_comboBox_SelectSource->setCurrentText(tr(lastSelectedSourceValue.toUtf8()));
            }

        //Populate the comboxbox for types of data

            //Get last value
            QString lastValue = settings.value("Statistics/TypeOfData").toString();

            //Generate list of values
            ui->Statistics_comboBox_TypeOfData->setItemData(0, "Total File Size", Qt::UserRole);
            ui->Statistics_comboBox_TypeOfData->setItemData(1, "Number of Files", Qt::UserRole);

            //Restore last selection value or default
            if (lastValue !=""){
                ui->Statistics_comboBox_TypeOfData->setCurrentText(tr(lastValue.toUtf8()));
            }
    }
    //----------------------------------------------------------------------
    void MainWindow::loadStatisticsCatalogFileToTable()
    {// Load the contents of the statistics file into the database

        //clear database table
            QSqlQuery deleteQuery;
            deleteQuery.exec("DELETE FROM statistics_catalog");

        // Get infos stored in the file
            QFile statisticsCatalogFile(statisticsCatalogFilePath);
            if(!statisticsCatalogFile.open(QIODevice::ReadOnly)) {
                return;
            }

            QTextStream textStream(&statisticsCatalogFile);

        //prepare query to load file info
            QSqlQuery insertQuery;
            QString insertSQL = QLatin1String(R"(
                                INSERT INTO statistics_catalog (
                                                date_time,
                                                catalog_name,
                                                catalog_file_count,
                                                catalog_total_file_size,
                                                record_type )
                                VALUES(
                                                :date_time,
                                                :catalog_name,
                                                :catalog_file_count,
                                                :catalog_total_file_size,
                                                :record_type )
                                            )");
            insertQuery.prepare(insertSQL);

        //set temporary values
            QString     line;
            QStringList fieldList;

            QString     dateTime;
            QString     catalogName;
            qint64      catalogFileCount;
            qint64      catalogTotalFileSize;
            QString     recordType;
            QRegularExpression tagExp("\t");

        //Skip titles line
            line = textStream.readLine();

        //load file to database
        while (!textStream.atEnd())
        {
            line = textStream.readLine();
            if (line.isNull())
                break;
            else
                {
                //Split the string with \t (tabulation) into a list
                    fieldList.clear();
                    fieldList = line.split(tagExp);
                    dateTime                = fieldList[0];
                    catalogName             = fieldList[1];
                    catalogFileCount        = fieldList[2].toLongLong();
                    catalogTotalFileSize    = fieldList[3].toLongLong();
                    recordType              = fieldList[4];

                    //Append data to the database
                    insertQuery.bindValue(":date_time", dateTime);
                    insertQuery.bindValue(":catalog_name", catalogName);
                    insertQuery.bindValue(":catalog_file_count", QString::number(catalogFileCount));
                    insertQuery.bindValue(":catalog_total_file_size", QString::number(catalogTotalFileSize));
                    insertQuery.bindValue(":record_type", recordType);
                    insertQuery.exec();
            }
        }
    }
    //----------------------------------------------------------------------
    void MainWindow::loadStatisticsStorageFileToTable()
    {// Load the contents of the storage statistics file into the database

        //clear database table
        QSqlQuery deleteQuery;
        deleteQuery.exec("DELETE FROM statistics_storage");

        // Get infos stored in the file
        QFile statisticsStorageFile(statisticsStorageFilePath);
        if(!statisticsStorageFile.open(QIODevice::ReadOnly)) {
            return;
        }

        QTextStream textStream(&statisticsStorageFile);

        //prepare query to load file info
        QSqlQuery insertQuery;
        QString insertSQL = QLatin1String(R"(
                                INSERT INTO statistics_storage (
                                                date_time,
                                                storage_id,
                                                storage_name,
                                                storage_free_space,
                                                storage_total_space,
                                                record_type )
                                VALUES(
                                                :date_time,
                                                :storage_id,
                                                :storage_name,
                                                :storage_free_space,
                                                :storage_total_space,
                                                :record_type )
                                            )");
        insertQuery.prepare(insertSQL);

        //set temporary values
        QString     line;
        QStringList fieldList;
        QString     dateTime;
        int         storageID;
        QString     storageName;
        qint64      storageFreeSpace;
        qint64      storageTotalSpace;
        QString     recordType;
        QRegularExpression tagExp("\t");

        //Skip titles line
        line = textStream.readLine();

        //load file to database
        while (!textStream.atEnd())
        {
            line = textStream.readLine();
            if (line.isNull())
                        break;
            else
            {
                        //Split the string with \t (tabulation) into a list
                        fieldList.clear();
                        fieldList = line.split(tagExp);
                        dateTime            = fieldList[0];
                        storageName         = fieldList[1];
                        storageFreeSpace    = fieldList[2].toLongLong();
                        storageTotalSpace   = fieldList[3].toLongLong();
                        storageID           = fieldList[4].toInt();
                        recordType          = fieldList[5];

                        //Append data to the database
                        insertQuery.bindValue(":date_time", dateTime);
                        insertQuery.bindValue(":storage_id", storageID);
                        insertQuery.bindValue(":storage_name", storageName);
                        insertQuery.bindValue(":storage_free_space", QString::number(storageFreeSpace));
                        insertQuery.bindValue(":storage_total_space", QString::number(storageTotalSpace));
                        insertQuery.bindValue(":record_type", recordType);
                        insertQuery.exec();
            }
        }
    }
    //----------------------------------------------------------------------
    void MainWindow::loadStatisticsChart()
    {// Plot the statistics data into a graph based on selections

        //Get inputs
            selectedTypeOfData = ui->Statistics_comboBox_TypeOfData->currentText();
            QString selectedSource = ui->Statistics_comboBox_SelectSource->currentText();
            QString selectedStorageforStats  = ui->Filters_label_DisplayStorage->text();
            QString selectedCatalogforStats  = ui->Filters_label_DisplayCatalog->text();

            qint64  maxValueGraphRange = 0.0;
            QString displayUnit;
            QLineSeries *series1 = new QLineSeries();
            QLineSeries *series2 = new QLineSeries();
            QLineSeries *series3 = new QLineSeries();
            bool    loadSeries1 = true;
            bool    loadSeries2 = true;
            bool    loadSeries3 = true;
            bool    invalidCombinaison = false;
            QString invalidCase;
            QString reportName;
            QString reportTypeOfData;
            qint64  numberOrSizeTotal = 0;
            qint64  freeSpace  = 0;
            qint64  totalSpace = 0;

            //Line formatting
            QRgb colorCatalogTotalSize = qRgb(32, 159, 223);
            QPen penCatalogTotalSize(colorCatalogTotalSize);
            penCatalogTotalSize.setWidth(2);
            QRgb colorCatalogNumberFiles = qRgb(146, 110, 228);
            QPen penCatalogNumberFiles(colorCatalogNumberFiles);
            penCatalogNumberFiles.setWidth(2);
            QRgb colorStorageTotalSpace = qRgb(153, 202, 83);
            QPen penStorageTotalSpace(colorStorageTotalSpace);
            penStorageTotalSpace.setWidth(2);
            QRgb colorStorageUsedSpace = qRgb(246, 166, 37);
            QPen penStorageUsedSpace(colorStorageUsedSpace);
            penStorageUsedSpace.setWidth(2);

        //Get the data depending on the type of source
            //Getting one catalog data
            if(selectedSource ==tr("catalog updates")){
                reportName = tr("Catalog updates");
                reportTypeOfData = selectedTypeOfData;

                QSqlQuery queryTotalSnapshots;
                QString querySQL = QLatin1String(R"(
                                    SELECT date_time, catalog_file_count, catalog_total_file_size
                                    FROM statistics_catalog
                                    WHERE catalog_name = :selectedCatalogforStats
                                    AND record_type != 'Storage'
                                  )");
                if ( !graphicStartDate.isNull() ){
                     querySQL = querySQL + " AND date_time > :graphStartDate ";
                }

                if (selectedDeviceType == "Location"){
                     invalidCombinaison = true;
                     invalidCase = tr("A Catalog should be selected for that report.");
                }
                else if (selectedDeviceType == "Storage"){
                     invalidCombinaison = true;
                     invalidCase = tr("A Catalog should be selected for that report.");
                }
                else if (selectedDeviceType == "Catalog"){
                    queryTotalSnapshots.prepare(querySQL);
                    queryTotalSnapshots.bindValue(":selectedCatalogforStats", selectedCatalogforStats);
                    queryTotalSnapshots.bindValue(":graphStartDate", graphicStartDate.date().toString("yyyy-MM-dd"));
                    queryTotalSnapshots.exec();

                    while (queryTotalSnapshots.next()){

                         //QDateTime datetime = QDateTime::fromString(queryTotalSnapshots.value(0).toString(),"yyyy-MM-dd hh:mm:ss");
                        QDateTime datetime = queryTotalSnapshots.value(0).toDateTime();

                        if ( selectedTypeOfData == tr("Number of Files") )
                       {
                           numberOrSizeTotal = queryTotalSnapshots.value(1).toLongLong();

                           if ( numberOrSizeTotal > maxValueGraphRange )
                               maxValueGraphRange = numberOrSizeTotal;
                       }
                        else if ( selectedTypeOfData == tr("Total File Size") )
                       {
                           numberOrSizeTotal = queryTotalSnapshots.value(2).toLongLong();
                           if ( freeSpace > 2000000000 ){
                               freeSpace = freeSpace/1024/1024/1024;
                               displayUnit = " ("+tr("GiB")+")";
                           }
                           else {
                               numberOrSizeTotal = numberOrSizeTotal/1024/1024;
                               displayUnit = " ("+tr("MiB")+")";
                           }

                           if ( numberOrSizeTotal > maxValueGraphRange )
                               maxValueGraphRange = numberOrSizeTotal;
                       }

                        series1->setName(selectedTypeOfData);
                        series1->append(datetime.toMSecsSinceEpoch(), numberOrSizeTotal);
                    }

                    loadSeries1 = true;
                    loadSeries2 = false;
                    loadSeries3 = false;

                    penStorageUsedSpace.setWidth(2);
                    if ( selectedTypeOfData == tr("Number of Files") )
                           series1->setPen(penCatalogNumberFiles);
                    else
                           series1->setPen(penCatalogTotalSize);
                }
            }

            //Getting the collection snapshots data
            else if(selectedSource == tr("collection snapshots")){
                reportName = tr("Collection snapshots");
                reportTypeOfData = selectedTypeOfData;

                //Add Catalog data
                QSqlQuery queryTotalSnapshots;
                QString querySQL = QLatin1String(R"(
                                    SELECT date_time, SUM(statistics_catalog.catalog_file_count), SUM(statistics_catalog.catalog_total_file_size)
                                    FROM statistics_catalog
                                    LEFT JOIN catalog ON catalog.catalog_name = statistics_catalog.catalog_name
                                    LEFT JOIN storage ON catalog.catalog_storage = storage.storage_name
                                    WHERE record_type = 'snapshot'
                                  )");

                if ( selectedDeviceName != tr("All") and selectedDeviceType=="Location" )
                    querySQL += " AND storage.storage_location = '" + selectedDeviceName + "' ";
                else if ( selectedDeviceName != tr("All") and selectedDeviceType=="Storage" )
                    querySQL += " AND catalog.catalog_storage = '" + selectedDeviceName + "' ";
                else if ( selectedDeviceName != tr("All") and selectedDeviceType=="Catalog" )
                    querySQL += " AND catalog.catalog_name = '" + selectedDeviceName + "' ";
                else
                    querySQL += " AND storage.storage_location !=''";


                if ( !graphicStartDate.isNull() ){
                     querySQL = querySQL + " AND date_time > :graphStartDate ";
                }

                querySQL = querySQL + " GROUP BY date_time ";

                queryTotalSnapshots.prepare(querySQL);
                queryTotalSnapshots.bindValue(":graphStartDate", graphicStartDate.date().toString("yyyy-MM-dd"));
                queryTotalSnapshots.exec();

                while (queryTotalSnapshots.next()){

                    QDateTime datetime = QDateTime::fromString(queryTotalSnapshots.value(0).toString(),"yyyy-MM-dd hh:mm:ss");

                    if ( selectedTypeOfData == tr("Number of Files") )
                    {
                       numberOrSizeTotal = queryTotalSnapshots.value(1).toLongLong();
                       if ( numberOrSizeTotal > maxValueGraphRange )
                           maxValueGraphRange = numberOrSizeTotal;
                    }
                    else if ( selectedTypeOfData == tr("Total File Size") )
                    {
                       numberOrSizeTotal = queryTotalSnapshots.value(2).toLongLong();
                       if ( numberOrSizeTotal > 2000000000 ){
                           numberOrSizeTotal = numberOrSizeTotal/1024/1024/1024;
                           displayUnit = " ("+tr("GiB")+")";
                       }
                       else {
                           numberOrSizeTotal = numberOrSizeTotal/1024/1024;
                           displayUnit = " ("+tr("MiB")+")";
                       }

                       if ( numberOrSizeTotal > maxValueGraphRange )
                           maxValueGraphRange = numberOrSizeTotal;
                    }

                    series1->append(datetime.toMSecsSinceEpoch(), numberOrSizeTotal);
                }

                series1->setName(tr("Catalogs") + " / "  + selectedTypeOfData);
                if ( selectedTypeOfData == tr("Number of Files") )
                    series1->setPen(penCatalogNumberFiles);
                else
                    series1->setPen(penCatalogTotalSize);

                //Add Storage data
                QSqlQuery queryStorageSnapshots;
                QString queryStorageSnapshotsSQL = QLatin1String(R"(
                                                        SELECT  date_time,
                                                                SUM(statistics_storage.storage_free_space),
                                                                SUM(statistics_storage.storage_total_space)
                                                        FROM statistics_storage
                                                        LEFT JOIN storage ON storage.storage_name = statistics_storage.storage_name
                                                    )");

                if ( selectedDeviceName != tr("All") and selectedDeviceType=="Location" ){
                        queryStorageSnapshotsSQL += "   WHERE record_type = 'snapshot' ";
                        queryStorageSnapshotsSQL += "   AND storage.storage_location = '" + selectedDeviceName + "' ";
                }
                else if ( selectedDeviceName != tr("All") and selectedDeviceType=="Storage" ){
                        queryStorageSnapshotsSQL += "   WHERE record_type = 'snapshot' ";
                        queryStorageSnapshotsSQL += "   AND storage.storage_name = '" + selectedDeviceName + "' ";
                }
                else if ( selectedDeviceName != tr("All") and selectedDeviceType=="Catalog" ){
                        queryStorageSnapshotsSQL += "   LEFT JOIN catalog ON catalog.catalog_storage = storage.storage_name ";
                        queryStorageSnapshotsSQL += "   WHERE record_type = 'snapshot' ";
                        queryStorageSnapshotsSQL += "   AND catalog.catalog_name = '" + selectedDeviceName + "' ";
                }
                if ( !graphicStartDate.isNull() ){
                       queryStorageSnapshotsSQL += "    AND date_time > :graphStartDate ";
                }

                queryStorageSnapshotsSQL += "           GROUP BY date_time ";

                queryStorageSnapshots.prepare(queryStorageSnapshotsSQL);
                queryStorageSnapshots.bindValue(":graphStartDate",graphicStartDate);
                queryStorageSnapshots.exec();

                while (queryStorageSnapshots.next()){
                    QDateTime datetime = QDateTime::fromString(queryStorageSnapshots.value(0).toString(),"yyyy-MM-dd hh:mm:ss");
                    freeSpace  = queryStorageSnapshots.value(1).toLongLong();
                    totalSpace = queryStorageSnapshots.value(2).toLongLong();
                    if ( selectedTypeOfData == tr("Number of Files") )
                    {
                        //Number of files does not apply to Storage
                        loadSeries2 = false;
                        loadSeries3 = false;
                    }
                    else if ( selectedTypeOfData == tr("Total File Size") )
                    {
                        if ( totalSpace > 2000000000 ){
                            freeSpace    = freeSpace/1024/1024/1024;
                            totalSpace   = totalSpace/1024/1024/1024;
                            displayUnit = " ("+tr("GiB")+")";
                        }
                        else {
                            freeSpace    = freeSpace/1024/1024;
                            totalSpace   = totalSpace/1024/1024;
                            displayUnit = " ("+tr("MiB")+")";
                        }

                        if ( totalSpace > maxValueGraphRange )
                            maxValueGraphRange = totalSpace;

                        series2->append(datetime.toMSecsSinceEpoch(), totalSpace);
                        series3->append(datetime.toMSecsSinceEpoch(), totalSpace-freeSpace);
                    }
                }

                //case: no catalog for the storage (display of the graph will fail if no point are in series1)
                if(series1->count()==0){
                    series1->append(0,0);
                }
                series2->setName(tr("Storage") + " / " + tr("Total space"));
                series3->setName(tr("Storage") + " / " + tr("Used space"));
            }

            //Getting one storage data
            else if(selectedSource ==tr("storage updates")){
                reportName = tr("Storage updates");
                reportTypeOfData = tr("Total File Size");

                QSqlQuery queryTotalSnapshots;
                QString querySQL = QLatin1String(R"(
                                                    SELECT date_time, SUM(storage_free_space), SUM(storage_total_space)
                                                    FROM statistics_storage sa
                                                    WHERE record_type = 'update'
                                                )");

                if ( !graphicStartDate.isNull() ){
                    querySQL += " AND date_time > :graphStartDate ";
                }

                if (selectedDeviceType == "Storage"){
                    querySQL += " AND storage_name = :selectedStorageforStats ";
                    querySQL += " GROUP BY date_time ";
                    queryTotalSnapshots.prepare(querySQL);
                    queryTotalSnapshots.bindValue(":selectedStorageforStats", selectedStorageforStats);
                    queryTotalSnapshots.bindValue(":graphStartDate", graphicStartDate.date().toString("yyyy-MM-dd"));
                }
                else if (selectedDeviceType == "Location"){
                    invalidCombinaison = true;
                    invalidCase = tr("A Storage or Catalog should be selected for that report.");
                }
                else if (selectedDeviceType == "Catalog"){
                    querySQL += " AND storage_name = :selectedStorageforStats ";
                    querySQL += " GROUP BY date_time ";
                    queryTotalSnapshots.prepare(querySQL);
                    queryTotalSnapshots.bindValue(":selectedStorageforStats", selectedCatalog->storageName);
                    queryTotalSnapshots.bindValue(":graphStartDate", graphicStartDate.date().toString("yyyy-MM-dd"));
                }

                queryTotalSnapshots.exec();

                while (queryTotalSnapshots.next()){

                       QDateTime datetime = QDateTime::fromString(queryTotalSnapshots.value(0).toString(),"yyyy-MM-dd hh:mm:ss");

                       freeSpace = queryTotalSnapshots.value(1).toLongLong();
                       if ( freeSpace > 2000000000 ){
                           freeSpace = freeSpace/1024/1024/1024;
                           displayUnit = " ("+tr("GiB")+")";
                       }
                       else {
                           freeSpace = freeSpace/1024/1024;
                           displayUnit = " ("+tr("MiB")+")";
                       }
                       if ( freeSpace > maxValueGraphRange )
                           maxValueGraphRange = freeSpace;


                       totalSpace = queryTotalSnapshots.value(2).toLongLong();
                       if ( totalSpace > 2000000000 ){
                           totalSpace = totalSpace/1024/1024/1024;
                           displayUnit = " ("+tr("GiB")+")";
                       }
                       else {
                           totalSpace = totalSpace/1024/1024;
                           displayUnit = " ("+tr("MiB")+")";
                       }

                       if ( totalSpace > maxValueGraphRange )
                           maxValueGraphRange = totalSpace;

                       series1->append(datetime.toMSecsSinceEpoch(), totalSpace-freeSpace);
                       series2->append(datetime.toMSecsSinceEpoch(), totalSpace);
                }

                series1->setName("Used Space");
                series1->setPen(penStorageUsedSpace);
                series2->setName("Total Space");
                series2->setPen(penStorageTotalSpace);

                //case: no catalog for the storage
                if(series1->count()==0){
                       series1->append(0,0);
                }

                loadSeries1 = true;
                loadSeries2 = true;
                loadSeries3 = false;
            }

        //Prepare the chart
            //Create new chart and prepare formating
            QChart *chart = new QChart();

            //Title
            chart->setTitle("<span style=\"font-weight: bold; font-size: 16px; font-color: #AAA,\">"
                            + reportName + "</span><br/>"
                            "<span style=\"font-weight: bold; font-size: 14px; font-color: #AAA,\">"
                            + selectedTypeOfData + " "
                            +" " + tr("of") + " <span style=\"font-style: italic; color: #000,\">"
                            + selectedDeviceName +"</span>"+ displayUnit+"</span>");

            //Format axis
            chart->setLocalizeNumbers(true);

            QDateTimeAxis *axisX = new QDateTimeAxis;
            //axisX->setTickCount(10);
            axisX->setFormat("yyyy-MM-dd");
            axisX->setTitleText(tr("Date"));
            chart->addAxis(axisX, Qt::AlignBottom);

            QValueAxis *axisY = new QValueAxis;
            axisY->setLabelFormat("%.0f");
            axisY->setTitleText(tr("Total"));

            //Calculate axisY max range value
                // Example: 848 365  >  get 6 digits, get the 8 and add one, and mutliply this by 10 power of 6-1 > so max range is 900 000
                //Get the number of digits
                int maxValueGraphRangeLength = QString::number((maxValueGraphRange)).length();

                //Get the first digit
                QString maxValueGraphRangeFirst = QString::number((maxValueGraphRange)).left(1);

                //Calculate the max range value
                maxValueGraphRange = (maxValueGraphRangeFirst.toLongLong()+1) * qPow(10, maxValueGraphRangeLength-1);

                //Increase max value from the statistics, so the highest value is not completely at the top
                //maxValueGraphRange = maxValueGraphRange*1.1;

            axisY->setRange(0 , maxValueGraphRange);
            chart->addAxis(axisY, Qt::AlignLeft);

        //Load series to chart
            if (loadSeries1==true){
                chart->addSeries(series1);
                series1->attachAxis(axisX);
                series1->attachAxis(axisY);
            }
            if (loadSeries2==true){
                chart->addSeries(series2);
                series2->attachAxis(axisX);
                series2->attachAxis(axisY);
            }
            if (loadSeries3==true){
                chart->addSeries(series3);
                series3->attachAxis(axisX);
                series3->attachAxis(axisY);
            }

        //Set the Legend
            chart->legend()->setVisible(true);
            chart->legend()->setAlignment(Qt::AlignLeft);
            chart->legend()->detachFromChart();
            chart->legend()->setBrush(QBrush(QColor(255, 255, 255, 220)));
            chart->legend()->setPen(QPen(QColor(192, 192, 192, 192)));
            chart->legend()->setBackgroundVisible(true);
            chart->legend()->setGeometry(QRectF(150, 100, 240, 90));
            chart->legend()->update();

            QFont font = chart->legend()->font();
            font.setPointSizeF(8);
            chart->legend()->setFont(font);

        //Load chart
            //clear if invalid combinaison
            if(invalidCombinaison == true){
                chart = new QChart();
                chart->setTitle("<p style=\"font-weight: bold; font-size: 18px; font-color: #AAA,\">"
                                + invalidCase + " "
                                +" <span style=\"font-style: italic; color: #000,\"></p>");
            }

            ui->Statistics_chartview_Graph1->setChart(chart);
            ui->Statistics_chartview_Graph1->setRubberBand(QChartView::RectangleRubberBand);
    }
    //----------------------------------------------------------------------
    void MainWindow::convertStatistics()
    {//Convert former statistics.csv to new catalog/storage csv files (v1.19>v1.20)

        QString statisticsFilePath = collectionFolder + "/" + "statistics.csv";
        QFile statisticsFile(statisticsFilePath);

        QFile statisticsCatalogFile(statisticsCatalogFilePath);
        QFile statisticsStorageFile(statisticsStorageFilePath);

        if (statisticsFile.open(QIODevice::ReadOnly|QIODevice::Text)
            and !statisticsCatalogFile.exists()
            and !statisticsStorageFile.exists()){

            //User confirmation
            int result = QMessageBox::warning(this,"Katalog",
                                                  tr("Katalog detected an old version to store device statistics.<br/> "
                                                     "- It is necessary to convert it to read it with this version.<br/>"
                                                     "- The old file will be renamed and kept after the conversion.<br/>"
                                                     "- It will not be possible to run it later if you run any update in the mean time.<br/>"
                                                     "You want to convert it now? <br/>"
                                                     ),
                                                  QMessageBox::Yes|QMessageBox::Cancel);

                if ( !(result == QMessageBox::Yes)){
                       return;
            }

            QTextStream textStream(&statisticsFile);
            //set temporary values
            QString     line;
            QStringList fieldList;

            QDateTime   dateTime;
            QString     catalogName;
            qint64      catalogFileCount;
            qint64      catalogTotalFileSize;
            QString     recordType;
            static QRegularExpression tagExp("\t");

            //Skip titles line
            line = textStream.readLine();

            //load file to database
            while (!textStream.atEnd())
            {
                   line = textStream.readLine();

                   if (line.isNull())
                       break;
                   else
                   {
                       //Split the string with \t (tabulation) into a list
                       fieldList.clear();
                       fieldList = line.split(tagExp);
                       dateTime                = QDateTime::fromString(fieldList[0],"yyyy-MM-dd hh:mm:ss");
                       catalogName             = fieldList[1];
                       catalogFileCount        = fieldList[2].toLongLong();
                       catalogTotalFileSize    = fieldList[3].toLongLong();
                       recordType              = fieldList[4];

                       if (recordType =="Storage"){//         = storage update
                            tempStorage = new Storage;
                            tempStorage->setName(catalogName);
                            tempStorage->setFreeSpace(catalogFileCount);
                            tempStorage->setTotalSpace(catalogTotalFileSize);
                            tempStorage->setDateUpdated(dateTime);
                            tempStorage->saveStatistics(dateTime);
                            tempStorage->saveStatisticsToFile(statisticsStorageFilePath,dateTime);
                       }
                       else if (recordType =="Update"){//      = catalog update
                            tempCatalog = new Catalog;
                            tempCatalog->setName(catalogName);
                            tempCatalog->setFileCount(catalogFileCount);
                            tempCatalog->setTotalFileSize(catalogTotalFileSize);
                            tempCatalog->setDateUpdated(dateTime);
                            tempCatalog->saveStatistics(dateTime);
                            tempCatalog->saveStatisticsToFile(statisticsCatalogFilePath,dateTime);
                       }
                       else if (recordType =="Snapshot"){//    = catalog snapshot
                            tempCatalog = new Catalog;
                            tempCatalog->setName(catalogName);
                            tempCatalog->setFileCount(catalogFileCount);
                            tempCatalog->setTotalFileSize(catalogTotalFileSize);
                            tempCatalog->saveStatistics(dateTime);
                            tempCatalog->saveStatisticsToFile(statisticsCatalogFilePath,dateTime);
                       }
                       else {
                            //qDebug()<<"line could not be processed: "+line;
                       }
                   }
            }
            statisticsFile.close();

            //rename old statistics file
            statisticsFile.rename(collectionFolder + "/" + "statistics_csv.bak");

            QMessageBox msgBox;
            msgBox.setWindowTitle("Katalog");
            msgBox.setText( tr("Conversion completed.") );
            msgBox.setIcon(QMessageBox::Information);
            msgBox.exec();
        }
    }
    //----------------------------------------------------------------------
    void MainWindow::saveStatiticsToFile()
    {
        //Prepare export file
        QFile statisticsFile(statisticsStorageFilePath);
        QTextStream out(&statisticsFile);

        //Get data
        QSqlQuery query;
        QString querySQL = QLatin1String(R"(
                                SELECT * FROM statistics_storage
                            )");
        query.prepare(querySQL);
        query.exec();

        //Iterate the records and generate lines
        while (query.next()) {
            const QSqlRecord record = query.record();
            for (int i=0, recCount = record.count() ; i<recCount ; ++i){
                   if (i>0)
                       out << '\t';
                   out << record.value(i).toString();
            }
            //-- Write the result in the file
            out << '\n';
        }

        if(statisticsFile.open(QIODevice::WriteOnly | QIODevice::Truncate)) {
            //out << textData;
            //Close the file
            //storageFile.close();
        }

        statisticsFile.close();
    }
    //----------------------------------------------------------------------
