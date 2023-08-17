#-------------------------------------------------------------------------------------------------------------------------------
#
#   __  __       _             _     _ _          _____ _           _        _           _   ____            _                 
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
#	This file automatize all firmware building process using AVR-GCC cross compiler.
#	
#	That software is available for all common operating systems (eg. Linux, Windows...) and you can download it free using
#	the following address:
#		https://www.microchip.com/en-us/tools-resources/develop/microchip-studio/gcc-compilers
#	
#	After the AVR-GCC installation, you need to create the Makefile.conf where you have to set the "AVR-GCC_HOME" symbol
#	to the installed compiler's foder.
#	
#	To make your life easier, Atmel/Microchip provides device-pack files at http://packs.download.atmel.com. The files have
#	extension .atpack but apart from that, they are just ZIP files, so you can unzip them and use them. These files contain
#	all you need
#
#	You can find further information about AVR-GCC, on the following web-page: https://gcc.gnu.org/wiki/avr-gcc
#
#	MPLabX
#	======
#	In order to allow me to use the Microchip's IDE MPLabX, I have rewritten this file to generate all files with the same 
#	(first) main rule. It is not an elegant solution but it integrate this Makefile with the MPLabX's building process, in
#	east way. In order to enable these intergration-rules, you have to set the BUILD4MPLABX symbol to 1 in the Makefile.conf
#	file
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

CC       = $(AVR-GCC_HOME)/bin/avr-gcc
STRIP    = $(AVR-GCC_HOME)/bin/avr-strip
OBJCOPY  = $(AVR-GCC_HOME)/bin/avr-objcopy
CFLAGS   = -Wall -Os -mmcu=atmega16 -ffunction-sections -fdata-sections -fshort-enums -fno-common -funsigned-char \
           -funsigned-bitfields 
SYMBOLS  = 
LDFLAGS  = 
INCLUDE  = -I$(AVR-GCC_HOME)/avr/include -I.

BUILD4MPLABX ?= 0

MPLABXTGTDIR     = dist/default/production
MPLABXTGTFILE    = firmware.production.hex
MPLABXTGTDBGDIR  = dist/default/debug
MPLABXTGTDBGFILE = firmware.debug.elf
FIRMWARE = firmware
#FIRMWARE = firmware-test

# Lists
SRCs := $(shell ls mb[eE]*.c)
OBJs := $(SRCs:%.c=%.o)
DEPs := $(SRCs:%.c=%.d)
DBJs := $(SRCs:%.c=%-dbg.o)


ifeq ($(BUILD4MPLABX),1)
.PHONY: clean all-mplabx make-mplabx-dir
else
.PHONY: clean all
endif

#-------------------------------------------------------------------------------------------------------------------------------
#                                                     R U L E S
#-------------------------------------------------------------------------------------------------------------------------------
ifeq ($(BUILD4MPLABX),1)
all-mplabx:			$(FIRMWARE).hex $(FIRMWARE)-gdb.elf make-mplabx-dir
				@echo "[ COPY ] $^"
				@cp -fv $(FIRMWARE).hex      $(MPLABXTGTDIR)/$(MPLABXTGTFILE)
				@cp -fv $(FIRMWARE)-gdb.elf  $(MPLABXTGTDBGDIR)/$(MPLABXTGTDBGFILE)
else
all:				$(FIRMWARE).hex $(FIRMWARE).strp.hex $(FIRMWARE)-gdb.hex
endif


firmware-test.elf:	firmware-test.o
				@echo "[  LD  ] $@"
				@$(CC) $(CFLAGS) $(LDFLAGS) $(XC8FLAGS) $<  -o $@

firmware-test.o:		firmware-test.c
				@echo "[  CC  ] $@"
				@$(CC) $(CFLAGS) -c $< $(SYMBOLS) $(INCLUDE) -o $@


$(FIRMWARE)-gdb.elf:	firmware-gdb.o $(DBJs)
				@echo "[  LD  ] $@"
				@$(CC) $(CFLAGS) $(LDFLAGS) $(XC8FLAGS) $^  -o $@

$(FIRMWARE).elf:		firmware.o $(OBJs)
				@echo "[  LD  ] $@"
				@$(CC) $(CFLAGS) $(LDFLAGS) $(XC8FLAGS) $^  -o $@


firmware.o:			firmware.c
				@echo "[  CC  ] $@"
				@$(CC) $(CFLAGS) -c $< $(SYMBOLS) $(INCLUDE) -o $@

firmware-gdb.o:		firmware.c
				@echo "[  CC  ] $@"
				@$(CC) $(CFLAGS) -g -c $< $(SYMBOLS) $(INCLUDE) -o $@

#-------------------------------------------------------------------------------------------------------------------------------
#                                              G E N E R I C   R U L E S
#-------------------------------------------------------------------------------------------------------------------------------
%.hex:			%.elf
				@echo "[ HEX  ] $@"
				@$(OBJCOPY) -O ihex $< $@

%.strp.elf:			%.elf
				@echo "[STRIPP] $@"
				@$(STRIP) --strip-all $< -o $@

mbes%.o:			mbes%.c mbes%.h
				@echo "[  CC  ] $@"
				@$(CC) $(CFLAGS) -c $< $(SYMBOLS) $(INCLUDE) -o $@

mbes%-dbg.o:		mbes%.c mbes%.h
				@echo "[CC-DBG] $@"
				@$(CC) $(CFLAGS) -g -c $< $(SYMBOLS) $(INCLUDE) -o $@

#-------------------------------------------------------------------------------------------------------------------------------
#                                                  O T H E R S
#-------------------------------------------------------------------------------------------------------------------------------
ifeq ($(BUILD4MPLABX),1)
make-mplabx-dir:		
				@for dir in $(MPLABXTGTDIR) $(MPLABXTGTDBGDIR); do       \
					echo "  + $$dir"; test -d $$dir || mkdir -p $$dir; \
				done
endif

clean:
				@echo "[ CLEAN ]"
				@rm -fv $(OBJs) $(DEPs) $(DBJs) firmware.o firmware-gdb.o firmware-test.o

cleanall:			clean
				@for ext in elf strp.elf hex strp.hex ; do \
					rm -fv $(FIRMWARE).$$ext firmware-test.$$ext firmware-gdb.$$ext;\
				done
ifeq ($(BUILD4MPLABX),1)
				@rm -rfv dist
endif


install:			$(FIRMWARE).hex
				@echo "[INSTALL]"
				@avrdude -v -c usbasp -p m16 -P usb -U flash:w:$<
