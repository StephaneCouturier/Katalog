#ifndef STORAGE_H
#define STORAGE_H


//#include <QMainWindow>
#include <QStandardItemModel>
#include <QDesktopServices>
//#include <QTreeView>
//QT_BEGIN_NAMESPACE
//class QTreeView; //forward declarations
//class QStandardItemModel;
//class QStandardItem;
//QT_END_NAMESPACE


class Storage: public QAbstractTableModel
{
    Q_OBJECT

public:
    Storage(QObject *parent = nullptr);

    int rowCount(const QModelIndex &parent = QModelIndex()) const override;
    int columnCount(const QModelIndex &parent = QModelIndex()) const override;
    QVariant data(const QModelIndex &index, int role = Qt::DisplayRole) const override;
    QVariant headerData(int section, Qt::Orientation orientation, int role = Qt::DisplayRole) const override;

    void populateStorageData(   const QList<QString> &storageName,
                                const QList<int> &storageID,
                                const QList<QString> &storageType,
                                const QList<QString> &storageLocation,
                                const QList<QString> &storagePath,
                                const QList<QString> &storageLabel,
                                const QList<QString> &storageFileSystemType,
                                const QList<qint64>  &storageBytesTotal,
                                const QList<qint64>  &storageBytesFree,
                                const QList<QIcon>   &storageIcon
                                );

private:
    QList<QString> storageName;
    QList<int> storageID;
    QList<QString> storageType;
    QList<QString> storageLocation;
    QList<QString> storagePath;
    QList<QString> storageLabel;
    QList<QString> storageFileSystemType;
    QList<qint64>  storageBytesTotal;
    QList<qint64>  storageBytesFree;
    QList<QIcon>   storageIcon;

    //QTreeView *storageTreeView;
    //QStandardItemModel *StorageStandardModel;
    //QList<QStandardItem *> prepareRow(const QString &storageName,
     //                                 const QString &storageID,
     //                                 const QString &storageType) const;

};
#endif // STORAGE_H
