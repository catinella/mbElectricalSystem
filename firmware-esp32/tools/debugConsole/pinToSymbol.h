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
//
//	Symbols description:
//		PTS_PINMAPFILE   Header file where every pin-symbol couple is defined
//		PTS_MAXPINS      Maximum number of pins defined in the header file
//		PTS_ROWMAXSIZE   Maximum length of the pin-symbol map file rows
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

#define PTS_MAXPINS      32
#define PTS_PINMAPFILE   "../mbesPinsMap.h"
#define PTS_DEFAREGEX    "^[ \t]*#define[ \t]\\+"
#define PTS_DEFBREGEX    "^[io]_[A-Z0-9]\\+ \\+\"[A-Z0-9][0-9]\""
#define PTS_ROWMAXSIZE   256
#define PTS_MAXSYMSIZE   24


// Pins-Symbols associations DB item
typedef struct {
	char pin[3];
	char symbol[32];
} ptsDbItem_t;


uint8_t pinToSymbol_get  (char *symbol, const char *pin);
uint8_t pinToSymbol_init (const char *headerFile);

#endif
