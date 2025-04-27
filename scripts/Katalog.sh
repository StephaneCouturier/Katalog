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
# Version:     2.4
# Purpose:     To run Katalog in linux in portable mode, using provided librairies
# Description:
# Author:      Stephane Couturier
#/////////////////////////////////////////////////////////////////////////////

#Inputs
    #echo " "
    #echo "inputs:"
    appname=`basename $0 | sed s,\.sh$,,`
    #echo "  appname:  " $appname

    dirname=`dirname $0`
    #echo "  dirname:  " $dirname

    tmp="${dirname#?}"
    #echo "  tmp:  " $tmp
    #echo "  PWD:        " $PWD

#Set dir name
    if [ "${dirname%$tmp}" != "/" ]; then
    dirname="$PWD/$dirname"
    #echo " "
    #echo "dir is not /"

    #echo "  dirname:    " $dirname
    fi

#Define LD_LIBRARY_PATH
    #LD_LIBRARY_PATH=/usr/lib/x86_64-linux-gnu/
    LD_LIBRARY_PATH="$dirname"
    #echo " "
    #echo "LD_LIBRARY_PATH:  " $LD_LIBRARY_PATH
    #echo " "
    export LD_LIBRARY_PATH


#Ensure Qt uses OpenSSL
    export QT_SSL_USE_OPENSSL=1
    #export QT_OPENSSL_VERSION_OVERRIDE="1.1"  # Adjust version if necessary

#Run the application with verbose plugin debugging
   #QT_DEBUG_PLUGINS=1 "$dirname/$appname" "$@"

#Run app
   "$dirname/$appname" "$@"
