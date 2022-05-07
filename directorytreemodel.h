#ifndef DIRECTORYTREEMODEL_H
#define DIRECTORYTREEMODEL_H

#include "directorytreeitem.h"
#include <QAbstractItemModel>
#include <QModelIndex>
#include <QVariant>
#include <QtSql>

class DirectoryTreeItem;

class DirectoryTreeModel : public QAbstractItemModel
{
    Q_OBJECT

public:
    DirectoryTreeModel(const QStringList &headers, QObject *parent = nullptr);
    ~DirectoryTreeModel();

    QVariant data(const QModelIndex &index, int role) const override;
    QVariant headerData(int section, Qt::Orientation orientation,
                        int role = Qt::DisplayRole) const override;

    QModelIndex index(int row, int column,
                      const QModelIndex &parent = QModelIndex()) const override;
    QModelIndex parent(const QModelIndex &index) const override;

    int rowCount(const QModelIndex &parent = QModelIndex()) const override;
    int columnCount(const QModelIndex &parent = QModelIndex()) const override;

    Qt::ItemFlags flags(const QModelIndex &index) const override;
    bool setData(const QModelIndex &index, const QVariant &value,
                 int role = Qt::EditRole) override;
    bool setHeaderData(int section, Qt::Orientation orientation,
                       const QVariant &value, int role = Qt::EditRole) override;

    bool insertColumns(int position, int columns,
                       const QModelIndex &parent = QModelIndex()) override;
    bool removeColumns(int position, int columns,
                       const QModelIndex &parent = QModelIndex()) override;
    bool insertRows(int position, int rows,
                    const QModelIndex &parent = QModelIndex()) override;
    bool removeRows(int position, int rows,
                    const QModelIndex &parent = QModelIndex()) override;

    QString modelCatalogName;// = "Maxtor_2Tb";
    QString modelCatalogPath;// = "/run/media/stephane/Maxtor_2Tb";
    void setModelCatlog(QString newModelCatalogName, QString newModelCatalogPath);
    void setupModelData(DirectoryTreeItem *parent);

private:
    DirectoryTreeItem *getItem(const QModelIndex &index) const;

    DirectoryTreeItem *rootItem;


};

#endif // DIRECTORYTREEMODEL_H
