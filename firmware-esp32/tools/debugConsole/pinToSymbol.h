/*------------------------------------------------------------------------------------------------------------------------------
//
//   __  __       _             _     _ _          _____ _           _        _           _   ____            _                 
// |  \/  | ___ | |_ ___  _ __| |__ (_) | _____  | ____| | ___  ___| |_ _ __(_) ___ __ _| | / ___| _   _ ___| |_ ___ _ __ ___  
// | |\/| |/ _ \| __/ _ \| '__| '_ \| | |/ / _ \ |  _| | |/ _ \/ __| __| '__| |/ __/ _` | | \___ \| | | / __| __/ _ \ '_ ` _ \ 
// | |  | | (_) | || (_) | |  | |_) | |   <  __/ | |___| |  __/ (__| |_| |  | | (_| (_| | |  ___) | |_| \__ \ ||  __/ | | | | |
// |_|  |_|\___/ \__\___/|_|  |_.__/|_|_|\_\___| |_____|_|\___|\___|\__|_|  |_|\___\__,_|_| |____/ \__, |___/\__\___|_| |_| |_|
//                                                                                                 |___/                       
//
// File: pinToSymbol.h
//
// Author: Silvano Catinella <catinella@yahoo.com>
//
// Description:
//	This module manage the pin-to-customized symbols associations DB. These couples must be defined in a C-language header
//	file.	Because the pins labels (they are symbols too) depend by the MCU and the used librarries, you have yo define
//	the used arch. You can set it using the TARGET_ARCH Make's symbol
//
//	Symbols description:
//		[PTS_PINMAPFILE]  Header file where every pin-symbol couple is defined
//		PTS_MAXPINS       Maximum number of pins defined in the header file
//		PTS_ROWMAXSIZE    Maximum length of the pin-symbol map file rows
//		PTS_MAXSYMSIZE
//		-------- Platform dependent symbols -------- 
//		PTS_DEFBREGEX     Regex used to match the pin definition (without ^#define). The pattern depends by the platform
//		PTS_PINLABSIZE    PIN's label-size. This size depends by the platform's library (avr.h, esp-idf...)
//		CONS_PINDEMATCH   Regex used to check for pin definiton
//
//
// License:
//	Copyright (C) 2023 Silvano Catinella <catinella@yahoo.com>
//
//	This program is free software: you can redistribute it and/or modify it under the terms of the GNU General Public
//	License as published by the Free Software Foundation, either version 3 of the License, or (at your option) any later
//	version.
//
//	This program is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied
//	warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU General Public License for more details.
//
//	You should have received a copy of the GNU General Public License along with this program. If not, see
//		<https://www.gnu.org/licenses/gpl-3.0.txt>.
//
------------------------------------------------------------------------------------------------------------------------------*/
#ifndef PINTOSYMBOL_UT
#define PINTOSYMBOL_UT

#include <stdint.h>
#include <werror.h>

//
// Platform independent symbols
//
#define PTS_MAXPINS      32
#define PTS_DEFAREGEX    "^[ \t]*#define[ \t]\\+"
#define PTS_ROWMAXSIZE   256
#define PTS_MAXSYMSIZE   24


//
// Platform dependant symbols
//
#ifdef TARGET_AVR8
#define PTS_DEFBREGEX    "^[io]_[A-Z0-9]\\+ \\+\"[A-Z0-9][0-9]\""
#define CONS_PINDEMATCH   "^[A-Z0-9][0-9]:[0-9]\\+$"
#define PTS_PINLABSIZE   3

#elifdef TARGET_ESP32
#define PTS_DEFBREGEX    "^[io]_[A-Z0-9]\\+ \\+GPIO_NUM_[0-9]\\+"
#define CONS_PINDEMATCH   "^GPIO_NUM_[0-9]\\+:[0-9]\\+$"
#define PTS_PINLABSIZE   16

#else
#error "ERROR! TARGET_<ARCH> has not been defined or it is an unknown one"
#endif


// Pins-Symbols associations DB item
typedef struct {
	char pin[PTS_PINLABSIZE];
	char symbol[32];
} ptsDbItem_t;

//------------------------------------------------------------------------------------------------------------------------------
//                                         P U B L I C   F U N C T I O N S
//------------------------------------------------------------------------------------------------------------------------------
werror pinToSymbol_get  (char *symbol, const char *pin);
werror pinToSymbol_init (const char *headerFile);
werror pinDef_get       (const char *log, char *pin, int *value);
void   pinDef_free      ();

#endif
