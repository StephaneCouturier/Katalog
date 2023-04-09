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

        //save selection in settings file;
        QSettings settings(settingsFilePath, QSettings:: IniFormat);
        settings.setValue("Statistics/SelectedSource", selectedSource);

        //Display selection combo boxes depending on data source
        if (selectedSource ==tr("collection snapshots")){
            ui->Statistics_label_DataType->show();
            ui->Statistics_comboBox_TypeOfData->show();
        }
        else if(selectedSource ==tr("selected catalog")){
            ui->Statistics_label_DataType->show();
            ui->Statistics_comboBox_TypeOfData->show();
        }
        else if(selectedSource ==tr("storage")){
            ui->Statistics_label_DataType->hide();
            ui->Statistics_comboBox_TypeOfData->hide();
        }

        //load the graph
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
    void MainWindow::on_Statistics_pushButton_EditStatisticsFile_clicked()
    {
        //statisticsCatalogFilePath = collectionFolder + "/" + "statistics.csv";
        QDesktopServices::openUrl(QUrl::fromLocalFile(statisticsCatalogFilePath));
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
        graphicStartDate = ui->Statistics_lineEdit_GraphicStartDate->text();

        QSettings settings(settingsFilePath, QSettings:: IniFormat);
        settings.setValue("Statistics/graphStartDate", graphicStartDate);

        //load the graph
        loadStatisticsChart();
    }
    //----------------------------------------------------------------------
    void MainWindow::on_Statistics_pushButton_ClearDate_clicked()
    {
        graphicStartDate = "";
        ui->Statistics_lineEdit_GraphicStartDate->setText(graphicStartDate);

        QSettings settings(settingsFilePath, QSettings:: IniFormat);
        settings.setValue("Statistics/graphStartDate", graphicStartDate);

        //load the graph
        loadStatisticsChart();

        ui->Statistics_calendarWidget->hide();
    }
    //----------------------------------------------------------------------
    void MainWindow::on_Statistics_calendarWidget_clicked(const QDate &date)
    {
        graphicStartDate = QVariant(date).toString();
        ui->Statistics_lineEdit_GraphicStartDate->setText(graphicStartDate);

        QSettings settings(settingsFilePath, QSettings:: IniFormat);
        settings.setValue("Statistics/graphStartDate", graphicStartDate);

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
            ui->Statistics_comboBox_SelectSource->setItemData(0, "selected catalog", Qt::UserRole);
            ui->Statistics_comboBox_SelectSource->setItemData(1, "collection snapshots", Qt::UserRole);
            ui->Statistics_comboBox_SelectSource->setItemData(2, "storage", Qt::UserRole);

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
            deleteQuery.exec("DELETE FROM statistics");

        // Get infos stored in the file
            QFile statisticsCatalogFile(statisticsCatalogFilePath);
            if(!statisticsCatalogFile.open(QIODevice::ReadOnly)) {
                return;
            }

            QTextStream textStream(&statisticsCatalogFile);

        //prepare query to load file info
            QSqlQuery insertQuery;
            QString insertSQL = QLatin1String(R"(
                                INSERT INTO statistics (
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
            //QString selectedLocationforStats = ui->Filters_label_DisplayLocation->text();
            QString selectedStorageforStats  = ui->Filters_label_DisplayStorage->text();
            QString selectedCatalogforStats  = ui->Filters_label_DisplayCatalog->text();

            qint64 maxValueGraphRange = 0.0;
            QString displayUnit;
            QLineSeries *series1 = new QLineSeries();
            QLineSeries *series2 = new QLineSeries();
            qint64 numberOrSizeTotal = 0;
            qint64 freeSpace = 0;
            qint64 totalSpace = 0;

        //Get the data depending on the type of source
            //Getting one catalog data
            if(selectedSource ==tr("selected catalog")){
                QSqlQuery queryTotalSnapshots;
                QString querySQL = QLatin1String(R"(
                                    SELECT date_time, catalog_file_count, catalog_total_file_size
                                    FROM statistics
                                    WHERE catalog_name = :selectedCatalogforStats
                                    AND record_type != 'Storage'
                                  )");
                if ( graphicStartDate != "" ){
                     querySQL = querySQL + " AND date_time > :graphStartDate ";
                }

                queryTotalSnapshots.prepare(querySQL);
                queryTotalSnapshots.bindValue(":selectedCatalogforStats",selectedCatalogforStats);
                queryTotalSnapshots.bindValue(":graphStartDate",graphicStartDate);
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
            }

            //Getting the collection snapshots data
            else if(selectedSource == tr("collection snapshots")){
                //Add Catalog data
                QSqlQuery queryTotalSnapshots;
                QString querySQL = QLatin1String(R"(
                                    SELECT date_time, SUM(statistics.catalog_file_count), SUM(statistics.catalog_total_file_size)
                                    FROM statistics
                                    LEFT JOIN catalog ON catalog.catalog_name = statistics.catalog_name
                                    LEFT JOIN storage ON catalog.catalog_storage = storage.storage_name
                                    WHERE record_type = 'Snapshot'
                                  )");

                if ( selectedDeviceName != tr("All") and selectedDeviceType=="Location" )
                    querySQL = querySQL + " AND storage.storage_location = '" + selectedDeviceName + "' ";
                else if ( selectedDeviceName != tr("All") and selectedDeviceType=="Storage" )
                    querySQL = querySQL + " AND catalog.catalog_storage = '" + selectedDeviceName + "' ";
                else if ( selectedDeviceName != tr("All") and selectedDeviceType=="Catalog" )
                    querySQL = querySQL + " AND catalog.catalog_name = '" + selectedDeviceName + "' ";

                if ( graphicStartDate != "" ){
                     querySQL = querySQL + " AND date_time > :graphStartDate ";
                }

                querySQL = querySQL + " GROUP BY date_time ";

                queryTotalSnapshots.prepare(querySQL);
                queryTotalSnapshots.bindValue(":graphStartDate",graphicStartDate);
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

                       series1->setName(selectedTypeOfData);
                       series1->append(datetime.toMSecsSinceEpoch(), numberOrSizeTotal+1000000000);
                }
                //Add Storage data
                QSqlQuery queryStorageSnapshots;
                QString queryStorageSnapshotsSQL = QLatin1String(R"(
                                    SELECT date_time, SUM(statistics.catalog_file_count), SUM(statistics.catalog_total_file_size)
                                    FROM statistics
                                    LEFT JOIN catalog ON catalog.catalog_name = statistics.catalog_name
                                    LEFT JOIN storage ON catalog.catalog_storage = storage.storage_name
                                    WHERE record_type = 'Snapshot'
                                  )");

                if ( selectedDeviceName != tr("All") and selectedDeviceType=="Location" )
                       queryStorageSnapshotsSQL += " AND storage.storage_location = '" + selectedDeviceName + "' ";
                else if ( selectedDeviceName != tr("All") and selectedDeviceType=="Storage" )
                       queryStorageSnapshotsSQL += " AND catalog.catalog_storage = '" + selectedDeviceName + "' ";
                else if ( selectedDeviceName != tr("All") and selectedDeviceType=="Catalog" )
                       queryStorageSnapshotsSQL += " AND catalog.catalog_name = '" + selectedDeviceName + "' ";

                if ( graphicStartDate != "" ){
                       queryStorageSnapshotsSQL += " AND date_time > :graphStartDate ";
                }

                queryStorageSnapshotsSQL = queryStorageSnapshotsSQL + " GROUP BY date_time ";

                queryStorageSnapshots.prepare(querySQL);
                queryStorageSnapshots.bindValue(":graphStartDate",graphicStartDate);
                queryStorageSnapshots.exec();

                while (queryStorageSnapshots.next()){

                    QDateTime datetime = QDateTime::fromString(queryStorageSnapshots.value(0).toString(),"yyyy-MM-dd hh:mm:ss");

//                    if ( selectedTypeOfData == tr("Number of Files") ){
//                        numberOrSizeTotal = queryStorageSnapshots.value(1).toLongLong();

//                        if ( numberOrSizeTotal > maxValueGraphRange )
//                        maxValueGraphRange = numberOrSizeTotal;

//                    }
//                    else if ( selectedTypeOfData == tr("Total File Size") ){
                        numberOrSizeTotal = queryStorageSnapshots.value(2).toLongLong();
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
                    //}

                    series2->setName("Storage total space"); //selectedTypeOfData
                    series2->append(datetime.toMSecsSinceEpoch(), numberOrSizeTotal);
                }
            }

            //Getting the storage data
            else if(selectedSource ==tr("storage")){
                QSqlQuery queryTotalSnapshots;
                QString querySQL = QLatin1String(R"(
                                                    SELECT date_time, SUM(storage_free_space), SUM(storage_total_space)
                                                    FROM statistics_storage sa
                                                )");

                if (selectedDeviceType == "Storage"){
                    querySQL = querySQL + " WHERE record_type = 'update' ";
                    querySQL = querySQL + " AND storage_name = :selectedStorageforStats ";
                    querySQL = querySQL + " GROUP BY date_time ";
                    queryTotalSnapshots.prepare(querySQL);
                    queryTotalSnapshots.bindValue(":selectedStorageforStats", selectedStorageforStats);
                }
//                else if (selectedDeviceType == "Location"){
//                    querySQL = querySQL + " LEFT JOIN storage so ON sa.catalog_name = so.storage_name ";
//                    querySQL = querySQL + " WHERE record_type = 'Storage' ";
//                    querySQL = querySQL + " AND storage_location = :storage_location ";
//                    querySQL = querySQL + " GROUP BY date_time ";
//                    queryTotalSnapshots.prepare(querySQL);
//                    queryTotalSnapshots.bindValue(":storageLocation", selectedLocationforStats);
//                }
                else if (selectedDeviceType == "Catalog"){
                    querySQL = querySQL + " WHERE record_type = 'update' ";
                    querySQL = querySQL + " AND storage_name = :selectedStorageforStats ";
                    querySQL = querySQL + " GROUP BY date_time ";
                    queryTotalSnapshots.prepare(querySQL);
                    queryTotalSnapshots.bindValue(":selectedStorageforStats", selectedCatalog->storageName);
                }

                if ( graphicStartDate != "" ){
                    querySQL = querySQL + " AND date_time > :graphStartDate ";
                    queryTotalSnapshots.bindValue(":graphStartDate",graphicStartDate);
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
                       series1->setName("Used Space");
                       series2->setName("Total Space");
                   }
            }

        //Prepare the chart and plot the data
            //Create new chart and prepare formating
            QChart *chart = new QChart();
            chart->addSeries(series1);
            chart->addSeries(series2);

            chart->legend()->hide();

            if (selectedDeviceType == "Storage"){
                selectedCatalogforStats = selectedFilterStorageName;

            }
            else if (selectedDeviceType == "Catalog"){
                selectedCatalogforStats = selectedCatalog->storageName;

            }

            chart->setTitle("<p style=\"font-weight: bold; font-size: 18px; font-color: #AAA,\">"
                            + selectedTypeOfData + " "
                            +" " + tr("of") + " <span style=\"font-style: italic; color: #000,\">"+selectedCatalogforStats+"</span>"+ displayUnit+"</p>");

            if(selectedSource =="collection snapshots"){
                chart->setTitle("<p style=\"font-weight: bold; font-size: 18px; font-color: #AAA,\">"
                                + selectedTypeOfData + " "
                                +" " + tr("of") + " " + tr("collection") + displayUnit+"</p>");
            }
            else{
                chart->setTitle("<p style=\"font-weight: bold; font-size: 18px; font-color: #AAA,\">"
                                + selectedTypeOfData + " "
                                +" " + tr("of") + " <span style=\"font-style: italic; color: #000,\">"+selectedCatalogforStats+"</span>"+ displayUnit+"</p>");
            }

            //Format axis
            chart->setLocalizeNumbers(true);

            QDateTimeAxis *axisX = new QDateTimeAxis;
            //axisX->setTickCount(10);
            axisX->setFormat("yyyy-MM-dd");
            axisX->setTitleText(tr("Date"));
            chart->addAxis(axisX, Qt::AlignBottom);
            series1->attachAxis(axisX);
            series2->attachAxis(axisX);
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

            series1->attachAxis(axisY);
            series2->attachAxis(axisY);
            //Legend
                //chart->legend()->setAlignment(Qt::AlignRight);
                chart->legend()->setVisible(true);
                chart->legend()->setAlignment(Qt::AlignLeft);
                chart->legend()->detachFromChart();
                chart->legend()->setBrush(QBrush(QColor(255, 255, 255, 220)));
                chart->legend()->setPen(QPen(QColor(192, 192, 192, 192)));
                //chart->legend()->attachToChart();
                chart->legend()->setBackgroundVisible(true);
                chart->legend()->setGeometry(QRectF(120, 70, 200, 75));
                chart->legend()->update();

            ui->Statistics_chartview_Graph1->setChart(chart);
            ui->Statistics_chartview_Graph1->setRubberBand(QChartView::RectangleRubberBand);
    }
    //----------------------------------------------------------------------
