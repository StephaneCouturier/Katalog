#!/bin/bash
#LICENCE
#    This file is part of Katalog
#
#    Copyright (C) 2020, the Katalog Development team
#
#    Author: Stephane Couturier (Symbioxy)
#
#    Katalog is free software; you can redistribute it and/or modify
#    it under the terms of the GNU General Public License as published by
#    the Free Software Foundation; either version 2 of the License, or
#    (at your option) any later version.
#
#    Katalog is distributed in the hope that it will be useful,
#    but WITHOUT ANY WARRANTY; without even the implied warranty of
#    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
#    GNU General Public License for more details.
#
#    You should have received a copy of the GNU General Public License
#    along with Katalog; if not, write to the Free Software
#    Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA
#
#/////////////////////////////////////////////////////////////////////////////
# Application: Katalog
# File Name:   Katalog_create_appimage_local.sh
# Version:     1.1
# Purpose:     Katalog Local AppImage Builder
# Description: Based on the portable script architecture but creates AppImage instead
# Author:      Stephane Couturier
#/////////////////////////////////////////////////////////////////////////////


# Colors for output
RED='\033[0;31m'
GREEN='\033[0;32m'
YELLOW='\033[1;33m'
BLUE='\033[0;34m'
PURPLE='\033[0;35m'
CYAN='\033[0;36m'
NC='\033[0m' # No Color

# ---------------------------------------------------------------------------
# Directory configuration — defaults match the standard dev layout.
# Both can be overridden on the command line:
#   --source <path>   Override SOURCE_DIR
#   --build  <path>   Override BUILD_BASE_DIR
# ---------------------------------------------------------------------------

# SOURCE_DIR — root of the Katalog source tree (contains core/, qt_widgets/, qt_quick/, …)
# This script is meant to be copied/run outside the source tree, so the path is explicit.
# Override with:  --source <path>
SOURCE_DIR="/home/stephane/Documents/Informatique/Katalog/_Source_Katalog2"

# BUILD_BASE_DIR — top-level output directory for AppImage artefacts.
# The cmake build tree goes in BUILD_BASE_DIR/build-appimage/;
# the final .AppImage file is placed directly in BUILD_BASE_DIR/.
BUILD_BASE_DIR="/home/stephane/Developments/Katalog/Application/Portable/AppImage"

# Derived paths — do not edit these; change SOURCE_DIR / BUILD_BASE_DIR above.
# BUILD_DIR is finalised in configure_variant() so K2 and K3 get separate cmake caches.
PROJECT_ROOT="$SOURCE_DIR"           # alias used throughout the script
BUILD_DIR=""                         # set by configure_variant: build-appimage-k2 or build-appimage-k3
APPDIR=""                            # set by configure_variant: $BUILD_DIR/AppDir

# ---------------------------------------------------------------------------
VERBOSE=false
CLEAN_BUILD=false
CMAKE_BUILD_TYPE="Release"
LINUXDEPLOY_DIR="$HOME/.local/share/linuxdeploy"   # linuxdeploy + appimagetool wrapper scripts

# Variant selection (K2 = Qt Widgets, K3 = Qt Quick/QML)
VARIANT="K2"

# Print functions
print_header() {
    echo -e "${BLUE}═══════════════════════════════════════════════════════════════${NC}"
    echo -e "${BLUE}           Katalog Local AppImage Builder${NC}"
    echo -e "${BLUE}═══════════════════════════════════════════════════════════════${NC}"
}

print_step() {
    echo -e "${CYAN}▶ $1${NC}"
}

print_success() {
    echo -e "${GREEN}✅ $1${NC}"
}

print_warning() {
    echo -e "${YELLOW}⚠️ $1${NC}"
}

print_error() {
    echo -e "${RED}❌ $1${NC}"
    exit 1
}

print_info() {
    echo -e "${PURPLE}ℹ️ $1${NC}"
}

# Help function
show_help() {
    echo "Katalog Local AppImage Builder"
    echo ""
    echo "Usage: $0 [options]"
    echo ""
    echo "Options:"
    echo "  -h, --help           Show this help message"
    echo "  -v, --verbose        Enable verbose output"
    echo "  -c, --clean          Clean build (remove build directory first)"
    echo "  -d, --debug          Build in Debug mode instead of Release"
    echo "  --k2                 Build Katalog 2 (Qt Widgets) [default]"
    echo "  --k3                 Build Katalog 3 (Qt Quick / QML)"
    echo "  --install-deps       Install required dependencies"
    echo "  --source <path>      Source tree root  (default: $SOURCE_DIR)"
    echo "  --build  <path>      AppImage output directory (default: $BUILD_BASE_DIR)"
    echo ""
    echo "Examples:"
    echo "  $0                   # Normal build (K2)"
    echo "  $0 --k3              # Build K3"
    echo "  $0 -c                # Clean build"
    echo "  $0 -v -d             # Verbose debug build"
    echo "  $0 --install-deps    # Install dependencies first"
    echo "  $0 --source /path/to/source --build /tmp/appimage-out"
    echo ""
}

# Parse command line arguments
parse_arguments() {
    while [[ $# -gt 0 ]]; do
        case $1 in
            -h|--help)
                show_help
                exit 0
                ;;
            -v|--verbose)
                VERBOSE=true
                shift
                ;;
            -c|--clean)
                CLEAN_BUILD=true
                shift
                ;;
            -d|--debug)
                CMAKE_BUILD_TYPE="Debug"
                shift
                ;;
            --k2)
                VARIANT="K2"
                shift
                ;;
            --k3)
                VARIANT="K3"
                shift
                ;;
            --install-deps)
                install_dependencies
                exit 0
                ;;
            --source)
                SOURCE_DIR="$2"
                PROJECT_ROOT="$SOURCE_DIR"
                shift 2
                ;;
            --build)
                BUILD_BASE_DIR="$2"
                # BUILD_DIR and APPDIR are recalculated in configure_variant
                shift 2
                ;;
            *)
                echo "Unknown option: $1"
                show_help
                exit 1
                ;;
        esac
    done
}

# Configure variant-specific variables
configure_variant() {
    if [ "$VARIANT" = "K3" ]; then
        APP_NAME="Katalog3"
        APP_ID="io.github.stephanecouturier.katalog3"
        BUILD_QT_WIDGETS="OFF"
        BUILD_QT_QUICK="ON"
        CMAKELISTS_PATH="$PROJECT_ROOT/qt_quick/CMakeLists.txt"
        PROJECT_CMAKE_NAME="Katalog3"
        # KF6 packages specific to K3 (no xmlgui/completion; adds archive)
        KF6_EXTRA_ZYPPER="kf6-karchive-devel"
        KF6_EXTRA_APT="libkf6archive-dev"
        QT6_EXTRA_APT="qt6-declarative-dev"
        QT6_EXTRA_ZYPPER="qt6-quick-devel"
    else
        APP_NAME="Katalog"
        APP_ID="io.github.stephanecouturier.katalog"
        BUILD_QT_WIDGETS="ON"
        BUILD_QT_QUICK="OFF"
        CMAKELISTS_PATH="$PROJECT_ROOT/qt_widgets/CMakeLists.txt"
        PROJECT_CMAKE_NAME="Katalog"
        # KF6 packages specific to K2
        KF6_EXTRA_ZYPPER="kf6-kxmlgui-devel kf6-kcompletion-devel"
        KF6_EXTRA_APT="libkf6xmlgui-dev libkf6completion-dev"
        QT6_EXTRA_APT=""
        QT6_EXTRA_ZYPPER=""
    fi

    # Each variant gets its own build directory so cmake caches never cross-contaminate.
    BUILD_DIR="$BUILD_BASE_DIR/build-appimage-${VARIANT,,}"
    APPDIR="$BUILD_DIR/AppDir"
}

# Check dependencies
check_dependencies() {
    print_step "Checking dependencies"

    local missing_deps=()

    # Check basic build tools
    for cmd in cmake ninja qmake6 ldd file; do
        if ! command -v $cmd &> /dev/null; then
            # Try alternative names
            case $cmd in
                ninja)
                    if ! command -v ninja-build &> /dev/null; then
                        missing_deps+=($cmd)
                    fi
                    ;;
                qmake6)
                    if ! command -v qmake-qt6 &> /dev/null && ! command -v qmake &> /dev/null; then
                        missing_deps+=($cmd)
                    fi
                    ;;
                *)
                    missing_deps+=($cmd)
                    ;;
            esac
        fi
    done

    # Check Qt6 development packages
    if ! pkg-config --exists Qt6Core Qt6Gui Qt6Sql Qt6Charts Qt6Network 2>/dev/null; then
        missing_deps+=("qt6-dev-packages")
    fi

    # Check KF6 packages - different approach for different distros
    local kf6_found=false

    if command -v zypper &> /dev/null; then
        # openSUSE: Check if KF6 devel packages are installed
        if zypper search --installed-only kf6-kcoreaddons-devel kf6-ki18n-devel kf6-kconfig-devel kf6-kiconthemes-devel $KF6_EXTRA_ZYPPER &>/dev/null; then
            kf6_found=true
        fi
    else
        # Ubuntu/Debian: Check pkg-config
        if pkg-config --exists KF6CoreAddons KF6I18n KF6Config KF6IconThemes 2>/dev/null; then
            kf6_found=true
        fi
    fi

    if [ "$kf6_found" = false ]; then
        missing_deps+=("kf6-dev-packages")
    fi

    # Check linuxdeploy tools — auto-install if missing.
    # Check squashfs-root content, not just wrapper scripts: a previous failed run
    # may have created the wrappers but left the squashfs-root directories empty.
    if [ ! -f "$LINUXDEPLOY_DIR/linuxdeploy-extracted/squashfs-root/AppRun" ] || \
       [ ! -f "$LINUXDEPLOY_DIR/linuxdeploy-qt-extracted/squashfs-root/AppRun" ] || \
       [ ! -f "$LINUXDEPLOY_DIR/runtime-x86_64" ]; then
        print_warning "linuxdeploy tools not found or incomplete — downloading/extracting now..."
        install_linuxdeploy
    fi

    if [ ${#missing_deps[@]} -ne 0 ]; then
        print_error "Missing dependencies: ${missing_deps[*]}"
        echo ""
        echo "Run: $0 --install-deps"
        echo ""
        if command -v zypper &> /dev/null; then
            echo "Or install manually on openSUSE:"
            echo "  sudo zypper install cmake ninja qt6-base-devel qt6-charts-devel qt6-tools-devel $QT6_EXTRA_ZYPPER kf6-kcoreaddons-devel kf6-ki18n-devel kf6-kconfig-devel kf6-kiconthemes-devel $KF6_EXTRA_ZYPPER desktop-file-utils appstream-glib-devel ImageMagick"
        else
            echo "Or install manually on Ubuntu/Debian:"
            echo "  sudo apt install cmake ninja-build qt6-base-dev qt6-charts-dev qt6-tools-dev $QT6_EXTRA_APT libkf6coreaddons-dev libkf6i18n-dev libkf6config-dev libkf6iconthemes-dev $KF6_EXTRA_APT desktop-file-utils appstream-util imagemagick"
        fi
        exit 1
    fi

    print_success "All dependencies satisfied"
}

# Install dependencies
install_dependencies() {
    print_step "Installing dependencies"

    # Detect package manager
    if command -v zypper &> /dev/null; then
        install_deps_opensuse
    elif command -v apt &> /dev/null; then
        install_deps_ubuntu
    else
        print_error "Unsupported package manager. Please install dependencies manually."
    fi

    # Install linuxdeploy
    install_linuxdeploy

    print_success "Dependencies installed"
}

# Install dependencies for openSUSE
install_deps_opensuse() {
    print_info "Installing packages for openSUSE ($VARIANT)..."
    sudo zypper refresh
    sudo zypper install -y \
        cmake \
        ninja \
        qt6-base-devel \
        qt6-tools-devel \
        qt6-charts-devel \
        qt6-sql-sqlite \
        qt6-networkauth-devel \
        $QT6_EXTRA_ZYPPER \
        kf6-kcoreaddons-devel \
        kf6-ki18n-devel \
        kf6-kconfig-devel \
        kf6-kiconthemes-devel \
        $KF6_EXTRA_ZYPPER \
        desktop-file-utils \
        appstream-glib-devel \
        ImageMagick \
        wget \
        file \
        squashfs \
        qt6-positioning
}

# Install dependencies for Ubuntu/Debian
install_deps_ubuntu() {
    print_info "Installing packages for Ubuntu/Debian ($VARIANT)..."
    sudo apt update
    sudo apt install -y \
        cmake \
        ninja-build \
        qt6-base-dev \
        qt6-base-dev-tools \
        qt6-tools-dev \
        qt6-tools-dev-tools \
        qt6-l10n-tools \
        qt6-charts-dev \
        libqt6sql6-sqlite \
        qt6-networkauth-dev \
        $QT6_EXTRA_APT \
        libkf6coreaddons-dev \
        libkf6i18n-dev \
        libkf6config-dev \
        libkf6iconthemes-dev \
        $KF6_EXTRA_APT \
        desktop-file-utils \
        appstream-util \
        imagemagick \
        squashfs-tools \
        wget \
        file
}

# Extract an AppImage to squashfs-root/ in the current directory.
# First tries the AppImage's own --appimage-extract (needs libfuse2 on the host).
# Falls back to unsquashfs on the raw SquashFS payload (no FUSE required).
# This fallback is essential on openSUSE Tumbleweed and other distros that ship
# only libfuse3 — many AppImages (including appimagetool itself) still embed the
# old runtime that requires libfuse.so.2.
extract_appimage() {
    local appimage_path="$1"   # absolute path to the .AppImage file
    local dest_label="$2"      # human-readable name for log messages

    # Attempt 1: built-in extraction (works when libfuse2 is available)
    if "$appimage_path" --appimage-extract 2>/dev/null && [ -d "squashfs-root" ]; then
        print_success "Extracted $dest_label via --appimage-extract"
        return 0
    fi

    # Attempt 2: unsquashfs on the raw SquashFS payload.
    # AppImages are [ELF runtime][SquashFS]; find the SquashFS magic 'hsqs'
    # (little-endian) to get its byte offset, then feed it to unsquashfs.
    local unsquashfs_bin
    unsquashfs_bin=$(command -v unsquashfs 2>/dev/null || echo "")
    if [ -n "$unsquashfs_bin" ]; then
        # 'hsqs' (SquashFS little-endian magic) can appear multiple times in an
        # AppImage — the first hit is often inside the ELF body and is invalid.
        # Iterate over ALL occurrences until unsquashfs accepts one.
        local offsets_list tried_count=0
        offsets_list=$(LANG=C grep -boa 'hsqs' "$appimage_path" 2>/dev/null | cut -d: -f1)
        if [ -z "$offsets_list" ]; then
            print_error "No SquashFS magic found in $dest_label — file may be corrupt. Re-run --install-deps."
        fi
        for try_offset in $offsets_list; do
            tried_count=$((tried_count + 1))
            rm -rf squashfs-root
            if "$unsquashfs_bin" -o "$try_offset" -d squashfs-root "$appimage_path" 2>/dev/null \
               && [ -f "squashfs-root/AppRun" ]; then
                print_success "Extracted $dest_label via unsquashfs (offset $try_offset)"
                return 0
            fi
        done
        rm -rf squashfs-root
        print_error "unsquashfs tried $tried_count offset(s) — none produced a valid AppDir for $dest_label."
    else
        print_error "unsquashfs not found. Run: sudo zypper install squashfs   (openSUSE) or   sudo apt install squashfs-tools   (Ubuntu)"
    fi
}

# Install linuxdeploy + type2-runtime.
# We do NOT download a separate appimagetool — linuxdeploy bundles its own.
# The type2-runtime is cached to ~/.cache/appimagetool/runtime-x86_64, which
# linuxdeploy's internal appimagetool checks automatically before packaging.
install_linuxdeploy() {
    print_step "Installing linuxdeploy and type2-runtime"

    mkdir -p "$LINUXDEPLOY_DIR"
    cd "$LINUXDEPLOY_DIR"

    if [ ! -f "linuxdeploy-x86_64.AppImage" ]; then
        wget -c "https://github.com/linuxdeploy/linuxdeploy/releases/download/continuous/linuxdeploy-x86_64.AppImage"
        chmod +x linuxdeploy-x86_64.AppImage
    fi

    if [ ! -f "linuxdeploy-plugin-qt-x86_64.AppImage" ]; then
        wget -c "https://github.com/linuxdeploy/linuxdeploy-plugin-qt/releases/download/continuous/linuxdeploy-plugin-qt-x86_64.AppImage"
        chmod +x linuxdeploy-plugin-qt-x86_64.AppImage
    fi

    if [ ! -f "runtime-x86_64" ]; then
        wget -q "https://github.com/AppImage/type2-runtime/releases/download/continuous/runtime-x86_64" \
            -O runtime-x86_64
        chmod +x runtime-x86_64
    fi

    # Extract AppImages — uses unsquashfs fallback when libfuse2 is unavailable
    if [ ! -d "linuxdeploy-extracted/squashfs-root" ]; then
        mkdir -p linuxdeploy-extracted
        cd linuxdeploy-extracted
        extract_appimage "$LINUXDEPLOY_DIR/linuxdeploy-x86_64.AppImage" "linuxdeploy"
        cd ..
    fi

    if [ ! -d "linuxdeploy-qt-extracted/squashfs-root" ]; then
        mkdir -p linuxdeploy-qt-extracted
        cd linuxdeploy-qt-extracted
        extract_appimage "$LINUXDEPLOY_DIR/linuxdeploy-plugin-qt-x86_64.AppImage" "linuxdeploy-plugin-qt"
        cd ..
    fi

    # Create wrapper scripts
    cat > linuxdeploy << EOF
#!/bin/bash
exec "$LINUXDEPLOY_DIR/linuxdeploy-extracted/squashfs-root/AppRun" "\$@"
EOF

    cat > linuxdeploy-plugin-qt << EOF
#!/bin/bash
exec "$LINUXDEPLOY_DIR/linuxdeploy-qt-extracted/squashfs-root/AppRun" "\$@"
EOF

    chmod +x linuxdeploy linuxdeploy-plugin-qt

    print_success "linuxdeploy and type2-runtime installed to $LINUXDEPLOY_DIR"
}

# Clean build directory
clean_build() {
    if [ "$CLEAN_BUILD" = true ] && [ -d "$BUILD_DIR" ]; then
        print_step "Cleaning build directory"
        rm -rf "$BUILD_DIR"
        print_success "Build directory cleaned"
    fi
}

# Setup build directory
setup_build_dir() {
    print_step "Setting up build directory"

    # Create the base build directory structure
    mkdir -p "$BUILD_BASE_DIR"
    mkdir -p "$BUILD_DIR"
    cd "$BUILD_DIR"

    print_success "Build directory ready: $BUILD_DIR"
    print_info "Build base directory: $BUILD_BASE_DIR"
}

# Build translations
build_translations() {
    print_step "Building translations"

    # Build translations first (if available)
    if ninja translations_lrelease 2>/dev/null || ninja ${APP_NAME}_lrelease 2>/dev/null; then
        print_success "Translations built successfully"
    else
        print_warning "Translation build failed, creating empty .qm files"
        # Create empty .qm files for missing translations
        for ts_file in "$PROJECT_ROOT"/translations/*.ts; do
            if [ -f "$ts_file" ]; then
                qm_file="${ts_file%.ts}.qm"
                if [ ! -f "$qm_file" ]; then
                    [ "$VERBOSE" = true ] && echo "Creating empty: $qm_file"
                    touch "$qm_file"
                fi
            fi
        done
    fi
}

# Configure and build
build_application() {
    print_step "Configuring CMake"

    local cmake_args=(
        "$PROJECT_ROOT"
        -GNinja
        -DCMAKE_BUILD_TYPE="$CMAKE_BUILD_TYPE"
        -DCMAKE_INSTALL_PREFIX=/usr
        -DBUILD_QT_WIDGETS="$BUILD_QT_WIDGETS"
        -DBUILD_QT_QUICK="$BUILD_QT_QUICK"
        -DBUILD_TESTS=OFF
    )

    if [ "$VERBOSE" = true ]; then
        cmake_args+=(-DCMAKE_VERBOSE_MAKEFILE=ON)
    fi

    cmake "${cmake_args[@]}" || print_error "CMake configuration failed"

    print_step "Building $APP_NAME ($CMAKE_BUILD_TYPE mode)"

    # Build translations first
    build_translations

    # Build main application
    if [ "$VERBOSE" = true ]; then
        ninja -v
    else
        ninja
    fi

    if [ $? -ne 0 ]; then
        print_error "Build failed"
    fi

    print_success "Build completed"
}

# Install to AppDir
install_to_appdir() {
    print_step "Installing to AppDir"

    mkdir -p "$APPDIR"
    DESTDIR="$APPDIR" ninja install

    print_success "Installed to AppDir"
}

# Prepare desktop integration files
prepare_desktop_files() {
    print_step "Preparing desktop integration files"

    # Create directories
    mkdir -p "$APPDIR/usr/share/applications"
    mkdir -p "$APPDIR/usr/share/metainfo"
    mkdir -p "$APPDIR/usr/share/icons/hicolor/64x64/apps"

    # Create desktop file
    cat > "$APPDIR/usr/share/applications/$APP_NAME.desktop" << EOF
[Desktop Entry]
Type=Application
Name=$APP_NAME
GenericName=File Catalog Manager
Comment=Create and manage catalogs of your files and folders
Comment[fr]=Créer et gérer des catalogues de vos fichiers et dossiers
Exec=$APP_NAME
Icon=$APP_NAME
Terminal=false
Categories=Utility;System;FileManager;
Keywords=catalog;files;folders;organization;storage;backup;
StartupNotify=true
StartupWMClass=$APP_NAME
EOF

    # Create AppData file
    cat > "$APPDIR/usr/share/metainfo/$APP_ID.appdata.xml" << EOF
<?xml version="1.0" encoding="UTF-8"?>
<component type="desktop-application">
  <id>$APP_ID</id>
  <metadata_license>MIT</metadata_license>
  <project_license>GPL-3.0+</project_license>
  <name>$APP_NAME</name>
  <summary>File Catalog Manager</summary>
  <description>
    <p>
      Katalog is a file catalog manager that helps you organize and track your files and folders across multiple storage devices.
    </p>
  </description>
  <screenshots>
    <screenshot type="default">
      <caption>Main interface</caption>
      <image>https://stephanecouturier.github.io/Katalog/img/Katalog_logo_1.20.png</image>
    </screenshot>
  </screenshots>
  <categories>
    <category>Utility</category>
    <category>FileManager</category>
    <category>System</category>
  </categories>
  <url type="homepage">https://stephanecouturier.github.io/Katalog</url>
  <url type="bugtracker">https://github.com/StephaneCouturier/Katalog/issues</url>
  <launchable type="desktop-id">$APP_ID.desktop</launchable>
  <provides>
    <binary>$APP_NAME</binary>
  </provides>
  <content_rating type="oars-1.1"/>
</component>
EOF

    # Create/copy icon
    if [ -f "$PROJECT_ROOT/assets/Katalog_logo_64.ico" ]; then
        # Convert ICO to PNG with exact 64x64 size
        convert "$PROJECT_ROOT/assets/Katalog_logo_64.ico[0]" -resize 64x64! "$APPDIR/usr/share/icons/hicolor/64x64/apps/$APP_NAME.png" 2>/dev/null || {
            print_warning "ICO conversion failed, creating fallback icon"
            create_fallback_icon
        }
    else
        print_warning "ICO file not found, creating fallback icon"
        create_fallback_icon
    fi

    # Validate files
    desktop-file-validate "$APPDIR/usr/share/applications/$APP_NAME.desktop" 2>/dev/null || print_warning "Desktop file validation warnings"
    appstream-util validate "$APPDIR/usr/share/metainfo/$APP_ID.appdata.xml" 2>/dev/null || print_warning "AppData validation warnings"

    print_success "Desktop integration files prepared"
}

# Create fallback icon
create_fallback_icon() {
    convert -size 64x64 xc:steelblue -gravity center -pointsize 28 -fill white -font DejaVu-Sans-Bold -annotate +0+0 "K" "$APPDIR/usr/share/icons/hicolor/64x64/apps/$APP_NAME.png"
}

# Manually deploy QML modules for K3 (Qt Quick) builds.
# linuxdeploy-plugin-qt often fails to bundle QML plugins for Qt6 even
# with QMLDIR set, so we copy the required module directories explicitly
# and patch AppRun to export the QML import path at runtime.
deploy_k3_qml_modules() {
    print_step "Deploying K3 QML modules"

    # Find Qt6 QML installation directory
    local qt_qml_dir
    qt_qml_dir=$(qmake6 -query QT_INSTALL_QML 2>/dev/null || \
                 qmake -query QT_INSTALL_QML 2>/dev/null)

    if [ -z "$qt_qml_dir" ] || [ ! -d "$qt_qml_dir" ]; then
        # Try common locations
        for dir in \
            /usr/lib/x86_64-linux-gnu/qt6/qml \
            /usr/lib/qt6/qml \
            /usr/lib64/qt6/qml; do
            if [ -d "$dir" ]; then
                qt_qml_dir="$dir"
                break
            fi
        done
    fi

    if [ -z "$qt_qml_dir" ] || [ ! -d "$qt_qml_dir" ]; then
        print_warning "Could not find Qt6 QML directory — QML modules will not be bundled"
        return
    fi

    print_info "Qt6 QML directory: $qt_qml_dir"

    local dest="$APPDIR/usr/qml"
    mkdir -p "$dest"

    # Qt Quick modules required by K3
    local modules=(
        "QtQuick"
        "QtQuick/Controls"
        "QtQuick/Layouts"
        "QtQuick/Dialogs"
        "Qt/labs/settings"
        "Qt/labs/platform"
    )

    for module in "${modules[@]}"; do
        local src="$qt_qml_dir/$module"
        local dst_parent="$dest/$(dirname "$module")"
        if [ -d "$src" ]; then
            mkdir -p "$dst_parent"
            cp -rn "$src" "$dst_parent/" 2>/dev/null || true
            print_success "Bundled: $module"
        else
            print_warning "Not found: $src"
        fi
    done

    # Kirigami QML plugin
    local kirigami_src="$qt_qml_dir/org/kde/kirigami"
    if [ -d "$kirigami_src" ]; then
        mkdir -p "$dest/org/kde"
        cp -rn "$kirigami_src" "$dest/org/kde/"
        print_success "Bundled: org.kde.kirigami"
    else
        print_warning "Kirigami QML not found at $kirigami_src"
        print_info "Install libkf6kirigami-dev (or kirigami2) to bundle Kirigami"
    fi

    # Patch AppRun to export QML import paths at runtime.
    # linuxdeploy's AppRun uses $HERE for the AppImage mount point.
    local apprun="$APPDIR/AppRun"
    if [ -f "$apprun" ] && ! grep -q "QML_IMPORT_PATH" "$apprun"; then
        sed -i 's|^exec |export QML2_IMPORT_PATH="${HERE}/usr/qml${QML2_IMPORT_PATH:+:$QML2_IMPORT_PATH}"\nexport QML_IMPORT_PATH="${HERE}/usr/qml${QML_IMPORT_PATH:+:$QML_IMPORT_PATH}"\nexec |' "$apprun"
        print_success "Patched AppRun with QML import paths"
    fi

    print_success "K3 QML modules deployed"
}

# Write a custom AppRun into AppDir BEFORE linuxdeploy runs.
# linuxdeploy respects a pre-existing AppRun and will not overwrite it.
# This fixes two issues on non-KDE desktops (e.g. Cinnamon):
#   - SIGSEGV: QT_QPA_PLATFORMTHEME set by the session (kde/qt6ct/gnome) points
#     to a plugin not bundled in the AppImage.  Qt6 auto-detects KDE via
#     KDE_SESSION_VERSION, so unsetting the var on non-KDE desktops is safe.
#   - KF6 data lookup: XDG_DATA_DIRS must include the AppDir share path.
write_apprun() {
    print_step "Writing custom AppRun"
    {
        printf '#!/bin/bash\n'
        printf 'HERE="$(dirname "$(readlink -f "${0}")")"\n'
        printf 'export PATH="${HERE}/usr/bin${PATH:+:$PATH}"\n'
        printf 'export LD_LIBRARY_PATH="${HERE}/usr/lib${LD_LIBRARY_PATH:+:$LD_LIBRARY_PATH}"\n'
        printf 'export QT_PLUGIN_PATH="${HERE}/usr/plugins${QT_PLUGIN_PATH:+:$QT_PLUGIN_PATH}"\n'
        printf 'export XDG_DATA_DIRS="${HERE}/usr/share${XDG_DATA_DIRS:+:$XDG_DATA_DIRS}"\n'
        if [ "$VARIANT" = "K3" ]; then
            printf 'export QML2_IMPORT_PATH="${HERE}/usr/qml${QML2_IMPORT_PATH:+:$QML2_IMPORT_PATH}"\n'
            printf 'export QML_IMPORT_PATH="${HERE}/usr/qml${QML_IMPORT_PATH:+:$QML_IMPORT_PATH}"\n'
        fi
        printf 'if [ -z "$KDE_SESSION_VERSION" ] && [ -z "$KDE_FULL_SESSION" ]; then\n'
        printf '    unset QT_QPA_PLATFORMTHEME\n'
        printf 'fi\n'
        printf 'exec "${HERE}/usr/bin/%s" "$@"\n' "$APP_NAME"
    } > "$APPDIR/AppRun"
    chmod +x "$APPDIR/AppRun"
    print_success "AppRun written"
}

# Create AppImage
create_appimage() {
    print_step "Creating AppImage"

    export PATH="$LINUXDEPLOY_DIR:$PATH"

    # Tell linuxdeploy-plugin-qt which Qt to use and set LD_LIBRARY_PATH so
    # the plugin's ldd calls resolve Qt from the correct installation.
    # On openSUSE, Qt lives in /usr/lib64; without this, the plugin may bundle
    # a different version than the binary was compiled against.
    local qmake_bin
    qmake_bin=$(command -v qmake6 2>/dev/null \
             || command -v qmake-qt6 2>/dev/null \
             || command -v qmake 2>/dev/null)
    export QMAKE="$qmake_bin"
    local qt_install_libs
    qt_install_libs=$(env -i PATH="$PATH" "$qmake_bin" -query QT_INSTALL_LIBS 2>/dev/null)
    if [ -n "$qt_install_libs" ] && [ -d "$qt_install_libs" ]; then
        export LD_LIBRARY_PATH="$qt_install_libs${LD_LIBRARY_PATH:+:$LD_LIBRARY_PATH}"
        print_info "Qt libs path: $qt_install_libs"
    fi
    print_info "Using qmake: $QMAKE ($("$QMAKE" -query QT_VERSION 2>/dev/null || echo 'version unknown'))"

    # Get version info
    local version
    version=$(grep -E "^project\($PROJECT_CMAKE_NAME VERSION" "$CMAKELISTS_PATH" | sed -E 's/.*VERSION ([0-9.]+).*/\1/')
    [ -z "$version" ] && version="x.x"
    local appimage_name="$APP_NAME-$version-$(date +%Y%m%d-%H%M)-x86_64.AppImage"

    # Verify AppDir structure
    if [ ! -f "$APPDIR/usr/bin/$APP_NAME" ]; then
        print_error "$APP_NAME executable not found in AppDir"
    fi

    if [ "$VERBOSE" = true ]; then
        echo "AppDir contents:"
        find "$APPDIR" -type f | head -20
    fi

    # Step 1: Write custom AppRun before linuxdeploy (linuxdeploy won't overwrite it)
    write_apprun

    # Step 2: Run linuxdeploy to bundle Qt/system dependencies
    print_info "Running linuxdeploy to bundle dependencies..."
    linuxdeploy \
        --appdir "$APPDIR" \
        --plugin qt \
        --executable "$APPDIR/usr/bin/$APP_NAME" \
        --desktop-file "$APPDIR/usr/share/applications/$APP_NAME.desktop" \
        --icon-file "$APPDIR/usr/share/icons/hicolor/64x64/apps/$APP_NAME.png"

    if [ $? -ne 0 ]; then
        print_error "Dependency bundling failed"
    fi

    # Step 2b: Report the Qt version bundled by linuxdeploy.
    # We no longer replace Qt libs here. linuxdeploy resolves Qt via ldd on the
    # Katalog binary, picking up the RUNTIME system Qt — the same version KF6 was
    # compiled against. Replacing with qmake6 -query QT_INSTALL_LIBS (the dev Qt
    # path) caused KF6 to crash because the dev Qt may omit backward-compat
    # private API symbols that the runtime Qt includes.
    local bundled_qt_ver
    bundled_qt_ver=$(strings "$APPDIR/usr/lib/libQt6Core.so.6" 2>/dev/null \
        | grep -E "^6\.[0-9]+\.[0-9]+$" | head -1)
    print_info "Bundled Qt: ${bundled_qt_ver:-unknown}  |  Build Qt: $("$qmake_bin" -query QT_VERSION 2>/dev/null)"

    # Step 2c: Strip plugins/libs not used by Katalog.
    # KF6FileMetaData pulls in GStreamer, FFmpeg, and PulseAudio as transitive
    # deps — ~150 MB that Katalog never loads at runtime.
    print_step "Stripping unused multimedia plugins"
    rm -rf "$APPDIR/usr/plugins/multimedia"
    rm -rf "$APPDIR/usr/plugins/geoservices"
    rm -rf "$APPDIR/usr/plugins/position"
    # Strip GStreamer/FFmpeg/audio libs (not needed for file cataloging)
    for lib in \
        libavcodec libavformat libavutil libswresample libswscale \
        libgst libgstreamer libgst-1.0 \
        libpulse libpulsecommon \
        libshaderc libSPIRV libglslang \
        libsndfile libFLAC libogg libvorbis \
        libgudev libdw libelf liborc; do
        find "$APPDIR/usr/lib" -maxdepth 1 -name "${lib}*.so*" -delete 2>/dev/null || true
    done
    print_success "Multimedia plugins removed"

    # Step 3: For K3, manually deploy QML modules
    if [ "$VARIANT" = "K3" ]; then
        deploy_k3_qml_modules
    fi

    # Step 4: Package as AppImage using linuxdeploy's bundled appimagetool with
    # the type2 runtime (fuse2 + fuse3 + fuse-free bwrap fallback).
    # We avoid downloading a separate appimagetool AppImage entirely — it uses a
    # newer format that cannot be reliably extracted without FUSE on the build host.
    # Instead: cache the type2-runtime to ~/.cache/appimagetool/runtime-x86_64,
    # the path linuxdeploy's own appimagetool checks automatically before embedding.
    local runtime_bin="$LINUXDEPLOY_DIR/runtime-x86_64"
    if [ ! -f "$runtime_bin" ]; then
        print_error "type2-runtime not found. Run: $0 --install-deps"
    fi
    # Use appimagetool bundled inside linuxdeploy's squashfs directly with
    # --runtime-file, bypassing its automatic (and slow/broken) GitHub download.
    local ld_appimagetool
    ld_appimagetool=$(find "$LINUXDEPLOY_DIR/linuxdeploy-extracted/squashfs-root" \
        -name "appimagetool" -type f 2>/dev/null | head -1)
    if [ -z "$ld_appimagetool" ] || [ ! -x "$ld_appimagetool" ]; then
        print_error "Could not find appimagetool inside linuxdeploy squashfs at $LINUXDEPLOY_DIR/linuxdeploy-extracted/squashfs-root"
    fi
    print_info "Using appimagetool: $ld_appimagetool"
    print_info "Packaging with type2-runtime (local, no download)..."
    ARCH=x86_64 "$ld_appimagetool" \
        --runtime-file "$runtime_bin" \
        "$APPDIR" \
        "$appimage_name"

    if [ $? -ne 0 ]; then
        print_error "AppImage creation failed"
    fi

    # linuxdeploy names the output file after the desktop file Name + ARCH,
    # e.g. Katalog-x86_64.AppImage. Rename to our versioned filename.
    local created_appimage=""
    if [ -f "$appimage_name" ]; then
        created_appimage="$appimage_name"
    else
        local found
        found=$(ls ${APP_NAME}-*.AppImage 2>/dev/null | head -1)
        if [ -n "$found" ]; then
            mv "$found" "$appimage_name"
            created_appimage="$appimage_name"
        fi
    fi

    if [ -n "$created_appimage" ]; then
        print_success "AppImage created: $created_appimage"

        echo ""
        echo "📦 AppImage Information:"
        ls -lh "$created_appimage"
        file "$created_appimage"

        print_step "Testing AppImage"
        chmod +x "$created_appimage"
        if ./"$created_appimage" --version 2>/dev/null; then
            print_success "AppImage basic test passed"
        else
            print_warning "AppImage version test failed (not critical)"
        fi

        # Move AppImage to build base directory (final location)
        mv "$created_appimage" "$BUILD_BASE_DIR/"
        print_success "AppImage saved to: $BUILD_BASE_DIR/$created_appimage"

        echo ""
        print_info "To test:"
        echo "  cd $BUILD_BASE_DIR"
        echo "  ./$created_appimage"
        echo ""

    else
        print_error "AppImage file not found after creation"
    fi
}

# Main function
main() {
    print_header

    parse_arguments "$@"
    configure_variant

    echo "Build Configuration:"
    echo "  Variant:      $VARIANT ($APP_NAME)"
    echo "  Project Root: $PROJECT_ROOT"
    echo "  Build Base Dir: $BUILD_BASE_DIR"
    echo "  Build Dir: $BUILD_DIR"
    echo "  Build Type: $CMAKE_BUILD_TYPE"
    echo "  Clean Build: $CLEAN_BUILD"
    echo "  Verbose: $VERBOSE"
    echo "  type2-runtime: $([ -f "$LINUXDEPLOY_DIR/runtime-x86_64" ] && echo "✅ Found" || echo "⬇️  Will be downloaded")"
    echo ""

    check_dependencies
    clean_build
    setup_build_dir
    build_application
    install_to_appdir
    prepare_desktop_files
    create_appimage

    echo ""
    print_success "Local AppImage build completed! 🎉"
}

# Run if called directly
if [[ "${BASH_SOURCE[0]}" == "${0}" ]]; then
    main "$@"
fi
