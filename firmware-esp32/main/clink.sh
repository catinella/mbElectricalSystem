#!/bin/bash
#-----------------------------------------------------------------------------------------------------------------------------------
#    __  __       _             _     _ _          _____ _           _        _           _   ____            _                 
#   |  \/  | ___ | |_ ___  _ __| |__ (_) | _____  | ____| | ___  ___| |_ _ __(_) ___ __ _| | / ___| _   _ ___| |_ ___ _ __ ___  
#   | |\/| |/ _ \| __/ _ \| '__| '_ \| | |/ / _ \ |  _| | |/ _ \/ __| __| '__| |/ __/ _` | | \___ \| | | / __| __/ _ \ '_ ` _ \ 
#   | |  | | (_) | || (_) | |  | |_) | |   <  __/ | |___| |  __/ (__| |_| |  | | (_| (_| | |  ___) | |_| \__ \ ||  __/ | | | | |
#   |_|  |_|\___/ \__\___/|_|  |_.__/|_|_|\_\___| |_____|_|\___|\___|\__|_|  |_|\___\__,_|_| |____/ \__, |___/\__\___|_| |_| |_|
#                                                                                                   |___/
#
# File name: clink.sh
#
# Author: Silvano Catinella <catinella@yahoo.com>
#
# Description:
#	This script creates a symbolic link where its name is the ESP-IDF standard firmware source.code file name, and the link
#	points to the file you want to build. I have created this scripts to allow you to compile one of the available test or
#	the official firmware. Unortunately, the firmware's file name cannot be changed by the CMakeLists.txt file, easily. In
#	fact, when you perform that change, you loose all the ESP-IDF include (-I<path>) setting.
#
#	use: clink.sh [src-file-name]  # Eg. clink.sh test01.c
#
#-----------------------------------------------------------------------------------------------------------------------------------

TFILE="firmware-esp32.c"
SFILE="$1"
err=0

cd "${0%/*}/" >/dev/null

[ -L "./$TFILE" ] && rm -f "./$TFILE" && echo "Old symbolic link removed"

[ -n "$SFILE" ] && {
	if [ -e $SFILE ]; then
		if ln -s "$SFILE" "$TFILE" ; then
			echo "Symbolic link created"
		else
			echo "ERROR! I cannot create the \"$TFILE\" symbolic link" 2>&1
			err=65
		fi
	else
		echo "ERROR! \"$SFILE\" not found"  2>&1
		err=67
	fi
}

cd - >/dev/null
exit $err
