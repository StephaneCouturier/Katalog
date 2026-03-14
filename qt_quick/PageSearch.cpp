#include<QDebug>
#include<QClipboard>
#include<QGuiApplication>

#include "core/search.h"
#include "PageSearch.h"
#include "filesview.h"
#include "core/collection.h"

PageSearch::PageSearch(QObject *parent) : QObject(parent)
{

}

QString PageSearch::returnClipboard()
{
    QClipboard *clipboard = QGuiApplication::clipboard();
    QString originalText = clipboard->text();
    return originalText;
}

QString PageSearch::returnCleanedText(QString inputText)
{
    QString cleanedSearchText = inputText;
    cleanedSearchText.replace("."," ");
    cleanedSearchText.replace(","," ");
    cleanedSearchText.replace(";"," ");
    cleanedSearchText.replace(":"," ");
    cleanedSearchText.replace("_"," ");
    cleanedSearchText.replace("-"," ");
    cleanedSearchText.replace("("," ");
    cleanedSearchText.replace(")"," ");
    cleanedSearchText.replace("["," ");
    cleanedSearchText.replace("]"," ");
    cleanedSearchText.replace("{"," ");
    cleanedSearchText.replace("}"," ");
    cleanedSearchText.replace("/"," ");
    cleanedSearchText.replace("\\"," ");
    cleanedSearchText.replace("'"," ");
    cleanedSearchText.replace("\""," ");
    return cleanedSearchText;
}

//----------------------------------------------------------------------
