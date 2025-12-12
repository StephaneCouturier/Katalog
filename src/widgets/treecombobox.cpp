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
// File Name:   treecombobox.cpp
// Purpose:     QComboBox subclass that displays a QTreeView in its popup
// Description: Used for hierarchical device selection
// Author:      Stephane Couturier
/////////////////////////////////////////////////////////////////////////////
*/
#include "treecombobox.h"

#include <QMouseEvent>
#include <QHeaderView>
#include <QApplication>

TreeComboBox::TreeComboBox(QWidget *parent)
    : QComboBox(parent)
    , m_treeView(new QTreeView(this))
    , m_idColumn(3)  // Default: column 3 contains device ID (matching existing model structure)
    , m_skipNextHide(false)
{
    // Configure the tree view
    m_treeView->setHeaderHidden(true);
    m_treeView->setFrameShape(QFrame::NoFrame);
    m_treeView->setEditTriggers(QTreeView::NoEditTriggers);
    m_treeView->setSelectionBehavior(QAbstractItemView::SelectRows);
    m_treeView->setSelectionMode(QAbstractItemView::SingleSelection);
    m_treeView->setRootIsDecorated(true);
    m_treeView->setItemsExpandable(true);
    m_treeView->setExpandsOnDoubleClick(false); // We handle this ourselves

    // Install event filter to handle expand/collapse clicks
    m_treeView->viewport()->installEventFilter(this);

    // Set the tree view as the popup view
    setView(m_treeView);
}

TreeComboBox::~TreeComboBox()
{
    // m_treeView is a child widget, automatically deleted
}

void TreeComboBox::setTreeModel(QAbstractItemModel *model)
{
    setModel(model);

    // Hide all columns except the first one (name)
    if (model) {
        for (int i = 1; i < model->columnCount(); ++i) {
            m_treeView->hideColumn(i);
        }
    }
}

int TreeComboBox::selectedDeviceId() const
{
    QModelIndex currentIdx = m_treeView->currentIndex();
    if (!currentIdx.isValid()) {
        return -1;
    }

    // Get the ID from the ID column of the same row
    QModelIndex idIdx = currentIdx.sibling(currentIdx.row(), m_idColumn);
    if (idIdx.isValid()) {
        return idIdx.data(Qt::DisplayRole).toInt();
    }

    return -1;
}

void TreeComboBox::setSelectedDeviceId(int deviceId)
{
    if (!model()) {
        return;
    }

    QModelIndex idx = findItemById(deviceId);
    if (idx.isValid()) {
        m_treeView->setCurrentIndex(idx);
        setRootModelIndex(idx.parent());
        setCurrentIndex(idx.row());
        updateDisplayText();
    }
}

void TreeComboBox::setIdColumn(int column)
{
    m_idColumn = column;
}

int TreeComboBox::idColumn() const
{
    return m_idColumn;
}

void TreeComboBox::expandToDepth(int depth)
{
    m_treeView->expandToDepth(depth);
}

void TreeComboBox::expandAll()
{
    m_treeView->expandAll();
}

void TreeComboBox::collapseAll()
{
    m_treeView->collapseAll();
}

void TreeComboBox::showPopup()
{
    // Reset root model index to show entire tree
    setRootModelIndex(QModelIndex());

    // Adjust popup size to show more of the tree
    if (model()) {
        // Calculate a reasonable height based on visible items
        int itemHeight = m_treeView->sizeHintForRow(0);
        if (itemHeight <= 0) {
            itemHeight = 25; // Default fallback
        }

        // Show approximately 15 items or fewer if tree is smaller
        int visibleItems = qMin(15, model()->rowCount() * 3); // Account for expanded items
        int popupHeight = qMax(200, itemHeight * visibleItems);

        // Set minimum size for the view
        m_treeView->setMinimumHeight(popupHeight);
    }

    QComboBox::showPopup();

    // Expand tree to show hierarchy (2 levels by default)
    m_treeView->expandToDepth(2);
}

void TreeComboBox::hidePopup()
{
    if (m_skipNextHide) {
        m_skipNextHide = false;
        return;
    }

    // Store the selected item before hiding
    QModelIndex currentIdx = m_treeView->currentIndex();
    if (currentIdx.isValid()) {
        // Set the root to the parent of selected item so QComboBox can find it
        setRootModelIndex(currentIdx.parent());
        setCurrentIndex(currentIdx.row());
    }

    QComboBox::hidePopup();

    // Emit signal with selected device ID
    int deviceId = selectedDeviceId();
    if (deviceId >= 0) {
        emit deviceSelected(deviceId);
    }

    updateDisplayText();
}

QTreeView* TreeComboBox::treeView() const
{
    return m_treeView;
}

bool TreeComboBox::eventFilter(QObject *object, QEvent *event)
{
    if (object == m_treeView->viewport() && event->type() == QEvent::MouseButtonPress) {
        QMouseEvent *mouseEvent = static_cast<QMouseEvent*>(event);
        QModelIndex index = m_treeView->indexAt(mouseEvent->pos());

        if (index.isValid()) {
            // Check if click is on the expand/collapse indicator (branch area)
            QRect itemRect = m_treeView->visualRect(index);

            // The branch indicator is typically to the left of the item
            // Get the indentation to determine where the indicator is
            int indent = m_treeView->indentation();
            int depth = 0;
            QModelIndex parent = index.parent();
            while (parent.isValid()) {
                depth++;
                parent = parent.parent();
            }

            // Calculate the x position where the item content starts
            int itemStart = indent * (depth + 1);

            // If click is in the branch area (before item content), it's an expand/collapse
            if (mouseEvent->pos().x() < itemStart) {
                // This is a click on the expand/collapse indicator
                m_skipNextHide = true;

                // Toggle expansion
                if (m_treeView->isExpanded(index)) {
                    m_treeView->collapse(index);
                } else {
                    m_treeView->expand(index);
                }

                return true; // Event handled
            }
        }
    }

    return QComboBox::eventFilter(object, event);
}

QModelIndex TreeComboBox::findItemById(int deviceId, const QModelIndex &parent) const
{
    if (!model()) {
        return QModelIndex();
    }

    int rowCount = model()->rowCount(parent);
    for (int row = 0; row < rowCount; ++row) {
        QModelIndex nameIdx = model()->index(row, 0, parent);
        QModelIndex idIdx = model()->index(row, m_idColumn, parent);

        if (idIdx.isValid() && idIdx.data(Qt::DisplayRole).toInt() == deviceId) {
            return nameIdx;
        }

        // Recursively search children
        if (model()->hasChildren(nameIdx)) {
            QModelIndex found = findItemById(deviceId, nameIdx);
            if (found.isValid()) {
                return found;
            }
        }
    }

    return QModelIndex();
}

void TreeComboBox::updateDisplayText()
{
    QModelIndex currentIdx = m_treeView->currentIndex();
    if (currentIdx.isValid()) {
        // Get the name from column 0
        QModelIndex nameIdx = currentIdx.sibling(currentIdx.row(), 0);
        if (nameIdx.isValid()) {
            setCurrentText(nameIdx.data(Qt::DisplayRole).toString());
        }
    }
}
