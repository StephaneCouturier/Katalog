#include "storagetreemodel.h"
#include <QStringList>

StorageTreeModel::StorageTreeModel(QObject *parent)
    : QAbstractItemModel(parent)
{
    QList<QVariant> rootData;
    rootData << tr("Storage")
             << tr("StoragePath")
             << tr("storageID")
             << tr("storageName")
             << tr("storageType")
             << tr("storageLocation")
             << tr("storagePath")
             << tr("storageLabel")
             << tr("storageFileSystem")
             << tr("storageTotalSpace")
             << tr("storageFreeSpace")
             << tr("storageBrandModel")
             << tr("storageSerialNumber")
             << tr("storageBuildDate")
             << tr("storageContentType")
             << tr("storageContainer")
             << tr("storageComment");

    rootItem = new TreeItem(rootData);

    setupModelData(rootItem);
}

StorageTreeModel::~StorageTreeModel()
{
    delete rootItem;
}

int StorageTreeModel::columnCount(const QModelIndex &parent) const
{
    if (parent.isValid())
        return static_cast<TreeItem*>(parent.internalPointer())->columnCount();
    else
        return rootItem->columnCount();
}

QVariant StorageTreeModel::data(const QModelIndex &index, int role) const
{
    if (!index.isValid())
        return QVariant();

    if (role != Qt::DisplayRole)
        return QVariant();

    TreeItem *item = static_cast<TreeItem*>(index.internalPointer());

    return item->data(index.column());
}

Qt::ItemFlags StorageTreeModel::flags(const QModelIndex &index) const
{
    if (!index.isValid())
        return 0;

    return QAbstractItemModel::flags(index);
}

QVariant StorageTreeModel::headerData(int section, Qt::Orientation orientation,
                               int role) const
{
    if (orientation == Qt::Horizontal && role == Qt::DisplayRole)
        return rootItem->data(section);

    return QVariant();
}

QModelIndex StorageTreeModel::index(int row, int column, const QModelIndex &parent)
            const
{
    if (!hasIndex(row, column, parent))
        return QModelIndex();

    TreeItem *parentItem;

    if (!parent.isValid())
        parentItem = rootItem;
    else
        parentItem = static_cast<TreeItem*>(parent.internalPointer());

    TreeItem *childItem = parentItem->child(row);
    if (childItem)
        return createIndex(row, column, childItem);
    else
        return QModelIndex();
}

QModelIndex StorageTreeModel::parent(const QModelIndex &index) const
{
    if (!index.isValid())
        return QModelIndex();

    TreeItem *childItem = static_cast<TreeItem*>(index.internalPointer());
    TreeItem *parentItem = childItem->parentItem();

    if (parentItem == rootItem)
        return QModelIndex();

    return createIndex(parentItem->row(), 0, parentItem);
}

int StorageTreeModel::rowCount(const QModelIndex &parent) const
{
    TreeItem *parentItem;
    if (parent.column() > 0)
        return 0;

    if (!parent.isValid())
        parentItem = rootItem;
    else
        parentItem = static_cast<TreeItem*>(parent.internalPointer());

    return parentItem->childCount();
}

int StorageTreeModel::findNode(unsigned int& hash, const QList<TreeItem*>& tList)
{
    for(int idx = 0; idx < tList.size(); ++idx)
    {
        unsigned int z = tList.at(idx)->getIndex();
        if(z == hash)
            return idx;
    }

    return -1;
}


//void StorageTreeModel::setSelectedCatalogPath(QString newSelectedCatalogPath){
//    selectedCatalogPath = newSelectedCatalogPath;
//}

void StorageTreeModel::setupModelData(TreeItem *parent)
{
    QList<TreeItem*> parents;
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
        QString querySQL = QLatin1String(R"(
                                SELECT storageLocation||'/'||storageName AS filePath,
                                       storageLocation||'/'||storageName AS id_file,
                                       storageID ,
                                       storageName,
                                       storageType,
                                       storageLocation,
                                       storagePath,
                                       storageLabel,
                                       storageFileSystem,
                                       storageTotalSpace,
                                       storageFreeSpace,
                                       storageBrandModel,
                                       storageSerialNumber,
                                       storageBuildDate,
                                       storageContentType,
                                       storageContainer,
                                       storageComment
                                FROM storage
                                ORDER BY storageLocation, storageName ASC
         )");

        query.prepare(querySQL);
//        query.bindValue(":selectedCatalogPath",selectedCatalogPath);
        query.exec();

        int idPath = query.record().indexOf("filePath");
        int idIdx = query.record().indexOf("id_file");

        while (query.next())
        {
           QString name = query.value(idPath).toString();
//           int id_file = query.value(idIdx).toInt();
           QString id_file = query.value(idIdx).toString();
           QString storageID = query.value(2).toString();
           QString storageName = query.value(3).toString();
           QString storageType = query.value(4).toString();
           QString storageLocation = query.value(5).toString();
           QString storagePath = query.value(6).toString();
           QString storageLabel = query.value(7).toString();
           QString storageFileSystem = query.value(8).toString();
           QString storageTotalSpace = query.value(9).toString();
           QString storageFreeSpace = query.value(10).toString();
           QString storageBrandModel = query.value(11).toString();
           QString storageSerialNumber = query.value(12).toString();
           QString storageBuildDate = query.value(13).toString();
           QString storageContentType = query.value(14).toString();
           QString storageContainer = query.value(15).toString();
           QString storageComment = query.value(16).toString();

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

                   //columnData << nChild;
                   columnData << id_file
                              << storageID
                              << storageName
                              << storageType
                              << storageLocation
                              << storagePath
                              << storageLabel
                              << storageFileSystem
                              << storageTotalSpace
                              << storageFreeSpace
                              << storageBrandModel
                              << storageSerialNumber
                              << storageBuildDate
                              << storageContentType
                              << storageContainer
                              << storageComment;

                   if(lastidx != -1)
                   {
                       parents.at(lastidx)->appendChild(new TreeItem(columnData, parents.at(lastidx), hash));
                       parents <<  parents.at(lastidx)->child( parents.at(lastidx)->childCount()-1);
                       lastidx = -1;
                   }
                   else
                   {
                       parents.last()->appendChild(new TreeItem(columnData, parents.last(), hash));
                       parents <<  parents.last()->child( parents.last()->childCount()-1);
                   }
               }
           }
        }
}
