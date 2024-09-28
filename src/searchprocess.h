#ifndef SEARCHPROCESS_H
#define SEARCHPROCESS_H

#include <QObject>
#include <QThread>
#include "mainwindow.h"

class SearchProcess : public QThread
{
    Q_OBJECT

public:
    explicit SearchProcess(MainWindow *mainWindow, QString databaseMode, QObject *parent = nullptr);
    void stop();
    QString databaseMode;
    QString connectionName;

protected:
    void run() override;

private:
    MainWindow *mainWindow;
    volatile bool isStopped;

    bool stopRequested = false;
    QMutex mutex;
    void searchFilesInCatalog(const Device *device);
    void searchFilesInDirectory(const QString &sourceDirectory);
    void processSearchResults();

signals:
    void searchProgress(int progress);
    void searchCompleted();
    void searchStopped();
    void searchResultsReady();
};

#endif // SEARCHPROCESS_H
