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

    //Definition of filetypes
    QStringList fileTypesPlain_Image, fileTypesPlain_Audio,fileTypesPlain_Video,fileTypesPlain_Text,fileTypesPlain_Other;
    fileTypesPlain_Image << "png" << "jpg" << "gif" << "xcf" << "tif" << "bmp";
    fileTypesPlain_Audio << "mp3" << "wav" << "ogg" << "aif";
    fileTypesPlain_Video << "wmv" << "avi" << "mp4" << "mkv" << "flv"  << "webm";
    fileTypesPlain_Text  << "txt" << "pdf" << "odt" << "idx" << "html" << "rtf" << "doc" << "docx" << "epub";
    fileTypesPlain_Other << "7z" << "zip" << "rar" << "gz" << "tar.gz" ;

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
                        bool isVideo = false;

                        // Priority: video duration first, then audio duration
                        if (videoDuration.isValid() && videoDuration.toInt() > 0) {
                            duration = videoDuration.toInt();
                            isVideo = true;
                        } else if (audioDuration.isValid() && audioDuration.toInt() > 0) {
                            duration = audioDuration.toInt();
                            isVideo = false;
                        }

                        if (duration > 0) {
                            if (isVideo) {
                                // Video Duration - show as H:MM:SS
                                int hours = duration / 3600;
                                int minutes = (duration % 3600) / 60;
                                int seconds = duration % 60;
                                return QString("%1:%2:%3")
                                    .arg(hours, 2, 10, QChar('0'))
                                    .arg(minutes, 2, 10, QChar('0'))
                                    .arg(seconds, 2, 10, QChar('0'));
                            } else {
                                // Audio Duration - show as MM:SS
                                int minutes = duration / 60;
                                int seconds = duration % 60;
                                return QString("%1:%2")
                                    .arg(minutes, 2, 10, QChar('0'))
                                    .arg(seconds, 2, 10, QChar('0'));
                            }
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
                    int fileTypeColumn = 6;
                    int mimeTypeColumn = 7;
                    QString fileType;
                    QString mimeType;

                    //Assign the icon per filetype
                    QModelIndex idx = index.sibling(index.row(), fileTypeColumn);
                    fileType = QSortFilterProxyModel::data(idx, Qt::DisplayRole).toString();
                    QModelIndex idx2 = index.sibling(index.row(), mimeTypeColumn);
                    mimeType = QSortFilterProxyModel::data(idx2, Qt::DisplayRole).toString();

                    if( QSortFilterProxyModel::data(idx, Qt::DisplayRole).toString()=="folder" ){
                        return QIcon::fromTheme("folder");
                    }
                    else if( fileType == "audio" ){
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
                    else //fileTypesPlain_None
                        return QIcon::fromTheme("application-x-zerosize");

                    // Fallback on the file extension
                    // QModelIndex idx = index.sibling(index.row(), 5);
                    // if( QSortFilterProxyModel::data(idx, Qt::DisplayRole).toString()=="folder" ){
                    //     return QIcon::fromTheme("folder");
                    // }
                    // else if( fileTypesPlain_Audio.contains(fileType,Qt::CaseInsensitive)){
                    //     return QIcon::fromTheme("audio-x-mpeg");
                    // }
                    // else if( fileTypesPlain_Image.contains(fileType,Qt::CaseInsensitive)){
                    //     return QIcon::fromTheme("image-jpeg");
                    // }
                    // else if(  fileTypesPlain_Text.contains(fileType,Qt::CaseInsensitive)){
                    //     return QIcon::fromTheme("view-list-text");
                    // }
                    // else if( fileTypesPlain_Video.contains(fileType,Qt::CaseInsensitive)){
                    //     return QIcon::fromTheme("video-mp4");
                    // }
                    // else if( fileTypesPlain_Other.contains(fileType,Qt::CaseInsensitive)){
                    //     return QIcon::fromTheme("document-open");
                    // }
                    // else //fileTypesPlain_None
                    //     return QIcon::fromTheme("application-x-zerosize");
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
