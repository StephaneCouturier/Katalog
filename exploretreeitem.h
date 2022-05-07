#ifndef EXPLORETREEITEM_H
#define EXPLORETREEITEM_H

#include <QList>
#include <QVariant>

class ExploreTreeItem
{
public:
    explicit ExploreTreeItem(const QList<QVariant> &data, ExploreTreeItem *parentItem = 0, unsigned int id = 0);
    ~ExploreTreeItem();

    void appendChild(ExploreTreeItem *child);

    ExploreTreeItem *child(int row);
    int childCount() const;
    int columnCount() const;
    QVariant data(int column) const;
    int row() const;

    unsigned int getIndex(){return _id;};

    ExploreTreeItem *parentItem();

private:

    QList<ExploreTreeItem*> m_childItems;
    QList<QVariant> m_itemData;
    ExploreTreeItem *m_parentItem;
    unsigned int _id;
};

#endif // EXPLORETREEITEM_H
