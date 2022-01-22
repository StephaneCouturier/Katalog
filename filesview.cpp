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
* /////////////////////////////////////////////////////////////////////////////
// Application: Katalog
// File Name:   filesview.cpp
// Purpose:     class to create a model used to display files and their attributes
// Description:
// Author:      Stephane Couturier
// Version:     1.00
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

    //Define list of column per type of data
    QList<int> filesizeColumnList, filecountColumnList, percentColumnList;
//               currencyColumnList, numberColumnList, colorColumnList, signedCurrencyColumnList;
//    percentColumnList       <<6 <<8 <<9 <<12;
//    currencyColumnList      <<3 <<4 <<5 <<13 <<14;
//    signedCurrencyColumnList        <<5 <<13 <<14;
//     numberColumnList        <<2 <<3;
//    colorColumnList         <<5 <<6 <<8  <<9     <<14;
      //filecountColumnList <<3;
      filesizeColumnList <<1;

    switch ( role )
         {

            case Qt::DisplayRole:
            {
                //Filename column
//                if( index.column()==0 ){
//                    //QSortFilterProxyModel::setIcon(QIcon("icon.jpg"));
//                    return QVariant("" + QSortFilterProxyModel::data(index, role).toString());
//                }

                //file size columns
                if( filesizeColumnList.contains(index.column()) ){
                    return QVariant( QLocale().formattedDataSize(QSortFilterProxyModel::data(index, role).toLongLong()) + "  ");
                }

                //Numbers columns (without units)
                else if( filecountColumnList.contains(index.column()) ){
                    return QVariant(QLocale().toString(QSortFilterProxyModel::data(index, role).toDouble(), 'f', 0)  + "  ");
                }

                //Percent columns
                else if( percentColumnList.contains(index.column()) ){
                    if ( QSortFilterProxyModel::data(index, role).toDouble() < 0 )
                        return QVariant(QLocale().toString(QSortFilterProxyModel::data(index, role).toDouble(), 'f', 2) + " %");
                    else if( percentColumnList.contains(index.column()) && QSortFilterProxyModel::data(index, role).toDouble() >= 0)
                        return QVariant("+" + QLocale().toString(QSortFilterProxyModel::data(index, role).toDouble(), 'f', 2) + " %");

                }

                else QSortFilterProxyModel::data(index, role) ;

                //Replace a value
//                   if ( QSortFilterProxyModel::data(index, role).toFloat() == 0 ){
//                        return QVariant("");
//                    }
                // is column not in any list

                break;
            }

//            case Qt::ForegroundRole:
//            {
//                QBrush redBrush, greenBrush;
//                      redBrush.setColor(QColor(190, 20, 30));
//                    greenBrush.setColor(QColor(20, 150, 30));

//                if( colorColumnList.contains(index.column() )){
//                    if (QSortFilterProxyModel::data(index, Qt::DisplayRole).toDouble() < 0)
//                        return QVariant (redBrush);
//                    else if(QSortFilterProxyModel::data(index, Qt::DisplayRole).toDouble() >= 0)
//                        return QVariant (greenBrush);
//                }
//                break;
//            }

            case Qt::FontRole:
            {
                if (index.column() == 0 ) {
//                    QFont boldFont;
//                    boldFont.setBold(true);
//                    return boldFont;
                }
                break;
            }

            case Qt::TextAlignmentRole:
            {
                //align numbers to the right
                if ( filecountColumnList.contains(index.column()) )
                    return QVariant ( Qt::AlignVCenter | Qt::AlignRight );

                if ( filesizeColumnList.contains(index.column()) )
                    return QVariant ( Qt::AlignVCenter | Qt::AlignRight );

//               if ( percentColumnList.contains(index.column()) )
//                   return QVariant ( Qt::AlignVCenter | Qt::AlignRight );

               break;
            }

            case Qt::DecorationRole:
            {
                //Filename column
                if( index.column()==0 ){
                    QIcon icon = QIcon::fromTheme("document-preview-archive");
                    //                    QFileIconProvider iconProvider;
//                    if( QSortFilterProxyModel::data(index, role).toString()=="" ){
//                        return iconProvider.icon(QFileIconProvider::Folder);
//                    }
//                    else if( QSortFilterProxyModel::data(index, role).toString()!="" ){
//                        return iconProvider.icon(QFileIconProvider::File);
//                    }


//                    QStandardItem *item = static_cast<QStandardItem *>(index.internalPointer());
//                    item->setIcon(QIcon(":icons/document-edit.png"));
                    //QSortFilterProxyModel::setIcon(QIcon("icon.jpg"));
                    //return QVariant("" + QSortFilterProxyModel::data(index, role).toString());
                    //return iconProvider.icon(QFileIconProvider::File);
                    //QFileIconProvider ip;
                    //setIcon(ip.icon(fi));

                    //setIconQIcon(":icons/document-edit.png"));
                    //QIcon icon = QIcon(":fallback-icons/document-edit.png"); filename-title-amarok.svg

                    //icon.actualSize(QSize(32,32));
                    //QFileInfo fileInfo("e:\\ss_source\\youcam-tutorial.exe");
                    //icon = iconProvider.icon(fileInfo);

                    return icon;
                    //return QIcon::fromTheme("document-open");
                    //return iconProvider.icon(QFileIconProvider::File);

                }

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
