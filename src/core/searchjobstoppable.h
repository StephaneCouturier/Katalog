#ifndef SEARCHJOBSTOPPABLE_H
#define SEARCHJOBSTOPPABLE_H

#include "search.h"
#include <QSqlDatabase>
#include <QMutex>
#include <QAtomicInt>
#include <QThread>
/**
 * @brief The SearchJobStoppable class
 * A clean, KJob-compatible search engine designed for database operations
 * Built with QtQuick compatibility and testability in mind
 */
class SearchJobStoppable : public Search
{
    Q_OBJECT

public:
    explicit SearchJobStoppable(QObject *parent = nullptr);
    virtual ~SearchJobStoppable();

    /**
     * @brief Set the database connection to use for search operations
     * @param connectionName Name of the QSqlDatabase connection
     */
    void setDatabaseConnection(const QString &connectionName);

    /**
     * @brief Main search method that coordinates the search process
     * @param selectedDevice The device to search in
     */
    void searchFiles(Device *selectedDevice) ;

    /**
     * @brief Stops an ongoing search
     */
    void stopSearch();

    /**
     * @brief Pauses the search (can be resumed later)
     */
    void pauseSearch();

    /**
     * @brief Resumes a paused search
     */
    void resumeSearch();

    /**
     * @brief Check if search was requested to stop
     */
    bool wasStopRequested() const { return m_stopRequested.loadAcquire(); }

    /**
     * @brief Check if search is currently paused
     */
    bool isPaused() const { return m_paused.loadAcquire(); }

    /**
     * @brief Set the progress refresh rate (files processed per progress update)
     */
    void setProgressRefreshRate(int rate) { progressRefreshRate = rate; }

protected:
    /**
     * @brief Search for files in a catalog
     * @param device The catalog device to search in
     * @param mutex Mutex for thread safety (unused in this implementation)
     * @param stopRequested Flag to indicate if the search should stop (unused)
     */
    void searchFilesInCatalog(Device *device, QMutex &mutex, bool &stopRequested) override;

    /**
     * @brief Search for files in a directory
     * @param sourceDirectory The directory to search in
     * @param mutex Mutex for thread safety (unused in this implementation)
     * @param stopRequested Flag to indicate if the search should stop (unused)
     */
    void searchFilesInDirectory(const QString &sourceDirectory, QMutex &mutex, bool &stopRequested) override;

    /**
     * @brief Process duplicates in search results
     * @param connectionName Database connection name
     */
    void processDuplicates(const QString &connectionName) override;

    /**
     * @brief Process differences between catalogs
     * @param connectionName Database connection name
     */
    void processDifferences(const QString &connectionName) override;

private:
    void checkStopAndPause();
    bool shouldContinue() const;
    void waitIfPaused();

    QString m_connectionName;
    QAtomicInt m_stopRequested{0};
    QAtomicInt m_paused{0};
    mutable QMutex m_pauseMutex;
};

#endif // SEARCHJOBSTOPPABLE_H
