#ifndef DATABASE_H
#define DATABASE_H

#include <QtSql>

QVariant addStorage(QSqlQuery &q,
                const int     &storageID,
                QString storageName,
                QString storageType,
                QString storageLocation,
                QString storagePath,
                QString storageLabel,
                QString storageFileSystem,
                qint64  storageTotalSpace,
                qint64  storageFreeSpace
                )
            {
                q.addBindValue(storageID);
                q.addBindValue(storageName);
                q.addBindValue(storageType);
                q.addBindValue(storageLocation);
                q.addBindValue(storagePath);
                q.addBindValue(storageLabel);
                q.addBindValue(storageFileSystem);
                q.addBindValue(storageTotalSpace);
                q.addBindValue(storageFreeSpace);
                q.exec();
                return 0;
            }
const auto STORAGE_SQL = QLatin1String(R"(
               create  table  if not exists  storage
               (
                 storageID  int  primary key default 0,
                 storageName  text  ,
                 storageType  text  ,
                 storageLocation  text  ,
                 storagePath  text  ,
                 storageLabel  text  ,
                 storageFileSystem  text  ,
                 storageTotalSpace  int  default 0,
                 storageFreeSpace  int  default 0
               )
            )");

const auto INSERT_STORAGE_SQL = QLatin1String(R"(
    insert into storage(storageID,
                        storageName,
                        storageType,
                        storageLocation,
                        storagePath,
                        storageLabel,
                        storageFileSystem,
                        storageTotalSpace,
                        storageFreeSpace)
                 values(?, ?, ?, ?, ?, ?, ?, ?, ?)
    )");


QSqlError initializeDatabase()
{
    QSqlDatabase db = QSqlDatabase::addDatabase("QSQLITE");
    db.setDatabaseName(":memory:");

    if (!db.open())
        return db.lastError();

    QStringList tables = db.tables();

    QSqlQuery q;
    if (!q.exec(STORAGE_SQL))
        return q.lastError();

    if (!q.prepare(INSERT_STORAGE_SQL))
        return q.lastError();

    return QSqlError();
}

#endif // DATABASE_H
