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
* [Updates with metadata](SpecCatalogMetadata)
* [Progress Reporting](SpecProgressReport)
* [Version Numbers](SpecVersions)

## **Code**

### Source code on GitHub
* **katalog_master** branch: https://github.com/StephaneCouturier/Katalog
```
git clone https://github.com/StephaneCouturier/Katalog.git
```
* **katalog_development** branch: https://github.com/StephaneCouturier/Katalog/tree/katalog_development

### Build
see page [Build from source](Dev-Build-from-source)


GitHub workflows:

- [Katalog 3 Linux qt6_AppImage](https://github.com/StephaneCouturier/Katalog/actions/workflows/Katalog_3_Build_linux_qt6_appimage.yml)
- [Katalog 3 macOS](https://github.com/StephaneCouturier/Katalog/actions/workflows/Katalog_3_Build_macOS.yml)
- [Katalog 2 Linux qt6_AppImage](https://github.com/StephaneCouturier/Katalog/actions/workflows/Katalog_2_Build_linux_qt6_appimage.yml)
- [Katalog 2 Linux qt5_AppImage](https://github.com/StephaneCouturier/Katalog/actions/workflows/Katalog_2_Build_linux_qt5_appimage.yml)
- [Katalog 2 macOS](https://github.com/StephaneCouturier/Katalog/actions/workflows/Katalog_2_Build_macOS.yml)

### **Testing**
see page [Tests](Tests)

## **Documentation**
Katalog's documentation & user guide is made using Docusaurus & published on Github pages (this site)
see page [Documentation](Dev-Documentation)

## Release Process
see page [Release Process](Dev-Release)

