#ifndef FILESVIEW_H
#define FILESVIEW_H

#include <QSortFilterProxyModel>

class FilesView  : public QSortFilterProxyModel
{
    Q_OBJECT

public:
    FilesView(QObject *parent = nullptr);

private:
    QString percentBrush;
    QVariant data( const QModelIndex &index, int role ) const;
    QVariant headerData(int section, Qt::Orientation orientation, int role) const;

};

#endif // FILESVIEW_H
