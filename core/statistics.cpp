#include "statistics.h"
#include <QSqlQuery>
#include <QSqlDatabase>
#include <QtMath>

StatChartData Statistics::getChartData(
    const QString   &connectionName,
    int              deviceId,
    const QString   &deviceType,
    const QString   &source,
    const QString   &dataType,
    const QDateTime &startDate)
{
    StatChartData result;

    // Determine scale from max values in the table for this device
    QSqlQuery queryMax(QSqlDatabase::database(connectionName));
    queryMax.prepare(QLatin1String(R"(
        SELECT MAX(device_total_file_size), MAX(device_total_space)
        FROM statistics_device
        WHERE device_id = :device_id
    )"));
    queryMax.bindValue(":device_id", deviceId);
    queryMax.exec();
    queryMax.next();

    qint64 maxFileSize  = queryMax.value(0).toLongLong();
    qint64 maxTotSpace  = queryMax.value(1).toLongLong();
    qint64 maxRaw       = qMax(maxFileSize, maxTotSpace);

    qint64 sizeDivider  = 1;
    if      (maxRaw >= qint64(1024) * 1024 * 1024 * 1024) { sizeDivider = qint64(1024) * 1024 * 1024 * 1024; result.unitKey = "TiB"; }
    else if (maxRaw >= qint64(1024) * 1024 * 1024)         { sizeDivider = qint64(1024) * 1024 * 1024;        result.unitKey = "GiB"; }
    else if (maxRaw >= qint64(1024) * 1024)                 { sizeDivider = qint64(1024) * 1024;               result.unitKey = "MiB"; }
    else if (maxRaw >= qint64(1024))                        { sizeDivider = qint64(1024);                      result.unitKey = "KiB"; }

    // Build the main query with optional filters
    QString sql = QLatin1String(R"(
        SELECT date_time, device_file_count, device_total_file_size,
               device_free_space, device_total_space
        FROM statistics_device
        WHERE device_id = :device_id
    )");
    if (!startDate.isNull())
        sql += QLatin1String(" AND date_time > :startDate");
    if (source == QLatin1String("snapshots"))
        sql += QLatin1String(" AND record_type = 'snapshot'");
    else if (source == QLatin1String("updates"))
        sql += QLatin1String(" AND (record_type = 'update' OR record_type = 'create')");
    sql += QLatin1String(" ORDER BY date_time");

    QSqlQuery query(QSqlDatabase::database(connectionName));
    query.prepare(sql);
    query.bindValue(":device_id", deviceId);
    if (!startDate.isNull())
        query.bindValue(":startDate", startDate.toString("yyyy-MM-dd") + " 00:00:00");
    query.exec();

    const bool countMode = (dataType == QLatin1String("count"));

    while (query.next()) {
        QDateTime dt  = QDateTime::fromString(query.value(0).toString(), "yyyy-MM-dd hh:mm:ss");
        qint64    x   = dt.toMSecsSinceEpoch();

        qreal val1 = countMode
            ? static_cast<qreal>(query.value(1).toLongLong())
            : static_cast<qreal>(query.value(2).toLongLong()) / sizeDivider;

        qreal freeSpace  = static_cast<qreal>(query.value(3).toLongLong()) / sizeDivider;
        qreal totalSpace = static_cast<qreal>(query.value(4).toLongLong()) / sizeDivider;
        qreal usedSpace  = totalSpace - freeSpace;

        result.series1.append({x, val1});
        result.series2.append({x, usedSpace});
        result.series3.append({x, totalSpace});

        result.maxValue = qMax(result.maxValue, val1);
        result.maxValue = qMax(result.maxValue, usedSpace);
        result.maxValue = qMax(result.maxValue, totalSpace);
    }

    // Suppress storage series for Catalog-type devices or when counting files
    if (deviceType == QLatin1String("Catalog") || countMode) {
        result.loadSeries2 = false;
        result.loadSeries3 = false;
    }
    if (countMode)
        result.unitKey.clear();

    return result;
}
