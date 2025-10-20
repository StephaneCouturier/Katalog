# Overview
## Files indexing & Storage devices manager
Katalog is a powerful <b>file catalog manager</b> that helps you organize and find files.<br/>
It gives a full view of the files across <b>multiple storage devices without needing them connected</b>.<br/>
Katalog creates comprehensive <b>indexes so you can search your entire file collection from one place</b>, whether you're managing Internal and External Drives, USB sticks, Network storage, or storage Discs (blueray, DVDs, CDs).

- **Catalog everything**: Create detailed indexes of files from any storage device
- **Search offline**: Find files instantly without connecting or mounting the original device
- **Organize the collection**: Manage multiple storage devices and their catalogs in a unified hierarchy


![](/img/screen_search_01.png)
<b>8 main tabs for 8 main features</b>
![](/img/global_tabwidget.png)
1. [Create](Create) catalogs of **files**
1. [Search](Search) files across multiple storage devices **without needing them connected**
1. [Explore](Explore) the catalogs' hierarchy and files
1. Organize storage [Devices](Devices) and their catalogs in a unified hierarchy with virtual devices
1. Get [Statistics](Statistics) about your file collections & storage usage
1. Personalize [Tags](Tags) and assigned them to directories for extra searching or statistics
1. Compare [BackUp](BackUp) catalogs to confirm the coverage of backed up files and folders between source and target devices
1. Personalize your experience with your [Settings](Settings) such as language and theme
<br/>

<div className="row">
  <div className="col col--6">
  <br/><br/>and a <b>[Selection](Selection) panel</b> to
  <br/>
  * Choose searching in <b>Catalogs</b> or directly in <b>Connected drives</b><br/>
  * Set the device in the <b>hierarchy to be used</b> Search or Create or get Statistics or manage BackUp coverage<br/>
  </div>
  <div className="col col--6" style={{maxWidth: '200px'}}>
    ![](/img/global_selection_panel.png)
  </div>
</div>

---
## Key features

### Rich File Information
- **[File Type intelligence](Create#enhanced-file-type-filtering)**: Standard detection from extensions, and MIME type verification
- **[Catalog options](Create#select-options-to-includeexclude-files)**: Only include a [type of files](Create#enhanced-file-type-filtering) , and [include/exclude directories or hidden files](Create#other-options)
- **[Metadata extraction](Create#metadata-extraction)**: Automatically extract metadata from images (dimensions, camera info), videos (duration, resolution), and audio files (artist, album, duration) or any other file type.
- **[Folder Tag system](Tags)**: Organize and categorize folders with custom tags

### Powerful Search & Discovery
- **[Advanced Search parameters](Search#search-text-criteria)**: Find files by name, path, size, date, file type, and metadata
- **[Smart Filtering](Search#file-criteria)**: Use multiple criteria simultaneously to narrow down results quickly
- **[Find Duplicates](Search#duplicates-on)**: Identify duplicate files across different storage devices
- **[Find Differences](Search#differences-on)**: See differences between two storage locations or backup versions


### Device & Catalog Management
- **[Device Organization](Devices)**: Organize storage devices in a hierarchical structure (Virtual > Storage > Catalogs)
- **[Update](DevicesCatalogs)**: Keep catalogs current with manual updates or automatic updates
- **[Import Support](DevicesCatalogs#import)**: Import catalogs from other tools like VVV


### Analysis & Management
- **[File Explorer](Explore)**: Browse catalog contents as if the device were connected
- **[Statistics](Statistics)**: Track your file collections & storage usage
- **[Backup Management](BackUp)**: Map and compare source directories with their backups
- **[Batch Operations](Search#batch-process)**: Export results and perform batch actions on files

### Advanced Capabilities
- **[Command Line Interface](CommandLines)**: Automate catalog updates and searches via command line (Linux)
- **[Database Flexibility](Settings#collection)**: Choose between CSV files or SQLite database storage

---
## Multi-Platform Support
| Main OS           | Distributions / Versions    | Packaging    |
|-------------------|-------------|-------------|
| <div style={{display: 'flex', alignItems: 'center', gap: '10px'}}><img src={require('/img/linux.png').default} width="40" /> GNU/Linux</div>         | Any 64bits, glibc 2.38+ <br/>Any 32bits, glibc 2.35     | AppImage <br/>Portable |
| <div style={{display: 'flex', alignItems: 'center', gap: '10px'}}><img src={require('/img/windows.png').default} width="40" /> Microsoft Windows</div> | 64bits:  Windows 10 & Windows 11    | Installer <br/> Portable       |
| <div style={{display: 'flex', alignItems: 'center', gap: '10px'}}><img src={require('/img/macos.png').default} width="40" /> Apple macOS</div>       | 14+      | Installer<br/> Portable       |

---
## Multi-Language Support

### App languages
Katalog is available in:

| Locale   | Language    |
|----------|-------------|
| bg_BG    | Bulgarian   |
| cz_CZ    | Czech       |
| da_DK    | Danish      |
| de_DE    | German      |
| en_US    | English     |
| es_ES    | Spanish     |
| et_EE    | Estonian    |
| fi_FI    | Finnish     |
| fr_FR    | French      |
| el_GR    | Greek       |
| hi_IN    | Hindi       |
| hr_HR    | Croatian    |
| hu_HU    | Hungarian   |
| id_ID    | Indonesian  |
| it_IT    | Italian     |
| ja_JP    | Japanese    |
| lt_LT    | Lithuanian  |
| lv_LV    | Latvian     |
| nb_NO    | Norwegian   |
| nl_NL    | Dutch       |
| pl_PL    | Polish      |
| pt_PT    | Portuguese  |
| ro_RO    | Romanian    |
| si_SI    | Slovenian   |
| sk_SK    | Slovak      |
| sr_RS    | Serbian     |
| sv_SE    | Swedish     |
| uk_UA    | Ukrainian   |
| zh_CN    | Chinese     |

### Documentation languages
Documentation is available in:
| Locale   | Language    |
|----------|-------------|
| en_US    | English     |
| cz_CZ    | Czech       |
| fr_FR    | French      |

---

**Ready to organize your file collection?** Start with the **[Tutorial](tutorial)** to create your first catalog in under 5 minutes.
