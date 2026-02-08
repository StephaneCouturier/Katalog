# Release Process
This page describes the steps to Release Katalog.
This includes:
- Building and loading the final packages
- Releases Notes & communication

## Local preparation
### Test AppImage creation

### Release folder
add version folder in:
/home/stephane/Documents/Informatique/Katalog/Release/


## GitHub

###  github task: Release x.x

    Non functional updates:
      - Version number, date
      - Language updates
      - Comments, code style, and structure.
      - Documentation
      - Clear Application Output errors and warnings

### PR from katalog_development to katalog_master
  - after last PUSH to katalog_development,
  - open pull request, title:  Katalog development to Katalog Master for release 2.9
  - confirm Merge > Pull request successfully merged and closed
  - Action test the creation of linux appimages, add: _v2.9_rc1
  
  - QtCreator: 
      - checkout katalog_master branch
      - fetch/pull master branch
      
  - update CMakeLists_qt5.txt (if needed, typically when the code has new classes)

## Linux
### pre-build & AppImages
  Test Qt5 and Qt6 Builds prior to release (which will trigger a build)

## Portable versions  


  
## Windows
### AdvancedInstaller
Create installer, move it to the Release fodler and rename

## macOS
  Test Qt5 and Qt6 Builds prior to release (which will trigger a build)




  
## Release communication
### Release notes draft (github)
### Load files (sourceforge)
  load linux files
  load windows files
  load macOS file
  set default windows file
  set default linux file
  set default macos file
### Release publish (sourceforge)
### Release publish (github)
### Release publish (Facebook)

### Load files (sourceforge)
