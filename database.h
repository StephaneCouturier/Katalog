#ifndef DATABASE_H
#define DATABASE_H

#include <QtSql>
#include <QLocale>

// Storage --------------------------------------------------------------

    //Create table query
            const auto STORAGE_SQL = QLatin1String(R"(
                           create  table  if not exists  storage(
                                storageID  int  primary key default 0,
                                storageName  text  ,
                                storageType  text  ,
                                storageLocation  text  ,
                                storagePath  text  ,
                                storageLabel  text  ,
                                storageFileSystem  text  ,
                                storageTotalSpace  int  default 0,
                                storageFreeSpace  int  default 0,
                                storageTotalSpaceDisplay  text  ,
                                storageFreeSpaceDisplay  text ,
                                storageBrandModel  text  ,
                                storageSerialNumber   text  ,
                                storageBuildDate  text  ,
                                storageContentType  text  ,
                                storageContainer  text  ,
                                storageComment  text)
                            )");

    //Insert row  query
            const auto INSERT_STORAGE_SQL = QLatin1String(R"(
                insert into storage(
                                storageID,
                                storageName,
                                storageType,
                                storageLocation,
                                storagePath,
                                storageLabel,
                                storageFileSystem,
                                storageTotalSpace,
                                storageFreeSpace,
                                storageTotalSpaceDisplay,
                                storageFreeSpaceDisplay,
                                storageBrandModel,
                                storageSerialNumber,
                                storageBuildDate,
                                storageContentType,
                                storageContainer,
                                storageComment)
                          values(?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?)
                        )");

    //Insert row binding
                QVariant addStorage(QSqlQuery &q,
                                const int     &storageID,
                                QString storageName,
                                QString storageType,
                                QString storageLocation,
                                QString storagePath,
                                QString storageLabel,
                                QString storageFileSystem,
                                qint64  storageTotalSpace,
                                qint64  storageFreeSpace,
                                QString storageTotalSpaceDisplay,
                                QString storageFreeSpaceDisplay,
                                QString storageBrandModel,
                                QString storageSerialNumber,
                                QString storageBuildDate,
                                QString storageContentType,
                                QString storageContainer,
                                QString storageComment
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
                                q.addBindValue(storageTotalSpaceDisplay);
                                q.addBindValue(storageFreeSpaceDisplay);
                                q.addBindValue(storageBrandModel);
                                q.addBindValue(storageSerialNumber);
                                q.addBindValue(storageBuildDate);
                                q.addBindValue(storageContentType);
                                q.addBindValue(storageContainer);
                                q.addBindValue(storageComment);
                                q.exec();
                                return 0;
                            }


// Catalog -----------------------------------------------------------------

    //Create table query
            const auto CATALOG_SQL = QLatin1String(R"(
                           create  table  if not exists  catalog(
                                    catalogID  int AUTO_INCREMENT primary key ,
                                    catalogFilePath  text ,
                                    catalogName  text  ,
                                    catalogDateUpdated  text  ,
                                    catalogSourcePath  text  ,
                                    catalogFileCount   int  ,
                                    catalogTotalFileSize  int ,
                                    catalogSourcePathIsActive  text ,
                                    catalogIncludeHidden  text  ,
                                    catalogFileType  text  ,
                                    catalogStorage  text  ,
                                    catalogIncludeSymblinks  text)
                                )");
    //Insert row  query
            const auto INSERT_CATALOG_SQL = QLatin1String(R"(
                insert into catalog(
                                    catalogFilePath,
                                    catalogName,
                                    catalogDateUpdated,
                                    catalogSourcePath,
                                    catalogFileCount,
                                    catalogTotalFileSize,
                                    catalogSourcePathIsActive,
                                    catalogIncludeHidden,
                                    catalogFileType,
                                    catalogStorage,
                                    catalogIncludeSymblinks)
                                values(?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?)
                                )");

    //Insert row binding
            inline QVariant addCatalog(QSqlQuery &q,
                            QString catalogFilePath,
                            QString catalogName,
                            QString catalogDateUpdated,
                            QString catalogSourcePath,
                            int catalogFileCount,
                            qint64 catalogTotalFileSize, //KFormat.formatByteSize
                            QString catalogSourcePathIsActive,
                            QString catalogIncludeHidden,
                            QString catalogFileType,
                            QString catalogStorage,
                            QString catalogIncludeSymblinks
                            )
                        {
                            q.addBindValue(catalogFilePath);
                            q.addBindValue(catalogName);
                            q.addBindValue(catalogDateUpdated);
                            q.addBindValue(catalogSourcePath);
                            q.addBindValue(catalogFileCount);
                            q.addBindValue(catalogTotalFileSize);
                            q.addBindValue(catalogSourcePathIsActive);
                            q.addBindValue(catalogIncludeHidden);
                            q.addBindValue(catalogFileType);
                            q.addBindValue(catalogStorage);
                            q.addBindValue(catalogIncludeSymblinks);
                            q.exec();
                            return 0;
                        }


// Database initialization ------------------------------------------------


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
