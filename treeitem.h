#ifndef TREEITEM_H
#define TREEITEM_H


#include <QList>
#include <QVariant>
#include <QtSql>

class TreeItem
{
public:
    explicit TreeItem(const QList<QVariant> &data, TreeItem *parentItem = 0, unsigned int id = 0);
    ~TreeItem();

    void appendChild(TreeItem *child);

    TreeItem *child(int row);
    int childCount() const;
    int columnCount() const;
    QVariant data(int column) const;
    int row() const;

    unsigned int getIndex(){return _id;};

    TreeItem *parentItem();

private:

    QList<TreeItem*> m_childItems;
    QList<QVariant> m_itemData;
    TreeItem *m_parentItem;
    unsigned int _id;
};


#endif // TREEITEM_H
