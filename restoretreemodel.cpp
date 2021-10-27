#include "restoretreemodel.h"
#include <QStringList>

RestoreTreeModel::RestoreTreeModel(QObject *parent)
    : QAbstractItemModel(parent)
{
    QList<QVariant> rootData;
    rootData << "Nom" << "Nombre d'éléments";
    rootItem = new TreeItem(rootData);
    setupModelData(rootItem);
}

RestoreTreeModel::~RestoreTreeModel()
{
    delete rootItem;
}

int RestoreTreeModel::columnCount(const QModelIndex &parent) const
{
    if (parent.isValid())
        return static_cast<TreeItem*>(parent.internalPointer())->columnCount();
    else
        return rootItem->columnCount();
}

QVariant RestoreTreeModel::data(const QModelIndex &index, int role) const
{
    if (!index.isValid())
        return QVariant();

    if (role != Qt::DisplayRole)
        return QVariant();

    TreeItem *item = static_cast<TreeItem*>(index.internalPointer());

    return item->data(index.column());
}

Qt::ItemFlags RestoreTreeModel::flags(const QModelIndex &index) const
{
    if (!index.isValid())
        return 0;

    return QAbstractItemModel::flags(index);
}

QVariant RestoreTreeModel::headerData(int section, Qt::Orientation orientation,
                               int role) const
{
    if (orientation == Qt::Horizontal && role == Qt::DisplayRole)
        return rootItem->data(section);

    return QVariant();
}

QModelIndex RestoreTreeModel::index(int row, int column, const QModelIndex &parent)
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

QModelIndex RestoreTreeModel::parent(const QModelIndex &index) const
{
    if (!index.isValid())
        return QModelIndex();

    TreeItem *childItem = static_cast<TreeItem*>(index.internalPointer());
    TreeItem *parentItem = childItem->parentItem();

    if (parentItem == rootItem)
        return QModelIndex();

    return createIndex(parentItem->row(), 0, parentItem);
}

int RestoreTreeModel::rowCount(const QModelIndex &parent) const
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

int RestoreTreeModel::findNode(unsigned int& hash, const QList<TreeItem*>& tList)
{
    for(int idx = 0; idx < tList.size(); ++idx)
    {
        unsigned int z = tList.at(idx)->getIndex();
        if(z == hash)
            return idx;
    }

    return -1;
}



void RestoreTreeModel::setupModelData(TreeItem *parent)
{
    QList<TreeItem*> parents;
    parents << parent;


//    create  table  if not exists  file
//    (
//      fileID  int AUTO_INCREMENT primary key ,
//      fileName  text  ,
//      filePath  text  ,
//      fileCatalogName text
//    )


    QString path = "my_db_path";
    QSqlDatabase db = QSqlDatabase::addDatabase("QSQLITE");
    db.setDatabaseName(path);
    if(db.open())
    {

        QSqlQuery queryCreate;
        QString queryCreateSQL = QLatin1String(R"(
                                create  table  if not exists  file
                                (
                                  path  text ,
                                  id_file  text
                                )
        )");
        queryCreate.prepare(queryCreateSQL);
        queryCreate.exec();

        QSqlQuery queryDelete("DELETE FROM file");


        QSqlQuery queryInsert;
        QString querySQL = QLatin1String(R"(
                INSERT INTO file(path, id_file) VALUES('/home/stephane/file1.txt','1')
                                        )");
        queryInsert.prepare(querySQL);
        queryInsert.exec();
        querySQL = QLatin1String(R"(
                        INSERT INTO file(path, id_file) VALUES('/home/mods/file1.txt','1')
                                                )");
        queryInsert.prepare(querySQL);
        queryInsert.exec();
        querySQL = QLatin1String(R"(
                        INSERT INTO file(path, id_file) VALUES('/home/mods/file2.txt','1')
                                                )");
        queryInsert.prepare(querySQL);
        queryInsert.exec();
        querySQL = QLatin1String(R"(
                        INSERT INTO file(path, id_file) VALUES('/home/file2.txt','1')
                                                )");
        queryInsert.prepare(querySQL);
        queryInsert.exec();

        QSqlQuery query("SELECT path, id_file FROM file");
        int idPath = query.record().indexOf("path");
        int idIdx = query.record().indexOf("id_file");

        while (query.next())
        {
           QString name = query.value(idPath).toString();
           int id_file = query.value(idIdx).toInt();

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
                       sQuery += QString::number(id_file);
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
}
