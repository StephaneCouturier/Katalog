#ifndef DIRECTORYTREEITEM_H
#define DIRECTORYTREEITEM_H

#include <QList>
#include <QVariant>

class DirectoryTreeItem
{
public:
    explicit DirectoryTreeItem(const QList<QVariant> &data, DirectoryTreeItem *parentItem = 0, unsigned int id = 0);
    ~DirectoryTreeItem();

    void appendChild(DirectoryTreeItem *child);

    DirectoryTreeItem *child(int row);
    int childCount() const;
    int columnCount() const;
    QVariant data(int column) const;
    int row() const;

    unsigned int getIndex(){return _id;};

    DirectoryTreeItem *parentItem();

private:

    QList<DirectoryTreeItem*> m_childItems;
    QList<QVariant> m_itemData;
    DirectoryTreeItem *m_parentItem;
    unsigned int _id;
};

#endif // DIRECTORYTREEITEM_H
