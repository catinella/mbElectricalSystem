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
#ifndef WERRCODE
#define WERRCODE

#include <stdint.h>

#define WERRCODE_LAST_SUCCESS 63
#define WERRCODE_LAST_WARNING 127
#define WERRCODE_LAST_ERROR   255

typedef enum {
	WERRCODE_ERROR_GENIRIC    = 0,

	WERRCODE_SUCCESS          = 1,

	WERRCODE_WARNING_RESBUSY  = 65,
	WERRCODE_WARNING_EMPTYLST = 67,
	
	WERRCODE_ERROR_OUTOFMEM   = 129

} werror;


static inline bool wErrCode_isSuccess (werror ec) {return((ec>0                     && ec<=WERRCODE_LAST_SUCCESS));}
static inline bool wErrCode_isWarning (werror ec) {return((ec>WERRCODE_LAST_SUCCESS && ec<=WERRCODE_LAST_WARNING));}
static inline bool ErrCode_isError   (werror ec) {return((ec>WERRCODE_LAST_WARNING && ec<=WERRCODE_LAST_ERROR));  }

// [!] The inline optimization has meaning just in GNU-C not in ANSI-C
//     DO NOT REMOVE static, it can cause un-predictable behavior

#endif
