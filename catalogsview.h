#ifndef CATALOGSVIEW_H
#define CATALOGSVIEW_H

#include <QSortFilterProxyModel>

class CatalogsView  : public QSortFilterProxyModel
{
    Q_OBJECT

public:
    CatalogsView(QObject *parent = nullptr);

private:
    QString percentBrush;
    QVariant data( const QModelIndex &index, int role ) const;
    QVariant headerData(int section, Qt::Orientation orientation, int role) const;

};

#endif // CATALOGSVIEW_H
