# Release Process
This page describes the steps to Release Katalog.
This includes:
- Preparation
- Building the packages
- Communication

---
## Release Check list

##  Folder
/Katalog/Release/x.xx


## GitHub
### Release issue template (Check list) 

Name:
Release x.xx

Description:
This issue is a check list to help prepare and publish the Release

Functional updates
- [ ] Version number, date
- [ ] Clean Application Output: no debug messages, errors, and warnings
- [ ] CMakeList_qt5.txt update if changes to CMakeList.txt
- [ ] macOS yml update if using new KF6 libraries

Language & documentation
- [ ] Language updates
- [ ] Comments, code style, and structure.
- [ ] Documentation (inc build and copy to /docs)

Packaging
- [ ] Linux pre-build & AppImages locally
- [ ] Linux Portable
- [ ] Windows Portable
- [ ] Windows AdvancedInstaller
- [ ] Merge and push katalog_development to katalog_master
- [ ] Linux Qt5 and Qt6 Builds with gitHub Actions
- [ ] macOS versions with gitHub Actions

Publishing
- [ ] Load packages to SourceForge
- [ ] Define Default package for Linux, Windows, macOS
- [ ] Release publish (sourceforge)
- [ ] Release publish (github)
- [ ] Submit Flatpak update
- [ ] Release publish (Facebook)
- [ ] Close GitHub issues and milestone
- [ ] Inform users who submitted bugs or ideas

---
## Description & Summaries

This section is the **single source of truth** for all Katalog descriptions.

Update here first, then propagate to each target location listed below.


### Level 1 — One-liner (generic name)
```
File catalog manager
```

| Target | Location |
|---|---|
| `Katalog.desktop` GenericName | `packaging/qt_widgets/Katalog.desktop` |
| `appdata.xml` summary | `packaging/qt_widgets/io.github.stephanecouturier.katalog.appdata.xml` |

### Level 2 — One sentence
```
Catalog your devices to search, analyze, and backup your files.
```

| Target | Location |
|---|---|
| `Katalog.desktop` Comment | `packaging/qt_widgets/Katalog.desktop` |
| `README.md` (root) | `README.md` |
| `CLAUDE.md` | `CLAUDE.md` |
| SourceForge / Short Summary | https://sourceforge.net/p/katalogg/admin/overview |

### Level 3 — Short summary (~3 lines)
```
Catalog your devices to search, analyze, and backup your files:
- Create catalogs from different sources or devices
- Search and explore files even when the devices are disconnected
- Organize and backup your collection of files, and get statistics
```

| Target | Location |
|---|---|
| Facebook "Bio" | External |
| SourceForge / Full Description | External |


### Level 4 — Normal summary & features (less than 10 lines)

```
Catalog your devices to search, analyze, and backup your files:
- Create catalogs from different devices, including metadata or checksums
- Search and explore files even when the devices are disconnected
- Search with advanced filters using including metadata or checksums, and find duplicates or differences
- Organize your collection of devices and files, and get statistics
- Backup or Archive files between catalogs
- Multiple language support, OpenSource, Cross-platform (Linux, Windows, macOS)
```

| Target | Location |
|---|---|
| Flathub description in `appdata.xml` | `packaging/qt_widgets/io.github.stephanecouturier.katalog.appdata.xml` |
| SourceForge / Features | External |
| `README.md` body | `README.md` |
| Flathub Description | External |


### Level 5 — Addition to Normal summary (~5 lines)

```
User documentation:
- Get started/Tutorial: https://stephanecouturier.github.io/Katalog/docs/tutorial
- Download (Linux Flathub):  https://flathub.org/en/apps/io.github.stephanecouturier.Katalog
- Download (other versions): https://sourceforge.net/projects/katalogg
- Documentation: https://stephanecouturier.github.io/Katalog/docs/Overview
- Development: https://stephanecouturier.github.io/Katalog/docs/Development-Overview
- Facebook: https://www.facebook.com/Katalog-107117844916308
```

| Target | Location |
|---|---|
| SourceForge / Full Description | External |
| `README.md` body | `README.md` |

### Level 6 — Not used, drafts
```
For digital archivists, content creators, system administrators, and anyone who needs to organize
and track large collections of files across multiple storage devices.
```

## Keywords

catalog;files;search;explore;folders;organization;storage;backup;archiving;statistics
