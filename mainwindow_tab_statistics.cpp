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

#include <QDesktopServices>
#include <QDateTimeAxis>
#include <QtCharts/QBarSeries>
#include <QtCharts/QBarSet>
#include <QtCharts/QLineSeries>
#include <QtCharts/QBarCategoryAxis>
#include <QtCharts/QValueAxis>

//UI------------------------------------------------------------------------

    void MainWindow::on_Statistics_comboBox_SelectSource_currentIndexChanged(const QString &selectedSource)
    {
        //save selection in settings file;
        QSettings settings(settingsFilePath, QSettings:: IniFormat);
        settings.setValue("Statistics/SelectedSource", selectedSource);

        //Display selection combo boxes depending on data source
        if (selectedSource ==tr("collection snapshots")){
//            ui->Statistics_label_Catalog->hide();
//            ui->Statistics_comboBox_SelectCatalog->hide();
            ui->Statistics_label_DataType->show();
            ui->Statistics_comboBox_TypeOfData->show();
        }
        else if(selectedSource ==tr("selected catalog")){
//            ui->Statistics_label_Catalog->show();
//            ui->Statistics_comboBox_SelectCatalog->show();
            ui->Statistics_label_DataType->show();
            ui->Statistics_comboBox_TypeOfData->show();
        }
        else if(selectedSource ==tr("storage")){
//            ui->Statistics_label_Catalog->hide();
//            ui->Statistics_comboBox_SelectCatalog->hide();
            ui->Statistics_label_DataType->hide();
            ui->Statistics_comboBox_TypeOfData->hide();
        }

        //load the graph
        loadStatisticsChart();
    }
    //----------------------------------------------------------------------
    void MainWindow::on_Statistics_comboBox_SelectCatalog_currentIndexChanged(const QString &selectedCatalog)
    {
        //save selection in settings file;
        QSettings settings(settingsFilePath, QSettings:: IniFormat);
        settings.setValue("Statistics/SelectedCatalog", selectedCatalog);

        selectedCatalogName = selectedCatalog;

        //load the graph
        loadStatisticsChart();
    }
    //----------------------------------------------------------------------
    void MainWindow::on_Statistics_comboBox_TypeOfData_currentIndexChanged(const QString &typeOfData)
    {
        //save selection in settings file;
        QSettings settings(settingsFilePath, QSettings:: IniFormat);
        settings.setValue("Statistics/TypeOfData", typeOfData);

        //load the graph
        loadStatisticsChart();
    }
    //----------------------------------------------------------------------
    void MainWindow::on_Statistics_pushButton_EditStatisticsFile_clicked()
    {
        statisticsFilePath = collectionFolder + "/" + "statistics.csv";
        QDesktopServices::openUrl(QUrl::fromLocalFile(statisticsFilePath));
    }
    //----------------------------------------------------------------------
    void MainWindow::on_Statistics_pushButton_Reload_clicked()
    {
        loadStatisticsData();
        loadStatisticsChart();
    }
    //----------------------------------------------------------------------

//Methods-------------------------------------------------------------------

    void MainWindow::loadStatisticsDataTypes()
    {   //Populate the comboxbox for types of data

        //Get last value
        QSettings settings(settingsFilePath, QSettings:: IniFormat);
        QString lastValue = settings.value("Statistics/TypeOfData").toString();

        //Generate list of values
        typeOfData << tr("Number of Files") << tr("Total File Size");
        listModel = new QStringListModel(this);
        listModel->setStringList(typeOfData);
        ui->Statistics_comboBox_TypeOfData->setModel(listModel);

        //Restore last selection value or default
        if (lastValue=="")
            ui->Statistics_comboBox_TypeOfData->setCurrentText(typeOfData[1]);
        else
            ui->Statistics_comboBox_TypeOfData->setCurrentText(lastValue);
    }
    //----------------------------------------------------------------------
    void MainWindow::loadStatisticsData()
    {
        // Load the contents of the statistics file into the database

        //clear database table
            QSqlQuery deleteQuery;
            deleteQuery.exec("DELETE FROM statistics");

        // Get infos stored in the file
            QFile statisticsFile(statisticsFilePath);
            if(!statisticsFile.open(QIODevice::ReadOnly)) {
                return;
            }

            QTextStream textStream(&statisticsFile);

        //prepare query to load file info
            QSqlQuery insertQuery;
            QString insertSQL = QLatin1String(R"(
                                INSERT INTO statistics (
                                                dateTime,
                                                catalogName,
                                                catalogFileCount,
                                                catalogTotalFileSize,
                                                recordType )
                                VALUES(
                                                :dateTime,
                                                :catalogName,
                                                :catalogFileCount,
                                                :catalogTotalFileSize,
                                                :recordType )
                                            )");
            insertQuery.prepare(insertSQL);

        //set temporary values
            QString     line;
            QStringList fieldList;
            int         fieldListCount;

            QString     dateTime;
            QString     catalogName;
            qint64      catalogFileCount;
            qint64      catalogTotalFileSize;
            QString     recordType;

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
                    QRegExp tagExp("\t"); //setpattern
                    fieldList.clear();
                    fieldList = line.split(tagExp);
                    fieldListCount = fieldList.count();
                    dateTime                = fieldList[0];
                    catalogName             = fieldList[1];
                    catalogFileCount        = fieldList[2].toLongLong();
                    catalogTotalFileSize    = fieldList[3].toLongLong();
                    if ( fieldListCount >4 ){
                        recordType          = fieldList[4];
                    }


                        //Append data to the database
                        insertQuery.bindValue(":dateTime", dateTime);
                        insertQuery.bindValue(":catalogName", catalogName);
                        insertQuery.bindValue(":catalogFileCount", QString::number(catalogFileCount));
                        insertQuery.bindValue(":catalogTotalFileSize", QString::number(catalogTotalFileSize));
                        insertQuery.bindValue(":recordType", recordType);
                        insertQuery.exec();
            }
        }
    }
    //----------------------------------------------------------------------
    void MainWindow::loadStatisticsChart()
    {
        // Plot the statistics data into a graph based on selections

        //Get inputs
            selectedTypeOfData = ui->Statistics_comboBox_TypeOfData->currentText();
            QString selectedSource = ui->Statistics_comboBox_SelectSource->currentText();

            //QString statisticsFilePath = collectionFolder + "/" + "statistics.csv";
            QString selectedStorageforStats = ui->Filters_label_DisplayStorage->text();
            QString selectedCatalogforStats = ui->Filters_label_DisplayCatalog->text();; // activeCatalog->name;//ui->Statistics_comboBox_SelectCatalog->currentText();
            //            activeCatalog->setCatalogName(selectedCatalogPath);
            //            activeCatalog->loadCatalogMetaData();
//            if (selectedDeviceType == "Catalog"){
//                selectedStorageforStats = activeCatalog->storage;
//            }

            qint64 maxValueGraphRange = 0.0;
            QString displayUnit;
            QLineSeries *series1 = new QLineSeries();
            QLineSeries *series2 = new QLineSeries();
            qint64 numberOrSizeTotal = 0;
            qint64 freeSpace = 0;
            qint64 totalSpace = 0;

        //Get the data
            //Getting one catalog data
            if(selectedSource ==tr("selected catalog")){
                QSqlQuery queryTotalSnapshots;
                QString querySQL = QLatin1String(R"(
                                    SELECT dateTime, catalogFileCount, catalogTotalFileSize
                                    FROM statistics
                                    WHERE catalogName = :selectedCatalogforStats
                                    AND recordType != 'Storage'
                                  )");
                queryTotalSnapshots.prepare(querySQL);
                queryTotalSnapshots.bindValue(":selectedCatalogforStats",selectedCatalogforStats);
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

                QSqlQuery queryTotalSnapshots;
                QString querySQL = QLatin1String(R"(
                                    SELECT dateTime, SUM(statistics.catalogFileCount), SUM(statistics.catalogTotalFileSize)
                                    FROM statistics
                                    LEFT JOIN catalog ON catalog.catalogName = statistics.catalogName
                                    LEFT JOIN storage ON catalog.catalogStorage = storage.storageName
                                    WHERE recordType = 'Snapshot'
                                  )");

                if ( selectedStorageLocation != tr("All") and selectedDeviceType=="Location" )
                    querySQL = querySQL + " AND storage.storageLocation = '" + selectedStorageLocation + "' ";
                else if ( selectedStorageName != tr("All") and selectedDeviceType=="Storage" )
                    querySQL = querySQL + " AND catalog.catalogStorage = '" + selectedStorageName + "' ";
                else if ( selectedCatalogName != tr("All") and selectedDeviceType=="Catalog" )
                    querySQL = querySQL + " AND catalog.catalogName = '" + selectedCatalogName + "' ";

                //add last part
                querySQL = querySQL + " GROUP BY datetime ";

                queryTotalSnapshots.prepare(querySQL);
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
                       series1->append(datetime.toMSecsSinceEpoch(), numberOrSizeTotal);
                   }
            }

            //Getting the storage data
            else if(selectedSource ==tr("storage")){

                QSqlQuery queryTotalSnapshots;
                QString querySQL = QLatin1String(R"(
                                                    SELECT dateTime, SUM(catalogFileCount), SUM(catalogTotalFileSize)
                                                    FROM statistics sa
                                                    WHERE recordType = 'Storage'
                                                )");
//
//                LEFT JOIN storage so ON sa.catalogName = so.storageName
//
//                if (selectedDeviceType == "Location"){
//                    querySQL = querySQL + " AND storageLocation = :storageLocation ";
//                    querySQL = querySQL + " GROUP BY datetime ";
//                    queryTotalSnapshots.prepare(querySQL);
//                    queryTotalSnapshots.bindValue(":storageLocation",selectedLocation);
//                }

                if (selectedDeviceType == "Storage"){
                    querySQL = querySQL + " AND catalogName = :selectedStorageforStats ";
                    querySQL = querySQL + " GROUP BY datetime ";
                    queryTotalSnapshots.prepare(querySQL);
                    queryTotalSnapshots.bindValue(":selectedStorageforStats",selectedStorageforStats);
                }
                else if (selectedDeviceType == "Catalog"){
                    querySQL = querySQL + " AND catalogName = :selectedStorageforStats ";
                    querySQL = querySQL + " GROUP BY datetime ";
                    queryTotalSnapshots.prepare(querySQL);
                    queryTotalSnapshots.bindValue(":selectedStorageforStats",activeCatalog->storageName);
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
            if(selectedSource ==tr("storage")){
                chart->addSeries(series2);
            }
            chart->legend()->hide();

            if (selectedDeviceType == "Storage"){
                selectedCatalogforStats = selectedStorageName;

            }
            else if (selectedDeviceType == "Catalog"){
                selectedCatalogforStats = activeCatalog->storageName;

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
