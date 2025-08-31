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
#testse
#/////////////////////////////////////////////////////////////////////////////
# Application: Katalog
# File Name:   Katalog_linux_create_portable.sh
# Version:     1.0
# Purpose:     Fixed Katalog Portable Creator, Supports both specific Qt installations and system Qt
# Description:
# Author:      Stephane Couturier
#/////////////////////////////////////////////////////////////////////////////

set -e  # Exit on error

# Default configuration
EXECUTABLE_PATH=""
OUTPUT_DIR=""
INCLUDE_ALL="false"
USE_SYSTEM_QT="false"

# Colors for output
RED='\033[0;31m'
GREEN='\033[0;32m'
YELLOW='\033[1;33m'
BLUE='\033[0;34m'
NC='\033[0m' # No Color

print_step() {
    echo -e "${BLUE}==== $1 ====${NC}"
}

print_success() {
    echo -e "${GREEN}✅ $1${NC}"
}

print_warning() {
    echo -e "${YELLOW}⚠️  $1${NC}"
}

print_error() {
    echo -e "${RED}❌ $1${NC}"
}

# Check if executable exists
check_executable() {
    if [ ! -f "$EXECUTABLE_PATH" ]; then
        print_error "Executable not found: $EXECUTABLE_PATH"
        exit 1
    fi
    
    if [ ! -x "$EXECUTABLE_PATH" ]; then
        print_error "File is not executable: $EXECUTABLE_PATH"
        exit 1
    fi
    
    print_success "Found executable: $EXECUTABLE_PATH"
}

# Detect Qt installation path
detect_qt_installation() {
    print_step "Detecting Qt5 installation path"
    
    # Get Qt5Core path from your executable
    QT_CORE_PATH=$(ldd "$EXECUTABLE_PATH" | grep "libQt5Core.so.5" | awk '{print $3}')
    
    if [ -z "$QT_CORE_PATH" ]; then
        print_error "Could not detect Qt5Core path from executable"
        exit 1
    fi
    
    echo "📍 Qt5Core found at: $QT_CORE_PATH"
    
    if [ "$USE_SYSTEM_QT" = "true" ]; then
        # System Qt mode: use whatever Qt the executable is linked to
        QT_LIB_DIR=$(dirname "$QT_CORE_PATH")
        QT_INSTALL_DIR=$(dirname "$QT_LIB_DIR")
        print_success "Using system Qt5 installation (wherever executable is linked)"
        echo "📍 System Qt5 lib directory: $QT_LIB_DIR"
        echo "📍 System Qt5 installation: $QT_INSTALL_DIR"
    else
        # Specific Qt installation mode: try to use your custom Qt installation
        if echo "$QT_CORE_PATH" | grep -q "/home/shared/Development/Qt/6.9.1/"; then
            # Great! Using your Qt installation
            QT_LIB_DIR=$(dirname "$QT_CORE_PATH")
            QT_INSTALL_DIR=$(dirname "$QT_LIB_DIR")
            print_success "Using your Qt 6.9.1 installation"
        else
            # Executable is linked to system Qt5, but we want to use your Qt installation
            print_warning "Executable is linked to system Qt5, but we'll use your Qt installation"
            echo "System Qt5 path: $QT_CORE_PATH"
            
            # Try to find your Qt installation
            EXPECTED_QT_PATH="/home/shared/Development/Qt/6.9.1"
            
            echo "🔍 Looking for your Qt 6.9.1 installation..."
            echo "Expected location: $EXPECTED_QT_PATH"
            
            if [ -d "$EXPECTED_QT_PATH/gcc_64/lib" ]; then
                QT_LIB_DIR="$EXPECTED_QT_PATH/gcc_64/lib"
                QT_INSTALL_DIR="$EXPECTED_QT_PATH/gcc_64"
                print_success "Found your Qt 6.9.1 installation"
            elif [ -d "$EXPECTED_QT_PATH/lib" ]; then
                QT_LIB_DIR="$EXPECTED_QT_PATH/lib"
                QT_INSTALL_DIR="$EXPECTED_QT_PATH"
                print_success "Found your Qt 6.9.1 installation (alternative layout)"
            else
                print_error "Could not find Qt 6.9.1 installation at expected location"
                echo "Checked locations:"
                echo "  - $EXPECTED_QT_PATH/gcc_64/lib"
                echo "  - $EXPECTED_QT_PATH/lib"
                echo ""
                echo "Suggestions:"
                echo "  1. Use --use-system-qt to use the Qt5 your executable is linked to"
                echo "  2. Update EXPECTED_QT_PATH in the script to match your Qt installation"
                echo "  3. Rebuild your executable against the correct Qt installation"
                exit 1
            fi
        fi
    fi
    
    echo "📍 Using Qt5 lib directory: $QT_LIB_DIR"
    echo "📍 Using Qt5 installation: $QT_INSTALL_DIR"
    
    # Verify Qt5Core exists in our chosen location
    if [ ! -f "$QT_LIB_DIR/libQt5Core.so.5" ]; then
        print_error "libQt5Core.so.5 not found in: $QT_LIB_DIR"
        exit 1
    fi
    
    # Check for Qt plugins - try multiple common locations
    if [ "$USE_SYSTEM_QT" = "true" ]; then
        # For system Qt, try common plugin locations
        possible_plugin_dirs=(
            "/usr/lib64/Qt5/plugins"
            "/usr/lib/Qt5/plugins"
            "/usr/lib/x86_64-linux-gnu/Qt5/plugins"
            "$QT_INSTALL_DIR/plugins"
        )
        
        QT_PLUGINS_DIR=""
        for plugin_dir in "${possible_plugin_dirs[@]}"; do
            if [ -d "$plugin_dir" ]; then
                QT_PLUGINS_DIR="$plugin_dir"
                echo "📍 Qt5 plugins directory: $QT_PLUGINS_DIR"
                break
            fi
        done
        
        if [ -z "$QT_PLUGINS_DIR" ]; then
            print_warning "Qt5 plugins directory not found in common locations"
            echo "Searched: ${possible_plugin_dirs[*]}"
        else
            print_success "Qt5 installation detected successfully"
        fi
    else
        # For specific Qt installations
        QT_PLUGINS_DIR="$QT_INSTALL_DIR/plugins"
        if [ -d "$QT_PLUGINS_DIR" ]; then
            echo "📍 Qt5 plugins directory: $QT_PLUGINS_DIR"
            print_success "Qt5 installation detected successfully"
        else
            print_warning "Qt5 plugins directory not found at: $QT_PLUGINS_DIR"
            echo "Will search for plugins in other locations"
        fi
    fi
}

# Create output directory structure
setup_output_dir() {
    print_step "Setting up output directory"
    
    # User manually removes directory - no rm command
    if [ -d "$OUTPUT_DIR" ]; then
        print_warning "Output directory already exists: $OUTPUT_DIR"
        echo "Please remove it manually before running this script"
        exit 1
    fi
    
    mkdir -p "$OUTPUT_DIR"
    mkdir -p "$OUTPUT_DIR/lib"
    
    print_success "Output directory created: $OUTPUT_DIR"
}

# Copy main executable
copy_executable() {
    print_step "Copying main executable"
    
    cp "$EXECUTABLE_PATH" "$OUTPUT_DIR/"
    chmod +x "$OUTPUT_DIR/Katalog"
    
    print_success "Executable copied"
}

# Copy library and create symlinks
copy_library_with_symlinks() {
    local lib_path="$1"
    local lib_name=$(basename "$lib_path")
    
    if [ ! -f "$OUTPUT_DIR/lib/$lib_name" ]; then
        cp "$lib_path" "$OUTPUT_DIR/lib/"
        echo "  📦 Copied: $lib_name"
        
        # Create symlinks if versioned
        if echo "$lib_name" | grep -q '\.so\.[0-9]*\.[0-9]*'; then
            local major_version=$(echo "$lib_name" | sed 's/\(.*\.so\.[0-9]*\)\..*/\1/')
            local so_name=$(echo "$lib_name" | sed 's/\(.*\.so\)\..*/\1/')
            
            cd "$OUTPUT_DIR/lib"
            if [ ! -e "$major_version" ]; then
                ln -s "$lib_name" "$major_version"
                echo "    🔗 Created symlink: $major_version -> $lib_name"
            fi
            if [ ! -e "$so_name" ]; then
                ln -s "$lib_name" "$so_name"
                echo "    🔗 Created symlink: $so_name -> $lib_name"
            fi
            cd - > /dev/null
        fi
    else
        echo "  ♻️  Already exists: $lib_name"
    fi
}

# Copy ALL Qt5 libraries from the detected installation
copy_qt_libraries() {
    if [ "$USE_SYSTEM_QT" = "true" ]; then
        print_step "Copying ALL Qt5 libraries from system Qt installation"
    else
        print_step "Copying ALL Qt5 libraries from detected installation"
    fi
    
    echo "📋 Copying Qt5 libraries from: $QT_LIB_DIR"
    echo ""
    
    local qt_lib_count=0
    
    # Get the actual Qt5 libraries the executable needs from ldd
    echo "🎯 Getting Qt5 libraries from ldd output..."
    
    # Parse ldd output to get Qt5 library paths
    ldd "$EXECUTABLE_PATH" | grep "libQt5" | while read -r line; do
        # Extract library path from ldd line (format: "libQt5Xyz.so.5 => /path/to/lib (address)")
        lib_path=$(echo "$line" | sed -n 's/.*=> \([^ ]*\) .*/\1/p')
        lib_name=$(basename "$lib_path")
        
        if [ -f "$lib_path" ]; then
            copy_library_with_symlinks "$lib_path"
            echo "  ✅ Copied: $lib_name"
        else
            echo "  ❌ Missing: $lib_name (expected at $lib_path)"
        fi
    done
    
    # Count how many we actually copied
    qt_lib_count=$(find "$OUTPUT_DIR/lib" -name "libQt5*.so.5*" | wc -l)
    
    # Copy the critical libQt5XcbQpa.so.5 from the same installation (if not already copied)
    if [ -f "$QT_LIB_DIR/libQt5XcbQpa.so.5" ]; then
        copy_library_with_symlinks "$QT_LIB_DIR/libQt5XcbQpa.so.5"
        if [ "$USE_SYSTEM_QT" = "true" ]; then
            echo "  ✅ Ensured libQt5XcbQpa.so.5 from system Qt installation"
        else
            echo "  ✅ Ensured libQt5XcbQpa.so.5 from your Qt installation"
        fi
    else
        if [ "$USE_SYSTEM_QT" = "true" ]; then
            print_warning "libQt5XcbQpa.so.5 not found in system Qt installation"
        else
            print_warning "libQt5XcbQpa.so.5 not found in your Qt installation"
        fi
    fi
    
    echo ""
    echo "📊 Total Qt5 libraries copied: $qt_lib_count"
    
    if [ "$USE_SYSTEM_QT" = "true" ]; then
        print_success "Qt5 libraries copied from system installation"
    else
        print_success "Qt5 libraries copied from your installation"
    fi
}

# Copy other dependencies
copy_other_dependencies() {
    print_step "Copying other dependencies"
    
    echo "📋 Processing non-Qt5 dependencies..."
    echo ""
    
    # Process ldd output for non-Qt5 libraries
    ldd "$EXECUTABLE_PATH" | while read line; do
        if echo "$line" | grep -q "vdso\|linux-vdso"; then
            continue
        fi
        
        lib_path=$(echo "$line" | sed -n 's/.*=> \([^ ]*\) .*/\1/p')
        
        if [ -n "$lib_path" ] && [ "$lib_path" != "not" ] && [ -f "$lib_path" ]; then
            lib_name=$(basename "$lib_path")
            
            # Skip Qt5 libraries (we already copied them)
            if echo "$lib_name" | grep -q "^libQt5"; then
                continue
            fi
            
            # Copy important non-Qt5 libraries
            case "$lib_name" in
                # Always copy KF6
                libKF6*)
                    copy_library_with_symlinks "$lib_path"
                    ;;
                # Copy important support libraries
                libicu*|libssl*|libcrypto*|libglib*|libgobject*|libfontconfig*|libfreetype*)
                    copy_library_with_symlinks "$lib_path"
                    ;;
                # Copy graphics/audio libraries
                libX*|libGL*|libxcb*|libpulse*|libwayland*|libFLAC*|libvorbis*|libogg*|libsndfile*)
                    copy_library_with_symlinks "$lib_path"
                    ;;
                # Copy other important libraries
                libpng*|libjpeg*|libexpat*|libpcre*|libdbus*|libz*|libzstd*|libbz2*|liblzma*)
                    copy_library_with_symlinks "$lib_path"
                    ;;
                # In include-all mode, copy everything else
                *)
                    if [ "$INCLUDE_ALL" = "true" ]; then
                        # Skip basic system libraries
                        case "$lib_name" in
                            libc.so*|libm.so*|libdl.so*|libpthread.so*|ld-linux*.so*|librt.so*|libgcc_s.so*|libstdc++.so*)
                                echo "  ⏭️  Skipping system lib: $lib_name"
                                ;;
                            *)
                                copy_library_with_symlinks "$lib_path"
                                ;;
                        esac
                    else
                        echo "  ⏭️  Skipping other: $lib_name"
                    fi
                    ;;
            esac
        fi
    done
    
    print_success "Other dependencies copied"
}

# Copy Qt5 plugins from the correct installation
copy_qt_plugins() {
    if [ "$USE_SYSTEM_QT" = "true" ]; then
        print_step "Copying Qt5 plugins from system Qt installation"
    else
        print_step "Copying Qt5 plugins from detected installation"
    fi
    
    if [ -z "$QT_PLUGINS_DIR" ] || [ ! -d "$QT_PLUGINS_DIR" ]; then
        print_warning "Qt5 plugins directory not found or not set: $QT_PLUGINS_DIR"
        print_warning "Application may not work properly without platform plugins"
        return
    fi
    
    mkdir -p "$OUTPUT_DIR/plugins"
    
    # Copy all plugin categories
    local total_plugins=0
    for plugin_type in platforms imageformats iconengines sqldrivers multimedia; do
        if [ -d "$QT_PLUGINS_DIR/$plugin_type" ]; then
            cp -r "$QT_PLUGINS_DIR/$plugin_type" "$OUTPUT_DIR/plugins/"
            local plugin_count=$(find "$OUTPUT_DIR/plugins/$plugin_type" -name "*.so" | wc -l)
            echo "  📦 Copied $plugin_count $plugin_type plugins"
            total_plugins=$((total_plugins + plugin_count))
        else
            echo "  ⏭️  No $plugin_type plugins found"
        fi
    done
    
    echo "📊 Total plugins copied: $total_plugins"
    
    if [ "$USE_SYSTEM_QT" = "true" ]; then
        print_success "Qt5 plugins copied from system installation"
    else
        print_success "Qt5 plugins copied from your installation"
    fi
}

# Copy KF6 icon themes for portable distribution
copy_kf6_icon_themes() {
    print_step "Copying KF6 Breeze icon themes for portable use"

    # Find where breeze icons are installed
    local breeze_icon_paths=(
        "/usr/share/icons/breeze"
        "/usr/local/share/icons/breeze"
        "/opt/kde/share/icons/breeze"
        "$QT_LIB_DIR/../share/icons/breeze"
    )

    local breeze_source=""
    for path in "${breeze_icon_paths[@]}"; do
        if [ -d "$path" ]; then
            breeze_source="$path"
            echo "✅ Found Breeze icons at: $path"
            break
        fi
    done

    if [ -z "$breeze_source" ]; then
        print_warning "Breeze icon theme not found in standard locations"
        print_warning "KF6BreezeIcons library will be available but no theme files"
        return
    fi

    # Create portable icon structure
    mkdir -p "$OUTPUT_DIR/share/icons"

    # Copy breeze theme
    cp -r "$breeze_source" "$OUTPUT_DIR/share/icons/"

    # Also copy breeze-dark if available
    local breeze_dark_source="${breeze_source%/*}/breeze-dark"
    if [ -d "$breeze_dark_source" ]; then
        cp -r "$breeze_dark_source" "$OUTPUT_DIR/share/icons/"
        echo "✅ Also copied Breeze Dark theme"
    fi

    # Count icons copied
    local icon_count=$(find "$OUTPUT_DIR/share/icons/breeze" -name "*.svg" | wc -l)
    echo "📊 Total Breeze icons copied: $icon_count"

    print_success "KF6 Breeze icon themes copied for portable use"
}

# Create launcher script
create_launcher() {
    print_step "Creating launcher script with KF6 icon support"

    local qt_mode=""
    if [ "$USE_SYSTEM_QT" = "true" ]; then
        qt_mode="System Qt Mode"
    else
        qt_mode="Specific Qt Installation"
    fi

    cat > "$OUTPUT_DIR/Katalog.sh" << 'EOF'
#!/bin/sh
# Katalog Portable Launcher with KF6 Icon Support

# Get script directory
appname=$(basename "$0" | sed 's/\.sh$//')
dirname=$(dirname "$0")
tmp="${dirname#?}"

# Set directory name to absolute path
if [ "${dirname%$tmp}" != "/" ]; then
    dirname="$PWD/$dirname"
fi

# Set up library path
export LD_LIBRARY_PATH="$dirname/lib:$LD_LIBRARY_PATH"

# Set up Qt plugin path
export QT_PLUGIN_PATH="$dirname/plugins:$QT_PLUGIN_PATH"

# Set up icon theme paths for KF6
export XDG_DATA_DIRS="$dirname/share:$XDG_DATA_DIRS"

# Ensure Qt uses OpenSSL
export QT_SSL_USE_OPENSSL=1

# Debug info if verbose
if [ "$KATALOG_VERBOSE" = "1" ]; then
    echo "Katalog Portable Launcher with KF6 Icon Support"
    echo "================================================="
    echo "Working directory: $dirname"
    echo "Libraries: $(find "$dirname/lib" -name "*.so*" -type f | wc -l)"
    echo "Plugins: $(find "$dirname/plugins" -name "*.so" 2>/dev/null | wc -l)"
    echo ""
    echo "KF6 Icon Support Check:"
    if [ -f "$dirname/lib/libKF6BreezeIcons.so.5" ]; then
        echo "  ✅ libKF6BreezeIcons.so.5 found in portable lib"
    else
        echo "  ❌ libKF6BreezeIcons.so.5 NOT found in portable lib"
    fi

    if [ -d "$dirname/share/icons/breeze" ]; then
        echo "  ✅ Breeze icon theme found in portable share"
        icon_count=$(find "$dirname/share/icons/breeze" -name "*.svg" 2>/dev/null | wc -l)
        echo "  📊 Available icons: $icon_count"
    else
        echo "  ❌ Breeze icon theme NOT found in portable share"
    fi

    echo "  🔍 XDG_DATA_DIRS: $XDG_DATA_DIRS"
    echo ""
    echo "Library dependency check:"
    if ldd "$dirname/Katalog" | grep -q "not found"; then
        echo "❌ Missing dependencies:"
        ldd "$dirname/Katalog" | grep "not found"
    else
        echo "✅ All dependencies satisfied"
    fi
    echo ""
    echo "Starting Katalog..."
fi

# Run the application
exec "$dirname/Katalog" "$@"
EOF

    chmod +x "$OUTPUT_DIR/Katalog.sh"
    print_success "Launcher script with KF6 support created"
}

# Create test script
create_test_script() {
    local qt_mode=""
    if [ "$USE_SYSTEM_QT" = "true" ]; then
        qt_mode="System Qt"
    else
        qt_mode="Specific Qt"
    fi
    
    cat > "$OUTPUT_DIR/test-qt-fixed.sh" << EOF
#!/bin/bash
echo "Testing Katalog portable version ($qt_mode mode)..."
echo "============================================"

# Set up environment
export LD_LIBRARY_PATH="\$(pwd)/lib:\$LD_LIBRARY_PATH"
export QT_PLUGIN_PATH="\$(pwd)/plugins:\$QT_PLUGIN_PATH"

# Check Qt5 libraries
echo "🔍 Qt5 libraries in portable distribution:"
find lib -name "libQt5*.so.5" | while read qt_lib; do
    echo "  • \$(basename "\$qt_lib")"
done

echo ""
echo "🔍 Critical check - libQt5XcbQpa.so.5:"
if [ -f "lib/libQt5XcbQpa.so.5" ]; then
    echo "✅ libQt5XcbQpa.so.5 found in portable lib"
else
    echo "❌ libQt5XcbQpa.so.5 NOT found in portable lib"
fi

# Check platform plugin dependencies
echo ""
echo "🔍 Testing xcb platform plugin dependencies:"
if [ -f "plugins/platforms/libqxcb.so" ]; then
    MISSING_DEPS=\$(ldd plugins/platforms/libqxcb.so 2>/dev/null | grep "not found" || true)
    if [ -z "\$MISSING_DEPS" ]; then
        echo "✅ XCB platform plugin dependencies satisfied"
    else
        echo "❌ XCB platform plugin missing dependencies:"
        echo "\$MISSING_DEPS"
    fi
else
    echo "❌ XCB platform plugin not found"
fi

# Check executable dependencies
echo ""
echo "🔍 Testing executable dependencies:"
MISSING_EXEC=\$(ldd Katalog | grep "not found" || true)
if [ -z "\$MISSING_EXEC" ]; then
    echo "✅ All executable dependencies satisfied"
else
    echo "❌ Missing executable dependencies:"
    echo "\$MISSING_EXEC"
fi

echo ""
echo "📊 Statistics:"
echo "  • Libraries: \$(find lib -name "*.so*" -type f | wc -l)"
echo "  • Symlinks: \$(find lib -type l | wc -l)"
echo "  • Plugins: \$(find plugins -name "*.so" 2>/dev/null | wc -l)"

echo ""
echo "🎉 $qt_mode portable version test completed!"
echo "Run with: ./Katalog.sh"
EOF
    
    chmod +x "$OUTPUT_DIR/test-qt-fixed.sh"
}

# Parse arguments properly
parse_arguments() {
    while [[ $# -gt 0 ]]; do
        case $1 in
            --include-all)
                INCLUDE_ALL="true"
                shift
                ;;
            --selective)
                INCLUDE_ALL="false"
                shift
                ;;
            --use-system-qt)
                USE_SYSTEM_QT="true"
                shift
                ;;
            --help|-h)
                echo "Usage: $0 [executable_path] [output_directory] [options]"
                echo ""
                echo "Options:"
                echo "  --include-all      Copy all non-system libraries"
                echo "  --selective        Copy only essential libraries (default)"
                echo "  --use-system-qt    Use system Qt5 installation (wherever executable is linked)"
                echo "  --help             Show this help"
                echo ""
                echo "Qt Installation Modes:"
                echo "  Default: Uses specific Qt installation at /home/shared/Development/Qt/6.9.1/"
                echo "  --use-system-qt: Uses whatever Qt5 the executable is currently linked to"
                echo ""
                echo "Examples:"
                echo "  $0 ./build/Katalog ./katalog-portable-qt-fixed"
                echo "  $0 ./build/Katalog ./katalog-portable-system-qt --use-system-qt"
                echo "  $0 ./build/Katalog ./katalog-portable-qt-fixed --include-all"
                exit 0
                ;;
            *)
                # First non-option argument is executable path
                if [ -z "$EXECUTABLE_PATH" ]; then
                    EXECUTABLE_PATH="$1"
                # Second non-option argument is output directory
                elif [ -z "$OUTPUT_DIR" ]; then
                    OUTPUT_DIR="$1"
                else
                    print_error "Unknown argument: $1"
                    exit 1
                fi
                shift
                ;;
        esac
    done
    
    # Set defaults if not provided
    if [ -z "$EXECUTABLE_PATH" ]; then
        EXECUTABLE_PATH="./build/Katalog"
    fi
    
    if [ -z "$OUTPUT_DIR" ]; then
        if [ "$USE_SYSTEM_QT" = "true" ]; then
            OUTPUT_DIR="./katalog-portable-system-qt"
        else
            OUTPUT_DIR="./katalog-portable-qt-fixed"
        fi
    fi
}

# Main execution
main() {
    # Parse arguments first
    parse_arguments "$@"
    
    local qt_mode_display=""
    if [ "$USE_SYSTEM_QT" = "true" ]; then
        qt_mode_display="System Qt Mode"
    else
        qt_mode_display="Specific Qt Installation"
    fi
    
    echo -e "${BLUE}"
    echo "═══════════════════════════════════════════════════════════════"
    echo "           Katalog Qt-Fixed Portable Creator"
    echo "              $qt_mode_display"
    echo "       Mode: $(if [ "$INCLUDE_ALL" = "true" ]; then echo "Include ALL"; else echo "Selective"; fi)"
    echo "═══════════════════════════════════════════════════════════════"
    echo -e "${NC}"
    
    check_executable
    detect_qt_installation
    setup_output_dir
    copy_executable
    copy_qt_libraries
    copy_other_dependencies
    copy_qt_plugins
    copy_kf6_icon_themes
    create_launcher
    create_test_script
    
    echo ""
    print_success "Qt portable distribution created!"
    echo ""
    echo -e "${YELLOW}Output directory: $OUTPUT_DIR${NC}"
    echo -e "${YELLOW}Qt5 installation used: $QT_INSTALL_DIR${NC}"
    echo -e "${YELLOW}Qt mode: $qt_mode_display${NC}"
    echo -e "${YELLOW}Libraries: $(find "$OUTPUT_DIR/lib" -name "*.so*" -type f | wc -l)${NC}"
    echo -e "${YELLOW}Symlinks: $(find "$OUTPUT_DIR/lib" -type l | wc -l)${NC}"
    echo ""
    echo -e "${GREEN}Test: cd $OUTPUT_DIR && ./test-qt-fixed.sh${NC}"
    echo -e "${GREEN}Run: cd $OUTPUT_DIR && ./Katalog.sh${NC}"
    echo ""
}

# Run main function
main "$@"
