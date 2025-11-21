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
// File Name:   backupprofilegenerator.cpp
// Purpose:     Implementation of LuckyBackup profile generator
// Description: Creates LuckyBackup-compatible profile files from Katalog
//              device mappings
// Author:      Stephane Couturier
/////////////////////////////////////////////////////////////////////////////
*/

#include "backupprofilegenerator.h"
#include <QSqlQuery>
#include <QSqlError>
#include <QSqlDatabase>
#include <QDir>
#include <QFile>
#include <QTextStream>
#include <QStandardPaths>
#include <QFileInfo>
#include <QDebug>

//------------------------------------------------------------------------------
// Constructor
//------------------------------------------------------------------------------

BackupProfileGenerator::BackupProfileGenerator(const QString& connectionName,
                                               QObject *parent)
    : QObject(parent)
    , m_connectionName(connectionName)
{
}

//------------------------------------------------------------------------------
// Public Methods - Profile Generation
//------------------------------------------------------------------------------

BackupProfileResult BackupProfileGenerator::generateProfile()
{
    return generateProfile(QList<int>()); // Empty list = all mappings
}

BackupProfileResult BackupProfileGenerator::generateProfile(const QList<int>& mappingIds)
{
    BackupProfileResult result;

    emit profileGenerationStarted();

    // Step 1: Ensure output directory exists
    if (!ensureProfilesDirectoryExists()) {
        result.success = false;
        result.errorMessage = tr("Failed to create LuckyBackup profiles directory");
        emit profileGenerationFailed(result.errorMessage);
        return result;
    }

    // Step 2: Load backup tasks from database
    QList<BackupTask> tasks = loadBackupTasks(mappingIds);

    if (tasks.isEmpty()) {
        result.success = false;
        result.errorMessage = tr("No backup mappings found in the database");
        emit profileGenerationFailed(result.errorMessage);
        return result;
    }

    emit profileGenerationProgress(0, tasks.size());

    // Step 3: Generate profile content
    QString profileContent = generateProfileContent(tasks);

    // Step 4: Generate filename and full path
    result.profileName = generateProfileFilename();
    result.profilePath = getDefaultProfilesDirectory() + "/" + result.profileName;

    // Step 5: Write to file
    result.success = writeProfileToFile(profileContent, result.profilePath);
    result.taskCount = tasks.size();

    if (result.success) {
        emit profileGenerationCompleted(result.profilePath, result.taskCount);
    } else {
        result.errorMessage = tr("Failed to write profile file to: %1").arg(result.profilePath);
        emit profileGenerationFailed(result.errorMessage);
    }

    return result;
}

//------------------------------------------------------------------------------
// Public Static Methods - Directory Management
//------------------------------------------------------------------------------

QString BackupProfileGenerator::getDefaultProfilesDirectory()
{
    return getUserHomeDirectory() + "/.luckyBackup/profiles";
}

bool BackupProfileGenerator::isProfilesDirectoryAccessible()
{
    QDir dir(getDefaultProfilesDirectory());
    if (!dir.exists()) {
        return false;
    }

    // Check if writable by attempting to create a test file
    QString testPath = getDefaultProfilesDirectory() + "/.test_write";
    QFile testFile(testPath);
    bool writable = testFile.open(QIODevice::WriteOnly);
    if (writable) {
        testFile.close();
        testFile.remove();
    }

    return writable;
}

bool BackupProfileGenerator::ensureProfilesDirectoryExists()
{
    QDir dir(getDefaultProfilesDirectory());
    if (dir.exists()) {
        return true;
    }

    // Create the directory with all parent directories
    return dir.mkpath(getDefaultProfilesDirectory());
}

//------------------------------------------------------------------------------
// Private Methods - Database Operations
//------------------------------------------------------------------------------

QList<BackupTask> BackupProfileGenerator::loadBackupTasks(const QList<int>& mappingIds)
{
    QList<BackupTask> tasks;

    QSqlQuery query(QSqlDatabase::database(m_connectionName));
    QString querySQL = QLatin1String(R"(
        SELECT
            dm.mapping_id,
            dm.mapping_name,
            d_source.device_id AS source_id,
            d_source.device_name AS source_name,
            d_source.device_path AS source_path,
            d_target.device_id AS target_id,
            d_target.device_name AS target_name,
            d_target.device_path AS target_path
        FROM device_mapping dm
        INNER JOIN device d_source ON dm.mapping_device_source_id = d_source.device_id
        INNER JOIN device d_target ON dm.mapping_device_target_id = d_target.device_id
        WHERE dm.mapping_type = 'Backup'
    )");

    // Add filter for specific mapping IDs if provided
    if (!mappingIds.isEmpty()) {
        querySQL += " AND dm.mapping_id IN (";
        for (int i = 0; i < mappingIds.size(); ++i) {
            if (i > 0) querySQL += ",";
            querySQL += QString::number(mappingIds[i]);
        }
        querySQL += ")";
    }

    querySQL += " ORDER BY dm.mapping_name ASC";

    if (!query.exec(querySQL)) {
        qDebug() << "ERROR: Failed to load backup tasks:" << query.lastError().text();
        return tasks;
    }

    while (query.next()) {
        BackupTask task;
        task.mappingId = query.value("mapping_id").toInt();
        task.name = query.value("mapping_name").toString();  // CORRECTED: Use mapping_name
        task.source = query.value("source_path").toString();
        task.destination = query.value("target_path").toString();
        task.sourceName = query.value("source_name").toString();
        task.targetName = query.value("target_name").toString();

        // Validate paths are not empty
        if (task.isValid()) {
            tasks.append(task);

            qDebug() << "Loaded backup task:"
                     << "ID=" << task.mappingId
                     << "Name=" << task.name
                     << "Source=" << task.source
                     << "Destination=" << task.destination;
        } else {
            qDebug() << "WARNING: Skipping invalid mapping ID" << task.mappingId
                     << "- missing source or destination path";
        }
    }

    qDebug() << "Total backup tasks loaded:" << tasks.size();
    return tasks;
}

//------------------------------------------------------------------------------
// Private Methods - Profile Content Generation
//------------------------------------------------------------------------------

QString BackupProfileGenerator::generateProfileContent(const QList<BackupTask>& tasks)
{
    QString content;

    // Warning header
    content += "***************************** WARNING *****************************\n";
    content += "Do NOT edit this file directly, unless you REALLY know what you are doing !!\n";
    content += "\n\n";

    // Profile header
    content += generateProfileHeader(tasks.size());
    content += "\n";

    // Generate each task section
    for (int i = 0; i < tasks.size(); ++i) {
        content += generateTaskSection(tasks[i], i);
        if (i < tasks.size() - 1) {
            content += "\n";
        }
    }

    // Profile end marker
    content += "\n[profile end]\n";

    return content;
}

QString BackupProfileGenerator::generateProfileHeader(int taskCount)
{
    QString userName = QDir::home().dirName();
    QString luckyBackupDir = getUserHomeDirectory() + "/.luckyBackup/";

    QString header;
    header += "[profile_global]\n";
    header += "appName=luckyBackup\n";
    header += "appVersion=0.5\n";
    header += QString("TotalTasks=%1\n").arg(taskCount);
    header += "\n";

    // Email configuration (default disabled)
    header += "[email]\n";
    header += "emailCommand=\n";
    header += "emailArguments=\n";
    header += "emailSubject=luckyBackup report\n";
    header += "emailNever=1\n";
    header += "emailError=0\n";
    header += "emailSchedule=0\n";
    header += "emailTLS=0\n";
    header += "emailFrom=\n";
    header += "emailTo=\n";
    header += "emailSMTP=\n";
    header += "emailBody=Profile:      %p\n";
    header += "emailBody=Date:         %d\n";
    header += "emailBody=Time:         %i\n";
    header += "emailBody=Errors found: %e\n";

    return header;
}

QString BackupProfileGenerator::generateTaskSection(const BackupTask& task, int taskIndex)
{
    QString userName = QDir::home().dirName();
    QString luckyBackupDir = getUserHomeDirectory() + "/.luckyBackup/";

    // Ensure destination ends with '/' for LuckyBackup compatibility
    QString destination = task.destination;
    if (!destination.endsWith('/')) {
        destination += '/';
    }

    QString section;
    section += QString("[Task] - %1\n").arg(taskIndex);
    section += QString("Name=%1\n").arg(task.name);
    section += "TypeDirContents=0\n";
    section += "TypeDirName=1\n";
    section += "TypeSync=0\n";
    section += QString("Source=%1\n").arg(task.source);
    section += QString("Destination=%1\n").arg(destination);
    section += "LastExecutionTime=\n";
    section += "LastExecutionErrors=-1\n";

    // rsync arguments
    section += "Args=-h\n";
    section += "Args=--progress\n";
    section += "Args=--stats\n";
    section += "Args=-r\n";
    section += "Args=-tgo\n";
    section += "Args=-p\n";
    section += "Args=-l\n";
    section += "Args=-D\n";
    section += "Args=--update\n";
    section += "Args=--delete-after\n";
    section += "Args=--filter=protect .luckybackup-snaphots/\n";
    section += QString("Args=%1\n").arg(task.source);
    section += QString("Args=%1\n").arg(destination);

    // Task options
    section += "ConnectRestore=\n";
    section += "KeepSnapshots=1\n";

    // Exclude options (all disabled)
    section += "Exclude=0\n";
    section += "ExcludeFromFile=0\n";
    section += "ExcludeFile=\n";
    section += "ExcludeTemp=0\n";
    section += "ExcludeCache=0\n";
    section += "ExcludeBackup=0\n";
    section += "ExcludeMount=0\n";
    section += "ExcludeLostFound=0\n";
    section += "ExcludeSystem=0\n";
    section += "ExcludeTrash=0\n";
    section += "ExcludeGVFS=0\n";

    // Include options (all disabled)
    section += "Include=0\n";
    section += "IncludeFromFile=0\n";
    section += "IncludeModeNormal=1\n";
    section += "IncludeFile=\n";

    // Remote options (all disabled)
    section += "Remote=0\n";
    section += "RemoteModule=0\n";
    section += "RemoteDestination=1\n";
    section += "RemoteSource=0\n";
    section += "RemoteSSH=0\n";
    section += "RemoteHost=\n";
    section += "RemoteUser=\n";
    section += "RemotePassword=\n";
    section += "RemoteSSHPassword=\n";
    section += "RemoteSSHPasswordStr=\n";
    section += "RemoteSSHOptions=\n";
    section += "RemoteSSHPort=0\n";

    // rsync options
    section += "OptionsUpdate=1\n";
    section += "OptionsDelete=1\n";
    section += "OptionsRecurse=1\n";
    section += "OptionsOwnership=1\n";
    section += "OptionsSymlinks=1\n";
    section += "OptionsPermissions=1\n";
    section += "OptionsDevices=1\n";
    section += "OptionsCVS=0\n";
    section += "OptionsHardLinks=0\n";
    section += "OptionsFATntfs=0\n";
    section += "OptionsSuper=0\n";
    section += "OptionsNumericIDs=0\n";
    section += "OptionsRestorent=0\n";
    section += "OptionsVss=0\n";

    // System paths
    section += QString("LuckyBackupDir=%1\n").arg(luckyBackupDir);
    section += "VshadowDir=/usr/bin\n";
    section += "RsyncCommand=rsync\n";
    section += "SshCommand=ssh\n";
    section += "DosdevCommand=/usr/bin/dosdev.exe\n";
    section += "CygpathCommand=/usr/bin/cygpath.exe\n";
    section += "TempPath=/tmp\n";

    // Warnings
    section += "ByPassWarning=0\n";
    section += "CloneWarning=1\n";
    section += "RepeatOnFail=0\n";
    section += "IncludeState=0\n";

    section += QString("[Task_end] - %1\n").arg(taskIndex);

    return section;
}

//------------------------------------------------------------------------------
// Private Methods - File Operations
//------------------------------------------------------------------------------

QString BackupProfileGenerator::generateProfileFilename()
{
    // Format: Katalog_YYYYMMDDHHmmss.profile
    QDateTime now = QDateTime::currentDateTime();
    QString timestamp = now.toString("yyyyMMddHHmmss");
    return QString("Katalog_%1.profile").arg(timestamp);
}

bool BackupProfileGenerator::writeProfileToFile(const QString& content,
                                                const QString& filepath)
{
    QFile file(filepath);

    if (!file.open(QIODevice::WriteOnly | QIODevice::Text)) {
        qDebug() << "ERROR: Cannot open file for writing:" << filepath;
        qDebug() << "Error:" << file.errorString();
        return false;
    }

    QTextStream out(&file);
    out << content;
    file.close();

    qDebug() << "Profile written successfully to:" << filepath;
    return true;
}

QString BackupProfileGenerator::getUserHomeDirectory()
{
    return QDir::homePath();
}
