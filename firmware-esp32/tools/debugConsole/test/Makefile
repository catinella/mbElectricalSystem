#-------------------------------------------------------------------------------------------------------------------------------
#
#  __  __       _             _     _ _          _____ _           _        _           _   ____            _
# |  \/  | ___ | |_ ___  _ __| |__ (_) | _____  | ____| | ___  ___| |_ _ __(_) ___ __ _| | / ___| _   _ ___| |_ ___ _ __ ___ 
# | |\/| |/ _ \| __/ _ \| '__| '_ \| | |/ / _ \ |  _| | |/ _ \/ __| __| '__| |/ __/ _` | | \___ \| | | / __| __/ _ \ '_ ` _ \
# | |  | | (_) | || (_) | |  | |_) | |   <  __/ | |___| |  __/ (__| |_| |  | | (_| (_| | |  ___) | |_| \__ \ ||  __/ | | | | |
# |_|  |_|\___/ \__\___/|_|  |_.__/|_|_|\_\___| |_____|_|\___|\___|\__|_|  |_|\___\__,_|_| |____/ \__, |___/\__\___|_| |_| |_|
#                                                                                                 |___/
#
# File:   Makefile
#
# Author: Silvano Catinella <catinella@yahoo.com>
#
# Description:
#	This file allows you to build the executable tests in easy way. But, before to start the building process, you have
#	to create a configuration file (Makefile.conf). This file must have the following symbol definitions
#		TARGET_ARCH    ?= {AVR8|ESP32}    # The target MCU family (it is used to find the MCU's pins names)
#		PTS_PINMAPFILE ?= <header file>   # The H-file where the sybols have been defined
#
#	[!] The "?=" assigment allows you to overwrite those values using shell variables
#
#
# License:
#	Copyright (C) 2023 Silvano Catinella <catinella@yahoo.com>
#
#	This program is free software: you can redistribute it and/or modify it under the terms of the GNU General Public
#	License as published by the Free Software Foundation, either version 3 of the License, or (at your option) any later
#	version.
#
#	This program is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied
#	warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU General Public License for more details.
#
#	You should have received a copy of the GNU General Public License along with this program. If not, see
#		<https://www.gnu.org/licenses/gpl-3.0.txt>.
#
#-------------------------------------------------------------------------------------------------------------------------------

srcs := $(shell ls *_test.c)
exes := $(srcs:.c=)

INCOPTS  ?= -I. -I../
GDB      ?= 0

include Makefile.conf

ifeq ($(GDB), 1)
	CCOPTS = -o0 -g
else
	CCOPTS = -o3
endif

ifeq ($(TARGET_ARCH), AVR8)
	ARCH = "-DTARGET_AVR8=1"
endif

ifeq ($(TARGET_ARCH), ESP32)
	ARCH = "-DTARGET_ESP32=1"
endif

SYMBOLS = $(ARCH) "-DPTS_PINMAPFILE=\"$(PTS_PINMAPFILE)\""


.PHONY: all clean cleanall help

all:	$(exes)

%.o:		%.c ../werror.h
		@echo "[ CC ] $@"
		@gcc -Wall $(CCOPTS) $(INCOPTS) $(SYMBOLS) -c $< -o $@

%_test:	%_test.o %.o
		@echo "[ LD ] $@"
		@gcc -Wall $(CCOPTS) $^ -o $@

%.o:		../%.c ../%.h
		@echo "[ CC* ] $@"
		@gcc -Wall $(CCOPTS) $(INCOPTS) $(SYMBOLS) -c $< -I. -o $@

clean:
		@echo "[CLEAN]"
		@rm -fv *.o

cleanall:	clean
		@rm -fv $(exes)
	
help:
		@echo "[CONFIG]"
		@echo "TARGET_ARCH=$(TARGET_ARCH)"
		@echo "PTS_PINMAPFILE=$(PTS_PINMAPFILE)"
