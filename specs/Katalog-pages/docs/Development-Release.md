# Release Process
This page describes the steps to Release Katalog.
This includes:
- Preparation
- Building the packages
- Communication

## Local preparation

### Release folder
/Katalog/Release/2.xx



## GitHub
### Release issue (Check list) template

Name:
Release x.x

Functional updates
- [ ] Version number, date
- [ ] Clean Application Output: no debug messages, errors, and warnings
- [ ] CMakeList_qt5.txt update if changes to CMakeList.txt
- [ ] macOS yml if using new KF6 libraries

Language & documentation
- [ ] Language updates
- [ ] Comments, code style, and structure.
- [ ] Documentation

Packaging
- [ ] Linux pre-build & AppImages locally
- [ ] Linux Qt5 and Qt6 Builds with gitHub Actions
- [ ] Linux Portable versions with gitHub Actions
- [ ] Linux Flatpak appdata or yml as needed
- [ ] macOS  macOS versions
- [ ] Windows Portable
- [ ] Windows AdvancedInstaller

Publishing
- [ ] Load packages to SoureForge
- [ ] Define Default package for Linux, Windows, macOS
- [ ] Release publish (sourceforge)
- [ ] Release publish (github)
- [ ] Release publish (Facebook)
- [ ] Close GitHub issues and milestone

