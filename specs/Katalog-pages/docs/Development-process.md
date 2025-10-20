# Development process
This page documents the practices for developing Katalog.<br/>

## **Specifications**




## **Development**
Katalo's Backlog is available here and view 3 views:
* [Full backlog](https://github.com/users/StephaneCouturier/projects/7/views/1?sliceBy[columnId]=Labels)
* [Next milestone](https://github.com/users/StephaneCouturier/projects/7/views/3) = current development: 
* [Next milestone + 1](https://github.com/users/StephaneCouturier/projects/7/views/5) = probable next developments:



## **Testing**




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

#### Database Schema Versioning:
- **Schema version = App version** when DB changes occur
- **Schema version unchanged** when no DB changes (like 2.7 staying at 2.6)


## **Documentation**
Katalog's documentation/user guide is made using Docusaurus & github pages on https://stephanecouturier.github.io/Katalog








