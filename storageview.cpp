#include "storageview.h"


#include <QFont>
#include <QBrush>
#include <QDebug>

StorageView::StorageView(QObject *parent)
    : QSortFilterProxyModel(parent)
{

}

QVariant StorageView::data(const QModelIndex &index, int role) const
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
      filesizeColumnList <<7 <<8;

    switch ( role )
         {

            case Qt::DisplayRole:
            {
                //Currency (Euro) columns
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
                if (index.column() == 1 ) {
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

//               if ( percentColumnList.contains(index.column()) )
//                   return QVariant ( Qt::AlignVCenter | Qt::AlignRight );

               break;
            }

            case Qt::BackgroundRole:
            {
                if (index.column()  == 2)  //change background
                    //return QBrush(Qt::red);
                break;
           }

        }

    return QSortFilterProxyModel::data(index, role);
}

QVariant StorageView::headerData(int section, Qt::Orientation orientation, int role) const
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
