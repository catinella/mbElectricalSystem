/*------------------------------------------------------------------------------------------------------------------------------
//
//   __  __       _             _     _ _          _____ _           _        _           _   ____            _                 
// |  \/  | ___ | |_ ___  _ __| |__ (_) | _____  | ____| | ___  ___| |_ _ __(_) ___ __ _| | / ___| _   _ ___| |_ ___ _ __ ___  
// | |\/| |/ _ \| __/ _ \| '__| '_ \| | |/ / _ \ |  _| | |/ _ \/ __| __| '__| |/ __/ _` | | \___ \| | | / __| __/ _ \ '_ ` _ \ 
// | |  | | (_) | || (_) | |  | |_) | |   <  __/ | |___| |  __/ (__| |_| |  | | (_| (_| | |  ___) | |_| \__ \ ||  __/ | | | | |
// |_|  |_|\___/ \__\___/|_|  |_.__/|_|_|\_\___| |_____|_|\___|\___|\__|_|  |_|\___\__,_|_| |____/ \__, |___/\__\___|_| |_| |_|
//                                                                                                 |___/                       
//
// File:   mbesUtilities.h
//
// Author: Silvano Catinella <catinella@yahoo.com>
//
// Description:
//	This file contains generic functions used by the project's modules
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
#ifndef MBESUTILITIES
#define MBESUTILITIES

#include <mbesMock.h>
#include <stdarg.h>
#include <stdint.h>
#include <stdbool.h>

#include <mbesPinsMap.h>
#include <mbesSerialConsole.h>

#ifndef DEBUG
#define DEBUG 1
#endif

typedef enum _mbesPinDir {
	INPUT,
	OUTPUT
} mbesPinDir;

//------------------------------------------------------------------------------------------------------------------------------
//                                                  F U N C T I O N S 
//------------------------------------------------------------------------------------------------------------------------------

#if MOCK == 1
void mbesSelector_shutdown();
void mbesUtilities_init();
#endif

void    logMsg               (const char *fmt, ...);
void    codeConverter        (const char *code, char *port, uint8_t *pinNumber);
uint8_t pinDirectionRegister (const char *code, mbesPinDir dir);
uint8_t pullUpEnabling       (const char *code);


//
// I/O Functions
//
uint8_t getPinsFromI2C       ();
uint8_t setPinsToI2C         ();
uint8_t getPinValue          (const char *code, uint8_t *value);
uint8_t setPinValue          (const char *code, uint8_t value);


#endif
