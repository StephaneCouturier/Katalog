# Overview
This page documents the practices for developing Katalog.<br/>

## Features management
### Roadmap
* [Roadmap](Development-Roadmap)

### Backlog
Katalo's Backlog is available in GitHub with 3 views:
* [Full backlog](https://github.com/users/StephaneCouturier/projects/7/views/1?sliceBy[columnId]=Labels)
* [Next milestone](https://github.com/users/StephaneCouturier/projects/7/views/3) = current development
* [Next milestone + 1](https://github.com/users/StephaneCouturier/projects/7/views/5) = probable next developments


### Specifications
Documentation of the implementation of specific features & code:
* [Updates with metadata](DevicesCatalogMetadata) (v2.8)
* [Progress Reporting](SpecProgressReport) (v2.8)
* [Version Numbers](SpecVersions) (v2.8)

## **Code**

### Source code on GitHub
* katalog_master branch: https://github.com/StephaneCouturier/Katalog
* Source Code: katalog_development branch: https://github.com/StephaneCouturier/Katalog/tree/katalog_development
```
git clone https://github.com/StephaneCouturier/Katalog.git
```

### Released packages on SourceForge
Desipe some feature in GitHub to provide downloads, SourceForge is still used as the place for user to download all versions.
Benefits: 
* auto selection of latest version based on Operating System
* download tracking and reporting system
* anonymous Help & Requests forum

## Practices
This provides information about how the source code is organized and any common practice used to facilitate its understanding, maintenance, and evolution.

### Model and file structure
* For data handling, many *objects* support the links with the database:  collection, device, storage, catalog, search, tag
* Each tab / screen of Katalog is managed in a different cpp file, belonging to the mainwindow code.

### Code practice
* Comments, comments, comments.
* variables:  first word lower cap, all other starting with capital letter:    thisIsAVariableName.
* database fields: to help with compatibility between SQLite and Postgres, fields are named in lower case, words separated by underscore: this_is_a_fied_name


## **Build**
see page [Build from source](Development-Build-from-source)
GitHub workflows:
- AppImage Qt5 (gblic 2.35)
- AppImage Qt6 (gblic 2.38)
- macOS latest version
- macOS legacy version (2.5 no KF6)





## **Testing**
* all manual


## **Documentation**
Katalog's documentation/user guide is made using Docusaurus & github pages on https://stephanecouturier.github.io/Katalog


## **Release**
### Version Number & date
KDE-aligned versioning policy that works with current major.minor approach.

#### Core Principles:
1. Single Source of Truth: **CMakeLists.txt defines the version**
2. Automatic Propagation: Version flows to code via CMake
3. Consistent Naming: Align with KDE practices while keeping the major.minor format (patch number not used in Katalog)
4. Automatic Dates: Build-time date generation

#### Version Format
- CMakeLists.txt: `VERSION 2.8`
- Tags: `v2.8`, `v2.9`
- Display: "Katalog 2.8"

#### Database Schema Versioning
see [SpecVersions](SpecVersions)
- **Schema version = App version** when DB changes occur
- **Schema version unchanged** when no DB changes (like 2.7 staying at 2.6)


### Load files to SourceForge
and set default version per OS
### Post release notes on SourceForge
### Post release notes on Facebook
### Close issues & close Milestone
