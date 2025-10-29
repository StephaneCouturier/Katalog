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
// File Name:   statusbarmessagebuilder.h
// Purpose:     Builder for creating formatted status bar messages
// Description: Provides consistent, translatable status messages with HTML formatting
//              Supports operations like SEARCH, UPDATE, EXPLORE with formatted output
// Author:      Stephane Couturier
/////////////////////////////////////////////////////////////////////////////
*/

#ifndef STATUSBARMESSAGEBUILDER_H
#define STATUSBARMESSAGEBUILDER_H

#include <QString>
#include <QLocale>

/**
 * @brief The StatusBarMessageBuilder class
 *
 * Builds formatted status bar messages with consistent structure and styling.
 * Supports HTML formatting for bold text, colors, and other visual emphasis.
 *
 * Message Structure:
 * OPERATION | Device Context | Process: X of Y (Z%) | Result: N | Current Item
 *
 * Example outputs:
 * - SEARCH | Catalog 1 of 5 (MyPhotos) | Update file types: 200 of 2000 (10.0%)
 * - UPDATE | Catalog 1 of 5 (MyPhotos) | Processed: 500 of 1000 (50.0%) | /home/user/file.txt
 * - EXPLORE | | Load index: 100 of 1000 (10.0%)
 *
 * Usage:
 * @code
 * QString message = StatusBarMessageBuilder()
 *     .setOperation(tr("SEARCH"))
 *     .setDeviceContext(1, 5, "MyPhotos")
 *     .setProcess(tr("Update file types"), 200, 2000)
 *     .build();
 * statusBar()->showMessage(message);
 * @endcode
 */
class StatusBarMessageBuilder
{
public:
    /**
     * @brief Format options for styling the message
     */
    struct FormatOptions {
        bool operationBold = true;           // Make operation text bold
        QString operationColor = "";         // Operation text color (empty = default)
        QString catalogNameColor = "#39b2e5"; // Catalog name color (Katalog theme blue)
        bool resultsBold = true;             // Make result numbers bold
        QString processColor = "";           // Process text color (empty = default)
        bool currentItemItalic = true;       // Make current item path italic
    };

    /**
     * @brief Constructor
     */
    StatusBarMessageBuilder();

    /**
     * @brief Set formatting options
     * @param options Format options to apply
     * @return Reference to this builder for chaining
     */
    StatusBarMessageBuilder& setFormatOptions(const FormatOptions& options);

    /**
     * @brief Set the operation name (e.g., "SEARCH", "UPDATE", "EXPLORE")
     * @param operation Operation name (will be translated by caller)
     * @return Reference to this builder for chaining
     */
    StatusBarMessageBuilder& setOperation(const QString& operation);

    /**
     * @brief Set device context for multi-device operations
     * @param currentIndex Current device/catalog index (1-based)
     * @param totalCount Total number of devices/catalogs
     * @param catalogName Name of current catalog
     * @return Reference to this builder for chaining
     *
     * Generates: "Catalog X of Y (CatalogName)"
     * If totalCount <= 1, this part is hidden
     */
    StatusBarMessageBuilder& setDeviceContext(int currentIndex, int totalCount, const QString& catalogName);

    /**
     * @brief Set process information
     * @param title Process title (e.g., "Update file types", "Processed")
     * @param currentCount Current progress count
     * @param totalCount Total count (0 = unknown, shows only current)
     * @return Reference to this builder for chaining
     *
     * Generates: "Title: X of Y (Z%)" or "Title: X" if total is 0
     */
    StatusBarMessageBuilder& setProcess(const QString& title, int currentCount, int totalCount = 0);

    /**
     * @brief Set result information
     * @param title Result title (e.g., "Found", "Files", "Folders")
     * @param count Result count
     * @return Reference to this builder for chaining
     *
     * Generates: "Title: N"
     */
    StatusBarMessageBuilder& setResult(const QString& title, int count);

    /**
     * @brief Set current item being processed
     * @param itemPath Path or name of current item
     * @return Reference to this builder for chaining
     *
     * Displays the path/filename being processed
     */
    StatusBarMessageBuilder& setCurrentItem(const QString& itemPath);

    /**
     * @brief Reset all values to defaults
     * @return Reference to this builder for chaining
     */
    StatusBarMessageBuilder& reset();

    /**
     * @brief Build the final formatted message
     * @return Formatted message string with HTML markup
     *
     * Combines all set components into a single message string.
     * Empty components are automatically hidden.
     */
    QString build() const;

    /**
     * @brief Get default format options
     * @return Default formatting options matching Katalog theme
     */
    static FormatOptions defaultFormatOptions();

private:
    // Message components
    QString m_operation;
    int m_deviceCurrentIndex = 0;
    int m_deviceTotalCount = 0;
    QString m_catalogName;
    QString m_processTitle;
    int m_processCurrentCount = -1;
    int m_processTotalCount = -1;
    QString m_resultTitle;
    int m_resultCount = -1;
    QString m_currentItem;

    // Formatting
    FormatOptions m_formatOptions;

    // Helper methods
    QString formatOperation() const;
    QString formatDeviceContext() const;
    QString formatProcess() const;
    QString formatResult() const;
    QString formatCurrentItem() const;
    double calculatePercent() const;
};

#endif // STATUSBARMESSAGEBUILDER_H
