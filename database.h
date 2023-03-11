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
// Purpose:     database queries
// Description:
// Author:      Stephane Couturier
////////////////////////////////////////////////////////////////////////////////
*/
#ifndef DATABASE_H
#define DATABASE_H

#include <QtSql>

//CREATE TABLES -----------------------------------------------------------------

        // CATALOG --------------------------------------------------------------

            const auto SQL_CREATE_CATALOG = QLatin1String(R"(
                       CREATE TABLE IF NOT EXISTS catalog(
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
                            PRIMARY KEY("catalog_name"))
            )");

        // STORAGE --------------------------------------------------------------

            const auto SQL_CREATE_STORAGE = QLatin1String(R"(
                       CREATE TABLE IF NOT EXISTS storage(
                            storage_id  int  primary key default 0,
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
                            storage_comment       TEXT)
            )");

        // FILESALL (storing all catalogs files)---------------------------------

            const auto SQL_CREATE_FILESALL = QLatin1String(R"(
                       CREATE TABLE IF NOT EXISTS filesall(
                            id_file           INTEGER,
                            file_name         TEXT,
                            file_path         TEXT,
                            file_size         NUMERIC,
                            file_date_updated TEXT,
                            file_catalog      TEXT,
                            file_full_path    TEXT,
                            PRIMARY KEY("id_file" AUTOINCREMENT))
                )");

        // FILE (one-off requests) ----------------------------------------------

            const auto SQL_CREATE_FILE = QLatin1String(R"(
                   CREATE TABLE IF NOT EXISTS  file(
                            id_file              INTEGER,
                            file_name            TEXT,
                            file_path            TEXT,
                            file_size            NUMERIC,
                            file_date_updated    TEXT,
                            file_catalog         TEXT,
                            file_full_path       TEXT,
                            PRIMARY KEY("id_file" AUTOINCREMENT))
            )");

        // FOLDER ---------------------------------------------------------------

            const auto SQL_CREATE_FOLDER = QLatin1String(R"(
                   CREATE TABLE IF NOT EXISTS  folder(
                            folder_hash          TEXT,
                            folder_catalog_name  TEXT,
                            folder_path          TEXT,
                            PRIMARY KEY(folder_hash,folder_catalog_name))
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

            const auto SQL_CREATE_STATISTICS = QLatin1String(R"(
                   CREATE TABLE IF NOT EXISTS  statistics(
                            date_time               TEXT,
                            catalog_name            TEXT,
                            catalog_file_count      NUMERIC,
                            catalog_total_file_size NUMERIC,
                            record_type             TEXT)
            )");

        // SEARCH ---------------------------------------------------------------

            const auto SQL_CREATE_SEARCH = QLatin1String(R"(
                   CREATE TABLE IF NOT EXISTS  search(
                            date_time                 TEXT,
                            text_checked              INTEGER,
                            text_phrase               TEXT,
                            text_criteria             TEXT,
                            text_search_in            TEXT,
                            file_type                 TEXT,
                            file_size_checked         INTEGER,
                            file_size_min             INTEGER,
                            file_size_min_unit        INTEGER,
                            file_size_max             INTEGER,
                            file_size_max_unit        INTEGER,
                            date_modified_checked     INTEGER,
                            date_modified_min         TEXT,
                            date_modified_max         TEXT,
                            duplicates_checked        INTEGER,
                            duplicates_name           INTEGER,
                            duplicates_size           INTEGER,
                            duplicates_date_modified  INTEGER,
                            differences_checked       INTEGER,
                            differences_name          INTEGER,
                            differences_size          INTEGER,
                            differences_date_modified INTEGER,
                            differences_catalogs      TEXT,
                            show_folders              INTEGER,
                            tag_checked               INTEGER,
                            tag                       TEXT,
                            search_location           TEXT,
                            search_storage            TEXT,
                            search_catalog            TEXT,
                            search_catalog_checked    INTEGER,
                            search_directory_checked  INTEGER,
                            selected_directory        TEXT,
                            text_exclude              TEXT,
                            case_sensitive            INTEGER)
            )");

        // TAG ------------------------------------------------------------------

            const auto SQL_CREATE_TAG = QLatin1String(R"(
                       CREATE TABLE IF NOT EXISTS  tag(
                            name		TEXT,
                            path		TEXT,
                            type		TEXT,
                            date_time	TEXT)
            )");

//-------------------------------------------------------------------------------


#endif // DATABASE_H
