#ifndef SEARCHPROGRESSMANAGER_H
#define SEARCHPROGRESSMANAGER_H

#include <QObject>
#include <QStatusBar>
#include <QString>
#include <QLocale>

// Forward declarations
class SearchManager;
class Search;

class SearchProgressManager : public QObject
{
    Q_OBJECT

public:
    explicit SearchProgressManager(QStatusBar *statusBar, QObject *parent = nullptr)
        : QObject(parent), m_statusBar(statusBar) {}

    void connectToSearchManager(SearchManager *searchManager);
    void setCurrentSearch(Search *currentSearch);

public slots:
    void updateFromSearchManager();
    void showMessage(const QString &message, int timeout = 0);
    void handleSpecialProgressValue(int filesProcessed);

signals:
    void statusBarUpdated();  // ADD THIS SIGNAL

private:
    QStatusBar *m_statusBar = nullptr;
    SearchManager *m_searchManager = nullptr;
    Search *m_currentSearch = nullptr;
};

#endif // SEARCHPROGRESSMANAGER_H
