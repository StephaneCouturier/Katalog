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
// Version:     1.00
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
//#include <QtCharts/QChartView>
//#include <QtCharts/QLegend>

void MainWindow::on_Statistics_comboBox_SelectCatalog_currentIndexChanged()
{
    loadStatisticsChart();
}
//----------------------------------------------------------------------
void MainWindow::on_Statistics_comboBox_TypeOfData_currentIndexChanged()
{
    loadStatisticsChart();
}
//----------------------------------------------------------------------
void MainWindow::on_Statistics_pushButton_EditStatisticsFile_clicked()
{
    QString statisticsFilePath = collectionFolder + "/" + "statistics.csv";
    QDesktopServices::openUrl(QUrl::fromLocalFile(statisticsFilePath));
}
//----------------------------------------------------------------------
void MainWindow::on_Statistics_pushButton_Reload_clicked()
{
    loadStatisticsChart();
}
//----------------------------------------------------------------------

//----------------------------------------------------------------------
void MainWindow::loadStatisticsDataTypes()
{
    typeOfData << tr("Number of Files") << tr("Total File Size");
    listModel = new QStringListModel(this);
    listModel->setStringList(typeOfData);
    ui->Statistics_comboBox_TypeOfData->setModel(listModel);
    ui->Statistics_comboBox_TypeOfData->setCurrentText(typeOfData[1]);
}

void MainWindow::loadStatisticsChart()
{
    selectedTypeOfData = ui->Statistics_comboBox_TypeOfData->currentText();

    QString statisticsFilePath = collectionFolder + "/" + "statistics.csv";
    QString selectedCatalogforStats = ui->Statistics_comboBox_SelectCatalog->currentText();
    qreal maxValueGraphRange = 0.0;
    QString displayUnit;
    QLineSeries *series = new QLineSeries();

    QFile statisticsFile(statisticsFilePath);
    if (!statisticsFile.open(QIODevice::ReadOnly | QIODevice::Text)) {
        return;
    }

    QTextStream stream(&statisticsFile);
    while (!stream.atEnd()) {
        QString line = stream.readLine();

        QStringList values = line.split("\t");
        qint64 number = 0;

        if ( selectedCatalogforStats  == values[1] ){
            QDateTime datetime = QDateTime::fromString(values[0],"yyyy-MM-dd hh:mm:ss");
            if ( selectedTypeOfData == tr("Number of Files") )
            {
                number = values[2].toLongLong();
                displayUnit = "";
                series->append(datetime.toMSecsSinceEpoch(), number);
                if ( number > maxValueGraphRange )
                    maxValueGraphRange = number;
            }
            else if ( selectedTypeOfData == tr("Total File Size") )
            {
                number = values[3].toLongLong();
                if ( number > 2000000000 ){
                    number = number/1024/1024/1024;
                    displayUnit = " ("+tr("GiB")+")";
                }
                else {
                    number = number/1024/1024;
                    displayUnit = " ("+tr("MiB")+")";
                }
                series->append(datetime.toMSecsSinceEpoch(), number);
                if ( number > maxValueGraphRange )
                    maxValueGraphRange = number;
            }
            else return;
        }
    }
    statisticsFile.close();

    QChart *chart = new QChart();
    chart->addSeries(series);
    chart->legend()->hide();
    chart->setTitle("<p style=\"font-weight: bold; font-size: 18px; font-color: #AAA,\">"
                    + selectedTypeOfData + " "
                    +" " + tr("of") + " <span style=\"font-style: italic; color: #000,\">"+selectedCatalogforStats+"</span>"+ displayUnit+"</p>");

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

        //Increase max value from the statistics so the highest value is not completely at the top
        //maxValueGraphRange = maxValueGraphRange*1.1;

        //Get the number of digits
        int maxValueGraphRangeLength = QString::number((maxValueGraphRange)).length();
        //Get the first digit
        QVariant maxValueGraphRangeFirst = QString::number((maxValueGraphRange)).left(1);

        //Calculate the max range value
        maxValueGraphRange = (maxValueGraphRangeFirst.toLongLong()+1) * qPow(10, maxValueGraphRangeLength-1);

    axisY->setRange(0 , maxValueGraphRange);
    chart->addAxis(axisY, Qt::AlignLeft);
    series->attachAxis(axisY);

    ui->Stats_chartview_Graph1->setChart(chart);

}
//----------------------------------------------------------------------
