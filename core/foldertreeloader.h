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
// File Name:   foldertreeloader.h
// Purpose:     Core utility for loading folder tree data from database
// Description: Provides data structures and methods for loading folder hierarchies
//              without any UI dependency, enabling reuse with different UI frameworks.
// Author:      Stephane Couturier
/////////////////////////////////////////////////////////////////////////////
*/
#ifndef FOLDERTREELOADER_H
#define FOLDERTREELOADER_H

#include <QString>
#include <QList>

struct FolderNode {
    QString name;
    QString fullPath;
    int fileCount = 0;
    QList<FolderNode*> children;

    ~FolderNode() { qDeleteAll(children); }
};

class FolderTreeLoader
{
public:
    /**
     * @brief Load directory tree from the filesall view (used by DirectoryTreeModel)
     * Returns a flat list of directory paths relative to catalogPath.
     */
    static QList<FolderNode*> loadDirectoryTree(const QString &connectionName,
                                                 const QString &catalogName,
                                                 const QString &catalogPath);

    /**
     * @brief Load explore tree from the folder table (used by ExploreTreeModel)
     * Returns a hierarchical tree of FolderNode with file counts.
     */
    static QList<FolderNode*> loadExploreTree(const QString &connectionName,
                                               int catalogId,
                                               const QString &catalogSourcePathRoot);
};

#endif // FOLDERTREELOADER_H
