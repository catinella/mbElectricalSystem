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
#		TTYSPEED ?= <bps>              Serial port troughput setting
#		BUILDER_NOSCAPECODES ?= {1|0}  iIt enables/disables the escape codes skipping feature 
#		TARGET_ARCH ?= {AVR8|ESP32}    It sets the target MCU family (it is used to adjust the MCU's pins names)
#		PTS_PINMAPFILE ?= <filename>   It sets the header-file name where customized symbols are defined
#
#	[!] The "?=" assigment allows you to overwrite those values using shell variables
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
include Makefile.conf

ifeq ($(GDB), 1)
	CCOPTS = -o0 -g
else
	CCOPTS = -o3
endif

ifeq ($(TARGET_ARCH), ESP32)
	ARCH = -DTARGET_ESP32=1
endif

ifeq ($(TARGET_ARCH), AVR8)
	ARCH = -DTARGET_AVR8=1
endif

SYMBOLS =                                              \
	"-DTTYSPEED=B$(TTYSPEED)"                        \
	"-DBUILDER_NOSCAPECODES=$(BUILDER_NOSCAPECODES)" \
	$(ARCH)                                          \
	"-DPTS_PINMAPFILE=\"$(PTS_PINMAPFILE)\""


INCOPTS = -I.

.PHONY: all clean cleanall help

all:			debugConsole

debugConsole.o:	debugConsole.c
			@echo "[ CC ] $@"
			@gcc -Wall $(CCOPTS) -c $(SYMBOLS) $(INCOPTS) -o $@ $<

%.o:			%.c %.h
			@echo "[ CC ] $@"
			@gcc -Wall $(CCOPTS) -c $(SYMBOLS) $(INCOPTS) -o $@ $<

debugConsole:	debugConsole.o stringBuilder.o pinToSymbol.o
			@echo "[ LD ] $@"
			@gcc -Wall $^ -o $@

clean:
			@echo "[ CLEAN ]"
			@rm -fv *.o

cleanall:		clean
			@rm -fv debugConsole

help:
			@echo "TTYSPEED = <BPS troughput>    # $(TTYSPEED)"
			@echo "BUILDER_NOSCAPECODES = {0|1}  # $(BUILDER_NOSCAPECODES)"
			@echo "TARGET_ARCH = {AVR8|ESP32}    # $(TARGET_ARCH)"
			@echo "PTS_PINMAPFILE = <file.h>     # $(PTS_PINMAPFILE)"
