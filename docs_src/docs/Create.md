---
version: "2.11"
---
# Create
![2.11](https://img.shields.io/badge/Version-2.11-blue)

## Summary
This page describes all the features of the **Create** screen and how to use them.<br/>
From this screen, the user can **create a catalog of files**.<br/>
It is done in 3 main steps:
1. Select the Source path: the device or directory with the files to be included in the new catalog.
1. Select options to include or exclude some particular files.
1. Select the [Storage](DevicesStorage) and define the catalog name, and create the catalog.

![Create screen overview](/img/screen_create_01.png)

## Select the Source path
There are 3 ways to  select the source path of the directory with the files to be included in the new [Catalog](DevicesCatalogs):
1. by typing the path in the text edit zone
1. by using the treeview of the file system, just expand and click on the right device or directory
1. or by clicking the button *Select* which will open a dialog window to help selecting the folder.

The selected path will always appear in the text edit zone, and the application will use this path to browse and catalog its contents.

## Select options to include/exclude files
Choose which types of files to include in your catalog. 

### Enhanced File Type Filtering
Katalog now uses intelligent file type detection based on the extension that supports hundreds of file formats automatically.

**How file type detection works:**
- **Initial detection**: File types are determined by analyzing file extensions using an intelligent extension-to-type cache built from your system's MIME database
- **Comprehensive support**: Supports hundreds of file formats (compared to previously limited hardcoded lists)
- **Future-proof**: New file formats are automatically recognized as your system is updated

**Accuracy note**: 
- File types are initially determined from the extension analysis during catalog creation. This is to maximize indexing performance.
- But there can be extension mistake or missing extensions
- To maximize accuracy, it is possible to run MIME verification on completed catalogs via the Devices page, and correct any files with misleading extensions.


### File Type categories
The contents of the catalog can be limited to a particular type of files.
This option will be applicable for the catalog moving forward. It can be changed later by editing the [Catalog](DevicesCatalogs).
       
        | Type  | Description |  Definition   | Extensions examples                               |
        | ------| -------------|-------------|-----------------------------------------------------|
        | All   | All file types without filtering | |     |
        | Audio | Music, podcasts, audio recordings, and sound files  |(as defined by MIME types) | MP3, FLAC, AAC, M4A, OGG, WAV, AIFF, Opus, WMA, MIDI, AMR (50+ formats)                               |
        | Image | Photos, graphics, diagrams, icons, and visual content  |(as defined by MIME types)| JPG, PNG, HEIC, WebP, TIFF, RAW, SVG, XCF, GIF, BMP (100+ formats)                  |
        | Text  | Documents, code files, markup, data files, and readable content. | (specific Katalog definition)<br/>Includes all files with MIME type starting with "text/" plus specific application files like PDF, Word documents, etc.| PDF, DOCX, ODT, Markdown, HTML, JSON, source code files, ebooks (100+ formats).     |
        | Video | Movies, clips, animations, and video content |  (as defined by MIME types) | MP4, MKV, AVI, WebM, MOV, FLV, 3GP, OGV, M2TS (40+ formats) |
        | Other | <br/>All other file types not covered by the above categories | (specific Katalog definition) | ZIP, RAR, EXE, DLL, ISO, application files that are not classified as Text. This includes executables, archives, and system files |
        | None  | Files for which the type could not be determined based on the extension | Files without extensions or with unknown extensions |  |

                            
### Metadata Extraction
Choose how much metadata to extract from your files during catalog creation.
<br/>This affects cataloging speed or collection size, but provides richer file information for searches & statistics.

**Available options:**
- **None**: No metadata extraction (fastest cataloging)
- **Media Basic**: Extract essential metadata from images, videos, and audio files
- **Media Extended**: Extract comprehensive metadata including technical details  
- **Full Extended**: Maximum metadata extraction for all supported file types

**What Media Basic is extracted:**
- **Audio**: Artist, album, track details, duration, bitrate
- **Images**: Dimensions, orientation
- **Videos**: Dimensions, Duration, codec & framerate

**What Extended metadata is extracted:**
This mechanism is built on the KFileMetaData library, which determines the file types supported and metadata.
The extracted metadata is stored as a JSON in the database.

**Performance impact:**
- **None**: Fastest option, suitable for large directories or when metadata isn't needed
- **Media Basic/Extended**: Moderate impact, processes only media files (images, videos, audio)
- **Full Extended**: Slower but most comprehensive, extracts from all supported file formats

**Supported file types for metadata:**
- **Images**: jpg, png, gif, bmp, tiff, webp, svg, heic, raw, xcf
- **Videos**: mp4, mkv, avi, mov, wmv, flv, webm, m4v, mpg, 3gp, ogv, vob
- **Audio**: mp3, wav, flac, ogg, m4a, aac, wma, opus, aiff, mid, amr

This setting applies only to this catalog and can be changed later by editing the [Catalog](DevicesCatalogs).

**Note:** Metadata extraction requires readable files. Files that are corrupted or have access restrictions will be skipped without affecting the cataloging process.

### File checksum
Checksum SH256 can be calculated during indexing for Duplicate/Difference Search.
⚠️ It is a much longer process than other indexing option as it reads ALL of the data to compute the checksums.
Like file metadata, the option can be selected at catalog creation or changed later, and when interrupting the process, the calculated checksum are saved and the next update will resume for remaining files.
The files checkum can be used as a duplicate search option or as a difference search option.

### Include Hidden files {#other-options}
Hidden files are not included by default, but this options enables to include them.<br/>
This option will be applicable for the catalog moving forward.<br/>
It can be changed later by editing the [Catalog](DevicesCatalogs).

### Global Parameters panel

The *Global Parameters* panel groups settings that apply across all catalogs. It can be collapsed or expanded using the toggle button at the top of the panel.

### Exclude directories (global) {#exclude-directories}

:::note
These exclusions are **global**: they apply to **all** catalogs, both when creating new catalogs and when updating existing ones.
:::

It is possible to exclude directories from being cataloged, both during catalog creation and updates.<br/>
Enter a path or text pattern, then click *Add*.<br/>
The entry is then visible in the list below.<br/>
Any entry can be removed by right-clicking on it and selecting *Remove*.<br/>

**How exclusion works:**

The exclusion uses **text matching**: any file or folder whose full path **contains** the exclusion text will be skipped. This means:

- **Full path**: entering `/home/user/Downloads/temp` will exclude that specific directory and all its contents.
- **Folder name**: entering `node_modules` will exclude **every** `node_modules` directory across all catalogs (e.g. `/project1/node_modules/...`, `/project2/node_modules/...`).
- **Partial path**: entering `.cache` will exclude directories like `/home/user/.cache/` but also `/home/user/.cachedata/` since the match is based on text containment.

The matching is **case-sensitive**.

![Global exclude directories list showing example entries](/img/screen_create_04_exclude.png)

### Exclude folders (per catalog)

In addition to global exclusions, it is possible to define excluded folders that apply only to the catalog being created.

- Enter a folder path manually, or browse to it using the *Pick* button.
- Click *Add* to add it to the pending list.
- Remove any entry by right-clicking on it and selecting *Remove*.

The per-catalog exclusions are saved with the catalog once creation completes. They are applied in addition to global exclusions — a folder skipped by either rule will not be indexed.

This option can be changed later by editing the [Catalog](DevicesCatalogs).

## Define & Create the catalog
#### Select the Storage device
A Catalog shall be associated with a [Storage](DevicesStorage) physical device, to facilitate later search or enable statistics.<br/>
By default, Katalog pre-creates a default Storage device, the local disk.<br/>
This can be updated later in the [Devices](DevicesTree) virtual tree screen.<br/>
If you need a different and new Storage for this catalog, click *Add Storage*, and add one using the [Devices](DevicesTree) or the [Storage](DevicesStorage) screens.

This choice will be applicable for the catalog moving forward.<br/>
It can be changed later by editing the [Catalog](DevicesCatalogs).

#### Enter a Name
Enter a name for your catalog.<br/>
Duplicate names are currently not allowed.

The button *Generate* can create a name based on the folder path, replacing slashes <code>/</code> by underscore <code>_</code>.

#### Create the catalog
When ready, click the button *Create Catalog* to save the catalog itself and start the process of cataloging the contents of the path recursively (all sub-directories will be included).

Once the process is completed,
- A message confirms the creation and provides the number of files and total file size of the selected folder for this catalog.
- your local drive, (a Storage device which was added automatically) was also updated, and the message provides a view of free, used, and total space:

![Catalog creation confirmation with file count and storage space summary](/img/screen_create_02.png)

the [Devices](DevicesTree) screen will be display to show the Catalog in the device tree.

The new catalog is automatically selected in the [Selection](Selection) panel, ready to be used to [Search](Search) for the contents.

## Performance Guide

### What Affects Scanning Speed?


| # | Factor | Impact Level | What drives it | Notes |
|---|--------|:---:|---|---|
| 1 | **File Checksum** | ⚫⚫⚫⚫⚫ Critical | **Total data volume** (reads 100% of bytes, 8 MB buffer), then disk speed, then file count | SHA-256 via Qt `QCryptographicHash`. A 2 TB HDD at ~100 MB/s = ~5.5 hours just for I/O. SSD at ~500 MB/s ≈ 1 hour. Resumable if interrupted. |
| 2 | **Storage hardware** | ⚫⚫⚫⚫ Very High | Sequential vs random I/O, throughput, latency | SSD ~100K files/min, HDD ~20–30K (fragmentation makes it worse). Network: highly variable. Affects ALL other factors. |
| 3 | **Metadata: Full Extended** | ⚫⚫⚫ High | **File count** (especially video: 5–15 ms/file parsing containers), file type distribution | Uses KFileMetaData. Video/audio require seeking into file structure. ~10× slowdown vs None reported. |
| 4 | **Metadata: Media Basic/Extended** | ⚫⚫⚫ High | File count of media files only | Reads headers only (images: ~2–3 ms). Non-media files skipped entirely — so file type mix matters a lot. |
| 6 | **Include Hidden files**  | ⚫⚫  Medium | Additional file count (can be massive on system/home drives: `.cache`, `.local`, `node_modules`…) | Multiplier effect: more files = more of everything above. Excluded directories are the mitigation. |
| 7 | **Database mode** | ⚫ Low-Medium | Write throughput during indexing | Memory > File > Hosted. Memory avoids SQLite write I/O during scan; File mode is I/O-bound on writes. Batch transactions (1000 files/batch) already mitigate this. |
| 8 | **File type filter** | ⚫ Low-Medium | Reduces file count processed for metadata | Filtering to Audio/Video/Image skips all Text/Other for metadata — significant if indexing a mixed drive. |
| 9 | **File count (total)** | ⚫ Low | Drives filesystem walk time, DB insert count | Even without any options, enumerating 500K files takes time. Independent of sizes. |
| 10 | **Excluded directories** | ⚫ Low | Reduces effective file count | `node_modules`, `.cache`, temp dirs can represent 20–30% of home drive file count. |
| 11 | **System load / CPU** | ⚫ Low | Parallel metadata extraction uses 4–8 cores | Checksum is I/O-bound so CPU matters less there. Metadata is CPU-bound. |

Other long processes, NOT USED in CREATE/UPDATE 
| # | Factor | Impact Level | What drives it | Notes |
|---|--------|:---:|---|---|
| 5 | **MIME type verification** | ⚫⚫⚫⚫ Very High | File count (reads partial content of each file) | Update-only operation. "Guessing" from extension is free; verification reads file bytes. Impact scales with file count, not size. |
| 12 | **Collection Export** | ⚫⚫⚫⚫ Very High | Export of tables to SQLite or csv|| 20 to over 1 hour for 1M files


Test Sample / data set:  60Gb, 40 000 files, with a file of 20Gb, and mix of text, images, video.

| Test | Sample  | SSD 2 TB (extrapolated) | HDD 2 TB (estimated) | Scaling basis |
|---------|:-:|:-:|:-:|---|
| Baseline indexing | 00:00:04 | **~00:02:37** | ~00:05–08:00 | File count |
| Media Basic | 00:01:23 | **~00:54:00** | ~02:00–03:00 | File count (media files only) |
| Media Extended | 00:01:52 | **~01:13:00** | ~03:00–04:00 | File count (heavier per video) |
| Full Extended | 00:01:23 | **~00:54:00** | ~02:00–03:00 | File count  |
| Checksum only | 00:09:20 | **~06:06:00** | ~20:00–30:00 | Total bytes / throughput |
| Checksum + Media Basic | 00:11:06 | **~07:15:00** | ~22:00–33:00 | Bytes (dominant) + count |


## Development
Some ideas of developments for this screen:
* catalog a cloud instance (ex: Nextcloud)
* Customize file types
* For more, see the backlog of [Create development](https://github.com/users/StephaneCouturier/projects/7/views/1?filterQuery=create).
