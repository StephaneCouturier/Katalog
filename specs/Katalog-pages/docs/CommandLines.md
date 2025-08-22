# Command Lines
## Summary
This page describes available command lines features and options that can be run from console.
Without the need of the graphical interface, some tasks can therefore be automated.<br/>
These commands currently run only under <b>Linux</b>.<br/>

## Synopsis

```bash
./Katalog.sh [ACTION] [OPTIONS] [ARGUMENTS]
```
## Actions

The following actions are available as positional arguments:

### `list_catalogs`
Lists all catalogs with their ID, active state, and name.

**Usage:**
```bash
./Katalog.sh list_catalogs [OPTIONS]
```

**Example:**
```bash
./Katalog.sh list_catalogs --verbose
```

![](/img/commandlines_02_list.png)

### `update_catalog`
Updates a specific catalog by device ID.

**Usage:**
```bash
./Katalog.sh update_catalog <deviceID> [OPTIONS]
```

**Arguments:**
- `deviceID` - The device ID of the catalog to update

**Example:**
```bash
./Katalog.sh update_catalog 5 --report
```
![](/img/commandlines_03_catalog.png)

### `update_all_active`
Updates all active catalogs in the collection.

**Usage:**
```bash
./Katalog.sh update_all_active [OPTIONS]
```

**Example:**
```bash
./Katalog.sh update_all_active --report --verbose
```
![](/img/commandlines_04_all.png)

### `search`
Executes a search using the last search criteria from history, with optional overrides.

**Usage:**
```bash
./Katalog.sh search [OPTIONS]
```

**Example:**
```bash
./Katalog.sh search --text "vacation" --type image --limit 100
```

### `restart`
Restarts the application (used internally for application lifecycle management).

**Usage:**
```bash
./Katalog.sh restart
```

## General Options

### `-h, --help`
Displays help information and exits.

### `-v, --version`
Displays version information and exits.

### `-c, --collection <path>`
Specifies the path to the collection folder.

**Example:**
```bash
./Katalog.sh search --collection "/path/to/my/collection"
```

### `-r, --report`
Shows detailed report for update operations in a graphical window (usual message box in the interface).

**Example:**
```bash
./Katalog.sh update_catalog 3 --report
```

> ./Katalog update_catalog 4 --report

This will display:
![](/img/commandlines_05_report.png)


### `--verbose`
Enables verbose output for debugging and detailed information.

**Example:**
```bash
./Katalog.sh list_catalogs --verbose
```

## Search Options

These options are available when using the `search` action to override search criteria:

### `--limit <number>`
Limits the number of files to display in search results.

**Example:**
```bash
./Katalog.sh search --limit 50
```

### `--selectedDeviceID <deviceID>`
Specifies which device ID to search in. 
- Default: uses settings file value
- When used with `--collection`: defaults to 0 (All devices)

**Example:**
```bash
./Katalog.sh search --selectedDeviceID 2
```

### `--text <search-term>`
Specifies the search text or phrase to look for.

**Example:**
```bash
./Katalog.sh search --text "family photos"
```

### `--type <file-type>`
Filters results by file type.

**Valid values:**
- `all` (default)
- `audio`
- `image` 
- `text`
- `video`

**Example:**
```bash
./Katalog.sh search --type audio
```

### `--size-min <size>`
Sets minimum file size filter.

**Format:** Number followed by unit (e.g., 1MB, 5GB)

**Example:**
```bash
./Katalog.sh search --size-min 1MB
```

### `--size-max <size>`
Sets maximum file size filter.

**Format:** Number followed by unit (e.g., 100MB, 2GB)

**Example:**
```bash
./Katalog.sh search --size-max 100MB
```

### `--date-after <date>`
Filters files modified after the specified date.

**Format:** YYYY-MM-DD

**Example:**
```bash
./Katalog.sh search --date-after 2023-01-01
```

### `--date-before <date>`
Filters files modified before the specified date.

**Format:** YYYY-MM-DD

**Example:**
```bash
./Katalog.sh search --date-before 2023-12-31
```

### `--case-sensitive`
Enables case-sensitive text search.

**Example:**
```bash
./Katalog.sh search --text "MyFile" --case-sensitive
```

### `--search-in <scope>`
Defines the search scope for text matching.

**Valid values:**
- `filenames` (default)
- `files-and-folders`
- `folder-paths`

**Example:**
```bash
./Katalog.sh search --text "documents" --search-in folder-paths
```

### `--text-criteria <criteria>`
Specifies how the search text should be matched.

**Valid values:**
- `all-words` (default)
- `exact-phrase`
- `begins-with`
- `any-word`

**Example:**
```bash
./Katalog.sh search --text "vacation photo" --text-criteria exact-phrase
```

### `--exclude <exclude-terms>`
Excludes files containing the specified terms.

**Example:**
```bash
./Katalog.sh search --text "photo" --exclude "backup temp"
```

### `--no-history`
Starts with default search criteria instead of loading from search history.

**Example:**
```bash
./Katalog.sh search --no-history --text "newfile"
```

## CSV Export Options

### `--csv`
Exports search results to a CSV file. The file is saved in the collection folder with a timestamp.

**Example:**
```bash
./Katalog.sh search --text "documents" --csv
```

## Exit Codes

- **0**: Success
- **1**: Error or failure
- **-1**: Internal code to continue to GUI mode (not returned to user)

## Examples

### Basic Catalog Management

List all catalogs:
```bash
./Katalog.sh list_catalogs
```

Update a specific catalog with detailed report:
```bash
./Katalog.sh update_catalog 3 --report --verbose
```

Update all active catalogs:
```bash
./Katalog.sh update_all_active --report
```

### Search Examples

Simple text search:
```bash
./Katalog.sh search --text "vacation"
```

Search for images larger than 5MB:
```bash
./Katalog.sh search --type image --size-min 5MB
```

Search for files modified in 2023:
```bash
./Katalog.sh search --date-after 2023-01-01 --date-before 2023-12-31
```

Complex search with multiple criteria:
```bash
./Katalog.sh search --text "project" --type text --search-in files-and-folders --case-sensitive --limit 200
```

Search in specific device and export to CSV:
```bash
./Katalog.sh search --selectedDeviceID 2 --text "documents" --csv --verbose
```

### Using Custom Collection Path

Search in a different collection:
```bash
./Katalog.sh --collection "/path/to/collection" search --text "photos"
```

Update catalogs in a specific collection:
```bash
./Katalog.sh --collection "/path/to/collection" update_all_active --report
```

## Notes

- When no action is specified, Katalog launches in GUI mode
- Search criteria are loaded from search history by default, unless `--no-history` is used
- Command line options override values from search history
- The `restart` action is primarily used internally for application lifecycle management
- All search operations respect the active state of catalogs
- File size units supported: KB, MB, GB, TB (case-insensitive)
- Date formats must be in YYYY-MM-DD format
- CSV exports are automatically timestamped and saved in the collection folder

## Troubleshooting

**Invalid device ID:**
Ensure the device ID exists by running `katalog list_catalogs` first.

**Database connection issues:**
Verify the collection path is correct and accessible.

**Search returns no results:**
Try using `--no-history` to start with default criteria, or check if the selected device contains indexed files.

**Permission errors:**
Ensure Katalog has read/write access to the collection folder and database files.

