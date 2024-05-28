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
// File Name:   storage.h
// Purpose:     Class/model for the storage device
// Description:
// Author:      Stephane Couturier
/////////////////////////////////////////////////////////////////////////////
*/

#ifndef STORAGE_H
#define STORAGE_H

#include <QStorageInfo>
#include <QSqlQuery>
#include <QRegularExpression>
#include <QSqlQuery>
#include <QVariant>
#include <QMessageBox>
#include <QCoreApplication>

class Storage
{

public:

    int ID;
    QString name;
    QString type;
    QString path;
    QString label;
    QString fileSystem;
    qint64  totalSpace;
    qint64  freeSpace;
    QString brand;
    QString model;
    QString serialNumber;
    QString buildDate;
    QString comment1;
    QString comment2;
    QString comment3;
    QDateTime dateTimeUpdated;

    void generateID();
    void insertStorage();
    void deleteStorage();
    void loadStorage();
    QList<qint64> updateStorageInfo(bool reportStorageUpdate);
    void saveStatistics(QDateTime dateTime);
    void saveStatisticsToFile(QString filePath, QDateTime dateTime);

};

#endif // STORAGE_H
