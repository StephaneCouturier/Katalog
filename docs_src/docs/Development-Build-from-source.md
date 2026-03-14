# Build from source

## **Summary**
This page provides guidance to build Katalog from source.

Katalog now uses **Qt6** and **KF6** librairies, with **cmake** (and not qmake anymore).

A CMakeLists file is also provided to build with Qt5 for older systems like Ubuntu 22.04 LTS.

Main steps to build Katalog:
* Available on any platform (Linux, Windows, Apple)
* Install Qt6 dev librairies (via your software manager, or https://www.qt.io/download, or Craft (Windows)
* Install KF6 librairies via your software manager (via Craft on Windows)
* Download the source code (under development from the katalog_development branch, or latest released from the katalog_master branch)
* Follow the command lines provided hereafter

## **Preparation**
### Required librairies
Katalog is built from Qt6 and KDE KF6 librairies and so glibc >2.8,
but it is also possible to build it on olders versions.

### Code Branches
#### katalog_master
latest released source code

#### katalog_development
code under development, contains bugs & potentially broken features.


### cmake Qt6 KF6 glibc 2.38 and after

#### Base
Based on instructions from: https://develop.kde.org/docs/getting-started/kirigami/setup-cpp/

For openSUSE:
```
zypper install cmake kf6-extra-cmake-modules kf6-kirigami-devel kf6-ki18n-devel kf6-kcoreaddons-devel kf6-kiconthemes-devel qt6-base-devel qt6-declarative-devel qt6-quickcontrols2-devel kf6-qqc2-desktop-style
```

#### Additional devel packages for Katalog
For openSUSE:
```
zypper install qt6-charts-devel qt6-linguist-devel kf6-kxmlgui-devel
```

### cmake Qt5 KF5 glibc before 2.38
Compiling the current state (v2.7 in preparation) with Qt5/KF5 with Kubuntu 22.04 has been done successfully once.
* installed required libs
* disabled translations in cmakelist.txt to focus on debugging the rest
* the code handles 6 code variations to remain compatible, no manual code change needed
* a special CMakeLists.txt file must be used (remove the _qt5 end text, overwritting the default Qt6 one): https://github.com/StephaneCouturier/Katalog/blob/katalog_master/CMakeLists_qt5.txt

#### Base (might be incomplete)
```
build-essential cmake extra-cmake-modules qt5base-dev qt5-declarative-dev
```

#### Additional devel packages for Katalog
```
libqt5charts5-dev qttools5-dev-tools qttools5-dev -y qt6-l10n-tools
```


## **Build**

### set up new build dir
```
mkdir build
```

```
cd build
```

or for a distant build folder:<br/>
```
mkdir /home/user/developments/katalog/build
```

### pre-compile
```
cmake -DCMAKE_BUILD_TYPE=Debug ..
```

or for a distant build folder:<br/>
```
cmake -DCMAKE_BUILD_TYPE=Debug /home/user/developments/katalog/build
```
```
cmake -DCMAKE_BUILD_TYPE=Release /home/user/developments/katalog/build
```

Typical output:
```
stephane@vbox:~/Documents/_Source_Katalog2> cmake -DCMAKE_BUILD_TYPE=Debug .
-- Building Katalog in Debug mode
-- Katalog Configuration Summary:
--   Version: 2.7
--   Qt6 Version: 6.9.0
--   Build Type: Debug
--   C++ Standard: 17
-- Configuring done (0.4s)
-- Generating done (0.2s)
-- Build files have been written to: /home/stephane/Documents/_Source_Katalog2
```
### Translation build commands

#### Update .ts files (not needed to build)
```
cmake --build . --target translations_lupdate
```
#### Compile .qm files
```
cmake --build . --target translations_lrelease
```
#### Copy .qm files to source
```
cmake --build . --target translations_copy
```


### Compile the program
```
make
```

### QtCreator or other IDE
Once the first build is successful, new Katalog builds can usually be launched directly from your QtCreator or the IDE of your choice.



## **Other tool**

### Memory check tool:
```
valgrind --tool=memcheck --leak-check=full --track-origins=yes ./Katalog
```

## **Run**

### Simple run from build directory
```
./Katalog
```

### Run with debugs to a log file
```
./Katalog > debug_output.txt 2>&1
```

### Generate portable

To be documented.



## **Documentation / Docusaurus**
### Build & run local server
```
npm run build
```
```
npm run start -- --locale fr
```



