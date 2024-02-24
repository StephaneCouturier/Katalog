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
// Purpose:     class to create a proxy model to display catalogs and their attributes
// Description:
// Author:      Stephane Couturier
/////////////////////////////////////////////////////////////////////////////
*/
#include "catalogsview.h"

#include <QFont>
#include <QBrush>
#include <QDebug>
#include <QFileIconProvider>
#include <QCoreApplication>

CatalogsView::CatalogsView(QObject *parent)
    : QSortFilterProxyModel(parent)
{

}

QVariant CatalogsView::data(const QModelIndex &index, int role) const
{

    //Define list of column per type of data
        QList<int> filesizeColumnList, filecountColumnList, booleanColumnList;
        filecountColumnList <<3;
        filesizeColumnList  <<4;
        booleanColumnList   <<7 <<8 <<9;
    //temporary string
        QString tempText;

    switch ( role )
         {
            case Qt::DisplayRole:
            {
                //File Size columns
                if( filesizeColumnList.contains(index.column()) ){
                    return QVariant( QLocale().formattedDataSize(QSortFilterProxyModel::data(index, role).toDouble(),2,QLocale::DataSizeIecFormat) );
                }

                //Numbers columns (no unit)
                else if( filecountColumnList.contains(index.column()) ){
                    return QVariant(QLocale().toString(QSortFilterProxyModel::data(index, role).toDouble(), 'f', 0)  + "  ");
                }

                //BooleanColumnList columns (display tick icon or nothing)
                else if( booleanColumnList.contains(index.column()) ){
                    return QVariant("");
                }

                //Text columns (translation)
                if( index.column() == 6 ){
                    // Get the original text
                    tempText = QSortFilterProxyModel::data(index, role).toString();

                    // Translate the text using the MainWindow context
                    return QCoreApplication::translate("MainWindow", tempText.toUtf8());
                }

                else QSortFilterProxyModel::data(index, role) ;

                break;
            }

            case Qt::FontRole:
            {
                if (index.column() == 0 ) {
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

               if( index.column() == 6 )
                   return QVariant ( Qt::AlignVCenter | Qt::AlignCenter );

               break;
            }

            case Qt::DecorationRole:
            {
                if( index.column()==0 ){
                    QModelIndex idx = index.sibling(index.row(), 7);
                    if( QSortFilterProxyModel::data(idx, Qt::DisplayRole).toBool()==true ){
                        return QIcon(QIcon::fromTheme("media-optical-blu-ray"));
                    }
                    else
                        return QIcon(QIcon::fromTheme("media-optical"));
                }
                else if ( booleanColumnList.contains(index.column()) ){
                    if( QSortFilterProxyModel::data(index, Qt::DisplayRole).toBool() == true ){
                        return QIcon(QIcon::fromTheme("dialog-ok-apply"));
                    }
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

QVariant CatalogsView::headerData(int section, Qt::Orientation orientation, int role) const
{
    QList<int> grayColumnList;
    grayColumnList    <<6 <<7 <<8 <<9 <<10 <<11;

    switch ( role )
         {
            case Qt::DisplayRole:
            {
                return QSortFilterProxyModel::headerData(section, orientation, role) ;
            }
            case Qt::BackgroundRole:
            {
                if (grayColumnList.contains(section))
                    return QBrush(QColor(245, 245, 245));
                break;
            }
        }
        return QVariant();
}
