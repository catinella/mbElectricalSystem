/*------------------------------------------------------------------------------------------------------------------------------
//
//   __  __       _             _     _ _          _____ _           _        _           _   ____            _                 
// |  \/  | ___ | |_ ___  _ __| |__ (_) | _____  | ____| | ___  ___| |_ _ __(_) ___ __ _| | / ___| _   _ ___| |_ ___ _ __ ___  
// | |\/| |/ _ \| __/ _ \| '__| '_ \| | |/ / _ \ |  _| | |/ _ \/ __| __| '__| |/ __/ _` | | \___ \| | | / __| __/ _ \ '_ ` _ \ 
// | |  | | (_) | || (_) | |  | |_) | |   <  __/ | |___| |  __/ (__| |_| |  | | (_| (_| | |  ___) | |_| \__ \ ||  __/ | | | | |
// |_|  |_|\___/ \__\___/|_|  |_.__/|_|_|\_\___| |_____|_|\___|\___|\__|_|  |_|\___\__,_|_| |____/ \__, |___/\__\___|_| |_| |_|
//                                                                                                 |___/                       
//
// File:   iInputInterface.h
//
// Author: Silvano Catinella <catinella@yahoo.com>
//
// Description:
//	This module is responsible on the input controls (eg. buttons, switches..) management
//
//
//	Symbols:
//	========
//		IINPUTIF_DEBOUNCETIME    number of round where the interface will be disabled
//		IINPUTIF_MAXITEMSNUMB    maximum number of allowed interfaces
//
//	Error codes convention:
//	=======================
//		+--------+-----------------------------------------------------+
//		| Codes  | Description                                         |
//		+--------+-----------------------------------------------------+
//		|      0 | Generic error                                       |
//		|      1 | Success                                             |
//		|  32-63 | Information (it is associated to a success status)  |
//		| 64-127 | Warning                                             |
//		|   >127 | Specific error                                      |
//		+--------+-----------------------------------------------------+
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
#ifndef IINPUTIF
#define IINPUTIF

#include <stdbool.h>
#include <stdint.h>
#include "../../werror/include/werror.h"
#include "moduleDB.h"

#define IINPUTIF_DEBOUNCETIME 10


//------------------------------------------------------------------------------------------------------------------------------
//                                                  F U N C T I O N S 
//------------------------------------------------------------------------------------------------------------------------------
werror iInputInterface_init ();
werror iInputInterface_new  (uint8_t *inputID, iInputType_t type, int8_t pin);
werror iInputInterface_get  (uint8_t inputID, bool *status);

#endif
