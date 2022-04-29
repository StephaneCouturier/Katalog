#include "directorytreemodel.h"
#include <QStringList>


DirectoryTreeModel::DirectoryTreeModel(QObject *parent)
    : QAbstractItemModel(parent)
{
    QList<QVariant> rootData;
    rootData << tr("Directory") << tr("Path");
    rootItem = new DirectoryTreeItem(rootData);

    setupModelData(rootItem);
}

DirectoryTreeModel::~DirectoryTreeModel()
{
    delete rootItem;
}

int DirectoryTreeModel::columnCount(const QModelIndex &parent) const
{
    if (parent.isValid())
        return static_cast<DirectoryTreeItem*>(parent.internalPointer())->columnCount();
    else
        return rootItem->columnCount();
}

QVariant DirectoryTreeModel::data(const QModelIndex &index, int role) const
{
    if (!index.isValid())
        return QVariant();

    if (role != Qt::DisplayRole)
        return QVariant();

    DirectoryTreeItem *item = static_cast<DirectoryTreeItem*>(index.internalPointer());

    return item->data(index.column());
}

Qt::ItemFlags DirectoryTreeModel::flags(const QModelIndex &index) const
{
    if (!index.isValid())
        return 0;

    return QAbstractItemModel::flags(index);
}

QVariant DirectoryTreeModel::headerData(int section, Qt::Orientation orientation,
                               int role) const
{
    if (orientation == Qt::Horizontal && role == Qt::DisplayRole)
        return rootItem->data(section);

    return QVariant();
}

QModelIndex DirectoryTreeModel::index(int row, int column, const QModelIndex &parent)
            const
{
    if (!hasIndex(row, column, parent))
        return QModelIndex();

    DirectoryTreeItem *parentItem;

    if (!parent.isValid())
        parentItem = rootItem;
    else
        parentItem = static_cast<DirectoryTreeItem*>(parent.internalPointer());

    DirectoryTreeItem *childItem = parentItem->child(row);
    if (childItem)
        return createIndex(row, column, childItem);
    else
        return QModelIndex();
}

QModelIndex DirectoryTreeModel::parent(const QModelIndex &index) const
{
    if (!index.isValid())
        return QModelIndex();

    DirectoryTreeItem *childItem = static_cast<DirectoryTreeItem*>(index.internalPointer());
//    DirectoryTreeItem *parentItem = childItem->parentItem();

//    if (parentItem == rootItem)
        return QModelIndex();

//    return createIndex(parentItem->row(), 0, parentItem);
}

int DirectoryTreeModel::rowCount(const QModelIndex &parent) const
{
    DirectoryTreeItem *parentItem;
    if (parent.column() > 0)
        return 0;

    if (!parent.isValid())
        parentItem = rootItem;
    else
        parentItem = static_cast<DirectoryTreeItem*>(parent.internalPointer());

    return parentItem->childCount();
}

int DirectoryTreeModel::findNode(unsigned int& hash, const QList<DirectoryTreeItem*>& tList)
{
    for(int idx = 0; idx < tList.size(); ++idx)
    {
        unsigned int z = tList.at(idx)->getIndex();
        if(z == hash)
            return idx;
    }

    return -1;
}


//void DirectoryTreeModel::setSelectedCatalogPath(QString newSelectedCatalogPath){
//    selectedCatalogPath = newSelectedCatalogPath;
//}

void DirectoryTreeModel::setupModelData(DirectoryTreeItem *parent)
{
    QList<DirectoryTreeItem*> parents;
    parents << parent;

// DEV: REPLACE BY CURRENT VALUE
        //setSelectedCatalogPath(selectedCatalogPath);
        //selectedCatalogPath = "/run/media/stephane";
// DEV: REPLACE BY CURRENT VALUE

        QSqlQuery query;
//        QString querySQL = QLatin1String(R"(
//                                SELECT DISTINCT (REPLACE(filePath, :selectedCatalogPath||'/', '')) AS filePath,
//                                        filePath AS id_file
//                                FROM file
//                                GROUP BY filePath
//                                ORDER BY filePath ASC
//         )");

//        QString querySQL = QLatin1String(R"(
//                                SELECT DISTINCT (filePath) AS filePath, filePath AS id_file
//                                FROM file
//                                GROUP BY filePath
//                                ORDER BY filePath ASC
//         )");

        QString querySQL = QLatin1String(R"(
                                SELECT count (DISTINCT (filePath))
                                FROM filesall
                                WHERE fileCatalog =:fileCatalog
                                GROUP BY filePath
                                ORDER BY filePath ASC
         )");



        query.prepare(querySQL);
        query.bindValue(":fileCatalog","Maxtor_2Tb");
        query.exec();

        int idPath = query.record().indexOf("filePath");
        int idIdx = query.record().indexOf("id_file");

        while (query.next())
        {
           QString name = query.value(idPath).toString();
//           int id_file = query.value(idIdx).toInt();
           QString id_file = query.value(idIdx).toString();

           QStringList nodeString = name.split("/", QString::SkipEmptyParts);

           QString temppath = "";

           int lastidx = 0;
           for(int node = 0; node < nodeString.count(); ++node)
           {
               temppath += nodeString.at(node);
               if(node != nodeString.count() - 1)
                   temppath += "\\";

               unsigned int hash = qHash(temppath);
               QList<QVariant> columnData;

               columnData << nodeString.at(node);

               int idx = findNode(hash, parents);

               if(idx != -1)
               {
                    lastidx = idx;
               }
               else
               {
                   QString sQuery =  "";
                   if(node == nodeString.count() - 1)
                   {
                       sQuery += "SELECT count(*) FROM version WHERE id_file=";
                       //sQuery += QString::number(id_file);
                       sQuery += id_file;
                       sQuery += ";";
                   }
                   else
                   {
                       sQuery += "SELECT count(*) FROM file WHERE path like '";
                       sQuery += temppath;
                       sQuery += "%';";
                   }


                   int nChild = 0;
                   QSqlQuery query2(sQuery);

                   if(query2.next())
                        nChild = query2.value(0).toInt();

                   columnData << nChild;
                   columnData << id_file;

                   if(lastidx != -1)
                   {
                       parents.at(lastidx)->appendChild(new DirectoryTreeItem(columnData, parents.at(lastidx), hash));
                       parents <<  parents.at(lastidx)->child( parents.at(lastidx)->childCount()-1);
                       lastidx = -1;
                   }
                   else
                   {
                       parents.last()->appendChild(new DirectoryTreeItem(columnData, parents.last(), hash));
                       parents <<  parents.last()->child( parents.last()->childCount()-1);
                   }
               }
           }
        }
}
