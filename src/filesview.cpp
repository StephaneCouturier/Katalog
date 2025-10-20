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
// File Name:   filesview.cpp
// Purpose:     Class/model to display a list of files
// Description:
// Author:      Stephane Couturier
/////////////////////////////////////////////////////////////////////////////
*/
#include "filesview.h"
#include "core/filetypemapping.h"

#include <QFont>
#include <QBrush>
#include <QDebug>
#include <QFileIconProvider>
#include <QStandardItem>
#include <qmimedatabase.h>

FilesView::FilesView(QObject *parent)
    : QSortFilterProxyModel(parent)
{

}

QVariant FilesView::data(const QModelIndex &index, int role) const
{
    //Define list of columns per type of data
    QList<int> filesizeColumnList, filecountColumnList, durationColumnList;
      filesizeColumnList <<1;
      durationColumnList <<12;

    switch ( role )
         {
            case Qt::DisplayRole:
            {
                //File Size columns
                if( filesizeColumnList.contains(index.column()) ){
                    QModelIndex idx = index.sibling(index.row(), 5);
                    if( QSortFilterProxyModel::data(idx, Qt::DisplayRole).toString()=="folder" )
                        return "";
                    else
                        return QVariant( QLocale().formattedDataSize(QSortFilterProxyModel::data(index, role).toLongLong()) + "  ");
                }
                //Duration columns (merged: video + audio duration)
                else if (durationColumnList.contains(index.column())) {
                    if (index.column() == 12) {
                        // Merged Duration column - check both video and audio
                        QVariant videoDuration = QSortFilterProxyModel::data(index.sibling(index.row(), 12), role);
                        QVariant audioDuration = QSortFilterProxyModel::data(index.sibling(index.row(), 15), role);

                        int duration = 0;

                        // Priority: video duration first, then audio duration
                        if (videoDuration.isValid() && videoDuration.toInt() > 0) {
                            duration = videoDuration.toInt();
                            //isVideo = true;
                        } else if (audioDuration.isValid() && audioDuration.toInt() > 0) {
                            duration = audioDuration.toInt();
                            //isVideo = false;
                        }

                        if (duration > 0) {
                                int hours = duration / 3600;
                                int minutes = (duration % 3600) / 60;
                                int seconds = duration % 60;
                                return QString("%1:%2:%3")
                                    .arg(hours, 2, 10, QChar('0'))
                                    .arg(minutes, 2, 10, QChar('0'))
                                    .arg(seconds, 2, 10, QChar('0'));
                        }
                        return "";
                    }
                }
                //Width columns (merged: image + video width)
                else if (index.column() == 10) {
                    // Merged Width column - check both image and video
                    QVariant imageWidth = QSortFilterProxyModel::data(index.sibling(index.row(), 10), role);
                    QVariant videoWidth = QSortFilterProxyModel::data(index.sibling(index.row(), 13), role);

                    // Priority: image width first, then video width
                    if (imageWidth.isValid() && imageWidth.toInt() > 0) {
                        return QVariant(QLocale().toString(imageWidth.toInt()));
                    } else if (videoWidth.isValid() && videoWidth.toInt() > 0) {
                        return QVariant(QLocale().toString(videoWidth.toInt()));
                    }
                    return "";
                }
                //Height columns (merged: image + video height)
                else if (index.column() == 11) {
                    // Merged Height column - check both image and video
                    QVariant imageHeight = QSortFilterProxyModel::data(index.sibling(index.row(), 11), role);
                    QVariant videoHeight = QSortFilterProxyModel::data(index.sibling(index.row(), 14), role);

                    // Priority: image height first, then video height
                    if (imageHeight.isValid() && imageHeight.toInt() > 0) {
                        return QVariant(QLocale().toString(imageHeight.toInt()));
                    } else if (videoHeight.isValid() && videoHeight.toInt() > 0) {
                        return QVariant(QLocale().toString(videoHeight.toInt()));
                    }
                    return "";
                }

                //Numbers columns (without units)
                else if( filecountColumnList.contains(index.column()) ){
                    return QVariant(QLocale().toString(QSortFilterProxyModel::data(index, role).toDouble(), 'f', 0)  + "  ");
                }
                else QSortFilterProxyModel::data(index, role) ;

                break;
            }

            case Qt::TextAlignmentRole:
            {
                //align numbers to the right
                if ( filecountColumnList.contains(index.column()) )
                    return QVariant ( Qt::AlignVCenter | Qt::AlignRight );

                if ( filesizeColumnList.contains(index.column()) )
                    return QVariant ( Qt::AlignVCenter | Qt::AlignRight );

               break;
            }

            case Qt::DecorationRole:
            {
                if( index.column()==0 ){

                    //Identification of filetype
                    int entryTypeColumn = 5;  // For Explore view: "folder" or "file"
                    int fileTypeColumn = 8;   // For both views: file_type or "folder" for search results
                    int mimeTypeColumn = 9;
                    QString entryType;
                    QString fileType;
                    QString mimeType;
                    QString fileName;

                    //Get data from columns
                    QModelIndex entryTypeIdx = index.sibling(index.row(), entryTypeColumn);
                    entryType = QSortFilterProxyModel::data(entryTypeIdx, Qt::DisplayRole).toString();
                    QModelIndex idx = index.sibling(index.row(), fileTypeColumn);
                    fileType = QSortFilterProxyModel::data(idx, Qt::DisplayRole).toString();
                    QModelIndex idx2 = index.sibling(index.row(), mimeTypeColumn);
                    mimeType = QSortFilterProxyModel::data(idx2, Qt::DisplayRole).toString();

                    // Get filename for extension-based icon lookup
                    fileName = QSortFilterProxyModel::data(index, Qt::DisplayRole).toString();

                    // Handle folders first
                    if( entryType == "folder" || fileType == "folder" ){
                        return QIcon::fromTheme("folder");
                    }

                    // TESTING: Advanced icon modes
                    // TODO: Add settings check here for icon mode selection
                    // For now, hardcoded & disabled, this will be replaced with settings check
                    bool useAdvancedIcons = false;
                    bool useMimeIcons = false;
                    if (useAdvancedIcons) {
                        if (useMimeIcons && !mimeType.isEmpty()) {
                            // MIME MODE: Use QMimeDatabase for MIME-based icons
                            QIcon mimeIcon = getMimeBasedIcon(mimeType);
                            if (!mimeIcon.isNull()) {
                                qDebug() << "Using MIME icon for" << fileName << "mimeType:" << mimeType;
                                return mimeIcon;
                            }
                            qDebug() << "MIME icon failed for" << mimeType << ", falling back to extension mode";
                        }

                        // EXTENSION MODE: Extension-based system icons
                        QIcon advancedIcon = getAdvancedIcon(fileName, fileType);
                        if (!advancedIcon.isNull()) {
                            qDebug() << "Using extension-based icon for" << fileName;
                            return advancedIcon;
                        }
                        qDebug() << "Extension-based icon failed for" << fileName << ", falling back to simple mode";
                        // Fall through to simple mode if advanced icon not found
                    }

                    // SIMPLE MODE (fallback): Generic 6 icons, 1 per user file type
                    if( fileType == "audio" ){
                        return QIcon::fromTheme("audio-x-mpeg");
                    }
                    else if( fileType == "image" ){
                        return QIcon::fromTheme("image-jpeg");
                    }
                    else if( fileType == "video" ){
                        return QIcon::fromTheme("video-mp4");
                    }
                    else if( fileType == "other" ){
                        if( FileTypeMapping::isUserTypeText(mimeType) ){
                            return QIcon::fromTheme("view-list-text");
                        }
                        else
                            return QIcon::fromTheme("document-open");
                    }
                    else //fileType = none
                        return QIcon::fromTheme("application-x-zerosize");
                }
                else if( index.column()==3 ){
                    // Column 3 - Path column, ONLY show icon for folders (when column 0 is hidden in folder view)

                    int fileTypeColumn = 8;
                    QModelIndex fileTypeIdx = index.sibling(index.row(), fileTypeColumn);
                    QString fileType = QSortFilterProxyModel::data(fileTypeIdx, Qt::DisplayRole).toString();

                    // Only show folder icon in column 3 for folder entries
                    if( fileType == "folder" ){
                        return QIcon::fromTheme("folder");
                    }
                    // For files, return no icon in column 3
                    return QVariant();
                }

                break;
            }
        }
    return QSortFilterProxyModel::data(index, role);
}

QVariant FilesView::headerData(int section, Qt::Orientation orientation, int role) const
{
    switch ( role )
         {
            case Qt::DisplayRole:
            {
                return QSortFilterProxyModel::headerData( section, orientation, role) ;
            }
        }
        return QVariant();
}

bool FilesView::lessThan(const QModelIndex &left, const QModelIndex &right) const
{
    int column = left.column();

    // Special handling for merged metadata columns
    if (column == 10) { // Width column (merged: image_width + video_width)
        // Get width values from both image and video sources
        QVariant leftImageWidth = sourceModel()->data(sourceModel()->index(left.row(), 10));
        QVariant leftVideoWidth = sourceModel()->data(sourceModel()->index(left.row(), 13));
        QVariant rightImageWidth = sourceModel()->data(sourceModel()->index(right.row(), 10));
        QVariant rightVideoWidth = sourceModel()->data(sourceModel()->index(right.row(), 13));

        // Determine actual width values (image takes priority)
        int leftWidth = 0;
        int rightWidth = 0;

        if (leftImageWidth.isValid() && leftImageWidth.toInt() > 0) {
            leftWidth = leftImageWidth.toInt();
        } else if (leftVideoWidth.isValid() && leftVideoWidth.toInt() > 0) {
            leftWidth = leftVideoWidth.toInt();
        }

        if (rightImageWidth.isValid() && rightImageWidth.toInt() > 0) {
            rightWidth = rightImageWidth.toInt();
        } else if (rightVideoWidth.isValid() && rightVideoWidth.toInt() > 0) {
            rightWidth = rightVideoWidth.toInt();
        }

        return leftWidth < rightWidth;
    }
    else if (column == 11) { // Height column (merged: image_height + video_height)
        // Get height values from both image and video sources
        QVariant leftImageHeight = sourceModel()->data(sourceModel()->index(left.row(), 11));
        QVariant leftVideoHeight = sourceModel()->data(sourceModel()->index(left.row(), 14));
        QVariant rightImageHeight = sourceModel()->data(sourceModel()->index(right.row(), 11));
        QVariant rightVideoHeight = sourceModel()->data(sourceModel()->index(right.row(), 14));

        // Determine actual height values (image takes priority)
        int leftHeight = 0;
        int rightHeight = 0;

        if (leftImageHeight.isValid() && leftImageHeight.toInt() > 0) {
            leftHeight = leftImageHeight.toInt();
        } else if (leftVideoHeight.isValid() && leftVideoHeight.toInt() > 0) {
            leftHeight = leftVideoHeight.toInt();
        }

        if (rightImageHeight.isValid() && rightImageHeight.toInt() > 0) {
            rightHeight = rightImageHeight.toInt();
        } else if (rightVideoHeight.isValid() && rightVideoHeight.toInt() > 0) {
            rightHeight = rightVideoHeight.toInt();
        }

        return leftHeight < rightHeight;
    }
    else if (column == 12) { // Duration column (merged: video_duration + audio_duration)
        // Get duration values from both video and audio sources
        QVariant leftVideoDuration = sourceModel()->data(sourceModel()->index(left.row(), 12));
        QVariant leftAudioDuration = sourceModel()->data(sourceModel()->index(left.row(), 15));
        QVariant rightVideoDuration = sourceModel()->data(sourceModel()->index(right.row(), 12));
        QVariant rightAudioDuration = sourceModel()->data(sourceModel()->index(right.row(), 15));

        // Determine actual duration values (video takes priority)
        int leftDuration = 0;
        int rightDuration = 0;

        if (leftVideoDuration.isValid() && leftVideoDuration.toInt() > 0) {
            leftDuration = leftVideoDuration.toInt();
        } else if (leftAudioDuration.isValid() && leftAudioDuration.toInt() > 0) {
            leftDuration = leftAudioDuration.toInt();
        }

        if (rightVideoDuration.isValid() && rightVideoDuration.toInt() > 0) {
            rightDuration = rightVideoDuration.toInt();
        } else if (rightAudioDuration.isValid() && rightAudioDuration.toInt() > 0) {
            rightDuration = rightAudioDuration.toInt();
        }

        return leftDuration < rightDuration;
    }

    // Default sorting for all other columns
    QVariant leftData = sourceModel()->data(left);
    QVariant rightData = sourceModel()->data(right);

    #if (QT_VERSION < QT_VERSION_CHECK(6, 0, 0))
    if (leftData.type() == QMetaType::QString && rightData.type() == QMetaType::QString) {
    #else
    if (leftData.typeId() == QMetaType::QString && rightData.typeId() == QMetaType::QString) {
    #endif
        QString leftString = leftData.toString();
        QString rightString = rightData.toString();

        if (caseSensitive) {
            return leftString < rightString;
        } else {
            return QString::compare(leftString, rightString, Qt::CaseInsensitive) < 0;
        }
    }

    return QSortFilterProxyModel::lessThan(left, right);
}

// Simplified debug version - add this to filesview.cpp
QIcon FilesView::getAdvancedIcon(const QString &fileName, const QString &fileType) const
{
    // Extract file extension
    QString extension = fileName;
    int lastDot = extension.lastIndexOf('.');
    if (lastDot > 0) {
        extension = extension.mid(lastDot + 1).toLower();
    } else {
        return QIcon(); // No extension, return null icon
    }

    // Debug: Print what we're looking for
    qDebug() << "getAdvancedIcon: fileName=" << fileName << "extension=" << extension << "fileType=" << fileType;

    // Start with very common icons that should exist on most systems
    QString iconName;

    if (fileType == "image") {
        if (extension == "pdf") {
            iconName = "application-pdf";
        } else if (extension == "png") {
            iconName = "image-png";
        } else if (extension == "jpg" || extension == "jpeg") {
            iconName = "image-jpeg";
        } else {
            iconName = "image-x-generic";
        }
    }
    else if (fileType == "audio") {
        if (extension == "mp3") {
            iconName = "audio-mpeg";
        } else {
            iconName = "audio-x-generic";
        }
    }
    else if (fileType == "video") {
        if (extension == "mp4") {
            iconName = "video-mp4";
        } else if (extension == "avi") {
            iconName = "video-x-msvideo";
        } else {
            iconName = "video-x-generic";
        }
    }
    else if (fileType == "other") {
        if (extension == "pdf") {
            iconName = "application-pdf";
        } else if (extension == "txt") {
            iconName = "text-plain";
        } else if (extension == "zip") {
            iconName = "application-zip";
        } else if (extension == "html" || extension == "htm") {
            iconName = "text-html";
        } else {
            iconName = "text-x-generic";
        }
    }

    if (!iconName.isEmpty()) {
        QIcon icon = QIcon::fromTheme(iconName);
        qDebug() << "Trying icon:" << iconName << "isNull:" << icon.isNull();

        if (!icon.isNull()) {
            qDebug() << "SUCCESS: Using icon" << iconName;
            return icon;
        } else {
            qDebug() << "FAILED: Icon" << iconName << "not found, trying fallback";

            // Try some very basic fallbacks
            QString fallback;
            if (fileType == "image") fallback = "image-jpeg";
            else if (fileType == "audio") fallback = "audio-x-mpeg";
            else if (fileType == "video") fallback = "video-mp4";
            else fallback = "text-x-generic";

            QIcon fallbackIcon = QIcon::fromTheme(fallback);
            qDebug() << "Fallback icon:" << fallback << "isNull:" << fallbackIcon.isNull();
            return fallbackIcon;
        }
    }

    qDebug() << "No icon mapping found for extension:" << extension;
    return QIcon(); // Return null icon to use simple mode fallback
}

// Add this MIME-based icon method after getAdvancedIcon:
QIcon FilesView::getMimeBasedIcon(const QString &mimeType) const
{
    if (mimeType.isEmpty()) {
        return QIcon();
    }

    qDebug() << "getMimeBasedIcon: mimeType=" << mimeType;

    // Use QMimeDatabase to get the icon name for this MIME type
    QMimeDatabase mimeDb;
    QMimeType mime = mimeDb.mimeTypeForName(mimeType);

    if (!mime.isValid()) {
        qDebug() << "Invalid MIME type:" << mimeType;
        return QIcon();
    }

    QString iconName = mime.iconName();
    qDebug() << "MIME icon name:" << iconName;

    if (!iconName.isEmpty()) {
        QIcon icon = QIcon::fromTheme(iconName);
        qDebug() << "MIME icon" << iconName << "isNull:" << icon.isNull();

        if (!icon.isNull()) {
            qDebug() << "SUCCESS: Using MIME icon" << iconName;
            return icon;
        }
    }

    // Try generic icon for MIME type family
    QString genericIconName = mime.genericIconName();
    qDebug() << "MIME generic icon name:" << genericIconName;

    if (!genericIconName.isEmpty()) {
        QIcon genericIcon = QIcon::fromTheme(genericIconName);
        qDebug() << "MIME generic icon" << genericIconName << "isNull:" << genericIcon.isNull();

        if (!genericIcon.isNull()) {
            qDebug() << "SUCCESS: Using MIME generic icon" << genericIconName;
            return genericIcon;
        }
    }

    qDebug() << "No MIME icon found for:" << mimeType;
    return QIcon();
}
