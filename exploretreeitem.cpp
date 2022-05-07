#include "exploretreeitem.h"

#include <QStringList>

ExploreTreeItem::ExploreTreeItem(const QList<QVariant> &data, ExploreTreeItem *parent, unsigned int id)
{
    m_parentItem = parent;
    m_itemData = data;
    _id = id;
}

ExploreTreeItem::~ExploreTreeItem()
{
    qDeleteAll(m_childItems);
}

void ExploreTreeItem::appendChild(ExploreTreeItem *item)
{
    m_childItems.append(item);
}

ExploreTreeItem *ExploreTreeItem::child(int row)
{
    return m_childItems.value(row);
}

int ExploreTreeItem::childCount() const
{
    return m_childItems.count();
}

int ExploreTreeItem::columnCount() const
{
    return m_itemData.count();
}

QVariant ExploreTreeItem::data(int column) const
{
    return m_itemData.value(column);
}

ExploreTreeItem *ExploreTreeItem::parentItem()
{
    return m_parentItem;
}

int ExploreTreeItem::row() const
{
    if (m_parentItem)
        return m_parentItem->m_childItems.indexOf(const_cast<ExploreTreeItem*>(this));

    return 0;
}
