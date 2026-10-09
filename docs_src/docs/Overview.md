---
version: "3.0"
---
# Overview
![3.0](https://img.shields.io/badge/Version-3.0-blue)

## Files indexing & Storage devices manager
Katalog is a <b>file catalog manager</b> that helps you organize and find files.<br/>
It gives a full view of the files across <b>multiple storage devices without needing them connected</b>.<br/>
Katalog creates <b>indexes so you can search your entire file collection from one place</b>, whether you manage internal and external drives, USB sticks, network storage, or discs (Blu-ray, DVD, CD).

- **Catalog everything**: create detailed indexes of the files of any storage device.
- **Search offline**: find files without connecting or mounting the original device.
- **Organize the collection**: manage storage devices and their catalogs in one hierarchy.
- **Back up**: copy or move the files missing from a backup, using the catalogs.

![Katalog window: Selection, Search and Results pages side by side](/img/interface_01_overview.png)

## Pages
The window shows the **[Selection](Selection)** page on the left: the tree of all the devices of the collection. Selecting a device sets the scope of the other pages. Each feature opens as a page next to it, from the drawer (see [Interface](Interface)):

1. [Search](Search) files across all the storage devices, **without needing them connected**, and act on the [results](SearchResults).
1. Organize the [Devices](Devices) and their catalogs in one hierarchy, with **virtual** devices to group them, and [edit](DevicesEdit) them.
1. [Explore](Explore) the folders and files of a catalog.
1. [Create](Create) catalogs of files.
1. Copy or move files with [Backup](BackUp) links, and check the coverage of the backups.
1. Follow the growth of the collection in [Statistics](Statistics).
1. Assign [Tags](Tags) to folders, for extra search possibilities.
1. Set up the collection and the application in the [Settings](Settings): data mode, import, theme, language.

---
## Key features

### Rich file information
- **[File type detection](Create#file-type)**: the type of each file is found from its extension, for hundreds of formats.
- **[Catalog options](Create#content-options)**: include only a [type of files](Create#file-type), sub-folders or not, [hidden files](Create#include-hidden-files), and [exclude folders or files](Create#global-parameters).
- **[Metadata extraction](Create#include-metadata)**: dimensions of images and videos, duration of videos and audio files, artist and album of music, or any metadata of other file types.
- **[File checksum](Create#include-checksum)**: SHA-256 checksums for exact duplicate and difference searches, and to verify files.
- **[Folder tags](Tags)**: organize and categorize folders with your own tags.

### Search & discovery
- **[Search criteria](Search)**: find files by name (several texts, exclusions, regular expressions), folder, size, date, file type and metadata.
- **[Find duplicates](Search#duplicates-on)**: identify duplicate files across devices.
- **[Find differences](Search#differences-on)**: see the differences between two devices, for example a folder and its backup.
- **[Actions on results](SearchResults#batch-process)**: filter the results, export them, verify checksums, extract metadata, move to trash.

### Devices & catalogs
- **[Device hierarchy](Devices#model)**: physical group, storage devices and catalogs, and virtual groups.
- **[Update](Devices#context-menu)**: keep the catalogs current, one by one or all the available ones at once.
- **[Path changes](DevicesEdit#path-change)**: when a drive is mounted somewhere else, fix all its catalogs in seconds, without reading the drive.
- **[Collection import & update](Settings#import-update)**: import devices from another Katalog collection or from VVV, and refresh them later.

### Analysis & management
- **[File explorer](Explore)**: browse the content of a catalog as if the device were connected, with the size of each folder.
- **[Statistics](Statistics)**: track the growth of the collection and of the storage space.
- **[Backup & archive](BackUp)**: copy or move files between catalogs, preview, run several links at once, and check the coverage.
- **[Data modes](Settings#database-memory-mode)**: keep the collection in a folder of files, in a SQLite database, or in a hosted database.

### Katalog 2
Katalog 2 remains available, with its [documentation](Katalog2), including the [command line](k2/CommandLines) (Linux).

---
## Multi-Platform Support

| Main OS           | Distributions / Minimum Versions    | Packaging    |
|-------------------|-------------|-------------|
| <div style={{display: 'flex', alignItems: 'center', gap: '10px'}}><img src={require('/img/linux.png').default} width="40" /> GNU/Linux</div>         | 64bits, glibc 2.38+ <br/>32bits, glibc 2.35     | [FlatHub/Flatpak](https://flathub.org/en/apps/io.github.stephanecouturier.Katalog)<br/>[AppImage](https://sourceforge.net/projects/katalogg/files/) <br/>[Portable](https://sourceforge.net/projects/katalogg/files/) |
| <div style={{display: 'flex', alignItems: 'center', gap: '10px'}}><img src={require('/img/windows.png').default} width="40" /> Microsoft Windows</div> | 64bits:  Windows 10 & Windows 11    | [Installer](https://sourceforge.net/projects/katalogg/files/) <br/> [Portable](https://sourceforge.net/projects/katalogg/files/)       |
| <div style={{display: 'flex', alignItems: 'center', gap: '10px'}}><img src={require('/img/macos.png').default} width="40" /> Apple macOS</div>       | 14 (Sonoma)  | [Installer](https://sourceforge.net/projects/katalogg/files/)<br/> [Portable](https://sourceforge.net/projects/katalogg/files/)       |


### macOS Compatibility

- **Minimum macOS version**: macOS 14 (Sonoma) or later
- **Tested on**: macOS 15 (Sequoia), Apple Silicon
- **Architecture**: Apple Silicon (M1/M2/M3) only. Intel Macs are **not** supported — Rosetta 2 translates Intel software to run on Apple Silicon, not the other way round, so there is no way to run this build on an Intel Mac.
- **Build environment**: Built on macOS 14 (Sonoma) with Qt 6 and KDE Frameworks 6 (KF6), via Homebrew

**Important: the app is not code-signed or notarized.**

macOS will block the app by default. To run Katalog, try one of these methods:

1. Right-click the app → select "Open" (not double-click) → confirm in the dialog
2. Or go to System Settings → Privacy & Security → scroll down and click "Open Anyway"
3. Or run in Terminal: `xattr -cr /path/to/Katalog.app` then open normally. Add `sudo` in front if permission is denied.

**Known limitation**: 

on Macs managed by an organization (MDM/corporate profile), these workarounds may be blocked by security policy. There is currently no solution for managed Macs — this would require signing the app with an Apple Developer certificate.

A legacy build (v2.5, pure Qt6, no KF6 dependencies) is also available with the same macOS requirements.

---
## Multi-Language Support

### App languages
Katalog is available in:

| Locale | Language | Locale | Language | Locale | Language |
|--------|----------|--------|----------|--------|----------|
| bg_BG  | Bulgarian  | hi_IN  | Hindi      | pl_PL  | Polish     |
| cs_CZ  | Czech      | hr_HR  | Croatian   | pt_PT  | Portuguese |
| da_DK  | Danish     | hu_HU  | Hungarian  | ro_RO  | Romanian   |
| de_DE  | German     | id_ID  | Indonesian | si_SI  | Slovenian  |
| en_US  | English    | it_IT  | Italian    | sk_SK  | Slovak     |
| es_ES  | Spanish    | ja_JP  | Japanese   | sr_RS  | Serbian    |
| et_EE  | Estonian   | lt_LT  | Lithuanian | sv_SE  | Swedish    |
| fi_FI  | Finnish    | lv_LV  | Latvian    | uk_UA  | Ukrainian  |
| fr_FR  | French     | nb_NO  | Norwegian  | zh_CN  | Chinese    |
| el_GR  | Greek      | nl_NL  | Dutch      |        |            |

### Documentation languages
Documentation is available in:
| Locale   | Language    |
|----------|-------------|
| en_US    | English     |
| cs_CZ    | Czech       |
| fr_FR    | French      |

---

**Ready to organize your file collection?** Start with the **[Tutorial](tutorial)** to create your first catalog in under 5 minutes.
