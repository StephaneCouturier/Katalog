#include "exploretreeview.h"

#include <QFont>
#include <QBrush>
#include <QDebug>
#include <QFileIconProvider>

ExploreTreeView::ExploreTreeView(QObject *parent)
    : QSortFilterProxyModel(parent)
{

}

QVariant ExploreTreeView::data(const QModelIndex &index, int role) const
{
    switch ( role )
         {
            case Qt::DecorationRole:
            {
                //Folder column
                if( index.column()==0 ){
                    return QIcon(QIcon::fromTheme("folder"));
                }

                break;
            }
        }

    return QSortFilterProxyModel::data(index, role);
}

QVariant ExploreTreeView::headerData(int section, Qt::Orientation orientation, int role) const
{
//    QList<int> grayColumnList;
//    grayColumnList    <<0 <<1;

    switch ( role )
         {
            case Qt::DisplayRole:
            {
                return QSortFilterProxyModel::headerData( section, orientation, role) ;
            }
            case Qt::BackgroundRole:
            {
//                if (grayColumnList.contains(section))  //change background
//                    return QBrush(QColor(245, 245, 245));
//                break;
            }
        }
        return QVariant();

}
