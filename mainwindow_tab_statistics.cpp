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
// File Name:   mainwindow_tab_stats.cpp
// Purpose:
// Description:
// Author:      Stephane Couturier
// Modified by: Stephane Couturier
// Created:     2020-10-08
// Version:     0.1
/////////////////////////////////////////////////////////////////////////////
*/

#include "mainwindow.h"
#include "ui_mainwindow.h"

#include <KMessageBox>
#include <KLocalizedString>
#include <QDesktopServices>

#include <QDateTimeAxis>
#include <QtCharts/QBarSeries>
#include <QtCharts/QBarSet>
#include <QtCharts/QLineSeries>
#include <QtCharts/QBarCategoryAxis>
#include <QtCharts/QValueAxis>
//#include <QtCharts/QChartView>
//#include <QtCharts/QLegend>

//#include <KMessageBox>
//#include <KLocalizedString>

void MainWindow::on_Stats_CB_SelectCatalog_currentIndexChanged()
{
    statsLoadChart();
}
void MainWindow::on_Stats_comboBox_TypeOfData_currentIndexChanged()
{
    statsLoadChart();
}
//----------------------------------------------------------------------
void MainWindow::on_Stats_PB_OpenStatsFile_clicked()
{
    //KMessageBox::information(this,"test:\n");
    QString statisticsFilePath = collectionFolder + "/" + "statistics.csv";
    QDesktopServices::openUrl(QUrl::fromLocalFile(statisticsFilePath));
}
//----------------------------------------------------------------------
void MainWindow::on_Stats_PB_Reload_clicked()
{
    statsLoadChart();
}
//----------------------------------------------------------------------

void MainWindow::statsLoadChart()
{
    selectedTypeOfData = ui->Stats_comboBox_TypeOfData->currentText();

    QString statisticsFilePath = collectionFolder + "/" + "statistics.csv";
    QString selectedCatalogforStats = ui->Stats_CB_SelectCatalog->currentText();
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
        //if (line.startsWith("#") || line.startsWith(":"))
        //    continue;
        QStringList values = line.split("\t");
        qint64 number = 0;

        if ( selectedCatalogforStats  == values[1] ){
            QDateTime datetime = QDateTime::fromString(values[0],"yyyy-MM-dd hh:mm:ss");
            if ( selectedTypeOfData == "Number of files" )
            {
                number = values[2].toLongLong();
                //number = number/1000;
                //displayUnit = "(k)";
                series->append(datetime.toMSecsSinceEpoch(), number);
                if ( number > maxValueGraphRange )
                    maxValueGraphRange = number;
            }
            else if ( selectedTypeOfData == "Total file size" )
            {
                number = values[3].toLongLong();
                //KMessageBox::information(this,"test:\n" + values[0] + "test:\n" +QString::number(number));
                number = number/1024/1024;
                displayUnit = "(MiB)";
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
    chart->setTitle("<p style=\"font-weight: bold; font-size: 22px; font-color: #AAA,\">"
                    +selectedTypeOfData+" "+displayUnit
                    +" of <span style=\"font-style: italic; color: #000,\">"+selectedCatalogforStats+"</span></p>");

    //Format axis
    QDateTimeAxis *axisX = new QDateTimeAxis;
    //axisX->setTickCount(10);
    axisX->setFormat("yyyy-MM-dd");
    axisX->setTitleText("Date");
    chart->addAxis(axisX, Qt::AlignBottom);
    series->attachAxis(axisX);

    QValueAxis *axisY = new QValueAxis;
    axisY->setLabelFormat("%i");
    axisY->setTitleText("Total");

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

void MainWindow::statsLoadChart2()
{
    //![1]
        QBarSet *set0 = new QBarSet("Jane");
        QBarSet *set1 = new QBarSet("John");
        QBarSet *set2 = new QBarSet("Axel");
        QBarSet *set3 = new QBarSet("Mary");
        QBarSet *set4 = new QBarSet("Sam");

        *set0 << 1 << 2 << 3 << 4 << 5 << 6;
        *set1 << 5 << 0 << 0 << 4 << 0 << 7;
        *set2 << 3 << 5 << 8 << 13 << 8 << 5;
        *set3 << 5 << 6 << 7 << 3 << 4 << 5;
        *set4 << 9 << 7 << 5 << 3 << 1 << 2;
    //![1]

    //![2]
        QBarSeries *barseries = new QBarSeries();
        barseries->append(set0);
        barseries->append(set1);
        barseries->append(set2);
        barseries->append(set3);
        barseries->append(set4);
    //![2]

    //![8]
        QLineSeries *lineseries = new QLineSeries();
        lineseries->setName("trend");
        lineseries->append(QPoint(0, 4));
        lineseries->append(QPoint(1, 15));
        lineseries->append(QPoint(2, 20));
        lineseries->append(QPoint(3, 4));
        lineseries->append(QPoint(4, 12));
        lineseries->append(QPoint(5, 17));
    //![8]

    //![3]
        QChart *chart = new QChart();
        chart->addSeries(barseries);
        chart->addSeries(lineseries);
        chart->setTitle("Line and barchart example");
    //![3]

    //![4]
        QStringList categories;
        categories << "Jan" << "Feb" << "Mar" << "Apr" << "May" << "Jun";
        QBarCategoryAxis *axisX = new QBarCategoryAxis();
        axisX->append(categories);
        chart->addAxis(axisX, Qt::AlignBottom);
        lineseries->attachAxis(axisX);
        barseries->attachAxis(axisX);
        axisX->setRange(QString("Jan"), QString("Jun"));

        QValueAxis *axisY = new QValueAxis();
        chart->addAxis(axisY, Qt::AlignLeft);
        lineseries->attachAxis(axisY);
        barseries->attachAxis(axisY);
        axisY->setRange(0, 20);
    //![4]

    //![5]
        chart->legend()->setVisible(true);
        chart->legend()->setAlignment(Qt::AlignBottom);
    //![5]

    //![6]
        QChartView *chartView = new QChartView(chart);
        chartView->setRenderHint(QPainter::Antialiasing);

    //QChartView *chartView = new QChartView(chart);
    //ui->Stats_chartview_graph1->setRenderHint(QPainter::Antialiasing);
    //ui->Stats_chartview_Graph2->setChart(chart);
}
