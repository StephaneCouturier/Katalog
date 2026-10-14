---
id: Dev-Build-from-source
title: Build from source
description: "How to build Katalog 2 and Katalog 3 from source: libraries, cmake options, build folders, translations and documentation"
---

# Build from source
![2.13](https://img.shields.io/badge/Version-2.13%20%2F%203.0-blue)

## **Summary**
This page provides guidance to build Katalog from source.

Katalog uses **Qt6** and **KF6** libraries, with **cmake** (and not qmake anymore).

Main steps to build Katalog:
* Available on any platform (Linux, Windows, Apple)
* Install Qt6 dev libraries (via your software manager, or https://www.qt.io/download, or [Craft](https://community.kde.org/Craft) for Windows or macOS)
* Install KF6 libraries via your software manager (or via Craft)
* Download the source code (latest released from the katalog_master branch or under development from the katalog_development branch)
* Choose which **variant** to build (see below)
* Follow the command lines provided hereafter

## **Source code**
### Variants
One source tree produces 2 programs. The shared, UI-agnostic business logic is built
first as a static library (`core`), and each variant links against it.

| Variant | cmake option | Default | Executable | Version |
|---------|--------------|---------|------------|---------|
| **Katalog 2** — QtWidgets / KXmlGui desktop | `BUILD_QT_WIDGETS` | `OFF` | `qt_widgets/Katalog` | 2.13 |
| **Katalog 3** — QtQuick / QML / Kirigami desktop | `BUILD_QT_QUICK` | `ON` | `qt_quick/Katalog` | 3.0 |

### Code Branches
#### katalog_master
latest released [source code](https://github.com/StephaneCouturier/Katalog)

#### katalog_development
code under development [source code](https://github.com/StephaneCouturier/Katalog/tree/katalog_development) 
<br/>May contain bugs & potentially broken features.


## **Preparation**

### Required libraries
Katalog is built from Qt6 and KDE KF6 libraries and so glibc 2.38 or later,
but it is also possible to build it on older versions.
Commands here below are provided for openSUSE, replace "zypper install" by the package manager command of your system.

### cmake Qt6 KF6 glibc 2.38 and after

#### Base
Based on instructions from: https://develop.kde.org/docs/getting-started/kirigami/setup-cpp/

```
zypper install cmake kf6-extra-cmake-modules kf6-kirigami-devel kf6-ki18n-devel kf6-kcoreaddons-devel kf6-kiconthemes-devel qt6-base-devel qt6-declarative-devel qt6-quickcontrols2-devel kf6-qqc2-desktop-style
```

#### Additional devel packages — both variants
The root `CMakeLists.txt` resolves its dependencies before either variant is selected, so
these are needed whichever one you build. `Qt6::Widgets`, `Charts` and `LinguistTools` are
requested at the root even for a QtQuick-only build.

```
zypper install qt6-charts-devel qt6-linguist-devel kf6-kconfig-devel kf6-kfilemetadata-devel
```

#### Additional devel packages for Katalog 2 only
```
zypper install kf6-kxmlgui-devel
```

#### Additional devel packages for Katalog 3 only
Katalog 3 needs **Qt 6.5 or later**, and adds `KF6I18n`, `KF6Archive`, and the QtCharts
QML module on top of the common packages. Qt Charts is used twice: linked from C++
(common, above), and imported from QML by the Statistics page.

```
zypper install kf6-karchive-devel qt6-charts-imports
```

#### Runtime dependency for Katalog 3
> **Kirigami is a runtime dependency, not a cmake one.** It is imported by QML
> (`org.kde.kirigami`) rather than found by `find_package`, so a missing Kirigami
> produces a *successful build* and a failure at startup, not a configure error.
> The QML module itself ships in `kf6-kirigami-imports` — not in `kf6-kirigami-devel`,
> which contains no QML at all — so on openSUSE both of these are needed for Katalog 3
> to start and to look right:
```
zypper install kf6-kirigami-imports kf6-qqc2-desktop-style
```
> The same applies to `qt6-charts-imports` above: every QML module is a *runtime*
> dependency, so a missing one never shows up as a build failure.

### (legacy) cmake Qt5 KF5 glibc before 2.38

> **Legacy, no longer maintained.** The instructions below were verified once, against
> the v2.7 code base. They are kept for reference and are not covered by current
> development; Katalog 3 does not build with Qt5 at all.

Compiling v2.7 with Qt5/KF5 on Kubuntu 22.04 was done successfully once:
* installed required libs
* disabled translations in cmakelist.txt to focus on debugging the rest
* the code handles 6 code variations to remain compatible, no manual code change needed
* a special Qt5 CMakeLists.txt file was used at the time; it is no longer part of the source tree

#### Base (might be incomplete)
```
build-essential cmake extra-cmake-modules qt5base-dev qt5-declarative-dev
```

#### Additional devel packages for Katalog
```
libqt5charts5-dev qttools5-dev-tools qttools5-dev -y qt6-l10n-tools
```


## **Build**

One folder per combination of version and build type. Create only the one you need now;
each extra combination is added later with the same three steps and leaves the existing
folders untouched.

| Combination | Folder | Result |
|-------------|--------|--------|
| Katalog 2 Debug | `build/Debug-QtWidgets` | `build/Debug-QtWidgets/qt_widgets/Katalog` |
| Katalog 2 Release | `build/Release-QtWidgets` | `build/Release-QtWidgets/qt_widgets/Katalog` |
| Katalog 3 Debug | `build/Debug-QtQuick` | `build/Debug-QtQuick/qt_quick/Katalog` |
| Katalog 3 Release | `build/Release-QtQuick` | `build/Release-QtQuick/qt_quick/Katalog` |

### 1. Set up the build dir
From the Katalog root folder, using the folder name from the table — here Katalog 3 Debug:
```
mkdir -p build/Debug-QtQuick
```

```
cd build/Debug-QtQuick
```

### 2. Configure
Run the line for the combination whose folder you just created. `../..` points back to the
Katalog root, two levels up:

Katalog 2 Debug:
```
cmake -DCMAKE_BUILD_TYPE=Debug -DBUILD_QT_WIDGETS=ON -DBUILD_QT_QUICK=OFF ../..
```

Katalog 2 Release:
```
cmake -DCMAKE_BUILD_TYPE=Release -DBUILD_QT_WIDGETS=ON -DBUILD_QT_QUICK=OFF ../..
```

Katalog 3 Debug:
```
cmake -DCMAKE_BUILD_TYPE=Debug -DBUILD_QT_QUICK=ON -DBUILD_QT_WIDGETS=OFF ../..
```

Katalog 3 Release:
```
cmake -DCMAKE_BUILD_TYPE=Release -DBUILD_QT_QUICK=ON -DBUILD_QT_WIDGETS=OFF ../..
```

Both options are always given, so the command says plainly what it builds. Katalog 3 is
the default (`BUILD_QT_QUICK=ON`, `BUILD_QT_WIDGETS=OFF`), so omitting them builds
Katalog 3.

Typical configure output
```
stephane@leap-dev:\~/Developments/Katalog/Source/build/Debug-QtQuick> cmake -DCMAKE_BUILD_TYPE=Debug -DBUILD_QT_QUICK=ON -DBUILD_QT_WIDGETS=OFF ../..   
-- The CXX compiler identification is GNU 15.3.0   
-- Detecting CXX compiler ABI info   
-- Detecting CXX compiler ABI info - done   
-- Check for working CXX compiler: /usr/bin/c++ - skipped   
-- Detecting CXX compile features   
-- Detecting CXX compile features - done   
-- Performing Test CMAKE_HAVE_LIBC_PTHREAD   
-- Performing Test CMAKE_HAVE_LIBC_PTHREAD - Success   
-- Found Threads: TRUE   
-- Performing Test HAVE_STDATOMIC   
-- Performing Test HAVE_STDATOMIC - Success   
-- Found WrapAtomic: TRUE   
-- Found OpenGL: /usr/lib64/libOpenGL.so   
-- Found WrapOpenGL: TRUE   
-- Found WrapVulkanHeaders: /usr/include   
-- Found Gettext: /usr/bin/msgmerge (found version "0.22.5")
-- Configuring done (1.5s)   
-- Generating done (0.1s)   
-- Build files have been written to: /home/stephane/Developments/Katalog/Source/build/Debug-QtQuick
```

CMake Warning like this can usually be ignored:
```
CMake Warning at /usr/lib64/cmake/Qt6Qml/Qt6QmlMacros.cmake:4293 (message):   
  The qml plugin 'qtchartsqml2plugin' is a dependency of 'Katalog3', but the   
  link target it defines (Qt6::qtchartsqml2) does not exist in the current   
  scope.  The plugin will not be linked.   
Call Stack (most recent call first):   
  /usr/lib64/cmake/Qt6Core/Qt6CoreMacros.cmake:740 (qt6_import_qml_plugins)   
  /usr/lib64/cmake/Qt6Core/Qt6CoreMacros.cmake:740 (cmake_language)   
  /usr/lib64/cmake/Qt6Core/Qt6CoreMacros.cmake:818 (\_qt_internal_finalize_executable)   
  /usr/lib64/cmake/Qt6Core/Qt6CoreMacros.cmake:787:EVAL:1 (qt6_finalize_target)   
  qt_quick/CMakeLists.txt:DEFERRED 
```


### 3. Compile
From inside that same folder:
```
make
```

The program is then at the path shown in the table above.

### Adding another combination later
Repeat the three steps with a different folder name and its matching configure line. The
folders are independent: rebuilding or deleting one never affects the others.

QtCreator or other IDE:

Once the first build is successful, new Katalog builds can usually be launched directly
from your QtCreator or the IDE of your choice. Configure one kit per variant, passing
the same `BUILD_QT_WIDGETS` / `BUILD_QT_QUICK` options as above.

### Run

#### Simple run
Straight after compiling, from inside that build folder:
```
./qt_quick/Katalog
```

Or from the Katalog root, using the path from the table above — here Katalog 3 Debug:
```
./build/Debug-QtQuick/qt_quick/Katalog
```

Replace the folder to run another combination, for example Katalog 2 Release:
```
./build/Release-QtWidgets/qt_widgets/Katalog
```

#### Run capturing debugs in a log file
```
./build/Debug-QtQuick/qt_quick/Katalog > debug_output.txt 2>&1
```

Both variants can open the same collection. They share a settings file only at release:
a pre-release Katalog 3 build (a version string containing `alpha` or `beta`) writes its
own `katalog3_prerelease_settings.ini`, so it starts as a fresh user and never reopens the
collection left open in Katalog 2.



## **Maintenance & other tools**
### Translations

Not part of building from source: compiling the catalogues is already done for you.
`qt_widgets/CMakeLists.txt` declares `add_dependencies(Katalog Katalog_lrelease)`, so the
`.qm` files are rebuilt whenever Katalog 2 is built. Katalog 3 does not compile them: it
bundles the `.qm` files already in `translations/` (`qt_quick/translations.qrc`), so a
Katalog 3 only build uses whatever was last copied there with `translations_copy` below. The commands below are for **changing**
translatable strings, not for producing a working binary.

They must be run from a **Katalog 2 build directory**. The targets come from
`qt6_add_translations()`, which is tied to an executable target and is therefore declared
inside `qt_widgets/CMakeLists.txt` — so a Katalog 3 only build (`BUILD_QT_WIDGETS=OFF`)
has no `translations_*` targets at all. The `.ts` files themselves are shared: they live
in `translations/` and cover both variants, including the K3 `.qml` sources.

#### Update .ts files
After adding or changing a `tr()` / `qsTr()` string, rescan the sources:
```
cmake --build . --target translations_lupdate
```

#### Compile .qm files
Done automatically by a normal build; run it explicitly only to refresh the `.qm` files
without building the program:
```
cmake --build . --target translations_lrelease
```

#### Copy .qm files to source
Copies the compiled `.qm` files from the build directory back into `translations/` in the
source tree, so they can be committed:
```
cmake --build . --target translations_copy
```

### Memory check tool
Use a Debug build — a Release one is optimised and gives poor stack traces. From the
Katalog root:
```
valgrind --tool=memcheck --leak-check=full --track-origins=yes ./build/Debug-QtWidgets/qt_widgets/Katalog
```
```
valgrind --tool=memcheck --leak-check=full --track-origins=yes ./build/Debug-QtQuick/qt_quick/Katalog
```



## **Packages**

Linux packages are AppImage and Flatpak (Flathub); there is no Linux portable build any more.
What a packaged Katalog 3 build must contain is specified in
[K3 Deployment](SpecK3Deployment).



## **Documentation / Docusaurus**
Goal: build & run a local server to test changes to the Documentation pages.


From the Katalog root, go in the docs_src directory
```
cd docs_src
```

Start the server (English)
```
npm run start
```
or start the server (other language fr or cs)
```
npm run start -- --locale fr
```

Check the production build (all languages) — this is the step that catches front matter
and broken-link errors, which the development server does not always report:
```
npm run build
```
