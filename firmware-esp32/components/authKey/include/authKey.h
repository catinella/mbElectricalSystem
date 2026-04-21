/*------------------------------------------------------------------------------------------------------------------------------
//
//   __  __       _             _     _ _          _____ _           _        _           _   ____            _                 
// |  \/  | ___ | |_ ___  _ __| |__ (_) | _____  | ____| | ___  ___| |_ _ __(_) ___ __ _| | / ___| _   _ ___| |_ ___ _ __ ___  
// | |\/| |/ _ \| __/ _ \| '__| '_ \| | |/ / _ \ |  _| | |/ _ \/ __| __| '__| |/ __/ _` | | \___ \| | | / __| __/ _ \ '_ ` _ \
// | |  | | (_) | || (_) | |  | |_) | |   <  __/ | |___| |  __/ (__| |_| |  | | (_| (_| | |  ___) | |_| \__ \ ||  __/ | | | | |
// |_|  |_|\___/ \__\___/|_|  |_.__/|_|_|\_\___| |_____|_|\___|\___|\__|_|  |_|\___\__,_|_| |____/ \__, |___/\__\___|_| |_| |_|
//                                                                                                 |___/                       
//
// File:   authKey.h
//
// Author: Silvano Catinella <catinella@yahoo.com>
//
// Description:
//	The authorized key's value is stored in the ESP32's NVS partition. This library provides a functions to manage the
//	resistive key storing/reading/authentication process.
//
//	Configurable parameters:
//		AUTHKEY_TOLERANCE  <n> // Tollerance in key evaluation
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
#ifndef __AUTHKEY__

#include <stddef.h>
#include <werror.h>

#define AUTHKEY_DEEPLEV   2

#ifndef AUTHKEY_TOLERANCE
#define AUTHKEY_TOLERANCE 50
#endif

#define AUTHKEY_LABELS    {"refVal_A", "refVal_B"}

typedef uint16_t authKey_t[AUTHKEY_DEEPLEV];


//werror authKey_save  (const authKey_t value);
werror authKey_read  (authKey_t value);
werror authKey_check (const authKey_t value);

#endif
