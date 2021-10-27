#ifndef RESTORETREEMODEL_H
#define RESTORETREEMODEL_H

#include "treeitem.h"
#include <QAbstractItemModel>
#include <QModelIndex>
#include <QVariant>
#include <QtSql>


class RestoreTreeModel : public QAbstractItemModel
{
    Q_OBJECT
public:
    explicit RestoreTreeModel(QObject* parent=0);
    ~RestoreTreeModel();

    QVariant data(const QModelIndex &index, int role) const Q_DECL_OVERRIDE;
    Qt::ItemFlags flags(const QModelIndex &index) const Q_DECL_OVERRIDE;
    QVariant headerData(int section, Qt::Orientation orientation,
                            int role = Qt::DisplayRole) const Q_DECL_OVERRIDE;
    QModelIndex index(int row, int column,
                          const QModelIndex &parent = QModelIndex()) const Q_DECL_OVERRIDE;
    QModelIndex parent(const QModelIndex &index) const Q_DECL_OVERRIDE;
    int rowCount(const QModelIndex &parent = QModelIndex()) const Q_DECL_OVERRIDE;
    int columnCount(const QModelIndex &parent = QModelIndex()) const Q_DECL_OVERRIDE;

private:
    void setupModelData(TreeItem *parent);
    int findNode(unsigned int& hash, const QList<TreeItem*>& tList);

    TreeItem *rootItem;

};

#endif // RESTORETREEMODEL_H
