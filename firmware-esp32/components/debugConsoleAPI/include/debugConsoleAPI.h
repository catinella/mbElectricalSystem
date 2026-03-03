/*------------------------------------------------------------------------------------------------------------------------------
//
//   __  __       _             _     _ _          _____ _           _        _           _   ____            _                 
// |  \/  | ___ | |_ ___  _ __| |__ (_) | _____  | ____| | ___  ___| |_ _ __(_) ___ __ _| | / ___| _   _ ___| |_ ___ _ __ ___  
// | |\/| |/ _ \| __/ _ \| '__| '_ \| | |/ / _ \ |  _| | |/ _ \/ __| __| '__| |/ __/ _` | | \___ \| | | / __| __/ _ \ '_ ` _ \ 
// | |  | | (_) | || (_) | |  | |_) | |   <  __/ | |___| |  __/ (__| |_| |  | | (_| (_| | |  ___) | |_| \__ \ ||  __/ | | | | |
// |_|  |_|\___/ \__\___/|_|  |_.__/|_|_|\_\___| |_____|_|\___|\___|\__|_|  |_|\___\__,_|_| |____/ \__, |___/\__\___|_| |_| |_|
//                                                                                                 |___/                       
//
// File: debugConsole.h
//
// Author: Silvano Catinella <catinella@yahoo.com>
//
// Description:
//	This library is used to keep track of the I/O pins status.
//	When the degbug session become a too much chaotic one, it is useful to monitor the PINs status in permanent way.
//	This small lib and the debug-console app allo you to achieve the result.
//
//	To show the pin's status on the debug-confole, you have to replace the I/O functions with the ones provided by this lib.
//	They will call internally the I/O functions and they will also send all needed information to the debug-console you
//	are running.
//
//	If you want enable/disable the monitoring without to change your code, define/remove the following symbol:
//		DBGCON_KEEPTRACK
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
#ifndef MBES_DEBUGCONSOLE
#define MBES_DEBUGCONSOLE
#include <stdint.h>
#include <stdint.h>

#ifdef TARGET_AVR8
#include <>
typedef const char* pinIdType;

#elifdef TARGET_ESP32
typedef uint8_t pinIdType;

#else
#error "ERROR! TARGET_<ARCH> is an unknown one"
#endif
	

uint8_t keepTrack_getGPIO (pinIdType pin);
void    keepTrack_setGPIO (pinIdType pin, uint8_t value);


#endif
