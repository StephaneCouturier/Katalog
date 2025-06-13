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
    explicit SearchProgressManager(QStatusBar *statusBar, QTimer *timer, QObject *parent = nullptr)
        : QObject(parent), m_statusBar(statusBar), m_statusBarTimer(timer) {}

    void connectToSearchManager(SearchManager *searchManager);
    void setCurrentSearch(Search *currentSearch);

public slots:
    void updateFromSearchManager();
    void showMessage(const QString &message, int timeout = 0);

signals:
    void statusBarUpdated();

private:
    QStatusBar *m_statusBar = nullptr;
    QTimer *m_statusBarTimer = nullptr;
    SearchManager *m_searchManager = nullptr;
    Search *m_currentSearch = nullptr;
};

#endif // SEARCHPROGRESSMANAGER_H
