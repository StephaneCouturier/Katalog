#!/bin/sh
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
# File Name:   Katalog.sh
# Version:     2.6
# Purpose:     To run Katalog in linux in portable mode, using provided librairies
# Description:
# Author:      Stephane Couturier
#/////////////////////////////////////////////////////////////////////////////

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

# Ensure Qt uses OpenSSL
export QT_SSL_USE_OPENSSL=1

# Check GLIBC compatibility
REQUIRED_GLIBC="2.38"
SYSTEM_GLIBC=$(ldd --version | head -n1 | grep -o '[0-9]\+\.[0-9]\+' | head -n1)
if [ "$(printf '%s\n' "$REQUIRED_GLIBC" "$SYSTEM_GLIBC" | sort -V | head -n1)" != "$REQUIRED_GLIBC" ]; then
    echo "Error: This system has GLIBC $SYSTEM_GLIBC, but Katalog requires GLIBC $REQUIRED_GLIBC or newer"
    echo "Katalog requires a newer Linux distribution or to be rebuild on this system"
    exit 1
fi

# Debug info if verbose
if [ "$KATALOG_VERBOSE" = "1" ]; then
    echo "Katalog Portable Launcher - System Qt Mode"
    echo "================================================="
    echo "Working directory: $dirname"
    echo "Libraries: $(find "$dirname/lib" -name "*.so*" -type f | wc -l)"
    echo "Symlinks: $(find "$dirname/lib" -type l | wc -l)"
    echo "Plugins: $(find "$dirname/plugins" -name "*.so" 2>/dev/null | wc -l)"
    echo ""
    echo "Qt6 libraries check:"
    find "$dirname/lib" -name "libQt6*.so.6" | while read qt_lib; do
        echo "  • $(basename "$qt_lib")"
    done
    echo ""
    echo "XCB Platform library:"
    if [ -f "$dirname/lib/libQt6XcbQpa.so.6" ]; then
        echo "  ✅ libQt6XcbQpa.so.6 found in portable lib"
    else
        echo "  ❌ libQt6XcbQpa.so.6 NOT found in portable lib"
    fi
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
