#include "tag.h"

Tag::Tag(QObject *parent) : QAbstractTableModel(parent)
{

}

int Tag::rowCount(const QModelIndex &parent) const
{
    Q_UNUSED(parent);
    return folderPath.length();
}

int Tag::columnCount(const QModelIndex &parent) const
{
    Q_UNUSED(parent);
    return 2;
}

QVariant Tag::data(const QModelIndex &index, int role) const
{
    if (!index.isValid() || role != Qt::DisplayRole) {
        return QVariant();
    }
    switch (index.column()){
    case 0: return QString(folderPath[index.row()]);
    case 1: return QString(tagName[index.row()]);
    }
    return QVariant();
}

QVariant Tag::headerData(int section, Qt::Orientation orientation, int role) const
{
    if (role == Qt::DisplayRole && orientation == Qt::Horizontal) {
        switch (section){
        case 0: return QString("Folder");
        case 1: return QString("Tag");
        }
    }
    return QVariant();
}

// Create a method to populate the model with data:
void Tag::populateTagData(const QList<QString> &tFolderPath,
                          const QList<QString> &tTagName)
{
    folderPath.clear();
    folderPath = tFolderPath;
    tagName.clear();
    tagName = tTagName;

    return;
}
