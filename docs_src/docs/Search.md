---
version: "2.12"
---
# Search
![2.12](https://img.shields.io/badge/Version-2.12-blue)

## Summary
This page describes all the features of the **Search** screen and how to use them.

The main features of this screen are:

* Search for files from the Catalogs or from connected drives
* Use multiple **criteria** to refine and reduce the number of results:
  * filter by [text in file name or path](Search#search-text-criteria),
  * filter by [file attributes](Search#file-criteria) such as size, type, or date,
  * filter by [file metadata](Search#file-metadata-criteria) such as image dimensions or audio/video duration,
  * highlight [duplicated files](Search#duplicates-on),
  * or highlight [differences](Search#differences-on) between 2 catalogs.
* Use the **Results** with a [context menu](Search#file-context-menu) (right-click) or [process results in batch](Search#batch-process),
* Re-use previous criteria from the Search History.
* All search criteria are saved and available the next time Katalog is opened, except for the results themselves.

![Search screen showing criteria panel on the left and results on the right](/img/screen_search_01.png)

## Search source

### Search in file catalogs
The **[Selection](Selection)** panel on the left provides optional filters to narrow which catalogs to search in:
- *Location*: limits catalogs to those matching the selected location as defined in the [Storage](DevicesStorage) screen.
- *Storage*: limits catalogs to those under a specific storage device.
- *Catalog*: narrows the search to a single specific catalog.

### Search in connected drives
Search directly on the computer and connected drives without needing a catalog.
This is useful to search in a specific folder, or to get a quick view after recent changes without updating a catalog first.

## Search criteria
This panel groups all criteria that refine and reduce the number of results. The panel can be hidden to save space.

They apply to file paths and names, file attributes, metadata, tags, and can identify duplicates or differences between 2 catalogs.
Results can be a list of files, or a list of folders containing matching files.

![Search criteria panel showing text, file, metadata, duplicates and differences sections](/img/screen_search_02.png)

### Search Text criteria {#search-text-criteria}

#### File name
Type the text to search for in file names and/or folder paths.

A "word" is a group of characters separated from another group by a space. This can be used to find folders, files, or files in certain folders.

The text field accepts multiple lines. Each line is treated as an independent search term and results matching **any** line are returned (OR logic). This works with all *With* modes.

- Press **Enter** to trigger the search.
- Press **Shift+Enter** to insert a new line.

![Search text field showing multiple lines used as independent OR search terms](/img/screen_search_05_list_for_file_name.png)

Buttons next to the text field:
- *Paste from Clipboard* — pastes clipboard content into the search field.
- *Clean Search Text* — removes special characters (`.  ,  _  -  (  )  [  ]  {  }  /  \  '  "`).

When a checkbox is placed before a criterion, it can be enabled or disabled without losing the entered value.

#### With
Specifies how the words in the *Text* field should be matched:

| Option | Behavior |
|--------|----------|
| *All Words* | Returns results only if all words are found (default) |
| *Exact Phrase* | Returns results where the exact phrase (including word order and spaces) is found |
| *Begins With* | File name must start with the text — only available with *File names only* |
| *Any Word* | Returns results if at least one of the words is found |
| *Regex* | The search text is used as a regular expression pattern (PCRE2 syntax) |

:::note
When using *Regex*, entering an invalid pattern returns no results. The *Case sensitive* option still applies.
:::

#### In
Specifies which part of the file path to search:

| Option | Behavior |
|--------|----------|
| *File names only* | Searches only in file names (default) |
| *File names or Folder paths* | Searches in both file names and folder paths |
| *Folder path only* | Searches only in folder paths (not available with *Begins With*) |

#### Case sensitive
Forces exact character matching (upper- and lower-case letters are treated as distinct).

#### Exclude
Excludes results if *any* of the words provided is found in the file path or name.

### File criteria {#file-criteria}

The entire file criteria section can be enabled or disabled with its checkbox.

#### Size
Set a minimum and/or maximum file size. Each bound accepts a number and a unit.

Available units: **Bytes**, **KiB**, **MiB**, **GiB**, **TiB**

#### File type
Filter results by file type. Available types:

| Type | Description |
|------|-------------|
| All | No type filter (default) |
| Audio | Audio files (e.g. mp3, ogg, wav, flac…) |
| Image | Image files (e.g. jpg, png, gif, raw…) |
| Text | Document and text files (e.g. pdf, docx, odt, epub…) |
| Video | Video files (e.g. mp4, mkv, avi, mov…) |
| Other | Files not matching any of the above types |
| None | Files with no recognized type |

File types are detected dynamically using the system MIME type database (KFileMetadata), so the exact list of extensions adapts to the system.

#### Dates
Set a minimum and/or maximum file modification date.

### File metadata criteria {#file-metadata-criteria}

The metadata section allows searching on content embedded inside files (requires the catalog to have been indexed with extended metadata).

![File metadata criteria section showing text, image dimensions and duration fields](/img/screen_search_metadata_criteria.png)

The entire metadata section can be enabled or disabled with its checkbox.

#### Metadata text
Search for text inside metadata fields such as artist name, album, title, author, or other embedded text content.

#### Metadata size (images and videos)
Filter by pixel dimensions:
- *Min / Max Height* — height range in pixels
- *Min / Max Width* — width range in pixels

#### Metadata duration (audio and video)
Filter by playback duration using a time range.

### Folder criteria

#### Show folders only
Displays folder paths as results instead of individual files.

:::note
*Show folders only* cannot be combined with *Find Differences*.
:::

#### Tags {#tags-criteria}
Filter results to files or folders that have a specific tag assigned.
Tags are defined in the [Tags](Tags) screen.

### Duplicates {#duplicates-on}

Find files that are potentially duplicated, based on one or a combination of:

| Field | Description |
|-------|-------------|
| *File Name* | Same file name |
| *File Size* | Same file size |
| *Date Modified* | Same modification date |
| *Checksum (SHA-256)* | Same or different checksum value |

The **checksum operator** (= or ≠) lets you find files with the *same* checksum (true duplicates) or files with *different* checksums despite other matching attributes.

**Scope options:**
- *Within selected device/catalog* — finds duplicates inside the currently selected device or catalog.
- *Compare two devices* — compares files between two selected devices or catalogs to find duplicates across them.

:::note
At least one field must be selected to run a duplicates search.
:::

### Differences {#differences-on}

Find files present in one device but not the other, or with different attribute values — useful to compare a source with a backup.

![Differences criteria section showing device selectors and attribute checkboxes](/img/screen_search_03_diff.png)

Select two devices (Virtual, Storage, or Catalog) to compare, and choose which attributes define a difference:

| Field | Description |
|-------|-------------|
| *File Name* | Files with different names |
| *File Size* | Files with different sizes |
| *Date Modified* | Files with different modification dates |
| *Checksum (SHA-256)* | Files with same or different checksum |

The **checksum operator** (= or ≠) works the same way as for Duplicates.

:::note
At least one field must be selected. *Find Differences* cannot be combined with *Show Folders Only* or *Find Duplicates*.
:::

## Running a search

### Search button
Click *Search* to start. The button changes state depending on the operation:

| State | Button label | Stop button |
|-------|-------------|-------------|
| Idle | *Search* (green) | Disabled |
| Running | *Pause* | Enabled |
| Paused | *Resume* | Enabled |

![Status bar showing the paused state during a search operation](/img/screen_search_statusbar_paused.png)

- **Pause / Resume** — suspend and continue the search without losing progress. Not available in Memory database mode.
- **Stop** — cancel the current search entirely. Partial results already found are shown.

### Reset
The *Reset All* button resets all criteria to their default values.

## Results

### Catalogs with results
The left panel lists the catalogs in which matching files were found.
Clicking on a catalog in this list re-runs the search restricted to that catalog only.
This panel can be hidden to save space.

### Files found
The right panel lists the files or folders matching the search criteria.

The header shows the count of files or duplicates found and the total size.
Click the statistics icon to open the **detailed statistics dialog**:

| Statistic | Description |
|-----------|-------------|
| Files found | Total count of matching results |
| Files processed | Total number of files examined |
| Completion | Percentage processed (if the search was stopped early, results are flagged as incomplete) |
| Total size | Sum of file sizes |
| Min / Max / Average size | Size distribution |
| Min / Max date | Date range of results |
| Catalogs processed | Number of catalogs searched out of total |

### File context menu {#file-context-menu}

Right-clicking on a result line opens a context menu:

**Navigation:**
| Action | Description |
|--------|-------------|
| *Open file* | Opens the file with the system's default application |
| *Open folder* | Opens the file's parent folder in the file manager |
| *Explore folder* | Navigates to the file's folder in Katalog's [Explore](Explore) screen |

**Metadata:**
| Action | Description |
|--------|-------------|
| *Show extended metadata (JSON)* | Displays the file's embedded metadata (available if the catalog was indexed with extended metadata, or if the file type supports it) |

**Copy to clipboard:** *(example: `/home/user/documents/filename.txt`)*
| Action | Copied value |
|--------|-------------|
| *Copy folder path* | `/home/user/documents` |
| *Copy file absolute path* | `/home/user/documents/filename.txt` |
| *Copy file name with extension* | `filename.txt` |
| *Copy file name without extension* | `filename` |

**Checksum:**
| Action | Condition | Description |
|--------|-----------|-------------|
| *Calculate Checksum (SHA-256)* | No checksum stored yet | Computes and saves the SHA-256 hash |
| *Copy Checksum* | Checksum is stored | Copies the hash to the clipboard |
| *Verify Checksum (SHA-256)* | Checksum is stored | Recomputes and compares with the stored value |

**File operations:**
| Action | Description |
|--------|-------------|
| *Move to Trash* | Moves the file to the system trash (with confirmation) |
| *Delete file* | Permanently deletes the file (with confirmation) |

### Batch process {#batch-process}

The *Process Results* button opens a menu of batch operations applied to all files in the results:

| Action | Description |
|--------|-------------|
| *Export Results* | Export to a new catalog (for further refined searching) **or** to a CSV file named with the date and saved in the [Collection folder](Settings#database-memory-mode) |
| *Rename (KRename)* | Opens all result files in [KRename](https://apps.kde.org/krename/) for batch renaming |
| *Verify Checksums* | For each result file: if no checksum is stored yet, calculates and saves the SHA-256 hash; if a checksum is already stored, compares the actual value against the stored one (matched / mismatched). Progress is shown in the status bar. |
| *Include Metadata* | Extracts extended metadata (image dimensions, audio/video duration, etc.) for each result file and saves it to the catalog. Useful for files originally indexed without metadata. Progress is shown in the status bar. |
| ⚠ *Move to Trash* | Moves all result files to the system trash (with confirmation showing count and total size) |
| ⚠ *Delete* | Permanently deletes all result files (with confirmation — no recovery) |

## Search History
Every time a search is run, the criteria and device selection are saved to a history file in the Collection folder.

This history is displayed in a table at the bottom of the screen. Clicking on a row restores all criteria — the search must then be triggered manually.

Two buttons let you manage the history list:
- *Keep last 10* — removes all but the 10 most recent entries (with confirmation).
- *Reset* — deletes all history entries (with confirmation).

This panel can be hidden to save space.

![Search history panel showing previous searches with timestamps and criteria](/img/screen_search_04_search_history.png)

## Development
Some ideas of developments for this screen:
* See the backlog of [Search development](https://github.com/users/StephaneCouturier/projects/7/views/1?filterQuery=search).
