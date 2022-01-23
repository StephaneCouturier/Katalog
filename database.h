#ifndef DATABASE_H
#define DATABASE_H

#include <QtSql>
#include <QLocale>

// Catalog -----------------------------------------------------------------

    //Create table query
            const auto CATALOG_SQL = QLatin1String(R"(
                           create  table  if not exists  catalog(
                                    catalogID  int AUTO_INCREMENT primary key ,
                                    catalogFilePath         TEXT ,
                                    catalogName             TEXT  ,
                                    catalogDateUpdated      TEXT  ,
                                    catalogSourcePath       TEXT  ,
                                    catalogFileCount        REAL  ,
                                    catalogTotalFileSize    REAL ,
                                    catalogSourcePathIsActive  REAL ,
                                    catalogIncludeHidden    TEXT  ,
                                    catalogFileType         TEXT  ,
                                    catalogStorage          TEXT  ,
                                    catalogIncludeSymblinks TEXT,
                                    catalogIsFullDevice     TEXT,
                                    catalogLoadedVersion    TEXT)
                                )");
    //Insert row  query
            const auto INSERT_CATALOG_SQL = QLatin1String(R"(
                INSERT INTO catalog(
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
                                    catalogIncludeSymblinks,
                                    catalogIsFullDevice,
                                    catalogLoadedVersion)
                                VALUES(?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?)
                                )");

    //Insert row binding
            inline QVariant addCatalog( QSqlQuery &q,
                                        QString catalogFilePath,
                                        QString catalogName,
                                        QString catalogDateUpdated,
                                        QString catalogSourcePath,
                                        qint64 catalogFileCount,
                                        qint64 catalogTotalFileSize,
                                        int catalogSourcePathIsActive,
                                        QString catalogIncludeHidden,
                                        QString catalogFileType,
                                        QString catalogStorage,
                                        QString catalogIncludeSymblinks,
                                        QString catalogIsFullDevice,
                                        QString catalogLoadedVersion
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
                            q.addBindValue(catalogIsFullDevice);
                            q.addBindValue(catalogLoadedVersion);
                            q.exec();
                            return 0;
                        }

// Storage --------------------------------------------------------------

    //Create table query
            const auto STORAGE_SQL = QLatin1String(R"(
                           create  table  if not exists  storage(
                                storageID  int  primary key default 0,
                                storageName         TEXT,
                                storageType         TEXT,
                                storageLocation     TEXT,
                                storagePath         TEXT,
                                storageLabel        TEXT,
                                storageFileSystem   TEXT,
                                storageTotalSpace   REAL default 0,
                                storageFreeSpace    REAL default 0,
                                storageBrandModel   TEXT,
                                storageSerialNumber TEXT,
                                storageBuildDate    TEXT,
                                storageContentType  TEXT,
                                storageContainer    TEXT,
                                storageComment      TEXT)
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
                                storageBrandModel,
                                storageSerialNumber,
                                storageBuildDate,
                                storageContentType,
                                storageContainer,
                                storageComment)
                          values(?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?)
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
                                q.addBindValue(storageBrandModel);
                                q.addBindValue(storageSerialNumber);
                                q.addBindValue(storageBuildDate);
                                q.addBindValue(storageContentType);
                                q.addBindValue(storageContainer);
                                q.addBindValue(storageComment);
                                q.exec();
                                return 0;
                            }

// FILESALL (storing all catalogs files)----------------------------------------------------

        //Create table query
                const auto FILESALL_SQL = QLatin1String(R"(
                               create  table  if not exists  filesall(
                                        id_file             INTEGER,
                                        fileName            TEXT,
                                        filePath            TEXT,
                                        fileSize            REAL,
                                        fileDateUpdated     TEXT,
                                        fileCatalog         TEXT,
                                        PRIMARY KEY("id_file" AUTOINCREMENT))
                                    )");

// FILE (one-off requests) ----------------------------------------------------

        //Create table query
                const auto FILE_SQL = QLatin1String(R"(
                               create  table  if not exists  file(
                                        id_file             INTEGER,
                                        fileName            TEXT,
                                        filePath            TEXT,
                                        fileSize            REAL,
                                        fileDateUpdated     TEXT,
                                        fileCatalog         TEXT,
                                        PRIMARY KEY("id_file" AUTOINCREMENT))
                                    )");

// STATISTICS ------------------------------------------------

        //Create table query
                const auto STATISTICS_SQL = QLatin1String(R"(
                               create  table  if not exists  statistics(
                                        dateTime             TEXT,
                                        catalogName          TEXT,
                                        catalogFileCount     REAL,
                                        catalogTotalFileSize REAL,
                                        recordType TEXT)
                                    )");


// SEARCH ----------------------------------------------------

        //Create table query
                const auto SEARCH_SQL = QLatin1String(R"(
                               create  table  if not exists  search(
                                        dateTime            TEXT,
                                        TextChecked         INTEGER,
                                        TextPhrase          TEXT,
                                        TextCriteria        TEXT,
                                        TextSearchIn        TEXT,
                                        FileType            TEXT,
                                        FileSizeChecked     INTEGER,
                                        FileSizeMin         INTEGER,
                                        FileSizeMinUnit     INTEGER,
                                        FileSizeMax         INTEGER,
                                        FileSizeMaxUnit     INTEGER,
                                        DateModifiedChecked	INTEGER,
                                        DateModifiedMin     TEXT,
                                        DateModifiedMax     TEXT,
                                        DuplicatesChecked	INTEGER,
                                        DuplicateName       INTEGER,
                                        DuplicateSize       INTEGER,
                                        DuplicateDateModified   INTEGER,
                                        ShowFolders         INTEGER,
                                        TagChecked          INTEGER,
                                        Tag                 TEXT,
                                        searchLocation      TEXT,
                                        searchStorage       TEXT,
                                        searchCatalog       TEXT,
                                        SearchCatalogChecked    INTEGER,
                                        SearchDirectoryChecked  INTEGER,
                                        SeletedDirectory    TEXT,
                                        TextExclude         TEXT,
                                        CaseSensitive       INTEGER)
                                    )");

// TAG ----------------------------------------------------

        //Create table query
                const auto TAG_SQL = QLatin1String(R"(
										create  table  if not exists  tag(
											Name		TEXT,
											Path		TEXT,
											Type		TEXT,
											dateTime	TEXT
                                        )
                )");

// Database initialization ------------------------------------------------

QSqlError initializeDatabase()
{
    QSqlDatabase db = QSqlDatabase::addDatabase("QSQLITE");
    db.setDatabaseName(":memory:");
    //db.setDatabaseName("/home/stephane/Development/katalog101.db");

    if (!db.open())
        return db.lastError();

    QStringList tables = db.tables();

    QSqlQuery q;
    if (!q.exec(STORAGE_SQL))
        return q.lastError();

    if (!q.prepare(INSERT_STORAGE_SQL))
        return q.lastError();

    if (!q.exec(FILE_SQL))
        return q.lastError();

    if (!q.exec(FILESALL_SQL))
        return q.lastError();

    if (!q.exec(STATISTICS_SQL))
        return q.lastError();

    if (!q.exec(SEARCH_SQL))
        return q.lastError();

    if (!q.exec(TAG_SQL))
        return q.lastError();

    return QSqlError();
}

#endif // DATABASE_H
