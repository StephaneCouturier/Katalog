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
#include <QList>
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
        bool operationUppercase = true;      // Auto-convert operation to uppercase
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
     * @brief Set the status (e.g., "Completed", "Paused", "Running")
     * @param status Status text (will be translated by caller)
     * @return Reference to this builder for chaining
     *
     * Generates: "Status" as second part
     */
    StatusBarMessageBuilder& setStatus(const QString& status);

    /**
     * @brief Set catalog index for batch operations
     * @param currentIndex Current catalog index (1-based)
     * @param totalCount Total number of catalogs
     * @return Reference to this builder for chaining
     */
    StatusBarMessageBuilder& setCatalogIndex(int currentIndex, int totalCount);

    /**
     * @brief Set catalog name
     * @param catalogName Name of current catalog
     * @return Reference to this builder for chaining
     */
    StatusBarMessageBuilder& setCatalogName(const QString& catalogName);

    /**
     * @brief Set device context (convenience method for single catalogs)
     * @param currentIndex Current device/catalog index (1-based)
     * @param totalCount Total number of devices/catalogs
     * @param catalogName Name of current catalog
     * @return Reference to this builder for chaining
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
     * @brief Set result information (single result — replaces any previously added results)
     * @param title Result title (e.g., "Found", "Files", "Folders")
     * @param count Result count
     * @return Reference to this builder for chaining
     *
     * Generates: "Title: N"
     */
    StatusBarMessageBuilder& setResult(const QString& title, int count);

    /**
     * @brief Append a result segment (supports multiple results in sequence)
     * @param title Result title (e.g., "To copy", "Conflicts", "Errors")
     * @param count Result count
     * @param size Optional byte size to display in parentheses after the count (-1 = none)
     * @return Reference to this builder for chaining
     *
     * Generates: "Title: N" or "Title: N (1.2 GB)" when size >= 0.
     * Multiple addResult() calls produce segments separated by " | ".
     */
    StatusBarMessageBuilder& addResult(const QString& title, int count, qint64 size = -1);

    /**
     * @brief Set data size progress (bytes transferred vs total)
     * @param current Bytes transferred so far
     * @param total Total bytes to transfer
     * @return Reference to this builder for chaining
     *
     * Generates: "1.2 GB / 4.5 GB"
     */
    StatusBarMessageBuilder& setSizeProgress(qint64 current, qint64 total);

    /**
     * @brief Set transfer speed
     * @param bytesPerSecond Current speed in bytes/second (< 0 = not yet known)
     * @return Reference to this builder for chaining
     *
     * Generates: "45 MB/s"
     */
    StatusBarMessageBuilder& setSpeed(double bytesPerSecond);

    /**
     * @brief Set time to completion
     * @param timeString Time string (e.g., "5m 23s", "1h 25m 10s")
     * @return Reference to this builder for chaining
     *
     * Displays the estimated time remaining
     */
    StatusBarMessageBuilder& setTimeToCompletion(const QString& timeString);

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
    // Result item (used by addResult / setResult)
    struct ResultItem {
        QString title;
        int     count = 0;
        qint64  size  = -1; // -1 = no size displayed
    };

    // Message components
    QString m_operation;
    QString m_status;
    int m_deviceCurrentIndex = 0;
    int m_deviceTotalCount = 0;
    QString m_catalogName;
    QString m_processTitle;
    int m_processCurrentCount = -1;
    int m_processTotalCount = -1;
    QList<ResultItem> m_results;
    qint64  m_sizeProgressCurrent = -1;
    qint64  m_sizeProgressTotal   = -1;
    double  m_speedBps            = -1.0;
    QString m_timeToCompletion;
    QString m_currentItem;

    // Formatting
    FormatOptions m_formatOptions;

    // Helper methods
    QString formatOperation() const;
    QString formatStatus() const;
    QString formatDeviceContext() const;
    QString formatProcess() const;
    QString formatResult() const;
    QString formatSizeProgress() const;
    QString formatSpeed() const;
    QString formatTimeToCompletion() const;
    QString formatCurrentItem() const;
    double calculatePercent() const;
};

#endif // STATUSBARMESSAGEBUILDER_H
