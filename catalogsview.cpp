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
// File Name:   catalogview.cpp
// Purpose:     class to create a model used to display catalogs and their attributes
// Description:
// Author:      Stephane Couturier
// Version:     1.00
/////////////////////////////////////////////////////////////////////////////
*/
#include "catalogsview.h"

#include <QFont>
#include <QBrush>
#include <QDebug>
#include <QFileIconProvider>

CatalogsView::CatalogsView(QObject *parent)
    : QSortFilterProxyModel(parent)
{

}

QVariant CatalogsView::data(const QModelIndex &index, int role) const
{

    //Define list of column per type of data
    QList<int> filesizeColumnList, filecountColumnList, percentColumnList;
      filecountColumnList <<3;
      filesizeColumnList <<4;
      //percentColumnList <<5;

    switch ( role )
         {
            case Qt::DisplayRole:
            {
                //File Size columns
                if( filesizeColumnList.contains(index.column()) ){
                    return QVariant( QLocale().formattedDataSize(QSortFilterProxyModel::data(index, role).toDouble()) + "  ");
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

                break;
            }

            case Qt::FontRole:
            {
                if (index.column() == 0 ) { //change font only for columns 0
                    QFont boldFont;
                    boldFont.setBold(true);
                    return boldFont;
                }
                break;
            }

            case Qt::TextAlignmentRole:
            {
               if ( filecountColumnList.contains(index.column()) )
                   return QVariant ( Qt::AlignVCenter | Qt::AlignRight );

               if ( filesizeColumnList.contains(index.column()) )
                   return QVariant ( Qt::AlignVCenter | Qt::AlignRight );

               if ( percentColumnList.contains(index.column()) )
                   return QVariant ( Qt::AlignVCenter | Qt::AlignRight );

               break;
            }

            case Qt::BackgroundRole:
            {
                if (index.column()  == 2)  //change background
                    //return QBrush(Qt::red);
                break;
            }

            case Qt::DecorationRole:
            {
                //Filename column
                if( index.column()==0 ){
                    QModelIndex idx = index.sibling(index.row(), 7);
                    if( QSortFilterProxyModel::data(idx, Qt::DisplayRole).toBool()==true ){
                        return QIcon(":/images/drive_green.png");
                    }
                    else
                        return QIcon(":/images/drive_gray.png");
                }
                break;
            }

            case Qt::ForegroundRole:
            {
                if( index.column()==0 ){
                    QBrush redBrush, greenBrush, blueBrush, grayBrush;
                          redBrush.setColor(QColor(190, 20, 30));
                        greenBrush.setColor(QColor( 20,150, 30));
                         blueBrush.setColor(QColor(  9, 86,118));
                         grayBrush.setColor(QColor( 60, 60, 60));

                    QModelIndex idx = index.sibling(index.row(), 7);
                    if( QSortFilterProxyModel::data(idx, Qt::DisplayRole).toBool()==true ){
                        return QVariant (greenBrush);
                    }
                    else
                        return QVariant (grayBrush);
                }
                break;
            }

        }

    return QSortFilterProxyModel::data(index, role);
}

QVariant CatalogsView::headerData(int section, Qt::Orientation orientation, int role) const
{
    QList<int> grayColumnList;
    grayColumnList    <<7 <<8 <<9 <<10 <<11;

    switch ( role )
         {
            case Qt::DisplayRole:
            {
                return QSortFilterProxyModel::headerData(section, orientation, role) ;
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
