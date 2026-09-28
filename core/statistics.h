#ifndef STATISTICS_H
#define STATISTICS_H

#include <QString>
#include <QDateTime>
#include <QList>

struct StatPoint {
    qint64 msecsSinceEpoch;
    qreal  value;
};

struct StatChartData {
    QList<StatPoint> series1;   // Catalogs: total size or file count
    QList<StatPoint> series2;   // Storage: used space
    QList<StatPoint> series3;   // Storage: total space
    bool    loadSeries1 = true;
    bool    loadSeries2 = true;
    bool    loadSeries3 = true;
    qreal   maxValue    = 0;
    QString unitKey;            // "", "KiB", "MiB", "GiB", "TiB"
};

class Statistics
{
public:
    // source:   "all" | "updates" | "snapshots"
    // dataType: "size" | "count"
    static StatChartData getChartData(
        const QString   &connectionName,
        int              deviceId,
        const QString   &deviceType,
        const QString   &source,
        const QString   &dataType,
        const QDateTime &startDate
    );

    // Summed {file count, total file size, free space, total space} of the
    // latest snapshot for one device type ("Catalog" / "Storage"); zeros if none.
    static QList<qint64> latestSnapshotTotals(const QString &connectionName,
                                              const QString &deviceType);
};

#endif // STATISTICS_H
