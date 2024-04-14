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
 * /////////////////////////////////////////////////////////////////////////////
// Application: Katalog
// File Name:   database.h
// Purpose:     database queries to create tables
// Description:
// Author:      Stephane Couturier
////////////////////////////////////////////////////////////////////////////////
*/
#ifndef DATABASE_H
#define DATABASE_H

#include <QtSql>

//CREATE TABLES -----------------------------------------------------------------

        // DEVICE  --------------------------------------------------------------

            const auto SQL_CREATE_DEVICE = QLatin1String(R"(
                        CREATE TABLE IF NOT EXISTS device(
                            device_id                  NUMERIC,
                            device_parent_id           NUMERIC,
                            device_name                TEXT,
                            device_type                TEXT,
                            device_external_id         NUMERIC,
                            device_path                TEXT,
                            device_total_file_size     NUMERIC default 0,
                            device_total_file_count    NUMERIC default 0,
                            device_total_space         NUMERIC default 0,
                            device_free_space          NUMERIC default 0,
                            device_active              NUMERIC,
                            device_group_id            NUMERIC,
                            device_date_updated        TEXT)
            )");

        // CATALOG --------------------------------------------------------------

            const auto SQL_CREATE_CATALOG = QLatin1String(R"(
                        CREATE TABLE IF NOT EXISTS catalog(
                            catalog_id                    NUMERIC,
                            catalog_file_path             TEXT,
                            catalog_name                  TEXT,
                            catalog_date_updated          TEXT,
                            catalog_source_path           TEXT,
                            catalog_file_count            NUMERIC,
                            catalog_total_file_size       NUMERIC,
                            catalog_source_path_is_active NUMERIC,
                            catalog_include_hidden        TEXT,
                            catalog_file_type             TEXT,
                            catalog_storage               TEXT,
                            catalog_include_symblinks     TEXT,
                            catalog_is_full_device        TEXT,
                            catalog_date_loaded           TEXT,
                            catalog_include_metadata      TEXT,
                            catalog_app_version           TEXT,
                            PRIMARY KEY("catalog_name"))
            )");

        // STORAGE --------------------------------------------------------------

            const auto SQL_CREATE_STORAGE = QLatin1String(R"(
                        CREATE TABLE IF NOT EXISTS storage(
                            storage_id            NUMERIC  primary key default 0,
                            storage_name          TEXT,
                            storage_type          TEXT,
                            storage_location      TEXT,
                            storage_path          TEXT,
                            storage_label         TEXT,
                            storage_file_system   TEXT,
                            storage_total_space   NUMERIC default 0,
                            storage_free_space    NUMERIC default 0,
                            storage_brand_model   TEXT,
                            storage_serial_number TEXT,
                            storage_build_date    TEXT,
                            storage_content_type  TEXT,
                            storage_container     TEXT,
                            storage_comment       TEXT,
                            storage_date_updated  TEXT)
            )");

        // DEVICE CATALOG ------------------------------------------------------

            const auto SQL_CREATE_DEVICE_CATALOG = QLatin1String(R"(
                        CREATE TABLE IF NOT EXISTS device_catalog(
                            device_id           NUMERIC,
                            catalog_name        TEXT,
                            directory_path      TEXT)
            )");

        // FILE (storing all catalogs files)-------------------------------------

            const auto SQL_CREATE_FILE = QLatin1String(R"(
                        CREATE TABLE IF NOT EXISTS file(
                            file_catalog_id   NUMERIC,
                            file_name         TEXT,
                            file_folder_path  TEXT,
                            file_size         NUMERIC,
                            file_date_updated TEXT,
                            file_catalog      TEXT,
                            file_full_path    TEXT)

            )");

        // FILETEMP (one-off requests) ------------------------------------------

            const auto SQL_CREATE_FILETEMP = QLatin1String(R"(
                        CREATE TABLE IF NOT EXISTS  filetemp(
                            file_catalog_id   NUMERIC,
                            file_name         TEXT,
                            file_folder_path  TEXT,
                            file_size         NUMERIC,
                            file_date_updated TEXT,
                            file_catalog      TEXT,
                            file_full_path    TEXT)
            )");

        // FOLDER ---------------------------------------------------------------

            const auto SQL_CREATE_FOLDER = QLatin1String(R"(
                        CREATE TABLE IF NOT EXISTS  folder(
                            folder_catalog_id  NUMERIC,
                            folder_path        TEXT,
                            PRIMARY KEY(folder_catalog_id,folder_path))
            )");

        // METADATA -------------------------------------------------------------

            const auto SQL_CREATE_METADATA = QLatin1String(R"(
                        CREATE TABLE IF NOT EXISTS  metadata(
                            catalog_name        TEXT,
                            file_name           TEXT,
                            file_path           TEXT,
                            field               TEXT,
                            value               TEXT)
            )");

        // STATISTICS -----------------------------------------------------------

            const auto SQL_CREATE_STATISTICS_DEVICE = QLatin1String(R"(
                        CREATE TABLE IF NOT EXISTS  statistics_device(
                            date_time               TEXT,
                            device_id               TEXT,
                            device_name             TEXT,
                            device_type             TEXT,
                            device_file_count       NUMERIC,
                            device_total_file_size  NUMERIC,
                            device_free_space       NUMERIC,
                            device_total_space      NUMERIC,
                            record_type             TEXT)
            )");

        // SEARCH ---------------------------------------------------------------

            const auto SQL_CREATE_SEARCH = QLatin1String(R"(
                        CREATE TABLE IF NOT EXISTS  search(
                            date_time                 TEXT,
                            text_checked              NUMERIC,
                            text_phrase               TEXT,
                            text_criteria             TEXT,
                            text_search_in            TEXT,
                            file_criteria_checked     NUMERIC,
                            file_type_checked         NUMERIC,
                            file_type                 TEXT,
                            file_size_checked         NUMERIC,
                            file_size_min             NUMERIC,
                            file_size_min_unit        NUMERIC,
                            file_size_max             NUMERIC,
                            file_size_max_unit        NUMERIC,
                            date_modified_checked     NUMERIC,
                            date_modified_min         TEXT,
                            date_modified_max         TEXT,
                            duplicates_checked        NUMERIC,
                            duplicates_name           NUMERIC,
                            duplicates_size           NUMERIC,
                            duplicates_date_modified  NUMERIC,
                            differences_checked       NUMERIC,
                            differences_name          NUMERIC,
                            differences_size          NUMERIC,
                            differences_date_modified NUMERIC,
                            differences_catalogs      TEXT,
                            folder_criteria_checked   NUMERIC,
                            show_folders              NUMERIC,
                            tag_checked               NUMERIC,
                            tag                       TEXT,
                            search_location           TEXT,
                            search_storage            TEXT,
                            search_catalog            TEXT,
                            search_catalog_checked    NUMERIC,
                            search_directory_checked  NUMERIC,
                            selected_directory        TEXT,
                            text_exclude              TEXT,
                            case_sensitive            NUMERIC)
            )");

        // TAG ------------------------------------------------------------------

            const auto SQL_CREATE_TAG = QLatin1String(R"(
                        CREATE TABLE IF NOT EXISTS  tag(
                            name		TEXT,
                            path		TEXT,
                            type		TEXT,
                            date_time	TEXT)
            )");

        // EXCLUDE --------------------------------------------------------------
            const auto SQL_CREATE_EXCLUDE = QLatin1String(R"(
                        CREATE TABLE IF NOT EXISTS parameter(
                                parameter_name      TEXT,
                                parameter_type      TEXT,
                                parameter_value1    TEXT,
                                parameter_value2    TEXT)
            )");

        // MIGRATION 1.22 to 2.0 ------------------------------------------------

            // STATISTICS -----------------------------------------------------------

            const auto SQL_CREATE_STATISTICS_CATALOG = QLatin1String(R"(
                        CREATE TABLE IF NOT EXISTS  statistics_catalog(
                            date_time               TEXT,
                            catalog_id              NUMERIC,
                            catalog_name            TEXT,
                            catalog_file_count      NUMERIC,
                            catalog_total_file_size NUMERIC,
                            record_type             TEXT)
            )");

            const auto SQL_CREATE_STATISTICS_STORAGE = QLatin1String(R"(
                        CREATE TABLE IF NOT EXISTS  statistics_storage(
                            date_time               TEXT,
                            storage_id              TEXT,
                            storage_name            TEXT,
                            storage_free_space      NUMERIC,
                            storage_total_space     NUMERIC,
                            record_type             TEXT)
            )");

            // VIRTUALSTORAGE  ------------------------------------------------------

            const auto SQL_CREATE_VIRTUAL_STORAGE = QLatin1String(R"(
                       CREATE TABLE IF NOT EXISTS virtual_storage(
                            virtual_storage_id          NUMERIC,
                            virtual_storage_parent_id   NUMERIC,
                            virtual_storage_name        TEXT)
            )");

            // VIRTUALSTORAGE CATALOG ------------------------------------------------------

            const auto SQL_CREATE_VIRTUAL_STORAGE_CATALOG = QLatin1String(R"(
                       CREATE TABLE IF NOT EXISTS virtual_storage_catalog(
                            virtual_storage_id      NUMERIC,
                            catalog_name            TEXT,
                            directory_path          TEXT)
            )");

//-------------------------------------------------------------------------------

#endif // DATABASE_H
