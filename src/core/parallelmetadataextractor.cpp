#include "parallelmetadataextractor.h"
#include "filemetadata.h"
#include <QDebug>
#include <QThread>

MetadataExtractionWorker::MetadataExtractionWorker(const QString& filePath,
                                                   const QString& fileName,
                                                   const QString& folderPath,
                                                   const QString& includeMetadata,
                                                   QList<MetadataExtractionResult>* results,
                                                   QMutex* resultsMutex,
                                                   int* completedCount,
                                                   int totalCount,
                                                   QWaitCondition* allDoneCondition)
    : m_filePath(filePath),
    m_fileName(fileName),
    m_folderPath(folderPath),
    m_includeMetadata(includeMetadata),
    m_results(results),
    m_resultsMutex(resultsMutex),
    m_completedCount(completedCount),
    m_totalCount(totalCount),
    m_allDoneCondition(allDoneCondition)
{
}

void MetadataExtractionWorker::run()
{
    QVariantMap metadata = FileMetadata::extractMetadata(m_filePath, m_includeMetadata);

    {
        QMutexLocker lock(m_resultsMutex);
        MetadataExtractionResult result;
        result.fileName = m_fileName;
        result.folderPath = m_folderPath;
        result.metadata = metadata;
        m_results->append(result);

        (*m_completedCount)++;

        if (*m_completedCount >= m_totalCount) {
            m_allDoneCondition->wakeAll();
        }
    }
}

ParallelMetadataExtractor::ParallelMetadataExtractor()
{
    m_threadPool = new QThreadPool();
}

ParallelMetadataExtractor::~ParallelMetadataExtractor()
{
    if (m_threadPool) {
        m_threadPool->waitForDone();
    }
}

QList<MetadataExtractionResult> ParallelMetadataExtractor::extractBatch(
    const QStringList& filePaths,
    const QStringList& fileNames,
    const QStringList& folderPaths,
    const QString& includeMetadata,
    int maxThreads)
{
    QList<MetadataExtractionResult> results;

    if (filePaths.isEmpty()) {
        return results;
    }

    m_threadPool->setMaxThreadCount(maxThreads);

    QMutex resultsMutex;
    int completedCount = 0;
    QWaitCondition allDoneCondition;

    for (int i = 0; i < filePaths.size(); ++i) {
        MetadataExtractionWorker* worker = new MetadataExtractionWorker(
            filePaths[i],
            fileNames[i],
            folderPaths[i],
            includeMetadata,
            &results,
            &resultsMutex,
            &completedCount,
            filePaths.size(),
            &allDoneCondition
            );

        m_threadPool->start(worker);
    }

    {
        QMutexLocker lock(&resultsMutex);
        while (completedCount < filePaths.size()) {
            allDoneCondition.wait(&resultsMutex);
        }
    }

    qDebug() << "Parallel extraction completed for" << results.size() << "files";
    return results;
}
