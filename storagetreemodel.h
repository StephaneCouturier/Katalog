#ifndef STORAGETREEMODEL_H
#define STORAGETREEMODEL_H

#include "treeitem.h"
#include <QAbstractItemModel>
#include <QModelIndex>
#include <QVariant>
#include <QtSql>

class TreeItem;

class StorageTreeModel : public QAbstractItemModel
{
    Q_OBJECT
public:
    explicit StorageTreeModel(QObject* parent = nullptr);
    ~StorageTreeModel();

    QVariant data(const QModelIndex &index, int role) const override;
    Qt::ItemFlags flags(const QModelIndex &index) const override;
    QVariant headerData(int section, Qt::Orientation orientation,
                            int role = Qt::DisplayRole) const override;
    QModelIndex index(int row, int column,
                          const QModelIndex &parent = QModelIndex()) const override;
    QModelIndex parent(const QModelIndex &index) const override;
    int rowCount(const QModelIndex &parent = QModelIndex()) const override;
    int columnCount(const QModelIndex &parent = QModelIndex()) const override;

private:
    void setupModelData(TreeItem *parent);

    int findNode(unsigned int& hash, const QList<TreeItem*>& tList);

    TreeItem *rootItem;

};

#endif // STORAGETREEMODEL_H
