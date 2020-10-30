#ifndef INITDB_H
#define INITDB_H

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


void addBook(QSqlQuery &q, const QString &title, int year, const QVariant &authorId,
             const QVariant &genreId, int rating)
{
    q.addBindValue(title);
    q.addBindValue(year);
    q.addBindValue(authorId);
    q.addBindValue(genreId);
    q.addBindValue(rating);
    q.exec();
}

QVariant addGenre(QSqlQuery &q, const QString &name)
{
    q.addBindValue(name);
    q.exec();
    return q.lastInsertId();
}

QVariant addAuthor(QSqlQuery &q, const QString &name, QDate birthdate)
{
    q.addBindValue(name);
    q.addBindValue(birthdate);
    q.exec();
    return q.lastInsertId();
}

const auto BOOKS_SQL = QLatin1String(R"(
    create table books(id integer primary key, title varchar, author integer,
                       genre integer, year integer, rating integer)
    )");

const auto AUTHORS_SQL =  QLatin1String(R"(
    create table authors(id integer primary key, name varchar, birthdate date)
    )");

const auto GENRES_SQL = QLatin1String(R"(
    create table genres(id integer primary key, name varchar)
    )");

const auto INSERT_AUTHOR_SQL = QLatin1String(R"(
    insert into authors(name, birthdate) values(?, ?)
    )");

const auto INSERT_BOOK_SQL = QLatin1String(R"(
    insert into books(title, year, author, genre, rating)
                      values(?, ?, ?, ?, ?)
    )");

const auto INSERT_GENRE_SQL = QLatin1String(R"(
    insert into genres(name) values(?)
    )");

QSqlError initDb()
{
    QSqlDatabase db = QSqlDatabase::addDatabase("QSQLITE");
    db.setDatabaseName(":memory:");

    if (!db.open())
        return db.lastError();

    QStringList tables = db.tables();
    if (tables.contains("books", Qt::CaseInsensitive)
        && tables.contains("authors", Qt::CaseInsensitive))
        return QSqlError();

    QSqlQuery q;
    if (!q.exec(STORAGE_SQL))
        return q.lastError();
    if (!q.exec(BOOKS_SQL))
        return q.lastError();
    if (!q.exec(AUTHORS_SQL))
        return q.lastError();
    if (!q.exec(GENRES_SQL))
        return q.lastError();


    if (!q.prepare(INSERT_STORAGE_SQL))
        return q.lastError();
    QVariant storageId1 = addStorage(q, 33, "Maxtor_2Tb",  "tbd",  "DK-External Drives 2'5",  "/run/media/stephane/Maxtor_2Tb",  "tbd", "tbd", 1854,  180);
    QVariant storageId3 = addStorage(q,
                                    31,
                                    "Maxtor_1Tb",
                                    "tbd",
                                    "DK-External Drives 2'5",
                                    "/run/media/stephane/Maxtor_1Tb",
                                    "tbd",
                                    "tbd",
                                    1000,
                                    10);

    if (!q.prepare(INSERT_AUTHOR_SQL))
        return q.lastError();
    QVariant asimovId = addAuthor(q, QLatin1String("Isaac Asimov"), QDate(1920, 2, 1));
    QVariant greeneId = addAuthor(q, QLatin1String("Graham Greene"), QDate(1904, 10, 2));
    QVariant pratchettId = addAuthor(q, QLatin1String("Terry Pratchett"), QDate(1948, 4, 28));

    if (!q.prepare(INSERT_GENRE_SQL))
        return q.lastError();
    QVariant sfiction = addGenre(q, QLatin1String("Science Fiction"));
    QVariant fiction = addGenre(q, QLatin1String("Fiction"));
    QVariant fantasy = addGenre(q, QLatin1String("Fantasy"));

    if (!q.prepare(INSERT_BOOK_SQL))
        return q.lastError();
    addBook(q, QLatin1String("Foundation"), 1951, asimovId, sfiction, 3);
    addBook(q, QLatin1String("Foundation and Empire"), 1952, asimovId, sfiction, 4);
    addBook(q, QLatin1String("Second Foundation"), 1953, asimovId, sfiction, 3);
    addBook(q, QLatin1String("Foundation's Edge"), 1982, asimovId, sfiction, 3);
    addBook(q, QLatin1String("Foundation and Earth"), 1986, asimovId, sfiction, 4);
    addBook(q, QLatin1String("Prelude to Foundation"), 1988, asimovId, sfiction, 3);
    addBook(q, QLatin1String("Forward the Foundation"), 1993, asimovId, sfiction, 3);
    addBook(q, QLatin1String("The Power and the Glory"), 1940, greeneId, fiction, 4);
    addBook(q, QLatin1String("The Third Man"), 1950, greeneId, fiction, 5);
    addBook(q, QLatin1String("Our Man in Havana"), 1958, greeneId, fiction, 4);
    addBook(q, QLatin1String("Guards! Guards!"), 1989, pratchettId, fantasy, 3);
    addBook(q, QLatin1String("Night Watch"), 2002, pratchettId, fantasy, 3);
    addBook(q, QLatin1String("Going Postal"), 2004, pratchettId, fantasy, 3);

    return QSqlError();
}

#endif // INITDB_H
