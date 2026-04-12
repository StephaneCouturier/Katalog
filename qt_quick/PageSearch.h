#ifndef PAGESEARCH_H
#define PAGESEARCH_H

#include<QObject>
#include<QSqlQueryModel>

class PageSearch : public QObject
{
    Q_OBJECT

public:
    explicit PageSearch(QObject *parent = nullptr);

    int lastSearchSortSection;
    int lastSearchSortOrder;

    //Search history
    int lastSearchHistorySortSection;
    int lastSearchHistorySortOrder;

public slots:
    QString returnClipboard();
    QString returnCleanedText(QString inputText);
};

#endif // PAGESEARCH_H
