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

CC       = $(AVR-GCC_HOME)/bin/avr-gcc
STRIP    = $(AVR-GCC_HOME)/bin/avr-strip
OBJCOPY  = $(AVR-GCC_HOME)/bin/avr-objcopy
CFLAGS   = -Wall -Os -mmcu=atmega16 -ffunction-sections -fdata-sections -fshort-enums -fno-common -funsigned-char \
           -funsigned-bitfields 
SYMBOLS  = 
LDFLAGS  = 

FIRMWARE = firmware

INCLUDE  = -I$(AVR-GCC_HOME)/avr/include -I.

SRCs    := $(shell ls mb[eE]*.c)
OBJs    := $(SRCs:%.c=%.o)
DEPs    := $(SRCs:%.c=%.d)

include Makefile.conf

.PHONY: clean

all:				$(FIRMWARE).elf $(FIRMWARE).strp.elf $(FIRMWARE).hex $(FIRMWARE).strp.hex

%.hex:			%.elf
				@echo "[ HEX  ] $@"
				@$(OBJCOPY) -O ihex $< $@

%.strp.elf:			%.elf
				@echo "[STRIPP] $@"
				@$(STRIP) --strip-all $< -o $@

firmware-test.elf:	firmware-test.o
				@echo "[  LD  ] $@"
				@$(CC) $(CFLAGS) $(LDFLAGS) $(XC8FLAGS) $<  -o $@

firmware-test.o:		firmware-test.c
				@echo "[  CC  ] $@"
				@$(CC) $(CFLAGS) -c $< $(SYMBOLS) $(INCLUDE) -o $@

$(FIRMWARE).elf:		$(OBJs)
				@echo "[  LD  ] $@"
				@$(CC) $(CFLAGS) $(LDFLAGS) $(XC8FLAGS) $^  -o $@

mbElectricalSystem.o:	mbElectricalSystem.c
				@echo "[  CC  ] $@"
				@$(CC) $(CFLAGS) -c $< $(SYMBOLS) $(INCLUDE) -o $@

mbes%.o:			mbes%.c mbes%.h
				@echo "[  CC  ] $@"
				@$(CC) $(CFLAGS) -c $< $(SYMBOLS) $(INCLUDE) -o $@

clean:
				@echo "[ CLEAN ]"
				@rm -fv $(OBJs) $(DEPs)

cleanall:			clean
				@for ext in elf strp.elf hex strp.hex ; do \
					rm -fv $(FIRMWARE).$$ext firmware-test.$$ext ;\
				done
