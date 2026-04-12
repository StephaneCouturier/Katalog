/*LICENCE
    This file is part of Katalog

    Copyright (C) 2020, the Katalog Development team

    Author: Stephane Couturier (Symbioxy)

    Katalog is free software; you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation; either version 2 of the License, or
    (at your option) any later version.

    Katalog is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with Katalog; if not, write to the Free Software
    Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA
*/
/*FILE DESCRIPTION
/////////////////////////////////////////////////////////////////////////////
// Application: Katalog
// File Name:   foldertreeloader.cpp
// Purpose:     Core utility for loading folder tree data from database
// Description:
// Author:      Stephane Couturier
/////////////////////////////////////////////////////////////////////////////
*/

#include "foldertreeloader.h"
#include "database.h"
#include <QSqlQuery>
#include <QSqlDatabase>
#include <QSqlError>
#include <QSqlRecord>
#include <QDebug>

QList<FolderNode*> FolderTreeLoader::loadDirectoryTree(const QString &connectionName,
                                                        int catalogId,
                                                        const QString &catalogPath)
{
    QList<FolderNode*> nodes;

    QSqlQuery query(QSqlDatabase::database(connectionName));

    // Check database type for SQL syntax differences
    Database::DatabaseType dbType = Database::getDatabaseType(connectionName);
    QString querySQL;
    if (dbType == Database::DatabaseType::SQLite) {
        querySQL = QLatin1String(R"(
            SELECT DISTINCT (REPLACE(file_path, :selectedCatalogPath||'/', ''))
            FROM filesall
            WHERE   file_catalog_id =:file_catalog_id
            ORDER BY file_path ASC
        )");
    } else {
        // MySQL/MariaDB version using CONCAT()
        querySQL = QLatin1String(R"(
            SELECT DISTINCT (REPLACE(file_path, CONCAT(:selectedCatalogPath, '/'), ''))
            FROM filesall
            WHERE   file_catalog_id =:file_catalog_id
            ORDER BY file_path ASC
        )");
    }

    query.prepare(querySQL);
    query.bindValue(":file_catalog_id", catalogId);
    query.bindValue(":selectedCatalogPath", catalogPath);
    query.exec();

    while (query.next()) {
        FolderNode *node = new FolderNode();
        node->name = query.value(0).toString();
        node->fullPath = node->name;
        nodes.append(node);
    }

    return nodes;
}

QList<FolderNode*> FolderTreeLoader::loadExploreTree(const QString &connectionName,
                                                      int catalogId,
                                                      const QString &catalogSourcePathRoot)
{
    QList<FolderNode*> rootNodes;

    QSqlQuery query(QSqlDatabase::database(connectionName));
    QString querySQL = QLatin1String(R"(
        SELECT DISTINCT (REPLACE(folder_path, :catalogSourcePathRoot, '')) AS file_path,
               folder_path AS full_path
        FROM  folder
        WHERE folder_catalog_id=:folder_catalog_id
        ORDER BY full_path ASC
    )");
    query.prepare(querySQL);
    query.bindValue(":folder_catalog_id", catalogId);
    query.bindValue(":catalogSourcePathRoot", catalogSourcePathRoot);
    query.exec();

    int idPath = query.record().indexOf("file_path");

    // Build a map for quick parent lookup: full temp path -> FolderNode
    QHash<QString, FolderNode*> nodeMap;

    while (query.next()) {
        QString relativePath = query.value(idPath).toString();
        QStringList parts = relativePath.split("/", Qt::SkipEmptyParts);

        QString tempPath;
        FolderNode *parentNode = nullptr;

        for (int i = 0; i < parts.count(); ++i) {
            tempPath += parts.at(i) + "/";

            if (nodeMap.contains(tempPath)) {
                parentNode = nodeMap[tempPath];
                continue;
            }

            // Create new node
            FolderNode *newNode = new FolderNode();
            newNode->name = parts.at(i);

            // Compute full path
            QString fullPath = catalogSourcePathRoot + "/" + tempPath;
            fullPath.truncate(fullPath.length() - 1); // remove trailing /
            newNode->fullPath = fullPath;

            // Get file count for this folder
            if (i == parts.count() - 1) {
                // Leaf node: count files from filesall
                QSqlQuery countQuery(QSqlDatabase::database(connectionName));
                QString countSQL = QLatin1String(R"(
                    SELECT count(*) FROM filesall WHERE file_path like :path
                )");
                countQuery.prepare(countSQL);
                countQuery.bindValue(":path", tempPath + "%");
                if (countQuery.exec() && countQuery.next()) {
                    newNode->fileCount = countQuery.value(0).toInt();
                }
            }

            // Add to parent or root
            if (parentNode) {
                parentNode->children.append(newNode);
            } else {
                rootNodes.append(newNode);
            }

            nodeMap[tempPath] = newNode;
            parentNode = newNode;
        }
    }

    return rootNodes;
}
