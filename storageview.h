#ifndef STORAGEVIEW_H
#define STORAGEVIEW_H

#include <QSortFilterProxyModel>

class StorageView  : public QSortFilterProxyModel
{
    Q_OBJECT

public:
    StorageView(QObject *parent = nullptr);

private:
    QString percentBrush;
    QVariant data( const QModelIndex &index, int role ) const;
    QVariant headerData(int section, Qt::Orientation orientation, int role) const;

};


#endif // STORAGEVIEW_H
