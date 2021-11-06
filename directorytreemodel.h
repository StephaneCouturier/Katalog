#ifndef DIRECTORYTREEMODEL_H
#define DIRECTORYTREEMODEL_H

#include "treeitem.h"
#include <QAbstractItemModel>
#include <QModelIndex>
#include <QVariant>
#include <QtSql>

class TreeItem;

class DirectoryTreeModel : public QAbstractItemModel
{
    Q_OBJECT
public:
    explicit DirectoryTreeModel(QObject* parent = nullptr);
    ~DirectoryTreeModel();

    QVariant data(const QModelIndex &index, int role) const override;
    Qt::ItemFlags flags(const QModelIndex &index) const override;
    QVariant headerData(int section, Qt::Orientation orientation,
                            int role = Qt::DisplayRole) const override;
    QModelIndex index(int row, int column,
                          const QModelIndex &parent = QModelIndex()) const override;
    QModelIndex parent(const QModelIndex &index) const override;
    int rowCount(const QModelIndex &parent = QModelIndex()) const override;
    int columnCount(const QModelIndex &parent = QModelIndex()) const override;

    //QString selectedCatalogPath;
    //void setSelectedCatalogPath(QString newSelectedCatalogPath);

private:
    void setupModelData(TreeItem *parent);

    int findNode(unsigned int& hash, const QList<TreeItem*>& tList);

    TreeItem *rootItem;

};

#endif // DIRECTORYTREEMODEL_H
