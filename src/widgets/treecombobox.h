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
// File Name:   treecombobox.h
// Purpose:     QComboBox subclass that displays a QTreeView in its popup
// Description: Used for hierarchical device selection
// Author:      Stephane Couturier
/////////////////////////////////////////////////////////////////////////////
*/
#ifndef TREECOMBOBOX_H
#define TREECOMBOBOX_H

#include <QComboBox>
#include <QTreeView>
#include <QStandardItemModel>

class TreeComboBox : public QComboBox
{
    Q_OBJECT

public:
    explicit TreeComboBox(QWidget *parent = nullptr);
    ~TreeComboBox() override;

    // Set the model for the tree view
    void setTreeModel(QAbstractItemModel *model);

    // Get/Set the selected device ID (stored in column with ID data)
    int selectedDeviceId() const;
    void setSelectedDeviceId(int deviceId);

    // Configure which column contains the device ID
    void setIdColumn(int column);
    int idColumn() const;

    // Expand tree to a specific depth
    void expandToDepth(int depth);
    void expandAll();
    void collapseAll();

    // Override to show the tree popup properly
    void showPopup() override;
    void hidePopup() override;

    // Get the tree view for additional customization
    QTreeView* treeView() const;

signals:
    // Emitted when a device is selected
    void deviceSelected(int deviceId);

protected:
    // Event filter to handle expand/collapse without closing popup
    bool eventFilter(QObject *object, QEvent *event) override;

private:
    QTreeView *m_treeView;
    int m_idColumn;
    bool m_skipNextHide;

    // Find and select item by device ID recursively
    QModelIndex findItemById(int deviceId, const QModelIndex &parent = QModelIndex()) const;
    void updateDisplayText();
};

#endif // TREECOMBOBOX_H
