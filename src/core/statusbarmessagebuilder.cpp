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

StatusBarMessageBuilder& StatusBarMessageBuilder::setDeviceContext(int currentIndex, int totalCount, const QString& catalogName)
{
    m_deviceCurrentIndex = currentIndex;
    m_deviceTotalCount = totalCount;
    m_catalogName = catalogName;
    return *this;
}

StatusBarMessageBuilder& StatusBarMessageBuilder::setProcess(const QString& title, int currentCount, int totalCount)
{
    m_processTitle = title;
    m_processCurrentCount = currentCount;
    m_processTotalCount = totalCount;
    return *this;
}

StatusBarMessageBuilder& StatusBarMessageBuilder::setResult(const QString& title, int count)
{
    m_resultTitle = title;
    m_resultCount = count;
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
    m_deviceCurrentIndex = 0;
    m_deviceTotalCount = 0;
    m_catalogName.clear();
    m_processTitle.clear();
    m_processCurrentCount = -1;
    m_processTotalCount = -1;
    m_resultTitle.clear();
    m_resultCount = -1;
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

    // Part 2: Device Context
    QString deviceContext = formatDeviceContext();
    if (!deviceContext.isEmpty()) {
        parts << deviceContext;
    }

    // Part 3: Process
    QString process = formatProcess();
    if (!process.isEmpty()) {
        parts << process;
    }

    // Part 4: Result
    QString result = formatResult();
    if (!result.isEmpty()) {
        parts << result;
    }

    // Part 5: Current Item
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

    // Apply formatting
    if (m_formatOptions.operationBold && m_formatOptions.operationColor.isEmpty()) {
        // Bold only
        return QString("<b>%1</b>").arg(opText);
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

QString StatusBarMessageBuilder::formatDeviceContext() const
{
    // Only show if multiple devices
    if (m_deviceTotalCount <= 1) {
        return QString();
    }

    QString deviceText = QString("Catalog %1 of %2")
                             .arg(m_deviceCurrentIndex)
                             .arg(m_deviceTotalCount);

    if (!m_catalogName.isEmpty()) {
        QString safeName = m_catalogName.toHtmlEscaped();

        if (!m_formatOptions.catalogNameColor.isEmpty()) {
            // Apply catalog name color
            deviceText += QString(" (<span style='color:%1;'>%2</span>)")
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
                           .arg(QLocale().toString(m_processCurrentCount))
                           .arg(QLocale().toString(m_processTotalCount))
                           .arg(QString::number(percent, 'f', 1));
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
    if (m_resultTitle.isEmpty() || m_resultCount < 0) {
        return QString();
    }

    QString resultValue = QLocale().toString(m_resultCount);

    // Apply optional bold
    if (m_formatOptions.resultsBold) {
        resultValue = QString("<b>%1</b>").arg(resultValue);
    }

    return QString("%1: %2")
        .arg(m_resultTitle.toHtmlEscaped(), resultValue);
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

    return (static_cast<double>(m_processCurrentCount) * 100.0) / static_cast<double>(m_processTotalCount);
}
