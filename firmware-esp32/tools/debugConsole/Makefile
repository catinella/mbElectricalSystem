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
#	
#	Configuration:
#		TTYSPEED=<bps>              serial port troughput
#		BUILDER_NOSCAPECODES={1|0}  enable/disable the escape codes skipping feature 
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

TTYSPEED ?= 115200
BUILDER_NOSCAPECODES ?= 0

ifeq ($(GDB), 1)
	CCOPTS = -o0 -g
else
	CCOPTS = -o3
endif

SYMBOLS = -DTTYSPEED=B$(TTYSPEED) -DBUILDER_NOSCAPECODES=$(BUILDER_NOSCAPECODES)
INCOPTS = -I.

.PHONY: all clean cleanall conf

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

conf:
			@echo "TTYSPEED = <BPS troughput>    # def. $(TTYSPEED)"
			@echo "BUILDER_NOSCAPECODES = {0|1}  # def. $(BUILDER_NOSCAPECODES)"
