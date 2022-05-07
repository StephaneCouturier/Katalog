#ifndef EXPLORETREEVIEW_H
#define EXPLORETREEVIEW_H

#include <QSortFilterProxyModel>

class ExploreTreeView  : public QSortFilterProxyModel
{
    Q_OBJECT

public:
    ExploreTreeView(QObject *parent = nullptr);

private:
    QString percentBrush;
    QVariant data( const QModelIndex &index, int role ) const;
    QVariant headerData(int section, Qt::Orientation orientation, int role) const;

};

#endif // EXPLORETREEVIEW_H
