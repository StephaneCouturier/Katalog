#include "directorytreeitem.h"

#include <QStringList>

DirectoryTreeItem::DirectoryTreeItem(const QList<QVariant> &data, DirectoryTreeItem *parent, unsigned int id)
{
    m_parentItem = parent;
    m_itemData = data;
    _id = id;
}

DirectoryTreeItem::~DirectoryTreeItem()
{
    qDeleteAll(m_childItems);
}

void DirectoryTreeItem::appendChild(DirectoryTreeItem *item)
{
    m_childItems.append(item);
}

DirectoryTreeItem *DirectoryTreeItem::child(int row)
{
    return m_childItems.value(row);
}

int DirectoryTreeItem::childCount() const
{
    return m_childItems.count();
}

int DirectoryTreeItem::columnCount() const
{
    return m_itemData.count();
}

QVariant DirectoryTreeItem::data(int column) const
{
    return m_itemData.value(column);
}

DirectoryTreeItem *DirectoryTreeItem::parentItem()
{
    return m_parentItem;
}

int DirectoryTreeItem::row() const
{
    if (m_parentItem)
        return m_parentItem->m_childItems.indexOf(const_cast<DirectoryTreeItem*>(this));

    return 0;
}
