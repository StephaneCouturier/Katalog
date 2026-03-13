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
// File Name:   backupprofilegenerator.h
// Purpose:     Generate LuckyBackup profile files from Katalog mappings
// Description: Backend service class to create LuckyBackup-compatible
//              profile files from device_mapping table data
// Author:      Stephane Couturier
/////////////////////////////////////////////////////////////////////////////
*/

#ifndef BACKUPPROFILEGENERATOR_H
#define BACKUPPROFILEGENERATOR_H

#include <QObject>
#include <QString>
#include <QDateTime>
#include <QList>

/**
 * @brief Result structure for profile generation operations
 */
struct BackupProfileResult {
    bool success;
    QString profilePath;      // Full path to generated profile file
    QString profileName;      // Name of the profile (without path)
    QString errorMessage;     // Error description if success=false
    int taskCount;           // Number of tasks included in profile

    BackupProfileResult()
        : success(false), taskCount(0) {}
};

/**
 * @brief Represents a single backup task within a LuckyBackup profile
 */
struct BackupTask {
    QString name;            // Task name (e.g., "BackUpTask1")
    QString source;          // Source directory path
    QString destination;     // Destination directory path
    int mappingId;          // Reference to device_mapping.mapping_id
    QString sourceName;     // Friendly name of source device
    QString targetName;     // Friendly name of target device

    BackupTask() : mappingId(-1) {}

    bool isValid() const {
        return !name.isEmpty() && !source.isEmpty() && !destination.isEmpty();
    }
};

/**
 * @brief Backend service class to generate LuckyBackup profile files
 *
 * This class converts Katalog device mappings into LuckyBackup-compatible
 * profile files (.profile format). It queries the device_mapping table,
 * retrieves source/target device paths, and generates the profile text
 * following LuckyBackup's format specification.
 *
 * Usage:
 * @code
 * BackupProfileGenerator generator("connectionName");
 * BackupProfileResult result = generator.generateProfile();
 * if (result.success) {
 *     qDebug() << "Profile created:" << result.profilePath;
 * }
 * @endcode
 */
class BackupProfileGenerator : public QObject
{
    Q_OBJECT

public:
    /**
     * @brief Construct a new Backup Profile Generator
     * @param connectionName Database connection name
     * @param parent Parent QObject
     */
    explicit BackupProfileGenerator(const QString& connectionName,
                                    QObject *parent = nullptr);

    /**
     * @brief Generate LuckyBackup profile from all active device mappings
     *
     * Creates a profile file named "Katalog_<timestamp>.profile" in the
     * default LuckyBackup profiles directory (~/.luckyBackup/profiles/).
     *
     * The profile includes all mappings from the device_mapping table where
     * mapping_type = 'Backup'.
     *
     * @return BackupProfileResult Structure containing success status,
     *         profile path, and error messages if any
     */
    BackupProfileResult generateProfile();

    /**
     * @brief Generate LuckyBackup profile from specific mapping IDs
     * @param mappingIds List of device_mapping IDs to include
     * @return BackupProfileResult
     */
    BackupProfileResult generateProfile(const QList<int>& mappingIds);

    /**
     * @brief Get the default LuckyBackup profiles directory
     * @return QString Path to ~/.luckyBackup/profiles/
     */
    static QString getDefaultProfilesDirectory();

    /**
     * @brief Check if LuckyBackup profiles directory exists and is writable
     * @return bool True if directory exists and is writable
     */
    static bool isProfilesDirectoryAccessible();

    /**
     * @brief Create LuckyBackup profiles directory if it doesn't exist
     * @return bool True if directory exists or was created successfully
     */
    static bool ensureProfilesDirectoryExists();

signals:
    /**
     * @brief Emitted when profile generation starts
     */
    void profileGenerationStarted();

    /**
     * @brief Emitted during profile generation with progress updates
     * @param current Current task number being processed
     * @param total Total number of tasks to process
     */
    void profileGenerationProgress(int current, int total);

    /**
     * @brief Emitted when profile generation completes successfully
     * @param profilePath Full path to the generated profile file
     * @param taskCount Number of tasks included in the profile
     */
    void profileGenerationCompleted(const QString& profilePath, int taskCount);

    /**
     * @brief Emitted when profile generation fails
     * @param errorMessage Description of the error
     */
    void profileGenerationFailed(const QString& errorMessage);

private:
    /**
     * @brief Load backup tasks from device_mapping table
     * @param mappingIds Optional list of specific mapping IDs (empty = all)
     * @return QList<BackupTask> List of backup tasks loaded from database
     */
    QList<BackupTask> loadBackupTasks(const QList<int>& mappingIds = QList<int>());

    /**
     * @brief Generate profile file content from tasks
     * @param tasks List of backup tasks to include
     * @return QString Complete profile file content in LuckyBackup format
     */
    QString generateProfileContent(const QList<BackupTask>& tasks);

    /**
     * @brief Generate the global profile header section
     * @param taskCount Total number of tasks in the profile
     * @return QString Profile header section text
     */
    QString generateProfileHeader(int taskCount);

    /**
     * @brief Generate a single task section for the profile
     * @param task Task data
     * @param taskIndex Zero-based task index in the profile
     * @return QString Task section text
     */
    QString generateTaskSection(const BackupTask& task, int taskIndex);

    /**
     * @brief Generate profile filename with timestamp
     * @return QString Filename in format "Katalog_<timestamp>.profile"
     */
    QString generateProfileFilename();

    /**
     * @brief Write profile content to file
     * @param content Profile file content
     * @param filepath Full path where to write the file
     * @return bool True if file was written successfully
     */
    bool writeProfileToFile(const QString& content, const QString& filepath);

    /**
     * @brief Get current user's home directory
     * @return QString Home directory path
     */
    static QString getUserHomeDirectory();

    // Member variables
    QString m_connectionName;  ///< Database connection name
};

#endif // BACKUPPROFILEGENERATOR_H
