#ifndef DIRECTORYTREEMODEL_H
#define DIRECTORYTREEMODEL_H

#include "treeitem.h"
#include <QAbstractItemModel>
#include <QModelIndex>
#include <QVariant>
#include <QtSql>


class DirectoryTreeModel : public QAbstractItemModel
{
    Q_OBJECT
public:
    explicit DirectoryTreeModel(QObject* parent=0);
    ~DirectoryTreeModel();

    QVariant data(const QModelIndex &index, int role) const Q_DECL_OVERRIDE;
    Qt::ItemFlags flags(const QModelIndex &index) const Q_DECL_OVERRIDE;
    QVariant headerData(int section, Qt::Orientation orientation,
                            int role = Qt::DisplayRole) const Q_DECL_OVERRIDE;
    QModelIndex index(int row, int column,
                          const QModelIndex &parent = QModelIndex()) const Q_DECL_OVERRIDE;
    QModelIndex parent(const QModelIndex &index) const Q_DECL_OVERRIDE;
    int rowCount(const QModelIndex &parent = QModelIndex()) const Q_DECL_OVERRIDE;
    int columnCount(const QModelIndex &parent = QModelIndex()) const Q_DECL_OVERRIDE;
    QString selectedCatalogPath;

private:
    void setupModelData(TreeItem *parent);
    void setSelectedCatalogPath(QString newSelectedCatalogPath);

    int findNode(unsigned int& hash, const QList<TreeItem*>& tList);

    TreeItem *rootItem;

};

#endif // DIRECTORYTREEMODEL_H
