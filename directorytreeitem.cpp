#include "directorytreeitem.h"

#include <QStringList>

DirectoryTreeItem::DirectoryTreeItem(const QVector<QVariant> &data, DirectoryTreeItem *parent)
    : itemData(data),
      parentItem(parent)
{}

DirectoryTreeItem::~DirectoryTreeItem()
{
    qDeleteAll(childItems);
}

DirectoryTreeItem *DirectoryTreeItem::child(int number)
{
    if (number < 0 || number >= childItems.size())
        return nullptr;
    return childItems.at(number);
}

int DirectoryTreeItem::childCount() const
{
    return childItems.count();
}

int DirectoryTreeItem::childNumber() const
{
    if (parentItem)
        return parentItem->childItems.indexOf(const_cast<DirectoryTreeItem*>(this));
    return 0;
}

int DirectoryTreeItem::columnCount() const
{
    return itemData.count();
}

QVariant DirectoryTreeItem::data(int column) const
{
    if (column < 0 || column >= itemData.size())
        return QVariant();
    return itemData.at(column);
}

bool DirectoryTreeItem::insertChildren(int position, int count, int columns)
{
    if (position < 0 || position > childItems.size())
        return false;

    for (int row = 0; row < count; ++row) {
        QVector<QVariant> data(columns);
        DirectoryTreeItem *item = new DirectoryTreeItem(data, this);
        childItems.insert(position, item);
    }

    return true;
}

bool DirectoryTreeItem::insertColumns(int position, int columns)
{
    if (position < 0 || position > itemData.size())
        return false;

    for (int column = 0; column < columns; ++column)
        itemData.insert(position, QVariant());

    for (DirectoryTreeItem *child : qAsConst(childItems))
        child->insertColumns(position, columns);

    return true;
}

DirectoryTreeItem *DirectoryTreeItem::parent()
{
    return parentItem;
}

bool DirectoryTreeItem::removeChildren(int position, int count)
{
    if (position < 0 || position + count > childItems.size())
        return false;

    for (int row = 0; row < count; ++row)
        delete childItems.takeAt(position);

    return true;
}

bool DirectoryTreeItem::removeColumns(int position, int columns)
{
    if (position < 0 || position + columns > itemData.size())
        return false;

    for (int column = 0; column < columns; ++column)
        itemData.remove(position);

    for (DirectoryTreeItem *child : qAsConst(childItems))
        child->removeColumns(position, columns);

    return true;
}

bool DirectoryTreeItem::setData(int column, const QVariant &value)
{
    if (column < 0 || column >= itemData.size())
        return false;

    itemData[column] = value;
    return true;
}






