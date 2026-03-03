/*------------------------------------------------------------------------------------------------------------------------------
//
//   __  __       _             _     _ _          _____ _           _        _           _   ____            _                 
// |  \/  | ___ | |_ ___  _ __| |__ (_) | _____  | ____| | ___  ___| |_ _ __(_) ___ __ _| | / ___| _   _ ___| |_ ___ _ __ ___  
// | |\/| |/ _ \| __/ _ \| '__| '_ \| | |/ / _ \ |  _| | |/ _ \/ __| __| '__| |/ __/ _` | | \___ \| | | / __| __/ _ \ '_ ` _ \ 
// | |  | | (_) | || (_) | |  | |_) | |   <  __/ | |___| |  __/ (__| |_| |  | | (_| (_| | |  ___) | |_| \__ \ ||  __/ | | | | |
// |_|  |_|\___/ \__\___/|_|  |_.__/|_|_|\_\___| |_____|_|\___|\___|\__|_|  |_|\___\__,_|_| |____/ \__, |___/\__\___|_| |_| |_|
//                                                                                                 |___/                       
//
// File:   moduleDB.h
//
// Author: Silvano Catinella <catinella@yahoo.com>
//
// Description:
//	This source file provides a simple data-storage service with concurrent accesses management
//
//	Symbols:
//	========
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
#ifndef MODULEDB
#define MODULEDB

#include <stdbool.h>
#include <stdint.h>
#include <stddef.h>
#include "../../werror/include/werror.h"


typedef enum {
	BUTTON,
	HOLDBUTTON,
	SWITCH
} iInputType_t;

typedef enum {
	MODULEDB_READ,
	MODULEDB_WRITE
 } moduleDBopType_t;

typedef struct {
	int8_t       pinID;
	iInputType_t type;
	uint32_t     timerOffset;  // It allows the timer to count for about 1000 hours!!!! 
	bool         status;
	uint8_t      FSM;
} iInputItem_t;


#define MODULEDB_MAXITEMSNUMB 64
#define MODULEDB_TIMEOUT      100/portTICK_PERIOD_MS

werror moduleDB_add  (uint8_t *inputID, iInputItem_t item);
werror moduleDB_rw   (uint8_t inputID,  iInputItem_t *item, moduleDBopType_t op);
werror moduleDB_iter (uint8_t *inputID, iInputItem_t *item);

#endif
