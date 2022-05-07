#ifndef DIRECTORYTREEITEM_H
#define DIRECTORYTREEITEM_H

#include <QList>
#include <QVariant>
#include <QtSql>
#include <QVector>

class DirectoryTreeItem
{
public:
    explicit DirectoryTreeItem(const QVector<QVariant> &data, DirectoryTreeItem *parent = nullptr);
    ~DirectoryTreeItem();

    DirectoryTreeItem *child(int number);
    int childCount() const;
    int columnCount() const;
    QVariant data(int column) const;
    bool insertChildren(int position, int count, int columns);
    bool insertColumns(int position, int columns);
    DirectoryTreeItem *parent();
    bool removeChildren(int position, int count);
    bool removeColumns(int position, int columns);
    int childNumber() const;
    bool setData(int column, const QVariant &value);

private:
    QVector<DirectoryTreeItem*> childItems;
    QVector<QVariant> itemData;
    DirectoryTreeItem *parentItem;

};
#endif // DIRECTORYTREEITEM_H
