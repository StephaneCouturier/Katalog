#ifndef PARALLELMETADATAEXTRACTOR_H
#define PARALLELMETADATAEXTRACTOR_H

#include <QString>
#include <QVariant>
#include <QRunnable>
#include <QMutex>
#include <QThreadPool>
#include <QWaitCondition>

// Result from a single file extraction
struct MetadataExtractionResult {
    QString fileName;
    QString folderPath;
    QVariantMap metadata;
};

// Worker task - extracts metadata for one file
class MetadataExtractionWorker : public QRunnable
{
public:
    MetadataExtractionWorker(const QString& filePath,
                             const QString& fileName,
                             const QString& folderPath,
                             const QString& includeMetadata,
                             QList<MetadataExtractionResult>* results,
                             QMutex* resultsMutex,
                             int* completedCount,
                             int totalCount,
                             QWaitCondition* allDoneCondition);

    void run() override;

private:
    QString m_filePath;
    QString m_fileName;
    QString m_folderPath;
    QString m_includeMetadata;
    QList<MetadataExtractionResult>* m_results;
    QMutex* m_resultsMutex;
    int* m_completedCount;
    int m_totalCount;
    QWaitCondition* m_allDoneCondition;
};

// Manager for parallel extraction
class ParallelMetadataExtractor
{
public:
    ParallelMetadataExtractor();
    ~ParallelMetadataExtractor();

    QList<MetadataExtractionResult> extractBatch(
        const QStringList& filePaths,
        const QStringList& fileNames,
        const QStringList& folderPaths,
        const QString& includeMetadata,
        int maxThreads = 4);

private:
    QThreadPool* m_threadPool;
};

#endif // PARALLELMETADATAEXTRACTOR_H
