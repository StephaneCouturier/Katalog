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
# Version:     1.0
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

# Configuration
SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
PROJECT_ROOT="$(dirname "$(dirname "$SCRIPT_DIR")")"
# Use dedicated build directory outside of synced source
BUILD_BASE_DIR="/home/shared/Development/Katalog/Build/AppImage"
BUILD_DIR="$BUILD_BASE_DIR/build-appimage"
APPDIR="$BUILD_DIR/AppDir"
VERBOSE=false
CLEAN_BUILD=false
CMAKE_BUILD_TYPE="Release"
LINUXDEPLOY_DIR="$HOME/.local/share/linuxdeploy"
APPIMAGETOOL_PATH="$BUILD_BASE_DIR/appimagetool-x86_64.AppImage"

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
    echo "  -h, --help          Show this help message"
    echo "  -v, --verbose       Enable verbose output"
    echo "  -c, --clean         Clean build (remove build directory first)"
    echo "  -d, --debug         Build in Debug mode instead of Release"
    echo "  --install-deps      Install required dependencies"
    echo ""
    echo "Examples:"
    echo "  $0                  # Normal build"
    echo "  $0 -c               # Clean build"
    echo "  $0 -v -d            # Verbose debug build"
    echo "  $0 --install-deps   # Install dependencies first"
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
            --install-deps)
                install_dependencies
                exit 0
                ;;
            *)
                echo "Unknown option: $1"
                show_help
                exit 1
                ;;
        esac
    done
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
    if ! pkg-config --exists Qt6Core Qt6Widgets Qt6Gui Qt6Sql Qt6Charts Qt6Network 2>/dev/null; then
        missing_deps+=("qt6-dev-packages")
    fi

    # Check KF6 packages - different approach for different distros
    local kf6_found=false

    if command -v zypper &> /dev/null; then
        # openSUSE: Check if KF6 devel packages are installed
        if zypper search --installed-only kf6-kcoreaddons-devel kf6-ki18n-devel kf6-kxmlgui-devel kf6-kconfig-devel kf6-kiconthemes-devel &>/dev/null; then
            kf6_found=true
        fi
    else
        # Ubuntu/Debian: Check pkg-config
        if pkg-config --exists KF6CoreAddons KF6I18n KF6XmlGui KF6Config KF6IconThemes 2>/dev/null; then
            kf6_found=true
        fi
    fi

    if [ "$kf6_found" = false ]; then
        missing_deps+=("kf6-dev-packages")
    fi

    # Check linuxdeploy
    if [ ! -f "$LINUXDEPLOY_DIR/linuxdeploy" ]; then
        missing_deps+=("linuxdeploy")
    fi

    if [ ${#missing_deps[@]} -ne 0 ]; then
        print_error "Missing dependencies: ${missing_deps[*]}"
        echo ""
        echo "Run: $0 --install-deps"
        echo ""
        if command -v zypper &> /dev/null; then
            echo "Or install manually on openSUSE:"
            echo "  sudo zypper install cmake ninja qt6-base-devel qt6-charts-devel qt6-tools-devel kf6-kcoreaddons-devel kf6-ki18n-devel kf6-kxmlgui-devel kf6-kconfig-devel kf6-kiconthemes-devel desktop-file-utils appstream-glib-devel ImageMagick"
        else
            echo "Or install manually on Ubuntu/Debian:"
            echo "  sudo apt install cmake ninja-build qt6-base-dev qt6-charts-dev qt6-tools-dev libkf6coreaddons-dev libkf6i18n-dev libkf6xmlgui-dev libkf6config-dev libkf6iconthemes-dev desktop-file-utils appstream-util imagemagick"
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
    print_info "Installing packages for openSUSE..."
    sudo zypper refresh
    sudo zypper install -y \
        cmake \
        ninja \
        qt6-base-devel \
        qt6-tools-devel \
        qt6-charts-devel \
        qt6-sql-sqlite \
        qt6-networkauth-devel \
        kf6-kcoreaddons-devel \
        kf6-ki18n-devel \
        kf6-kxmlgui-devel \
        kf6-kconfig-devel \
        kf6-kiconthemes-devel \
        desktop-file-utils \
        appstream-glib-devel \
        ImageMagick \
        wget \
        file
}

# Install dependencies for Ubuntu/Debian
install_deps_ubuntu() {
    print_info "Installing packages for Ubuntu/Debian..."
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
        libkf6coreaddons-dev \
        libkf6i18n-dev \
        libkf6xmlgui-dev \
        libkf6config-dev \
        libkf6iconthemes-dev \
        desktop-file-utils \
        appstream-util \
        imagemagick \
        wget \
        file
}

# Install linuxdeploy
install_linuxdeploy() {
    print_step "Installing linuxdeploy"

    mkdir -p "$LINUXDEPLOY_DIR"
    cd "$LINUXDEPLOY_DIR"

    # Download if not present
    if [ ! -f "linuxdeploy-x86_64.AppImage" ]; then
        wget -c "https://github.com/linuxdeploy/linuxdeploy/releases/download/continuous/linuxdeploy-x86_64.AppImage"
        chmod +x linuxdeploy-x86_64.AppImage
    fi

    if [ ! -f "linuxdeploy-plugin-qt-x86_64.AppImage" ]; then
        wget -c "https://github.com/linuxdeploy/linuxdeploy-plugin-qt/releases/download/continuous/linuxdeploy-plugin-qt-x86_64.AppImage"
        chmod +x linuxdeploy-plugin-qt-x86_64.AppImage
    fi

    # Extract AppImages
    if [ ! -d "linuxdeploy-extracted" ]; then
        mkdir -p linuxdeploy-extracted
        cd linuxdeploy-extracted
        ../linuxdeploy-x86_64.AppImage --appimage-extract > /dev/null
        cd ..
    fi

    if [ ! -d "linuxdeploy-qt-extracted" ]; then
        mkdir -p linuxdeploy-qt-extracted
        cd linuxdeploy-qt-extracted
        ../linuxdeploy-plugin-qt-x86_64.AppImage --appimage-extract > /dev/null
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

    print_success "linuxdeploy installed to $LINUXDEPLOY_DIR"
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
    if ninja Katalog_lrelease 2>/dev/null; then
        print_success "Translations built successfully"
    else
        print_warning "Translation build failed, creating empty .qm files"
        # Create empty .qm files for missing translations
        for ts_file in "$PROJECT_ROOT"/src/translations/*.ts; do
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
        -DBUILD_TESTS=OFF
    )

    if [ "$VERBOSE" = true ]; then
        cmake_args+=(-DCMAKE_VERBOSE_MAKEFILE=ON)
    fi

    cmake "${cmake_args[@]}" || print_error "CMake configuration failed"

    print_step "Building Katalog ($CMAKE_BUILD_TYPE mode)"

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
    cat > "$APPDIR/usr/share/applications/Katalog.desktop" << EOF
[Desktop Entry]
Type=Application
Name=Katalog
GenericName=File Catalog Manager
Comment=Create and manage catalogs of your files and folders
Comment[fr]=Créer et gérer des catalogues de vos fichiers et dossiers
Exec=Katalog
Icon=Katalog
Terminal=false
Categories=Utility;System;FileManager;
Keywords=catalog;files;folders;organization;storage;backup;
StartupNotify=true
StartupWMClass=Katalog
EOF

    # Create AppData file
    cat > "$APPDIR/usr/share/metainfo/io.github.stephanecouturier.katalog.appdata.xml" << EOF
<?xml version="1.0" encoding="UTF-8"?>
<component type="desktop-application">
  <id>io.github.stephanecouturier.katalog</id>
  <metadata_license>MIT</metadata_license>
  <project_license>GPL-3.0+</project_license>
  <name>Katalog</name>
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
  <launchable type="desktop-id">io.github.stephanecouturier.katalog.desktop</launchable>
  <provides>
    <binary>Katalog</binary>
  </provides>
  <content_rating type="oars-1.1"/>
  <releases>
    <release version="2.7" date="2025-08-31"/>
  </releases>
</component>
EOF

    # Create/copy icon
    if [ -f "$PROJECT_ROOT/src/images/Katalog_logo_64.ico" ]; then
        # Convert ICO to PNG with exact 64x64 size
        convert "$PROJECT_ROOT/src/images/Katalog_logo_64.ico[0]" -resize 64x64! "$APPDIR/usr/share/icons/hicolor/64x64/apps/Katalog.png" 2>/dev/null || {
            print_warning "ICO conversion failed, creating fallback icon"
            create_fallback_icon
        }
    else
        print_warning "ICO file not found, creating fallback icon"
        create_fallback_icon
    fi

    # Validate files
    desktop-file-validate "$APPDIR/usr/share/applications/Katalog.desktop" 2>/dev/null || print_warning "Desktop file validation warnings"
    appstream-util validate "$APPDIR/usr/share/metainfo/io.github.stephanecouturier.katalog.appdata.xml" 2>/dev/null || print_warning "AppData validation warnings"

    print_success "Desktop integration files prepared"
}

# Create fallback icon
create_fallback_icon() {
    convert -size 64x64 xc:steelblue -gravity center -pointsize 28 -fill white -font DejaVu-Sans-Bold -annotate +0+0 "K" "$APPDIR/usr/share/icons/hicolor/64x64/apps/Katalog.png"
}

# Create AppImage
create_appimage() {
    print_step "Creating AppImage"

    # Set up environment
    export PATH="$LINUXDEPLOY_DIR:$PATH"

    # Get version info
    local version="2.7-$(date +%Y%m%d-%H%M)"
    local appimage_name="Katalog-$version-x86_64.AppImage"

    # Verify AppDir structure
    if [ ! -f "$APPDIR/usr/bin/Katalog" ]; then
        print_error "Katalog executable not found in AppDir"
    fi

    if [ "$VERBOSE" = true ]; then
        echo "AppDir contents:"
        find "$APPDIR" -type f | head -20
    fi

    # Check for local appimagetool and use direct approach
    if [ -f "$APPIMAGETOOL_PATH" ]; then
        print_info "Using local appimagetool directly (offline mode)"
        print_info "Appimagetool location: $APPIMAGETOOL_PATH"

        # First run linuxdeploy without the appimage plugin
        print_info "Running linuxdeploy to bundle dependencies..."
        linuxdeploy \
            --appdir "$APPDIR" \
            --plugin qt \
            --executable "$APPDIR/usr/bin/Katalog" \
            --desktop-file "$APPDIR/usr/share/applications/Katalog.desktop" \
            --icon-file "$APPDIR/usr/share/icons/hicolor/64x64/apps/Katalog.png"

        if [ $? -ne 0 ]; then
            print_error "Dependency bundling failed"
        fi

        # Now create AppImage directly with local appimagetool
        print_info "Creating AppImage with local appimagetool..."

        # Copy your downloaded runtime to the appimagetool cache
        mkdir -p ~/.cache/appimagetool
        if [ -f ~/Downloads/runtime-x86_64 ]; then
            cp ~/Downloads/runtime-x86_64 ~/.cache/appimagetool/
            chmod +x ~/.cache/appimagetool/runtime-x86_64
            print_info "Using your downloaded runtime file"
        fi

        # Use the local appimagetool directly
        "$APPIMAGETOOL_PATH" "$APPDIR" "$appimage_name"

    else
        print_info "Using linuxdeploy with online appimagetool..."

        # Try to ensure runtime is cached
        if [ ! -f ~/.cache/appimagetool/runtime-x86_64 ]; then
            print_step "Pre-downloading AppImage runtime to avoid network issues"
            mkdir -p ~/.cache/appimagetool
            if [ -f ~/Downloads/runtime-x86_64 ]; then
                cp ~/Downloads/runtime-x86_64 ~/.cache/appimagetool/
                chmod +x ~/.cache/appimagetool/runtime-x86_64
                print_success "Runtime copied from Downloads"
            elif wget -O ~/.cache/appimagetool/runtime-x86_64 https://github.com/AppImage/type2-runtime/releases/download/continuous/runtime-x86_64; then
                chmod +x ~/.cache/appimagetool/runtime-x86_64
                print_success "Runtime cached successfully"
            else
                print_warning "Runtime download failed, appimagetool will try to download it"
            fi
        fi

        # Create AppImage with linuxdeploy
        linuxdeploy \
            --appdir "$APPDIR" \
            --plugin qt \
            --executable "$APPDIR/usr/bin/Katalog" \
            --desktop-file "$APPDIR/usr/share/applications/Katalog.desktop" \
            --icon-file "$APPDIR/usr/share/icons/hicolor/64x64/apps/Katalog.png" \
            --output appimage
    fi

    if [ $? -ne 0 ]; then
        print_error "AppImage creation failed"
    fi

    # Check for created AppImage (different naming depending on method)
    local created_appimage=""
    if [ -f "$appimage_name" ]; then
        created_appimage="$appimage_name"
    elif ls Katalog-*.AppImage 1> /dev/null 2>&1; then
        created_appimage=$(ls Katalog-*.AppImage | head -1)
        mv "$created_appimage" "$appimage_name"
        created_appimage="$appimage_name"
    fi

    if [ -n "$created_appimage" ]; then
        print_success "AppImage created: $created_appimage"

        # Show file info
        echo ""
        echo "📦 AppImage Information:"
        ls -lh "$created_appimage"
        file "$created_appimage"

        # Test basic functionality
        print_step "Testing AppImage"
        chmod +x "$created_appimage"
        if ./"$created_appimage" --version 2>/dev/null; then
            print_success "AppImage basic test passed"
        else
            print_warning "AppImage version test failed (not critical)"
        fi

        # Move AppImage to build base directory (final location)
        mv "$created_appimage" "$BUILD_BASE_DIR/"
        print_success "AppImage created in build directory: $BUILD_BASE_DIR/$created_appimage"

        # Create a symlink in the project root for easy access
        #ln -sf "$BUILD_BASE_DIR/$created_appimage" "$PROJECT_ROOT/$(basename "$created_appimage")"
        #print_info "Symlink created in project root: $PROJECT_ROOT/$(basename "$created_appimage")"

        echo ""
        print_info "To test themes and icons:"
        echo "  cd $BUILD_BASE_DIR"
        echo "  ./$created_appimage"
        echo ""
        #print_info "Or use the symlink from project root:"
        #echo "  cd $PROJECT_ROOT"
        #echo "  ./$(basename "$created_appimage")"

    else
        print_error "AppImage file not found after creation"
    fi
}

# Main function
main() {
    print_header

    parse_arguments "$@"

    echo "Build Configuration:"
    echo "  Project Root: $PROJECT_ROOT"
    echo "  Build Base Dir: $BUILD_BASE_DIR"
    echo "  Build Dir: $BUILD_DIR"
    echo "  Build Type: $CMAKE_BUILD_TYPE"
    echo "  Clean Build: $CLEAN_BUILD"
    echo "  Verbose: $VERBOSE"
    echo "  Appimagetool: $([ -f "$APPIMAGETOOL_PATH" ] && echo "✅ Found" || echo "❌ Missing")"
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
