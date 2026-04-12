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
// File Name:   statusbarmessagebuilder.cpp
// Purpose:     Implementation of status bar message builder
// Description: Builds formatted, translatable status messages with HTML styling
// Author:      Stephane Couturier
/////////////////////////////////////////////////////////////////////////////
*/

#include "statusbarmessagebuilder.h"
#include <QStringList>

StatusBarMessageBuilder::StatusBarMessageBuilder()
{
    m_formatOptions = defaultFormatOptions();
}

StatusBarMessageBuilder& StatusBarMessageBuilder::setFormatOptions(const FormatOptions& options)
{
    m_formatOptions = options;
    return *this;
}

StatusBarMessageBuilder& StatusBarMessageBuilder::setOperation(const QString& operation)
{
    m_operation = operation;
    return *this;
}

StatusBarMessageBuilder& StatusBarMessageBuilder::setStatus(const QString& status)
{
    m_status = status;
    return *this;
}

StatusBarMessageBuilder& StatusBarMessageBuilder::setCatalogIndex(int currentIndex, int totalCount)
{
    m_deviceCurrentIndex = currentIndex;
    m_deviceTotalCount = totalCount;
    return *this;
}

StatusBarMessageBuilder& StatusBarMessageBuilder::setCatalogName(const QString& catalogName)
{
    m_catalogName = catalogName;
    return *this;
}

StatusBarMessageBuilder& StatusBarMessageBuilder::setDeviceContext(int currentIndex, int totalCount, const QString& catalogName)
{
    // Convenience method that sets both
    setCatalogIndex(currentIndex, totalCount);
    setCatalogName(catalogName);
    return *this;
}

StatusBarMessageBuilder& StatusBarMessageBuilder::setProcess(const QString& title, qint64 currentCount, qint64 totalCount)
{
    m_processTitle = title;
    m_processCurrentCount = currentCount;
    m_processTotalCount = totalCount;
    return *this;
}


StatusBarMessageBuilder& StatusBarMessageBuilder::setResult(const QString& title, int count)
{
    m_results.clear();
    m_results.append({title, count, -1});
    return *this;
}

StatusBarMessageBuilder& StatusBarMessageBuilder::addResult(const QString& title, int count, qint64 size)
{
    m_results.append({title, count, size});
    return *this;
}

StatusBarMessageBuilder& StatusBarMessageBuilder::setSizeProgress(qint64 current, qint64 total)
{
    m_sizeProgressCurrent = current;
    m_sizeProgressTotal   = total;
    return *this;
}

StatusBarMessageBuilder& StatusBarMessageBuilder::setSpeed(double bytesPerSecond)
{
    m_speedBps = bytesPerSecond;
    return *this;
}

StatusBarMessageBuilder &StatusBarMessageBuilder::setTimeToCompletion(const QString &timeString)
{
    m_timeToCompletion = timeString;
    return *this;
}

StatusBarMessageBuilder& StatusBarMessageBuilder::setCurrentItem(const QString& itemPath)
{
    m_currentItem = itemPath;
    return *this;
}

StatusBarMessageBuilder& StatusBarMessageBuilder::reset()
{
    m_operation.clear();
    m_status.clear();
    m_deviceCurrentIndex = 0;
    m_deviceTotalCount = 0;
    m_catalogName.clear();
    m_processTitle.clear();
    m_processCurrentCount = -1;
    m_processTotalCount = -1;
    m_results.clear();
    m_sizeProgressCurrent = -1;
    m_sizeProgressTotal   = -1;
    m_speedBps            = -1.0;
    m_timeToCompletion.clear();
    m_currentItem.clear();
    return *this;
}

QString StatusBarMessageBuilder::build() const
{
    QStringList parts;

    // Part 1: Operation
    QString operation = formatOperation();
    if (!operation.isEmpty()) {
        parts << operation;
    }

    // Part 2: Status
    QString status = formatStatus();
    if (!status.isEmpty()) {
        parts << status;
    }
    // Part 3: Device Context
    QString deviceContext = formatDeviceContext();
    if (!deviceContext.isEmpty()) {
        parts << deviceContext;
    }

    // Part 4: Process
    QString process = formatProcess();
    if (!process.isEmpty()) {
        parts << process;
    }

    // Part 5: Result
    QString result = formatResult();
    if (!result.isEmpty()) {
        parts << result;
    }

    // Part 6: Size progress (bytes transferred / total)
    QString sizeProgress = formatSizeProgress();
    if (!sizeProgress.isEmpty()) {
        parts << sizeProgress;
    }

    // Part 7: Transfer speed
    QString speed = formatSpeed();
    if (!speed.isEmpty()) {
        parts << speed;
    }

    // Part 10: Time to completion
    QString timeToCompletion = formatTimeToCompletion();
    if (!timeToCompletion.isEmpty()) {
        parts << timeToCompletion;
    }

    // Part 11: Current Item
    QString currentItem = formatCurrentItem();
    if (!currentItem.isEmpty()) {
        parts << currentItem;
    }

    return parts.join(" | ");
}

StatusBarMessageBuilder::FormatOptions StatusBarMessageBuilder::defaultFormatOptions()
{
    FormatOptions options;
    options.operationBold = true;
    options.operationColor = "";
    options.catalogNameColor = "#39b2e5";  // Katalog theme blue
    options.resultsBold = true;
    options.processColor = "";
    options.currentItemItalic = true;
    return options;
}

QString StatusBarMessageBuilder::formatOperation() const
{
    if (m_operation.isEmpty()) {
        return QString();
    }

    QString opText = m_operation.toHtmlEscaped();

    // Convert to uppercase if enabled
    if (m_formatOptions.operationUppercase) {
        opText = opText.toUpper();
    }

    // Apply formatting
    if (m_formatOptions.operationBold && m_formatOptions.operationColor.isEmpty()) {
        // Bold only
        return QString("&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;<b>%1</b>").arg(opText);
    }
    else if (!m_formatOptions.operationColor.isEmpty()) {
        // Color with optional bold
        QString style = QString("color:%1;").arg(m_formatOptions.operationColor);
        if (m_formatOptions.operationBold) {
            style += " font-weight:bold;";
        }
        return QString("<span style='%1'>%2</span>").arg(style, opText);
    }

    return opText;
}

QString StatusBarMessageBuilder::formatStatus() const
{
    if (m_status.isEmpty()) {
        return QString();
    }

    return m_status.toHtmlEscaped();
}

QString StatusBarMessageBuilder::formatDeviceContext() const
{
    // Show if we have a catalog name (even for single catalog)
    if (m_deviceTotalCount == 0 || m_catalogName.isEmpty()) {
        return QString();
    }

    QString deviceText = QString("Catalog %1 of %2")
                             .arg(m_deviceCurrentIndex)
                             .arg(m_deviceTotalCount);

    if (!m_catalogName.isEmpty()) {
        QString safeName = m_catalogName.toHtmlEscaped();

        if (!m_formatOptions.catalogNameColor.isEmpty()) {
            // Apply catalog name color
            deviceText += " | "+ QString("<span style='color:%1;'>%2</span>")
                              .arg(m_formatOptions.catalogNameColor, safeName);
        } else {
            deviceText += QString(" (%1)").arg(safeName);
        }
    }

    return deviceText;
}

QString StatusBarMessageBuilder::formatProcess() const
{
    if (m_processTitle.isEmpty() || m_processCurrentCount < 0) {
        return QString();
    }

    QString processText = m_processTitle.toHtmlEscaped() + ": ";

    if (m_processTotalCount > 0) {
        // Show "X of Y (Z%)"
        double percent = calculatePercent();
        processText += QString("%1 of %2 (%3%)")
                           .arg(QLocale().toString(m_processCurrentCount),
                                QLocale().toString(m_processTotalCount),
                                QString::number(percent, 'f', 0));
    } else {
        // Show only "X" (total unknown)
        processText += QLocale().toString(m_processCurrentCount);
    }


    // Apply optional color
    if (!m_formatOptions.processColor.isEmpty()) {
        processText = QString("<span style='color:%1;'>%2</span>")
        .arg(m_formatOptions.processColor, processText);
    }

    return processText;
}

QString StatusBarMessageBuilder::formatResult() const
{
    if (m_results.isEmpty()) {
        return QString();
    }

    QStringList parts;
    for (const ResultItem &item : m_results) {
        QString countValue = QLocale().toString(item.count);
        if (m_formatOptions.resultsBold) {
            countValue = QString("<b>%1</b>").arg(countValue);
        }

        QString part = QString("%1: %2")
                           .arg(item.title.toHtmlEscaped(), countValue);

        if (item.size >= 0) {
            part += QString(" (%1)").arg(QLocale().formattedDataSize(item.size));
        }

        parts << part;
    }

    return parts.join(" | ");
}

QString StatusBarMessageBuilder::formatSizeProgress() const
{
    if (m_sizeProgressCurrent < 0 || m_sizeProgressTotal <= 0)
        return QString();

    return QString("%1 / %2")
        .arg(QLocale().formattedDataSize(m_sizeProgressCurrent),
             QLocale().formattedDataSize(m_sizeProgressTotal));
}

QString StatusBarMessageBuilder::formatSpeed() const
{
    if (m_speedBps < 0.0)
        return QString();

    return QLocale().formattedDataSize(static_cast<qint64>(m_speedBps)) + "/s";
}

QString StatusBarMessageBuilder::formatTimeToCompletion() const
{
    if (m_timeToCompletion.isEmpty()) {
        return QString();
    }

    // Return the time value directly (no label/prefix)
    return m_timeToCompletion;
}

QString StatusBarMessageBuilder::formatCurrentItem() const
{
    if (m_currentItem.isEmpty()) {
        return QString();
    }

    QString safeItem = m_currentItem.toHtmlEscaped();

    //Apply optional italic
    if (m_formatOptions.currentItemItalic) {
        return QString("<i>%1</i>").arg(safeItem);
    }

    return safeItem;
}

double StatusBarMessageBuilder::calculatePercent() const
{
    if (m_processTotalCount <= 0 || m_processCurrentCount < 0) {
        return 0.0;
    }

    return (static_cast<double>(m_processCurrentCount) * 100.0)
           / static_cast<double>(m_processTotalCount);
}
