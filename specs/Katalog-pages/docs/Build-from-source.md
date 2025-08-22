# Build from source

## Summary
This page provides guidance to build Katalog from source.

Katalog now uses **Qt6** and **KF6** librairies, with cmake (and not qmake anymore).

Main steps to build Katalog:
* Available on any platform (Linux, Windows, Apple)
* Install Qt: https://www.qt.io/download
* Install KF6 librairies (via Craft on Windows)
* Download the source code
* Follow command lines provided hereafter



## Build

### set up new build dir

cmake -DCMAKE_BUILD_TYPE=Debug /home/stephane/Documents/Informatique/Katalog/\_Source_Katalog2
cmake -DCMAKE_BUILD_TYPE=Release /home/stephane/Documents/Informatique/Katalog/\_Source_Katalog2
make

## Translation build commands

### Update .ts files (not needed to build)

cmake --build . --target translations_lupdate

### Compile .qm files

cmake --build . --target translations_lrelease

### Copy .qm files to source

cmake --build . --target translations_copy

## Other

##Memory check tool:
valgrind --tool=memcheck --leak-check=full --track-origins=yes ./Katalog
