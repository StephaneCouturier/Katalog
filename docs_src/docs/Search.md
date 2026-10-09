---
version: "3.0"
---
# Search
![3.0](https://img.shields.io/badge/Version-3.0-blue)

## Summary
This page describes the **Search** page: the search criteria, running a search, and the search history.

* Search for files in the **catalogs** of the selected device, or directly in a **connected drive**.
* Combine **criteria** to narrow the results:
  * the [file name](#search-text-criteria), with one or several search texts,
  * the [file attributes](#file-criteria): type, size and date,
  * the [file metadata](#file-metadata-criteria): text, image or video dimensions, duration,
  * the [folder criteria](#folder-criteria): list folders only, or only tagged folders,
  * find [duplicates](#duplicates-on), or [differences](#differences-on) between 2 devices.
* Re-use the criteria of a previous search from the [history](#search-history).

The results open in the **Results** page, to the right of the Search page: see [Search results](SearchResults).

![Search page with "sunset" in the file name criteria, next to the Results page](/img/search_01_overview.png)

## Where to search {#search-source}

| Option | Description |
|--------|-------------|
| *Catalogs* | Searches in the catalogs of the device selected in the [Selection](Selection): a catalog, all the catalogs of a storage device, or of a virtual device. With no device selected (*All*), searches in all catalogs. |
| *Connected drive* | Searches directly in a folder of the computer or of a connected drive, without a catalog. Type the path, or use *Select the path*. Useful to search in a folder that has no catalog, or to see recent changes without updating a catalog first. |

## Search criteria
The criteria are grouped in sections. The checkbox in front of a section title turns the whole section on or off **without losing what was entered** in it. The same applies to the checkboxes in front of a single criterion (*Type*, *Size*, *Date*...).

### File name {#search-text-criteria}

![File name criteria with two search texts and one exclude text](/img/search_02_file_name.png)

#### text
The text to look for. It can be **one or several search texts**, one per row:
* *+* adds a row, *−* removes it (the last row cannot be removed),
* a file is found when it matches **any** of the rows,
* the cross in a row empties it.

Buttons next to the rows:
* *Paste the text from the clipboard*: replaces the rows with the content of the clipboard, one row per line. Useful to search for a list copied from a spreadsheet, a text editor or a file manager.
* *Clean the search Text from characters such as _ - . ,*: replaces these characters with spaces in every row.

Pasting several lines with `Ctrl+V` into a row adds them as new rows. `Enter` in any row runs the search.

#### with
How the words of each row are matched:

| Option | Behavior |
|--------|----------|
| *All Words* | All the words of the row are found, in any order (default). |
| *Exact Phrase* | The row is found as typed, words in the same order. |
| *Begins With* | The file name starts with the row. Only with *File names only*. |
| *Any Word* | At least one word of the row is found. |
| *Regex* | The row is a regular expression (PCRE2 syntax). An invalid expression finds nothing. |

A "word" is a group of characters separated from another by a space.

#### in
Where the text is looked for:

| Option | Behavior |
|--------|----------|
| *File names only* | In the file names (default). |
| *File names or Folder paths* | In the file names and in the folder paths. |
| *Folder path only* | In the folder paths only. |

#### case sensitive
Upper- and lower-case letters are treated as different.

#### exclude
Files are left out of the results when they match **any** of the exclude rows. The exclude rows work like the search rows: *+*, *−*, paste and clean.

### File attributes {#file-criteria}

#### Type
Only files of one type. When *Type* is not checked, all types are searched.

| Type | Description |
|------|-------------|
| *All* | No type filter. |
| *Audio* | Audio files (mp3, ogg, wav, flac...). |
| *Image* | Image files (jpg, png, gif, raw...). |
| *Text* | Documents and text files (pdf, docx, odt, epub...). |
| *Video* | Video files (mp4, mkv, avi, mov...). |
| *Other* | Files of none of the types above. |
| *None* | Files with no recognized type. |

File types are recognized from the system's file type database, so the exact list of extensions follows the system.

#### Size
A minimum and a maximum size, each with its unit: Bytes, KiB, MiB, GiB or TiB.

#### Date
A minimum and a maximum modification date. *Select a date* opens a calendar, with shortcuts: *Now*, *1 day ago*, *1 week ago*, *1 month ago*, *1 year ago*.

### File metadata {#file-metadata-criteria}
Criteria on information stored inside the files. They need catalogs created or updated with the *Include metadata* option (see [Create](Create)).

| Criterion | Description |
|-----------|-------------|
| *Text* | Text in the metadata, such as an artist, album, title or author. |
| *Size* | *Width* and *Height* in pixels, each with a minimum and a maximum (images and videos). |
| *Duration* | Minimum and maximum duration, as hours:minutes:seconds (audio and video). |

### Folder criteria {#folder-criteria}

#### only list folders in results
The results are the folders that contain matching files, instead of the files.

#### Tag {#tags-criteria}
Only files in the folders that have this tag. Tags are assigned in the [Tags](Tags) page.

### Duplicates {#duplicates-on}
Finds files that are probably the same file in several places.

*On*: what two files must have in common, one or several of:

| Field | Description |
|-------|-------------|
| *Name* | Same file name. |
| *Size* | Same size. |
| *Date* | Same modification date. |
| *Checksum* | With `=`, same checksum: the content is identical. With `≠`, different checksum: same name or size, but a different content. |

Checksums are available only for catalogs created or updated with the *Include checksum* option (see [Create](Create)).

*Scope*:
* *Within selected device*: duplicates inside the selected device,
* *Compare two devices*: duplicates between *Device 1* and *Device 2*, for example a computer and an old laptop.

### Differences {#differences-on}
Compares two devices, *Between* and *And*, for example a folder and its backup. Finds the files present in only one of them, or present in both but with a different value on the fields chosen in *On*: *Name*, *Size*, *Date*, *Checksum* (`=` or `≠`, as for Duplicates).

Differences is available only when searching in *Catalogs*.

:::note
*Duplicates* and *Differences* cannot be used at the same time.
:::

## Running a search {#running}
The buttons at the top of the page:

| Button | Action |
|--------|--------|
| *Search* | Runs the search. `Enter` in a text field does the same. |
| *Pause* / *Resume* | Pauses the running search and resumes it. |
| *Stop* | Stops the running search. The results found so far are kept. |
| *Reset* | Puts all criteria back to their default values. |
| *History* | Opens the [search history](#search-history). |
| *Close* | Closes the Search page. |

The progress of the search is shown in the [activity panel](Interface#activity-panel).<br/>
A search cannot start while a catalog is being created or a device updated: *Search* is disabled in the meantime.

## Search history {#search-history}

![Search history: past searches with their date, device and criteria](/img/search_03_history.png)

Every search is saved in the history of the collection, with its criteria and the device it was run on.

*History* opens the list of past searches. Each line shows the date, the device searched (or *All*), and a summary of the criteria.<br/>
Clicking a line **restores all its criteria and the selected device**. Click *Search* to run it again.

| Button | Action |
|--------|--------|
| *Keep Last 10* | Deletes all but the 10 most recent searches, after confirmation. |
| *Clear* | Deletes the whole history, after confirmation. |
| *Cancel* | Closes the history. |

## Development
* See the backlog of [Search development](https://github.com/users/StephaneCouturier/projects/7/views/1?filterQuery=search).
