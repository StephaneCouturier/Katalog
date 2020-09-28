#include "mainwindow.h"
#include "ui_mainwindow.h"
#include "storage.h"

#include <QSortFilterProxyModel>

Storage::Storage(QObject *parent) : QAbstractTableModel(parent)
{

}

//Storage::Storage(QObject *parent): QStandardItemModel(parent)   , treeView(new QTreeView(this))
//, standardModel(new QStandardItemModel(this))
//{
//    QList<QStandardItem *> preparedRow = prepareRow("2' External Drives", "", "2000000");
//    QStandardItem *item = standardModel->invisibleRootItem();
//    // adding a row to the invisible root item produces a root element
//    item->appendRow(preparedRow);

//    QList<QStandardItem *> secondRow = prepareRow("Maxtor_1Tb1", "32", "1000000");
//    // adding a row to an item starts a subtree
//    preparedRow.first()->appendRow(secondRow);

//    QList<QStandardItem *> thirdRow = prepareRow("Maxtor_1Tb2", "33", "1000000");
//    // adding a row to an item starts a subtree
//    preparedRow.first()->appendRow(thirdRow);

//    //ui->TrV_Storage->setModel(standardModel);
//    //treeView->expandAll();
//}


int Storage::rowCount(const QModelIndex &parent) const
{
    Q_UNUSED(parent);
    return storageName.length();
}

int Storage::columnCount(const QModelIndex &parent) const
{
    Q_UNUSED(parent);
    return 10;
}

QVariant Storage::data(const QModelIndex &index, int role) const
{
    if (!index.isValid() || role != Qt::DisplayRole) {
        return QVariant();
    }
        switch (index.column()){
        case 0: return QString(storageLocation[index.row()]);
        case 1: return QIcon(storageIcon[index.row()]);
        case 2: return QString(storageName[index.row()]);
        case 3: return QString(storageID[index.row()]);
        case 4: return QString(storageType[index.row()]);
        case 5: return QString(storagePath[index.row()]);
        case 6: return QString(storageLabel[index.row()]);
        case 7: return QString(storageFileSystemType[index.row()]);
        case 8: return qint64(storageBytesTotal[index.row()]);
        case 9: return qint64(storageBytesFree[index.row()]);
    }
    return QVariant();
}

QVariant Storage::headerData(int section, Qt::Orientation orientation, int role) const
{
    if (role == Qt::DisplayRole && orientation == Qt::Horizontal) {
        switch (section){
        case 0: return QString("Location");
        case 1: return QString("Icon");
        case 2: return QString("Name");
        case 3: return QString("ID");
        case 4: return QString("Type");
        case 5: return QString("Path");
        case 6: return QString("Label");
        case 7: return QString("FileSystem");
        case 8: return QString("Total");
        case 9: return QString("Free");
        }
    }
    return QVariant();
}

// Create a method to populate the model with data:
void Storage::populateStorageData(  const QList<QString> &sNames,
                                    const QList<QString> &sIDs,
                                    const QList<QString> &sTypes,
                                    const QList<QString> &sLocations,
                                    const QList<QString> &sPaths,
                                    const QList<QString> &sLabels,
                                    const QList<QString> &sFileSystemTypes,
                                    const QList<qint64>  &sBytesTotals,
                                    const QList<qint64>  &sBytesFrees,
                                    const QList<QIcon>   &sIcons
                                    )
{
    storageName.clear();
    storageName = sNames;
    storageIcon.clear();
    storageIcon = sIcons;
    storageID.clear();
    storageID = sIDs;
    storageType.clear();
    storageType = sTypes;
    storageLocation.clear();
    storageLocation = sLocations;
    storagePath.clear();
    storagePath = sPaths;
    storageLabel.clear();
    storageLabel = sLabels;
    storageFileSystemType.clear();
    storageFileSystemType = sFileSystemTypes;
    storageBytesTotal.clear();
    storageBytesTotal = sBytesTotals;
    storageBytesFree.clear();
    storageBytesFree = sBytesFrees;

    return;
}

//void Storage::loadStorageModel()
//{
//    //Set up temporary lists
//    QList<QString>  sNames;
//    QList<QString>  sIDs;
//    QList<QString>   sTypes;

//    //Get data
//    QStringList fileTypes;
//    fileTypes << "*.idx";


//    // Create model
//    Storage *storage = new Storage(this);

//    // Populate model with data
//    storage->populateData(sNames,
//                          sIDs,
//                          sTypes
//                          );

//    QSortFilterProxyModel *proxyModel = new QSortFilterProxyModel(this);
//    proxyModel->setSourceModel(storage);

    // Connect model to tree/table view
//    ui->TrV_Storage->setModel(proxyModel);
//    ui->TrV_Storage->QTreeView::sortByColumn(0,Qt::AscendingOrder);
//    ui->TrV_Storage->header()->setSectionResizeMode(QHeaderView::Interactive);
//    ui->TrV_Storage->header()->resizeSection(0, 350); //Name
//    ui->TrV_Storage->header()->resizeSection(1, 350); //ID
//    ui->TrV_Storage->header()->resizeSection(2, 150); //Type
    //ui->TrV_CatalogList->header()->hideSection(0); //Path
//}
