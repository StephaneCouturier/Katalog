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
// Purpose:     methods for the screen Statistics
// Description:
// Author:      Stephane Couturier
// Version:     1.01
/////////////////////////////////////////////////////////////////////////////
*/

#include "mainwindow.h"
#include "ui_mainwindow.h"
//#include "database.h"

#include <QDesktopServices>
#include <QDateTimeAxis>
#include <QtCharts/QBarSeries>
#include <QtCharts/QBarSet>
#include <QtCharts/QLineSeries>
#include <QtCharts/QBarCategoryAxis>
#include <QtCharts/QValueAxis>
//#include <QtCharts/QChartView>
//#include <QtCharts/QLegend>

//----------------------------------------------------------------------
void MainWindow::on_Statistics_comboBox_SelectSource_currentIndexChanged(const QString &selectedSource)
{
    //save selection in settings file;
    QSettings settings(settingsFilePath, QSettings:: IniFormat);
    settings.setValue("Statistics/SelectedSource", selectedSource);

    //load the graph
    loadStatisticsChart();
}
//----------------------------------------------------------------------
void MainWindow::on_Statistics_comboBox_SelectCatalog_currentIndexChanged(const QString &selectedCatalog)
{
    //save selection in settings file;
    QSettings settings(settingsFilePath, QSettings:: IniFormat);
    settings.setValue("Statistics/SelectedCatalog", selectedCatalog);

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
//----------------------------------------------------------------------
//----------------------------------------------------------------------
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
        //QMessageBox::information(this,"Katalog",tr("No statistic file found."));
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
        //QRegExp tagExp; tagExp.setPattern("\t");


        //skip titles
        line = textStream.readLine();

    //load file
    while (!textStream.atEnd())
    {
        line = textStream.readLine();
        if (line.isNull())
            break;
        else
            {  //if (line.left(1)!="<")

                //Split the string with \t (tabulation) into a list
                QRegExp tagExp("\t"); //setpattern
                fieldList.clear();
                fieldList = line.split(tagExp);

                fieldListCount = fieldList.count();
                //QMessageBox::information(this,"Katalog","fieldListCount : \n" + QString::number(fieldListCount));

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

    selectedTypeOfData = ui->Statistics_comboBox_TypeOfData->currentText();
    QString selectedSource = ui->Statistics_comboBox_SelectSource->currentText();

    QString statisticsFilePath = collectionFolder + "/" + "statistics.csv";
    QString selectedCatalogforStats = ui->Statistics_comboBox_SelectCatalog->currentText();
    qint64 maxValueGraphRange = 0.0;
    QString displayUnit;
    QLineSeries *series = new QLineSeries();
    qint64 number = 0;

    //Get the data
    if(selectedSource ==tr("selected catalog")){
        //DEV note: this code is getting the selected catalog from Catalog screen and parses the statistics file.
        //DEV note: this can be replaced by getting the data from the internal database instead.

        QSqlQuery queryTotalSnapshots;
        QString querySQL = QLatin1String(R"(
                                            SELECT dateTime, catalogFileCount, catalogTotalFileSize
                                            FROM statistics
                                            WHERE catalogName = :selectedCatalogforStats
                                        )");
        queryTotalSnapshots.prepare(querySQL);
        queryTotalSnapshots.bindValue(":selectedCatalogforStats",selectedCatalogforStats);
        queryTotalSnapshots.exec();


        while (queryTotalSnapshots.next()){

               QDateTime datetime = QDateTime::fromString(queryTotalSnapshots.value(0).toString(),"yyyy-MM-dd hh:mm:ss");

               if ( selectedTypeOfData == tr("Number of Files") )
               {
                   number = queryTotalSnapshots.value(1).toLongLong();
                   //QMessageBox::information(this,"Katalog","Ok." + QString::number(number));
                   //maxValueGraphRange = 2000000;

                   if ( number > maxValueGraphRange )
                       maxValueGraphRange = number;

                  // QMessageBox::information(this,"Katalog","Ok." + QString::number(maxValueGraphRange));

               }
               else if ( selectedTypeOfData == tr("Total File Size") )
               {
                   number = queryTotalSnapshots.value(2).toLongLong();
                   if ( number > 2000000000 ){
                       number = number/1024/1024/1024;
                       displayUnit = " ("+tr("GiB")+")";
                   }
                   else {
                       number = number/1024/1024;
                       displayUnit = " ("+tr("MiB")+")";
                   }

                   if ( number > maxValueGraphRange )
                       maxValueGraphRange = number;
               }
               series->append(datetime.toMSecsSinceEpoch(), number);
        }
    }
    else if(selectedSource ==tr("collection snapshots")){

        QSqlQuery queryTotalSnapshots;
        QString querySQL = QLatin1String(R"(
                                            SELECT dateTime, SUM(catalogFileCount), SUM(catalogTotalFileSize)
                                            FROM statistics
                                            WHERE recordType = 'Snapshot'
                                            GROUP BY datetime
                                        )");
        queryTotalSnapshots.prepare(querySQL);
        queryTotalSnapshots.exec();


        while (queryTotalSnapshots.next()){

               QDateTime datetime = QDateTime::fromString(queryTotalSnapshots.value(0).toString(),"yyyy-MM-dd hh:mm:ss");

               if ( selectedTypeOfData == tr("Number of Files") )
               {
                   number = queryTotalSnapshots.value(1).toLongLong();
                   //QMessageBox::information(this,"Katalog","Ok." + QString::number(number));
                   //maxValueGraphRange = 2000000;

                   if ( number > maxValueGraphRange )
                       maxValueGraphRange = number;

                  // QMessageBox::information(this,"Katalog","Ok." + QString::number(maxValueGraphRange));

               }
               else if ( selectedTypeOfData == tr("Total File Size") )
               {
                   number = queryTotalSnapshots.value(2).toLongLong();
                   if ( number > 2000000000 ){
                       number = number/1024/1024/1024;
                       displayUnit = " ("+tr("GiB")+")";
                   }
                   else {
                       number = number/1024/1024;
                       displayUnit = " ("+tr("MiB")+")";
                   }

                   if ( number > maxValueGraphRange )
                       maxValueGraphRange = number;
               }

               series->append(datetime.toMSecsSinceEpoch(), number);

           }
    }

    //Prepare the chart and plot the data

    QChart *chart = new QChart();
    chart->addSeries(series);
    chart->legend()->hide();
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
    QDateTimeAxis *axisX = new QDateTimeAxis;
    //axisX->setTickCount(10);
    axisX->setFormat("yyyy-MM-dd");
    axisX->setTitleText(tr("Date"));
    chart->addAxis(axisX, Qt::AlignBottom);
    series->attachAxis(axisX);

    QValueAxis *axisY = new QValueAxis;
    axisY->setLabelFormat("%i");
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
    series->attachAxis(axisY);

    ui->Stats_chartview_Graph1->setChart(chart);
    ui->Stats_chartview_Graph1->setRubberBand(QChartView::RectangleRubberBand);

}
//----------------------------------------------------------------------
