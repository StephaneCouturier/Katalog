#ifndef EXPLORETREEMODEL_H
#define EXPLORETREEMODEL_H

#include "exploretreeitem.h"
#include <QAbstractItemModel>
#include <QModelIndex>
#include <QVariant>

class ExploreTreeModel : public QAbstractItemModel
{
    Q_OBJECT
public:
    explicit ExploreTreeModel(QObject* parent=0);
    ~ExploreTreeModel();

    QVariant data(const QModelIndex &index, int role) const Q_DECL_OVERRIDE;
    Qt::ItemFlags flags(const QModelIndex &index) const Q_DECL_OVERRIDE;
    QVariant headerData(int section, Qt::Orientation orientation,
                            int role = Qt::DisplayRole) const Q_DECL_OVERRIDE;
    QModelIndex index(int row, int column,
                          const QModelIndex &parent = QModelIndex()) const Q_DECL_OVERRIDE;
    QModelIndex parent(const QModelIndex &index) const Q_DECL_OVERRIDE;
    int rowCount(const QModelIndex &parent = QModelIndex()) const Q_DECL_OVERRIDE;
    int columnCount(const QModelIndex &parent = QModelIndex()) const Q_DECL_OVERRIDE;

    QString modelFileCatalog;
    QString modelCatalogPath;

    void setModelCatalog(QString newModelFileCatalog, QString newModelCatalogPath);

private:
    void setupModelData(ExploreTreeItem *parent);
    int findNode(unsigned int& hash, const QList<ExploreTreeItem*>& tList);

    ExploreTreeItem *rootItem;

};
#endif // EXPLORETREEMODEL_H
