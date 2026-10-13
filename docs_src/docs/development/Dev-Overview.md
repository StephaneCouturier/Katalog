# Overview
This page documents the practices for developing Katalog.<br/>

## Features management
### Roadmap
* [Roadmap](Dev-Roadmap)

### Backlog
Katalo's Backlog is available in GitHub with 3 views:
* [Full backlog](https://github.com/users/StephaneCouturier/projects/7/views/1?sliceBy[columnId]=Labels)
* [Next milestone](https://github.com/users/StephaneCouturier/projects/7/views/3) = current development, preparing next release
* [Next milestone + 1](https://github.com/users/StephaneCouturier/projects/7/views/5) = probable next developments


### Specifications
* [Specification pages](Specifications)

Documentation of the implementation of specific features & code:
* [Updates with metadata](../DevicesCatalogMetadata)
* [Progress Reporting](SpecProgressReport)
* [Version Numbers](SpecVersions)

## **Code**

### Source code on GitHub
* **katalog_master** branch: https://github.com/StephaneCouturier/Katalog
```
git clone https://github.com/StephaneCouturier/Katalog.git
```
* **katalog_development** branch: https://github.com/StephaneCouturier/Katalog/tree/katalog_development



## Practices
This provides information about how the source code is organized and any common practice used to facilitate its understanding, maintenance, and evolution.

### Model and file structure
* For data handling, many *objects* support the links with the database:  collection, device, storage, catalog, search, tag
* Each tab / screen of Katalog is managed in a different cpp file, belonging to the mainwindow code.

### Code practice
* Comments, comments, comments.
* variables:  first word lower cap, all other starting with capital letter:    thisIsAVariableName.
* database fields: to help with compatibility between SQLite and Postgres, fields are named in lower case, words separated by underscore: this_is_a_fied_name


### Build
see page [Build from source](Dev-Build-from-source)


GitHub workflows:

- [Katalog 3 Linux qt6_AppImage](https://github.com/StephaneCouturier/Katalog/actions/workflows/Katalog_3_Build_linux_qt6_appimage.yml)
- [Katalog 3 macOS](https://github.com/StephaneCouturier/Katalog/actions/workflows/Katalog_3_Build_macOS.yml)
- [Katalog 2 Linux qt6_AppImage](https://github.com/StephaneCouturier/Katalog/actions/workflows/Katalog_2_Build_linux_qt6_appimage.yml)
- [Katalog 2 Linux qt5_AppImage](https://github.com/StephaneCouturier/Katalog/actions/workflows/Katalog_2_Build_linux_qt5_appimage.yml)
- [Katalog 2 macOS](https://github.com/StephaneCouturier/Katalog/actions/workflows/Katalog_2_Build_macOS.yml)

## **Testing**
see page [Tests](Tests)

## **Documentation**
Katalog's documentation & user guide is made using Docusaurus & published on Github pages (this site)
see page [Documentation](Dev-Documentation)

### Release Process
see page [Release Process](Dev-Release)

### Load files to SourceForge
Default version per OS is set.
### Release notes
- on Github
- on SourceForge
- on Facebook

### Close issues & close Milestone
