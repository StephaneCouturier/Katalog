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
        QString selectedSource = ui->Statistics_comboBox_SelectSource->currentText();

        //Save selection in settings file;
        QSettings settings(collection->settingsFilePath, QSettings:: IniFormat);
        settings.setValue("Statistics/SelectedSource", selectedSource);

        //Load the graph
        loadStatisticsChart();
    }
    //----------------------------------------------------------------------
    void MainWindow::on_StatisticsComboBoxSelectCatalogCurrentIndexChanged(const QString &selectedCatalog)
    {
        //Save selection in settings file;
        QSettings settings(collection->settingsFilePath, QSettings:: IniFormat);
        settings.setValue("Statistics/SelectedCatalog", selectedCatalog);

        //Load the graph
        loadStatisticsChart();
    }
    //----------------------------------------------------------------------
    void MainWindow::on_Statistics_comboBox_TypeOfData_currentTextChanged()
    {
        QString typeOfData = ui->Statistics_comboBox_TypeOfData->itemData(ui->Statistics_comboBox_TypeOfData->currentIndex(),Qt::UserRole).toString();

        //Save selection in settings file;
        QSettings settings(collection->settingsFilePath, QSettings:: IniFormat);
        settings.setValue("Statistics/TypeOfData", typeOfData);

        //Load the graph
        loadStatisticsChart();
    }

    //----------------------------------------------------------------------
    void MainWindow::on_Statistics_pushButton_EditDeviceStatisticsFile_clicked()
    {
        QDesktopServices::openUrl(QUrl::fromLocalFile(collection->statisticsDeviceFilePath));
    }
    //----------------------------------------------------------------------
    void MainWindow::on_Statistics_pushButton_Reload_clicked()
    {
        loadStatisticsChart();
    }
    //----------------------------------------------------------------------
    void MainWindow::on_Statistics_lineEdit_GraphicStartDate_returnPressed()
    {
        graphicStartDate.setDate(QDate::fromString(ui->Statistics_lineEdit_GraphicStartDate->text(),"yyyy-MM-dd"));
        graphicStartDate.setTime(QTime::fromString("00:00:00"));

        QSettings settings(collection->settingsFilePath, QSettings:: IniFormat);
        settings.setValue("Statistics/graphStartDate", graphicStartDate.toString("yyyy-MM-dd"));

        //Load the graph
        loadStatisticsChart();
    }
    //----------------------------------------------------------------------
    void MainWindow::on_Statistics_pushButton_ClearDate_clicked()
    {
        graphicStartDate = *new QDateTime;
        ui->Statistics_lineEdit_GraphicStartDate->setText("");

        QSettings settings(collection->settingsFilePath, QSettings:: IniFormat);
        settings.setValue("Statistics/graphStartDate", "");

        //Load the graph
        loadStatisticsChart();

        ui->Statistics_calendarWidget->hide();
    }
    //----------------------------------------------------------------------
    void MainWindow::on_Statistics_calendarWidget_clicked(const QDate &date)
    {
        graphicStartDate.setDate(date);
        graphicStartDate.setTime(QTime::fromString("00:00:00"));
        ui->Statistics_lineEdit_GraphicStartDate->setText(graphicStartDate.date().toString("yyyy-MM-dd"));

        QSettings settings(collection->settingsFilePath, QSettings:: IniFormat);
        settings.setValue("Statistics/graphStartDate", graphicStartDate.date().toString("yyyy-MM-dd"));

        //Load the graph
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
    {//Populate the Statistics comboxboxes

        //Populate the comboxbox for selected source

            //Get last value
            QSettings settings(collection->settingsFilePath, QSettings:: IniFormat);
            QString lastSelectedSourceValue = settings.value("Statistics/SelectedSource").toString();

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
    void MainWindow::loadStatisticsChart()
    {// Plot the statistics data into a graph based on selections

        //Get inputs
            selectedTypeOfData = ui->Statistics_comboBox_TypeOfData->currentText();
            QString selectedSource = ui->Statistics_comboBox_SelectSource->currentText();
            qint64  maxValueGraphRange = 0;
            QString displayUnitText = "B";
            QLineSeries *series1 = new QLineSeries(); //Catalog data
            QLineSeries *series2 = new QLineSeries(); //Storage used space
            QLineSeries *series3 = new QLineSeries(); //Storage total space
            bool    loadSeries1 = true;
            bool    loadSeries2 = true;
            bool    loadSeries3 = true;
            bool    invalidCombinaison = false;
            QString invalidCase;
            QString reportName;
            QString reportTypeOfData;
            qreal   numberOrSizeTotal = 0;
            qreal   freeSpace  = 0;
            qreal   totalSpace = 0;

        //Prepare series formatting
            QRgb colorCatalogTotalSize = qRgb(32, 159, 223);
            QPen penCatalogTotalSize(colorCatalogTotalSize);
            penCatalogTotalSize.setWidth(2);
            QRgb colorCatalogNumberFiles = qRgb(146, 110, 228);
            QPen penCatalogNumberFiles(colorCatalogNumberFiles);
            penCatalogNumberFiles.setWidth(2);
            QRgb colorStorageUsedSpace = qRgb(246, 166, 37);
            QPen penStorageUsedSpace(colorStorageUsedSpace);
            penStorageUsedSpace.setWidth(2);
            QRgb colorStorageTotalSpace = qRgb(153, 202, 83);
            QPen penStorageTotalSpace(colorStorageTotalSpace);
            penStorageTotalSpace.setWidth(2);

        //Scale and unit setting
            qint64 sizeDivider = 1;
            QSqlQuery queryMaxValue;
            QString queryMaxValueSQL = QLatin1String(R"(
                                    SELECT MAX(device_total_file_size), MAX(device_total_space)
                                    FROM statistics_device
                                    WHERE device_id =:device_id
                                )");
            queryMaxValue.prepare(queryMaxValueSQL);
            queryMaxValue.bindValue(":device_id", selectedDevice->ID);
            queryMaxValue.exec();
            queryMaxValue.next();
            qint64 maxTotalFileSize = queryMaxValue.value(0).toLongLong();
            qint64 maxTotalSpace    = queryMaxValue.value(1).toLongLong();
            qint64 maxValue = qMax(maxTotalFileSize, maxTotalSpace);

            if (maxValue >= qint64(1024) * 1024 * 1024 * 1024) {
                sizeDivider = qint64(1024) * 1024 * 1024 * 1024;
                displayUnitText = " ("+tr("TiB")+")";
            }
            else if (maxValue >= qint64(1024) * 1024 * 1024) {
                sizeDivider = qint64(1024) * 1024 * 1024;
                displayUnitText = " ("+tr("GiB")+")";
            }
            else if (maxValue >= qint64(1024) * 1024) {
                sizeDivider = qint64(1024) * 1024;
                displayUnitText = " ("+tr("MiB")+")";
            }
            else if (maxValue >= qint64(1024)) {
                sizeDivider = qint64(1024);
                displayUnitText = " ("+tr("KiB")+")";
            }

        //Get the data depending on the type of source

            //Get virtual device data
            //else if(selectedSource ==tr("all records")){
                reportName = tr("All device records");
                reportTypeOfData = tr("Total File Size");

                QSqlQuery queryStatistics;
                QString queryStatisticsSQL = QLatin1String(R"(
                                            SELECT date_time, device_file_count, device_total_file_size, device_free_space, device_total_space
                                            FROM statistics_device
                                            WHERE device_id =:device_id
                                    )");

                // Add conditions
                    //graph start date
                    if (!graphicStartDate.isNull()) {
                        queryStatisticsSQL += " AND date_time > :graphStartDate ";
                    }

                    //data source
                    if(selectedSource ==tr("snapshots only")){
                        queryStatisticsSQL += " AND record_type = 'snapshot' ";
                    }
                    else if(selectedSource ==tr("updates only")){
                        queryStatisticsSQL += " AND (record_type = 'update' OR record_type = 'create') ";
                    }

                queryStatistics.prepare(queryStatisticsSQL);
                queryStatistics.bindValue(":device_id", selectedDevice->ID);
                queryStatistics.bindValue(":graphStartDate", graphicStartDate.toString("yyyy-MM-dd") + " 00:00:00");
                queryStatistics.exec();

                while (queryStatistics.next()){

                    QDateTime datetime = QDateTime::fromString(queryStatistics.value(0).toString(),"yyyy-MM-dd hh:mm:ss");

                    if ( selectedTypeOfData == tr("Number of Files") )
                    {
                        numberOrSizeTotal = queryStatistics.value(1).toLongLong();
                        loadSeries2 = false;
                        loadSeries3 = false;

                    }
                    else if ( selectedTypeOfData == tr("Total File Size") )
                    {
                        numberOrSizeTotal = static_cast<qreal>(queryStatistics.value(2).toLongLong()) / sizeDivider;
                        if ( numberOrSizeTotal > maxValueGraphRange )
                            maxValueGraphRange = numberOrSizeTotal;
                    }

                    series1->append(datetime.toMSecsSinceEpoch(), numberOrSizeTotal);

                    freeSpace = static_cast<qreal>(queryStatistics.value(3).toLongLong()) / sizeDivider;

                    totalSpace = static_cast<qreal>(queryStatistics.value(4).toLongLong()) / sizeDivider;

                    series2->append(datetime.toMSecsSinceEpoch(), totalSpace - freeSpace );
                    series3->append(datetime.toMSecsSinceEpoch(), totalSpace);

                    if ( numberOrSizeTotal > maxValueGraphRange )
                        maxValueGraphRange = numberOrSizeTotal;

                    if ( freeSpace > maxValueGraphRange )
                        maxValueGraphRange = freeSpace;

                    if ( totalSpace > maxValueGraphRange )
                        maxValueGraphRange = totalSpace;
                }

                //Series name and color               
                series1->setName(tr("Catalogs") + " / "  + selectedTypeOfData);
                if ( selectedTypeOfData == tr("Number of Files") )
                    series1->setPen(penCatalogNumberFiles);
                else
                    series1->setPen(penCatalogTotalSize);

                if (selectedDevice->type == "Catalog"){
                    loadSeries2 = false;
                    loadSeries3 = false;
                }

                series2->setName(tr("Storage") + " / " + tr("Used space"));
                series2->setPen(penStorageUsedSpace);
                series3->setName(tr("Storage") + " / " + tr("Total space"));
                series3->setPen(penStorageTotalSpace);

        //Prepare the chart
            //Create new chart and prepare formating
            QChart *chart = new QChart();

            //Title
            chart->setTitle("<span style=\"font-weight: bold; font-size: 16px; font-color: #AAA,\">"
                            + reportName + "</span><br/>"
                            "<span style=\"font-weight: bold; font-size: 14px; font-color: #AAA,\">"
                            + selectedTypeOfData + " "
                            +" " + tr("of") + " <span style=\"font-style: italic; color: #000,\">"
                            + selectedDevice->name +"</span>"+ displayUnitText +"</span>");

            //Format axis
            chart->setLocalizeNumbers(true);

            QDateTimeAxis *axisX = new QDateTimeAxis;
            axisX->setFormat("yyyy-MM-dd");
            axisX->setTitleText(tr("Date"));
            chart->addAxis(axisX, Qt::AlignBottom);

            QValueAxis *axisY = new QValueAxis;
            axisY->setTitleText(tr("Total"));

            //Calculate axisY max range value and format it
                // Example: 848 365  >  get 6 digits, get the 8 and add one,
                // and mutliply this by 10 power of 6-1 > so max range is 900 000

                //Get the number of digits
                int maxValueGraphRangeLength = QString::number((maxValueGraphRange)).length();

                //Get the first digit
                QString maxValueGraphRangeFirst = QString::number((maxValueGraphRange)).left(1);

                //Calculate the max range value
                maxValueGraphRange = static_cast<qreal>(maxValueGraphRangeFirst.toLongLong()+1) * qPow(10, maxValueGraphRangeLength-1);

                // Calculate the step size for the scale
                qreal stepSize = static_cast<qreal>(maxValueGraphRange) / 10.0;

                // Set the step size for the axis
                axisY->setTickCount(10); // Number of divisions on the axis
                axisY->setRange(0, maxValueGraphRange + stepSize); // Set the maximum value slightly larger to include the last division

                // Set the label format to display decimals when numbers are small
                if (maxValueGraphRange < 10.0){
                    axisY->setLabelFormat("%.1f");
                }
                else
                    axisY->setLabelFormat("%.0f");

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

        if(collection->databaseMode=="Memory"){

            QString statisticsFilePath = collection->collectionFolder + "/" + "statistics.csv";
            QFile statisticsFile(statisticsFilePath);

            QFile statisticsCatalogFile(collection->statisticsCatalogFilePath);
            QFile statisticsStorageFile(collection->statisticsStorageFilePath);

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

                           Storage *tempStorage    = new Storage();

                           if (recordType =="Storage"){//         = storage update
                                tempStorage = new Storage;
                                tempStorage->name = catalogName;
                                tempStorage->freeSpace = catalogFileCount;
                                tempStorage->totalSpace = catalogTotalFileSize;
                                tempStorage->dateTimeUpdated = dateTime;
                                tempStorage->saveStatistics(dateTime);
                                tempStorage->saveStatisticsToFile(collection->statisticsStorageFilePath,dateTime);
                           }
                           else if (recordType =="Update"){//      = catalog update
                                Catalog tempCatalog;
                                tempCatalog.name = catalogName;
                                tempCatalog.fileCount = catalogFileCount;
                                tempCatalog.totalFileSize = catalogTotalFileSize;
                                tempCatalog.setDateUpdated(dateTime);
                                tempCatalog.saveStatistics(dateTime);
                                tempCatalog.saveStatisticsToFile(collection->statisticsCatalogFilePath,dateTime);
                           }
                           else if (recordType =="Snapshot"){//    = catalog snapshot
                                Catalog tempCatalog;
                                tempCatalog.name = catalogName;
                                tempCatalog.fileCount = catalogFileCount;
                                tempCatalog.totalFileSize = catalogTotalFileSize;
                                tempCatalog.saveStatistics(dateTime);
                                tempCatalog.saveStatisticsToFile(collection->statisticsCatalogFilePath,dateTime);
                           }
                           else {
                                //qDebug()<<"line could not be processed: "+line;
                           }
                       }
                }
                statisticsFile.close();

                //rename old statistics file
                statisticsFile.rename(collection->collectionFolder + "/" + "statistics_csv.bak");

                QMessageBox msgBox;
                msgBox.setWindowTitle("Katalog");
                msgBox.setText( tr("Conversion completed.") );
                msgBox.setIcon(QMessageBox::Information);
                msgBox.exec();
            }
        }
    }
    //----------------------------------------------------------------------
