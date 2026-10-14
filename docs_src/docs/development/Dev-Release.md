---
id: Dev-Release
title: Release Process
description: "Steps to prepare, package and publish a Katalog release"
---

# Release Process
![Approved](https://img.shields.io/badge/Status-Approved-brightgreen) ![2.13](https://img.shields.io/badge/Version-2.13%20%2F%203.0-blue)

This page describes the steps to Release Katalog.
This includes:
- Preparation
- Building the packages
- Communication

---
## Version Number & date
KDE-aligned versioning policy that works with current major.minor approach.

### Core Principles:
1. Single Source of Truth: **CMakeLists.txt defines the version**
2. Automatic Propagation: Version flows to code via CMake
3. Consistent Naming: Align with KDE practices while keeping the major.minor format (patch number only used in Katalog for Flathub publishing)
4. Automatic Dates: Build-time date generation

### Version Format
- CMakeLists.txt: `VERSION 2.8`
  - Katalog 3: `qt_quick/CMakeLists.txt` — `project(... VERSION 3.0)` and `KATALOG_VERSION_STRING`
  - Katalog 2: `qt_widgets/CMakeLists.txt` — `project(... VERSION 2.13)`
  - A pre-release version string contains `alpha` or `beta`; it makes Katalog 3 use its own pre-release settings file. The release string must contain neither.
- Tags: `v2.8`, `v2.9`
- Display: "Katalog 2.8"

### Database Schema Versioning
see [SpecVersions](SpecVersions)
- **Schema version = App version** when DB changes occur
- **Schema version unchanged** when no DB changes (like 2.7 staying at 2.6)


## Release Check list

### Local folder
/Katalog/Release/x.xx


### GitHub
#### Release issue template (Check list) 

Name:
Release x.xx

Description:

This issue is a check list to help preparing and publishing the Release

Functional updates
- [ ] Version number, date (Katalog 3 and Katalog 2 CMakeLists.txt)
- [ ] No pre-release marker left: search `beta` / `alpha` case-insensitively in version strings and every user-visible text (translated or not), e.g. dialog titles
- [ ] Database schema version, if the schema changed (see [SpecVersions](SpecVersions))
- [ ] Clean Application Output: no debug messages, errors, and warnings
- [ ] macOS yml if using new KF6 libraries

Language & documentation
- [ ] Language updates: no unfinished translation left, `.qm` files copied to `translations/`
- [ ] Comments, code style, and structure.
- [ ] Documentation: English, French and Czech pages, version badges, `npm run build` without error, copy to /docs
- [ ] Flatpak metainfo (`packaging/linux/io.github.stephanecouturier.katalog.appdata.xml`): new `<release>` entry with its notes, screenshot URLs still valid on `katalog_master`
- [ ] Release notes: changes affecting existing users (settings file, compatibility of collections with older versions — see [SpecVersions](SpecVersions))

Packaging
- [ ] Linux AppImage locally
- [ ] Windows Portable
- [ ] Windows AdvancedInstaller
- [ ] Merge and push katalog_development to katalog_master (see [Merge to katalog_master](#merge-to-katalog_master))
- [ ] Linux AppImages with GitHub Actions
- [ ] macOS versions with GitHub Actions

Publishing
- [ ] Load packages to SourceForge
- [ ] Define Default package for Linux, Windows, macOS
- [ ] Release publish (sourceforge)
- [ ] Release publish (github)
- [ ] Flatpak: update `tag` and `commit` in `packaging/linux/io.github.stephanecouturier.Katalog.yml`, then submit the update to Flathub
- [ ] Release publish (Facebook)
- [ ] Close GitHub issues and milestone
- [ ] Inform users who submitted bugs or ideas

### Merge to katalog_master {#merge-to-katalog_master}
`katalog_master` only receives releases, so the merge is a fast-forward: master moves to the
last commit of `katalog_development`, with no merge commit. Commit everything on
`katalog_development` first.

```
git checkout katalog_master
git pull origin katalog_master
git merge --ff-only katalog_development
git push origin katalog_master
git checkout katalog_development
```

If `--ff-only` stops with "Not possible to fast-forward", master has a commit that development
does not: look at it (`git log katalog_development..katalog_master`) before going further.

### Packages on SourceForge
Despite some features in GitHub to provide downloads, SourceForge is still used as the place for user to download all versions.

https://sourceforge.net/projects/katalogg/files/

Benefits: 
* auto selection of latest version based on Operating System
* download tracking and reporting system
* anonymous Help & Requests forum

Default version per OS is set.

### Release notes
- on Github
- on SourceForge
- on Facebook

### Close issues & close Milestone

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
| `Katalog.desktop` GenericName | `packaging/linux/Katalog.desktop` |
| `appdata.xml` summary | `packaging/linux/io.github.stephanecouturier.katalog.appdata.xml` |

### Level 2 — One sentence
```
Catalog your devices to search, analyze, and backup your files.
```

| Target | Location |
|---|---|
| `Katalog.desktop` Comment | `packaging/linux/Katalog.desktop` |
| `README.md` (root) | `README.md` |
| `CLAUDE.md` | `CLAUDE.md` |
| SourceForge / Short Summary | https://sourceforge.net/p/katalogg/admin/overview |
| Katalog / About page | mainj.cpp |

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
| Flathub description in `appdata.xml` | `packaging/linux/io.github.stephanecouturier.katalog.appdata.xml` |
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
- Development: https://stephanecouturier.github.io/Katalog/docs/Dev-Overview
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
