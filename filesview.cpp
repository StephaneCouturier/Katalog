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
// File Name:   filesview.cpp
// Purpose:
// Description:
// Author:      Stephane Couturier
/////////////////////////////////////////////////////////////////////////////
*/
#include "filesview.h"

#include <QFont>
#include <QBrush>
#include <QDebug>
#include <QFileIconProvider>
#include <QStandardItem>

FilesView::FilesView(QObject *parent)
    : QSortFilterProxyModel(parent)
{

}

QVariant FilesView::data(const QModelIndex &index, int role) const
{
    //Define list of columns per type of data
    QList<int> filesizeColumnList, filecountColumnList;
      filesizeColumnList <<1;

    //Definition of filetypes
    QStringList fileTypesPlain_Image, fileTypesPlain_Audio,fileTypesPlain_Video,fileTypesPlain_Text;
    fileTypesPlain_Image << "png" << "jpg" << "gif" << "xcf" << "tif" << "bmp";
    fileTypesPlain_Audio << "mp3" << "wav" << "ogg" << "aif";
    fileTypesPlain_Video << "wmv" << "avi" << "mp4" << "mkv" << "flv"  << "webm";
    fileTypesPlain_Text  << "txt" << "pdf" << "odt" << "idx" << "html" << "rtf" << "doc" << "docx" << "epub";

    switch ( role )
         {
            case Qt::DisplayRole:
            {
                //File Size columns
                if( filesizeColumnList.contains(index.column()) ){
                    QModelIndex idx = index.sibling(index.row(), 5);
                    if( QSortFilterProxyModel::data(idx, Qt::DisplayRole).toString()=="folder" )
                        return "";
                    else
                        return QVariant( QLocale().formattedDataSize(QSortFilterProxyModel::data(index, role).toLongLong()) + "  ");
                }

                //Numbers columns (without units)
                else if( filecountColumnList.contains(index.column()) ){
                    return QVariant(QLocale().toString(QSortFilterProxyModel::data(index, role).toDouble(), 'f', 0)  + "  ");
                }
                else QSortFilterProxyModel::data(index, role) ;

                break;
            }

            case Qt::TextAlignmentRole:
            {
                //align numbers to the right
                if ( filecountColumnList.contains(index.column()) )
                    return QVariant ( Qt::AlignVCenter | Qt::AlignRight );

                if ( filesizeColumnList.contains(index.column()) )
                    return QVariant ( Qt::AlignVCenter | Qt::AlignRight );

               break;
            }

            case Qt::DecorationRole:
            {
                if( index.column()==0 ){

                    //Identification of filetype
                    QString fullFileName, fileName, fileType;
                    fullFileName = QSortFilterProxyModel::data(index, Qt::DisplayRole).toString();
                    fileName = fullFileName.left(fullFileName.lastIndexOf("."));
                    fileType = fullFileName.remove(fileName+".");

                    //Assign the icon per filetype
                    QModelIndex idx = index.sibling(index.row(), 5);
                    if( QSortFilterProxyModel::data(idx, Qt::DisplayRole).toString()=="folder" ){
                        return QIcon::fromTheme("folder");
                    }
                    else if( fileTypesPlain_Image.contains(fileType,Qt::CaseInsensitive)){
                        return QIcon::fromTheme("image-jpeg");
                    }
                    else if( fileTypesPlain_Audio.contains(fileType,Qt::CaseInsensitive)){
                        return QIcon::fromTheme("audio-x-mpeg");
                    }
                    else if( fileTypesPlain_Video.contains(fileType,Qt::CaseInsensitive)){
                        return QIcon::fromTheme("video-mp4");
                    }
                    else if(  fileTypesPlain_Text.contains(fileType,Qt::CaseInsensitive)){
                        return QIcon::fromTheme("view-list-text");
                    }
                    else
                        return QIcon::fromTheme("application-x-zerosize");
                }

                break;
            }

            case Qt::BackgroundRole:
            {
                if (1 == index.row() % 2)
                    return QColor(247, 247, 247);
                else
                    return QColor(255, 255, 255);

                break;
            }

        }
    return QSortFilterProxyModel::data(index, role);
}

QVariant FilesView::headerData(int section, Qt::Orientation orientation, int role) const
{
    QList<int> grayColumnList;
    grayColumnList    <<7 <<8 <<9 <<10 <<11;

    switch ( role )
         {
            case Qt::DisplayRole:
            {
                return QSortFilterProxyModel::headerData( section, orientation, role) ;
            }
            case Qt::BackgroundRole:
            {
                if (grayColumnList.contains(section))  //change background
                    //return QBrush(QColor(245, 245, 245));
                break;
            }
        }
        return QVariant();
}
