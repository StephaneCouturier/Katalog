
#ifndef TREEITEM_H
#define TREEITEM_H

#include <QVariant>
#include <QList>

//! [0]
class TreeItem
{
public:
    explicit TreeItem(const QList<QVariant> &data, TreeItem *parentItem = nullptr);
    ~TreeItem();

    void appendChild(TreeItem *child);

    TreeItem *child(int row);
    int childCount() const;
    int columnCount() const;
    QVariant data(int column) const;
    int row() const;
    TreeItem *parentItem();

private:
    QList<TreeItem *> m_childItems;
    QList<QVariant> m_itemData;
    TreeItem *m_parentItem;
};
//! [0]

#endif // TREEITEM_H
