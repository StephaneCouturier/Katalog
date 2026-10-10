---
version: "3.0"
---
# Create
![3.0](https://img.shields.io/badge/Version-3.0-blue)

## Summary
This page describes the **Create** page, where a new **catalog** is created: an index of the files of one folder, kept in the collection so that it can be searched and explored even when the device is disconnected.

It takes three steps:
1. [Define the catalog](#definition): the folder to index, its storage device and its name.
1. Choose the [content options](#content-options): file types, sub-folders, hidden files, metadata, checksums and exclusions.
1. Click *Create*.

![Create page: catalog definition, content options and global parameters](/img/create_01_overview.png)

## Catalog definition {#definition}

| Field | Description |
|-------|-------------|
| *Source path* | The folder to index. It is filled with the path of the device selected on the [Selection](Selection) page, and can be typed or chosen with *Select the path*. |
| *Storage* | The storage device where the folder is: the drive, disk or computer (see [Devices](Devices#model)). Only storage devices are listed. A new collection already has one, *Local disk*. To add another one, use *Add Storage* on the [Devices](Devices#page-actions) page. |
| *Catalog name* | The name of the new catalog. Each catalog needs its own name. *Generate name from path* (the button beside the field) makes one from the source path, with `_` in place of `/` and `:`. |

When a field is missing or wrong, a message at the top of the page says what is needed, for example *Provide a name for this new catalog.* It disappears when the field is corrected. The same happens when the source folder does not exist, or when another catalog already has the name.

When the source folder holds no file, a question is asked first: the folder may be empty, or the device may not be mounted there. *Yes* creates the catalog anyway, empty.

## Content options {#content-options}
The options decide what is indexed. They can be changed later by editing the catalog (see [Edit device](DevicesEdit#content-options)); the catalog then needs an update to follow them.

### File type
The catalog can be limited to one type of files.

| Type | Description | Definition | Extensions examples |
|------|-------------|------------|---------------------|
| All | All file types, without filtering. | | |
| Audio | Music, podcasts, audio recordings and sound files. | As defined by MIME types. | MP3, FLAC, AAC, M4A, OGG, WAV, AIFF, Opus, WMA, MIDI, AMR (50+ formats) |
| Image | Photos, graphics, diagrams, icons and visual content. | As defined by MIME types. | JPG, PNG, HEIC, WebP, TIFF, RAW, SVG, XCF, GIF, BMP (100+ formats) |
| Text | Documents, code files, markup, data files and readable content. | Specific Katalog definition: all files with a MIME type starting with `text/`, plus application files like PDF and Word documents. | PDF, DOCX, ODT, Markdown, HTML, JSON, source code files, ebooks (100+ formats) |
| Video | Movies, clips, animations and video content. | As defined by MIME types. | MP4, MKV, AVI, WebM, MOV, FLV, 3GP, OGV, M2TS (40+ formats) |
| Other | All other file types. | Specific Katalog definition. | ZIP, RAR, EXE, DLL, ISO: executables, archives, system files and application files not classified as Text. |
| None | Files whose type cannot be found from the extension. | Files without extension or with an unknown extension. | |

The type of a file is found from its **extension**, using the MIME database of the system: hundreds of formats are recognised, and new ones as the system is updated. This keeps indexing fast. A file with a wrong or missing extension can therefore get the wrong type.

### Include subdirectories
Ticked (the default), the files of all the sub-folders are indexed. Unticked, only the files directly in the source path.

### Include hidden files
*None* (the default) skips hidden files and folders; *All* includes them.<br/>
On a home folder or a system drive, hidden folders (`.cache`, `.local`…) can hold many files: excluding the unneeded ones keeps the catalog small.

### Include metadata
How much information is read from the content of the files. It makes indexing longer, but gives richer results in [Search](Search) (dimensions, duration, artist…) and in [Statistics](Statistics).

| Level | What is read |
|-------|--------------|
| *None* | No metadata: fastest. |
| *Media Basic* | The essentials of images, videos and audio files: dimensions and orientation of images; dimensions, duration, codec and frame rate of videos; artist, album, track, duration and bit rate of audio files. |
| *Media Extended* | All the metadata of images, videos and audio files, including technical details. |
| *Full Extended* | All the metadata of every supported file type. |

The extended levels use the KFileMetaData library, which decides the supported types and fields; their metadata can be seen with *Show extended metadata (JSON)* in the search results and in Explore.<br/>
Media levels read only media files: other files are skipped. Files that are damaged or cannot be read are skipped without stopping the catalog.

Supported media files:
* **Images**: jpg, png, gif, bmp, tiff, webp, svg, heic, raw, xcf
* **Videos**: mp4, mkv, avi, mov, wmv, flv, webm, m4v, mpg, 3gp, ogv, vob
* **Audio**: mp3, wav, flac, ogg, m4a, aac, wma, opus, aiff, mid, amr

### Include checksum
*SHA-256* calculates a checksum of each file: a fingerprint of its content. Checksums make the [duplicate and difference searches](Search) exact, and allow verifying later that files did not change.<br/>
⚠️ It is much longer than the other options, because **all** the data of every file is read (see the [Performance guide](#performance)).

### Exclude folders or files
Paths or names that are not indexed **in this catalog**.<br/>
Type a path or a name in *Path or name to exclude*, or choose a folder with *Select the path*, then click *Add*. The *Remove* button beside an entry takes it out of the list.<br/>
The list is saved with the catalog.

## Global Parameters {#global-parameters}
The *Global Parameters* section holds settings shared by **all the catalogs** of the collection. The button on its title line folds or unfolds it.

### Global exclude directory {#exclude-directories}
Paths excluded from **every catalog**, when it is created and each time it is updated.<br/>
Type a path in *Path to exclude globally* (or use *Select the path*), then click *Add*. *Remove* beside an entry takes it out.

### How exclusions work
Global and per-catalog exclusions add up: a file or folder skipped by either list is not indexed.<br/>
An exclusion is **text matching**: every file or folder whose full path **contains** the text is skipped. The matching is case-sensitive.
* **Full path**: `/home/username/Downloads/temp` skips that folder and all its content.
* **Folder name**: `node_modules` skips **every** `node_modules` folder, wherever it is.
* **Part of a name**: `.cache` skips `/home/username/.cache/`, but also `/home/username/.cachedata/`, since both paths contain the text.

## Creating the catalog {#creating}
*Create* saves the catalog and starts indexing. Progress is shown in the activity panel at the bottom of the window (see [Interface](Interface)). If another operation is running, the creation waits in the queue.

Indexing runs in two phases:
1. the **list of files** is read and saved: the catalog can be used as soon as this phase ends,
2. then, when chosen, the **metadata** and **checksums** are read.

*Stop* ends the creation:
* during the first phase, the catalog is not created,
* during the second phase, the catalog is **kept** with its full list of files; the metadata or checksums not read yet are completed by the next update of the catalog.

When the creation ends, the new catalog is selected on the [Selection](Selection) page and the [Devices](Devices) page opens with a report: the catalog name and path, its number of files and total size, and, when its storage was updated, the storage's used, free and total space with what was added. If a metadata or checksum scan was not finished, a message explains it instead.<br/>
*Close* (or `Esc`) closes the page.

## Performance guide {#performance}

### What affects scanning speed?


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

Other long processes, not used by Create or Update:

| # | Factor | Impact Level | What drives it | Notes |
|---|--------|:---:|---|---|
| 5 | **MIME type verification** | ⚫⚫⚫⚫ Very High | File count (reads partial content of each file) | Update-only operation. "Guessing" from extension is free; verification reads file bytes. Impact scales with file count, not size. |
| 12 | **Collection Export** | ⚫⚫⚫⚫ Very High | Export of tables to SQLite or csv|| 20 to over 1 hour for 1M files


Test sample / data set:  60Gb, 40 000 files, with a file of 20Gb, and mix of text, images, video.

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
* Catalog a cloud instance (for example Nextcloud).
* Customize file types.
* For more, see the backlog of [Create development](https://github.com/users/StephaneCouturier/projects/7/views/1?filterQuery=create).
